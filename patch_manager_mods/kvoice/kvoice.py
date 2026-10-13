#!/usr/bin/env python3
"""kvoice command line: probe, measure, ingest, clip, verify and ledger for K2 voice clips (doc 41 V-B, task 4b).

Commands:
  probe-header <file.wav>         container (vo58 / bare / other), MP3 frame count, decoded duration, loudness
  measure <file>                  EBU R128 integrated loudness (LUFS) and true peak (dBTP)
  ingest <src_audio> --out DIR    decode any ffmpeg-readable audio to s16le mono 32 kHz (PCM WAV, scratch only)
  clip <src_audio> --stage DIR --resref kpvNNN (--lip-tsv FILE | --no-lip) --source-id ID
                                  normalize to -18.5 LUFS, wrap (vo58) or keep bare, build lip, pack into the
                                  stage, add a ledger row (ship_mode local) to <stage>/ledger.json
  verify <stage> [--shippable] [--file x.wav]    structure checks R1-R9; exit 1 on any problem
  ledger <stage>                  one line per ledger row of <stage>/ledger.json

Limits:
  Verifier checks structure only. It does not show that the engine plays a clip.
  No auto-install into the game: output goes to a stage dir, and the game dir is refused.
  Engine behaviour is unverified until the E1-E8 live tests (doc 41) pass.

Exit codes: 0 ok; 1 problems found (verify, probe of a non-voice file); 2 refused or failed (usage, game dir,
silent or too-short clip, unregistered source id, unreadable input).
Needs ffmpeg on PATH or at ~/.local/bin/ffmpeg. Otherwise stdlib plus the kvoice_* modules.
"""
import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))
import kvoice_audio as ka  # noqa: E402
import kvoice_container as kc  # noqa: E402
import kvoice_ledger as KL  # noqa: E402
import kvoice_lip as L  # noqa: E402
import kvoice_norm as kn  # noqa: E402
import kvoice_pack as kp  # noqa: E402
import kvoice_verify as kv  # noqa: E402

ks = KL.ks  # k1_port_sources: Ledger, get_source, SourceError
KNOWN_ERRORS = (kp.PackError, kn.NormalizeError, ka.AudioError, kc.ContainerError, ks.SourceError,
                ValueError, OSError)


def _read(path) -> bytes:
    return Path(path).read_bytes()


def _clean(exc) -> str:
    return " ".join(str(exc).split())


def _pcm_seconds(pcm: bytes) -> float:
    return len(pcm) / 2 / kc.SR


def _decode_any(path) -> bytes:
    """Any ffmpeg-readable audio file -> s16le mono 32 kHz PCM bytes."""
    cmd = [ka.ffmpeg_path(), "-hide_banner", "-loglevel", "error", "-i", str(path),
           "-f", "s16le", "-ac", str(kc.CH), "-ar", str(kc.SR), "pipe:1"]
    p = subprocess.run(cmd, capture_output=True)
    if p.returncode != 0 or not p.stdout:
        raise ka.AudioError("decode failed for %s (rc=%d): %s"
                            % (path, p.returncode, p.stderr.decode("latin1")[-300:]))
    return p.stdout


def _measure_payload(payload: bytes):
    """LUFS and dBTP of a bare MP3 payload (written to a temp file that is removed on return)."""
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_") as td:
        path = os.path.join(td, "payload.mp3")
        with open(path, "wb") as fh:
            fh.write(payload)
        return ka.measure(path, "mp3")


def _container_of(b: bytes):
    """Return (container, payload, reason): ('vo58', mp3, None) | ('bare', mp3, None) | ('other', None, reason)."""
    try:
        return "vo58", kc.strip(b), None
    except kc.ContainerError as exc:
        reason = _clean(exc)
    if kc.frame_header(b, 0) is not None:
        return "bare", b, None
    return "other", None, reason


def _tag_file(name, problem) -> str:
    rule, _, msg = problem.partition(": ")
    return "%s: %s: %s" % (rule, name, msg)


def cmd_probe(args) -> int:
    container, payload, reason = _container_of(_read(args.file))
    if container == "other":
        print("container: other (%s)" % reason)
        return 1
    frames = kc.walk_frames(payload, 0)[0]
    decoded_s = _pcm_seconds(ka.decode_pcm(payload))
    I, TP = _measure_payload(payload)
    print("container: %s" % container)
    print("frames: %d" % frames)
    if container == "vo58":
        print("expected_s: %.4f (frame count, LAME gapless trim)" % (kc.expected_pcm_samples(frames) / kc.SR))
    print("decoded_s: %.4f" % decoded_s)
    print("I: %.2f LUFS" % I)
    print("TP: %.2f dBTP" % TP)
    return 0


def cmd_measure(args) -> int:
    container, payload, _reason = _container_of(_read(args.file))
    if container == "other":
        I, TP = ka.measure(args.file)  # ffmpeg probes the format (PCM WAV or other audio)
    else:
        I, TP = _measure_payload(payload)
    print("I: %.2f LUFS" % I)
    print("TP: %.2f dBTP" % TP)
    return 0


def cmd_ingest(args) -> int:
    out = kp.refuse_game_dir(args.out)  # refused before any decode or write
    dest = out / (Path(args.src).stem + ".wav")
    if dest.resolve() == Path(args.src).resolve():
        raise kp.PackError("ingest would overwrite its own source: %s" % dest)
    pcm = _decode_any(args.src)
    out.mkdir(parents=True, exist_ok=True)
    ka.write_pcm_wav(pcm, str(dest))
    print("ingested: %s" % dest)
    print("seconds: %.4f  format: s16le mono %d Hz" % (_pcm_seconds(pcm), kc.SR))
    return 0


