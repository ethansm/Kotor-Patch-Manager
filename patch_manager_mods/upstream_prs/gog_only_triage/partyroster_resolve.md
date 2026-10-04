# PartyRosterLimit32K2 -> Steam: R-C resolution (2026-10-04)

Read-only RE. Outputs: this file and `partyroster_resolve.tsv` (257 rows: 211 hooks H000-H210 in toml order, 43 functions `FN:`, 3 data `DATA:`).
Method: offline objdump of GOG (server memblocks) and the vanilla Steam exe (sha 6A522E71, scratch copy); function-level body diff with address operands masked; exact `.text` byte searches; RTTI vtable checks. No Ghidra findBytes, no DB or game-dir writes.

## 1. Verdict: GO (conditional on 2 tasks outside the hooks)

The Steam exe contains the same code as GOG for every function the patch touches. All 56 functions that contain hooks are the same size and
instruction-for-instruction identical to GOG (address operands masked; 0 diffs), so all 211 hook sites, all `[ebp+-x]` frame slots read by the 43 detour
exports, and all struct offsets carry over unchanged. All 43 hard-coded function addresses and the 3 data addresses now have anchored Steam equivalents (section 3). Findings:

- 211/211 hooks: same instruction, same byte offset, same 7-instruction context on Steam (verdict SAME for all; 0 DIFFERENT, 0 UNSURE).
- 204/211 original_bytes are byte-identical on Steam. The 7 that differ only in address operands (SEH-handler imm32, one rel32 call, one push of a string address); Steam bytes are in the tsv (cols `steam_orig_bytes`).
- 5 detours carry `consumed_exit_address`; Steam values: H184 0x8AB2EB, H185 0x8AB0FA, H192 0x5FA3AA, H203 0x5FAD4B, H204 0x5FB164 (section 2).
- Replacement bytes are plain numbers (0x20, 0xADB4, ...) and stay unchanged. The Steam hooks toml = GOG toml with `address`, 7 `original_bytes`, 5 `consumed_exit_address` and `target_versions` swapped.
- The C++ needs only a per-version table (42 function addresses + operator new + app-manager pointer + 2 data words). One set of 43 detour exports serves both builds because frame layouts are identical.
- Count correction: the GOG toml has 40 one-byte simple hooks (not 41): 127 four-byte + 40 one-byte + 1 five-byte = 168 simple. Single-byte detours (only the opcode byte is fixed): H171, H182, H185.

Two non-hook tasks:
1. **GUI conflict (section 4):** the patch GUI is safe on vanilla Steam, but the live Override `partyselect_p.gui` is the ultrawide variant; installing the patch's copy would overwrite it and give a 4:3 layout. It needs a 32-slot ultrawide regeneration (the generator's partyselect cluster lists slots 0-11 only).
2. **Hook overlap:** only the upstream `full-party-select` Steam patch overlaps (H000, 0x89E319); the manifest already declares that conflict.

## 2. The four open items (anchors)

