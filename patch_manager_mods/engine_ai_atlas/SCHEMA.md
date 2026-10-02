# Knowledge-DB schema additions for the AI atlas (lead-approved 2026-10-02)

## struct_fields (new; created only if missing by the persist scripts)
```sql
CREATE TABLE IF NOT EXISTS struct_fields(
  class TEXT NOT NULL,          -- 'CSWSCreature', 'CSWSCombatRound', 'CPathfindInformation', 'CAction' (0x74 node), ...
  offset INTEGER NOT NULL,      -- byte offset from the class base (store int; print as hex)
  size INTEGER,                 -- bytes (1/2/4/8/N); NULL if unknown
  type TEXT,                    -- 'int', 'float', 'objid', 'ptr CSWSCombatRound', 'CExoArrayList<objid>', ...
  name TEXT,                    -- proposed name
  confidence TEXT,              -- high (quoted asm reader/writer) / medium / low
  evidence TEXT,                -- one quoted line with address, or the findings file reference
  readers TEXT,                 -- space-separated hex instruction addresses (or fn addresses)
  writers TEXT,
  source TEXT,                  -- 'engine-ai-atlas AI<n>' / 'review_ai_engine' / ...
  date TEXT,
  PRIMARY KEY(class, offset)
);
```
Rules: an offset relative to a sub-object (combat round, stats, pathinfo) is stored under that sub-object's class, with a
row on the owner pointing at it (`CSWSCreature +0x10DC ptr CSWSCombatRound`). Persist scripts insert missing rows and
update a row only when the new confidence is higher or equal and the source is newer.

## name_map conventions (unchanged table)
- `steam_addr` is UNIQUE: add only missing rows; never overwrite a row from another source silently.
- Atlas rows use `source = 'engine-ai-atlas AI<n>'`; `high` only where the wave gate quoted the instruction.
- A demotion keeps the row, sets `confidence = 'refuted'` and prepends `DEMOTED 2026-10-02 (AI<n>): <reason>. ` to
  `evidence`.

## candidates
AI bug rows use `subsystem = 'ai'` and ids `AI-<short>` (e.g. `AI-FM2`).
