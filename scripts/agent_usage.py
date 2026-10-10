#!/usr/bin/env python3
"""Per-agent token usage and estimated cost for the "### Agents Used" section.

Reads this session's subagent transcripts (read-only; prints to stdout/stderr only):
  <projects>/<project>/<session>/subagents/agent-<id>.jsonl  (+ agent-<id>.meta.json)

Columns:
  Peak ctx   largest single-request prompt (input + cache write + cache read); output excluded.
  Processed  sum over requests of all four counters (output included).
  Est. cost  each request priced from agent_usage_rates.json; tier set by that request's prompt size.

Output holds only agent id, type, model, description, counts and cost. No prompt text, paths or session id.
Exit codes: 0 ok, 1 transcripts found but no usage parsed, 2 subagents dir missing or bad config.
"""
import argparse
import collections
import datetime as dt
import glob
import json
import os
import re
import sys
from pathlib import Path

RATES_PATH = Path(__file__).resolve().parent / "agent_usage_rates.json"
COUNTERS = ("input_tokens", "cache_creation_input_tokens", "cache_read_input_tokens", "output_tokens")
RATE_KEYS = ("input", "output", "cache_write_5m", "cache_write_1h", "cache_read")
SYNTHETIC = "<synthetic>"
HAIKU_PREFIX = "claude-haiku-"
ROW_CAP = 5
DESC_MAX = 40
STALE_DAYS = 60
FLAG = "🔺"
SESSION_RE = re.compile(r"[A-Za-z0-9_-]{1,100}")
UTC = dt.timezone.utc
EPOCH = dt.datetime(1970, 1, 1, tzinfo=UTC)


class RatesError(Exception):
    pass


def parse_ts(value):
    """ISO8601 string -> aware datetime (naive assumed UTC). None when unparseable."""
    if not isinstance(value, str):
        return None
    try:
        ts = dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError:
        return None
    return ts if ts.tzinfo else ts.replace(tzinfo=UTC)


def load_rates(path=None):
    """Validate the rates file. Raises RatesError (fail closed) on any schema problem."""
    path = RATES_PATH if path is None else path
    try:
        raw = json.loads(Path(path).read_text(encoding="utf-8"))
        as_of = dt.date.fromisoformat(raw["as_of"])
        threshold = raw["tier_threshold_tokens"]
        models = raw["models"]
        if not isinstance(threshold, int) or isinstance(threshold, bool) or threshold <= 0 or not isinstance(models, list):
            raise ValueError("tier_threshold_tokens must be a positive int and models a list")
        entries = []
        for m in models:
            for tier in ("low", "high"):
                rates = m[tier]
                if set(rates) != set(RATE_KEYS) or not all(isinstance(v, (int, float)) and v >= 0 for v in rates.values()):
                    raise ValueError(f"{tier} rates must be exactly {RATE_KEYS}, non-negative numbers")
            entries.append({"match": re.compile(m["match"]), "low": m["low"], "high": m["high"], "source": m.get("source", "")})
    except (OSError, ValueError, KeyError, TypeError, re.error) as exc:
        raise RatesError(f"bad rates file {Path(path).name}: {exc}") from exc
    return {"as_of": as_of, "threshold": threshold, "models": entries}


def find_rate(model, rates):
    for entry in rates["models"]:
        if entry["match"].fullmatch(model):
            return entry
    return None


def prompt_tokens(req):
    return req["in"] + req["cw5"] + req["cw1"] + req["cr"]


def price_request(model, req, rates):
    """USD for one request, or None when the model has no rate on file."""
    entry = find_rate(model, rates)
    if entry is None:
        return None
    tier = entry["high"] if prompt_tokens(req) > rates["threshold"] else entry["low"]
    usd = (req["in"] * tier["input"] + req["out"] * tier["output"] + req["cw5"] * tier["cache_write_5m"]
           + req["cw1"] * tier["cache_write_1h"] + req["cr"] * tier["cache_read"])
    return usd / 1_000_000


