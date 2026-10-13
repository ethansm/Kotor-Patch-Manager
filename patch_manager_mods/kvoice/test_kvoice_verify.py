#!/usr/bin/env python3
"""Tests for kvoice_verify.py: rules R1-R9 (one negative case each), a synthetic positive, and the vanilla control.

Synthetic sine-tone clips built with kvoice_norm and kvoice_container in tempdirs. Nothing is written next to
the code or into the game dir. The vanilla control copies up to three real StreamVoice .wav files into a
tempdir (regular files only; symlinks are never followed) and SKIPs when the install is absent.
Needs ffmpeg. Run: python3 -u test_kvoice_verify.py   (exit 0 = all PASS or SKIP)
"""
import json
import math
import os
import shutil
import struct
import sys
import tempfile
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kvoice_container as kc  # noqa: E402
import kvoice_lip as L  # noqa: E402
import kvoice_ledger as KL  # noqa: E402
import kvoice_norm as kn  # noqa: E402
import kvoice_pack as kp  # noqa: E402
import kvoice_verify as V  # noqa: E402

K2_DIR = Path(os.environ.get(
    "KVOICE_K2_DIR", "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")).expanduser()
RESREF = "kpvtest1"
TESTS = []
_CACHE = {}


class Skip(Exception):
    """Environment lacks the vanilla install; counted as a pass."""


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def has(problems, rule, needle=""):
    return any(p.startswith(rule + ":") and needle in p for p in problems)


def good_mp3():
    """Bare 32 kHz mono 48 kbps MP3 of a 1 s sine tone, loudness-normalised by kvoice_norm."""
    if "mp3" not in _CACHE:
        n = int(1.0 * kc.SR)
        pcm = b"".join(struct.pack("<h", int(0.3 * 32767 * math.sin(2 * math.pi * 440.0 * i / kc.SR)))
                       for i in range(n))
        _CACHE["mp3"], _report = kn.normalize_encode(pcm)
    return _CACHE["mp3"]


def good_wav():
    return kc.wrap(good_mp3())


def duration_s(mp3=None):
    return kc.expected_pcm_samples(kc.check_payload(mp3 or good_mp3())) / kc.SR


def lip_for(length=None, mp3=None):
    dur = duration_s(mp3) if length is None else length
    return L.write_lip([(0.0, 0), (0.1, 11), (0.2, 12)], dur)


def build_stage(td, resref=RESREF, mp3=None, lip=None, container="vo58"):
    """Pack one clip with pack_clip and write a stage ledger with a non-vanilla row. Returns the stage Path."""
    stage = Path(td) / "stage"
    mp3 = mp3 or good_mp3()
    lip = lip or lip_for(mp3=mp3)
    out = kp.pack_clip(stage, resref, mp3, lip, container=container)
    rows = []
    for key in ("wav", "lip"):
        rows.append({"file": out[key], "kind": "audio", "source_id": "synthetic_test",
                     "sha256": kp.sha256_bytes(Path(out[key]).read_bytes()), "local_only": False})
    kp.write_ledger(stage, {"schema": 1, "entries": rows, "approvals": {}})
    return stage


def load_manifest(stage):
    return json.loads((Path(stage) / "manifest.json").read_text(encoding="utf-8"))


def save_manifest(stage, obj):
    (Path(stage) / "manifest.json").write_text(json.dumps(obj), encoding="utf-8")


# ---------------------------------------------------------------- positive and R1-R6 on one wav

@test
def test_positive_synthetic_wav_has_no_problems():
    probs = V.verify_wav(good_wav())
    check(probs == [], "synthetic VO clip should pass R1-R6, got %r" % probs)


@test
def test_r1_bare_mp3_is_flagged_as_r1_not_r3():
    probs = V.verify_wav(good_mp3())
    check(probs and all(p.startswith("R1:") for p in probs), "bare MP3 should be R1 only, got %r" % probs)
    check(not has(probs, "R3"), "bare MP3 must not be reported as R3, got %r" % probs)


@test
def test_r1_empty_input_is_flagged():
    check(has(V.verify_wav(b""), "R1"), "empty input should fail R1")


@test
def test_r2_second_riff_at_0x1d6():
    wav = bytearray(good_wav())
    wav[0x1D6:0x1DA] = b"RIFF"
    check(has(V.verify_wav(bytes(wav)), "R2"), "RIFF at 0x1D6 should fail R2")


