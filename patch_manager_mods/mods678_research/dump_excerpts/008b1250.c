// FN 008b1250 FUN_008b1250 size=2530 ret=12 callers=008ac330,008b1cc0,008a75f0,008b29e0,008b2f70 callees=0051c8b0,0073fb90,0051c0e0,0071ebd0,00615cc0,00733df0,00710eb0,007107f0,009196ee,007e5d80,007408f0,00416bd0,00734270,007feec0,007337c0,00416e30,00414840,00710ca0,0073f350,007356b0,00733780,00733570,00733540,00710840,00734190,00416aa0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_008b1250(int *param_1,int param_2,uint param_3,uint param_4)

{
  undefined1 uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined4 uVar5;
  undefined1 local_2ac [24];
  undefined1 local_294 [100];
  undefined1 local_230 [8];
  undefined1 local_228 [16];
  undefined1 local_218 [16];
  undefined1 local_208 [24];
  undefined1 local_1f0 [100];
  undefined1 local_18c [8];
  undefined1 local_184 [16];
  undefined1 local_174 [16];
  undefined1 local_164 [16];
  undefined1 local_154 [16];
  undefined1 local_144 [100];
  undefined1 local_e0 [16];
  undefined1 local_d0 [16];
  undefined1 local_c0 [16];
  undefined1 local_b0 [16];
  undefined1 local_a0 [16];
  undefined1 local_90 [16];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  undefined4 local_50;
  int local_4c;
  undefined1 local_48 [8];
  undefined4 local_40;
  undefined1 local_3c [8];
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  int *local_28;
  undefined1 local_24 [16];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00978c4d;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_007107f0(local_14);
  param_1[0x74] = param_2;
  param_1[0xeb] = param_1[0xeb] & 0xfffffffdU | (param_3 & 1) << 1;
  param_1[0xeb] = param_1[0xeb] & 0xfffffffbU | (param_4 & 1) << 2;
  if (param_1[0x74] == 0x7f000000) {
    FUN_00710840("UIBIT_EQP_ITM1");
    FUN_00414840(local_218,0);
    FUN_00710840("UIBIT_EQP_ITM1");
    FUN_00414840(local_228,0);
    uVar2 = FUN_007408f0(local_230,0x16b);
    local_8 = 0xc;
    FUN_00416e30(uVar2);
    local_8 = 0xffffffff;
    FUN_00733780();
    FUN_00710840("fnt_d16x16");
    uVar2 = FUN_00733540();
    local_8 = 0xd;
    uVar2 = FUN_00416aa0(uVar2,local_294,&DAT_00a7fe24,0x22,0x3f800000);
    local_8._0_1_ = 0xe;
    FUN_00416bd0(uVar2);
    local_8 = CONCAT31(local_8._1_3_,0xd);
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
    FUN_00710eb0(local_2ac,"inone");
  }
  else {
    local_2c = *(undefined4 *)(DAT_00a1b4a4 + 8);
    local_34 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    uVar2 = FUN_0051c8b0(param_2);
    local_30 = FUN_0051c0e0(uVar2);
    if (local_30 == 0) {
      FUN_00710840("UIBIT_EQP_ITM1");
      FUN_00414840(local_174,0);
      FUN_00710840("UIBIT_EQP_ITM1");
      FUN_00414840(local_184,0);
      uVar2 = FUN_007408f0(local_18c,0x16b);
      local_8 = 9;
      FUN_00416e30(uVar2);
      local_8 = 0xffffffff;
      FUN_00733780();
      FUN_00710840("fnt_d16x16");
      uVar2 = FUN_00733540();
      local_8 = 10;
      uVar2 = FUN_00416aa0(uVar2,local_1f0,&DAT_00a7fe24,0x22,0x3f800000);
      local_8._0_1_ = 0xb;
      FUN_00416bd0(uVar2);
      local_8 = CONCAT31(local_8._1_3_,10);
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      FUN_00710eb0(local_208,"inone");
    }
    else {
      FUN_00733540();
      local_8 = 0;
      uVar5 = 0;
      puVar4 = local_48;
      uVar2 = FUN_0073f350(puVar4,0);
      FUN_007356b0(uVar2,puVar4,uVar5);
      iVar3 = FUN_00734190(0x3c,0);
      if ((iVar3 != -1) || (iVar3 = FUN_00734190(0x7b,0), iVar3 != -1)) {
        FUN_0073fb90();
        local_50 = FUN_007e5d80();
        iVar3 = DAT_00a1b4b4;
        *(undefined4 *)(DAT_00a1b4b4 + 0x58) = local_50;
        uVar1 = FUN_007feec0();
        *(undefined1 *)(iVar3 + 0x3c) = uVar1;
        *(uint *)(iVar3 + 0x5c) = *(uint *)(iVar3 + 0x5c) & 0xfffffffe | 1;
        FUN_0071ebd0(local_48);
      }
      if (param_3 != 0) {
        FUN_00733570(&DAT_0099d77c);
        local_8._0_1_ = 1;
        FUN_00733570(&DAT_0099761c);
        local_8._0_1_ = 2;
        uVar2 = FUN_007408f0(local_68,0x7e5a);
        local_8._0_1_ = 3;
        FUN_00733df0(local_60,local_58);
        local_8._0_1_ = 4;
        FUN_00733df0(local_70,uVar2);
        local_8._0_1_ = 5;
        uVar2 = FUN_00733df0(local_80,local_78);
        local_8._0_1_ = 6;
        FUN_007337c0(uVar2);
        local_8._0_1_ = 5;
        FUN_00733780();
        local_8._0_1_ = 4;
        FUN_00733780();
        local_8._0_1_ = 3;
        FUN_00733780();
        local_8._0_1_ = 2;
        FUN_00733780();
        local_8._0_1_ = 1;
        FUN_00733780();
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
      }
      if (param_4 == 0) {
        param_1[0x26] = DAT_00a7fde8;
        param_1[0x27] = DAT_00a7fdec;
        param_1[0x28] = DAT_00a7fdf0;
        param_1[0x7f] = DAT_00a7fde8;
        param_1[0x80] = DAT_00a7fdec;
        param_1[0x81] = DAT_00a7fdf0;
        param_1[0x44] = DAT_00a7fdac;
        param_1[0x45] = DAT_00a7fdb0;
        param_1[0x46] = DAT_00a7fdb4;
        param_1[0x9d] = DAT_00a7fdac;
        param_1[0x9e] = DAT_00a7fdb0;
        param_1[0x9f] = DAT_00a7fdb4;
        param_1[0x25] = 0x3f800000;
        param_1[0x7e] = 0x3f800000;
      }
      else {
        param_1[0x26] = DAT_00a7fe48;
        param_1[0x27] = DAT_00a7fe4c;
        param_1[0x28] = DAT_00a7fe50;
        param_1[0x7f] = DAT_00a7fe48;
        param_1[0x80] = DAT_00a7fe4c;
        param_1[0x81] = DAT_00a7fe50;
        param_1[0x25] = _DAT_0098c014;
        param_1[0x7e] = _DAT_0098c014;
        param_1[0x44] = DAT_00a7fe48;
        param_1[0x45] = DAT_00a7fe4c;
        param_1[0x46] = DAT_00a7fe50;
        param_1[0x9d] = DAT_00a7fe48;
        param_1[0x9e] = DAT_00a7fe4c;
        param_1[0x9f] = DAT_00a7fe50;
      }
      FUN_00416e30(local_48);
      FUN_00733540();
      local_8 = CONCAT31(local_8._1_3_,7);
      local_4c = *(int *)(local_30 + 0x2cc);
      local_40 = *(undefined4 *)(local_30 + 0x298);
      if (local_4c < 2) {
        FUN_00710840("UIBIT_EQP_ITM1");
        FUN_00414840(local_d0,0);
        FUN_00710840("UIBIT_EQP_ITM1");
        FUN_00414840(local_e0,0);
      }
      else if (local_4c < 100) {
        FUN_00710840("UIBIT_EQP_ITM2");
        FUN_00414840(local_b0,0);
        FUN_00710840("UIBIT_EQP_ITM2");
        FUN_00414840(local_c0,0);
        FUN_00734270(local_3c,&DAT_00986b94,local_4c);
      }
      else {
        FUN_00710840("UIBIT_EQP_ITM3");
        FUN_00414840(local_90,0);
        FUN_00710840("UIBIT_EQP_ITM3");
        FUN_00414840(local_a0,0);
        FUN_00734270(local_3c,&DAT_00986b94,local_4c);
      }
      FUN_00710840("fnt_d16x16");
      uVar2 = FUN_00416aa0(local_3c,local_144,&DAT_00a7fe24,0x22,0x3f800000);
      local_8._0_1_ = 8;
      FUN_00416bd0(uVar2);
      local_8._0_1_ = 7;
      FUN_00733780();
      uVar2 = FUN_00615cc0(local_154);
      FUN_00710ca0(local_164,uVar2);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
    }
  }
  (**(code **)(*param_1 + 4))(param_1 + 1);
  local_28 = param_1 + 0xb6;
  param_1[0xbe] = param_1[0xbe] & 0xfffffffcU | 2;
  FUN_00414840(local_24,0);
  ExceptionList = local_10;
  __security_check_cookie(local_14 ^ (uint)&stack0xfffffffc);
  return;
}

