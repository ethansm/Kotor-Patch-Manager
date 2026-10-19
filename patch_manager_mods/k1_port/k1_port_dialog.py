#!/usr/bin/env python3
"""K1 conversation -> K2 dialog converter for the install-time character importer (doc 38 s26, doc 41 s6).

An NPC's K1 conversation (DLG), its voice-over and its lip files are ripped from the K1 install and served by the
character pack. The graph logic here is pure stdlib (testable without pykotor); the pykotor reads/writes live in a
backend class used only by the hidden `_dialog` helper that the driver runs under the k1port venv python.

Plain graph (JSON-able), the only thing the pure code sees:
  {"entries": [node], "replies": [node], "starters": [link], "root": {...}}
  node = {"strref", "text", "vo", "speaker", "listener", "script", "script2", "sound", "quest", "delay", "wait_flags",
          "camera_angle", "camera_id", "camera_anim", "camera_effect", "camera_fov", "camera_height", "target_height",
          "fade_type", "fade_delay", "fade_length", "animations": [{"animation", "participant"}], "links": [link]}
  link = {"index", "active", "active2", "is_child"}        (index = position in the entries/replies list)
convert_dialog() returns the K2-safe graph (same schema, stripped fields), a report and the assets.

POLICY (verified facts, 10-18): the K1 text strrefs do not match the K2 dialog.tlk, so every node text is embedded
literally (stringref -1 + the K1 TLK English string; the engine shows inline text and animates). The K1 condition and
action scripts do not exist in K2, so ALL script references are stripped and only what is reachable without a
condition is kept: a starter or link with an Active/Active2 script is dropped (counted, deterministic). Because the
engine plays the FIRST starter (and the first reply->entry link) whose condition passes, every starter or reply->entry
link after the first unconditional one is unreachable and is dropped as `shadowed` (it would only burn the voice budget).
Quest/journal/plot-XP fields, node Sound, camera video effects, static camera ids and animated cameras, ambient track,
camera model, stunts and the end/abort scripts are removed; Speaker, Listener, AnimList, timing and the camera angle
fields K2 reads are kept (camera angle > 3 needs a static/animated camera: reset to 0).

Voice (budgeted: the asset pool is shared by every pack): breadth-first from the kept starters, a node whose VO_ResRef
has a wav in <K1>/streamwaves/ gets voiced until `max_voiced` distinct VO names are spent; every later node keeps its
text with an empty VO_ResRef (silent). The wav bytes are copied unchanged (58-byte container + MP3). A lip is served only
when it passes kvoice_lip.check_lip (a corrupt lip crashed the game at 0x0091CF5B); a missing or failing lip means that
line ships without a lip. VO names that are valid resrefs ([a-z0-9_]{1,16}, trailing underscore allowed) keep their
lowercase name; any other name becomes `kv` + 14 hex of sha1(lowercase name), consistently in the DLG, wav and lip.

Helper (driver: `_dialog`, k1port venv python): read K1DIR TLKFILE RESREF | lips K1DIR NAME... | write GRAPH_JSON.
Each prints one JSON object as its LAST stdout line.
"""
from __future__ import annotations

import base64
import hashlib
import json
import os
import re
import sys
from collections import deque
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "kvoice"))
import kvoice_lip  # noqa: E402  stdlib only

RESREF_RE = re.compile(r"[a-z0-9_]{1,16}")
DEFAULT_MAX_VOICED = 30
MAX_VOICED_LIMIT = 80
SAFE_CAMERA_ANGLES = (0, 1, 2, 3)  # 4+ need a static/animated camera of the K1 area
POLICY = ("scripts stripped; links/starters with a condition script dropped; links after the first unconditional "
          "starter/reply->entry link dropped (shadowed); text embedded literally from the K1 TLK; voice budgeted breadth-first")
