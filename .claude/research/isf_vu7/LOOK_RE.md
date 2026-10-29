# VU7 LOOK_RE: inventory list row restyle (height, icon, font)

Exe: Steam swkotor2.exe, sha256 prefix 6a522e71631d (`backups/vanilla_clean/swkotor2.exe`). Dumps: `/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/`. Vtable slots read from the PE image (read-only). Asm is `decomp/<bb>/<addr>.asm`.

Row object: 0x3B4 bytes. Ctor 0x8B0A60 (`8b/008b0a60.asm`): base ctor 0x41A510; vftable 0x9A76C4 at +0x0; CSWGuiBorder x3 at +0x1D4, +0x24C, +0x2C4 (0x78 apart); CSWGuiText at +0x33C (ends 0x3B4). Border params at border+0x14 (= +0x1E8, +0x260, +0x2D8). Text params at text+0x18 (= +0x354).

## Q1. vt58 object and row height (PARTIAL, height formula CONFIRMED)

- `*(panel+0x864)` = LB+0x2DC (LB = panel+0x588). `8a/008a75f0.asm` 0x8A76B5 `MOV EDX,[ECX+0x864]`; 0x8A76CF `CALL [EDX+0x58]`. The returned object is "vt58": template with sub-objects at +0x70, +0xE8, +0x160.
- Row vt+0xA0 = 0x8B0FB0 (PE vtable read: 0x9A76C4+0xA0). Called only on NEW rows: `8a/008a75f0.c:25-44` (branch `*param_3 == panel[0x24B4]`); reused pool rows skip it.
- Args `(iVar5+0x160, iVar5+0x70, iVar5+0xE8, width, 1)`, width = LB[+0x2A4] - 2*LB[+0x2D0] (`008a75f0.c:39`). The +0x160/+0x70/+0xE8 pointers are template objects, NOT a height rect.
- Inside 0x8B0FB0 (`8b/008b0fb0.c`): text params copied from template+0x160+0x18 (= +0x178) via 0x416BD0 (asm 0x8B0FCC `ADD ECX,0x178`); border params copied from template+0x70+0x14 and +0xE8+0x14 via 0x4144F0 (lines 16-30 area). Colours set at row+0x98.. (line 22-24).
- ROW HEIGHT (CONFIRMED): tail of 0x8B0FB0, asm 0x8B11C6 `CALL 0x479AB0` -> 0x8B11EE `FMUL double [0x99EFA0]` -> 0x8B11F4 `CALL 0x91C860` (ftol). Rect `{0, 0, width, h}` built at `008b0fb0.c` tail, then row vt+4 (0x8B1217). `h = trunc(36.0 * S)`, 36.0 = double at VA 0x99EFA0 (read from exe).
- S = `FUN_00479AB0` (`47/00479ab0.c`): `S = (max(0, DAT_009F42A4 - 0x3FE) >> 10) * _DAT_0098C014 + 1.0`. At base S = 1.0 -> h = 36 px. DAT_009F42A4 identity not confirmed (resolution-like).
- Row vt+4 = 0x8B0C30 (PE vtable read; named `CSWGuiInGameItemEntry::SetExtent` by ledger, probable). Body (`8b/008b0c30.c` lines 1-16): guard `if (0x24 < param_2[2])` tests WIDTH > 36, not height. If width <= 36 nothing is stored. Stores rect at row+4 (x), +8 (y), +0xC (w), +0x10 (h).

## Q2. LB fields (CONFIRMED for width/inset, UNSURE for scroll step)

- LB+0x2A4 (dword) = list width base; LB+0x2D0 (byte) = horizontal inset, used as `width = +0x2A4 - 2*(+0x2D0)` (`008a75f0.asm` 0x8A76E9-0x8A76FD).
- Row height is NOT an LB field. It comes from `36.0 * S` (Q1).
- LB+0x2DC (template pointer) writer: not found. `rg "0x864|[0x219]"` hits only `8a/008a75f0.c` and `66/00665f50.c:429` (unrelated vtable slot). Class of the template object: unknown.
- Scroll step: UNSURE. No field found in the CreateItemEntry slice.

## Q3. Which border is the icon (UNSURE)

- Border params per border: #1 +0x1E8, #2 +0x260, #3 +0x2D8. SetItem writes resrefs there (`ADD ECX,0x1e8/0x260` asm 0x8B1712/0x8B173C; `ADD EAX,0x2D8` asm 0x8B1BE8).
- SetItem sets frame resrefs `UIBIT_EQP_ITM1/2/3` by stack tier (`8b/008b1250.c:186-212`; tiers `<2`, `<100`, else). Those are frame art, not the item icon.
- No `CSWSItem::GetIcon` call is visible in SetItem. Callee list: 0x51C8B0, 0x51C0E0 (GetItemByGameObjectID), 0x7E5D80, 0x7FEEC0, 0x7356B0, 0x734190, 0x615CC0. The earlier FINDINGS claim "icon = GetIcon into border fill" is NOT supported by SetItem's visible code.
- Border rect/dimension for the icon: not located. Candidate: CSWGuiBorderParams::dimension in the template copies (B = +0x70, C = +0xE8).

## Q4. Single CSWGuiText: name or stack count (PARTIAL)

- The single CSWGuiText (row+0x33C, params +0x354) shows the STACK COUNT.
  - count = CSWSItem+0x2CC (`008b1250.c:191`).
  - count string via `FUN_00734270(local_3c, DAT_00986B94, count)` only when count >= 2 (`008b1250.c:203-211`).
  - font `fnt_d16x16` set on every SetItem (`008b1250.c:72, 99, 213`; 0x416AA0 -> SetBaseFont 0x416DE0), then 0x416BD0 with ECX = row+0x354 (asm 0x8B1873 `ADD ECX,0x354` -> 0x8B1879 `CALL 0x416BD0`; 0x8B1A06 -> 0x8B1A0C).
- The item NAME goes to the base-class text params at row+0x178: `008b1250.c:188` `FUN_00416e30(local_48)`; asm 0x8B16B4 `ADD ECX,0x178` -> 0x8B16BA `CALL 0x416E30`. Its font is inherited from the template copy in 0x8B0FB0 (new rows only).
- SetItem re-runs on reused pool rows (`008a75f0.c` calls SetItem at line 110 on every populate). So the count font and frame resrefs are reset on every populate. Name font is not reset by SetItem, only by the 0x8B0FB0 template copy (new rows).
- SetItem tail (`008b1250.c:227`) calls row vt+4 (SetExtent) with row+4, which re-asserts the stored rect each populate.

## Recommendation for VU7c

1. Row height (36 px * S): apply after createRow in ISF (`InventorySortFilter.cpp`), by calling row vt+4 with `{0, 0, width, newH}`. Keep width > 36 or the guard drops it. Alternatively write row+0x10 directly. Pool rows keep the value (0x8B0FB0 runs only on new rows). Changing the global double at 0x99EFA0 would hit every user, so do not patch it. Scroll-step effect on LB is OPEN.
2. Icon size: blocked: needs live REA on which border holds the icon fill (and its dimension field). Static dumps show no GetIcon in SetItem.
3. Count font: cheapest apply point is a post-SetItem SetBaseFont on row+0x354 (after 0x416BD0), re-applied every populate. Do not edit the `fnt_d16x16` string globally.
4. Name font: apply to row+0x178 params after createRow (per row). Template A+0x178 edits only affect new rows.

OPEN: who writes LB+0x2DC (template object class and its +0x70/+0xE8/+0x160 field meanings); scroll step field; which border is the icon.