@test
def test_r3_header_field_changed():
    wav = bytearray(good_wav())
    wav[24:28] = struct.pack("<I", 44100)  # declared sample rate field
    probs = V.verify_wav(bytes(wav))
    check(has(probs, "R3"), "changed header byte should fail R3, got %r" % probs)
    check(not has(probs, "R1"), "R1 must pass when only the header differs")


@test
def test_r4_truncated_frame_chain():
    wav = good_wav()
    probs = V.verify_wav(wav[:-100])
    check(has(probs, "R4", "breaks"), "truncated last frame should fail R4 (chain breaks), got %r" % probs)


@test
def test_r4_no_frames_after_header():
    probs = V.verify_wav(kc.build_header())
    check(has(probs, "R4", "no MPEG-1 L3 frame"), "header only should fail R4, got %r" % probs)


@test
def test_r5_size_formula_broken_by_trailing_bytes():
    probs = V.verify_wav(good_wav() + b"\x00" * 7)
    check(has(probs, "R5"), "trailing bytes should fail R5, got %r" % probs)
    check(has(probs, "R4"), "trailing bytes also break the chain (R4)")


@test
def test_r6_undecodable_payload():
    probs = V.verify_wav(kc.build_header() + b"\x00" * 200)
    check(has(probs, "R6", "ffmpeg decode failed"), "undecodable payload should fail R6, got %r" % probs)
    probs_empty = V.verify_wav(kc.build_header())
    check(has(probs_empty, "R6", "no payload"), "empty payload should fail R6, got %r" % probs_empty)


@test
def test_vanilla_positive_control_r1_to_r6():
    src_dir = K2_DIR / "StreamVoice"
    if not src_dir.is_dir():
        raise Skip("vanilla StreamVoice dir absent: %s" % src_dir)
    picks = []
    with os.scandir(src_dir) as it:
        for ent in sorted(it, key=lambda e: e.name):
            if ent.name.endswith(".wav") and not ent.is_symlink() and ent.is_file(follow_symlinks=False):
                picks.append(ent.name)
            if len(picks) == 3:
                break
    if not picks:
        raise Skip("no regular .wav in %s" % src_dir)
    with tempfile.TemporaryDirectory(prefix="kvoice_vanilla_") as td:
        for name in picks:
            copy = Path(td) / name
            shutil.copyfile(src_dir / name, copy)  # regular source file, copied into the tempdir
            probs = V.verify_wav(copy.read_bytes())
            check(probs == [], "vanilla %s should pass R1-R6, got %r" % (name, probs))


# ---------------------------------------------------------------- stage rules R7-R9

@test
def test_positive_stage_passes_shippable_and_plain():
    with tempfile.TemporaryDirectory(prefix="kvoice_vstage_") as td:
        stage = build_stage(td)
        check(V.verify_stage(stage) == [], "clean stage should pass, got %r" % V.verify_stage(stage))
        check(V.verify_stage(stage, shippable=True) == [], "clean stage should pass shippable")


@test
def test_stage_bare_container_is_r1_not_r3():
    with tempfile.TemporaryDirectory(prefix="kvoice_vbare_") as td:
        stage = build_stage(td, container="bare", mp3=good_mp3())
        probs = V.verify_stage(stage)
        check(has(probs, "R1", RESREF + ".wav"), "bare stage clip should fail R1, got %r" % probs)
        check(not has(probs, "R3"), "bare stage clip must not be reported as R3")


@test
def test_r7_lip_first_key_not_zero():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr7_") as td:
        stage = build_stage(td)
        dur = duration_s()
        bad = L.SIG + struct.pack("<fI", dur, 2) + struct.pack("<fB", 0.1, 0) + struct.pack("<fB", dur, 0)
        (stage / "Override" / (RESREF + ".lip")).write_bytes(bad)
        probs = V.verify_stage(stage)
        check(has(probs, "R7", "first key"), "lip with first key 0.1 should fail R7, got %r" % probs)


@test
def test_r8_lip_length_off_by_100_ms():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr8_") as td:
        stage = build_stage(td, lip=lip_for(length=duration_s() + 0.100))
        probs = V.verify_stage(stage)
        check(has(probs, "R8"), "lip 100 ms long should fail R8, got %r" % probs)
        check(not has(probs, "R7"), "a valid lip of the wrong length must not fail R7")


