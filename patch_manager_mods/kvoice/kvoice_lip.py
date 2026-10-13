"""KOTOR II .lip lip-sync file: write, check, and Rhubarb mapping. Stdlib only.

Structure only; engine behaviour unverified until E3.

Layout: b"LIP V1.0" + float32 length + uint32 count + count x {float32 time, uint8 shape}.
Shape table is the rakata/PyKotor 16-name table (derived, not engine-attested).
Rules checked (rakata's claimed validity gate, plus basic sanity): first key at t=0,
last key at the header length, times monotonic, shapes 0-15, never zero bytes.
"""
import math
import struct

SIG = b"LIP V1.0"
HEADER_SIZE = 16  # 8-byte signature + float32 length + uint32 count
KEY_SIZE = 5      # float32 time + uint8 shape
SHAPE_REST = 0
SHAPE_NAMES = (
    "rest", "ee", "eh", "ah", "oh", "ooh", "y", "s",
    "f", "ng", "th", "m/p/b", "t/d", "sh", "l/r", "k/g",
)
# Rhubarb mouth letter -> KOTOR shape. B (most consonants and EE) is a catch-all; t/d is an approximation.
RHUBARB_TO_SHAPE = {"X": 0, "A": 11, "B": 12, "C": 2, "D": 3, "E": 4, "F": 5, "G": 8, "H": 14}
TIME_TOL = 1e-5  # relative tolerance for last key vs header length


def rhubarb_to_lip(tsv):
    """Rhubarb TSV text (one 'time<TAB>letter' per line) -> [(time_s, shape)]. Malformed lines are skipped."""
    cues = []
    for line in tsv.splitlines():
        parts = line.strip().split("\t")
        if len(parts) != 2:
            continue
        letter = parts[1].strip().upper()
        if letter not in RHUBARB_TO_SHAPE:
            raise ValueError("unknown Rhubarb letter %r" % letter)
        cues.append((float(parts[0]), RHUBARB_TO_SHAPE[letter]))
    return cues


def parse_lip(data):
    """Return (length, [(time, shape), ...]). Raises ValueError on a malformed container."""
    if len(data) < HEADER_SIZE:
        raise ValueError("short header (%d bytes)" % len(data))
    if data[:8] != SIG:
        raise ValueError("bad signature %r" % data[:8])
    length, count = struct.unpack_from("<fI", data, 8)
    if len(data) != HEADER_SIZE + KEY_SIZE * count:
        raise ValueError("size %d != 16 + 5*%d" % (len(data), count))
    keys = [struct.unpack_from("<fB", data, HEADER_SIZE + KEY_SIZE * i) for i in range(count)]
    return length, keys


def check_lip(data):
    """Return a list of rule problems. An empty list means the bytes pass the structure rules."""
    if not data:
        return ["zero-byte file"]
    try:
        length, keys = parse_lip(data)
    except ValueError as exc:
        return [str(exc)]
    problems = []
    if not math.isfinite(length) or length <= 0:
        problems.append("length %r is not a positive finite number" % length)
    if not keys:
        return problems + ["no keys"]
    times = [t for t, _ in keys]
    if not all(math.isfinite(t) for t in times):
        problems.append("non-finite key time")
    else:
        if times[0] != 0.0:
            problems.append("first key time %.6f != 0" % times[0])
        if abs(times[-1] - length) > length * TIME_TOL + 1e-9:
            problems.append("last key %.6f != length %.6f" % (times[-1], length))
        if any(b < a for a, b in zip(times, times[1:])):
            problems.append("times not monotonic")
    bad = [(i, s) for i, (_, s) in enumerate(keys) if s > 15]
    if bad:
        problems.append("shape %d at key %d outside 0-15" % (bad[0][1], bad[0][0]))
    return problems


def write_lip(cues, length_s):
    """Build LIP bytes from [(time_s, shape)] and the audio length in seconds.

    Adds a rest key at t=0 when the first cue is later, and a rest key at length_s when the last
    cue ends early. Raises ValueError rather than return bytes that check_lip would reject.
    """
    length = float(length_s)
    keys = [(float(t), int(s)) for t, s in cues]
    if not keys or keys[0][0] != 0.0:
        keys.insert(0, (0.0, SHAPE_REST))
    if keys[-1][0] < length:
        keys.append((length, SHAPE_REST))
    for _, s in keys:
        if not 0 <= s <= 15:
            raise ValueError("shape out of range 0-15: %r" % s)
    try:
        body = bytearray(SIG)
        body += struct.pack("<fI", length, len(keys))
        for t, s in keys:
            body += struct.pack("<fB", t, s)
    except (OverflowError, struct.error) as exc:
        raise ValueError("cannot pack LIP: %s" % exc)
    data = bytes(body)
    problems = check_lip(data)
    if problems:
        raise ValueError("; ".join(problems))
    return data
