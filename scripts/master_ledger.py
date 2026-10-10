#!/usr/bin/env python3
"""master_ledger.py - Worktree-wide audit ledger and collision guard for multi-agent workflows.

Tracks high-impact actions (DLL hot-swaps, rollbacks, build gates, plan milestones, locks)
across all sessions (Claude Code, Antigravity/Gemini, subagents, human) into an append-only JSONL log.

Usage:
  master_ledger.py tail [-n 10]
  master_ledger.py status
  master_ledger.py log <EVENT> [k=v ...]
  master_ledger.py filter [--event EVENT] [--mod MOD] [--plan PLAN] [-n LIMIT]
  master_ledger.py check-collision --mod <MOD> [--within-minutes 30]

Standard events:
  HOTSWAP_APPLY    Live DLL(s) swapped into KOTOR II
  HOTSWAP_ROLLBACK Live DLL(s) rolled back from backup
  BUILD_GATE       Pre-install gate pass/fail + DLL md5
  PHASE_DONE       Master plan phase marked completed
  SESSION_HANDOFF  Session context handoff generated
  CONTRACT_CHECK   Subagent write scope verified
  LOCK_ACQUIRE     Agent acquired exclusive worktree lock on a mod
  LOCK_RELEASE     Agent released worktree lock on a mod
"""

import argparse
import datetime
import json
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_LEDGER = REPO_ROOT / ".claude" / "master_ledger.jsonl"

KNOWN_MOD_ALIASES = {
    "queue": "queue_hud_src",
    "hotbar": "combat_hotbar_src",
    "diversity": "npc_diversity_src",
    "letterbox": "dialogue_letterbox_fix_src",
    "companion": "companion_ai_src",
    "selfheal": "companion_selfheal_src",
}


def canonical_mod_name(name: str | None) -> str | None:
    """Normalizes any mod string, path, or alias into canonical snake_case_src format."""
    if not name:
        return None
    raw = str(name).strip().replace("patch_manager_mods/", "").rstrip("/")
    if not raw:
        return None
    if raw in ("_shared_src", "_shared"):
        return "_shared_src"

    low = raw.lower()
    for alias, canonical in KNOWN_MOD_ALIASES.items():
        if alias in low and not low.endswith("_src"):
            return canonical

    clean = raw.replace("-", "_")
    if not clean.endswith("_src") and not clean.startswith("test") and clean != "demo":
        clean = f"{clean}_src"
    return clean


def kpm_mod_name(name: str | None) -> str | None:
    """Converts canonical mod name into KPM manifest kebab-case (e.g. queue_hud_src -> queue-hud)."""
    canon = canonical_mod_name(name)
    if not canon or canon in ("_shared_src", "demo"):
        return canon
    return canon.removesuffix("_src").replace("_", "-")


def is_test_environment() -> bool:
    """Detects whether code is executing within an automated test harness."""
    if os.environ.get("MASTER_LEDGER_TEST") == "1" or os.environ.get("PYTEST_CURRENT_TEST"):
        return True

    main_mod = sys.modules.get("__main__")
    if main_mod and getattr(main_mod, "__file__", None):
        main_name = Path(main_mod.__file__).name
        if main_name.startswith("test_") or main_name.endswith("_test.py") or main_name == "run_tests.py":
            return True

    if "unittest" in sys.modules:
        for arg in sys.argv:
            if "test" in arg.lower():
                return True

    return False


def get_ledger_path(override: str | Path | None = None) -> Path:
    if override:
        return Path(override).expanduser().resolve()
    env_path = os.environ.get("MASTER_LEDGER_PATH")
    if env_path:
        return Path(env_path).expanduser().resolve()
    return DEFAULT_LEDGER