def cmd_clip(args) -> int:
    stage = kp.refuse_game_dir(args.stage)  # refused before any decode or write
    kp.check_resref(args.resref)
    ks.get_source(args.source_id)  # unregistered id: refused before any write
    led = ks.Ledger(path=stage / "ledger.json")  # malformed ledger: refused before any write
    pcm = _decode_any(args.src)
    payload, report = kn.normalize_encode(pcm)  # SilentClipError / NormalizeError: refused before any write
    length_s = _pcm_seconds(ka.decode_pcm(payload))
    if args.lip_tsv:
        text = Path(args.lip_tsv).read_text(encoding="utf-8")
        cues = [(t + kn.LEAD_IN_S, s) for t, s in L.rhubarb_to_lip(text)]  # cues follow the lead-in
    else:
        cues = [(0.0, L.SHAPE_REST)]
    lip = L.write_lip(cues, length_s)
    res = kp.pack_clip(stage, args.resref, payload, lip, container=args.container)
    # verify_stage needs a row for every Override file, so the lip gets one too (kind other, container none)
    KL.row_for_clip(led, res["wav"], container=args.container, source_resref=Path(args.src).stem,
                    slot=None, ship_mode="local", source_id=args.source_id, kind="audio")
    KL.row_for_clip(led, res["lip"], container="none", source_resref=Path(args.src).stem,
                    slot=None, ship_mode="local", source_id=args.source_id, kind="other")
    ledger_path = kp.write_ledger(stage, led)
    print("wav: %s" % res["wav"])
    print("lip: %s" % res["lip"])
    print("I=%.2f LUFS TP=%.2f dBTP gain=%.2f dB passes=%d lead_in=%.2f s"
          % (report["I"], report["TP"], report["gain_db"], report["passes"], report["lead_in_s"]))
    print("ledger: %s (source %s, ship_mode local)" % (ledger_path, args.source_id))
    return 0


def cmd_verify(args) -> int:
    problems = kv.verify_stage(args.stage, shippable=args.shippable)
    if args.file:
        name = Path(args.file).name
        problems += [_tag_file(name, p) for p in kv.verify_wav(_read(args.file))]
    for p in problems:
        print(p)
    if problems:
        return 1
    print("OK: 0 problems")
    return 0


def cmd_ledger(args) -> int:
    path = Path(args.stage) / "ledger.json"
    if not path.is_file():
        print("no ledger.json in %s" % args.stage)
        return 1
    led = ks.Ledger(path=path)
    rows = led.entries()
    for e in rows:
        print("%s | source=%s | container=%s | ship_mode=%s | personal_only=%s | local_only=%s | removal_group=%s"
              % (Path(str(e.get("file"))).name, e.get("source_id"), e.get("container"), e.get("ship_mode"),
                 e.get("personal_only"), e.get("local_only"), e.get("removal_group") or "-"))
    print("rows: %d, flagged: %d" % (len(rows), len(led.flagged())))
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(prog="kvoice.py", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True, metavar="COMMAND")

    s = sub.add_parser("probe-header", help="container, frame count, decoded duration and loudness of a clip")
    s.add_argument("file")
    s.set_defaults(func=cmd_probe)

    s = sub.add_parser("measure", help="LUFS and true peak of a clip or audio file")
    s.add_argument("file")
    s.set_defaults(func=cmd_measure)

    s = sub.add_parser("ingest", help="decode any ffmpeg-readable audio to s16le mono 32 kHz (scratch only)")
    s.add_argument("src")
    s.add_argument("--out", required=True, metavar="DIR")
    s.set_defaults(func=cmd_ingest)

    s = sub.add_parser("clip", help="normalize, wrap, lip, pack into a stage, add a ledger row")
    s.add_argument("src")
    s.add_argument("--stage", required=True, metavar="DIR")
    s.add_argument("--resref", required=True, help="kpv-prefixed resref, [a-z0-9_] after kpv, 16 chars max")
    lip = s.add_mutually_exclusive_group(required=True)
    lip.add_argument("--lip-tsv", metavar="FILE", help="Rhubarb TSV (time<TAB>letter); cues are shifted by the lead-in")
    lip.add_argument("--no-lip", action="store_true", help="write a rest-only lip spanning the clip")
    s.add_argument("--container", choices=kp.CONTAINERS, default="vo58",
                   help="vo58 = 58-byte VO header (default); bare = MP3 unchanged (verify flags R1)")
    s.add_argument("--source-id", required=True,
                   help="registered id from k1_port/sources.json; vanilla_k2 for kvoice clips derived from "
                        "vanilla K2 audio (forces personal_only and local_only)")
    s.set_defaults(func=cmd_clip)

    s = sub.add_parser("verify", help="structure checks R1-R9 of a stage; exit 1 on any problem")
    s.add_argument("stage")
    s.add_argument("--shippable", action="store_true", help="also flag ledger rows marked local_only (R9)")
    s.add_argument("--file", metavar="x.wav", help="also check one loose .wav with the R1-R6 rules")
    s.set_defaults(func=cmd_verify)

    s = sub.add_parser("ledger", help="summary of <stage>/ledger.json")
    s.add_argument("stage")
    s.set_defaults(func=cmd_ledger)
    return p


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    try:
        return args.func(args)
    except KNOWN_ERRORS as exc:
        print("error: %s" % _clean(exc), file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
