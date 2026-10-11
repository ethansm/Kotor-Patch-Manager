// FN 005d3a20 FUN_005d3a20 size=311 ret=4 callers=005d2f80,005bd6a0 callees=006b83f0,006b8440,006042a0,00603f60

undefined4 FUN_005d3a20(int param_1)

{
  bool bVar1;
  char cVar2;
  int iVar3;
  short *psVar4;
  int local_8;
  
  iVar3 = FUN_00603f60(0x39,0);
  if ((iVar3 != 0) && (iVar3 = FUN_006b83f0(0x5d), iVar3 == 0)) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x28c); local_8 = local_8 + 1) {
      iVar3 = FUN_006042a0(local_8);
      if ((*(byte *)(iVar3 + 0x11) == 0xff) ||
         ((1 << (*(byte *)(iVar3 + 0x11) & 0x1f) & *(uint *)(param_1 + 0x2d4)) != 0)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
      if ((bVar1) && (psVar4 = (short *)FUN_006042a0(local_8), *psVar4 == 0x39)) {
        iVar3 = FUN_006042a0(local_8);
        cVar2 = FUN_006b8440(*(undefined2 *)(iVar3 + 2));
        if (cVar2 == '\0') {
          return 0;
        }
      }
    }
  }
  return 1;
}

