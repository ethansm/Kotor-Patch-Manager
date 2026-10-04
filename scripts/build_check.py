#!/usr/bin/env python3
"""Compile mod sources for Windows exactly the way KPM does, and check the DLL exports (doc 20, build gate).

  python3 scripts/build_check.py                                  # every mod under patch_manager_mods/*_src
  python3 scripts/build_check.py --mod buff_duration_hud_src --mod perf_lab_src/perflab
  python3 scripts/build_check.py --mod level_up_override_src --keep   # keep $TMPDIR/kotor-tests/build/<key>/
  python3 scripts/build_check.py --kpm-dir /tmp/fake/kpm --repo /tmp/fake/repo --build-root /tmp/fake/build

Changed mod .cpp files are otherwise first compiled for Windows at the next real KPM build. This stages each mod
under $TMPDIR/kotor-tests/build/<mod-key>/<Name>/ (+ a sibling _shared/ copied from patch_manager_mods/_shared_src,
so "../_shared/X.h" resolves), builds windows_x86.dll with the compiler, flags and link line of KPM's
Patches/create-patch.py, and checks that the DLL's PE export table contains every detour `function` of the mod's
hooks toml. Nothing is written under the KPM tree (read-only: Patches/Common and lib/ are used in place).

KPM correspondence (Patches/create-patch.py in the kotor-patch-manager checkout):
  :365-383   WINDOWS_TOOLCHAINS: the MinGW toolchain (CXX_WIN, i686-w64-mingw32-g++-posix, i686-w64-mingw32-g++) and
             its flags -std=c++17 -shared -O2 -s -static -static-libgcc -static-libstdc++ -DWIN32 -DNDEBUG
             -D_WINDOWS -D_USRDLL (MSVC, listed first there, is not available on this host)
  :439-444   find_sources: root *.cpp + */*.cpp of the patch dir
  :166-198   UnixDriver.build_library: Common/*.cpp + Common/GameAPI/*.cpp compiled one object each with the flags
             minus -shared/-s, `-I Common -I lib`, archived with ar into libcommon.a (cached; ours is keyed by
             content under $TMPDIR/kotor-tests/common-cache/, never in the KPM tree)
  :200-235   UnixDriver.build_module: dlltool -d lib/sqlite3.def -D sqlite3.dll -l libsqlite3.a, then ONE
             `g++ <flags> -I Common -I lib <sources> exports.def -o windows_x86.dll libcommon.a -L<tmp> -lsqlite3
             -lkernel32 <#pragma comment(lib) names as -l...>`
  :660-672   pragma_libraries; :641-657 exports.def (the committed one is used as is)
  :725-751   compile_module: the Windows module links Common/GameAPI and sqlite
KPM's find_sources leaves *_test.cpp / t32_*.cpp out since fork commit f743ac4 (ethansm/local-mods; doc 20d F13;
upstream create-patch.py still compiles them). The staging here excludes them too (any suffix), so a build matches
both, since the patch_manager_mods dirs hold host unit tests next to the DLL sources.

Findings (scripts/_static_common.Reporter conventions):
  FAIL  compile/link error (compiler stderr is printed), no DLL produced, DLL unreadable, or a detour `function`
        of the hooks toml is not in the DLL export table (KPM would log "Function not found" and skip the hook)
  WARN  a generated .inc missing from the repo source dir was borrowed from KPM Patches/<Name>/ (repo not
        self-contained); an exports.def name absent from the DLL (cannot normally happen: the .def is linked in)
  SKIP  no MinGW compiler, or no KPM Patches/Common + lib/ to link against (exit 0, like the other checkers; the
        run_tests.py suite entry should also declare `requires = ["which:i686-w64-mingw32-g++"]`)
  PASS  built and exported, with the wall time
Exit 1 on any FAIL.
"""
import argparse
import concurrent.futures
import contextlib
import fcntl
import hashlib
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _static_common as sc  # noqa: E402
import check_exports as ce  # noqa: E402

