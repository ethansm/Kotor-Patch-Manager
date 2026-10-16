"""Tests for k1_port_audio_test_build (needs the k1port venv; re-execs itself under it, SKIP line if missing)."""
import os
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
VENV_PY = Path(os.environ.get("K1PORT_VENV") or "~/Games/KOTOR2_Mod_Workspace/venvs/k1port").expanduser() / "bin" / "python"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_audio_test_build as ab  # noqa: E402

LIP = b"LIP V1.0" + bytes(8) + bytes(5)
WAV = b"RIFF2\0\0\0WAVEfmt " + bytes(60)


def fake_k1(root):
    from pykotor.resource.formats.erf import ERF, ERFType, write_erf
    from pykotor.resource.type import ResourceType
    k1 = root / "swkotor"
    (k1 / "StreamWaves" / "M35AA" / "JUHA11").mkdir(parents=True)  # mixed case on purpose
    (k1 / "StreamWaves" / "M35AA" / "JUHA11" / "NM35AAJUHA11395_.wav").write_bytes(WAV)
    (k1 / "lips").mkdir()
    erf = ERF(ERFType.MOD)
    erf.set_data("nm35aajuha11395_", ResourceType.LIP, LIP)
    (k1 / "lips" / "localization.mod").write_bytes(__import__("pykotor.resource.formats.erf", fromlist=["bytes_erf"]).bytes_erf(erf))
    return k1


def main():
    from pykotor.resource.generics.dlg import read_dlg
    ok = 0
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        out = root / "out"
        res = ab.build(fake_k1(root), out, adopt=False)
        assert res["wav_bytes"] == len(WAV) and res["lip_bytes"] == len(LIP), res
        ov, sv = out / "override", out / "streamvoice"
        assert sorted(p.name for p in ov.iterdir()) == ["kvjdlg01.dlg", "kvjv0001.lip", "kvjv0001.wav", "kvjv0002.wav",
                                                       "kvjv0003.lip", "kvjv0003.wav"], list(ov.iterdir())
        assert sorted(p.name for p in sv.iterdir()) == ["kvjv0001.wav", "kvjv0002.wav", "kvjv0003.wav"]
        print("PASS file sets"); ok += 1
        assert (ov / "kvjv0001.lip").read_bytes() == LIP and (ov / "kvjv0003.lip").read_bytes().startswith(b"BAD ")
        assert len(list(ov.glob("kvjv0002.lip"))) == 0 and (ov / "kvjv0002.wav").read_bytes() == WAV
        print("PASS lip variants valid / none / corrupt"); ok += 1
        d = read_dlg((ov / "kvjdlg01.dlg").read_bytes())
        vo = [str(e.vo_resref) for e in d.all_entries()]
        assert len(d.starters) == 1 and sorted(vo) == ["kvjv0001", "kvjv0002", "kvjv0003"], vo
        assert all(len(str(e.vo_resref)) <= 16 and e.text.get(0, 0) for e in d.all_entries())
        print("PASS dialog: 1 starter, 3 entries with VO and inline text"); ok += 1
        assert d.starters[0].node.links[0].node.links[0].node is not None
        print("PASS chain entry -> reply -> entry"); ok += 1
    try:
        ab.find_ci(Path("/nonexistent_k1"), "a/b")
    except ab.BuildError:
        print("PASS missing source refused"); ok += 1
    print(f"{ok}/5 PASS")
    return 0 if ok == 5 else 1


if __name__ == "__main__":
    if not VENV_PY.exists():
        print(f"SKIP test_k1_port_audio_test_build: {VENV_PY} missing (pykotor venv not set up)")
        sys.exit(0)
    if Path(sys.prefix).resolve() != VENV_PY.parent.parent.resolve():
        os.execv(str(VENV_PY), [str(VENV_PY), "-I", str(Path(__file__).resolve()), *sys.argv[1:]])
    sys.exit(main())
