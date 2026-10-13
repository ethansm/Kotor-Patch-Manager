#!/usr/bin/env python3
"""scripts/dump_game_scripts.py - decompile KOTOR II NCS scripts to NSS text.

Read-only on the game dir. Writes <name>.nss for each ok script plus _index.tsv
(columns: name, bytes, status, error) into --out.

Usage:
  .venv/bin/python scripts/dump_game_scripts.py --out DIR [--names a,b,c | --all] [--game DIR]

Source: the .nss shipped in Scripts.bif is used as-is when present (ground truth); otherwise
pykotor.resource.formats.ncs.decompile_ncs (DeNCS; needs pykotor's dencs package, absent from this venv). Each script runs under a
30 s timeout (SIGALRM, main thread only). No fallback decompiler: a silent empty `void main()`
is worse than an explicit fail row.
"""
from __future__ import annotations

import argparse
import re
import signal
import sys
import threading
from contextlib import contextmanager
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Iterable, Iterator, Sequence

DEFAULT_GAME = Path.home() / '.local/share/Steam/steamapps/common/Knights of the Old Republic II'
DEFAULT_NAMES = (
    'k_inc_walkways', 'k_inc_generic', 'k_def_spawn01', 'k_def_ambient', 'k_def_ambmob',
    'k_def_heartbt01', 'k_ai_master', 'k_def_dialogue01', 'k_def_endconv', 'k_def_attacked01',
    'k_def_death01', 'k_def_userdef01', 'k_def_spn_t_none',
)
TIMEOUT_S = 30.0
INDEX_HEADER = ('name', 'bytes', 'status', 'error')
SAFE_NAME = re.compile(r'[a-z0-9_]+')


class ScriptTimeout(BaseException):
    """BaseException so a decompiler's own `except Exception` cannot swallow the alarm."""


@dataclass(frozen=True)
class DumpRow:
    name: str
    nbytes: int
    status: str  # 'ok' | 'fail'
    error: str = ''
    source: str | None = None


def clean_field(text: object) -> str:
    """Single line with no tabs: every whitespace run collapses to one space."""
    return ' '.join(str(text).split())


def resolve_names(names: str | None, use_all: bool, chitin_names: Iterable[str] = ()) -> list[str]:
    """Pick script names: --names list, else every chitin/override NCS name (--all), else DEFAULT_NAMES."""
    if names is not None and use_all:
        raise ValueError('--names and --all are mutually exclusive')
    if names is not None:
        raw = names.split(',')
    elif use_all:
        raw = sorted(n.strip().lower() for n in chitin_names if SAFE_NAME.fullmatch(n.strip().lower()))
    else:
        raw = list(DEFAULT_NAMES)
    ordered: dict[str, None] = {}
    for item in raw:
        n = item.strip().lower()
        if not n:
            continue
        if not SAFE_NAME.fullmatch(n):
            raise ValueError(f'unsafe script name: {item!r}')
        ordered.setdefault(n, None)
    if not ordered:
        raise ValueError('no script names resolved')
    return list(ordered)


def is_inside(path: Path, parent: Path) -> bool:
    """True when path equals parent or lives under it (both resolved)."""
    p, q = path.resolve(), parent.resolve()
    return p == q or q in p.parents


@contextmanager
def time_limit(seconds: float) -> Iterator[None]:
    """Raise ScriptTimeout after `seconds`. No-op when SIGALRM is unavailable or off the main thread."""
    usable = seconds > 0 and hasattr(signal, 'SIGALRM') and threading.current_thread() is threading.main_thread()
    if not usable:
        yield
        return

    def _on_alarm(signum, frame):
        raise ScriptTimeout(f'timeout after {seconds:g}s')

    prev = signal.signal(signal.SIGALRM, _on_alarm)
    signal.setitimer(signal.ITIMER_REAL, seconds)
    try:
        yield
    finally:
        signal.setitimer(signal.ITIMER_REAL, 0)
        signal.signal(signal.SIGALRM, prev)


