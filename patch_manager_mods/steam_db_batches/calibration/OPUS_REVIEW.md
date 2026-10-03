# Opus calibration gate: Steam address-DB batch pipeline (exe 6a522e71)

Date 2026-10-03. Inputs: DESIGN.md s1 + s4.2, CALIBRATION_draft.md, RECALL_STUDY.md, review_scores.md (chunk 1),
chunk 2 scored here (`score_review.py key_2.tsv sonnet_r2.tsv`), GOG_LABEL_SUSPECTS.md, REVIEWER_BRIEF.md,
RE_AGENT_GUIDE.md s3, `tools/batch.py` + `tools/bdlib.py` (gate code read). On-disk sources: Steam dump
`steam_6a522e71/decomp`, server `fp.tsv` for GOG, Steam **and Mac x86** (`k2_mac_KOTOR2_x86/fp.tsv`, which has real
Aspyr symbol names: an oracle for meaning that is independent of the GOG DB labels).

## Verdict: SIGN-OFF WITH CHANGES

Both AUTO routes are sound as **body matchers**. 0 of 1167 decoys/negatives and 0 of 1759 shift/cluster rows reached
AUTO, and my 8-row meaning spot-check found 8/8 correct. Three failure modes are not covered by the calibration:
1. **GOG label errors pass straight through AUTO.** The gates prove GOG body == Steam body, then ship the GOG name.
   GOG labels are wrong in whole runs: CGuiInGame has at least 8 mislabelled functions, rotated by one slot, and
   CSWCObject has a swapped pair (section 5). The calibration cannot see this: gold pairs come from the same GOG names,
   so wherever gold agrees with the GOG label, the error is invisible.
2. **Wrong trusted pairs seed `run_ok` and the callee anchors.** 3 of the 451 current `corpus.pairs` entries pair the
   wrong bodies (sizes 68/129, 90/112, 679/395). One of them actively blocks a correct row through G6 (section 4c).
3. **Positional twins on the sig route.** The decoy margin looks only at the previous and next start. 8 of the 164
   sig-route positives have a same-size, same-retn twin within +-8 starts whose SIG is within the margin. Examples:
   ReadField*/WriteField*, AddMoveToPointAction vs ToFront (identical fingerprint sets, 339 B, 3 starts apart), and the
   texture-cache family. Those 8 are AUTO on position alone.

Also: when a reviewer `mismatch` gives a `corrected_addr`, the row becomes `likely` at that address with no re-gate and
no Opus check (`decide_review`). That path would have persisted CAL-2-45's GOG-label error, AnimationPause -> 0x7ef730.

## Required changes (exact rule text)

**R1 Seed hygiene (DESIGN s1 and s4.2 `run_ok`, bdlib `apply_untrust`).**
> A trusted row enters `pairs` (run_ok seed, A-CALLEE anchor, gold positive) only if its GOG name equals the trusted
> Steam name exactly (no `base_name` suffix stripping) AND the GOG and Steam fingerprints agree on body: equal `retn`, and
> either equal `size` or A-SIG >= 0.5 with intersection >= 3. A trusted row that fails is listed in `gate_report.md`
> under `## Trusted pair body conflicts` and is not a seed. It stays trusted as a G6 holder until Opus rules on it.

Current effect: drops 4 entries. 0x474890->0x7ef350 and 0x4df1f0->0x7cf710 have correct Steam names on the wrong GOG
bodies. 0x53cf90->0x8827a0 is a wrong Steam row. 0x606e70->0x734f60 pairs CompareNoCase(char*) with another overload.
It keeps 0x5cfd00->0x850770 (genuine layout growth, SIG 0.57), 0x4db020 and 0x858000 (SIG 1.0).

**R2 Sig-route twin window (s4.2 `auto_route=sig`, replace the decoy clause).**
> ... the best decoy's SIG <= a_sig - 0.25, where the decoys are the previous and next Steam starts, every other
> candidate of the row, AND every Steam start within +-8 starts of the candidate with the same `size` and `retn`.

Calibrated cost: 8 of 164 sig positives move to REVIEW (all 8 listed above). There is no precision cost.

