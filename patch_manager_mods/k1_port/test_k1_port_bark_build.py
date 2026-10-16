"""Tests for the pure helpers of k1_port_bark_build (needs the k1port venv; re-execs under it, SKIP if missing)."""
import io
import os
import sys
import wave
from pathlib import Path

HERE = Path(__file__).resolve().parent
VENV_PY = Path(os.environ.get("K1PORT_VENV") or "~/Games/KOTOR2_Mod_Workspace/venvs/k1port").expanduser() / "bin" / "python"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_bark_build as bb  # noqa: E402


def main():
    from pykotor.common.misc import ResRef
    from pykotor.resource.formats.twoda import TwoDA, bytes_2da, read_2da
    from pykotor.resource.generics.utc import UTC, bytes_utc, read_utc
    ok = 0
    w = wave.open(io.BytesIO(bb.tone_wav(0.5)))
    assert (w.getnchannels(), w.getframerate(), w.getsampwidth(), w.getnframes()) == (1, 22050, 2, 11025)
    assert bb.tone_wav(0.1)[:4] == b"RIFF" and bb.tone_wav(0.1)[8:12] == b"WAVE"
    print("PASS tone is RIFF PCM16 mono 22050"); ok += 1

    t = TwoDA()
    for h in ("label", "resref", "strref", "gender", "type"):
        t.add_column(h)
    for i in range(3):
        t.add_row(str(i), {"label": f"L{i}", "resref": f"r{i}", "strref": "5", "gender": "0", "type": ""})
    raw = bytes_2da(t)
    new, row = bb.append_soundset_row(raw)
    t2 = read_2da(new)
    assert row == 3 and t2.get_height() == 4
    assert [t2.get_cell(3, h) for h in t2.get_headers()] == ["kvj_juhani", "kvjssf", "-1", "1", ""]
    assert all(t2.get_cell(i, "label") == f"L{i}" and t2.get_cell(i, "strref") == "5" for i in range(3))
    print("PASS soundset row appended, old rows untouched"); ok += 1

    u = UTC()
    u.conversation, u.soundset_id, u.appearance_id, u.tag = ResRef("204ithr"), 57, 818, "k1p_juhani"
    base = bytes_utc(u)
    a = read_utc(bb.patch_utc(base, conversation="kvjdlg01"))
    b = read_utc(bb.patch_utc(base, soundset=70))
    c = read_utc(bb.patch_utc(base, conversation="kvjdlg01", soundset=70))
    assert (str(a.conversation), a.soundset_id) == ("kvjdlg01", 57)
    assert (str(b.conversation), b.soundset_id) == ("204ithr", 70)
    assert (str(c.conversation), c.soundset_id, c.appearance_id, c.tag) == ("kvjdlg01", 70, 818, "k1p_juhani")
    print("PASS utc variants change only the named fields"); ok += 1
    try:
        bb.find_ci(Path("/nonexistent"), "a")
    except bb.BuildError:
        print("PASS missing source refused"); ok += 1
    print(f"{ok}/4 PASS")
    return 0 if ok == 4 else 1


if __name__ == "__main__":
    if not VENV_PY.exists():
        print(f"SKIP test_k1_port_bark_build: {VENV_PY} missing (pykotor venv not set up)")
        sys.exit(0)
    if Path(sys.prefix).resolve() != VENV_PY.parent.parent.resolve():
        os.execv(str(VENV_PY), [str(VENV_PY), "-I", str(Path(__file__).resolve()), *sys.argv[1:]])
    sys.exit(main())
