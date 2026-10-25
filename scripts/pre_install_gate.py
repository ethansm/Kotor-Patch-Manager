#!/usr/bin/env python3
"""Pre-install gate (doc 20 §4.2 / Phase 6, doc 24 T8): prove the source being installed is the source being smoked.

  scripts/pre_install_gate.sh --mod perf_lab_src/perflab                 # GATE PASS / GATE FAIL <n> <step>: <reason>
  scripts/pre_install_gate.sh --mod load_hang_probe_src --mod perf_lab_src/perflab
  scripts/pre_install_gate.sh --selftest                                # two broken builds must both FAIL

Steps (stop at the first failure; exit 1 on FAIL, 0 on PASS):
  1 default   run_tests.py, scoped: the tests/static checks compat_check.py plans for the gated mods' source files
              (tracked + untracked, incl. a nested mod's parent dir) + dirty files elsewhere under patch_manager_mods +
              the _shared_src/tests/include headers they #include + GLOBAL_SUITES, via --ids-file. Any git/compat_check
              failure, a gap of a gated mod, or a planned/global id missing from `run_tests.py --list` falls back to
              the full default tier with a WARN; --full-default forces the full tier;
              --skip-default skips it with a warning (selftest only). A failure set of exactly
              {suite:check-exports} is tolerated with a WARN (it checks the stale live kpatch; see step 3)
  2 build     build_check.py --mod <m> --keep --build-root <scratch>/build (a fresh windows_x86.dll from source)
  3 scratch   for every smokes.toml row whose kpatch (or extra_dlls kpatch) carries the mod's manifest id: copy the
              live kpatch to <scratch>/Patches/<same rel path> and replace binaries/windows_x86.dll with the fresh DLL
              (a mod with no smoke row gets its live kpatch looked up by manifest id); then check_exports.py runs on
              those scratch kpatches, so a new detour export is judged on the fresh DLL, not the stale KPM one;
              Patches/Common (kpatch_api.h etc.) is copied in too, since the smokes compile with -I$KPM_DIR/Patches/Common
  4 wine      KPM_DIR=<scratch> run_tests.py --wine wine:<row>... (only the matched rows; rows of other mods are
              never run, even when all their kpatches happen to sit in the scratch tree as co-loaded DLLs); each
              matched row must PASS (SKIP only for missing DISPLAY). A mod with no smoke row prints NOTE and still passes.
  5 report    GATE PASS mods=<ids> smokes=<rows> head=<sha12> + the md5 of every rebuilt DLL (the md5 that
              stage_and_verify_install.py verify compares)

On PASS one JSON record per mod is written (atomically) to $TMPDIR/kotor-tests/gate-records/<mod id>.json with
{mod_id, md5, dll, head, time, scratch, dirty, step1}; kpm_hotswap.py reads it. A GATE FAIL deletes the
mods' old records.

Read-only towards $KPM_DIR and the game dir: everything is written under $TMPDIR/kotor-tests/gate-<pid>-<epoch>/ (a
fresh dir per run, so concurrent gates never clobber each other; asserted;
scripts/prune_test_cache.py prunes old gate-<pid>-<epoch> dirs).
"""
import argparse
import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import time
import tomllib
import zipfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
MODS = REPO / "patch_manager_mods"
PY = str(REPO / ".venv" / "bin" / "python") if (REPO / ".venv" / "bin" / "python").exists() else sys.executable
RUN_TESTS = REPO / "scripts" / "run_tests.py"
BUILD_CHECK = REPO / "scripts" / "build_check.py"
SMOKES_TOML = MODS / "perf_lab_src" / "harness" / "smokes.toml"
TMP_BASE = Path(os.environ.get("TMPDIR") or f"/tmp/claude-{os.getuid()}") / "kotor-tests"
KPM_DIR = Path(os.environ.get("KPM_DIR") or "~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager").expanduser()
GAME_DIR = Path(os.environ.get("KOTOR2_ROOT")
                or "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II").expanduser()
