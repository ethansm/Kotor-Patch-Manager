# REA findings ledger

Every `rea-recon` run appends what it learned here, so the next agent finds it with
`python3 scripts/query_ghidra.py address <hex>` / `symbol <name>` instead of re-deriving it in Ghidra.

- One TSV per run: `<YYYYMMDD>_<topic-slug>.tsv` (the agent writes it with Write; never edit another run's file).
- Header (tab-separated, exactly these columns):

```
addr	kind	name	confidence	evidence	topic	date
```

| column | meaning |
|---|---|
| `addr` | `0x%08x` lowercase, Steam exe 6a522e71 (swkotor2.exe) unless `topic` names another binary |
| `kind` | `function`, `global`, `vtable`, `string`, `field` (field: `addr` = owning function, `name` = `Class+0xOFF purpose`), `callsite`, `xref` (name = `callers N: <first 5>; callees: <list or none>; size <n> B RET <n>`, evidence = tool + quoted asm line), `data` (name = what the data holds, evidence = `read_bytes 0xADDR: <hex>` or asm line), `table` (name = `<table kind> N entries`, evidence = first entry quoted) |
| `name` | the fact: a function/global name, or a short claim (`CSWGuiLabel scalar deleting dtor`) |
| `confidence` | `confirmed` (quoted anchor), `probable` (shape only), `refuted` (a name/claim proven wrong) |
| `evidence` | REA tool + quoted line, e.g. `procedure_assembly 0x72e042: MOV EAX,[ESI+0x4]` (<= 200 chars, no tabs) |
| `topic` | the brief's question id or doc, e.g. `WI-87`, `37_bug_board BG12` |
| `date` | `YYYY-MM-DD` |

Trust: ledger rows are agent observations, not validated address-DB rows. `query_ghidra.py` labels them
`[rea]`. Promotion into `name_map` / the Steam address DB still goes through the batch pipeline
(`steam_db_batches/DESIGN.md`) and a lead write.

## Conventions
1. To override an earlier row, write a new row whose evidence contains `supersedes: <file>:<line>`.
2. Scripted topics `<B>-s<k>` are written only by `rea_session.py`-based tools (a future tool, not yet built) via `rea_ledger.append_rows`, never by agents.
3. `python3 patch_manager_mods/ghidra_knowledge/tools/rea_ledger.py lint` checks:
   - `L1`: `kind` is in the kind list above.
   - `L2`: anchor kinds (`function`, `global`, `vtable`, `string`, `field`) quote `0x<6-8 hex>: <text>` in evidence.
   - `L4`: exactly 7 tab-separated cells, and the file starts with the exact header.
   - `L5`: `addr` is lowercase `0x` hex, `date` is `YYYY-MM-DD`, `confidence` is a known value.
   - `L6`: no `(addr, kind)` pair has both a `confirmed` and a `refuted` row, unless one carries `supersedes:`.
   - `L8`: a `function` row whose topic is `<id>-q<n>` has an `xref` row with the same addr in the same file.

## Agent definition
The live `rea-recon` definition is user-scope (`~/.claude/agents/rea-recon.md`); a tracked copy is in
`patch_manager_mods/ghidra_knowledge/agents/rea-recon.md`. Keep them equal when editing either.
