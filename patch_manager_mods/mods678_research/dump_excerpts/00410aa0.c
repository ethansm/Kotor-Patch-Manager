// FN 00410aa0 FUN_00410aa0 size=1306 ret=8 callers=007ac470,007b12c0 callees=007116d0,00733570,00840ef0,0084a640,0064ddb0,00736200,00733780,004142e0,00410670

void __thiscall FUN_00410aa0(int param_1,undefined4 param_2,int param_3)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined4 *local_30;
  undefined4 *local_2c;
  int local_28;
  int local_24;
  int local_1c;
  undefined4 *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00948458;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0x68) = param_2;
  if (param_3 != 0) {
    switch(param_2) {
    case 0x33:
      if (param_3 == 1) {
        param_2 = 0x3e;
      }
      else if (param_3 == -1) {
        param_2 = 0x3d;
      }
      break;
    case 0x34:
      if (param_3 == 1) {
        param_2 = 0x40;
      }
      else if (param_3 == -1) {
        param_2 = 0x3f;
      }
      break;
    case 0x37:
      if (param_3 == 1) {
        param_2 = 0x3a;
      }
      else if (param_3 == -1) {
        param_2 = 0x39;
      }
      break;
    case 0x38:
      if (param_3 == 1) {
        param_2 = 0x3c;
      }
      else if (param_3 == -1) {
        param_2 = 0x3b;
      }
    }
    param_3 = 1;
  }
  if (param_3 != 0) {
    local_1c = FUN_00736200(uVar2);
    switch(param_2) {
    case 0x2f:
    case 0x30:
      bVar1 = *(byte *)(param_1 + 0x72);
      if (bVar1 == 0) {
        *(undefined1 *)(param_1 + 0x72) = (undefined1)param_2;
      }
      else if ((0x30 < bVar1) && (bVar1 < 0x33)) {
        if ((uint)(local_1c - *(int *)(param_1 + 100)) < 0x96) {
          ExceptionList = local_10;
          return;
        }
        *(undefined1 *)(param_1 + 0x72) = (undefined1)param_2;
      }
      *(int *)(param_1 + 100) = local_1c;
      break;
    case 0x31:
    case 0x32:
      bVar1 = *(byte *)(param_1 + 0x72);
      if (bVar1 == 0) {
        *(undefined1 *)(param_1 + 0x72) = (undefined1)param_2;
      }
      else if ((0x2e < bVar1) && (bVar1 < 0x31)) {
        if ((uint)(local_1c - *(int *)(param_1 + 100)) < 0x96) {
          ExceptionList = local_10;
          return;
        }
        *(undefined1 *)(param_1 + 0x72) = (undefined1)param_2;
      }
      *(int *)(param_1 + 100) = local_1c;
    }
  }
  switch(param_2) {
  case 0xb4:
  case 0xdf:
    if (param_3 != 0) {
      param_2 = 0x28;
    }
    break;
  case 0xb5:
  case 0xbb:
    param_2 = 0x27;
    break;
  case 0xb6:
    param_2 = 0x3d;
    break;
  case 0xb7:
    param_2 = 0x3e;
    break;
  case 0xb8:
    param_2 = 0x3f;
    break;
  case 0xb9:
    param_2 = 0x40;
  }
  if (*(int *)(param_1 + 0x98) == 0) {
    FUN_0084a640(0);
    local_8 = 0;
    FUN_00840ef0(*(undefined4 *)(param_1 + 0x8c));
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0x8c); local_14 = local_14 + 1) {
      *(undefined4 *)(local_28 + local_14 * 4) =
           *(undefined4 *)(*(int *)(param_1 + 0x88) + local_14 * 4);
    }
    for (local_14 = 0; local_14 < local_24; local_14 = local_14 + 1) {
      iVar3 = FUN_004142e0(*(undefined4 *)(local_28 + local_14 * 4));
      if (iVar3 != -1) {
        (**(code **)(**(int **)(local_28 + local_14 * 4) + 0x3c))(param_2,param_3);
      }
    }
    local_8 = 0xffffffff;
    Free();
  }
  else {
    (**(code **)(**(int **)(*(int *)(param_1 + 0x94) + -4 + *(int *)(param_1 + 0x98) * 4) + 0x3c))
              (param_2,param_3);
  }
  local_14 = *(int *)(param_1 + 0x8c);
  while (local_14 = local_14 + -1, -1 < local_14) {
    local_18 = *(undefined4 **)(*(int *)(param_1 + 0x88) + local_14 * 4);
    if (((uint)local_18[0x12] >> 8 & 3) == 2) {
      FUN_00410670(local_18);
      local_30 = local_18;
      local_2c = local_18;
      if (local_18 != (undefined4 *)0x0) {
        (**(code **)*local_18)(1);
      }
    }
    else if (((uint)local_18[0x12] >> 8 & 3) != 0) {
      local_18[0x12] = local_18[0x12] & 0xffffff7f;
      local_18[0x12] = local_18[0x12] & 0xfffffcff;
      FUN_00410670(local_18);
    }
  }
  if ((*(byte *)(DAT_00a1b490 + 0x34) & 2) != 0) {
    FUN_00733570("RIMS:MAINMENU");
    local_8 = 1;
    FUN_007116d0(local_38);
    local_8 = 0xffffffff;
    FUN_00733780();
    *(byte *)(DAT_00a1b490 + 0x34) = *(byte *)(DAT_00a1b490 + 0x34) & 0xfd;
  }
  if ((*(byte *)(DAT_00a1b490 + 0x34) & 1) != 0) {
    FUN_00733570("RIMS:CHARGEN");
    local_8 = 2;
    FUN_007116d0(local_40);
    local_8 = 0xffffffff;
    FUN_00733780();
    *(byte *)(DAT_00a1b490 + 0x34) = *(byte *)(DAT_00a1b490 + 0x34) & 0xfe;
  }
  ExceptionList = local_10;
  return;
}

