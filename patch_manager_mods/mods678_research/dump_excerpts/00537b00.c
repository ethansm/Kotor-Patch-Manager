// FN 00537b00 FUN_00537b00 size=105 ret=4 callers=0051c0e0 callees=0053dfb0

undefined4 __thiscall FUN_00537b00(int param_1,undefined4 param_2)

{
  char cVar1;
  undefined4 uVar2;
  int *local_8;
  
  local_8 = (int *)0x0;
  uVar2 = *(undefined4 *)(param_1 + 0x1005c);
  cVar1 = FUN_0053dfb0(param_2,&local_8);
  if (((cVar1 == DAT_0099447f) && (local_8 != (int *)0x0)) && ((char)local_8[2] == DAT_00994475)) {
    uVar2 = (**(code **)(*local_8 + 0x38))(param_1,uVar2);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

