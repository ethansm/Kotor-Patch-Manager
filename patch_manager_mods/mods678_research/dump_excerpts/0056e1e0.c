// FN 0056e1e0 FUN_0056e1e0 size=107 ret=0 callers=006e3a40,006d7390 callees=006d0620,006d5200

bool FUN_0056e1e0(void)

{
  int iVar1;
  bool bVar2;
  
  iVar1 = FUN_006d0620(0x10);
  if (iVar1 == 0) {
    bVar2 = false;
  }
  else {
    iVar1 = FUN_006d5200(*(undefined4 *)(iVar1 + 0xc));
    bVar2 = *(char *)(iVar1 + 0x1a) != '\0';
  }
  return bVar2;
}

