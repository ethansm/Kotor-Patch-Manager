#!/usr/bin/env python3
"""Recipe auto-derivation and NPC discovery for the install-time K1 -> K2 character importer (doc 38 s26).

The user picks ANY K1 creature template (a UTC resref); this module derives the recipe the committed hand recipes
(recipes/carth.toml, recipes/juhani.toml) were written by hand from. Read only on the K1 install.

  facts   = gather_facts(reader, "p_carth")        K1 UTC + appearance.2da + heads.2da + soundset.2da + MDL texture scan
  derived = derive(facts)                          pure: names, textures, audio, defaults  -> Derived
  text    = render(derived)                        recipe TOML (deterministic: same facts, same bytes)
  rows    = list_npcs(reader)                      every K1 creature template with the columns `npcs` prints

Where each value comes from (verified against the real K1 data for p_carth and p_juhani, 10-18):
  UTC (templates.bif / Override / module rims)   appearance_id, soundset_id, gender_id, conversation, tag, first_name
  appearance.2da row <appearance_id>             modeltype (B = body + head), modela / modelb (body models), normalhead
                                                 (a heads.2da row id). The texa/texb columns are NOT used: the textures
                                                 are read from the meshes themselves (every texture a mesh references).
  heads.2da row <normalhead>                     column "head" = the head model
  soundset.2da row <soundset_id>                 column "resref" = the K1 SSF (e.g. p_carth)
The `reader` is an object with find_utc / iter_utcs / twoda_row / mdl_textures / ssf_exists / texture_exists / dlg_exists (see
PykotorReader, which runs under the k1port venv python via the driver's hidden `_scan` helper; the tests use a fixture
reader). The pure functions here are stdlib only.
"""
from __future__ import annotations

import json
import re
import sys
import unicodedata
from dataclasses import dataclass, field
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import k1_port_recipe as krecipe  # noqa: E402  stdlib only

PREFIX = "k1p_"                 # every k2 name the auto recipes introduce starts with this
RESREF_MAX = 16
TEMPLATE_ROW = 17               # appearance.2da row copied for the new row (same as the hand recipes)
SOUNDSET_FALLBACK = {0: 11, 1: 57}  # K2 soundset.2da n_commm / n_commonf, by gender
NO_SOUNDSET = (0xFFFF, 0xFFFFFFFF, -1)  # UTC "no soundset" values
HEAD_TXI = "clamp 1"            # head textures, like the hand recipes
AUTO_DIR = "recipes_auto"       # <out>/recipes_auto/<id>.toml


class ScanError(Exception):
    """A problem with the K1 data or the derived names. str() = location: cause. Fix: ..."""

    def __init__(self, location: str, cause: str, fix: str = ""):
        self.location, self.cause, self.fix = location, cause, fix
        super().__init__(f"{location}: {cause}" + (f". Fix: {fix}" if fix else ""))


# ------------------------------------------------------------------------------------------- naming (pure)

def auto_id(resref: str) -> str:
    """Recipe id of a `utc:<resref>` config entry: the resref without a leading `p_` (p_carth -> carth)."""
    r = resref.lower()
    if r.startswith("p_") and len(r) > 2:
        r = r[2:]
    return r


def k2_stem(k1_name: str) -> str:
    """The K2 resref of a K1 model/texture: `k1p_` + the K1 name without a leading `p_`, cut to 16 characters.
    p_carthba -> k1p_carthba, p_carthba01 -> k1p_carthba01, n_commoner_a_m01 -> k1p_n_commoner_a (truncated)."""
    stem = k1_name.lower()
    if stem.startswith("p_") and len(stem) > 2:
        stem = stem[2:]
    return (PREFIX + stem)[:RESREF_MAX]


def utc_name(rid: str) -> str:
    """K2 UTC resref / tag base of a recipe id: `k1p_<id>` cut to 16 characters."""
    return (PREFIX + rid)[:RESREF_MAX]


