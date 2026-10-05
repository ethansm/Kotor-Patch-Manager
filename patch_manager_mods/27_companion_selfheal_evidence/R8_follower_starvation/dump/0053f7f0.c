// FN 0053f7f0 FUN_0053f7f0 size=642 ret=112 callers=00566410,0069b660,00567f00,005683d0,005479c0,00693860,00568480,00656af0,0066ad10,005e1340,007e5730,0068ea70,005bf940,00669c80,0066ab20,005f6060,0068c9b0,00547940,00568540,005e0fa0,005d4b60,0065be70,00566130,005e0f30,0066a0f0,006942f0,0068cb50,0066a590,00547b40,0053fd50,00669910,00669fc0,00547bb0,0057d790,0068db00,00565310,00695f10,005e1150,005680d0,00693ac0,0069b980,00690d70,00564610,00568020,0066ac60,005648d0,00564770,0057d700,00567e40,00547a90,0057de70,0066a230,005478c0,00567490 callees=0053f7a0,00919723,0053fa80,0054aa30,0053e6f0,0053fc10

void __thiscall
FUN_0053f7f0(int param_1,undefined4 param_2,short param_3,int param_4,undefined4 param_5,int param_6
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
  FUN_0054aa30(local_28);
  FUN_0053fc10();
  ExceptionList = local_10;
  return;
}

