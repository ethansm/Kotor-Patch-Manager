#!/usr/bin/env python3
"""kvoice verifier: structure rules R1-R9 for kvoice .wav clips and their stage.

STRUCTURE ONLY. These rules show that a clip has the vanilla VO layout, that ffmpeg decodes it, and that
a stage is internally consistent (pairing, sha256, ledger rows, local_only). They do not prove that the
engine plays the file; that is E1/E2 in game.

verify_wav(bytes): R1-R6 on one .wav.
  R1  RIFF at offset 0. A failure stops the check, so a bare MP3 (E2 matrix) is reported as R1 only, never R3.
  R2  no second RIFF at 0x1D6.
  R3  first 58 bytes == kvoice_container.build_header().
  R4  MP3 frame chain from offset 58 is one 32 kHz mono 48 kbps 216-byte run that reaches EOF.
  R5  file size == 58 + 216 * frames (frames as walked).
  R6  ffmpeg decodes the payload (kvoice_audio.decode_pcm on data[58:]).
verify_stage(stage_dir, shippable=False): R1-R6 per .wav, plus
  R7  each .lip passes kvoice_lip.check_lip.
  R8  lip length within 36 ms of the decoded duration, using expected_pcm_samples(frames) / 32000 s
      (the formula, not a second decode).
  R9  resref [a-z0-9_]{1,16} with the kpv prefix; wav and lip paired; sha256 matches manifest.json;
      one ledger.json row per file; with shippable=True no row may be local_only.
Symlinks in a stage are reported and never followed. A stage that resolves inside the game dir is
refused under R9 before anything is read.
Problem strings start with the rule id, e.g. 'R3: ...'. verify_stage puts the file name after the id.
Stdlib plus kvoice_audio (ffmpeg), kvoice_container, kvoice_lip and kvoice_pack.
"""
import json
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))
import kvoice_audio as ka  # noqa: E402
import kvoice_container as kc  # noqa: E402
import kvoice_lip as L  # noqa: E402
import kvoice_pack as kp  # noqa: E402

HDR = kc.HDR_LEN
R2_OFFSET = 0x1D6
LIP_TOL_S = 0.036  # one MPEG-1 L3 frame at 32 kHz (1152 samples)
LIP_EPS_S = 1e-6   # float32 rounding of the lip header length
WAV_EXT, LIP_EXT = ".wav", ".lip"


def verify_wav(data) -> list:
    """Return the R1-R6 problems of one .wav, each as 'Rn: ...'. An empty list means the structure passes."""
    data = bytes(data)
    if data[:4] != b"RIFF":
        return ["R1: no RIFF at offset 0 (not a 58-byte VO container; bare MP3 or other input)"]
    problems = []
    if data[R2_OFFSET:R2_OFFSET + 4] == b"RIFF":
        problems.append("R2: second RIFF at 0x%X" % R2_OFFSET)
    if data[:HDR] != kc.build_header():
        problems.append("R3: first 58 bytes differ from build_header()")
    frames, first, bad = kc.walk_frames(data, HDR)
    if first is None:
        problems.append("R4: no MPEG-1 L3 frame at offset %d" % HDR)
    else:
        fmt = (first["sr"], first["ch"], first["br"], first["len"])
        if fmt != (kc.SR, kc.CH, kc.BR_KBPS, kc.FRAME_LEN):
            problems.append("R4: first frame %r is not 32 kHz mono 48 kbps 216-byte" % (fmt,))
        if bad is not None:
            problems.append("R4: frame chain breaks at offset %d (not EOF)" % bad)
    expect = HDR + kc.FRAME_LEN * frames
    if len(data) != expect:
        problems.append("R5: size %d != 58 + 216*%d = %d" % (len(data), frames, expect))
    payload = data[HDR:]
    if not payload:
        problems.append("R6: no payload after the 58-byte header")
    else:
        try:
            ka.decode_pcm(payload)
        except ka.AudioError as exc:
            problems.append("R6: ffmpeg decode failed: %s" % " ".join(str(exc).split()))
    return problems


def _tag(name, problem) -> str:
    """'Rn: msg' -> 'Rn: name: msg'."""
    rule, _, msg = problem.partition(": ")
    return "%s: %s: %s" % (rule, name, msg)


def _sha_file(path) -> str:
    return kp.sha256_bytes(Path(path).read_bytes())