def ssf_name(utc: str) -> str:
    """K2 SSF resref: `<k2 utc>ssf` within 16 characters (the utc part is cut, never the suffix)."""
    return utc[:RESREF_MAX - 3] + "ssf"


def dlg_name(utc: str) -> str:
    """K2 DLG resref: `<k2 utc>dlg` within 16 characters (the utc part is cut, never the suffix)."""
    return utc[:RESREF_MAX - 3] + "dlg"


def claim_names(pairs: list[tuple[str, str]]) -> dict[str, str]:
    """pairs = [(k1 label, k2 name)] in order. Returns {k2: k1}. The same k1 label may repeat with the same k2 name;
    two different k1 labels on one k2 name (truncation clash) raise ScanError with a hint."""
    owner: dict[str, str] = {}
    for k1, k2 in pairs:
        if k2 in owner and owner[k2] != k1:
            raise ScanError(f"k2 name {k2!r}", f"two K1 names map to it: {owner[k2]!r} and {k1!r} (K2 resrefs are cut to "
                            f"{RESREF_MAX} characters)",
                            "run `scan --write DIR`, rename one k2 name in the written recipe by hand and put that file in "
                            "[build] recipes_dir (a recipe file beats auto-derivation)")
        owner[k2] = k1
    return owner


def ascii_name(text: str, fallback: str) -> str:
    """Printable ASCII display name <= 63 chars (accents folded, anything else dropped); `fallback` when nothing is left."""
    folded = unicodedata.normalize("NFKD", text or "").encode("ascii", "ignore").decode("ascii")
    folded = "".join(c for c in folded if 0x20 <= ord(c) <= 0x7E).strip()
    return folded[:63] or fallback


def _clean(value) -> str:
    s = "" if value is None else str(value).strip()
    return "" if s == "****" else s


# ------------------------------------------------------------------------------------------- facts

def gather_facts(reader, resref: str) -> dict | None:
    """Everything the derivation needs, as a JSON-serializable dict; None when the K1 install has no such creature
    template. Never raises for missing 2DA rows or models (the facts say what is missing; derive() decides)."""
    utc = reader.find_utc(resref)
    if utc is None:
        return None
    facts = {k: utc.get(k) for k in ("utc", "source", "tag", "first_name", "last_name", "appearance_id", "soundset_id",
                                     "gender_id", "conversation")}
    facts["utc"] = resref.lower()
    app_id = _int(facts.get("appearance_id"))
    facts["appearance"] = _row(reader, "appearance", app_id)
    app = facts["appearance"]
    facts["head"] = None
    if app is not None:
        nh = _clean(app.get("normalhead"))
        if nh.isdigit():
            hrow = _row(reader, "heads", int(nh))
            if hrow is not None:
                facts["head"] = {"row": int(nh), "head": _clean(hrow.get("head")).lower()}
    ss_id = _int(facts.get("soundset_id"))
    facts["soundset"] = None if ss_id is None or ss_id in NO_SOUNDSET else _row(reader, "soundset", ss_id)
    ssf = _clean((facts["soundset"] or {}).get("resref")).lower()
    facts["ssf_found"] = bool(ssf) and bool(reader.ssf_exists(ssf))
    conv = _clean(facts.get("conversation")).lower()
    dlg_exists = getattr(reader, "dlg_exists", None)  # a reader without it (older doubles) finds no DLG
    facts["dlg_found"] = bool(conv) and krecipe.RESREF_RE.fullmatch(conv) is not None and dlg_exists is not None and bool(dlg_exists(conv))
    models = {}
    names: list[str] = []
    if app is not None and _clean(app.get("modeltype")).upper() == "B":
        names = [_clean(app.get("modela")).lower(), _clean(app.get("modelb")).lower()]
    if facts["head"] and facts["head"]["head"]:
        names.append(facts["head"]["head"])
    for name in names:
        if not name or name in models:
            continue
        tex = reader.mdl_textures(name)
        if tex is None:
            models[name] = {"found": False, "textures": [], "missing_textures": []}
            continue
        have = [t for t in tex if reader.texture_exists(t)]
        models[name] = {"found": True, "textures": have, "missing_textures": [t for t in tex if t not in have]}
    facts["models"] = models
    return facts


