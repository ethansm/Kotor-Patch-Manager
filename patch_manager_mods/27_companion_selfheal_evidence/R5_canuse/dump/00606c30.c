// FN 00606c30 FUN_00606c30 size=200 ret=0 callers=005d2f80,005bd6a0,00645830 callees=0071bcf0,00606a80,00733570,00733780

char FUN_00606c30(void)

{
  uint uVar1;
  uint uVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954fb8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = *(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x118);
  for (local_18 = 0; local_18 < *(int *)(local_1c + 0x24); local_18 = local_18 + 1) {
    FUN_00733570("MAXSINGLEITEMVALUE");
    local_8 = 0;
    uVar3 = extraout_var;
    FUN_0071bcf0(local_18,local_28,&local_20);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1,uVar3);
    uVar2 = FUN_00606a80();
    if (uVar2 <= local_20) break;
  }
  ExceptionList = local_10;
  return (char)local_18 + '\x01';
}

