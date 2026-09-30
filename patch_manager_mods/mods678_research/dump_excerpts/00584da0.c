// FN 00584da0 FUN_00584da0 size=333 ret=8 callers=005dda60,005a2710,005db8f0,005b7ae0,005f2fe0,0057b940,00661c30,00617b40,006971f0,005bd6a0,005d4830,0059b520,00660710,005d25e0,00599f00,0057e140,005a2c70,00622930,00566130,005813c0,005a0c50,00600380,00581630,005bd610,0057de70,005d19b0,0056efd0,0057b280,006bc9a0,006d7390,005b96f0,00541fd0,005a65d0,005dfbe0,0066cb50,0059fd20,00549f70,0059cba0,00568540,0065be70,00567490,00542e00,006b6bd0,00541430,005a8660,005fe0c0,005814a0,005479c0,00690a90,0059c550,005a5990,00543ae0,0059b8d0,005b8000,00581710,00567f00,00597d90,00568020,005a06f0,005dec20,0056ceb0,00597750,005433a0 callees=005e2540,005e25d0,00648cc0,00919723,005e1bd0,009196fd,0051c600,0051c2e0

void __thiscall FUN_00584da0(int param_1,undefined2 param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int local_40;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0eb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_0051c600(*(undefined4 *)(param_1 + 4));
  if (iVar2 != 0) {
    if (param_3 == 0) {
      iVar3 = FUN_00919723(0x34,uVar1);
      local_8 = 0;
      if (iVar3 == 0) {
        local_40 = 0;
      }
      else {
        local_40 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      param_3 = local_40;
      FUN_005e1bd0(9,param_2);
    }
    else {
      FUN_005e1bd0(9,param_2);
    }
    FUN_0051c2e0();
    FUN_00648cc0(*(undefined4 *)(iVar2 + 4),0xb,param_3,0);
  }
  if ((param_3 != 0) && (param_3 != 0)) {
    FUN_005e25d0();
    FUN_009196fd(param_3);
  }
  ExceptionList = local_10;
  return;
}

