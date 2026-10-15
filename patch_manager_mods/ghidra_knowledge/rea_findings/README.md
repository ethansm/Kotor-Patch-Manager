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
| `kind` | `function`, `global`, `vtable`, `string`, `field` (field: `addr` = owning function, `name` = `Class+0xOFF purpose`), `callsite` |
| `name` | the fact: a function/global name, or a short claim (`CSWGuiLabel scalar deleting dtor`) |
| `confidence` | `confirmed` (quoted anchor), `probable` (shape only), `refuted` (a name/claim proven wrong) |
| `evidence` | REA tool + quoted line, e.g. `procedure_assembly 0x72e042: MOV EAX,[ESI+0x4]` (<= 200 chars, no tabs) |
| `topic` | the brief's question id or doc, e.g. `WI-87`, `37_bug_board BG12` |
| `date` | `YYYY-MM-DD` |

Trust: ledger rows are agent observations, not validated address-DB rows. `query_ghidra.py` labels them
`[rea]`. Promotion into `name_map` / the Steam address DB still goes through the batch pipeline
(`steam_db_batches/DESIGN.md`) and a lead write.

## Agent definition
The live `rea-recon` definition is user-scope (`~/.claude/agents/rea-recon.md`); a tracked copy is in
`patch_manager_mods/ghidra_knowledge/agents/rea-recon.md`. Keep them equal when editing either.
