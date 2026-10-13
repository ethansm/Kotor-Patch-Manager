#!/usr/bin/env python3
"""Tests for kvoice_container.py. Synthetic MP3 frames only (no ffmpeg, no game files), except one optional
vanilla positive control: it reads the directory named by KVOICE_VANILLA_VO (read-only) and prints SKIP
when that is unset or holds no .wav files.
Run: python3 -u test_kvoice_container.py   (cwd may be anything; exit 0 = all PASS or SKIP)
"""
import glob
import os
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import kvoice_container as kc  # noqa: E402

TESTS = []
BR_TABLE = kc.MP3_HDR_RATE["br"]
SR_TABLE = kc.MP3_HDR_RATE["sr"]


class Skip(Exception):
    """Requirement absent: reported as SKIP, not counted as a pass or a failure."""


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def fake_frame(bri=3, sri=2, mono=True, pad=0):
    """One MPEG-1 L3 frame with a valid header and zero body. Default bri=3 (48 kbps), sri=2 (32 kHz), mono
    gives header ff fb 38 c4, the vanilla value."""
    h = 0xFFFB0000 | (bri << 12) | (sri << 10) | (pad << 9) | ((3 if mono else 0) << 6) | 0x04
    frame_len = 144 * BR_TABLE[bri] * 1000 // SR_TABLE[sri] + pad
    return struct.pack(">I", h) + bytes(frame_len - 4)


VO_PAYLOAD = fake_frame() * 7                     # 7 clean 32 kHz mono 48 kbps frames, 1512 bytes


@test
def t_header_layout():
    h = kc.build_header()
    check(len(h) == 58 == kc.HDR_LEN, "header length %d" % len(h))
    ref = (b"RIFF" + struct.pack("<I", 50) + b"WAVE" + b"fmt " + struct.pack("<I", 18)
           + struct.pack("<HHIIHHH", 1, 1, 22050, 22050, 1, 8, 0) + b"fact" + struct.pack("<II", 4, 0)
           + b"data" + struct.pack("<I", 0))
    check(len(ref) == 58, "reference layout is %d bytes" % len(ref))
    check(h == ref, "build_header() differs from the spec layout: %r" % h)
    check(struct.unpack("<I", h[4:8])[0] == 50, "RIFF size must be the constant 50, not the file size")


@test
def t_frame_header_parse():
    fh = kc.frame_header(fake_frame(3, 2), 0)
    check(fh == {"sr": 32000, "br": 48, "ch": 1, "len": 216}, "48k frame parsed as %r" % (fh,))
    fh = kc.frame_header(fake_frame(9, 0), 0)
    check(fh == {"sr": 44100, "br": 128, "ch": 1, "len": 417}, "128k/44.1k frame parsed as %r" % (fh,))
    fh = kc.frame_header(fake_frame(3, 2, pad=1), 0)
    check(fh["len"] == 217, "padding bit not counted: %r" % (fh,))
    check(kc.frame_header(b"\x00" * 16, 0) is None, "non-sync bytes accepted")
    check(kc.frame_header(b"\xff\xf3\x38\xc4", 0) is None, "MPEG-2 header accepted")
    check(kc.frame_header(b"\xff\xfb\x08\xc4", 0) is None, "free-format bitrate index accepted")
    check(kc.frame_header(b"\xff\xfb\x3c\xc4", 0) is None, "reserved sample-rate index accepted")
    check(kc.frame_header(b"\xff\xfb", 0) is None, "short buffer accepted")


@test
def t_walk_frames():
    n, first, bad = kc.walk_frames(VO_PAYLOAD, 0)
    check((n, bad) == (7, None), "clean chain walked as n=%d bad=%r" % (n, bad))
    check(first["sr"] == 32000 and first["len"] == 216, "first frame %r" % (first,))
    n, _, bad = kc.walk_frames(VO_PAYLOAD + b"\x00\x01", 0)
    check((n, bad) == (7, 7 * 216), "trailing junk: n=%d bad=%r" % (n, bad))
    n, _, bad = kc.walk_frames(VO_PAYLOAD[:-10], 0)
    check(bad == 7 * 216, "truncated last frame: bad=%r" % (bad,))
    n, _, bad = kc.walk_frames(fake_frame() * 3 + fake_frame(9, 0), 0)
    check((n, bad) == (3, 3 * 216), "sample-rate change: n=%d bad=%r" % (n, bad))
    n, _, bad = kc.walk_frames(fake_frame() * 2 + fake_frame(pad=1), 0)
    check((n, bad) == (2, 2 * 216), "frame-length change (padding): n=%d bad=%r" % (n, bad))
    n, first, bad = kc.walk_frames(b"", 0)
    check((n, first, bad) == (0, None, None), "empty input: %r" % ((n, first, bad),))


