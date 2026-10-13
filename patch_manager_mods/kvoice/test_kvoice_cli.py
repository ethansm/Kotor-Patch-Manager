#!/usr/bin/env python3
"""Tests for kvoice.py (the kvoice CLI): clip -> verify round trip, refusals, probe, measure, ingest, ledger.

Synthetic sine tones and silence only, built in tempdirs. The game dir is used as a PATH STRING or as a fake dir
inside a tempdir (KVOICE_K2_DIR); no test writes to the real game dir. Needs ffmpeg.
The vanilla round-trip control runs only when KVOICE_VANILLA_VO58 names a vanilla .wav; otherwise it is SKIPPED.
Run: python3 -u test_kvoice_cli.py   (exit 0 = no FAIL)
"""
import json
import math
import os
import re
import struct
import subprocess
import sys
import tempfile
import wave
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kvoice_audio as ka  # noqa: E402
import kvoice_container as kc  # noqa: E402
import kvoice_lip as L  # noqa: E402
import kvoice_norm as kn  # noqa: E402
import kvoice_verify as kv  # noqa: E402

CLI = HERE / "kvoice.py"
SRC_ID = "vanilla_k2"  # registered; kvoice clips derive from vanilla K2 audio, so rows are local and personal only
GAME_STAGE_DEFAULT = "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/kvoice_cli_stage"
TESTS = []


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def run_cli(*args, env=None):
    e = {k: v for k, v in os.environ.items() if k != "KVOICE_K2_DIR"}
    e.update(env or {})
    return subprocess.run([sys.executable, "-u", str(CLI), *map(str, args)], capture_output=True,
                          text=True, encoding="utf-8", errors="replace", env=e, timeout=300)


def write_wav(path, samples, sr=32000, ch=1):
    """samples: int16 values, interleaved when ch == 2."""
    with wave.open(str(path), "wb") as w:
        w.setnchannels(ch)
        w.setsampwidth(2)
        w.setframerate(sr)
        w.writeframes(struct.pack("<%dh" % len(samples), *samples))


def tone(seconds, sr=32000, ch=1, freq=440.0, amp=0.3):
    out = []
    for i in range(int(seconds * sr)):
        v = int(amp * 32767 * math.sin(2 * math.pi * freq * i / sr))
        out.extend([v] * ch)
    return out


def silence(seconds, sr=32000):
    return [0] * int(seconds * sr)


def make_stage(td, container="vo58", resref="kpvtone01", lip_tsv=None):
    """Clip a 1 s tone into td/stage. Returns the stage path. td must be a Path."""
    td.mkdir(parents=True, exist_ok=True)
    src = td / "tone.wav"
    if not src.exists():
        write_wav(src, tone(1.0))
    stage = td / "stage"
    args = ["clip", src, "--stage", stage, "--resref", resref, "--source-id", SRC_ID, "--container", container]
    args += ["--lip-tsv", lip_tsv] if lip_tsv else ["--no-lip"]
    r = run_cli(*args)
    check(r.returncode == 0, "clip failed rc=%d: %s" % (r.returncode, r.stderr[-400:]))
    return stage


