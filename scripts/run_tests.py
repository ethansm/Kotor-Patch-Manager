#!/usr/bin/env python3
"""Single entry point for every test in the repo (doc 20 section 4.1, Phase 0).

  python3 scripts/run_tests.py                 # host C++ tests + Python tests + explicit suites
  python3 scripts/run_tests.py --list          # what would run, and what would SKIP
  python3 scripts/run_tests.py -k twoda -v     # filter on item id, print full output
  python3 scripts/run_tests.py --wine          # also the Wine smoke tests (harness/smokes.toml)
  python3 scripts/run_tests.py --selftest      # prove the runner reports failures as FAIL

Items and how they are found
  cpp:<stem>     patch_manager_mods/**/*_test.cpp. Built with
                 g++ -std=c++17 -O1 -Wall -Wextra -I <test dir> -I tests/include -fsanitize=address,undefined
                 into $TMPDIR/kotor-tests/host/, run with cwd = the test's directory.
                 (research/**/parity/*_test.cpp are NOT discovered here; parity.py builds those.)
  py:<path>      **/test_*.py, run as `python3 <file>` with cwd = its directory. A `tests/` directory that holds
                 test_*.py plus a helpers.py is ONE item, `py:<dir>(unittest)`, run as
                 `.venv/bin/python -m unittest discover -s <dir>` from the repo root.
  suite:<name>   explicit entries of tests/suites.toml (parity scripts, kpatch checkers).
  wine:<n>[mode] with --wine: smokes from patch_manager_mods/perf_lab_src/harness/smokes.toml.

Discovery skips .venv, __pycache__, .git, node_modules, any `retired/`, `testdata/`, `fixtures/` directory
and any nested git worktree/clone (a directory that has its own .git). tests/suites.toml [discovery] exclude
lists test-looking files to leave out, with reasons.

Colocated test.toml (optional, next to a *_test.cpp)
    cxxflags    = ["-DFOO", "-I../x"]   # extra compiler flags; build cwd = the test's dir
    args        = ["testdata/spells.2da"]   # argv passed to the test binary; run cwd = the test's dir
    skip_reason = "needs X"             # item is SKIP, not run
  When several tests share a directory, give per-test tables keyed by the test's file stem; they override the
  top-level keys of the same name, which act as defaults for the whole directory:
    cxxflags = ["-DCOMMON"]
    [twoda_test]
    args = ["testdata/spells.2da"]
  $VAR and ~ in args/cxxflags are expanded.

Result per item: PASS (exit 0), FAIL (positive exit status, or build failure), SKIP (missing requirement),
ERROR (timeout, killed by a signal, could not launch). Exit status 1 if any FAIL or ERROR.
Builds and logs live under $TMPDIR/kotor-tests/ (default /tmp/claude-<uid>/kotor-tests), never in the repo.
"""
import argparse
import concurrent.futures as cf
import json
import os
import re
import shutil
import signal
import subprocess
import sys
import threading
import time
import tomllib
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))
try:
    import _common  # PROTON_WINE, GAME_DIR (same env overrides as the compatch scripts)
    _PROTON_WINE = Path(_common.PROTON_WINE)
    _GAME_DIR = Path(_common.GAME_DIR)
except Exception:  # pragma: no cover - _common is optional
    _PROTON_WINE = Path("~/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine").expanduser()
    _GAME_DIR = Path("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II").expanduser()

TMP = Path(os.environ.get("TMPDIR") or f"/tmp/claude-{os.getuid()}") / "kotor-tests"
KPM_DIR = Path(os.environ.get("KPM_DIR") or "~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager").expanduser()
VENV_PY = REPO / ".venv" / "bin" / "python"
HARNESS = REPO / "patch_manager_mods" / "perf_lab_src" / "harness"
SMOKES_TOML = HARNESS / "smokes.toml"
SUITES_TOML = REPO / "tests" / "suites.toml"
CPP_ROOT = REPO / "patch_manager_mods"

PRUNE_DIRS = {".venv", "__pycache__", ".git", "node_modules", "retired", "testdata", "fixtures"}
TAIL_LINES = 40
_print_lock = threading.Lock()


# ---------------------------------------------------------------- data model

@dataclass
class Item:
    id: str
    kind: str                      # cpp | py | suite | wine
    skip: str = ""                 # non-empty = SKIP with this reason
    cwd: Path = REPO
    env: dict = field(default_factory=dict)
    timeout: int = 300
    build_cmd: list | None = None  # optional build step (cwd = build_cwd or cwd)
    build_cwd: Path | None = None
    build_timeout: int = 600
    cmd: list | None = None        # the run step
    pre: object = None             # optional callable() -> str|None run before build; str = ERROR message