# create-patch.py:365-383 (MinGW toolchain flags) and :378-379 (compiler candidates; the bare g++/clang++ fallbacks
# are not used here: they would not produce a Windows PE on this host)
CXX_ENV = "CXX_WIN"
COMPILERS = ("i686-w64-mingw32-g++-posix", "i686-w64-mingw32-g++")
FLAGS = ("-std=c++17", "-shared", "-O2", "-s", "-static", "-static-libgcc", "-static-libstdc++",
         "-DWIN32", "-DNDEBUG", "-D_WINDOWS", "-D_USRDLL")

SRC_SUFFIXES = {".cpp", ".h", ".hpp", ".inc", ".def", ".toml"}   # what a build can consume (not .tga/.md/.java/logs)
SOURCE_DEPTH = 1   # find_sources: *.cpp and */*.cpp (headers/.inc are staged at any depth)
MAX_ERR_LINES = 200


def is_test_source(name: str) -> bool:
    """create-patch.py does not skip these (yet); the host unit tests would not link into the DLL."""
    return name.endswith("_test.cpp") or name.startswith("t32_")


# ---------------------------------------------------------------- toolchain

def find_compiler() -> str | None:
    """create-patch.py compiler_candidates: $CXX_WIN, then the MinGW presets (absolute path or PATH lookup)."""
    for name in (os.environ.get(CXX_ENV), *COMPILERS):
        if name:
            found = shutil.which(name)
            if found:
                return found
    return None


def tool_env(compiler: str) -> dict:
    """UnixDriver.environment: the compiler's own directory first on PATH (a cross compiler spawns ld by name)."""
    env = dict(os.environ)
    env["PATH"] = f"{Path(compiler).resolve().parent}{os.pathsep}{env.get('PATH', '')}"
    return env


def compiler_identity(compiler: str) -> str:
    parts = []
    for flag in ("-dumpmachine", "-dumpversion"):
        p = subprocess.run([compiler, flag], capture_output=True, text=True)
        parts.append(p.stdout.strip() or "unknown")
    return "-".join(parts)


def find_tool(compiler: str, name: str) -> str | None:
    """UnixDriver.tool (the gcc branch): <triple>-<name> beside the compiler or on PATH, then the bare name."""
    triple = subprocess.run([compiler, "-dumpmachine"], capture_output=True, text=True).stdout.strip()
    bin_dir = Path(compiler).resolve().parent
    for cand in ([f"{triple}-{name}"] if triple else []) + [name]:
        if (bin_dir / cand).is_file():
            return str(bin_dir / cand)
        found = shutil.which(cand)
        if found:
            return found
    return None


def pragma_libraries(cpp_files) -> list:
    """create-patch.py:660-672: -l flags for `#pragma comment(lib, "x")` (GCC ignores the pragma itself)."""
    names = []
    for source in cpp_files:
        for m in re.finditer(r'#pragma\s+comment\s*\(\s*lib\s*,\s*"([^"]+)"',
                             Path(source).read_text(errors="replace")):
            name = re.sub(r"\.lib$", "", m.group(1), flags=re.IGNORECASE)
            if name not in names:
                names.append(name)
    return [f"-l{n}" for n in names]


# ---------------------------------------------------------------- Common library (create-patch.py:166-198, 675-722)

def common_key(common_dir: Path, lib_dir: Path, compiler: str) -> str:
    """Cache key: flags + compiler identity + every Common file's path, size and content hash."""
    h = hashlib.sha256()
    h.update(" ".join(FLAGS).encode())
    h.update(compiler_identity(compiler).encode())
    for root in (common_dir,):
        for p in sorted(root.rglob("*")):
            if p.is_file():
                h.update(str(p.relative_to(root)).encode())
                h.update(hashlib.sha256(p.read_bytes()).digest())
    for p in sorted(lib_dir.glob("*.h")):
        h.update(p.name.encode())
        h.update(hashlib.sha256(p.read_bytes()).digest())
    return h.hexdigest()[:16]