**R3 Mac-name contradiction check, new invariant G7 (both routes).**
> G7 `mac`: let V(f) = the set of `disp` and `imm` values v with 0x10 <= v < 0x100000 in fp.tsv. For every Mac x86
> function with exactly the GOG `Class::Name`, overlap = |{x in V(S): exists y in V(M), |x-y| <= 4}| / |V(S)|. If |V(S)| >= 2,
> |V(M)| >= 2 and overlap == 0 for every such Mac function, flag `mac_contra` -> REVIEW (never REJECT). No Mac function
> of that name, or too few values -> pass (recorded `g7=n/a`).

Calibrated on the 253 AUTO positives: 196 have overlap >= 0.5, 2 are between 0 and 0.5 (ShowUpgradeScreen 0.42,
LoadVisibility 0.33), 0 have overlap 0, and 55 are n/a. So the check costs 0 recall. It flags 4 of 4 known GOG-label
pairs at 0.0:
- GetMiniMapVisible -> 0x7d0560;
- IsAnimationPlayingInDialog -> 0x7cb6e0;
- GetCanOrientInDialog -> 0x7ceaf0;
- AnimationPause -> 0x7ef730.

Each correct counterpart scores 0.5-1.0.

**R4 Name uniqueness and demotions (s4.2 G6, s4.5).**
> G6 also fails `review:name_held_elsewhere` when a trusted row holds the GOG name at a different Steam address (AUTO
> blocked). `persist` never inserts a second address for a name that a trusted row already holds. It never demotes a
> trusted row to `wrong` unless an `overrides.tsv` row from Opus names that address.

**R5 Reviewer corrections (s4.4 `decide_review`).**
> A reviewer `mismatch` with `corrected_addr` gives `unresolved` plus a `recheck` entry. The corrected address is added
> as a candidate and re-gated in the next `gates` run, and is shipped only via that gate or an Opus override. It never
> becomes `likely` by itself.

**R6 Reviewer brief (REVIEWER_BRIEF.md, Offset shift few-shot).** Replace "Struct layouts are identical between GOG and
Steam (all 21 shared DB offsets agree), so every GOG displacement..." with:
> Struct layouts are almost always identical (all 21 shared DB offsets agree), but Aspyr enlarged some GUI panel
> classes. Example: CSWGuiSaveLoad is allocated `PUSH 0x1e44` on Steam (`0088284d`) vs `0x1b74` on GOG, so its last
> field moved from +0x1b70 to +0x1e40 (`00851296 MOV [EAX+0x1e40],ECX` in the ctor). A shifted offset is a layout
> difference ONLY if it is a tail field of a class whose ctor/allocation size differs by the same amount; quote both
> lines. Any other unexplained difference, including a small uniform shift like -4, means a different function: `mismatch`.

Add under Verdicts:
> If you give `corrected_addr`, answer question 2 for the corrected function too. If its body contradicts the GOG
> name, answer `label_suspect` with `corrected_addr` filled.

**R7 Re-run calibration before the first real batch.** Apply the gold corrections below plus R1-R4, re-run
`calibrate.py` and `recall_study.py`, and confirm that 0 decoys/negatives/stress rows reach AUTO. No new reviewer chunk
is needed.

## Other rule changes made during calibration (item 2)

| change | ruling | why |
|---|---|---|
| A-VT informational only | **approved** | It only tightens: `class_methods` labels come from the same labelling, so VT is circular evidence. |
| G1 library-name -> `warn:autolabel` (REVIEW) except `_*`/`Catch_All@`/`Unwind@` and CRT_ADDRS | **approved** | `warn:autolabel` is not `pass`. Both routes require `g1 == pass` (`sig_route_ok`; `classify` adds `why:g1`), so it can never yield AUTO. It only turns false REJECTs into REVIEW. |
| `B1_ONLY_SOURCES = {steam-db-pr3-phase2}` | **approved, and confirmed needed** | The branch DB holds Option A rows that B1 rejected: GetMiniMapVisible 0x7d05a0, RemoveGUIPopUps 0x7d0560, IsAnimationPlayingInDialog 0x7cb6e0 (B1 verdict `rejected`). The rule keeps them out of the trust index (checked: `trusted_names()` is empty for all three). |
| `label_suspect` rows never shipped | **approved**, with R5 | Also: when the Steam counterpart of the correct name is proven by meaning (Mac symbol + Steam anchor), the Steam row ships under that name with an Opus override. Example: AnimationPause = 0x7ef350. |

## Reviewer tier and Opus sample rate (item 3)