| Item | GOG | Steam | Anchors |
| --- | --- | --- | --- |
| CSWGuiButton ctor | 0x512A40 | **0x41A510** (hypothesis confirmed) | (a) stores vtable GOG 0x989BAC / Steam 0x987A1C; both resolve via RTTI COL->TypeDescriptor to `.?AVCSWGuiButton@@`. (b) Bodies identical (134 B, ret 0, same 4 callees: base ctor 0x916930->0x418570, label ctor x2 0x912E20->0x414A50, 0x915570->0x4171A0). (c) Steam `CSWGuiPartySelectionButton` ctor 0x89CDA0 (vtable 0x9A6D9C, RTTI `.?AVCSWGuiPartySelectionButton@@`) calls 0x41A510 as base ctor, then zeroes `[+0x1D0]`, so sizeof(CSWGuiButton)=0x1D0 = patch constant. (d) 246 callers both; `push 0x1D0; call new` sites 9/9. (e) 48-byte prefix with the SEH imm32 wildcarded is unique in .text on both. dtor 0x4CE630 -> 0x41A3D0 also identical (255/256 callers). |
| CSWGuiPartySelectionButton::SetSelected | 0x58A930 (229 B, ret 4) | **0x89CE20** (229 B, ret 4) | Body identical; writes `[this+0x1D0]=arg` (the field the Steam ctor zeroes), two 3-dword copies from .data triplets (12 refs, constant delta +0x6C8A0 GOG->Steam), then call 0x912DC0->0x4149F0 with args (1,1,0). 7 callers on both, same containing functions (HandleInputEvent, OnPanelAdded x3, OnToggled x2, OnDone). First 0x30 bytes unique in .text on both. In the vtable neighbourhood: GOG dtor 0x58A8F0 / Steam 0x89CDE0, same +0x40 spacing. |
| CExoArrayList<ptr>::DelIndex | 0x903DF0 (75 B, ret 4) | **0x49B840** | Generic 4-byte-element instantiation (MSVC folded it: 122 callers on both). The whole 75-byte body is unique in .text on both exes (exact byte match; Steam hit only 0x49B840). `CSWGuiPanel::BringControlToFront` (GOG 0x90E240 / Steam 0x40FDC0, identical) calls DelIndex on `this+0x24`, exactly the control array the patch edits (`GUI_PANEL_CONTROL_ARRAY_OFFSET`); `SendControlToBack` 0x90E290 / 0x40FE10 does too. Any identical 4-byte DelIndex would behave the same, but this is the one the panel itself uses. |
| Effect type word | 0x99C95C (rdata, dword 4) | **0x995F2C** (dword 4) | Identical use sites: `GetNPCObject` (identical body) GOG 0x70082B `mov edx,[0x99C95C]` -> Steam 0x5FACBB `mov edx,[0x995F2C]`, stored as WORD to `CGameEffect+8` right after `CGameEffect::ctor(1)` (0x7345E0 -> 0x5E4920). Same word is stamped by `EffectResurrection` (GOG 0x78B4AB / Steam 0x675ECC), so value 4 = the resurrection effect type, which is what the patch applies to a dead companion on load. 9 xrefs on both; same neighbouring table `02 03 04 05 06 07` (rdata constants). |
| Effect subtype word | 0x99C8B8 (rdata, dword 0) | **0x9967AC** (dword 0) | GOG 0x70083F `mov dx,[0x99C8B8]` -> Steam 0x5FACCF `mov dx,[0x9967AC]`; low 3 bits merged into `CGameEffect+0xA` (`and ecx,~7; or ecx,edx`), value 0 = instant/default duration type. 73 xrefs on both; neighbour dwords `3,1,2`. |

Other vtable-index and layout checks (Steam = GOG): CSWGuiButton vtable slot 16 (patch SetActive index 0x40/4) GOG 0x918530 / Steam 0x41A290 bodies identical;
CSWSCreature slot 4 (AsServerObject) 0x51BBD0 / 0x458940 identical, slot 39 (status) 0x6A80A0 / 0x5413C0 identical; slot 0 scalar-deleting destructor of CResGFF and CSWSCreature identical (both RTTI-verified vtables);
alloc sizes `push 0x1220` creature 20/20 sites, `push 0xA0` CResGFF 25/25, `push 0x8C` CGameEffect 227/227; APP_MANAGER_PTR GOG 0xA11C04 -> **Steam 0xA1B4A4** (WinMain stores the new CAppManager into it on both; `[app+8]` server / `[app+4]` client consumers identical, e.g. PartyMenuStealth vs Steam 0x523FB5).

## 3. Function and data table (also in the tsv as FN:/DATA:)

Every row: normalised body identical GOG vs Steam, same `ret N`, consistent callee pairs (0 conflicts). Mapper votes alone were not used. 39 of 42 are unique in `.text` by masked full-body byte search; the 4 thin wrappers (GetCreatureByGameObjectID server/client, ServerToClientObjectId, GetPlayerCreatureId) are not unique by shape, so they are anchored by callee body identity (0x7BADC0->0x537B70, 0x4383D0->0x78BDF0, 0x444780->0x798160, 0x7BCFB0->0x539D60), caller counts (517/517, 88/88, 120/120, 135/135) and the DB name_map.

