// FN 005904c0 FUN_005904c0 size=147 ret=0 callers=007469f0,007511b0 callees=00623320,009196fd,005936d0,007a1780

undefined4 __fastcall FUN_005904c0(int param_1)

{
  int iVar1;
  undefined4 local_8;
  
  local_8 = 0;
  if ((*(int *)(param_1 + 0xb00) != 0) && (iVar1 = FUN_007a1780(), iVar1 == 0)) {
    iVar1 = FUN_005936d0();
    if (iVar1 != 0) {
      FUN_00623320();
      FUN_009196fd(iVar1);
    }
    local_8 = 1;
  }
  return local_8;
}

