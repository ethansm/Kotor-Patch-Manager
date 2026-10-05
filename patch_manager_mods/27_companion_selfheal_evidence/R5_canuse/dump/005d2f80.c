// FN 005d2f80 FUN_005d2f80 size=821 ret=12 callers=008d34e0,0057b4c0,00633c70,0077b670,0074b3f0,0077c780,008a8df0,00645830 callees=005d3650,005d3c90,006d6e30,005d4190,0051bd40,005d3480,006a4e00,00606c30,005d3ee0,005d3a20,005bda40,005d32c0,006d5200,005d3b60

/* WARNING: Removing unreachable block (ram,0x005d3177) */

bool __thiscall FUN_005d2f80(int *param_1,int param_2,undefined4 param_3,int param_4)

{
  int iVar1;
  bool bVar2;
  byte bVar3;
  byte bVar4;
  short sVar5;
  short sVar6;
  int iVar7;
  int iVar8;
  bool bVar9;
  int local_c;
  
  if (param_2 == 0) {
    bVar9 = false;
  }
  else {
    iVar7 = FUN_005d3480(param_2);
    bVar9 = iVar7 != 0;
    if ((bVar9) && (iVar7 = FUN_005d32c0(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if (bVar9 == true) {
      iVar7 = FUN_005d3650(param_2,param_4);
      if (iVar7 == 0) {
        bVar9 = false;
      }
      else {
        iVar7 = FUN_006d6e30();
        if ((param_4 != 0) &&
           ((*(char *)(iVar7 + 0xac) == '-' || (*(char *)(iVar7 + 0xac) == '\x1a')))) {
          sVar5 = (**(code **)(*param_1 + 0x9c))(0);
          sVar6 = (**(code **)(*param_1 + 0x98))(1);
          if (sVar5 == sVar6) {
            bVar9 = false;
          }
        }
      }
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d3a20(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d3b60(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d3c90(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d3ee0(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d4190(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if (param_1[0x43a] == 1) {
      bVar3 = FUN_006a4e00(0);
      bVar4 = FUN_00606c30();
      if ((bVar3 < bVar4) && (iVar7 = FUN_0051bd40(), *(int *)(iVar7 + 0xd0) != 0)) {
        bVar9 = false;
      }
    }
    if (bVar9 == true) {
      iVar7 = FUN_006d5200(*(undefined4 *)(param_2 + 0xc));
      if ((((iVar7 != 0) && (*(char *)(iVar7 + 8) != '\b')) && (*(char *)(iVar7 + 9) != '\0')) &&
         (iVar1 = param_1[0x14f], iVar8 = FUN_006d5200(*(undefined4 *)(param_2 + 0xc)),
         1 < (int)((uint)*(byte *)(iVar8 + 0x1b) - iVar1))) {
        bVar9 = false;
      }
      bVar2 = false;
      for (local_c = 0; local_c < 0x14; local_c = local_c + 1) {
        if ((1 << ((byte)local_c & 0x1f) & *(uint *)(iVar7 + 4)) != 0) {
          bVar2 = true;
          break;
        }
      }
      if ((bVar2) && (iVar7 = FUN_005bda40(param_2,1 << ((byte)local_c & 0x1f)), iVar7 == 0)) {
        bVar9 = false;
      }
    }
  }
  return bVar9;
}