Match precision was 39/40 on chunk 1 (pre-brief) and 39/39 on chunk 2, 78/79 combined. Chunk 2's 97.5% recall
"miss" (CAL-2-45) is a gold error: the reviewer was right about the body (section 4b). The sample is too small to
demonstrate 99%. The Wilson 95% lower bound is 91% for 39/39 (the only post-brief data) and 93% for 78/79. Label
detection is untested after the brief: the one post-brief label case (CAL-2-45) came back `mismatch` + corrected
instead of `label_suspect`.

**Ruling: the reviewer tier is acceptable as a filter, not as a final gate. Reviewer `match` rows still pass an Opus
sample:**
- **100% Opus** for these rows:
  - every reviewer `corrected_addr`;
  - every `label_suspect`;
  - every `match` that cites a layout/offset difference;
  - every `match` on a row flagged `mac_contra`, twin (R2) or `name_held_elsewhere`.
- **20% random sample (minimum 5) of the remaining reviewer `match` rows per batch**, C0 rows first.
- Every chunk carries at least 10 decoy cards with 0 tolerance. Any decoy accepted, or any Opus-found error in the
  sample, sends 100% of that batch's matches to Opus.
- After 3 consecutive batches with 0 errors in the sample and 0 decoy accepts, the rate may drop to 10%.
- AUTO rows also get a per-batch Opus meaning check of 5 sig + 3 c2, as done below.

## Gold disputes (item 4)

**(a) CAL-1-34 GOG 0x4df1f0 `CGuiInGame::GetCanOrientInDialog`. Verdict: GOG label suspect. Gold Steam name right, gold
pair wrong.**
- GOG fp 0x4df1f0 (90 B, RET 4, disp `0x9c`, imm `0x440`) equals Steam 0x7ceaf0 exactly. The reviewer was right on the
  body.
- 0x7ceaf0 copies a Vector3: `007ceb22 MOV EDX,[ECX+0x9c]` / `007ceb28 ADD EDX,0x440`.
- The real GetCanOrientInDialog is Steam 0x7cf710:
  - `007cf719 CMP dword ptr [EBP+0x8],0x7f000000`;
  - `007cf725 ADD ECX,0xf24`, then a list walk.
  - This matches Mac `CGuiInGame::GetCanOrientInDialog` 0x26161c (GetNext; disp 0xf24; imm 0x7f000000).
  - Its twin 0x7cf8f0 (`007cf905 ADD ECX,0xf28`) matches Mac GetCanHeadFollowInDialog.
- Its GOG body is 0x4dfe10 (112 B, imm 0xf24 x3, 0x7f000000). The GOG DB labels it `IsOverlayDialogAnimation`, a
  duplicate of the correct label at 0x4db950.
- **Steam row:** `CGuiInGame::GetCanOrientInDialog = 0x7cf710`, thiscall, param 4. The GOG reference should be
  0x4dfe10, not 0x4df1f0.

**(b) CAL-2-45 GOG 0x474890 `CSWCObject::AnimationPause`. Verdict: GOG label suspect, a swapped pair with 0x4744b0.
Gold Steam name right, gold pair wrong.**
- GOG fp 0x474890 (68 B, imm `0x4b0,0x52f,0x640,0x6bf`) equals Steam 0x7ef730, a pure id-range test. The reviewer was
  right on the body.
- That range test matches Mac `AnimationPlayOutOfPlace` (51 B, imm `0xfffff9c0`, `0x80`).
- AnimationPause is Steam 0x7ef350:
  - `006eba0a PUSH 0x99af6c` ("Pause", strings.tsv), then `006eba3f MOV [0x00a0fe2c],EAX` stores the column handle;
  - 0x7ef350 passes `DAT_00a0fe2c` to the 2DA int lookup FUN_0071b8b0;
  - this matches Mac `AnimationPause` (C2DA::GetINTEntry; disp 0x68,0xcc,0x130).
- Its GOG body is 0x4744b0 (129 B, the same fingerprint as 0x7ef350; run delta 0x37aea0, the same as
  0x4748e0->0x7ef780). The GOG DB labels it `AnimationPlayOutOfPlace`.
- **Steam rows:** `AnimationPause = 0x7ef350` (keep, GOG ref 0x4744b0) and `AnimationPlayOutOfPlace = 0x7ef730`
  (GOG ref 0x474890; Batch 1 `likely` -> verified by the Mac shape).

