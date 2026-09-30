// FN 005fe4b0 FUN_005fe4b0 size=2012 ret=4 callers=00694400 callees=00561f30,00734010,0058a330,005fdca0,00733570,007e6c80,005f7f30,00710840,005faaf0,0073fb90,00733540,00632830,0051c100,00543f10,0071a3d0,00521060,0058a2c0,0051c2a0,00716e60,007f18c0,005fd9a0,007f1650,0071a5a0,00600380,005fe410,0051c8f0,00733780,00568bd0,005fdd30,00574500,0058ae90,00919723,005453c0,007338d0,00606100,00572a70,005fa960,005f8b00

undefined4 __thiscall FUN_005fe4b0(int *param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10c;
  undefined4 *local_104;
  undefined1 local_c8 [11];
  char local_bd;
  undefined1 local_bc [8];
  undefined1 local_b4 [8];
  int local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 *local_9c;
  undefined4 *local_98;
  undefined1 local_94 [16];
  int local_84;
  undefined4 *local_80;
  undefined4 local_7c;
  int local_78;
  int local_74;
  int local_70;
  uint local_6c;
  int local_68;
  int local_64;
  int local_60;
  undefined4 local_5c;
  undefined1 local_58 [4];
  int local_54;
  undefined4 *local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  int local_20;
  undefined4 *local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954c72;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  local_34 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  local_14 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  FUN_0051c2a0();
  local_30 = FUN_00521060();
  local_48 = (undefined4 *)FUN_0051c100(*(undefined4 *)(local_30 + 0x38));
  local_1c = (undefined4 *)0x0;
  local_38 = FUN_005453c0();
  local_18 = 1;
  local_2c = 0;
  local_28 = 0;
  local_24 = 0;
  local_44 = 0;
  local_40 = 0;
  local_3c = 0;
  local_20 = param_1[0x46];
  if (param_2 == -1) {
    if (param_1[0x46] == -1) {
      local_7c = 1;
      local_8 = 0xffffffff;
      FUN_00733780();
      ExceptionList = local_10;
      return local_7c;
    }
    local_54 = FUN_0051c100(param_1[param_1[0x46] + 7]);
    *(undefined4 *)(*(int *)(local_54 + 0x1198) + 0x6c) = 0;
    *(undefined4 *)(local_54 + 0x10e8) = 0;
    FUN_005f7f30(param_1[0x46],0);
    local_84 = FUN_00919723(0x1220);
    local_8._0_1_ = 1;
    if (local_84 == 0) {
      local_104 = (undefined4 *)0x0;
    }
    else {
      local_104 = (undefined4 *)FUN_00561f30(0x7f000000,0);
    }
    local_80 = local_104;
    local_8 = (uint)local_8._1_3_ << 8;
    local_1c = local_104;
    FUN_005fdca0();
    FUN_00710840(&DAT_009972a8);
    iVar1 = FUN_00572a70(local_94,0);
    if (iVar1 == 0) {
      local_9c = local_1c;
      local_98 = local_1c;
      if (local_1c != (undefined4 *)0x0) {
        (**(code **)*local_1c)(1);
      }
      local_a0 = 0;
      local_8 = 0xffffffff;
      FUN_00733780();
      ExceptionList = local_10;
      return local_a0;
    }
    FUN_005fdd30();
    local_2c = local_48[0x25];
    local_28 = local_48[0x26];
    local_24 = local_48[0x27];
    local_44 = local_48[0x28];
    local_40 = local_48[0x29];
    local_3c = local_48[0x2a];
    FUN_005fd9a0(param_1[0x46],1);
    local_20 = -1;
  }
  else {
    iVar1 = FUN_005fa960(param_2);
    if (iVar1 != 0) {
      if (param_1[0x46] == param_2) {
        local_a4 = 1;
        local_8 = 0xffffffff;
        FUN_00733780();
        ExceptionList = local_10;
        return local_a4;
      }
      if (param_1[0x46] == -1) {
        FUN_007338d0("GAMEINPROGRESS:PC");
        local_ac = FUN_00919723(0xa0);
        local_8._0_1_ = 2;
        if (local_ac == 0) {
          local_10c = 0;
        }
        else {
          local_10c = FUN_00716e60();
        }
        local_a8 = local_10c;
        local_8._0_1_ = 0;
        local_5c = local_10c;
        FUN_00733570(&DAT_009938e4);
        local_8._0_1_ = 3;
        FUN_00733570(&DAT_00995250);
        local_8._0_1_ = 4;
        FUN_0071a3d0(local_58,local_bc,local_b4);
        local_8._0_1_ = 3;
        FUN_00733780();
        local_8._0_1_ = 0;
        FUN_00733780();
        FUN_00574500(local_5c,local_58);
        uVar2 = FUN_00734010();
        FUN_00733570(uVar2);
        local_8._0_1_ = 5;
        iVar1 = FUN_0071a5a0(local_c8,0x7eb);
        local_bd = '\x01' - (iVar1 != 0);
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        if (local_bd != '\0') {
          local_8 = 0xffffffff;
          FUN_00733780();
          ExceptionList = local_10;
          return 0;
        }
      }
      else {
        uVar2 = FUN_0051c8f0();
        local_60 = FUN_0051c100(uVar2);
        *(undefined4 *)(*(int *)(local_60 + 0x1198) + 0x6c) = 0;
        *(undefined4 *)(local_60 + 0x10e8) = 0;
        FUN_005f7f30(param_1[0x46],0);
      }
      uVar2 = FUN_005faaf0(param_2,0,1);
      local_1c = (undefined4 *)FUN_0051c100(uVar2);
      if (local_1c == (undefined4 *)0x0) {
        uVar2 = FUN_005faaf0(param_2,1,1);
        local_1c = (undefined4 *)FUN_0051c100(uVar2);
        local_2c = local_48[0x25];
        local_28 = local_48[0x26];
        local_24 = local_48[0x27];
        local_44 = local_48[0x28];
        local_40 = local_48[0x29];
        local_3c = local_48[0x2a];
      }
      else {
        local_18 = 0;
      }
      if (param_1[0x46] == -1) {
        local_64 = FUN_0058ae90();
        if (local_64 != 0) {
          FUN_007f18c0(0);
          FUN_007f1650(0,1,0,0);
          *(undefined4 *)(local_64 + 0xe4) = 1;
        }
        if (local_48 != (undefined4 *)0x0) {
          (**(code **)*local_48)(1);
        }
      }
      else {
        FUN_005fd9a0(param_1[0x46],1);
      }
      local_20 = param_2;
    }
  }
  if (local_1c == (undefined4 *)0x0) {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 0;
  }
  else {
    FUN_0073fb90();
    FUN_007e6c80();
    FUN_0058a330(1,1);
    *(undefined4 *)(local_1c[0x466] + 0x6c) = 1;
    local_1c[0x43a] = 1;
    FUN_005fe410(0x7f000000);
    local_68 = FUN_0058a2c0(1);
    if (local_68 != 0) {
      for (local_6c = 0; local_6c < *(uint *)(local_68 + 0x10); local_6c = local_6c + 1) {
        local_70 = FUN_00606100(local_6c);
        if (local_70 != 0) {
          FUN_00600380(local_1c[1],0,0,0x7f000000);
        }
      }
    }
    if (local_18 != 0) {
      FUN_00568bd0(local_38,local_2c,local_28,local_24,0,0);
      FUN_00543f10(&local_44);
    }
    *(undefined4 *)(local_30 + 0x38) = local_1c[1];
    FUN_00632830(local_1c);
    iVar1 = *param_1;
    while (local_74 = iVar1 + -1, -1 < local_74) {
      local_78 = param_1[iVar1 + 1];
      FUN_005f8b00(local_78);
      FUN_005fd9a0(local_78,1);
      iVar1 = local_74;
    }
    param_1[0x46] = local_20;
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 1;
  }
  ExceptionList = local_10;
  return uVar2;
}

