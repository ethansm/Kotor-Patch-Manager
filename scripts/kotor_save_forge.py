#!/usr/bin/env python3
"""KOTOR II save forge (Phase 6b M1): build NEW test save slots from a source slot + a scenario in
tests/luo_scenarios/saves.toml.  Run with the repo venv:  .venv/bin/python3 scripts/kotor_save_forge.py ...

Commands: list | inspect <slot> | forge <source-slot> <scenario-id> [--name N] [--write] [--skip-refused] | cleanup [--yes]

Design (research/companion_levelup/phase6_evidence/M0_save_layout.md):
  * active party members live in SAVEGAME.sav availnpc<n>.utc; standing NPCs ALSO in the LASTMODULE nested .git
    Creature List -> both copies are edited.  The PC is Mod_PlayerList[0] of the nested module.ifo of LASTMODULE.
  * GLOBALVARS ValBoolean is MSB-first; pykotor GlobalVars (LSB-first) is never used - raw GFF edit.
  * pykotor drops duplicate GFF fields on rewrite, so only edited resources are re-serialised; everything else is copied
    byte-for-byte.  Existing slots are never written; only a NEW slot (max+1) is created, atomically (tmp dir + rename).
"""
from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import os
import re
import shutil
import sys
import tempfile
import tomllib
from pathlib import Path

from pykotor.resource.formats.erf import ERF, read_erf, bytes_erf
from pykotor.resource.formats.gff import read_gff, bytes_gff
from pykotor.resource.formats.gff.gff_data import GFFFieldType, GFFList
from pykotor.resource.type import ResourceType

TOOL_VERSION = "1.0.0"
DEFAULT_SAVES = Path("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/cloudsaves/62703561").expanduser()
DEFAULT_SCENARIOS = Path(__file__).resolve().parent.parent / "tests" / "luo_scenarios" / "saves.toml"
TEST_PREFIX = "LUO TEST"
MANIFEST = "forge_manifest.json"
CLOUD_NOTE = "NOTE: Steam Cloud may sync this slot until `cleanup`."

# classes.2da row -> label (verified against replay/pre_checkpoint_2da/classes.2da by the unit test).
CLASS_LABELS = ["Soldier", "Scout", "Scoundrel", "JediGuardian", "JediConsular", "JediSentinel", "CombatDroid",
                "ExpertDroid", "Minion", "TechSpecialist", "BountyHunter", "JediWeaponmaster", "JediMaster",
                "JediWatchman", "SithMarauder", "SithLord", "SithAssassin"]
# npc.2da row for tags (availnpc<n>); used only for messages when the creature is absent from the save.
NPC_INDEX = {"atton": 0, "baodur": 1, "mand": 2, "disciple": 3, "handmaiden": 4, "g0t0": 5, "kreia": 6,
             "mira": 7, "3cfd": 7, "t3m4": 8, "visas": 9, "hanharr": 10, "mandalore": 2}


class ForgeError(Exception):
    pass


def class_id(name: str) -> int:
    key = name.lower()
    for i, lab in enumerate(CLASS_LABELS):
        if lab.lower() == key:
            return i
    raise ForgeError(f"unknown class name {name!r} (classes.2da labels: {', '.join(CLASS_LABELS)})")


def class_label(i: int) -> str:
    return CLASS_LABELS[i] if 0 <= i < len(CLASS_LABELS) else f"class#{i}"


def md5(b: bytes) -> str:
    return hashlib.md5(b).hexdigest()


# ----------------------------------------------------------------------------------------------- game running
def game_running() -> bool:
    """True if a swkotor2.exe process exists (wine: argv token basename, or the `S:\\...swkotor2.exe` title)."""
    me = os.getpid()
    for p in Path("/proc").iterdir():
        if not p.name.isdigit() or int(p.name) == me:
            continue
        try:
            raw = (p / "cmdline").read_bytes()
        except OSError:
            continue
        toks = [t.decode("utf-8", "replace") for t in raw.split(b"\0") if t]
        full = " ".join(toks)
        if re.match(r"^S:.*swkotor2\.exe", full, re.I):
            return True
        for t in toks:
            if re.split(r"[\\/]", t)[-1].lower() == "swkotor2.exe":
                return True
    return False


