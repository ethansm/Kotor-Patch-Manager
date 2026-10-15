#!/usr/bin/env python3
"""Tests for k1_port_char_build.py. Synthetic tempdir game dirs only; the real game dir is never touched.

pykotor is needed, so this file re-execs itself under the k1port venv (python -I). When the venv python is
missing it prints SKIP and exits 0.
Run: python3 -u test_k1_port_char_build.py   (cwd may be anything; exit 0 = all PASS)
"""
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_char_build.py"

def venv_python(env=os.environ) -> Path:
    """The k1port venv interpreter: $K1PORT_VENV (a venv dir, as setup_venv.sh reads it), else the HOME default."""
    base = env.get("K1PORT_VENV") or "~/Games/KOTOR2_Mod_Workspace/venvs/k1port"
    return Path(os.path.expanduser(base)) / "bin" / "python"


VENV_PY = venv_python()
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_char_build as cb  # noqa: E402  (no pykotor import at module level)
import test_k1_port_probe_build as tp  # noqa: E402  (fixtures: utc_bytes, mod_bytes, tree_hash)

APP_ROWS, HEAD_ROWS, SOUND_ROWS = 20, 7, 12
TESTS = []
KW = dict(tag="juhani", label="K1P_Juhani", new_row=APP_ROWS, new_head_row=HEAD_ROWS, body_a="k1p_juhaniba",
          body_b="k1p_juhanibb", head="k1p_juhanih", soundset=5, gender=1, name="Juhani")


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def table(headers, rows, cell):
    from pykotor.resource.formats.twoda import TwoDA, bytes_2da
    t = TwoDA()
    for h in headers:
        t.add_column(h)
    for i in range(rows):
        t.add_row(str(i), {h: cell(i, h) for h in headers})
    return bytes_2da(t)


def make_game(root: Path, *, app_rows=APP_ROWS, head_rows=HEAD_ROWS) -> Path:
    game = root / "game"
    (game / "override").mkdir(parents=True)
    (game / "modules").mkdir()
    headers = ["label", "race", "modeltype", "normalhead", "backuphead", "modela", "texa", "texaevil", "modelb",
               "texb", "texbevil", "modelc", "texc"]
    (game / "override" / "appearance.2da").write_bytes(table(
        headers, app_rows, lambda i, h: f"ROW_{i}" if h == "label" else {"modeltype": "B", "texaevil": "ev"}.get(h, f"{h}{i}")))
    (game / "override" / "heads.2da").write_bytes(table(
        ["head", "alttexture", "headtexve"], head_rows, lambda i, h: f"{h}{i}"))
    (game / "override" / "soundset.2da").write_bytes(table(["label"], SOUND_ROWS, lambda i, h: f"S{i}"))
    mod = tp.mod_bytes([("c_ithorian003", tp.utc_bytes(72, 3, tag="c_ithorian003"))])
    (game / "modules" / "204tel.mod").write_bytes(mod)
    return game


def refused(root, needle, **over):
    game = make_game(root, **{k: over.pop(k) for k in ("app_rows", "head_rows") if k in over})
    out = root / "out"
    try:
        cb.build(game, out, **{**KW, **over})
    except cb.BuildError as exc:
        check(needle in str(exc), f"refusal text lacks {needle!r}: {exc}")
        check(not out.exists(), "a refused build must write nothing")
        return
    raise AssertionError(f"expected BuildError containing {needle!r}")


@test
def appends_rows_and_builds_utc(root):
    from pykotor.resource.formats.twoda import read_2da
    from pykotor.resource.generics.utc import read_utc
    game = make_game(root)
    before = tp.tree_hash(game)
    info = cb.build(game, root / "out", **KW)
    check(tp.tree_hash(game) == before, "the game dir must not change")
    a = read_2da((root / "out" / "appearance.2da").read_bytes())
    h = read_2da((root / "out" / "heads.2da").read_bytes())
    check(a.get_height() == APP_ROWS + 1 and h.get_height() == HEAD_ROWS + 1, "tables grew by one row each")
    row = {c: a.get_cell(APP_ROWS, c) for c in a.get_headers()}
    check(a.get_label(APP_ROWS) == str(APP_ROWS), f"row id must stay numeric: {a.get_label(APP_ROWS)!r}")
    check(row["label"] == "K1P_Juhani" and row["race"] == "k1p_juhanibb", row)
    check((row["modela"], row["texa"], row["modelb"], row["texb"]) == ("k1p_juhaniba",) * 2 + ("k1p_juhanibb",) * 2, row)
    check(row["normalhead"] == str(HEAD_ROWS) and row["modeltype"] == "B" and row["modelc"] == "modelc17", row)
    check(row["texaevil"] == "" and row["texbevil"] == "" and row["backuphead"] == "", row)
    check({c: h.get_cell(HEAD_ROWS, c) for c in h.get_headers()} == {"head": "k1p_juhanih", "alttexture": "",
                                                                      "headtexve": ""}, "head row")
    for i in range(APP_ROWS):  # existing rows untouched
        check(a.get_cell(i, "modela") == f"modela{i}", f"row {i} changed")
    u = read_utc((root / "out" / "k1p_juhani.utc").read_bytes())
    check((u.tag, u.resref, u.appearance_id, u.soundset_id, u.gender_id, u.texture_variation) ==
          ("k1p_juhani", "k1p_juhani", APP_ROWS, 5, 1, 1), vars(u))
    check(u.first_name.get(0, 0) == "Juhani", u.first_name)
    on_disk = json.loads((root / "out" / "char_build.json").read_text())
    check(on_disk == info and set(info["sha256"]) == {"appearance.2da", "heads.2da", "k1p_juhani.utc"}, info)


