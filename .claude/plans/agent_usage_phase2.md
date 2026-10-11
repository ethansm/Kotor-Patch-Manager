# agent_usage phase 2: robustness, output safety, test gaps

Subject at HEAD 317e36f: `scripts/agent_usage.py` (sha256 5279d5e1...), `scripts/agent_usage_rates.json`, `scripts/test_agent_usage.py` (30 tests).
Triage (authoritative for WHAT): `$S/triage.md`. Paths (shell state does not persist; start EVERY Bash call with this line):
`R=/home/ethansm/orca/workspaces/local-setup/KOTOR-II; S=/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/0ce7e469-1b97-4731-81a8-0bd16e9a154a/scratchpad; H="python3 -I $S/plan2/harness.py"`
Planner pre-work (verified 2026-10-10, scratch only): reference fix `$S/plan2/fixed/{agent_usage.py,agent_usage_rates.json,test_agent_usage.py}` (REF), harness `$S/plan2/harness.py`, HEAD copy `$S/plan2/head/agent_usage.py`.
REF results: 72/72 tests OK; repro 123/123 PASS (HEAD: 53 FAIL); mutants 34/34 (baseline + 33 caught). Every code block below is an exact excerpt of REF.

## A. Goal and scope
Goal: land every triage fix and decision; no other behavior change.

| Triage item | Fix | Chunk |
|---|---|---|
| B1 overflow, B8 bool, B9 negative counters | `valid_count`: int, not bool, 0..10**12, else drift | P1 |
| B2/C7 unhashable requestId | `request_key`: requestId / message.id only if non-empty str, else line key (not drift) | P1 |
| B3, B5 RecursionError (line, meta) | catch with ValueError: line = drift, meta = {} | P1 |
| B4/E2/C1 unguarded open, B12 FIFO | `is_file()` guard + `OSError` skip; stderr names agent id only; meta also `is_file()` | P1 |
| B7 silent non-object lines | drift if line non-object or message neither object nor null | P1 |
| B10/A2 cw1 > total; A1 split residual | single rule cw1 = 1h key or 0, cw5 = total - cw1; cw1 > total = drift | P1 |
| B11 key collision | tagged keys ("req", id) / ("line", n) | P1 |
| A3 last-record ts | ts = earliest record per key | P1 |
| B-open b, A5/B16 | validate --ids/--since before parsing; `--ids ","` exit 2 "--ids has no ids"; `--ids ""` no-op | P1 |
| B-open c | parse_ts accepts trailing lowercase z | P1 |
| B15 non-UTF-8 stdout | `sys.stdout.reconfigure(errors="replace")` | P1 |
| C invalid --session message; E1 | separate "invalid session id" message; --no-desc help names JSON | P1 |
| C4, C5, B6, B17, B-open d | `sanitize`: Cc -> space, other category C dropped | P2 |
| C5/B6 backup | JSON `ensure_ascii=True`; JSON model/agent_type sanitized | P2 |
| E3/C2 id cell; C3 agent_type | `md_cell(r["id"])`, id sanitized; agent_type capped at DESC_MAX (md + JSON) | P2 |
| A6 legend lead clause; E6 legend wording | `legend_text`: one clause per trigger; "unverified estimate" unless all verified flags true | P2 |
| E4 hidden flag | "+N more (🔺 k)" | P2 |
| A4 float noise | JSON `round(cost, 10)`; Markdown unchanged | P2 |
| B13, B14, E7 rates | `valid_rate` (number, not bool, finite, >= 0); RecursionError caught; empty models exit 2 | P2 |
| E6 source text, E5/C6 docstring | rates `source` names what merge.py covers; docstring lists all output fields | P2 |
| D survivors (15) + D stderr convention | new tests | P3a |
| One regression test per B/A/C/E item | new tests | P3b |

Decided judgment calls (do not reopen): E4 keep legend, count hidden flags; E1 --no-desc also drops JSON description (documented in --help/docstring); C6 extra JSON fields kept; E7 empty models exit 2; A5 `--ids ","` exit 2; B7 drift definition (see F1); B2 non-str ids fall back to the line key; A1 single cache rule; A3 earliest record = first usage; C3 cap = DESC_MAX.
Out of scope: CLAUDE.md and AGENTS.md (flags unchanged, no doc edit); rate values; c39 usage-outside-message (stays silent per B7); any file other than the three scripts files; push.

