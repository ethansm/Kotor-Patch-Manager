// FN 007b3870 FUN_007b3870 size=86 ret=12 callers=00788d90,007b3a00 callees=0072fc50,00413a80

void __thiscall FUN_007b3870(int param_1,uint *param_2,uint *param_3,uint *param_4)

{
  uint *extraout_EAX;
  
  if (DAT_00a1b48c != (SchedulerResourceManagement *)0x0) {
    Concurrency::details::SchedulerResourceManagement::Statistics
              (DAT_00a1b48c,param_2,param_3,param_4);
    param_2 = extraout_EAX;
  }
  if (*(int *)(param_1 + 0x274) != 0) {
    FUN_00413a80(param_2,param_3,param_4);
  }
  return;
}