def _int(v):
    try:
        return int(v)
    except (TypeError, ValueError):
        return None


def _row(reader, table: str, idx):
    if idx is None or idx < 0:
        return None
    row = reader.twoda_row(table, idx)
    return None if row is None else {str(k).lower(): _clean(v) for k, v in row.items()}


# ------------------------------------------------------------------------------------------- derivation

@dataclass
class Derived:
    id: str
    utc_src: str                      # the K1 creature template resref
    source: str                       # where it was found
    display_name: str
    tag: str
    utc: str
    dlg: str                          # K1 conversation resref
    soundset: int
    template_row: int
    gender: int
    body_a: tuple[str, str]
    body_b: tuple[str, str] | None
    head: tuple[str, str]
    textures: list[tuple[str, str, str]] = field(default_factory=list)  # (k1, k2, txi)
    audio: tuple[str, str] | None = None
    dialog: tuple[str, str] | None = None  # (k1 dlg, k2 dlg) when the conversation exists as a K1 DLG: the recipe gets [dialog]
    k1_rows: str = ""                 # one comment line: the K1 rows the values came from
    notes: list[str] = field(default_factory=list)


def derive(facts: dict, rid: str | None = None) -> Derived:
    """Facts -> Derived recipe. Raises ScanError (with a fix) for a creature the importer cannot build."""
    utc_src = facts["utc"]
    rid = rid or auto_id(utc_src)
    if not krecipe.ID_RE.fullmatch(rid):
        raise ScanError("--id", f"bad id {rid!r}: must match [a-z0-9_]{{1,31}}", "pick another id")
    app = facts.get("appearance")
    where = f"K1 {utc_src}.utc"
    if app is None:
        raise ScanError(where, f"appearance_type {facts.get('appearance_id')} is not a row of the K1 appearance.2da",
                        "pick another creature template (npcs --grep NAME)")
    mtype = _clean(app.get("modeltype")).upper()
    label = _clean(app.get("label")) or "?"
    if mtype != "B":
        raise ScanError(where, f"appearance row {facts['appearance_id']} ({label}) has model type {mtype or '-'!r}, not B "
                        "(body + head); the importer builds characters with a head model",
                        "pick a humanoid creature template (npcs lists the type column)")
    body_names = [n for n in (_clean(app.get("modela")).lower(), _clean(app.get("modelb")).lower()) if n]
    if not body_names:
        raise ScanError(where, f"appearance row {facts['appearance_id']} ({label}) has no modela/modelb body model",
                        "pick another creature template")
    head = (facts.get("head") or {}).get("head") or ""
    if not head:
        raise ScanError(where, f"appearance row {facts['appearance_id']} ({label}) has no usable heads.2da row "
                        f"(normalhead {_clean(app.get('normalhead')) or '-'})", "pick a creature template with a head")
    models = facts.get("models", {})
    for m in body_names + [head]:
        if not krecipe.RESREF_RE.fullmatch(m):
            raise ScanError(where, f"model name {m!r} is not a valid resref", "pick another creature template")
        if not models.get(m, {}).get("found"):
            raise ScanError(where, f"K1 model {m}.mdl is not in the K1 install", "check [k1] dir is a full KOTOR 1 install")

    # textures: every texture each mesh uses, in body_a, body_b, head order; the head mesh's own get the head txi
    head_tex = set(models[head]["textures"])
    tex_order: list[str] = []
    for m in body_names:  # body textures first (a body mesh may also use the head texture: that one goes with the head)
        for t in models[m]["textures"]:
            if t not in tex_order and t not in head_tex:
                tex_order.append(t)
    tex_order += [t for t in models[head]["textures"] if t not in tex_order]
    for t in tex_order:
        if not krecipe.RESREF_RE.fullmatch(t):
            raise ScanError(where, f"texture name {t!r} is not a valid resref", "pick another creature template")
    if not tex_order:
        raise ScanError(where, "its meshes reference no texture that exists in the K1 install", "check [k1] dir is a full KOTOR 1 install")
    skipped = sorted({t for m in body_names + [head] for t in models[m].get("missing_textures", [])})

    # k2 names + collision check
    d_utc = utc_name(rid)
    pairs = [(f"model {m}", k2_stem(m)) for m in body_names + [head]] + [(f"texture {t}", k2_stem(t)) for t in tex_order]
    pairs.append((f"utc of {rid}", d_utc))
    audio = None
    gender = 1 if _int(facts.get("gender_id")) == 1 else 0
    ss = facts.get("soundset")
    ssf = _clean((ss or {}).get("resref")).lower()
    notes: list[str] = []
    if ss is not None and ssf and facts.get("ssf_found") and krecipe.RESREF_RE.fullmatch(ssf):
        audio = (ssf, ssf_name(d_utc))
        pairs.append((f"ssf of {rid}", audio[1]))
    else:
        why = ("the soundset row names no SSF" if ss is not None and not ssf else
               f"K1 {ssf}.ssf is not in the K1 install" if ss is not None and ssf else
               f"soundset_id {facts.get('soundset_id')} is not a K1 soundset.2da row")
        notes.append(f"no [audio]: {why}; the K2 fallback soundset is used")
    dialog = None
    conv = _clean(facts.get("conversation")).lower()
    if facts.get("dlg_found") and krecipe.RESREF_RE.fullmatch(conv):
        dialog = (conv, dlg_name(d_utc))
        pairs.append((f"dlg of {rid}", dialog[1]))
    claim_names(pairs)

    k2 = {m: k2_stem(m) for m in body_names + [head]}
    name = ascii_name(_clean(facts.get("first_name")) or _clean(facts.get("last_name")) or _clean(facts.get("tag")), rid)
    if skipped:
        notes.append("mesh textures not found in the K1 install (skipped): " + ", ".join(skipped))
    return Derived(
        id=rid, utc_src=utc_src, source=str(facts.get("source") or ""), display_name=name, tag=d_utc, utc=d_utc,
        dlg=_clean(facts.get("conversation")).lower(), soundset=SOUNDSET_FALLBACK[gender], template_row=TEMPLATE_ROW,
        gender=gender,
        body_a=(body_names[0], k2[body_names[0]]),
        body_b=(body_names[1], k2[body_names[1]]) if len(body_names) > 1 else None,
        head=(head, k2[head]),
        textures=[(t, k2_stem(t), HEAD_TXI if t in head_tex else "") for t in tex_order],
        audio=audio, dialog=dialog,
        k1_rows=(f"appearance.2da row {facts['appearance_id']} ({label}, modeltype B), heads.2da row {facts['head']['row']}, "
                 f"soundset.2da row {facts.get('soundset_id')}, gender_id {facts.get('gender_id')}, conversation "
                 f"{_clean(facts.get('conversation')) or '-'}"),
        notes=notes)