## B. Context-budget split
Measured: five Haiku 5.5 reviewers started near 34K prompt tokens; four passed 100K at requests 11-18; the 9-request agent stayed at 85K. Per-Bash hook attachments (about 3K chars, two per call) and hidden tool output drove most growth; visible tool results added only 11-18K.
Model used here: growth per agent = about 2.5K tokens per Bash call (1.5K hooks + capped output) + ranged reads + edit echoes.
1. Prep is one step (P0). Fixtures already exist under `$S/review/*` (reviewer-built); the planner built and verified one harness. One Bash call runs many checks and prints only FAIL lines (cap 40) plus SUMMARY lines.
2. One fresh agent per chunk, strictly serial: all chunks touch the same two files; never two coding agents on `scripts/agent_usage.py` at once; never continue a finished agent with SendMessage.
3. Each agent starts from this file alone. Brief = `Contract: worker` + `Write Scope: <glob>` + output of `python3 scripts/agent_contract.py print worker` + "Execute Chunk Pn of $R/.claude/plans/agent_usage_phase2.md. Read lines 1-8 of the plan, then only your chunk: `sed -n '/^### Chunk Pn:/,/^### Chunk P/p' <plan>`. Stop at the Bash budget. Report the 5-line worker contract only."
4. The lead reads only the agent's STATUS line, then re-runs the chunk's exit-gate command itself (one Bash call, tail-limited). Never the transcript.
5. Reads are ranged (Read offset/limit at the anchors given). No whole-file reads of the test file, no `git diff` body (`--stat` only), no raw harness `-v`.
6. Bash cap 8 per agent. An agent at 6 calls without a green gate stops and reports `STATUS: FAIL` with the FAIL lines.
7. Ladder (per FAIL name): Haiku layer 1 -> new Haiku layer 2 with the FAIL lines plus leave to Read the REF function that `$H fncmp` names -> Haiku layer 3 same -> Sonnet only after 3 Haiku failures on the same FAIL name.

| Chunk | Owner | Reads | Edits | Bash budget / cap | Est. growth | Est. end |
|---|---|---|---|---|---|---|
| P0 | lead | none | 0 | 2 / 2 | ~4K (lead) | n/a |
| P1 | Haiku | chunk ~4K + 4 ranges ~2.5K | 9 | 3 / 8 | ~22K | ~56K |
| P2 | Haiku | chunk ~5K + rg + ranges ~3K | 13 | 4 / 8 | ~27K | ~61K |
| P3a | Haiku | chunk ~2K + REF ranges ~3K | 5 | 3 / 8 | ~18K | ~52K |
| P3b | Haiku | chunk ~2K + REF range ~4K | 1 | 3 / 8 | ~16K | ~50K |
| P4 | Haiku | chunk + section D ~2K | 0 | 2 / 8 | ~8K | ~42K |
Rejected: P1+P2 in one agent (about 49K growth, end about 83K, inside the 85-100K zone where reviewers degraded).
Lead procedure per chunk: (1) `cd $R && python3 scripts/agent_contract.py snapshot`; (2) Agent(model haiku, brief from rule 3); (3) read STATUS; `printf '%s\n' "<report>" | python3 scripts/agent_contract.py check --scope <glob> -`; re-run the exit-gate command; tick the box in section G.

## C. Phases

### Chunk P0: verify harness, fixtures, baseline (owner: lead; spawning a 34K agent for 2 commands costs more than it saves)
1. `cd $R && sha256sum $S/plan2/harness.py $S/plan2/fixed/agent_usage.py $S/plan2/fixed/agent_usage_rates.json $S/plan2/fixed/test_agent_usage.py scripts/agent_usage.py | cut -c1-8; git status --short -- scripts/agent_usage.py scripts/agent_usage_rates.json scripts/test_agent_usage.py`
   Expect `91a5b313 2a10c8bb 49bf9f76 966cda49 5279d5e1` in order and no git status lines.
2. `$H snap | head -1; $H repro | tail -1; $H repro --script $S/plan2/fixed/agent_usage.py | tail -1; $H mut --script $S/plan2/fixed/agent_usage.py --tests $S/plan2/fixed/test_agent_usage.py | tail -2` (about 90 s)
   Expect: `INFO snapshot ...`, `HARNESS FAIL (53)`, `HARNESS OK`, `SUMMARY mut: pass=34 fail=0 skip=0`, `HARNESS OK`.
3. Exit gate: both steps as expected. If `$S/review` or `$S/plan2` is missing (reboot cleared /tmp): STOP, report "fixtures gone", do not start P1.

