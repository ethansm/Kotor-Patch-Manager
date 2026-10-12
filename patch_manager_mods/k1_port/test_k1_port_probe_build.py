#!/usr/bin/env python3
"""Tests for k1_port_probe_build.py. Synthetic tempdir game dirs only; the real game dir is never touched.

pykotor is needed, so this file re-execs itself under the k1port venv (python -I). When the venv python
is missing it prints SKIP and exits 0 (run_tests.py has no py-item SKIP hook).
Run: python3 -u test_k1_port_probe_build.py   (cwd may be anything; exit 0 = all PASS)
"""
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_probe_build.py"
VENV_PY = Path(os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python"))
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_probe_build as kb  # noqa: E402  (no pykotor import at module level)

HEADERS = ["label", "race", "modela", "modelb", "texa", "texb", "hp"]
TEXA_COLS = ("race", "modela", "modelb", "texa", "texb")
TESTS = []
_TWODA_CACHE = {}


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def sha_of(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def tree_hash(root: Path) -> str:
    h = hashlib.sha256()
    for p in sorted(root.rglob("*")):
        rel = p.relative_to(root).as_posix()
        if p.is_dir():
            h.update(f"D {rel}\n".encode())
        else:
            h.update(f"F {rel} {sha_of(p.read_bytes())}\n".encode())
    return h.hexdigest()


def expect_refusal(fn, needle: str) -> str:
    try:
        fn()
    except kb.BuildError as exc:
        check(needle in str(exc), f"refusal text lacks {needle!r}: {exc}")
        return str(exc)
    raise AssertionError(f"expected BuildError containing {needle!r}, but it succeeded")


def base_2da_bytes(rows: int = 812) -> bytes:
    """A synthetic appearance.2da: row 17 is N_CommM, every other row is a distinct ROW_nnn."""
    if rows not in _TWODA_CACHE:
        from pykotor.resource.formats.twoda import TwoDA, bytes_2da
        t = TwoDA()
        for h in HEADERS:
            t.add_column(h)
        for i in range(rows):
            t.add_row(str(i), {
                "label": "N_CommM" if i == kb.TEMPLATE_ROW else f"ROW_{i:03d}",
                "race": f"race{i}", "modela": f"ma{i}", "modelb": f"mb{i}",
                "texa": f"ta{i}", "texb": f"tb{i}", "hp": str(i * 3),
            })
        _TWODA_CACHE[rows] = bytes_2da(t)
    return _TWODA_CACHE[rows]


def utc_bytes(appearance: int = 17, variation: int = 2, tag: str = "per_miner_002") -> bytes:
    from pykotor.resource.generics.utc import UTC, bytes_utc
    u = UTC()
    u.tag = tag
    u.hp = 33
    u.max_hp = 33
    u.strength = 14
    u.appearance_id = appearance
    u.texture_variation = variation
    return bytes_utc(u)


def mod_bytes(entries) -> bytes:
    from pykotor.resource.formats.erf import ERF, bytes_erf
    from pykotor.resource.type import ResourceType
    erf = ERF()
    for resref, data in entries:
        erf.set_data(resref, ResourceType.UTC, data)
    return bytes_erf(erf, ResourceType.MOD)


def make_game(root: Path, *, name: str = "game", rows: int = 812, utc: bool = True,
              utc_appearance: int = 17) -> Path:
    game = root / name
    (game / "override").mkdir(parents=True)
    (game / "modules").mkdir()
    (game / "chitin.key").write_bytes(b"KEY")
    (game / "override" / "bystander.utc").write_bytes(b"UTC bystander, not ours")
    (game / "override" / "appearance.2da").write_bytes(base_2da_bytes(rows))
    entries = [("per_miner_003", utc_bytes(17, 9, tag="decoy"))]  # decoy: must not be picked
    if utc:
        entries.append(("per_miner_002", utc_bytes(utc_appearance, 2)))
    (game / "modules" / "201TEL.mod").write_bytes(mod_bytes(entries))
    return game


def read_table(path: Path):
    from pykotor.resource.formats.twoda import read_2da
    return read_2da(str(path))


def norm(v):
    """Recursive plain-data view of a parsed UTC, for field-by-field comparison."""
    import enum
    if v is None or isinstance(v, (bool, int, float, str, bytes)):
        return v
    if isinstance(v, enum.Enum):
        return v.name
    if isinstance(v, (list, tuple)):
        return [norm(x) for x in v]
    if isinstance(v, dict):
        return {str(k): norm(x) for k, x in v.items()}
    if hasattr(v, "__dict__") and not isinstance(v, type):
        return {k: norm(x) for k, x in vars(v).items() if not k.startswith("__")}
    return str(v)


def run_cli(*args, python=None):
    env = dict(os.environ)
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    return subprocess.run([python or sys.executable, "-I", str(CLI), *args],
                          capture_output=True, text=True, env=env, timeout=240)


@test
def test_row_812_appended_with_expected_cells(root):
    game = make_game(root)
    out = root / "out"
    base = (game / "override" / "appearance.2da").read_bytes()
    res = kb.build(game, out)
    check(set(res) == {"appearance_2da", "utc", "base_sha256", "new_row"}, f"result keys: {sorted(res)}")
    check(res["new_row"] == 812, f"new_row {res['new_row']}")
    check(res["base_sha256"] == sha_of(base), "base_sha256 is not the sha of the live appearance.2da")
    app = (out / "appearance.2da").read_bytes()
    check(res["appearance_2da"] == sha_of(app), "appearance_2da sha does not match the written file")
    check(res["utc"] == sha_of((out / "per_miner_002.utc").read_bytes()), "utc sha does not match the written file")
    t = read_table(out / "appearance.2da")
    check(t.get_height() == 813, f"height {t.get_height()}")
    # The probe label is pykotor's row label (set_label), i.e. the leading index column of the file.
    check(t.get_label(812) == "K1P_CommM_Probe", f"row 812 row label {t.get_label(812)}")
    for c in TEXA_COLS:
        check(t.get_cell(812, c) == "k1p_commm", f"row 812 {c} = {t.get_cell(812, c)}")
    check(t.get_cell(812, "hp") == t.get_cell(17, "hp"), "row 812 hp must copy row 17")
    base_t = read_table(game / "override" / "appearance.2da")
    for i in range(812):
        check(t.get_label(i) == base_t.get_label(i), f"row {i} label changed")
        for h in HEADERS:
            check(t.get_cell(i, h) == base_t.get_cell(i, h), f"row {i} {h} changed")


@test
def test_texa_and_texture_var_args(root):
    game = make_game(root)
    out = root / "out"
    res = kb.build(game, out, texa="k1p_alt", texture_var=7)
    t = read_table(out / "appearance.2da")
    for c in TEXA_COLS:
        check(t.get_cell(812, c) == "k1p_alt", f"row 812 {c} = {t.get_cell(812, c)}")
    from pykotor.resource.generics.utc import read_utc
    u = read_utc((out / "per_miner_002.utc").read_bytes())
    check(u.texture_variation == 7, f"texture_variation {u.texture_variation}")
    check(res["utc"] == sha_of((out / "per_miner_002.utc").read_bytes()), "utc sha mismatch")


@test
def test_utc_only_appearance_and_variation_change(root):
    from pykotor.resource.generics.utc import read_utc
    game = make_game(root)
    out = root / "out"
    kb.build(game, out)
    orig = read_utc(utc_bytes(17, 2))
    new = read_utc((out / "per_miner_002.utc").read_bytes())
    check(new.appearance_id == 812, f"appearance_id {new.appearance_id}")
    check(new.texture_variation == 4, f"texture_variation {new.texture_variation}")

    def strip(d):
        return {k: v for k, v in d.items() if k not in ("appearance_id", "texture_variation")}

    n_orig, n_new = norm(orig), norm(new)
    check(n_new.get("tag") == "per_miner_002", "output must come from per_miner_002, not the decoy")
    check("hp" in n_new and n_new["hp"] == 33, "compared field set looks empty")
    check(strip(n_orig) == strip(n_new), "a UTC field other than appearance_id/texture_variation changed")


@test
def test_height_not_812_refuses(root):
    game = make_game(root, rows=813)
    out = root / "out"
    before = tree_hash(game)
    msg = expect_refusal(lambda: kb.build(game, out), "813 rows")
    check("expected 812" in msg, f"message: {msg}")
    check(not out.exists(), "a refused build must not create the out dir")
    check(tree_hash(game) == before, "game dir changed on refusal")


@test
def test_missing_utc_refuses(root):
    game = make_game(root, utc=False)  # only the decoy per_miner_003 is in the module
    out = root / "out"
    before = tree_hash(game)
    expect_refusal(lambda: kb.build(game, out), "per_miner_002.utc")
    check(not out.exists(), "a refused build must not create the out dir")
    check(tree_hash(game) == before, "game dir changed on refusal")


@test
def test_utc_appearance_mismatch_refuses(root):
    game = make_game(root, utc_appearance=18)  # template row is 17
    out = root / "out"
    expect_refusal(lambda: kb.build(game, out), "appearance_id is 18, expected 17")
    check(not out.exists(), "a refused build must write nothing")


@test
def test_out_inside_game_refuses(root):
    game = make_game(root)
    before = tree_hash(game)
    for bad in (game, game / "override" / "probe_out"):
        expect_refusal(lambda b=bad: kb.build(game, b), "inside the game dir")
    check(not (game / "override" / "probe_out").exists(), "refused --out must not be created")
    check(tree_hash(game) == before, "game dir changed on refusal")
    r = run_cli("--game-dir", game, "--out", game / "override")
    check(r.returncode == 1 and "inside the game dir" in r.stderr, f"CLI rc {r.returncode}: {r.stderr}")


@test
def test_game_dir_untouched_on_success(root):
    game = make_game(root)
    before = tree_hash(game)
    kb.build(game, root / "out")
    check(tree_hash(game) == before, "successful build must not modify the game dir")


@test
def test_bad_inputs_refuse(root):
    game = make_game(root)
    out = root / "out"
    expect_refusal(lambda: kb.build(game, out, texa="K1P CommM"), "texa")
    expect_refusal(lambda: kb.build(game, out, texture_var=256), "texture_var")
    expect_refusal(lambda: kb.build(game, out, template_row=900), "template row 900")
    check(not out.exists(), "refused builds must write nothing")


@test
def test_cli_json_and_plain_output(root):
    game = make_game(root)
    base = (game / "override" / "appearance.2da").read_bytes()
    out = root / "out"
    r = run_cli("--game-dir", game, "--out", out, "--json")
    check(r.returncode == 0, f"rc {r.returncode}: {r.stderr}")
    res = json.loads(r.stdout)
    check(res == {"appearance_2da": sha_of((out / "appearance.2da").read_bytes()),
                  "utc": sha_of((out / "per_miner_002.utc").read_bytes()),
                  "base_sha256": sha_of(base), "new_row": 812}, f"JSON result: {res}")
    r2 = run_cli("--game-dir", game, "--out", root / "out2")
    check(r2.returncode == 0, f"plain rc {r2.returncode}: {r2.stderr}")
    lines = r2.stdout.splitlines()
    check(lines[:3] == [f"appearance_2da: {res['appearance_2da']}", f"utc: {res['utc']}",
                        f"base_sha256: {res['base_sha256']}"], f"plain output: {r2.stdout}")


@test
def test_cli_refusal_exit_1(root):
    game = make_game(root, rows=811)
    out = root / "out"
    before = tree_hash(game)
    r = run_cli("--game-dir", game, "--out", out)
    check(r.returncode == 1, f"rc {r.returncode}")
    check("error: live appearance.2da has 811 rows" in r.stderr, f"stderr: {r.stderr}")
    check(r.stdout == "" and not out.exists(), "refused CLI run must print no hashes and write nothing")
    check(tree_hash(game) == before, "game dir changed on refusal")


@test
def test_cli_reexecs_from_base_python(root):
    # Base interpreter, not the venv. If it lacks pykotor, the CLI must re-exec into the venv and succeed.
    base = getattr(sys, "_base_executable", None) or "/usr/bin/python3"
    game = make_game(root)
    out = root / "out"
    r = run_cli("--game-dir", game, "--out", out, python=base)
    check(r.returncode == 0, f"rc {r.returncode} under {base}: {r.stderr}")
    check(r.stdout.startswith("appearance_2da: "), f"stdout: {r.stdout}")


def main() -> int:
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_build_test_"))
        try:
            fn(root)
            print(f"PASS {fn.__name__}")
            passed += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
        finally:
            shutil.rmtree(root, ignore_errors=True)
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    if not VENV_PY.exists():
        print(f"SKIP test_k1_port_probe_build: {VENV_PY} missing (pykotor venv not set up)")
        sys.exit(0)
    if Path(sys.prefix).resolve() != VENV_PY.parent.parent.resolve():
        os.execv(str(VENV_PY), [str(VENV_PY), "-I", str(Path(__file__).resolve()), *sys.argv[1:]])
    sys.exit(main())
