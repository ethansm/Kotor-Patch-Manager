// FN 0074e910 FUN_0074e910 size=684 ret=4 callers=00747210,0074b3f0 callees=0074ebc0,00745d40

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0074e910(int param_1,uint param_2)

{
  int iVar1;
  bool bVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  if (param_2 != (*(uint *)(param_1 + 0xa904) >> 1 & 1)) {
    bVar2 = true;
    *(uint *)(param_1 + 0xa904) = *(uint *)(param_1 + 0xa904) & 0xfffffffd | (param_2 & 1) << 1;
    if ((param_2 == 0) && ((*(uint *)(param_1 + 0x6e54) >> 1 & 1) == 0)) {
      bVar2 = false;
    }
    for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
      iVar1 = param_1 + 0x644c + local_8 * 0x148;
      *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffffd | (param_2 & 1) << 1;
    }
    *(uint *)(param_1 + 0x6e54) = *(uint *)(param_1 + 0x6e54) & 0xfffffffd | (param_2 & 1) << 1;
    *(uint *)(param_1 + 0x68e4) = *(uint *)(param_1 + 0x68e4) & 0xfffffffd | (param_2 & 1) << 1;
    *(uint *)(param_1 + 0x6ab4) = *(uint *)(param_1 + 0x6ab4) & 0xfffffffd | (param_2 & 1) << 1;
    *(uint *)(param_1 + 0x6c84) = *(uint *)(param_1 + 0x6c84) & 0xfffffffd | (param_2 & 1) << 1;
    if (*(int *)(param_1 + 0x732c) == 0) {
      *(uint *)(param_1 + 0x6f9c) = *(uint *)(param_1 + 0x6f9c) & 0xfffffffd | (param_2 & 1) << 1;
      *(uint *)(param_1 + 0x70e4) = *(uint *)(param_1 + 0x70e4) & 0xfffffffd | (param_2 & 1) << 1;
    }
    for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
      iVar1 = param_1 + 0x5f2c + local_8 * 0x148;
      *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffffd | (param_2 & 1) << 1;
    }
    if (param_2 == 0) {
      local_c = -(uint)*(byte *)(param_1 + 0xa900);
      *(undefined4 *)(param_1 + 0x7334) = _DAT_00987474;
    }
    else {
      local_c = (uint)*(byte *)(param_1 + 0xa900);
    }
    if (bVar2) {
      FUN_0074ebc0(param_1 + 0x596c,local_c);
      FUN_0074ebc0(param_1 + 0x5ab4,local_c);
      FUN_0074ebc0(param_1 + 0x5d5c,local_c);
      *(uint *)(param_1 + 0x5d48) = *(int *)(param_1 + 0x5d48) + local_c;
      for (local_8 = 0; local_8 < 0xd; local_8 = local_8 + 1) {
        FUN_00745d40(local_c);
      }
    }
  }
  return;
}