| constant | GOG | Steam | size/ret | callers G/S |
| --- | --- | --- | --- | --- |
| GET_NPC_OBJECT | 0x700660 | 0x5faaf0 | 630/ret 12 | 31/31 |
| ADD_GAME_IN_PROGRESS | 0x703650 | 0x5fdca0 | 138/ret 0 | 8/8 |
| REMOVE_GAME_IN_PROGRESS | 0x7036e0 | 0x5fdd30 | 138/ret 0 | 8/8 |
| CLIENT_GET_CREATURE_BY_OBJECT_ID | 0x40ca10 | 0x73f550 | 28/ret 4 | 88/88 |
| GFF_READ_FIELD_VOID | 0x6257a0 | 0x719640 | 254/ret 24 | 10/10 |
| GFF_WRITE_FIELD_DWORD | 0x625cf0 | 0x719b90 | 96/ret 12 | 146/146 |
| GFF_WRITE_FIELD_VOID | 0x626310 | 0x71a1b0 | 174/ret 16 | 8/8 |
| GET_CURRENT_PARTY_TABLE | 0x64c2f0 | 0x51c8d0 | 22/ret 0 | 227/227 |
| GUI_PANEL_STOP_LOAD_FROM_LAYOUT | 0x90da20 | 0x40f5a0 | 117/ret 0 | 73/73 |
| PARTY_SELECTION_BUTTON_SET_SELECTED | 0x58a930 | 0x89ce20 | 229/ret 4 | 7/7 |
| GUI_PANEL_INIT_CONTROL | 0x90daa0 | 0x40f620 | 970/ret 16 | 1037/1038 |
| GUI_PANEL_ADD_CONTROL | 0x90e200 | 0x40fd80 | 57/ret 4 | 22/22 |
| GUI_PANEL_SET_ACTIVE_CONTROL | 0x90d080 | 0x40ec00 | 143/ret 8 | 33/33 |
| EXO_ARRAY_LIST_POINTER_DEL_INDEX | 0x903df0 | 0x49b840 | 75/ret 4 | 122/122 |
| GUI_CONTROL_ADD_EVENT | 0x916d90 | 0x418af0 | 177/ret 12 | 617/619 |
| GUI_BUTTON_SET_ENABLED | 0x9184b0 | 0x41a210 | 81/ret 4 | 1/1 |
| GUI_SET_MOVE_TO_CONTROL | 0x916990 | 0x4186f0 | 38/ret 8 | 105/105 |
| GUI_BUTTON_CTOR | 0x512a40 | 0x41a510 | 134/ret 0 | 246/246 |
| GUI_BUTTON_DTOR | 0x4ce630 | 0x41a3d0 | 140/ret 0 | 255/256 |
| CEXOSTRING_CTOR_CSTR | 0x605680 | 0x733570 | 126/ret 4 | 4154/4170 |
| CEXOSTRING_DTOR | 0x605890 | 0x733780 | 60/ret 0 | 10796/10817 |
| CREATURE_CTOR | 0x67ab60 | 0x561f30 | 4666/ret 8 | 20/20 |
| CREATURE_LOAD_FROM_TEMPLATE | 0x68b5a0 | 0x572a70 | 1346/ret 8 | 10/10 |
| CREATURE_SAVE_CREATURE | 0x68d030 | 0x574500 | 3243/ret 8 | 16/16 |
| CRESGFF_CTOR | 0x622ff0 | 0x716e60 | 312/ret 0 | 26/26 |
| CRESGFF_CREATE_FILE | 0x626530 | 0x71a3d0 | 446/ret 12 | 21/21 |
| CRESGFF_WRITE_FILE | 0x626700 | 0x71a5a0 | 198/ret 8 | 20/20 |
| CREATURE_STATS_GET_LEVEL | 0x6e1280 | 0x6a4e00 | 540/ret 4 | 107/107 |
| CREATURE_RECEIVE_EXPERIENCE | 0x6a34d0 | 0x58a9f0 | 608/ret 4 | 5/5 |
| OBJECT_CLEAR_ALL_ACTIONS | 0x6a7d60 | 0x541080 | 779/ret 4 | 58/58 |
| SERVER_GET_PLAYER_CREATURE_ID | 0x64c310 | 0x51c8f0 | 22/ret 0 | 135/135 |
| CREATURE_GET_FACTION | 0x6db8d0 | 0x5d0cb0 | 78/ret 0 | 61/61 |
| FACTION_ADD_MEMBER | 0x7e4850 | 0x6d3330 | 256/ret 8 | 14/14 |
| CREATURE_STATS_SET_MOVEMENT_RATE | 0x6f67a0 | 0x6ba320 | 761/ret 4 | 6/6 |
| CREATURE_HANDLE_ALIGNMENT_INFLUENCE | 0x683a70 | 0x56ae40 | 2646/ret 0 | 9/9 |
| SERVER_GET_AI_MASTER | 0x64bca0 | 0x51c280 | 31/ret 0 | 227/227 |
| AI_MASTER_SET_AI_LEVEL | 0x64db40 | 0x51e120 | 71/ret 8 | 23/23 |
| GAME_EFFECT_CTOR | 0x7345e0 | 0x5e4920 | 416/ret 4 | 227/227 |
| OBJECT_APPLY_EFFECT | 0x6aaef0 | 0x544210 | 1067/ret 12 | 190/190 |
| GET_CREATURE_BY_OBJECT_ID | 0x64bb20 | 0x51c100 | 28/ret 4 | 517/517 |
| TRANSFER_INVENTORY | 0x6fe4d0 | 0x5f8960 | 416/ret 4 | 5/5 |
| SERVER_TO_CLIENT_OBJECT_ID | 0x40d3a0 | 0x73fec0 | 28/ret 4 | 120/120 |
| OPERATOR_NEW | 0x921347 | 0x919723 | 101 vs 100 / ret 0 | not 16-aligned on either; callees 0x92B6C4->0x923A62, 0x927F56->0x9215E6 (_malloc); 20 `push 0x1220` sites each |
| APP_MANAGER_PTR | 0xA11C04 | 0xA1B4A4 | data | xrefs 6545/6548 |
| EFFECT_TYPE_WORD | 0x99C95C | 0x995F2C | data (4) | 9/9 |
| EFFECT_SUBTYPE_WORD | 0x99C8B8 | 0x9967AC | data (0) | 73/73 |

