#!/usr/bin/env python3
"""kvoice loudness normalisation: gain to -18.5 LUFS (+/-0.3 LU), true peak <= -1.0 dBTP, then MP3 encode.

Targets come from the vanilla StreamVoice corpus (20 clips): integrated median -18.5 LUFS, true peak median
-2.5 dBTP (max +0.1). Input is s16le mono 32 kHz PCM. Output is the bare MP3 payload (no 58-byte header;
kvoice_container.wrap adds it). Every pass re-encodes from PCM, never from a previous MP3 (no generation loss).
The true-peak ceiling is hard: a clip that cannot reach the target under it raises NormalizeError.
"""
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import kvoice_audio as ka  # noqa: E402
import kvoice_container as kc  # noqa: E402

TARGET_LUFS = -18.5
TOL_LU = 0.3
TP_CEIL_DBTP = -1.0
MAX_PASSES = 3
MAX_TP_SHORTFALL_LU = 3.0      # a peak-limited clip may land at most this far under the target; quieter is refused
TP_LIMIT_MARGIN_DB = 0.6      # a result this close under the ceiling counts as peak-limited (encoder TP jitter ~0.5 dB)
LEAD_IN_S = 0.15              # prepended silence; decode shows silence == lead-in exactly (120 ms floor + margin)
SILENT_FLOOR_LUFS = -60.0     # below this the clip is silence or noise floor: refuse
MIN_CLIP_S = 0.4              # BS.1770 gating block is 400 ms; shorter clips have no defined loudness


class NormalizeError(RuntimeError):
    """Clip cannot be brought to target within tolerance, or ceiling/passes prevent it."""


class SilentClipError(NormalizeError):
    """Clip is all zeros, or measures below the silence floor."""


def gain_for(I: float, TP: float, target: float = TARGET_LUFS, ceil: float = TP_CEIL_DBTP):
    """Gain in dB to reach target loudness, capped so true peak stays <= ceil. Returns (gain_db, limited_by_tp)."""
    g = target - I
    if TP + g > ceil:
        return ceil - TP, True
    return g, False


def normalize_encode(pcm: bytes, target: float = TARGET_LUFS, tol: float = TOL_LU, ceil: float = TP_CEIL_DBTP,
                     max_passes: int = MAX_PASSES):
    """Normalise s16le mono 32 kHz PCM and encode it. Returns (mp3_payload, report).

    Raises SilentClipError for silent or sub-floor input, NormalizeError for too-short input or when the
    no pass stays under the true-peak ceiling, or the loudness misses the tolerance without being peak-limited.
    A peak-limited clip (cannot reach the target under the ceiling) is returned quieter with report['tp_limited'] True.
    """
    if len(pcm) % 2:
        raise NormalizeError("PCM byte count is odd")
    if pcm.count(0) == len(pcm):
        raise SilentClipError("clip is all zero samples")
    if len(pcm) / 2 / kc.SR < MIN_CLIP_S:
        raise NormalizeError("clip shorter than %.1f s" % MIN_CLIP_S)
    lead = b"\x00" * (int(round(LEAD_IN_S * kc.SR)) * 2)
    body = lead + pcm
    with tempfile.TemporaryDirectory(prefix="kvoice_norm_") as td:
        wav = os.path.join(td, "in.wav")
        ka.write_pcm_wav(body, wav)
        I0, TP0 = ka.measure(wav)
        if I0 < SILENT_FLOOR_LUFS:
            raise SilentClipError("clip measures %.1f LUFS, below the %.1f LUFS floor" % (I0, SILENT_FLOOR_LUFS))
        gain, limited = gain_for(I0, TP0, target, ceil)
        trace = []
        best = None  # loudest pass that stays under the true-peak ceiling: (I, TP, gain, payload)
        for n in range(1, max_passes + 1):
            out = os.path.join(td, "pass%d.mp3" % n)
            ka.encode_file(wav, out, gain)
            I, TP = ka.measure(out, "mp3")
            trace.append({"pass": n, "gain_db": round(gain, 3), "I": I, "TP": TP})
            if TP <= ceil and (best is None or I > best[0]):
                with open(out, "rb") as fh:
                    best = (I, TP, gain, fh.read())
            if abs(I - target) <= tol and TP <= ceil:
                break
            delta = target - I
            if TP + delta > ceil:
                delta = ceil - TP
            if abs(delta) < 0.01:  # stalled: ceiling holds the gain where it is
                break
            gain += delta
        if best is None:
            raise NormalizeError("true peak stays above the %.1f dBTP ceiling after %d pass(es); trace=%r"
                                 % (ceil, len(trace), trace))
        I, TP, gain, payload = best
        # A peaky clip cannot reach the target without breaking the ceiling: accept it quieter, flagged.
        tp_limited = (target - MAX_TP_SHORTFALL_LU <= I < target - tol) and TP >= ceil - TP_LIMIT_MARGIN_DB
        if abs(I - target) > tol and not tp_limited:
            raise NormalizeError("target not met after %d pass(es): I=%.2f LUFS (want %.1f +/-%.1f), TP=%.2f dBTP "
                                 "(ceiling %.1f); trace=%r" % (len(trace), I, target, tol, TP, ceil, trace))
    kc.check_payload(payload)
    report = {"I": I, "TP": TP, "gain_db": round(gain, 3), "passes": len(trace), "tp_limited": tp_limited,
              "src_I": I0, "src_TP": TP0, "limited_by_tp_first_pass": limited,
              "lead_in_s": LEAD_IN_S, "trace": trace}
    return payload, report
