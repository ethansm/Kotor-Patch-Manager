#!/usr/bin/env python3
"""kvoice container layer: the 58-byte fake WAV header that K2 StreamVoice .wav files carry before the MP3 frames.

Layout (measured identical in 3/3 copied and 20/20 sampled vanilla clips):
  off 0 'RIFF' | off 4 uint32 50 (constant, NOT the file size) | off 8 'WAVE' | off 12 'fmt '
  off 16 uint32 18 | off 20 <HHIIHHH = 1,1,22050,22050,1,8,0 (declared 8-bit 22050 mono; ignored, audio is 32 kHz)
  off 38 'fact' 4 0 | off 50 'data' 0 | off 58 MP3 frames (ff fb 38 c4 = MPEG-1 L3, 48 kbps, 32 kHz, mono, CBR)
File size = 58 + 216 * frames. Decoded samples = (frames - 1) * 1152 - 1105 (LAME gapless trim, Xing/LAME frame 0).

Stdlib only. Structure only: this does not prove the engine plays the file.
"""
import struct

HDR_LEN = 58
SR, CH, BR_KBPS = 32000, 1, 48       # MPEG-1 L3, 32 kHz, mono, 48 kbps
FRAME_LEN = 216                      # 144 * 48000 / 32000, no padding
GAPLESS_TRIM = 1105                  # LAME encoder delay 576 + decoder delay 529
MP3_HDR_RATE = {  # MPEG-1 Layer III bitrates (kbps) and sample rates, indexed by header bits
    "br": [0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0],
    "sr": [44100, 48000, 32000, 0],
}


class ContainerError(ValueError):
    """Input is not a clean 58-byte VO container, or not a clean 32 kHz mono 48 kbps MP3 frame chain."""


def build_header() -> bytes:
    """The 58-byte prefix that every vanilla K2 StreamVoice .wav carries before the MP3 frames."""
    h = bytearray(HDR_LEN)
    h[0:4] = b"RIFF"
    struct.pack_into("<I", h, 4, 50)                     # RIFF size = 58 - 8 (constant, NOT file size)
    h[8:12] = b"WAVE"
    h[12:16] = b"fmt "
    struct.pack_into("<I", h, 16, 18)                    # fmt size (WAVEFORMATEX incl. cbSize)
    struct.pack_into("<HHIIHHH", h, 20, 1, 1, 22050, 22050, 1, 8, 0)  # declared 8-bit 22050 mono (ignored)
    h[38:42] = b"fact"
    struct.pack_into("<II", h, 42, 4, 0)                 # fact size 4, dwSampleLength 0
    h[50:54] = b"data"
    struct.pack_into("<I", h, 54, 0)                     # data size 0 (MP3 follows at 58)
    return bytes(h)


def frame_header(b: bytes, off: int):
    """Parse one MPEG-1 Layer III frame header at off. Returns a dict or None (not a usable frame)."""
    if off + 4 > len(b):
        return None
    h = struct.unpack_from(">I", b, off)[0]
    if (h >> 21) & 0x7FF != 0x7FF or ((h >> 19) & 3) != 3 or ((h >> 17) & 3) != 1:
        return None                                      # need MPEG-1 Layer III sync
    bri, sri = (h >> 12) & 0xF, (h >> 10) & 3
    if bri in (0, 15) or sri == 3:
        return None
    br, sr = MP3_HDR_RATE["br"][bri], MP3_HDR_RATE["sr"][sri]
    return {"sr": sr, "br": br, "ch": 1 if (h >> 6) & 3 == 3 else 2,
            "len": 144 * br * 1000 // sr + ((h >> 9) & 1)}


def walk_frames(b: bytes, start: int):
    """Walk the MP3 frame chain from start. Returns (n, first, bad).

    n: frames in the uniform run. first: header dict of the first frame (None if none).
    bad: offset where the run stops (sync lost, params or length change, or a frame overruns EOF);
    None when the run ends exactly at EOF.
    """
    off, n, first = start, 0, None
    while off < len(b):
        fh = frame_header(b, off)
        if fh is None:
            return n, first, off
        key = (fh["sr"], fh["ch"], fh["br"], fh["len"])
        if first is None:
            first = fh
        elif key != (first["sr"], first["ch"], first["br"], first["len"]):
            return n, first, off
        n += 1
        off += fh["len"]
    return n, first, None if off == len(b) else off


def check_payload(payload: bytes) -> int:
    """Return the frame count of a bare MP3 payload that is one clean 32 kHz mono 48 kbps CBR chain; else raise."""
    n, first, bad = walk_frames(payload, 0)
    if first is None:
        raise ContainerError("no MPEG-1 L3 frame at payload offset 0")
    if (first["sr"], first["ch"], first["br"], first["len"]) != (SR, CH, BR_KBPS, FRAME_LEN):
        raise ContainerError("first frame %r is not 32 kHz mono 48 kbps, 216-byte frames" % (first,))
    if bad is not None:
        raise ContainerError("MP3 frame chain breaks at payload offset %d (or trailing junk)" % bad)
    return n


def _problem(b: bytes):
    """None when b is a clean 58-byte VO container, else the reason it is not."""
    if len(b) < HDR_LEN + 4:
        return "shorter than the 58-byte header plus one frame"
    if b[:HDR_LEN] != build_header():
        return "58-byte header differs from the vanilla template"
    try:
        check_payload(b[HDR_LEN:])
    except ContainerError as e:
        return str(e)
    return None


def detect(b: bytes) -> bool:
    """True when b is a clean 58-byte VO container: vanilla header, then one unbroken 32 kHz mono 48 kbps chain."""
    return _problem(b) is None


def strip(b: bytes) -> bytes:
    """Return the bare MP3 payload of a 58-byte VO container. Refuses anything else (bare MP3, other WAV, junk)."""
    reason = _problem(b)
    if reason is not None:
        raise ContainerError("not a 58-byte VO container: " + reason)
    return b[HDR_LEN:]


def wrap(mp3: bytes) -> bytes:
    """58-byte prefix + MP3 frames (the vanilla VO container). The payload must be a clean 32 kHz mono 48 kbps chain."""
    payload = bytes(mp3)
    check_payload(payload)
    return build_header() + payload


def expected_pcm_samples(frames: int) -> int:
    """Decoded mono sample count of an n-frame LAME-tagged 32 kHz payload (gapless trim applied)."""
    if frames < 1:
        raise ValueError("frames must be >= 1")
    return (frames - 1) * 1152 - GAPLESS_TRIM