Notes: `GET_CURRENT_PARTY_TABLE` (0x64C2F0 -> 0x51C8D0, `return [this+4]+0x1F0B4`) keeps its GOG-DB mislabel ("GetSwoopUpgradeRowProvider"); only the body counts. 3 DB-vote rows (SetEnabled 0x41A210, SetMoveToControl 0x4186F0, GetServerAIMaster 0x51C280) are now body-verified. Caller-count differences of +1 on Steam (GUI_PANEL_INIT_CONTROL 1037/1038, ADD_EVENT 617/619, button dtor 255/256, CExoString 4154/4170, 10796/10817) are Aspyr additions elsewhere, not mismatches.

## 4. Hook table (full per-hook rows in the tsv)

Containing functions, GOG start -> Steam start (all bodies identical, ratio 1.0, 0 differing instruction runs):
- 0x4ceb50->0x7be4c0 Initialize ; 0x4dc020->0x7cb920 ShowPartySelection ; 0x4e0e60->0x7d0760 UpdateCreatedInGameGUI
- 0x58aa40->0x89cf30 CSWGuiPartySelection ; 0x58b740->0x89dc30 ~CSWGuiPartySelection ; 0x58b980->0x89de70 HandleInputEvent
- 0x58bb10->0x89e000 OnPanelAdded ; 0x58c530->0x89ea20 OnPanelRemoved ; 0x58c660->0x89eb50 OnToggled
- 0x58c8a0->0x89ed90 OnDone ; 0x58cb00->0x89eff0 OnEnter ; 0x58d520->0x89fa10 UpdateCount
- 0x58d650->0x89fb40 SetForcedNPC ; 0x58d6e0->0x89fbd0 AcceptParty ; 0x58e370->0x8a0860 SetAddRemoveLabel
- 0x59ecf0->0x8aaea0 OnPrevNPCClicked ; 0x59eeb0->0x8ab060 OnNextNPCClicked ; 0x5b78d0->0x8c7510 PopulateItemList
- 0x5b93f0->0x8c9030 BuildUpgradeList ; 0x6e5820->0x6a93a0 GetAttackModifierVersus ; 0x6e9880->0x6ad400 ModifyAlignment
- 0x6fb270->0x6bedf0 AutoLevelup ; 0x6fd210->0x5f76a0 ClearTable ; 0x6fda30->0x5f7ec0 UpdateMembers
- 0x6fdaa0->0x5f7f30 SaveMember ; 0x6fdca0->0x5f8130 SaveMemberByObject ; 0x6fe8f0->0x5f8d80 AddMember
- 0x6fed90->0x5f9220 CreateParty ; 0x6ffb60->0x5f9ff0 AddNPC ; 0x700170->0x5fa600 AddNPC
- 0x700400->0x5fa890 AssignPUP ; 0x700490->0x5fa920 RemoveNPC ; 0x7004d0->0x5fa960 GetIsNPCAvailable
- 0x700530->0x5fa9c0 GetInfluence ; 0x700560->0x5fa9f0 SetInfluence ; 0x7005f0->0x5faa80 SetNPCObject
- 0x700660->0x5faaf0 GetNPCObject ; 0x700be0->0x5fb070 GetIsMember ; 0x700c60->0x5fb0f0 GetIsAvailable
- 0x700cb0->0x5fb140 GetNPCSelectability ; 0x700ce0->0x5fb170 SetNPCSelectability ; 0x700d10->0x5fb1a0 SaveTableInfo
- 0x701d10->0x5fbf60 LoadTableInfo ; 0x703350->0x5fd9a0 KillNPCObject ; 0x7035a0->0x5fdbf0 GetNPCID
- 0x703770->0x5fddc0 ClearMembers ; 0x703820->0x5fde70 ClearMember ; 0x703a70->0x5fe0c0 DistributeExperience
- 0x703d60->0x5fe3b0 GetIsLeader ; 0x703dc0->0x5fe410 SetLeader ; 0x703e60->0x5fe4b0 SwitchPlayerCharacter
- 0x704780->0x5fedd0 AutoLevelUpNPCs ; 0x7048d0->0x5fef20 UnstealthParty ; 0x704d60->0x5ff3b0 GetNextPC
- 0x7b1a30->0x69c460 ExecuteCommandRebuildPartyTable ; 0x8079d0->0x661c30 HandlePlayerToServerLevelUpMessage

