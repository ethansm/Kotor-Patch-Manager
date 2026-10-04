// FN 007b3ca0 FUN_007b3ca0 size=568 ret=0 callers=007b3680 callees=004122a0,0073fb90,0074cfe0,007e5d80,0073f4d0,007a0530,0073f750

void __fastcall FUN_007b3ca0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  int local_78;
  int local_8;
  
  iVar1 = FUN_0073f750();
  if (((*(int *)(iVar1 + 0xcc) == 0) && (*(int *)(*(int *)(param_1 + 0x40) + 0x30) == 0)) &&
     (*(char *)(*(int *)(param_1 + 4) + 0x75) != '\x05')) {
    if (((*(int *)(param_1 + 0x2b8) == 0x7f000000) ||
        (*(int *)(param_1 + 0x2b4) != *(int *)(param_1 + 0x4b0))) ||
       (*(int *)(param_1 + 0x2b8) != *(int *)(param_1 + 0x2b4))) {
      if (*(int *)(param_1 + 0x4b0) != 0x7f000000) {
        FUN_007a0530(*(undefined4 *)(param_1 + 0x4b0),0);
      }
      iVar1 = FUN_0073f750();
      if (*(int *)(iVar1 + 0x98) != 0) {
        FUN_0073f750();
        FUN_0074cfe0(1);
      }
    }
    else {
      *(undefined4 *)(param_1 + 0x2b8) = 0x7f000000;
      local_8 = 0;
      if (0 < *(int *)(param_1 + 0x4d0)) {
        local_8 = *(int *)(param_1 + 0x4cc);
      }
      if (local_8 != 0) {
        FUN_0073fb90();
        uVar2 = FUN_007e5d80();
        piVar3 = (int *)FUN_0073f4d0(*(undefined4 *)(local_8 + 0x1c));
        if (piVar3 == (int *)0x0) {
          local_78 = 0;
        }
        else {
          local_78 = (**(code **)(*piVar3 + 0xc))();
        }
        if (local_78 != 0) {
          FUN_004122a0(6);
          (**(code **)(local_8 + 0xc))(*(undefined4 *)(local_8 + 8),uVar2);
        }
      }
    }
  }
  return;
}

