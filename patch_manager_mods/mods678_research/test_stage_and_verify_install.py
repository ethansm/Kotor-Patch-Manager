"""Tests for copy_into / under in stage_and_verify_install.py (tmp dirs only; never touches the game dir or KPM repo)."""
import importlib.util, os, time
from pathlib import Path

_spec = importlib.util.spec_from_file_location("stage_and_verify_install", Path(__file__).with_name("stage_and_verify_install.py"))
sav = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(sav)


def test_copy_into_same_file_is_noop(tmp_path):
    stage = tmp_path / "stage"; stage.mkdir()
    f = stage / "a.kpatch"; f.write_bytes(b"data")
    os.utime(f, (1000, 1000))
    dst = sav.copy_into(str(f), str(stage))
    assert dst == str(f)
    assert f.read_bytes() == b"data" and int(f.stat().st_mtime) == 1000


def test_copy_into_different_file_copies(tmp_path):
    stage = tmp_path / "stage"; stage.mkdir()
    src = tmp_path / "src" / "b.kpatch"; src.parent.mkdir(); src.write_bytes(b"xyz")
    dst = sav.copy_into(str(src), str(stage))
    assert dst == str(stage / "b.kpatch") and Path(dst).read_bytes() == b"xyz"
    # second call (dst exists, different file) overwrites
    src.write_bytes(b"new")
    sav.copy_into(str(src), str(stage))
    assert Path(dst).read_bytes() == b"new"


def test_copy_into_rename_and_symlink_same_file(tmp_path):
    stage = tmp_path / "stage"; stage.mkdir()
    src = tmp_path / "x y.kpatch"; src.write_bytes(b"1")
    assert sav.copy_into(str(src), str(stage), "x_y.kpatch") == str(stage / "x_y.kpatch")
    link = tmp_path / "link.kpatch"; link.symlink_to(stage / "x_y.kpatch")
    sav.copy_into(str(stage / "x_y.kpatch"), str(stage), "x_y.kpatch")  # same path, no SameFileError


def test_under(tmp_path):
    stage = tmp_path / "stage"; stage.mkdir()
    assert sav.under(str(stage / "a.kpatch"), str(stage))
    assert not sav.under(str(tmp_path / "other" / "a.kpatch"), str(stage))
    assert not sav.under(str(tmp_path / "stage2" / "a.kpatch"), str(stage))
