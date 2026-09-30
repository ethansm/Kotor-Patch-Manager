// FN 00742a00 FUN_00742a00 size=1990 ret=4 callers=00743540 callees=005f67b0,00733570,00734010,0071bcf0,0041bcf0,007f2230,007408f0,00734270,00733780,0041bcb0,007f2540,00842d00,00733df0,007337c0,00733540,00742750

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00742a00(int param_1,int *param_2)

{
  bool bVar1;
  short sVar2;
  uint uVar3;
  undefined4 uVar4;
  int iVar5;
  undefined1 local_c8 [8];
  undefined1 local_c0 [8];
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  int local_50;
  int *local_4c;
  int local_48;
  int local_44;
  int local_40;
  float local_3c;
  float local_38;
  uint local_34;
  float local_30;
  int *local_24;
  undefined4 local_20;
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00962213;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(uVar3);
  local_8 = 0;
  local_14 = 0x3f800000;
  if (param_2 != (int *)0x0) {
    local_20 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    local_24 = (int *)FUN_007f2540();
    if (local_24 != (int *)0x0) {
      local_30 = 1.0;
      FUN_00733540(uVar3);
      local_8._0_1_ = 1;
      FUN_007f2230(local_1c,&local_34);
      if (local_34 == 0) {
        if (*(float *)(param_1 + 0x1b9c) <= (float)_DAT_00986b38) {
          local_40 = -1;
          iVar5 = param_1 + 0x54 + *(char *)(param_1 + 0x1bab) * 0x750;
          if (((((*(uint *)(iVar5 + 0x218) & 1) == 0) && ((*(uint *)(iVar5 + 1000) & 1) == 0)) &&
              ((*(uint *)(iVar5 + 0x5b8) & 1) == 0)) && ((*(uint *)(iVar5 + 0x48) & 1) == 0)) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            local_40 = (int)*(char *)(param_1 + 0x1bab);
          }
          if ((local_40 < 0) || (*(int *)(param_1 + 4 + local_40 * 0xc) < 1)) {
            uVar4 = (**(code **)(*param_2 + 0x70))(local_60);
            local_8._0_1_ = 3;
            FUN_007337c0(uVar4);
            local_8._0_1_ = 1;
            FUN_00733780();
            iVar5 = (**(code **)(*param_2 + 0x4c))();
            if (iVar5 != 0) {
              (**(code **)(*param_2 + 0x4c))();
              local_4c = (int *)FUN_00842d00();
              if ((local_4c != (int *)0x0) && (local_4c[0xdf] != 0)) {
                uVar4 = 0;
                (**(code **)(*local_4c + 0x48))(0);
                iVar5 = FUN_005f67b0(uVar4);
                if (iVar5 == 0) {
                  local_50 = 0;
                  FUN_00733570("UseSearch");
                  local_8._0_1_ = 4;
                  iVar5 = (**(code **)(*param_2 + 0x4c))();
                  FUN_0071bcf0(*(undefined4 *)(iVar5 + 0x10c),local_68,&local_50);
                  local_8._0_1_ = 1;
                  FUN_00733780();
                  if (local_50 == 0) {
                    FUN_00733570(&DAT_0099d77c);
                    local_8._0_1_ = 0xb;
                    FUN_00733570(&DAT_0099761c);
                    local_8._0_1_ = 0xc;
                    uVar4 = FUN_007408f0(local_b0,0x1ea6e);
                    local_8._0_1_ = 0xd;
                    FUN_00733df0(local_a8,local_a0);
                    local_8._0_1_ = 0xe;
                    FUN_00733df0(local_b8,uVar4);
                    local_8._0_1_ = 0xf;
                    uVar4 = FUN_00733df0(local_c8,local_c0);
                    local_8._0_1_ = 0x10;
                    FUN_007337c0(uVar4);
                    local_8._0_1_ = 0xf;
                    FUN_00733780();
                    local_8._0_1_ = 0xe;
                    FUN_00733780();
                    local_8._0_1_ = 0xd;
                    FUN_00733780();
                    local_8._0_1_ = 0xc;
                    FUN_00733780();
                    local_8._0_1_ = 0xb;
                    FUN_00733780();
                    local_8._0_1_ = 1;
                    FUN_00733780();
                  }
                  else {
                    FUN_00733570(&DAT_0099d77c);
                    local_8._0_1_ = 5;
                    FUN_00733570(&DAT_0099761c);
                    local_8._0_1_ = 6;
                    uVar4 = FUN_007408f0(local_80,0x1ea6f);
                    local_8._0_1_ = 7;
                    FUN_00733df0(local_78,local_70);
                    local_8._0_1_ = 8;
                    FUN_00733df0(local_88,uVar4);
                    local_8._0_1_ = 9;
                    uVar4 = FUN_00733df0(local_98,local_90);
                    local_8._0_1_ = 10;
                    FUN_007337c0(uVar4);
                    local_8._0_1_ = 9;
                    FUN_00733780();
                    local_8._0_1_ = 8;
                    FUN_00733780();
                    local_8._0_1_ = 7;
                    FUN_00733780();
                    local_8._0_1_ = 6;
                    FUN_00733780();
                    local_8._0_1_ = 5;
                    FUN_00733780();
                    local_8._0_1_ = 1;
                    FUN_00733780();
                  }
                }
              }
            }
          }
          else {
            local_44 = 0;
            if (*(int *)(param_1 + 0x24 + *(char *)(param_1 + 0x1baa) * 0xc + local_40 * 4) != -1) {
              for (local_48 = 0; local_48 < *(int *)(param_1 + 4 + local_40 * 0xc);
                  local_48 = local_48 + 1) {
                if (*(int *)(*(int *)(param_1 + local_40 * 0xc) + 8 + local_48 * 0x3c) ==
                    *(int *)(param_1 + 0x24 + *(char *)(param_1 + 0x1baa) * 0xc + local_40 * 4)) {
                  local_44 = local_48 * 0x3c + *(int *)(param_1 + local_40 * 0xc);
                }
              }
            }
            if (local_44 == 0) {
              local_44 = *(int *)(param_1 + local_40 * 0xc);
            }
            FUN_007337c0(local_44);
            if (1 < *(int *)(local_44 + 0x34)) {
              uVar4 = FUN_00734010(*(undefined4 *)(local_44 + 0x34));
              FUN_00734270(local_1c,"%s (%d)",uVar4);
            }
          }
        }
        else {
          local_3c = *(float *)(param_1 + 0x1b9c) / (float)_DAT_00991370;
          if (local_3c < (float)_DAT_00987f08) {
            local_30 = local_3c + local_3c;
          }
          uVar4 = FUN_007408f0(local_58,*(undefined4 *)(param_1 + 0x1ba4));
          local_8._0_1_ = 2;
          FUN_007337c0(uVar4);
          local_8._0_1_ = 1;
          FUN_00733780();
        }
      }
      else {
        local_38 = (float)((float10)local_34 / (float10)_DAT_009921c0);
        if (local_38 < (float)_DAT_00987f08) {
          local_30 = local_38 + local_38;
        }
      }
      sVar2 = (**(code **)(*local_24 + 0x98))(1);
      FUN_0041bcb0((int)sVar2);
      sVar2 = (**(code **)(*local_24 + 0x9c))(0);
      FUN_0041bcf0((int)sVar2);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
    }
  }
  FUN_00742750(local_1c);
  *(undefined4 *)(param_1 + 0x1780) = local_14;
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}

