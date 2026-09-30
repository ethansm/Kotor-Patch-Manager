// FN 00743540 FUN_00743540 size=782 ret=12 callers=0074cfe0 callees=00742a00,0077df90,007418a0,007ecfa0,00741660,0077e370,00743850

void __thiscall FUN_00743540(int param_1,undefined4 param_2,int param_3,undefined4 param_4)

{
  bool bVar1;
  undefined1 uVar2;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_14;
  undefined4 local_8;
  
  uVar2 = FUN_0077df90(param_4);
  *(undefined1 *)(param_1 + 0x1baa) = uVar2;
  for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
    for (local_14 = 0; local_14 < *(int *)(param_1 + 4 + local_8 * 0xc); local_14 = local_14 + 1) {
      FUN_007ecfa0();
    }
    *(undefined4 *)(param_1 + 4 + local_8 * 0xc) = 0;
    FUN_0077e370(param_4,local_8,local_8 * 0xc + param_1);
  }
  local_8 = 0;
  do {
    if (2 < local_8) {
      FUN_00742a00(param_4);
      FUN_00743850(param_2,param_4);
      return;
    }
    bVar1 = false;
    for (local_14 = 0; local_14 < *(int *)(param_1 + 4 + local_8 * 0xc); local_14 = local_14 + 1) {
      if (*(int *)(*(int *)(param_1 + local_8 * 0xc) + 8 + local_14 * 0x3c) ==
          *(int *)(param_1 + 0x24 + *(char *)(param_1 + 0x1baa) * 0xc + local_8 * 4)) {
        FUN_00741660(0,1 < *(int *)(param_1 + 4 + local_8 * 0xc));
        if (((*(uint *)(*(int *)(param_1 + local_8 * 0xc) + 0x30 + local_14 * 0x3c) & 1) == 0) ||
           (param_3 != 0)) {
          local_38 = 0;
        }
        else {
          local_38 = 1;
        }
        FUN_007418a0(*(int *)(param_1 + local_8 * 0xc) + 0x20 + local_14 * 0x3c,
                     local_14 * 0x3c + *(int *)(param_1 + local_8 * 0xc),local_38,0);
        bVar1 = true;
        break;
      }
    }
    if (!bVar1) {
      if (*(int *)(param_1 + 4 + local_8 * 0xc) < 1) {
        FUN_00741660(1,0);
      }
      else {
        FUN_00741660(0,1 < *(int *)(param_1 + 4 + local_8 * 0xc));
        if (((*(uint *)(*(int *)(param_1 + local_8 * 0xc) + 0x30) & 1) == 0) || (param_3 != 0)) {
          local_3c = 0;
        }
        else {
          local_3c = 1;
        }
        FUN_007418a0(*(int *)(param_1 + local_8 * 0xc) + 0x20,
                     *(undefined4 *)(param_1 + local_8 * 0xc),local_3c,0);
      }
    }
    local_8 = local_8 + 1;
  } while( true );
}

