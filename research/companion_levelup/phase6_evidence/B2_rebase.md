# B2 early-conversion re-base check (Phase 6)

Script `B2_rebase_check.py` (run with `../../.venv/bin/python` for the 2da advisories; plain `python3` skips them), output `B2_rebase_results.txt`.
Oracle = the real C++ `levelNow` (tools/parity parity_bin), driven level by level to character level 30; Python twin compared at every step.

Coverage: 34 presets with hold_class != 0; conversion after L base-class levels, L = utc start level (first record clvl - 1) .. convert_at-1; hold on and off;
384 scenarios, 7616 C++ calls: PATH 1354, REBASE 4240, VANILLA 2022 (all VANILLA = Jedi class level beyond the preset's last Jedi record), HOLD 0.
Presets whose path starts in the Jedi class (disciple/handmaiden early, mira early) have a single plausible conversion level (convert_at-1 = on-time).
parity.py gained `ln_early_cases` (6944 explicit cases, 0 mismatches).

Result: (1) every early conversion continues on Jedi records (REBASE/PATH), never HOLD, never VANILLA while a record exists; (3) no HOLD with the hold class;
(2) every Jedi record (feats, powers) is still applied. Lost items are only feats of base-class records that sit between conversion and convert_at
(inherent: one class per level slot). No power is lost anywhere.

Advisory (2da snapshot, not the Steam install): Jedi picks arrive at an earlier character level than designed, so some powers/feats fall below their min character
level (235 power rows, 15 feat rows) and 94 feat rows lose a prerequisite that lived in a skipped base record. See results file for per-preset detail.