# ----------------------------------------------------------------------------------------------- GFF helpers
def set_like(struct, label: str, value):
    """Set a field keeping its existing GFF type."""
    t = struct.what_type(label)
    getattr(struct, "set_" + t.name.lower())(label, value)


def find_res(erf: ERF, resref: str, ext: str):
    for r in erf:
        if str(r.resref).lower() == resref.lower() and r.restype.extension == ext:
            return r
    return None


def locstr(v) -> str:
    try:
        return str(v.get(0)) if hasattr(v, "get") else str(v)
    except Exception:
        return repr(v)


# ----------------------------------------------------------------------------------------------- GUID / globals
def luo_bits(gff_bytes: bytes):
    """-> (found, guid_or_None).  found = number of LUO_G00..31 names in CatBoolean."""
    root = read_gff(gff_bytes).root
    names = [x.acquire("Name", "") for x in root.get_list("CatBoolean")]
    vb = root.acquire("ValBoolean", b"")
    idx = {n: i for i, n in enumerate(names)}
    bits = {}
    for k in range(32):
        i = idx.get("LUO_G%02d" % k)
        if i is not None:
            bits[k] = (vb[i // 8] >> (7 - i % 8)) & 1 if i // 8 < len(vb) else 0
    found = len(bits)
    if found == 32:
        return found, sum(b << k for k, b in bits.items())
    return found, None


def guid_text(found: int, guid) -> str:
    if guid is not None:
        return "g%08x" % guid
    return "none" if found == 0 else f"partial {found}/32"


def edit_guid(gff_bytes: bytes, spec: str):
    """Return (new_bytes, description) applying guid spec (never keep)."""
    g = read_gff(gff_bytes)
    root = g.root
    cat = list(root.get_list("CatBoolean"))
    names = [x.acquire("Name", "") for x in cat]
    vb = root.acquire("ValBoolean", b"")
    vals = [(vb[i // 8] >> (7 - i % 8)) & 1 if i // 8 < len(vb) else 0 for i in range(len(names))]
    found, cur = luo_bits(gff_bytes)
    luo = {"LUO_G%02d" % k for k in range(32)}
    if spec == "none":
        keep = [(c, v) for c, n, v in zip(cat, names, vals) if n not in luo]
        add = {}
    elif spec.startswith("partial:"):
        n = int(spec.split(":")[1])
        if not 0 < n < 32:
            raise ForgeError(f"guid {spec!r}: n must be 1..31")
        base = cur if cur is not None else 0x5EED0005  # source without a GUID: use a fixed pattern (reported)
        keep = [(c, v) for c, nm, v in zip(cat, names, vals) if nm not in luo]
        add = {"LUO_G%02d" % k: (base >> k) & 1 for k in range(n)}
    elif re.fullmatch(r"g[0-9a-fA-F]{8}", spec):
        want = int(spec[1:], 16)
        keep = [(c, v) for c, nm, v in zip(cat, names, vals) if nm not in luo]
        add = {"LUO_G%02d" % k: (want >> k) & 1 for k in range(32)}
    else:
        raise ForgeError(f"bad guid spec {spec!r} (keep | none | partial:<n> | g<8 hex>)")
    new = GFFList()
    outvals = []
    for c, v in keep:
        new.append(c)
        outvals.append(v)
    sid = cat[0].struct_id if cat else 0
    for nm, v in add.items():
        s = new.add(sid)
        s.set_string("Name", nm)
        outvals.append(v)
    nbytes = len(outvals) // 8 + 1
    packed = bytearray(nbytes)
    for i, v in enumerate(outvals):
        if v:
            packed[i // 8] |= 1 << (7 - i % 8)
    root.set_list("CatBoolean", new)
    root.set_binary("ValBoolean", bytes(packed))
    note = f"GUID {guid_text(found, cur)} -> {spec} (CatBoolean {len(names)} -> {len(outvals)}, ValBoolean {len(vb)} -> {nbytes} B, MSB-first)"
    if spec.startswith("partial:") and cur is None:
        note += " [source had no GUID: pattern 0x5eed0005 used for the kept bits]"
    return bytes_gff(g), note


# ----------------------------------------------------------------------------------------------- slot model
class Slot:
    def __init__(self, path: Path):
        self.path = path
        self.files = {p.name: p.read_bytes() for p in sorted(path.iterdir()) if p.is_file()}
        self.nfo = read_gff(self.files["savenfo.res"]).root
        self.name = self.nfo.acquire("SAVEGAMENAME", "")
        self.last = str(self.nfo.acquire("LASTMODULE", "")).lower()
        self.outer = read_erf(self.files["SAVEGAME.sav"]) if "SAVEGAME.sav" in self.files else None

    @property
    def number(self):
        m = re.match(r"(\d+) - ", self.path.name)
        return int(m.group(1)) if m else None

    def nested(self, mod: str):
        r = find_res(self.outer, mod, "sav")
        return read_erf(r.data) if r else None

    def availnpcs(self):
        out = {}
        for r in self.outer:
            m = re.fullmatch(r"availnpc(\d+)", str(r.resref).lower())
            if m and r.restype.extension == "utc":
                out[int(m.group(1))] = read_gff(r.data).root
        return out

    def pc(self):
        e = self.nested(self.last)
        if e is None:
            return None
        ifo = find_res(e, "module", "ifo")
        if ifo is None:
            return None
        pl = read_gff(ifo.data).root.get_list("Mod_PlayerList")
        return pl.at(0) if len(pl) else None

    def party(self):
        pt = read_gff(self.files["PARTYTABLE.res"]).root
        return pt, [(m.acquire("PT_MEMBER_ID", -1), m.acquire("PT_IS_LEADER", 0)) for m in pt.get_list("PT_MEMBERS")]

    def guid(self):
        if "GLOBALVARS.res" not in self.files:
            return 0, None
        return luo_bits(self.files["GLOBALVARS.res"])


def slots_in(saves: Path):
    return [d for d in sorted(saves.iterdir()) if d.is_dir() and re.match(r"\d{6} - ", d.name)]


def resolve_slot(saves: Path, key: str) -> Path:
    ds = slots_in(saves)
    for d in ds:
        if d.name == key:
            return d
    if key.isdigit():
        for d in ds:
            if int(d.name[:6]) == int(key):
                return d
    hits = [d for d in ds if d.name[9:].lower() == key.lower()]
    if not hits:
        hits = [d for d in ds if key.lower() == Slot(d).name.lower()]
    if len(hits) == 1:
        return hits[0]
    raise ForgeError(f"slot {key!r}: {'ambiguous' if hits else 'not found'} in {saves}")


def creature_summary(c) -> str:
    cls = c.acquire("ClassList", None) or []
    cs = "+".join(f"{class_label(x.acquire('Class', -1))} {x.acquire('ClassLevel', 0)}" for x in cls)
    return f"{cs or '-'}  XP {c.acquire('Experience', '?')}"


def fmt_time(s) -> str:
    s = int(s)
    return f"{s // 3600}h{s % 3600 // 60:02d}m"


def cmd_list(saves: Path):
    print(f"saves dir: {saves}")
    for d in slots_in(saves):
        s = Slot(d)
        found, guid = s.guid()
        print(f"{d.name} | savenfo '{s.name}' | area '{s.nfo.acquire('AREANAME', '')}' | played {fmt_time(s.nfo.acquire('TIMEPLAYED', 0))}"
              f" | GUID {guid_text(found, guid)}" + (" | FORGED" if (d / MANIFEST).exists() else ""))
        if s.outer is None:
            print("    (no SAVEGAME.sav)")
            continue
        pc = s.pc()
        if pc is not None:
            print(f"    PC       {creature_summary(pc)}")
        pt, members = s.party()
        av = s.availnpcs()
        for mid, lead in members:
            u = av.get(mid)
            print(f"    member {mid} {u.acquire('Tag', '?') if u else '?'}{' (leader flag)' if lead else ''}  "
                  f"{creature_summary(u) if u else 'no availnpc copy'}")
        if not members:
            print("    party: no NPC members")


def cmd_inspect(saves: Path, key: str):
    s = Slot(resolve_slot(saves, key))
    found, guid = s.guid()
    print(f"slot {s.path.name}  savenfo '{s.name}'  LASTMODULE {s.last.upper()}  area '{s.nfo.acquire('AREANAME', '')}'")
    print(f"files: " + ", ".join(f"{k} ({len(v)} B)" for k, v in s.files.items()))
    print(f"GUID: {guid_text(found, guid)}")
    root = read_gff(s.files["GLOBALVARS.res"]).root
    names = [x.acquire("Name", "") for x in root.get_list("CatBoolean")]
    vb = root.acquire("ValBoolean", b"")
    print("LUO_G* globals (msb-first):", " ".join(
        f"{n}={(vb[i // 8] >> (7 - i % 8)) & 1}" for i, n in enumerate(names) if n.startswith("LUO_G")) or "none")
    pt, members = s.party()
    print(f"PARTYTABLE: members={members} NUM={pt.acquire('PT_NUM_MEMBERS', '?')} controlled={pt.acquire('PT_CONTROLLED_NP', '?')} "
          f"xp_pool={pt.acquire('PT_XP_POOL', '?')}")
    print("  avail/select: " + " ".join(f"{i}:{a.acquire('PT_NPC_AVAIL', 0)}/{a.acquire('PT_NPC_SELECT', 0)}"
                                        for i, a in enumerate(pt.get_list("PT_AVAIL_NPCS"))))
    pc = s.pc()
    print(f"PC (Mod_PlayerList[0] of {s.last.upper()}.sav module.ifo): {creature_summary(pc) if pc else 'absent'}")
    git = None
    nest = s.nested(s.last)
    if nest is not None:
        g = find_res(nest, s.last, "git")
        git = read_gff(g.data).root if g else None
    gitc = {}
    if git is not None:
        for c in git.get_list("Creature List"):
            gitc[c.acquire("Tag", "")] = c
    print("members / NPCs:")
    for n, u in s.availnpcs().items():
        tag = u.acquire("Tag", "")
        act = any(mid == n for mid, _ in members)
        print(f"  availnpc{n}.utc tag={tag!r} {'ACTIVE ' if act else 'inactive'} {creature_summary(u)} | "
              f"git copy ({s.last.upper()}.git): {creature_summary(gitc[tag]) if tag in gitc else 'absent'}")


# ----------------------------------------------------------------------------------------------- forge
def load_scenario(path: Path, sid: str):
    data = tomllib.loads(path.read_text(encoding="utf-8"))
    sc = data.get("scenario", {})
    if sid not in sc:
        hits = [k for k in sc if k.startswith(sid + "_")]
        if len(hits) != 1:
            raise ForgeError(f"scenario {sid!r} not found in {path} (have: {', '.join(sc)})")
        sid = hits[0]
    return sid, sc[sid]


def apply_creature_edit(c, ed: dict, where: str, notes: list, unproven: list):
    if "xp" in ed:
        old = c.acquire("Experience", None)
        c.set_uint32("Experience", int(ed["xp"]))
        notes.append(f"{where}: Experience {old} -> {ed['xp']}")
    if "classes" in ed:
        cl = c.get_list("ClassList")
        want = [(class_id(n), int(l)) for n, l in ed["classes"]]
        if len(want) < len(cl):
            raise ForgeError(f"{where}: classes lists {len(want)} entries but the creature has {len(cl)}; removing a class is refused")
        for i, (cid, lvl) in enumerate(want):
            if i < len(cl):
                e = cl.at(i)
                if e.acquire("Class", -1) != cid:
                    raise ForgeError(f"{where}: ClassList[{i}] is {class_label(e.acquire('Class', -1))}, scenario wants {class_label(cid)}; "
                                     f"changing the class of an existing entry is refused (order must match)")
                old = e.acquire("ClassLevel", 0)
                e.set_int16("ClassLevel", lvl)
                notes.append(f"{where}: ClassList[{i}] {class_label(cid)} level {old} -> {lvl}")
            else:
                sid = cl.at(0).struct_id if len(cl) else 0
                e = cl.add(sid)
                e.set_int32("Class", cid)
                e.set_int16("ClassLevel", lvl)
                e.set_list("SpellsPerDayList", GFFList())
                e.set_list("KnownList0", GFFList())
                notes.append(f"{where}: ClassList[{i}] ADDED {class_label(cid)} level {lvl} (empty KnownList0)")
                unproven.append(f"{where}: added class {class_label(cid)} - engine-validity unproven (no feats/powers/LvlStat)")
        c.set_list("ClassList", cl)  # get_list returns a copy of the list: write it back


def forge_plan(slot: Slot, sid: str, sc: dict, skip_refused: bool):
    """Compute edited resources without touching disk. Returns (changes, notes, unproven, refused)
    changes: dict key -> new bytes where key is a tuple path: ('GLOBALVARS.res',) / ('SAVEGAME.sav', 'availnpc0.utc') / (..)."""
    save = sc.get("save", {})
    notes, unproven, refused = [], [], []
    outer = slot.outer
    newfiles = {}          # top-level file -> bytes
    outer_set = {}         # (resref, ext) -> bytes
    nested_set = {}        # (modresref, resref, ext) -> bytes
    modname = slot.last
    nest = slot.nested(modname)
    git = ifo = None
    if nest is not None:
        gr, ir = find_res(nest, modname, "git"), find_res(nest, "module", "ifo")
        git = read_gff(gr.data) if gr else None
        ifo = read_gff(ir.data) if ir else None
    avail = {}
    for r in outer:
        m = re.fullmatch(r"availnpc(\d+)", str(r.resref).lower())
        if m and r.restype.extension == "utc":
            avail[int(m.group(1))] = read_gff(r.data)
    tagidx = {g.root.acquire("Tag", "").lower(): n for n, g in avail.items()}
    ptg = read_gff(slot.files["PARTYTABLE.res"])
    dirty = set()  # which objects were touched: 'git','ifo','pt', ('avail',n)

    for ed in save.get("creature", []):
        tag = ed.get("tag", "").lower()
        try:
            if tag == "pc":
                if ifo is None or not len(ifo.root.get_list("Mod_PlayerList")):
                    raise ForgeError("pc: no Mod_PlayerList[0] in the LASTMODULE module.ifo")
                if "in_party" in ed:
                    raise ForgeError("pc: in_party is meaningless for the PC (refused)")
                apply_creature_edit(ifo.root.get_list("Mod_PlayerList").at(0), ed, f"pc[{modname}.ifo Mod_PlayerList[0]]", notes, unproven)
                dirty.add("ifo")
                if "leader" in ed:
                    if ed["leader"]:
                        cur = ptg.root.acquire("PT_CONTROLLED_NP", None)
                        if cur != -1:
                            set_like(ptg.root, "PT_CONTROLLED_NP", -1)
                            dirty.add("pt")
                            notes.append(f"pc: PT_CONTROLLED_NP {cur} -> -1 (PC controlled); PT_IS_LEADER flags untouched (semantics unresolved, M0 Q2)")
                            unproven.append("pc leader: PT_IS_LEADER meaning vs PC unresolved (M0 section 8 Q2)")
                        else:
                            notes.append("pc: leader requested; PT_CONTROLLED_NP already -1 (no change)")
                    else:
                        raise ForgeError("pc: leader=false is refused (cannot make the PC a non-leader)")
                continue
            if tag not in tagidx:
                raise ForgeError(f"{tag}: no availnpc<n>.utc with Tag {tag!r} in the source slot "
                                 f"(npc.2da row {NPC_INDEX.get(tag, '?')}); the forge cannot synthesise a creature")
            n = tagidx[tag]
            apply_creature_edit(avail[n].root, ed, f"{tag}[availnpc{n}.utc]", notes, unproven)
            dirty.add(("avail", n))
            if git is not None:
                for c in git.root.get_list("Creature List"):
                    if c.acquire("Tag", "").lower() == tag:
                        apply_creature_edit(c, ed, f"{tag}[{modname}.git Creature List]", notes, unproven)
                        dirty.add("git")
            if "in_party" in ed:
                members = ptg.root.get_list("PT_MEMBERS")
                present = [i for i in range(len(members)) if members.at(i).acquire("PT_MEMBER_ID", -1) == n]
                if ed["in_party"] and not present:
                    if len(members) >= 2:
                        raise ForgeError(f"{tag}: party already has {len(members)} NPC members (max 2); in_party refused")
                    m = members.add(members.at(0).struct_id if len(members) else 0)
                    m.set_int32("PT_MEMBER_ID", n)
                    m.set_uint8("PT_IS_LEADER", 0)
                    ptg.root.set_list("PT_MEMBERS", members)
                    set_like(ptg.root, "PT_NUM_MEMBERS", len(members))
                    a = ptg.root.get_list("PT_AVAIL_NPCS").at(n)
                    set_like(a, "PT_NPC_AVAIL", 1)
                    set_like(a, "PT_NPC_SELECT", 1)
                    notes.append(f"{tag}: PARTYTABLE PT_MEMBERS += {n} (leader 0), PT_NUM_MEMBERS {len(members)}, PT_AVAIL_NPCS[{n}] avail=1 select=1")
                    if git is not None and any(c.acquire("Tag", "").lower() == tag for c in git.root.get_list("Creature List")):
                        unproven.append(f"{tag}: in_party added while a standing-NPC git copy exists - which copy the engine uses is unproven (M0 Q1)")
                    dirty.add("pt")
                elif not ed["in_party"] and present:
                    for i in reversed(present):
                        members.remove(i)
                    ptg.root.set_list("PT_MEMBERS", members)
                    set_like(ptg.root, "PT_NUM_MEMBERS", len(members))
                    notes.append(f"{tag}: removed from PT_MEMBERS (NUM {len(members)})")
                    dirty.add("pt")
                else:
                    notes.append(f"{tag}: in_party={ed['in_party']} already true in source (no change)")
            if "leader" in ed:
                raise ForgeError(f"{tag}: NPC leader=true refused - PT_IS_LEADER vs PT_CONTROLLED_NP semantics unresolved (M0 Q2)")
        except ForgeError as e:
            if not skip_refused:
                raise
            refused.append(str(e))

    guid_spec = save.get("guid", "keep")
    if guid_spec != "keep":
        gb, note = edit_guid(slot.files["GLOBALVARS.res"], guid_spec)
        newfiles["GLOBALVARS.res"] = gb
        notes.append("GLOBALVARS.res: " + note)

    if "pt" in dirty:
        newfiles["PARTYTABLE.res"] = bytes_gff(ptg)
    for k in dirty:
        if isinstance(k, tuple):
            outer_set[(f"availnpc{k[1]}", "utc")] = bytes_gff(avail[k[1]])
    if "git" in dirty:
        nested_set[(modname, modname, "git")] = bytes_gff(git)
    if "ifo" in dirty:
        nested_set[(modname, "module", "ifo")] = bytes_gff(ifo)
    return newfiles, outer_set, nested_set, notes, unproven, refused


def next_slot_number(saves: Path) -> int:
    nums = [int(d.name[:6]) for d in slots_in(saves)]
    return max(nums) + 1 if nums else 0


def safe_name(n: str) -> str:
    if not n or re.search(r'[\\/:*?"<>|\0]', n) or n in (".", ".."):
        raise ForgeError(f"bad slot name {n!r}")
    return n


def cmd_forge(saves, src_key, sid, name, write, skip_refused, scen_path):
    sid, sc = load_scenario(scen_path, sid)
    src = Slot(resolve_slot(saves, src_key))
    save_name = safe_name(name or f"{TEST_PREFIX} {sid}")
    num = next_slot_number(saves)
    target = saves / f"{num:06d} - {save_name}"
    newfiles, outer_set, nested_set, notes, unproven, refused = forge_plan(src, sid, sc, skip_refused)
    status = sc.get("status", "")
    print(f"scenario {sid} ({status}): {sc.get('title', '')}")
    print(f"source slot: {src.path.name}\ntarget slot: {target}  (savenfo SAVEGAMENAME '{save_name}', SAVENUMBER {num})")
    for n in notes:
        print("  edit:", n)
    for r in refused:
        print("  REFUSED:", r)
    for u in unproven:
        print("  UNPROVEN:", u)
    if status == "unproven" or unproven:
        print("  engine-validity unproven")
    if not name or name.startswith(TEST_PREFIX):
        pass
    else:
        print(f"  WARNING: --name does not start with '{TEST_PREFIX}' - `cleanup` will not remove this slot")
    changed_keys = (list(newfiles) + [f"SAVEGAME.sav::{r}.{e}" for r, e in outer_set] +
                    [f"SAVEGAME.sav::{m}.sav::{r}.{e}" for m, r, e in nested_set])
    print("  resources rewritten:", ", ".join(changed_keys + ["SAVEGAME.sav", "savenfo.res"]) if (outer_set or nested_set) else
          ", ".join(changed_keys + ["savenfo.res"]))
    if not write:
        print("DRY RUN: nothing written (use --write).")
        return 0
    if game_running():
        raise ForgeError("swkotor2.exe is running - refusing to write (close the game first)")
    if target.exists():
        raise ForgeError(f"{target} already exists; never overwriting")

    # nested modules
    files = dict(src.files)
    files.update(newfiles)
    changes = {}  # manifest: key -> {before, after}
    for k, b in newfiles.items():
        changes[k] = {"md5_before": md5(src.files[k]), "md5_after": md5(b)}
    if outer_set or nested_set:
        outer = read_erf(src.files["SAVEGAME.sav"])
        by_mod = {}
        for (m, r, e), b in nested_set.items():
            by_mod.setdefault(m, {})[(r, e)] = b
        for m, edits in by_mod.items():
            res = find_res(outer, m, "sav")
            nested = read_erf(res.data)
            for (r, e), b in edits.items():
                old = find_res(nested, r, e).data
                nested.set_data(r, ResourceType.from_extension(e), b)
                changes[f"SAVEGAME.sav::{m}.sav::{r}.{e}"] = {"md5_before": md5(old), "md5_after": md5(b)}
            nb = bytes_erf(nested, ResourceType.SAV)
            outer.set_data(m, ResourceType.SAV, nb)
            changes[f"SAVEGAME.sav::{m}.sav"] = {"md5_before": md5(res.data), "md5_after": md5(nb)}
        for (r, e), b in outer_set.items():
            old = find_res(outer, r, e).data
            outer.set_data(r, ResourceType.from_extension(e), b)
            changes[f"SAVEGAME.sav::{r}.{e}"] = {"md5_before": md5(old), "md5_after": md5(b)}
        files["SAVEGAME.sav"] = bytes_erf(outer, ResourceType.SAV)
        changes["SAVEGAME.sav"] = {"md5_before": md5(src.files["SAVEGAME.sav"]), "md5_after": md5(files["SAVEGAME.sav"])}
    nfo = read_gff(files["savenfo.res"])
    nfo.root.set_string("SAVEGAMENAME", save_name)
    set_like(nfo.root, "SAVENUMBER", num)
    files["savenfo.res"] = bytes_gff(nfo)
    changes["savenfo.res"] = {"md5_before": md5(src.files["savenfo.res"]), "md5_after": md5(files["savenfo.res"])}
    manifest = {"tool": "kotor_save_forge.py", "tool_version": TOOL_VERSION,
                "created": datetime.datetime.now().isoformat(timespec="seconds"),
                "source_slot": src.path.name, "scenario": sid, "scenario_status": status, "slot_number": num,
                "save_name": save_name, "scenario_save": sc.get("save", {}), "edits": notes, "refused": refused,
                "unproven": unproven, "changed_resources": changes}
    files[MANIFEST] = (json.dumps(manifest, indent=2) + "\n").encode()
    tmp = Path(tempfile.mkdtemp(prefix=".forge_tmp_", dir=saves))
    try:
        for fn, b in files.items():
            (tmp / fn).write_bytes(b)
        os.rename(tmp, target)
    except Exception:
        shutil.rmtree(tmp, ignore_errors=True)
        raise
    print(f"WROTE {target}")
    print(CLOUD_NOTE)
    return 0


# ----------------------------------------------------------------------------------------------- cleanup
def forged_slots(saves: Path):
    out = []
    for d in slots_in(saves):
        if (d / MANIFEST).exists() and (d / "savenfo.res").exists():
            nm = read_gff((d / "savenfo.res").read_bytes()).root.acquire("SAVEGAMENAME", "")
            if nm.startswith(TEST_PREFIX):
                out.append(d)
    return out


def cmd_cleanup(saves: Path, yes: bool, ask=None):
    ask = ask or input
    ds = forged_slots(saves)
    if not ds:
        print("no LUO TEST slots with a forge_manifest.json")
        return 0
    for d in ds:
        print("  would remove:", d)
    if not yes and ask(f"Remove {len(ds)} slot(s)? Type 'yes': ").strip().lower() != "yes":
        print("aborted; nothing removed")
        return 1
    for d in ds:
        shutil.rmtree(d)
        print("removed", d)
    print("(Steam Cloud may re-sync the deletion.)")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--saves-dir", type=Path, default=DEFAULT_SAVES)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("list")
    p = sub.add_parser("inspect"); p.add_argument("slot")
    p = sub.add_parser("forge")
    p.add_argument("source"); p.add_argument("scenario"); p.add_argument("--name")
    p.add_argument("--write", action="store_true"); p.add_argument("--skip-refused", action="store_true",
                                                                    help="apply the creature edits that can be done, list the refused ones")
    p.add_argument("--scenarios", type=Path, default=DEFAULT_SCENARIOS)
    p = sub.add_parser("cleanup"); p.add_argument("--yes", action="store_true")
    # allow --saves-dir after the subcommand too
    for sp in sub.choices.values():
        sp.add_argument("--saves-dir", type=Path, default=argparse.SUPPRESS)
    a = ap.parse_args(argv)
    try:
        if a.cmd == "list":
            return cmd_list(a.saves_dir) or 0
        if a.cmd == "inspect":
            return cmd_inspect(a.saves_dir, a.slot) or 0
        if a.cmd == "forge":
            return cmd_forge(a.saves_dir, a.source, a.scenario, a.name, a.write, a.skip_refused, a.scenarios)
        return cmd_cleanup(a.saves_dir, a.yes)
    except ForgeError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
