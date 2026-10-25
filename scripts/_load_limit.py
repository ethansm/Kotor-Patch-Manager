"""Machine-wide load limits shared by the install/test tooling (run_tests, pre_install_gate, build_check, kpm_hotswap).

Several sessions run gates at once on one 12-core / 31 GB box; each used to start one compiler per core, and an
ASan+UBSan test build alone takes ~450 MB, so two gates swapped the machine to a halt. Everything here coordinates
through flock files under $TMPDIR/kotor-tests/locks/, so the limits hold across processes, threads and worktrees.

  heavy_slot()        one of N machine-wide slots for a compile (or other CPU/RAM-heavy step); also waits while
                      MemAvailable is under the reserve. N = $KOTOR_HEAVY_SLOTS or max(2, cpu_count - 2).
  exclusive(name)     one holder machine-wide (wine prefix, create-patch.py, one build key dir).
  lower_priority()    nice 10 + idle I/O class for this process and its children ($KOTOR_NICE=0 turns it off).

Env: KOTOR_HEAVY_SLOTS, KOTOR_MEM_RESERVE_MB (default 2048), KOTOR_MEM_WAIT_S (default 300), KOTOR_NICE.
"""
import contextlib
import fcntl
import os
import subprocess
import sys
import time
from pathlib import Path

TMP = Path(os.environ.get("TMPDIR") or f"/tmp/claude-{os.getuid()}") / "kotor-tests"
LOCKS = TMP / "locks"


def slot_count() -> int:
    env = os.environ.get("KOTOR_HEAVY_SLOTS", "")
    if env.isdigit() and int(env) > 0:
        return int(env)
    return max(2, (os.cpu_count() or 4) - 2)


def mem_available_mb() -> int:
    """MemAvailable from /proc/meminfo in MB; a large number when it cannot be read (never blocks off Linux)."""
    try:
        with open("/proc/meminfo") as f:
            for line in f:
                if line.startswith("MemAvailable:"):
                    return int(line.split()[1]) // 1024
    except OSError:
        pass
    return 1 << 30


def _wait_for_memory():
    reserve = int(os.environ.get("KOTOR_MEM_RESERVE_MB") or 2048)
    limit = float(os.environ.get("KOTOR_MEM_WAIT_S") or 300)
    t0, warned = time.monotonic(), False
    while mem_available_mb() < reserve:
        if time.monotonic() - t0 > limit:
            print(f"WARN  load_limit: MemAvailable under {reserve} MB for {limit:.0f}s; going ahead", file=sys.stderr)
            return
        if not warned:
            print(f"NOTE  load_limit: waiting for MemAvailable >= {reserve} MB", file=sys.stderr, flush=True)
            warned = True
        time.sleep(1.0)


def _open_lock(name: str):
    LOCKS.mkdir(parents=True, exist_ok=True)
    return open(LOCKS / f"{name}.lock", "a+")


@contextlib.contextmanager
def heavy_slot():
    """Hold one of slot_count() machine-wide slots for the body. Each call opens its own file, so threads of one
    process compete for slots like separate processes do (flock is per open file description)."""
    n = slot_count()
    delay = 0.05
    while True:
        _wait_for_memory()
        for i in range(n):
            f = _open_lock(f"slot.{i}")
            try:
                fcntl.flock(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except OSError:
                f.close()
                continue
            try:
                yield
            finally:
                fcntl.flock(f, fcntl.LOCK_UN)
                f.close()
            return
        time.sleep(delay)
        delay = min(delay * 2, 0.5)


@contextlib.contextmanager
def exclusive(name: str, label: str = ""):
    """Hold the machine-wide lock `name` for the body; prints once if another holder makes us wait."""
    f = _open_lock(name)
    try:
        try:
            fcntl.flock(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError:
            f.seek(0)
            holder = f.read().strip()
            print(f"NOTE  waiting for the {label or name} lock (held by {holder or 'another run'})",
                  file=sys.stderr, flush=True)
            fcntl.flock(f, fcntl.LOCK_EX)
        f.seek(0)
        f.truncate()
        f.write(f"pid {os.getpid()}\n")
        f.flush()
        yield
    finally:
        fcntl.flock(f, fcntl.LOCK_UN)
        f.close()


_lowered = False


def lower_priority():
    """nice +10 and idle I/O for this process (children inherit). Idempotent; $KOTOR_NICE=0 disables."""
    global _lowered
    if _lowered or os.environ.get("KOTOR_NICE") == "0":
        return
    _lowered = True
    try:
        os.nice(10)
    except OSError:
        pass
    try:
        subprocess.run(["ionice", "-c", "3", "-p", str(os.getpid())],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except OSError:
        pass
    os.environ["KOTOR_NICE"] = "0"   # children are already lowered; do not add another +10
