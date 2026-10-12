#!/usr/bin/env python3
"""Tests for k1_port_spawn_build.py. Synthetic tempdir game dirs only; the real game dir is never touched.

pykotor is needed, so this file re-execs itself under the k1port venv (python -I). When the venv python
is missing it prints SKIP and exits 0 (run_tests.py has no py-item SKIP hook).
The real-compiler test SKIPs (not PASS) when the kit compile.sh is missing. Fake compilers cover the rest.
Run: python3 -u test_k1_port_spawn_build.py   (cwd may be anything; exit 0 = no FAIL)
"""
import contextlib
import hashlib
import io
import os
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_spawn_build as sb  # noqa: E402  (no pykotor import at module level)
import k1_port_probe_build as pb  # noqa: E402

TESTS = []
UTC_NAME = "k1p_probe_npc.utc"
NCS_NAME = "k_def_heartbt01.ncs"
FAKE_OK = 'printf "NCS V1.0B-FAKE" > "$1"'


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def tree_hash(root: Path) -> str:
    h = hashlib.sha256()
    for p in sorted(root.rglob("*")):
        rel = p.relative_to(root).as_posix()
        if p.is_dir():
            h.update(f"D {rel}\n".encode())
        else:
            h.update(f"F {rel} {hashlib.sha256(p.read_bytes()).hexdigest()}\n".encode())
    return h.hexdigest()


def expect_refusal(fn, needle: str) -> str:
    try:
        fn()
    except sb.BuildError as exc:
        check(needle in str(exc), f"refusal text lacks {needle!r}: {exc}")
        return str(exc)
    raise AssertionError(f"expected BuildError containing {needle!r}, but it succeeded")


def make_game(root: Path, appearance: int = 72, resref: str = "c_ithorian003") -> Path:
    """A synthetic game dir: override/appearance.2da (existence only) + modules/204tel.mod with one UTC."""
    from pykotor.common.language import LocalizedString
    from pykotor.common.misc import ResRef
    from pykotor.resource.formats.erf import ERF, ERFType, bytes_erf
    from pykotor.resource.generics.utc import UTC, bytes_utc
    from pykotor.resource.type import ResourceType
    game = root / "game"
    (game / "override").mkdir(parents=True, exist_ok=True)
    (game / "override" / "appearance.2da").write_bytes(b"2DA V2.b\n\n")  # _read_live only checks existence
    (game / "modules").mkdir(parents=True, exist_ok=True)
    u = UTC()
    u.appearance_id = appearance
    u.texture_variation = 1
    u.tag = "c_ithorian003"
    u.resref = ResRef(resref)
    u.first_name = LocalizedString.from_english("Ithorian Template")
    u.strength = 14
    u.max_hp = 33
    u.hp = 33
    erf = ERF(ERFType.MOD)
    erf.set_data(resref, ResourceType.UTC, bytes_utc(u))
    (game / "modules" / "204tel.mod").write_bytes(bytes_erf(erf))
    return game


def fake_compiler(root: Path, body: str) -> Path:
    p = root / "fake_compile.sh"
    p.write_text("#!/bin/sh\n" + body + "\n")
    p.chmod(0o755)
    return p


@contextlib.contextmanager
def compiler_at(path):
    old = sb.COMPILE_SH
    sb.COMPILE_SH = Path(path)
    try:
        yield
    finally:
        sb.COMPILE_SH = old


def out_listing(out: Path) -> list:
    return sorted(p.name for p in out.iterdir()) if out.is_dir() else []


@test
def test_constants_match_contract():
    check(sb.SPAWN_RESREF == "k1p_probe_npc", f"spawn resref is {sb.SPAWN_RESREF!r}")
    check(pb.NEW_ROW == 812 and sb.TEMPLATE_APPEARANCE == 72, "appearance ids drifted from 812/72")
    check(sb.SPAWN_TEXTURE_VAR == 4, "texture_variation must be 4")
    check(sb.SPAWN_NAME == "K1 Probe Citizen", f"spawn name is {sb.SPAWN_NAME!r}")
    check(sb.MODULE_REL == "modules/204tel.mod", f"module is {sb.MODULE_REL!r}")
    check(pb.RESREF_RE.fullmatch(sb.SPAWN_RESREF) is not None, "spawn resref fails RESREF_RE")
    check(sb.BuildError is pb.BuildError, "BuildError must be the probe build's class")


