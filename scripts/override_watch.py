#!/usr/bin/env python3
"""Zero-install file-open observer for KOTOR II audio/resource lookups (inotify, stdlib ctypes, Linux only).

Logs every OPEN of a file whose name matches --match in the watched dirs, so a live test shows WHICH paths the
engine tried (Override/kvjv0001.wav? StreamVoice/kvjv0001.wav? .lip? .ssf?) without a native hook.
Run it BEFORE launching the game, leave it running, read the summary after:

    python3 scripts/override_watch.py --match 'kvj|kpv' --log /tmp/kvj_watch.log            # Ctrl-C prints a summary
    python3 scripts/override_watch.py --match kvj --seconds 600 --log w.log &               # timed

Default dirs under the K2 install: override, StreamVoice, StreamSounds, lips. Only files that already exist or
are created while watching can be seen; a lookup that finds no file leaves no event (the absence of an OPEN for
a staged name is itself the result). Read-only: it never touches the game files.
Output line: `<HH:MM:SS.mmm> OPEN <dir>/<name>`; the summary counts OPENs per path.
"""
import argparse
import collections
import ctypes
import ctypes.util
import os
import re
import select
import struct
import sys
import time
from pathlib import Path

DEFAULT_GAME = "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
DEFAULT_DIRS = ("override", "StreamVoice", "StreamSounds", "lips")
IN_ACCESS, IN_OPEN, IN_CREATE, IN_MOVED_TO = 0x1, 0x20, 0x100, 0x80
EVENT = struct.Struct("iIII")


def parse_events(buf):
    """Yield (wd, mask, name) from a raw inotify read buffer."""
    off = 0
    while off + EVENT.size <= len(buf):
        wd, mask, _cookie, ln = EVENT.unpack_from(buf, off)
        name = buf[off + EVENT.size:off + EVENT.size + ln].split(b"\0", 1)[0].decode("latin1")
        off += EVENT.size + ln
        yield wd, mask, name


def watch(dirs, match, seconds=None, log=None, out=sys.stdout):
    """Watch dirs until seconds elapse or KeyboardInterrupt. Returns Counter of 'dir/name' -> OPEN count."""
    libc = ctypes.CDLL(ctypes.util.find_library("c") or "libc.so.6", use_errno=True)
    fd = libc.inotify_init1(os.O_NONBLOCK)
    if fd < 0:
        raise OSError(ctypes.get_errno(), "inotify_init1 failed")
    wds = {}
    for d in dirs:
        wd = libc.inotify_add_watch(fd, os.fsencode(str(d)), IN_OPEN | IN_CREATE | IN_MOVED_TO)
        if wd < 0:
            print(f"skip {d}: {os.strerror(ctypes.get_errno())}", file=sys.stderr)
            continue
        wds[wd] = Path(d).name
    rx = re.compile(match, re.IGNORECASE)
    counts = collections.Counter()
    end = None if seconds is None else time.monotonic() + seconds
    fh = open(log, "a") if log else None
    try:
        while end is None or time.monotonic() < end:
            if not select.select([fd], [], [], 0.5)[0]:
                continue
            for wd, mask, name in parse_events(os.read(fd, 65536)):
                if not name or not rx.search(name):
                    continue
                kind = "OPEN" if mask & IN_OPEN else "CREATE"
                path = f"{wds.get(wd, wd)}/{name}"
                if kind == "OPEN":
                    counts[path] += 1
                t = time.time()
                line = f"{time.strftime('%H:%M:%S', time.localtime(t))}.{int(t % 1 * 1000):03d} {kind} {path}"
                print(line, file=out, flush=True)
                if fh:
                    fh.write(line + "\n")
                    fh.flush()
    except KeyboardInterrupt:
        pass
    finally:
        os.close(fd)
        if fh:
            fh.close()
    return counts


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--game-dir", default=DEFAULT_GAME)
    ap.add_argument("--dirs", nargs="+", default=list(DEFAULT_DIRS), help="subdirs of the game dir to watch")
    ap.add_argument("--match", default="kvj|kpv", help="regex on the file name (case-insensitive)")
    ap.add_argument("--seconds", type=float, default=None, help="stop after N seconds (default: until Ctrl-C)")
    ap.add_argument("--log", default=None, help="append lines to this file")
    a = ap.parse_args(argv)
    game = Path(a.game_dir).expanduser()
    dirs = [game / d for d in a.dirs if (game / d).is_dir()]
    if not dirs:
        print(f"error: none of {a.dirs} exist under {game}", file=sys.stderr)
        return 2
    print(f"watching {', '.join(d.name for d in dirs)} for /{a.match}/ (Ctrl-C = summary)", file=sys.stderr)
    counts = watch(dirs, a.match, a.seconds, a.log)
    print(f"summary: {sum(counts.values())} OPEN(s) on {len(counts)} path(s)")
    for path, n in counts.most_common():
        print(f"  {n:4d}  {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