Hook classes and verdicts:

| class | count | verdict | what was checked |
| --- | --- | --- | --- |
| simple 4-byte imm32 (panel/record offsets, alloc size 0x50B4) | 127 | SAME x127 | same instruction text, same operand slot, bytes equal on Steam |
| simple 1-byte | 40 | SAME x40 | see below |
| simple 5-byte NOP (collapse call) | 1 | SAME | H000 GOG 0x58BE29 -> 0x89E319, bytes `E8 42 26 00 00` equal; upstream FullPartySelect already hooks 0x89E319 on Steam, independent confirmation |
| detours | 43 | SAME x43 | all covered ranges end on an instruction boundary on Steam; none cross a branch except H171/H185 (both skip_original) |

One-byte simple hooks (H128-H167), operand role per class, all identical on Steam with 7/7 context instructions equal:
- 33 x `0C -> 20`: `push 0xC` (H128 0x58AAAD->0x89CF9D, H130 0x58B932->0x89DE22) or `cmp DWORD PTR [ebp+-x],0xC` (H129 0x58AC35->0x89D125, H131 0x58BE89->0x89E379, H132-H135, H136/H137 GetAttackModifierVersus 0x6AB180/0x6AB5AE, H138 ModifyAlignment 0x6AD602, H139 AutoLevelup 0x6C015C, H140/H141 upgrade screens 0x8C76C8/0x8C9173, H142 level-up message 0x661FE2 [ebp-0x34], H143-H146, H147 0x8AAF18, H150/H151, H155-H160, H162, H164-H167). Each is the roster-limit compare/loop bound (the roster count 12), at the same operand position (offset 1, 3, 4 or 6 depending on the addressing form; GOG and Steam offsets are equal).
- H148 `mov [edx+0x50EC],0xB` -> 0x1F (equip panel last-NPC index; GOG 0x59EDB1 / Steam 0x8AAF61, offset 6).
- H149 `push 0 -> push 1` (GetNPCObject createObject arg; GOG 0x59EDE6 / Steam 0x8AAF96, offset 1).
- H152-H154, H161, H163 `push ecx|edx -> push eax` (GOG 0x703AFD, 0x703BFA, 0x70497B, 0x6FF4D5, 0x6FF53F / Steam 0x5FE14D, 0x5FE24A, 0x5FEFCB, 0x5F9965, 0x5F99CF): the object-id argument of `GetCreatureByGameObjectID` (0x64BB20 / 0x51C100); eax carries the detour result. Each sits exactly at the end of the preceding detour range (H186/H187/H188/H169/H170), same on Steam.