SHAPE_NODE_KEYS = ("strref", "text", "vo", "speaker", "listener", "script", "script2", "sound", "quest", "delay",
                   "wait_flags", "camera_angle", "camera_id", "camera_anim", "camera_effect", "camera_fov",
                   "camera_height", "target_height", "fade_type", "fade_delay", "fade_length")


class DialogError(Exception):
    """A problem with the K1 conversation or its inputs. str() = location: cause. Fix: ..."""

    def __init__(self, location: str, cause: str, fix: str = ""):
        self.location, self.cause, self.fix = location, cause, fix
        super().__init__(f"{location}: {cause}" + (f". Fix: {fix}" if fix else ""))


# ------------------------------------------------------------------------------------------- voice names + index

def build_vo_index(root) -> tuple[dict, dict]:
    """One recursive, case-insensitive index of the K1 streamwaves tree: ({lowercase stem: Path}, info). Symlinked
    directories and files are never followed or indexed; when two files share a stem the first in sorted path order wins
    (the others are listed in info["duplicates"]). A missing dir is an empty index."""
    index: dict[str, Path] = {}
    info = {"files": 0, "duplicates": [], "skipped_symlinks": 0}
    root = Path(root)
    if root.is_symlink() or not root.is_dir():
        return index, info
    for dirpath, dirs, files in os.walk(root, followlinks=False):
        keep = []
        for d in sorted(dirs):
            if os.path.islink(os.path.join(dirpath, d)):
                info["skipped_symlinks"] += 1
            else:
                keep.append(d)
        dirs[:] = keep
        for fn in sorted(files):
            if not fn.lower().endswith(".wav"):
                continue
            full = Path(dirpath) / fn
            if full.is_symlink():
                info["skipped_symlinks"] += 1
                continue
            info["files"] += 1
            stem = fn[:-4].lower()
            if stem in index:
                info["duplicates"].append(stem)
            else:
                index[stem] = full
    return index, info


def rename_vo(orig: str, taken) -> str:
    """The K2 name of a K1 VO_ResRef: itself (lowercase) when it is a valid resref, else kv + 14 hex of sha1."""
    low = orig.strip().lower()
    taken = set(taken)
    if RESREF_RE.fullmatch(low) and low not in taken:
        return low
    salt = 0
    while True:
        seed = low if salt == 0 else f"{low}#{salt}"
        out = "kv" + hashlib.sha1(seed.encode("utf-8", "replace")).hexdigest()[:14]
        if out not in taken:
            return out
        salt += 1


def _load_bytes(value) -> bytes:
    if isinstance(value, (bytes, bytearray)):
        return bytes(value)
    try:
        return Path(value).read_bytes()
    except OSError:
        return b""


# ------------------------------------------------------------------------------------------- graph walk

def _conditional(link: dict) -> bool:
    return bool(str(link.get("active") or "").strip() or str(link.get("active2") or "").strip())


def _select(links, target_kind: str, target_len: int, drop_cond: bool, shadow: bool):
    """(kept links, stats) of one link list in order. A link whose target index is out of range is `bad`. With drop_cond a
    conditional link is dropped; with shadow every link after the first KEPT link to an entry is dropped (the engine takes
    the first entry whose condition passes)."""
    kept, st, shadowed = [], {"cond": 0, "shadow": 0, "bad": 0, "scripts": set()}, False
    for link in links:
        i = link.get("index")
        if not isinstance(i, int) or isinstance(i, bool) or not 0 <= i < target_len:
            st["bad"] += 1
            continue
        if shadowed:
            st["shadow"] += 1
            continue
        if drop_cond and _conditional(link):
            st["cond"] += 1
            st["scripts"].update(s for s in (str(link.get("active") or "").strip().lower(),
                                             str(link.get("active2") or "").strip().lower()) if s)
            continue
        kept.append(link)
        if shadow and target_kind == "e":
            shadowed = True
    return kept, st


