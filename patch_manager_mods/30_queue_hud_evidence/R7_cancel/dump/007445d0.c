// FN 007445d0 FUN_007445d0 size=1017 ret=8 callers=00751710 callees=00754e20,00541080,007cbdb0,0077d800,0073f4d0,00740960,0073f750,0077fd20,00740860,0073fea0,007f2500,004122a0,0073fb90,00741780,007e5d80,0074ec10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007445d0(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_50;
  int local_14;
  int local_8;
  
  if (*(int *)(param_1 + 0x1644) != 0) {
    FUN_00754e20();
  }
  if (*(int *)(param_1 + 4 + param_2 * 0xc) != 0) {
    FUN_0073fb90();
    iVar2 = FUN_007e5d80();
    local_8 = 0;
    if (*(int *)(param_1 + 0x24 + *(char *)(param_1 + 0x1baa) * 0xc + param_2 * 4) != -1) {
      for (local_14 = 0; local_14 < *(int *)(param_1 + 4 + param_2 * 0xc); local_14 = local_14 + 1)
      {
        if (*(int *)(*(int *)(param_1 + param_2 * 0xc) + 8 + local_14 * 0x3c) ==
            *(int *)(param_1 + 0x24 + *(char *)(param_1 + 0x1baa) * 0xc + param_2 * 4)) {
          local_8 = local_14 * 0x3c + *(int *)(param_1 + param_2 * 0xc);
          break;
        }
      }
    }
    if (local_8 == 0) {
      local_8 = *(int *)(param_1 + param_2 * 0xc);
    }
    if ((iVar2 != 0) && (local_8 != 0)) {
      piVar3 = (int *)FUN_0073f4d0(*(undefined4 *)(local_8 + 0x1c));
      if (piVar3 == (int *)0x0) {
        local_50 = 0;
      }
      else {
        local_50 = (**(code **)(*piVar3 + 0xc))();
      }
      if (local_50 != 0) {
        if ((*(uint *)(local_8 + 0x30) & 1) != 0) {
          if ((*(int *)(local_8 + 0xc) == 0) &&
             ((*(int *)(local_8 + 0xc) == 0 ||
              (((*(int *)(local_8 + 0x10) == 0 && (*(int *)(local_8 + 0x14) == 0)) &&
               (*(int *)(local_8 + 0x18) == -1)))))) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            bVar1 = false;
            if ((*(uint *)(iVar2 + 0x458) & 1) == 0) {
              iVar4 = FUN_00740860();
              if ((iVar4 == 0) && (*(char *)(param_1 + 0x1baa) == '\x03')) {
                bVar1 = true;
              }
              FUN_007f2500();
              if (((*(char *)(param_1 + 0x1baa) == '\0') && (*(int *)(local_8 + 8) == 0x3f5)) ||
                 (*(char *)(param_1 + 0x1baa) == '\x03')) {
                *(undefined4 *)(*(int *)(param_1 + 0x1644) + 0x7334) = _DAT_00987474;
                FUN_0074ec10(0x1b2c7);
              }
            }
            else {
              iVar4 = FUN_00740960();
              if (iVar4 != 0) {
                uVar5 = 10;
                uVar6 = 1;
                FUN_0073f750(1,10);
                FUN_007cbdb0(uVar6,uVar5);
              }
            }
            uVar6 = 6;
            FUN_0073fea0(6);
            FUN_004122a0(uVar6);
            (**(code **)(local_8 + 0xc))(*(undefined4 *)(local_8 + 8),iVar2);
            FUN_00741780(1);
            if ((*(uint *)(iVar2 + 0x458) & 1) != 0) {
              return;
            }
            if (bVar1) {
              FUN_0077fd20(1);
            }
            iVar2 = FUN_0077d800();
            if (iVar2 == 0) {
              return;
            }
            uVar6 = 0;
            FUN_0077d800(0);
            FUN_00541080(uVar6);
            return;
          }
        }
        if ((*(uint *)(local_8 + 0x30) >> 1 & 0xf) != 0) {
          switch(*(uint *)(local_8 + 0x30) >> 1 & 0xf) {
          case 1:
            *(undefined4 *)(param_1 + 0x1ba4) = 0x96d5;
            break;
          case 2:
            *(undefined4 *)(param_1 + 0x1ba4) = 0x96d6;
            break;
          case 3:
            *(undefined4 *)(param_1 + 0x1ba4) = 0x96d7;
            break;
          case 4:
            *(undefined4 *)(param_1 + 0x1ba4) = 0xa5b6;
            break;
          case 5:
            *(undefined4 *)(param_1 + 0x1ba4) = 0xa602;
            break;
          case 6:
            *(undefined4 *)(param_1 + 0x1ba4) = 0xbb40;
          }
          *(undefined4 *)(param_1 + 0x1b9c) = _DAT_00986b68;
        }
        uVar6 = 2;
        FUN_0073fea0(2);
        FUN_004122a0(uVar6);
      }
    }
  }
  return;
}

