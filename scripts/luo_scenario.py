#!/usr/bin/env python3
"""luo_scenario.py - apply a [scenario.<id>.config] table from tests/luo_scenarios/saves.toml to the game-dir
levelup_override.ini / levelup_playthroughs.ini (Phase 6b M2).

  plan    <id> [--guid gXXXXXXXX] [--game-dir DIR]          print the unified diff that apply would make
  apply   <id> [--guid ..] [--game-dir DIR] --yes           back up, write, print the diff (game must be closed)
  restore [--game-dir DIR] [--yes]                          put the latest backup back
  status  [--game-dir DIR]                                  latest applied scenario + do the files still match

Only keys the DLL reads (LevelUpOverride.cpp config parser) are emitted: mode, apply, verbose, preset.<tag>,
instant.<tag>, hold.<tag>, pause.<tag>. Unrelated lines/sections are kept verbatim.
"""
import argparse, datetime, difflib, hashlib, json, os, re, shutil, subprocess, sys, tomllib

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_TOML = os.path.join(HERE, "..", "tests", "luo_scenarios", "saves.toml")
DEFAULT_GAME_DIR = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
OVR, PLY = "levelup_override.ini", "levelup_playthroughs.ini"
BACKUP_ROOT = "luo_scenario_backup"
SCALAR_KEYS = ("mode", "apply", "verbose")          # read by the C++ parser
DEV_PREFIXES = ("preset.", "instant.", "hold.", "pause.")
MODES = ("off", "log", "rewrite")
GUID_RE = re.compile(r"^g[0-9a-f]{8}$")
COMPANION_RE = re.compile(r"^[a-z0-9_]+(\|(instant=[01]|hold=[01]|pause=\d+))*$")


class Err(Exception):
    pass


def game_running():
    r = subprocess.run(["pgrep", "-fa", r"swkotor2\.exe"], capture_output=True, text=True)
    return [l for l in r.stdout.splitlines() if "pgrep" not in l]


def read(path):
    try:
        with open(path, "rb") as f:
            return f.read().decode("utf-8", "surrogateescape")
    except FileNotFoundError:
        return None


def write(path, text):
    with open(path, "wb") as f:
        f.write(text.encode("utf-8", "surrogateescape"))


def sha(text):
    return None if text is None else hashlib.sha256(text.encode("utf-8", "surrogateescape")).hexdigest()


# ---------------------------------------------------------------- scenario
def load_scenario(toml_path, sid):
    with open(toml_path, "rb") as f:
        data = tomllib.load(f)
    sc = data.get("scenario", {}).get(sid)
    if sc is None:
        raise Err(f"unknown scenario '{sid}' in {toml_path}; known: {', '.join(sorted(data.get('scenario', {})))}")
    return sc.get("config", {}), []


def resolve(cfg, guid_arg):
    """-> (scalars{key:val}, dev{key:val}, companions{tag:str}, guid|None, warnings[])"""
    warns = []
    known = {"mode", "apply", "verbose", "companions", "playthrough_guid", "dev"}
    for k in cfg:
        if k not in known:
            warns.append(f"unsupported config key '{k}' ignored (the DLL does not read it)")
    scal = {}
    for k in SCALAR_KEYS:
        if k in cfg:
            v = cfg[k]
            if k == "mode":
                v = str(v).lower()
                if v not in MODES:
                    raise Err(f"mode must be one of {MODES}, got {v!r}")
            else:
                if isinstance(v, bool):
                    v = int(v)
                if str(v) not in ("0", "1"):
                    raise Err(f"{k} must be 0|1, got {v!r}")
            scal[k] = str(v)
    dev = {}
    for k, v in (cfg.get("dev") or {}).items():
        kl = k.lower()
        if not kl.startswith(DEV_PREFIXES) or kl.split(".", 1)[1] == "":
            warns.append(f"unsupported dev key '{k}' ignored (C++ reads only preset./instant./hold./pause.<tag>)")
            continue
        dev[kl] = str(v)
    comps = {}
    for tag, v in (cfg.get("companions") or {}).items():
        if not COMPANION_RE.match(str(v).lower()):
            raise Err(f"companions.{tag}: bad value {v!r} (want preset|instant=0|1|hold=0|1|pause=N)")
        comps[tag.lower()] = str(v)
    pg = cfg.get("playthrough_guid", "none")
    guid = None
    if pg == "save":
        if comps:
            if not guid_arg:
                raise Err("scenario has playthrough_guid = \"save\": pass --guid gXXXXXXXX (see scripts/kotor_save_forge.py list)")
            guid = guid_arg.lower()
    elif pg == "none":
        if comps:
            warns.append("playthrough_guid = \"none\": companions table ignored (no section written)")
            comps = {}
    elif GUID_RE.match(str(pg)):
        guid = pg
    else:
        raise Err(f"playthrough_guid must be 'save', 'none' or g<8 hex>, got {pg!r}")
    if guid is not None and not GUID_RE.match(guid):
        raise Err(f"--guid must look like g<8 lowercase hex>, got {guid!r}")
    if pg == "save" and not comps and not guid_arg:
        pass  # nothing to write for the guid
    return scal, dev, comps, guid, warns


