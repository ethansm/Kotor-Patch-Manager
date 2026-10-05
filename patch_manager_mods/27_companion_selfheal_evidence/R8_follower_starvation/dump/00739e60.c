// FN 00739e60 FUN_00739e60 size=184 ret=4 callers=0054aa30 callees=00919723

int * __thiscall FUN_00739e60(int *param_1,int param_2)

{
  int *local_18;
  
  local_18 = (int *)FUN_00919723(0xc);
  if (local_18 == (int *)0x0) {
    local_18 = (int *)0x0;
  }
  else {
    local_18[2] = 0;
    local_18[1] = 0;
    *local_18 = 0;
  }
  *local_18 = param_1[1];
  local_18[1] = 0;
  if (param_1[1] != 0) {
    *(int **)(param_1[1] + 4) = local_18;
  }
  param_1[1] = (int)local_18;
  local_18[2] = param_2;
  if (*param_1 == 0) {
    *param_1 = (int)local_18;
  }
  param_1[2] = param_1[2] + 1;
  return local_18;
}

