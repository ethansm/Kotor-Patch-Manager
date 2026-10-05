// FN 00590430 FUN_00590430 size=132 ret=0 callers=0051cba0,0059de30,005df920,0051cb10,0057a560,00568540,008b0320,008b05e0,0065e670 callees=00623320,007f2a90,009196fd,007a1780

void __fastcall FUN_00590430(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0xb00) != 0) {
    while (iVar1 = FUN_007a1780(), iVar1 == 0) {
      iVar1 = FUN_007f2a90();
      if (iVar1 != 0) {
        FUN_00623320();
        FUN_009196fd(iVar1);
      }
    }
  }
  return;
}