def _q(s: str) -> str:
    return json.dumps(s)  # a JSON string is a valid TOML basic string for ASCII text


def render(d: Derived) -> str:
    """The recipe TOML. Deterministic (no dates, no paths other than the K1 template name and where it was found)."""
    L = [f"# {d.display_name} (K1 creature template {d.utc_src}, {d.source or 'K1 install'}). AUTO-DERIVED by k1_port_scan.py from",
         "# your K1 install: instructions only, no assets, no 2DA row numbers (the builder assigns them). Edit a copy in",
         "# [build] recipes_dir to override it (a recipe file beats auto-derivation).",
         f"# K1 rows: {d.k1_rows}",
         f"# dialog: {d.dlg or '-'}  (K1 conversation; "
         + ("converted by the [dialog] table below)" if d.dialog else "not found as a DLG in the K1 install, so no [dialog] table)")]
    L += [f"# NOTE: {n}" for n in d.notes]
    L += ["[character]",
          f"id = {_q(d.id)}",
          f"display_name = {_q(d.display_name)}",
          f"tag = {_q(d.tag)}",
          f"utc = {_q(d.utc)}",
          'dlg = ""',
          f"soundset = {d.soundset}   # FALLBACK only (K2 {'n_commonf' if d.gender == 1 else 'n_commm'}); with [audio] the builder appends a new row",
          f"template_row = {d.template_row}",
          f"gender = {d.gender}", "",
          "[models]",
          'converter = "mdlops"       # pykotor-written bodies are wrong on K2 (doc 38 s22); MDLOps two-pass path with the K2 supermodel chain',
          f"body_a = {{ k1 = {_q(d.body_a[0])}, k2 = {_q(d.body_a[1])} }}"]
    if d.body_b is not None:
        L.append(f"body_b = {{ k1 = {_q(d.body_b[0])}, k2 = {_q(d.body_b[1])} }}")
    L.append(f"head   = {{ k1 = {_q(d.head[0])}, k2 = {_q(d.head[1])} }}")
    for k1, k2, txi in d.textures:
        L += ["", "[[textures]]", f"k1 = {_q(k1)}", f"k2 = {_q(k2)}", "force_opaque = true"]
        if txi:
            L.append(f"txi = {_q(txi)}")
    if d.audio is not None:
        L += ["", "[audio]", f"k1_ssf = {_q(d.audio[0])}", f"k2_ssf = {_q(d.audio[1])}"]
    if d.dialog is not None:
        L += ["", "[dialog]", f"k1_dlg = {_q(d.dialog[0])}", f"k2_dlg = {_q(d.dialog[1])}",
              f"max_voiced = {krecipe.DIALOG_DEFAULT_VOICED}   # voice budget: distinct VO lines (a wav + a lip each) in the shared asset pool"]
    return "\n".join(L) + "\n"