def parse_transcript(path):
    """Return (requests, drift_lines). Streamed partials share a requestId; the last record wins."""
    by_key, drift = {}, 0
    with open(path, encoding="utf-8", errors="replace") as fh:
        for n, line in enumerate(fh):
            if not line.strip():
                continue
            try:
                rec = json.loads(line)
            except ValueError:
                drift += 1
                continue
            msg = rec.get("message") if isinstance(rec, dict) else None
            if not isinstance(msg, dict) or "usage" not in msg:
                continue
            usage, model = msg["usage"], msg.get("model")
            if not isinstance(usage, dict) or not isinstance(model, str) or not all(isinstance(usage.get(k), int) for k in COUNTERS):
                drift += 1
                continue
            cc = usage.get("cache_creation") if isinstance(usage.get("cache_creation"), dict) else {}
            cw1 = cc.get("ephemeral_1h_input_tokens")
            cw1 = cw1 if isinstance(cw1, int) else 0
            cw5 = cc.get("ephemeral_5m_input_tokens")
            cw5 = cw5 if isinstance(cw5, int) else usage["cache_creation_input_tokens"] - cw1
            key = rec.get("requestId") or msg.get("id") or f"line{n}"
            by_key[key] = {"ts": parse_ts(rec.get("timestamp")), "model": model, "in": usage["input_tokens"],
                           "cw5": cw5, "cw1": cw1, "cr": usage["cache_read_input_tokens"], "out": usage["output_tokens"]}
    return list(by_key.values()), drift


def read_meta(path):
    try:
        meta = json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return {}
    return meta if isinstance(meta, dict) else {}


def summarize(agent_id, requests, meta, rates):
    counts = collections.Counter(r["model"] for r in requests if r["model"] != SYNTHETIC)
    model = counts.most_common(1)[0][0] if counts else "unknown"
    cost = 0.0
    for r in requests:
        usd = price_request(model if r["model"] == SYNTHETIC else r["model"], r, rates)
        if usd is None:
            cost = None
            break
        cost += usd
    prompts = [prompt_tokens(r) for r in requests]
    desc = meta.get("description")
    return {
        "full_id": agent_id,
        "id": agent_id[:8],
        "agent_type": meta.get("agentType") if isinstance(meta.get("agentType"), str) else "unknown",
        "description": desc if isinstance(desc, str) else "",
        "model": model,
        "requests": len(requests),
        "peak_ctx": max(prompts),
        "processed": sum(p + r["out"] for p, r in zip(prompts, requests)),
        "cache_write_1h": sum(r["cw1"] for r in requests),
        "first_ts": min((r["ts"] for r in requests if r["ts"]), default=None),
        "flagged": model.startswith(HAIKU_PREFIX) and max(prompts) > rates["threshold"],
        "cost_usd": cost,
    }


def totals(rows):
    priced = [r["cost_usd"] for r in rows if r["cost_usd"] is not None]
    return {"agents": len(rows), "processed": sum(r["processed"] for r in rows),
            "cost_usd": sum(priced) if priced or not rows else None,
            "partial": len(priced) != len(rows)}


def sanitize(text, limit=None):
    text = " ".join(re.sub(r"[\x00-\x1f\x7f]+", " ", text).split())
    if limit and len(text) > limit:
        text = text[: limit - 3] + "..."
    return text


def md_cell(text):
    return text.replace("|", "\\|")


def fmt_cost(usd):
    if usd is None:
        return "n/a"
    return f"${usd:.4f}" if usd < 1 else f"${usd:.2f}"


def render_markdown(rows, tot, show_desc, show_all, rates):
    headers = ["Id", "Type", "Description", "Model", "Peak ctx", "Processed", "Est. cost"]
    if not show_desc:
        headers.pop(2)
    width = len(headers)

    def cells(r):
        peak = f"{FLAG} {r['peak_ctx']:,}" if r["flagged"] else f"{r['peak_ctx']:,}"
        out = [r["id"], md_cell(sanitize(r["agent_type"])), md_cell(sanitize(r["description"], DESC_MAX)),
               md_cell(sanitize(r["model"])), peak, f"{r['processed']:,}", fmt_cost(r["cost_usd"])]
        if not show_desc:
            out.pop(2)
        return out

    lines = ["| " + " | ".join(headers) + " |", "|" + "---|" * width]
    shown = rows if show_all else rows[:ROW_CAP]
    lines += ["| " + " | ".join(cells(r)) + " |" for r in shown]
    if len(rows) > len(shown):
        lines.append("| " + " | ".join([f"+{len(rows) - len(shown)} more"] + [""] * (width - 1)) + " |")
    total = [""] * width
    total[0] = f"Total ({tot['agents']})"
    total[headers.index("Processed")] = f"{tot['processed']:,}"
    total[headers.index("Est. cost")] = fmt_cost(tot["cost_usd"]) + (" (partial)" if tot["partial"] else "")
    lines.append("| " + " | ".join(total) + " |")

    flagged = any(r["flagged"] for r in rows)
    unpriced = any(r["cost_usd"] is None for r in rows)
    if flagged or unpriced:
        legend = (f"{FLAG} a request exceeded {rates['threshold'] // 1000}K prompt (Haiku higher tier where rates exist); "
                  f"n/a = no rate on file for that model; rates as of {rates['as_of'].isoformat()}, estimate.")
        if any(r["cache_write_1h"] and r["cost_usd"] is not None for r in rows):
            legend += " 1h cache-write rate unverified."
        lines += ["", legend]
    return "\n".join(lines)


