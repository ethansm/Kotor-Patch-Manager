# ISF controller filter: bumper path into the mode-setter (0x008A8B30)

Binary: swkotor2.exe, sha256 6a522e71...ffef, ghidra 12.1.4, x86 PE (open_binary, path `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe`).
REA calls: 3 on 0x008A8B30 (procedure_callers, procedure_pseudo_code, xrefs); 3 on 0x008A8880 (procedure_pseudo_code, procedure_assembly, read_bytes 0x8A8994 len 108).
Static table: `patch_manager_mods/ghidra_knowledge/steam_functions.tsv` rows 10275-10276 give size 398 and RET 4 for 0x008A8B30. static_facts.py not run (no shell in this agent).
Ledger: `patch_manager_mods/ghidra_knowledge/rea_findings/20261027_isf-controller-filter.tsv` (11 rows).

## 1. Callers and bumper handler
- procedure_callers 0x008A8B30 returns one direct caller: `0x8a8880` (FUN_008A8880).
- xrefs 0x008A8B30 returns two call sites, both inside 0x008A8880: `0x008A899B` and `0x008A89FB`. No data or vtable references are listed.
- The handler is FUN_008A8880. It switches on input code `[EBP+8]-0x28` (range check `<= 0xB7`) and returns early unless the press flag `[EBP+0xC] != 0` (`0x8a8889`).
- Prev-filter case: input code 0x2F, block 0x008A8949, CALL at 0x008A899B.
- Next-filter case: input code 0x30, block 0x008A89A5, CALL at 0x008A89FB.
- The block-to-code mapping comes from the decompiler switch. The jump-table bytes (0x008A8A50, 0x008A8A6C) were not dumped.
- Listing of 0x008A8880 has no CALL or JMP to 0x008A8CF0 (H6a), so this path bypasses it.

## 2. Mode computation and SetNextFilter convention
Mode step is inline in the caller. No table, and the mode-setter does not step the mode.
- Next (0x008A89A5 block): if cur == 0 then 6; else cur+1, and if > 11 then 0.
- Prev (0x008A8949 block): if cur == 6 then 0; else cur-1, and if < 0 then 11.
- Current mode is read from `[FUN_0073F750(ECX=[[0x00A1B4A4]+4])+0xFC1]` (0x008A8955..0x008A8963).

Mode-setter 0x008A8B30:
- Convention: thiscall. ECX = panel (this). One stack arg `int mode` (absolute 0..11). Callee pops 4, since both call sites are followed directly by `JMP` with no `ADD ESP` (RET 4 from steam_functions.tsv).
- Body: if `0 <= mode < 12`, store mode byte at `[FUN_0073F750()+0xFC1]`, call FUN_00417140 on several globals, then set `[panel+0x24D0] |= 4`. Otherwise it is a no-op.
- The mode is NOT stored on the panel object. The only panel field it touches is the flag word at +0x24D0 (bit 0x4), which the mod's comment describes as a deferred repopulate.

## 3. Hookable windows (replace the CALL rel32; 5 bytes each)
| Site | CALL addr | Original bytes (read_bytes) | Pre-call bytes | Live at CALL |
|---|---|---|---|---|
| A, prev (0x2F) | 0x008A899B | `E8 90 01 00 00` | 0x8A8997 `51` PUSH ECX (mode); 0x8A8998 `8B 4D D4` MOV ECX,[EBP-0x2C] | ECX=panel; [ESP+4]=mode; return 0x008A89A0 |
| B, next (0x30) | 0x008A89FB | `E8 30 01 00 00` | 0x8A89F7 `52` PUSH EDX (mode); 0x8A89F8 `8B 4D D4` MOV ECX,[EBP-0x2C] | ECX=panel; [ESP+4]=mode; return 0x008A8A00 |

- Patch: replace the 5-byte CALL with `E8 <hook - next_ip>`. Site A next_ip = 0x008A89A0, site B next_ip = 0x008A8A00. Original rel32 values are 0x190 and 0x130 (both target 0x008A8B30).
- The hook must `RET 4`. If it calls the original mode-setter, push mode, set ECX = panel, and CALL 0x008A8B30.
- Direction is fixed per site (A = prev, B = next). Panel is `[EBP-0x2C]`. The computed mode is `[EBP-4]`.

## 4. Shared call site?
No. Left and right do not share a site. Each input code (0x2F, 0x30) has its own CALL inside FUN_008A8880. Which physical bumper is 0x2F vs 0x30 is not determined here.

## Pseudo-C (ghidra decompile, condensed)
```c
// FUN_008A8880: input dispatcher (ECX = panel, param_1 = input code, param_2 = pressed)
if (param_2 != 0) {
  switch (param_1) {
  case 0x2f:  // prev filter (block 0x008A8949, CALL 0x008A899B)
    iVar1 = FUN_0073F750();                      // ECX = [[0x00A1B4A4]+4]
    if (*(byte *)(iVar1 + 0xfc1) == 6) local_8 = 0;
    else { local_8 = *(byte *)(iVar1 + 0xfc1) - 1;
           if ((int)local_8 < 0) local_8 = 0xb; }
    FUN_008A8B30(local_8);                       // thiscall, ECX = panel
    break;
  case 0x30:  // next filter (block 0x008A89A5, CALL 0x008A89FB)
    iVar1 = FUN_0073F750();
    if (*(byte *)(iVar1 + 0xfc1) == 0) local_8 = 6;
    else { local_8 = *(byte *)(iVar1 + 0xfc1) + 1;
           if (0xb < local_8) local_8 = 0; }
    FUN_008A8B30(local_8);
    break;
  /* 0x39, 0x3a: vtable+0x3c on panel+0x878 with 0x31/0x32 */
  }
}
FUN_0040EAA0(param_1, param_2);                  // forwarded tail call

// FUN_008A8B30: mode-setter (ECX = panel, param_1 = mode)
void FUN_008A8B30(int param_1) {
  if ((param_1 < 0xc) && (-1 < param_1)) {
    iVar1 = FUN_0073F750();
    *(undefined1 *)(iVar1 + 0xfc1) = (undefined1)param_1;   // mode store (not panel)
    /* 7x FUN_00417140(&DAT_00a7fe24); switch(mode) FUN_00417140(&DAT_00a7fdac) for 0,6..11 */
    *(uint *)(in_ECX + 0x24d0) |= 4;                         // panel deferred-repopulate flag
  }
}
```

## OPEN
- Which physical bumper maps to input code 0x2F (prev) vs 0x30 (next). REA has no controller map, and the mod source has none.
- Jump-table mapping of input codes to case blocks. Decompiler used; tables not dumped.
- The ECX passed to 0x0073F750 inside 0x008A8B30 is not shown. The asm of 0x008A8B30 was not fetched. If the getter depends on ECX, the write target may differ from the reader's object. Recommended hook: re-point the CALL to the original 0x008A8B30 after editing the mode argument, so the engine does its own store.
- Mode values: `patch_manager_mods/inventory_sort_filter_src/SortLogic.h:90` sets `kEngineModeNew = 1`. The engine bumper ring never yields 1..5 (ring is {0,6..11}). Brief's "mode 1 = New" is not verified.
- Callees of 0x008A8B30 not fetched.
- H6a 0x008A8CF0 not analysed beyond the absence of a reference from the bumper path.
