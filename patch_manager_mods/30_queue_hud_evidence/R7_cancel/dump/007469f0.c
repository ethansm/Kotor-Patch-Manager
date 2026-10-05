// FN 007469f0 FUN_007469f0 size=1310 ret=8 callers= callees=0073ff80,00746950,00541080,00744410,005210f0,00751750,0077d800,005904c0,00740880,007469a0,00753680,0073fea0,00744260,00741410,004122a0,0073fb90,005b7ae0,00740380,00751710,007e5d80,007468e0,00752450,0074d520,007525d0

/* WARNING: Removing unreachable block (ram,0x00746d6c) */
/* WARNING: Removing unreachable block (ram,0x00746eb5) */
/* WARNING: Removing unreachable block (ram,0x00746ec2) */
/* WARNING: Removing unreachable block (ram,0x00746ee1) */
/* WARNING: Removing unreachable block (ram,0x00746eef) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_007469f0(int param_1)

{
  bool bVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  uint uVar5;
  bool bVar6;
  undefined4 uVar7;
  int *extraout_var;
  int *piVar8;
  int local_3c;
  int local_38;
  uint local_24;
  uint local_20;
  
  FUN_0073fb90();
  piVar8 = extraout_var;
  iVar4 = FUN_007e5d80();
  if (iVar4 != 0) {
    iVar4 = FUN_0077d800();
    bVar6 = false;
    if (iVar4 != 0) {
      bVar6 = *(int *)(iVar4 + 0x10dc) != 0;
    }
    bVar2 = false;
    local_24 = 0;
    local_20 = 0;
    param_1 = param_1 + -0x27;
    switch(param_1) {
    case 0:
      DAT_00a7ead4 = 1;
      cVar3 = FUN_00741410();
      if (cVar3 != '\0') {
        if (DAT_00a10340 < 0) {
          FUN_0073ff80(0xef,1);
        }
        else {
          iVar4 = FUN_007469a0(DAT_00a10340);
          if ((*(char *)(iVar4 + 0x74c) == '\0') || (*(char *)(iVar4 + 0x74d) != '\0')) {
            FUN_00746950();
            FUN_0073ff80(0xef,1);
          }
          else if ((DAT_00a10340 < 0) || (5 < DAT_00a10340)) {
            FUN_00751710(DAT_00a10340 + -6,0);
          }
          else {
            FUN_00751750(DAT_00a10340,0);
          }
        }
      }
      DAT_00a7ead4 = 0;
      break;
    case 1:
      if (DAT_00a10340 == -1) {
        if (iVar4 != 0) {
          bVar1 = true;
          if ((bVar6) && (uVar5 = FUN_005210f0(), 1 < uVar5)) {
            FUN_005904c0();
            bVar1 = false;
          }
          if (bVar1) {
            FUN_005904c0();
            FUN_00740880(0);
            FUN_005b7ae0(0,0);
            FUN_00541080(1);
            piVar8[0x1657] = _DAT_00987474;
            piVar8[0x1c] = _DAT_00987474;
          }
        }
      }
      else {
        bVar2 = true;
        local_20 = 1;
      }
      break;
    case 2:
      FUN_00740380(1,1);
      break;
    case 3:
      FUN_00753680();
      break;
    case 8:
      cVar3 = FUN_00741410();
      if (cVar3 != '\0') {
        local_20 = 0xffffffff;
      }
      break;
    case 9:
      cVar3 = FUN_00741410();
      local_20 = (uint)(cVar3 != '\0');
      break;
    case 10:
      cVar3 = FUN_00741410();
      if (cVar3 != '\0') {
        local_24 = 0xffffffff;
      }
      break;
    case 0xb:
      cVar3 = FUN_00741410();
      local_24 = (uint)(cVar3 != '\0');
    }
    if (local_20 != 0) {
      uVar7 = 7;
      FUN_0073fea0(7);
      FUN_004122a0(uVar7);
      local_38 = FUN_007469a0(DAT_00a10340);
      if (local_38 != 0) {
        FUN_007468e0(0);
      }
      if (bVar2) {
        FUN_00746950();
        (**(code **)(*piVar8 + 8))(0,0);
      }
      else {
        if ((DAT_00a10340 == -1) && (local_20 == 0xffffffff)) {
          DAT_00a10340 = 0;
        }
        for (local_3c = 0; local_3c < 9; local_3c = local_3c + 1) {
          DAT_00a10340 = (int)(DAT_00a10340 + 9 + local_20) % 9;
          local_38 = FUN_007469a0(DAT_00a10340);
          if ((local_38 != 0) && (*(char *)(local_38 + 0x74d) == '\0')) break;
        }
        if ((local_38 != 0) && (*(char *)(local_38 + 0x74d) == '\0')) {
          FUN_007468e0(1);
          (**(code **)(*piVar8 + 8))(local_38 + 0x1d0,0);
        }
      }
    }
    if (local_24 != 0) {
      uVar7 = 7;
      FUN_0073fea0(7,param_1,piVar8,*(undefined4 *)(DAT_00a1b4a4 + 4));
      FUN_004122a0(uVar7);
      FUN_007469a0(DAT_00a10340);
      if (DAT_00a10340 < 6) {
        if ((int)local_24 < 0) {
          FUN_007525d0(DAT_00a10340);
        }
        else {
          FUN_00752450(DAT_00a10340);
        }
      }
      else if ((int)local_24 < 0) {
        FUN_00744410(DAT_00a10340 + -6);
      }
      else {
        FUN_00744260(DAT_00a10340 + -6);
      }
    }
  }
  return;
}

