#!/usr/bin/env python3
"""Single entry point for every test in the repo (doc 20 section 4.1, Phase 0).

  python3 scripts/run_tests.py                 # host C++ tests + Python tests + explicit suites
  python3 scripts/run_tests.py --list          # what would run, and what would SKIP
  python3 scripts/run_tests.py -k twoda -v     # filter on item id, print full output
  python3 scripts/run_tests.py --wine          # also the Wine smoke tests (harness/smokes.toml) and the build tier
  python3 scripts/run_tests.py --build         # also tier="build" suites (scripts/build_check.py, ~8-20 s)
  python3 scripts/run_tests.py --selftest      # prove the runner reports failures as FAIL

Items and how they are found
  cpp:<stem>     patch_manager_mods/**/*_test.cpp and tests/**/*_test.cpp. Built with
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
import contextlib
import functools
import hashlib
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
import _load_limit  # machine-wide compile slots / exclusive locks / low priority (shared with the other gates)
try:
    import _common  # PROTON_WINE, GAME_DIR (same env overrides as the compatch scripts)
    _PROTON_WINE = Path(_common.PROTON_WINE)
    _GAME_DIR = Path(_common.GAME_DIR)
except Exception:  # pragma: no cover - _common is optional
    _PROTON_WINE = Path("~/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine").expanduser()
    _GAME_DIR = Path("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II").expanduser()

TMP = Path(os.environ.get("TMPDIR") or f"/tmp/claude-{os.getuid()}") / "kotor-tests"
TREE_MUTATING_IDS = {"py:scripts/test_compat_check.py"}  # write live repo files temporarily; run after the parallel pool
os.environ["MASTER_LEDGER_TEST"] = "1"
KPM_DIR = Path(os.environ.get("KPM_DIR") or "~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager").expanduser()
VENV_PY = REPO / ".venv" / "bin" / "python"
HARNESS = REPO / "patch_manager_mods" / "perf_lab_src" / "harness"
SMOKES_TOML = HARNESS / "smokes.toml"
SUITES_TOML = REPO / "tests" / "suites.toml"
CPP_ROOT = REPO / "patch_manager_mods"
CPP_ROOTS = (CPP_ROOT, REPO / "tests")   # tests/: cross-mod model tests (e.g. tests/wndproc_chain, doc 20 T3)

PRUNE_DIRS = {".venv", "__pycache__", ".git", "node_modules", "retired", "testdata", "fixtures"}
TAIL_LINES = 40
_print_lock = threading.Lock()
USE_CACHE = True        # --no-cache sets False: always recompile (the cache entry is still refreshed)
WINE_RUN_DIR = None     # per-run smoke build root (TMP/wine/run-<pid>-<epoch>); set by main() for --wine runs


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
    src: Path | None = None        # cpp/py items: the test source
    covers: list[str] = field(default_factory=list)  # patterns/interfaces covered by this item
    consumer: str = ""             # mod directory if this is a consumer contract test
    cache: object = None           # CacheInfo for cpp items whose build may be skipped (content-addressed exe)


@dataclass
class Result:
    id: str
    kind: str
    status: str                    # PASS FAIL SKIP ERROR
    seconds: float = 0.0
    reason: str = ""
    output: str = ""


# ---------------------------------------------------------------- helpers

def canonical_include_flags() -> list[str]:
    """Header dirs every wine smoke needs: repo _shared_src, then the canonical KPM Common (KPM_COMMON_DIR overrides).
    Independent of KPM_DIR, which pre_install_gate points at a scratch tree without Patches/Common."""
    common = Path(os.environ.get("KPM_COMMON_DIR") or "~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common").expanduser()
    dirs = [REPO / "patch_manager_mods" / "_shared_src", common]
    return [f"-I{d}" for d in dirs if d.is_dir()]


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
        st, why, out = build_item(item)
        if st != "OK":
            return done("FAIL" if st == "FAIL" else "ERROR", f"build failed ({why})", out)
        log = out
    if item.kind == "suite":  # parity suites compile internally: hold a machine-wide slot for the whole run
        with _load_limit.heavy_slot():
            st, why, out = run_cmd(item.cmd, item.cwd, item.env, item.timeout)
    else:
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


def find_mod_dir(p: Path) -> str:
    """Find the mod directory relative to patch_manager_mods/ for a file."""
    curr = p if p.is_dir() else p.parent
    while curr != REPO and curr != CPP_ROOT and curr.parent != curr:
        if (curr / "manifest.toml").is_file():
            try:
                return str(curr.resolve().relative_to(CPP_ROOT.resolve())).replace("\\", "/")
            except ValueError:
                return curr.name
        curr = curr.parent
    if p.parent != CPP_ROOT and p.parent != REPO:
        try:
            return str(p.parent.resolve().relative_to(CPP_ROOT.resolve())).replace("\\", "/")
        except ValueError:
            return p.parent.name
    return ""


def cpp_item(src: Path, sanitize=True, extra_flags=(), tag="", include_dirs=None, cfg=None, exe_dir=None,
             covers=None, consumer=None) -> Item:
    stem = src.stem
    d = src.parent
    if cfg is None:
        raw = load_toml(d / "test.toml")
        sub = raw.get(stem) if isinstance(raw.get(stem), dict) else {}
        cfg = {k: v for k, v in raw.items() if not isinstance(v, dict)}
        cfg.update(sub)
    flags =["-std=c++17", "-O1", "-Wall", "-Wextra", "-I", str(d), "-I", str(REPO / "tests" / "include")]
    for inc in include_dirs or []:
        flags += ["-I", str(inc)]
    if sanitize:
        flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    flags += list(extra_flags)
    post = [expand(f) for f in cfg.get("cxxflags", [])]  # after the source so -l libs link; -D/-I work anywhere
    # Content-addressed exe name: same-stem tests (parent dir in the name) never share one, and any change to the
    # source path, flags or compiler selects a different file, so a running old exe is never overwritten.
    key = cache_key(src, ["g++"] + flags + [str(src)] + post, d)
    exe = (exe_dir or TMP / "host") / f"{d.name}__{stem}{tag}-{key[:12]}"
    # --coverage embeds the object path (-> .gcda name) in the binary: it must be compiled straight to its final name.
    cacheable = "--coverage" not in flags and "--coverage" not in post

    if covers is None:
        cov = cfg.get("covers", [])
        if isinstance(cov, str):
            covers = [cov]
        elif isinstance(cov, (list, tuple)):
            covers = [str(x) for x in cov]
        else:
            covers = []
    else:
        covers = [str(x) for x in covers]

    if consumer is None:
        if src.name.endswith("_contract_test.cpp"):
            consumer = find_mod_dir(src)
        else:
            consumer = ""
    consumer = str(cfg.get("consumer", consumer) or "")

    return Item(
        id=f"cpp:{stem}{tag}", kind="cpp", skip=expand(cfg.get("skip_reason", "")), cwd=d,
        build_cmd=["g++"] + flags + [str(src), "-o", str(exe)] + post, build_cwd=d,
        cmd=[str(exe)] + [expand(a) for a in cfg.get("args", [])], timeout=120,
        env={"KT_VERBOSE": "", "UBSAN_OPTIONS": "print_stacktrace=1",
             "ASAN_OPTIONS": "abort_on_error=0"},
        pre=lambda: _prepare_build(exe), src=src,
        covers=covers, consumer=consumer, cache=CacheInfo(exe, key) if cacheable else None,
    )


# ---------------------------------------------------------------- build cache (cpp items)
#
# The exe is content-addressed: <dir>__<stem><tag>-<key12>. key = sha256(abs source path, compiler argv without -o,
# build cwd, g++ version). Next to it, <exe>.deps.json records the sha256 of every file the compiler read (-MMD), so
# a build is skipped only when the key matches AND every dependency still has the same content. Content hashes, not
# mtimes: mutate_header.py rewrites headers in scratch trees and must always see a rebuild.

@dataclass
class CacheInfo:
    exe: Path
    key: str


@functools.lru_cache(maxsize=None)
def compiler_version() -> str:
    try:
        p = subprocess.run(["g++", "-dumpfullversion"], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                           text=True, timeout=30)
        return p.stdout.strip() if p.returncode == 0 else ""
    except (OSError, subprocess.SubprocessError):
        return ""


def cache_key(src: Path, argv, cwd: Path) -> str:
    """sha256 of (abs source path, full compiler argv minus -o, build cwd, compiler version)."""
    argv = [str(a) for a in argv]
    if "-o" in argv:
        i = argv.index("-o")
        del argv[i:i + 2]
    blob = json.dumps([str(Path(src).resolve()), argv, str(cwd), compiler_version()])
    return hashlib.sha256(blob.encode("utf-8")).hexdigest()


def parse_depfile(text: str) -> list[str]:
    """Dependencies of a make-style `target: dep dep \\<newline> dep` rule: backslash-newline continuations, `\\ `
    escaped spaces, `$$` and `\\#` handled. The target (token ending in ':') is dropped; order kept, no duplicates."""
    toks, cur, i, n = [], [], 0, len(text)
    while i < n:
        ch = text[i]
        nxt = text[i + 1:i + 2]
        if ch == "\\" and nxt in ("\n", "\r"):      # line continuation == whitespace
            i += 3 if (nxt == "\r" and text[i + 2:i + 3] == "\n") else 2
            if cur:
                toks.append("".join(cur))
                cur = []
            continue
        if ch == "\\" and nxt in (" ", "#", "\\"):
            cur.append(nxt)
            i += 2
            continue
        if ch == "$" and nxt == "$":
            cur.append("$")
            i += 2
            continue
        if ch.isspace():
            if cur:
                toks.append("".join(cur))
                cur = []
        else:
            cur.append(ch)
        i += 1
    if cur:
        toks.append("".join(cur))
    for k, t in enumerate(toks):
        if t.endswith(":"):
            toks = toks[k + 1:]
            break
    seen, out = set(), []
    for t in toks:
        if t and t not in seen:
            seen.add(t)
            out.append(t)
    return out


def _sha256_file(p: Path):
    h = hashlib.sha256()
    try:
        with open(p, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
    except OSError:
        return None
    return h.hexdigest()


def _sidecar(exe: Path) -> Path:
    exe = Path(exe)
    return exe.with_name(exe.name + ".deps.json")


def cache_valid(exe: Path, key: str) -> bool:
    """True when the exe and its sidecar exist, the key matches and every recorded dependency still hashes the same."""
    try:
        if not Path(exe).is_file():
            return False
        data = json.loads(_sidecar(exe).read_text(encoding="utf-8"))
        deps = data.get("deps")
        if data.get("key") != key or not isinstance(deps, dict) or not deps:
            return False
        return all(_sha256_file(Path(p)) == h for p, h in deps.items())
    except (OSError, ValueError, AttributeError):
        return False


def _write_sidecar(exe: Path, key: str, dfile: Path, cwd: Path, t_start_ns: int, tag: str) -> bool:
    """Record the content hashes of the compiler's dependencies. False (no sidecar) if anything looks unsafe."""
    deps = {}
    for d in parse_depfile(dfile.read_text(encoding="utf-8", errors="replace")):
        p = Path(os.path.normpath(os.path.join(cwd, d)))
        h = _sha256_file(p)
        if h is None or p.stat().st_mtime_ns > t_start_ns:   # changed while compiling: do not vouch for this exe
            return False
        deps[str(p)] = h
    if not deps:
        return False
    side = _sidecar(exe)
    tmp = side.with_name(side.name + f".tmp.{tag}")
    tmp.write_text(json.dumps({"key": key, "deps": deps}, indent=1), encoding="utf-8")
    os.replace(tmp, side)
    return True