@test
def test_clip_verify_round_trip():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t1_") as td:
        td = Path(td)
        src = td / "tone.wav"
        write_wav(src, tone(1.0))
        tsv = td / "rhubarb.tsv"
        tsv.write_text("0.00\tX\n0.20\tA\n0.40\tX\n", encoding="utf-8")
        stage = td / "stage"
        r = run_cli("clip", src, "--stage", stage, "--resref", "kpvtone01", "--lip-tsv", tsv,
                    "--source-id", SRC_ID)
        check(r.returncode == 0, "clip rc=%d: %s" % (r.returncode, r.stderr[-400:]))
        wav = stage / "Override" / "kpvtone01.wav"
        lip_path = stage / "Override" / "kpvtone01.lip"
        check(wav.is_file() and lip_path.is_file(), "clip wrote no wav or lip")

        v = run_cli("verify", stage)
        check(v.returncode == 0, "verify rc=%d: %s %s" % (v.returncode, v.stdout, v.stderr[-300:]))
        check(not [ln for ln in v.stdout.splitlines() if ln.startswith("R")], "verify printed problems: %r" % v.stdout)

        dec_s = len(ka.decode_pcm(kc.strip(wav.read_bytes()))) / 2 / kc.SR
        delta = abs(dec_s - (1.0 + kn.LEAD_IN_S))
        check(delta <= 0.036, "decoded %.4f s vs source+lead-in %.4f s: delta %.4f s > 36 ms"
              % (dec_s, 1.0 + kn.LEAD_IN_S, delta))

        lip = lip_path.read_bytes()
        check(L.check_lip(lip) == [], "lip rejected: %r" % L.check_lip(lip))
        length, keys = L.parse_lip(lip)
        check(abs(length - dec_s) <= 0.036, "lip length %.4f s vs decoded %.4f s" % (length, dec_s))
        check(any(abs(t - 0.35) < 1e-4 and s == 11 for t, s in keys),
              "lead-in offset not applied: cue 0.20 s (A=11) should sit at 0.35 s, keys=%r" % keys)

        led = json.loads((stage / "ledger.json").read_text(encoding="utf-8"))
        check(len(led["entries"]) == 2, "ledger must hold one row per Override file: %r" % led["entries"])
        row = next(e for e in led["entries"] if e["file"].endswith("kpvtone01.wav"))
        check(row["ship_mode"] == "local" and row["source_id"] == SRC_ID and row["container"] == "vo58",
              "ledger row fields: %r" % row)
        check(row["local_only"] is True and row["personal_only"] is True,
              "vanilla source must force local_only and personal_only: %r" % row)
        lip_row = next(e for e in led["entries"] if e["file"].endswith("kpvtone01.lip"))
        check(lip_row["container"] == "none" and lip_row["ship_mode"] == "local",
              "lip ledger row fields: %r" % lip_row)


@test
def test_clip_silent_refused():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t2_") as td:
        td = Path(td)
        src = td / "silent.wav"
        write_wav(src, silence(1.0))
        stage = td / "stage"
        r = run_cli("clip", src, "--stage", stage, "--resref", "kpvsil01", "--no-lip", "--source-id", SRC_ID)
        check(r.returncode == 2, "silent clip must exit 2, got %d: %s" % (r.returncode, r.stderr[-300:]))
        check("zero" in r.stderr.lower(), "stderr should name the silence: %r" % r.stderr)
        check(not (stage / "Override").exists(), "a refused silent clip must write nothing")


@test
def test_clip_bare_flags_R1():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t3_") as td:
        stage = make_stage(Path(td), container="bare", resref="kpvbare01")
        v = run_cli("verify", stage)
        check(v.returncode == 1, "bare clip must fail verify, rc=%d" % v.returncode)
        check(any(ln.startswith("R1:") and "kpvbare01.wav" in ln for ln in v.stdout.splitlines()),
              "verify must print an R1 line for the bare wav: %r" % v.stdout)
        led = json.loads((stage / "ledger.json").read_text(encoding="utf-8"))
        wav_row = next(e for e in led["entries"] if e["file"].endswith("kpvbare01.wav"))
        check(wav_row["container"] == "bare", "ledger must record container bare")


