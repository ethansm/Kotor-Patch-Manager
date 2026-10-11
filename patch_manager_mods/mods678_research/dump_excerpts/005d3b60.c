// FN 005d3b60 FUN_005d3b60 size=298 ret=4 callers=005d2f80,005bd6a0 callees=006042a0,00603f60

undefined4 __thiscall FUN_005d3b60(int *param_1,int param_2)

{
  ushort uVar1;
  bool bVar2;
  ushort uVar3;
  int iVar4;
  short *psVar5;
  int local_c;
  
  iVar4 = FUN_00603f60(0x3c,0);
  if (iVar4 != 0) {
    for (local_c = 0; local_c < *(int *)(param_2 + 0x28c); local_c = local_c + 1) {
      iVar4 = FUN_006042a0(local_c);
      if ((*(byte *)(iVar4 + 0x11) == 0xff) ||
         ((1 << (*(byte *)(iVar4 + 0x11) & 0x1f) & *(uint *)(param_2 + 0x2d4)) != 0)) {
        bVar2 = true;
      }
      else {
        bVar2 = false;
      }
      if ((bVar2) && (psVar5 = (short *)FUN_006042a0(local_c), *psVar5 == 0x3c)) {
        iVar4 = FUN_006042a0(local_c);
        uVar1 = *(ushort *)(iVar4 + 2);
        uVar3 = (**(code **)(*param_1 + 0x88))();
        if (uVar1 != (uVar3 & 0xff)) {
          if ((uVar1 != 0) && (uVar1 != 1)) {
            return 0;
          }
          if ((uVar3 & 0xff) != 2) {
            return 0;
          }
        }
      }
    }
  }
  return 1;
}

