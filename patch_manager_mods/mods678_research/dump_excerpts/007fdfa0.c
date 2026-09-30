// FN 007fdfa0 FUN_007fdfa0 size=618 ret=8 callers=008301e0,0078eba0 callees=006d5200,00733570,0071df90,00919f10,007335f0,00733df0,007107f0,007337c0,00733780,00733540,009196ee

void __thiscall FUN_007fdfa0(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  size_t local_60;
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  undefined4 *local_3c;
  undefined1 local_38 [28];
  uint local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096c289;
  local_10 = ExceptionList;
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(local_1c);
  local_8 = 1;
  FUN_007107f0();
  local_8 = 2;
  FUN_00733540();
  local_8._0_1_ = 3;
  if (param_3 == 0) {
    FUN_007335f0(param_1 + 0x18c);
    local_8._0_1_ = 2;
    FUN_00733780();
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  else {
    if (*(int *)(param_1 + 0x18c) == 0) {
      local_60 = 0;
    }
    else {
      local_60 = _strlen(*(char **)(param_1 + 0x18c));
    }
    if (((int)local_60 < 1) || (*(int *)(param_1 + 0x114) == 0)) {
      local_3c = (undefined4 *)FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
      FUN_0071df90(*local_3c,local_38,DAT_0099c4a2);
      FUN_007337c0(local_38);
      if (*(int *)(param_1 + 0x114) == 0) {
        FUN_0071df90(0,local_38,DAT_0099c4a2);
        FUN_00733570(&DAT_00986bfc);
        local_8._0_1_ = 4;
        FUN_00733df0(local_4c,local_44);
        local_8._0_1_ = 5;
        uVar1 = FUN_00733df0(local_54,local_38);
        local_8._0_1_ = 6;
        FUN_007337c0(uVar1);
        local_8._0_1_ = 5;
        FUN_00733780();
        local_8._0_1_ = 4;
        FUN_00733780();
        local_8._0_1_ = 3;
        FUN_00733780();
      }
      FUN_007335f0(local_18);
      local_8._0_1_ = 2;
      FUN_00733780();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
    }
    else {
      FUN_007335f0(param_1 + 0x18c);
      local_8._0_1_ = 2;
      FUN_00733780();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
    }
  }
  ExceptionList = local_10;
  __security_check_cookie(local_1c ^ (uint)&stack0xfffffffc);
  return;
}

