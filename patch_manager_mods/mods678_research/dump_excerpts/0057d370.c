// FN 0057d370 FUN_0057d370 size=348 ret=8 callers=005a7820,005644b0,0058e730,005a8410,00546900,005a7610,007c41f0,006dac70,006d7390,00564770,005bbf40,00691a30,005d29f0,005b96f0,006dab20,005d25e0,00597d90,0066df70,0056e620 callees=005143c0,00543f10,0051c080,0053dfb0,004452b0

void __thiscall FUN_0057d370(int *param_1,int param_2,int param_3)

{
  bool bVar1;
  char cVar2;
  short sVar3;
  int iVar4;
  undefined4 *puVar5;
  int *piVar6;
  undefined1 local_20 [12];
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  if (param_3 == 0) {
    iVar4 = (**(code **)(*param_1 + 0x94))();
    if (iVar4 != 0) {
      return;
    }
    if (param_1[0x46b] == 0) {
      bVar1 = false;
    }
    else {
      sVar3 = (**(code **)(*param_1 + 0x9c))(0);
      bVar1 = sVar3 < 1;
    }
    if (bVar1) {
      return;
    }
  }
  if (((param_2 == DAT_00994484) && (param_1[0x413] != DAT_00994484)) &&
     (param_1[0x413] != param_1[1])) {
    piVar6 = &local_8;
    iVar4 = param_1[0x413];
    FUN_0051c080(iVar4,piVar6);
    cVar2 = FUN_0053dfb0(iVar4,piVar6);
    if ((cVar2 == DAT_0099447f) && (DAT_00994473 < *(byte *)(local_8 + 8))) {
      FUN_004452b0(&local_14,local_8 + 0x94,param_1 + 0x25);
      puVar5 = (undefined4 *)FUN_005143c0(local_20,&local_14);
      local_14 = *puVar5;
      local_10 = puVar5[1];
      local_c = puVar5[2];
      FUN_00543f10(&local_14);
    }
  }
  param_1[0x413] = param_2;
  return;
}

