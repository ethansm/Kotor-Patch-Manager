#!/usr/bin/env python3
"""Source registry, source picker and removal ledger for the K1 -> K2 asset port.

Stdlib only: runs under system python3 and the pykotor venv. No pykotor import.

Registry: sources.json next to this module (override: env K1PORT_SOURCES). Metadata only, no assets.
Approval rule: requires_author_approval is derived from license_class (permission_required,
personal_use_only, unknown -> True; the rest -> False). A stored value that conflicts is ignored with a warning.
Unregistered source ids are rejected everywhere (SourceError).

Ledger: env K1PORT_LEDGER, default ~/Games/KOTOR2_Mod_Workspace/k1_port_stage/ledger.json.
add / remove / mark_approved change memory only; call save() to persist (atomic write).

CLI:
  python3 k1_port_sources.py show [--json]
  python3 k1_port_sources.py pick --json '<candidates json>'
  python3 k1_port_sources.py ledger [--json]
Exit: 0 ok, 1 registry/ledger/candidate error, 2 bad arguments.
"""
from __future__ import annotations

import argparse
import copy
import json
import math
import os
import sys
import tempfile
import warnings
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent
SCHEMA = 1
APPROVAL_CLASSES = frozenset({"permission_required", "personal_use_only", "unknown"})
REMOVAL_PENDING = "pending_author_approval"
KINDS = ("texture", "model", "other")
DEFAULT_LEDGER = "~/Games/KOTOR2_Mod_Workspace/k1_port_stage/ledger.json"


class SourceError(KeyError):
    """Unknown source id, invalid registry, invalid candidate or ledger. A KeyError, printed without quotes."""

    def __str__(self) -> str:
        return str(self.args[0]) if self.args else ""


def default_registry_path() -> Path:
    env = os.environ.get("K1PORT_SOURCES")
    return Path(env).expanduser() if env else HERE / "sources.json"


REGISTRY_PATH = default_registry_path()


def load_registry(path=None) -> dict:
    """Return {"schema", "classes", "sources": {id: record}}. Approval is always derived from the class."""
    p = Path(path).expanduser() if path is not None else default_registry_path()
    try:
        raw = json.loads(p.read_text(encoding="utf-8"))
    except FileNotFoundError as e:
        raise SourceError(f"registry not found: {p}") from e
    except json.JSONDecodeError as e:
        raise SourceError(f"registry is not valid JSON: {p}: {e}") from e
    if not isinstance(raw, dict) or raw.get("schema") != SCHEMA:
        raise SourceError(f"registry schema must be {SCHEMA}: {p}")
    classes = raw.get("classes")
    if not isinstance(classes, dict) or not all(
        isinstance(v, int) and not isinstance(v, bool) for v in classes.values()
    ):
        raise SourceError(f"registry 'classes' must map class name -> int rank: {p}")
    missing = sorted(APPROVAL_CLASSES - set(classes))
    if missing:
        raise SourceError(f"registry 'classes' lacks approval classes {missing}: {p}")
    sources = raw.get("sources")
    if not isinstance(sources, dict):
        raise SourceError(f"registry 'sources' must be an object: {p}")
    out = {}
    for sid, src in sources.items():
        if not isinstance(src, dict):
            raise SourceError(f"source {sid!r} must be an object: {p}")
        cls = src.get("license_class")
        if cls not in classes:
            raise SourceError(f"source {sid!r}: unknown license_class {cls!r}")
        derived = cls in APPROVAL_CLASSES
        if "requires_author_approval" in src and src["requires_author_approval"] != derived:
            warnings.warn(
                f"k1_port_sources: source {sid!r} stores requires_author_approval="
                f"{src['requires_author_approval']!r} but class {cls!r} gives {derived}; using {derived}",
                stacklevel=2,
            )
        rec = dict(src)
        rec.update({
            "id": sid,
            "name": src.get("name") or sid,
            "author": src.get("author"),
            "url": src.get("url"),
            "license_class": cls,
            "requires_author_approval": derived,
            "credits": list(src.get("credits") or []),
            "evidence": src.get("evidence"),
            "local_dir": src.get("local_dir"),
            "notes": src.get("notes") or "",
        })
        out[sid] = rec
    return {"schema": SCHEMA, "classes": dict(classes), "sources": out}


def _lookup(reg: dict, source_id) -> dict:
    if not isinstance(source_id, str) or source_id not in reg["sources"]:
        raise SourceError(f"unregistered source id: {source_id!r} (registry: {default_registry_path()})")
    return reg["sources"][source_id]