class _Walk:
    def __init__(self):
        self.order: list = []        # [("e"|"r", index)] in breadth-first discovery order
        self.starters: list = []     # the kept starter links
        self.links: dict = {}        # {(kind, index): [kept link]}
        self.cond = self.shadow = self.bad = 0
        self.scripts: set = set()
        self.starter_stats: dict = {}


def _walk(graph: dict, drop_cond: bool, shadow: bool) -> _Walk:
    ents, reps = graph["entries"], graph["replies"]
    w = _Walk()
    w.starters, st = _select(graph.get("starters", []), "e", len(ents), drop_cond, shadow)
    w.starter_stats = st
    w.cond, w.shadow, w.bad = st["cond"], st["shadow"], st["bad"]
    w.scripts |= st["scripts"]
    seen, q = set(), deque()
    for link in w.starters:
        key = ("e", link["index"])
        if key not in seen:
            seen.add(key)
            q.append(key)
    while q:
        key = q.popleft()
        w.order.append(key)
        node = (ents if key[0] == "e" else reps)[key[1]]
        tk = "r" if key[0] == "e" else "e"
        sel, st = _select(node.get("links", []), tk, len(reps) if tk == "r" else len(ents), drop_cond, shadow)
        w.links[key] = sel
        w.cond += st["cond"]
        w.shadow += st["shadow"]
        w.bad += st["bad"]
        w.scripts |= st["scripts"]
        for link in sel:
            k = (tk, link["index"])
            if k not in seen:
                seen.add(k)
                q.append(k)
    return w


def _resolve_text(node: dict, tlk, missing: list) -> tuple[str, str]:
    """(text, source). The K1 TLK English string when the node has a strref, else the text embedded in the DLG."""
    ref = node.get("strref", -1)
    if isinstance(ref, int) and not isinstance(ref, bool) and ref >= 0:
        got = tlk(ref)
        if got is not None:
            return str(got), "tlk"
        missing.append(ref)
    emb = str(node.get("text") or "")
    return (emb, "embedded") if emb else ("", "empty")


def _count_links(graph: dict) -> int:
    return len(graph.get("starters", [])) + sum(len(n.get("links", [])) for n in graph["entries"] + graph["replies"])


# ------------------------------------------------------------------------------------------- conversion

