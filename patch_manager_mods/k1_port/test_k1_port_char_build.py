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
    letters = "abcdefghijklmn"
    headers = ["label", "race", "modeltype", "normalhead", "backuphead", "height", "headtrack", "texaevil", "texbevil",
               "texievil", "texlevil", "texnevil"] + [f"{k}{L}" for L in letters for k in ("model", "tex")]
    blank_17 = {"modelj", "texj", "modell", "texl"}  # row 17 (commoner) has no Revan-armour / slave-outfit body
    f_row = {"modela": "P_Single", "texa": "P_Single", "race": "P_Single", "modeltype": "F"}

    def cell(i, h):
        if h == "label":
            return f"ROW_{i}"
        if i == 17 and h in blank_17:
            return ""
        if i == 18:  # single-model (Zaalbar-like) row: only race/modela/texa, no body letters, no head
            return f_row.get(h, "")
        if h in ("texievil", "texlevil", "texnevil", "texbevil"):  # Mira-like evil textures on row 16
            return f"{h}{i}" if i == 16 else ""
        return {"modeltype": "B", "texaevil": "ev" if i != 5 else "", "height": "1.6" if i == 5 else "1"}.get(h, f"{h}{i}")
    (game / "override" / "appearance.2da").write_bytes(table(headers, app_rows, cell))
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
        check(a.get_cell(i, "modela") == ("P_Single" if i == 18 else f"modela{i}"), f"row {i} changed")
    u = read_utc((root / "out" / "k1p_juhani.utc").read_bytes())
    check((u.tag, u.resref, u.appearance_id, u.soundset_id, u.gender_id, u.texture_variation) ==
          ("k1p_juhani", "k1p_juhani", APP_ROWS, 5, 1, 1), vars(u))
    check(u.first_name.get(0, 0) == "Juhani", u.first_name)
    on_disk = json.loads((root / "out" / "char_build.json").read_text())
    check(on_disk == info and set(info["sha256"]) == {"appearance.2da", "heads.2da", "k1p_juhani.utc"}, info)


@test
def conversation_and_soundset_table_options(root):
    from pykotor.resource.generics.utc import read_utc
    game = make_game(root)
    base = cb.build(game, root / "base", **KW)
    check(base["conversation"] != "kvjdlg01", base)
    # extra row 12 exists only in the staged table
    extra = root / "sound_table.2da"
    extra.write_bytes(table(["label"], SOUND_ROWS + 1, lambda i, h: f"S{i}"))
    kw = dict(KW, soundset=SOUND_ROWS, conversation="kvjdlg01", soundset_table=extra)
    info = cb.build(game, root / "out2", **kw)
    u = read_utc((root / "out2" / "k1p_juhani.utc").read_bytes())
    check(str(u.conversation) == "kvjdlg01" and u.soundset_id == SOUND_ROWS and info["conversation"] == "kvjdlg01", vars(u))
    u0 = read_utc((root / "base" / "k1p_juhani.utc").read_bytes())
    check(str(u0.conversation) == base["conversation"], "default must keep the template conversation")


@test
def live_soundset_table_still_bounds_the_row(root):
    refused(root, "soundset 12", soundset=SOUND_ROWS)  # live table has only 12 rows (0..11)


@test
def bad_conversation_refused(root):
    refused(root, "conversation must be", conversation="Bad Name")


@test
def missing_soundset_table_refused(root):
    refused(root, "missing soundset table", soundset_table=root / "nope.2da")


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


def built_row(root, name="out", **over):
    """Build with KW + over against a fresh make_game; returns (info, appearance row dict of the new row, out dir)."""
    from pykotor.resource.formats.twoda import read_2da
    game = make_game(root)
    info = cb.build(game, root / name, **{**KW, **over})
    a = read_2da((root / name / "appearance.2da").read_bytes())
    return info, {c: a.get_cell(APP_ROWS, c) for c in a.get_headers()}, root / name


LETTERS = "abcdefghijklmn"


def all_letters_set(row):
    return all(row[f"{k}{L}"].strip() for L in LETTERS for k in ("model", "tex"))


