// FN 00603f60 FUN_00603f60 size=475 ret=8 callers=005d3650,0060ea50,006adec0,005d3c90,0060fee0,006bb3a0,0058a610,00610fc0,005d32c0,005d3b60,0060b6b0,0060a0d0,0060a990,00609110,0060e0d0,005d3480,00609cb0,005d3ee0,005ad9b0,00600f20,005d3a20,006c06b0,0060da50,0060a4f0,006c0330,0060e6b0 callees=00604260,006042a0

undefined4 __thiscall FUN_00603f60(int param_1,short param_2,short param_3)

{
  bool bVar1;
  int iVar2;
  short *psVar3;
  int local_8;
  
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x288); local_8 = local_8 + 1) {
    iVar2 = FUN_00604260(local_8);
    if ((*(byte *)(iVar2 + 0x11) == 0xff) ||
       ((1 << (*(byte *)(iVar2 + 0x11) & 0x1f) & *(uint *)(param_1 + 0x2d4)) != 0)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if ((bVar1) && (psVar3 = (short *)FUN_00604260(local_8), *psVar3 == param_2)) {
      if (param_3 == 0) {
        return 1;
      }
      iVar2 = FUN_00604260(local_8);
      if (*(short *)(iVar2 + 2) == param_3) {
        return 1;
      }
    }
  }
  local_8 = 0;
  do {
    if (*(int *)(param_1 + 0x28c) <= local_8) {
      return 0;
    }
    iVar2 = FUN_006042a0(local_8);
    if ((*(byte *)(iVar2 + 0x11) == 0xff) ||
       ((1 << (*(byte *)(iVar2 + 0x11) & 0x1f) & *(uint *)(param_1 + 0x2d4)) != 0)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if ((bVar1) && (psVar3 = (short *)FUN_006042a0(local_8), *psVar3 == param_2)) {
      if (param_3 == 0) {
        return 1;
      }
      iVar2 = FUN_006042a0(local_8);
      if (*(short *)(iVar2 + 2) == param_3) {
        return 1;
      }
    }
    local_8 = local_8 + 1;
  } while( true );
}

