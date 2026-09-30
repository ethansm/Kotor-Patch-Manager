// FN 008a7520 FUN_008a7520 size=195 ret=0 callers= callees=0051c8d0,005f7a30,006169e0,00605f40,0041d290

void __fastcall FUN_008a7520(int param_1)

{
  undefined4 *puVar1;
  int local_8;
  
  FUN_0041d290();
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x24b4); local_8 = local_8 + 1) {
    puVar1 = *(undefined4 **)(*(int *)(param_1 + 0x24b0) + local_8 * 4);
    if (puVar1 != (undefined4 *)0x0) {
      (**(code **)*puVar1)(1);
    }
    *(undefined4 *)(*(int *)(param_1 + 0x24b0) + local_8 * 4) = 0;
  }
  FUN_006169e0(1);
  param_1 = param_1 + 0x24bc;
  FUN_0051c8d0(param_1);
  FUN_005f7a30();
  FUN_00605f40(param_1);
  return;
}

