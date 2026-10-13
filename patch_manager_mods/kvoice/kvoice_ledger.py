#!/usr/bin/env python3
"""kvoice licence-ledger rows for voice clips (doc 41 V-B).

Thin wrapper over k1_port_sources.Ledger. row_for_clip() registers either:
  - a direct copy of a registered source (source_id -> Ledger.add), or
  - a derived copy of a ledgered parent (parent_sha256 -> Ledger.add_derived; fails closed
    when the parent sha256 is not in the ledger).

Rule: a row whose parent source id starts with 'vanilla_' is always personal_only and local_only.
Does not save. The caller calls ledger.save().
"""
import hashlib
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
K1PORT = HERE.parent / "k1_port"
if str(K1PORT) not in sys.path:
    sys.path.insert(0, str(K1PORT))

import k1_port_sources as ks  # noqa: E402

CONTAINERS = ("vo58", "bare", "none")
VANILLA_PREFIX = "vanilla_"


def sha256_of(path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def is_vanilla(source_id) -> bool:
    return str(source_id).startswith(VANILLA_PREFIX)


def _parent_source_id(ledger, parent_sha256: str) -> str:
    """Same parent choice as Ledger.add_derived: a flagged entry wins."""
    want = str(parent_sha256 or "").lower()
    parents = ledger.find_by_sha(want) if want else []
    if not parents:
        raise ks.SourceError(f"derived clip: parent sha256 {want[:12] or '<empty>'} is not in the ledger")
    parents.sort(key=lambda e: 0 if e.get("removal_group") and not e.get("author_approved") else 1)
    return parents[0]["source_id"]


def row_for_clip(ledger, clip_path, *, container, source_resref, slot, ship_mode,
                 source_id=None, parent_sha256=None, kind="audio",
                 personal_only=False, local_only=False, **extra) -> dict:
    """Register one clip and return a copy of the stored entry. Pass exactly one of source_id
    (direct copy) or parent_sha256 (derived copy). container must be one of CONTAINERS."""
    if (source_id is None) == (parent_sha256 is None):
        raise ValueError("pass exactly one of source_id or parent_sha256")
    if container not in CONTAINERS:
        raise ValueError(f"container must be one of {CONTAINERS}, got {container!r}")
    clip = Path(clip_path)
    sha = sha256_of(clip)
    file_str = str(clip.resolve())
    fields = dict(extra, container=container, source_resref=source_resref, slot=slot, ship_mode=ship_mode)
    if parent_sha256 is not None:
        if is_vanilla(_parent_source_id(ledger, parent_sha256)):
            personal_only = True  # add_derived already forces local_only
        return ledger.add_derived(file_str, kind, parent_sha256=parent_sha256, sha256=sha,
                                  personal_only=bool(personal_only), **fields)
    if is_vanilla(source_id):
        personal_only = True
        local_only = True
    return ledger.add(file_str, kind, source_id, sha256=sha,
                      personal_only=bool(personal_only), local_only=bool(local_only), **fields)
