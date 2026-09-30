// FN 00796b50 FUN_00796b50 size=70 ret=8 callers=007b3050,007401f0,00798d70 callees=007ce710

void FUN_00796b50(int param_1,undefined4 param_2)

{
  undefined4 local_14;
  
  if (param_1 == 0) {
    local_14 = 0x7f000000;
  }
  else {
    local_14 = *(undefined4 *)(param_1 + 4);
  }
  FUN_007ce710(local_14,param_2);
  return;
}

