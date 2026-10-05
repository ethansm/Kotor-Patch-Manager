// FN 005bf7f0 FUN_005bf7f0 size=328 ret=4 callers=005d56b0 callees=0051c8b0,005453c0,0073fb90

undefined4 __fastcall FUN_005bf7f0(int *param_1)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  
  uVar5 = DAT_009944bc;
  if (((param_1[0x46b] != 0) && (param_1[0x140] != 0)) &&
     (iVar3 = (**(code **)(*param_1 + 0x94))(), uVar5 = DAT_009944bc, iVar3 != 1)) {
    if (param_1[0x46b] == 0) {
      bVar1 = false;
    }
    else {
      sVar2 = (**(code **)(*param_1 + 0x9c))(0);
      bVar1 = sVar2 < 1;
    }
    uVar5 = DAT_009944bc;
    if (((!bVar1) && (param_1[0xe0] != 0)) &&
       (((*(ushort *)(param_1 + 0x445) & 2) == 2 &&
        (iVar3 = FUN_005453c0(), uVar5 = DAT_009944b4, iVar3 != 0)))) {
      iVar4 = FUN_0073fb90();
      iVar3 = param_1[1];
      iVar4 = FUN_0051c8b0(*(undefined4 *)(iVar4 + 0x24));
      uVar5 = DAT_009944bc;
      if (iVar4 != iVar3) {
        param_1[0x46c] = 3;
        uVar5 = DAT_009944b4;
      }
    }
  }
  return uVar5;
}

