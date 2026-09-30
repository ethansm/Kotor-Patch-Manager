// FN 0074da40 FUN_0074da40 size=535 ret=0 callers=0074cfe0,007ca060,00751b00 callees=00733570,00654860,0073fb90,006b83f0,0051c890,0074cfe0,007e5d80,0077d800,00733780

void __fastcall FUN_0074da40(int param_1)

{
  int iVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  int local_50;
  undefined1 local_34 [8];
  uint local_2c;
  int local_28;
  undefined1 local_21;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00963178;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_1c = -1;
  local_20 = 0xffffffff;
  local_14 = 0xffffffff;
  FUN_0073fb90(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_28 = FUN_007e5d80();
  if (local_28 == 0) {
    local_50 = 0;
  }
  else {
    local_50 = FUN_0077d800();
  }
  local_18 = local_50;
  if ((local_28 != 0) && (local_50 != 0)) {
    local_21 = 0;
    iVar1 = FUN_006b83f0(0xcb);
    if (iVar1 != 0) {
      FUN_00733570("CANDEROUS_CUR_IMPLANT");
      local_8 = 0;
      puVar3 = &local_21;
      puVar2 = local_34;
      FUN_0051c890(puVar2,puVar3);
      FUN_00654860(puVar2,puVar3);
      local_8 = 0xffffffff;
      FUN_00733780();
      switch(local_21) {
      case 1:
        local_1c = 0x408;
        break;
      case 2:
        local_1c = 0x409;
        break;
      case 3:
        local_1c = 0x40a;
        break;
      case 4:
        local_1c = 0x40b;
      }
    }
    local_2c = (uint)*(ushort *)(*(int *)(local_18 + 0x1198) + 0x146);
    switch(local_2c) {
    case 9:
      local_20 = 0x410;
      break;
    case 10:
      local_20 = 0x411;
      break;
    case 0xb:
      local_20 = 0x412;
      break;
    case 0xc:
      local_20 = 0x413;
      break;
    case 0xd:
      local_20 = 0x414;
      break;
    default:
      local_20 = -1;
    }
    local_14 = *(uint *)(local_18 + 0x1110);
    if (0 < (int)local_14) {
      local_14 = local_14 & 0xdfffffff;
    }
    if (0 < local_1c) {
      *(int *)(param_1 + 0x1c7c) = local_1c;
    }
    if (0 < local_20) {
      *(int *)(param_1 + 0x1c8c) = local_20;
    }
    if (0 < (int)local_14) {
      *(uint *)(param_1 + 0x1c90) = local_14;
    }
    FUN_0074cfe0(0);
  }
  ExceptionList = local_10;
  return;
}

