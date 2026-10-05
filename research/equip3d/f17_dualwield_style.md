# F17: engine rule weaponwield -> creature animation style

Status: 2026-10-05. Method: local disassembly only (scripts/disasm_va.py steam|gog, query_ghidra.py), plus the existing
Ghidra decompile `patch_manager_mods/equipment_3d_character_src/research/R7f_wield_style_0076d590_2026-09-30.txt`.
No headless Ghidra run needed. No code edited.

## Deciding function

| build | address | size | name |
|---|---|---|---|
| Steam (Aspyr) | `0x0076d590` | 742 B | `CSWCCreature::UpdateWeaponClass` (name_map, conf medium; body confirms) |
| GOG | `0x00419400` | 742 B | `UpdateWeaponClass` (query_ghidra symbol, trusted) |

GOG->Steam delta = +0x354190. Same size, same instruction sequence (identical stores of 1,2,3,5,7,9 / 4 / 6 / 8 and
`0xc5`/`0xc7`/`0xd0` writes). GOG jump table `0x4196e8`, Steam jump table `0x76d878`.

The GOG anchors in the brief (`Wield` 0x004f5400 / 0x004f3df0, `CSWCAnimBaseHeadWield` 0x004f3850) are the visual
wield-model attach in CSWCAnimBase, NOT the style decision. They are not the answer.

## Data flow (this = `this` is the CSWCCreature)

- `0x0076e610(slot)` returns the equipped item OID for an inventory slot. Slot `0x10` = right weapon, `0x20` = left weapon.
  `0x7f000000` = OBJECT_INVALID = empty slot.
- Item lookup `0x0073f530(oid)`, then baseitem row via `0x006d5200(item+0x0c)` on the baseitems cache (`[0xa1b4d0]+0x34`).
- baseitem byte `+0x08` = `weaponwield` (the style input). baseitem byte `+0x09` is copied to creature `+0xc7` (a second
  per-weapon class byte; not needed for the style).
- Result style byte -> creature `+0xc5`. The animation prefix is `g<style>r1` (idle) via the creature anim code.
  A second flag (right/left item ranged-ness via `0x007fee70`/`0x00615c80`) -> creature `+0xd0`; feat `0xcf` check -> `+0xd4`.

## Engine rule (full table)

Notation: R/L = weaponwield of the item in the right/left slot. "-" = slot empty. Data has weaponwield 0..6 only
(EquipWeaponTable.inc: 0 for 76 rows, 1..6 otherwise). Style 0 is the default and means "no style-specific idle".

| rHas | lHas | R | L | style (`+0xc5`) | `+0xc7` |
|---|---|---|---|---|---|
| no | no | - | - | **8** (unarmed) | 2 |
| no | yes | - | any | **0** | unchanged |
| yes | no | 1 | - | 1 | R.byte9 |
| yes | no | 2 | - | 2 | R.byte9 |
| yes | no | 3 | - | 3 | R.byte9 |
| yes | no | 4 | - | **5** | R.byte9 |
| yes | no | 5 | - | **7** | R.byte9 |
| yes | no | 6 | - | **9** | R.byte9 |
| yes | no | 0 or >=7 | - | 0 | R.byte9 |
| yes | yes | any (ignored) | 2 | **4** (dual melee) | L.byte9 |
| yes | yes | any (ignored) | 4 | **6** (dual blaster) | L.byte9 |
| yes | yes | any (ignored) | 0,1,3,5,6,>=7 | 0 | L.byte9 |

Notes:
- Both-hands style depends ONLY on the LEFT weaponwield. R is never read in that branch.
- If an item OID is set but the item/baseitem lookup fails, the style stays 0 and later writes are skipped.
- Right-only mapping is a jump table over (wield-1) in 0..5; anything else falls to the default 0.

## Excerpt (Steam, objdump-style from disasm_va.py)

```
76d5bf: push 0x20 ; call 0x76e610            ; left slot oid -> [ebp-4]   (right: push 0x10 at 76d5b2 -> [ebp-0xc])
76d5d0: cmp [ebp-0xc],0x7f000000 ; jne 76d627 ; cmp [ebp-4],0x7f000000 ; jne 76d5fd
76d5e2: mov byte [ebp-5],0x8                 ; both empty -> style 8
76d5f4: mov byte [eax+0xc7],0x2
76d5fd: ... call 0x73f530 ; test ; je 76d622 ; mov byte [ebp-5],0x0   ; left only -> style 0
76d690: cmp [ebp-4],0x7f000000 ; jne 76d709  ; right only:
76d6b2: cmp dword [ebp-0x78],5 ; ja 76d6e4 ; jmp [ecx*4+0x76d878]     ; switch(R.wield-1)
76d6c2: =1  76d6c8: =2  76d6ce: =3  76d6d4: =5  76d6da: =7  76d6e0: =9 ; (jump table dwords 76d6c2,c8,ce,d4,da,e0)
76d789: cmp byte [ebp-0x7c],2 ; je 76d797 (style 4) ; cmp byte [ebp-0x7c],4 ; je 76d79d (style 6)   ; both: L.wield
76d7cf: mov al,[ebp-5] ; mov [edx+0xc5],al   ; commit style to creature+0xc5
```

Check command: `python3 scripts/disasm_va.py steam 0x0076d590 --len 64` shows the prologue
(`push ebp; mov ebp,esp; sub esp,0x7c; mov [ebp-0x74],ecx; mov [ebp-0x10],0; ...`), and `--len 640` reaches the style stores
at 0x76d6c2..0x76d6e0 and 0x76d797/0x76d79d.

## Diff vs our `styleFor` (Equip3dLogic.h:162)

Rule parity: our code matches the engine on every table row where the engine's own inputs are known.
- both hands: `lWield==2 ? 4 : (lWield==4 ? 6 : 0)` = engine (R ignored). OK.
- right only: map `{0,1,2,3,5,7,9}` for 1..6, else 0 = engine. OK.
- left only: 0 = engine. OK.
- nothing: ours returns 0 (deliberate); engine returns **8** (unarmed g8r1). Intentional deviation, already commented.

Remaining differences (mod-side, not rule errors):
1. Inputs: engine uses equipped slots 0x10/0x20; ours uses `rHas/lHas` = "weapon actually attached". If an equipped weapon fails
   to attach (model missing), ours drops to a different row than the engine would use. Feed it equipped-ness, not attach success,
   if exact parity is wanted.
2. Ours never models creature `+0xc7` (baseitem byte 9) or `+0xd0`; the engine rewrites them in the same function.
3. A weaponwield value outside 1..6 is already handled (0) on both sides.

## Confidence

HIGH for the rule (Steam and GOG both disassembled and agree; Ghidra decompile R7f consistent; jump-table dwords checked by
hand). MEDIUM for the semantic labels "dual melee / dual blaster" (inferred from left wield 2 / 4; the 2da meaning of wield
values was not re-read). Not verified live here.