def _compile_cached(item: Item):
    c = item.cache
    exe, cwd = Path(c.exe), item.build_cwd or item.cwd
    tag = f"{os.getpid()}.{threading.get_ident()}"
    tmp = exe.with_name(exe.name + f".tmp.{tag}")
    dfile = tmp.with_name(tmp.name + ".d")
    cmd = list(item.build_cmd)
    cmd[cmd.index("-o") + 1] = str(tmp)
    cmd[1:1] = ["-MMD", "-MF", str(dfile)]
    t_start_ns = time.time_ns()
    try:
        st, why, out = run_cmd(cmd, cwd, item.env, item.build_timeout)
        if st != "OK":
            return st, why, out
        try:
            _sidecar(exe).unlink()          # never leave an old sidecar next to a new exe
        except FileNotFoundError:
            pass
        os.replace(tmp, exe)                 # atomic: a peer running the old exe is unaffected
        try:
            _write_sidecar(exe, c.key, dfile, cwd, t_start_ns, tag)
        except (OSError, ValueError):
            pass                             # no usable .d: the build still counts, it just is not cached
        return st, why, out
    finally:
        for p in (tmp, dfile):
            try:
                p.unlink()
            except OSError:
                pass


def build_item(item: Item):
    """The build step of an item -> (status, reason, output). cpp items with a CacheInfo skip the compiler when
    nothing changed; every real compile holds a machine-wide heavy slot."""
    c = item.cache
    if c is None:
        with _load_limit.heavy_slot():
            return run_cmd(item.build_cmd, item.build_cwd or item.cwd, item.env, item.build_timeout)

    def hit():
        if USE_CACHE and cache_valid(c.exe, c.key):
            try:
                os.utime(c.exe)              # a pruner deletes by mtime
            except OSError:
                pass
            return True
        return False
    if hit():
        return "OK", "", ""
    with _load_limit.heavy_slot():
        if hit():                            # a peer run built it while we waited for a slot
            return "OK", "", ""
        return _compile_cached(item)


