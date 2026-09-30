// FN 008c8880 FUN_008c8880 size=1960 ret=0 callers= callees=0051c100,00604ec0,0056df50,005f7a30,00919723,0040f3c0,006d0620,00410530,0041e870,0051c8d0,006d05a0,008c9030,008d0780,008cff80,00605e80

void __fastcall FUN_008c8880(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_9c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097aedb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((*(byte *)(param_1 + 0x31e) == 0) || (4 < *(byte *)(param_1 + 0x31e))) {
    param_1[0x12] = param_1[0x12] & 0xffffff7f;
    if (((uint)param_1[0x12] >> 8 & 3) != 2) {
      param_1[0x12] = param_1[0x12] & 0xfffffcffU | 0x100;
    }
  }
  else if (param_1[0x323] == 0) {
    FUN_008c9030(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    FUN_0041e870(0,0);
    (**(code **)(*param_1 + 8))(param_1 + 0x1a,0);
    FUN_0040f3c0();
  }
  else {
    iVar1 = FUN_0051c100(*(undefined4 *)(param_1[0x323] + 0x2a8));
    iVar2 = FUN_006d05a0(param_1[0x323]);
    iVar3 = FUN_00919723(0x3bc);
    local_8 = 0;
    if (iVar3 == 0) {
      local_9c = 0;
    }
    else {
      local_9c = FUN_008cff80();
    }
    local_8 = 0xffffffff;
    param_1[0x324] = local_9c;
    if (iVar2 < 1) {
      if (*(int *)(param_1[0x323] + 0x2cc) < 2) {
        iVar1 = param_1[0x323];
        FUN_0051c8d0(iVar1);
        FUN_005f7a30();
        FUN_00605e80(iVar1);
      }
      else {
        iVar1 = FUN_00604ec0(1);
        param_1[0x323] = iVar1;
      }
    }
    else {
      FUN_008d0780(*(undefined4 *)(param_1[0x323] + 0x14),1,*(undefined4 *)(iVar1 + 4),iVar2);
      if ((*(int *)(param_1[0x324] + 0x1d8) == 0x10) || (*(int *)(param_1[0x324] + 0x1d8) == 0x20))
      {
        iVar1 = FUN_006d0620(0x10);
        iVar2 = FUN_006d0620(0x20);
        if (iVar2 == 0) {
          param_1[799] = param_1[799] & 0xfffffffb;
          param_1[799] = param_1[799] & 0xfffffffe;
          param_1[799] = param_1[799] & 0xfffffffd;
          param_1[800] = 0x7f000000;
          FUN_0056df50(iVar1);
        }
        else if (iVar1 == param_1[0x323]) {
          param_1[799] = param_1[799] & 0xfffffffb;
          param_1[799] = param_1[799] | 1;
          param_1[799] = param_1[799] & 0xfffffffd;
          param_1[800] = *(int *)(iVar2 + 0x14);
          FUN_0056df50(iVar2);
          FUN_0056df50(iVar1);
        }
        else if (iVar2 == param_1[0x323]) {
          param_1[799] = param_1[799] & 0xfffffffb;
          param_1[799] = param_1[799] | 1;
          param_1[799] = param_1[799] | 2;
          param_1[800] = *(int *)(iVar1 + 0x14);
          FUN_0056df50(iVar2);
          FUN_0056df50(iVar1);
        }
      }
      else if ((*(int *)(param_1[0x324] + 0x1d8) == 0x40000) ||
              (*(int *)(param_1[0x324] + 0x1d8) == 0x80000)) {
        iVar1 = FUN_006d0620(0x40000);
        iVar2 = FUN_006d0620(0x80000);
        if (iVar2 == 0) {
          param_1[799] = param_1[799] | 4;
          param_1[799] = param_1[799] & 0xfffffffe;
          param_1[799] = param_1[799] & 0xfffffffd;
          param_1[800] = 0x7f000000;
          FUN_0056df50(iVar1);
        }
        else if (iVar1 == param_1[0x323]) {
          param_1[799] = param_1[799] | 4;
          param_1[799] = param_1[799] | 1;
          param_1[799] = param_1[799] & 0xfffffffd;
          param_1[800] = *(int *)(iVar2 + 0x14);
          FUN_0056df50(iVar2);
          FUN_0056df50(iVar1);
        }
        else if (iVar2 == param_1[0x323]) {
          param_1[799] = param_1[799] | 4;
          param_1[799] = param_1[799] | 1;
          param_1[799] = param_1[799] | 2;
          param_1[800] = *(int *)(iVar1 + 0x14);
          FUN_0056df50(iVar2);
          FUN_0056df50(iVar1);
        }
      }
      else {
        param_1[799] = param_1[799] & 0xfffffffb;
        param_1[799] = param_1[799] & 0xfffffffe;
        param_1[799] = param_1[799] & 0xfffffffd;
        FUN_0056df50(param_1[0x323]);
      }
    }
    *(char *)(param_1[0x325] + 0x3d2c) = (char)param_1[0x31e];
    *(int *)(param_1[0x325] + 0x3d34) = param_1[0x323];
    FUN_00410530(param_1[0x325],3,1);
  }
  ExceptionList = local_10;
  return;
}

