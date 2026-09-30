// FN 00842d00 FUN_00842d00 size=53 ret=0 callers=007b40f0,0077df90,007aaec0,007ac470,0077e0c0,0079d840,007c93e0,007c9850,00842d40,0077e370,00841180,00742a00,008427a0,00841c00,007c84e0,007eef90,00843ab0,00843b80,0080d050,007f1c00 callees=007f2540

undefined4 FUN_00842d00(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_007f2540();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_007f2540();
    uVar3 = (**(code **)(*piVar2 + 0x48))(piVar2,uVar3);
  }
  return uVar3;
}