def convert_dialog(graph: dict, tlk, vo_index, lip_lookup, *, max_voiced: int = DEFAULT_MAX_VOICED,
                   k2_dlg: str = "dialog", taken_names=(), write_dlg=None, shadow: bool = True) -> dict:
    """Convert a plain K1 graph. `tlk(strref) -> str | None`; `vo_index` {lowercase stem: Path | bytes}; `lip_lookup(names)
    -> {name: bytes | None}` is called once with the voiced K1 names; `write_dlg(out_graph) -> bytes` (optional; without it
    the assets hold no dlg but asset_count still counts it). Returns {"graph", "report", "assets": [(name, bytes, kind)],
    "voiced": [...]} with kind one of dlg | wav | lip. Raises DialogError when nothing survives."""
    if isinstance(max_voiced, bool) or not isinstance(max_voiced, int) or not 0 <= max_voiced <= MAX_VOICED_LIMIT:
        raise DialogError("dialog.max_voiced", f"{max_voiced!r} is not an integer 0..{MAX_VOICED_LIMIT}", "fix the recipe")
    ents, reps = graph["entries"], graph["replies"]
    everything = _walk(graph, False, False)
    by_cond = _walk(graph, True, False)
    kept = _walk(graph, True, shadow)
    if not kept.starters:
        raise DialogError("K1 conversation", f"none of its {len(graph.get('starters', []))} starter(s) is free of a condition "
                          "script, so nothing survives the script-stripping policy",
                          "this NPC's dialog needs K1 scripts; remove the [dialog] table")
    keep_e = sorted(i for k, i in kept.order if k == "e")
    keep_r = sorted(i for k, i in kept.order if k == "r")
    new_e = {old: n for n, old in enumerate(keep_e)}
    new_r = {old: n for n, old in enumerate(keep_r)}
    remap = {"e": new_e, "r": new_r}

    # voice, breadth-first over the kept graph
    voiced: dict[str, dict] = {}   # K1 name (lower) -> {"name", "data"}
    node_vo: dict = {}             # (kind, index) -> K2 vo name or ""
    taken = set(taken_names)
    nodes_with_vo = silent_no_wav = silent_budget = 0
    no_wav_names: list[str] = []
    for key in kept.order:
        node = (ents if key[0] == "e" else reps)[key[1]]
        orig = str(node.get("vo") or "").strip().lower()
        node_vo[key] = ""
        if not orig:
            continue
        nodes_with_vo += 1
        if orig in voiced:
            node_vo[key] = voiced[orig]["name"]
            continue
        src = vo_index.get(orig)
        data = _load_bytes(src) if src is not None else b""
        if not data:
            silent_no_wav += 1
            if orig not in no_wav_names:
                no_wav_names.append(orig)
            continue
        if len(voiced) >= max_voiced:
            silent_budget += 1
            continue
        name = rename_vo(orig, taken | {v["name"] for v in voiced.values()})
        voiced[orig] = {"name": name, "data": data}
        node_vo[key] = name

    # lips for the voiced names (one batch)
    got = lip_lookup(list(voiced)) if voiced else {}
    lips: dict[str, bytes] = {}
    lips_missing: list[str] = []
    lips_failed: list[dict] = []
    for orig, v in voiced.items():
        data = got.get(orig)
        if not data:
            lips_missing.append(v["name"])
            v["lip"] = "missing"
            continue
        problems = kvoice_lip.check_lip(data)
        if problems:
            lips_failed.append({"name": v["name"], "problems": problems})
            v["lip"] = "failed"
            continue
        lips[v["name"]] = data
        v["lip"] = "ok"

    # the K2-safe graph
    text_stats = {"tlk": 0, "embedded": 0, "empty": 0}
    tlk_missing: list = []
    stripped_scripts = stripped_other = camera_reset = 0
    speakers, participants = set(), set()

    def out_node(kind: str, idx: int) -> dict:
        nonlocal stripped_scripts, stripped_other, camera_reset
        n = (ents if kind == "e" else reps)[idx]
        text, src = _resolve_text(n, tlk, tlk_missing)
        text_stats[src] += 1
        if str(n.get("script") or "").strip() or str(n.get("script2") or "").strip():
            stripped_scripts += 1
        if (str(n.get("sound") or "").strip() or str(n.get("quest") or "").strip() or n.get("camera_effect") is not None
                or n.get("camera_id") is not None or n.get("camera_anim") is not None):
            stripped_other += 1
        angle = n.get("camera_angle", 0) or 0
        if angle not in SAFE_CAMERA_ANGLES:
            camera_reset += 1
            angle = 0
        if kind == "e" and str(n.get("speaker") or ""):
            speakers.add(str(n["speaker"]))
        anims = [{"animation": int(a.get("animation", 0)), "participant": str(a.get("participant") or "")}
                 for a in n.get("animations", [])]
        for a in anims:
            if a["participant"] and a["participant"].upper() != "OWNER":
                participants.add(a["participant"])
        tk = "r" if kind == "e" else "e"
        return {"text": text, "vo": node_vo[(kind, idx)], "speaker": str(n.get("speaker") or "") if kind == "e" else "",
                "listener": str(n.get("listener") or ""), "delay": int(n.get("delay", -1)),
                "wait_flags": int(n.get("wait_flags", 0) or 0), "camera_angle": int(angle),
                "camera_fov": n.get("camera_fov"), "camera_height": n.get("camera_height"),
                "target_height": n.get("target_height"), "fade_type": int(n.get("fade_type", 0) or 0),
                "fade_delay": n.get("fade_delay"), "fade_length": n.get("fade_length"), "animations": anims,
                "links": [{"index": remap[tk][l["index"]], "is_child": bool(l.get("is_child"))}
                          for l in kept.links.get((kind, idx), [])]}

    root_in = graph.get("root", {})
    out_graph = {
        "entries": [out_node("e", i) for i in keep_e],
        "replies": [out_node("r", i) for i in keep_r],
        "starters": [{"index": new_e[l["index"]]} for l in kept.starters],
        "root": {"skippable": bool(root_in.get("skippable")), "old_hit_check": bool(root_in.get("old_hit_check")),
                 "unequip_items": bool(root_in.get("unequip_items")), "unequip_hands": bool(root_in.get("unequip_hands")),
                 "conversation_type": int(root_in.get("conversation_type", 0) or 0),
                 "computer_type": int(root_in.get("computer_type", 0) or 0)},
    }
    dlg_bytes = write_dlg(out_graph) if write_dlg is not None else None

    assets: list[tuple[str, bytes, str]] = []
    if dlg_bytes is not None:
        assets.append((f"{k2_dlg}.dlg", dlg_bytes, "dlg"))
    assets += [(f"{v['name']}.wav", v["data"], "wav") for v in sorted(voiced.values(), key=lambda v: v["name"])]
    assets += [(f"{n}.lip", d, "lip") for n, d in sorted(lips.items())]

    total = len(ents) + len(reps)
    st_all = len(graph.get("starters", []))
    unconditional = sum(1 for l in graph.get("starters", []) if not _conditional(l))
    report = {
        "policy": POLICY,
        "nodes_total": total, "entries_total": len(ents), "replies_total": len(reps),
        "nodes_kept": len(keep_e) + len(keep_r), "entries_kept": len(keep_e), "replies_kept": len(keep_r),
        "nodes_reachable": len(everything.order),
        "nodes_dropped": total - len(keep_e) - len(keep_r),
        "nodes_dropped_by_condition": len(everything.order) - len(by_cond.order),
        "nodes_dropped_shadowed": len(by_cond.order) - len(kept.order),
        "nodes_unreachable": total - len(everything.order),
        "links_total": _count_links(graph),
        "links_kept": len(kept.starters) + sum(len(v) for v in kept.links.values()),
        "links_dropped_by_condition": kept.cond, "links_dropped_shadowed": kept.shadow, "links_bad": kept.bad,
        "starters_total": st_all, "starters_unconditional": unconditional, "starters_kept": len(kept.starters),
        "condition_scripts_dropped": sorted(kept.scripts),
        "nodes_with_script_stripped": stripped_scripts, "nodes_with_other_fields_stripped": stripped_other,
        "camera_angle_reset": camera_reset,
        "text_from_tlk": text_stats["tlk"], "text_embedded": text_stats["embedded"], "text_empty": text_stats["empty"],
        "tlk_missing": sorted(set(tlk_missing)),
        "speakers": sorted(speakers), "animation_participants": sorted(participants),
        "voice_budget": max_voiced, "nodes_with_vo": nodes_with_vo,
        "voiced_lines": len(voiced), "voiced_nodes": sum(1 for v in node_vo.values() if v),
        "silent_no_wav": silent_no_wav, "silent_no_wav_names": no_wav_names, "silent_budget": silent_budget,
        "vo_renamed": sorted(o for o, v in voiced.items() if v["name"] != o),
        "lips_ok": len(lips), "lips_missing": lips_missing, "lips_failed": lips_failed,
        "asset_count": 1 + len(voiced) + len(lips),
        "wav_bytes": sum(len(v["data"]) for v in voiced.values()),
    }
    return {"graph": out_graph, "report": report, "assets": assets,
            "voiced": [{"k1": o, "name": v["name"], "size": len(v["data"]), "lip": v["lip"]} for o, v in voiced.items()]}


