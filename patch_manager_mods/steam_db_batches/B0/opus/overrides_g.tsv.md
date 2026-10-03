# B0 Opus gate, part g (8 rows, no Sonnet review)

Gate: Opus, 2026-10-03, following `OPUS_GATE_BRIEF.md`. Verdict rows are in `overrides_g.tsv`.

These are B0_SUMMARY "Open items" 1: ids 504-506, 935, 937-940. They were AUTO before the G8 re-gate and REVIEW
(`why:c1`) after it. They were in no card chunk and no Opus part, so finalize had marked them `unresolved`.

## Method
- Compared GOG and Steam `fp.tsv` rows by script. Size, retn, n_insn, disp multiset and imm multiset are identical on
  all 8 rows, and each fingerprint is unique in both binaries (1 GOG hit, 1 Steam hit).
- Mapped every GOG callee to a Steam callee by fingerprint. Each callee has exactly one fp-equal partner inside the
  Steam callee list, except where several 28 B CServerExoApp getters share a fingerprint. Of AddItem's 24 callees,
  2 have no fp-equal partner (CExoLocString::GetString and AddPazaakCard; same-size partners at 0x7356b0 (311 B) and 0x58aed0 (417 B)).
- Quoted asm CALL lines come from `steam_6a522e71/decomp/60/<addr>.asm` (grep output).
- Name check: Mac x86 symbols of the same name. Mac order is CompareItem, MergeItem, SplitItem, CopyItem,
  InsertItem, FindItemWithBaseItemType, FindItemWithTag, ClearNewItems. This is the GOG/Steam order, except that
  the CItemRepository ctor and AddItem sit elsewhere on Mac. I found no label rotation, because each body's callee
  set fits its own Mac symbol and no other.

## Rows
| GOG | Steam | name | anchor | Mac fit | verdict |
|---|---|---|---|---|---|
| 0x00667f40 | 0x00604710 | CSWSItem::CompareItem | 006048f1 CALL CExoString!= (0x733ae0), 00604917 CALL CExoLocString!= (0x7353e0), 2x GetActiveProperty 0x604260, 2x GetPassiveProperty 0x6042a0 | Mac CompareItem calls the same 2 operators (Get*Property inlined) | verified |
| 0x006685f0 | 0x00604dc0 | CSWSItem::MergeItem | 00604dd0 CALL 0x604710 (CompareItem), 00604df8 CALL 0x6d5200 GetBaseItem | Mac MergeItem = GetBaseItem + CompareItem | verified |
| 0x006686f0 | 0x00604ec0 | CSWSItem::SplitItem | 00604f15 CALL new 0x919723, 00604f35 CALL 0x5ffd60 (CSWSItem ctor, 772 B), 00604f60 CALL 0x606600 (CopyItem, 1143 B) | Mac SplitItem = ctor + CopyItem + operator new | verified |
| 0x006687e0 | 0x00604fb0 | CItemRepository::CItemRepository | 00604fbf CALL 0x84a640 (CExoArrayList ctor); stores param_2 to [this], sets bit0 of [this+8] from param_3, returns this. 8 callers in both binaries, including CopyItem (0x669e30 / 0x606600) | Mac ctor (57 B, array ctor inlined) | verified |
| 0x00668850 | 0x00605020 | CItemRepository::InsertItem | 00605064 CALL 0x414280 (CExoArrayList::insert), 0060507e CALL 0x83ea60 (Add); index-in-range test chooses between them | Mac InsertItem = Insert + Add | verified |
| 0x006688c0 | 0x00605090 | CItemRepository::AddItem | 22 of 24 callees fp-equal (the other 2 same size): 0x604dc0 MergeItem, 0x600380 SetPossessor, 00605159 CALL 0x5813c0 AddGold, 0x581630 AddChemical, 0x581710 AddComponent, 0x64b760 SendServerPlayerItemUpdate_DestroyItem | Mac AddItem has the same callee set (MergeItem, SetPossessor, AddGold, DestroyItem msg) | verified |
| 0x006693b0 | 0x00605b80 | CItemRepository::FindItemWithBaseItemType | 00605be0 CALL 0x51c0e0 GetItemByGameObjectID, 00605c35 CALL 0x6d6e30 CSWItem::GetBaseItem, 00605c1c self-recursion | Mac: CSWItem::GetBaseItem + GetItemByGameObjectID + self | verified |
| 0x006694e0 | 0x00605cb0 | CItemRepository::FindItemWithTag | 00605d04 CALL 0x51c0e0 GetItemByGameObjectID, 00605d1c CALL 0x7339d0 CExoString==, 00605d58 self-recursion | Mac: CExoString== + GetItemByGameObjectID + self | verified |

## Totals
verified 8, likely 0, label_suspect 0, rejected 0, unresolved 0. No Steam address collides with another B0 row.

sample_errors: 0 (this part has no sampled reviewer rows)
