// FN 0078c350 FUN_0078c350 size=2883 ret=4 callers=00781be0 callees=0078cee0,0073f3d0,0072e850,0073f2d0,007f4c30,0073f2b0,007f3ea0,0073fea0,0072fd70,0091c860,0051a940,007f4550,0091ebad,0072fde0,007f4220,007f3c70,0073f3b0,0072e840,0091d220,007f3de0,0051a920,00734270,00733780,007bbff0,007f44b0,007f4610,0072fb00,0072f9d0,0073eee0,007faae0,00733540,00739aa0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0078c350(int param_1)

{
  longlong lVar1;
  char cVar2;
  int iVar3;
  float *pfVar4;
  undefined4 uVar5;
  float10 fVar6;
  float10 fVar7;
  float10 fVar8;
  undefined8 uVar9;
  double dVar10;
  float local_158;
  int local_150;
  int local_14c;
  float local_148;
  float local_144;
  float local_140;
  undefined1 local_90 [8];
  float local_88;
  undefined4 local_84;
  float local_80;
  int local_7c;
  float local_78;
  int *local_74;
  uint local_70;
  float local_6c;
  float local_68;
  undefined4 local_64;
  int local_60;
  int local_5c;
  float local_58;
  float local_54;
  uint local_50;
  undefined1 local_4c [12];
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  float local_30;
  uint local_2c;
  uint local_28;
  undefined4 local_24;
  uint local_20;
  undefined1 local_19;
  int local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096655b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x18) != 0) {
    FUN_0073f3d0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    lVar1 = FUN_0051a940();
    local_30 = (float)((float10)lVar1 / (float10)_DAT_00996518);
    local_19 = DAT_00a30828;
    DAT_00a30828 = 0;
    local_74 = *(int **)(*(int *)(param_1 + 0x18) + 0x40);
    local_84 = *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x18) + 0x48) + 0x188);
    local_38 = 0;
    FUN_0072fb00();
    fVar6 = (float10)FUN_007bbff0();
    local_88 = (float)fVar6;
    local_34 = 0;
    local_54 = 0.0;
    FUN_0072fd70();
    iVar3 = FUN_0072fb00();
    if ((*(uint *)(iVar3 + 8) >> 4 & 1) != 0) {
      local_54 = local_54 * (float)_DAT_00986d98;
    }
    fVar6 = (float10)FUN_0072f9d0();
    if ((float10)_DAT_00986d98 <= fVar6) {
      fVar6 = (float10)FUN_0072f9d0();
      local_140 = (float)fVar6;
    }
    else {
      local_140 = _DAT_00987474;
    }
    if (local_140 <= (float)_DAT_00986b50) {
      fVar6 = (float10)FUN_0072f9d0();
      if ((float10)_DAT_00986d98 <= fVar6) {
        fVar6 = (float10)FUN_0072f9d0();
        local_148 = (float)fVar6;
      }
      else {
        local_148 = _DAT_00987474;
      }
      local_144 = local_148;
    }
    else {
      local_144 = 1.0;
    }
    local_80 = local_144;
    iVar3 = FUN_0073fea0();
    local_5c = (int)*(short *)(iVar3 + 0x6c);
    iVar3 = FUN_0091c860();
    if (iVar3 < 2) {
      local_14c = 2;
    }
    else {
      local_14c = FUN_0091c860();
    }
    local_18 = local_14c;
    local_2c = (uint)(*DAT_00a1b49c <= local_14c);
    local_50 = (uint)(local_5c - local_14c <= *DAT_00a1b49c);
    iVar3 = FUN_0073fea0();
    local_7c = (int)*(short *)(iVar3 + 0x6e);
    iVar3 = FUN_0091c860();
    if (iVar3 < 2) {
      local_150 = 2;
    }
    else {
      local_150 = FUN_0091c860();
    }
    local_18 = local_150;
    local_20 = (uint)(DAT_00a1b49c[1] <= local_150);
    local_70 = (uint)(local_7c - local_150 <= DAT_00a1b49c[1]);
    cVar2 = FUN_0072e840();
    if ((cVar2 == '\0') || (cVar2 = FUN_0072e850(), cVar2 == '\0')) {
      local_50 = 0;
      local_2c = 0;
      local_70 = 0;
      local_20 = 0;
    }
    cVar2 = FUN_0073eee0();
    if (cVar2 != '\0') {
      local_70 = 0;
      local_20 = 0;
    }
    local_28 = DAT_00a7fcbc;
    local_78 = 0.0;
    local_58 = 0.0;
    FUN_0072fde0();
    if (local_78 <= (float)_DAT_00986d98) {
      local_158 = _DAT_00987474;
    }
    else {
      local_158 = local_78;
    }
    local_14 = local_158;
    if (local_158 != (float)_DAT_00986b38) {
      local_80 = local_158;
    }
    iVar3 = FUN_0072fb00();
    FUN_007f3de0((float)(int)((-(uint)((*(uint *)(iVar3 + 8) >> 4 & 1) != 0) & 2) - 1) * local_58);
    iVar3 = FUN_0072fb00();
    if ((*(uint *)(iVar3 + 8) >> 1 & 1) != 0) {
      local_28 = (uint)(local_28 == 0);
    }
    if (local_80 == (float)_DAT_00986b38) {
      if (((((local_2c == 0) && (local_50 == 0)) && (local_20 == 0)) && (local_70 == 0)) ||
         ((local_28 != 0 || (iVar3 = FUN_0072fb00(), *(char *)(iVar3 + 0x75) == '\x05')))) {
        if ((local_28 == 0) && (iVar3 = FUN_0072fb00(), *(char *)(iVar3 + 0x75) != '\x05')) {
          FUN_007f3ea0();
        }
        else {
          FUN_007f3ea0();
          FUN_007f3c70(((local_88 * local_54) / (float)_DAT_00989ee0) *
                       (float)(int)((uint)(DAT_00a7ebbc != 0) * 2 + -1));
        }
      }
      else {
        if ((local_2c != 0) || (local_50 != 0)) {
          FUN_007f4220();
        }
        if ((local_20 != 0) || (local_70 != 0)) {
          FUN_007f3de0((float)(int)(((uint)(DAT_00a106d8 != 0) * 2 + -1) *
                                   ((uint)(local_20 != 0) * 2 + -1)) * (float)_DAT_00986b50);
        }
      }
    }
    else {
      FUN_007f4220();
    }
    if (*(float *)(param_1 + 0xac) != (float)_DAT_00986b38) {
      *(undefined4 *)(*(int *)(param_1 + 0x18) + 0x104) = 1;
    }
    if (local_74 != (int *)0x0) {
      fVar6 = (float10)(**(code **)(*local_74 + 0x6c))();
      _DAT_00a1ffc0 = (float)((fVar6 * (float10)_DAT_0098d898) / (float10)_DAT_0098d808);
      _DAT_00a1ffc4 = _DAT_00a1ffc0;
      _DAT_00a1ffc8 = _DAT_00a1ffc0;
    }
    (**(code **)(*local_74 + 0x20))(local_4c);
    local_6c = 0.0;
    local_68 = 0.0;
    local_64 = 0;
    if (*(int *)(param_1 + 0xd0) != 0) {
      pfVar4 = (float *)(**(code **)(*local_74 + 0x2c))();
      local_6c = (float)*(int *)(param_1 + 0xd0) * *pfVar4;
    }
    if (*(int *)(param_1 + 0xd4) != 0) {
      iVar3 = (**(code **)(*local_74 + 0x2c))();
      local_68 = (float)*(int *)(param_1 + 0xd4) * *(float *)(iVar3 + 4);
    }
    FUN_007f4550();
    DAT_00a30828 = local_19;
    local_40 = 0;
    local_24 = *(undefined4 *)(*(int *)(param_1 + 0x18) + 0x40);
    local_60 = *(int *)(*(int *)(param_1 + 0x18) + 0x48);
    if (local_60 != 0) {
      local_40 = *(undefined4 *)(local_60 + 0x188);
    }
    local_3c = FUN_0078cee0();
    FUN_007faae0();
    if (*(int *)(param_1 + 800) == 0) {
      FUN_007f4610();
    }
    fVar6 = (float10)FUN_007f4c30(local_30,*(undefined4 *)(param_1 + 0xac));
    *(float *)(param_1 + 0xac) = (float)fVar6;
    dVar10 = _fabs((double)*(float *)(param_1 + 0xac));
    if ((float)_DAT_0098c1e8 < (float)dVar10) {
      FUN_007f44b0();
    }
    iVar3 = FUN_0073f2d0();
    if ((iVar3 == 1) && (*(int *)(*(int *)(param_1 + 0x18) + 0x80) == 1)) {
      fVar6 = (float10)(**(code **)(*local_74 + 0x6c))();
      fVar7 = (float10)(**(code **)(*local_74 + 0x68))();
      fVar8 = (float10)(**(code **)(*local_74 + 100))();
      FUN_00733540();
      local_8 = 0;
      FUN_0073f3b0(0x56,(double)(float)fVar6,(double)(float)fVar7,(double)(float)fVar8);
      uVar9 = FUN_0051a920();
      uVar5 = __aulldiv(uVar9,1000,0);
      FUN_00734270(local_90,"%d %c %f %f %f\n",uVar5);
      FUN_0073f2b0();
      FUN_00739aa0();
      *(undefined4 *)(*(int *)(param_1 + 0x18) + 0x80) = 0;
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    *(undefined4 *)(param_1 + 0xac) = 0;
    *(undefined4 *)(param_1 + 0xb0) = 0;
    *(undefined4 *)(param_1 + 0xb4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x18) + 0x80) = 0;
  }
  ExceptionList = local_10;
  return;
}

