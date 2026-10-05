// FN 00590220 FUN_00590220 size=75 ret=0 callers=006d9500 callees=007f2a90

int * __fastcall FUN_00590220(int param_1)

{
  int *piVar1;
  
  if (*(int *)(param_1 + 0xb00) == 0) {
    piVar1 = (int *)0x0;
  }
  else {
    piVar1 = (int *)FUN_007f2a90();
    if ((piVar1 == (int *)0x0) || (*(int *)(param_1 + 0xa94) < *piVar1)) {
      piVar1 = (int *)0x0;
    }
  }
  return piVar1;
}

