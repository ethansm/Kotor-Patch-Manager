// FN 0079d6e0 FUN_0079d6e0 size=133 ret=4 callers=00740880 callees=007f7220,007f6fb0,007ceb50,00726f80

void __thiscall FUN_0079d6e0(int param_1,int param_2)

{
  int iVar1;
  
  if (param_2 == 0) {
    *(undefined4 *)(param_1 + 0x3e0) = 0;
  }
  if (*(int *)(param_1 + 800) != param_2) {
    *(int *)(param_1 + 800) = param_2;
    FUN_007ceb50(param_1,*(undefined4 *)(param_1 + 0x40));
    iVar1 = FUN_00726f80();
    if (iVar1 != 0) {
      if (*(int *)(param_1 + 800) == 0) {
        FUN_00726f80();
        FUN_007f7220();
      }
      else {
        FUN_00726f80();
        FUN_007f6fb0();
      }
    }
  }
  return;
}

