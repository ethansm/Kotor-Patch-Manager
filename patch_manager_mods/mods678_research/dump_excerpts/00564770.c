// FN 00564770 FUN_00564770 size=339 ret=68 callers=005bf940,005648d0,00565310,0065b4f0,0068cee0,0066a590,0068cb50,005e1150,005d4b60 callees=0053f7f0,0057d370

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_00564770(int param_1,undefined2 param_2,int param_3,undefined4 param_4,undefined4 param_5,
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
  FUN_0053f7f0(1,param_2,2,param_3,2,param_3 + 4,2,param_3 + 8,3,&param_4,3,&param_5,1,&local_8,2,
               &param_7,1,&param_10,2,&param_8,2,param_15,2,param_15 + 4,1,&param_16,1,&param_17);
  FUN_0057d370(0x7f000000,0);
  return 1;
}