# ---------------------------------------------------------------- ini edits
def nl_of(text):
    return "\r\n" if "\r\n" in text else "\n"


def set_keys(text, kv):
    """Replace every `key=` line (case-insensitive key) with key=value; append missing keys at the end."""
    if not kv:
        return text
    nl = nl_of(text)
    lines = text.splitlines(keepends=True)
    seen = set()
    for i, ln in enumerate(lines):
        s = ln.strip()
        if not s or s[0] in ";#" or "=" not in s:
            continue
        k = s.split("=", 1)[0].strip().lower()
        if k in kv:
            end = ln[len(ln.rstrip("\r\n")):]
            lines[i] = f"{k}={kv[k]}{end}"
            seen.add(k)
    out = "".join(lines)
    add = [f"{k}={v}" for k, v in kv.items() if k not in seen]
    if add:
        if out and not out.endswith("\n"):
            out += nl
        out += nl.join(add) + nl
    return out


def set_section(text, guid, comps):
    """Within [guid] replace/append `tag=value` lines; other sections and lines are untouched."""
    nl = nl_of(text)
    lines = text.splitlines(keepends=True)
    hdr = f"[{guid}]"
    start = next((i for i, l in enumerate(lines) if l.strip().lower() == hdr), None)
    if start is None:
        if lines and not lines[-1].endswith("\n"):
            lines[-1] += nl
        lines += [hdr + nl] + [f"{t}={v}{nl}" for t, v in comps.items()]
        return "".join(lines)
    end = next((i for i in range(start + 1, len(lines)) if lines[i].lstrip().startswith("[")), len(lines))
    seen = set()
    for i in range(start + 1, end):
        s = lines[i].strip()
        if not s or s[0] in ";#" or "=" not in s:
            continue
        t = s.split("=", 1)[0].strip().lower()
        if t in comps:
            lines[i] = f"{t}={comps[t]}{lines[i][len(lines[i].rstrip(chr(13) + chr(10))):]}"
            seen.add(t)
    # insert missing tags after the last non-blank line of the section
    last = end
    while last > start + 1 and not lines[last - 1].strip():
        last -= 1
    if last > start + 1 and not lines[last - 1].endswith("\n"):
        lines[last - 1] += nl
    new = [f"{t}={v}{nl}" for t, v in comps.items() if t not in seen]
    lines[last:last] = new
    return "".join(lines)


def compute(game_dir, toml_path, sid, guid_arg):
    cfg, _ = load_scenario(toml_path, sid)
    scal, dev, comps, guid, warns = resolve(cfg, guid_arg)
    old = {OVR: read(os.path.join(game_dir, OVR)), PLY: read(os.path.join(game_dir, PLY))}
    new = dict(old)
    kv = {**scal, **dev}
    if kv:
        new[OVR] = set_keys(old[OVR] or "", kv)
    if guid and comps:
        new[PLY] = set_section(old[PLY] or "", guid, comps)
    return old, new, warns


def diff_text(old, new):
    out = []
    for name in (OVR, PLY):
        a, b = old[name], new[name]
        if a == b:
            continue
        out += difflib.unified_diff((a or "").splitlines(keepends=True), (b or "").splitlines(keepends=True),
                                    f"a/{name}" if a is not None else "/dev/null", f"b/{name}")
        out.append("\n")
    return "".join(x if x.endswith("\n") else x + "\n" for x in out) or "(no changes)\n"


