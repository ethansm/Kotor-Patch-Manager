// FN 00605e80 FUN_00605e80 size=180 ret=4 callers=008c9460,008ddf20,005f4e60,008d6090,008df680,00566650,0062ba20,008cdb00,00606460,00606e20,008cfb10,00580900,008c8880,008b7700,008dac20,005f4ed0,005bde40,005f5770,005bdb60,006010b0,00580330,0058b5a0,00622930,008cfd90 callees=0049b840

undefined4 __thiscall FUN_00605e80(int param_1,int param_2)

{
  int local_8;
  
  if (param_2 != 0) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x10); local_8 = local_8 + 1) {
      if (*(int *)(param_2 + 0x14) == *(int *)(*(int *)(param_1 + 0xc) + local_8 * 4)) {
        FUN_0049b840(local_8);
        if ((*(uint *)(param_2 + 0x2c8) >> 7 & 1) != 0) {
          *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
          *(uint *)(param_2 + 0x2c8) = *(uint *)(param_2 + 0x2c8) & 0xffffff7f;
        }
        return 1;
      }
    }
  }
  return 0;
}

