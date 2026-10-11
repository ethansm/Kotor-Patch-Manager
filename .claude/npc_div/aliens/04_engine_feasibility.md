# 04 Engine feasibility: alien species rows via the SetAppearance hook

Recon, read-only. Game not touched. Labels: VERIFIED / INFERRED / UNSURE (A1 convention).
Sources: patch_manager_mods/34_npc_diversity_dispatcher.md; .claude/review/npc_div/A1_engine_hooks.md;
npc_diversity_src/{DiversityLogic.h, NpcDiversityDispatcher.cpp, NpcDivLayout.h, DiversityRows.h};
override/appearance.2da (read with repo pykotor, 812 rows, 94 cols); C2_telos_census.md; D2_2da_integrity.md.

## 1. Verdict
- Body swap through the same hook: FEASIBLE. Class dispatch and body builder read pDesc+0x18 after SetAppearance entry (A1 f7, f12, VERIFIED).
- Full species swap: NOT PROVEN. Voice, head, movement speed and footsteps are unresolved (section 2, 5).
- Use only existing vanilla F-type alien rows. No appearance.2da or heads.2da write.

## 2. Hook reach: what an entry-time row edit controls
| Field (pDesc unless noted) | Consumer | Status |
|---|---|---|
| +0x18 row | class cell (0x85A1F0 -> 0x85C150), 2DA floats, body builder (0x85CC00 or 0x85B940) | VERIFIED read after entry (A1 f7, f12). Entry edit is consumed. |
| +0x01 textureVar | GetBType "%s%02d" (class 3 only) | VERIFIED (A1 f6). Inert for F/S/L rows (A1 f8). |
| +0x00 subType | GetBType letter index (class 3 only) | VERIFIED (A1 f6). Inert for F rows. |
| +0x14 descMode14 | zero/nonzero EVIL column switch | VERIFIED (A1 f5). Not a class. |
| +0x1B gender | copied in 0x85CC00; not read by GetBType | Meaning UNSURE (A1 f6). |
| stats+0x80 head index (this+0x310) | head helper 0x85D430 when a4 == 0 | VERIFIED read (A1 f9, f12). Row normalhead as source: UNSURE. |
| stats+0x94 appearance | not read by SetAppearance | VERIFIED (A1 f12). |
| soundset / voice | no pDesc field; no write in the 8 callers | UNSURE. Likely UTC SoundSetFile at load (INFERRED, not in repo). |
| race | row columns only (`race`, `racialtype`=20 on all sampled alien rows) | INFERRED: no pDesc race field. |
| height / hitradius / creperspace / sizecategory | row columns | Alien F rows equal commoners (hitradius 0.25, creperspace 0.4, height 1, sizecat 3). Read site of the 0x85A1F0 floats: UNSURE. |
| walkdist / rundist / driveanim* | row columns | Read site UNSURE. Alien rows differ (section 3). |
| footsteptype / soundapptype | row columns | Read site UNSURE. |
| animations | embedded in MDL; bound in CreateBodyAppearance (doc 34 s1.2) | INFERRED: follows the model. |
| body clothing | body slot items (g_a_clothes06/010) | INFERRED: may define the visible body (C2 F5). Check in game. |

## 3. Row evidence (override/appearance.2da)
| row | label | modeltype | modela | normalhead | walk/run | drive walk/run | footstep type |
|---|---|---|---|---|---|---|---|
| 190 | Commoner_Mal_White | B | N_CommM | 13 | 1.813/3.66 | 1.813/3.66 | 0 |
| 185 | Commoner_Fem_White | B | N_CommF | 12 | 1.6/3.66 | 1.6/3.66 | 0 |
| 39 | Alien_Rodian_01 | F | N_Rodian | blank | 2/4 | 1.7/5.4 | 1 |
| 24 | Alien_Duros_01 | F | N_Duros | blank | 1.9/3.66 | 1.813/3.66 | 0 |
| 12 | Alien_Bith_01 | F | N_Bith | blank | 1.813/3.66 | 1.813/3.66 | 1 |
| 36 | Alien_Rakata_01 | F | N_Rakata | blank | 2.05/4.05 | 2.05/4.05 | 0 |
| 50 | Alien_Wookie_Female_01 | F | N_WookieF | blank | 1.7/5.4 | 1.7/5.4 | 0 |
| 51 | Alien_Wookie_Male_01 | F | N_WookieM | blank | 1.9/5.4 | not read | 0 |
| 72 | Alien_Ithorian_01 | S | blank (race C_Ithorian) | blank | 1/2 | 2/4 | 0 |
| 56 | Droid_Assassin | L | blank (race C_DrdAssassin) | blank | 1.75/2.33 | 2/4 | 2 |

