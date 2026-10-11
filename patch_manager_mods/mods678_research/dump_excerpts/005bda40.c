// FN 005bda40 FUN_005bda40 size=278 ret=8 callers=005bd610,005d2f80,005bd6a0,008ac330 callees=006b83f0,006d4fc0,006d5200

undefined4 FUN_005bda40(int param_1)

{
  undefined2 uVar1;
  int iVar2;
  undefined4 uVar3;
  uint local_8;
  
  iVar2 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  if (*(int *)(iVar2 + 4) == 0) {
    uVar3 = 0;
  }
  else {
    iVar2 = FUN_006b83f0(0x5d);
    if (iVar2 == 0) {
      for (local_8 = 0; iVar2 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc)),
          (int)local_8 < (int)(uint)*(byte *)(iVar2 + 0x80); local_8 = local_8 + 1) {
        FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
        uVar1 = FUN_006d4fc0(local_8 & 0xff);
        iVar2 = FUN_006b83f0(uVar1);
        if (iVar2 == 0) {
          return 0;
        }
      }
      uVar3 = 1;
    }
    else {
      uVar3 = 1;
    }
  }
  return uVar3;
}

