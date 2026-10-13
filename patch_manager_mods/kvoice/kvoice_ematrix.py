#!/usr/bin/env python3
"""kvoice E-matrix: synthetic experiment clips for the K2 StreamVoice probes (doc 41, Task 1).

Writes <out>/Override/ clips and <out>/manifest.json (schema 1: "files" maps "Override/<file>" to sha256,
the shape kvoice_verify.verify_stage reads). Matrix:
  E1  kpve1a.wav                      RIFF PCM16 mono 22050 Hz, 2 s 1 kHz tone, -17 LUFS
  E2  kpve2a.wav                      same PCM as E1
      kpve2b.wav                      58-byte container + MP3 (kvoice_container.build_header)
      kpve2c.mp3                      bare MP3 (verify_wav reports R1)
      kpve2d.wav                      first 470 bytes of --sfx-prefix-from, then RIFF PCM (R2); skipped if no source
  E3  kpve3a.wav                      container, no .lip
      kpve3b.wav + kpve3b.lip         container + valid LIP (rest/ah/oh/rest, last key = length)
      kpve3c.wav + kpve3c.lip         container + LIP whose first key is at 0.25 s (R7)
  E8  KpvE8Mixed.wav                  mixed case (fails R9 name rule on purpose)
      kpve8_sixteenchr.wav            stem exactly 16 chars (passes the length rule)
      kpve8_seventeench.wav           stem exactly 17 chars (fails the length rule on purpose)

Synthetic tone only. The only vanilla bytes touched are the 470-byte prefix read for kpve2d.wav; they
are written into the runtime out dir and must never be committed. Stdlib plus kvoice_audio (ffmpeg).

CLI: python3 kvoice_ematrix.py --out <dir> [--sfx-prefix-from <vanilla.wav>]
"""
import argparse
import hashlib
import io
import json
import math
import os
import re
import struct
import sys
import wave
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kvoice_audio as ka  # noqa: E402
import kvoice_container as kc  # noqa: E402
import kvoice_lip as kl  # noqa: E402
import kvoice_pack as kp  # noqa: E402
import kvoice_verify as kv  # noqa: E402

PCM_SR = 22050            # E1/E2a PCM rate
MP3_SR = kc.SR            # 32000, the rate encode_mp3 takes
DUR_S = 2.0
TONE_HZ = 1000.0
REF_AMP = 0.5             # reference amplitude measured for loudness, then scaled to the target
TARGET_LUFS = -17.0
PREFIX_LEN = 0x1D6        # 470: kvoice_verify R2_OFFSET, the copy length for kpve2d.wav
RESREF_RE = re.compile(r"kpv[a-z0-9_]*")
RESREF_MAX = 16
MANIFEST_SCHEMA = 1
SHAPE_REST = kl.SHAPE_REST
SHAPE_AH = kl.RHUBARB_TO_SHAPE["D"]   # Rhubarb D (wide open) -> ah
SHAPE_OH = kl.RHUBARB_TO_SHAPE["E"]   # Rhubarb E (rounded) -> oh


def _tone_pcm(sr, amp):
    """DUR_S seconds of a 1 kHz sine, s16le mono at sr."""
    n = int(round(sr * DUR_S))
    w = 2.0 * math.pi * TONE_HZ / sr
    return struct.pack("<%dh" % n, *(int(round(amp * 32767.0 * math.sin(w * i))) for i in range(n)))


def _pcm_wav_bytes(pcm, sr):
    buf = io.BytesIO()
    with wave.open(buf, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sr)
        w.writeframes(pcm)
    return buf.getvalue()


def _gain_db(sr, scratch_dir):
    """dB that moves a REF_AMP tone at sr to TARGET_LUFS, measured with kvoice_audio.measure."""
    probe = Path(scratch_dir) / ("_measure_%d.wav" % sr)
    probe.write_bytes(_pcm_wav_bytes(_tone_pcm(sr, REF_AMP), sr))
    try:
        lufs, _ = ka.measure(str(probe))
    finally:
        probe.unlink(missing_ok=True)
    return TARGET_LUFS - lufs


def _scaled_amp(gain_db):
    return REF_AMP * 10.0 ** (gain_db / 20.0)


def _read_prefix(src):
    with open(src, "rb") as fh:
        prefix = fh.read(PREFIX_LEN)
    if len(prefix) < PREFIX_LEN:
        raise ValueError("sfx prefix source %s is shorter than %d bytes" % (os.path.basename(src), PREFIX_LEN))
    return prefix


def _lip_first_key_late():
    """Hand-packed LIP (SIG, float32 length, uint32 count, {float32 t, uint8 shape}) with first key at 0.25 s."""
    keys = [(0.25, SHAPE_REST), (0.5, SHAPE_AH), (1.0, SHAPE_OH), (DUR_S, SHAPE_REST)]
    body = struct.pack("<8sfI", kl.SIG, DUR_S, len(keys))
    for t, s in keys:
        body += struct.pack("<fB", t, s)
    return body


