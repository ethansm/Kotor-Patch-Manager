#!/usr/bin/env python3
"""kvoice audio layer: ffmpeg decode, EBU R128 measure and libmp3lame encode for 32 kHz mono 48 kbps VO.

ffmpeg is shutil.which('ffmpeg'), else ~/.local/bin/ffmpeg (7.0.2 with libmp3lame; no ffprobe, so
measure with decoded byte counts). PCM convention everywhere: s16le mono 32 kHz bytes.
Encodes go through temp files because the mp3 muxer needs a seekable output for the Xing/LAME tag.
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile
import wave

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
from kvoice_container import SR, CH, BR_KBPS  # noqa: E402

FFMPEG_FALLBACK = os.path.expanduser("~/.local/bin/ffmpeg")
_I_RE = re.compile(r"\n\s+I:\s+(-?[\d.]+|-?inf) LUFS")
_TP_RE = re.compile(r"True peak:\s*\n\s+Peak:\s+(-?[\d.]+|-?inf) dBFS")


class AudioError(RuntimeError):
    """ffmpeg missing, failed, or produced no usable output."""


def ffmpeg_path() -> str:
    found = shutil.which("ffmpeg")
    if found:
        return found
    if os.path.isfile(FFMPEG_FALLBACK) and os.access(FFMPEG_FALLBACK, os.X_OK):
        return FFMPEG_FALLBACK
    raise AudioError("ffmpeg not found on PATH or at %s" % FFMPEG_FALLBACK)


def write_pcm_wav(pcm: bytes, path: str) -> None:
    """Write s16le mono 32 kHz PCM as a proper PCM WAV (the encoder input)."""
    if len(pcm) % 2:
        raise AudioError("PCM byte count %d is odd" % len(pcm))
    with wave.open(path, "wb") as w:
        w.setnchannels(CH)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(pcm)


def decode_pcm(mp3: bytes) -> bytes:
    """Decode a bare MP3 payload (no 58-byte header) to s16le mono 32 kHz PCM."""
    p = subprocess.run([ffmpeg_path(), "-hide_banner", "-loglevel", "error", "-f", "mp3", "-i", "pipe:0",
                        "-f", "s16le", "-ac", str(CH), "-ar", str(SR), "pipe:1"],
                       input=bytes(mp3), capture_output=True)
    if p.returncode != 0 or not p.stdout:
        raise AudioError("decode failed (rc=%d): %s" % (p.returncode, p.stderr.decode("latin1")[-300:]))
    return p.stdout


def measure(path: str, input_format: str = None):
    """EBU R128 integrated loudness (LUFS) and true peak (dBTP) of a file via the ebur128 filter.

    input_format: pass "mp3" for a bare MP3 file (no extension sniffing needed); None lets ffmpeg probe.
    """
    args = [ffmpeg_path(), "-hide_banner", "-nostats"]
    if input_format:
        args += ["-f", input_format]
    args += ["-i", path, "-af", "ebur128=peak=true", "-f", "null", "-"]
    p = subprocess.run(args, capture_output=True)
    err = p.stderr.decode("latin1")
    if p.returncode != 0:
        raise AudioError("measure failed on %s: %s" % (path, err[-300:]))
    mi, mt = _I_RE.search(err), _TP_RE.search(err)
    if not mi or not mt:
        raise AudioError("ebur128 summary missing for %s" % path)
    return float(mi.group(1)), float(mt.group(1))


def encode_file(src_wav: str, out_mp3: str, gain_db: float) -> None:
    """Apply gain, force 32 kHz mono, encode CBR 48 kbps MPEG-1 L3 with Xing/LAME tag and no ID3."""
    cmd = [ffmpeg_path(), "-hide_banner", "-loglevel", "error", "-y", "-i", src_wav,
           "-af", "volume=%.3fdB" % gain_db, "-ac", str(CH), "-ar", str(SR),
           "-c:a", "libmp3lame", "-b:a", "%dk" % BR_KBPS, "-id3v2_version", "0", "-write_xing", "1",
           "-map_metadata", "-1", "-f", "mp3", out_mp3]
    p = subprocess.run(cmd, capture_output=True)
    if p.returncode != 0 or not os.path.isfile(out_mp3):
        raise AudioError("encode failed (rc=%d): %s" % (p.returncode, p.stderr.decode("latin1")[-300:]))


def encode_mp3(pcm: bytes, gain_db: float = 0.0) -> bytes:
    """PCM (s16le mono 32 kHz) to a bare MP3 payload at the given gain. Temp files are removed on return."""
    with tempfile.TemporaryDirectory(prefix="kvoice_enc_") as td:
        wav = os.path.join(td, "in.wav")
        out = os.path.join(td, "out.mp3")
        write_pcm_wav(pcm, wav)
        encode_file(wav, out, gain_db)
        with open(out, "rb") as fh:
            return fh.read()
