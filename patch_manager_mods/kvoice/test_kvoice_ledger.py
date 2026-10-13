#!/usr/bin/env python3
"""Tests for kvoice_ledger.py. Tempdirs and temp ledgers only; the default ledger is never touched.

The prune gate checks run k1_port_prune.py gate under python -I with --ledger (and K1PORT_LEDGER) in the tempdir.
Run: python3 -u test_kvoice_ledger.py   (cwd may be anything; exit 0 = all PASS)
"""
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
K1PORT = HERE.parent / "k1_port"
PRUNE_CLI = K1PORT / "k1_port_prune.py"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
os.environ.pop("K1PORT_SOURCES", None)
os.environ.pop("K1PORT_LEDGER", None)
import kvoice_ledger as kl  # noqa: E402
import k1_port_sources as ks  # noqa: E402

TESTS = []
VANILLA = "vanilla_k2"
FREE_SRC = "leilukin_k1_twilek_f_2179"  # credit_required, no author approval


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def write(path: Path, data: bytes) -> str:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    return str(path.resolve())


def new_ledger(root: Path):
    led = ks.Ledger(root / "stage" / "ledger.json")
    check(str(led.path).startswith(str(root)), f"ledger must be temp, got {led.path}")
    return led


def row(led, clip, **kw):
    base = dict(container="vo58", source_resref="n_test", slot=1, ship_mode="personal")
    base.update(kw)
    return kl.row_for_clip(led, clip, **base)


@test
def test_vanilla_direct_forces_personal_and_local(root: Path):
    led = new_ledger(root)
    a = write(root / "stage" / "kvoice" / "n_a_01.wav", b"clip-a")
    r = row(led, a, source_id=VANILLA)
    check(r["personal_only"] is True and r["local_only"] is True, f"default flags: {r}")
    b = write(root / "stage" / "kvoice" / "n_a_02.wav", b"clip-b")
    r2 = row(led, b, source_id=VANILLA, personal_only=False, local_only=False)
    check(r2["personal_only"] is True and r2["local_only"] is True, f"explicit False not forced: {r2}")


@test
def test_vanilla_derived_forces_personal(root: Path):
    led = new_ledger(root)
    parent = write(root / "stage" / "src" / "p.wav", b"parent-bytes")
    row(led, parent, source_id=VANILLA)
    psha = kl.sha256_of(parent)
    out = write(root / "stage" / "out" / "o.wav", b"derived-bytes")
    r = row(led, out, parent_sha256=psha)
    check(r["derived"] is True and r["source_id"] == VANILLA, f"derived of vanilla: {r}")
    check(r["parent_sha256"] == psha, "parent sha256 must be recorded")
    check(r["personal_only"] is True and r["local_only"] is True, f"vanilla derived not forced: {r}")


@test
def test_non_vanilla_direct_not_forced(root: Path):
    led = new_ledger(root)
    clip = write(root / "stage" / "kvoice" / "n_x_01.wav", b"free-clip")
    r = row(led, clip, source_id=FREE_SRC, ship_mode="release")
    check(r["personal_only"] is False, f"non-vanilla personal_only forced: {r}")
    check(r["local_only"] is False, f"non-vanilla local_only forced: {r}")


@test
def test_non_vanilla_derived_not_forced(root: Path):
    led = new_ledger(root)
    parent = write(root / "stage" / "src" / "f.wav", b"free-parent")
    row(led, parent, source_id=FREE_SRC)
    out = write(root / "stage" / "out" / "f.wav", b"free-derived")
    r = row(led, out, parent_sha256=kl.sha256_of(parent))
    check(r["source_id"] == FREE_SRC, f"derived must inherit parent source: {r}")
    check(r["personal_only"] is False, f"non-vanilla derived personal_only forced: {r}")


@test
def test_kind_audio_accepted(root: Path):
    led = new_ledger(root)
    clip = write(root / "stage" / "kvoice" / "n_k_01.wav", b"kind-clip")
    r = row(led, clip, source_id=VANILLA)
    check(r["kind"] == "audio", f"kind must be audio, got {r['kind']}")
    other = write(root / "stage" / "kvoice" / "n_k_02.wav", b"kind-bad")
    try:
        row(led, other, source_id=VANILLA, kind="sound")
    except ValueError:
        return
    raise AssertionError("kind outside KINDS must be rejected")


