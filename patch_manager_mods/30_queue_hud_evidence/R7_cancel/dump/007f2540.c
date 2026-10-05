// FN 007f2540 FUN_007f2540 size=208 ret=0 callers=0087f360,0077cf70,00742a00,00752760,00842d00,0077d800,0077e370,0087be80,007fee70,0074fb20,00798d70 callees=0051c8b0,0051c0a0,0054a8e0

undefined4 __fastcall FUN_007f2540(int param_1)

{
  undefined4 uVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  if (*(int *)(param_1 + 0xf8) == 0) {
    if (*(int *)(param_1 + 0xe4) != 0) {
      return 0;
    }
    if (*(int *)(DAT_00a1b4a4 + 8) == 0) {
      return 0;
    }
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    piVar2 = (int *)FUN_0051c0a0(uVar1);
    if (piVar2 != (int *)0x0) {
      uVar3 = (**(code **)(*piVar2 + 0x10))(param_1,uVar4,uVar3);
      *(undefined4 *)(param_1 + 0xf8) = uVar3;
      if (*(int *)(param_1 + 0xf8) != 0) {
        FUN_0054a8e0(param_1);
      }
    }
  }
  return *(undefined4 *)(param_1 + 0xf8);
}