def py_item(path: Path, tag="", covers=None, consumer="") -> Item:
    cov = [str(x) for x in covers] if covers else []
    return Item(id=f"py:{rel(path)}{tag}", kind="py", cwd=path.parent, cmd=[sys.executable, path.name],
                timeout=300, env={"PYTHONDONTWRITEBYTECODE": "1"}, src=path, covers=cov, consumer=str(consumer or ""))


def unittest_item(d: Path, covers=None, consumer="") -> Item:
    skip = "" if VENV_PY.exists() else f"missing {rel(VENV_PY)}"
    cov = [str(x) for x in covers] if covers else []
    return Item(id=f"py:{rel(d)}(unittest)", kind="py", skip=skip, cwd=REPO,
                cmd=[str(VENV_PY), "-m", "unittest", "discover", "-s", str(d)], timeout=600,
                env={"PYTHONDONTWRITEBYTECODE": "1"}, src=d, covers=cov, consumer=str(consumer or ""))


def discover(sanitize=True, tiers=()):
    suites_cfg = load_toml(SUITES_TOML)
    excludes = {}
    for e in suites_cfg.get("discovery", {}).get("exclude", []):
        excludes[(REPO / e["path"]).resolve()] = e.get("reason", "excluded")
    items = []

    # C++ host tests under patch_manager_mods/ and tests/ (parity tests live in research/ and are run by parity.py;
    # tests/selftest holds the runner's own fixtures).
    seen = {}
    for dp, fns in (x for root in CPP_ROOTS for x in walk(root)):
        if "selftest" in dp.relative_to(REPO).parts:
            continue
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
        if s.get("tier") and s["tier"] not in tiers:
            continue
        cmd = list(s["cmd"])
        skip = missing_requirement(s.get("requires"))
        if cmd[0] == "python":
            if s.get("python") == "venv":
                cmd[0] = str(VENV_PY)
                skip = skip or ("" if VENV_PY.exists() else f"missing {rel(VENV_PY)}")
            else:
                cmd[0] = sys.executable
        cmd = [expand(c) if i else c for i, c in enumerate(cmd)]
        cov = s.get("covers", [])
        suite_covers = [str(x) for x in cov] if isinstance(cov, list) else ([str(cov)] if isinstance(cov, str) else [])
        suite_src = (REPO / s["src"]) if "src" in s else None
        items.append(Item(id=f"suite:{s['name']}", kind="suite", skip=skip,
                          cwd=suite_cwd(s.get("cwd")), cmd=cmd,
                          timeout=int(s.get("timeout_s", 300)), env={"PYTHONDONTWRITEBYTECODE": "1"},
                          src=suite_src, covers=suite_covers, consumer=str(s.get("consumer", ""))))
    return items