**(c) GOG 0x53cf90 `CSWGuiMainMenu::OnLoadGamePicked`. Verdict: gold wrong.**
- The GOG fp row: 679 B, **retn 0**, callees 0x53d6a0 (PopulateModules), ...; imm `0xfffffffd` x11.
- This equals Steam 0x8824f0 in every field, and its caller structure matches: GOG `OnWarpButtonPressed` 0x53e810
  calls 0x53cf90, and Steam 0x883d70 calls 0x8824f0. It also matches Mac `OnLoadGamePicked` (calls PopulateModules,
  ListBox Set/GetControl, imm 0xfd x10).
- Gold 0x8827a0 (395 B, RET 4: `0088284d PUSH 0x1e44`, `00882880 CALL 0x00850770` SaveLoad ctor) is the counterpart
  of GOG 0x53d240 `CSWGuiMainMenu::OnLoadSaveGame`: 395 B, RET 4, calls ctor 0x5cfd00, `new(0x1b74)`. Mac
  `OnLoadSaveGame` also calls the CSWGuiSaveLoad ctor.
- Batch 1 identified the Load button handler correctly but gave it the wrong symbol.
- **Steam rows:** `OnLoadGamePicked = 0x8824f0` (param 0) and `OnLoadSaveGame = 0x8827a0` (thiscall, param 4).
  Branch row id 460 is wrong. The positive-REJECT in the calibration and the shift_m1 "stress hit" at 0x8824f0 are
  this gold error, not gate failures.

**(d) CAL-2-56 GOG 0x5d1bf0 `CSWGuiSaveLoad::VerifyLoadGame` -> 0x8528b0. Verdict: gold right, reviewer `match` right,
and this is a genuine Aspyr layout difference.**
- The fingerprints differ only in disp `0x1b70` vs `0x1e40` (`008528f8 CMP dword ptr [ECX+0x1e40],0x0`). Callees,
  `PUSH 0x7d9b` and the other 6 disp / 4 imm values are identical.
- CSWGuiSaveLoad grew by 0x2d0 on Steam:
  - allocation `0x1e44` vs `0x1b74`;
  - the ctor's tail fields are `0x1e2c,0x1e3c,0x1e40` vs `0x1b5c,0x1b6c,0x1b70`.
- The brief's "layouts identical" claim therefore needs the R6 qualifier.
- **Steam row:** keep 0x8528b0.

## Gold corrections list

| gog_addr | name (GOG DB) | old steam | new steam / label_suspect | reason |
|---|---|---|---|---|
| 0x4df1f0 | CGuiInGame::GetCanOrientInDialog | 0x7cf710 | label_suspect (body = 0x7ceaf0) | 4a. GetCanOrientInDialog stays 0x7cf710 by meaning, GOG body 0x4dfe10 |
| 0x4dfe10 | CGuiInGame::IsOverlayDialogAnimation | - | label_suspect (body = 0x7cf710 = GetCanOrientInDialog) | 4a. Duplicate GOG label (also at 0x4db950) |
| 0x474890 | CSWCObject::AnimationPause | 0x7ef350 | label_suspect (body = 0x7ef730 = AnimationPlayOutOfPlace) | 4b |
| 0x4744b0 | CSWCObject::AnimationPlayOutOfPlace | 0x7ef730 (B1 likely) | label_suspect (body = 0x7ef350 = AnimationPause) | 4b, swapped pair |
| 0x53cf90 | CSWGuiMainMenu::OnLoadGamePicked | 0x8827a0 | **0x8824f0** | 4c. Identical fingerprint, retn 0, Warp caller |
| 0x53d240 | CSWGuiMainMenu::OnLoadSaveGame | - | **0x8827a0** | 4c. Identical fingerprint, SaveLoad ctor |
| 0x5d1bf0 | CSWGuiSaveLoad::VerifyLoadGame | 0x8528b0 | 0x8528b0 (no change) | 4d. Layout growth |
| 0x4e0c60 | CGuiInGame::GetMiniMapVisible | (branch 0x7d05a0) | label_suspect (body = 0x7d0560 = ShowDialogMessage) | section 5 |
| 0x4e0ca0 | CGuiInGame::RemoveGUIPopUps | (branch 0x7d0560) | label_suspect (body = 0x7d05a0 = GetMiniMapVisible) | section 5 |
| 0x4e0d20 | CGuiInGame::ResetInterfaceFont | (name_map 0x7d0620, mapper medium) | label_suspect (body = 0x7d0620 = RemoveGUIPopUps) | section 5 |
| 0x4dbde0 | CGuiInGame::IsAnimationPlayingInDialog | (branch 0x7cb6e0) | label_suspect (body = 0x7cb6e0 = DialogGuiDoneScroll) | section 5 |
| 0x4db4b0 | CGuiInGame::DestroyDeathMenuGUI | (name_map 0x7cadb0, mapper medium) | label_suspect (body = 0x7cadb0 = IsAnimationPlayingInDialog) | section 5 |

