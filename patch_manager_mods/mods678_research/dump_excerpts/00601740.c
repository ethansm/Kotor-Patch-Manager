// FN 00601740 FUN_00601740 size=4784 ret=8 callers=00602be0,00601610 callees=00718760,00604140,00734690,007186e0,006014f0,006d6e30,00604260,0055e970,00718c80,006046c0,00735360,00718810,007337c0,00718b00,00718890,00718a00,00606d00,006d5200,006029f0,00603dd0,00603860,00733780,00718f90,00718c00,0091c860,007190e0,0051bd90,00603ee0,006041d0,009196ee,00735400,006042a0

/* WARNING: Removing unreachable block (ram,0x006018a0) */
/* WARNING: Removing unreachable block (ram,0x00601922) */

void __thiscall FUN_00601740(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined1 uVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  undefined2 *puVar5;
  undefined1 local_90 [8];
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined1 local_64 [11];
  char local_59;
  int local_58;
  undefined1 local_54 [4];
  int local_50;
  byte local_49;
  undefined4 local_48;
  uint local_44;
  int local_40;
  undefined4 local_3c;
  int local_38;
  undefined2 local_34;
  undefined1 local_30 [20];
  uint local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954eab;
  local_10 = ExceptionList;
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_80 = *(undefined4 *)(param_1 + 0xc);
  local_84 = FUN_00718c80(param_3,"BaseItem",&local_38,local_80);
  *(undefined4 *)(param_1 + 0xc) = local_84;
  local_88 = *(undefined4 *)(param_1 + 0x14);
  FUN_006014f0(local_88);
  FUN_00718f90(local_64,param_3,&DAT_00993538,&local_38,param_1 + 0x28);
  local_8 = 0;
  uVar2 = FUN_00734690(local_90);
  local_8._0_1_ = 1;
  FUN_007337c0(uVar2);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  uVar1 = FUN_00718a00(param_3,"Identified",&local_38,1);
  FUN_00606d00(uVar1);
  uVar2 = FUN_007190e0(local_6c,param_3,"Description",&local_38,param_1 + 0x2b0);
  local_8 = 2;
  FUN_00735400(uVar2);
  local_8 = 0xffffffff;
  FUN_00735360();
  uVar2 = FUN_007190e0(local_74,param_3,"DescIdentified",&local_38,param_1 + 0x2b8);
  local_8 = 3;
  FUN_00735400(uVar2);
  local_8 = 0xffffffff;
  FUN_00735360();
  uVar2 = FUN_007190e0(local_7c,param_3,"LocalizedName",&local_38,param_1 + 0x2c0);
  local_8 = 4;
  FUN_00735400(uVar2);
  local_8 = 0xffffffff;
  FUN_00735360();
  uVar3 = FUN_00718b00(param_3,"StackSize",&local_38,*(uint *)(param_1 + 0x2cc) & 0xffff);
  *(uint *)(param_1 + 0x2cc) = uVar3 & 0xffff;
  uVar3 = FUN_00718a00(param_3,"Stolen",&local_38,*(uint *)(param_1 + 0x2c8) >> 5 & 1);
  *(uint *)(param_1 + 0x2c8) = *(uint *)(param_1 + 0x2c8) & 0xffffffdf | (uVar3 & 1) << 5;
  uVar2 = FUN_00718c00(param_3,"Upgrades",&local_38,*(undefined4 *)(param_1 + 0x2d4));
  *(undefined4 *)(param_1 + 0x2d4) = uVar2;
  uVar3 = FUN_00718a00(param_3,"UpgradeLevel",&local_38,*(uint *)(param_1 + 0x2d4) & 0xff);
  *(uint *)(param_1 + 0x2f0) = uVar3 & 0xff;
  uVar2 = FUN_00718c80(param_3,"UpgradeSlot0",&local_38,0xffffffff);
  *(undefined4 *)(param_1 + 0x2f4) = uVar2;
  uVar2 = FUN_00718c80(param_3,"UpgradeSlot1",&local_38,0xffffffff);
  *(undefined4 *)(param_1 + 0x2f8) = uVar2;
  uVar2 = FUN_00718c80(param_3,"UpgradeSlot2",&local_38,0xffffffff);
  *(undefined4 *)(param_1 + 0x2fc) = uVar2;
  uVar2 = FUN_00718c80(param_3,"UpgradeSlot3",&local_38,0xffffffff);
  *(undefined4 *)(param_1 + 0x300) = uVar2;
  uVar2 = FUN_00718c80(param_3,"UpgradeSlot4",&local_38,0xffffffff);
  *(undefined4 *)(param_1 + 0x304) = uVar2;
  uVar2 = FUN_00718c80(param_3,"UpgradeSlot5",&local_38,0xffffffff);
  *(undefined4 *)(param_1 + 0x308) = uVar2;
  uVar3 = FUN_00718a00(param_3,"Dropable",&local_38,0);
  *(uint *)(param_1 + 0x2c8) = *(uint *)(param_1 + 0x2c8) & 0xfffffff7 | (uVar3 & 1) << 3;
  uVar3 = FUN_00718a00(param_3,"Pickpocketable",&local_38,0);
  *(uint *)(param_1 + 0x2c8) = *(uint *)(param_1 + 0x2c8) & 0xffffffef | (uVar3 & 1) << 4;
  uVar3 = FUN_00718a00(param_3,"NonEquippable",&local_38,*(uint *)(param_1 + 0x2c8) >> 6 & 1);
  *(uint *)(param_1 + 0x2c8) = *(uint *)(param_1 + 0x2c8) & 0xffffffbf | (uVar3 & 1) << 6;
  local_59 = FUN_00718a00(param_3,"ModelVariation",&local_38,*(undefined1 *)(param_1 + 0x2a4));
  if ((local_38 == 0) &&
     (local_59 = FUN_00718a00(param_3,"ModelPart1",&local_38,local_59), local_59 == '\0')) {
    local_59 = '\x01';
  }
  *(char *)(param_1 + 0x2a4) = local_59;
  iVar4 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  if (*(char *)(iVar4 + 0x18) == '\x01') {
    iVar4 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    *(undefined1 *)(param_1 + 0x2a5) = *(undefined1 *)(iVar4 + 0xaf);
    uVar1 = FUN_00718a00(param_3,"TextureVar",&local_38,1);
    *(undefined1 *)(param_1 + 0x2a6) = uVar1;
  }
  uVar3 = FUN_00718a00(param_3,"Charges",&local_38,0x32);
  *(uint *)(param_1 + 0x298) = uVar3 & 0xff;
  uVar3 = FUN_00718a00(param_3,"MaxCharges",&local_38,*(uint *)(param_1 + 0x298) & 0xff);
  *(uint *)(param_1 + 0x29c) = uVar3 & 0xff;
  uVar3 = FUN_00718a00(param_3,"NewItem",&local_38,*(uint *)(param_1 + 0x2c8) >> 7 & 1);
  *(uint *)(param_1 + 0x2c8) = *(uint *)(param_1 + 0x2c8) & 0xffffff7f | (uVar3 & 1) << 7;
  uVar3 = FUN_00718a00(param_3,"DELETING",&local_38,*(uint *)(param_1 + 0x2c8) >> 8 & 1);
  *(uint *)(param_1 + 0x2c8) = *(uint *)(param_1 + 0x2c8) & 0xfffffeff | (uVar3 & 1) << 8;
  uVar2 = FUN_00718c00(param_3,"AddCost",&local_38,*(undefined4 *)(param_1 + 0x2a0));
  *(undefined4 *)(param_1 + 0x2a0) = uVar2;
  iVar4 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  if (*(int *)(iVar4 + 0x70) != 0) {
    FUN_006029f0(param_2,param_3);
  }
  uVar3 = FUN_00718a00(param_3,&DAT_00996df4,&local_38,*(uint *)(param_1 + 0x10c) & 0xff);
  *(uint *)(param_1 + 0x10c) = uVar3 & 0xff;
  iVar4 = FUN_00718760(local_30,param_3,"PropertiesList");
  if ((iVar4 == 1) && (iVar4 = FUN_00718810(local_30), iVar4 != 0)) {
    local_58 = FUN_00718810(local_30);
    local_14 = 0;
    for (local_18 = 0; local_18 < local_58; local_18 = local_18 + 1) {
      FUN_00718890(local_54,local_30,local_18);
      iVar4 = FUN_007186e0(local_54);
      if (((iVar4 != 0) &&
          (local_34 = FUN_00718b00(local_54,"PropertyName",&local_38,0), local_38 != 0)) &&
         (iVar4 = FUN_00603ee0(local_34), iVar4 != 0)) {
        local_14 = local_14 + 1;
      }
    }
    FUN_00604140(local_14);
    local_40 = 0;
    for (local_18 = 0; local_18 < local_58; local_18 = local_18 + 1) {
      FUN_00718890(local_54,local_30,local_18);
      iVar4 = FUN_007186e0(local_54);
      if (iVar4 != 0) {
        local_34 = FUN_00718b00(local_54,"PropertyName",&local_38,0);
        iVar4 = FUN_00603ee0(local_34);
        if (iVar4 != 0) {
          if (local_38 != 0) {
            puVar5 = (undefined2 *)FUN_00604260(local_40);
            *puVar5 = local_34;
          }
          local_34 = FUN_00718b00(local_54,"Subtype",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_00604260(local_40);
            *(undefined2 *)(iVar4 + 2) = local_34;
          }
          local_49 = FUN_00718a00(local_54,"CostTable",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_00604260(local_40);
            *(byte *)(iVar4 + 4) = local_49;
          }
          local_34 = FUN_00718b00(local_54,"CostValue",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_00604260(local_40);
            *(undefined2 *)(iVar4 + 6) = local_34;
          }
          local_49 = FUN_00718a00(local_54,"Param1",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_00604260(local_40);
            *(byte *)(iVar4 + 8) = local_49;
          }
          local_49 = FUN_00718a00(local_54,"Param1Value",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_00604260(local_40);
            *(byte *)(iVar4 + 9) = local_49;
          }
          local_49 = FUN_00718a00(local_54,"ChanceAppear",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_00604260(local_40);
            *(byte *)(iVar4 + 10) = local_49;
          }
          local_49 = FUN_00718a00(local_54,"Useable",&local_38,1);
          iVar4 = FUN_00604260(local_40);
          *(uint *)(iVar4 + 0xc) = (uint)local_49;
          local_49 = FUN_00718a00(local_54,"UsesPerDay",&local_38,0xff);
          iVar4 = FUN_00604260(local_40);
          *(byte *)(iVar4 + 0x10) = local_49;
          local_49 = FUN_00718a00(local_54,"UpgradeType",&local_38,0xff);
          iVar4 = FUN_00604260(local_40);
          *(byte *)(iVar4 + 0x11) = local_49;
          local_34 = FUN_00718b00(local_54,"PropUpgrade",&local_38,0xff);
          iVar4 = FUN_00604260(local_40);
          *(undefined2 *)(iVar4 + 0x12) = local_34;
          local_40 = local_40 + 1;
        }
      }
    }
    FUN_006041d0(local_58 - *(int *)(param_1 + 0x288));
    local_50 = 0;
    for (local_18 = 0; local_18 < local_58; local_18 = local_18 + 1) {
      FUN_00718890(local_54,local_30,local_18);
      iVar4 = FUN_007186e0(local_54);
      if (iVar4 != 0) {
        local_34 = FUN_00718b00(local_54,"PropertyName",&local_38,0);
        iVar4 = FUN_00603ee0(local_34);
        if (iVar4 == 0) {
          if (local_38 != 0) {
            puVar5 = (undefined2 *)FUN_006042a0(local_50);
            *puVar5 = local_34;
          }
          local_34 = FUN_00718b00(local_54,"Subtype",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_006042a0(local_50);
            *(undefined2 *)(iVar4 + 2) = local_34;
          }
          local_49 = FUN_00718a00(local_54,"CostTable",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_006042a0(local_50);
            *(byte *)(iVar4 + 4) = local_49;
          }
          local_34 = FUN_00718b00(local_54,"CostValue",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_006042a0(local_50);
            *(undefined2 *)(iVar4 + 6) = local_34;
          }
          local_49 = FUN_00718a00(local_54,"Param1",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_006042a0(local_50);
            *(byte *)(iVar4 + 8) = local_49;
          }
          local_49 = FUN_00718a00(local_54,"Param1Value",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_006042a0(local_50);
            *(byte *)(iVar4 + 9) = local_49;
          }
          local_49 = FUN_00718a00(local_54,"ChanceAppear",&local_38,0);
          if (local_38 != 0) {
            iVar4 = FUN_006042a0(local_50);
            *(byte *)(iVar4 + 10) = local_49;
          }
          local_49 = FUN_00718a00(local_54,"Useable",&local_38,0);
          iVar4 = FUN_006042a0(local_50);
          *(uint *)(iVar4 + 0xc) = (uint)local_49;
          local_49 = FUN_00718a00(local_54,"UsesPerDay",&local_38,0xff);
          iVar4 = FUN_006042a0(local_50);
          *(byte *)(iVar4 + 0x10) = local_49;
          local_49 = FUN_00718a00(local_54,"UpgradeType",&local_38,0xff);
          iVar4 = FUN_006042a0(local_50);
          *(byte *)(iVar4 + 0x11) = local_49;
          local_34 = FUN_00718b00(local_54,"PropUpgrade",&local_38,0xff);
          iVar4 = FUN_006042a0(local_50);
          *(undefined2 *)(iVar4 + 0x12) = local_34;
          local_50 = local_50 + 1;
        }
      }
    }
  }
  FUN_00603dd0(1);
  FUN_00603860();
  if ((*(int *)(param_1 + 0x288) == 0) && (*(int *)(param_1 + 0x28c) == 0)) {
    FUN_00606d00(1);
  }
  uVar2 = FUN_006046c0();
  *(undefined4 *)(param_1 + 0x284) = uVar2;
  uVar2 = *(undefined4 *)(param_1 + 0x14);
  iVar4 = param_1 + 0x28;
  FUN_0051bd90(iVar4,uVar2);
  FUN_0055e970(iVar4,uVar2);
  iVar4 = FUN_006d6e30();
  local_44 = (uint)*(byte *)(iVar4 + 0x89);
  local_48 = 0x3f800000;
  local_3c = 0;
  for (local_18 = 0; local_18 < *(int *)(param_1 + 0x28c); local_18 = local_18 + 1) {
  }
  local_44 = FUN_0091c860();
  *(uint *)(param_1 + 0x2d0) = local_44;
  ExceptionList = local_10;
  __security_check_cookie(local_1c ^ (uint)&stack0xfffffffc);
  return;
}

