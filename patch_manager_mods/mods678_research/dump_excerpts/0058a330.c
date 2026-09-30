// FN 0058a330 FUN_0058a330 size=318 ret=8 callers=006305e0,005f8960,00690000,005f8d80,0066c990,0089fbd0,005fe4b0,00630810,00574500,005f9220,0069b840,00599a10 callees=005752c0,0051c280,00919723,009196fd,0051e120

void __thiscall FUN_0058a330(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 local_50;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009516ab;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(int *)(param_1 + 0x11ac) = param_2;
  if (param_3 != 0) {
    if (param_2 == 0) {
      if (*(int *)(param_1 + 0x500) != 0) {
        FUN_009196fd(*(undefined4 *)(param_1 + 0x500),uVar1);
        *(undefined4 *)(param_1 + 0x500) = 0;
      }
      if (*(int *)(param_1 + 0x78) != 2) {
        uVar3 = 2;
        FUN_0051c280(param_1,2);
        FUN_0051e120(param_1,uVar3);
      }
    }
    else {
      if (*(int *)(param_1 + 0x500) == 0) {
        iVar2 = FUN_00919723(0x3c);
        local_8 = 0;
        if (iVar2 == 0) {
          local_50 = 0;
        }
        else {
          local_50 = FUN_005752c0();
        }
        *(undefined4 *)(param_1 + 0x500) = local_50;
      }
      local_8 = 0xffffffff;
      if (*(int *)(param_1 + 0x78) != 4) {
        uVar3 = 4;
        FUN_0051c280(param_1,4);
        FUN_0051e120(param_1,uVar3);
      }
    }
  }
  ExceptionList = local_10;
  return;
}