@test
def fill_blank_letters_pure_female_male_and_noop(root):
    row = {f"{k}{L}": f"{k}{L}_x" for L in LETTERS for k in ("model", "tex")}
    check(cb.fill_blank_letters(row, body_b="k1p_xbb", female=True) == {}, "a full row needs no fill")
    row.update(modelj="", texj="", modell="", texl="")
    fem = cb.fill_blank_letters(row, body_b="k1p_xbb", female=True)
    check(fem == {"modelj": "k1p_xbb", "texj": "k1p_xbb", "modell": "PFBL", "texl": "PFBLA"}, fem)
    male = cb.fill_blank_letters(row, body_b="k1p_xbb", female=False)
    check(male == {"modelj": "k1p_xbb", "texj": "k1p_xbb", "modell": "k1p_xbb", "texl": "k1p_xbb"}, male)
    row.update(modelg=" ", texg="")  # whitespace is blank, any letter is aliased (not only j / l)
    check(cb.fill_blank_letters(row, body_b="k1p_xbb", female=False)["modelg"] == "k1p_xbb", "blank g")
    row.update(modelk="PMBKM", texk="")  # model set, texture blank: not filled (the guard reports it)
    check("modelk" not in cb.fill_blank_letters(row, body_b="b", female=True) and
          "texk" not in cb.fill_blank_letters(row, body_b="b", female=True), "a set model is left alone")


@test
def guard_lists_blank_cells(root):
    row = {f"{k}{L}": "v" for L in LETTERS for k in ("model", "tex")}
    cb.check_no_blank_letters(row)  # full row passes
    row["texk"] = ""
    del row["modeln"]
    try:
        cb.check_no_blank_letters(row)
    except cb.BuildError as exc:
        check("texk" in str(exc) and "modeln" in str(exc), str(exc))
        return
    raise AssertionError("blank/missing cells must raise BuildError")


@test
def female_row_gets_pfbl_and_aliased_j(root):
    info, row, _ = built_row(root)  # template 17: modelj/texj/modell/texl blank, KW gender 1
    check((row["modell"], row["texl"]) == ("PFBL", "PFBLA"), row)
    check((row["modelj"], row["texj"]) == ("k1p_juhanibb",) * 2, row)
    check(all_letters_set(row), "no blank letter")
    check(info["filled_cells"] == {"modelj": "k1p_juhanibb", "texj": "k1p_juhanibb", "modell": "PFBL", "texl": "PFBLA"}, info)
    check(row["modelc"] == "modelc17" and row["modeln"] == "modeln17", "filled cells are untouched")


@test
def male_row_aliases_slave_letter_to_clothes_body(root):
    info, row, _ = built_row(root, tag="carth", label="K1P_Carth", body_a="k1p_carthba", body_b="k1p_carthbb",
                             head="k1p_carthh", gender=0)
    check((row["modell"], row["texl"], row["modelj"], row["texj"]) == ("k1p_carthbb",) * 4, row)
    check(all_letters_set(row), "no blank letter")


@test
def full_party_row_template_is_a_noop_fill_and_keeps_physics(root):
    info, row, _ = built_row(root, template_row=5)  # row 5: every letter set, height 1.6, texaevil empty
    check(info["filled_cells"] == {}, info["filled_cells"])
    check(row["modelj"] == "modelj5" and row["texl"] == "texl5" and row["modell"] == "modell5", row)
    check(row["height"] == "1.6" and row["headtrack"] == "headtrack5", row)
    check(row["race"] == "k1p_juhanibb" and row["label"] == "K1P_Juhani" and row["modela"] == "k1p_juhaniba", row)


@test
def every_evil_texture_column_is_cleared(root):
    _info, row, _ = built_row(root, template_row=16)  # Mira-like: all evil textures set
    for col in cb.CLEARED:
        check(row[col] == "", f"{col} leaked: {row[col]!r}")
    check({"texaevil", "texbevil", "texievil", "texlevil", "texnevil", "backuphead"} == set(cb.CLEARED), cb.CLEARED)


@test
def blank_cell_after_fill_is_refused_and_writes_nothing(root):
    game = make_game(root)
    from pykotor.resource.formats.twoda import read_2da, bytes_2da
    t = read_2da((game / "override" / "appearance.2da").read_bytes())
    t.set_cell(5, "modelk", "PMBKM")
    t.set_cell(5, "texk", "")  # model set, texture blank: fill_blank_letters leaves it, the guard must refuse
    (game / "override" / "appearance.2da").write_bytes(bytes_2da(t))
    try:
        cb.build(game, root / "out", **{**KW, "template_row": 5})
    except cb.BuildError as exc:
        check("blank body cells texk" in str(exc), str(exc))
        check(not (root / "out").exists(), "a refused build must write nothing")
        return
    raise AssertionError("a leftover blank cell must be refused")


