// FN 00615cc0 FUN_00615cc0 size=145 ret=4 callers=0077bcf0,0077c340,0087c590,008b3c00,008b1250,008ad930,008dddd0,0058a610,0074e0b0,0077c1b0,008d0780,008cd8e0,00843500,008427a0 callees=006d4e60,006d6e30,00710840

undefined4 __thiscall FUN_00615cc0(int param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = FUN_006d6e30();
  if (iVar1 == 0) {
    FUN_00710840(&DAT_00986925);
  }
  else if (*(char *)(iVar1 + 0x18) == '\0') {
    FUN_006d4e60(param_2,*(undefined1 *)(param_1 + 0x2a4),0x6d);
  }
  else {
    FUN_006d4e60(param_2,*(undefined1 *)(param_1 + 0x2a6),0x6d);
  }
  return param_2;
}

