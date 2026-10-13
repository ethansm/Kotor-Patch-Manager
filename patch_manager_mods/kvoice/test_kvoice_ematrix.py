#!/usr/bin/env python3
"""Tests for kvoice_ematrix (doc 41 Task 1, E-matrix builder). Synthetic tone only; never writes a game dir."""
import atexit
import hashlib
import json
import os
import shutil
import sys
import tempfile
import wave
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kvoice_audio as ka  # noqa: E402
import kvoice_container as kc  # noqa: E402
import kvoice_lip as kl  # noqa: E402
import kvoice_pack as kp  # noqa: E402
import kvoice_verify as kv  # noqa: E402
import kvoice_ematrix as em  # noqa: E402

DEFAULT_NAMES = [
    "kpve1a.wav", "kpve2a.wav", "kpve2b.wav", "kpve2c.mp3",
    "kpve3a.wav", "kpve3b.wav", "kpve3b.lip", "kpve3c.wav", "kpve3c.lip",
    "KpvE8Mixed.wav", "kpve8_sixteenchr.wav", "kpve8_seventeench.wav",
]
SFX_EXTRA = "kpve2d.wav"
BAD_NAMES = {"KpvE8Mixed.wav", "kpve8_seventeench.wav"}

_CACHE = {}


def _tmpdir():
    tmp = tempfile.mkdtemp(prefix="kvoice_ematrix_test_")
    atexit.register(shutil.rmtree, tmp, True)
    return tmp


def _fake_source(tmp):
    """Synthetic stand-in for a vanilla SFX file (not vanilla bytes): RIFF header then a fixed pattern."""
    path = os.path.join(tmp, "fake_sfx.wav")
    data = b"RIFF" + b"\x00" * 4 + b"WAVE" + bytes(i % 251 for i in range(1000))
    Path(path).write_bytes(data)
    return path, data


def _shared(key, with_sfx=False):
    """Build once per key (ffmpeg encodes are slow). Returns (root, manifest, source_bytes_or_None)."""
    if key not in _CACHE:
        tmp = _tmpdir()
        out = os.path.join(tmp, "stage")
        sfx, src = _fake_source(tmp) if with_sfx else (None, None)
        manifest = em.build_ematrix(out, sfx)
        _CACHE[key] = (Path(out), manifest, src)
    return _CACHE[key]


def _ids(data):
    return {p.split(":", 1)[0] for p in kv.verify_wav(data)}


def test_layout_and_manifest_sha256():
    root, man, _ = _shared("default")
    assert sorted(p.name for p in (root / "Override").iterdir()) == sorted(DEFAULT_NAMES)
    assert sorted(os.listdir(root)) == ["Override", "manifest.json"], os.listdir(root)
    assert man["schema"] == 1
    assert set(man["files"]) == {"Override/" + n for n in DEFAULT_NAMES}
    for key, sha in man["files"].items():
        assert hashlib.sha256((root / key).read_bytes()).hexdigest() == sha, key
    on_disk = json.loads((root / "manifest.json").read_text(encoding="utf-8"))
    assert on_disk == man


def test_e1_pcm_wav_22050_mono_minus17_lufs():
    root, _, _ = _shared("default")
    path = root / "Override" / "kpve1a.wav"
    with wave.open(str(path), "rb") as w:
        assert (w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()) == (1, 2, 22050, 44100)
    lufs, _ = ka.measure(str(path))
    assert abs(lufs - (-17.0)) <= 0.5, lufs
    assert "R1" not in _ids(path.read_bytes())


def test_e2_container_and_bare_mp3():
    root, man, _ = _shared("default")
    ov = root / "Override"
    b = (ov / "kpve2b.wav").read_bytes()
    c = (ov / "kpve2c.mp3").read_bytes()
    assert b[:kc.HDR_LEN] == kc.build_header()
    assert b[kc.HDR_LEN:] == c
    assert c[:2] == b"\xff\xfb"
    assert _ids(b) == set(), kv.verify_wav(b)
    assert _ids(c) == {"R1"}, kv.verify_wav(c)
    assert man["entries"]["Override/kpve2c.mp3"]["expected_rules"] == ["R1"]
    assert man["entries"]["Override/kpve2b.wav"]["expected_rules"] == []
    lufs, _ = ka.measure(str(ov / "kpve2c.mp3"), input_format="mp3")
    assert abs(lufs - (-17.0)) <= 1.0, lufs


