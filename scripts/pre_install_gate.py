#!/usr/bin/env python3
"""Pre-install gate (doc 20 §4.2 / Phase 6, doc 24 T8): prove the source being installed is the source being smoked.

  scripts/pre_install_gate.sh --mod perf_lab_src/perflab                 # GATE PASS / GATE FAIL <n> <step>: <reason>
  scripts/pre_install_gate.sh --mod load_hang_probe_src --mod perf_lab_src/perflab
  scripts/pre_install_gate.sh --selftest                                # two broken builds must both FAIL

Steps (stop at the first failure; exit 1 on FAIL, 0 on PASS):
  1 default   run_tests.py, scoped: the tests/static checks compat_check.py plans for the gated mods' tracked source
              files + GLOBAL_SUITES (intersected with `run_tests.py --list`), via --ids-file. compat_check failing or
              planning nothing falls back to the full default tier with a WARN; --full-default forces the full tier;
              --skip-default skips it with a warning (selftest only). A failure set of exactly
              {suite:check-exports} is tolerated with a WARN (it checks the stale live kpatch; see step 3)
  2 build     build_check.py --mod <m> --keep --build-root <scratch>/build (a fresh windows_x86.dll from source)
  3 scratch   for every smokes.toml row whose kpatch (or extra_dlls kpatch) carries the mod's manifest id: copy the
              live kpatch to <scratch>/Patches/<same rel path> and replace binaries/windows_x86.dll with the fresh DLL
              (a mod with no smoke row gets its live kpatch looked up by manifest id); then check_exports.py runs on
              those scratch kpatches, so a new detour export is judged on the fresh DLL, not the stale KPM one;
              Patches/Common (kpatch_api.h etc.) is copied in too, since the smokes compile with -I$KPM_DIR/Patches/Common
  4 wine      KPM_DIR=<scratch> run_tests.py --wine -k wine: ; each matched row must PASS (SKIP only for missing
              DISPLAY). Rows of other mods SKIP (their kpatch is absent from the scratch tree). A mod with no smoke
              row prints NOTE and still passes.
  5 report    GATE PASS mods=<ids> smokes=<rows> head=<sha12> + the md5 of every rebuilt DLL (the md5 that
              stage_and_verify_install.py verify compares)

On PASS one JSON record per mod is written (atomically) to $TMPDIR/kotor-tests/gate-records/<mod id>.json with
{mod_id, md5, dll, head, time, scratch, dirty}; kpm_hotswap.py reads it. A GATE FAIL deletes the
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
                      dirty: bool = False) -> Path:
    """Atomically write the PASS record kpm_hotswap.py reads: {mod_id, md5, dll, head, time, scratch, dirty}."""
    d = records_dir or TMP_BASE / "gate-records"
    d.mkdir(parents=True, exist_ok=True)
    rec = {"mod_id": mod_id, "md5": md5(dll), "dll": str(dll), "head": git_head(),
           "time": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
           "scratch": str(scratch), "dirty": bool(dirty)}
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

def _git(repo: Path, *args) -> str:
    p = subprocess.run(["git", "-C", str(repo), *args], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    return p.stdout.decode("utf-8", "replace")


def mod_source_files(mods, repo: Path = REPO) -> list:
    """Repo-relative tracked + untracked (not ignored) files under each gated mod's dir AND under its top-level
    patch_manager_mods/<x>_src dir (a nested --mod such as perf_lab_src/perflab has sibling py tests one level up)."""
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
            files += [f for f in out.split("\0") if f and f not in files]
    return files


DIRTY_SUFFIXES = {".h", ".hpp", ".inc", ".cpp", ".py", ".toml"}


def dirty_scope_files(exclude, repo: Path = REPO) -> list:
    """Dirty/untracked paths (git status --porcelain -uall) under patch_manager_mods/**, tests/include/** that can
    change a build or test and are not already in `exclude`. They go to compat_check too: a shared header edit
    must select the tests of its includers."""
    out = _git(repo, "status", "--porcelain", "-uall", "-z").split("\0")
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
    return bool(_git(repo, "status", "--porcelain", "--", str(rel)).strip())


def scoped_ids(mods, scratch: Path, runner=run, repo: Path = REPO):
    """Item ids for the scoped step 1, or None when the scope cannot be computed (caller falls back to the full tier)."""
    files = mod_source_files(mods, repo)
    if not files:
        print("WARN  step 1: no source files found for the gated mods")
        return None
    dirty = dirty_scope_files(set(files), repo)
    if dirty:
        print(f"INFO  step 1 also scopes {len(dirty)} dirty shared file(s): {', '.join(dirty[:5])}"
              + (", ..." if len(dirty) > 5 else ""))
    scratch.mkdir(parents=True, exist_ok=True)
    cj = scratch / "compat.json"
    cj.unlink(missing_ok=True)
    rc, out = runner([PY, str(COMPAT_CHECK), *files, *dirty, "--list", "--json", str(cj)], quiet=True)
    try:
        data = json.loads(cj.read_text())
        plan = data["plan"]
    except (OSError, ValueError, KeyError, TypeError):
        print(f"WARN  step 1: compat_check.py gave no usable plan (rc={rc})")
        return None
    if rc != 0:
        print(f"WARN  step 1: compat_check.py rc={rc}")
        return None
    # files we picked ourselves from the gated mod dirs (README, fixtures, a nested mod's parent dir) may be
    # unclassified by compat_check by construction; anything else unclassified (dirty files elsewhere) is a real gap
    own = set(files)
    bad = [f"gap: {g}" for g in data.get("gaps") or []] + \
          [f"unclassified: {u}" for u in data.get("unclassified") or [] if u not in own]
    if bad:
        print(f"WARN  step 1: compat_check.py reports {len(bad)} gap/unclassified item(s): {'; '.join(bad[:5])}")
        return None
    tops = set()
    for m in mods:
        try:
            tops.add(mod_dir(m, repo).relative_to((Path(repo) / "patch_manager_mods").resolve()).parts[0])
        except (ValueError, IndexError):
            pass
    other_builds = [e["id"].split(":", 1)[-1] for e in plan
                    if e.get("kind") == "build" and e.get("id")
                    and e["id"].split(":", 1)[-1].split("/")[0] not in tops]
    if other_builds:
        print(f"INFO  step 1: {len(other_builds)} other mod(s) planned for build, not built by this gate: "
              f"{', '.join(other_builds[:5])}")
    wanted = [e["id"] for e in plan if e.get("kind") != "build" and e.get("id")]
    rc, out = runner([PY, str(RUN_TESTS), "--list"], quiet=True)
    if rc != 0:
        print(f"WARN  step 1: run_tests.py --list rc={rc}")
        return None
    known = {l.split()[0] for l in out.splitlines() if l.strip()}
    ids = []
    for i in wanted + GLOBAL_SUITES:
        if i not in known:
            print(f"WARN  step 1: {i} is not in run_tests.py --list; dropped from the scoped set")
        elif i not in ids:
            ids.append(i)
    return ids or None


def step_default(runner=run, mods=None, scratch: Path | None = None, full: bool = False):
    cmd = [PY, str(RUN_TESTS)]
    scratch = scratch or default_scratch()
    ids = None if full or not mods else scoped_ids(mods, scratch, runner)
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
        return
    if rc != 0 or bad:
        raise GateFail(1, "default", f"run_tests.py rc={rc}; failing: {', '.join(bad) or '(see output)'}")


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
    rc, out = runner([PY, str(RUN_TESTS), "--wine", "-k", "wine:"], env=env)
    return judge_wine(rows, parse_wine(out))


def git_head() -> str:
    p = subprocess.run(["git", "-C", str(REPO), "rev-parse", "HEAD"], stdout=subprocess.PIPE, text=True)
    return p.stdout.strip()[:12]


# ---------------------------------------------------------------- gate

def gate(mods, skip_default=False, scratch=None, runner=run, full_default=False, records_dir=None) -> int:
    scratch = safe_scratch(scratch or default_scratch())
    try:
        ids = {}
        for m in mods:
            mf = mod_dir(m) / "manifest.toml"
            if not mf.is_file():
                raise GateFail(0, "args", f"{m}: no manifest.toml under {mod_dir(m)}")
            ids[m] = manifest_id(mf.read_text())
        if skip_default:
            print("WARN  step 1 default tier skipped (--skip-default)")
        else:
            step_default(runner, mods, scratch, full_default)
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
    for r in skipped:
        print(f"WARN  wine:{r} SKIP (no DISPLAY): not proven by this gate")
    print(f"GATE PASS mods={','.join(ids.values())} smokes={','.join(all_rows) or '-'} head={git_head()}")
    for m in mods:
        print(f"  md5 {md5(dlls[m])}  {ids[m]}  {dlls[m]}")
    for m in mods:
        try:
            write_gate_record(ids[m], dlls[m], scratch, records_dir, mod_dirty(m))
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
