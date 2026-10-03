# B0 kdb_overrides: Opus review of the 4 draft rows

Sources: the KDB name_map (read-only), the Steam dump `steam_6a522e71/decomp`, the fp.tsv files (GOG / Steam / Mac),
B0/final_rows.tsv, overrides.tsv, branch_functions.csv and opus/overrides_f1.tsv.md, plus the persist logic in
tools/batch.py `plan_persist` and bdlib `is_trusted`.

| steam | action | decision | change to the draft |
|---|---|---|---|
| 0x008827a0 | rename -> CSWGuiMainMenu::OnLoadSaveGame | approve_edited | The note quoted the GOG size 0x1b74. The Steam size is 0x1e44. The source is PR3, not "Batch 1". Anchors added. |
| 0x0054d3d0 | demote | approve_edited | Asm anchors added. |
| 0x006076d0 | rename -> CSWSItem::GetUpgradableType | approve_edited | The note said "721 B". The body is 181 B. **Needs a companion demote of 0x00746fc0** (see below). |
| 0x00740fc0 | rename -> CClientExoApp::GetCharacterChangeInProgress | approve_edited | Asm anchors added. No duplicate name. |

## 0x008827a0: rename to CSWGuiMainMenu::OnLoadSaveGame

- **Current KDB row:** `CSWGuiMainMenu::OnLoadGamePicked` (steam-db-pr3-savegame, high, trusted). Its evidence says "GOG 0x53cf90" and "GOG DB param 0, Steam retn 4". That retn mismatch was the warning sign.
- **Fingerprints:**
  - Steam 0x8827a0: 395 B, retn 4, 115 insn. This equals GOG 0x53d240 `OnLoadSaveGame` (395/4/115). The disp lists are identical. The imm lists are identical except one value: GOG 0x1b74 is Steam 0x1e44, which is the CSWGuiSaveLoad layout growth of +0x2d0.
  - Steam 0x8824f0: 679 B, retn 0, 217 insn. This equals GOG 0x53cf90 `OnLoadGamePicked` (679/0/217), with identical disp and imm lists.
- **Anchors:**
  - `0088284d PUSH 0x1e44` / `00882852 CALL 0x00919723` (operator new)
  - `00882880 CALL 0x00850770` (KDB: CSWGuiSaveLoad::Constructor, high)
  - `00882928 RET 0x4`
- **B0 consistency:** B0 ships OnLoadGamePicked at 0x8824f0 (final_rows, branch_functions id 460, kdb_upsert, all verified). Its note says "OnLoadSaveGame = 0x8827a0".
- **KDB name check:** the KDB has no OnLoadSaveGame row anywhere, so the rename creates no duplicate.
- **Why the rename is needed:** it also lets the 0x8824f0 upsert land. Without it, `trusted_holders` would skip that upsert under R4, because 0x8827a0 still holds OnLoadGamePicked as trusted.
- **Note fixes:** the draft's "new(0x1b74)" quoted the GOG value. The draft's "Batch 1 wrong symbol" is wrong too: the source is `steam-db-pr3-savegame`.

## 0x0054d3d0: demote

- **Current KDB row:** `CSWSArea::SetHitInfoForCreature` (batch5_reverse_engineering, high). Its evidence is "Ported from GOG 0x707770 (verified)". Under bdlib this source is not trusted.
- **The GOG label is wrong.** Steam 0x54d3d0 equals GOG 0x707770 (894/32/260, identical disp and imm lists). The Mac binary shows what that body really is:
  - Mac EvaluateOverlappingTargets 0x4261de: calls GetCreatureByGameObjectID, disp 0x2c/0x30/0x380, no IntersectCircle2d.
  - Mac SetHitInfoForCreature 0x426444: calls IntersectCircle2d 0x31701b, disp up to 0x44.
- **Steam 0x54d3d0 has the EvaluateOverlappingTargets shape:**
  - `0054d404 CALL 0x0051c100` (KDB: CServerExoApp::GetCreatureByGameObjectID, high)
  - `0054d40f MOV ECX,[EAX+0x2c]`, `0054d43f ADD EDX,0x94`, `0054d551 MOV EDX,[ECX+0x380]`, `0054d74b RET 0x20`
  - It does not call 0x5192c0 and has no 0x44 disp.