def derive_text(facts: dict, rid: str | None = None) -> tuple[Derived, str]:
    """derive + render + a round trip through the recipe validator, so a bad derivation fails here with a hint."""
    d = derive(facts, rid)
    text = render(d)
    try:
        import tomllib
        krecipe.parse_recipe(tomllib.loads(text), Path(f"{d.id}.toml"))
    except (krecipe.RecipeError, ValueError) as e:  # tomllib.TOMLDecodeError is a ValueError
        raise ScanError(f"derived recipe for {facts['utc']}", str(e), "report this creature template") from None
    return d, text


# ------------------------------------------------------------------------------------------- npc listing

def npc_row(reader, utc: dict) -> dict:
    """One `npcs` row from a normalized UTC dict."""
    app = _row(reader, "appearance", _int(utc.get("appearance_id")))
    models, mtype = [], ""
    head = ""
    if app is not None:
        mtype = _clean(app.get("modeltype")).upper()
        models = [m.lower() for m in (_clean(app.get("modela")), _clean(app.get("modelb"))) if m]
        nh = _clean(app.get("normalhead"))
        if mtype == "B" and nh.isdigit():
            h = _row(reader, "heads", int(nh))
            head = _clean((h or {}).get("head")).lower()
    ss_id = _int(utc.get("soundset_id"))
    ss = None if ss_id is None or ss_id in NO_SOUNDSET else _row(reader, "soundset", ss_id)
    return {"utc": str(utc["utc"]).lower(), "tag": _clean(utc.get("tag")), "name": _clean(utc.get("first_name")),
            "appearance": _int(utc.get("appearance_id")), "type": mtype,
            "models": "+".join(models) + (f"/{head}" if head else ""),
            "soundset": "" if ss_id is None or ss_id in NO_SOUNDSET else f"{ss_id}:{_clean((ss or {}).get('resref')).lower()}",
            "conversation": _clean(utc.get("conversation")).lower(), "source": str(utc.get("source") or "")}


