// FN 0055ac80 FUN_0055ac80 size=2985 ret=8 callers=0055a900 callees=0044b8a0,005143c0,0045cfe0,00550e30,0091cb24,009196fd,006fa3f0,004452b0,0051c100,00557ef0,00550980,00919723,005532f0,0044c8a0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_0055ac80(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  float *pfVar2;
  int iVar3;
  undefined4 *puVar4;
  float10 fVar5;
  float local_1c4;
  float local_1c0;
  undefined4 local_1bc;
  undefined1 local_1b8 [12];
  undefined1 local_1ac [12];
  undefined1 local_1a0 [12];
  float *local_194;
  float local_190;
  float local_18c;
  undefined4 local_188;
  float local_184;
  float local_180;
  undefined4 local_17c;
  float local_178;
  float local_174;
  undefined4 local_170;
  float local_16c;
  float local_168;
  undefined4 local_164;
  undefined1 local_160 [12];
  undefined1 local_154 [12];
  undefined1 local_148 [12];
  undefined1 local_13c [12];
  undefined1 local_130 [12];
  undefined1 local_124 [12];
  undefined1 local_118 [12];
  undefined1 local_10c [12];
  int local_100;
  int local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  float *local_ec;
  float local_e8;
  int local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  int local_ac;
  undefined4 local_a8;
  int local_a4;
  undefined4 local_a0;
  int local_9c;
  undefined4 local_98;
  int local_94;
  undefined4 local_90;
  int local_8c;
  int local_88;
  float *local_84;
  float local_80;
  float local_7c;
  float local_78;
  int local_74;
  float local_70;
  float local_6c;
  float local_68;
  undefined4 local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  undefined4 local_44;
  undefined1 local_40 [12];
  float local_34;
  float local_30;
  undefined4 local_2c;
  float local_28;
  int local_24;
  int local_20;
  int local_1c;
  float local_18;
  int local_14;
  float local_10;
  float local_c;
  int local_8;
  
  local_c = (float)param_1[0x1d];
  local_18 = (float)param_1[0x1e];
  local_28 = (float)param_1[0x17];
  local_10 = (float)param_1[0x18];
  local_14 = param_1[1];
  local_8 = param_1[5];
  local_20 = FUN_00557ef0();
  local_1c = DAT_009944bc;
  if ((local_20 != 0) && (*(int *)(local_20 + 0x124) == param_1[0x20])) {
    param_1[0xf] = param_1[0xf] + 1;
    *(int **)(local_20 + 0x1b0) = param_1;
    if (((float)_DAT_00986b70 < (float)param_1[0x1a]) && (param_1[0x90] == 0)) {
      local_34 = local_c;
      local_30 = local_18;
      local_2c = 0;
      local_4c = local_28;
      local_48 = local_10;
      local_44 = 0;
      local_58 = local_28;
      local_54 = local_10;
      local_50 = 0.0;
      FUN_004452b0(local_40,&local_4c,&local_34);
      fVar5 = (float10)FUN_0044b8a0(local_40,local_40);
      local_60 = (float)fVar5;
      local_5c = (float)param_1[0x1a];
      if ((float)_DAT_00994dc8 <= local_60) {
        if (local_60 <= local_5c * local_5c) {
          fVar5 = (float10)FUN_0091cb24((double)local_60);
          uVar1 = FUN_004452b0(local_13c,&local_34,&local_4c);
          uVar1 = FUN_005143c0(local_148,uVar1);
          uVar1 = FUN_0045cfe0(local_154,(float)fVar5,uVar1);
          pfVar2 = (float *)FUN_0044c8a0(local_160,&local_4c,uVar1);
          local_58 = *pfVar2;
          local_54 = pfVar2[1];
          local_50 = pfVar2[2];
        }
        else {
          uVar1 = FUN_004452b0(local_10c,&local_34,&local_4c);
          uVar1 = FUN_005143c0(local_118,uVar1);
          uVar1 = FUN_0045cfe0(local_124,local_5c - (float)_DAT_00986b70,uVar1);
          pfVar2 = (float *)FUN_0044c8a0(local_130,&local_4c,uVar1);
          local_58 = *pfVar2;
          local_54 = pfVar2[1];
          local_50 = pfVar2[2];
        }
      }
      local_28 = local_58;
      local_10 = local_54;
    }
    if (param_1[0xf] == 1) {
      local_16c = local_28;
      local_168 = local_10;
      local_164 = 0;
      local_178 = local_c;
      local_174 = local_18;
      local_170 = 0;
      local_24 = FUN_00550980(&local_178,&local_16c,&local_14,local_8,1,0);
      param_1[0x67] = local_24;
    }
    else {
      local_24 = param_1[0x67];
    }
    if (param_1[0x66] == 1) {
      if ((local_24 != 1) && (param_1[0x21] == 0)) {
        local_6c = _DAT_0098c014;
        local_64 = 0;
        local_74 = 0;
        for (local_78 = _DAT_0098be34; (float)_DAT_009946e8 < local_78;
            local_78 = local_78 / (float)_DAT_00987490) {
          local_184 = (local_28 - local_c) * local_6c + local_c;
          local_180 = (local_10 - local_18) * local_6c + local_18;
          local_17c = 0;
          local_190 = local_c;
          local_18c = local_18;
          local_188 = 0;
          local_80 = local_180;
          local_7c = local_184;
          iVar3 = FUN_00550980(&local_190,&local_184,&local_14,local_8,1,0);
          if (iVar3 == 1) {
            local_6c = local_6c + local_78;
            local_70 = local_7c;
            local_68 = local_80;
            local_74 = 1;
          }
          else {
            local_6c = local_6c - local_78;
          }
        }
        if (local_74 == 1) {
          local_28 = local_70;
          local_10 = local_68;
          local_24 = 1;
        }
        else {
          local_24 = -4;
        }
      }
      param_1[0x65] = 0;
      param_1[0x66] = 0;
    }
    if (local_24 == 1) {
      param_1[0x23] = 4;
      local_194 = (float *)FUN_00919723();
      *local_194 = local_c;
      local_194[1] = local_18;
      local_194[2] = local_28;
      local_194[3] = local_10;
      param_1[0x24] = (int)local_194;
      local_1c = DAT_009944b8;
      local_84 = local_194;
    }
    else {
      local_88 = FUN_0051c100();
      if ((local_88 == 0) || (*(int *)(local_88 + 0x78) != 0)) {
        local_8c = 100;
        if (param_1[0x65] == 0) {
          if ((((local_24 == -2) || (local_24 == -1)) || (local_24 == -3)) || (local_24 == 0)) {
            if (*(int *)(local_20 + 0x25c) == 0) {
              if (DAT_00a7e0d4 != 0) {
                local_a4 = param_1[0xb];
                local_a0 = FUN_0051c100();
              }
              local_1c = FUN_00550e30(param_1,param_2,0);
            }
            else {
              if (DAT_00a7e0d4 != 0) {
                local_9c = param_1[0xb];
                local_98 = FUN_0051c100();
              }
              local_1c = FUN_005532f0(param_1,param_2,0);
            }
          }
          else {
            local_1c = DAT_009944bc;
          }
        }
        else {
          local_1c = FUN_00550e30(param_1,param_2,0);
          if (DAT_00a7e0d4 != 0) {
            local_94 = param_1[0xb];
            local_90 = FUN_0051c100();
          }
        }
        if (DAT_00a7e0d4 != 0) {
          local_ac = param_1[0xb];
          local_a8 = FUN_0051c100();
        }
        if ((local_8c < param_1[0xf]) && (local_1c == DAT_009944b4)) {
          local_1c = DAT_009944bc;
        }
      }
      else {
        local_1c = DAT_009944bc;
      }
    }
    if (local_1c == DAT_009944bc) {
      if (*param_1 == 0) {
        param_1[0x21] = 0;
        local_c4 = (float)param_1[0x1d];
        local_c0 = (float)param_1[0x1e];
        local_e0 = (float)param_1[0x17];
        local_dc = (float)param_1[0x18];
        local_d8 = 0.0;
        local_bc = 0;
        local_b8 = 0;
        local_b4 = 0;
        local_b0 = 0;
        local_d4 = 0.0;
        local_d0 = 0.0;
        local_cc = 0.0;
        local_e4 = 0;
        local_28 = local_e0;
        local_18 = local_c0;
        local_10 = local_dc;
        local_c = local_c4;
        puVar4 = (undefined4 *)FUN_004452b0(local_1a0,&local_e0,&local_c4);
        local_b8 = *puVar4;
        local_b4 = puVar4[1];
        local_b0 = puVar4[2];
        local_c8 = _DAT_0098c014;
        local_e8 = _DAT_0098be34;
        if (local_24 != -1) {
          for (; (float)_DAT_009946e8 < local_e8; local_e8 = local_e8 / (float)_DAT_00987490) {
            FUN_0045cfe0(local_1ac,local_c8,&local_b8);
            pfVar2 = (float *)FUN_0044c8a0(local_1b8,&local_c4);
            local_e0 = *pfVar2;
            local_dc = pfVar2[1];
            local_d8 = pfVar2[2];
            local_1c4 = local_c;
            local_1c0 = local_18;
            local_1bc = 0;
            iVar3 = FUN_00550980(&local_1c4,&local_e0,&local_14,local_8,0,0);
            if (iVar3 == 1) {
              local_d4 = local_e0;
              local_d0 = local_dc;
              local_cc = local_d8;
              local_e4 = 1;
              local_c8 = local_c8 + local_e8;
            }
            else {
              local_c8 = local_c8 - local_e8;
            }
          }
        }
        if (local_e4 != 0) {
          param_1[0x23] = 4;
          local_ec = (float *)FUN_00919723();
          *local_ec = local_c;
          local_ec[1] = local_18;
          local_ec[2] = local_d4;
          local_ec[3] = local_d0;
          param_1[0x24] = (int)local_ec;
          local_1c = DAT_009944b8;
        }
      }
      param_1[0x78] = DAT_0099b5a0;
    }
    if ((local_1c == DAT_009944b8) && (param_1[0x21] == 1)) {
      local_fc = param_1[0x23];
      local_f0 = param_1[0x24];
      local_f8 = FUN_00919723();
      local_f4 = local_fc;
      for (local_100 = 0; local_f4 = local_f4 + -2, local_100 < local_fc; local_100 = local_100 + 2)
      {
        *(undefined4 *)(local_f8 + local_100 * 4) = *(undefined4 *)(local_f0 + local_f4 * 4);
        *(undefined4 *)(local_f8 + 4 + local_100 * 4) = *(undefined4 *)(local_f0 + 4 + local_f4 * 4)
        ;
      }
      param_1[0x24] = local_f8;
      if (local_f0 != 0) {
        FUN_009196fd();
      }
      param_1[0x21] = 0;
      FUN_006fa3f0();
    }
    if ((local_1c == DAT_009944bc) || (local_1c == DAT_009944b8)) {
      param_1[0x22] = 1;
    }
  }
  return local_1c;
}

