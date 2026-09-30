// FN 006dab20 FUN_006dab20 size=93 ret=4 callers=005d29f0,006da4f0,006d7390 callees=005915f0,0057d370,005b7ae0

void FUN_006dab20(void)

{
  undefined4 uStack00000004;
  undefined4 uVar1;
  int *extraout_var;
  int *piVar2;
  
  uStack00000004 = 1;
  FUN_0057d370(0x7f000000,0);
  uVar1 = 10000;
  piVar2 = extraout_var;
  (**(code **)(*extraout_var + 0x7c))(10000);
  FUN_005b7ae0(0,1);
  FUN_005915f0(uVar1,piVar2,piVar2[0x437]);
  return;
}