@test
def t_check_payload():
    check(kc.check_payload(VO_PAYLOAD) == 7, "clean payload frame count")
    for bad_payload, label in ((b"", "empty"), (b"RIFF" + bytes(100), "RIFF bytes"),
                               (fake_frame(9, 0) * 3, "44.1 kHz 128 kbps payload"),
                               (VO_PAYLOAD + b"\x00", "trailing junk")):
        try:
            kc.check_payload(bad_payload)
        except kc.ContainerError:
            continue
        raise AssertionError("check_payload accepted %s" % label)


@test
def t_wrap_strip_roundtrip():
    c = kc.wrap(VO_PAYLOAD)
    check(len(c) == 58 + 216 * 7, "container size %d != 58 + 216*7" % len(c))
    check(c[:58] == kc.build_header(), "wrap() prefix is not the vanilla header")
    check(kc.detect(c), "detect() rejects a container that wrap() just built")
    check(kc.strip(c) == VO_PAYLOAD, "strip(wrap(x)) != x")


@test
def t_wrap_refuses_bad_payload():
    for bad_payload in (b"", b"RIFF" + bytes(200), fake_frame(9, 0) * 3, VO_PAYLOAD + b"\x01"):
        try:
            kc.wrap(bad_payload)
        except kc.ContainerError:
            continue
        raise AssertionError("wrap() accepted a payload it must refuse (len %d)" % len(bad_payload))


@test
def t_strip_refuses_non_containers():
    good = kc.wrap(VO_PAYLOAD)
    size = len(good)
    cases = {
        "bare MP3 (no header)": VO_PAYLOAD,
        "44.1 kHz frames behind a valid header": kc.build_header() + fake_frame(9, 0) * 3,
        "RIFF size set to file size": good[:4] + struct.pack("<I", size - 8) + good[8:],
        "fmt declares 44100 Hz": good[:24] + struct.pack("<I", 44100) + good[28:],
        "trailing junk after last frame": good + b"\x00\x01",
        "truncated container": good[:-5],
        "empty input": b"",
        "RIFF stub only": b"RIFF",
        "one byte past header": kc.build_header() + b"\xff",
    }
    for label, blob in cases.items():
        check(not kc.detect(blob), "detect() accepted: %s" % label)
        try:
            kc.strip(blob)
        except kc.ContainerError:
            continue
        raise AssertionError("strip() accepted: %s" % label)


@test
def t_expected_pcm_samples():
    check(kc.expected_pcm_samples(30) == 32303, "30 frames -> %d samples" % kc.expected_pcm_samples(30))
    check(kc.expected_pcm_samples(1) == -1105, "1 frame formula")
    try:
        kc.expected_pcm_samples(0)
    except ValueError:
        return
    raise AssertionError("expected_pcm_samples(0) did not raise")


@test
def t_vanilla_positive_control():
    """Optional: every vanilla StreamVoice .wav in KVOICE_VANILLA_VO must be detected and stripped."""
    d = os.environ.get("KVOICE_VANILLA_VO", "")
    wavs = sorted(glob.glob(os.path.join(d, "*.wav"))) if d else []
    if not wavs:
        raise Skip("KVOICE_VANILLA_VO unset or holds no .wav (no game files)")
    for path in wavs[:5]:
        with open(path, "rb") as fh:
            raw = fh.read()
        check(raw[:58] == kc.build_header(), "%s: first 58 bytes differ from build_header()" % os.path.basename(path))
        check(kc.detect(raw), "%s: detect() rejects a vanilla clip" % os.path.basename(path))
        payload = kc.strip(raw)
        check(kc.wrap(payload) == raw, "%s: wrap(strip(x)) != x" % os.path.basename(path))


def main() -> int:
    passed, skipped = 0, 0
    for fn in TESTS:
        try:
            fn()
            print("PASS %s" % fn.__name__)
            passed += 1
        except Skip as e:
            print("SKIP %s: %s" % (fn.__name__, e))
            skipped += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print("FAIL %s: %s: %s" % (fn.__name__, type(e).__name__, e))
    total = len(TESTS) - skipped
    print("%d/%d PASS%s" % (passed, total, " (%d skipped)" % skipped if skipped else ""))
    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
