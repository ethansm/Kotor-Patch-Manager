// FN 00576750 FUN_00576750 size=1355 ret=0 callers=00631260,00574180,00630810,00572a70 callees=0051c280,0057fce0,00734010,005e4e80,006d0620,0055e970,006d3330,00734270,005e5ea0,00733780,0057ff50,005e4920,00733540,00919723,0058a2c0,0051cc80,0051bd90,00544210,00606100,0051e120,00664490

void __fastcall FUN_00576750(int *param_1)

{
  short sVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  uint uVar5;
  int local_bc;
  undefined2 local_a0;
  undefined1 local_34 [8];
  int local_2c;
  undefined4 local_28;
  uint local_24;
  int local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00950fb3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = FUN_0051c280(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (*(int *)(param_1[0x466] + 0x6c) == 1) {
    local_1c = 1;
    FUN_0051e120(param_1,4);
    for (local_24 = 0; local_24 < 0x14; local_24 = local_24 + 1) {
      local_20 = FUN_006d0620(local_1c);
      if (local_20 != 0) {
        *(uint *)(local_20 + 0x2c8) = *(uint *)(local_20 + 0x2c8) & 0xfffffff7 | 8;
      }
      local_1c = local_1c * 2;
    }
    local_24 = 0;
    while( true ) {
      iVar2 = FUN_0058a2c0(1);
      if (*(uint *)(iVar2 + 0x10) <= local_24) break;
      uVar5 = local_24;
      FUN_0058a2c0(1);
      local_20 = FUN_00606100(uVar5);
      *(uint *)(local_20 + 0x2c8) = *(uint *)(local_20 + 0x2c8) & 0xfffffff7 | 8;
      local_24 = local_24 + 1;
    }
  }
  iVar2 = param_1[1];
  piVar4 = param_1 + 6;
  FUN_0051bd90(piVar4,iVar2);
  FUN_0055e970(piVar4,iVar2);
  param_1[0x43a] = *(int *)(param_1[0x466] + 0x6c);
  local_18 = FUN_0051cc80();
  if (param_1[0x43a] == 1) {
    if ((local_18 != 0) && (*(int *)(local_18 + 0x18) != 0)) {
      FUN_006d3330(param_1[1],0);
    }
    *(undefined4 *)(param_1[0x466] + 0x7c) = 0;
  }
  else {
    local_28 = *(undefined4 *)(param_1[0x466] + 0x78);
    *(undefined4 *)(param_1[0x466] + 0x78) = 0xffffffff;
    local_2c = FUN_00664490(local_28);
    if (local_2c == 0) {
      FUN_00733540();
      local_8 = 0;
      uVar3 = FUN_00734010(local_28);
      FUN_00734270(local_34,
                   "Cannot set creature %s to faction %d because faction does not exist! Setting to Hostile1."
                   ,uVar3);
      FUN_00664490(1);
      FUN_006d3330(param_1[1],0);
      *(undefined4 *)(param_1[0x466] + 0x7c) = 1;
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    else {
      FUN_006d3330(param_1[1],0);
      *(undefined4 *)(param_1[0x466] + 0x7c) = local_28;
    }
  }
  FUN_0057fce0();
  param_1[0x85] = 1;
  iVar2 = FUN_00919723(0x8c);
  local_8 = 1;
  if (iVar2 == 0) {
    local_bc = 0;
  }
  else {
    local_bc = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_a0 = (undefined2)DAT_0099602c;
  *(undefined2 *)(local_bc + 8) = local_a0;
  *(ushort *)(local_bc + 10) = *(ushort *)(local_bc + 10) & 0xfff8 | DAT_009967bc;
  FUN_005e5ea0(param_1[1]);
  FUN_005e4e80(0,*(undefined2 *)(param_1[0x466] + 0xe0));
  FUN_00544210(local_bc,0,0);
  param_1[0x85] = 0;
  sVar1 = (**(code **)(*param_1 + 0x9c))(0);
  if (sVar1 < 1) {
    (**(code **)(*param_1 + 0x7c))(0x2718);
    param_1[0x3b] = 0;
    if (param_1[0x43a] != 0) {
      sVar1 = (**(code **)(*param_1 + 0x9c))(0);
      if (-10 < sVar1) goto LAB_00576c54;
    }
    *(undefined2 *)(param_1 + 0x445) = 0;
  }
LAB_00576c54:
  if (param_1[0x43a] == 1) {
    param_1[0x3d] = 1;
  }
  FUN_0057ff50(0);
  ExceptionList = local_10;
  return;
}

