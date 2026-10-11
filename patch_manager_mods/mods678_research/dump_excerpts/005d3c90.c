// FN 005d3c90 FUN_005d3c90 size=575 ret=4 callers=005d2f80,005bd6a0 callees=0071b1d0,00734010,00733570,009370c7,00733780,006042a0,00733540,00603f60

undefined4 __thiscall FUN_005d3c90(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  short *psVar3;
  char *_Str2;
  char *_Str1;
  undefined1 local_34 [8];
  undefined4 local_2c;
  uint local_28;
  char local_21;
  int local_20;
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953770;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  iVar2 = FUN_00603f60(0x3e,0);
  if (iVar2 == 0) {
    local_2c = 1;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    local_21 = '\0';
    local_20 = 0;
    for (local_1c = 0; local_1c < *(int *)(param_2 + 0x28c); local_1c = local_1c + 1) {
      iVar2 = FUN_006042a0(local_1c);
      if ((*(byte *)(iVar2 + 0x11) == 0xff) ||
         ((1 << (*(byte *)(iVar2 + 0x11) & 0x1f) & *(uint *)(param_2 + 0x2d4)) != 0)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
      if ((bVar1) && (psVar3 = (short *)FUN_006042a0(local_1c), *psVar3 == 0x3e)) {
        local_20 = local_20 + 1;
        iVar2 = FUN_006042a0(local_1c);
        local_28 = (uint)*(ushort *)(iVar2 + 2);
        if (local_28 == 0) {
          if (*(int *)(param_1 + 0x1200) != 0) {
            local_21 = '\x01';
          }
        }
        else {
          FUN_00733570("ExpectedTag");
          local_8._0_1_ = 1;
          FUN_0071b1d0(local_28,local_34,local_18);
          local_8 = (uint)local_8._1_3_ << 8;
          FUN_00733780();
          _Str2 = (char *)FUN_00734010();
          _Str1 = (char *)FUN_00734010();
          iVar2 = __stricmp(_Str1,_Str2);
          if (iVar2 == 0) {
            local_21 = '\x01';
          }
        }
      }
    }
    if (local_20 < 1) {
      local_8 = 0xffffffff;
      FUN_00733780();
      local_2c = 1;
    }
    else if (local_21 == '\0') {
      local_8 = 0xffffffff;
      FUN_00733780();
      local_2c = 0;
    }
    else {
      local_8 = 0xffffffff;
      FUN_00733780();
      local_2c = 1;
    }
  }
  ExceptionList = local_10;
  return local_2c;
}

