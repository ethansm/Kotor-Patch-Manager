// FN 007b40f0 FUN_007b40f0 size=6909 ret=0 callers=007b3a00,007b12c0 callees=00733570,0071bcf0,005453c0,007408f0,0058c3b0,00710eb0,00845730,0077d800,0077e0c0,00733780,0073f4d0,0051c600,007ecfa0,0073f750,007aae00,007ed7d0,0073fb90,007e5d80,00842d00,007337c0,0051c8f0,0073fec0,00733540,0087be80

void __fastcall FUN_007b40f0(int param_1)

{
  uint *puVar1;
  undefined4 *puVar2;
  bool bVar3;
  short sVar4;
  int iVar5;
  int *piVar6;
  undefined4 uVar7;
  int *local_3dc;
  undefined1 local_258 [16];
  code *local_248;
  undefined4 local_244;
  undefined4 local_240;
  undefined4 local_23c;
  undefined1 local_238 [8];
  undefined1 local_230 [16];
  code *local_220;
  undefined4 local_21c;
  undefined4 local_218;
  undefined4 local_214;
  undefined1 local_210 [8];
  undefined1 local_208 [16];
  code *local_1f8;
  undefined4 local_1f4;
  undefined4 local_1f0;
  undefined4 local_1ec;
  undefined1 local_1e8 [8];
  undefined1 local_1e0 [16];
  code *local_1d0;
  undefined4 local_1cc;
  undefined4 local_1c8;
  undefined4 local_1c4;
  undefined1 local_1c0 [8];
  undefined1 local_1b8 [16];
  code *local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  undefined4 local_19c;
  undefined1 local_198 [8];
  undefined1 local_190 [16];
  code *local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined1 local_170 [8];
  undefined1 local_168 [16];
  code *local_158;
  undefined4 local_154;
  undefined4 local_150;
  undefined4 local_14c;
  undefined1 local_148 [8];
  undefined1 local_140 [16];
  code *local_130;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined1 local_120 [8];
  undefined1 local_118 [16];
  code *local_108;
  undefined4 local_104;
  undefined4 local_100;
  undefined4 local_fc;
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
  int local_e8;
  undefined1 local_e4 [16];
  code *local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined1 local_c4 [8];
  undefined1 local_bc [16];
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined1 local_9c [8];
  int local_94;
  int local_90;
  undefined4 local_8c;
  undefined4 local_88;
  int local_7c;
  int *local_78;
  undefined4 local_74;
  int local_70;
  int local_6c;
  int local_68;
  int *local_64;
  int local_60;
  undefined4 local_5c;
  int local_58;
  undefined4 local_54;
  int local_50;
  int local_4c;
  undefined4 local_48;
  int local_3c;
  int local_38;
  int local_34;
  int *local_30;
  int local_2c;
  undefined4 local_28;
  int *local_24;
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00968554;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_2c = 0;
  FUN_0073fb90(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_20 = FUN_007e5d80();
  if (local_20 == 0) {
    local_3dc = (int *)0x0;
  }
  else {
    local_3dc = (int *)FUN_0077d800();
  }
  local_14 = local_3dc;
  iVar5 = FUN_0073f750();
  local_18 = *(undefined4 *)(*(int *)(iVar5 + 0x98) + 0x68);
  local_30 = (int *)FUN_0073f4d0(local_18);
  local_24 = (int *)0x0;
  local_1c = 1;
  if (local_20 != 0) {
    if ((local_14 != (int *)0x0) && (iVar5 = (**(code **)(*local_14 + 0x94))(), iVar5 == 0)) {
      if (local_14[0x46b] == 0) {
        bVar3 = false;
      }
      else {
        sVar4 = (**(code **)(*local_14 + 0x9c))(0);
        bVar3 = sVar4 < 1;
      }
      if (!bVar3) {
        local_1c = 0;
      }
    }
    if (local_30 != (int *)0x0) {
      local_24 = (int *)(**(code **)(*local_30 + 0xc))();
    }
    if (local_24 != (int *)0x0) {
      local_28 = FUN_0077e0c0(local_24);
      for (local_34 = 0; local_34 < *(int *)(param_1 + 0x4d0); local_34 = local_34 + 1) {
        FUN_007ecfa0();
      }
      FUN_0058c3b0(0);
      switch(local_28) {
      case 0:
        FUN_0058c3b0(1);
        uVar7 = FUN_007408f0(local_9c,0x7dec);
        local_8 = 0;
        FUN_007337c0(uVar7);
        local_8 = 0xffffffff;
        FUN_00733780();
        local_ac = 0;
        local_a8 = 0;
        local_a4 = 0;
        local_a0 = 0xffffffff;
        iVar5 = *(int *)(param_1 + 0x4cc);
        *(undefined4 *)(iVar5 + 0xc) = 0;
        *(undefined4 *)(iVar5 + 0x10) = 0;
        *(undefined4 *)(iVar5 + 0x14) = 0;
        *(undefined4 *)(iVar5 + 0x18) = 0xffffffff;
        *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x1c) = 0x7f000000;
        *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8) = 0x404;
        FUN_00710eb0(local_bc,"i_noaction");
        break;
      case 1:
      case 3:
        iVar5 = (**(code **)(*local_24 + 0x2c))();
        if (iVar5 != 0) {
          (**(code **)(*local_24 + 0x2c))();
          local_38 = FUN_0077d800();
          if ((local_38 != 0) && (*(int *)(local_38 + 600) == 0)) {
            FUN_0058c3b0(local_2c + 1);
            uVar7 = FUN_007408f0(local_c4,0x173);
            local_8 = 1;
            FUN_007337c0(uVar7);
            local_8 = 0xffffffff;
            FUN_00733780();
            local_d4 = FUN_0077cf70;
            local_d0 = 0;
            local_cc = 0;
            local_c8 = 0;
            puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
            *puVar2 = FUN_0077cf70;
            puVar2[1] = 0;
            puVar2[2] = 0;
            puVar2[3] = 0;
            *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
            *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3ea;
            FUN_00710eb0(local_e4,"i_dialog");
          }
        }
        iVar5 = (**(code **)(*local_24 + 0x4c))();
        if (iVar5 == 0) {
          iVar5 = (**(code **)(*local_24 + 0x14))();
          if (iVar5 != 0) {
            local_88 = *(undefined4 *)(DAT_00a1b4a4 + 4);
            (**(code **)(*local_24 + 0x14))();
            local_8c = FUN_0087be80();
            iVar5 = FUN_007aae00();
            local_90 = *(int *)(iVar5 + 0x2e4);
            FUN_00733540();
            local_8 = 8;
            (**(code **)(*local_24 + 0x14))();
            sVar4 = FUN_007ed7d0();
            if ((sVar4 == 0x2742) || (sVar4 = FUN_007ed7d0(), sVar4 == 0x2743)) {
              bVar3 = true;
            }
            else {
              bVar3 = false;
            }
            if ((!bVar3) &&
               (iVar5 = (**(code **)(*local_24 + 0x14))(), *(int *)(iVar5 + 0x13c) == 0)) {
              FUN_0058c3b0(local_2c + 1);
              uVar7 = FUN_007408f0(local_198,0x16d);
              local_8._0_1_ = 9;
              FUN_007337c0(uVar7);
              local_8._0_1_ = 8;
              FUN_00733780();
              *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3f2;
              local_1a8 = FUN_0087bb20;
              local_1a4 = 0;
              local_1a0 = 0;
              local_19c = 0;
              puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
              *puVar2 = FUN_0087bb20;
              puVar2[1] = 0;
              puVar2[2] = 0;
              puVar2[3] = 0;
              *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
              FUN_00710eb0(local_1b8,"i_opendoor");
              local_2c = local_2c + 1;
              iVar5 = (**(code **)(*local_24 + 0x14))();
              if ((*(int *)(iVar5 + 0x104) == 0) &&
                 ((local_90 == 0 &&
                  (iVar5 = (**(code **)(*local_24 + 0x14))(), *(int *)(iVar5 + 0x10c) != 0)))) {
                FUN_0058c3b0(local_2c + 1);
                uVar7 = FUN_007408f0(local_1c0,0x170);
                local_8._0_1_ = 10;
                FUN_007337c0(uVar7);
                local_8 = CONCAT31(local_8._1_3_,8);
                FUN_00733780();
                *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3f5;
                local_1d0 = FUN_0087bd10;
                local_1cc = 0;
                local_1c8 = 0;
                local_1c4 = 0;
                puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
                *puVar2 = FUN_0087bd10;
                puVar2[1] = 0;
                puVar2[2] = 0;
                puVar2[3] = 0;
                *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
                FUN_00710eb0(local_1e0,"i_attack");
                local_2c = local_2c + 1;
              }
            }
            local_8 = 0xffffffff;
            FUN_00733780();
          }
        }
        else {
          local_48 = *(undefined4 *)(DAT_00a1b4a4 + 4);
          (**(code **)(*local_24 + 0x4c))();
          local_3c = FUN_00842d00();
          (**(code **)(*local_24 + 0x4c))();
          iVar5 = FUN_007aae00();
          local_4c = *(int *)(iVar5 + 0x2e4);
          FUN_00733540();
          local_8 = 2;
          if ((local_3c == 0) || (*(int *)(local_3c + 0x37c) == 0)) {
            iVar5 = (**(code **)(*local_24 + 0x4c))();
            if ((*(int *)(iVar5 + 300) != 0) &&
               ((local_3c != 0 && (*(int *)(local_3c + 0x4a4) == 0)))) {
              FUN_0058c3b0(local_2c + 1);
              uVar7 = FUN_007408f0(local_148,0x16e);
              local_8._0_1_ = 6;
              FUN_007337c0(uVar7);
              local_8 = CONCAT31(local_8._1_3_,2);
              FUN_00733780();
              *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3f7;
              local_158 = FUN_00841ce0;
              local_154 = 0;
              local_150 = 0;
              local_14c = 0;
              puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
              *puVar2 = FUN_00841ce0;
              puVar2[1] = 0;
              puVar2[2] = 0;
              puVar2[3] = 0;
              *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
              FUN_00710eb0(local_168,"i_useplace");
              local_2c = local_2c + 1;
            }
            iVar5 = (**(code **)(*local_24 + 0x4c))();
            if ((*(int *)(iVar5 + 0x110) == 0) && (local_4c == 0)) {
              FUN_0058c3b0(local_2c + 1);
              uVar7 = FUN_007408f0(local_170,0x170);
              local_8._0_1_ = 7;
              FUN_007337c0(uVar7);
              local_8 = CONCAT31(local_8._1_3_,2);
              FUN_00733780();
              *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3f5;
              local_180 = FUN_00841d40;
              local_17c = 0;
              local_178 = 0;
              local_174 = 0;
              puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
              *puVar2 = FUN_00841d40;
              puVar2[1] = 0;
              puVar2[2] = 0;
              puVar2[3] = 0;
              *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
              FUN_00710eb0(local_190,"i_attack");
              local_2c = local_2c + 1;
            }
          }
          else {
            iVar5 = (**(code **)(*local_24 + 0x4c))();
            if (*(int *)(iVar5 + 300) != 0) {
              FUN_0058c3b0(local_2c + 1);
              local_54 = 0x16d;
              FUN_00733570("UseSearch");
              local_8._0_1_ = 3;
              iVar5 = (**(code **)(*local_24 + 0x4c))();
              local_e8 = FUN_0071bcf0(*(undefined4 *)(iVar5 + 0x10c),local_f0,&local_60);
              local_8._0_1_ = 2;
              FUN_00733780();
              if (local_e8 == 0) {
                if (*(int *)(local_3c + 0x498) != 0) {
                  local_54 = 0xbc36;
                }
              }
              else if (local_60 != 0) {
                local_54 = 0xbc36;
              }
              uVar7 = FUN_007408f0(local_f8,local_54);
              local_8._0_1_ = 4;
              FUN_007337c0(uVar7);
              local_8 = CONCAT31(local_8._1_3_,2);
              FUN_00733780();
              *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3f7;
              local_108 = FUN_00841ce0;
              local_104 = 0;
              local_100 = 0;
              local_fc = 0;
              puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
              *puVar2 = FUN_00841ce0;
              puVar2[1] = 0;
              puVar2[2] = 0;
              puVar2[3] = 0;
              *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
              FUN_00710eb0(local_118,"i_openplace");
              local_5c = FUN_0051c8f0();
              local_58 = FUN_0051c600(local_5c);
              local_50 = FUN_0073fec0(*(undefined4 *)(local_58 + 0x38));
              local_64 = (int *)FUN_0073f4d0(local_50);
              if ((((local_64 != (int *)0x0) &&
                   (iVar5 = (**(code **)(*local_64 + 0x2c))(), iVar5 != 0)) &&
                  (local_68 = (**(code **)(*local_64 + 0x2c))(), *(int *)(local_20 + 4) != local_50)
                  ) && ((*(int *)(local_68 + 0x3bc) != 0 && (iVar5 = FUN_0077d800(), iVar5 != 0))))
              {
                piVar6 = (int *)FUN_0077d800();
                if (piVar6[0x46b] == 0) {
                  bVar3 = false;
                }
                else {
                  sVar4 = (**(code **)(*piVar6 + 0x9c))(0);
                  bVar3 = sVar4 < 1;
                }
                if (bVar3) {
                  (**(code **)(*local_24 + 0x4c))();
                  iVar5 = FUN_00842d00();
                  if (iVar5 != 0) {
                    (**(code **)(*local_24 + 0x4c))();
                    iVar5 = FUN_00842d00();
                    if (*(int *)(iVar5 + 0x244) == 0) {
                      puVar1 = (uint *)(*(int *)(param_1 + 0x4cc) + 0x30 + local_2c * 0x3c);
                      *puVar1 = *puVar1 & 0xfffffffe;
                      puVar1 = (uint *)(*(int *)(param_1 + 0x4cc) + 0x30 + local_2c * 0x3c);
                      *puVar1 = *puVar1 & 0xffffffe1 | 0xc;
                    }
                  }
                }
              }
              local_2c = local_2c + 1;
            }
            iVar5 = (**(code **)(*local_24 + 0x4c))();
            if (((*(int *)(iVar5 + 0x110) == 0) && (local_4c == 0)) &&
               (iVar5 = (**(code **)(*local_24 + 0x4c))(), *(int *)(iVar5 + 0x118) != 0)) {
              FUN_0058c3b0(local_2c + 1);
              uVar7 = FUN_007408f0(local_120,0x170);
              local_8._0_1_ = 5;
              FUN_007337c0(uVar7);
              local_8 = CONCAT31(local_8._1_3_,2);
              FUN_00733780();
              *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3f5;
              local_130 = FUN_00841d40;
              local_12c = 0;
              local_128 = 0;
              local_124 = 0;
              puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
              *puVar2 = FUN_00841d40;
              puVar2[1] = 0;
              puVar2[2] = 0;
              puVar2[3] = 0;
              *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
              FUN_00710eb0(local_140,"i_attack");
              local_74 = FUN_0051c8f0();
              local_70 = FUN_0051c600(local_74);
              local_6c = FUN_0073fec0(*(undefined4 *)(local_70 + 0x38));
              local_78 = (int *)FUN_0073f4d0(local_6c);
              if ((((local_78 != (int *)0x0) &&
                   (iVar5 = (**(code **)(*local_78 + 0x2c))(), iVar5 != 0)) &&
                  (local_7c = (**(code **)(*local_78 + 0x2c))(), *(int *)(local_20 + 4) != local_6c)
                  ) && ((*(int *)(local_7c + 0x3bc) != 0 && (iVar5 = FUN_0077d800(), iVar5 != 0))))
              {
                piVar6 = (int *)FUN_0077d800();
                if (piVar6[0x46b] == 0) {
                  bVar3 = false;
                }
                else {
                  sVar4 = (**(code **)(*piVar6 + 0x9c))(0);
                  bVar3 = sVar4 < 1;
                }
                if (bVar3) {
                  (**(code **)(*local_24 + 0x4c))();
                  iVar5 = FUN_00842d00();
                  if (iVar5 != 0) {
                    (**(code **)(*local_24 + 0x4c))();
                    iVar5 = FUN_00842d00();
                    if (*(int *)(iVar5 + 0x244) == 0) {
                      puVar1 = (uint *)(*(int *)(param_1 + 0x4cc) + 0x30 + local_2c * 0x3c);
                      *puVar1 = *puVar1 & 0xfffffffe;
                      puVar1 = (uint *)(*(int *)(param_1 + 0x4cc) + 0x30 + local_2c * 0x3c);
                      *puVar1 = *puVar1 & 0xffffffe1 | 0xc;
                    }
                  }
                }
              }
              local_2c = local_2c + 1;
            }
          }
          local_8 = 0xffffffff;
          FUN_00733780();
        }
        break;
      case 2:
        iVar5 = (**(code **)(*local_24 + 0x3c))();
        if (iVar5 != 0) {
          iVar5 = FUN_00845730(1);
          if (iVar5 != 0) {
            piVar6 = (int *)(**(code **)(*local_24 + 0x3c))();
            iVar5 = (**(code **)(*piVar6 + 0x138))();
            if (iVar5 != 0) {
              FUN_0058c3b0(local_2c + 1);
              uVar7 = FUN_007408f0(local_1e8,0x172);
              local_8 = 0xb;
              FUN_007337c0(uVar7);
              local_8 = 0xffffffff;
              FUN_00733780();
              local_1f8 = FUN_0087f2d0;
              local_1f4 = 0;
              local_1f0 = 0;
              local_1ec = 0;
              puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
              *puVar2 = FUN_0087f2d0;
              puVar2[1] = 0;
              puVar2[2] = 0;
              puVar2[3] = 0;
              *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
              *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3f4;
              FUN_00710eb0(local_208,"i_disablemine");
              local_2c = local_2c + 1;
            }
          }
          iVar5 = FUN_00845730(1);
          if (iVar5 != 0) {
            FUN_0058c3b0(local_2c + 1);
            uVar7 = FUN_007408f0(local_210,0x5fb);
            local_8 = 0xc;
            FUN_007337c0(uVar7);
            local_8 = 0xffffffff;
            FUN_00733780();
            local_220 = FUN_0087f330;
            local_21c = 0;
            local_218 = 0;
            local_214 = 0;
            puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
            *puVar2 = FUN_0087f330;
            puVar2[1] = 0;
            puVar2[2] = 0;
            puVar2[3] = 0;
            *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
            *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x402;
            FUN_00710eb0(local_230,"i_recovermine");
          }
        }
        break;
      case 4:
        iVar5 = (**(code **)(*local_24 + 0x2c))();
        if (iVar5 != 0) {
          (**(code **)(*local_24 + 0x2c))();
          local_94 = FUN_0077d800();
          if ((local_94 != 0) && (iVar5 = FUN_005453c0(), *(int *)(iVar5 + 0x2e4) == 0)) {
            FUN_0058c3b0(local_2c + 1);
            uVar7 = FUN_007408f0(local_238,0x177);
            local_8 = 0xd;
            FUN_007337c0(uVar7);
            local_8 = 0xffffffff;
            FUN_00733780();
            local_248 = FUN_0077ce00;
            local_244 = 0;
            local_240 = 0;
            local_23c = 0;
            puVar2 = (undefined4 *)(*(int *)(param_1 + 0x4cc) + 0xc + local_2c * 0x3c);
            *puVar2 = FUN_0077ce00;
            puVar2[1] = 0;
            puVar2[2] = 0;
            puVar2[3] = 0;
            *(int *)(*(int *)(param_1 + 0x4cc) + 0x1c + local_2c * 0x3c) = local_24[1];
            *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 8 + local_2c * 0x3c) = 0x3eb;
            FUN_00710eb0(local_258,"i_attack");
          }
        }
      }
    }
  }
  ExceptionList = local_10;
  return;
}