- **The real SetHitInfoForCreature is Steam 0x556040.** It equals GOG 0x710260 (1130/20), and B0 ships it verified: `0055609a CALL 0x005192c0` (IntersectCircle2d).
- **Both Steam bodies are siblings:** NoCreaturesOnLine 0x54d750 calls both 0x54d3d0 and 0x556040, as on Mac.
- **Demote is right.** The current name is proven wrong for this body. A rename to `CSWSArea::EvaluateOverlappingTargets` would also be justified, but that is optional and not proposed here.

## 0x006076d0: rename to CSWSItem::GetUpgradableType

- **Current KDB row:** `CSWSItem::GetUpgradeType` (mods678-batch3-dump, high, trusted). The real symbol is GetUpgradableType in the GOG, Linux (`CSWSItem__GetUpgradableType`) and Mac binaries, so "GetUpgradeType" is a paraphrase.
- **Fingerprints:** Steam 0x6076d0 is 181 B, RET 4, 58 insn. This equals GOG 0x66af00 GetUpgradableType (181/4/58, disp 0xac/0x2f0, identical imm list including 0x1f/0x26/0x27/0x28/0x29).
- **Anchors:**
  - `006076e0 CMP dword ptr [EAX+0x2f0],0x0`
  - `006076f0 CALL 0x006d6e30` (counterpart of GOG callee 0x4ecc00 CSWItem::GetBaseItem)
  - `006076fb MOV DL,byte ptr [ECX+0xac]`
  - `00607782 RET 0x4`
  - Mac GetUpgradableType has disp 0xac/0x2f0 and calls GetBaseItem.
- **Note fix:** the draft's "721 B body" was wrong. 721 B is GetPropertyDescription at 0x607790. The body here is 181 B.
- **Duplicate problem.** The KDB already holds `CSWSItem::GetUpgradableType` at **0x00746fc0** (gog_item_cluster, high, evidence "Ported from GOG 0x66af00 (verified); retn=0"). That row is wrong:
  - Steam 0x746fc0 is 79 B, retn 0. It loops over 0x20 entries with stride 0x1d0: `00746fe2 CMP [EBP-0x8],0x20`, `00746feb IMUL ECX,ECX,0x1d0`, `00746ff4 MOV EAX,[EDX+ECX*1+0x10074]`, `0074700e RET`.
  - It has no item fields and no GetBaseItem call.
  - B0 moved branch id 519 from 0x746fc0 to 0x6076d0, but nothing in the KDB plan touches 0x746fc0.
  - gog_item_cluster is not a trusted source, so persist will not skip the upsert for 0x6076d0. After the rename, though, name_map would hold the name at two addresses.
- **Required companion row.** Add this to kdb_overrides.tsv together with the 0x6076d0 rename. I have not added it to the review TSV, because that file has one row per draft row:

  ```
  0x00746fc0	demote		gog_item_cluster port of GOG 0x66af00 is wrong: 79 B RET, 00746feb IMUL ECX,ECX,0x1d0 loop over 0x20 slots; GetUpgradableType is 0x6076d0 (181 B RET 4)
  ```
- **Side note:** the KDB `functions` table still has `006076d0 CSWSItem::GetUpgradeType`. The name_map override does not change that table.

## 0x00740fc0: rename to CClientExoApp::GetCharacterChangeInProgress

- **Current KDB row:** `GetCharacterChangeInProgress (client app, plain RET)` (level-up-P4-static-confirmed, high, trusted).
- **Fingerprints:** Steam 0x740fc0 is 31 B, retn 0, 12 insn, disp 0x4/0x4ac. This equals GOG 0x40e480 CClientExoApp::GetCharacterChangeInProgress (31/0/12, same disp and imm).
- **Anchors:**
  - `00740fcc MOV ECX,dword ptr [EAX+0x4]`
  - `00740fcf MOV EDX,dword ptr [ECX+0x4ac]`
  - `00740fde RET`
  - Setter twin 0x740fe0 (KDB: CClientExoApp::SetCharacterChangeInProgress): `00740ff8 MOV [EDX+0x4ac],EAX`, `00741001 RET 0x4`.
  - Mac Get/Set 0x1c17fa/0x1c180c have the same shape (disp 0x4/0x4a8 on Mac).
- **Duplicate check:** the only name_map rows matching `%CharacterChangeInProgress%` are 0x740fc0 itself and the setter 0x740fe0. No other address holds this name, so the rename creates no second address.
- **What the rename does:** base_name changes from `GetCharacterChangeInProgress` to `CClientExoApp::GetCharacterChangeInProgress`, so the rename fires. The B0 upsert at 0x740fc0 then has the same name and is skipped as "already trusted". The result is one row with the qualified name.