CHECK_EXPORTS = REPO / "scripts" / "check_exports.py"
DEFERRED = {"suite:check-exports"}   # judged on the fresh DLL in step 3 instead
COMPAT_CHECK = REPO / "scripts" / "compat_check.py"
# repo-wide static checks that no per-file plan reliably selects; always part of the scoped step 1
GLOBAL_SUITES = [f"suite:{n}" for n in (
    "check-hook-bytes", "check-hook-spans", "check-hook-sources", "check-exports", "kpm-validate-patches",
    "check-gui-ids", "check-effect-decode", "check-shared-headers", "check-raw-addresses", "check-subclassers",
    "provider_manifests")]
DLL_MEMBER = "binaries/windows_x86.dll"
RESULT_RE = re.compile(r"^(PASS|FAIL|ERROR|SKIP)\s+wine:([^\s\[]+)(\[\S*\])?\s+(.*)$")

sys.path.insert(0, str(REPO / "scripts"))
try:
    import master_ledger as ml
except ImportError:
    ml = None
try:
    import _load_limit
except ImportError:
    _load_limit = None


class GateFail(Exception):
    def __init__(self, n, step, reason):
        super().__init__(f"GATE FAIL {n} {step}: {reason}")
        self.n, self.step, self.reason = n, step, reason


# ---------------------------------------------------------------- helpers

def safe_scratch(path: Path) -> Path:
    """Refuse any scratch dir outside TMP_BASE or inside the KPM tree / game dir."""
    p = Path(path).resolve()
    if not p.is_relative_to(TMP_BASE.resolve()):
        raise SystemExit(f"refusing scratch dir {p}: not under {TMP_BASE}")
    for forbidden in (KPM_DIR, GAME_DIR):
        if p.is_relative_to(forbidden.resolve()):
            raise SystemExit(f"refusing scratch dir {p}: inside {forbidden}")
    return p


def default_scratch() -> Path:
    """A fresh per-run scratch dir (concurrent gates must never share one: step_scratch rmtree's <scratch>/Patches)."""
    return TMP_BASE / f"gate-{os.getpid()}-{int(time.time())}"


def write_gate_record(mod_id: str, dll: Path, scratch: Path, records_dir: Path | None = None,
                      dirty: bool = False, step1: str = "full") -> Path:
    """Atomically write the PASS record kpm_hotswap.py reads:
    {mod_id, md5, dll, head, time, scratch, dirty, step1 = scoped|full|skipped}."""
    d = records_dir or TMP_BASE / "gate-records"
    d.mkdir(parents=True, exist_ok=True)
    rec = {"mod_id": mod_id, "md5": md5(dll), "dll": str(dll), "head": git_head(),
           "time": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
           "scratch": str(scratch), "dirty": bool(dirty), "step1": step1}
    dest = d / f"{mod_id}.json"
    tmp = d / f".{mod_id}.json.{os.getpid()}.tmp"
    try:
        tmp.write_text(json.dumps(rec, indent=2) + "\n")
        os.replace(tmp, dest)
    except OSError:
        tmp.unlink(missing_ok=True)
        raise
    return dest


def drop_gate_records(mod_ids, records_dir: Path | None = None):
    """A failed gate must not leave an older PASS record behind."""
    d = records_dir or TMP_BASE / "gate-records"
    for mid in mod_ids:
        if mid:
            (d / f"{mid}.json").unlink(missing_ok=True)


def mod_dir(mod: str, repo: Path = REPO) -> Path:
    p = Path(mod)
    return (p if p.is_absolute() else repo / "patch_manager_mods" / p).resolve()


def mod_key(mod: str, repo: Path = REPO) -> str:
    """build_check.discover's key: the dir relative to patch_manager_mods, parts joined with '__'."""
    d = mod_dir(mod, repo)
    try:
        return "__".join(d.relative_to((repo / "patch_manager_mods").resolve()).parts)
    except ValueError:
        return d.name


def manifest_id(text: str) -> str:
    return tomllib.loads(text).get("patch", {}).get("id", "")


def kpatch_id(kpatch: Path) -> str:
    try:
        with zipfile.ZipFile(kpatch) as z:
            return manifest_id(z.read("manifest.toml").decode("utf-8"))
    except (OSError, KeyError, zipfile.BadZipFile, tomllib.TOMLDecodeError):
        return ""


