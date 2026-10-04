// FN 00539d30 FUN_00539d30 size=37 ret=4 callers=0051c8b0,00539f30,00539e30 callees=

uint FUN_00539d30(uint param_1)

{
  if (param_1 == 0x7f000000) {
    param_1 = 0x7f000000;
  }
  else {
    param_1 = ~DAT_0099449c & param_1;
  }
  return param_1;
}