def _resref_ok(name):
    """Stem is kpv-prefixed, lowercase [a-z0-9_], and at most 16 characters (kvoice_verify R9 name rule)."""
    stem = os.path.splitext(name)[0]
    return len(stem) <= RESREF_MAX and RESREF_RE.fullmatch(stem) is not None


def _expected_rules(name, data):
    if name.endswith(".lip"):
        return ["R7"] if kl.check_lip(data) else []
    return sorted({p.split(":", 1)[0] for p in kv.verify_wav(data)})


def _sha256(data):
    return hashlib.sha256(data).hexdigest()


def build_ematrix(out_dir, sfx_prefix_from=None) -> dict:
    """Write the E-matrix clips and manifest.json under out_dir and return the manifest dict.

    Raises kvoice_pack.PackError before any write when out_dir is the game dir or inside it.
    sfx_prefix_from: a vanilla SFX .wav whose first 470 bytes become kpve2d.wav's prefix; None skips kpve2d.
    """
    root = Path(kp.refuse_game_dir(out_dir))
    ovr = root / "Override"
    ovr.mkdir(parents=True, exist_ok=True)
    prefix = _read_prefix(sfx_prefix_from) if sfx_prefix_from is not None else None

    gain_pcm = _gain_db(PCM_SR, root)
    gain_mp3 = _gain_db(MP3_SR, root)
    wav_pcm = _pcm_wav_bytes(_tone_pcm(PCM_SR, _scaled_amp(gain_pcm)), PCM_SR)
    mp3 = ka.encode_mp3(_tone_pcm(MP3_SR, _scaled_amp(gain_mp3)), 0.0)
    container = kc.build_header() + mp3
    lip_ok = kl.write_lip([(0.0, SHAPE_REST), (0.5, SHAPE_AH), (1.0, SHAPE_OH), (DUR_S, SHAPE_REST)], DUR_S)
    lip_late = _lip_first_key_late()

    clips = [
        ("kpve1a.wav", "E1", wav_pcm),
        ("kpve2a.wav", "E2", wav_pcm),
        ("kpve2b.wav", "E2", container),
        ("kpve2c.mp3", "E2", mp3),
    ]
    skipped = {}
    if prefix is None:
        skipped["Override/kpve2d.wav"] = "no --sfx-prefix-from source; E2d not written"
    else:
        clips.append(("kpve2d.wav", "E2", prefix + wav_pcm))
    clips += [
        ("kpve3a.wav", "E3", container),
        ("kpve3b.wav", "E3", container),
        ("kpve3b.lip", "E3", lip_ok),
        ("kpve3c.wav", "E3", container),
        ("kpve3c.lip", "E3", lip_late),
        ("KpvE8Mixed.wav", "E8", container),
        ("kpve8_sixteenchr.wav", "E8", container),
        ("kpve8_seventeench.wav", "E8", container),
    ]

    files, entries = {}, {}
    for name, experiment, data in clips:
        (ovr / name).write_bytes(data)
        key = "Override/" + name
        files[key] = _sha256(data)
        entries[key] = {
            "experiment": experiment,
            "bytes": len(data),
            "sha256": files[key],
            "expected_rules": _expected_rules(name, data),
            "resref_ok": _resref_ok(name),
        }

    manifest = {
        "schema": MANIFEST_SCHEMA,
        "experiment": "doc41-ematrix",
        "files": files,
        "entries": entries,
        "skipped": skipped,
        "tone": {
            "hz": TONE_HZ,
            "seconds": DUR_S,
            "target_lufs": TARGET_LUFS,
            "pcm_sr": PCM_SR,
            "mp3_sr": MP3_SR,
            "gain_db": {str(PCM_SR): gain_pcm, str(MP3_SR): gain_mp3},
        },
    }
    (root / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return manifest


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Build the kvoice E-matrix clips and manifest.json (synthetic tone only).")
    ap.add_argument("--out", required=True, help="output stage dir; must not be the game dir")
    ap.add_argument("--sfx-prefix-from", default=None, metavar="VANILLA_WAV",
                    help="vanilla .wav whose first 470 bytes become kpve2d.wav; omit to skip E2d")
    args = ap.parse_args(argv)
    try:
        manifest = build_ematrix(args.out, args.sfx_prefix_from)
    except (kp.PackError, ka.AudioError, ValueError, OSError) as exc:
        print("kvoice_ematrix: %s" % exc, file=sys.stderr)
        return 1
    print("wrote %d clips under %s/Override (skipped: %s)"
          % (len(manifest["files"]), args.out, ", ".join(sorted(manifest["skipped"])) or "none"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