def report_lines(rep: dict) -> list[str]:
    """The human summary of convert_dialog()'s report (printed by dry-run and build)."""
    renamed = f", {len(rep['vo_renamed'])} renamed" if rep["vo_renamed"] else ""
    return [
        f"nodes {rep['nodes_total']} total ({rep['entries_total']} entries, {rep['replies_total']} replies): {rep['nodes_kept']} kept, "
        f"{rep['nodes_dropped_by_condition']} dropped by condition, {rep['nodes_dropped_shadowed']} shadowed, "
        f"{rep['nodes_unreachable']} unreachable; links {rep['links_total']}: {rep['links_kept']} kept, "
        f"{rep['links_dropped_by_condition']} dropped by condition, {rep['links_dropped_shadowed']} shadowed",
        f"starters {rep['starters_total']} total, {rep['starters_unconditional']} without a condition, {rep['starters_kept']} kept; "
        f"{len(rep['condition_scripts_dropped'])} distinct condition script(s) dropped; scripts stripped on "
        f"{rep['nodes_with_script_stripped']} node(s), other fields on {rep['nodes_with_other_fields_stripped']}",
        f"text: {rep['text_from_tlk']} from the K1 TLK, {rep['text_embedded']} embedded, {rep['text_empty']} empty"
        + (f", {len(rep['tlk_missing'])} strref(s) missing in the TLK" if rep["tlk_missing"] else ""),
        f"voice: {rep['voiced_lines']}/{rep['voice_budget']} voiced line(s) ({rep['voiced_nodes']} node(s){renamed}), "
        f"{rep['silent_budget']} node(s) silent for the budget, {rep['silent_no_wav']} silent (no wav in streamwaves); "
        f"lips {rep['lips_ok']} ok, {len(rep['lips_missing'])} missing, {len(rep['lips_failed'])} failed; "
        f"{rep['asset_count']} pack asset(s) (dlg + wav + lip)",
    ]


