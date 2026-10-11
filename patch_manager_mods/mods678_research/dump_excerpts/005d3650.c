// FN 005d3650 FUN_005d3650 size=970 ret=8 callers=005d2f80,005bd6a0 callees=006042e0,006d6e30,006042a0,00603f60

undefined4 __thiscall FUN_005d3650(int param_1,int param_2,int param_3)

{
  byte bVar1;
  short sVar2;
  bool bVar3;
  int iVar4;
  undefined4 uVar5;
  short *psVar6;
  int local_1c;
  int local_18;
  int local_14;
  uint local_10;
  uint local_c;
  char local_5;
  
  local_14 = 1;
  iVar4 = FUN_00603f60(10,0);
  if (iVar4 != 0) {
    local_18 = 0;
    FUN_006042e0(&local_18,10,0x81);
    if (local_18 != 0) {
      local_14 = 0;
    }
  }
  iVar4 = FUN_006d6e30();
  local_5 = *(char *)(iVar4 + 0xb4);
  if ((local_5 != '\0') && (local_14 != 0)) {
    if (local_5 == '\x01') {
      if (*(short *)(*(int *)(param_1 + 0x1198) + 0xe0) != DAT_0099a8bc) {
        if (param_3 != 0) {
          return 0;
        }
        return 1;
      }
    }
    else {
      if (local_5 != '\x02') {
        return 0;
      }
      if (*(short *)(*(int *)(param_1 + 0x1198) + 0xe0) != DAT_0099a8b8) {
        if (param_3 != 0) {
          return 0;
        }
        return 1;
      }
    }
  }
  local_10 = 1 << (*(byte *)(*(int *)(param_1 + 0x1198) + 0xec) & 0x1f);
  iVar4 = FUN_006d6e30();
  local_c = *(uint *)(iVar4 + 0xb8);
  if ((local_c & local_10) == 0) {
    iVar4 = FUN_00603f60(0x2d,0);
    if ((iVar4 == 0) && (iVar4 = FUN_00603f60(0x3d,0), iVar4 == 0)) {
      uVar5 = 1;
    }
    else {
      for (local_1c = 0; local_1c < *(int *)(param_2 + 0x28c); local_1c = local_1c + 1) {
        iVar4 = FUN_006042a0(local_1c);
        if ((*(byte *)(iVar4 + 0x11) == 0xff) ||
           ((1 << (*(byte *)(iVar4 + 0x11) & 0x1f) & *(uint *)(param_2 + 0x2d4)) != 0)) {
          bVar3 = true;
        }
        else {
          bVar3 = false;
        }
        if ((bVar3) && (psVar6 = (short *)FUN_006042a0(local_1c), *psVar6 == 0x2d)) {
          FUN_006042a0(local_1c);
          sVar2 = *(short *)(*(int *)(param_1 + 0x1198) + 0xe0);
          iVar4 = FUN_006042a0(local_1c);
          if (sVar2 == *(short *)(iVar4 + 2)) {
            return 1;
          }
        }
        iVar4 = FUN_006042a0(local_1c);
        if ((*(byte *)(iVar4 + 0x11) == 0xff) ||
           ((1 << (*(byte *)(iVar4 + 0x11) & 0x1f) & *(uint *)(param_2 + 0x2d4)) != 0)) {
          bVar3 = true;
        }
        else {
          bVar3 = false;
        }
        if ((((bVar3) && (psVar6 = (short *)FUN_006042a0(local_1c), *psVar6 == 0x3d)) &&
            (FUN_006042a0(local_1c), *(short *)(*(int *)(param_1 + 0x1198) + 0xe0) != DAT_0099a8b8))
           && (bVar1 = *(byte *)(*(int *)(param_1 + 0x1198) + 0xec), iVar4 = FUN_006042a0(local_1c),
              (ushort)bVar1 == *(ushort *)(iVar4 + 2))) {
          return 1;
        }
      }
      uVar5 = 0;
    }
  }
  else {
    uVar5 = 0;
  }
  return uVar5;
}

