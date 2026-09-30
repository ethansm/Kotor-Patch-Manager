// FN 00797df0 FUN_00797df0 size=199 ret=0 callers=00797d90,0073fe60 callees=00879e60,007aa460

void __fastcall FUN_00797df0(int param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  while( true ) {
    if (6 < local_8) goto LAB_00797e82;
    if (*(int *)(param_1 + 0x104 + local_8 * 4) != 0) break;
    local_8 = local_8 + 1;
  }
  for (local_8 = 0; local_8 < 7; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1 + 0x104 + local_8 * 4) = 0;
  }
  *(undefined4 *)(param_1 + 0x100) = 1;
  *(undefined4 *)(param_1 + 0xf8) = 0;
  *(undefined4 *)(param_1 + 0xfc) = 0;
  FUN_00879e60();
LAB_00797e82:
  if ((*(int *)(param_1 + 0x18) != 0) && (*(int *)(*(int *)(param_1 + 0x18) + 0x48) != 0)) {
    FUN_007aa460(0);
  }
  return;
}