@test
def test_clip_refuses_game_dir():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t4_") as td:
        td = Path(td)
        src = td / "tone.wav"
        write_wav(src, tone(1.0))
        gd = td / "gamedir"
        gd.mkdir()
        r = run_cli("clip", src, "--stage", gd / "stage", "--resref", "kpvgame01", "--no-lip",
                    "--source-id", SRC_ID, env={"KVOICE_K2_DIR": str(gd)})
        check(r.returncode == 2 and "refusing" in r.stderr,
              "stage under KVOICE_K2_DIR must be refused: rc=%d %r" % (r.returncode, r.stderr[-300:]))
        check(not (gd / "stage").exists(), "refused stage must not be created")

    default_stage = Path(GAME_STAGE_DEFAULT).expanduser()
    existed = default_stage.exists()
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t4b_") as td:
        src = Path(td) / "tone.wav"
        write_wav(src, tone(1.0))
        r = run_cli("clip", src, "--stage", GAME_STAGE_DEFAULT, "--resref", "kpvgame02", "--no-lip",
                    "--source-id", SRC_ID)
    check(r.returncode == 2 and "refusing" in r.stderr,
          "default game dir stage must be refused: rc=%d %r" % (r.returncode, r.stderr[-300:]))
    check(default_stage.exists() == existed, "refusal must not create anything under the default game dir")


@test
def test_clip_bad_inputs_refused():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t5_") as td:
        td = Path(td)
        src = td / "tone.wav"
        write_wav(src, tone(1.0))
        stage = td / "stage"
        r = run_cli("clip", src, "--stage", stage, "--resref", "notkpv", "--no-lip", "--source-id", SRC_ID)
        check(r.returncode == 2 and not stage.exists(), "non-kpv resref must be refused, rc=%d" % r.returncode)
        r = run_cli("clip", src, "--stage", stage, "--resref", "kpvunk01", "--no-lip",
                    "--source-id", "not_registered")
        check(r.returncode == 2 and "unregistered" in r.stderr and not stage.exists(),
              "unregistered source must be refused before any write: rc=%d %r" % (r.returncode, r.stderr[-200:]))
        r = run_cli("clip", src, "--stage", stage, "--resref", "kpvnosrc", "--no-lip")
        check(r.returncode == 2, "clip without --source-id must exit 2, rc=%d" % r.returncode)


@test
def test_probe_header_containers():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t6_") as td:
        td = Path(td)
        stage = make_stage(td / "vo", container="vo58")
        wav = stage / "Override" / "kpvtone01.wav"
        r = run_cli("probe-header", wav)
        check(r.returncode == 0 and "container: vo58" in r.stdout, "vo58 probe: rc=%d %r" % (r.returncode, r.stdout))
        frames = int(re.search(r"frames: (\d+)", r.stdout).group(1))
        check(frames == (wav.stat().st_size - kc.HDR_LEN) // kc.FRAME_LEN,
              "probe frame count %d does not match file size" % frames)
        check("decoded_s:" in r.stdout and "I:" in r.stdout, "probe output incomplete: %r" % r.stdout)

        stage_b = make_stage(td / "bare", container="bare", resref="kpvbare02")
        r = run_cli("probe-header", stage_b / "Override" / "kpvbare02.wav")
        check(r.returncode == 0 and "container: bare" in r.stdout, "bare probe: rc=%d %r" % (r.returncode, r.stdout))

        junk = td / "junk.wav"
        junk.write_bytes(b"not audio at all " * 20)
        r = run_cli("probe-header", junk)
        check(r.returncode == 1 and "container: other" in r.stdout, "junk probe: rc=%d %r" % (r.returncode, r.stdout))