### Chunk P1: input robustness and I/O (owner Haiku; Write Scope `scripts/agent_usage.py`; Bash budget 3)
1. Read `scripts/agent_usage.py` ranges (HEAD numbering): 30-33, 43-51, 98-133, 243-301.
2. After the line `STALE_DAYS = 60` insert the line `MAX_COUNT = 10 ** 12`.
3. Replace all of `def parse_ts` (lines 43-51) with:
```python
def parse_ts(value):
    """ISO8601 string -> aware datetime (naive assumed UTC; trailing Z or z = UTC). None when unparseable."""
    if not isinstance(value, str):
        return None
    if value[-1:] in ("Z", "z"):
        value = value[:-1] + "+00:00"
    try:
        ts = dt.datetime.fromisoformat(value)
    except ValueError:
        return None
    return ts if ts.tzinfo else ts.replace(tzinfo=UTC)
```
4. Replace all of `def parse_transcript` (lines 98-125) with these three functions (two blank lines between functions):
```python
def valid_count(value):
    return isinstance(value, int) and not isinstance(value, bool) and 0 <= value <= MAX_COUNT


def request_key(rec, msg, n):
    """("req", requestId or message.id) when a non-empty str, else ("line", n); tags keep the two apart."""
    for value in (rec.get("requestId"), msg.get("id")):
        if isinstance(value, str) and value:
            return ("req", value)
    return ("line", n)


def parse_transcript(path):
    """Return (requests, drift_lines). Partials sharing a key: counters from the last record, ts from the earliest.

    Drift: unparseable or too-deep JSON, a non-object line, a "message" that is neither an object nor null, or a
    usage block that fails validation. Lines without usage (user turns, tool results, null message) are not drift.
    """
    by_key, drift = {}, 0
    with open(path, encoding="utf-8", errors="replace") as fh:
        for n, line in enumerate(fh):
            if not line.strip():
                continue
            try:
                rec = json.loads(line)
            except (ValueError, RecursionError):
                drift += 1
                continue
            msg = rec.get("message") if isinstance(rec, dict) else None
            if not isinstance(rec, dict) or not isinstance(msg, (dict, type(None))):
                drift += 1
                continue
            if msg is None or "usage" not in msg:
                continue
            usage, model = msg["usage"], msg.get("model")
            if not isinstance(usage, dict) or not isinstance(model, str) or not all(valid_count(usage.get(k)) for k in COUNTERS):
                drift += 1
                continue
            cc = usage.get("cache_creation")
            cw1 = cc.get("ephemeral_1h_input_tokens") if isinstance(cc, dict) else None
            cw1 = cw1 if valid_count(cw1) else 0
            cc_total = usage["cache_creation_input_tokens"]
            if cw1 > cc_total:
                drift += 1
                continue
            cw5 = cc_total - cw1
            key = request_key(rec, msg, n)
            ts = parse_ts(rec.get("timestamp"))
            prev = by_key.get(key)
            if prev and prev["ts"] and (ts is None or prev["ts"] < ts):
                ts = prev["ts"]
            by_key[key] = {"ts": ts, "model": model, "in": usage["input_tokens"], "cw5": cw5, "cw1": cw1,
                           "cr": usage["cache_read_input_tokens"], "out": usage["output_tokens"]}
    return list(by_key.values()), drift
```
5. Replace all of `def read_meta` (lines 128-133) with:
```python
def read_meta(path):
    path = Path(path)
    try:
        if not path.is_file():
            return {}
        meta = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError, RecursionError):
        return {}
    return meta if isinstance(meta, dict) else {}
```
6. `main` edit M1: directly after `def main(argv=None):` insert:
```python
    if hasattr(sys.stdout, "reconfigure"):
        try:
            sys.stdout.reconfigure(errors="replace")
        except (ValueError, OSError):
            pass
```
7. `main` edit M2: `help="omit descriptions")` becomes `help="omit descriptions (Markdown column and JSON key)")`.
8. `main` edit M3: between `    args = ap.parse_args(argv)` + blank line and `    try:` insert this block followed by one blank line:
```python
    prefixes = since = None
    if args.ids:
        prefixes = [p.strip() for p in args.ids.split(",") if p.strip()]
        if not prefixes:
            return fail(2, "--ids has no ids (fix: pass comma-separated agent id prefixes)")
    if args.since:
        since = parse_ts(args.since)
        if since is None:
            return fail(2, f"bad --since {args.since!r} (want ISO8601, e.g. 2026-10-10T00:00:00Z)")
```
9. `main` edit M4: replace the two lines `if not SESSION_RE.fullmatch(session):` / `return fail(2, "no session id (...)")` with:
```python
    if not session:
        return fail(2, "no session id (cause: CLAUDE_CODE_SESSION_ID unset; fix: pass --session ID)")
    if not SESSION_RE.fullmatch(session):
        return fail(2, "invalid session id (cause: want 1-100 chars of A-Z a-z 0-9 _ -; fix: pass --session ID)")
```
10. `main` edit M5: replace `    for f in files:` + `        requests, bad = parse_transcript(f)` with the block below, and change `summarize(f.stem[len("agent-"):], requests, meta, rates)` to `summarize(agent_id, requests, meta, rates)`:
```python
    for f in files:
        agent_id = f.stem[len("agent-"):]
        try:
            if not f.is_file():
                raise OSError("not a regular file")
            requests, bad = parse_transcript(f)
        except OSError:
            print(f"agent_usage: skipped agent {sanitize(agent_id, DESC_MAX) or '?'} "
                  f"(cause: transcript is not a readable regular file)", file=sys.stderr)
            continue
```
11. `main` edit M6: replace the 8 lines from `    if args.ids:` (second occurrence, after the parse loop) through `rows = [r for r in rows if r["first_ts"] and r["first_ts"] >= since]` with:
```python
    if prefixes is not None:
        rows = [r for r in rows if any(r["full_id"].startswith(p) for p in prefixes)]
    if since is not None:
        rows = [r for r in rows if r["first_ts"] and r["first_ts"] >= since]
```
12. Exit gate (one Bash call; harness prints only FAIL, INFO, SUMMARY, HARNESS lines, so no tail): `$H unit; $H fncmp parse_ts valid_count request_key parse_transcript read_meta main; $H repro | cut -c1-48`
    Expect: unit pass=1; fncmp pass=6 fail=0; repro `HARNESS FAIL (11)` with exactly these FAIL names (all owned by P2): B/r04_rate_bool, B/r05_rate_inf, B/r07_deep_rates, B/r12_models_empty, C/fname, C/inj:md, C/inj:json, C/c1, E/wide, E/pipe, E/rates_empty_models. A fncmp FAIL prints the first differing line; fix that line only.

