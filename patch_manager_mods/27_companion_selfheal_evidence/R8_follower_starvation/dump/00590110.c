// FN 00590110 FUN_00590110 size=98 ret=0 callers=005648d0,00589c30,00565310,00547bb0,006d9500 callees=007a1720,00561430

undefined4 __fastcall FUN_00590110(int param_1)

{
  int iVar1;
  int *piVar2;
  
  if (((*(int *)(param_1 + 0xb00) != 0) && (iVar1 = FUN_007a1720(), iVar1 != 0)) &&
     (piVar2 = (int *)FUN_00561430(iVar1), *piVar2 <= *(int *)(param_1 + 0xa94))) {
    return 1;
  }
  return 0;
}