# ------------------------------------------------------------------------------------------- pykotor backend (helper only)

class PykotorBackend:
    """Reads/writes DLGs, TLK strings and lip archives with pykotor. Import-safe without pykotor; every method imports it
    lazily, so use it only under the k1port venv python."""

    @staticmethod
    def _prep():
        import logging
        logging.disable(logging.CRITICAL)
        sys.setrecursionlimit(max(sys.getrecursionlimit(), 20000))

    def read(self, k1, resref: str, tlk_path):
        """(graph, strings {strref: text}, source label, sha256 of the K1 DLG) or None when the K1 install has no such DLG."""
        self._prep()
        import k1_port_textures as kt
        with kt.pykotor_cwd():
            from pykotor.extract.installation import Installation, SearchLocation
            from pykotor.resource.formats.tlk import read_tlk
            from pykotor.resource.generics.dlg import read_dlg
            from pykotor.resource.type import ResourceType
            inst = Installation(str(k1))
            res = None
            for locs in ([SearchLocation.OVERRIDE, SearchLocation.CHITIN], [SearchLocation.MODULES]):
                res = inst.resource(resref, ResourceType.DLG, locs)
                if res is not None:
                    break
            if res is None:
                return None
            dlg = read_dlg(bytes(res.data))
            graph = self._graph(dlg)
            refs = sorted({n["strref"] for n in graph["entries"] + graph["replies"] if n["strref"] >= 0})
            tlk = read_tlk(Path(tlk_path).read_bytes())
            strings = {}
            for ref in refs:
                entry = tlk.get(ref)
                if entry is not None:
                    strings[ref] = str(entry.text)
            return graph, strings, Path(str(res.filepath)).name, hashlib.sha256(bytes(res.data)).hexdigest()

    @staticmethod
    def _graph(dlg) -> dict:
        from pykotor.common.language import Gender, Language
        entries = sorted(dlg.all_entries(), key=lambda n: n.list_index)
        replies = sorted(dlg.all_replies(), key=lambda n: n.list_index)
        pos_e = {id(n): i for i, n in enumerate(entries)}
        pos_r = {id(n): i for i, n in enumerate(replies)}

        def link(l) -> dict:
            node = l.node
            idx = pos_e.get(id(node)) if id(node) in pos_e else pos_r.get(id(node))
            return {"index": -1 if idx is None else idx, "active": str(l.active1), "active2": str(l.active2),
                    "is_child": bool(l.is_child)}

        def node(n) -> dict:
            emb = ""
            try:
                emb = n.text.get(Language.ENGLISH, Gender.MALE, use_fallback=True) or ""
            except Exception:  # noqa: BLE001  an odd locstring: no embedded text
                emb = ""
            return {"strref": int(n.text.stringref), "text": str(emb), "vo": str(n.vo_resref),
                    "speaker": str(getattr(n, "speaker", "") or ""), "listener": str(n.listener),
                    "script": str(n.script1), "script2": str(n.script2), "sound": str(n.sound), "quest": str(n.quest),
                    "delay": int(n.delay), "wait_flags": int(n.wait_flags), "camera_angle": int(n.camera_angle),
                    "camera_id": n.camera_id, "camera_anim": n.camera_anim, "camera_effect": n.camera_effect,
                    "camera_fov": n.camera_fov, "camera_height": n.camera_height, "target_height": n.target_height,
                    "fade_type": int(n.fade_type), "fade_delay": n.fade_delay, "fade_length": n.fade_length,
                    "animations": [{"animation": int(a.animation_id), "participant": str(a.participant)} for a in n.animations],
                    "links": [link(l) for l in n.links]}

        return {"entries": [node(n) for n in entries], "replies": [node(n) for n in replies],
                "starters": [link(l) for l in dlg.starters],
                "root": {"skippable": bool(dlg.skippable), "old_hit_check": bool(dlg.old_hit_check),
                         "unequip_items": bool(dlg.unequip_items), "unequip_hands": bool(dlg.unequip_hands),
                         "conversation_type": int(dlg.conversation_type), "computer_type": int(dlg.computer_type)}}

    def lips(self, k1, names) -> dict:
        """{name: (bytes, mod file name) or None}: every <K1>/lips/*.mod ERF is searched in sorted name order."""
        self._prep()
        from pykotor.resource.formats.erf import read_erf
        want = {n.lower() for n in names}
        found: dict = {}
        lips_dir = _find_child_ci(Path(k1), "lips")
        if lips_dir is not None and lips_dir.is_dir() and not lips_dir.is_symlink():
            for mod in sorted(lips_dir.iterdir(), key=lambda p: p.name.lower()):
                if not want - set(found) or mod.is_symlink() or not mod.is_file() or mod.suffix.lower() != ".mod":
                    continue
                for r in read_erf(mod.read_bytes()):
                    name = r.resref.get().lower()
                    if name in want and name not in found and r.restype.extension.lower() == "lip":
                        found[name] = (bytes(r.data), mod.name)
        return {n: found.get(n.lower()) for n in names}

    def write(self, graph: dict) -> bytes:
        """The K2-safe DLG bytes, built the way k1_port_audio_test_build.build_dlg builds the hand-path test dialog (inline
        English text, stringref -1, no scripts) with the quest/plot fields neutral."""
        self._prep()
        from pykotor.common.language import LocalizedString
        from pykotor.common.misc import ResRef
        from pykotor.resource.generics.dlg import (DLG, DLGAnimation, DLGComputerType, DLGConversationType, DLGEntry, DLGLink,
                                                   DLGReply, bytes_dlg)
        d = DLG()
        r = graph.get("root", {})
        d.skippable, d.old_hit_check = bool(r.get("skippable")), bool(r.get("old_hit_check"))
        d.unequip_items, d.unequip_hands = bool(r.get("unequip_items")), bool(r.get("unequip_hands"))
        d.conversation_type = DLGConversationType(int(r.get("conversation_type", 0)))
        d.computer_type = DLGComputerType(int(r.get("computer_type", 0)))
        ents = [DLGEntry() for _ in graph["entries"]]
        reps = [DLGReply() for _ in graph["replies"]]

        def fill(node, spec, idx):
            node.list_index = idx
            node.text = LocalizedString.from_english(spec["text"])
            node.vo_resref = ResRef(spec["vo"]) if spec.get("vo") else ResRef.from_blank()
            node.listener = spec.get("listener", "")
            node.delay = int(spec.get("delay", -1))
            node.wait_flags = int(spec.get("wait_flags", 0))
            node.camera_angle = int(spec.get("camera_angle", 0))
            node.camera_id = None
            node.camera_fov, node.camera_height = spec.get("camera_fov"), spec.get("camera_height")
            node.target_height = spec.get("target_height")
            node.fade_type = int(spec.get("fade_type", 0))
            node.fade_delay, node.fade_length = spec.get("fade_delay"), spec.get("fade_length")
            node.plot_index, node.plot_xp_percentage = -1, 0.0
            for a in spec.get("animations", []):
                anim = DLGAnimation()
                anim.animation_id, anim.participant = int(a["animation"]), a.get("participant", "")
                node.animations.append(anim)

        for i, (node, spec) in enumerate(zip(ents, graph["entries"])):
            fill(node, spec, i)
            node.speaker = spec.get("speaker", "")
            for j, l in enumerate(spec["links"]):
                link = DLGLink(reps[l["index"]], j)
                link.is_child = bool(l.get("is_child"))
                node.links.append(link)
        for i, (node, spec) in enumerate(zip(reps, graph["replies"])):
            fill(node, spec, i)
            for j, l in enumerate(spec["links"]):
                link = DLGLink(ents[l["index"]], j)
                link.is_child = bool(l.get("is_child"))
                node.links.append(link)
        for j, l in enumerate(graph["starters"]):
            d.starters.append(DLGLink(ents[l["index"]], j))
        return bytes_dlg(d)


