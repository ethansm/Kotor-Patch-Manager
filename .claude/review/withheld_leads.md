# Leads withheld from Map workers (Reduce only)

Workers never saw these, so Map recall stays a blind signal. Use them to correct worker findings, not to
invent new ones. merge.py scores recall separately and adds any known issue the workers missed.

1. BUILD_GATE `py:x 0.1s` FAIL rows are test-fixture leakage, not build failures. `scripts/test_pre_install_gate.py:148`
   has a fake runner returning `(1, "FAIL   py:x  0.1s\n")`. The uncommitted `scripts/pre_install_gate.py` edit
   (+19) calls `master_ledger.log_event("BUILD_GATE", ...)` in `gate()`, so unit tests wrote 36 FAIL rows into the
   production `.claude/master_ledger.jsonl`. The untracked `scripts/master_ledger.py` later gained a "Test Isolation
   Guard" (~line 180) and 438 rows were archived to `.claude/master_ledger.test_archive.jsonl`. Classify as
   gate-integrity / ledger pollution. Any worker finding calling py:x a build failure or pointer-math issue must be
   downgraded or rejected. Real record: 7 genuine gate FAILs and 4 PASSes in the period; 11 of the 18 mods changed
   after the boundary have no recorded PASS.
2. `HotbarLogic.h` AH3 `isExile` (line ~89) is true for the whole `0xFFFFFFF0..0xFFFFFFFE` range OR
   `partyCount == 1 || isMember0 || partySlot == 0`. That is wide: a normal party leader in slot 0 may be keyed as
   exile. Boundary / mis-keying risk; needs boundary tests.
3. a. `npc_diversity_src/manifest.toml`: `kind` changed `"mod"` -> `"patch"` in `83354c0` "Fix gate failures".
   Review as a possible allowlist-style edit made to pass `kpm-validate-patches`, not as a fix of the cause.
   b. `tests/static/subclassers.toml` (dirty): `install_hint`/`restore_hint` line numbers bumped for 2 entries and a
   new `gui_input_src/GuiInputProvider.cpp` WindowHook entry. Allowlist edit: verify each against the source.
4. `hotbar_logic_test.cpp`: the `normalizeSaveFolder` assertions were removed (commit range, -31 lines). The
   function moved with the save-events migration; `se_registry_test.cpp` has ~22 `normalizeSaveFolder` references.
   Not a defect if every case (SAVES: prefix, case, `/` and `\\` separators, empty, too short, no name, non-digit)
   survived there. One dirty expectation `characterKeyFor(0xFFFFFFFE,1)=="objfffffffe"` was also removed.

## Investigation items not visible in code (add to the ledger as human items, do not re-verify)
- `NpcDiversity.kpatch` live md5 `c227395c` differs from the MOD_INSTALL ledger md5 `a6cd5c64`; the file was
  rewritten (2026-10-08 22:23Z) after the install record (21:15Z). The "47022c21 passed its gates" claim has no
  BUILD_GATE row; no gate row ever names `npc_diversity_src`.
- `tests/static/raw_addresses.toml` one-line change in `83354c0`: AGENTS.md requires a prior
  `query_ghidra.py address <hex>` lookup. Human-only (excluded file).
- The 14:09:31Z 10-09 `check-shared-headers` FAIL at HEAD has no recorded fix.