@dataclass
class Result:
    id: str
    kind: str
    status: str                    # PASS FAIL SKIP ERROR
    seconds: float = 0.0
    reason: str = ""
    output: str = ""


# ---------------------------------------------------------------- helpers

def expand(s: str) -> str:
    """Expand ${VAR:-default}, $VAR and ~ (REPO, KPM_DIR, KOTOR2_ROOT are always defined)."""
    env = {"REPO": str(REPO), "KPM_DIR": str(KPM_DIR), "KOTOR2_ROOT": str(_GAME_DIR), **os.environ}

    def sub(m):
        name, default = m.group(1), m.group(2)
        return env.get(name) or (default if default is not None else "")
    s = re.sub(r"\$\{(\w+)(?::-([^}]*))?\}", sub, s)
    s = re.sub(r"\$(\w+)", lambda m: env.get(m.group(1), m.group(0)), s)
    return os.path.expanduser(s)


def suite_cwd(cwd) -> Path:
    """A suite's cwd: repo-relative, or expanded when it starts with $ or ~ (e.g. $KPM_DIR)."""
    if not cwd:
        return REPO
    return Path(expand(cwd)) if cwd[0] in "$~" else REPO / cwd


def missing_requirement(requires) -> str:
    for r in requires or []:
        if r.startswith("which:"):
            if not shutil.which(r[6:]):
                return f"missing {r[6:]}"
        elif not Path(expand(r)).exists():
            return f"missing {expand(r)}"
    return ""


_FATAL_SIGNALS = {int(x) for x in (signal.SIGILL, signal.SIGABRT, signal.SIGBUS, signal.SIGFPE, signal.SIGKILL,
                                   signal.SIGSEGV, signal.SIGTERM)}


def signal_name(n: int) -> str:
    try:
        return signal.Signals(n).name
    except ValueError:
        return f"signal {n}"


