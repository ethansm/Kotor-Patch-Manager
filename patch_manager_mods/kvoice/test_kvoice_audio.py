#!/usr/bin/env python3
"""Tests for kvoice_audio.py. Synthetic 1 kHz tones only; ffmpeg (libmp3lame) required, no game files.
Temp files only; nothing is written next to the code.
Run: python3 -u test_kvoice_audio.py   (cwd may be anything; exit 0 = all PASS)
"""
import math
import os
import struct
import subprocess
import sys
import tempfile
import wave
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import kvoice_audio as ka  # noqa: E402
import kvoice_container as kc  # noqa: E402

TESTS = []


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def sine_pcm(seconds: float, amp: float, freq: float = 1000.0) -> bytes:
    """s16le mono 32 kHz sine. amp is a fraction of full scale."""
    n = int(round(seconds * kc.SR))
    return b"".join(struct.pack("<h", int(round(amp * 32767 * math.sin(2 * math.pi * freq * i / kc.SR))))
                    for i in range(n))


@test
def t_ffmpeg_has_libmp3lame():
    path = ka.ffmpeg_path()
    check(os.path.isfile(path), "ffmpeg_path() returned a missing file: %s" % path)
    enc = subprocess.run([path, "-hide_banner", "-encoders"], capture_output=True, text=True)
    check(" libmp3lame " in enc.stdout, "ffmpeg at %s has no libmp3lame encoder" % path)


@test
def t_write_pcm_wav_roundtrip():
    pcm = sine_pcm(0.05, 0.1)
    with tempfile.TemporaryDirectory(prefix="kvoice_audio_test_") as td:
        p = os.path.join(td, "x.wav")
        ka.write_pcm_wav(pcm, p)
        with wave.open(p, "rb") as w:
            check((w.getnchannels(), w.getsampwidth(), w.getframerate()) == (1, 2, 32000), "wav params")
            check(w.readframes(w.getnframes()) == pcm, "wav payload differs from input PCM")
    try:
        ka.write_pcm_wav(b"\x00" * 3, os.path.join(td, "odd.wav"))
    except ka.AudioError:
        return
    raise AssertionError("odd PCM byte count accepted")


@test
def t_encode_produces_clean_container_payload():
    pcm = sine_pcm(1.0, 0.1)
    payload = ka.encode_mp3(pcm, 0.0)
    n = kc.check_payload(payload)
    check(n > 0, "no frames")
    check(len(kc.wrap(payload)) == 58 + 216 * n, "wrapped size != 58 + 216*frames")
    check(kc.strip(kc.wrap(payload)) == payload, "strip(wrap(payload)) != payload")


@test
def t_decode_length_matches_gapless_formula():
    payload = ka.encode_mp3(sine_pcm(1.0, 0.1), 0.0)
    n = kc.check_payload(payload)
    dec = ka.decode_pcm(payload)
    check(len(dec) % 2 == 0, "decoded PCM has odd byte count")
    check(len(dec) // 2 == kc.expected_pcm_samples(n),
          "decoded %d samples, formula %d for %d frames" % (len(dec) // 2, kc.expected_pcm_samples(n), n))
    check(abs(len(dec) // 2 - 32000) <= 1152, "decoded duration %d samples is not ~1.0 s" % (len(dec) // 2))


@test
def t_decode_refuses_garbage():
    try:
        ka.decode_pcm(b"\x12" * 4000)
    except ka.AudioError:
        return
    raise AssertionError("decode_pcm accepted garbage input")


@test
def t_measure_true_peak_of_sine():
    with tempfile.TemporaryDirectory(prefix="kvoice_audio_test_") as td:
        p = os.path.join(td, "tone.wav")
        ka.write_pcm_wav(sine_pcm(1.0, 0.1), p)
        I, TP = ka.measure(p)
    # amp 0.1 of full scale is -20.0 dBFS peak; true peak must sit on that within 0.3 dB
    check(abs(TP - (-20.0)) <= 0.3, "true peak %.2f dBTP, want about -20.0" % TP)
    check(-30.0 < I < -15.0, "integrated loudness %.2f LUFS out of range for a -20 dBFS tone" % I)


@test
def t_gain_moves_loudness_and_peak():
    pcm = sine_pcm(1.0, 0.1)
    with tempfile.TemporaryDirectory(prefix="kvoice_audio_test_") as td:
        base = os.path.join(td, "g0.mp3")
        cut = os.path.join(td, "g6.mp3")
        open(base, "wb").write(ka.encode_mp3(pcm, 0.0))
        open(cut, "wb").write(ka.encode_mp3(pcm, -6.0))
        I0, TP0 = ka.measure(base, "mp3")
        I6, TP6 = ka.measure(cut, "mp3")
    check(abs((I0 - I6) - 6.0) <= 0.1, "-6 dB gain moved loudness by %.2f LU" % (I0 - I6))
    check(abs((TP0 - TP6) - 6.0) <= 0.2, "-6 dB gain moved true peak by %.2f dB" % (TP0 - TP6))


@test
def t_encode_file_refuses_missing_input():
    with tempfile.TemporaryDirectory(prefix="kvoice_audio_test_") as td:
        try:
            ka.encode_file(os.path.join(td, "nope.wav"), os.path.join(td, "out.mp3"), 0.0)
        except ka.AudioError:
            return
    raise AssertionError("encode_file accepted a missing input")


def main() -> int:
    passed = 0
    for fn in TESTS:
        try:
            fn()
            print("PASS %s" % fn.__name__)
            passed += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print("FAIL %s: %s: %s" % (fn.__name__, type(e).__name__, e))
    print("%d/%d PASS" % (passed, len(TESTS)))
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
