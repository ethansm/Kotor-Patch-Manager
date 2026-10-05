// FN 00590270 FUN_00590270 size=434 ret=8 callers=00591890,00590a50,00591670,00590d10,005919b0,00590930,005917e0 callees=0058c370,00623320,0054aa30,009196fd,007a1720,005210f0,005210d0,00561430

void __thiscall FUN_00590270(int param_1,int *param_2,int param_3)

{
  uint uVar1;
  int *piVar2;
  int local_10;
  int local_8;
  
  if (*(int *)(param_1 + 0xb00) == 0) {
    if (param_2 != (int *)0x0) {
      FUN_00623320();
      FUN_009196fd(param_2);
    }
  }
  else {
    uVar1 = FUN_005210f0();
    if (uVar1 < 4) {
      if (param_3 == -1) {
        local_8 = FUN_007a1720();
        while (local_8 != 0) {
          piVar2 = (int *)FUN_00561430(local_8);
          if (*param_2 < *piVar2) {
            FUN_005210d0(param_2,local_8);
            return;
          }
          FUN_0058c370(&local_8);
        }
        FUN_0054aa30(param_2);
      }
      else {
        local_8 = FUN_007a1720();
        for (local_10 = 0; (local_8 != 0 && (local_10 < param_3)); local_10 = local_10 + 1) {
          FUN_0058c370(&local_8);
        }
        if (local_8 == 0) {
          FUN_0054aa30(param_2);
        }
        else {
          FUN_005210d0(param_2,local_8);
        }
      }
    }
    else if (param_2 != (int *)0x0) {
      FUN_00623320();
      FUN_009196fd(param_2);
    }
  }
  return;
}