def ensure_common_library(kpm_dir: Path, compiler: str, cache_root: Path):
    """Build Common/GameAPI into libcommon.a once and reuse it. Returns (library Path, error text or '').

    Same compile as UnixDriver.build_library (flags minus -shared/-s, one object per source, `-I Common -I lib`),
    in a cache dir of our own so the KPM tree is never written. A flock serialises concurrent runs.
    """
    common_dir, lib_dir = kpm_dir / "Patches" / "Common", kpm_dir / "lib"
    flags = [f for f in FLAGS if f not in ("-shared", "-s")]
    sources = [*sorted(common_dir.glob("*.cpp")), *sorted((common_dir / "GameAPI").glob("*.cpp"))]
    cache_root.mkdir(parents=True, exist_ok=True)
    with open(cache_root / ".lock", "w") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        cache = cache_root / common_key(common_dir, lib_dir, compiler)
        library = cache / "libcommon.a"
        if library.is_file():
            return library, ""
        shutil.rmtree(cache, ignore_errors=True)
        objdir = cache / "obj"
        objdir.mkdir(parents=True)
        env = tool_env(compiler)

        def one(src: Path):
            obj = objdir / f"{src.parent.name}-{src.stem}.o"
            r = subprocess.run([compiler, *flags, "-I", str(common_dir), "-I", str(lib_dir), "-c", str(src),
                                "-o", str(obj)], capture_output=True, text=True, env=env)
            return obj, r

        objs = []
        with concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
            for obj, r in pool.map(one, sources):
                if r.returncode != 0:
                    shutil.rmtree(cache, ignore_errors=True)
                    return None, f"Common library compilation failed ({obj.name}):\n{r.stderr}"
                objs.append(obj)
        ar = find_tool(compiler, "ar")
        if not ar:
            return None, "could not find ar for " + compiler
        r = subprocess.run([ar, "rcs", str(library), *(str(o) for o in objs)], capture_output=True, text=True, env=env)
        if r.returncode != 0:
            shutil.rmtree(cache, ignore_errors=True)
            return None, "Common library archiving failed:\n" + r.stderr
        shutil.rmtree(objdir, ignore_errors=True)
        return library, ""


# ---------------------------------------------------------------- staging

def stage_mod(src: Path, name: str, key_dir: Path, shared_src: Path) -> Path:
    """key_dir/<Name>/ (mod sources) + key_dir/_shared/ (copy of _shared_src); returns the <Name> dir.

    Copies what a build consumes (.cpp/.h/.hpp/.inc/.def/.toml): .cpp at the root and one level down (find_sources),
    headers and .inc at any depth. *_test.cpp, t32_*, and the SKIP_DIRS (testdata, retired, ...) are left out.
    """
    shutil.rmtree(key_dir, ignore_errors=True)
    dest = key_dir / name
    dest.mkdir(parents=True)
    src = Path(src)
    for root, dirs, files in os.walk(src):
        dirs[:] = sorted(d for d in dirs if d not in sc.SKIP_DIRS)
        depth = len(Path(root).relative_to(src).parts)
        for f in sorted(files):
            suf = Path(f).suffix.lower()
            if suf not in SRC_SUFFIXES or is_test_source(f):
                continue
            if suf == ".cpp" and depth > SOURCE_DEPTH:
                continue
            out = dest / Path(root).relative_to(src) / f
            out.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(Path(root) / f, out)
    shared = key_dir / "_shared"
    shared.mkdir()
    if Path(shared_src).is_dir():
        for p in sorted(Path(shared_src).iterdir()):
            if p.is_file() and p.suffix.lower() in SRC_SUFFIXES and not is_test_source(p.name) and p.name != "test.toml":
                shutil.copy2(p, shared / p.name)
    return dest


def find_sources(patch_dir: Path) -> list:
    """create-patch.py:439-444: *.cpp and */*.cpp of the staged patch dir."""
    return sorted([*patch_dir.glob("*.cpp"), *patch_dir.glob("*/*.cpp")])


# ---------------------------------------------------------------- one mod

@dataclass
class ModSrc:
    dir: Path
    key: str                         # path relative to patch_manager_mods with / -> __
    name: str                        # KPM Patches/<Name> dir name (fallback: the source dir name)
    id: str = ""
    funcs: set = field(default_factory=set)   # detour `function` names from the hooks toml
    kpm_patch_dir: Path | None = None         # KPM Patches/<Name>/ (read-only; generated .inc fallback)