- Rows 12, 24, 36, 39, 50, 51 are F-type with blank normalhead (VERIFIED, pykotor read). Head is in the full mesh or comes from stats (UNSURE which).
- Telos census already spawns rows 24 and 39 in vanilla UTCs (C2 census, VERIFIED): their MDLs are expected complete (INFERRED).
- S rows keep the model in `race`, modela/texa blank: 70/70 vanilla S rows (D2, VERIFIED). Excluded from the design.
- Walk/drive speeds differ per row: a commoner-to-Rodian swap changes gait scale unless the mover re-reads the row.

## 4. Proposed minimal design
Pool = existing vanilla F-type rows. No new 2DA rows, no heads.2da change (avoids the appearance.2da corruption class in CRASH_INVESTIGATION.md).
```
// DiversityLogic.h (new, ~12 lines)
constexpr uint16_t kAlienMale[]   = {39, 24, 12, 36, 51};  // Rodian, Duros, Bith, Rakata, Wookie M (F-type)
constexpr uint16_t kAlienFemale[] = {39, 24, 12, 36, 50};  // same set, Wookie F
constexpr uint32_t kAlienPct = 3;
// male commoner:   r = BandRoll(hash) (salt 1, existing); r in [15,18)  -> kAlienMale[Mix(hash,5) % 5]
// female commoner: r in [20,23) (after fat 0-14, Czerka 15-19)          -> kAlienFemale[Mix(hash,5) % 5]
// Salt 5 unused (salts 1-4 taken). PickTextureVar(alien row) == 0, so textureVar is not written.
```
| Scope (area -> species rows) | Rows | Pct |
|---|---|---|
| All generic male commoners (190-series) | 39, 24, 12, 36, 51 | 3 |
| All generic female commoners (185-series) | 39, 24, 12, 36, 50 | 3 |
| Nar Shaddaa / Telos only | same as above | 5. BLOCKED: area ResRef offset not decoded. Ship global 3 first. |

Tests: add band and pool cases to diversity_logic_test.cpp. Keep ShouldSkipDescriptor gating unchanged.

## 5. Engine-risk list (ranked)
1. Movement: walk/run drive speeds differ per row (Rodian 1.7/5.4 vs commoner 1.813/3.66). Foot-slide if not re-read. UNSURE.
2. Head: F rows have blank normalhead. Head index still comes from stats+0x80 (A1 f12). A human or llk head may sit on an alien body. C2 F2 claims row normalhead drives heads (not played). CONFLICT, OPEN.
3. Voice: soundset not touched by SetAppearance; human voice set likely stays. Footsteps follow footsteptype (UNSURE).
4. Client-only: server stats (m_nAppearanceType, doc 34 s1.3, not re-verified here) keep the commoner row. GetAppearanceType and server-side row columns stay vanilla. Save data unchanged (INFERRED).
5. Story NPCs: `ExcludeKind kind = ExcludeKind::None;` is hard-coded and tags are unread, so exclusion is inactive. Named NPCs on commoner rows can turn alien. BLOCKER for ship (C2 F1).
6. Animation set: animations live in the MDL. Check with `scripts/list_creature_anims.py` on N_Rodian, N_Duros, N_Bith, N_Rakata, N_WookieM/F before ship.
7. Non-humanoid rows excluded: S (72, 73, 77) and L droid rows (56 and others). Do not add them.
8. Class 2 and 0x0B (F/S/L) skip GetBType (A1 f8). Hook 1 never sees alien rows. No crash path, no clothing variety on aliens.
9. Gender gate: +0x1B meaning UNSURE (A1 f6). C2 F4 shows table gender mismatch. Gate on the commoner row, not on +0x1B.
10. GUI callers (0x880740, 0x89EFF0, 0x8F92B0, 0x84E6F0, 0x906570) also reach SetAppearance. Aliens may show there if those descriptors carry commoner rows (UNSURE). Existing exposure, larger now.

## 6. Probe before ship (log only)
- Per remap, log the final pDesc+0x18, stats+0x80, and whether an alien row was written. The RMP line already logs newApp (FormatRmpExt).
- Play check: existing Telos save, then `python3 scripts/kotor2_logs.py check npc-diversity_log.txt --new`. Look at head on body, gait, voice, footsteps.

## 7. Discrepancies found
- Brief says rows 802-815. Generated DiversityRows.h has APP_COUNT 812 (rows 802-811). Rows 812-815 were removed in Wave 4 (doc 34 s8, D1).
- Head source conflict: A1 f12 (stats+0x80) vs C2 F2 (row normalhead).
- ExcludeKind is always None: story exclusion is inactive (risk 5).

## 8. Open (needs REA probe or game log)
- Read sites: drive speeds, footsteps, 0x85A1F0 float columns.
- Whether 0x85D430 head path runs for F rows.
- +0x1B gender meaning.
- Area ResRef offset for an area-scoped table.
