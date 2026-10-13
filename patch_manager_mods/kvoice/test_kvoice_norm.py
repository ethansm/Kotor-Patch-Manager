#!/usr/bin/env python3
"""Tests for kvoice_norm.py. Synthetic tones and impulses only; ffmpeg required, no game files.
Temp files only; nothing is written next to the code.
Run: python3 -u test_kvoice_norm.py   (cwd may be anything; exit 0 = all PASS)
"""
import math
import os
import struct
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import kvoice_audio as ka  # noqa: E402
import kvoice_container as kc  # noqa: E402
import kvoice_norm as kn  # noqa: E402

TESTS = []


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def sine_pcm(seconds: float, amp: float, freq: float = 1000.0) -> bytes:
    n = int(round(seconds * kc.SR))
    return b"".join(struct.pack("<h", int(round(amp * 32767 * math.sin(2 * math.pi * freq * i / kc.SR))))
                    for i in range(n))


def independent_measure(payload: bytes):
    """Measure a payload from its own file, separate from the normaliser's internal checks."""
    with tempfile.TemporaryDirectory(prefix="kvoice_norm_test_") as td:
        p = os.path.join(td, "p.mp3")
        open(p, "wb").write(payload)
        return ka.measure(p, "mp3")


@test
def t_gain_for_math():
    g, lim = kn.gain_for(-20.0, -6.0)
    check(abs(g - 1.5) < 1e-9 and not lim, "unlimited gain %r" % ((g, lim),))
    g, lim = kn.gain_for(-20.0, -2.4)          # +1.5 dB would put TP at -0.9, over the -1.0 ceiling
    check(abs(g - 1.4) < 1e-9 and lim, "TP-limited gain %r" % ((g, lim),))
    g, lim = kn.gain_for(-14.0, 0.1)           # loud clip: cut 4.5 dB, TP lands at -4.4
    check(abs(g + 4.5) < 1e-9 and not lim, "loud clip gain %r" % ((g, lim),))
    g, lim = kn.gain_for(-18.5, -1.0)          # already on target and on the ceiling
    check(abs(g) < 1e-9 and not lim, "on-target gain %r" % ((g, lim),))


def _converges(amp: float, label: str):
    payload, rep = kn.normalize_encode(sine_pcm(1.0, amp))
    I, TP = independent_measure(payload)
    check(abs(I - kn.TARGET_LUFS) <= kn.TOL_LU, "%s: I=%.2f LUFS, want %.1f +/- %.1f" % (label, I, kn.TARGET_LUFS, kn.TOL_LU))
    check(TP <= kn.TP_CEIL_DBTP, "%s: TP=%.2f dBTP over the %.1f ceiling" % (label, TP, kn.TP_CEIL_DBTP))
    check(1 <= rep["passes"] <= kn.MAX_PASSES, "%s: passes=%r" % (label, rep["passes"]))
    check(kc.detect(kc.wrap(payload)), "%s: wrap() of normalised payload is not a clean container" % label)
    return rep


@test
def t_quiet_tone_hits_target():
    _converges(0.02, "quiet tone")


@test
def t_loud_tone_hits_target():
    _converges(0.5, "loud tone")


@test
def t_lead_in_silence_at_least_120_ms():
    payload, _ = kn.normalize_encode(sine_pcm(1.0, 0.1))
    d = ka.decode_pcm(payload)
    samples = struct.unpack("<%dh" % (len(d) // 2), d)
    first = next(i for i, v in enumerate(samples) if abs(v) > 100)   # -50 dBFS counts as sound
    check(first >= int(0.120 * kc.SR), "leading silence %.1f ms, want >= 120 ms" % (first * 1000.0 / kc.SR))
    expect = (kn.LEAD_IN_S + 1.0) * kc.SR
    check(abs(len(samples) - expect) <= 1152, "decoded %d samples, want about %d" % (len(samples), expect))


@test
def t_silent_clip_refused():
    try:
        kn.normalize_encode(b"\x00" * 2 * kc.SR)
    except kn.SilentClipError:
        return
    raise AssertionError("all-zero clip was not refused as silent")


@test
def t_near_silent_clip_refused():
    try:
        kn.normalize_encode(sine_pcm(1.0, 0.0005))     # about -69 dBFS, under the -60 LUFS floor
    except kn.SilentClipError:
        return
    raise AssertionError("near-silent clip was not refused")


@test
def t_short_clip_refused():
    try:
        kn.normalize_encode(sine_pcm(0.2, 0.1))
    except kn.SilentClipError:
        raise AssertionError("short clip reported as silent, want a plain NormalizeError")
    except kn.NormalizeError:
        return
    raise AssertionError("0.2 s clip accepted")


@test
def t_odd_pcm_refused():
    try:
        kn.normalize_encode(b"\x01")
    except kn.NormalizeError:
        return
    raise AssertionError("odd PCM byte count accepted")


@test
def t_true_peak_ceiling_refuses_unreachable_target():
    """Quiet tone plus one full-scale sample: the peak forces gain far below the target, so refuse."""
    pcm = bytearray(sine_pcm(1.0, 0.01))
    struct.pack_into("<h", pcm, 2 * 16000, 32440)
    try:
        kn.normalize_encode(bytes(pcm))
    except kn.SilentClipError:
        raise AssertionError("peaky clip reported as silent")
    except kn.NormalizeError as e:
        check("target not met" in str(e), "unexpected message: %s" % e)
        return
    raise AssertionError("peaky clip normalised despite the true-peak ceiling")


@test
def t_peaky_clip_accepted_quieter_and_flagged():
    """Real vanilla VO is peaky (-19.9 LUFS / -0.5 dBTP): the ceiling wins, the clip lands under target, tp_limited."""
    n = 2 * 32000
    s = [int(0.12 * 32767 * math.sin(2 * math.pi * 440 * i / 32000)) for i in range(n)]
    for k in range(4000, n, 8000):
        s[k] = int(0.97 * 32767)
    payload, rep = kn.normalize_encode(struct.pack("<%dh" % n, *s))
    I, TP = independent_measure(payload)
    check(rep["tp_limited"], "report should flag tp_limited: %r" % rep)
    check(TP <= kn.TP_CEIL_DBTP, "true peak %.2f over the ceiling" % TP)
    check(kn.TARGET_LUFS - kn.MAX_TP_SHORTFALL_LU <= I < kn.TARGET_LUFS - kn.TOL_LU, "loudness %.2f" % I)


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
