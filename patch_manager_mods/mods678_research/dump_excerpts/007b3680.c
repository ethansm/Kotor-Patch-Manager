// FN 007b3680 FUN_007b3680 size=384 ret=0 callers=007b12c0 callees=0072fb00,007b3ca0,004132e0,007449f0,0072fc30,00413a80

void __fastcall FUN_007b3680(int param_1)

{
  uint uVar1;
  char cVar2;
  int iVar3;
  uint local_8;
  
  DAT_00a7fcc0 = 0;
  if (DAT_00a1b48c != 0) {
    FUN_0072fc30(0);
  }
  if (*(int *)(param_1 + 0x274) != 0) {
    FUN_00413a80(*(undefined4 *)(*(int *)(param_1 + 0x274) + 0x38),
                 *(uint *)(*(int *)(param_1 + 0x274) + 0x1c) >> 2 & 1,0);
  }
  uVar1 = DAT_00a7fcbc;
  local_8 = DAT_00a7fcbc;
  iVar3 = FUN_0072fb00();
  if ((((*(uint *)(iVar3 + 8) >> 1 & 1) != 0) && (*(int *)(param_1 + 0x9c) != 2)) &&
     (*(int *)(param_1 + 0x9c) != 3)) {
    local_8 = (uint)(uVar1 == 0);
  }
  if (((local_8 == 0) && (*(int *)(param_1 + 0x9c) != 4)) ||
     ((*(int *)(param_1 + 0x9c) == 2 || (*(int *)(param_1 + 0x9c) == 3)))) {
    if ((*(int *)(*(int *)(param_1 + 0x274) + 8) == 0) ||
       (cVar2 = FUN_007449f0(*(undefined4 *)(*(int *)(param_1 + 0x274) + 8)), cVar2 != '\x01')) {
      iVar3 = FUN_004132e0();
      if ((iVar3 == 0) && (*(int *)(DAT_00a1b4a4 + 8) != 0)) {
        FUN_007b3ca0();
      }
    }
    else if (*(int *)(DAT_00a1b4a4 + 8) != 0) {
      FUN_007b3ca0();
    }
  }
  return;
}