def smoke_rows(mod_id: str, smokes_toml: Path, kpm_dir: Path) -> list:
    """Names of the smokes.toml rows whose kpatch (or an extra_dlls kpatch) carries mod_id."""
    if not mod_id or not smokes_toml.is_file():
        return []
    cfg = tomllib.loads(smokes_toml.read_text())
    cache, out = {}, []
    for s in cfg.get("smoke", []):
        kps = [s["kpatch"]] + [x["kpatch"] for x in s.get("extra_dlls", [])]
        for kp in kps:
            if kp not in cache:
                cache[kp] = kpatch_id(kpm_dir / "Patches" / kp)
        if any(cache[kp] == mod_id for kp in kps):
            out.append(s["name"])
    return out


def swap_dll(src: Path, dest: Path, dll: Path):
    """Copy the kpatch zip src to dest with binaries/windows_x86.dll replaced; other members byte-identical."""
    dest.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(src) as zin, zipfile.ZipFile(dest, "w") as zout:
        if DLL_MEMBER not in zin.namelist():
            raise GateFail(3, "scratch", f"{src} has no {DLL_MEMBER}")
        for info in zin.infolist():
            data = dll.read_bytes() if info.filename == DLL_MEMBER else zin.read(info.filename)
            zout.writestr(info, data, compress_type=info.compress_type)


def md5(path: Path) -> str:
    return hashlib.md5(path.read_bytes()).hexdigest()