def detect_agent_identity() -> str:
    """Detect current agent identity from environment or process tree."""
    # 1. Explicit override
    if os.environ.get("AGENT_ID"):
        return os.environ["AGENT_ID"]

    # 2. Antigravity environment
    if os.environ.get("ANTIGRAVITY_AGENT") == "1" or "ANTIGRAVITY_CONVERSATION_ID" in os.environ:
        conv = os.environ.get("ANTIGRAVITY_CONVERSATION_ID", "")
        return f"antigravity:{conv[:8]}" if conv else "antigravity"

    # 3. Explicit conversation/session variables
    for var in ("CONVERSATION_ID", "SESSION_ID", "CLAUDE_SESSION_ID"):
        val = os.environ.get(var)
        if val:
            return val

    # 4. Process tree inspection (e.g. Claude Code or OpenCode parent)
    try:
        pid = os.getppid()
        for _ in range(8):
            if pid <= 1:
                break
            cmd_path = Path(f"/proc/{pid}/cmdline")
            if not cmd_path.exists():
                break
            cmd = cmd_path.read_text(errors="replace").replace("\0", " ")
            if "claude" in cmd.lower():
                m = re.search(r"--resume\s+([a-f0-9-]+)", cmd)
                return f"claude:{m.group(1)[:8]}" if m else "claude-code"
            if "opencode" in cmd.lower():
                return "opencode"
            status_text = Path(f"/proc/{pid}/status").read_text(errors="replace")
            m_ppid = re.search(r"^PPid:\s+(\d+)", status_text, re.MULTILINE)
            if m_ppid:
                pid = int(m_ppid.group(1))
            else:
                break
    except Exception:
        pass

    # 5. Fallback to user / human
    return os.environ.get("USER") or "human"


def current_agent_id() -> str:
    return detect_agent_identity()


def current_parent_agent_id() -> str | None:
    return os.environ.get("PARENT_AGENT_ID") or os.environ.get("PARENT_SESSION_ID")


def log_event(
    event: str,
    data: dict | None = None,
    ledger_path: str | Path | None = None,
) -> dict:
    """Append a structured event to master_ledger.jsonl. Never raises (logs error to stderr)."""
    ts = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    record = {
        "ts": ts,
        "event": event.strip().upper(),
        "agent": current_agent_id(),
    }
    parent = current_parent_agent_id()
    if parent:
        record["parent_agent"] = parent
    if data:
        for k, v in data.items():
            if k == "mod" and v:
                record["mod"] = canonical_mod_name(v)
            elif k == "mods" and isinstance(v, list):
                record["mods"] = [canonical_mod_name(m) for m in v if m]
            else:
                record[k] = v

    # Test Isolation Guard: Suppress writing to production ledger during automated tests
    if is_test_environment() and ledger_path is None and "MASTER_LEDGER_PATH" not in os.environ:
        return record

    dest = get_ledger_path(ledger_path)
    try:
        dest.parent.mkdir(parents=True, exist_ok=True)
        line = json.dumps(record, separators=(",", ":")) + "\n"
        with open(dest, "a", encoding="utf-8") as f:
            f.write(line)
            f.flush()
    except Exception as e:
        sys.stderr.write(f"WARN [master_ledger]: Failed to write ledger event '{event}': {e}\n")

    return record


