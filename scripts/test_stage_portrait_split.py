"""Tests for stage_portrait_split.py helpers (tmp dirs only)."""
import importlib.util, re, zipfile
from pathlib import Path

_spec = importlib.util.spec_from_file_location("stage_portrait_split", Path(__file__).with_name("stage_portrait_split.py"))
sps = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(sps)


def test_docstring_has_no_hardcoded_versions():
    assert not re.search(r"\d+\.\d+\.\d+", sps.__doc__)


def test_manifest_version_reads(tmp_path):
    p = tmp_path / "a.kpatch"
    with zipfile.ZipFile(p, "w") as z:
        z.writestr("manifest.toml", 'id = "x"\nversion = "1.2.3"\n')
    assert sps.manifest_version(p) == "1.2.3"


def test_manifest_version_missing(tmp_path):
    p = tmp_path / "b.kpatch"
    with zipfile.ZipFile(p, "w") as z:
        z.writestr("other.txt", "hi")
    assert sps.manifest_version(p) is None
    q = tmp_path / "c.kpatch"
    with zipfile.ZipFile(q, "w") as z:
        z.writestr("manifest.toml", 'id = "x"\n')
    assert sps.manifest_version(q) is None
    assert sps.manifest_version(tmp_path / "nope.kpatch") is None
