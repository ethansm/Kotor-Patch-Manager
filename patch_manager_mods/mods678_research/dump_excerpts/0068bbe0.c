// FN 0068bbe0 FUN_0068bbe0 size=121 ret=8 callers= callees=006fdaf0,006fd9c0,0051c100

undefined4 FUN_0068bbe0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = 0;
    local_c = FUN_0051c100(local_10);
    if (local_c != 0) {
      local_8 = *(undefined4 *)(local_c + 0x11ac);
    }
    iVar1 = FUN_006fd9c0(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}

