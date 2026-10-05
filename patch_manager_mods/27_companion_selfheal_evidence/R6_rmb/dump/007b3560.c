// FN 007b3560 FUN_007b3560 size=283 ret=4 callers=007b12c0 callees=0072fb00,0072fc30,00413a80,004131e0

void __thiscall FUN_007b3560(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_8;
  
  DAT_00a7fcc0 = 1;
  if (DAT_00a1b48c != 0) {
    FUN_0072fc30(1);
  }
  if (*(int *)(param_1 + 0x274) != 0) {
    FUN_00413a80(*(undefined4 *)(*(int *)(param_1 + 0x274) + 0x38),
                 *(uint *)(*(int *)(param_1 + 0x274) + 0x1c) >> 2 & 1,1);
  }
  uVar1 = DAT_00a7fcbc;
  local_8 = DAT_00a7fcbc;
  iVar2 = FUN_0072fb00();
  if ((((*(uint *)(iVar2 + 8) >> 1 & 1) != 0) && (*(int *)(param_1 + 0x9c) != 2)) &&
     (*(int *)(param_1 + 0x9c) != 3)) {
    local_8 = (uint)(uVar1 == 0);
  }
  if (((local_8 == 0) && (*(int *)(param_1 + 0x9c) != 4)) ||
     ((*(int *)(param_1 + 0x9c) == 2 || (*(int *)(param_1 + 0x9c) == 3)))) {
    if (param_2 == 0) {
      *(undefined4 *)(param_1 + 0x2b8) = *(undefined4 *)(param_1 + 0x2b4);
    }
    FUN_004131e0(param_2);
  }
  return;
}