def get_source(source_id: str) -> dict:
    return copy.deepcopy(_lookup(load_registry(), source_id))


def requires_approval(source_id: str) -> bool:
    return bool(get_source(source_id)["requires_author_approval"])


def removal_group(source_id: str):
    return REMOVAL_PENDING if requires_approval(source_id) else None


def _fmt_quality(q) -> str:
    return "none" if q == -math.inf else str(q)


def pick(candidates) -> dict:
    """Choose one candidate. Approval-free candidates win over flagged ones, then lowest class rank,
    then highest quality, then source_id, then input order. Returns chosen / rejected / warning."""
    if not isinstance(candidates, (list, tuple)) or not candidates:
        raise SourceError("pick needs a non-empty list of candidates")
    reg = load_registry()
    rows = []
    for idx, cand in enumerate(candidates):
        if not isinstance(cand, dict) or "source_id" not in cand:
            raise SourceError(f"candidate {idx} must be an object with a 'source_id'")
        src = _lookup(reg, cand["source_id"])
        q = cand.get("quality")
        if q is None:
            q = -math.inf
        elif isinstance(q, bool) or not isinstance(q, (int, float)) or math.isnan(q):
            raise SourceError(f"candidate {idx} ({cand['source_id']}): quality must be a number")
        rows.append({
            "idx": idx, "cand": cand, "sid": src["id"], "flag": src["requires_author_approval"],
            "cls": src["license_class"], "rank": reg["classes"][src["license_class"]], "quality": q,
        })
    order = sorted(rows, key=lambda r: (r["flag"], r["rank"], -r["quality"], r["sid"], r["idx"]))
    best = order[0]
    free_exists = not best["flag"]
    rejected = []
    for r in order[1:]:
        if r["flag"] and free_exists:
            reason = (f"requires author approval (class {r['cls']}); "
                      f"approval-free candidate {best['sid']!r} preferred")
        elif r["rank"] != best["rank"]:
            reason = f"class {r['cls']} (rank {r['rank']}) ranks below {best['cls']} (rank {best['rank']})"
        elif r["quality"] != best["quality"]:
            reason = f"quality {_fmt_quality(r['quality'])} below chosen {_fmt_quality(best['quality'])}"
        else:
            reason = f"tie on class and quality; source_id {r['sid']!r} sorts after {best['sid']!r}"
        rejected.append({"candidate": r["cand"], "reason": reason})
    warning = None if free_exists else "all candidates require author approval"
    return {"chosen": best["cand"], "rejected": rejected, "warning": warning}


def _now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def _empty_ledger() -> dict:
    return {"schema": SCHEMA, "entries": [], "approvals": {}}


