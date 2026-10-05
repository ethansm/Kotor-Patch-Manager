// FN 00540ca0 FUN_00540ca0 size=642 ret=112 callers=00566410,005d25e0,005be1f0,005644b0,005bca60,005c7370,0057d4d0,005bbf40,005cf660,005db8f0,005b43c0,005cf8a0,005dfbe0,0068db00,00565310,005bc1d0,005b3380,005bbb70,005dad10,005646c0,00670d10,005bc700,005b2c10,005dda60,005d19b0,005bf000,005cff50,00568540,005da4d0,005bb5a0,005b5360,005bb920,0065be70,00566130,005648d0,005dec20,006d9500,005ceed0,005b5ad0,005d75b0,005be950,0066a0f0,0066a230,00567490,006d7390,005db0a0 callees=0053f7a0,00919723,0053fa80,0054aa10,0053e6f0,0053fc10

void __thiscall
FUN_00540ca0(int param_1,undefined4 param_2,short param_3,int param_4,undefined4 param_5,int param_6
            ,undefined4 param_7,int param_8,undefined4 param_9,int param_10,undefined4 param_11,
            int param_12,undefined4 param_13,int param_14,undefined4 param_15,int param_16,
            undefined4 param_17,int param_18,undefined4 param_19,int param_20,undefined4 param_21,
            int param_22,undefined4 param_23,int param_24,undefined4 param_25,int param_26,
            undefined4 param_27,int param_28,undefined4 param_29)

{
  undefined2 uVar1;
  int iVar2;
  undefined4 *local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009821bb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_00919723(0x74,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar2 == 0) {
    local_28 = (undefined4 *)0x0;
  }
  else {
    local_28 = (undefined4 *)FUN_0053e6f0();
  }
  local_8 = 0xffffffff;
  *local_28 = param_2;
  local_28[0x1c] = 1;
  if (param_3 == -1) {
    uVar1 = FUN_0053f7a0();
    *(undefined2 *)(local_28 + 0x1b) = uVar1;
  }
  else if (param_3 == -2) {
    *(undefined2 *)(local_28 + 0x1b) = *(undefined2 *)(param_1 + 0x16);
  }
  else {
    *(short *)(local_28 + 0x1b) = param_3;
  }
  if (param_4 != 0) {
    FUN_0053fa80(local_28,0,param_4,param_5);
    if (param_6 != 0) {
      FUN_0053fa80(local_28,1,param_6,param_7);
      if (param_8 != 0) {
        FUN_0053fa80(local_28,2,param_8,param_9);
        if (param_10 != 0) {
          FUN_0053fa80(local_28,3,param_10,param_11);
          if (param_12 != 0) {
            FUN_0053fa80(local_28,4,param_12,param_13);
            if (param_14 != 0) {
              FUN_0053fa80(local_28,5,param_14,param_15);
              if (param_16 != 0) {
                FUN_0053fa80(local_28,6,param_16,param_17);
                if (param_18 != 0) {
                  FUN_0053fa80(local_28,7,param_18,param_19);
                  if (param_20 != 0) {
                    FUN_0053fa80(local_28,8,param_20,param_21);
                    if (param_22 != 0) {
                      FUN_0053fa80(local_28,9,param_22,param_23);
                      if (param_24 != 0) {
                        FUN_0053fa80(local_28,10,param_24,param_25);
                        if (param_26 != 0) {
                          FUN_0053fa80(local_28,0xb,param_26,param_27);
                          if (param_28 != 0) {
                            FUN_0053fa80(local_28,0xc,param_28,param_29);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  FUN_0054aa10(local_28);
  FUN_0053fc10();
  ExceptionList = local_10;
  return;
}

