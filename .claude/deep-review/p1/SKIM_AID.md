# Skim aid, p1 (digest of verified_findings.md, refutation_audit.md, PILOT_REPORT.md)

Facts copied from the three files. Tiers: 6 CONFIRMED, 2 REFUTED. Format: file:lines | claim | verdict | quote-checked | what to eyeball.

## CONFIRMED (6)

1. patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:592-601 | animLen returns true with a 2.0 s fallback for rows 3/538/539, so the "not in table" branch in playFlourish is unreachable | CONFIRMED (low, downgraded to medium in reduce) | yes | Whether a flourish on those rows plays a 2.0 s clip with no animation; check the pre-change animLen.
2. patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:709-713 | Comment at 709 describes a recovery path that exists only on the same-character path; destroyCreature runs on character change | CONFIRMED (low) | yes | Misleading comment and tick re-attach on a null creature; not a crash per the finding.
3. patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1412-1414 | `changed` is always false on the non-force path, so the "PLAYTHROUGH loaded/missing" log at 1429 never fires | CONFIRMED (low) | yes | Diagnostics only; log absence on first load and polled reload.
4. patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1434-1435 | Planned conversion loses MOVEFILE_WRITE_THROUGH on rename (atomicWrite has none) | CONFIRMED (low, latent) | yes | Claim wording is slightly overstated; the real risk is rename durability across a crash, not data flush.
5. patch_manager_mods/level_up_override_src/common_contract_test.cpp:163-165 | Address-vs-nullptr checks on non-weak functions are always true at runtime | CONFIRMED (low) | yes | Misleading naming; the cast still gives a compile-time signature check.
6. patch_manager_mods/level_up_override_src/common_contract_test.cpp:196-199 | Throttle check on unchanged file would pass even with throttle or force removed | CONFIRMED (low, medium in reduce) | yes | Whether this is the only FileWatch check for level_up_override.

## REFUTED (2)

7. patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:2043-2048 | party::table() raw walk matches the engine getter body *(*(A+4)+0x270) | REFUTED (Haiku high; Sonnet refute-audit: refute-holds) | yes | Only residual risk is the getter body differing on the GOG build.
8. research/companion_levelup/tools/parity/unit_gap_test.cpp:130 | Resetting g_tablePoll clears primed and checked, so the stale-stamp concern does not apply | REFUTED (Haiku high) | yes | Line cite is slightly off; CARRY checks are at 136-137.

## Notes

- PILOT_REPORT.md: decoys 8/8 caught; no answer key, so recall is not measured by controls.
- Human skim of findings is pending (user).