@dataclass
class BuildResult:
    dll: Path | None = None
    stderr: str = ""                 # compiler/linker output when it failed (also the reason text)
    seconds: float = 0.0


def compile_mod(stage_dir: Path, name: str, compiler: str, kpm_dir: Path, library: Path) -> BuildResult:
    """The create-patch.py:200-235 link line for the Windows module."""
    t0 = time.monotonic()
    common_dir, lib_dir = kpm_dir / "Patches" / "Common", kpm_dir / "lib"
    sources = find_sources(stage_dir)
    out = stage_dir / "build" / "windows_x86.dll"
    out.parent.mkdir(parents=True, exist_ok=True)
    env = tool_env(compiler)
    if not sources:
        return BuildResult(stderr=f"no .cpp sources staged for {name}", seconds=time.monotonic() - t0)
    dlltool = find_tool(compiler, "dlltool")
    if not dlltool:
        return BuildResult(stderr=f"could not find dlltool for {compiler}", seconds=time.monotonic() - t0)
    import_dir = stage_dir.parent / "implib"
    import_dir.mkdir(exist_ok=True)
    made = subprocess.run([dlltool, "-d", str(lib_dir / "sqlite3.def"), "-D", "sqlite3.dll",
                           "-l", str(import_dir / "libsqlite3.a")], capture_output=True, text=True, env=env)
    if made.returncode != 0:
        return BuildResult(stderr="failed to build the sqlite3 import lib.\n" + made.stderr,
                           seconds=time.monotonic() - t0)
    requested = pragma_libraries(sources)
    exports_def = stage_dir / "exports.def"   # create-patch.py:641: a committed exports.def is used as is
    if not exports_def.is_file():
        exports_def = generate_exports_def(stage_dir, sources, name)
    link_sources = [*sources, exports_def]
    cmd = [compiler, *FLAGS, "-I", str(common_dir), "-I", str(lib_dir), *(str(s) for s in link_sources),
           "-o", str(out), str(library), "-L", str(import_dir), "-lsqlite3", "-lkernel32", *requested]
    r = subprocess.run(cmd, capture_output=True, text=True, env=env)
    dt = time.monotonic() - t0
    if r.returncode != 0:
        return BuildResult(stderr=r.stderr or r.stdout, seconds=dt)
    return BuildResult(dll=out, stderr=r.stderr, seconds=dt)


def generate_exports_def(patch_dir: Path, cpp_files, name: str) -> Path:
    """create-patch.py:641-657 (mods that commit no exports.def)."""
    out = patch_dir / "exports.def"
    names = []
    for cpp in cpp_files:
        for line in cpp.read_text(errors="ignore").splitlines():
            if re.search(r"extern.*__cdecl", line):
                parts = [p for p in re.split(r"[ (]+", line.strip()) if p]
                if len(parts) >= 5:
                    names.append(parts[4])
    out.write_text("\n".join(["LIBRARY " + name, "EXPORTS", *("    " + n for n in names)]) + "\n")
    return out


