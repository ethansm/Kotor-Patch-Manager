// FN 006d9500 FUN_006d9500 size=3986 ret=4 callers=005d56b0 callees=005644b0,00547bb0,006d6e30,00568540,00604260,00733570,005676d0,00540ca0,0051c0e0,00623320,0058fc60,0058fcd0,00565310,00566410,00576430,0051c0a0,0091dba0,0051c720,0051c370,009196fd,00566650,005908a0,00733780,00590110,0051ad20,0071bcf0,005648d0,00563da0,006c1450,00590220,005453c0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_006d9500(int *param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  int local_c4;
  int local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  int local_b0;
  int local_ac;
  undefined1 local_a8 [8];
  int local_a0;
  int local_9c;
  int local_98;
  int local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  int *local_70;
  int local_6c;
  int local_68;
  uint local_64;
  undefined4 local_60;
  uint local_5c;
  int local_58;
  int local_54;
  int *local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  char local_31;
  undefined2 local_30;
  undefined4 local_2c;
  int local_28;
  undefined2 local_24;
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095d76b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_1c = 0;
  local_18 = 0x7f000000;
  if (*(int *)(param_1[0x437] + 0xaa8) != 0) {
    return DAT_009944b4;
  }
  ExceptionList = &local_10;
  iVar2 = FUN_0051c720(2);
  if (iVar2 != 0) {
    ExceptionList = local_10;
    return DAT_009944b4;
  }
  if (*(int *)(param_1[0x437] + 0xb18) != 0) {
    local_98 = *(int *)(param_1[0x437] + 0xb18);
    local_94 = local_98;
    if (local_98 != 0) {
      FUN_00623320(uVar1);
      FUN_009196fd(local_94);
    }
    iVar2 = param_1[0x437];
    *(undefined4 *)(iVar2 + 0xb18) = 0;
    if (*(int *)(iVar2 + 0xb18) != 0) {
      *(undefined1 *)(iVar2 + 0xb1c) = *(undefined1 *)(*(int *)(iVar2 + 0xb18) + 0x10);
    }
  }
  iVar2 = FUN_00590110();
  if ((iVar2 == 0) || (0 < *(int *)(param_1[0x437] + 0xab8))) {
    FUN_00590110();
    if (*(int *)(param_1[0x437] + 0xa84) != 0) {
      ExceptionList = local_10;
      return DAT_009944b4;
    }
    ExceptionList = local_10;
    return DAT_009944b8;
  }
  local_14 = 0;
  _memcpy((void *)(param_2 + 0x38),&local_14,4);
  local_20 = FUN_00590220();
  if (local_20 == 0) goto switchD_006d986e_caseD_2;
  local_24 = *(undefined2 *)(local_20 + 4);
  local_18 = *(undefined4 *)(local_20 + 0x14);
  local_38 = *(undefined4 *)(local_20 + 8);
  local_2c = *(undefined4 *)(local_20 + 0xc);
  local_31 = *(char *)(local_20 + 0x10);
  local_30 = *(undefined2 *)(local_20 + 0x5c);
  local_28 = *(int *)(local_20 + 0x74);
  iVar2 = param_1[0x437];
  *(int *)(iVar2 + 0xb18) = local_20;
  if (*(int *)(iVar2 + 0xb18) != 0) {
    *(undefined1 *)(iVar2 + 0xb1c) = *(undefined1 *)(*(int *)(iVar2 + 0xb18) + 0x10);
  }
  FUN_005908a0(local_31);
  if ((((local_31 != '\n') && (local_31 != '\r')) &&
      ((local_20 == 0 || (*(int *)(local_20 + 0x24) != 0xb5)))) &&
     ((((*(int *)(local_20 + 0x24) != 200 && (*(int *)(local_20 + 0x24) != 0xb6)) &&
       (*(int *)(local_20 + 0x24) != 0xb8)) &&
      ((*(int *)(local_20 + 0x24) != 0xc9 && (*(int *)(local_20 + 0x24) != 0x10d)))))) {
    FUN_00563da0(1,0,0);
  }
  switch(local_31) {
  case '\x01':
  case '\v':
    local_3c = 0;
    local_40 = 0;
    puVar6 = &local_40;
    puVar4 = &local_3c;
    FUN_0051c370(puVar4,puVar6);
    FUN_0051ad20(puVar4,puVar6);
    if (local_28 == 0) {
      FUN_00568540(local_18,local_30,0,0,0,1,1,local_31,local_24,local_38,local_2c,0,0,4,0);
      uVar5 = DAT_009944c0;
    }
    else {
      local_48 = *(undefined4 *)(local_20 + 0x78);
      local_44 = *(undefined4 *)(local_20 + 0x7c);
      local_4c = *(undefined4 *)(local_20 + 0x80);
      FUN_00568540(local_18,0,0,0,0,1,1,local_31,local_24,local_38,local_2c,1,local_48,local_44,
                   local_4c);
      uVar5 = DAT_009944c0;
    }
    break;
  default:
    goto switchD_006d986e_caseD_2;
  case '\x03':
    FUN_0058fc60(1,param_1[1]);
    FUN_0058fcd0(local_38,0);
    goto switchD_006d986e_caseD_2;
  case '\x06':
    FUN_0058fc60(1,param_1[1]);
    FUN_0058fcd0(local_38,0);
    FUN_00566650(local_18,*(undefined4 *)(local_20 + 0x1c),*(undefined4 *)(local_20 + 0x54));
    goto switchD_006d986e_caseD_2;
  case '\a':
    FUN_0058fc60(1,param_1[1]);
    FUN_0058fcd0(local_38,0);
    FUN_005676d0(local_18,*(undefined4 *)(local_20 + 0x20),1,0,*(undefined4 *)(local_20 + 0x54),0);
switchD_006d986e_caseD_2:
    uVar5 = DAT_009944b4;
    break;
  case '\t':
    FUN_00565310(*(undefined4 *)(local_20 + 0x24),*(undefined4 *)(local_20 + 0x28),
                 *(undefined4 *)(local_20 + 0x2c),*(undefined4 *)(local_20 + 0x30),
                 *(undefined4 *)(local_20 + 0x34),*(undefined4 *)(local_20 + 0x38),
                 *(undefined4 *)(local_20 + 0x3c),*(undefined4 *)(local_20 + 0x40),
                 *(undefined4 *)(local_20 + 0x44),*(undefined4 *)(local_20 + 0x84),
                 *(undefined4 *)(local_20 + 0x48),1,*(undefined4 *)(local_20 + 0x4c),
                 *(undefined1 *)(local_20 + 0x50),*(undefined4 *)(local_20 + 0x54),
                 *(undefined4 *)(local_20 + 0x5c),*(undefined1 *)(local_20 + 0x60),1);
    local_a0 = *(int *)(param_1[0x437] + 0xb18);
    if (local_a0 != 0) {
      local_9c = local_a0;
      FUN_00623320();
      FUN_009196fd(local_9c);
    }
    iVar2 = param_1[0x437];
    *(undefined4 *)(iVar2 + 0xb18) = 0;
    uVar5 = DAT_009944c0;
    if (*(int *)(iVar2 + 0xb18) != 0) {
      *(undefined1 *)(iVar2 + 0xb1c) = *(undefined1 *)(*(int *)(iVar2 + 0xb18) + 0x10);
      uVar5 = DAT_009944c0;
    }
    break;
  case '\n':
    local_50 = (int *)FUN_0051c0a0(*(undefined4 *)(local_20 + 0x44));
    if (((local_50 != (int *)0x0) && (iVar2 = (**(code **)(*local_50 + 0x10))(), iVar2 != 0)) &&
       (iVar2 = (**(code **)(*local_50 + 0x30))(), iVar2 != 0)) {
      piVar3 = (int *)(**(code **)(*local_50 + 0x30))();
      iVar2 = (**(code **)(*piVar3 + 0x94))();
      if (iVar2 != 0) goto switchD_006d986e_caseD_2;
    }
    local_54 = FUN_0051c0e0(*(undefined4 *)(local_20 + 100));
    if ((local_54 != 0) &&
       (local_58 = FUN_00604260(*(undefined4 *)(local_20 + 0x68)), local_58 != 0)) {
      local_5c = (uint)*(ushort *)(local_58 + 2);
      local_68 = 0;
      FUN_00733570("FORCEHOSTILE");
      local_8 = 0;
      FUN_0071bcf0(local_5c,local_a8,&local_68);
      local_8 = 0xffffffff;
      FUN_00733780();
      local_60 = 0;
      local_64 = FUN_00576430(local_5c);
      if (local_64 != local_5c) {
        local_60 = 1;
        local_5c = local_64;
      }
      local_6c = FUN_006c1450(local_5c);
      if ((local_6c != 0) && (*(int *)(local_6c + 0x154) != 0)) {
        FUN_00563da0(1,0,0);
      }
      if ((((local_68 < 1) && (*(int *)(local_20 + 0x44) != param_1[1])) &&
          (*(int *)(local_6c + 0x154) != 0)) &&
         ((iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) != '1' &&
          (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) != '\x06')))) {
        *(undefined4 *)(local_20 + 0x44) = 0x7f000000;
      }
    }
    FUN_005648d0(*(undefined4 *)(local_20 + 100),*(undefined4 *)(local_20 + 0x68),
                 *(undefined4 *)(local_20 + 0x6c),*(undefined4 *)(local_20 + 0x38),
                 *(undefined4 *)(local_20 + 0x3c),*(undefined4 *)(local_20 + 0x40),
                 *(undefined4 *)(local_20 + 0x44),*(undefined4 *)(local_20 + 0x84),
                 *(undefined4 *)(local_20 + 0x48),1,1);
    local_b0 = *(int *)(param_1[0x437] + 0xb18);
    if (local_b0 != 0) {
      local_ac = local_b0;
      FUN_00623320();
      FUN_009196fd(local_ac);
    }
    iVar2 = param_1[0x437];
    *(undefined4 *)(iVar2 + 0xb18) = 0;
    uVar5 = DAT_009944c0;
    if (*(int *)(iVar2 + 0xb18) != 0) {
      *(undefined1 *)(iVar2 + 0xb1c) = *(undefined1 *)(*(int *)(iVar2 + 0xb18) + 0x10);
      uVar5 = DAT_009944c0;
    }
    break;
  case '\f':
    iVar2 = FUN_005453c0();
    local_74 = *(undefined4 *)(iVar2 + 0x124);
    local_70 = (int *)FUN_0051c0a0(*(undefined4 *)(local_20 + 0x14));
    if (((local_70 == (int *)0x0) || (iVar2 = (**(code **)(*local_70 + 0x10))(), iVar2 == 0)) ||
       (*(int *)(local_20 + 0x14) == 0x7f000000)) {
      local_88 = 0;
      local_90 = _DAT_00995174;
      local_8c = 0x7f000000;
      FUN_00540ca0(0x12,*(undefined2 *)(param_2 + 0x6c),2,local_20 + 0x38,2,local_20 + 0x3c,2,
                   local_20 + 0x40,3,&local_74,3,&local_8c,1,local_20 + 0x54,2,&local_90,1,&local_88
                   ,0,0,0,0,0,0,0,0,0,0);
      local_d0 = 0;
      local_cc = 0;
      local_c8 = 0;
      FUN_005644b0(*(undefined2 *)(param_2 + 0x6c),local_20 + 0x38,local_74,0x7f000000,
                   *(undefined4 *)(local_20 + 0x54),_DAT_00995174,_DAT_009876fc,0,0,0,0,0,0,
                   &local_d0,0,0,0);
      iVar2 = *(int *)(param_1[0x437] + 0xb18);
      if (iVar2 != 0) {
        FUN_00623320();
        FUN_009196fd(iVar2);
      }
      iVar2 = param_1[0x437];
      *(undefined4 *)(iVar2 + 0xb18) = 0;
      uVar5 = DAT_009944c0;
      if (*(int *)(iVar2 + 0xb18) != 0) {
        *(undefined1 *)(iVar2 + 0xb1c) = *(undefined1 *)(*(int *)(iVar2 + 0xb18) + 0x10);
        uVar5 = DAT_009944c0;
      }
    }
    else {
      local_78 = 0;
      local_84 = _DAT_00995174;
      local_80 = 1;
      local_7c = 0;
      FUN_00540ca0(0x11,*(undefined2 *)(param_2 + 0x6c),3,local_20 + 0x14,1,local_20 + 0x54,2,
                   &local_84,2,&local_84,1,&local_80,1,&local_7c,1,&local_78,0,0,0,0,0,0,0,0,0,0,0,0
                  );
      local_bc = 0;
      local_b8 = 0;
      local_b4 = 0;
      iVar2 = (**(code **)(*local_70 + 0x10))();
      FUN_005644b0(*(undefined2 *)(param_2 + 0x6c),iVar2 + 0x94,local_74,
                   *(undefined4 *)(local_20 + 0x14),*(undefined4 *)(local_20 + 0x54),_DAT_00995174,
                   _DAT_009876fc,0,0,0,0,0,0,&local_bc,0,0,0);
      local_c4 = *(int *)(param_1[0x437] + 0xb18);
      if (local_c4 != 0) {
        local_c0 = local_c4;
        FUN_00623320();
        FUN_009196fd(local_c0);
      }
      iVar2 = param_1[0x437];
      *(undefined4 *)(iVar2 + 0xb18) = 0;
      uVar5 = DAT_009944c0;
      if (*(int *)(iVar2 + 0xb18) != 0) {
        *(undefined1 *)(iVar2 + 0xb1c) = *(undefined1 *)(*(int *)(iVar2 + 0xb18) + 0x10);
        uVar5 = DAT_009944c0;
      }
    }
    break;
  case '\r':
    FUN_00566410(local_2c,local_18,0,0,1,1);
    uVar5 = DAT_009944c0;
    break;
  case '\x0e':
    uVar5 = 1;
    (**(code **)(*param_1 + 0x10))(1);
    FUN_00547bb0(uVar5);
    uVar5 = DAT_009944c0;
  }
  ExceptionList = local_10;
  return uVar5;
}

