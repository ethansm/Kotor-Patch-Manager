// FN 008cdb00 FUN_008cdb00 size=2043 ret=4 callers= callees=008cb290,004122a0,008cd2c0,0071b1d0,00604ec0,005f7a30,0051c0e0,00919723,007338d0,00615e90,00600380,0051c8d0,004142e0,0049b840,00710810,00606d00,00605e80,00601610,006d6e30,005ffd60,0041ffb0,008ce300,00615d60,009370c7,00733780,00605090,00733570,0051c8f0,00733540,0083ea60,00734010

void __thiscall FUN_008cdb00(int param_1,int param_2)

{
  char cVar1;
  uint uVar2;
  undefined4 uVar3;
  char *_Str2;
  char *_Str1;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int local_d8;
  undefined1 local_84 [8];
  undefined4 *local_7c;
  undefined4 *local_78;
  undefined1 local_74 [16];
  int local_64;
  int local_60;
  undefined1 local_5c [8];
  int local_54;
  int local_50;
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  int local_28;
  undefined4 local_24;
  int local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097b6fb;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_2 + 0x50) != 0) {
    if (*(int *)(param_2 + 0x4c) == 0x2d) {
      FUN_004122a0(0);
    }
    local_20 = 0;
    if (param_2 == param_1 + 0x3860) {
      local_20 = FUN_0041ffb0(uVar2);
    }
    else {
      local_20 = param_2;
    }
    if (local_20 != 0) {
      if ((*(uint *)(local_20 + 0x1dc) >> 1 & 1) == 0) {
        local_14 = *(undefined4 *)(DAT_00a1b4a4 + 8);
        FUN_0051c8d0();
        local_24 = FUN_005f7a30();
        local_1c = *(int *)(*(int *)(param_1 + 0x3dbc) + 0x5c);
        local_18 = 1;
        if ((local_1c == 1) && (*(char *)(param_1 + 0x3d2c) == '\x01')) {
          if (*(int *)(local_20 + 0x1e0) == -1) {
            local_18 = 0;
          }
          else {
            FUN_00733540();
            local_8 = 0;
            FUN_00733540();
            local_8 = CONCAT31(local_8._1_3_,1);
            iVar4 = FUN_006d6e30();
            cVar1 = *(char *)(iVar4 + 0xac);
            if (cVar1 == '\'') {
              FUN_007338d0("DoubleMdlVar");
            }
            else if (cVar1 == '(') {
              FUN_007338d0("ShortMdlVar");
            }
            else if (cVar1 == ')') {
              FUN_007338d0("LongMdlVar");
            }
            FUN_0071b1d0(*(undefined4 *)(local_20 + 0x1e0),local_38,local_30);
            local_64 = FUN_00919723(0x30c);
            local_8._0_1_ = 2;
            if (local_64 == 0) {
              local_d8 = 0;
            }
            else {
              local_d8 = FUN_005ffd60(0x7f000000);
            }
            local_60 = local_d8;
            local_8 = CONCAT31(local_8._1_3_,1);
            local_3c = local_d8;
            FUN_00710810(local_30);
            FUN_00601610(local_74);
            uVar7 = 0x7f000000;
            uVar6 = 0;
            uVar5 = 0;
            uVar3 = FUN_0051c8f0(0,0,0x7f000000);
            FUN_00600380(uVar3,uVar5,uVar6,uVar7);
            FUN_00606d00(1);
            *(undefined4 *)(local_3c + 0x2d4) = *(undefined4 *)(*(int *)(param_1 + 0x3d34) + 0x2d4);
            for (local_40 = 0; local_40 < 6; local_40 = local_40 + 1) {
              if (local_40 != 1) {
                FUN_00615d60(*(undefined4 *)(param_1 + 0x3d54 + local_40 * 4),local_40);
                *(undefined4 *)(local_3c + 0x2f4 + local_40 * 4) =
                     *(undefined4 *)(*(int *)(param_1 + 0x3d34) + 0x2f4 + local_40 * 4);
              }
            }
            *(uint *)(local_3c + 0x2c8) =
                 *(uint *)(local_3c + 0x2c8) & 0xffffffdf |
                 (*(uint *)(*(int *)(param_1 + 0x3d34) + 0x2c8) >> 5 & 1) << 5;
            local_7c = *(undefined4 **)(param_1 + 0x3d34);
            local_78 = local_7c;
            if (local_7c == (undefined4 *)0x0) {
              uVar3 = 0;
            }
            else {
              uVar3 = (**(code **)*local_7c)(1);
            }
            *(int *)(param_1 + 0x3d34) = local_3c;
            FUN_008cd2c0(uVar2,uVar3);
            local_8 = local_8 & 0xffffff00;
            FUN_00733780();
            local_8 = 0xffffffff;
            FUN_00733780();
          }
        }
        else if ((*(int *)(local_20 + 0x1e0) == -1) &&
                (local_44 = *(int *)(local_20 + 0x1d0), local_44 != 0x7f000000)) {
          local_18 = 0;
        }
        if (local_18 != 0) {
          if (*(int *)(param_1 + 0x3d54 + local_1c * 4) != 0) {
            uVar7 = 0x7f000000;
            uVar6 = 0;
            uVar5 = 0;
            uVar3 = FUN_0051c8f0(0,0,0x7f000000);
            FUN_00600380(uVar3,uVar5,uVar6,uVar7);
            local_48 = FUN_004142e0(*(undefined4 *)(param_1 + 0x3d54 + local_1c * 4));
            if (local_48 == -1) {
              FUN_00605090(param_1 + 0x3d54 + local_1c * 4,1,0,0);
              FUN_0083ea60(*(undefined4 *)(param_1 + 0x3d54 + local_1c * 4));
            }
            else {
              FUN_0049b840(local_48);
              FUN_00605090(param_1 + 0x3d54 + local_1c * 4,1,0,0);
            }
            if ((local_1c != 1) || (*(char *)(param_1 + 0x3d2c) != '\x01')) {
              FUN_00615e90(*(undefined4 *)(param_1 + 0x3d54 + local_1c * 4),local_1c);
              *(undefined4 *)(*(int *)(param_1 + 0x3d34) + 0x2f4 + local_1c * 4) = 0xffffffff;
              *(undefined4 *)(*(int *)(param_1 + 0x3d54 + local_1c * 4) + 0x2f4 + local_1c * 4) =
                   0xffffffff;
            }
            *(undefined4 *)(param_1 + 0x3d54 + local_1c * 4) = 0;
          }
          local_28 = FUN_0051c0e0(*(undefined4 *)(local_20 + 0x1d0));
          if (local_28 != 0) {
            local_48 = FUN_004142e0(local_28);
            if (local_48 == -1) {
              if (*(int *)(local_28 + 0x2cc) < 2) {
                FUN_00605e80(local_28);
              }
              else {
                local_28 = FUN_00604ec0(1);
              }
              FUN_0083ea60(local_28);
            }
            else {
              FUN_0049b840(local_48);
              if (*(int *)(local_28 + 0x2cc) < 2) {
                FUN_00605e80(local_28);
              }
              else {
                local_28 = FUN_00604ec0(1);
              }
            }
            local_50 = *(int *)(*(int *)(param_1 + 0x3db4) + 0x24);
            local_54 = 0;
            for (local_4c = 0; local_4c < local_50; local_4c = local_4c + 1) {
              FUN_00733540();
              local_8 = 3;
              FUN_00733570("Template");
              local_8._0_1_ = 4;
              FUN_0071b1d0(local_4c,local_84,local_5c);
              local_8 = CONCAT31(local_8._1_3_,3);
              FUN_00733780();
              _Str2 = (char *)FUN_00734010();
              _Str1 = (char *)FUN_00734010();
              iVar4 = __stricmp(_Str1,_Str2);
              if (iVar4 == 0) {
                local_54 = local_4c;
                local_8 = 0xffffffff;
                FUN_00733780();
                break;
              }
              local_8 = 0xffffffff;
              FUN_00733780();
            }
            *(int *)(param_1 + 0x3d54 + local_1c * 4) = local_28;
            if ((local_1c != 1) || (*(char *)(param_1 + 0x3d2c) != '\x01')) {
              *(undefined4 *)(param_1 + 0x3d9c + local_1c * 4) = *(undefined4 *)(local_20 + 0x1e0);
              FUN_00615d60(local_28,local_1c);
              *(int *)(*(int *)(param_1 + 0x3d34) + 0x2f4 + local_1c * 4) = local_54;
            }
          }
        }
        FUN_008cb290();
      }
      else {
        FUN_008ce300(0);
      }
    }
  }
  ExceptionList = local_10;
  return;
}

