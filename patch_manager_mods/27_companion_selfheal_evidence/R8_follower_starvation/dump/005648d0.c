// FN 005648d0 FUN_005648d0 size=2612 ret=44 callers=0057b4c0,006d9500 callees=0053f7f0,005d1750,00591890,005644b0,005642d0,00604260,00540ca0,0073fb90,007e5120,00563c90,0051c0e0,004452b0,00590110,007335f0,0073fec0,00564770,007e6be0,006c1450,00576430,0053f560,005453c0,0051c0a0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_005648d0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,int param_5,
            int param_6,int param_7,float *param_8,undefined4 param_9,int param_10,int param_11,
            int param_12)

{
  int iVar1;
  undefined4 uVar2;
  float10 fVar3;
  float *pfStack_120;
  undefined4 *puStack_11c;
  float *pfStack_118;
  float local_114;
  float local_110;
  float local_10c;
  undefined4 local_108;
  int local_104;
  int local_f4;
  undefined4 local_f0;
  int local_ec;
  float local_dc;
  undefined4 local_cc;
  int local_c8;
  int local_c4;
  undefined4 local_c0;
  ushort local_ba;
  undefined4 local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  int local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined1 *local_80;
  int local_7c;
  int local_78;
  undefined4 local_74;
  undefined4 local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  undefined4 local_5c;
  float local_58;
  float local_54;
  float local_50;
  int local_4c;
  int local_48;
  undefined4 local_44;
  int local_40;
  undefined4 local_3c;
  int local_38;
  int local_34;
  int local_30;
  int *local_2c;
  int local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  float local_14;
  undefined4 local_10;
  uint local_c;
  uint local_8;
  
  local_1c = 0;
  local_2c = (int *)0x0;
  local_10 = 1;
  local_104 = param_1;
  if (param_12 == 0) {
    local_9c = *(undefined4 *)(param_1 + 0x10dc);
    pfStack_118 = (float *)param_9;
    puStack_11c = (undefined4 *)param_10;
    pfStack_120 = param_8;
    FUN_00591890(param_2,param_3,param_4,param_5,param_6,param_7);
    local_a0 = *(int *)(local_104 + 0x520);
    if (local_a0 == 0) {
      local_a4 = *(undefined4 *)(local_104 + 0x10dc);
      pfStack_118 = (float *)0x564981;
      iVar1 = FUN_00590110();
      if (iVar1 != 0) {
        pfStack_118 = (float *)0x4;
        puStack_11c = (undefined4 *)0x564992;
        FUN_00563c90();
      }
    }
    pfStack_118 = (float *)0x3f;
    puStack_11c = (undefined4 *)0x56499f;
    iVar1 = FUN_0053f560();
    if (iVar1 == 0) {
      local_44 = 1;
      pfStack_118 = (float *)0x0;
      puStack_11c = (undefined4 *)0x0;
      pfStack_120 = (float *)0x0;
      FUN_0053f7f0(0x3f,0xffff,1,&local_44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    }
    return 1;
  }
  *(undefined4 *)(param_1 + 0x568) = 0x7f000000;
  *(undefined4 *)(param_1 + 0x56c) = 0x7f000000;
  local_30 = param_5;
  local_34 = param_6;
  local_38 = param_7;
  if (param_8 != (float *)0x7f000000) {
    local_a8 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    pfStack_118 = param_8;
    puStack_11c = (undefined4 *)0x564a55;
    local_2c = (int *)FUN_0051c0a0();
    if (local_2c == (int *)0x0) {
      return 0;
    }
    pfStack_118 = (float *)0x564a66;
    local_1c = FUN_005453c0();
    pfStack_118 = (float *)0x564a74;
    iVar1 = FUN_005453c0();
    if (local_1c != iVar1) {
      return 0;
    }
    param_5 = local_2c[0x25];
    param_6 = local_2c[0x26];
    param_7 = local_2c[0x27];
  }
  if (local_2c != (int *)0x0) {
    pfStack_118 = (float *)0x564aba;
    iVar1 = (**(code **)(*local_2c + 0x30))();
    if (iVar1 != 0) {
      local_ac = local_2c[1];
      local_b0 = *(int *)(local_104 + 4);
      if ((local_ac != local_b0) && (local_b4 = *(int *)(local_104 + 0x11ac), local_b4 == 0)) {
        pfStack_118 = param_8;
        puStack_11c = (undefined4 *)0x564b15;
        local_48 = FUN_005d1750();
        if (local_48 == 0) {
          return 0;
        }
        if (((*(byte *)(local_48 + 4) >> 2 & 3) == 1) ||
           (((*(byte *)(local_48 + 4) >> 4 & 1) == 1 && ((*(byte *)(local_48 + 4) & 1) == 0)))) {
          return 0;
        }
      }
    }
  }
  local_b8 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  pfStack_118 = (float *)param_2;
  puStack_11c = (undefined4 *)0x564b83;
  local_18 = FUN_0051c0e0();
  if (local_18 == 0) {
    uVar2 = 0;
  }
  else {
    pfStack_118 = (float *)param_3;
    puStack_11c = (undefined4 *)0x564b98;
    local_4c = FUN_00604260();
    if (local_4c != 0) {
      local_ba = *(ushort *)(local_4c + 2);
      local_c = (uint)local_ba;
    }
    pfStack_118 = (float *)local_c;
    puStack_11c = (undefined4 *)0x564bd1;
    pfStack_118 = (float *)FUN_00576430();
    local_c0 = DAT_00a1b4d0;
    puStack_11c = (undefined4 *)0x564bf5;
    local_c = (uint)pfStack_118;
    local_40 = FUN_006c1450();
    if (local_40 == 0) {
      uVar2 = 0;
    }
    else {
      pfStack_118 = (float *)0x564c10;
      iVar1 = FUN_005453c0();
      if (iVar1 == 0) {
        uVar2 = 0;
      }
      else {
        local_c4 = *(int *)(local_104 + 0xec);
        if (local_c4 == 0) {
          uVar2 = 0;
        }
        else {
          pfStack_118 = (float *)0x564c48;
          iVar1 = FUN_005453c0();
          local_c8 = iVar1 + 0x120;
          local_cc = *(undefined4 *)(iVar1 + 0x124);
          pfStack_118 = param_8;
          local_80 = (undefined1 *)&pfStack_120;
          local_3c = local_cc;
          local_108 = FUN_007335f0(local_40 + 0x24);
          fVar3 = (float10)FUN_005642d0();
          local_10c = (float)fVar3;
          local_20 = 1;
          local_8 = 0xffff;
          local_28 = 0;
          local_24 = 1;
          local_14 = local_10c;
          if (param_11 == 0) {
            pfStack_118 = (float *)(local_104 + 0x94);
            puStack_11c = &param_5;
            pfStack_120 = &local_58;
            FUN_004452b0();
            local_dc = local_50 * local_50 + local_54 * local_54 + local_58 * local_58;
            local_110 = local_14 * local_14;
            if (local_110 < local_dc) {
              local_8c = 0;
              local_88 = 0;
              local_84 = 0;
              pfStack_118 = (float *)local_10;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_00564770(local_8 & 0xffff,&param_5,local_3c,param_8,local_20,local_14,0,0,0,0,0,0,
                           0,&local_8c);
              local_28 = 1;
              local_8 = 0xfffe;
            }
            if (param_8 == (float *)0x7f000000) {
              pfStack_118 = (float *)0x0;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_0053f7f0(0x12,local_8 & 0xffff,2,&local_30,2,&local_34,2,&local_38,3,&local_3c,3,
                           &param_8,1,&local_20,2,&local_14,1,&local_10,0,0,0,0,0,0,0);
              if (local_28 == 0) {
                local_28 = 1;
                local_8 = 0xfffe;
              }
              pfStack_118 = (float *)0x0;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_0053f7f0(0x31,local_8 & 0xffff,2,&local_30,2,&local_34,2,&local_38,0,0,0,0,0,0,0,0
                           ,0,0,0,0,0,0,0,0,0);
            }
            else {
              local_5c = 0;
              pfStack_118 = (float *)0x0;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_0053f7f0(0x11,local_8 & 0xffff,3,&param_8,1,&local_20,2,&local_14,2,&local_14,1,
                           &local_24,1,&local_5c,1,&local_10,0,0,0,0,0,0,0,0,0);
              if (local_28 == 0) {
                local_28 = 1;
                local_8 = 0xfffe;
              }
              pfStack_118 = (float *)0x0;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_0053f7f0(0x13,local_8 & 0xffff,3,&param_8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                           0);
            }
            pfStack_118 = (float *)0x0;
            puStack_11c = (undefined4 *)0x0;
            pfStack_120 = (float *)0x0;
            FUN_0053f7f0(0x2e,local_8 & 0xffff,3,&param_2,1,&param_3,1,&param_4,3,&param_8,2,
                         &local_30,2,&local_34,2,&local_38,0,0,0,0,0,0,0,0,0);
          }
          else {
            pfStack_118 = (float *)0x0;
            puStack_11c = (undefined4 *)0x0;
            pfStack_120 = (float *)0x0;
            FUN_00540ca0(0x2e,0xffff,3,&param_2,1,&param_3,1,&param_4,3,&param_8,2,&local_30,2,
                         &local_34,2,&local_38,0,0,0,0,0,0,0,0,0);
            if (param_8 == (float *)0x7f000000) {
              if (local_28 == 0) {
                local_28 = 1;
                local_8 = 0xfffe;
              }
              pfStack_118 = (float *)0x0;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_00540ca0(0x31,local_8 & 0xffff,2,&local_30,2,&local_34,2,&local_38,0,0,0,0,0,0,0,0
                           ,0,0,0,0,0,0,0,0,0);
              pfStack_118 = (float *)0x0;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_00540ca0(0x12,local_8 & 0xffff,2,&local_30,2,&local_34,2,&local_38,3,&local_3c,3,
                           &param_8,1,&local_20,2,&local_14,1,&local_10,0,0,0,0,0,0,0);
            }
            else {
              if (local_28 == 0) {
                local_28 = 1;
                local_8 = 0xfffe;
              }
              pfStack_118 = (float *)0x0;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_00540ca0(0x13,local_8 & 0xffff,3,&param_8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                           0);
              local_70 = 0;
              pfStack_118 = (float *)0x0;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_00540ca0(0x11,local_8 & 0xffff,3,&param_8,1,&local_20,2,&local_14,2,&local_14,1,
                           &local_24,1,&local_70,1,&local_10,0,0,0,0,0,0,0,0,0);
            }
            pfStack_118 = (float *)(local_104 + 0x94);
            puStack_11c = &param_5;
            pfStack_120 = &local_68;
            FUN_004452b0();
            local_6c = local_68 * local_68 + local_64 * local_64 + local_60 * local_60;
            local_114 = local_14 * local_14;
            if (local_114 < local_6c) {
              local_98 = 0;
              local_94 = 0;
              local_90 = 0;
              pfStack_118 = (float *)local_10;
              puStack_11c = (undefined4 *)0x0;
              pfStack_120 = (float *)0x0;
              FUN_005644b0(local_8 & 0xffff,&param_5,local_3c,param_8,local_20,local_14,0,0,0,0,0,0,
                           0,&local_98);
              local_ec = *(int *)(local_104 + 0x11ac);
              if ((local_ec != 0) && ((float)_DAT_0098c1d8 < local_6c)) {
                local_74 = *(undefined4 *)(DAT_00a1b4a4 + 4);
                pfStack_118 = *(float **)(local_104 + 4);
                puStack_11c = (undefined4 *)0x565247;
                local_f0 = pfStack_118;
                pfStack_118 = (float *)FUN_0073fec0();
                puStack_11c = (undefined4 *)0x565250;
                FUN_0073fb90();
                puStack_11c = (undefined4 *)0x565257;
                local_78 = FUN_007e6be0();
                if (-1 < local_78) {
                  pfStack_118 = (float *)0x565268;
                  local_f4 = FUN_0073fb90();
                  local_7c = local_f4 + 0x24 + local_78 * 0x88;
                  pfStack_118 = (float *)0xa5ed;
                  puStack_11c = (undefined4 *)0x565291;
                  FUN_007e5120();
                }
              }
              local_28 = 1;
              local_8 = 0xfffe;
            }
          }
          if (param_10 == 0) {
            *(float **)(local_104 + 0x568) = param_8;
          }
          else {
            pfStack_118 = (float *)0x5652cd;
            iVar1 = FUN_005453c0();
            *(undefined4 *)(local_104 + 0x568) = *(undefined4 *)(iVar1 + 0x124);
          }
          uVar2 = 1;
        }
      }
    }
  }
  return uVar2;
}