def list_npcs(reader) -> list[dict]:
    return sorted((npc_row(reader, u) for u in reader.iter_utcs()), key=lambda r: r["utc"])


def match_npc(row: dict, pattern: str) -> bool:
    """Case-insensitive: the pattern is a regex; a pattern that is not a valid regex matches as plain text."""
    hay = [row["utc"], row["tag"], row["name"]]
    try:
        rx = re.compile(pattern, re.IGNORECASE)
        return any(rx.search(h) for h in hay)
    except re.error:
        p = pattern.lower()
        return any(p in h.lower() for h in hay)


NPC_COLUMNS = (("utc", "utc"), ("tag", "tag"), ("name", "name"), ("appearance", "app"), ("type", "t"), ("models", "models"),
               ("soundset", "soundset"), ("conversation", "conversation"), ("source", "source"))


def format_npcs(rows: list[dict]) -> list[str]:
    cells = [[str(r[k]) if r[k] != "" and r[k] is not None else "-" for k, _ in NPC_COLUMNS] for r in rows]
    heads = [h for _, h in NPC_COLUMNS]
    widths = [max([len(heads[i])] + [len(c[i]) for c in cells]) for i in range(len(heads))]
    out = ["  ".join(h.ljust(widths[i]) for i, h in enumerate(heads)).rstrip()]
    out += ["  ".join(c[i].ljust(widths[i]) for i in range(len(heads))).rstrip() for c in cells]
    return out


# ------------------------------------------------------------------------------------------- pykotor reader (helper only)

