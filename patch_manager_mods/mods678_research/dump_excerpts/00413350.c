// FN 00413350 FUN_00413350 size=104 ret=0 callers=007b3810 callees=00413fe0,00413f00,00413c50

undefined4 __fastcall FUN_00413350(int param_1)

{
  undefined4 uVar1;
  
  if ((*(uint *)(param_1 + 0x1c) >> 3 & 1) != 0) {
    FUN_00413f00();
  }
  if (*(int *)(param_1 + 0x10) == 0) {
    FUN_00413c50();
    FUN_00413fe0(1);
    if (*(int *)(param_1 + 8) == 0) {
      uVar1 = 0;
    }
    else {
      (**(code **)(**(int **)(param_1 + 8) + 0x84))();
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

