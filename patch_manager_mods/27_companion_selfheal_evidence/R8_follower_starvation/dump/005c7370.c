// FN 005c7370 FUN_005c7370 size=1884 ret=16 callers=005c6340 callees=005644b0,005195a0,005d5120,00513cc0,0054b650,0073fb90,00540ca0,007e8720,007e89e0,0054b130,0051c100,0051c8b0,0054bdc0,005453c0,0044c8a0

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005c7370(int param_1,int param_2)

{
  int iVar1;
  float *pfVar2;
  float *pfVar3;
  float10 fVar4;
  undefined4 *puVar5;
  undefined1 *puVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  float *local_d0;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  float local_74;
  float local_70;
  float local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_4c;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20 [4];
  undefined1 local_1c [4];
  float local_18 [3];
  int local_c;
  uint local_8;
  
  FUN_0073fb90();
  FUN_007e8720();
  local_30 = *(undefined4 *)(param_2 + 0x28);
  local_2c = *(undefined4 *)(param_2 + 0x2c);
  local_28 = *(undefined4 *)(param_2 + 0x30);
  local_44 = *(undefined4 *)(param_2 + 0x34);
  iVar1 = FUN_007e89e0(local_44);
  local_4c = *(undefined4 *)(iVar1 + 0xc);
  local_68 = 0;
  local_64 = 0;
  local_60 = 0x3f800000;
  pfVar3 = (float *)(param_2 + 0x38);
  fVar4 = (float10)FUN_00513cc0(local_4c);
  pfVar2 = (float *)FUN_005195a0(local_68,local_64,local_60,(float)fVar4);
  if ((DAT_00a0f190 == 0) ||
     ((*pfVar2 != (float)_DAT_00986b50 && (*pfVar2 != (float)_DAT_00986d98)))) {
    local_74 = (1.0 - (pfVar2[3] * pfVar2[3] + pfVar2[2] * pfVar2[2]) * (float)_DAT_00987490) *
               *pfVar3 + (pfVar2[1] * pfVar2[2] - pfVar2[3] * *pfVar2) * (float)_DAT_00987490 *
                         *(float *)(param_2 + 0x3c) +
               (pfVar2[2] * *pfVar2 + pfVar2[1] * pfVar2[3]) * (float)_DAT_00987490 *
               *(float *)(param_2 + 0x40);
    local_70 = (pfVar2[3] * *pfVar2 + pfVar2[1] * pfVar2[2]) * (float)_DAT_00987490 * *pfVar3 +
               (1.0 - (pfVar2[3] * pfVar2[3] + pfVar2[1] * pfVar2[1]) * (float)_DAT_00987490) *
               *(float *)(param_2 + 0x3c) +
               (pfVar2[2] * pfVar2[3] - pfVar2[1] * *pfVar2) * (float)_DAT_00987490 *
               *(float *)(param_2 + 0x40);
    local_6c = (pfVar2[1] * pfVar2[3] - pfVar2[2] * *pfVar2) * (float)_DAT_00987490 * *pfVar3 +
               (pfVar2[1] * *pfVar2 + pfVar2[2] * pfVar2[3]) * (float)_DAT_00987490 *
               *(float *)(param_2 + 0x3c) +
               (1.0 - (pfVar2[2] * pfVar2[2] + pfVar2[1] * pfVar2[1]) * (float)_DAT_00987490) *
               *(float *)(param_2 + 0x40);
  }
  else {
    local_74 = *pfVar3;
    local_70 = *(float *)(param_2 + 0x3c);
    local_6c = *(float *)(param_2 + 0x40);
  }
  local_d0 = &local_74;
  FUN_0044c8a0(local_20,&local_30,local_d0);
  uVar9 = 0;
  uVar8 = 0;
  uVar7 = 1;
  puVar6 = local_20;
  FUN_005453c0(puVar6,1,0,0);
  fVar4 = (float10)FUN_0054b130(puVar6,uVar7,uVar8,uVar9);
  local_18[0] = (float)fVar4;
  *(undefined4 *)(param_2 + 0x18) = local_30;
  *(undefined4 *)(param_2 + 0x1c) = local_2c;
  *(undefined4 *)(param_2 + 0x20) = local_28;
  *(undefined4 *)(param_2 + 0x24) = local_44;
  iVar1 = *(int *)(param_1 + 0x380);
  *(undefined4 *)(iVar1 + 0x254) = 0x7f000000;
  *(undefined4 *)(iVar1 + 600) = 0;
  *(undefined4 *)(iVar1 + 0x25c) = 0;
  *(undefined4 *)(iVar1 + 0x260) = 0;
  local_18[1] = 1.4013e-45;
  local_40 = _DAT_0098c014;
  local_34 = 0x7f000000;
  iVar1 = FUN_005453c0();
  local_3c = *(undefined4 *)(iVar1 + 0x124);
  local_24 = *(undefined4 *)(param_1 + 0xec);
  *(undefined4 *)(param_1 + 0xec) = 1;
  local_38 = FUN_005d5120(0x3d);
  local_8 = 0xffff;
  if (local_38 != 0) {
    local_8 = (uint)*(ushort *)(local_38 + 0x6c);
  }
  uVar7 = *(undefined4 *)(param_1 + 0x380);
  puVar6 = local_20;
  FUN_005453c0(puVar6,uVar7);
  iVar1 = FUN_0054b650(puVar6,uVar7);
  if (iVar1 == 0) {
    puVar10 = &local_30;
    uVar9 = 0;
    uVar7 = *(undefined4 *)(param_1 + 0x380);
    puVar5 = &local_30;
    uVar8 = _DAT_0098be28;
    FUN_005453c0(puVar5,_DAT_0098be28,uVar7,0,puVar10);
    FUN_0054bdc0(puVar5,uVar8,uVar7,uVar9,puVar10);
    FUN_00540ca0(0x40,local_8 & 0xffff,2,&local_30,2,&local_2c,2,&local_28,3,&local_3c,3,&local_34,1
                 ,local_18 + 1,2,&local_40,0,0,0,0,0,0,0,0,0,0,0,0);
    local_8c = 0;
    local_88 = 0;
    local_84 = 0;
    FUN_005644b0(0xfffe,&local_30,*(undefined4 *)(param_1 + 0x90),0x7f000000,1,0,_DAT_009919f0,0,0,0
                 ,0,0,0,&local_8c,0,0,0);
    *(undefined4 *)(param_2 + 4) = 7;
    *(undefined4 *)(param_1 + 0x11b0) = 0xffffffff;
  }
  else {
    FUN_00540ca0(0x40,local_8 & 0xffff,2,local_20,2,local_1c,2,local_18,3,&local_3c,3,&local_34,1,
                 local_18 + 1,2,&local_40,0,0,0,0,0,0,0,0,0,0,0,0);
    local_80 = 0;
    local_7c = 0;
    local_78 = 0;
    FUN_005644b0(0xfffe,local_20,*(undefined4 *)(param_1 + 0x90),0x7f000000,1,0,_DAT_009919f0,0,0,0,
                 0,0,0,&local_80,0,0,0);
    *(undefined4 *)(param_2 + 4) = 6;
    *(undefined4 *)(param_1 + 0x11b0) = 0xffffffff;
  }
  *(undefined4 *)(param_1 + 0xec) = local_24;
  local_c = FUN_0073fb90();
  local_18[2] = (float)FUN_0051c8b0(*(undefined4 *)(local_c + 0x24));
  iVar1 = FUN_0051c100(local_18[2]);
  if (iVar1 != 0) {
    *(undefined4 *)(param_2 + 0x74) = *(undefined4 *)(iVar1 + 0x94);
    *(undefined4 *)(param_2 + 0x78) = *(undefined4 *)(iVar1 + 0x98);
    *(undefined4 *)(param_2 + 0x7c) = *(undefined4 *)(iVar1 + 0x9c);
  }
  *(undefined4 *)(param_2 + 0x68) = 1;
  return DAT_009944b8;
}

