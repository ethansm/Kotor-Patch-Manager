// FN 00563c90 FUN_00563c90 size=134 ret=4 callers=005680d0,00547bb0,0065b4f0,0057de70,00568540,0065be70,00545760,0057b280,005683d0,005648d0,00565310,00566410,006c7a30,00568480,0065b270,00656af0,0065e670 callees=00571980,0051c600,005718a0,00541080

void __thiscall FUN_00563c90(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = FUN_0051c600(*(undefined4 *)(param_1 + 4));
  if (iVar1 != 0) {
    FUN_005718a0();
    if (((param_2 == 1) || (param_2 == 2)) &&
       (iVar1 = FUN_00571980(*(undefined1 *)(param_1 + 0x1116)), iVar1 == 1)) {
      FUN_00541080(1);
    }
    if ((param_2 == 8) || (param_2 == 4)) {
      FUN_00541080(1);
    }
  }
  return;
}