# ---------------------------------------------------------------- coverage (--coverage)


def cov_roots(base: Path = CPP_ROOT) -> list:
    """Coverage header roots: every dir under base holding a test.toml (testdata fixtures excluded)."""
    return [dp.resolve() for dp, fns in walk(base) if "test.toml" in fns and "testdata" not in dp.relative_to(base).parts]


# -fkeep-*: unused inline/static functions would otherwise be dropped by the compiler and read as "no code" instead of "not hit".
COV_FLAGS = ["--coverage", "-O0", "-fkeep-inline-functions", "-fkeep-static-functions"]


def parse_gcov(text: str):
    """Parse one gcov text report -> (source path as written, {line: count}) for executable lines only.
    A count line is `  <count|-|#####|=====>:  <lineno>:<source>`; '-' = not executable, #####/===== = executable, 0 hits."""
    source, lines = "", {}
    for raw in text.splitlines():
        parts = raw.split(":", 2)
        if len(parts) < 3:
            continue
        cnt, no = parts[0].strip(), parts[1].strip()
        if not no.isdigit():
            continue
        if int(no) == 0:
            if parts[2].startswith("Source:"):
                source = parts[2][len("Source:"):].strip()
            continue
        if cnt == "-":
            continue
        cnt = cnt.rstrip("*")
        if cnt.startswith("#") or cnt.startswith("="):
            lines[int(no)] = 0
        elif cnt.isdigit():
            lines[int(no)] = int(cnt)
    return source, lines


def split_gcov(text: str) -> list:
    """Split `gcov -t` output (several files back to back) into one text per `0:Source:` header."""
    blocks, cur = [], []
    for ln in text.splitlines():
        if re.match(r"\s*-:\s*0:Source:", ln) and cur:
            blocks.append("\n".join(cur))
            cur = []
        cur.append(ln)
    if cur:
        blocks.append("\n".join(cur))
    return blocks


def merge_cov(acc: dict, source: str, lines: dict):
    """Fold one test's {line: count} into acc[source]; counts add, so a line is hit if any test hit it."""
    d = acc.setdefault(source, {})
    for ln, c in lines.items():
        d[ln] = d.get(ln, 0) + c


def cov_counts(lines: dict):
    return sum(1 for c in lines.values() if c > 0), len(lines)


def cov_line(name: str, hit: int, total: int) -> str:
    pct = "n/a" if total == 0 else f"{100.0 * hit / total:.1f}%"
    return f"COV  {pct}  {hit}/{total}  {name}"


def cov_summary(acc: dict, want, label=lambda p: p):
    """COV lines for every source in acc that `want(path)` accepts, sorted by path."""
    return [cov_line(label(p), *cov_counts(acc[p])) for p in sorted(acc) if want(p)]


@dataclass
class CovRun:
    acc: dict
    lines: list
    failed: list
    results: list


def _is_cov_header(p: Path, roots=None) -> bool:
    """A .h sitting directly in a coverage root."""
    return p.suffix == ".h" and p.parent.resolve() in (cov_roots() if roots is None else roots)