### Chunk P2: output, rates, docs (owner Haiku; Write Scope `scripts/agent_usage.py scripts/agent_usage_rates.json`; Bash budget 4)
1. `cd $R && rg -n '^def |^import |^Output holds|"id": agent_id|^    flagged = ' scripts/agent_usage.py`, then Read each target range below.
2. Docstring: replace the two lines starting `Output holds only` and `Exit codes:` with:
```text
Output fields: agent id (8 chars), type, model, description, request count, peak ctx, processed,
1h cache-write tokens, first-usage time, flag and cost; JSON adds rates_as_of, drift_lines and a total.
--no-desc drops descriptions from both Markdown and JSON. Never prompt text, transcript paths or session id.
Exit codes: 0 ok, 1 transcripts found but no usage parsed, 2 bad arguments, subagents dir missing or bad config.
```
3. Imports: add `import math` after `import json`; add `import unicodedata` after `import sys`.
4. Replace all of `def load_rates` with:
```python
def valid_rate(value):
    return isinstance(value, (int, float)) and not isinstance(value, bool) and math.isfinite(value) and value >= 0


def load_rates(path=None):
    """Validate the rates file. Raises RatesError (fail closed) on any schema problem."""
    path = RATES_PATH if path is None else path
    try:
        raw = json.loads(Path(path).read_text(encoding="utf-8"))
        as_of = dt.date.fromisoformat(raw["as_of"])
        threshold = raw["tier_threshold_tokens"]
        models = raw["models"]
        if not isinstance(threshold, int) or isinstance(threshold, bool) or threshold <= 0:
            raise ValueError("tier_threshold_tokens must be a positive int")
        if not isinstance(models, list) or not models:
            raise ValueError("models must be a non-empty list")
        entries = []
        for m in models:
            for tier in ("low", "high"):
                rates = m[tier]
                if set(rates) != set(RATE_KEYS) or not all(valid_rate(v) for v in rates.values()):
                    raise ValueError(f"{tier} rates must be exactly {RATE_KEYS}, finite non-negative numbers")
            verified = m.get("verified", {})
            if not isinstance(verified, dict):
                raise ValueError("verified must be an object")
            entries.append({"match": re.compile(m["match"]), "low": m["low"], "high": m["high"],
                            "source": m.get("source", ""), "verified": all(verified.get(k) is True for k in RATE_KEYS)})
    except (OSError, ValueError, KeyError, TypeError, RecursionError, re.error) as exc:
        raise RatesError(f"bad rates file {Path(path).name}: {exc}") from exc
    return {"as_of": as_of, "threshold": threshold, "models": entries, "verified": all(e["verified"] for e in entries)}
```
5. In `summarize`: `"id": agent_id[:8],` becomes `"id": sanitize(agent_id)[:8],`.
6. Replace all of `def sanitize` with:
```python
def sanitize(text, limit=None):
    """Controls (Cc) -> space; other Unicode category C chars (Cf bidi/format, Cs surrogate, Co, Cn) dropped."""
    out = []
    for ch in text:
        cat = unicodedata.category(ch)
        if cat == "Cc":
            out.append(" ")
        elif cat[0] != "C":
            out.append(ch)
    text = " ".join("".join(out).split())
    if limit and len(text) > limit:
        text = text[: limit - 3] + "..."
    return text
```
7. After `def fmt_cost` (and its two trailing blank lines) insert:
```python
def round_cost(usd):
    return None if usd is None else round(usd, 10)


def legend_text(rows, rates):
    """One clause per trigger that fired (flag, n/a), then the rates clause; "" when nothing fired."""
    clauses = []
    if any(r["flagged"] for r in rows):
        clauses.append(f"{FLAG} a request exceeded {rates['threshold'] // 1000}K prompt (Haiku higher tier where rates exist)")
    if any(r["cost_usd"] is None for r in rows):
        clauses.append("n/a = no rate on file for that model")
    if not clauses:
        return ""
    quality = "estimate" if rates["verified"] else "unverified estimate"
    clauses.append(f"rates as of {rates['as_of'].isoformat()}, {quality}")
    return "; ".join(clauses) + "."
```
8. `render_markdown` edit R1: `out = [r["id"], md_cell(sanitize(r["agent_type"])),` becomes `out = [md_cell(r["id"]), md_cell(sanitize(r["agent_type"], DESC_MAX)),` (rest of the line unchanged).
9. `render_markdown` edit R2: replace the 4 lines from `shown = rows if show_all else rows[:ROW_CAP]` through the `+{len(rows) - len(shown)} more` append with:
```python
    shown = rows if show_all else rows[:ROW_CAP]
    hidden = rows[len(shown):]
    lines += ["| " + " | ".join(cells(r)) + " |" for r in shown]
    if hidden:
        more = f"+{len(hidden)} more"
        hidden_flags = sum(1 for r in hidden if r["flagged"])
        if hidden_flags:
            more += f" ({FLAG} {hidden_flags})"
        lines.append("| " + " | ".join([more] + [""] * (width - 1)) + " |")
```
10. `render_markdown` edit R3: replace the blank line after the Total `lines.append(...)` plus everything through `lines += ["", legend]` (the `flagged =` / `unpriced =` / legend block) with (no blank line before it):
```python
    legend = legend_text(rows, rates)
    if legend:
        lines += ["", legend]
```
11. Replace all of `def render_json` with:
```python
def render_json(rows, tot, show_desc, drift, rates):
    agents = []
    for r in rows:
        obj = {"id": r["id"], "agent_type": sanitize(r["agent_type"], DESC_MAX), "model": sanitize(r["model"]),
               "requests": r["requests"], "peak_ctx": r["peak_ctx"], "processed": r["processed"],
               "cache_write_1h_tokens": r["cache_write_1h"],
               "first_ts": r["first_ts"].isoformat() if r["first_ts"] else None,
               "flagged": r["flagged"], "cost_usd": round_cost(r["cost_usd"])}
        if show_desc:
            obj["description"] = sanitize(r["description"], DESC_MAX)
        agents.append(obj)
    total = dict(tot, cost_usd=round_cost(tot["cost_usd"]))
    return json.dumps({"rates_as_of": rates["as_of"].isoformat(), "drift_lines": drift, "agents": agents, "total": total},
                      indent=2, ensure_ascii=True)
```
12. `scripts/agent_usage_rates.json` line 9: the `"source"` value becomes `"input, output, cache_write_5m, cache_read mirrored from ~/.claude/skills/deep-haiku-review/bundle/engine/merge.py:33-45; cache_write_1h is not in merge.py (third-party listing, 2x input); no rate verified against an Anthropic price page"`.
13. Exit gate (one Bash call): `cd $R && cmp scripts/agent_usage.py $S/plan2/fixed/agent_usage.py && cmp scripts/agent_usage_rates.json $S/plan2/fixed/agent_usage_rates.json && echo REF-IDENTICAL; $H unit; $H repro | cut -c1-100`
    Expect `REF-IDENTICAL`, unit pass=1, repro A 5 / B 93 / C 17 / E 8 pass, `HARNESS OK`. On a cmp miss: `$H fncmp valid_rate load_rates summarize sanitize round_cost legend_text render_markdown render_json` names the line; header lines: `diff <(sed -n 1,45p scripts/agent_usage.py) <(sed -n 1,45p $S/plan2/fixed/agent_usage.py)`.

