// FN 005644b0 FUN_005644b0 size=339 ret=68 callers=005a7820,005dda60,005cf8a0,005db8f0,005b43c0,005db0a0,005bc1d0,005bc700,005ceed0,005da4d0,005bb920,00565310,005bbf40,005cff50,005bf000,005d25e0,005bfc00,00586b70,005d19b0,005bca60,005b2c10,005dad10,005b5ad0,005bbb70,006d9500,006d7390,005b5360,005648d0,005b3380,0057d4d0,005bb5a0,005c7370,005d75b0,005dec20,005dfbe0 callees=0057d370,00540ca0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_005644b0(int param_1,undefined2 param_2,int param_3,undefined4 param_4,undefined4 param_5,
            uint param_6,undefined4 param_7,float param_8,int param_9,undefined4 param_10,
            uint param_11,uint param_12,int param_13,uint param_14,int param_15,undefined4 param_16,
            undefined4 param_17,uint param_18)

{
  int iVar1;
  uint local_8;
  
  local_8 = param_9 << 1 | param_6;
  if (param_8 != (float)_DAT_00986b38) {
    local_8 = local_8 | 4;
  }
  local_8 = (param_11 & 7) << 4 | param_13 << 3 | local_8;
  if (param_12 != 0) {
    local_8 = (param_12 & 3) << 7 | local_8;
  }
  local_8 = (param_18 & 1) << 10 | (param_14 & 1) << 9 | local_8;
  *(undefined4 *)(param_1 + 0x11b0) = 2;
  iVar1 = *(int *)(param_1 + 0x380);
  *(undefined4 *)(iVar1 + 0x26c) = 0;
  *(undefined4 *)(iVar1 + 0x270) = 0;
  *(undefined4 *)(iVar1 + 0x274) = 0;
  FUN_00540ca0(1,param_2,2,param_3,2,param_3 + 4,2,param_3 + 8,3,&param_4,3,&param_5,1,&local_8,2,
               &param_7,1,&param_10,2,&param_8,2,param_15,2,param_15 + 4,1,&param_16,1,&param_17);
  FUN_0057d370(0x7f000000,0);
  return 1;
}