def run_coverage(srcs, jobs=4, want=None, verbose=False, items=None) -> CovRun:
    """Build srcs (or the prepared `items`) with --coverage, run them, gcov the objects, merge per header.
    Output files live under $TMPDIR/kotor-tests/coverage/."""
    if want is None:
        roots = cov_roots()
        want = lambda p: _is_cov_header(p, roots)  # noqa: E731
    cdir = TMP / "coverage"
    shutil.rmtree(cdir, ignore_errors=True)
    bin_dir = cdir / "bin"
    bin_dir.mkdir(parents=True)
    if items is None:
        items = [cpp_item(Path(s), False, extra_flags=COV_FLAGS, exe_dir=bin_dir) for s in srcs]
    if not shutil.which("gcov"):
        raise SystemExit("gcov not found")
    with cf.ThreadPoolExecutor(max(1, jobs)) as ex:
        results = list(ex.map(execute, items))
    acc, failed = {}, []
    for it, r in zip(items, results):
        report(r, verbose)
        if r.status in ("FAIL", "ERROR"):
            failed.append(r.id)
        stem = Path(it.cmd[0]).name
        gcdas = sorted(bin_dir.glob(f"{stem}-{it.src.stem}.gcda"))  # gcc names it <exe>-<src> when the two differ
        for g in gcdas:
            # gcov runs in the test's build cwd (sources are recorded relative to it) with -t: report on stdout, no files.
            st, why, out = run_cmd(["gcov", "-t", "-o", str(bin_dir), str(g)], it.cwd, None, 120)
            (cdir / f"{stem}.gcov.txt").write_text(out, encoding="utf-8")
            for block in split_gcov(out):
                source, lines = parse_gcov(block)
                if not source:
                    continue
                path = Path(source)
                path = (path if path.is_absolute() else it.cwd / path).resolve()
                if want(path):
                    merge_cov(acc, str(path), lines)
    out_lines = cov_summary(acc, lambda p: True, lambda p: rel(Path(p)))
    (cdir / "summary.txt").write_text("\n".join(out_lines) + "\n", encoding="utf-8")
    return CovRun(acc, out_lines, failed, results)


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


def wine_plan(cfg_path: Path, dll_from_override: str | None = None):
    """Return (smokes, error). Each smoke dict gets `_dlls` (runtime DLL paths) and `_dll_missing`."""
    if not cfg_path.is_file():
        return [], f"smokes.toml missing ({rel(cfg_path)})"
    cfg = load_toml(cfg_path)
    default_from = dll_from_override or cfg.get("dll_from", "kpatch")
    dlls, missing = find_runtime_dlls(cfg)
    return [{**s,
             "dll_from": dll_from_override or s.get("dll_from") or default_from,
             "_dlls": dlls,
             "_dll_missing": missing} for s in cfg.get("smoke", [])], ""


def smoke_skip(s) -> str:
    if not shutil.which("i686-w64-mingw32-g++"):
        return "missing i686-w64-mingw32-g++"
    if not wine_binary().exists():
        return f"missing {wine_binary()}"
    dll_from = s.get("dll_from", "kpatch")
    if dll_from != "build":
        kp = KPM_DIR / "Patches" / s["kpatch"]
        if not kp.exists():
            return f"missing {kp}"
    for x in s.get("extra_dlls", []):
        x_from = x.get("dll_from", dll_from)
        if x_from != "build":
            if not (KPM_DIR / "Patches" / x["kpatch"]).exists():
                return f"missing {KPM_DIR / 'Patches' / x['kpatch']}"
    if s.get("runtime_dlls") and s["_dll_missing"]:
        return f"missing {s['_dll_missing']} (mingw_runtime_dirs)"
    if s.get("needs_display") and not os.environ.get("DISPLAY"):
        return "missing DISPLAY"
    return ""


_build_locks: dict[str, threading.Lock] = {}
_build_locks_guard = threading.Lock()


def _get_build_lock(key: str) -> threading.Lock:
    with _build_locks_guard:
        if key not in _build_locks:
            _build_locks[key] = threading.Lock()
        return _build_locks[key]


def resolve_mod_for_smoke(kpatch: Path | str, mod: str | None = None) -> Path | None:
    if mod:
        p = Path(mod)
        return p if p.is_absolute() else REPO / "patch_manager_mods" / p
    kpatch_str = str(kpatch)
    stem = Path(kpatch_str).stem.lower()
    parent = Path(kpatch_str).parent.name.lower()
    try:
        import _static_common as sc
        mods = sc.load_mods(REPO, KPM_DIR, variants=True)
        for m in mods:
            p_name = (m.kpm_dir.name if m.kpm_dir else m.dir.name).lower()
            if stem == p_name or stem == m.dir.name.lower() or stem == m.id.replace("-", "").lower():
                return m.dir
            if parent and (parent == p_name or parent == m.dir.name.lower()):
                return m.dir
    except Exception as e:
        print(f"warning: _static_common lookup failed ({e}); falling back to directory name match", file=sys.stderr)
    for d in (REPO / "patch_manager_mods").iterdir():
        if d.is_dir():
            clean = d.name.replace("_src", "").replace("-", "").lower()
            if clean and clean == stem:
                return d
    return None


def wine_root() -> Path:
    """Where smoke build dirs live: the per-run dir during a --wine run, else the shared TMP/wine."""
    return WINE_RUN_DIR or (TMP / "wine")


