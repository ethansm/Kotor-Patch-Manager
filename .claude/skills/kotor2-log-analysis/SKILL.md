---
name: kotor2-log-analysis
description: >-
  Analyze KOTOR II live mod logs (any *_log.txt in the game dir: force power hotbar, level-up override, buff HUD,
  shield bar/probe, inv3d/equip3d, chareffects, equip icon fix, mouse probe, hang probe, letterbox, perf-lab,
  companion/ranged AI, inventory sort) without bloating agent context, and extract Ghidra dump excerpts.
  Use when the user asks to check/read/analyze mod logs, a play session, "what happened in game", perf-lab runs,
  or to dump decompiled functions.
---

# KOTOR II log analysis

Rule: never `cat`, `tail`, `head`, `grep` or Read a file in the game dir. Every live log goes through `scripts/kotor2_logs.py`, which routes it to the right parser. Each parser's output is short, deterministic, and ends with one verdict line.

## 1. Workflow (2 commands)

```bash
python3 scripts/kotor2_logs.py status              # live logs: size, age, lines, parser (~1 KB)
python3 scripts/kotor2_logs.py check <log_name>    # routed, compact analysis
```

- `<log_name>` is a bare filename in the game dir (e.g. `force_power_hotbar_log.txt`) or a path.
- Extra args pass through to the routed tool, e.g. `check shieldprobe_log.txt --new`.
- Read the last line first (`VERDICT:` / `RESULT:`). If it's PASS/CLEAN, report it and stop. Read the body only for FAIL/ALERTS.
- `python3 scripts/kotor2_logs.py routes` prints the routing table.

## 2. "What's new since I last looked" (no manual line offsets)

```bash
python3 scripts/kotor2_logs.py check <log_name> --new          # only lines added since last --new, then advance
python3 scripts/kotor2_logs.py check <log_name> --new --no-advance
```

Bookmarks live in `~/.cache/kotor2_logs/bookmarks.json`. A rotated or truncated log restarts from line 1 and prints `rotated:`. Use this instead of writing "next log from line N" into docs or memory. This works for the `log_triage` route and `luo_log_check.py` (`levelup_override_log.txt`); other parsers take their own range flags (section 4).

## 3. Generic summary output (`log_triage` route)

Used for every log without a dedicated parser (hotbar, buffhud, shieldprobe, inv3d, equip3d, chareffects, equipiconfix, hangprobe, kplog files). It shows the last launch only (`--all-launches` / `--launch N` to change), capped at 1.8 KB:
- banner, and `patterns: U unique of N lines`
- top signatures, numbers/hex collapsed to `#`, as `count  first_line  signature`
- `alerts:` (warn/err/fail/invalid/... deduplicated), `cap:` if the line cap was hit
- last 5 lines, then `VERDICT: CLEAN|ALERTS(n)|EMPTY`

To look at one signature's real lines, use the `first_line` number with a bounded range only:
`sed -n '<first_line>,+5p' "<game dir>/<log>"`. Never widen this beyond about 20 lines.

## 4. Dedicated parsers (what `check` runs; call directly only for extra flags)

| Log | Tool | Useful flags |
|---|---|---|
| `levelup_override_log.txt` | `scripts/luo_log_check.py` | `--new`, `--no-advance`, `--scenario ID`, `--from/--to LINE`, `--expect-version`, `--json` |
| `k2-dialogue-letterbox-fix_log.txt` | `patch_manager_mods/engine_bug_research/tools/analyze_engfix_logs.py FILE` | `--json` |
| perflab / robustnessfixes / texmem / activationfix | `patch_manager_mods/perf_lab_src/analyze_runs.py --game-dir G` | `--summary`, `--latest N`, `--from-offsets F`, `--out F.md` (default without flags is ~37 KB; `check` uses `--summary --latest 1`) |
| `ai_trace.txt`, `ranged_ai_log.txt`, `inventory_sort_filter_log.txt` | `scripts/archive_play_logs.py` then `patch_manager_mods/mods678_research/analyze_play_logs.py DIR` | `check` chains both into a temp dir |
| KpLog files (`# kplog session` header) | `scripts/kplog_parse.py FILE... --stats` | `--patch`, `--level WARN`, `--since MS`, `--merge` |

Archive logs before a reinstall or new test round: `python3 scripts/archive_play_logs.py` (copies to `~/Games/KOTOR2_Mod_Workspace/logs_archive/<ts>_<tag>/`).

## 5. Ghidra dump excerpts

```bash
bash patch_manager_mods/mods678_research/copy_dump_excerpts.sh 0x0074CFE0 0x005E3210
```

This copies `<addr>.c` and `<addr>.asm` from the steam_6a522e71 full dump into `patch_manager_mods/mods678_research/dump_excerpts/`, next to the script (not the current directory). Check `wc -l` before reading. For symbol or struct facts, `python3 scripts/query_ghidra.py symbol|address|struct` is cheaper.

## 6. A log with no route and odd output

If `log_triage` mis-splits launches or misses an alert class, fix the rule in `scripts/log_triage.py` and add a fixture + test (`scripts/test_log_triage.py`). Don't work around it with ad-hoc greps. To add a dedicated route, edit `ROUTES` in `scripts/kotor2_logs.py` and its test.