Single-byte detours:
- H171 GOG 0x58B3F0 -> Steam 0x89D8E0, `E8 rel32`. Steam bytes `E8 BB 1C B7 FF` -> 0x40F5A0 = CSWGuiPanel::StopLoadFromLayout (GOG 0x90DA20); original skipped, the C++ calls it itself via `ADDRESS_GUI_PANEL_STOP_LOAD_FROM_LAYOUT`. SAME.
- H182 GOG 0x701336 -> 0x5FB7C6, `68 imm32` = address of "PT_AISTATE" (GOG 0x99B514, Steam 0x9970C0, same string and same rodata neighbour "PT_NPC_INFLU"). Steam original_bytes `68 C0 70 99 00`; absolute operand, so replay in the trampoline is safe. SAME.
- H183 `6A 00 8D 45 88` (LoadTableInfo, GOG 0x7024C2 -> 0x5FC712), next insn pushes the same string (0x99B514 / 0x9970C0). Bytes equal.
- H185 GOG 0x59F13B -> 0x8AB2EB, `E9 0A FE FF FF` (same displacement). Targets GOG 0x59EF4A / Steam 0x8AB0FA = its `consumed_exit_address`. SAME.

Bytes that differ on Steam (use for the Steam toml): H171 `E8 BB 1C B7 FF`; H172 (0x89DC30) `55 8B EC 6A FF 68 FA 71 97 00`; H175/H176 (0x5F7F30, 0x5F8130) `55 8B EC 6A FF 68 CB 47 95 00`; H182 `68 C0 70 99 00`; H192 (0x5F9FF0) `55 8B EC 6A FF 68 3B 49 95 00`; H206 (0x89E000) `55 8B EC 6A FF 68 28 72 97 00`.
`consumed_exit_address` Steam: H184 0x8AB2EB, H185 0x8AB0FA, H192 0x5FA3AA, H203 0x5FAD4B, H204 0x5FB164 (each lands on an instruction boundary with identical context).

## 5. partyselect_p.gui

Extracted read-only from `chitin.key` -> `Gui.bif` with PyKotor (`Chitin(...)`): `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/eeafb247-6865-493b-94e8-e84981ae4a54/scratchpad/rc/rc/partyselect_p_vanilla.gui` (42,864 B, sha 0C8AF92D...). Also copied the live Override file (687A32A5..., same size 42,864, sha differs) and the patch file (97,652 B, sha CCF7D1F0...).