### Chunk P3a: tests for the 15 reviewer-D survivors (owner Haiku; Write Scope `scripts/test_agent_usage.py`; Bash budget 3)
Copy REF ranges verbatim (`REF=$S/plan2/fixed/test_agent_usage.py`; repo anchors are HEAD numbering).
1. Imports/constants: replace repo lines 3-17 with REF lines 3-24 (adds `datetime as dt`, `os`, `re`, `subprocess`, `from unittest import mock`, `SCRIPT`, `DEEP`).
2. After `Base.rates` (repo `return A.load_rates(A.RATES_PATH)`) insert REF lines 63-80 (`run_cli`, `data`, module-level `with_usage`).
3. `test_total_equals_sum_of_unrounded_rows`: last line becomes REF line 172 (`assertAlmostEqual(..., places=10)`; JSON costs are now rounded).
4. `test_missing_subagents_dir_exit_2` body becomes REF lines 206-210 (stderr redirected, asserts "no subagent transcripts").
5. After the last line of `Filters.test_no_message_text_leaks` (repo line 268) insert REF lines 297-403 (starts with a blank line, ends at the last assertion; the two existing blank lines before `if __name__` stay). Content (input -> expected; mutant killed):

| Test | Input -> expected | Kills |
|---|---|---|
| Filters.test_shown_rows_are_earliest_in_order | 7-agent setUp, default md -> ids 00aa0..04aa4 in order | sort_reversed |
| Filters.test_since_keeps_agent_at_exact_cutoff | --since 2026-10-05T01:00:00Z -> [04aa4, 05aa5, 06aa6] | since_gt_not_ge |
| Filters.test_hidden_flag_counted_in_more_row | + Haiku 150000 @10-08 -> "+3 more (🔺 1)", legend clause present | E4 |
| test_low_tier_cache_read_priced_at_low_rate | Haiku cr=1000 -> 1e-05 | low_tier_cache_read_uses_high, all_tiers_cache_read_high |
| test_output_excluded_from_tier_decision | in=100000 out=500000 -> 0.26 (low tier) | output_counted_in_tier_decision |
| test_rows_sorted_by_first_ts_not_filename | b@10-02, a@10-03, c@10-01 -> JSON ids [c, b, a] | sort_reversed |
| test_markdown_flag_marker | Haiku 150000 -> "🔺 150,000" | md_flag_marker_dropped |
| test_markdown_description_truncated | desc "x"*80 -> "x"*37+"...", no "x"*38 | md_desc_not_truncated |
| test_invalid_session_message | --session ../x -> 2, "invalid session id", no "unset" | session_regex_guard_removed |
| test_unset_session_message | env unset, no --session -> 2, "CLAUDE_CODE_SESSION_ID unset" | C open item |
| test_session_env_fallback | env CLAUDE_CODE_SESSION_ID -> 0, 1 agent | session_env_fallback_removed |
| test_projects_dir_env_fallback | env AGENT_USAGE_PROJECTS_DIR -> 0, 1 agent | projects_env_fallback_removed |
| test_legend_flag_clause_only_when_flagged | Haiku 150000 -> "a request exceeded 100K prompt", no "n/a =" | legend_ignores_flag_branch |
| test_legend_na_only_has_no_flag_clause | sonnet -> "n/a = no rate on file", no "exceeded" | A6 |
| test_legend_quality_follows_verified_flags | default -> "unverified estimate"; all verified true -> ", estimate." | unverified_1h_legend_removed |
| test_naive_timestamp_assumed_utc | ts 2026-10-10T01:00:00 + --since ...Z -> first_ts "...+00:00" | naive_ts_utc_assumption_removed |
| test_cost_format_boundary_one_dollar | fmt_cost(1.0)="$1.00", fmt_cost(0.99994)="$0.9999" | fmt_cost_boundary_le_1 |
`cw5_fallback_no_1h_subtract` is killed by the existing `test_cache_write_split_5m_and_1h` under the new single rule (verified).
6. Exit gate (one Bash call): `cd $R && grep -c '    def test_' scripts/test_agent_usage.py; $H unit; $H mut --set d15`
   Expect `47`, unit pass=1, `SUMMARY mut: pass=16 fail=0`.

