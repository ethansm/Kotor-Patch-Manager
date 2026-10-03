# Upstream KPM PR series: research and drafts (phase after PRs #291-#294)

Resume from `../steam_db_pr3/SESSION_CONTEXT_2026-10-03.md` ("Next phase").

| dir | task | status |
| --- | --- | --- |
| gameapi_signatures/ | R1: 22 wrapper_issue rows on K2 Steam / K2 GOG / K1 1.03 | done; lead gate PASS |
| ekc_steam/ | R2: ExpandedKeyboardControl Steam port spec | done; lead gate PASS |
| gog_only_triage/ | R3: PartyRosterLimit32K2, K2ClassMenuClothes, Movie Patch | done (triage only) |
| acc_parity/ | R4: AdditionalConsoleCommands parity, GameAPI init simulation | done; lead gate PASS |
| drafts/ | PR / issue bodies (nothing posted) | Phase 3 |

## Lead gate, Phase 1 (2026-10-03)
Re-verified on disk (Steam vanilla 6a522e71, scratch copy of the exe):
- R1: ShowUpgradeScreen `ret 0x18` (Steam 0x7CDA9A, GOG 0x4DE19A per server export); ActionMenuSecurity `ret 0x8`
  (Steam 0x842792, GOG 0x536012); ExitProgram thunk reads `[ecx+4]` at 0x73F29A.
- R2: all 6 hook original_bytes equal the exe at their addresses (draft TOML parsed and compared); hook 1 (0x72CB63
  `mov [ebp-0x144],eax`) and hook 6 (0x72E0BC, EDX = `[ebp-0x1c]` = object written at +0x3E4/+0x3E8) contexts read;
  RTTI decode: CSWGuiMainInterface vftable 0x99DCC4 slot0 = 0x74A750 (hook 5), Inventory 0x9A72AC, Journal 0x9A24DC,
  Messages 0x99E5EC, Fade 0x9A044C, Pause 0x9A7BAC.
- R4: every K2 DB (GOG, upstream Steam, branch Steam 4324098) has 0 rows in `classes` (K1 1.03: 977).
  `GameVersion::GetClassSize` throws with no row, so `CExoString::classSize` stays -1 and `CExoString(const char*)`
  leaves objectPtr null: `runscript` passes a null string on K2 GOG and Steam (regression since upstream 840f515).

## Decisions (lead)
- I1 scope = R1 "Commit 1" only (7 rows identical on all three games, no in-tree callers). The K2-only rows
  (StartLoadFromLayout, InitControl, ShowPartySelection, ShowUpgradeScreen, CRes/CResGFF/CSWGuiBorder Destructor) are
  used in-tree only by K1-only patches (ModOptions, Snake, K1GUITests, PlanetsLimit, PartyRosterLimit32K1), so they
  are latent on K2; offered as a follow-up in the PR body because they need a new title-gating pattern.
- runscript / `classes` fix: needs a user decision (GOG DB is read-only by rule). Options: classes rows in the Steam
  DB PR (CExoString 8) + issue for GOG, or a `malloc(8)` fallback in CExoString.cpp.