@test
def test_measure_and_ingest():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t7_") as td:
        td = Path(td)
        stage = make_stage(td, container="vo58")
        r = run_cli("measure", stage / "Override" / "kpvtone01.wav")
        m = re.search(r"I: (-?[\d.]+) LUFS", r.stdout)
        check(r.returncode == 0 and m, "measure failed: rc=%d %r" % (r.returncode, r.stdout))
        check(abs(float(m.group(1)) - kn.TARGET_LUFS) <= kn.TOL_LU + 0.05,
              "clip loudness %s not at target %.1f" % (m.group(1), kn.TARGET_LUFS))

        stereo = td / "stereo.wav"
        write_wav(stereo, tone(1.0, sr=44100, ch=2), sr=44100, ch=2)
        out = td / "ing"
        r = run_cli("ingest", stereo, "--out", out)
        check(r.returncode == 0, "ingest rc=%d: %s" % (r.returncode, r.stderr[-300:]))
        with wave.open(str(out / "stereo.wav"), "rb") as w:
            check(w.getframerate() == kc.SR and w.getnchannels() == kc.CH and w.getsampwidth() == 2,
                  "ingest output must be s16le mono 32 kHz")
            check(abs(w.getnframes() - kc.SR) <= 64, "ingest duration off: %d frames" % w.getnframes())

        gd = td / "gamedir"
        gd.mkdir()
        r = run_cli("ingest", stereo, "--out", gd / "scratch", env={"KVOICE_K2_DIR": str(gd)})
        check(r.returncode == 2 and not (gd / "scratch").exists(), "ingest into the game dir must be refused")

        before = stereo.read_bytes()
        r = run_cli("ingest", stereo, "--out", td)
        check(r.returncode == 2 and stereo.read_bytes() == before,
              "ingest must refuse to overwrite its own source: rc=%d" % r.returncode)


@test
def test_verify_failures_and_file_flag():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t8_") as td:
        td = Path(td)
        r = run_cli("verify", td / "nope")
        check(r.returncode == 1 and any(ln.startswith("R9:") for ln in r.stdout.splitlines()),
              "missing stage must report R9: rc=%d %r" % (r.returncode, r.stdout))
        stage = make_stage(td / "good", container="vo58")
        bare = make_stage(td / "bare", container="bare", resref="kpvbare03")
        r = run_cli("verify", stage, "--file", bare / "Override" / "kpvbare03.wav")
        check(r.returncode == 1 and any(ln.startswith("R1: kpvbare03.wav:") for ln in r.stdout.splitlines()),
              "--file must tag problems with the file name: rc=%d %r" % (r.returncode, r.stdout))


@test
def test_ledger_summary():
    with tempfile.TemporaryDirectory(prefix="kvoice_cli_t9_") as td:
        td = Path(td)
        stage = make_stage(td, container="vo58")
        r = run_cli("ledger", stage)
        check(r.returncode == 0 and "rows: 2, flagged: 0" in r.stdout and "source=vanilla_k2" in r.stdout
              and "ship_mode=local" in r.stdout, "ledger summary: rc=%d %r" % (r.returncode, r.stdout))
        empty = td / "empty"
        empty.mkdir()
        r = run_cli("ledger", empty)
        check(r.returncode == 1, "a stage without ledger.json must exit 1, rc=%d" % r.returncode)


@test
def test_help_states_limits():
    r = run_cli("--help")
    check(r.returncode == 0, "--help rc=%d" % r.returncode)
    for phrase in ("structure only", "No auto-install into the game", "E1-E8"):
        check(phrase in r.stdout, "--help must state %r" % phrase)


@test
def test_vanilla_round_trip_control():
    path = os.environ.get("KVOICE_VANILLA_VO58")
    if not path:
        raise Skip("KVOICE_VANILLA_VO58 not set (point it at a vanilla .wav to run the control)")
    check(Path(path).is_file(), "KVOICE_VANILLA_VO58 is not a file: %s" % path)
    r = run_cli("probe-header", path)
    check(r.returncode == 0 and "container: vo58" in r.stdout, "vanilla probe: rc=%d %r" % (r.returncode, r.stdout))
    check(kv.verify_wav(Path(path).read_bytes()) == [], "vanilla clip must pass R1-R6")


def main():
    passed = skipped = 0
    for fn in TESTS:
        try:
            fn()
        except Skip as exc:
            print("SKIP %s: %s" % (fn.__name__, exc))
            skipped += 1
        except Exception as exc:  # AssertionError or anything unexpected
            print("FAIL %s: %s: %s" % (fn.__name__, type(exc).__name__, exc))
        else:
            print("PASS %s" % fn.__name__)
            passed += 1
    total = len(TESTS) - skipped
    print("%d/%d PASS, %d SKIP" % (passed, total, skipped))
    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
