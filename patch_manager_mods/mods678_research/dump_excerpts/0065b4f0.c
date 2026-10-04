// FN 0065b4f0 FUN_0065b4f0 size=2138 ret=4 callers=0065be70 callees=0044b8a0,005143c0,00632a20,005915f0,005e36a0,005b7ae0,005718a0,004452b0,00563c90,005e3b60,006e3a40,00632ef0,005e3de0,0058fc60,0058fcd0,00564770,005e3720,005e3e40,005453c0,0051c0a0,006f9c90

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0065b4f0(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  float *pfVar4;
  uint uVar5;
  uint uVar6;
  float10 fVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  uint uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 *puVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  undefined4 uVar18;
  undefined4 uVar19;
  float local_128;
  float local_124;
  float local_120;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  float local_110;
  float local_10c;
  float local_108;
  undefined1 local_104 [12];
  undefined1 local_f8 [12];
  undefined1 local_ec [12];
  undefined1 local_e0 [12];
  float local_d4;
  float local_d0;
  float local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  ushort local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  undefined4 local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  int local_70;
  int local_6c;
  int *local_68;
  undefined1 local_64 [12];
  int local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  int local_48;
  undefined1 local_44 [12];
  int *local_38;
  int local_34;
  uint local_30;
  undefined4 local_2c;
  float local_28;
  float local_24;
  float local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  byte local_9;
  undefined4 local_8;
  
  local_38 = (int *)0x0;
  local_34 = FUN_00632ef0();
  fVar7 = (float10)FUN_005e3b60(0x3f800000,0x20);
  local_20 = (float)fVar7;
  fVar7 = (float10)FUN_005e3b60(0x3f800000,0x20);
  local_24 = (float)fVar7;
  fVar7 = (float10)FUN_005e3b60(0x3f800000,0x20);
  local_28 = (float)fVar7;
  local_30 = FUN_005e3720(8);
  local_30 = local_30 & 0xff;
  local_1c = FUN_005e36a0();
  local_18 = FUN_005e36a0();
  local_9 = FUN_005e3720(8);
  local_8 = FUN_00632ef0();
  iVar1 = FUN_005e3de0(1);
  if ((iVar1 != 0) || (iVar1 = FUN_005e3e40(1), iVar1 != 0)) {
    return 0;
  }
  local_38 = (int *)FUN_00632a20();
  local_14 = DAT_00994484;
  iVar1 = FUN_00632a20();
  if (iVar1 == 0) {
    return 0;
  }
  if ((local_38 != (int *)0x0) && (iVar1 = FUN_005453c0(), iVar1 != 0)) {
    piVar2 = (int *)FUN_00632a20();
    iVar1 = (**(code **)(*piVar2 + 0x30))();
    if (iVar1 != 0) {
      piVar2 = (int *)FUN_00632a20();
      iVar1 = (**(code **)(*piVar2 + 0x30))();
      if (*(int *)(iVar1 + 0x10f0) != 0) {
        return 0;
      }
      piVar2 = (int *)FUN_00632a20();
      iVar1 = (**(code **)(*piVar2 + 0x30))();
      local_58 = *(int *)(iVar1 + 0x380);
      if (*(int *)(local_58 + 0x84) == 0) {
        local_b4 = *(undefined4 *)(local_58 + 100);
        local_b8 = *(undefined4 *)(local_58 + 0x60);
        local_bc = *(undefined4 *)(local_58 + 0x5c);
        local_48 = *(int *)(local_58 + 0x70);
        local_54 = local_bc;
        local_50 = local_b8;
        local_4c = local_b4;
      }
      else {
        local_c0 = *(undefined4 *)(local_58 + 0x7c);
        local_c4 = *(undefined4 *)(local_58 + 0x78);
        local_c8 = *(undefined4 *)(local_58 + 0x74);
        local_48 = *(int *)(local_58 + 0x80);
        local_54 = local_c8;
        local_50 = local_c4;
        local_4c = local_c0;
      }
      local_d4 = local_20;
      local_d0 = local_24;
      local_cc = local_28;
      FUN_004452b0(local_44,&local_d4,&local_54);
      if ((local_48 == local_34) &&
         (fVar7 = (float10)FUN_0044b8a0(local_44,local_44), fVar7 < (float10)_DAT_00987538)) {
        *(uint *)(local_58 + 0x28) = local_30 & 0xff;
        return 1;
      }
      FUN_006f9c90();
      if (local_9 == 1) {
        *(undefined4 *)(local_58 + 0x194) = 1;
      }
    }
    local_2c = 0;
    iVar1 = (**(code **)(*local_38 + 0x30))();
    if (iVar1 != 0) {
      local_7c = local_20;
      local_78 = local_24;
      local_74 = local_28;
      local_6c = (**(code **)(*local_38 + 0x30))();
      local_70 = 0;
      FUN_004452b0(local_64,local_6c + 0x94,&local_7c);
      fVar7 = (float10)FUN_0044b8a0(local_64,local_64);
      if ((fVar7 < (float10)_DAT_00996430) && (*(char *)(local_6c + 0x1116) == '\x02')) {
        local_80 = _DAT_009988e8;
        local_94 = 0.0;
        local_90 = 0.0;
        local_8c = 0.0;
        local_ac = 0.0;
        local_a8 = 0.0;
        local_a4 = 0.0;
        local_a0 = 0.0;
        local_9c = 0.0;
        local_98 = 0.0;
        local_84 = *(undefined4 *)(local_6c + 0x1118);
        local_68 = (int *)FUN_0051c0a0(local_84);
        if ((local_68 != (int *)0x0) && (iVar1 = (**(code **)(*local_68 + 0x10))(), iVar1 != 0)) {
          local_70 = (**(code **)(*local_68 + 0x10))();
        }
        if (local_70 == 0) {
          return 1;
        }
        FUN_0058fc60(1,*(undefined4 *)(local_6c + 4));
        FUN_0058fcd0(500,0);
        uVar3 = FUN_004452b0(local_e0,local_70 + 0x94,local_6c + 0x94);
        pfVar4 = (float *)FUN_005143c0(local_ec,uVar3);
        local_a0 = *pfVar4;
        local_9c = pfVar4[1];
        local_98 = pfVar4[2];
        uVar3 = FUN_004452b0(local_f8,&local_7c,local_6c + 0x94);
        pfVar4 = (float *)FUN_005143c0(local_104,uVar3);
        local_94 = *pfVar4;
        local_90 = pfVar4[1];
        local_8c = local_98;
        fVar7 = (float10)FUN_0044b8a0(&local_a0,&local_94);
        local_88 = (float)fVar7;
        if (local_80 < local_88 == (local_80 == local_88)) {
          if (local_88 < -local_80 == (local_88 == -local_80)) {
            local_110 = local_8c * local_9c - local_90 * local_98;
            local_10c = local_94 * local_98 - local_8c * local_a0;
            local_108 = local_90 * local_a0 - local_94 * local_9c;
            local_ac = local_110;
            local_a8 = local_10c;
            local_a4 = local_108;
            if (local_108 < (float)_DAT_00986b38) {
              FUN_006e3a40(5,500,local_84);
            }
            else {
              FUN_006e3a40(4,500,local_84);
            }
          }
          else {
            FUN_006e3a40(3,500,local_84);
          }
        }
        else {
          FUN_006e3a40(2,500,local_84);
        }
        return 1;
      }
    }
    local_10 = (**(code **)(*local_38 + 0x30))();
    if ((local_10 != 0) && (*(int *)(local_10 + 0xec) == 1)) {
      local_b0 = 0xffff;
      FUN_005718a0();
      if (*(char *)(local_10 + 0x1116) == '\x01') {
        local_b0 = 0xfffe;
      }
      uVar18 = 1;
      uVar3 = 0;
      (**(code **)(*local_38 + 0x30))(0,1);
      FUN_005b7ae0(uVar3,uVar18);
      (**(code **)(*local_38 + 0x30))();
      FUN_005915f0();
      uVar3 = 8;
      (**(code **)(*local_38 + 0x30))(8);
      FUN_00563c90(uVar3);
      local_11c = 0;
      local_118 = 0;
      local_114 = 0;
      local_128 = local_20;
      local_124 = local_24;
      local_120 = local_28;
      uVar19 = 0;
      uVar17 = 0;
      uVar16 = 0;
      puVar15 = &local_11c;
      uVar14 = 0;
      uVar13 = 0;
      uVar6 = (uint)local_9;
      uVar12 = 0;
      uVar9 = 0;
      pfVar4 = &local_128;
      uVar5 = (uint)local_b0;
      iVar1 = local_34;
      uVar3 = local_8;
      uVar18 = local_1c;
      uVar8 = local_2c;
      uVar10 = local_18;
      uVar11 = local_30;
      (**(code **)(*local_38 + 0x30))
                (uVar5,pfVar4,local_34,local_8,local_1c,local_2c,0,local_18,local_30,0,uVar6,0,0,
                 puVar15,0,0,0);
      FUN_00564770(uVar5,pfVar4,iVar1,uVar3,uVar18,uVar8,uVar9,uVar10,uVar11,uVar12,uVar6,uVar13,
                   uVar14,puVar15,uVar16,uVar17,uVar19);
    }
    return 1;
  }
  return 0;
}