def run(cmd, env=None, quiet=False) -> tuple:
    """(returncode, combined output); output is echoed unless quiet."""
    p = subprocess.run(cmd, cwd=REPO, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if not quiet:
        sys.stdout.write(p.stdout)
        sys.stdout.flush()
    return p.returncode, p.stdout


# ---------------------------------------------------------------- steps

def _git(repo: Path, *args):
    """git stdout, or None when git fails (callers then fall back to the full tier)."""
    try:
        p = subprocess.run(["git", "-C", str(repo), *args], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    except OSError:
        return None
    return p.stdout.decode("utf-8", "replace") if p.returncode == 0 else None


def mod_source_files(mods, repo: Path = REPO):
    """Repo-relative tracked + untracked (not ignored) files under each gated mod's dir AND under its top-level
    patch_manager_mods/<x>_src dir (a nested --mod such as perf_lab_src/perflab has sibling py tests one level up).
    None when git fails."""
    files, root = [], Path(repo).resolve()
    for m in mods:
        d = mod_dir(m, repo)
        try:
            rel = d.relative_to(root)
        except ValueError:
            continue
        dirs = [rel]
        pm = root / "patch_manager_mods"
        parts = d.relative_to(pm).parts if d.is_relative_to(pm) else ()
        if len(parts) > 1:
            dirs.append(Path("patch_manager_mods") / parts[0])
        for dd in dirs:
            out = _git(repo, "ls-files", "-co", "--exclude-standard", "-z", "--", str(dd))
            if out is None:
                return None
            files += [f for f in out.split("\0") if f and f not in files]
    return files


SHARED_PREFIXES = ("patch_manager_mods/_shared_src/", "tests/include/")
HEADER_EXTS = {".h", ".hpp", ".inc", ".cpp"}


def include_closure(files, repo: Path = REPO, shared_only: bool = True):
    """Quoted/angle #includes of `files`, resolved recursively with compat_graph's rules, that land in
    patch_manager_mods/_shared_src or tests/include (repo-relative, sorted). compat_check only walks REVERSE includes
    from the files it is given, so the tests of the shared headers a mod compiles are selected by passing the headers
    themselves as touched files. None when the graph helper is unavailable."""
    try:
        from compat_graph import CompatGraph, INCLUDE_RE
        graph = CompatGraph(repo_root=Path(repo))
    except Exception as e:   # noqa: BLE001
        print(f"WARN  step 1: cannot resolve includes ({e})")
        return None
    root = Path(repo).resolve()
    seen, found, stack = set(), set(), [root / f for f in files if Path(f).suffix in HEADER_EXTS]
    while stack:
        src = stack.pop()
        if src in seen or not src.is_file():
            continue
        seen.add(src)
        try:
            text = src.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        for m in INCLUDE_RE.finditer(text):
            try:
                res, _ = graph._resolve_include(src, m.group(1), m.group(2))
            except Exception:   # noqa: BLE001
                continue
            if not isinstance(res, Path):
                continue
            try:
                rel = res.relative_to(root).as_posix()
            except ValueError:
                continue
            if not shared_only or rel.startswith(SHARED_PREFIXES):
                found.add(rel)
            stack.append(res)
    return sorted(found)


DIRTY_SUFFIXES = {".h", ".hpp", ".inc", ".cpp", ".py", ".toml"}


def dirty_scope_files(exclude, repo: Path = REPO):
    """Dirty/untracked paths (git status --porcelain -uall) under patch_manager_mods/**, tests/include/** that can
    change a build or test and are not already in `exclude`. They go to compat_check too: a shared header edit
    must select the tests of its includers. None when git fails."""
    raw = _git(repo, "status", "--porcelain", "-uall", "-z")
    if raw is None:
        return None
    out = raw.split("\0")
    paths, i = [], 0
    while i < len(out):
        e = out[i]
        i += 1
        if len(e) < 4:
            continue
        xy, path = e[:2], e[3:]
        paths.append(path)
        if "R" in xy or "C" in xy:   # the next entry is the original path
            if i < len(out) and out[i]:
                paths.append(out[i])
            i += 1
    res = []
    for f in paths:
        if f in exclude or f in res or Path(f).suffix not in DIRTY_SUFFIXES:
            continue
        if f.startswith("patch_manager_mods/") or f.startswith("tests/include/"):
            res.append(f)
    return res


def mod_dirty(mod: str, repo: Path = REPO) -> bool:
    try:
        rel = mod_dir(mod, repo).relative_to(Path(repo).resolve())
    except ValueError:
        return False
    out = _git(repo, "status", "--porcelain", "--", str(rel))
    return out is None or bool(out.strip())   # a failing git counts as dirty


def _gated_tops(mods, repo: Path):
    tops = set()
    for m in mods:
        try:
            tops.add(mod_dir(m, repo).relative_to((Path(repo) / "patch_manager_mods").resolve()).parts[0])
        except (ValueError, IndexError):
            pass
    return tops


def _is_research(entry) -> bool:
    return any(w.startswith("research/") or "/research/" in w for w in entry.get("why") or [] if isinstance(w, str))


def scoped_ids(mods, scratch: Path, runner=run, repo: Path = REPO):
    """Item ids for the scoped step 1, or None when the scope cannot be computed (caller falls back to the full tier)."""
    files = mod_source_files(mods, repo)
    if files is None:
        print("WARN  step 1: git ls-files failed")
        return None
    if not files:
        print("WARN  step 1: no source files found for the gated mods")
        return None
    dirty = dirty_scope_files(set(files), repo)
    if dirty is None:
        print("WARN  step 1: git status failed")
        return None
    # Other sessions' dirty files elsewhere (tools, other mods) cannot change the gated build or its tests unless the
    # gated sources include them: keep dirty shared headers (their includers' tests) and dirty files in the include
    # closure; the rest is INFO (it used to pull e.g. 165 s of unrelated k1_port py tests into every gate).
    # Known limit: only #include edges count here, not py imports or provider fan-out (no DLL mod pair needs them today).
    reach = include_closure(files, repo, shared_only=False)
    if reach is None:
        return None
    reach = set(reach)
    skipped = [f for f in dirty if not f.startswith(SHARED_PREFIXES) and f not in reach]
    dirty = [f for f in dirty if f not in skipped]
    if skipped:
        print(f"INFO  step 1: {len(skipped)} dirty file(s) outside the gated mods' include closure not scoped: "
              f"{', '.join(skipped[:3])}" + (", ..." if len(skipped) > 3 else ""))
    if dirty:
        print(f"INFO  step 1 also scopes {len(dirty)} dirty shared file(s): {', '.join(dirty[:5])}"
              + (", ..." if len(dirty) > 5 else ""))
    closure = include_closure(files + dirty, repo)
    if closure is None:
        return None
    closure = [f for f in closure if f not in files and f not in dirty]
    if closure:
        print(f"INFO  step 1 also scopes {len(closure)} shared header(s) the gated sources include")
    scratch.mkdir(parents=True, exist_ok=True)
    cj = scratch / "compat.json"
    cj.unlink(missing_ok=True)
    rc, out = runner([PY, str(COMPAT_CHECK), *files, *dirty, *closure, "--list", "--json", str(cj)], quiet=True)
    try:
        data = json.loads(cj.read_text())
        plan = data["plan"]
    except (OSError, ValueError, KeyError, TypeError):
        print(f"WARN  step 1: compat_check.py gave no usable plan (rc={rc})")
        return None
    if rc != 0:
        print(f"WARN  step 1: compat_check.py rc={rc}")
        return None
    tops = _gated_tops(mods, repo)
    # Unclassified files we passed ourselves (mod dirs, dirty files elsewhere in patch_manager_mods) can only matter
    # through include/import edges, which compat_check already follows: INFO only. Anything else is a real gap.
    own = set(files) | set(dirty)
    unclassified = [u for u in data.get("unclassified") or []]
    ignored = [u for u in unclassified if u in own]
    if ignored:
        print(f"INFO  step 1: {len(ignored)} unclassified file(s) outside manifest dirs (not a gap): "
              f"{', '.join(ignored[:3])}" + (", ..." if len(ignored) > 3 else ""))
    bad = [f"unclassified: {u}" for u in unclassified if u not in own]
    for g in data.get("gaps") or []:
        mod = str(g).split()[0] if str(g).split() else ""
        if mod.split("/")[0] in tops:
            bad.append(f"gap: {g}")
        else:
            print(f"INFO  step 1: gap of another mod in the plan (not gated here): {g}")
    if bad:
        print(f"WARN  step 1: compat_check.py reports {len(bad)} gap/unclassified item(s): {'; '.join(bad[:5])}")
        return None
    other_builds = [e["id"].split(":", 1)[-1] for e in plan
                    if e.get("kind") == "build" and e.get("id")
                    and e["id"].split(":", 1)[-1].split("/")[0] not in tops]
    if other_builds:
        print(f"INFO  step 1: {len(other_builds)} other mod(s) planned for build, not built by this gate: "
              f"{', '.join(other_builds[:5])}")
    rc, out = runner([PY, str(RUN_TESTS), "--list"], quiet=True)
    if rc != 0:
        print(f"WARN  step 1: run_tests.py --list rc={rc}")
        return None
    known = {l.split()[0] for l in out.splitlines() if l.strip()}
    ids = []
    missing = [i for i in GLOBAL_SUITES if i not in known]
    if missing:
        print(f"WARN  step 1: global suite(s) not in run_tests.py --list: {', '.join(missing)}")
        return None
    dropped = []
    for e in plan:
        i = e.get("id")
        if e.get("kind") == "build" or not i:
            continue
        if i not in known:
            if _is_research(e):
                print(f"INFO  step 1: research item {i} is not in the default tier; skipped")
            else:
                dropped.append(i)
        elif i not in ids:
            ids.append(i)
    if dropped:
        print(f"WARN  step 1: planned item(s) not in run_tests.py --list: {', '.join(dropped[:5])}")
        return None
    ids += [i for i in GLOBAL_SUITES if i not in ids]
    return ids or None


def step_default(runner=run, mods=None, scratch: Path | None = None, full: bool = False) -> str:
    """Returns "scoped" or "full" (the tier that ran)."""
    cmd = [PY, str(RUN_TESTS)]
    scratch = scratch or default_scratch()
    ids = None if full or not mods else scoped_ids(mods, scratch, runner)
    mode = "scoped" if ids else "full"
    if ids:
        idf = scratch / "step1_ids.txt"
        idf.write_text("\n".join(ids) + "\n")
        cmd += ["--ids-file", str(idf)]
        print(f"== step 1/4: run_tests.py (scoped: {len(ids)} items; --full-default for all)", flush=True)
    else:
        if not full and mods:
            print("WARN  step 1: falling back to the full default tier")
        print("== step 1/4: run_tests.py (full default tier)", flush=True)
    rc, out = runner(cmd)
    bad = [l.split()[1] for l in out.splitlines() if re.match(r"^(FAIL|ERROR)\s+\S", l)]
    if bad and set(bad) <= DEFERRED:
        print("WARN  step 1: suite:check-exports failed against the stale KPM kpatch; deferred to step 3 (fresh DLL)")
        return mode
    if rc != 0 or bad:
        raise GateFail(1, "default", f"run_tests.py rc={rc}; failing: {', '.join(bad) or '(see output)'}")
    return mode


def step_build(mods, build_root: Path, repo: Path = REPO, runner=run) -> dict:
    """Fresh DLL per mod -> {mod: dll path}."""
    cmd = [PY, str(BUILD_CHECK), "--keep", "--build-root", str(build_root), "--repo", str(repo)]
    for m in mods:
        cmd += ["--mod", m]
    rc, out = runner(cmd)
    if rc != 0:
        fails = [l for l in out.splitlines() if l.startswith("FAIL")]
        raise GateFail(2, "build", "; ".join(fails[:3]) or f"build_check.py rc={rc}")
    dlls = {}
    for m in mods:
        hits = [p for p in (build_root / mod_key(m, repo)).glob("*/build/windows_x86.dll")]
        if len(hits) != 1:
            raise GateFail(2, "build", f"{m}: expected one built windows_x86.dll under {build_root / mod_key(m, repo)}, "
                                       f"found {len(hits)} (build skipped? no MinGW or KPM Common/lib)")
        dlls[m] = hits[0]
    return dlls


def step_scratch(dll_by_id: dict, scratch: Path, smokes_toml: Path = SMOKES_TOML, kpm_dir: Path = KPM_DIR,
                 packed: dict | None = None) -> dict:
    """Populate <scratch>/Patches with the matched kpatches (fresh DLLs swapped in). Returns {mod id: [rows]}.
    `packed`, if given, is filled with {mod id: [scratch kpatch paths carrying its fresh DLL]}."""
    packed = {} if packed is None else packed
    patches = scratch / "Patches"
    shutil.rmtree(patches, ignore_errors=True)
    common = kpm_dir / "Patches" / "Common"
    if common.is_dir():   # smokes compile with -I$KPM_DIR/Patches/Common (kpatch_api.h etc.), so the scratch needs it too
        shutil.copytree(common, patches / "Common")
    cfg = tomllib.loads(smokes_toml.read_text())
    rows = {mid: smoke_rows(mid, smokes_toml, kpm_dir) for mid in dll_by_id}
    wanted = {r for rs in rows.values() for r in rs}
    for s in cfg.get("smoke", []):
        if s["name"] not in wanted:
            continue
        for kp in [s["kpatch"]] + [x["kpatch"] for x in s.get("extra_dlls", [])]:
            dest = patches / kp
            if dest.exists():
                continue
            src = kpm_dir / "Patches" / kp
            mid = kpatch_id(src)
            if mid in dll_by_id:
                swap_dll(src, dest, dll_by_id[mid])
                packed.setdefault(mid, []).append(dest)
            else:   # a co-loaded DLL of another mod: as installed
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src, dest)
    for mid in dll_by_id:   # no smoke row: still pack its live kpatch (by manifest id) so exports get checked
        if mid in packed:
            continue
        for src in sorted((kpm_dir / "Patches").glob("**/*.kpatch")):
            if kpatch_id(src) == mid:
                dest = patches / src.relative_to(kpm_dir / "Patches")
                swap_dll(src, dest, dll_by_id[mid])
                packed[mid] = [dest]
                break
    return rows


def step_exports(ids, packed: dict, scratch: Path, runner=run):
    """check_exports.py on the scratch kpatches carrying the fresh DLLs (not the stale live ones)."""
    for mid in ids:
        if mid not in packed:
            print(f"NOTE no live kpatch for {mid}: exports not checked against a packed build")
    kps = [p for mid in ids for p in packed.get(mid, [])]
    if not kps:
        return
    cmd = [PY, str(CHECK_EXPORTS), "--quiet", "--kpm-dir", str(scratch)]
    for mid in ids:
        cmd += ["--mod", mid]
    for p in kps:
        cmd += ["--kpatch", str(p)]
    rc, out = runner(cmd)
    if rc != 0:
        fails = [l for l in out.splitlines() if l.startswith("FAIL")]
        raise GateFail(3, "scratch", "; ".join(fails[:3]) or f"check_exports.py rc={rc}")


def parse_wine(out: str) -> dict:
    """{row: [(status, mode, reason)]} from run_tests.py report lines."""
    res = {}
    for line in out.splitlines():
        m = RESULT_RE.match(line)
        if m:
            res.setdefault(m.group(2), []).append((m.group(1), m.group(3) or "", m.group(4)))
    return res


def judge_wine(rows, res: dict) -> list:
    """Raise GateFail on any FAIL/ERROR/missing/non-DISPLAY SKIP of a matched row. Returns the DISPLAY-skipped rows."""
    skipped = []
    for r in rows:
        got = res.get(r)
        if not got:
            raise GateFail(4, "wine", f"wine:{r} produced no result line")
        for st, mode, reason in got:
            if st in ("FAIL", "ERROR"):
                raise GateFail(4, "wine", f"wine:{r}{mode} {st} {reason.strip()}")
            if st == "SKIP":
                if "DISPLAY" in reason:
                    skipped.append(r)
                else:
                    raise GateFail(4, "wine", f"wine:{r}{mode} SKIP {reason.strip()}")
    return skipped


def step_wine(rows, scratch: Path, runner=run) -> list:
    if not rows:
        return []
    env = {**os.environ, "KPM_DIR": str(scratch)}
    rc, out = runner([PY, str(RUN_TESTS), "--wine", *(f"wine:{r}" for r in rows)], env=env)
    return judge_wine(rows, parse_wine(out))


def git_head() -> str:
    p = subprocess.run(["git", "-C", str(REPO), "rev-parse", "HEAD"], stdout=subprocess.PIPE, text=True)
    return p.stdout.strip()[:12]


# ---------------------------------------------------------------- gate

def gate(mods, skip_default=False, scratch=None, runner=run, full_default=False, records_dir=None) -> int:
    scratch = safe_scratch(scratch or default_scratch())
    ids = {}
    for m in mods:   # collect every id first so a failure on mod N still invalidates the records of the others
        mf = mod_dir(m) / "manifest.toml"
        try:
            ids[m] = manifest_id(mf.read_text()) if mf.is_file() else ""
        except (OSError, tomllib.TOMLDecodeError):
            ids[m] = ""
    step1 = "skipped"
    try:
        for m in mods:
            if not (mod_dir(m) / "manifest.toml").is_file():
                raise GateFail(0, "args", f"{m}: no manifest.toml under {mod_dir(m)}")
        if skip_default:
            print("WARN  step 1 default tier skipped (--skip-default)")
        else:
            step1 = step_default(runner, mods, scratch, full_default)
        print("== step 2/4: build from source", flush=True)
        dlls = step_build(mods, scratch / "build", runner=runner)
        print("== step 3/4: scratch KPM tree + exports check on the fresh DLLs", flush=True)
        packed = {}
        rows = step_scratch({ids[m]: dlls[m] for m in mods}, scratch, packed=packed)
        step_exports([ids[m] for m in mods], packed, scratch, runner)
        all_rows = []
        for mid, rs in rows.items():
            if rs:
                print(f"INFO  {mid}: smokes {', '.join(rs)}")
            else:
                print(f"NOTE no Wine smoke for {mid}")
            all_rows += [r for r in rs if r not in all_rows]
        print("== step 4/4: Wine smokes against the fresh DLLs", flush=True)
        skipped = step_wine(all_rows, scratch, runner)
    except GateFail as e:
        print(str(e), flush=True)
        drop_gate_records(ids.values(), records_dir)
        if ml:
            ml.log_event("BUILD_GATE", {
                "verdict": "FAIL",
                "error": str(e),
                "head": git_head(),
            })
        return 1
    except Exception:
        drop_gate_records(ids.values(), records_dir)
        raise
    for r in skipped:
        print(f"WARN  wine:{r} SKIP (no DISPLAY): not proven by this gate")
    print(f"GATE PASS mods={','.join(ids.values())} smokes={','.join(all_rows) or '-'} head={git_head()}")
    for m in mods:
        print(f"  md5 {md5(dlls[m])}  {ids[m]}  {dlls[m]}")
    for m in mods:
        try:
            write_gate_record(ids[m], dlls[m], scratch, records_dir, mod_dirty(m), step1)
        except OSError as e:
            print(f"WARN  could not write the gate record for {ids[m]}: {e}")
    if ml:
        ml.log_event("BUILD_GATE", {
            "verdict": "PASS",
            "mods": list(ids.values()),
            "smokes": all_rows,
            "head": git_head(),
        })
    return 0


def selftest() -> int:
    """(a) a syntax error must FAIL at step 2; (b) a DLL without the smoke's exports must FAIL at step 4."""
    base = safe_scratch(TMP_BASE / f"gate-selftest-{os.getpid()}-{int(time.time())}")
    shutil.rmtree(base, ignore_errors=True)
    ok = True

    print("== selftest (a): syntax error in perflab source -> step 2 FAIL", flush=True)
    fake = base / "repo"
    src = MODS / "perf_lab_src" / "perflab"
    dst = fake / "patch_manager_mods" / "perf_lab_src" / "perflab"
    shutil.copytree(src, dst)
    if (MODS / "_shared_src").is_dir():
        shutil.copytree(MODS / "_shared_src", fake / "patch_manager_mods" / "_shared_src")
    cpp = dst / "PerfLab.cpp"
    cpp.write_text(cpp.read_text() + "\nint gate_selftest_broken( {\n")
    try:
        step_build(["perf_lab_src/perflab"], base / "build_a", repo=fake)
        print("  selftest BAD (a): broken source built")
        ok = False
    except GateFail as e:
        good = e.n == 2
        print(f"  selftest {'ok ' if good else 'BAD'} (a): {e}")
        ok &= good

    print("== selftest (b): perflab smokes fed load_hang_probe's DLL -> step 4 FAIL (missing export)", flush=True)
    try:
        dll = step_build(["load_hang_probe_src"], base / "build_b")["load_hang_probe_src"]
        rows = step_scratch({"perf-lab": dll}, base / "scratch_b")["perf-lab"]
        if not rows:
            raise GateFail(3, "scratch", "no perf-lab smoke rows found (live PerfLab.kpatch missing?)")
        step_wine(rows[:1], base / "scratch_b")
        print("  selftest BAD (b): wrong DLL passed the smoke")
        ok = False
    except GateFail as e:
        good = e.n == 4
        print(f"  selftest {'ok ' if good else 'BAD'} (b): {e}")
        ok &= good
    print("SELFTEST PASS" if ok else "SELFTEST FAIL")
    return 0 if ok else 1


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--mod", action="append", default=[], metavar="DIR",
                    help="mod source dir relative to patch_manager_mods (e.g. perf_lab_src/perflab); repeatable")
    ap.add_argument("--skip-default", action="store_true", help="skip step 1 (run_tests.py default tier)")
    ap.add_argument("--full-default", action="store_true",
                    help="step 1 runs the whole default tier instead of the scoped ids")
    ap.add_argument("--selftest", action="store_true", help="two broken builds must FAIL (steps 2 and 4)")
    a = ap.parse_args(argv)
    if _load_limit:
        _load_limit.lower_priority()
    if a.selftest:
        return selftest()
    if not a.mod:
        ap.error("--mod is required (or --selftest)")
    return gate(a.mod, a.skip_default, full_default=a.full_default)


if __name__ == "__main__":
    sys.exit(main())