| | controls | LBL_CHAR / BTN_NPC / LBL_NA | other |
| --- | --- | --- | --- |
| Steam vanilla (BIF) | 53 | 12 / 12 / 12 | LBL_NAMEBACK, LBL_BAR0-4, LBL_3D, LBL_TITLE, LBL_COUNT, LBL_BEVEL_M/L, LBL_NPC_NAME/LEVEL, BTN_ACCEPT/BACK/DONE |
| Patch | 115 | 32 / 32 / 32 | same set plus BTN_PAGE_PREV (295,516,50x28), BTN_PAGE_NEXT (350,516,50x28) |
| Live Override | 53 | 12 / 12 / 12 | ultrawide variant: only LEFT/WIDTH of 52 controls differ from vanilla |

Field-level diff (all GFF fields, controls keyed by TAG): the patch differs from Steam vanilla only in (1) slot control positions and MOVETO links (a 16-per-page grid: x 74-403, y 135-464), (2) 20 extra slots per prefix, each with exactly the same field set as slot 0, (3) BTN_ACCEPT/BACK/DONE MOVETO links re-pointed at the new indices, (4) LBL_COUNT text "PAGE" instead of strref/"2", (5) two new buttons. Every non-slot control keeps type, extent, borders, fonts and colours. All 15 resref values (borders, fonts, fills, `po_no`, `pnl_pause_pc`) are the same as vanilla's, so every texture/font exists on Steam. Tags and IDs are unique (114 IDs, 0-147), no duplicate tags.

Conclusion: the patch GUI is a clean derivative of the vanilla layout and is safe on vanilla Steam (same structure as the GOG copy it was built from, since Steam vanilla matches in every untouched control). It is NOT drop-in for this workspace: the live Override copy is the ultrawide-scaled partyselect, and the patch's 800x600 layout would replace it. `scripts/generate_ultrawide_gui_overrides.py` handles only `LBL_CHAR/BTN_NPC/LBL_NA` 0-11 around centre 241; it needs the 32-slot names, the two page buttons, and the shifted x range before a Steam-ultrawide GUI can be generated. A KPM-installed `Additional/` copy also needs a decision on who owns that Override file.

## 6. Remaining risks

1. Ultrawide/Override ownership (above) and no 5:4/16:9 art for the 2-page grid; test the 16-slot page layout on the live ultrawide setup.
2. Save data: the patch writes `PRL32_VER`/`PRL32_DATA` into the party GFF (saves with 13+ members lose extended members if the mod is removed). Needs the full in-game test (recruit 13+, save/load, level-up, equip next/prev, kill/unstealth).
3. Beta upstream (0.0.1b); no Steam testing by the author. C++ 43 exports and ~1,300 lines are unchanged risk beyond addresses.
4. Interaction with live Steam mods that read the 12-slot party table or hook party-select screens was not exercised; the static overlap scan (206 hook files across all KPM worktrees and patch_manager_mods) found only FullPartySelect at 0x89E319.
5. The runtime table selector must treat an unknown sha as GOG (EKC pattern) and refuse to load on a Steam exe with the wrong sha; `GameFunction` indirection touches ~45 call sites.
6. 4 wrappers (above) are anchored by callee/caller evidence, not by unique bytes; spot-check them in a first run with logging (call GetCreatureByGameObjectID with a known party object id).
7. The live exe is vanilla + 4gb-patch (header flag only); hook bytes verified against the pure vanilla copy, so KPM's sha/bytes check on the live exe should still pass, but confirm with the stage/verify script.

Files: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/eeafb247-6865-493b-94e8-e84981ae4a54/scratchpad/rc` = /tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/eeafb247-6865-493b-94e8-e84981ae4a54/scratchpad/rc (lib.py, allhooks.py, funcdiff.py, guideep.py, extracted GUIs).