@test
def test_wrapper_passthrough_line():
    line = 'ExecuteScript("k_ai_master", OBJECT_SELF, KOTOR_DEFAULT_EVENT_ON_HEARTBEAT);'
    check(sb.WRAPPER_NSS.count(line) == 1, "pass-through ExecuteScript line must appear exactly once")
    check(sb.WRAPPER_NSS.rstrip().endswith(line + "\n}"), "pass-through must be the last statement in main()")


@test
def test_wrapper_tag_gate():
    gate = 'GetTag(oSelf) == "204_Ith_Rec" && !GetIsObjectValid(GetObjectByTag("k1p_probe_npc"))'
    spawn = 'CreateObject(OBJECT_TYPE_CREATURE, "k1p_probe_npc", Location(vSpawn, fBack));'
    check(gate in sb.WRAPPER_NSS, "tag gate (204_Ith_Rec + not-already-spawned) missing")
    check(sb.WRAPPER_NSS.index(gate) < sb.WRAPPER_NSS.index(spawn), "spawn must sit inside the tag gate")
    check(sb.WRAPPER_NSS.index(spawn) < sb.WRAPPER_NSS.index("ExecuteScript("), "spawn must run before pass-through")
    check("GetLocalInt" not in sb.WRAPPER_NSS and "GetResRef" not in sb.WRAPPER_NSS,
          "K2 has no GetLocalInt/GetResRef; gate must be tag-based")


@test
def test_refuses_out_inside_game_dir():
    with tempfile.TemporaryDirectory(prefix="k1sp_") as td:
        game = make_game(Path(td))
        before = tree_hash(game)
        with compiler_at(fake_compiler(Path(td), FAKE_OK)):
            expect_refusal(lambda: sb.build(game, game / "stage"), "inside the game dir")
            expect_refusal(lambda: sb.build(game, game), "inside the game dir")
        check(tree_hash(game) == before, "game dir changed after a refusal")


@test
def test_refuses_missing_module():
    with tempfile.TemporaryDirectory(prefix="k1sp_") as td:
        root = Path(td)
        game = make_game(root)
        (game / "modules" / "204tel.mod").unlink()
        out = root / "out"
        with compiler_at(fake_compiler(root, FAKE_OK)):
            expect_refusal(lambda: sb.build(game, out), "missing live file")
        check(not out.exists(), "out dir created on refusal")


@test
def test_refuses_wrong_appearance():
    with tempfile.TemporaryDirectory(prefix="k1sp_") as td:
        root = Path(td)
        game = make_game(root, appearance=17)
        out = root / "out"
        with compiler_at(fake_compiler(root, FAKE_OK)):
            expect_refusal(lambda: sb.build(game, out), "appearance_id is 17, expected 72")
        check(not out.exists(), "out dir created on refusal")


@test
def test_refuses_missing_template_utc():
    with tempfile.TemporaryDirectory(prefix="k1sp_") as td:
        root = Path(td)
        game = make_game(root, resref="c_ithorian004")
        out = root / "out"
        with compiler_at(fake_compiler(root, FAKE_OK)):
            expect_refusal(lambda: sb.build(game, out), "found 0")
        check(not out.exists(), "out dir created on refusal")


@test
def test_cli_refuses_out_inside_game_dir():
    with tempfile.TemporaryDirectory(prefix="k1sp_") as td:
        game = make_game(Path(td))
        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            rc = sb.main(["--game-dir", str(game), "--out", str(game / "x")])
        check(rc == 1, f"CLI exit code is {rc}, expected 1")
        check("error:" in err.getvalue() and "inside the game dir" in err.getvalue(), f"stderr: {err.getvalue()!r}")


