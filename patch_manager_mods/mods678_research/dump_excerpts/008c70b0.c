// FN 008c70b0 FUN_008c70b0 size=582 ret=0 callers= callees=00733780,0071c3d0,007335f0,007337c0,0051c0e0,00919723,00710840,006076d0,008c7510,0040f3c0,0071ad40,00410530

void FUN_008c70b0(void)

{
  byte bVar1;
  undefined1 auStack_88 [4];
  undefined4 uStack_84;
  undefined4 uStack_80;
  undefined4 uStack_7c;
  undefined4 *puStack_78;
  uint uStack_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 *local_68;
  int local_64;
  int local_60;
  undefined4 local_5c;
  int local_58;
  int local_54;
  int local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined1 local_40 [8];
  undefined4 *local_38;
  undefined4 *local_34;
  undefined1 *local_2c;
  int local_28;
  undefined4 *local_24;
  uint local_20;
  int local_1c;
  int local_18;
  undefined4 *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097ac83;
  local_10 = ExceptionList;
  uStack_74 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  puStack_78 = (undefined4 *)0x54;
  uStack_7c = 0x8c70df;
  local_28 = FUN_00919723();
  local_8 = 0;
  if (local_28 == 0) {
    local_68 = (undefined4 *)0x0;
  }
  else {
    puStack_78 = (undefined4 *)0x0;
    local_2c = auStack_88;
    FUN_00710840("upgrade");
    local_68 = (undefined4 *)FUN_0071ad40();
  }
  local_24 = local_68;
  local_8 = 0xffffffff;
  local_14 = local_68;
  puStack_78 = (undefined4 *)0x8c7135;
  FUN_0071c3d0();
  local_18 = 1;
  puStack_78 = (undefined4 *)(local_64 + 0x18d0);
  uStack_7c = 0x8c714e;
  local_6c = FUN_007335f0();
  local_44 = *(undefined4 *)(local_64 + 0x868);
  local_8 = 1;
  puStack_78 = (undefined4 *)local_40;
  uStack_7c = 0x8c7176;
  FUN_007337c0();
  local_8 = 0xffffffff;
  puStack_78 = (undefined4 *)0x8c7185;
  FUN_00733780();
  if (*(int *)(local_64 + 0x18cc) != 0) {
    local_48 = *(undefined4 *)(local_64 + 0x1c);
    puStack_78 = (undefined4 *)0x1;
    uStack_7c = 3;
    uStack_80 = *(undefined4 *)(local_64 + 0x868);
    uStack_84 = 0x8c71b0;
    FUN_00410530();
  }
  if (*(int *)(local_64 + 0x18a4) != 0x7f000000) {
    local_4c = *(undefined4 *)(DAT_00a1b4a4 + 8);
    puStack_78 = *(undefined4 **)(local_64 + 0x18a4);
    uStack_7c = 0x8c71e0;
    local_1c = FUN_0051c0e0();
    if (local_1c != 0) {
      local_18 = 0;
      puStack_78 = local_14;
      uStack_7c = 0x8c7200;
      bVar1 = FUN_006076d0();
      local_20 = (uint)bVar1;
      local_50 = *(int *)(local_64 + 0x18c0);
      *(byte *)(local_50 + 0xc78) = bVar1;
      local_54 = *(int *)(local_64 + 0x18c0);
      *(int *)(local_54 + 0xc8c) = local_1c;
      local_58 = *(int *)(local_64 + 0x18c0);
      *(undefined4 *)(local_58 + 0xc84) = 1;
      local_5c = *(undefined4 *)(local_64 + 0x1c);
      puStack_78 = (undefined4 *)0x1;
      uStack_7c = 3;
      uStack_80 = *(undefined4 *)(local_64 + 0x18c0);
      uStack_84 = 0x8c726e;
      FUN_00410530();
    }
  }
  if (local_18 != 0) {
    puStack_78 = (undefined4 *)0x8c727c;
    FUN_008c7510();
  }
  local_38 = local_14;
  local_34 = local_14;
  if (local_14 == (undefined4 *)0x0) {
    local_70 = 0;
  }
  else {
    puStack_78 = (undefined4 *)0x1;
    uStack_7c = 0x8c729c;
    local_70 = (**(code **)*local_14)();
  }
  local_14 = (undefined4 *)0x0;
  if (*(int *)(local_64 + 0x18c8) != 0) {
    local_60 = local_64 + 0x698;
    *(uint *)(local_64 + 0x6e0) = *(uint *)(local_64 + 0x6e0) & 0xfffffffd;
  }
  puStack_78 = (undefined4 *)0x8c72e7;
  FUN_0040f3c0();
  ExceptionList = local_10;
  return;
}

