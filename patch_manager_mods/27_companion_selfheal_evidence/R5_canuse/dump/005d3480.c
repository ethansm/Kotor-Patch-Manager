// FN 005d3480 FUN_005d3480 size=426 ret=4 callers=005d2f80,005bd6a0 callees=006b83f0,006042a0,00603f60

undefined4 __thiscall FUN_005d3480(int param_1,int param_2)

{
  short sVar1;
  bool bVar2;
  int iVar3;
  undefined4 uVar4;
  short *psVar5;
  int local_8;
  
  iVar3 = FUN_00603f60(0x2b,0);
  if (iVar3 == 0) {
    uVar4 = 1;
  }
  else {
    iVar3 = FUN_006b83f0(0x5d);
    if (iVar3 == 0) {
      sVar1 = *(short *)(*(int *)(param_1 + 0x1198) + 0x18a);
      for (local_8 = 0; local_8 < *(int *)(param_2 + 0x28c); local_8 = local_8 + 1) {
        iVar3 = FUN_006042a0(local_8);
        if ((*(byte *)(iVar3 + 0x11) == 0xff) ||
           ((1 << (*(byte *)(iVar3 + 0x11) & 0x1f) & *(uint *)(param_2 + 0x2d4)) != 0)) {
          bVar2 = true;
        }
        else {
          bVar2 = false;
        }
        if ((bVar2) && (psVar5 = (short *)FUN_006042a0(local_8), *psVar5 == 0x2b)) {
          iVar3 = FUN_006042a0(local_8);
          switch(*(undefined2 *)(iVar3 + 2)) {
          case 0:
            return 1;
          case 1:
            if ((sVar1 < 0x3c) && (0x28 < sVar1)) {
              return 1;
            }
            break;
          case 2:
            if (0x3b < sVar1) {
              return 1;
            }
            break;
          case 3:
            if (sVar1 < 0x29) {
              return 1;
            }
            break;
          case 4:
            if (sVar1 == 0) {
              return 1;
            }
            break;
          case 5:
            if (sVar1 == 100) {
              return 1;
            }
          }
        }
      }
      uVar4 = 0;
    }
    else {
      uVar4 = 1;
    }
  }
  return uVar4;
}