### Chunk P3b: regression tests for B, A, C, E items (owner Haiku; Write Scope `scripts/test_agent_usage.py`; Bash budget 3)
1. Read REF lines 406-588 and insert them verbatim directly before the line `if __name__ == "__main__":` (they end with two blank lines; the file then has two blank lines after SurvivorGaps and two before `if __name__`). Content:

| Class | Tests (input -> expected) | Items |
|---|---|---|
| InputRobustness (11) | counter_validator: ok 10, max 10**12, True, -5, 10**12+1, 10**310 -> drift 4, requests 2, processed 10**12+10; unhashable_ids_fall_back -> drift 0, requests 2; request_id_cannot_collide_with_line_key ("line1" + id-less line) -> requests 2; deep_json_line_is_drift -> drift 1; drift_rules (2 plain, 6 bad, 1 ok) -> drift 6; cache_1h_above_total_is_drift (cc 100, 1h 500) -> drift 1; cache_split_total_wins_over_5m_key (cc 5000, 5m 2000) -> peak 5000, 1h 0, cost 0.000625; first_ts_is_earliest_partial -> first_ts 2026-10-09T23:59:59+00:00, processed 60, --since 10-10 -> []; lowercase_z_timestamp; bad_args_checked_before_parse (only "not json": --since garbage -> 2, --ids "," -> 2); empty_ids_string_is_noop -> 1 agent | B1 B8 B9 B2 B11 B3 B7 B10 A1 A3 B-c B-b B16 |
| FileGuards (4) | dir + broken symlink + self-loop -> rc 0, 1 agent, 3 x "skipped agent", no temp root or session id on stderr; mode-0 file (skipped as root) -> "skipped agent noread00"; FIFO transcript + FIFO meta via subprocess (timeout 20) -> rc 0; deep meta -> agent_type "unknown" | B4 C1 E2 B12 B5 |
| OutputSafety (8) | sanitize("a‮b⁧c\x9bd\ud800e") == "abc de"; surrogate model/type/desc via subprocess md+json -> rc 0, UTF-8 stdout; PYTHONIOENCODING=ascii -> rc 0, b"? 150,000"; filename agent-\xff\xfeab.jsonl -> rc 0 (linux); id "pi\|pe000" -> 8 unescaped pipes; agentType "t"*10000 -> md lines < 300, JSON len 40; in=1 + in=2 -> JSON cost 3e-07 exactly; desc "café 🔺" -> ASCII JSON that round-trips | C4 C5 B6 B15 B17 E3 C3 A4 |
| RatesValidation (2) | load_rates raises RatesError for bool, 1e999, NaN, empty models, verified list, deep JSON; empty models via CLI -> rc 2, stdout "", "bad rates file" | B13 B14 E7 |
2. Exit gate (one Bash call): `cd $R && cmp scripts/test_agent_usage.py $S/plan2/fixed/test_agent_usage.py && echo REF-IDENTICAL; grep -c '    def test_' scripts/test_agent_usage.py; $H unit; $H mut`
   Expect `REF-IDENTICAL` (soft; a mismatch is fine only if all else passes), `72`, unit pass=1, `SUMMARY mut: pass=34 fail=0` (about 40 s; the is_file mutant waits on the 20 s FIFO timeout by design).

