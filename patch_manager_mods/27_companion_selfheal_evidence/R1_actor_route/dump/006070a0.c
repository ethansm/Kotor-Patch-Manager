// FN 006070a0 FUN_006070a0 size=1460 ret=8 callers=005b8000,0058b360 callees=0051c280,00603860,0051c370,0051ad20,0072fb00,006d6e30,0051c600,00604260,0051c2e0,0064b760,0051e170

void FUN_006070a0(undefined4 param_1,undefined4 param_2)

{
  bool bVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  undefined4 uVar11;
  undefined4 *puVar12;
  undefined4 uVar13;
  int extraout_var;
  int local_30;
  undefined4 local_24;
  uint local_20;
  undefined4 local_1c;
  int local_18;
  uint local_14;
  int local_10;
  int local_c;
  short *local_8;
  
  iVar4 = FUN_0072fb00();
  if (*(int *)(iVar4 + 0x94) == 0) {
    iVar4 = extraout_var;
    local_8 = (short *)FUN_00604260(param_1);
    local_20 = (uint)(ushort)local_8[3];
    local_14 = 0;
    bVar2 = false;
    bVar3 = false;
    iVar5 = local_20 - 1;
    switch(local_20) {
    case 1:
      bVar1 = *(int *)(iVar4 + 0x2cc) < 2;
      if (bVar1) {
        local_8[6] = 0;
        local_8[7] = 0;
      }
      else {
        *(int *)(iVar4 + 0x2cc) = *(int *)(iVar4 + 0x2cc) + -1;
      }
      local_14 = (uint)bVar1;
      break;
    case 2:
      bVar2 = true;
      local_18 = 5;
      break;
    case 3:
      bVar2 = true;
      local_18 = 4;
      break;
    case 4:
      bVar2 = true;
      local_18 = 3;
      break;
    case 5:
      bVar2 = true;
      local_18 = 2;
      break;
    case 6:
      bVar2 = true;
      local_18 = 1;
      break;
    case 8:
    case 9:
    case 10:
    case 0xb:
    case 0xc:
      bVar3 = true;
      if ((char)local_8[8] != '\0') {
        *(char *)(local_8 + 8) = (char)local_8[8] + -1;
      }
      if ((char)local_8[8] == '\0') {
        local_8[6] = 0;
        local_8[7] = 0;
      }
      break;
    case 0xe:
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x12:
      bVar3 = true;
      puVar12 = &local_24;
      puVar10 = &local_1c;
      FUN_0051c370(puVar10,puVar12);
      FUN_0051ad20(puVar10,puVar12);
      *(undefined4 *)(local_8 + 10) = local_1c;
      *(undefined4 *)(local_8 + 0xc) = local_24;
      local_8[6] = 0;
      local_8[7] = 0;
    }
    if (bVar2) {
      local_c = *(int *)(iVar4 + 0x298);
      if (local_18 <= local_c) {
        *(int *)(iVar4 + 0x298) = local_c - local_18;
      }
      for (local_10 = 0; local_10 < *(int *)(iVar4 + 0x288); local_10 = local_10 + 1) {
        local_8 = (short *)FUN_00604260(local_10);
        if (((local_8 != (short *)0x0) && (*local_8 == 10)) &&
           (((((local_20 = (uint)(ushort)local_8[3], local_20 == 2 && (*(int *)(iVar4 + 0x298) < 5))
              || ((local_20 == 3 && (*(int *)(iVar4 + 0x298) < 4)))) ||
             ((local_20 == 4 && (*(int *)(iVar4 + 0x298) < 3)))) ||
            (((local_20 == 5 && (*(int *)(iVar4 + 0x298) < 2)) ||
             ((local_20 == 6 && (*(int *)(iVar4 + 0x298) < 1)))))))) {
          local_8[6] = 0;
          local_8[7] = 0;
        }
      }
    }
    if ((char)local_8[6] == '\0') {
      iVar6 = FUN_006d6e30();
      if (((*(char *)(iVar6 + 0xac) == '\x19') ||
          (iVar6 = FUN_006d6e30(), *(char *)(iVar6 + 0xac) == '-')) ||
         (iVar6 = FUN_006d6e30(), *(char *)(iVar6 + 0xac) == '/')) {
        local_14 = 1;
      }
      else if (bVar2) {
        local_30 = 0;
        while ((local_30 < *(int *)(iVar4 + 0x288) &&
               (((local_8 = (short *)FUN_00604260(local_30), local_8 == (short *)0x0 ||
                 (*local_8 != 10)) ||
                ((((local_20 = (uint)(ushort)local_8[3], local_20 != 2 &&
                   ((local_20 != 3 && (local_20 != 4)))) && ((local_20 != 5 && (local_20 != 6)))) ||
                 ((char)local_8[6] == '\0'))))))) {
          local_30 = local_30 + 1;
        }
        if (local_30 == *(int *)(iVar4 + 0x288)) {
          local_14 = 1;
        }
      }
    }
    if ((local_14 == 0) || (bVar3)) {
      FUN_00603860();
    }
    else {
      iVar6 = FUN_0051c600(param_2);
      if (iVar6 != 0) {
        uVar9 = *(undefined4 *)(iVar4 + 0x14);
        FUN_0051c2e0(iVar6,uVar9);
        FUN_0064b760(iVar6,uVar9);
      }
      *(uint *)(iVar4 + 0x2c8) = *(uint *)(iVar4 + 0x2c8) & 0xfffffeff | 0x100;
      uVar9 = *(undefined4 *)(iVar4 + 0x14);
      uVar13 = 0;
      uVar8 = 0;
      uVar7 = 0;
      uVar11 = DAT_00992420;
      FUN_0051c280(0,0,param_2,uVar9,DAT_00992420,0,iVar5,iVar4,*(undefined4 *)(DAT_00a1b4a4 + 8));
      FUN_0051e170(uVar7,uVar8,param_2,uVar9,uVar11,uVar13);
    }
  }
  return;
}