def run_cmd(cmd, cwd, env=None, timeout=300):
    """Run one command in its own process group. Returns (status, reason, output) where status is
    'OK' (exit 0), 'FAIL', or 'ERROR'."""
    full_env = {**os.environ, **(env or {})}
    try:
        p = subprocess.Popen([str(c) for c in cmd], cwd=str(cwd), env=full_env, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL, start_new_session=True)
    except (OSError, ValueError) as e:
        return "ERROR", f"could not launch: {e}", ""
    try:
        out, _ = p.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        try:
            os.killpg(p.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        try:
            out, _ = p.communicate(timeout=10)
        except subprocess.TimeoutExpired:
            out = b""
        return "ERROR", f"timeout after {timeout}s", out.decode("utf-8", "replace")
    text = out.decode("utf-8", "replace")
    # Kill stragglers that outlived the main process (Wine helpers etc.).
    try:
        os.killpg(p.pid, signal.SIGKILL)
    except (ProcessLookupError, PermissionError):
        pass
    rc = p.returncode
    if rc == 0:
        return "OK", "", text
    if rc < 0:
        return "ERROR", f"killed by {signal_name(-rc)}", text
    if rc - 128 in _FATAL_SIGNALS:  # a shell/wrapper reporting a signal death as 128+N
        return "ERROR", f"killed by {signal_name(rc - 128)} (exit {rc})", text
    return "FAIL", f"exit {rc}", text


def execute(item: Item) -> Result:
    """The one build-then-run code path shared by every kind of item (and --selftest)."""
    t0 = time.monotonic()

    def done(status, reason="", output=""):
        return Result(item.id, item.kind, status, time.monotonic() - t0, reason, output)

    if item.skip:
        return done("SKIP", item.skip)
    if item.pre:
        msg = item.pre()
        if msg:
            return done("ERROR", msg)
    log = ""
    if item.build_cmd:
        st, why, out = run_cmd(item.build_cmd, item.build_cwd or item.cwd, item.env, item.build_timeout)
        if st != "OK":
            return done("FAIL" if st == "FAIL" else "ERROR", f"build failed ({why})", out)
        log = out
    st, why, out = run_cmd(item.cmd, item.cwd, item.env, item.timeout)
    out = (log + out) if (log and st != "OK") else out
    if st == "OK":
        return done("PASS", "", out)
    return done(st, why, out)


# ---------------------------------------------------------------- discovery

def walk(root: Path):
    """Yield (dirpath, filenames) under root, pruning junk dirs and nested git checkouts."""
    for dp, dns, fns in os.walk(root):
        dpp = Path(dp)
        dns[:] = sorted(d for d in dns if d not in PRUNE_DIRS and not (dpp / d / ".git").exists())
        yield dpp, sorted(fns)


def load_toml(path: Path) -> dict:
    try:
        return tomllib.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        return {}
    except tomllib.TOMLDecodeError as e:
        raise SystemExit(f"{path}: {e}")


def rel(p: Path) -> str:
    try:
        return str(p.relative_to(REPO))
    except ValueError:
        return str(p)


def _prepare_build(exe: Path):
    if not shutil.which("g++"):
        return "g++ not found"
    exe.parent.mkdir(parents=True, exist_ok=True)
    return None


def cpp_item(src: Path, sanitize=True, extra_flags=(), tag="", include_dirs=None, cfg=None) -> Item:
    stem = src.stem
    d = src.parent
    if cfg is None:
        raw = load_toml(d / "test.toml")
        sub = raw.get(stem) if isinstance(raw.get(stem), dict) else {}
        cfg = {k: v for k, v in raw.items() if not isinstance(v, dict)}
        cfg.update(sub)
    exe = TMP / "host" / (f"{stem}{tag}")
    flags = ["-std=c++17", "-O1", "-Wall", "-Wextra", "-I", str(d), "-I", str(REPO / "tests" / "include")]
    for inc in include_dirs or []:
        flags += ["-I", str(inc)]
    if sanitize:
        flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    flags += list(extra_flags)
    post = [expand(f) for f in cfg.get("cxxflags", [])]  # after the source so -l libs link; -D/-I work anywhere
    return Item(
        id=f"cpp:{stem}{tag}", kind="cpp", skip=expand(cfg.get("skip_reason", "")), cwd=d,
        build_cmd=["g++"] + flags + [str(src), "-o", str(exe)] + post, build_cwd=d,
        cmd=[str(exe)] + [expand(a) for a in cfg.get("args", [])], timeout=120,
        env={"KT_VERBOSE": "", "UBSAN_OPTIONS": "print_stacktrace=1",
             "ASAN_OPTIONS": "abort_on_error=0"},
        pre=lambda: _prepare_build(exe),
    )


def py_item(path: Path, tag="") -> Item:
    return Item(id=f"py:{rel(path)}{tag}", kind="py", cwd=path.parent, cmd=[sys.executable, path.name],
                timeout=300, env={"PYTHONDONTWRITEBYTECODE": "1"})


def unittest_item(d: Path) -> Item:
    skip = "" if VENV_PY.exists() else f"missing {rel(VENV_PY)}"
    return Item(id=f"py:{rel(d)}(unittest)", kind="py", skip=skip, cwd=REPO,
                cmd=[str(VENV_PY), "-m", "unittest", "discover", "-s", str(d)], timeout=600,
                env={"PYTHONDONTWRITEBYTECODE": "1"})


def discover(sanitize=True):
    suites_cfg = load_toml(SUITES_TOML)
    excludes = {}
    for e in suites_cfg.get("discovery", {}).get("exclude", []):
        excludes[(REPO / e["path"]).resolve()] = e.get("reason", "excluded")
    items = []

    # C++ host tests, only under patch_manager_mods/ (parity tests live in research/ and are run by parity.py).
    seen = {}
    for dp, fns in walk(CPP_ROOT):
        for fn in fns:
            if fn.endswith("_test.cpp"):
                src = dp / fn
                it = cpp_item(src, sanitize)
                if it.id in seen:  # same stem twice: disambiguate by directory
                    it.id = f"cpp:{rel(src)}"
                seen[it.id] = 1
                if src.resolve() in excludes:
                    it.skip = f"excluded: {excludes[src.resolve()]}"
                items.append(it)

    # Python tests.
    unittest_dirs = set()
    pyfiles = []
    for dp, fns in walk(REPO):
        for fn in fns:
            if fn.startswith("test_") and fn.endswith(".py"):
                pyfiles.append(dp / fn)
        if dp.name == "tests" and (dp / "helpers.py").exists() and any(
                f.startswith("test_") and f.endswith(".py") for f in fns):
            unittest_dirs.add(dp)
    for d in sorted(unittest_dirs):
        it = unittest_item(d)
        if d.resolve() in excludes:
            it.skip = f"excluded: {excludes[d.resolve()]}"
        items.append(it)
    for f in pyfiles:
        if f.parent in unittest_dirs:
            continue
        it = py_item(f)
        if f.resolve() in excludes:
            it.skip = f"excluded: {excludes[f.resolve()]}"
        items.append(it)

    # Explicit suites.
    for s in suites_cfg.get("suite", []):
        cmd = list(s["cmd"])
        skip = missing_requirement(s.get("requires"))
        if cmd[0] == "python":
            if s.get("python") == "venv":
                cmd[0] = str(VENV_PY)
                skip = skip or ("" if VENV_PY.exists() else f"missing {rel(VENV_PY)}")
            else:
                cmd[0] = sys.executable
        cmd = [expand(c) if i else c for i, c in enumerate(cmd)]
        items.append(Item(id=f"suite:{s['name']}", kind="suite", skip=skip,
                          cwd=suite_cwd(s.get("cwd")), cmd=cmd,
                          timeout=int(s.get("timeout_s", 300)), env={"PYTHONDONTWRITEBYTECODE": "1"}))
    return items


# ---------------------------------------------------------------- wine smokes

def wine_binary() -> Path:
    return Path(os.environ.get("PROTON_WINE") or _PROTON_WINE).expanduser()


def wine_env() -> dict:
    return {"WINEPREFIX": str(TMP / "wineprefix"), "WINEDEBUG": "-all",
            "WINEDLLOVERRIDES": "mscoree,mshtml="}  # no Mono/Gecko install prompts


def find_runtime_dlls(cfg) -> tuple[dict, str]:
    """Locate the MinGW runtime DLLs. Returns ({name: path}, missing_name_or_empty).
    smokes.toml: mingw_runtime_dirs (list of dirs/globs, searched in order, first hit wins; legacy single
    mingw_runtime_dir also accepted) and mingw_runtime_dll_names (default: the three below)."""
    import glob
    names = cfg.get("mingw_runtime_dll_names") or ["libgcc_s_dw2-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll"]
    dirs = list(cfg.get("mingw_runtime_dirs") or [])
    if cfg.get("mingw_runtime_dir"):
        dirs.append(cfg["mingw_runtime_dir"])
    found = {}
    for n in names:
        for pat in dirs:
            hits = sorted(glob.glob(expand(pat)))
            for d in hits:
                if (Path(d) / n).is_file():
                    found[n] = Path(d) / n
                    break
            if n in found:
                break
        if n not in found:
            return found, n
    return found, ""


def wine_plan(cfg_path: Path):
    """Return (smokes, error). Each smoke dict gets `_dlls` (runtime DLL paths) and `_dll_missing`."""
    if not cfg_path.is_file():
        return [], f"smokes.toml missing ({rel(cfg_path)})"
    cfg = load_toml(cfg_path)
    dlls, missing = find_runtime_dlls(cfg)
    return [{**s, "_dlls": dlls, "_dll_missing": missing} for s in cfg.get("smoke", [])], ""


def smoke_skip(s) -> str:
    if not shutil.which("i686-w64-mingw32-g++"):
        return "missing i686-w64-mingw32-g++"
    if not wine_binary().exists():
        return f"missing {wine_binary()}"
    kp = KPM_DIR / "Patches" / s["kpatch"]
    if not kp.exists():
        return f"missing {kp}"
    for x in s.get("extra_dlls", []):
        if not (KPM_DIR / "Patches" / x["kpatch"]).exists():
            return f"missing {KPM_DIR / 'Patches' / x['kpatch']}"
    if s.get("runtime_dlls") and s["_dll_missing"]:
        return f"missing {s['_dll_missing']} (mingw_runtime_dirs)"
    if s.get("needs_display") and not os.environ.get("DISPLAY"):
        return "missing DISPLAY"
    return ""


def _extract_dll(kpatch: Path, dest: Path):
    with zipfile.ZipFile(kpatch) as z:
        dest.write_bytes(z.read("binaries/windows_x86.dll"))


def smoke_build(s) -> Result:
    """Compile the smoke, extract the patch DLL(s), copy runtime DLLs. Returns Result (PASS = built)."""
    t0 = time.monotonic()
    name = s["name"]
    bdir = TMP / "wine" / name
    rid = f"wine:{name}"

    def res(status, reason="", out=""):
        return Result(rid, "wine", status, time.monotonic() - t0, reason, out)
    try:
        if bdir.exists():
            shutil.rmtree(bdir)
        bdir.mkdir(parents=True)
        cmd = (["i686-w64-mingw32-g++", "-std=c++17", "-O1", "-I", str(HARNESS), "-I", str(REPO / "tests" / "include")]
               + [str(HARNESS / s["src"]), "-o", str(bdir / f"{name}.exe")] + [expand(f) for f in s.get("cxxflags", [])])
        st, why, out = run_cmd(cmd, HARNESS, None, 600)
        if st != "OK":
            return res("FAIL" if st == "FAIL" else "ERROR", f"build failed ({why})", out)
        _extract_dll(KPM_DIR / "Patches" / s["kpatch"], bdir / s["dll_name"])
        for x in s.get("extra_dlls", []):
            _extract_dll(KPM_DIR / "Patches" / x["kpatch"], bdir / x["dll_name"])
        if s.get("runtime_dlls"):
            for p in s["_dlls"].values():
                shutil.copy2(p, bdir / p.name)
    except (OSError, KeyError, TypeError, zipfile.BadZipFile) as e:
        return res("ERROR", f"{type(e).__name__}: {e}")
    return res("PASS", "", out)


def init_prefix() -> str:
    """Create the private wine prefix once. Returns '' or an error message."""
    prefix = TMP / "wineprefix"
    if (prefix / "system.reg").exists():
        return ""
    prefix.mkdir(parents=True, exist_ok=True)
    st, why, out = run_cmd([str(wine_binary()), "wineboot", "--init"], prefix, wine_env(), 600)
    if st == "ERROR":
        return f"wineboot: {why}\n{out[-2000:]}"
    return ""


def wine_kill_server():
    ws = wine_binary().with_name("wineserver")
    if ws.exists():
        run_cmd([str(ws), "-k"], REPO, wine_env(), 30)


def wine_run_item(s, mode) -> Result:
    name = s["name"]
    rid = f"wine:{name}" + (f"[{mode}]" if mode else "")
    t0 = time.monotonic()
    cmd = [str(wine_binary()), f"{name}.exe"] + ([mode] if mode else [])
    st, why, out = run_cmd(cmd, TMP / "wine" / name, wine_env(), int(s.get("timeout_s", 120)))
    if st == "ERROR" and why.startswith("timeout"):
        wine_kill_server()
    status = "PASS" if st == "OK" else st
    return Result(rid, "wine", status, time.monotonic() - t0, "" if st == "OK" else why, out)


# ---------------------------------------------------------------- reporting

def tail(text: str, n=TAIL_LINES) -> str:
    return "\n".join(text.rstrip("\n").splitlines()[-n:])


def report(r: Result, verbose: bool):
    line = f"{r.status:<5}  {r.id}  {r.seconds:.1f}s" + (f"  {r.reason}" if r.reason else "")
    with _print_lock:
        print(line, flush=True)
        if verbose and r.output.strip():
            print(r.output.rstrip("\n"), flush=True)
        elif r.status in ("FAIL", "ERROR") and r.output.strip():
            print("    --- last %d lines ---" % TAIL_LINES)
            print("\n".join("    " + l for l in tail(r.output).splitlines()), flush=True)


def selftest(jobs: int, verbose: bool) -> int:
    """Run deliberately failing items through the real build/run/classify path; each must be FAIL."""
    sd = REPO / "tests" / "selftest"
    items = [
        cpp_item(sd / "fail_cpp.cpp", True, tag="", cfg={}),
        cpp_item(sd / "fail_cpp.cpp", True, extra_flags=["-DNDEBUG"], tag="_ndebug", cfg={}),
        py_item(sd / "fail_py.py"),
    ]
    for it in items:
        it.id = "selftest:" + it.id.split(":", 1)[1]
    with cf.ThreadPoolExecutor(jobs) as ex:
        results = list(ex.map(execute, items))
    ok = True
    for r in results:
        report(r, verbose)
        good = r.status == "FAIL"
        if r.kind == "cpp":
            good = good and "1 == 2" in r.output and "1 failed" in r.output
        print(f"  selftest {'ok ' if good else 'BAD'} {r.id} (want FAIL, got {r.status})")
        ok &= good
    print("SELFTEST " + ("PASSED: failing tests are reported as FAIL" if ok else "FAILED"))
    return 0 if ok else 1


def main() -> int:
    ap = argparse.ArgumentParser(description="Run every test in the repo.", formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog="See the module docstring (head of this file) for discovery rules and test.toml.")
    ap.add_argument("--list", action="store_true", help="list items and would-skip reasons; run nothing")
    ap.add_argument("-k", metavar="SUBSTR", help="only items whose id contains SUBSTR")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4, help="parallel builds/host runs (default: cpu count)")
    ap.add_argument("--wine", action="store_true", help="also run the Wine smokes from harness/smokes.toml")
    ap.add_argument("--json", metavar="PATH", help="write machine-readable results")
    ap.add_argument("-v", action="store_true", help="print the full output of every item (as each finishes)")
    ap.add_argument("--no-sanitize", action="store_true", help="drop -fsanitize=address,undefined from C++ builds")
    ap.add_argument("--selftest", action="store_true", help="check that failing tests are reported as FAIL")
    a = ap.parse_args()

    if a.selftest:
        return selftest(a.jobs, a.v)

    t_start = time.monotonic()
    items = discover(sanitize=not a.no_sanitize)
    smokes, wine_err = ([], "")
    if a.wine:
        smokes, wine_err = wine_plan(SMOKES_TOML)
    if a.k:
        items = [i for i in items if a.k in i.id]

    # Wine items: (smoke, mode) -> id
    wine_items = []  # (id, smoke, mode, skip)
    for s in smokes:
        skip = smoke_skip(s)
        for mode in (s.get("modes") or [""]):
            wid = f"wine:{s['name']}" + (f"[{mode}]" if mode else "")
            if not a.k or a.k in wid:
                wine_items.append((wid, s, mode, skip))

    if a.list:
        for i in items:
            print(f"{i.id}  [{i.kind}]" + (f"  SKIP ({i.skip})" if i.skip else ""))
        if a.wine:
            if wine_err:
                print(f"wine  [wine]  ERROR ({wine_err})")
            for wid, s, mode, skip in wine_items:
                print(f"{wid}  [wine]" + (f"  SKIP ({skip})" if skip else ""))
        return 0

    results: list[Result] = []

    def emit(r):
        results.append(r)
        report(r, a.v)

    TMP.mkdir(parents=True, exist_ok=True)
    runnable_smokes = {}
    for wid, s, mode, skip in wine_items:
        if not skip:
            runnable_smokes[s["name"]] = s
    builds = {}
    with cf.ThreadPoolExecutor(max(1, a.jobs)) as ex:
        futs = {ex.submit(execute, i): i for i in items}
        bfuts = {ex.submit(smoke_build, s): n for n, s in runnable_smokes.items()}
        for f in cf.as_completed(set(futs) | set(bfuts)):
            if f in futs:
                emit(f.result())
            else:
                builds[bfuts[f]] = f.result()

    if a.wine:
        if wine_err:
            emit(Result("wine", "wine", "ERROR", 0.0, wine_err))
        prefix_err = ""
        if runnable_smokes and any(b.status == "PASS" for b in builds.values()):
            t0 = time.monotonic()
            prefix_err = init_prefix()
            if prefix_err:
                emit(Result("wine:prefix", "wine", "ERROR", time.monotonic() - t0, "wine prefix setup failed", prefix_err))
        reported_build = set()
        for wid, s, mode, skip in wine_items:
            if skip:
                emit(Result(wid, "wine", "SKIP", 0.0, skip))
                continue
            b = builds[s["name"]]
            if b.status != "PASS":
                if s["name"] not in reported_build:  # one build failure line per smoke
                    reported_build.add(s["name"])
                    emit(b)
                continue
            if prefix_err:
                emit(Result(wid, "wine", "ERROR", 0.0, "wine prefix unavailable"))
                continue
            emit(wine_run_item(s, mode))
        if runnable_smokes:
            wine_kill_server()

    wall = time.monotonic() - t_start
    counts = {k: sum(1 for r in results if r.status == k) for k in ("PASS", "FAIL", "SKIP", "ERROR")}
    print(f"\n{counts['PASS']} passed, {counts['FAIL']} failed, {counts['ERROR']} error, {counts['SKIP']} skipped "
          f"({len(results)} items) in {wall:.1f}s")
    if a.json:
        Path(a.json).write_text(json.dumps(
            {"wall_seconds": round(wall, 2), "counts": counts,
             "results": [{"id": r.id, "kind": r.kind, "status": r.status, "seconds": round(r.seconds, 2),
                          "reason": r.reason, "tail": tail(r.output)} for r in sorted(results, key=lambda r: r.id)]},
            indent=2), encoding="utf-8")
    return 1 if counts["FAIL"] or counts["ERROR"] else 0


if __name__ == "__main__":
    sys.exit(main())