Branch-DB (B0) consequences, for the lead after the B0 review:
- id 460 OnLoadGamePicked: 0x8827a0 -> 0x8824f0, param 0. Add OnLoadSaveGame 0x8827a0, param 4.
- id 465 RemoveGUIPopUps: 0x7d0560 is wrong -> 0x7d0620 (Mac shape: disp 0x38/0x40/0x54/0x94/0xa0/0xa4/0xb0, one
  callee). Mark it likely until anchored.
- id 464 IsAnimationPlayingInDialog: 0x7cb6e0 is wrong -> 0x7cadb0 (`007cae2e CALL 0x007ef780` AnimationDialog; Mac
  shape disp 0x1b8/0x1c0). Likely.
- ids 109 / 229 / 463 / 455 stay (their GOG references in `notes` are wrong for 109, 229 and 463).

## GOG_LABEL_SUSPECTS rows 1-4 (item 5)

| # | GOG | ruling | evidence |
|---|---|---|---|
| 1 | 0x4E0C60 `GetMiniMapVisible` | **CONFIRMED wrong. Refuted: the name it fits is `ShowDialogMessage`, not RemoveGUIPopUps** | Steam twin 0x7d0560: `007d0570 CMP EDX,[ECX+0x40]` / `007d0573 JNZ 0x007d058d`, so it calls vt+0x84 when the fields are **equal** (the row text says `!=`; fix it). Mac `ShowDialogMessage` 0x268b38: 33 B, disp 0x3c,0x40,0x88 (Mac vtable slots are +4: `DialogGuiDoneScroll` has the same 0x84->0x88 offset). Mac order SetCombatMessage, ShowDialogMessage, GetMiniMapVisible, RemoveGUIPopUps vs GOG labels ShowDialogMessage, SetCombatMessage, GetMiniMapVisible, RemoveGUIPopUps, ResetInterfaceFont: a **rotation**, not a pair swap. GOG 0x4e0bf0 (disp 0x98,0xa900) fits Mac `GetInterfaceOffset` (0x98,0xa770). |
| 2 | 0x4E0CA0 `RemoveGUIPopUps` | **CONFIRMED: it is GetMiniMapVisible** | Mac GetMiniMapVisible calls `GetClientOptions` + `GetMiniGame`, disp 0x98,0x58e4. GOG calls 0x40ccf0/0x40d580, disp 0x98,0x59b4. Steam `007d0602 MOV EAX,[EDX+0x59b4]`, bit 1. The base is the object from the 0x7400a0 path, not `this` (minor wording fix). The real RemoveGUIPopUps is GOG 0x4e0d20 = Steam 0x7d0620. |
| 3 | 0x4DBDE0 `IsAnimationPlayingInDialog` | **CONFIRMED wrong. Name found: `DialogGuiDoneScroll`** | Steam 0x7cb6e0: `007cb6f3 MOV dword ptr [ECX+0xfb4],0x1`, then `007cb70b MOV EAX,[EDX+0x84]` / `CALL EAX` on [+0x3c]. Mac `DialogGuiDoneScroll` 0x26563c: 44 B, disp 0x3c,0x64,0x88,0xfb4. The real IsAnimationPlayingInDialog is GOG 0x4db4b0 (labelled DestroyDeathMenuGUI) = Steam 0x7cadb0. |
| 4 | 0x4DF1F0 `GetCanOrientInDialog` | **CONFIRMED wrong** | See 4a. The body is a Vector3 getter at [[this+0x9c]+0x440..0x448]: Steam 0x7ceaf0, which Batch 1 called GetAreaTransitionPosition (that name has no Mac counterpart; treat it as unconfirmed). The real GetCanOrientInDialog is GOG 0x4dfe10 / Steam 0x7cf710. |

