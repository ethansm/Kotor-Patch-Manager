// FN 006dac70 FUN_006dac70 size=123 ret=8 callers=006da4f0,006d7390 callees=0057d370,0057e3d0,005921e0,0091dba0

void __thiscall FUN_006dac70(int param_1,undefined4 param_2,int param_3)

{
  if (param_3 != 0) {
    _memcpy((void *)(param_3 + 0x38),&param_2,4);
  }
  FUN_0057d370(param_2,0);
  *(undefined4 *)(param_1 + 0x88) = param_2;
  FUN_0057e3d0(param_2);
  FUN_005921e0(param_2);
  *(undefined4 *)(param_1 + 0x52c) = 1;
  return;
}

