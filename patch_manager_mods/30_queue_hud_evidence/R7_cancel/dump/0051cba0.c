// FN 0051cba0 FUN_0051cba0 size=56 ret=4 callers=007f2500 callees=00590430,0051c100

void FUN_0051cba0(undefined4 param_1)

{
  int iVar1;
  undefined4 extraout_var;
  
  iVar1 = FUN_0051c100(param_1);
  if (iVar1 != 0) {
    FUN_00590430(extraout_var,*(undefined4 *)(iVar1 + 0x10dc));
  }
  return;
}