### Chunk P4: full verification (owner Haiku; read-only, Write Scope none; Bash budget 2)
1. `$H all 2>&1 | cut -c1-200 | tail -40` (3-5 min; reads `~/.claude/projects` read-only; writes only under `$S/plan2/work`).
2. If `real/gate4` FAILs, apply F2 exactly; re-run `$H real 2>&1 | cut -c1-200` once (second Bash call).
3. Exit gate: `HARNESS OK`; every SUMMARY line fail=0 (unit, A, B, C, E, mut, real, git). Report in TESTS: the SUMMARY counts plus `INFO real sessions=N ... agents=M`.

## D. Verification gates (all required before commit; all inside `$H all`)
1. Unit: `python3 scripts/run_tests.py test_agent_usage` passes (scoped; never bare run_tests.py). Harness name `unit/run_tests.py test_agent_usage`.
2. Real session cfae0c38-3942-4d57-88ac-872e71647e20, `--since 2026-10-10T00:00:00Z --json`: a8a447b6 peak 33220, processed 33225, cost 0.0021671750 (abs diff <= 1e-9); ad4257d0 peak 34820, processed 206657, cost 0.004279455. Also A6: with `--all`, no "exceeded" clause unless a row shows 🔺. Names `real/gate2:*`, `real/A6_legend_clause`.
3. Session 0ce7e469 `--ids aba5d398 --json`: peak 106704, processed 1688841, cost 0.101102965 (<= 1e-9). Name `real/gate3:aba5d398`.
4. Every `~/.claude/projects/*/*/subagents` session (unique ids matching the session regex) run with `--json`: drift_lines 0, rc in {0, 1}, no Traceback. INFO line reports session and agent counts. Name `real/gate4:zero_new_drift_all_sessions` (F2 covers pre-existing drift).
5. Reviewer repros (`repro`): all B fixtures (67 mut dirs + 18 rates variants + 8 extras), C (17), E (8), A (5): documented rc 0/1/2, no Traceback, no `/subagents` path on stderr, no session id on stdout or stderr (projects-dir text excluded).
6. Mutation (`mut`): the 15 reviewer-D survivors (MUTANTS[:15], patterns re-anchored to the new code) plus 18 new-logic mutants all caught; BASELINE passes.
7. Git: `git diff --stat` covers exactly the three scripts files; CLAUDE.md and AGENTS.md blob ids equal the P0 snapshot; INFO lines show each file identical to REF.

## E. Commit (owner: lead, after P4 `HARNESS OK`)
1. Write `$S/plan2/commit_msg.txt` with the Write tool:
```text
fix(agent_usage): harden transcript parsing, output safety and rates validation

- validate counters (int, not bool, 0..1e12); drift = non-object line, non-object message, bad usage, 1h > total
- str-only tagged request keys; earliest timestamp per request; single cache-write split rule
- skip non-regular or unreadable transcripts (id-only message); RecursionError handled; FIFO-safe meta
- sanitize drops Unicode category C; ASCII JSON; escaped id cell; agent_type capped; stdout errors=replace
- legend clauses per trigger with verified-flag wording; "+N more (flag k)"; JSON cost rounded to 1e-10
- rates reject bool/inf/nan/empty models; accurate source text; --ids/--since validated before parsing
- tests: 42 new (15 reviewer-D survivors, B/A/C/E regressions); 33 of 33 mutants caught

Co-Authored-By: Claude Haiku 5.5 <noreply@anthropic.com>
```
2. `cd $R && git commit -F $S/plan2/commit_msg.txt -- scripts/agent_usage.py scripts/agent_usage_rates.json scripts/test_agent_usage.py && git show --stat --oneline HEAD | head -5 && git status --short CLAUDE.md AGENTS.md`
   Expect 3 files changed; CLAUDE.md and AGENTS.md still ` M` (unrelated edits not swept). The index-blob method is not needed (docs untouched). No push.

