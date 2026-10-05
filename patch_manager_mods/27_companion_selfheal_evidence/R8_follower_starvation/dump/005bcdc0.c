// FN 005bcdc0 FUN_005bcdc0 size=750 ret=4 callers=005d56b0 callees=0044b8a0,0073fec0,007e6be0,0051c080,005453c0,0073fb90,0053dfb0,004452b0,0091dba0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005bcdc0(int *param_1,int param_2)

{
  bool bVar1;
  float fVar2;
  short sVar3;
  int iVar4;
  undefined4 uVar5;
  float10 fVar6;
  undefined1 *puVar7;
  int local_9c;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  int local_48;
  undefined4 local_44;
  int local_40;
  undefined4 local_3c;
  int local_38;
  int local_34;
  float local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20 [4];
  undefined1 local_1c [4];
  undefined1 local_18 [7];
  char local_11;
  undefined1 local_10 [12];
  
  iVar4 = (**(code **)(*param_1 + 0x94))();
  uVar5 = DAT_009944bc;
  if (iVar4 != 1) {
    if (param_1[0x46b] == 0) {
      bVar1 = false;
    }
    else {
      sVar3 = (**(code **)(*param_1 + 0x9c))(0);
      bVar1 = sVar3 < 1;
    }
    uVar5 = DAT_009944bc;
    if (!bVar1) {
      _memcpy(&local_24,(void *)(param_2 + 0x38),4);
      _memcpy(&local_28,(void *)(param_2 + 0x3c),4);
      _memcpy(&local_2c,(void *)(param_2 + 0x40),4);
      _memcpy(&local_34,(void *)(param_2 + 0x44),4);
      _memcpy(local_20,(void *)(param_2 + 0x48),4);
      _memcpy(local_1c,(void *)(param_2 + 0x4c),4);
      _memcpy(&local_30,(void *)(param_2 + 0x50),4);
      puVar7 = local_18;
      iVar4 = local_34;
      FUN_0051c080(local_34,puVar7);
      local_11 = FUN_0053dfb0(iVar4,puVar7);
      uVar5 = DAT_009944bc;
      if (local_11 == DAT_0099447f) {
        iVar4 = FUN_005453c0();
        if ((iVar4 == 0) || (iVar4 = FUN_005453c0(), local_34 == *(int *)(iVar4 + 0x124))) {
          local_9c = 0;
        }
        else {
          local_9c = 1;
        }
        local_38 = local_9c;
        local_54 = local_24;
        local_50 = local_28;
        local_4c = local_2c;
        FUN_004452b0(local_10,param_1 + 0x25,&local_54);
        if ((local_38 != 0) ||
           (fVar2 = (float)_DAT_00987538, fVar6 = (float10)FUN_0044b8a0(local_10,local_10),
           uVar5 = DAT_009944b8, (float10)((local_30 + fVar2) * (local_30 + fVar2)) < fVar6)) {
          local_44 = *(undefined4 *)(DAT_00a1b4a4 + 4);
          uVar5 = FUN_0073fec0(param_1[1]);
          local_3c = uVar5;
          FUN_0073fb90(uVar5);
          local_40 = FUN_007e6be0(uVar5);
          uVar5 = DAT_009944b8;
          if (-1 < local_40) {
            iVar4 = FUN_0073fb90();
            local_48 = iVar4 + 0x24 + local_40 * 0x88;
            if (DAT_00a7e0c4 != 0) {
              FUN_0044b8a0(local_10,local_10);
            }
            *(undefined4 *)(local_48 + 4) = 5;
            uVar5 = DAT_009944b8;
          }
        }
      }
    }
  }
  return uVar5;
}

