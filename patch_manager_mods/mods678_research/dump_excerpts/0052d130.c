// FN 0052d130 FUN_0052d130 size=896 ret=48 callers=0052b9c0,005cc870,005cb430,0052bb60,005555c0,0052d0e0,00550980 callees=0051c0a0,00552d90

uint __thiscall
FUN_0052d130(int *param_1,float *param_2,float *param_3,float param_4,float param_5,float param_6,
            undefined4 param_7,undefined4 *param_8,undefined4 param_9,undefined4 param_10,
            int param_11,int param_12,int param_13)

{
  bool bVar1;
  int *piVar2;
  int iVar3;
  uint uVar4;
  float *pfVar5;
  float *pfVar6;
  float local_44;
  float local_40;
  int local_24;
  float local_20;
  float local_1c;
  uint local_8;
  
  local_8 = 1;
  if (0 < param_1[0x12]) {
    if (*param_3 <= *param_2) {
      local_20 = *param_3;
      local_44 = *param_2;
    }
    else {
      local_20 = *param_2;
      local_44 = *param_3;
    }
    if (param_3[1] <= param_2[1]) {
      local_1c = param_3[1];
      local_40 = param_2[1];
    }
    else {
      local_1c = param_2[1];
      local_40 = param_3[1];
    }
    for (local_24 = 0; local_24 < param_1[0x12]; local_24 = local_24 + 1) {
      if (((param_13 == 0) ||
          (piVar2 = (int *)FUN_0051c0a0(*(undefined4 *)(param_1[0x14] + local_24 * 4)),
          piVar2 == (int *)0x0)) ||
         ((iVar3 = (**(code **)(*piVar2 + 0x18))(), iVar3 != 0 && (*(char *)(iVar3 + 0x31c) == '\0')
          ))) {
        pfVar5 = (float *)(param_1[0x13] + (local_24 * 2 + 1) * 0xc);
        pfVar6 = (float *)(param_1[0x13] + local_24 * 0x18);
        if (((((local_20 - param_6 < *pfVar6) && (local_44 + param_6 < *pfVar6)) ||
             ((*pfVar5 < local_44 + param_6 && (*pfVar5 < local_20 - param_6)))) ||
            ((((local_1c - param_6 < pfVar6[1] && (local_40 + param_6 < pfVar6[1])) ||
              ((pfVar5[1] < local_40 + param_6 && (pfVar5[1] < local_1c - param_6)))) ||
             ((param_4 < pfVar6[2] && (param_5 < pfVar6[2])))))) ||
           ((pfVar5[2] < param_5 && (pfVar5[2] < param_4)))) {
          bVar1 = true;
        }
        else {
          bVar1 = false;
        }
        if (((!bVar1) &&
            (iVar3 = (**(code **)(*param_1 + 4))
                               (*(undefined4 *)(param_1[0x14] + local_24 * 4),param_10), iVar3 == 0)
            ) && (iVar3 = FUN_00552d90(*(undefined4 *)(param_1[0x14] + local_24 * 4),param_2,param_3
                                       ,param_4,param_5,param_6,param_7,param_9,param_12),
                 iVar3 == 0)) {
          *param_8 = *(undefined4 *)(param_1[0x14] + local_24 * 4);
          if (param_12 != 0) {
            *(undefined4 *)(param_12 + 0x40) = *param_8;
          }
          if (param_11 == 1) {
            uVar4 = (**(code **)(*param_1 + 0x18))
                              (param_2,param_3,param_4,param_5,param_6,param_7,param_8,param_9,
                               param_10,1);
            local_8 = uVar4 & local_8;
          }
          else {
            local_8 = 0;
          }
        }
      }
    }
  }
  return local_8;
}

