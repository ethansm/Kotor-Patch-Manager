// FN 005d32c0 FUN_005d32c0 size=437 ret=4 callers=005d2f80,005bd6a0 callees=006b83f0,005d4240,006042a0,00603f60

undefined4 __thiscall FUN_005d32c0(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  short *psVar4;
  byte local_3a;
  uint local_10;
  int local_8;
  
  iVar2 = FUN_00603f60(0x2c,0);
  if (iVar2 == 0) {
    uVar3 = 1;
  }
  else {
    iVar2 = FUN_005d4240(param_2,0);
    if (iVar2 == 1) {
      uVar3 = 1;
    }
    else {
      iVar2 = FUN_006b83f0(0x5d);
      if (iVar2 == 0) {
        for (local_8 = 0; local_8 < *(int *)(param_2 + 0x28c); local_8 = local_8 + 1) {
          iVar2 = FUN_006042a0(local_8);
          if ((*(byte *)(iVar2 + 0x11) == 0xff) ||
             ((1 << (*(byte *)(iVar2 + 0x11) & 0x1f) & *(uint *)(param_2 + 0x2d4)) != 0)) {
            bVar1 = true;
          }
          else {
            bVar1 = false;
          }
          if ((bVar1) && (psVar4 = (short *)FUN_006042a0(local_8), *psVar4 == 0x2c)) {
            iVar2 = FUN_006042a0(local_8);
            for (local_10 = 0;
                (int)local_10 < (int)(uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0x8d);
                local_10 = local_10 + 1) {
              if ((local_10 & 0xff) < (uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0x8d)) {
                local_3a = *(byte *)(*(int *)(param_1 + 0x1198) + 0xab + (local_10 & 0xff) * 0x28);
              }
              else {
                local_3a = 0xff;
              }
              if (*(ushort *)(iVar2 + 2) == (ushort)local_3a) {
                return 1;
              }
            }
          }
        }
        uVar3 = 0;
      }
      else {
        uVar3 = 1;
      }
    }
  }
  return uVar3;
}

