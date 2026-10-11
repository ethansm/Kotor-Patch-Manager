// FN 005d3ee0 FUN_005d3ee0 size=653 ret=4 callers=005d2f80,005bd6a0 callees=0071bcf0,00733570,00733780,006f0190,006042a0,00603f60

undefined4 __thiscall FUN_005d3ee0(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  short *psVar3;
  uint uVar4;
  undefined1 *puVar5;
  int *piVar6;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953798;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_00603f60(0x45,0);
  if (iVar2 != 0) {
    for (local_1c = 0; local_1c < *(int *)(param_2 + 0x28c); local_1c = local_1c + 1) {
      iVar2 = FUN_006042a0(local_1c);
      if ((*(byte *)(iVar2 + 0x11) == 0xff) ||
         ((1 << (*(byte *)(iVar2 + 0x11) & 0x1f) & *(uint *)(param_2 + 0x2d4)) != 0)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
      if ((bVar1) && (psVar3 = (short *)FUN_006042a0(local_1c), *psVar3 == 0x45)) {
        iVar2 = FUN_006042a0(local_1c);
        local_14 = (uint)*(ushort *)(iVar2 + 2);
        FUN_00733570("Value");
        local_8 = 0;
        iVar2 = FUN_006042a0(local_1c);
        piVar6 = &local_18;
        puVar5 = local_28;
        uVar4 = (uint)*(ushort *)(iVar2 + 6);
        FUN_006f0190(0x1a);
        FUN_0071bcf0(uVar4,puVar5,piVar6);
        local_8 = 0xffffffff;
        FUN_00733780();
        switch(local_14) {
        case 0:
          local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0xed);
          break;
        case 1:
          local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0xef);
          break;
        case 2:
          local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0xf1);
          break;
        case 3:
          local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0xf1);
          break;
        case 4:
          local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0xf5);
          break;
        case 5:
          local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0xf7);
        }
        if ((int)local_20 < local_18) {
          ExceptionList = local_10;
          return 0;
        }
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

