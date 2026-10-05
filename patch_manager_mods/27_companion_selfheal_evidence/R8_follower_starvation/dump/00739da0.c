// FN 00739da0 FUN_00739da0 size=181 ret=4 callers=0054aa10 callees=00919723

undefined4 * __thiscall FUN_00739da0(int *param_1,undefined4 param_2)

{
  undefined4 *local_18;
  
  local_18 = (undefined4 *)FUN_00919723(0xc);
  if (local_18 == (undefined4 *)0x0) {
    local_18 = (undefined4 *)0x0;
  }
  else {
    local_18[2] = 0;
    local_18[1] = 0;
    *local_18 = 0;
  }
  local_18[1] = *param_1;
  *local_18 = 0;
  if (*param_1 != 0) {
    *(undefined4 **)*param_1 = local_18;
  }
  *param_1 = (int)local_18;
  local_18[2] = param_2;
  if (param_1[1] == 0) {
    param_1[1] = (int)local_18;
  }
  param_1[2] = param_1[2] + 1;
  return local_18;
}

