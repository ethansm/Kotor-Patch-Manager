#!/usr/bin/env python3
"""Offline scenario runner (Phase 6b M3, T2/T3): tests/luo_scenarios/saves.toml -> fake engine -> the REAL LevelUpOverride.cpp.

For every scenario with save/config data this writes a data file for scenario_test.cpp (fake creatures, party leader, LUO_G globals table, ini
files, action script, expectations), compiles it with host g++ (like parity.py) and runs ONE process per scenario (the mod keeps global state).
Then runs `scenario_test --unit` (onLevelUp DECIDE/DRY/SKIP/APPLIED on fake caller chains, LUO_G removal safety).
Usage (from research/companion_levelup): python3 tools/parity/scenarios.py [--m32] [--only S1_atton_hold_default] [-v]
Exit 1 on any failure. --m32 builds and runs the 32-bit variant (T3); it is skipped with a note if `g++ -m32` cannot link here.
"""
import os, subprocess, sys, tempfile, tomllib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))            # research/companion_levelup
REPO = os.path.dirname(os.path.dirname(ROOT))
TOML = os.path.join(REPO, "tests", "luo_scenarios", "saves.toml")
INI = os.path.join(ROOT, "runtime", "levelup_paths.ini")
XP = os.path.join(HERE, "testdata", "exptable.tsv")
SRC = os.path.join(HERE, "scenario_test.cpp")

CLASS_IDS = {n: i for i, n in enumerate("Soldier Scout Scoundrel JediGuardian JediConsular JediSentinel CombatDroid ExpertDroid Minion TechSpecialist "
                                       "BountyHunter JediWeaponMaster JediMaster JediWatchman SithMarauder SithLord SithAssassin".split())}
FIXTURE_GUID = 0xFB7BDBF7   # "keep": the GUID the live test save carries (doc 08 run logs)
TIMEOUT = 120

# TODO(move into saves.toml as [scenario.<id>.expect].decisions): decision keys the harness emits are
#   <tag>.canLevelUp = HOLD|PASS|NONE   (HOLD = real CanLevelUp 1 gated to 0; NONE = the real engine cannot level)
#   <tag>.autoLevel  = APPLIED|WOULD_REWRITE|ALREADY|SKIP|DRY|NONE|BLOCKED   <tag>.guard = HOLD|PASS (GUI button guard, tag = party leader)
#   guid = g<hex> after `load`, load.found = LUO_G names present, save.globals = LUO_G names in the saved table, banked = n of the last BANKED line
EXPECT_DECISIONS = {
    "S1_atton_hold_default": {"atton.canLevelUp": "HOLD", "atton.autoLevel": "BLOCKED", "banked": "1", "guid": "gfb7bdbf7"},
    "S2_atton_pause3_many": {"atton.canLevelUp": "HOLD", "atton.autoLevel": "BLOCKED", "banked": "12"},   # levels 4..15 = 12 held levels
    "S3_atton_converted_early": {"atton.canLevelUp": "PASS", "atton.autoLevel": "APPLIED"},
    "S5_guid_partial": {"guid": "g00000000", "load.found": "16"},
    "S6_guid_orphan": {"guid": "g5eed0006", "load.found": "32"},
    "S7_guid_cleared": {"guid": "g00000000", "load.found": "0", "save.globals": "0"},
    "S10_pc_led_levelable": {"pc.canLevelUp": "PASS", "pc.guard": "PASS", "pc.autoLevel": "NONE"},
}


def build(exe, m32=False):
    cmd = ["g++", "-std=c++17", "-O1", "-Wall", "-Wextra"] + (["-m32"] if m32 else []) + [
        "-I", os.path.join(HERE, "stub"), "-I", os.path.join(HERE, "stub", "inc"), "-include", os.path.join(HERE, "stub", "m3_win_shim.h"), "-o", exe, SRC]
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r


def m32_ok():
    with tempfile.TemporaryDirectory() as d:
        c = os.path.join(d, "t.cpp")
        open(c, "w").write("#include <vector>\nint main(){std::vector<int> v(3);return v.size()==3?0:1;}\n")
        r = subprocess.run(["g++", "-m32", "-o", os.path.join(d, "t"), c], capture_output=True, text=True)
        if r.returncode:
            return False
        return subprocess.run([os.path.join(d, "t")]).returncode == 0


def guid_of(save):
    """(guid, present bits) of the scenario's save as the fake globals table sees it."""
    g = save.get("guid", "keep")
    if g == "keep":
        return FIXTURE_GUID, 32
    if g == "none":
        return 0, 0
    if g.startswith("partial:"):
        return FIXTURE_GUID, int(g.split(":")[1])
    if g.startswith("g"):
        return int(g[1:], 16), 32
    raise SystemExit("bad guid spec %r" % g)


