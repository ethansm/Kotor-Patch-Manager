#!/usr/bin/env python3
"""Tests for kvoice_pack.py: stage writes, manifest sha256, ledger save, and the game-dir and resref refusals.

Synthetic clips only, built in tempdirs. The game dir is used as a PATH STRING ONLY: no test creates,
writes or lists files there. Symlinks created by the tests point into the game dir but live in a tempdir.
Needs ffmpeg. Run: python3 -u test_kvoice_pack.py   (exit 0 = all PASS)
"""
import json
import math
import os
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

K2_DIR = Path(os.environ.get(
    "KVOICE_K2_DIR", "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")).expanduser()
TESTS = []
_CACHE = {}


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def expect_pack_error(fn, msg):
    try:
        fn()
    except kp.PackError:
        return
    raise AssertionError(msg + " (no PackError raised)")


def good_mp3():
    if "mp3" not in _CACHE:
        n = int(1.0 * kc.SR)
        pcm = b"".join(struct.pack("<h", int(0.3 * 32767 * math.sin(2 * math.pi * 440.0 * i / kc.SR)))
                       for i in range(n))
        _CACHE["mp3"], _report = kn.normalize_encode(pcm)
    return _CACHE["mp3"]


def good_lip(mp3=None):
    frames = kc.check_payload(mp3 or good_mp3())
    return L.write_lip([(0.0, 0), (0.1, 11)], kc.expected_pcm_samples(frames) / kc.SR)


def no_game_files(stage_path, resref):
    """Nothing was written for a refused stage (checked by path string; read-only)."""
    return not (Path(stage_path) / "Override" / (resref + ".wav")).exists()


@test
def test_refuses_game_dir_itself():
    mp3, lip = good_mp3(), good_lip()
    expect_pack_error(lambda: kp.pack_clip(K2_DIR, "kpvtest1", mp3, lip),
                      "game dir itself must be refused")
    check(no_game_files(K2_DIR, "kpvtest1"), "refused pack must not write under the game dir")


@test
def test_refuses_subdir_of_game_dir():
    mp3, lip = good_mp3(), good_lip()
    expect_pack_error(lambda: kp.pack_clip(K2_DIR / "Override" / "stage", "kpvtest1", mp3, lip),
                      "a stage under the game dir must be refused")


@test
def test_refuses_symlink_that_resolves_into_game_dir():
    mp3, lip = good_mp3(), good_lip()
    with tempfile.TemporaryDirectory(prefix="kvoice_pk1_") as td:
        link = Path(td) / "stage_link"
        os.symlink(K2_DIR, link)  # the link lives in the tempdir; the target is only a path string
        expect_pack_error(lambda: kp.pack_clip(link, "kpvtest1", mp3, lip),
                          "a stage symlink into the game dir must be refused (readlink -f)")
        check(no_game_files(K2_DIR, "kpvtest1"), "refused pack must not write under the game dir")


@test
def test_refuses_override_symlink_into_game_dir():
    mp3, lip = good_mp3(), good_lip()
    with tempfile.TemporaryDirectory(prefix="kvoice_pk2_") as td:
        stage = Path(td) / "stage"
        stage.mkdir()
        os.symlink(K2_DIR / "Override", stage / "Override")  # Override -> game dir (path string only)
        expect_pack_error(lambda: kp.pack_clip(stage, "kpvtest1", mp3, lip),
                          "an Override symlink into the game dir must be refused")
        check(no_game_files(K2_DIR, "kpvtest1"), "refused pack must not write under the game dir")


@test
def test_write_ledger_refuses_game_dir():
    expect_pack_error(lambda: kp.write_ledger(K2_DIR, {"schema": 1, "entries": [], "approvals": {}}),
                      "write_ledger must refuse the game dir")
    check(not (K2_DIR / "ledger.json").exists(), "refused ledger must not be written under the game dir")


@test
def test_refuses_non_kpv_and_bad_length_resrefs():
    mp3, lip = good_mp3(), good_lip()
    bad = ["abc1", "KPVtest", "kpv-bad", "xkpv1", "kpv" + "x" * 14, "kpv\n", "", "kpv.a"]
    with tempfile.TemporaryDirectory(prefix="kvoice_pk3_") as td:
        stage = Path(td) / "stage"
        for resref in bad:
            expect_pack_error(lambda r=resref: kp.pack_clip(stage, r, mp3, lip),
                              "resref %r must be refused" % resref)
        check(not stage.exists(), "refused resrefs must not create the stage")
        out = kp.pack_clip(stage, "kpv" + "x" * 13, mp3, lip)  # 16 characters is the maximum
        check(Path(out["wav"]).is_file(), "a 16-character kpv resref must be accepted")