def test_e2d_prefix_copy_and_skip_note():
    root, man, _ = _shared("default")
    assert not (root / "Override" / SFX_EXTRA).exists()
    assert "Override/kpve2d.wav" in man["skipped"]
    root2, man2, src = _shared("sfx", with_sfx=True)
    d = (root2 / "Override" / SFX_EXTRA).read_bytes()
    assert d[:em.PREFIX_LEN] == src[:em.PREFIX_LEN]
    assert d[em.PREFIX_LEN:em.PREFIX_LEN + 4] == b"RIFF"
    assert "R2" in _ids(d), kv.verify_wav(d)
    assert "Override/kpve2d.wav" not in man2["skipped"]
    assert man2["entries"]["Override/kpve2d.wav"]["expected_rules"] == sorted(_ids(d))


def test_e2d_short_source_rejected_before_writes():
    tmp = _tmpdir()
    short = os.path.join(tmp, "short.wav")
    Path(short).write_bytes(b"RIFF" + b"\x00" * 99)
    stage = os.path.join(tmp, "stage")
    try:
        em.build_ematrix(stage, short)
    except ValueError as exc:
        assert "shorter" in str(exc), exc
    else:
        raise AssertionError("short sfx source was accepted")
    assert list((Path(stage) / "Override").iterdir()) == []
    assert not os.path.exists(os.path.join(stage, "manifest.json"))


def test_e3_lip_valid_and_late_first_key():
    root, man, _ = _shared("default")
    ov = root / "Override"
    ok = (ov / "kpve3b.lip").read_bytes()
    assert kl.check_lip(ok) == []
    length, keys = kl.parse_lip(ok)
    assert length == 2.0
    assert [t for t, _ in keys] == [0.0, 0.5, 1.0, 2.0]
    assert [s for _, s in keys] == [em.SHAPE_REST, em.SHAPE_AH, em.SHAPE_OH, em.SHAPE_REST]
    late = (ov / "kpve3c.lip").read_bytes()
    probs = kl.check_lip(late)
    assert any(p.startswith("first key time") for p in probs), probs
    assert man["entries"]["Override/kpve3c.lip"]["expected_rules"] == ["R7"]
    assert man["entries"]["Override/kpve3b.lip"]["expected_rules"] == []
    assert (ov / "kpve3a.wav").exists() and not (ov / "kpve3a.lip").exists()


def test_e8_resref_lengths_and_case():
    assert len("kpve8_sixteenchr") == 16 and len("kpve8_seventeench") == 17
    _, man, _ = _shared("default")
    entries = man["entries"]
    assert entries["Override/kpve8_sixteenchr.wav"]["resref_ok"] is True
    assert entries["Override/kpve8_seventeench.wav"]["resref_ok"] is False
    assert entries["Override/KpvE8Mixed.wav"]["resref_ok"] is False
    for name in DEFAULT_NAMES:
        if name not in BAD_NAMES:
            assert entries["Override/" + name]["resref_ok"] is True, name


def test_refuses_game_dir_before_any_write():
    tmp = _tmpdir()
    stage = os.path.join(tmp, "stage")
    with mock.patch.object(kp, "refuse_game_dir", side_effect=kp.PackError("game dir")) as guard:
        try:
            em.build_ematrix(stage)
        except kp.PackError:
            pass
        else:
            raise AssertionError("game-dir guard did not refuse")
    guard.assert_called_once_with(stage)
    assert not os.path.exists(os.path.join(stage, "Override"))
    assert not os.path.exists(os.path.join(stage, "manifest.json"))


def test_cli_main_builds_and_bad_source_returns_1():
    tmp = _tmpdir()
    stage = os.path.join(tmp, "cli")
    assert em.main(["--out", stage]) == 0
    assert os.path.isfile(os.path.join(stage, "manifest.json"))
    assert em.main(["--out", os.path.join(tmp, "cli2"), "--sfx-prefix-from", os.path.join(tmp, "missing.wav")]) == 1


TESTS = [
    test_layout_and_manifest_sha256,
    test_e1_pcm_wav_22050_mono_minus17_lufs,
    test_e2_container_and_bare_mp3,
    test_e2d_prefix_copy_and_skip_note,
    test_e2d_short_source_rejected_before_writes,
    test_e3_lip_valid_and_late_first_key,
    test_e8_resref_lengths_and_case,
    test_refuses_game_dir_before_any_write,
    test_cli_main_builds_and_bad_source_returns_1,
]


def main():
    passed = 0
    for fn in TESTS:
        try:
            fn()
        except Exception as exc:  # AssertionError or anything unexpected
            print("FAIL %s: %s: %s" % (fn.__name__, type(exc).__name__, exc))
        else:
            print("PASS %s" % fn.__name__)
            passed += 1
    print("%d/%d PASS" % (passed, len(TESTS)))
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