@test
def test_r8_lip_length_within_36_ms_passes():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr8ok_") as td:
        stage = build_stage(td, lip=lip_for(length=duration_s() + 0.030))
        probs = V.verify_stage(stage)
        check(not has(probs, "R8"), "lip 30 ms long is inside the 36 ms tolerance, got %r" % probs)


@test
def test_r9_resref_not_kpv():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr9a_") as td:
        stage = build_stage(td)
        ov = stage / "Override"
        os.rename(ov / (RESREF + ".wav"), ov / "abc1.wav")
        os.rename(ov / (RESREF + ".lip"), ov / "abc1.lip")
        probs = V.verify_stage(stage)
        check(has(probs, "R9", "kpv-prefixed"), "non-kpv resref should fail R9, got %r" % probs)


@test
def test_r9_wav_without_lip():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr9b_") as td:
        stage = build_stage(td)
        os.remove(stage / "Override" / (RESREF + ".lip"))
        probs = V.verify_stage(stage)
        check(has(probs, "R9", "has no paired .lip"), "unpaired wav should fail R9, got %r" % probs)


@test
def test_r9_manifest_sha_mismatch():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr9c_") as td:
        stage = build_stage(td)
        m = load_manifest(stage)
        m["files"]["Override/" + RESREF + ".wav"] = "00" * 32
        save_manifest(stage, m)
        probs = V.verify_stage(stage)
        check(has(probs, "R9", "sha256 differs"), "wrong manifest sha should fail R9, got %r" % probs)


@test
def test_r9_manifest_missing():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr9d_") as td:
        stage = build_stage(td)
        os.remove(stage / "manifest.json")
        probs = V.verify_stage(stage)
        check(has(probs, "R9", "manifest.json missing"), "missing manifest should fail R9, got %r" % probs)


@test
def test_r9_no_ledger_row_per_file():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr9e_") as td:
        stage = build_stage(td)
        kp.write_ledger(stage, {"schema": 1, "entries": [], "approvals": {}})
        probs = V.verify_stage(stage)
        check(has(probs, "R9", "has no ledger.json row"), "no ledger rows should fail R9, got %r" % probs)


@test
def test_r9_local_only_blocks_shippable_only():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr9f_") as td:
        stage = build_stage(td)
        # Rebuild the ledger through row_for_clip with a vanilla source: that sets local_only.
        led = KL.ks.Ledger(path=Path(td) / "source_ledger.json")
        for key in ("wav", "lip"):
            KL.row_for_clip(led, stage / "Override" / (RESREF + (".wav" if key == "wav" else ".lip")),
                            container="vo58", source_resref=RESREF, slot="voice", ship_mode="personal",
                            source_id="vanilla_k2")
        kp.write_ledger(stage, led)
        check(V.verify_stage(stage) == [], "local_only is fine in a non-shippable stage, got %r"
              % V.verify_stage(stage))
        probs = V.verify_stage(stage, shippable=True)
        check(has(probs, "R9", "local_only"), "local_only row should fail a shippable stage, got %r" % probs)


@test
def test_r9_symlink_in_override_is_not_followed():
    with tempfile.TemporaryDirectory(prefix="kvoice_vr9g_") as td:
        stage = build_stage(td)
        outside = Path(td) / "outside.wav"
        outside.write_bytes(good_wav())
        os.symlink(outside, stage / "Override" / "kpvlink1.wav")
        probs = V.verify_stage(stage)
        check(has(probs, "R9", "kpvlink1.wav"), "symlink in Override should fail R9, got %r" % probs)


@test
def test_r9_stage_inside_game_dir_is_refused():
    probs = V.verify_stage(K2_DIR)
    check(len(probs) == 1 and probs[0].startswith("R9:") and "game dir" in probs[0],
          "stage in the game dir should be refused under R9 only, got %r" % probs)


def main():
    passed = 0
    for fn in TESTS:
        try:
            fn()
        except Skip as exc:
            print("SKIP %s: %s" % (fn.__name__, exc))
            passed += 1
        except Exception as exc:  # AssertionError or anything unexpected
            print("FAIL %s: %s: %s" % (fn.__name__, type(exc).__name__, exc))
        else:
            print("PASS %s" % fn.__name__)
            passed += 1
    print("%d/%d PASS" % (passed, len(TESTS)))
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