def render_json(rows, tot, show_desc, drift, rates):
    agents = []
    for r in rows:
        obj = {"id": r["id"], "agent_type": r["agent_type"], "model": r["model"], "requests": r["requests"],
               "peak_ctx": r["peak_ctx"], "processed": r["processed"], "cache_write_1h_tokens": r["cache_write_1h"],
               "first_ts": r["first_ts"].isoformat() if r["first_ts"] else None,
               "flagged": r["flagged"], "cost_usd": r["cost_usd"]}
        if show_desc:
            obj["description"] = sanitize(r["description"], DESC_MAX)
        agents.append(obj)
    return json.dumps({"rates_as_of": rates["as_of"].isoformat(), "drift_lines": drift, "agents": agents, "total": tot},
                      indent=2, ensure_ascii=False)


def fail(code, msg):
    print(f"agent_usage: {msg}", file=sys.stderr)
    return code


def main(argv=None):
    ap = argparse.ArgumentParser(description="Per-agent tokens and estimated cost for the current session's subagents.")
    ap.add_argument("--session", help="session id (default: $CLAUDE_CODE_SESSION_ID)")
    ap.add_argument("--since", help="keep agents whose first usage is at/after this ISO8601 time")
    ap.add_argument("--ids", help="comma-separated agent id prefixes to keep")
    ap.add_argument("--all", action="store_true", help="markdown: print every row (default: 5 rows, then '+N more')")
    ap.add_argument("--no-desc", action="store_true", help="omit descriptions")
    ap.add_argument("--json", action="store_true", help="machine-readable output, never capped")
    ap.add_argument("--projects-dir", help="default: $AGENT_USAGE_PROJECTS_DIR or ~/.claude/projects")
    args = ap.parse_args(argv)

    try:
        rates = load_rates(RATES_PATH)
    except RatesError as exc:
        return fail(2, f"{exc} (fix: restore scripts/agent_usage_rates.json)")
    age = (dt.date.today() - rates["as_of"]).days
    if age > STALE_DAYS:
        print(f"agent_usage: warning: rates as of {rates['as_of']} are {age} days old; re-check list prices", file=sys.stderr)

    session = args.session or os.environ.get("CLAUDE_CODE_SESSION_ID", "")
    if not SESSION_RE.fullmatch(session):
        return fail(2, "no session id (cause: CLAUDE_CODE_SESSION_ID unset; fix: pass --session ID)")
    projects = Path(args.projects_dir or os.environ.get("AGENT_USAGE_PROJECTS_DIR") or "~/.claude/projects").expanduser()
    subdirs = sorted(glob.glob(os.path.join(glob.escape(str(projects)), "*", session, "subagents")))
    files = [f for d in subdirs for f in sorted(Path(d).glob("agent-*.jsonl"))]
    if not files:
        return fail(2, f"no subagent transcripts under {projects} (cause: session has no subagents or wrong "
                       f"session; fix: pass --session ID or --projects-dir DIR)")

    rows, drift, parsed = [], 0, 0
    for f in files:
        requests, bad = parse_transcript(f)
        drift += bad
        parsed += len(requests)
        if requests:
            meta = read_meta(f.with_name(f.stem + ".meta.json"))
            rows.append(summarize(f.stem[len("agent-"):], requests, meta, rates))
    if drift:
        print(f"agent_usage: layout drift: {drift} lines skipped", file=sys.stderr)
    if parsed == 0:
        return fail(1, f"no usage records parsed from {len(files)} transcripts (cause: transcript layout changed; "
                       f"fix: inspect one agent-*.jsonl for message.usage)")

    if args.ids:
        prefixes = [p.strip() for p in args.ids.split(",") if p.strip()]
        rows = [r for r in rows if any(r["full_id"].startswith(p) for p in prefixes)]
    if args.since:
        since = parse_ts(args.since)
        if since is None:
            return fail(2, f"bad --since {args.since!r} (want ISO8601, e.g. 2026-10-10T00:00:00Z)")
        rows = [r for r in rows if r["first_ts"] and r["first_ts"] >= since]
    rows.sort(key=lambda r: (r["first_ts"] or EPOCH, r["full_id"]))

    tot = totals(rows)
    if args.json:
        print(render_json(rows, tot, not args.no_desc, drift, rates))
    else:
        print(render_markdown(rows, tot, not args.no_desc, args.all, rates))
    return 0


if __name__ == "__main__":
    sys.exit(main())
