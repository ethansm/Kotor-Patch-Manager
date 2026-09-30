// FN 008a8100 FUN_008a8100 size=703 ret=4 callers= callees=007feec0,0073fb90,00416e30,00919f10,0051c0e0,0071ebd0,0041d290,0041e9a0,00607790,00733780,00733570,007e5d80,00734190,005210a0,004178b0,0041fd90

void __thiscall FUN_008a8100(int param_1,int param_2)

{
  int iVar1;
  undefined1 uVar2;
  uint uVar3;
  size_t local_68;
  undefined1 local_4c [8];
  undefined4 local_44;
  int local_40;
  int local_3c;
  undefined1 local_38 [8];
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00978140;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_2 + 0x50) != 0) {
    local_28 = param_2;
    local_24 = *(int *)(param_2 + 0x1d0);
    if (local_24 == 0x7f000000) {
      *(uint *)(param_1 + 0x24d0) = *(uint *)(param_1 + 0x24d0) & 0xfffffffd;
      FUN_00733570("Error: Invalid item");
      local_8 = 1;
      FUN_00416e30(local_4c);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    else {
      local_30 = FUN_0051c0e0(local_24);
      FUN_00607790(local_38);
      local_8 = 0;
      local_40 = FUN_00734190(0x3c,0);
      local_3c = FUN_00734190(0x7b,0);
      if ((local_40 != -1) || (local_3c != -1)) {
        FUN_0073fb90(uVar3);
        local_44 = FUN_007e5d80();
        iVar1 = DAT_00a1b4b4;
        *(undefined4 *)(DAT_00a1b4b4 + 0x58) = local_44;
        uVar2 = FUN_007feec0();
        *(undefined1 *)(iVar1 + 0x3c) = uVar2;
        *(uint *)(iVar1 + 0x5c) = *(uint *)(iVar1 + 0x5c) & 0xfffffffe | 1;
        FUN_0071ebd0(local_38);
      }
      FUN_00416e30(local_38);
      *(uint *)(param_1 + 0x24d0) =
           *(uint *)(param_1 + 0x24d0) & 0xfffffffd | (*(uint *)(local_28 + 0x5c) & 1) << 1;
      if ((*(uint *)(local_30 + 0x2c8) >> 7 & 1) != 0) {
        FUN_005210a0(local_24);
      }
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    FUN_0041d290();
    local_20 = *(undefined4 *)(param_1 + 0x6c);
    local_1c = *(undefined4 *)(param_1 + 0x70);
    local_18 = *(undefined4 *)(param_1 + 0x74);
    local_14 = *(undefined4 *)(param_1 + 0x78);
    if (*(int *)(param_1 + 0x158) == 0) {
      local_68 = 0;
    }
    else {
      local_68 = _strlen(*(char **)(param_1 + 0x158));
    }
    if ((int)local_68 < 1) {
      local_14 = FUN_004178b0();
    }
    else {
      local_14 = (**(code **)(*(int *)(param_1 + 0x140) + 8))();
    }
    (**(code **)(*(int *)(param_1 + 0x68) + 4))(&local_20);
    local_2c = param_1 + 0x68;
    FUN_0041fd90(&local_2c,1,0,0,0);
    FUN_0041e9a0(param_1 + 0x68,0);
  }
  ExceptionList = local_10;
  return;
}

