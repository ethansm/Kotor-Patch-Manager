// FN 0053f560 FUN_0053f560 size=123 ret=4 callers=00566410,00547bb0,005648d0,005683d0,00565310,0057b4c0,00568480,00568540 callees=007a1780,00561430,007a1720,0058c370

uint * __thiscall FUN_0053f560(undefined4 param_1,ushort param_2)

{
  int iVar1;
  uint *local_c;
  int local_8;
  
  iVar1 = FUN_007a1780(param_1);
  if (iVar1 != 1) {
    local_8 = FUN_007a1720();
    local_c = (uint *)FUN_00561430(local_8);
    while (local_8 != 0) {
      if (*local_c == (uint)param_2) {
        return local_c;
      }
      local_c = (uint *)FUN_0058c370(&local_8);
    }
  }
  return (uint *)0x0;
}

