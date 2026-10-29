// NAMED 0x00418af0 CSWGuiControl::AddEvent evidence=ev_1a3c2b237a698cf64bae6a915368abdda04643cc45c7c3fbc5b3341f8e194051 names=1 exe=6a522e71631d

void CSWGuiControl__AddEvent(int param_1,undefined4 param_2,int param_3)

{
  int in_ECX;
  undefined4 *puVar1;
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (*(int *)(in_ECX + 0x40) <= local_c) {
      if (param_3 != 0) {
        FUN_008a5fb0(param_2,param_3,param_1);
      }
      return;
    }
    puVar1 = (undefined4 *)(local_c * 0xc + *(int *)(in_ECX + 0x3c));
    if (puVar1[2] == param_1) break;
    local_c = local_c + 1;
  }
  if (param_3 != 0) {
    *puVar1 = param_2;
    puVar1[1] = param_3;
    return;
  }
  FUN_007a1940(local_c);
  return;
}