class Ledger:
    """Asset removal ledger. Entries are keyed by the file string given to add()."""

    def __init__(self, path=None):
        if path is None:
            path = os.environ.get("K1PORT_LEDGER") or DEFAULT_LEDGER
        self.path = Path(path).expanduser()
        self._data = _empty_ledger()
        self.load()

    def load(self):
        if not self.path.exists():
            self._data = _empty_ledger()
            return self
        try:
            raw = json.loads(self.path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as e:
            raise SourceError(f"ledger is not valid JSON: {self.path}: {e}") from e
        if not isinstance(raw, dict) or raw.get("schema") != SCHEMA or not isinstance(raw.get("entries"), list):
            raise SourceError(f"ledger schema must be {SCHEMA} with an 'entries' list: {self.path}")
        raw.setdefault("approvals", {})
        self._data = raw
        return self

    def save(self):
        """Atomic write: temp file in the same directory, fsync, os.replace. Returns the path."""
        self.path.parent.mkdir(parents=True, exist_ok=True)
        fd, tmp = tempfile.mkstemp(dir=str(self.path.parent), prefix=f".{self.path.name}.", suffix=".tmp")
        try:
            with os.fdopen(fd, "w", encoding="utf-8") as fh:
                json.dump(self._data, fh, indent=2, sort_keys=True)
                fh.write("\n")
                fh.flush()
                os.fsync(fh.fileno())
            os.replace(tmp, self.path)
        except BaseException:
            try:
                os.unlink(tmp)
            except FileNotFoundError:
                pass
            raise
        return self.path

    def add(self, file, kind: str, source_id: str, sha256=None, **extra) -> dict:
        """Add or replace (same file string) the entry for file. Returns a copy of the stored entry.
        If the source already has a recorded author approval, the new entry inherits it."""
        if kind not in KINDS:
            raise ValueError(f"kind must be one of {KINDS}, got {kind!r}")
        src = get_source(source_id)
        approvals = self._data["approvals"]
        approved = source_id in approvals and src["requires_author_approval"]
        entry = dict(extra)
        entry.update({
            "file": str(file),
            "kind": kind,
            "source_id": source_id,
            "license_class": src["license_class"],
            "requires_author_approval": src["requires_author_approval"],
            "removal_group": None if approved or not src["requires_author_approval"] else REMOVAL_PENDING,
            "credits": list(src["credits"]),
            "sha256": sha256,
            "added": _now(),
            "author_approved": bool(approved),
        })
        if approved:
            entry["approval_note"] = approvals[source_id]["note"]
            entry["approved_at"] = approvals[source_id]["time"]
        entries = self._data["entries"]
        for i, old in enumerate(entries):
            if old.get("file") == entry["file"]:
                entries[i] = entry
                break
        else:
            entries.append(entry)
        return copy.deepcopy(entry)

    def entries(self) -> list:
        return copy.deepcopy(self._data["entries"])

    def flagged(self) -> list:
        return [e for e in self.entries() if e.get("removal_group") and not e.get("author_approved")]

    def remove(self, file) -> int:
        """Drop entries for file. Returns how many were removed."""
        name = str(file)
        before = len(self._data["entries"])
        self._data["entries"] = [e for e in self._data["entries"] if e.get("file") != name]
        return before - len(self._data["entries"])

    def mark_approved(self, source_id: str, note: str) -> int:
        """Record the author's approval: every entry of source_id is unflagged. Returns entries updated."""
        get_source(source_id)
        now = _now()
        n = 0
        for e in self._data["entries"]:
            if e.get("source_id") == source_id:
                e.update(author_approved=True, removal_group=None, approval_note=note, approved_at=now)
                n += 1
        self._data["approvals"][source_id] = {"note": note, "time": now}
        return n

    def find_by_sha(self, sha) -> list:
        want = str(sha).lower()
        return [copy.deepcopy(e) for e in self._data["entries"] if str(e.get("sha256") or "").lower() == want]


def _cmd_show(args) -> int:
    reg = load_registry()
    rows = [
        {"id": s["id"], "name": s["name"], "license_class": s["license_class"],
         "requires_author_approval": s["requires_author_approval"],
         "removal_group": REMOVAL_PENDING if s["requires_author_approval"] else None}
        for s in reg["sources"].values()
    ]
    if args.json:
        print(json.dumps(rows, indent=2))
        return 0
    print(f"{'id':<30}{'class':<22}{'approval':<10}")
    for r in rows:
        print(f"{r['id']:<30}{r['license_class']:<22}{'required' if r['requires_author_approval'] else 'no':<10}")
    return 0


def _cmd_pick(args) -> int:
    try:
        candidates = json.loads(args.json)
    except json.JSONDecodeError as e:
        print(f"error: --json is not valid JSON: {e}", file=sys.stderr)
        return 1
    print(json.dumps(pick(candidates), indent=2))
    return 0


def _cmd_ledger(args) -> int:
    led = Ledger()
    ents = led.entries()
    flagged = len(led.flagged())
    if args.json:
        print(json.dumps({"path": str(led.path), "entries": ents, "flagged": flagged}, indent=2))
        return 0
    print(f"ledger: {led.path}")
    for e in ents:
        print(f"{e['file']}  kind={e['kind']}  source={e['source_id']}  class={e['license_class']}  "
              f"removal={e['removal_group'] or '-'}  approved={e['author_approved']}")
    print(f"entries: {len(ents)}  flagged: {flagged}")
    return 0


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="K1 port source registry, picker and removal ledger.")
    sub = parser.add_subparsers(dest="cmd", required=True)
    p_show = sub.add_parser("show", help="list registered sources")
    p_show.add_argument("--json", action="store_true")
    p_pick = sub.add_parser("pick", help="pick one candidate from a JSON list")
    p_pick.add_argument("--json", required=True, metavar="CANDIDATES", help="JSON list of candidate objects")
    p_ledger = sub.add_parser("ledger", help="show ledger entries and flagged count")
    p_ledger.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    handlers = {"show": _cmd_show, "pick": _cmd_pick, "ledger": _cmd_ledger}
    try:
        return handlers[args.cmd](args)
    except SourceError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    except OSError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
