// FN 008a8880 FUN_008a8880 size=464 ret=8 callers= callees=0040eaa0,004122a0,0073f750,007ca060,00623320,008a8b30,00740b80,0073fee0

void __thiscall FUN_008a8880(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  uint local_8;
  
  if (param_3 != 0) {
    switch(param_2) {
    case 0x28:
    case 0x2d:
    case 0x2e:
    case 0xdf:
      uVar2 = 0;
      FUN_0073f750(0);
      iVar1 = FUN_007ca060(uVar2);
      if (iVar1 != 0) {
        FUN_0073fee0(0,1);
      }
      break;
    case 0x2a:
    case 0xce:
      FUN_00740b80(0,0,1);
      FUN_004122a0(1);
      *(uint *)(param_1 + 0x24d0) = *(uint *)(param_1 + 0x24d0) | 4;
      FUN_00623320();
      break;
    case 0x2f:
      iVar1 = FUN_0073f750();
      if (*(byte *)(iVar1 + 0xfc1) == 6) {
        local_8 = 0;
      }
      else {
        local_8 = *(byte *)(iVar1 + 0xfc1) - 1;
        if ((int)local_8 < 0) {
          local_8 = 0xb;
        }
      }
      FUN_008a8b30(local_8);
      break;
    case 0x30:
      iVar1 = FUN_0073f750();
      if (*(byte *)(iVar1 + 0xfc1) == 0) {
        local_8 = 6;
      }
      else {
        local_8 = *(byte *)(iVar1 + 0xfc1) + 1;
        if (0xb < local_8) {
          local_8 = 0;
        }
      }
      FUN_008a8b30(local_8);
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0x878) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0x878) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}