@test
def single_model_f_template_sets_race_only(root):
    from pykotor.resource.formats.twoda import read_2da
    kw = dict(tag="zaalbar", label="K1P_Zaalbar", body_a="k1p_zaalbar", body_b=None, head=None, new_head_row=None,
              gender=0, template_row=18, utc="k1p_zaalbaru")
    game = make_game(root)
    info = cb.build(game, root / "out", **{**KW, **kw})
    a = read_2da((root / "out" / "appearance.2da").read_bytes())
    row = {c: a.get_cell(APP_ROWS, c) for c in a.get_headers()}
    check((row["race"], row["modela"], row["texa"], row["modeltype"]) == ("k1p_zaalbar",) * 3 + ("F",), row)
    check(row["label"] == "K1P_Zaalbar" and row["normalhead"] == "" and row["modelb"] == "" and row["modelc"] == "", row)
    check("P_Single" not in row.values(), f"stale vanilla model left in the row: {row}")
    check(info["filled_cells"] == {} and info["heads_row"] is None and info["modeltype"] == "F", info)
    check(sorted(p.name for p in (root / "out").iterdir()) == ["appearance.2da", "char_build.json", "k1p_zaalbaru.utc"],
          "no heads.2da for a headless row")
    check(set(info["sha256"]) == {"appearance.2da", "k1p_zaalbaru.utc"} and "heads.2da" not in info["base_sha256"], info)


@test
def single_model_importer_shape_body_b_equals_body_a_with_head(root):
    """What k1_port_import.char_build_argv passes today for a one-model recipe: --body-b == --body-a, plus a head."""
    info, row, out = built_row(root, body_a="k1p_zaalbar", body_b="k1p_zaalbar", head="k1p_zaalbarh", template_row=18,
                               gender=0, tag="zaalbar", label="K1P_Zaalbar")
    check((row["race"], row["modela"], row["modeltype"], row["modelb"]) == ("k1p_zaalbar", "k1p_zaalbar", "F", ""), row)
    check(row["normalhead"] == str(HEAD_ROWS) and (out / "heads.2da").is_file() and info["heads_row"] == HEAD_ROWS, info)


@test
def single_model_with_different_body_b_refused(root):
    refused(root, "differs from body_a", template_row=18, body_b="k1p_other")


@test
def body_template_without_body_b_or_head_refused(root):
    refused(root, "body_b and head are required", body_b=None)
    refused(Path(tempfile.mkdtemp(dir=root)), "body_b and head are required", head=None, new_head_row=None)


@test
def headless_chain_needs_only_appearance(root):
    game = make_game(root)
    kw = dict(KW, template_row=18, body_b=None, head=None, new_head_row=None, tag="zaalbar", label="K1P_Zaalbar",
              body_a="k1p_zaalbar")
    cb.build(game, root / "one", **{**kw, "tag": "z1", "label": "K1P_Z1"})
    (root / "one" / "heads.2da").unlink(missing_ok=True)
    info = cb.build(game, root / "two", chain_from=root / "one", **{**kw, "new_row": APP_ROWS + 1})
    check(info["chained_from"] == str((root / "one").resolve()) and info["heads_row"] is None, info)


@test
def cli_single_model_without_head_args(root):
    game = make_game(root)
    cmd = [str(VENV_PY), "-I", str(CLI), "--game-dir", str(game), "--tag", "zaalbar", "--label", "K1P_Zaalbar",
           "--new-row", str(APP_ROWS), "--body-a", "k1p_zaalbar", "--soundset", "3", "--gender", "0", "--name", "Zaalbar",
           "--template-row", "18", "--utc", "k1p_zaalbaru", "--out", str(root / "o"), "--json"]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    check(r.returncode == 0, r.stderr[-300:])
    check(json.loads(r.stdout)["heads_row"] is None and not (root / "o" / "heads.2da").exists(), r.stdout[:200])


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
