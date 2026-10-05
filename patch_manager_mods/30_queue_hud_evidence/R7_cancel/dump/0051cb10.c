// FN 0051cb10 FUN_0051cb10 size=143 ret=4 callers=007f24c0 callees=0057e3d0,00590430,00541080,0051c100

void FUN_0051cb10(undefined4 param_1)

{
  int iVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  
  iVar1 = FUN_0051c100(param_1);
  if (iVar1 != 0) {
    uVar2 = extraout_var;
    FUN_00541080(1);
    FUN_00590430(uVar2);
    *(undefined4 *)(*(int *)(iVar1 + 0x10dc) + 0xb20) = 0x7f000000;
    FUN_0057e3d0(0x7f000000);
    *(undefined4 *)(iVar1 + 0x548) = 0x7f000000;
    *(undefined4 *)(iVar1 + 0x568) = 0x7f000000;
    *(undefined4 *)(iVar1 + 0x56c) = 0x7f000000;
  }
  return;
}

