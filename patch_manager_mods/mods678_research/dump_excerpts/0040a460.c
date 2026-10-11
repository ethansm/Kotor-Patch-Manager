// FN 0040a460 FUN_0040a460 size=743 ret=0 callers=0040a160,0040a750,007b0e90 callees=00734270,0073f2b0,00739aa0,0072fac0,0073f2d0,00733570,0091d220,0073f350,00736240,0091cf18,0072f9f0,00733540,00736320,0073f3b0,0072fae0,0072fb00,00408ee0,0051a920,00413ba0,0073fe60,007362c0,00733780

undefined4 FUN_0040a460(short param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined8 uVar5;
  undefined1 *puVar6;
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  undefined1 local_34 [12];
  int local_28;
  int local_24 [2];
  undefined1 local_1c [8];
  short local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009480e0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_0073f2d0(uVar1);
  if ((iVar2 != 0) && (iVar2 = FUN_0073f2b0(), iVar2 != 0)) {
    FUN_00733540();
    local_8 = 0;
    FUN_0073f3b0(0x43,param_1);
    uVar5 = FUN_0051a920();
    uVar3 = __aulldiv(uVar5,1000,0);
    FUN_00734270(local_1c,"%d %c %d\n",uVar3);
    puVar6 = local_1c;
    FUN_0073f2b0(puVar6);
    FUN_00739aa0(puVar6);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  iVar2 = FUN_0073f350();
  if (iVar2 == 2) {
    local_14 = 0xb0;
  }
  else if (iVar2 == 5) {
    local_14 = 3;
  }
  else {
    local_14 = 0x60;
  }
  if (param_1 == local_14) {
    if (local_14 == 3) {
      DAT_00a7fcbc = 0;
    }
    FUN_00736240();
    local_8 = 1;
    FUN_00733540();
    local_8._0_1_ = 2;
    FUN_00733570("EnableCheats");
    local_8._0_1_ = 3;
    FUN_00733570("Game Options");
    local_8._0_1_ = 4;
    FUN_00733570(".\\swkotor2.ini");
    local_8._0_1_ = 5;
    FUN_00736320(local_24,local_44,local_3c,local_34);
    local_8._0_1_ = 4;
    FUN_00733780();
    local_8._0_1_ = 3;
    FUN_00733780();
    local_8 = CONCAT31(local_8._1_3_,2);
    FUN_00733780();
    if (local_24[0] == 0) {
      local_28 = 0;
    }
    else {
      local_28 = FUN_0091cf18(local_24[0]);
    }
    if (local_28 == 1) {
      if (DAT_00a1b4e8 == 0) {
        iVar4 = FUN_0072fb00();
        DAT_00a1b760 = (uint)(iVar4 == 0);
        FUN_0072fac0();
        FUN_0072f9f0();
        FUN_0073fe60(uVar1,iVar2,*(undefined4 *)(DAT_00a1b4a4 + 4));
        DAT_00a1b4e8 = 1;
      }
      else {
        if (DAT_00a1b760 != 0) {
          FUN_0072fae0();
        }
        DAT_00a1b4e8 = 0;
      }
    }
    local_8 = CONCAT31(local_8._1_3_,1);
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_007362c0();
    uVar3 = 1;
  }
  else {
    iVar2 = FUN_0072fb00();
    if (iVar2 != 0) {
      if (DAT_00a1b4e8 != 0) {
        FUN_00408ee0((undefined1)param_1);
        ExceptionList = local_10;
        return 1;
      }
      if ((DAT_00a1b49c != 0) && (iVar2 = FUN_00413ba0(param_1), iVar2 != 0)) {
        ExceptionList = local_10;
        return 1;
      }
    }
    uVar3 = 0;
  }
  ExceptionList = local_10;
  return uVar3;
}