@test
def test_container_values(root: Path):
    led = new_ledger(root)
    for i, c in enumerate(("vo58", "bare", "none")):
        clip = write(root / "stage" / "kvoice" / f"c_{i}.wav", f"c-{c}".encode())
        r = row(led, clip, source_id=VANILLA, container=c)
        check(r["container"] == c, f"container {c} not recorded")
    bad = write(root / "stage" / "kvoice" / "c_bad.wav", b"c-bad")
    try:
        row(led, bad, source_id=VANILLA, container="mp3")
    except ValueError:
        return
    raise AssertionError("container outside CONTAINERS must be rejected")


@test
def test_source_xor_parent(root: Path):
    led = new_ledger(root)
    clip = write(root / "stage" / "kvoice" / "x.wav", b"xor")
    for kw in ({"source_id": VANILLA, "parent_sha256": "ab" * 32}, {}):
        try:
            row(led, clip, **kw)
        except ValueError:
            continue
        raise AssertionError(f"source_id/parent_sha256 must be exactly one: {kw}")


@test
def test_derived_fails_closed_without_parent(root: Path):
    led = new_ledger(root)
    clip = write(root / "stage" / "kvoice" / "orphan.wav", b"orphan")
    try:
        row(led, clip, parent_sha256="00" * 32)
    except ks.SourceError:
        return
    raise AssertionError("derived clip with unknown parent sha256 must fail closed")


@test
def test_fields_recorded(root: Path):
    led = new_ledger(root)
    data = b"fields-clip"
    clip = write(root / "stage" / "kvoice" / "n_f_07.wav", data)
    r = row(led, clip, source_id=VANILLA, container="bare", source_resref="n_fields", slot=7, ship_mode="personal")
    check(r["sha256"] == hashlib.sha256(data).hexdigest(), "sha256 must match clip bytes")
    check(r["container"] == "bare", "container not recorded")
    check(r["source_resref"] == "n_fields", "source resref not recorded")
    check(r["slot"] == 7, "slot not recorded")
    check(r["ship_mode"] == "personal", "ship_mode not recorded")
    check(r["file"] == clip, f"file must be the resolved clip path: {r['file']}")


def run_gate(ledger: Path, *args):
    env = dict(os.environ)
    env.pop("K1PORT_SOURCES", None)
    env["K1PORT_LEDGER"] = str(ledger)
    cmd = [sys.executable, "-I", str(PRUNE_CLI), "gate", "--ledger", str(ledger), *args]
    return subprocess.run(cmd, capture_output=True, text=True, env=env, timeout=120)


@test
def test_prune_gate_passes_registered_wav(root: Path):
    led = new_ledger(root)
    stage = root / "stage"
    clip = write(stage / "kvoice" / "n_g_01.wav", b"gate-registered")
    row(led, clip, source_id=VANILLA)
    led.save()
    r = run_gate(led.path, "--strict", str(stage))
    check(r.returncode == 0, f"registered wav must pass gate, rc {r.returncode}: {r.stdout} {r.stderr}")


@test
def test_prune_gate_blocks_unregistered_wav(root: Path):
    led = new_ledger(root)
    stage = root / "stage"
    clip = write(stage / "kvoice" / "n_g_02.wav", b"gate-registered-2")
    row(led, clip, source_id=VANILLA)
    led.save()
    stray = write(stage / "kvoice" / "stray.wav", b"gate-stray-not-in-ledger")
    r = run_gate(led.path, "--strict", str(stage))
    check(r.returncode == 1, f"unregistered wav must block gate, rc {r.returncode}: {r.stdout} {r.stderr}")
    check("unregistered asset" in r.stderr and stray in r.stderr,
          f"gate must name the stray wav: {r.stderr}")


def main() -> int:
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="kvoice_ledger_test_")).resolve()
        try:
            fn(root)
            print(f"PASS {fn.__name__}")
            passed += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
        finally:
            shutil.rmtree(root, ignore_errors=True)
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