def _read_rows(ledger_path):
    """Entries list of a schema-1 ledger.json, or None when unreadable or malformed."""
    try:
        raw = json.loads(Path(ledger_path).read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return None
    if isinstance(raw, dict) and isinstance(raw.get("entries"), list):
        return raw["entries"]
    return None


def verify_stage(stage_dir, shippable=False) -> list:
    """Return the R1-R9 problems of a stage. An empty list means every rule passes.

    shippable=True also flags any ledger row marked local_only for a stage file (R9).
    """
    try:
        stage = kp.refuse_game_dir(stage_dir)
    except kp.PackError as exc:
        return ["R9: %s" % exc]
    override = stage / "Override"
    if override.is_symlink():
        return ["R9: Override is a symlink in the stage; refusing to follow it"]
    if not override.is_dir():
        return ["R9: no Override dir in stage %s" % stage]

    problems = []
    clips = {}  # resref -> {".wav": Path, ".lip": Path}
    with os.scandir(override) as it:
        entries = sorted(it, key=lambda e: e.name)
    for ent in entries:
        if ent.is_symlink() or not ent.is_file(follow_symlinks=False):
            problems.append("R9: %s is not a regular file (symlinks are not followed)" % ent.name)
            continue
        stem, ext = os.path.splitext(ent.name)
        if ext not in (WAV_EXT, LIP_EXT):
            problems.append("R9: unexpected file %s in Override" % ent.name)
            continue
        clips.setdefault(stem, {})[ext] = Path(ent.path)
    if not clips:
        problems.append("R9: no clips in Override")

    expected = {}  # "Override/<name>" -> Path
    for stem in sorted(clips):
        files = clips[stem]
        if not kp.RESREF_RE.fullmatch(stem):
            problems.append("R9: %s: resref must be kpv-prefixed [a-z0-9_]{1,16}" % stem)
        if WAV_EXT not in files:
            problems.append("R9: %s.lip has no paired .wav" % stem)
        if LIP_EXT not in files:
            problems.append("R9: %s.wav has no paired .lip" % stem)
        for ext, path in files.items():
            expected["Override/" + stem + ext] = path

        frames = None
        if WAV_EXT in files:
            data = files[WAV_EXT].read_bytes()
            wav_probs = verify_wav(data)
            problems += [_tag(stem + WAV_EXT, p) for p in wav_probs]
            if not any(p.startswith(("R1:", "R4:")) for p in wav_probs):
                frames = kc.walk_frames(data, HDR)[0]
        if LIP_EXT in files:
            lip_bytes = files[LIP_EXT].read_bytes()
            lip_probs = L.check_lip(lip_bytes)
            problems += ["R7: %s.lip: %s" % (stem, p) for p in lip_probs]
            if frames is not None:
                try:
                    length, _keys = L.parse_lip(lip_bytes)
                except ValueError:
                    length = None  # already reported under R7
                if length is not None:
                    duration = kc.expected_pcm_samples(frames) / kc.SR
                    if abs(length - duration) > LIP_TOL_S + LIP_EPS_S:
                        problems.append("R8: %s.lip: length %.4f s differs from decoded %.4f s by more than 36 ms"
                                        % (stem, length, duration))

    mpath = stage / "manifest.json"
    if not mpath.is_file():
        problems.append("R9: manifest.json missing")
    else:
        try:
            listed = kp.read_manifest(stage)["files"]
        except kp.PackError as exc:
            problems.append("R9: %s" % exc)
        else:
            for rel, path in sorted(expected.items()):
                if rel not in listed:
                    problems.append("R9: %s not listed in manifest.json" % rel)
                elif listed[rel] != _sha_file(path):
                    problems.append("R9: %s sha256 differs from manifest.json" % rel)
            for rel in sorted(set(listed) - set(expected)):
                problems.append("R9: manifest.json lists %s but no such file is in the stage" % rel)

    rows = _read_rows(stage / "ledger.json") if (stage / "ledger.json").is_file() else None
    if rows is None:
        problems.append("R9: ledger.json missing or not a schema-1 ledger with an entries list")
    else:
        for rel, path in sorted(expected.items()):
            name = os.path.basename(rel)
            matches = [r for r in rows if isinstance(r, dict) and os.path.basename(str(r.get("file", ""))) == name]
            if not matches:
                problems.append("R9: %s has no ledger.json row" % rel)
            digest = _sha_file(path)
            for row in matches:
                if row.get("sha256") and row["sha256"] != digest:
                    problems.append("R9: %s ledger sha256 differs from the file" % rel)
                if shippable and row.get("local_only"):
                    problems.append("R9: %s is local_only in a shippable stage" % rel)
    return problems