@test
def chain_from_counts_on_from_the_previous_package(root):
    from pykotor.resource.formats.twoda import read_2da
    game = make_game(root)
    cb.build(game, root / "one", **KW)
    two = {**KW, "tag": "carth", "label": "K1P_Carth", "new_row": APP_ROWS + 1, "new_head_row": HEAD_ROWS + 1,
           "body_a": "k1p_carthba", "body_b": "k1p_carthbb", "head": "k1p_carthh", "gender": 0}
    info = cb.build(game, root / "two", chain_from=root / "one", **two)
    a = read_2da((root / "two" / "appearance.2da").read_bytes())
    h = read_2da((root / "two" / "heads.2da").read_bytes())
    check(a.get_height() == APP_ROWS + 2 and h.get_height() == HEAD_ROWS + 2, "both rows present")
    check(a.get_cell(APP_ROWS, "modela") == "k1p_juhaniba" and a.get_cell(APP_ROWS + 1, "modela") == "k1p_carthba", "rows")
    check(a.get_cell(APP_ROWS + 1, "normalhead") == str(HEAD_ROWS + 1) and h.get_cell(HEAD_ROWS + 1, "head") == "k1p_carthh", "head")
    check(info["chained_from"] == str((root / "one").resolve()), info)
    try:  # without the chain the live count is one lower than --new-row
        cb.build(game, root / "three", **two)
    except cb.BuildError as exc:
        check("--new-row is 21" in str(exc), str(exc))
    else:
        raise AssertionError("an unchained build with the chained row numbers must be refused")


@test
def stale_row_counts_are_refused(root):
    refused(root, "--new-row is 21", app_rows=APP_ROWS, new_row=APP_ROWS + 1)


@test
def stale_head_count_refused(root):
    refused(root, "--new-head-row is 9", new_head_row=HEAD_ROWS + 2)


@test
def soundset_out_of_range_refused(root):
    refused(root, "soundset 99", soundset=99)


@test
def bad_names_refused(root):
    refused(root, "body_a must be", body_a="K1P_Bad Name")
    refused(Path(tempfile.mkdtemp(dir=root)), "gender must be", gender=2)


@test
def template_utc_with_other_appearance_refused(root):
    refused(root, "expected 99", expect_appearance=99)


@test
def out_inside_game_dir_refused(root):
    game = make_game(root)
    try:
        cb.build(game, game / "override", **KW)
    except cb.BuildError as exc:
        check("inside the game dir" in str(exc), str(exc))
        return
    raise AssertionError("an --out inside the game dir must be refused")


@test
def cli_runs_and_reports_json(root):
    game = make_game(root)
    cmd = [str(VENV_PY), "-I", str(CLI), "--game-dir", str(game), "--tag", "carth", "--label", "K1P_Carth",
           "--new-row", str(APP_ROWS), "--new-head-row", str(HEAD_ROWS), "--body-a", "k1p_carthba",
           "--body-b", "k1p_carthbb", "--head", "k1p_carthh", "--soundset", "3", "--gender", "0", "--name", "Carth",
           "--out", str(root / "o"), "--json"]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    check(r.returncode == 0, r.stderr[-300:])
    check(json.loads(r.stdout)["utc"] == "k1p_carth" and (root / "o" / "k1p_carth.utc").is_file(), r.stdout[:200])
    bad = subprocess.run([*cmd[:-3], "--new-row", "3", "--out", str(root / "o2")], capture_output=True, text=True,
                         timeout=120)
    check(bad.returncode == 1 and "error:" in bad.stderr and not (root / "o2").exists(), bad.stderr[-200:])


def main() -> int:
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_char_test_"))
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


@test
def venv_python_honours_k1port_venv(root):
    venv = root / "venv"
    check(venv_python({"K1PORT_VENV": str(venv)}) == venv / "bin" / "python", "K1PORT_VENV ignored")
    check(venv_python({"K1PORT_VENV": ""}) == venv_python({}), "empty K1PORT_VENV must fall back to the default")
    check(venv_python({}) == Path(os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python")),
          "default venv path changed")


@test
def missing_venv_prints_visible_skip(root):
    """A missing venv still exits 0 (run_tests has no SKIP hook) but the SKIP line must name the K1PORT_VENV path."""
    env = dict(os.environ, HOME=str(root / "h"), K1PORT_VENV=str(root / "no_venv"))
    r = subprocess.run([sys.executable, str(Path(__file__).resolve())], env=env, capture_output=True, text=True,
                       timeout=120)
    check(r.returncode == 0 and "SKIP test_k1_port_char_build:" in r.stdout and str(root / "no_venv") in r.stdout,
          f"rc {r.returncode} stdout {r.stdout[-300:]!r} stderr {r.stderr[-300:]!r}")


if __name__ == "__main__":
    if not VENV_PY.exists():
        print(f"SKIP test_k1_port_char_build: {VENV_PY} missing (pykotor venv not set up)")
        sys.exit(0)
    if Path(sys.prefix).resolve() != VENV_PY.parent.parent.resolve():
        os.execv(str(VENV_PY), [str(VENV_PY), "-I", str(Path(__file__).resolve()), *sys.argv[1:]])
    sys.exit(main())