# ---------------------------------------------------------------- commands
def need_closed():
    r = game_running()
    if r:
        raise Err("swkotor2.exe is running; close the game first:\n" + "\n".join(r))


def cmd_plan(a):
    old, new, warns = compute(a.game_dir, a.toml, a.scenario, a.guid)
    for w in warns:
        print("warning:", w, file=sys.stderr)
    print(diff_text(old, new), end="")


def cmd_apply(a):
    old, new, warns = compute(a.game_dir, a.toml, a.scenario, a.guid)
    for w in warns:
        print("warning:", w, file=sys.stderr)
    d = diff_text(old, new)
    if not a.yes:
        print(d, end="")
        print("not applied: pass --yes to back up and write", file=sys.stderr)
        return 2
    need_closed()
    ts = datetime.datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    bdir = os.path.join(a.game_dir, BACKUP_ROOT, ts)
    os.makedirs(bdir)
    for n in (OVR, PLY):
        if old[n] is not None:
            write(os.path.join(bdir, n), old[n])
    for n in (OVR, PLY):
        if new[n] != old[n]:
            write(os.path.join(a.game_dir, n), new[n])
    state = {"scenario": a.scenario, "guid": a.guid, "applied_at": ts, "restored": False,
             "backup_existed": {n: old[n] is not None for n in (OVR, PLY)},
             "written_sha256": {n: sha(new[n]) for n in (OVR, PLY)}}
    write(os.path.join(bdir, "state.json"), json.dumps(state, indent=2) + "\n")
    print(d, end="")
    print(f"applied {a.scenario}; backup: {bdir}")


def latest_backup(game_dir):
    root = os.path.join(game_dir, BACKUP_ROOT)
    if not os.path.isdir(root):
        return None
    ds = sorted(x for x in os.listdir(root) if os.path.isfile(os.path.join(root, x, "state.json")))
    return os.path.join(root, ds[-1]) if ds else None


def cmd_restore(a):
    b = latest_backup(a.game_dir)
    if not b:
        raise Err("no backup found under " + os.path.join(a.game_dir, BACKUP_ROOT))
    st = json.loads(read(os.path.join(b, "state.json")))
    print(f"latest backup: {b} (scenario {st['scenario']}{', already restored' if st.get('restored') else ''})")
    if not a.yes:
        print("not restored: pass --yes", file=sys.stderr)
        return 2
    need_closed()
    for n in (OVR, PLY):
        dst = os.path.join(a.game_dir, n)
        if st["backup_existed"][n]:
            shutil.copyfile(os.path.join(b, n), dst)
        elif os.path.exists(dst):
            os.remove(dst)
    st["restored"] = True
    write(os.path.join(b, "state.json"), json.dumps(st, indent=2) + "\n")
    print("restored")


def cmd_status(a):
    b = latest_backup(a.game_dir)
    if not b:
        print("no scenario applied (no backups)")
        return
    st = json.loads(read(os.path.join(b, "state.json")))
    print(f"latest: {st['scenario']} guid={st.get('guid')} at {st['applied_at']} backup={b}")
    if st.get("restored"):
        print("state: restored (backup put back)")
        return
    for n in (OVR, PLY):
        cur = sha(read(os.path.join(a.game_dir, n)))
        print(f"  {n}: {'matches' if cur == st['written_sha256'][n] else 'DIFFERS from'} what the scenario wrote")


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    for name in ("plan", "apply", "restore", "status"):
        s = sub.add_parser(name)
        if name in ("plan", "apply"):
            s.add_argument("scenario")
            s.add_argument("--guid")
        if name in ("apply", "restore"):
            s.add_argument("--yes", action="store_true")
        s.add_argument("--game-dir", default=DEFAULT_GAME_DIR)
        s.add_argument("--toml", default=DEFAULT_TOML)
    a = p.parse_args(argv)
    try:
        return {"plan": cmd_plan, "apply": cmd_apply, "restore": cmd_restore, "status": cmd_status}[a.cmd](a) or 0
    except Err as e:
        print("error:", e, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