## F. Risks and open questions (decision chosen)
1. Drift definition: decided in triage (B7): drift = unparseable or too-deep JSON, non-object line, message present and not an object, failed usage validation, 1h > total. Plain lines without usage stay silent (174 of 217 real lines). Planner reading: `"message": null` counts as absent (silent), so real sessions stay at 0 drift; c39 usage-outside-message stays silent.
2. Gate-4 fallback: a session whose drift equals the HEAD script's drift prints as `INFO pre-existing drift` and does not fail (not caused by this work). New drift fails: if the only new drift is 1 line in session 0ce7e469 (a transcript mid-write), re-run `$H real` once; if it persists, STOP, do not commit, report the `INFO drift shapes` line (types and keys only, no content) to the lead as a triage question.
3. Hook-attachment cause of context growth is inferred from size residuals, not confirmed. Mitigation: caps in section B (Bash cap 8, stop at 6), tail-limited output, fresh agent per chunk.
4. Rates are unverified estimates (all verified flags false); the legend now says "unverified estimate"; values unchanged here.
5. The planner did not read `~/.claude` (gates 2-4 unrun by the planner). Gate values come from reviewer A on HEAD; the fix changes only JSON rounding for those agents. The real-session copy under `$S/review/b-robustness/proj` shows 0 drift with REF.
6. A reboot clears `/tmp`: fixtures, harness and REF go with it. Decision: P0 STOP rule; regeneration is out of scope.
7. sanitize detail: Cc becomes a space (keeps the tested "\n" -> " " behavior); Cf, Cs, Co, Cn are dropped (a ZWJ inside an emoji sequence is dropped; accepted).
8. Commit trailer: Haiku 5.5 only, as instructed.
9. Gate 7 doc check fails if another session edits CLAUDE.md or AGENTS.md after P0. Decision: the lead runs `git diff --stat -- CLAUDE.md AGENTS.md`; if the edit is not from this plan, `$H snap` again and re-run `$H all`; the commit never includes those files.
10. Planner simulation: applying this plan's code blocks and REF ranges to the HEAD files reproduces REF byte for byte (all three files).

## G. Handoff
- [ ] Chunk P0: verify harness, fixtures, baseline (lead)
- [x] Chunk P1: input robustness and I/O
- [x] Chunk P2: output, rates, docs
- [x] Chunk P3a: survivor tests
- [x] Chunk P3b: regression tests
- [x] Chunk P4: full verification
- [x] Commit (section E, lead): 0d67415
Resume: `python3 scripts/plan_manager.py resume .claude/plans/agent_usage_phase2.md`; if that does not parse this plan, `sed -n '/^## G\./,$p' .claude/plans/agent_usage_phase2.md` and start the first unchecked chunk.
Lead summary:
1. Six chunks (P0 lead; P1, P2, P3a, P3b, P4 Haiku), strictly serial, one fresh agent each; largest Bash budget 4, cap 8.
2. Code is exact and pre-verified: REF in `$S/plan2/fixed` passes 72 tests, 123 repros, 34 of 34 mutation checks.
3. P1/P2 gates use `$H fncmp` and `cmp` against REF, so transcription errors show the exact line.
4. Commit only after `$H all` is green, explicit three-path commit, Haiku trailer, no push.
5. Main open risk: real-session drift shapes unseen by the planner; F2 decides.

### Agents Used
- 5 Haiku agents, run serially: P1 PASS, P2 PASS, P3a PASS, P3b PASS, P4 FAIL on one gate. Cause confirmed, harness re-run, passed.
- Peak context = largest single-request prompt per agent.

| Chunk | Agent | Peak ctx |
|---|---|---|
| P1 | a16fb115 | 55,835 |
| P2 | afa1b67f | 54,986 |
| P3a | a20f848b | 64,361 |
| P3b | a30b19dd | 53,860 |
| P4 | a4fba27e | 43,787 |

- Over 100K: 0 of 5. Under 100K: 5 of 5. Flagged (🔺): 0.
- Median 54,986; mean 54,566; range 43,787 to 64,361.
- agent_usage.py: 1,808,817 tokens processed, est. $0.0719.
