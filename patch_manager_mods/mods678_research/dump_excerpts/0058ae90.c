// FN 0058ae90 FUN_0058ae90 size=53 ret=0 callers=0068f440,006e24b0,0079d840,005fdaf0,0066b8a0,005b43c0,00577fa0,0080efb0,005c3f70,0089fbd0,0057a560,005fe4b0,00578850,006db4a0,005fd9a0,0076d9e0,00586750,00815d00,008d34e0,005c6340,0069b060,00534600,0053af20,0056c310,005b8000,0056d350,006cf5d0,005d75b0,0079f430,0056d6a0,005f8c60,00798d70,00594810,008a8df0 callees=0054a840

undefined4 FUN_0058ae90(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_0054a840();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_0054a840();
    uVar3 = (**(code **)(*piVar2 + 0x2c))(piVar2,uVar3);
  }
  return uVar3;
}