New suspects to add to the file:
- 0x4dfe10 (duplicate IsOverlayDialogAnimation);
- 0x474890 / 0x4744b0 (swapped AnimationPause / AnimationPlayOutOfPlace);
- 0x4e0bf0 / 0x4e0d20 / 0x4db4b0 (the rotations above);
- likely 0x4dbe50 `IsFadingInDialog`: disp 0x3c,0x40 fits Mac `IsCameraDialog`. Unverified.

The impact line should read: the KPM GOG `GetMiniMapVisible` wrapper calls `ShowDialogMessage`.

## Spot-check (item 1): random AUTO positives, `random.Random(20261003)` over novote_A/B_pos

| route | GOG -> Steam | name | on-disk anchor (Steam asm) | independent meaning check | verdict |
|---|---|---|---|---|---|
| sig | 0x4d9c50 -> 0x7c9560 | CGuiInGame::ResetTalkAnimations | `007c956c MOV ECX,[EAX+0x198]`, `007c9604 CMP ECX,0x2710`. fp equals GOG (750 B, 0x2710 x8) | Mac: 3 callees GetGameObject / GetCurrentAnimation / CanPlayDialogAnimations; Steam has 3 callees, disp 0x198,0x19c | CONFIRMED |
| sig | 0x90e320 -> 0x40fea0 | CSWGuiPanel::GetControl | `0040feac MOV ECX,[EAX+0x28]` (bound), `0040febd MOV ECX,[EAX+0x24]` (array) | Mac GetControl disp 0x24,0x28 | CONFIRMED |
| sig | 0x40e3e0 -> 0x740f20 | CClientExoApp::GetPazaakIsRunning | `00740f2f MOV EDX,[ECX+0x70]` via [this+4] | Mac disp 0x4,0x70 | CONFIRMED |
| sig | 0x40e340 -> 0x740e80 | CClientExoApp::GetTargetChanging | `00740e8f FLD float ptr [ECX+0x378]` vs a constant | Mac disp 0x4,0x378 | CONFIRMED |
| sig | 0x626310 -> 0x71a1b0 | CResGFF::WriteFieldVOID | `0071a1e0 PUSH 0xd` (GFF type 13 = VOID), `0071a1db CALL 0x00717aa0` | Mac WriteFieldVOID imm 0xd, disp 0x5c,0x98 | CONFIRMED (a WriteField* twin family; R2 would not have flagged this one) |
| c2 | 0x62bb40 -> 0x721bb0 | CResGFF::WriteResource_2 | `00721ca1 CALL 0x0071a8b0` (Pack). Size 685 and RET 0x14 equal | overload suffix `_2` = GOG DB name | CONFIRMED |
| c2 | 0x90e0d0 -> 0x40fc50 | CSWGuiPanel::GetFullScreenBG | `0040fcc9 CMP [0x009f42a8],0x3c0` / `0040fcd5 PUSH 0x9874b4` ('1280x960'), `0040fcf0 PUSH 0x9874a8` ('1024x768') | Mac GetFullScreenBG builds a CExoString with operator+ | CONFIRMED |
| c2 | 0x653640 -> 0x6d7390 | CSWSCreature::AIActionPhysicalAttacks | `006d935d PUSH 0x99a92c` ('g_turretbl002', 1 ref), `006d79de CALL 0x005716a0` (DesiredAttackRange) | Mac name present, 6.5 KB body | CONFIRMED |

Near-miss negatives: every decoy, negative or stress row with exactly one discriminating family was REVIEW or REJECT.
Not one SIG-only near-miss had `run_ok=yes`. Some had equal size: A_decoy_next 1, A_decoy_prev 2, B_decoy_next 1, and
several shift rows. So `run_ok` is the load-bearing gate of the sig route, and that is why R1 (seed hygiene) is
required.

Two RECALL_STUDY R1*/R3* hits:
- CServerExoApp::SetLoadFromSaveGame vs decoy 0x51cd60 is a real low-entropy twin. Both are 36 B, disp {0x4},
  imm {0x4,0x8}. The distinguishing `[[this+4]+0x100c4]` is cut by the export's `disp <= 0x10000` filter, and only
  the adjacency margin caught it. R2 generalises that protection.
- OnLoadGamePicked shift_m1 is the gold error from 4c.

Other observation: 107 of the 164 sig positives share no value that occurs in 50 or fewer Steam fingerprints. The sig
route is mostly "position + size + common shape", which is acceptable only with clean seeds (R1) and the twin window
(R2).
