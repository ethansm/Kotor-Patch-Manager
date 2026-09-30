// FN 006d7390 FUN_006d7390 size=8297 ret=4 callers=005d56b0 callees=005644b0,0057c990,0051c910,00584da0,00540ca0,004452b0,005716a0,0044c8a0,0091dba0,005142f0,005f5dd0,0044b8a0,005143c0,00590ff0,0051c720,00734f60,00517980,0052bb60,005912b0,00571570,0056e1e0,007e5120,0061d320,006dacf0,006e29b0,007e5da0,005453c0,00585f10,0071b3f0,0057d370,0057e3d0,0073fb90,006dac70,0073fec0,006b83f0,0077d800,0058fc60,0058fcd0,007e6be0,0051c100,006dab20,0051c0a0,0058e730,006d0620,00436200,00590de0,0052be60,006d9410,005908c0,006dab80

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_006d7390(int *param_1,int param_2)

{
  bool bVar1;
  short sVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  undefined4 *puVar6;
  float10 fVar7;
  int *piVar8;
  undefined4 *puVar9;
  int iVar10;
  undefined4 uVar11;
  undefined4 local_26c;
  undefined4 local_268;
  undefined4 local_264;
  undefined4 local_260;
  undefined4 local_25c;
  undefined4 local_258;
  undefined1 local_254 [12];
  undefined4 local_248;
  undefined1 local_244 [12];
  undefined1 local_238 [12];
  float local_22c;
  float local_228;
  float local_224;
  undefined1 local_220 [12];
  undefined1 local_214 [12];
  undefined1 local_208 [12];
  undefined1 local_1fc [12];
  undefined4 local_1f0;
  undefined4 local_1ec;
  undefined4 local_1e8;
  undefined4 local_1e4;
  undefined4 local_1e0;
  undefined4 local_1dc;
  int local_1d8;
  int local_1d4;
  int local_1d0;
  int local_1cc;
  int local_1c8;
  int local_1c4;
  int local_1c0;
  undefined4 local_1bc;
  float local_1b8;
  int local_1b4;
  int local_1b0;
  undefined4 local_1ac;
  undefined4 local_1a8;
  float local_1a4 [3];
  int local_198;
  int local_194;
  float local_190;
  float local_18c;
  int local_188;
  int local_184;
  float local_180;
  int local_17c;
  uint local_178;
  undefined4 local_174;
  int local_170;
  int local_16c;
  int local_168;
  float local_164;
  undefined1 local_160 [8];
  undefined4 local_158;
  undefined1 local_154 [12];
  undefined4 local_148;
  undefined4 local_144;
  undefined4 local_140;
  undefined1 local_13c [4];
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_130;
  int local_f0;
  int local_ec;
  int local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined1 local_dc [4];
  undefined4 local_d8;
  int local_d4;
  int local_d0;
  int local_cc;
  float local_c8;
  int local_c4;
  float local_c0;
  int local_bc;
  undefined4 local_b8;
  float local_b4;
  int local_b0;
  int local_ac;
  float local_a8;
  int local_a4;
  int local_a0;
  int local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  float local_80;
  float local_7c;
  float local_78;
  undefined4 local_74;
  uint local_70;
  int local_6c;
  int local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  int *local_54;
  float local_50;
  undefined4 local_4c;
  undefined4 local_48;
  int local_44;
  int local_40;
  int local_3c;
  short local_38 [2];
  int local_34;
  float local_30;
  int *local_2c;
  undefined4 local_28;
  undefined4 local_24;
  int local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  int local_10;
  int local_c;
  float local_8;
  
  local_5c = 0x7f000000;
  local_3c = 0;
  local_10 = 0;
  local_c = 0;
  local_8 = 0.0;
  local_30 = 0.0;
  local_50 = 0.0;
  local_48 = 1;
  *(undefined4 *)(param_1[0xe0] + 0xc) = 0;
  *(undefined4 *)(param_1[0xe0] + 0x38) = 0x7f000000;
  param_1[0x15c] = 3000;
  local_20 = 0;
  local_18 = 0x7f000000;
  local_38[0] = 0;
  local_44 = 0;
  local_14 = 0;
  local_4c = 0;
  local_28 = 0;
  local_24 = 4;
  local_58 = 0;
  local_60 = 0;
  local_1c = 0;
  _memcpy(&local_20,(void *)(param_2 + 0x38),4);
  local_1c = local_1c + 1;
  _memcpy(&local_18,(void *)(param_2 + 0x38 + local_1c * 4),4);
  local_1c = local_1c + 1;
  _memcpy(&local_44,(void *)(param_2 + 0x38 + local_1c * 4),4);
  local_1c = local_1c + 1;
  _memcpy(&local_14,(void *)(param_2 + 0x38 + local_1c * 4),4);
  local_1c = local_1c + 1;
  _memcpy(&local_4c,(void *)(param_2 + 0x38 + local_1c * 4),4);
  local_1c = local_1c + 1;
  _memcpy(&local_28,(void *)(param_2 + 0x38 + local_1c * 4),4);
  local_1c = local_1c + 1;
  _memcpy(local_38,(void *)(param_2 + 0x38 + local_1c * 4),2);
  local_1c = local_1c + 1;
  if (local_20 != 0) {
    _memcpy(&local_58,(void *)(param_2 + 0x38 + local_1c * 4),4);
    local_1c = local_1c + 1;
    _memcpy(&local_24,(void *)(param_2 + 0x38 + local_1c * 4),4);
    local_1c = local_1c + 1;
    _memcpy(&local_60,(void *)(param_2 + 0x38 + local_1c * 4),4);
    local_1c = local_1c + 1;
  }
  if (param_1[0x46b] != 0) {
    local_64 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    uVar3 = FUN_0073fec0(param_1[1]);
    FUN_0073fb90(uVar3);
    local_68 = FUN_007e6be0(uVar3);
    if (-1 < local_68) {
      iVar4 = FUN_0073fb90();
      local_6c = iVar4 + 0x24 + local_68 * 0x88;
      FUN_007e5120(0x1b2c7);
    }
  }
  iVar4 = (**(code **)(*param_1 + 0x94))();
  if (iVar4 != 0) {
LAB_006d77ac:
    FUN_006dab20(1);
    return DAT_009944bc;
  }
  if (param_1[0x46b] == 0) {
    bVar1 = false;
  }
  else {
    sVar2 = (**(code **)(*param_1 + 0x9c))(0);
    bVar1 = sVar2 < 1;
  }
  if ((((bVar1) || ((*(ushort *)(param_1 + 0x445) & 0x80) != 0x80)) ||
      ((*(ushort *)(param_1 + 0x445) & 4) != 4)) || (local_18 == param_1[1])) goto LAB_006d77ac;
  local_34 = 0;
  local_2c = (int *)FUN_0051c0a0(local_18);
  local_54 = (int *)0x0;
  if ((local_2c != (int *)0x0) && (iVar4 = (**(code **)(*local_2c + 0x10))(), iVar4 != 0)) {
    local_54 = (int *)(**(code **)(*local_2c + 0x10))();
    local_70 = 0;
    iVar4 = (**(code **)(*local_54 + 0x30))();
    if (iVar4 != 0) {
      piVar5 = (int *)(**(code **)(*local_54 + 0x30))();
      if (piVar5[0x46b] == 0) {
        local_70 = 0;
      }
      else {
        sVar2 = (**(code **)(*piVar5 + 0x9c))(0);
        local_70 = (uint)(sVar2 < 1);
      }
    }
    iVar4 = (**(code **)(*local_54 + 0x94))();
    if ((iVar4 == 0) && (local_70 == 0)) {
      local_34 = FUN_006dab80(local_18,1);
    }
  }
  local_40 = 0;
  if (local_34 == 0) {
    return DAT_009944b8;
  }
  local_10 = local_54[0x25];
  local_c = local_54[0x26];
  local_8 = (float)local_54[0x27];
  local_3c = FUN_005453c0();
  local_a0 = FUN_00590de0(local_18);
  if (local_a0 != 0) {
    local_14 = 0x277d;
  }
  iVar4 = (**(code **)(*local_54 + 0x18))();
  if (iVar4 != 0) {
    local_d8 = 0;
    *(int *)(param_1[0xe0] + 0x38) = local_18;
    FUN_00585f10(local_18,&local_10,&local_d8,1,0);
    *(undefined4 *)(param_1[0xe0] + 0xc) = local_d8;
  }
  fVar7 = (float10)FUN_00571570(local_18,local_a0,0);
  local_30 = (float)fVar7;
  fVar7 = (float10)FUN_005716a0(local_18,local_a0);
  local_50 = (float)fVar7;
  local_d4 = 0;
  local_74 = 0x7f000000;
  local_b0 = param_1[0x25];
  local_ac = param_1[0x26];
  local_a8 = (float)param_1[0x27] + (float)_DAT_00989f10;
  local_d0 = 0;
  local_cc = 0;
  local_c8 = 0.0;
  if (((char)local_54[2] == DAT_00994479) || ((char)local_54[2] == DAT_00994478)) {
    FUN_00585f10(local_54[1],&local_d0,local_dc,0,0);
    local_10 = local_d0;
    local_c = local_cc;
    local_8 = local_c8;
  }
  else {
    local_d0 = local_54[0x25];
    local_cc = local_54[0x26];
    local_c8 = (float)local_54[0x27];
  }
  local_c8 = local_c8 + (float)_DAT_00989f10;
  local_8c = 0;
  local_88 = 0;
  local_84 = 0;
  iVar4 = FUN_005453c0();
  if (iVar4 != 0) {
    local_a8 = local_a8 + (float)_DAT_00987538;
    local_c8 = local_c8 - (float)_DAT_00987538;
    iVar4 = param_1[1];
    uVar11 = 0;
    uVar3 = FUN_0056e1e0(0);
    puVar6 = &local_74;
    puVar9 = &local_8c;
    piVar5 = &local_d0;
    piVar8 = &local_b0;
    iVar10 = local_18;
    FUN_005453c0(piVar8,piVar5,puVar9,puVar6,iVar4,local_18,uVar3);
    local_d4 = FUN_0052bb60(piVar8,piVar5,puVar9,puVar6,iVar4,iVar10,uVar3,uVar11);
    iVar4 = FUN_0056e1e0();
    if (((iVar4 == 0) && (local_d4 != 0)) && (iVar4 = (**(code **)(*local_54 + 0x30))(), iVar4 != 0)
       ) {
      local_e8 = param_1[0xe0];
      local_e4 = *(undefined4 *)(local_e8 + 8);
      local_e0 = *(undefined4 *)(local_e8 + 0x30);
      *(int *)(local_e8 + 0x30) = local_18;
      *(undefined4 *)(local_e8 + 8) = _DAT_0098c628;
      *(undefined4 *)(local_e8 + 0x30) = local_e0;
      *(undefined4 *)(local_e8 + 8) = local_e4;
    }
    if (local_d4 == 0) {
      iVar4 = FUN_00517980(param_1 + 0x156,param_1 + 0x25);
      if ((iVar4 != 0) && (param_1[0x43a] != 0)) {
        FUN_00584da0(0xda,0);
        local_1e4 = 0;
        local_1e0 = 0;
        local_1dc = 0;
        param_1[0x156] = 0;
        param_1[0x157] = 0;
        param_1[0x158] = 0;
        param_1[0x152] = 0x7f000000;
        uVar3 = 0x7f000000;
        iVar4 = local_18;
        fVar7 = (float10)FUN_00571570(param_1[1],0,1);
        local_ec = FUN_0057c990((float)fVar7,iVar4,uVar3);
        if (local_ec == 0x7f000000) {
          FUN_006dab20(1);
          return DAT_009944b8;
        }
        local_2c = (int *)FUN_0051c0a0(local_ec);
        if ((local_2c != (int *)0x0) && (*(char *)((int)local_2c + 8) == DAT_00994474)) {
          FUN_006dac70(local_ec,param_2);
          return DAT_009944b4;
        }
        FUN_006dab20(1);
        return DAT_009944b8;
      }
      param_1[0x156] = param_1[0x25];
      param_1[0x157] = param_1[0x26];
      param_1[0x158] = param_1[0x27];
    }
    else {
      local_1f0 = 0;
      local_1ec = 0;
      local_1e8 = 0;
      param_1[0x156] = 0;
      param_1[0x157] = 0;
      param_1[0x158] = 0;
    }
  }
  FUN_004452b0(&local_80,param_1 + 0x25,&local_10);
  local_78 = 0.0;
  fVar7 = (float10)FUN_0044b8a0(&local_80,&local_80);
  local_c0 = (float)fVar7;
  local_98 = 0;
  local_94 = 0;
  local_90 = 0;
  local_b8 = 0;
  if ((local_c0 < local_50 * local_50 - (float)_DAT_0098d898) ||
     ((local_54 != (int *)0x0 && (iVar4 = (**(code **)(*local_54 + 0x18))(), iVar4 != 0)))) {
    iVar4 = (**(code **)(*local_54 + 0x18))();
    if (iVar4 == 0) {
      local_22c = -local_80;
      local_228 = -local_7c;
      local_224 = -local_78;
      FUN_005143c0(local_154,&local_22c);
      uVar3 = FUN_00436200(local_238,local_154,local_50);
      puVar6 = (undefined4 *)FUN_004452b0(local_244,local_54 + 0x25,uVar3);
      local_98 = *puVar6;
      local_94 = puVar6[1];
      local_90 = puVar6[2];
    }
    else {
      FUN_004452b0(&local_148,&local_10,local_54 + 0x25);
      puVar6 = (undefined4 *)FUN_005143c0(local_1fc,&local_148);
      local_148 = *puVar6;
      local_144 = puVar6[1];
      local_140 = puVar6[2];
      uVar3 = FUN_00436200(local_208,&local_148,local_50,_DAT_0098bd68);
      uVar3 = FUN_00436200(local_214,uVar3);
      puVar6 = (undefined4 *)FUN_0044c8a0(local_220,&local_10,uVar3);
      local_98 = *puVar6;
      local_94 = puVar6[1];
      local_90 = puVar6[2];
    }
    FUN_0052be60();
    local_248 = *(undefined4 *)(param_1[0xe0] + 4);
    uVar3 = *(undefined4 *)(param_1[0xe0] + 0x14);
    iVar4 = FUN_005453c0();
    local_f0 = FUN_0051c910(param_1[1],iVar4 + 0x110,param_1 + 0x25,&local_98,&local_248,uVar3,1,
                            local_13c);
    if (local_f0 != 1) {
      local_98 = local_138;
      local_94 = local_134;
      local_90 = local_130;
      uVar3 = FUN_004452b0(local_254,local_54 + 0x25,&local_98);
      fVar7 = (float10)FUN_005142f0(uVar3);
      local_50 = (float)fVar7;
      local_b8 = 1;
    }
  }
  local_b4 = 0.0;
  if ((local_54 != (int *)0x0) && (iVar4 = (**(code **)(*local_54 + 0x18))(), iVar4 != 0)) {
    FUN_004452b0(local_160,param_1 + 0x25,&local_98);
    local_158 = 0;
    fVar7 = (float10)FUN_0044b8a0(local_160,local_160);
    local_b4 = (float)fVar7;
  }
  local_9c = 1;
  if ((local_54 != (int *)0x0) && (iVar4 = (**(code **)(*local_54 + 0x30))(), iVar4 != 0)) {
    local_164 = 0.0;
    FUN_0071b3f0((short)param_1[0x461],DAT_00a0fdf0,&local_164);
    if (local_164 < (float)_DAT_0098c1e8) {
      local_9c = 0;
    }
  }
  if ((local_9c != 0) &&
     (((((iVar4 = FUN_005453c0(), local_3c != iVar4 ||
         ((local_30 + (float)_DAT_00987df0) * (local_30 + (float)_DAT_00987df0) < local_c0)) ||
        ((local_c0 < local_50 * local_50 - (float)_DAT_0098d898 && (param_1[0x46b] == 0)))) ||
       (local_d4 == 0)) ||
      (((local_54 != (int *)0x0 && (iVar4 = (**(code **)(*local_54 + 0x18))(), iVar4 != 0)) &&
       (local_50 * local_50 < local_b4)))))) {
    local_168 = FUN_0056e1e0();
    local_16c = 0;
    if ((local_c0 < local_50 * local_50 - (float)_DAT_0098d898) ||
       (((local_54 != (int *)0x0 && (iVar4 = (**(code **)(*local_54 + 0x18))(), iVar4 != 0)) &&
        (local_50 * local_50 < local_b4)))) {
      local_16c = 1;
    }
    if (param_1[0x14b] != 0) {
      FUN_006dab20(1);
      return DAT_009944bc;
    }
    if (local_3c == 0) {
      iVar4 = (**(code **)(*local_54 + 0x30))();
      if ((iVar4 == 0) || (param_1[0x452] != 0x7f000000)) {
        FUN_006dab20(1);
        return DAT_009944bc;
      }
      iVar4 = (**(code **)(*local_54 + 0x30))();
      local_5c = *(undefined4 *)(iVar4 + 0x350);
      iVar4 = (**(code **)(*local_54 + 0x30))();
      local_10 = *(int *)(iVar4 + 0x354);
      local_c = *(int *)(iVar4 + 0x358);
      local_8 = *(float *)(iVar4 + 0x35c);
    }
    else {
      local_5c = *(undefined4 *)(local_3c + 0x124);
    }
    if (local_d4 != 0) {
      local_40 = FUN_006e29b0(local_54,local_38);
    }
    if (local_40 == 0) {
      local_178 = (uint)*(ushort *)(param_1[0x466] + 0x146);
      uVar3 = 0;
      FUN_0073fb90(0);
      local_17c = FUN_007e5da0(uVar3);
      local_170 = 0x7f000000;
      if (local_17c != 0) {
        iVar4 = FUN_0077d800();
        local_170 = *(int *)(iVar4 + 4);
      }
      iVar4 = (**(code **)(*local_2c + 0x18))();
      if (iVar4 == 0) {
        iVar4 = (**(code **)(*local_2c + 0x48))();
        if (iVar4 != 0) {
          FUN_00585f10(local_18,&local_10,local_1a4 + 2,1,0);
          local_50 = local_1a4[2];
          piVar8 = param_1 + 0x25;
          piVar5 = &local_198;
          (**(code **)(*local_2c + 0x48))(piVar5,piVar8);
          FUN_005f5dd0(piVar5,piVar8);
          local_10 = local_198;
          local_c = local_194;
          local_8 = local_190;
          local_16c = 0;
        }
      }
      else {
        FUN_00585f10(local_18,&local_10,&local_18c,1,0);
        local_50 = local_18c;
        uVar3 = 0;
        piVar8 = param_1 + 0x25;
        piVar5 = &local_188;
        (**(code **)(*local_2c + 0x18))(piVar5,piVar8,0);
        FUN_0061d320(piVar5,piVar8,uVar3);
        local_10 = local_188;
        local_c = local_184;
        local_8 = local_180;
        *(int *)(param_1[0xe0] + 0x38) = local_18;
        *(float *)(param_1[0xe0] + 0xc) = local_18c;
      }
      FUN_00540ca0(0xc,*(undefined2 *)(param_2 + 0x6c),1,&local_20,3,&local_18,1,&local_44,1,
                   &local_14,1,&local_4c,1,&local_28,1,local_38,1,&local_58,1,&local_24,1,&local_60,
                   0,0,0,0,0,0);
      local_174 = 1;
      if (local_16c == 0) {
        local_1a4[1] = 0.0;
        local_1a4[0] = local_50;
        if (local_168 != 0) {
          local_1a4[0] = local_30;
        }
        if (((local_178 != 0xb) && (local_178 != 0xc)) || (local_170 == param_1[1])) {
          FUN_00540ca0(0x11,*(undefined2 *)(param_2 + 0x6c),3,&local_18,1,&local_174,2,local_1a4,2,
                       &local_30,1,&local_48,1,local_1a4 + 1,1,&local_168,0,0,0,0,0,0,0,0,0,0,0,0);
        }
        FUN_00540ca0(0x13,*(undefined2 *)(param_2 + 0x6c),3,&local_18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0);
      }
      if (local_16c != 0) {
        local_1a8 = 0;
        if (((local_54 != (int *)0x0) && (iVar4 = (**(code **)(*local_54 + 0x18))(), iVar4 != 0)) &&
           ((float)_DAT_0098b968 < local_b4)) {
          local_1a8 = 1;
        }
        local_260 = 0;
        local_25c = 0;
        local_258 = 0;
        FUN_005644b0(*(undefined2 *)(param_2 + 0x6c),&local_98,local_5c,0x7f000000,local_1a8,0,0,0,0
                     ,0,0,0,0,&local_260,0,0,0);
        if (param_1[0x46b] == 0) {
          return DAT_009944b8;
        }
        if (local_c0 <= (float)_DAT_0098c1d8) {
          return DAT_009944b8;
        }
        local_1ac = *(undefined4 *)(DAT_00a1b4a4 + 4);
        uVar3 = FUN_0073fec0(param_1[1]);
        FUN_0073fb90(uVar3);
        local_1b0 = FUN_007e6be0(uVar3);
        if (local_1b0 < 0) {
          return DAT_009944b8;
        }
        iVar4 = FUN_0073fb90();
        local_1b4 = iVar4 + 0x24 + local_1b0 * 0x88;
        FUN_007e5120(0xa5ed);
        return DAT_009944b8;
      }
      if (local_40 != 0) {
        return DAT_009944b8;
      }
      local_1b8 = local_50;
      if (local_168 != 0) {
        local_1b8 = local_30;
      }
      iVar4 = (**(code **)(*local_54 + 0x30))();
      if (iVar4 != 0) {
        iVar4 = (**(code **)(*local_54 + 0x30))();
        *(float *)(param_1[0xe0] + 0xc) = local_50 - *(float *)(*(int *)(iVar4 + 0x380) + 8);
        *(int *)(param_1[0xe0] + 0x38) = local_18;
      }
      if (((local_178 == 0xb) || (local_178 == 0xc)) && (local_170 != param_1[1])) {
        return DAT_009944b8;
      }
      local_26c = 0;
      local_268 = 0;
      local_264 = 0;
      FUN_005644b0(*(undefined2 *)(param_2 + 0x6c),&local_10,local_5c,local_18,local_174,local_1b8,0
                   ,0,0,0,0,0,0,&local_26c,0,0,local_168);
      if (param_1[0x46b] == 0) {
        return DAT_009944b8;
      }
      if (local_c0 <= (float)_DAT_0098c1d8) {
        return DAT_009944b8;
      }
      local_1bc = *(undefined4 *)(DAT_00a1b4a4 + 4);
      uVar3 = FUN_0073fec0(param_1[1]);
      FUN_0073fb90(uVar3);
      local_1c0 = FUN_007e6be0(uVar3);
      if (local_1c0 < 0) {
        return DAT_009944b8;
      }
      iVar4 = FUN_0073fb90();
      local_1c4 = iVar4 + 0x24 + local_1c0 * 0x88;
      FUN_007e5120(0xa5ed);
      return DAT_009944b8;
    }
  }
  if (param_1[0x437] == 0) {
    FUN_006dab20(1);
    return DAT_009944bc;
  }
  iVar4 = FUN_0051c720(2);
  if (iVar4 != 0) {
    return DAT_009944b4;
  }
  if (*(int *)(param_1[0x437] + 0xaa8) != 0) {
    return DAT_009944b4;
  }
  local_a4 = FUN_00590ff0(local_18);
  local_bc = FUN_005912b0(local_18,&local_a4);
  if (local_a4 == 0) {
    local_14 = 0x2719;
    FUN_0058e730(local_18,0,local_bc,local_38[0],local_20);
    goto LAB_006d8e26;
  }
  iVar4 = FUN_006d9410();
  if (iVar4 == 0) {
    iVar4 = (**(code **)(*local_54 + 0x30))();
    if (iVar4 != 0) {
      (**(code **)(*local_54 + 0x30))();
      iVar4 = FUN_006d9410();
      if (iVar4 != 0) goto LAB_006d8d0f;
    }
  }
  else {
LAB_006d8d0f:
    local_14 = 0x277d;
  }
  if (local_bc != 0) {
    FUN_0058e730(local_18,local_a4,local_bc,local_38[0],local_20);
    local_1c8 = FUN_0051c100(local_18);
    if (local_1c8 != 0) {
      FUN_0058e730(param_1[1],local_a4,local_bc == 0,0,local_20);
      *(undefined4 *)(local_1c8 + 0x51c) = 1;
    }
  }
LAB_006d8e26:
  if (*(int *)(param_1[0x437] + 0xa84) == 0) {
    return DAT_009944b4;
  }
  if (((local_44 == 0xb) || (local_40 != 0)) && ((short)param_1[0x3fa] == 0)) {
    local_1cc = FUN_006d0620(0x10);
    if (local_1cc == 0) {
      if ((local_44 == 0xb) && (local_38[0] == 0xdc)) {
        iVar4 = FUN_005908c0(*(undefined1 *)(param_1[0x437] + 0xabc));
        *(short *)(iVar4 + 100) = local_38[0];
      }
      else {
        iVar4 = FUN_006b83f0(0xcf);
        if ((iVar4 != 0) && ((local_44 == 0xb && (local_40 == 0)))) {
          iVar4 = FUN_005908c0(*(undefined1 *)(param_1[0x437] + 0xabc));
          *(short *)(iVar4 + 100) = local_38[0];
        }
      }
    }
    else {
      iVar4 = FUN_005908c0(*(undefined1 *)(param_1[0x437] + 0xabc));
      *(short *)(iVar4 + 100) = local_38[0];
    }
  }
  (**(code **)(*param_1 + 0x7c))(local_14);
  FUN_0057e3d0(local_18);
  FUN_0058fc60(1,param_1[1]);
  FUN_0058fcd0(local_4c,0);
  *(int *)(param_1[0x437] + 0xab8) = *(int *)(param_1[0x437] + 0xab8) + 1;
  if (local_a4 != 0) {
    if (local_bc == 0) {
      local_1d4 = FUN_0051c100(local_18);
      if (local_1d4 != 0) {
        FUN_0058fc60(1,*(undefined4 *)(*(int *)(local_1d4 + 0x10dc) + 0xb14));
        FUN_0058fcd0(local_4c,0);
      }
    }
    else {
      local_1d0 = FUN_0051c100(*(undefined4 *)(param_1[0x437] + 0xb14));
      if (local_1d0 != 0) {
        FUN_0058fc60(1,*(undefined4 *)(*(int *)(local_1d0 + 0x10dc) + 0xb10));
        FUN_0058fcd0(local_4c,1);
      }
    }
  }
  if (*(int *)(param_1[0x437] + 0xadc) != 0x7f000000) {
    local_1d8 = *(int *)(param_1[0x437] + 0xadc);
    if (local_1d8 == 0x7f000000) {
      FUN_0058fc60(0,0x7f000000);
      FUN_0058fcd0(0,0);
      FUN_006dab20(1);
      return DAT_009944b8;
    }
    FUN_006dac70(local_1d8,param_2);
    *(undefined4 *)(param_1[0x437] + 0xadc) = 0x7f000000;
    local_18 = local_1d8;
    local_34 = 1;
  }
  FUN_0057d370(local_18,0);
  local_c4 = 0;
  if ((local_9c == 0) && (iVar4 = FUN_00734f60("g_turretbl002"), iVar4 != 0)) {
    local_c4 = 1;
  }
  if (local_9c == 0) {
    if (local_c4 != 0) {
      return DAT_009944b8;
    }
    if ((local_30 + (float)_DAT_00987df0) * (local_30 + (float)_DAT_00987df0) <= local_c0) {
      return DAT_009944b8;
    }
  }
  FUN_006dacf0(local_18,local_28,local_4c,local_20,local_58,local_24,local_60);
  return DAT_009944b8;
}

