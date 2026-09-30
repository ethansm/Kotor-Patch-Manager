// FN 0057c990 FUN_0057c990 size=2524 ret=12 callers=005959e0,0057de70,0057dc20,006da4f0,006d7390 callees=005d1750,0051c100,005e1680,005453c0,0052bb60,0051c0a0,004452b0,005142f0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall FUN_0057c990(int param_1,float param_2,int param_3,int param_4)

{
  bool bVar1;
  char cVar2;
  short sVar3;
  int iVar4;
  int *piVar5;
  undefined4 uVar6;
  float10 fVar7;
  uint local_170;
  uint local_168;
  undefined1 local_b8 [12];
  undefined1 local_ac [12];
  undefined4 local_a0;
  undefined4 local_9c;
  float local_98;
  undefined4 local_94;
  undefined4 local_90;
  float local_8c;
  int local_88;
  undefined4 local_84;
  undefined4 local_80;
  float local_7c;
  uint local_78;
  undefined4 local_74;
  undefined4 local_70;
  float local_6c;
  undefined4 local_68;
  undefined4 local_64;
  float local_60;
  int local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  uint local_4c;
  int local_48;
  int local_44;
  int local_40;
  undefined4 local_3c;
  int local_38;
  int local_34;
  int local_30;
  int *local_2c;
  int *local_28;
  float local_24;
  float local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  undefined1 local_8 [4];
  
  local_40 = FUN_005453c0();
  if (local_40 == 0) {
    local_1c = 0x7f000000;
  }
  else {
    local_2c = (int *)(local_40 + 0x194);
    for (local_48 = 0;
        (local_48 < *(int *)(local_40 + 0x198) &&
        (*(int *)(*local_2c + local_48 * 4) != *(int *)(param_1 + 4))); local_48 = local_48 + 1) {
    }
    if (local_48 < *(int *)(local_40 + 0x198)) {
      local_38 = 0;
      local_20 = param_2;
      local_1c = 0x7f000000;
      local_44 = 1;
      local_c = 1;
      local_3c = *(undefined4 *)(DAT_00a1b4a4 + 8);
      local_18 = 0;
      local_14 = 0;
      local_10 = 0;
      for (; (-1 < local_48 - local_38 || (local_48 + local_38 < local_2c[1]));
          local_38 = local_38 + 1) {
        if (local_44 != 0) {
          if (local_48 - local_38 < 0) {
            local_44 = 0;
          }
          else {
            local_34 = *(int *)(*local_2c + (local_48 - local_38) * 4);
            local_28 = (int *)FUN_0051c0a0(local_34);
            if (((local_28 != (int *)0x0) && (local_34 != *(int *)(param_1 + 4))) &&
               (local_34 != param_3)) {
              local_4c = 0;
              if (param_4 == 0x7f000000) {
                cVar2 = FUN_005e1680(local_34);
                local_4c = (uint)(cVar2 == '\x02');
              }
              else {
                iVar4 = (**(code **)(*local_28 + 0x30))();
                if (iVar4 == 0) {
LAB_0057cb6d:
                  local_168 = 0;
                }
                else {
                  iVar4 = param_4;
                  (**(code **)(*local_28 + 0x30))(param_4);
                  cVar2 = FUN_005e1680(iVar4);
                  if (cVar2 != '\x02') goto LAB_0057cb6d;
                  local_168 = 1;
                }
                local_4c = local_168;
              }
              iVar4 = (**(code **)(*local_28 + 0x30))();
              if ((iVar4 != 0) && (local_4c != 0)) {
                piVar5 = (int *)(**(code **)(*local_28 + 0x30))();
                iVar4 = (**(code **)(*piVar5 + 0x94))();
                if (iVar4 == 0) {
                  (**(code **)(*local_28 + 0x30))();
                  piVar5 = (int *)(**(code **)(*local_28 + 0x30))();
                  if (piVar5[0x46b] == 0) {
                    bVar1 = false;
                  }
                  else {
                    sVar3 = (**(code **)(*piVar5 + 0x9c))(0);
                    bVar1 = sVar3 < 1;
                  }
                  if (((!bVar1) && (local_30 = FUN_005d1750(local_28[1]), local_30 != 0)) &&
                     ((*(byte *)(local_30 + 4) & 1) != 0)) {
                    local_58 = 0;
                    local_54 = 0;
                    local_50 = 0;
                    if (param_4 == 0x7f000000) {
                      local_58 = *(undefined4 *)(param_1 + 0x94);
                      local_54 = *(undefined4 *)(param_1 + 0x98);
                      local_50 = *(undefined4 *)(param_1 + 0x9c);
                    }
                    else {
                      local_5c = FUN_0051c100(param_4);
                      if (local_5c != 0) {
                        local_58 = *(undefined4 *)(local_5c + 0x94);
                        local_54 = *(undefined4 *)(local_5c + 0x98);
                        local_50 = *(undefined4 *)(local_5c + 0x9c);
                      }
                    }
                    iVar4 = (**(code **)(*local_28 + 0x30))();
                    uVar6 = FUN_004452b0(local_ac,&local_58,iVar4 + 0x94);
                    fVar7 = (float10)FUN_005142f0(uVar6);
                    local_24 = (float)fVar7 - *(float *)(*(int *)(param_1 + 0x380) + 8);
                    iVar4 = (**(code **)(*local_28 + 0x30))();
                    local_24 = local_24 - *(float *)(*(int *)(iVar4 + 0x380) + 8);
                    if (local_24 < local_20) {
                      local_68 = *(undefined4 *)(param_1 + 0x94);
                      local_64 = *(undefined4 *)(param_1 + 0x98);
                      local_60 = *(float *)(param_1 + 0x9c) + (float)_DAT_00989f10;
                      iVar4 = (**(code **)(*local_28 + 0x30))();
                      local_74 = *(undefined4 *)(iVar4 + 0x94);
                      local_70 = *(undefined4 *)(iVar4 + 0x98);
                      local_6c = *(float *)(iVar4 + 0x9c) + (float)_DAT_00989f10;
                      if ((local_40 != 0) &&
                         (iVar4 = FUN_0052bb60(&local_68,&local_74,&local_18,local_8,
                                               *(undefined4 *)(param_1 + 4),local_34,1,0),
                         iVar4 != 0)) {
                        local_20 = local_24;
                        local_1c = local_34;
                      }
                    }
                  }
                }
              }
              iVar4 = (**(code **)(*local_28 + 0x10))();
              if ((iVar4 != 0) &&
                 (iVar4 = (**(code **)(*local_28 + 0x10))(),
                 local_20 < *(float *)(param_1 + 0x94) - *(float *)(iVar4 + 0x94))) {
                local_44 = 0;
              }
            }
          }
        }
        if (local_c != 0) {
          if (local_48 + local_38 < local_2c[1]) {
            local_34 = *(int *)(*local_2c + (local_48 + local_38) * 4);
            local_28 = (int *)FUN_0051c0a0(local_34);
            if (((local_28 != (int *)0x0) && (local_34 != *(int *)(param_1 + 4))) &&
               (local_34 != param_3)) {
              local_78 = 0;
              if (param_4 == 0x7f000000) {
                cVar2 = FUN_005e1680(local_34);
                local_78 = (uint)(cVar2 == '\x02');
              }
              else {
                iVar4 = (**(code **)(*local_28 + 0x30))();
                if (iVar4 == 0) {
LAB_0057cfbf:
                  local_170 = 0;
                }
                else {
                  iVar4 = param_4;
                  (**(code **)(*local_28 + 0x30))(param_4);
                  cVar2 = FUN_005e1680(iVar4);
                  if (cVar2 != '\x02') goto LAB_0057cfbf;
                  local_170 = 1;
                }
                local_78 = local_170;
              }
              iVar4 = (**(code **)(*local_28 + 0x30))();
              if ((iVar4 != 0) && (local_78 != 0)) {
                piVar5 = (int *)(**(code **)(*local_28 + 0x30))();
                iVar4 = (**(code **)(*piVar5 + 0x94))();
                if (iVar4 == 0) {
                  (**(code **)(*local_28 + 0x30))();
                  piVar5 = (int *)(**(code **)(*local_28 + 0x30))();
                  if (piVar5[0x46b] == 0) {
                    bVar1 = false;
                  }
                  else {
                    sVar3 = (**(code **)(*piVar5 + 0x9c))(0);
                    bVar1 = sVar3 < 1;
                  }
                  if (((!bVar1) && (local_30 = FUN_005d1750(local_28[1]), local_30 != 0)) &&
                     ((*(byte *)(local_30 + 4) & 1) != 0)) {
                    local_84 = 0;
                    local_80 = 0;
                    local_7c = 0.0;
                    if (param_4 == 0x7f000000) {
                      local_84 = *(undefined4 *)(param_1 + 0x94);
                      local_80 = *(undefined4 *)(param_1 + 0x98);
                      local_7c = *(float *)(param_1 + 0x9c);
                    }
                    else {
                      local_88 = FUN_0051c100(param_4);
                      if (local_88 != 0) {
                        local_84 = *(undefined4 *)(local_88 + 0x94);
                        local_80 = *(undefined4 *)(local_88 + 0x98);
                        local_7c = *(float *)(local_88 + 0x9c);
                      }
                    }
                    iVar4 = (**(code **)(*local_28 + 0x30))();
                    uVar6 = FUN_004452b0(local_b8,&local_84,iVar4 + 0x94);
                    fVar7 = (float10)FUN_005142f0(uVar6);
                    local_24 = (float)fVar7 - *(float *)(*(int *)(param_1 + 0x380) + 8);
                    iVar4 = (**(code **)(*local_28 + 0x30))();
                    local_24 = local_24 - *(float *)(*(int *)(iVar4 + 0x380) + 8);
                    if (local_24 < local_20) {
                      local_94 = local_84;
                      local_90 = local_80;
                      local_8c = local_7c + (float)_DAT_00989f10;
                      iVar4 = (**(code **)(*local_28 + 0x30))();
                      local_a0 = *(undefined4 *)(iVar4 + 0x94);
                      local_9c = *(undefined4 *)(iVar4 + 0x98);
                      local_98 = *(float *)(iVar4 + 0x9c) + (float)_DAT_00989f10;
                      if ((local_40 != 0) &&
                         (iVar4 = FUN_0052bb60(&local_94,&local_a0,&local_18,local_8,
                                               *(undefined4 *)(param_1 + 4),local_34,1,0),
                         iVar4 != 0)) {
                        local_20 = local_24;
                        local_1c = local_34;
                      }
                    }
                  }
                }
              }
              iVar4 = (**(code **)(*local_28 + 0x10))();
              if ((iVar4 != 0) &&
                 (iVar4 = (**(code **)(*local_28 + 0x10))(),
                 local_20 < *(float *)(iVar4 + 0x94) - *(float *)(param_1 + 0x94))) {
                local_c = 0;
              }
            }
          }
          else {
            local_c = 0;
          }
        }
      }
    }
    else {
      local_1c = 0x7f000000;
    }
  }
  return local_1c;
}

