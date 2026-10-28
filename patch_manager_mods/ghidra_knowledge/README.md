# Ghidra knowledge base

`kotor2_ghidra_knowledge.db` (SQLite) persists facts learned from Ghidra
headless analysis of the KOTOR2 Steam/Aspyr binary so future sessions don't
re-run expensive headless Ghidra jobs (each one costs several minutes plus a
large decompile/disassembly dump) to rediscover the same class layouts,
vtable addresses, function boundaries, or stack-frame offsets.

**Rule: any time a Ghidra headless job resolves a class, function, vtable,
struct offset, or stack-frame offset that could plausibly matter again,
persist it here before moving on.** Cheap to do at the time; expensive to
redo cold in a future session.

This lives in the `local-setup`/`KOTOR-II` worktree because, per current
policy, that's the only repo we commit to for now (see
[[kotor2-patch-manager]] memory) — the `kotor-patch-manager` clone itself
stays uncommitted until a mod is ready to contribute upstream.

**Briefing a Haiku/Sonnet analyst or judge?** Read `RE_AGENT_GUIDE.md` first (tiered funnel, ABI/decimal/thunk/decoy
traps, prompt schema, calibration). Pre-processing helper: `tools/re_card_prep.py`.

## Schema

- `binaries` — one row per known binary hash (GOG, Steam pristine, Steam+LAA),
  with its Ghidra project path/program name if one exists locally.
- `classes` — RTTI-derived class name + vftable/vftable_meta_ptr address,
  per binary.
- `class_methods` — a class's vtable dump (index, address, size, purpose),
  per binary. Full vtable dumps are expensive (headless run + parsing) —
  keep them complete even for boilerplate slots, so a future "is this vtable
  slot already known" check is a single query, not a re-dump.
- `functions` — standalone functions worth remembering (not tied to a vtable
  slot), with a purpose note and why it was ruled in/out for whatever task
  found it.
- `stack_offsets` — named locals within a specific function (e.g. "this",
  "xScale") and their EBP-relative offset, plus notes on how confident/
  verified that offset is. Cross-binary comparison rows (e.g. "matches GOG
  exactly" / "differs from GOG") go in `notes`.
- `hooks_ported` — for each patch's hook function, the GOG source
  address/bytes and the Steam target address/bytes once found, so a repeat
  or adjacent port doesn't redo the search.
- `lessons` — freeform methodology notes (Ghidra API gotchas, porting
  gotchas, decompiler quirks) that aren't tied to one address. Read these
  FIRST before starting a new Ghidra investigation — several are non-obvious
  API traps (e.g. namespace-qualified symbol names) that cost real time to
  discover once already.
- `ghidra_scripts` — which of the ad-hoc `.java` scripts in
  `kotor-patch-manager/tools/ghidra_scripts/` are generic/reusable
  templates vs. one-off, and what to edit to reuse them.

## Querying

**Free script tools first** (no sqlite needed): `python3 scripts/query_ghidra.py symbol|address|struct|stats`
(also reads the REA ledger `rea_findings/`, tagging rows `[rea]` / `[superseded]`), and
`python3 patch_manager_mods/ghidra_knowledge/tools/rea_ledger.py lint` for ledger hygiene. Named pseudo-C for a whole class: `python3 patch_manager_mods/ghidra_knowledge/tools/named_decomp.py run --class <Class>`
($0 tokens, one live REA session; output in the dump dir `named_decomp/`, outside git). Class doc page:
`python3 patch_manager_mods/ghidra_knowledge/tools/fn_doc.py page <Class>` (-> `steam_docs/<Class>.md`; also
`draft-input`, `check-drafts`). Validated purposes into the KDB:
`python3 patch_manager_mods/ghidra_knowledge/tools/persist_purposes.py <validated.tsv> [--apply]` (dry-run default,
never overwrites human purposes). All static facts for one address: `python3 patch_manager_mods/ghidra_knowledge/tools/static_facts.py addr <hex>`;
live-only facts in batch: `python3 patch_manager_mods/ghidra_knowledge/tools/rea_facts.py run --addrs FILE`; last resort, the `rea-recon` agent v2
(facts file first, 3 REA calls per address). Names that mod code or research notes already know:
`tools/ingest_names.py gap|extract|ledger|cards|score` (candidates -> `probable` ledger rows -> blind Haiku cards with decoys
-> `promote.tsv`), then `tools/promote_names.py --promote F --source ingest-<topic> [--apply]` writes verified names to
`name_map` (user OK first; INGEST_PLAN.md). The full tool order
(static dumps at $0, live REA last) is in `RE_AGENT_GUIDE.md` §2.1.

**No `sqlite3` CLI is installed on this machine** (confirmed 2026-09-29,
cost real time to discover) — the `sqlite3` shell command below will fail
with "command not found". Use `python3 -c "import sqlite3; ..."` or a
heredoc script instead, e.g.:

```bash
python3 -c "
import sqlite3
db = sqlite3.connect('patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db')
for row in db.execute('SELECT scope, topic, note FROM lessons ORDER BY date'):
    print(row)
"
```

If a real `sqlite3` CLI is ever installed, the queries below still apply
verbatim inside `sqlite3 patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db`.

Useful starting queries:

```sql
-- read the methodology lessons before starting new Ghidra work
SELECT scope, topic, note FROM lessons ORDER BY date;

-- has this class already been mapped?
SELECT * FROM classes WHERE class_name LIKE '%Map%';

-- has this class's vtable already been dumped/decoded (slot-by-slot)?
SELECT * FROM class_methods WHERE class_name = 'CSWGuiUpgrade' ORDER BY vtable_index;

-- has this function address already been looked at?
SELECT * FROM functions WHERE address = '00523870';
SELECT * FROM class_methods WHERE address = '00894320';

-- has this hook already been ported to Steam?
SELECT * FROM hooks_ported WHERE hook_function = 'preserveAreaMapAspect';

-- what's a function's known local variable layout?
SELECT * FROM stack_offsets WHERE function_address = '00894320';
```

## Maintenance

- Keep this file (schema doc) and the `.db` file both committed together.
- When a fact here turns out to be wrong (e.g. re-analysis on a patched
  binary contradicts an old row), update or delete the row — don't leave
  stale rows, since a wrong cached fact is worse than no cached fact.
- This DB is purely a research cache for US (the porting workflow). It is
  NOT read by `KPatchLauncher`/`KotorPatcher.dll` at runtime — that's a
  separate, unrelated SQLite DB (`AddressDatabases/*.db` in the
  `kotor-patch-manager` clone) that the framework itself consumes. Don't
  conflate the two.