def _find_child_ci(parent: Path, name: str):
    try:
        with os.scandir(parent) as it:
            for e in it:
                if e.name.lower() == name.lower():
                    return Path(e.path)
    except OSError:
        return None
    return None


# ------------------------------------------------------------------------------------------- hidden helper

def helper_main(argv: list, backend_factory=None) -> int:
    """Hidden helper (driver: `_dialog`), run under the k1port venv python. ops:
       read K1DIR TLKFILE RESREF  -> {"found": bool, "graph", "strings": {strref: text}, "source", "sha256"}
       lips K1DIR NAME...         -> {"lips": {name: {"b64", "mod"} | null}}
       write GRAPH_JSON           -> {"data_b64"}
    The JSON object is the LAST stdout line (pykotor may log above it). `backend_factory()` is for tests."""
    op = argv[0] if argv else ""
    try:
        backend = (backend_factory or PykotorBackend)()
        if op == "read":
            got = backend.read(Path(argv[1]), argv[3], Path(argv[2]))
            if got is None:
                print(json.dumps({"found": False}))
            else:
                graph, strings, source, sha = got
                print(json.dumps({"found": True, "graph": graph, "strings": {str(k): v for k, v in strings.items()},
                                  "source": source, "sha256": sha}))
        elif op == "lips":
            got = backend.lips(Path(argv[1]), argv[2:])
            print(json.dumps({"lips": {n: (None if v is None else {"b64": base64.b64encode(v[0]).decode("ascii"), "mod": v[1]})
                                       for n, v in got.items()}}))
        elif op == "write":
            graph = json.loads(Path(argv[1]).read_text(encoding="utf-8"))
            print(json.dumps({"data_b64": base64.b64encode(backend.write(graph)).decode("ascii")}))
        else:
            print(f"unknown _dialog op {op!r}", file=sys.stderr)
            return 2
    except Exception as e:  # noqa: BLE001  pykotor raises several error types
        print(f"error: _dialog {op}: {type(e).__name__}: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(helper_main(sys.argv[1:]))