def read_events(ledger_path: str | Path | None = None) -> list[dict]:
    dest = get_ledger_path(ledger_path)
    if not dest.is_file():
        return []
    records = []
    with open(dest, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                records.append(json.loads(line))
            except json.JSONDecodeError:
                continue
    return records


def format_event_line(rec: dict) -> str:
    ts = rec.get("ts", "")[:19].replace("T", " ")
    event = rec.get("event", "UNKNOWN")
    agent = rec.get("agent", "")
    parent = rec.get("parent_agent")
    agent_label = f"{agent} <- {parent}" if parent else agent

    detail_parts = []
    if "backend" in rec:
        detail_parts.append(f"backend={rec['backend']}")
    if "role" in rec:
        detail_parts.append(f"role={rec['role']}")
    if "action" in rec:
        detail_parts.append(f"action={rec['action']}")
    if "model" in rec:
        detail_parts.append(f"model={rec['model']}")
    if "actual_model" in rec and rec.get("actual_model") != rec.get("model"):
        detail_parts.append(f"actual_model={rec['actual_model']}")
    if "duration_ms" in rec:
        try:
            d_ms = int(rec["duration_ms"])
            if d_ms >= 1000:
                detail_parts.append(f"duration={d_ms / 1000:.1f}s")
            else:
                detail_parts.append(f"duration={d_ms}ms")
        except (ValueError, TypeError):
            detail_parts.append(f"duration={rec['duration_ms']}")
    if "tokens_in" in rec and "tokens_out" in rec:
        try:
            tin = int(rec["tokens_in"])
            tout = int(rec["tokens_out"])
            tin_str = f"{tin/1000:.1f}k" if tin >= 1000 else str(tin)
            tout_str = f"{tout/1000:.1f}k" if tout >= 1000 else str(tout)
            detail_parts.append(f"tokens={tin_str}/{tout_str}")
        except (ValueError, TypeError):
            detail_parts.append(f"tokens_in={rec['tokens_in']} tokens_out={rec['tokens_out']}")
    elif "tokens_in" in rec:
        detail_parts.append(f"tokens_in={rec['tokens_in']}")
    if "session" in rec and rec["session"]:
        detail_parts.append(f"session={str(rec['session'])[:12]}")
    if "subagent" in rec:
        detail_parts.append(f"target={rec['subagent']}")
    if "decision" in rec:
        detail_parts.append(f"decision={rec['decision']}")
    if "confidence" in rec:
        detail_parts.append(f"conf={rec['confidence']}")
    if "latency_ms" in rec:
        detail_parts.append(f"latency={rec['latency_ms']}ms")
    if "predicted_duration_ms" in rec:
        try:
            p_ms = int(rec["predicted_duration_ms"])
            detail_parts.append(f"est_dur={p_ms / 1000:.1f}s" if p_ms >= 1000 else f"est_dur={p_ms}ms")
        except (ValueError, TypeError):
            pass
    if "write_scope" in rec and rec["write_scope"]:
        ws = rec["write_scope"]
        scope_str = ",".join(str(s) for s in ws) if isinstance(ws, list) else str(ws)
        detail_parts.append(f"scope=[{scope_str}]")
    if "changed_count" in rec:
        detail_parts.append(f"diff={rec['changed_count']}_files")
    if "mod" in rec:
        detail_parts.append(f"mod={rec['mod']}")
    if "mods" in rec:
        mods_val = rec["mods"]
        if isinstance(mods_val, list):
            mods_val = ",".join(str(m) for m in mods_val)
        detail_parts.append(f"mods={mods_val}")
    if "phase" in rec:
        detail_parts.append(f"phase={rec['phase']}")
    if "plan" in rec:
        detail_parts.append(f"plan={Path(str(rec['plan'])).name}")
    if "verdict" in rec:
        detail_parts.append(f"verdict={rec['verdict']}")
    if "status" in rec:
        detail_parts.append(f"status={rec['status']}")
    if "commit" in rec and rec["commit"]:
        detail_parts.append(f"commit={rec['commit'][:8]}")
    if "backup" in rec and rec["backup"]:
        detail_parts.append(f"backup={Path(str(rec['backup'])).name}")
    if "notes" in rec and rec["notes"]:
        notes = str(rec["notes"]).strip()
        if len(notes) > 40:
            notes = notes[:37] + "..."
        detail_parts.append(f"notes={notes!r}")

    detail = " ".join(detail_parts)
    return f"[{ts}] {event:<16} ({agent_label}) {detail}"


def cmd_tail(args, ledger_path: Path):
    events = read_events(ledger_path)
    count = args.n if args.n else 10
    subset = events[-count:] if len(events) > count else events
    if not subset:
        print(f"Master ledger empty: {ledger_path}")
        return 0
    for e in subset:
        print(format_event_line(e))
    return 0


def cmd_filter(args, ledger_path: Path):
    events = read_events(ledger_path)
    matched = []
    for e in events:
        if args.event and e.get("event", "").upper() != args.event.upper():
            continue
        if args.mod:
            target_canon = (canonical_mod_name(args.mod) or args.mod).lower()
            m = (canonical_mod_name(e.get("mod")) or str(e.get("mod", ""))).lower()
            ms = [(canonical_mod_name(x) or str(x)).lower() for x in e.get("mods", [])] if isinstance(e.get("mods"), list) else []
            if target_canon != m and target_canon not in ms:
                continue
        if args.plan:
            target_plan = args.plan.lower()
            p = Path(str(e.get("plan", ""))).name.lower()
            if target_plan not in p:
                continue
        matched.append(e)

    if args.n and len(matched) > args.n:
        matched = matched[-args.n:]

    if not matched:
        print("No matching events found.")
        return 0
    for e in matched:
        print(format_event_line(e))
    return 0


def cmd_check_collision(args, ledger_path: Path):
    """Checks if another agent has performed active work on the mod recently."""
    events = read_events(ledger_path)
    now = datetime.datetime.now(datetime.timezone.utc)
    target_mod = (canonical_mod_name(args.mod) or args.mod).lower()
    within_sec = args.within_minutes * 60

    conflicts = []
    my_agent = current_agent_id()

    for e in reversed(events):
        ts_str = e.get("ts")
        if not ts_str:
            continue
        try:
            ts = datetime.datetime.fromisoformat(ts_str.replace("Z", "+00:00"))
        except ValueError:
            continue
        age = (now - ts).total_seconds()
        if age > within_sec:
            break

        # Check if event affects this mod
        m = (canonical_mod_name(e.get("mod")) or str(e.get("mod", ""))).lower()
        ms = [(canonical_mod_name(x) or str(x)).lower() for x in e.get("mods", [])] if isinstance(e.get("mods"), list) else []
        if target_mod == m or target_mod in ms:
            agent = e.get("agent", "unknown")
            if agent != my_agent:
                conflicts.append((e, int(age // 60)))

    if conflicts:
        print(f"COLLISION WARNING: Mod '{args.mod}' touched recently by another agent:")
        for e, mins_ago in conflicts[:5]:
            print(f"  - {mins_ago}m ago by {e.get('agent')}: {e.get('event')} ({format_event_line(e)})")
        return 1

    print(f"CLEAN: Mod '{args.mod}' has no recent external activity (< {args.within_minutes}m).")
    return 0


def cmd_status(args, ledger_path: Path):
    events = read_events(ledger_path)
    if not events:
        print(f"Master ledger is empty ({ledger_path})")
        return 0

    print(f"Master Ledger: {ledger_path} ({len(events)} total events)")
    print("\n--- Recent Mod Actions ---")
    mod_latest = {}
    plan_latest = {}
    for e in events:
        event_name = e.get("event")
        # mod track
        for mod in ([e.get("mod")] if e.get("mod") else []) + (e.get("mods") if isinstance(e.get("mods"), list) else []):
            if mod:
                mod_latest[str(mod)] = e
        # plan track
        plan = e.get("plan")
        if plan:
            plan_name = Path(str(plan)).name
            plan_latest[plan_name] = e

    for mod, e in sorted(mod_latest.items())[:10]:
        print(f"  {mod:<24} {e.get('event'):<16} {e.get('ts')[:19].replace('T', ' ')}")

    if plan_latest:
        print("\n--- Recent Plan Milestones ---")
        for plan, e in sorted(plan_latest.items())[:5]:
            phase = e.get("phase", "-")
            status = e.get("status") or e.get("verdict") or "-"
            print(f"  {plan:<32} Phase {phase:<6} [{status}] {e.get('ts')[:19].replace('T', ' ')}")

    return 0


def is_test_pollution(rec: dict) -> bool:
    """Determines whether a ledger record is automated test pollution."""
    mod = rec.get("mod")
    mods = rec.get("mods")
    if mod in ("demo", "demo_src") or mods in (["demo"], ["demo_src"], "demo", "demo_src"):
        return True
    if "demo_swap_" in str(rec.get("backup", "")):
        return True
    if "gate-unittest" in str(rec.get("error", "")):
        return True
    plan = str(rec.get("plan", ""))
    if any(pat in plan for pat in ("test_plan", "test_saved_state", "autoresolve_test", "tmp")):
        return True
    return False


def cmd_prune_test_data(args, ledger_path: Path) -> int:
    events = read_events(ledger_path)
    if not events:
        print(f"Ledger is empty: {ledger_path}")
        return 0

    clean_events = []
    polluted_events = []
    for e in events:
        if is_test_pollution(e):
            polluted_events.append(e)
        else:
            clean_events.append(e)

    if not polluted_events:
        print("No test pollution found in ledger.")
        return 0

    archive_path = ledger_path.parent / "master_ledger.test_archive.jsonl"
    with open(archive_path, "a", encoding="utf-8") as f:
        for pe in polluted_events:
            f.write(json.dumps(pe, separators=(",", ":")) + "\n")

    # Atomic rewrite of clean ledger
    tmp_path = ledger_path.with_suffix(".tmp")
    with open(tmp_path, "w", encoding="utf-8") as f:
        for ce in clean_events:
            f.write(json.dumps(ce, separators=(",", ":")) + "\n")
    tmp_path.replace(ledger_path)

    print(f"Pruned {len(polluted_events)} test events to {archive_path.name}. Remaining clean events: {len(clean_events)}")
    return 0


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="Master audit ledger and collision detector for agent runs.")
    parser.add_argument("--ledger", help="Path to master_ledger.jsonl", default=None)
    sub = parser.add_subparsers(dest="cmd")

    p_tail = sub.add_parser("tail", help="Tail last N entries")
    p_tail.add_argument("-n", type=int, default=10, help="Number of records (default 10)")

    p_status = sub.add_parser("status", help="Show current state summary across mods and plans")

    p_filter = sub.add_parser("filter", help="Filter ledger entries")
    p_filter.add_argument("--event", help="Filter by event name (e.g. HOTSWAP_APPLY)")
    p_filter.add_argument("--mod", help="Filter by mod ID")
    p_filter.add_argument("--plan", help="Filter by plan name or path")
    p_filter.add_argument("-n", type=int, default=20, help="Max entries to display")

    p_check = sub.add_parser("check-collision", help="Check if a mod was touched recently by another agent")
    p_check.add_argument("--mod", required=True, help="Mod ID to inspect")
    p_check.add_argument("--within-minutes", type=int, default=30, help="Recency threshold in minutes (default 30)")

    p_prune = sub.add_parser("prune-test-data", help="Archive and strip automated test run entries")

    p_log = sub.add_parser("log", help="Manually append an event")
    p_log.add_argument("event", help="Event name (e.g. PHASE_DONE, LOCK_ACQUIRE)")
    p_log.add_argument("fields", nargs="*", help="Key=value pairs to include in payload")

    args = parser.parse_args(argv)
    ledger_path = get_ledger_path(args.ledger)

    if args.cmd == "tail" or not args.cmd:
        return cmd_tail(args if args.cmd else argparse.Namespace(n=10), ledger_path)
    elif args.cmd == "status":
        return cmd_status(args, ledger_path)
    elif args.cmd == "filter":
        return cmd_filter(args, ledger_path)
    elif args.cmd == "check-collision":
        return cmd_check_collision(args, ledger_path)
    elif args.cmd == "prune-test-data":
        return cmd_prune_test_data(args, ledger_path)
    elif args.cmd == "log":
        payload = {}
        for f in args.fields:
            if "=" in f:
                k, v = f.split("=", 1)
                payload[k.strip()] = v.strip()
        log_event(args.event, payload, ledger_path)
        print(f"Logged event {args.event.upper()} to {ledger_path}")
        return 0

    return 0


if __name__ == "__main__":
    sys.exit(main())
