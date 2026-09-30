// FN 008cd8e0 FUN_008cd8e0 size=522 ret=0 callers=008cb290,008ceb00,008cbde0,008cfa60 callees=00607790,00733780,006d5200,00710eb0,007107f0,009196ee,008ccd70,00733540,00414840,00710ca0,008cb230,00615cc0

void __fastcall FUN_008cd8e0(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_78 [8];
  undefined1 local_70 [16];
  undefined1 local_60 [16];
  undefined1 local_50 [16];
  undefined4 local_40;
  int local_3c;
  int local_38;
  undefined1 local_34 [16];
  uint local_24;
  int local_20;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097b6a0;
  local_10 = ExceptionList;
  local_24 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_007107f0(local_24);
  FUN_00733540();
  local_8 = 0;
  switch(*(undefined1 *)(param_1 + 0x3d2c)) {
  case 1:
    local_38 = param_1 + 0x17f8;
    break;
  case 3:
    local_38 = param_1 + 0x1fa8;
    break;
  case 4:
  case 2:
    local_38 = param_1 + 0x1fa8;
  }
  local_20 = 0;
  uVar1 = FUN_008cb230();
  local_3c = (uVar1 & 0xff) + local_20;
  for (local_1c = local_20; local_1c < local_3c; local_1c = local_1c + 1) {
    if (*(int *)(param_1 + 0x3d54 + local_1c * 4) == 0) {
      FUN_00710eb0(local_70,(&PTR_s_i_powerc_009a84ec)
                            [(*(byte *)(param_1 + 0x3d2c) - 1) * 0x12 + local_1c * 3]);
    }
    else {
      local_40 = FUN_006d5200(*(undefined4 *)(*(int *)(param_1 + 0x3d54 + local_1c * 4) + 0xc));
      uVar2 = FUN_00615cc0(local_50);
      FUN_00710ca0(local_60,uVar2);
      *(undefined4 *)(local_38 + 0x84 + local_1c * 0x148) = 0x3f800000;
    }
    FUN_00414840(local_34,0);
  }
  uVar2 = FUN_00607790(local_78);
  local_8._0_1_ = 1;
  FUN_008ccd70(uVar2);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  __security_check_cookie(local_24 ^ (uint)&stack0xfffffffc);
  return;
}

