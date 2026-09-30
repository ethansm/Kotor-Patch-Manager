// FN 00798d70 FUN_00798d70 size=5780 ret=16 callers=0079b700,0079a4c0 callees=00734010,0058b140,00478c40,005142f0,007e5da0,00773bf0,006b7bb0,006d6fe0,007e2380,00561430,005210f0,0077d800,007f2540,0058c370,0051ca40,00478a90,006a20f0,00557ef0,0046ce40,007a1780,00838f70,0057eca0,0051c0a0,007e6be0,00710930,006b83f0,00796b50,007a1720,005e9210,0058b210,00733780,0058ae90,00726f80,0051bd90,007ed7d0,006c1450,005d53a0,0073fb90,006bd9c0,005ee7b0,0051c8f0,0073fec0,00733540

/* WARNING: Removing unreachable block (ram,0x00799f3c) */
/* WARNING: Switch with 1 destination removed at 0x00799595 : 9 cases all go to same destination */
/* WARNING: Switch with 1 destination removed at 0x007996cd : 11 cases all go to same destination */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00798d70(int param_1,int *param_2,int param_3,int param_4)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  float10 fVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  undefined1 local_21c [8];
  undefined1 local_214 [16];
  undefined1 local_204 [12];
  undefined1 local_1f8 [12];
  undefined1 local_1ec [12];
  undefined1 local_1e0 [12];
  undefined1 local_1d4 [12];
  undefined1 local_1c8 [12];
  undefined1 local_1bc [12];
  undefined1 local_1b0 [12];
  undefined1 local_1a4 [12];
  undefined1 local_198 [12];
  undefined1 local_18c [12];
  undefined1 local_180 [12];
  undefined1 local_174 [12];
  undefined1 local_168 [12];
  undefined1 local_15c [12];
  undefined1 local_150 [12];
  undefined1 local_144 [12];
  undefined1 local_138 [12];
  undefined1 local_12c [12];
  undefined1 local_120 [12];
  undefined1 local_114 [12];
  undefined1 local_108 [12];
  undefined1 local_fc [12];
  undefined1 local_f0 [12];
  undefined1 local_e4 [12];
  float local_d8;
  float local_d4;
  float local_d0;
  int local_cc;
  undefined1 local_c8 [8];
  undefined4 local_c0;
  uint local_bc;
  int *local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  float local_9c;
  undefined4 local_98;
  undefined4 local_94;
  float local_90;
  undefined4 local_8c;
  undefined4 local_88;
  float local_84;
  undefined4 local_80;
  undefined4 local_7c;
  float local_78;
  undefined4 local_74;
  undefined4 local_70;
  float local_6c;
  int local_68;
  undefined4 local_64;
  undefined4 local_60;
  float local_5c;
  int local_58;
  undefined4 *local_54;
  int local_50;
  int local_4c;
  int local_48;
  undefined4 local_44;
  int *local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  int *local_30;
  int *local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00967826;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00796b50(param_2,0);
  if (param_2 == (int *)0x0) {
    local_14 = FUN_007e5da0(0);
    if (local_14 != 0) {
      FUN_00773bf0(0x7f000000,_DAT_00987704);
    }
  }
  else {
    if (((param_3 != 0) && (param_2 != (int *)0x0)) && (local_18 = FUN_007e5da0(0), local_18 != 0))
    {
      FUN_00773bf0(param_2[1],_DAT_00987704);
    }
    if ((param_3 != 0) && ((param_4 != 0 || (*(int *)(param_1 + 800) != 0)))) {
      iVar2 = FUN_00726f80(uVar1);
      local_1c = (**(code **)(**(int **)(iVar2 + 0x40) + 0x80))(0x106a);
      if (local_1c != 0) {
        FUN_007e2380(param_2);
      }
      if (((*(int *)(param_1 + 800) != 0) && (param_2 != (int *)0x0)) &&
         (iVar2 = (**(code **)(*param_2 + 0x2c))(), iVar2 != 0)) {
        piVar3 = (int *)(**(code **)(*param_2 + 0x2c))();
        iVar2 = (**(code **)(*piVar3 + 0x138))();
        if (iVar2 != 0) {
          local_20 = 0;
          (**(code **)(*param_2 + 0xa4))(&DAT_00a7fdd0,0,0,1);
          local_24 = 0;
          while ((local_24 < *(int *)(param_1 + 0x318) && (local_20 == 0))) {
            if (*(int *)(*(int *)(param_1 + 0x314) + local_24 * 8) == param_2[1]) {
              *(undefined4 *)(*(int *)(param_1 + 0x314) + 4 + local_24 * 8) = _DAT_0098be34;
              local_20 = 1;
            }
            local_24 = local_24 + 1;
          }
          if (local_20 == 0) {
            local_28 = *(int *)(param_1 + 0x318);
            FUN_00838f70(local_28 + 1);
            *(int *)(*(int *)(param_1 + 0x314) + local_28 * 8) = param_2[1];
            *(undefined4 *)(*(int *)(param_1 + 0x314) + 4 + local_28 * 8) = _DAT_0098be34;
          }
        }
      }
    }
    if (DAT_00a7e090 != 0) {
      local_2c = (int *)FUN_007f2540();
      if (DAT_00a7ebc0 != 0) {
        if (DAT_00a7ebc0 == -9999) {
          DAT_00a7ebc0 = FUN_0051c8f0();
        }
        local_30 = (int *)FUN_0051c0a0(DAT_00a7ebc0);
        if ((local_30 != (int *)0x0) && (iVar2 = (**(code **)(*local_30 + 0x10))(), iVar2 != 0)) {
          local_2c = (int *)(**(code **)(*local_30 + 0x10))();
        }
      }
      if (local_2c != (int *)0x0) {
        local_3c = 6;
        local_44 = 0x17;
        local_34 = 0x25;
        FUN_00734010();
        local_40 = (int *)(**(code **)(*local_2c + 0x30))();
        if (local_40 != (int *)0x0) {
          local_4c = *(int *)(local_40[0xe0] + 0x8c) / 2;
          local_54 = *(undefined4 **)(local_40[0xe0] + 0x90);
          for (local_50 = 0; local_50 < local_4c; local_50 = local_50 + 1) {
          }
          if ((DAT_00a7e094 != 0) && (local_54 != (undefined4 *)0x0)) {
            for (local_68 = 0; local_68 < 200; local_68 = local_68 + 1) {
              if (DAT_00a106fc != 0) {
                (&DAT_00a7ebd8)[local_68] = 0xffffffff;
              }
              FUN_00478c40((&DAT_00a7ebd8)[local_68]);
              (&DAT_00a7ebd8)[local_68] = 0xffffffff;
            }
            DAT_00a106fc = 0;
            local_64 = *local_54;
            local_60 = local_54[1];
            local_5c = 0.0;
            FUN_0051bd90();
            iVar2 = FUN_00557ef0();
            fVar5 = (float10)FUN_0051ca40(iVar2 + 0x110,&local_64,0);
            local_5c = (float)fVar5;
            for (local_50 = 1; local_50 < local_4c; local_50 = local_50 + 1) {
              local_74 = local_54[local_50 * 2];
              local_70 = local_54[local_50 * 2 + 1];
              local_6c = 0.0;
              FUN_0051bd90();
              iVar2 = FUN_00557ef0();
              fVar5 = (float10)FUN_0051ca40(iVar2 + 0x110,&local_74,0);
              local_84 = (float)fVar5;
              local_a4 = local_64;
              local_a0 = local_60;
              local_9c = local_5c;
              local_98 = local_64;
              local_94 = local_60;
              local_90 = local_5c;
              local_8c = local_74;
              local_88 = local_70;
              local_80 = local_74;
              local_7c = local_70;
              local_78 = local_84;
              local_6c = local_84;
              uVar4 = FUN_00478a90(&local_a4,0xffffffff,".\\clientexoappinternal.cpp",0x355a);
              (&DAT_00a7ebd4)[local_50] = uVar4;
            }
          }
          FUN_005210f0();
          iVar2 = FUN_007a1780();
          if (iVar2 == 0) {
            local_ac = FUN_007a1720();
            local_50 = 0;
            do {
              if (local_ac == 0) break;
              local_a8 = FUN_00561430(local_ac);
              FUN_0058c370(&local_ac);
              local_50 = local_50 + 1;
            } while ((local_50 < 6) || (local_ac == 0));
          }
          FUN_0058b140();
          FUN_0058b210();
          uVar4 = 0;
          FUN_0073fb90(0);
          FUN_007e5da0(uVar4);
          local_58 = FUN_0077d800();
          local_d8 = (float)local_40[0x25] - *(float *)(local_58 + 0x94);
          local_d4 = (float)local_40[0x26] - *(float *)(local_58 + 0x98);
          local_d0 = (float)local_40[0x27] - *(float *)(local_58 + 0x9c);
          FUN_005142f0(&local_d8);
          (**(code **)(*local_40 + 0xc4))();
          FUN_0058ae90();
          FUN_007ed7d0();
          if (local_40[0x46b] != 0) {
            uVar4 = FUN_0073fec0(local_40[1]);
            FUN_0073fb90(uVar4);
            local_b4 = FUN_007e6be0(uVar4);
            iVar2 = FUN_0073fb90();
            local_b0 = iVar2 + 0x24 + local_b4 * 0x88;
          }
          switch(*(undefined1 *)((int)local_40 + 0xff5)) {
          case 0:
            break;
          case 1:
            break;
          case 2:
            break;
          case 0x10:
          }
          switch(*(undefined4 *)(local_40[0xe0] + 0x1e0)) {
          case 1:
            break;
          case 2:
            break;
          case 3:
            break;
          case 4:
            break;
          case 5:
          }
        }
        local_48 = (**(code **)(*local_2c + 0x48))();
        if (local_48 != 0) {
          FUN_005ee7b0(local_e4,local_48 + 0x468);
          FUN_005ee7b0(local_f0,local_48 + 0x468);
          FUN_005ee7b0(local_fc,local_48 + 0x474);
          FUN_005ee7b0(local_108,local_48 + 0x474);
          FUN_0046ce40(local_114);
          FUN_0046ce40(local_120);
          FUN_0046ce40(local_12c);
        }
        local_38 = (**(code **)(*local_2c + 0x18))();
        if (local_38 != 0) {
          FUN_005ee7b0(local_138,*(int *)(local_38 + 0x37c) + 0x70);
          FUN_005ee7b0(local_144,*(int *)(local_38 + 0x37c) + 0x70);
          FUN_005ee7b0(local_150,*(int *)(local_38 + 0x37c) + 0x70);
          FUN_005ee7b0(local_15c,*(int *)(local_38 + 0x37c) + 0x7c);
          FUN_005ee7b0(local_168,*(int *)(local_38 + 0x37c) + 0x7c);
          FUN_005ee7b0(local_174,*(int *)(local_38 + 0x37c) + 0x7c);
          FUN_005ee7b0(local_180,*(int *)(local_38 + 0x380) + 0x70);
          FUN_005ee7b0(local_18c,*(int *)(local_38 + 0x380) + 0x70);
          FUN_005ee7b0(local_198,*(int *)(local_38 + 0x380) + 0x70);
          FUN_005ee7b0(local_1a4,*(int *)(local_38 + 0x380) + 0x7c);
          FUN_005ee7b0(local_1b0,*(int *)(local_38 + 0x380) + 0x7c);
          FUN_005ee7b0(local_1bc,*(int *)(local_38 + 0x380) + 0x7c);
          FUN_005ee7b0(local_1c8,*(int *)(local_38 + 900) + 0x70);
          FUN_005ee7b0(local_1d4,*(int *)(local_38 + 900) + 0x70);
          FUN_005ee7b0(local_1e0,*(int *)(local_38 + 900) + 0x70);
          FUN_005ee7b0(local_1ec,*(int *)(local_38 + 900) + 0x7c);
          FUN_005ee7b0(local_1f8,*(int *)(local_38 + 900) + 0x7c);
          FUN_005ee7b0(local_204,*(int *)(local_38 + 900) + 0x7c);
        }
      }
    }
    if ((DAT_00a7e08c != 0) && (local_b8 = (int *)FUN_007f2540(), local_b8 != (int *)0x0)) {
      local_c0 = 6;
      FUN_00734010();
      FUN_00733540();
      local_8 = 0;
      puVar7 = local_c8;
      puVar6 = local_214;
      (**(code **)(*local_b8 + 0x80))(puVar6,puVar7);
      FUN_00710930(puVar6);
      FUN_00734010();
      iVar2 = (**(code **)(*local_b8 + 0x30))();
      if (iVar2 != 0) {
        (**(code **)(*local_b8 + 0x30))();
        (**(code **)(*local_b8 + 0x30))();
        FUN_005e9210(0x3e);
        (**(code **)(*local_b8 + 0x30))();
        FUN_005d53a0();
        (**(code **)(*local_b8 + 0x30))();
        (**(code **)(*local_b8 + 0x30))();
        (**(code **)(*local_b8 + 0x30))();
        FUN_0057eca0();
        local_cc = 0;
        for (local_bc = 0; (int)local_bc < **(int **)(DAT_00a1b4d0 + 0x104); local_bc = local_bc + 1
            ) {
          (**(code **)(*local_b8 + 0x30))();
          iVar2 = FUN_006bd9c0(0,local_bc,1);
          if ((iVar2 != 0) && (iVar2 = FUN_006c1450(local_bc), iVar2 != 0)) {
            FUN_006c1450(local_bc);
            FUN_00734010();
            local_cc = local_cc + 1;
          }
        }
        local_cc = 0;
        for (local_bc = 0; (int)local_bc < (int)(uint)*(ushort *)(DAT_00a1b4d0 + 0x11c);
            local_bc = local_bc + 1) {
          (**(code **)(*local_b8 + 0x30))();
          iVar2 = FUN_006b83f0(local_bc & 0xffff);
          if ((iVar2 != 0) && (iVar2 = FUN_006a20f0(local_bc & 0xffff), iVar2 != 0)) {
            puVar6 = local_21c;
            FUN_006a20f0(local_bc & 0xffff);
            uVar4 = FUN_006d6fe0(puVar6);
            local_8._0_1_ = 1;
            FUN_00734010(puVar7,uVar1,uVar4);
            local_8 = (uint)local_8._1_3_ << 8;
            FUN_00733780();
            local_cc = local_cc + 1;
          }
        }
        (**(code **)(*local_b8 + 0x30))();
        FUN_006b7bb0(0,0,0);
        (**(code **)(*local_b8 + 0x30))();
        FUN_006b7bb0(1,0,0);
        (**(code **)(*local_b8 + 0x30))();
        FUN_006b7bb0(2,0,0);
        (**(code **)(*local_b8 + 0x30))();
        FUN_006b7bb0(3,0,0);
        (**(code **)(*local_b8 + 0x30))();
        FUN_006b7bb0(4,0,0);
        (**(code **)(*local_b8 + 0x30))();
        FUN_006b7bb0(5,0,0);
        (**(code **)(*local_b8 + 0x30))();
        FUN_006b7bb0(6,0,0);
        (**(code **)(*local_b8 + 0x30))();
        FUN_006b7bb0(7,0,0);
      }
      iVar2 = (**(code **)(*local_b8 + 0x18))();
      if (iVar2 != 0) {
        (**(code **)(*local_b8 + 0x18))();
      }
      local_8 = 0xffffffff;
      FUN_00733780();
    }
  }
  ExceptionList = local_10;
  return;
}