def check_mod(rep, m: ModSrc, compiler: str, kpm_dir: Path, library: Path, shared_src: Path,
              build_root: Path, keep: bool, err_stream=sys.stdout) -> bool:
    """Stage, build and export-check one mod. Returns True if no FAIL."""
    key_dir = build_root / m.key
    t0 = time.monotonic()
    try:
        stage = stage_mod(m.dir, m.name, key_dir, shared_src)
        if m.kpm_patch_dir is not None and Path(m.kpm_patch_dir).is_dir():
            # Generated tables (scripts/gen_*_table.py) that exist only in the KPM copy of the patch: supply them
            # so the rest of the mod still gets compiled, but say so (the repo source is not self-contained).
            borrowed = []
            for p in sorted(Path(m.kpm_patch_dir).glob("*.inc")):
                if not (stage / p.name).exists() and not (stage.parent / "_shared" / p.name).exists():
                    shutil.copy2(p, stage / p.name)
                    borrowed.append(p.name)
            if borrowed:
                rep.warn("inc", f"{m.key}: {', '.join(borrowed)} not in the repo source dir, borrowed from KPM "
                         f"Patches/{m.name}/ (generated table missing from git?)")
        res = compile_mod(stage, m.name, compiler, kpm_dir, library)
        ok = True
        if res.dll is None:
            rep.fail("build", f"{m.key} ({m.name}): compile/link failed after {res.seconds:.1f}s")
            if err_stream is not None:
                lines = res.stderr.rstrip().splitlines()
                shown = lines[-MAX_ERR_LINES:]
                if len(lines) > len(shown):
                    print(f"  ... {len(lines) - len(shown)} earlier line(s) omitted", file=err_stream)
                for line in shown:
                    print(f"  | {line}", file=err_stream)
            return False
        try:
            exported = set(sc.PeImage(res.dll).exports())
        except (ValueError, OSError) as e:
            rep.fail("dll", f"{m.key}: built DLL is unreadable: {e}")
            return False
        funcs = {f for f in m.funcs if f}
        missing = sorted(funcs - exported)
        if missing:
            rep.fail("exports", f"{m.key}: detour function(s) not exported by the built DLL (KPM would skip the "
                     f"hook at runtime): {', '.join(missing)}")
            ok = False
        defp = m.dir / "exports.def"
        if defp.is_file():
            stale = sorted(set(sc.parse_def(defp.read_text(encoding="utf-8"))) - exported)
            if stale:
                rep.warn("def", f"{m.key}: exports.def name(s) absent from the built DLL: {', '.join(stale)}")
        if ok:
            rep.passed("build", f"{m.key} ({m.name}): built {len(exported)} export(s), {len(funcs)} detour "
                       f"function(s) present, {time.monotonic() - t0:.1f}s")
        return ok
    finally:
        if not keep:
            shutil.rmtree(key_dir, ignore_errors=True)


# ---------------------------------------------------------------- discovery

def _has_src_ancestor(rel: Path) -> bool:
    return any(part.endswith("_src") for part in rel.parts)


def discover(repo: Path, kpm_dir: Path, only=None) -> list:
    """ModSrc list. only=None: every dir under patch_manager_mods with an exports.def and a *_src path component
    (SKIP_DIRS pruned). only=[path...]: those dirs (relative to patch_manager_mods, or absolute)."""
    root = Path(repo) / "patch_manager_mods"
    if only:
        dirs = []
        for o in only:
            p = Path(o)
            dirs.append((p if p.is_absolute() else root / p).resolve())
    else:
        dirs = []
        for dirpath, dirnames, filenames in os.walk(root):
            dirnames[:] = sorted(d for d in dirnames if d not in sc.SKIP_DIRS)
            d = Path(dirpath)
            if "exports.def" in filenames and _has_src_ancestor(d.relative_to(root)):
                dirs.append(d.resolve())
    try:
        indexed = {m.dir.resolve(): m for m in sc.load_mods(repo, kpm_dir, variants=True)}
    except Exception:  # a broken toml elsewhere must not stop a build check
        indexed = {}
    out = []
    for d in sorted(dirs):
        try:
            key = "__".join(d.relative_to(root.resolve()).parts)
        except ValueError:
            key = d.name
        m = indexed.get(d)
        if m is not None:
            name = m.kpm_dir.name if m.kpm_dir is not None else d.name
            out.append(ModSrc(d, key, name, m.id, ce.detour_functions(m.hooks) - {""}, m.kpm_dir))
            continue
        funcs, pid = set(), ""
        hp = d / sc.HOOKS_TOML
        if hp.is_file():
            funcs = ce.detour_functions(sc.parse_hooks(hp.read_bytes())[1]) - {""}
        out.append(ModSrc(d, key, d.name, pid, funcs))
    return out


# ---------------------------------------------------------------- run

