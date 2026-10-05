// FN 005d4190 FUN_005d4190 size=164 ret=4 callers=005d2f80,005bd6a0 callees=00734f60

undefined4 FUN_005d4190(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00734f60("baodur");
  if ((iVar1 == 0) ||
     (((((*(int *)(param_1 + 0xc) != 0x23 && (*(int *)(param_1 + 0xc) != 0x24)) &&
        (*(int *)(param_1 + 0xc) != 0x25)) &&
       ((*(int *)(param_1 + 0xc) != 0x61 && (*(int *)(param_1 + 0xc) != 0x62)))) &&
      ((*(int *)(param_1 + 0xc) != 99 &&
       ((*(int *)(param_1 + 0xc) != 100 && (*(int *)(param_1 + 0xc) != 0x66)))))))) {
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

