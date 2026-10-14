#!/usr/bin/env python3
"""Tests for kvoice_lip.py. Synthetic sine-tone WAV and in-memory LIP bytes; stdlib only.

The positive control reads loose override/*.lip from a vanilla K2 install (KVOICE_K2_DIR overrides
the default path) and SKIPs (counts as pass) when the install is absent. Symlinks are never followed.
"""
import math
import os
import struct
import sys
import tempfile
import wave
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kvoice_lip as L  # noqa: E402

K2_DIR = Path(os.environ.get(
    "KVOICE_K2_DIR", "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")).expanduser()
TSV = "0.00\tX\n0.10\tA\n0.20\tB\n0.30\tC\n0.40\tD\n0.50\tE\n0.60\tF\n0.70\tG\n0.75\tH\n"
TSV_CUES = [(0.0, 0), (0.1, 11), (0.2, 12), (0.3, 2), (0.4, 3), (0.5, 4), (0.6, 5), (0.7, 8), (0.75, 14)]
EXPECTED_KEYS = TSV_CUES + [(0.8, 0)]  # write_lip appends the rest key at the audio length
TESTS = []


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def f32(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]


def keys_close(got, want):
    return len(got) == len(want) and all(abs(t - et) < 1e-6 and s == es for (t, s), (et, es) in zip(got, want))


def write_sine_wav(path, seconds=0.8, rate=8000, hz=440.0):
    """Mono 16-bit sine tone. Returns its length in seconds, read back from the file."""
    frames = int(round(seconds * rate))
    samples = b"".join(struct.pack("<h", int(12000 * math.sin(2 * math.pi * hz * i / rate)))
                       for i in range(frames))
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(samples)
    with wave.open(str(path), "rb") as w:
        return w.getnframes() / float(w.getframerate())


def raw_lip(length, keys, sig=L.SIG):
    """LIP bytes built without validation, for the negative cases."""
    out = bytearray(sig) + struct.pack("<fI", length, len(keys))
    for t, s in keys:
        out += struct.pack("<fB", t, s)
    return bytes(out)


@test
def test_round_trip():
    with tempfile.TemporaryDirectory() as tmp:
        wav = Path(tmp) / "tone.wav"
        length = write_sine_wav(wav)
        check(abs(length - 0.8) < 1e-9, "wav length %r != 0.8" % length)
        cues = L.rhubarb_to_lip(TSV)
        check(cues == TSV_CUES, "rhubarb mapping %r" % cues)
        blob = L.write_lip(cues, length)
        out = Path(tmp) / "tone.lip"
        out.write_bytes(blob)
        data = out.read_bytes()
    check(data == blob, "file bytes differ from write_lip output")
    problems = L.check_lip(data)
    check(problems == [], "check_lip: %s" % problems)
    got_len, keys = L.parse_lip(data)
    check(got_len == f32(length), "header length %r != %r" % (got_len, f32(length)))
    check(len(data) == 16 + 5 * len(EXPECTED_KEYS), "size %d" % len(data))
    check(keys_close(keys, EXPECTED_KEYS), "keys %r != %r" % (keys, EXPECTED_KEYS))
    # Normalisation: a late first cue gets a rest key at 0; an early last cue gets one at length.
    got = L.parse_lip(L.write_lip([(0.1, 3)], 0.5))[1]
    check(keys_close(got, [(0.0, 0), (0.1, 3), (0.5, 0)]), "normalised keys %r" % got)
    check(len(L.write_lip([], 0.5)) > 0, "empty cue list produced zero bytes")


@test
def test_neg_zero_byte():
    problems = L.check_lip(b"")
    check(problems and "zero-byte" in problems[0], "zero-byte problems %r" % problems)


@test
def test_neg_bad_signature():
    data = raw_lip(0.8, [(0.0, 0), (0.8, 0)], sig=b"LIP V2.0")
    problems = L.check_lip(data)
    check(any("signature" in p for p in problems), "bad-signature problems %r" % problems)


@test
def test_neg_non_monotonic():
    data = raw_lip(0.8, [(0.0, 0), (0.5, 3), (0.3, 1), (0.8, 0)])
    problems = L.check_lip(data)
    check(any("monotonic" in p for p in problems), "non-monotonic problems %r" % problems)


@test
def test_neg_shape_out_of_range():
    data = raw_lip(0.8, [(0.0, 0), (0.4, 16), (0.8, 0)])
    problems = L.check_lip(data)
    check(any("shape" in p for p in problems), "shape-16 problems %r" % problems)
    try:
        L.write_lip([(0.0, 16)], 0.8)
    except ValueError:
        pass
    else:
        raise AssertionError("write_lip accepted shape 16")


@test
def test_vanilla_lip_positive_control():
    if K2_DIR.is_symlink() or not K2_DIR.is_dir():
        raise Skip("K2 install absent at %s" % K2_DIR)
    override = K2_DIR / "override"
    if override.is_symlink() or not override.is_dir():
        raise Skip("no real override/ dir under %s" % K2_DIR)
    paths = []
    with os.scandir(override) as it:
        for entry in it:
            # kpv* are the doc 41 E-matrix probe clips; kpve3c.lip is invalid on purpose (R7) and may be staged live
            if entry.name.lower().endswith(".lip") and not entry.name.lower().startswith("kpv") \
                    and entry.is_file(follow_symlinks=False):
                paths.append(entry.path)
    if not paths:
        raise Skip("no loose .lip files in %s" % override)
    checked, failures = 0, []
    for p in sorted(paths):
        data = Path(p).read_bytes()
        if not data:
            continue  # zero-byte placeholders are absent, not invalid
        checked += 1
        problems = L.check_lip(data)
        if problems:
            failures.append("%s: %s" % (Path(p).name, problems[0]))
    check(checked > 0, "no non-empty .lip files to check")
    check(not failures, "%d of %d real .lip failed: %s" % (len(failures), checked, failures[:3]))


def main():
    passed = 0
    for fn in TESTS:
        try:
            fn()
        except Skip as exc:
            print(f"SKIP {fn.__name__}: {exc}")
            passed += 1
        except Exception as exc:  # AssertionError or anything unexpected
            print(f"FAIL {fn.__name__}: {exc}")
        else:
            print(f"PASS {fn.__name__}")
            passed += 1
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