class PykotorReader:
    """Reader over a K1 install via pykotor (read only). Import this only under the k1port venv python."""

    def __init__(self, k1: Path):
        import logging
        logging.disable(logging.CRITICAL)
        from pykotor.extract.installation import Installation, SearchLocation
        from pykotor.resource.type import ResourceType
        self.rt, self.loc = ResourceType, SearchLocation
        self.inst = Installation(str(k1))
        self._tables: dict = {}
        self._tex_cache: dict = {}

    # -- lookups
    def _kind(self, filepath) -> str:
        p = Path(str(filepath))
        parent = p.parent.name.lower()
        if parent == "override":
            return f"override:{p.name}"
        if parent == "modules":
            return f"module:{p.name}"
        return f"chitin:{p.name}"

    def _normalize(self, resref: str, data: bytes, source: str) -> dict:
        from pykotor.resource.generics.utc import read_utc
        u = read_utc(bytes(data))

        def s(loc):
            try:
                return str(self.inst.string(loc) or "")
            except Exception:  # noqa: BLE001  a locstring with an unreadable strref
                return ""
        return {"utc": resref.lower(), "source": source, "tag": str(u.tag), "first_name": s(u.first_name),
                "last_name": s(u.last_name), "appearance_id": int(u.appearance_id), "soundset_id": int(u.soundset_id),
                "gender_id": int(u.gender_id), "conversation": str(u.conversation)}

    def _res(self, name: str, rtype):
        """Override, chitin, then the module archives (pykotor loads each location once, on its first use: the module
        archives cost seconds, so they are searched only after a miss)."""
        for locs in ([self.loc.OVERRIDE, self.loc.CHITIN], [self.loc.MODULES]):
            res = self.inst.resource(name, rtype, locs)
            if res is not None:
                return res
        return None

    def find_utc(self, resref: str) -> dict | None:
        res = self._res(resref, self.rt.UTC)
        if res is None:
            return None
        return self._normalize(resref, res.data, self._kind(res.filepath))

    def iter_utcs(self):
        seen = set()
        sources = [self.inst.override_resources(), self.inst.chitin_resources()]
        for m in self.inst.modules_list():
            try:
                sources.append(self.inst.module_resources(m))
            except Exception:  # noqa: BLE001  a broken rim is skipped; the rest still lists
                continue
        for lst in sources:
            for fr in lst:
                if fr.restype() != self.rt.UTC:
                    continue
                key = fr.resname().lower()
                if key in seen:
                    continue
                seen.add(key)
                try:
                    yield self._normalize(key, fr.data(), self._kind(fr.filepath()))
                except Exception:  # noqa: BLE001  an unreadable UTC is skipped
                    continue

    def twoda_row(self, name: str, idx: int) -> dict | None:
        if name not in self._tables:
            from pykotor.resource.formats.twoda import read_2da
            res = self.inst.resource(name, self.rt.TwoDA, [self.loc.OVERRIDE, self.loc.CHITIN])
            self._tables[name] = None if res is None else read_2da(bytes(res.data))
        t = self._tables[name]
        if t is None or idx < 0 or idx >= t.get_height():
            return None
        return {h: t.get_cell(idx, h) for h in t.get_headers()}

    def mdl_textures(self, resref: str) -> list[str] | None:
        from pykotor.resource.formats.mdl import read_mdl
        mdl = self._res(resref, self.rt.MDL)
        if mdl is None:
            return None
        mdx = self._res(resref, self.rt.MDX)
        m = read_mdl(bytes(mdl.data), source_ext=bytes(mdx.data) if mdx is not None else None)
        out: list[str] = []
        for node in m.all_nodes():
            if node.mesh is None:
                continue
            for t in (node.mesh.texture_1, node.mesh.texture_2):
                t = str(t or "").strip().lower()
                if t and t != "null" and t not in out:
                    out.append(t)
        return out

    def texture_exists(self, name: str) -> bool:
        if name not in self._tex_cache:
            locs = [self.loc.OVERRIDE, self.loc.TEXTURES_TPA, self.loc.TEXTURES_TPB, self.loc.TEXTURES_TPC,
                    self.loc.TEXTURES_GUI, self.loc.CHITIN]
            self._tex_cache[name] = any(self.inst.resource(name, rt, locs) is not None
                                        for rt in (self.rt.TPC, self.rt.TGA))
        return self._tex_cache[name]

    def ssf_exists(self, name: str) -> bool:
        return self.inst.resource(name, self.rt.SSF, [self.loc.OVERRIDE, self.loc.CHITIN]) is not None

    def dlg_exists(self, name: str) -> bool:
        return self._res(name, self.rt.DLG) is not None


def helper_main(argv: list, reader_factory=None) -> int:
    """Hidden helper (driver: `_scan`), run under the k1port venv python: derive K1DIR RESREF... | npcs K1DIR.
    derive prints {"facts": {resref: facts or null}} (one pykotor start for all resrefs), npcs {"rows": [...]}; the JSON
    is the LAST stdout line (pykotor may log above it). `reader_factory(k1dir)` is for tests."""
    op = argv[0] if argv else ""
    try:
        make = reader_factory or PykotorReader
        if op in ("derive", "npcs"):
            import contextlib
            ctx = contextlib.nullcontext()
            if reader_factory is None:
                import k1_port_textures as kt
                ctx = kt.pykotor_cwd()
            with ctx:
                reader = make(Path(argv[1]))
                if op == "derive":
                    print(json.dumps({"facts": {r: gather_facts(reader, r) for r in argv[2:]}}))
                else:
                    print(json.dumps({"rows": list_npcs(reader)}))
        else:
            print(f"unknown _scan op {op!r}", file=sys.stderr)
            return 2
    except Exception as e:  # noqa: BLE001  pykotor raises several error types
        print(f"error: _scan {op}: {type(e).__name__}: {e}", file=sys.stderr)
        return 1
    return 0
