// FN 00591890 FUN_00591890 size=283 ret=36 callers=005648d0 callees=00590270,0058c7d0,00919723

void __thiscall
FUN_00591890(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
            undefined4 param_10)

{
  int iVar1;
  int local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fa7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x88,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_30 = 0;
  }
  else {
    local_30 = FUN_0058c7d0();
  }
  local_8 = 0xffffffff;
  *(undefined1 *)(local_30 + 0x10) = 10;
  *(undefined4 *)(local_30 + 0x84) = param_10;
  *(undefined4 *)(local_30 + 100) = param_2;
  *(undefined4 *)(local_30 + 0x68) = param_3;
  *(undefined4 *)(local_30 + 0x6c) = param_4;
  *(undefined4 *)(local_30 + 0x38) = param_5;
  *(undefined4 *)(local_30 + 0x3c) = param_6;
  *(undefined4 *)(local_30 + 0x40) = param_7;
  *(undefined4 *)(local_30 + 0x44) = param_8;
  *(undefined4 *)(local_30 + 0x48) = param_9;
  if (*(int *)(*(int *)(param_1 + 0xb04) + 0x568) == 0x7f000000) {
    *(undefined4 *)(*(int *)(param_1 + 0xb04) + 0x568) = param_8;
  }
  FUN_00590270(local_30,0xffffffff);
  ExceptionList = local_10;
  return;
}

