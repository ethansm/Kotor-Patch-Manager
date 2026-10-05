# Force Power Hotbar / Action Hotbar evidence

Evidence for [`../07_force_power_hotbar.md`](../07_force_power_hotbar.md).

Names keep 'force_power_hotbar' (user decision 2026-10-04, rename to Action Hotbar applies to code/ids only).

- `action_bar_refresh/` -- research on resyncing the vanilla action bar (0x74DA40) on stance/form edges (`FINDINGS.md`).
- `target_modes/` -- target-mode research: Ghidra decomps/disasm of the use/invoke path, caller lists, `FINDINGS.md`, `EDGE_CASES.md`, `TARGETABLE.md`, spells targeting TSV, badge preview PNG.
- `catalog_exile_named.txt` -- early probe catalog of the Exile's action-bar entries (items by name, group, count).
- `key_gate_events_build1b.txt` -- build 1b key-gate log: HUD gate ACTIVE/inactive and per-key pass/swallow decisions.
- `probe_dump_exile_build2.txt` -- build 2 probe dump of the Exile's action-bar groups with raw entry words.
- `POLISH_REVIEW_2026-10-04.md` -- three-pass polish review feeding doc 24 phases HP1-HP7.
- `FEEDBACK_2026-09-30.md` -- 2026-09-30 code review of the first build, with 2026-10-05 resolution entries.
