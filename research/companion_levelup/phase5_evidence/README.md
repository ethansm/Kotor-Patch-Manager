# Phase 5 evidence (level-up override 0.3.0: UX + save binding + R11)

Plan: `../PHASE5_PLAN.md`. Doc: `patch_manager_mods/08_level_up_override.md` ("Phase 5 UX research", "Phase 5 U facts", ...).

Static Ghidra output only (via `../tools/ghidra_run.sh`, one headless run at a time). File prefixes:
- `U1_*` ctor-time control creation site in CSWGuiInGameCharacter ctor 0x84C3A0 (+ fallbacks).
- `U2_*` R11 banked message: FeedbackMessage path / custom-string-to-player API.
- `U3_*` save-binding leftovers (0x656640 entry, 0x656758 capture, 0x401C52, autosave restore).
- `U4_*` class display names + XP table ("levels banked").