@test
def test_utc_fields_and_two_files():
    from pykotor.common.language import Gender, Language
    with tempfile.TemporaryDirectory(prefix="k1sp_") as td:
        root = Path(td)
        game = make_game(root)
        out = root / "out"
        with compiler_at(fake_compiler(root, FAKE_OK)):
            res = sb.build(game, out)
        check(out_listing(out) == sorted([UTC_NAME, NCS_NAME]), f"out must hold exactly two files, got {out_listing(out)}")
        check(res["files"] == [UTC_NAME, NCS_NAME], f"result files: {res['files']}")
        api = pb._pykotor()
        u = api.read_utc((out / UTC_NAME).read_bytes())
        check(u.appearance_id == 812, f"appearance_id is {u.appearance_id}")
        check(u.texture_variation == 4, f"texture_variation is {u.texture_variation}")
        check(u.tag == "k1p_probe_npc", f"tag is {u.tag!r}")
        check(u.resref.get() == "k1p_probe_npc", f"template resref is {u.resref.get()!r}")
        check(u.first_name.get(Language.ENGLISH, Gender.MALE) == "K1 Probe Citizen",
              f"first name is {u.first_name.get(Language.ENGLISH, Gender.MALE)!r}")
        check(u.strength == 14 and u.max_hp == 33 and u.hp == 33, "template fields must be kept as-is")
        check((out / NCS_NAME).read_bytes() == b"NCS V1.0B-FAKE", "ncs must be the compiler output, not a copied file")


@test
def test_failing_compiler_leaves_nothing():
    cases = [
        ("missing kit", None, "compile kit missing"),
        ("nonzero exit", 'printf "NCS V1.0B" > "$1"; exit 3', "exited 3"),
        ("no output", "exit 0", "no .ncs"),
        ("empty output", ': > "$1"', "missing or empty"),
        ("bad magic", 'printf "XXXX" > "$1"', "does not start with"),
    ]
    for label, body, needle in cases:
        with tempfile.TemporaryDirectory(prefix="k1sp_") as td:
            root = Path(td)
            game = make_game(root)
            out = root / "out"
            path = root / "nope" / "compile.sh" if body is None else fake_compiler(root, body)
            with compiler_at(path):
                expect_refusal(lambda: sb.build(game, out), needle)
            check(out_listing(out) == [], f"{label}: out must hold no files, got {out_listing(out)}")


@test
def test_real_compiler_produces_ncs():
    if not sb.COMPILE_SH.is_file():
        raise Skip(f"compile.sh missing at {sb.COMPILE_SH}")
    with tempfile.TemporaryDirectory(prefix="k1sp_") as td:
        root = Path(td)
        game = make_game(root)
        out = root / "out"
        sb.build(game, out)
        data = (out / NCS_NAME).read_bytes()
        check(len(data) > 0 and data.startswith(sb.NCS_MAGIC), f"real .ncs header is {data[:10]!r}")
        check(out_listing(out) == sorted([UTC_NAME, NCS_NAME]), f"out must hold exactly two files, got {out_listing(out)}")


def _reexec_tests_under_venv():
    """Exec THIS test file under the k1port venv python -I when pykotor is missing. Returns a SKIP reason or None."""
    try:
        import pykotor  # noqa: F401
        return None
    except ImportError:
        pass
    venv_py = sb.VENV_PY
    if Path(sys.prefix).resolve() == venv_py.parent.parent.resolve():
        return f"pykotor not importable inside the venv {venv_py.parent.parent}"
    if not venv_py.exists():
        return f"pykotor not importable and the venv python is missing: {venv_py}"
    os.execv(str(venv_py), [str(venv_py), "-I", str(Path(__file__).resolve()), *sys.argv[1:]])


def main() -> int:
    reason = _reexec_tests_under_venv()
    if reason:
        print(f"SKIP all: {reason}")
        return 0
    passed = skipped = failed = 0
    for fn in TESTS:
        try:
            fn()
        except Skip as exc:
            skipped += 1
            print(f"SKIP {fn.__name__}: {exc}", flush=True)
        except Exception as exc:  # AssertionError or unexpected error: both FAIL
            failed += 1
            print(f"FAIL {fn.__name__}: {type(exc).__name__}: {exc}", flush=True)
        else:
            passed += 1
            print(f"PASS {fn.__name__}", flush=True)
    total = len(TESTS)
    print(f"{passed}/{total} PASS, {skipped} SKIP, {failed} FAIL")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