def run(repo=sc.REPO, kpm_dir=sc.KPM_DIR, only=None, keep=False, reporter=None, jobs=4, build_root=None,
        cache_root=None, err_stream=sys.stdout, compiler=None):
    """Build and check the selected mods; returns the list of Findings (also streamed to `reporter`)."""
    rep = reporter or sc.Reporter(out=None)
    mark = len(rep.findings)
    t_start = time.monotonic()
    compiler = compiler or find_compiler()
    if compiler is None:
        rep.skip("toolchain", "no i686-w64-mingw32-g++ (set CXX_WIN or install mingw-w64): nothing built")
        return rep.findings[mark:]
    kpm_dir = Path(kpm_dir)
    if not (kpm_dir / "Patches" / "Common").is_dir() or not (kpm_dir / "lib" / "sqlite3.def").is_file():
        rep.skip("kpm", f"no Patches/Common + lib/sqlite3.def under {kpm_dir} (set KPM_DIR): cannot link")
        return rep.findings[mark:]
    mods = discover(repo, kpm_dir, only)
    if not mods:
        rep.info("scope", "no mod sources selected")
        return rep.findings[mark:]
    for o in (only or []):
        p = Path(o)
        if not (p if p.is_absolute() else Path(repo) / "patch_manager_mods" / p).is_dir():
            rep.fail("scope", f"--mod {o}: no such directory")
    tmp = sc.tmp_dir() / "kotor-tests"
    build_root = Path(build_root) if build_root else tmp / "build"
    cache_root = Path(cache_root) if cache_root else tmp / "common-cache"
    shared_src = Path(repo) / "patch_manager_mods" / "_shared_src"

    t0 = time.monotonic()
    library, err = ensure_common_library(kpm_dir, compiler, cache_root)
    if library is None:
        rep.fail("common", err.strip().splitlines()[0] if err.strip() else "Common library build failed")
        if err_stream is not None:
            for line in err.rstrip().splitlines()[-MAX_ERR_LINES:]:
                print(f"  | {line}", file=err_stream)
        return rep.findings[mark:]
    rep.info("common", f"Common/GameAPI library ready ({time.monotonic() - t0:.1f}s)")

    def one(m):
        local = sc.Reporter(out=None)
        buf = []

        class Buf:  # collect stderr dumps so parallel jobs do not interleave
            def write(self, s):
                buf.append(s)

            def flush(self):
                pass
        ok = check_mod(local, m, compiler, kpm_dir, library, shared_src, build_root, keep,
                       err_stream=Buf() if err_stream is not None else None)
        return m, local, "".join(buf)

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        futs = [pool.submit(one, m) for m in mods]
        for fut in concurrent.futures.as_completed(futs):
            m, local, text = fut.result()
            for f in local.findings:
                rep.add(f.sev, f.check, f.msg)
            if text and err_stream is not None:
                err_stream.write(text)
    if not keep:
        with contextlib.suppress(OSError):
            build_root.rmdir()   # only succeeds if empty
    rep.info("time", f"{len(mods)} mod(s) in {time.monotonic() - t_start:.1f}s")
    return rep.findings[mark:]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--mod", action="append", default=[], metavar="DIR",
                    help="mod source dir relative to patch_manager_mods/ (e.g. perf_lab_src/perflab) or absolute "
                         "(repeatable; default: every *_src dir with an exports.def)")
    ap.add_argument("--quiet", action="store_true", help="hide PASS and INFO lines")
    ap.add_argument("--keep", action="store_true", help="keep the staged build dirs under $TMPDIR/kotor-tests/build/")
    ap.add_argument("--jobs", type=int, default=4, help="mods built in parallel (default 4)")
    ap.add_argument("--repo", default=str(sc.REPO), help="repo root holding patch_manager_mods/ (default: this checkout)")
    ap.add_argument("--kpm-dir", default=str(sc.KPM_DIR), help="KPM checkout (read-only) with Patches/Common and lib/")
    ap.add_argument("--build-root", default=None, help="staging root (default $TMPDIR/kotor-tests/build)")
    a = ap.parse_args(argv)
    rep = sc.Reporter(quiet=a.quiet)
    run(repo=a.repo, kpm_dir=a.kpm_dir, only=a.mod or None, keep=a.keep, reporter=rep, jobs=a.jobs,
        build_root=a.build_root)
    return rep.finish("build_check")


if __name__ == "__main__":
    sys.exit(main())