def dump_one(
    name: str,
    lookup: Callable[[str], object | None],
    decompile: Callable[[bytes], str],
    timeout_s: float = TIMEOUT_S,
) -> DumpRow:
    """Look up one NCS resource and decompile it. Never raises; failures become fail rows."""
    try:
        res = lookup(name)
    except Exception as exc:  # one bad lookup must not stop the run
        return DumpRow(name, 0, 'fail', clean_field(f'lookup {type(exc).__name__}: {exc}'))
    data = getattr(res, 'data', None) if res is not None else None
    if data is None:
        return DumpRow(name, 0, 'fail', 'no NSS/NCS resource in game dir')
    data = bytes(data)
    nbytes = len(data)
    try:
        with time_limit(timeout_s):
            source = decompile(data)
    except ScriptTimeout as exc:
        return DumpRow(name, nbytes, 'fail', clean_field(exc))
    except Exception as exc:
        return DumpRow(name, nbytes, 'fail', clean_field(f'{type(exc).__name__}: {exc}'))
    return DumpRow(name, nbytes, 'ok', '', source)


def format_index_rows(rows: Sequence[DumpRow]) -> list[str]:
    """_index.tsv lines, header first. Every line has exactly four tab-separated fields."""
    lines = ['\t'.join(INDEX_HEADER)]
    for r in rows:
        lines.append('\t'.join([clean_field(r.name), str(int(r.nbytes)), r.status, clean_field(r.error)]))
    return lines


def write_outputs(out_dir: Path, rows: Sequence[DumpRow]) -> tuple[int, int]:
    """Write <name>.nss for ok rows and _index.tsv. Returns (ok_count, fail_count)."""
    out_dir.mkdir(parents=True, exist_ok=True)
    ok = 0
    for r in rows:
        if r.status == 'ok':
            text = r.source or ''
            (out_dir / f'{r.name}.nss').write_text(text if text.endswith('\n') else text + '\n', encoding='utf-8')
            ok += 1
    (out_dir / '_index.tsv').write_text('\n'.join(format_index_rows(rows)) + '\n', encoding='utf-8')
    return ok, len(rows) - ok


def main(argv: Sequence[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description='Decompile KOTOR II NCS scripts to NSS (read-only on the game dir).')
    ap.add_argument('--out', required=True, type=Path, help='output directory (created if missing)')
    sel = ap.add_mutually_exclusive_group()
    sel.add_argument('--names', default=None, help='comma-separated script names')
    sel.add_argument('--all', action='store_true', help='every NCS in chitin + override (long run)')
    ap.add_argument('--game', type=Path, default=DEFAULT_GAME, help='KOTOR II install dir (read-only)')
    a = ap.parse_args(argv)

    game = a.game.expanduser()
    if not game.is_dir():
        print(f'ERROR: game dir not found: {game}', file=sys.stderr)
        return 2
    if is_inside(a.out, game):
        print(f'ERROR: --out must not be inside the game dir: {a.out}', file=sys.stderr)
        return 2

    import logging

    logging.disable(logging.CRITICAL)
    from pykotor.common.misc import Game
    from pykotor.extract.installation import Installation
    from pykotor.resource.formats.ncs import decompile_ncs
    from pykotor.resource.formats.ncs.ncs_auto import read_ncs
    from pykotor.resource.type import ResourceType

    inst = Installation(str(game))
    chitin_names: list[str] = []
    if a.all:
        chitin_names = [
            r.resname()
            for r in [*inst.chitin_resources(), *inst.override_resources()]
            if r.restype() in (ResourceType.NCS, ResourceType.NSS)
        ]
    try:
        names = resolve_names(a.names, a.all, chitin_names)
    except ValueError as exc:
        print(f'ERROR: {exc}', file=sys.stderr)
        return 2

    def lookup(nm: str):
        # Scripts.bif ships the real .nss sources (incl. k_inc_* includes); prefer them over decompiling.
        return inst.resource(nm, ResourceType.NSS) or inst.resource(nm, ResourceType.NCS)

    def decompile(data: bytes) -> str:
        if not data.startswith(b'NCS '):
            return data.decode('latin-1')
        return decompile_ncs(read_ncs(data), Game.K2)

    rows: list[DumpRow] = []
    for i, name in enumerate(names, 1):
        row = dump_one(name, lookup, decompile)
        rows.append(row)
        tail = f' {row.error}' if row.error else ''
        print(f'[{i}/{len(names)}] {name} {row.status} {row.nbytes}{tail}', file=sys.stderr)

    ok, fail = write_outputs(a.out, rows)
    print(f'ok={ok} fail={fail} out={a.out}')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