def _ensure_built_dll(mod_dir: Path) -> Path:
    """Ensure mod is built via build_check.py --keep and return path to its built DLL.
    TMP/build is shared with other runs, so check+build holds a machine-wide lock per build key."""
    try:
        m_rel = mod_dir.relative_to(REPO / "patch_manager_mods")
    except ValueError:
        m_rel = mod_dir
    key = "__".join(m_rel.parts)
    with _get_build_lock(key), _load_limit.exclusive(f"build-{key}", f"build {key}"):
        build_root = TMP / "build"
        dll_candidates = list((build_root / key).glob("*/build/*.dll"))

        rebuild_needed = not dll_candidates
        if dll_candidates:
            dll_mtime = max(p.stat().st_mtime for p in dll_candidates)
            src_dirs = [mod_dir, REPO / "patch_manager_mods" / "_shared_src", KPM_DIR, REPO / "scripts" / "build_check.py"]
            for sdir in src_dirs:
                scan = [sdir] if sdir.is_file() else sdir.rglob("*") if sdir.is_dir() else []
                for p in scan:
                    if p.is_file():
                        if p.stat().st_mtime > dll_mtime:
                            rebuild_needed = True
                            break
                if rebuild_needed:
                    break

        if rebuild_needed:
            cmd = [sys.executable, str(REPO / "scripts/build_check.py"), "--mod", str(m_rel), "--keep"]
            proc = subprocess.run(cmd, cwd=REPO, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            if proc.returncode != 0:
                raise RuntimeError(f"build_check failed for {m_rel}:\n{proc.stdout}")
            dll_candidates = list((build_root / key).glob("*/build/*.dll"))
            if not dll_candidates:
                raise RuntimeError(f"build_check --keep for {m_rel} produced no DLL in {build_root / key}")

        return max(dll_candidates, key=lambda p: p.stat().st_mtime)


def _extract_dll(kpatch: Path, dest: Path, dll_from: str = "kpatch", mod: str | None = None):
    if dll_from == "build":
        mod_dir = resolve_mod_for_smoke(kpatch, mod)
        if not mod_dir:
            raise RuntimeError(f"could not resolve mod directory for kpatch {kpatch.name}")
        dll_path = _ensure_built_dll(mod_dir)
        shutil.copy2(dll_path, dest)
        return

    with zipfile.ZipFile(kpatch) as z:
        dest.write_bytes(z.read("binaries/windows_x86.dll"))


def smoke_build(s) -> Result:
    """Compile the smoke, extract the patch DLL(s), copy runtime DLLs. Returns Result (PASS = built)."""
    t0 = time.monotonic()
    name = s["name"]
    bdir = wine_root() / name
    rid = f"wine:{name}"

    def res(status, reason="", out=""):
        return Result(rid, "wine", status, time.monotonic() - t0, reason, out)
    try:
        if bdir.exists():
            shutil.rmtree(bdir)
        bdir.mkdir(parents=True)
        cmd = (["i686-w64-mingw32-g++", "-std=c++17", "-O1", "-I", str(HARNESS), "-I", str(REPO / "tests" / "include")]
               + canonical_include_flags()
               + [str(HARNESS / s["src"]), "-o", str(bdir / f"{name}.exe")] + [expand(f) for f in s.get("cxxflags", [])])
        with _load_limit.heavy_slot():
            st, why, out = run_cmd(cmd, HARNESS, None, 600)
        if st != "OK":
            return res("FAIL" if st == "FAIL" else "ERROR", f"build failed ({why})", out)
        dll_from = s.get("dll_from", "kpatch")
        _extract_dll(KPM_DIR / "Patches" / s["kpatch"], bdir / s["dll_name"],
                     dll_from=dll_from, mod=s.get("mod"))
        for x in s.get("extra_dlls", []):
            x_from = x.get("dll_from", dll_from)
            _extract_dll(KPM_DIR / "Patches" / x["kpatch"], bdir / x["dll_name"],
                         dll_from=x_from, mod=x.get("mod"))
        if s.get("runtime_dlls"):
            for p in s["_dlls"].values():
                shutil.copy2(p, bdir / p.name)
    except (OSError, KeyError, TypeError, zipfile.BadZipFile, RuntimeError) as e:
        return res("FAIL" if isinstance(e, RuntimeError) else "ERROR", f"{type(e).__name__}: {e}")
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
    st, why, out = run_cmd(cmd, wine_root() / name, wine_env(), int(s.get("timeout_s", 120)))
    if st == "ERROR" and why.startswith("timeout"):
        wine_kill_server()
    status = "PASS" if st == "OK" else st
    return Result(rid, "wine", status, time.monotonic() - t0, "" if st == "OK" else why, out)


def wine_stage(wine_items, runnable_smokes, builds, wine_err, emit):
    """Run the wine smokes already built. Everything that touches the shared prefix or the wineserver (prefix init,
    every run, the final `wineserver -k`) holds the machine-wide "wineprefix" lock, so a peer run's wine is never killed."""
    if wine_err:
        emit(Result("wine", "wine", "ERROR", 0.0, wine_err))
    needs_wine = any(not skip for _, _, _, skip in wine_items) and any(b.status == "PASS" for b in builds.values())
    lock = _load_limit.exclusive("wineprefix", "Wine prefix") if needs_wine else contextlib.nullcontext()
    with lock:
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
        if needs_wine:
            wine_kill_server()


def finish_wine_run_dir(results):
    """Remove this run's smoke build dir; keep it (and say where) when any wine item FAILed or ERRORed."""
    d = WINE_RUN_DIR
    if d is None or not d.exists():
        return
    if any(r.kind == "wine" and r.status in ("FAIL", "ERROR") for r in results):
        print(f"NOTE  wine build dir kept for inspection: {d}", flush=True)
    else:
        shutil.rmtree(d, ignore_errors=True)


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
    ap.add_argument("--id", action="append", default=[], help="only run item with exact id (repeatable)")
    ap.add_argument("--ids-file", metavar="PATH", help="file containing item ids to run, one per line")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4, help="parallel builds/host runs (default: cpu count)")
    ap.add_argument("--wine", action="store_true", help="also run the Wine smokes from harness/smokes.toml")
    ap.add_argument("--dll-from", choices=["kpatch", "build"], default=None,
                    help="DLL source for Wine smokes: 'kpatch' (extract from .kpatch) or 'build' (use build_check --keep). Overrides smokes.toml.")
    ap.add_argument("--build", action="store_true", help="also run tier=\"build\" suites (Windows source builds; implied by --wine)")
    ap.add_argument("--json", nargs="?", const="-", default=None, metavar="PATH",
                    help="write machine-readable results (to PATH, or stdout if omitted / '-')")
    ap.add_argument("-v", action="store_true", help="print the full output of every item (as each finishes)")
    ap.add_argument("--no-sanitize", action="store_true", help="drop -fsanitize=address,undefined from C++ builds")
    ap.add_argument("--selftest", action="store_true", help="check that failing tests are reported as FAIL")
    ap.add_argument("--coverage", action="store_true",
                    help="build host C++ tests with --coverage -O0, run, gcov; print per-header COV lines (every dir with a test.toml)")
    ap.add_argument("--mutate", metavar="HEADER", help="run the mutation gate on one header (scripts/mutate_header.py)")
    ap.add_argument("filters", nargs="*", default=[], help="optional positional filter strings or item IDs (e.g. suite:check-hook-bytes, diversity)")
    ap.add_argument("--no-cache", action="store_true", help="recompile every C++ test even if nothing changed (the cache entry is refreshed)")
    a = ap.parse_args()
    global USE_CACHE, WINE_RUN_DIR
    _load_limit.lower_priority()
    USE_CACHE = not a.no_cache

    if a.selftest:
        return selftest(a.jobs, a.v)

    if a.mutate:
        cmd = [sys.executable, str(Path(__file__).resolve().parent / "mutate_header.py"), a.mutate, "--jobs", str(a.jobs)]
        if a.list:
            cmd.append("--list")
        if a.json:
            cmd += ["--json", a.json]
        if a.no_sanitize:
            cmd.append("--no-sanitize")
        if a.v:
            cmd.append("-v")
        return subprocess.call(cmd)

    target_ids = set(a.id)
    if a.ids_file:
        p = Path(a.ids_file)
        if not p.is_file():
            sys.stderr.write(f"error: --ids-file not found: {a.ids_file}\n")
            return 2
        for line in p.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if line and not line.startswith("#"):
                target_ids.add(line)

    if a.coverage:
        cpp = [i for i in discover(sanitize=False) if i.kind == "cpp" and (not a.k or a.k in i.id) and (not target_ids or i.id in target_ids)]
        if a.list:
            for i in cpp:
                print(f"{i.id}  [cpp]" + (f"  SKIP ({i.skip})" if i.skip else ""))
            return 0
        t0 = time.monotonic()
        TMP.mkdir(parents=True, exist_ok=True)
        bin_dir = TMP / "coverage" / "bin"
        run_items = []
        for i in cpp:
            ci = cpp_item(i.src, False, extra_flags=COV_FLAGS, exe_dir=bin_dir)
            ci.id, ci.skip = i.id, i.skip
            run_items.append(ci)
        cr = run_coverage(None, a.jobs, items=run_items, verbose=a.v)
        for line in cr.lines:
            print(line)
        print(f"\ncoverage: {len(cr.lines)} header(s), {len(run_items)} test(s), output in {TMP / 'coverage'} in {time.monotonic() - t0:.1f}s")
        return 1 if cr.failed else 0

    t_start = time.monotonic()
    requested = set(target_ids) | set(a.filters)
    include_build_tier = a.build or a.wine or any(x == "suite:build-check" for x in requested)
    items = discover(sanitize=not a.no_sanitize, tiers=("build",) if include_build_tier else ())
    smokes, wine_err = ([], "")
    include_wine = a.wine or any(x.startswith("wine:") for x in requested)
    if include_wine:
        smokes, wine_err = wine_plan(SMOKES_TOML, dll_from_override=a.dll_from)
    if a.k:
        items = [i for i in items if a.k in i.id]
    pos_subs = []
    if a.filters:
        pos_exact = {f for f in a.filters if any(f == i.id for i in items) or f.startswith("wine:")}
        pos_subs = [f for f in a.filters if f not in pos_exact]
        if pos_exact:
            target_ids.update(pos_exact)
    if target_ids or pos_subs:
        # exact ids and substring filters are a union (mixing them used to intersect to nothing)
        items = [i for i in items if i.id in target_ids or any(sub in i.id for sub in pos_subs)]

    # Wine items: (smoke, mode) -> id
    wine_items = []  # (id, smoke, mode, skip)
    for s in smokes:
        skip = smoke_skip(s)
        for mode in (s.get("modes") or [""]):
            wid = f"wine:{s['name']}" + (f"[{mode}]" if mode else "")
            match_target = (not target_ids or wid in target_ids or any(wid == t or wid.startswith(f"{t}[") for t in target_ids)
                            or any(sub in wid for sub in pos_subs))
            match_k = not a.k or a.k in wid
            match_filters = not a.filters or not pos_subs or any(sub in wid for sub in pos_subs) or match_target
            if match_k and match_filters and match_target:
                wine_items.append((wid, s, mode, skip))

    if a.list:
        if a.json:
            table = []
            for i in items:
                table.append({
                    "id": i.id,
                    "kind": i.kind,
                    "src": rel(i.src) if i.src else None,
                    "covers": list(i.covers),
                    "consumer": i.consumer,
                    "skip": i.skip,
                })
            if include_wine:
                if wine_err:
                    table.append({
                        "id": "wine",
                        "kind": "wine",
                        "src": None,
                        "covers": [],
                        "consumer": "",
                        "skip": f"ERROR ({wine_err})",
                    })
                for wid, s, mode, skip in wine_items:
                    table.append({
                        "id": wid,
                        "kind": "wine",
                        "src": rel(HARNESS / s["src"]) if "src" in s else None,
                        "covers": [],
                        "consumer": "",
                        "skip": skip,
                    })
            out_str = json.dumps(table, indent=2)
            if a.json == "-":
                print(out_str)
            else:
                Path(a.json).write_text(out_str, encoding="utf-8")
            return 0

        for i in items:
            print(f"{i.id}  [{i.kind}]" + (f"  SKIP ({i.skip})" if i.skip else ""))
        if include_wine:
            if wine_err:
                print(f"wine  [wine]  ERROR ({wine_err})")
            for wid, s, mode, skip in wine_items:
                print(f"{wid}  [wine]" + (f"  SKIP ({skip})" if skip else ""))
        return 0

    # Every requested id / filter must match something; --json output is not written on this exit-2 path.
    matched_ids = {i.id for i in items} | {w for w, *_ in wine_items}
    missing = sorted(t for t in target_ids
                     if t not in matched_ids and not (t.startswith("wine:") and wine_err)
                     and not any(w.startswith(f"{t}[") for w in matched_ids))
    missing += sorted(s for s in pos_subs if not any(s in m for m in matched_ids)) if a.filters else []
    if missing:
        sys.stderr.write("error: no test item matched: " + ", ".join(missing) + "\n")
        return 2

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
    if include_wine and runnable_smokes:   # smoke build dirs are per run: a peer run never sees (or deletes) ours
        WINE_RUN_DIR = TMP / "wine" / f"run-{os.getpid()}-{int(time.time())}"
    # Items that rewrite live tree files (test_compat_check swaps K2Addr.h for a scratch diff) run alone, after the
    # pool: in parallel they made check-shared-headers / check-raw-addresses / k2addr tests see the edit (10-18 gate FAILs).
    serial_last = [i for i in items if i.id in TREE_MUTATING_IDS]
    items = [i for i in items if i.id not in TREE_MUTATING_IDS]
    with cf.ThreadPoolExecutor(max(1, a.jobs)) as ex:
        futs = {ex.submit(execute, i): i for i in items}
        bfuts = {ex.submit(smoke_build, s): n for n, s in runnable_smokes.items()}
        for f in cf.as_completed(set(futs) | set(bfuts)):
            if f in futs:
                emit(f.result())
            else:
                builds[bfuts[f]] = f.result()
    for i in serial_last:
        emit(execute(i))

    if include_wine:
        wine_stage(wine_items, runnable_smokes, builds, wine_err, emit)
        finish_wine_run_dir(results)

    wall = time.monotonic() - t_start
    counts = {k: sum(1 for r in results if r.status == k) for k in ("PASS", "FAIL", "SKIP", "ERROR")}
    print(f"\n{counts['PASS']} passed, {counts['FAIL']} failed, {counts['ERROR']} error, {counts['SKIP']} skipped "
          f"({len(results)} items) in {wall:.1f}s")
    if a.json:
        out_str = json.dumps(
            {"wall_seconds": round(wall, 2), "counts": counts,
             "results": [{"id": r.id, "kind": r.kind, "status": r.status, "seconds": round(r.seconds, 2),
                          "reason": r.reason, "tail": tail(r.output)} for r in sorted(results, key=lambda r: r.id)]},
            indent=2)
        if a.json == "-":
            print(out_str)
        else:
            Path(a.json).write_text(out_str, encoding="utf-8")
    return 1 if counts["FAIL"] or counts["ERROR"] else 0


if __name__ == "__main__":
    sys.exit(main())
