// FN 0072e730 FUN_0072e730 size=266 ret=12 callers=0072e030,0072c7e0 callees=0054aa10,00919723,0091c630

void FUN_0072e730(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *local_24;
  undefined4 *local_20;
  
  local_20 = (undefined4 *)FUN_00919723(0x10);
  if (local_20 == (undefined4 *)0x0) {
    local_20 = (undefined4 *)0x0;
  }
  else {
    _memset(local_20,0,0x10);
  }
  *local_20 = param_3;
  local_20[2] = 0;
  local_20[1] = 0;
  local_24 = (undefined4 *)FUN_00919723(0x20);
  if (local_24 == (undefined4 *)0x0) {
    local_24 = (undefined4 *)0x0;
  }
  else {
    *local_24 = CExoInputEventDesc::vftable;
    local_24[1] = 0;
    local_24[2] = 0;
    local_24[3] = 0;
  }
  local_20[3] = local_24;
  *(undefined4 *)(local_20[3] + 0x18) = param_1;
  *(undefined4 *)(local_20[3] + 0x1c) = param_2;
  *(undefined4 *)(local_20[3] + 0x10) = 0;
  *(undefined4 *)(local_20[3] + 0x14) = 0;
  FUN_0054aa10(local_20);
  return;
}

