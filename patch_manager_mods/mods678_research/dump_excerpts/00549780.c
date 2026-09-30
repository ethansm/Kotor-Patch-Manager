// FN 00549780 FUN_00549780 size=1861 ret=0 callers=0056efd0,005f2fe0 callees=00919723,0051e170,00735360,00515620,0071bcf0,00733570,00515d50,00735240,005f5160,00588d50,005812e0,005f0f10,00735400,00515dd0,005f6990,005453c0,005f67b0,0064b6a0,005f6630,0051c280,005eed40,0057e3f0,0051c2e0,00733780

undefined4 __fastcall FUN_00549780(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  float10 fVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  undefined4 *local_120;
  undefined4 *local_118;
  undefined1 local_a8 [16];
  undefined1 local_98 [8];
  int local_90;
  undefined1 local_8c [8];
  int local_84;
  undefined4 *local_80;
  undefined4 *local_7c;
  undefined4 *local_78;
  undefined4 *local_74;
  undefined4 *local_70;
  undefined4 *local_6c;
  int local_68;
  undefined4 *local_64;
  undefined1 local_60 [8];
  int local_58;
  undefined1 local_54 [11];
  char local_49;
  int local_48;
  uint local_44;
  int local_40;
  undefined4 *local_3c;
  undefined4 local_38;
  undefined1 local_34 [4];
  undefined4 local_30;
  uint local_2c;
  undefined4 *local_28;
  int local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fbb9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (((char)param_1[2] == DAT_00994474) || ((char)param_1[2] == DAT_00994478)) {
    local_24 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_40 = (**(code **)(*param_1 + 0x48))();
    if ((local_24 == 0) && (local_40 == 0)) {
      uVar1 = 0x7f000000;
    }
    else {
      iVar2 = param_1[1];
      FUN_0051c2e0(iVar2);
      FUN_0064b6a0(iVar2);
      iVar2 = FUN_005453c0();
      if (iVar2 == 0) {
        uVar1 = 0x7f000000;
      }
      else {
        local_2c = 0;
        if (local_24 == 0) {
          if (local_40 != 0) {
            iVar2 = FUN_005f67b0(0);
            if (iVar2 == 0) {
              ExceptionList = local_10;
              return 0x7f000000;
            }
            local_20 = FUN_005f6630();
          }
        }
        else {
          local_20 = FUN_00588d50();
          local_44 = (uint)*(byte *)(local_24 + 0x10ee);
          if (local_44 == 0) {
            FUN_00733570("BODY_BAG");
            local_8 = 0;
            iVar2 = FUN_0071bcf0(*(undefined2 *)(local_24 + 0x1184),local_54,&local_44);
            local_49 = '\x01' - (iVar2 != 0);
            local_8 = 0xffffffff;
            FUN_00733780();
            if (local_49 != '\0') {
              local_44 = 0;
            }
          }
          FUN_00733570("Corpse");
          local_8 = 1;
          local_58 = FUN_0071bcf0(local_44,local_60,&local_48);
          local_8 = 0xffffffff;
          FUN_00733780();
          if (local_58 != 0) {
            local_2c = (uint)(local_48 != 0);
          }
          if (((local_2c == 0) && (iVar2 = FUN_0057e3f0(1,0,0), iVar2 == 0)) &&
             (iVar2 = FUN_005812e0(), iVar2 == 0)) {
            ExceptionList = local_10;
            return 0x7f000000;
          }
        }
        local_68 = FUN_00919723(0x4b0);
        local_8 = 2;
        if (local_68 == 0) {
          local_118 = (undefined4 *)0x0;
        }
        else {
          local_118 = (undefined4 *)FUN_005eed40(0x7f000000);
        }
        local_64 = local_118;
        local_8 = 0xffffffff;
        local_3c = local_118;
        if (local_118 == (undefined4 *)0x0) {
          uVar1 = 0x7f000000;
        }
        else {
          iVar2 = FUN_005f0f10(local_20);
          if (iVar2 == 0) {
            local_70 = local_3c;
            local_6c = local_3c;
            if (local_3c != (undefined4 *)0x0) {
              (**(code **)*local_3c)(1);
            }
            uVar1 = 0x7f000000;
          }
          else {
            FUN_005f5160(param_1[1],1);
            local_3c[0x126] = 1;
            if (local_24 != 0) {
              local_3c[0x128] = param_1[1];
              local_3c[0x127] = 0;
            }
            local_78 = (undefined4 *)FUN_00919723(0x10);
            if (local_78 == (undefined4 *)0x0) {
              local_120 = (undefined4 *)0x0;
            }
            else {
              local_78[1] = 0;
              local_78[2] = 0;
              local_78[3] = 0;
              local_120 = local_78;
            }
            local_74 = local_120;
            local_28 = local_120;
            if (local_120 == (undefined4 *)0x0) {
              local_80 = local_3c;
              local_7c = local_3c;
              if (local_3c != (undefined4 *)0x0) {
                (**(code **)*local_3c)(1);
              }
              uVar1 = 0x7f000000;
            }
            else {
              *local_120 = local_3c[1];
              local_1c = param_1[0x25];
              local_18 = param_1[0x26];
              local_14 = param_1[0x27];
              local_120[1] = local_1c;
              local_120[2] = local_18;
              local_120[3] = local_14;
              FUN_00735240();
              local_8 = 3;
              if (local_24 == 0) {
                if (local_40 != 0) {
                  FUN_00733570(&DAT_0099353c);
                  local_8._0_1_ = 5;
                  local_90 = FUN_0071bcf0(*(undefined1 *)(local_40 + 0x3ec),local_98,&local_38);
                  local_8 = CONCAT31(local_8._1_3_,3);
                  FUN_00733780();
                  if (local_90 != 0) {
                    local_30 = local_38;
                    FUN_00735400(local_34);
                  }
                }
              }
              else {
                FUN_00733570(&DAT_0099353c);
                local_8._0_1_ = 4;
                local_84 = FUN_0071bcf0(*(undefined1 *)(local_24 + 0x10ee),local_8c,&local_38);
                local_8 = CONCAT31(local_8._1_3_,3);
                FUN_00733780();
                if (local_84 != 0) {
                  local_30 = local_38;
                  FUN_00735400(local_34);
                }
              }
              local_3c[0x129] = local_2c;
              local_3c[0xe3] = (uint)(local_2c == 0);
              if ((local_2c != 0) && (*(int *)(local_3c[0xf1] + 0x10) == 0)) {
                local_3c[0xdf] = 0;
              }
              local_3c[0x3f] = (uint)(local_2c == 0);
              fVar3 = (float10)FUN_00515dd0(param_1 + 0x28,0);
              fVar3 = (float10)FUN_00515d50(param_1 + 0x28,(float)fVar3);
              uVar1 = FUN_00515620(local_a8,(float)fVar3);
              FUN_005f6990(uVar1);
              iVar2 = FUN_005453c0();
              uVar1 = *(undefined4 *)(iVar2 + 0x124);
              iVar2 = param_1[1];
              uVar5 = 500;
              uVar4 = 0;
              uVar6 = DAT_00992438;
              puVar7 = local_28;
              FUN_0051c280(0,500,iVar2,uVar1,DAT_00992438,local_28);
              FUN_0051e170(uVar4,uVar5,iVar2,uVar1,uVar6,puVar7);
              local_3c[0x91] = 1;
              uVar1 = local_3c[1];
              local_8 = 0xffffffff;
              FUN_00735360();
            }
          }
        }
      }
    }
  }
  else {
    uVar1 = 0x7f000000;
  }
  ExceptionList = local_10;
  return uVar1;
}