@test
def test_pack_vo58_writes_wrapped_wav_and_manifest_sha():
    mp3, lip = good_mp3(), good_lip()
    with tempfile.TemporaryDirectory(prefix="kvoice_pk4_") as td:
        stage = Path(td) / "stage"
        out = kp.pack_clip(stage, "kpvtest1", mp3, lip, container="vo58")
        wav = Path(out["wav"]).read_bytes()
        check(wav == kc.wrap(mp3), "vo58 wav must be the 58-byte header plus the MP3 payload")
        check(kc.strip(wav) == mp3, "stripping the wav must give back the MP3 payload")
        check(Path(out["lip"]).read_bytes() == lip, "lip must be written unchanged")
        manifest = json.loads((stage / "manifest.json").read_text(encoding="utf-8"))
        check(manifest["schema"] == 1, "manifest schema must be 1")
        check(manifest["files"]["Override/kpvtest1.wav"] == kp.sha256_bytes(wav), "wav sha256 in manifest")
        check(manifest["files"]["Override/kpvtest1.lip"] == kp.sha256_bytes(lip), "lip sha256 in manifest")


@test
def test_pack_bare_writes_mp3_unchanged():
    mp3, lip = good_mp3(), good_lip()
    with tempfile.TemporaryDirectory(prefix="kvoice_pk5_") as td:
        out = kp.pack_clip(Path(td) / "stage", "kpvbare1", mp3, lip, container="bare")
        check(Path(out["wav"]).read_bytes() == mp3, "bare container must write the MP3 unchanged")


@test
def test_pack_rejects_bad_clip_and_bad_lip_without_writing():
    mp3, lip = good_mp3(), good_lip()
    with tempfile.TemporaryDirectory(prefix="kvoice_pk6_") as td:
        stage = Path(td) / "stage"
        expect_pack_error(lambda: kp.pack_clip(stage, "kpvtest1", b"\x00" * 300, lip),
                          "a non-MP3 payload must be refused for vo58")
        expect_pack_error(lambda: kp.pack_clip(stage, "kpvtest1", mp3, b"LIP V1.0 junk"),
                          "a malformed lip must be refused")
        expect_pack_error(lambda: kp.pack_clip(stage, "kpvtest1", mp3, lip, container="ogg"),
                          "an unknown container must be refused")
        expect_pack_error(lambda: kp.pack_clip(stage, "kpvtest1", b"\x00" * 300, lip, container="bare"),
                          "a bare payload with no MP3 frame at offset 0 must be refused")
        check(not (stage / "Override" / "kpvtest1.wav").exists(), "refused clips must write nothing")


@test
def test_manifest_accumulates_across_clips():
    mp3, lip = good_mp3(), good_lip()
    with tempfile.TemporaryDirectory(prefix="kvoice_pk7_") as td:
        stage = Path(td) / "stage"
        kp.pack_clip(stage, "kpvtest1", mp3, lip)
        kp.pack_clip(stage, "kpvtest2", mp3, lip)
        files = json.loads((stage / "manifest.json").read_text(encoding="utf-8"))["files"]
        check(sorted(files) == ["Override/kpvtest1.lip", "Override/kpvtest1.wav",
                                "Override/kpvtest2.lip", "Override/kpvtest2.wav"],
              "manifest must list both clips, got %r" % sorted(files))


@test
def test_write_ledger_saves_stage_ledger():
    mp3, lip = good_mp3(), good_lip()
    with tempfile.TemporaryDirectory(prefix="kvoice_pk8_") as td:
        stage = Path(td) / "stage"
        out = kp.pack_clip(stage, "kpvtest1", mp3, lip)
        led = KL.ks.Ledger(path=Path(td) / "source_ledger.json")  # fresh path; load() finds nothing
        KL.row_for_clip(led, out["wav"], container="vo58", source_resref="kpvtest1", slot="voice",
                        ship_mode="personal", source_id="vanilla_k2")
        path = kp.write_ledger(stage, led)
        check(path == stage / "ledger.json", "ledger must be saved to <stage>/ledger.json")
        saved = json.loads(path.read_text(encoding="utf-8"))
        check(saved["schema"] == KL.ks.SCHEMA and len(saved["entries"]) == 1, "one row saved, schema kept")
        check(saved["entries"][0]["local_only"] is True, "vanilla row must stay local_only")
        check(not (Path(td) / "source_ledger.json").exists(), "write_ledger must not save the source ledger")


@test
def test_write_ledger_rejects_bad_shape():
    with tempfile.TemporaryDirectory(prefix="kvoice_pk9_") as td:
        stage = Path(td) / "stage"
        expect_pack_error(lambda: kp.write_ledger(stage, {"schema": 2, "entries": []}),
                          "a wrong schema must be refused")
        expect_pack_error(lambda: kp.write_ledger(stage, {"schema": 1}),
                          "a ledger without entries must be refused")
        check(not (stage / "ledger.json").exists(), "refused ledgers must not be written")


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