def scenario_dat(sid, sc):
    """Data-file block for one scenario; None if it has no save/config data."""
    save, cfg, exp = sc.get("save"), sc.get("config"), sc.get("expect", {})
    if save is None and cfg is None:
        return None
    save, cfg = save or {}, cfg or {}
    guid, nbits = guid_of(save)
    L = ["SCENARIO " + sid, "GLOBALS %08x %d" % (guid, nbits)]
    tags = []
    for c in save.get("creature", []):
        classes = ",".join("%d:%d" % (CLASS_IDS[n], lv) for n, lv in c.get("classes", [])) or "2:7"   # missing classes = keep: a stand-in Scoundrel 7
        L.append("CREATURE %s %s %d %d %d" % (c["tag"], classes, c.get("xp", 0), 1 if c.get("in_party", True) else 0, 1 if c.get("leader") else 0))
        tags.append(c)
    L.append("INI mode=%s" % cfg.get("mode", "rewrite"))
    L.append("INI apply=%s" % cfg.get("apply", 1))
    L.append("INI verbose=%s" % cfg.get("verbose", 1))
    for k, v in cfg.get("dev", {}).items():
        L.append("INI %s=%s" % (k, v))
    pg = cfg.get("playthrough_guid", "none")
    ptg = {"none": None, "save": (guid if nbits == 32 else None)}.get(pg, int(pg[1:], 16) if pg.startswith("g") else None)
    if ptg:
        L.append("PTGUID %08x" % ptg)
        for t, spec in cfg.get("companions", {}).items():
            L.append("COMPANION %s %s" % (t, spec))
    else:
        L.append("PTGUID none")
    # default action script: load the save's globals, evaluate every creature at the hold gate, auto-level it, GUI-guard the leader, save
    acts = ["load"]
    for c in tags:
        if c.get("in_party", True) or c.get("leader"):
            acts += ["tick " + c["tag"], "auto " + c["tag"]]
    for c in tags:
        if c.get("leader"):
            acts.append("guard " + c["tag"])
    acts.append("save")
    L += ["ACT " + a for a in sc.get("actions", acts)]
    L += ["EVENT " + e for e in exp.get("events", [])]
    L += ["ABSENT " + e for e in exp.get("absent", [])]
    dec = dict(EXPECT_DECISIONS.get(sid, {}))
    dec.update(exp.get("decisions", {}))
    L += ["DECISION %s %s" % (k, v) for k, v in dec.items()]
    L.append("END")
    return "\n".join(L)


# Scenarios that exist only offline (UI actions the toml cannot express yet): the options page Configure -> Apply flow creates the GUID and the
# 32 globals at once ("no write until Apply"), and a save then carries them.
EXTRA = {"X1_ui_apply_writes_globals": """SCENARIO X1_ui_apply_writes_globals
GLOBALS 00000000 0
CREATURE atton 2:7 28000 1 0
INI mode=rewrite
INI apply=1
INI verbose=1
PTGUID none
ACT load
ACT save
ACT ui atton
ACT click cfg
ACT click apply
ACT save
EVENT ^UI click apply
EVENT ^GUID new g[0-9a-f]{8} live-write ok=1
EVENT ^PLAYTHROUGH write g[0-9a-f]{8} tag=atton ok=1
DECISION save.globals 32
DECISION load.found 0
DECISION ui.open 0
END"""}


def run(exe, args, env):
    return subprocess.run([exe] + args, capture_output=True, text=True, timeout=TIMEOUT, env=env)


def main():
    argv = sys.argv[1:]
    m32 = "--m32" in argv
    verbose = "-v" in argv
    only = argv[argv.index("--only") + 1] if "--only" in argv else None
    if m32 and not m32_ok():
        print("scenarios: -m32 SKIPPED (g++ -m32 cannot build and run a C++ program on this host)")
        return 0
    with open(TOML, "rb") as f:
        data = tomllib.load(f)
    # the XP fixture must stay the closed form the toml header documents: L*(L-1)/2*1000
    for line in open(XP):
        if line.startswith("#") or not line.strip():
            continue
        lv, x = map(int, line.split())
        want = lv * (lv - 1) // 2 * 1000 if lv < 51 else 0xFFFFFFFF
        if x != want:
            print("scenarios: FAIL exptable fixture level %d = %d, closed form %d" % (lv, x, want))
            return 1
    tmp = os.environ.get("TMPDIR") or tempfile.gettempdir()
    exe = os.path.join(HERE, "scenario_test_bin32" if m32 else "scenario_test_bin")
    r = build(exe, m32)
    if r.returncode:
        sys.stderr.write(r.stderr)
        print("scenarios: COMPILE FAILED")
        return 1
    env = dict(os.environ, LUO_EXPTABLE=XP)
    blocks = {}
    for sid, sc in data.get("scenario", {}).items():
        b = scenario_dat(sid, sc)
        if b:
            blocks[sid] = b
    blocks.update(EXTRA)
    fails, ran = 0, 0
    with tempfile.NamedTemporaryFile("w", suffix=".dat", delete=False, dir=tmp) as df:
        df.write("\n".join(blocks.values()) + "\n")
        dat = df.name
    try:
        for sid in blocks:
            if only and sid != only:
                continue
            p = run(exe, [INI, dat, sid], env)
            ran += 1
            if p.returncode:
                fails += 1
                sys.stdout.write(p.stdout)
                sys.stderr.write(p.stderr)
                print("scenarios: FAIL %s (rc %d)" % (sid, p.returncode))
            elif verbose:
                sys.stdout.write(p.stdout)
        if not only:
            p = run(exe, [INI, "--unit"], env)
            sys.stdout.write(p.stdout)
            if p.returncode:
                fails += 1
                sys.stderr.write(p.stderr)
    finally:
        os.unlink(dat)
    print("scenarios%s: %d scenarios run, %d failed%s" % (" (-m32)" if m32 else "", ran, fails, "" if not only else " (only %s)" % only))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
