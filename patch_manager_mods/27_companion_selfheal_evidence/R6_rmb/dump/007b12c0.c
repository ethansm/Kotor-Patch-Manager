// FN 007b12c0 FUN_007b12c0 size=7087 ret=8 callers=0073ff80,007b0e90,007ac470 callees=007a06b0,007f7220,00740330,0073f450,0078cee0,007b40f0,007b3810,0077d800,007d0f50,007b3830,007b3560,0077fd20,00754db0,007a0750,007ca550,00751710,00740350,007d02b0,00410aa0,007c9df0,007b34a0,0077b020,00838e10,00733570,007a14c0,00654bb0,00838ec0,0079c3b0,0051c890,0079e420,0077fe50,00726f80,0079c060,0079bf40,007f24c0,0079bf00,00410530,0075bb00,0072f990,00752450,00623320,0075ca30,007f70e0,0079b700,007f6dd0,00740240,007b33f0,00751750,00740960,0073f750,00740a20,0073fea0,00744260,004122a0,007408a0,007b3680,007e5d80,00838380,007a1780,007b3850,00740930,0079f770,00752360,00733780,0073f4d0,007ca060,007400a0,00740860,0073fb90,007b3050,007cd510,0072f9f0

uint __thiscall FUN_007b12c0(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  undefined1 *puVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int local_318;
  undefined1 local_64 [8];
  int local_5c;
  int local_58;
  int local_54;
  int *local_50;
  int local_4c;
  undefined4 local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  byte local_2e;
  byte local_2d;
  int local_2c;
  int local_28;
  int local_24;
  int *local_20;
  int local_1c;
  undefined4 local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00968498;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_14 = 0;
  if (param_2 < 0xc9) {
    if (param_2 == 200) {
      if (*(int *)(param_1 + 0x40) == 0) {
        return 0;
      }
      if (*(int *)(*(int *)(param_1 + 0x40) + 0x34) != 1) {
        return 0;
      }
      if (*(int *)(*(int *)(param_1 + 0x40) + 0x98) == 0) {
        return 0;
      }
      ExceptionList = &local_10;
      (**(code **)(**(int **)(*(int *)(param_1 + 0x40) + 0x98) + 0x3c))(0x3d,param_3);
      ExceptionList = local_10;
      return local_14;
    }
    switch(param_2) {
    case 1:
      goto switchD_007b1346_caseD_1;
    case 2:
      goto switchD_007b1346_caseD_2;
    case 3:
    case 4:
    case 0x49:
    case 0x4a:
    case 0x4b:
    case 0x4c:
      if (((param_3 != 0) && (*(int *)(param_1 + 0x2c0) == 0)) &&
         (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05')) {
        DAT_00a106dc = 1;
        ExceptionList = &local_10;
        FUN_00726f80();
        FUN_007f7220();
        FUN_007b3050(0,1);
      }
      break;
    case 5:
      goto switchD_007b1346_caseD_5;
    case 6:
      goto switchD_007b1346_caseD_6;
    case 9:
      goto switchD_007b1346_caseD_9;
    case 10:
      goto switchD_007b1346_caseD_a;
    case 0xb:
      goto switchD_007b1346_caseD_b;
    case 0xf:
    case 0x48:
      if ((param_3 != 0) && (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05')) {
        DAT_00a106dc = 1;
        ExceptionList = &local_10;
        FUN_00726f80();
        FUN_007f7220();
        FUN_007b3050(0,1);
      }
      break;
    case 0x16:
    case 0x19:
      if ((((param_3 != 0) && (*(int *)(param_1 + 0x18) != 0)) &&
          (*(int *)(*(int *)(param_1 + 0x18) + 0x48) != 0)) &&
         (((ExceptionList = &local_10, iVar2 = FUN_0079bf00(), iVar2 == 0 &&
           (local_40 = *(int *)(*(int *)(*(int *)(param_1 + 0x18) + 0x48) + 0x268), local_40 != 0))
          && (*(int *)(local_40 + 0x24) != 0)))) {
        if (*(int *)(local_40 + 0x80) == 2) {
          FUN_00838380();
        }
        else {
          FUN_00838e10(0);
        }
      }
      break;
    case 0x1a:
      goto switchD_007b1346_caseD_1a;
    case 0x41:
      ExceptionList = &local_10;
      uVar1 = FUN_007b33f0(param_3);
      local_14 = uVar1 | local_14;
      break;
    case 0x42:
      ExceptionList = &local_10;
      uVar1 = FUN_007b34a0(param_3);
      local_14 = uVar1 | local_14;
      break;
    case 0x43:
      if (param_3 == 0) {
        ExceptionList = &local_10;
        FUN_007b3680();
      }
      else {
        ExceptionList = &local_10;
        FUN_007b3560(param_3 == -1);
      }
      break;
    case 0x44:
      if (param_3 == 0) {
        ExceptionList = &local_10;
        FUN_007b3830();
      }
      else {
        ExceptionList = &local_10;
        FUN_007b3810();
      }
      break;
    case 0x45:
      if (param_3 == 0) {
        ExceptionList = &local_10;
        FUN_00623320();
      }
      else {
        ExceptionList = &local_10;
        FUN_00623320();
      }
      break;
    case 0x46:
      ExceptionList = &local_10;
      FUN_007b3850(param_3);
      break;
    case 0x50:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        FUN_007a14c0();
      }
      break;
    case 0xb5:
      if ((param_3 != 0) && (*(int *)(param_1 + 0x9c) == 2)) {
        ExceptionList = &local_10;
        FUN_00410aa0(param_2,param_3);
      }
    }
  }
  else {
    switch(param_2) {
    case 0xc9:
      if (((*(int *)(param_1 + 0x40) != 0) && (*(int *)(*(int *)(param_1 + 0x40) + 0x34) == 1)) &&
         (*(int *)(*(int *)(param_1 + 0x40) + 0x98) != 0)) {
        ExceptionList = &local_10;
        (**(code **)(**(int **)(*(int *)(param_1 + 0x40) + 0x98) + 0x3c))(0x3e,param_3);
      }
      break;
    case 0xcc:
switchD_007b1346_caseD_6:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        iVar2 = FUN_0073f750(uVar1);
        if (*(int *)(iVar2 + 0x98) != 0) {
          iVar2 = FUN_0073f750();
          *(undefined4 *)(*(int *)(iVar2 + 0x98) + 0x15230) = 0;
        }
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
          DAT_00a106dc = 1;
          FUN_00726f80();
          FUN_007f7220();
          FUN_007b3050(0,1);
        }
        else {
          local_2e = -(*(int *)(param_1 + 800) != 0) & 2;
          FUN_0079b700(0,local_2e);
          if ((*(int *)(param_1 + 800) != 0) &&
             ((*(uint *)(*(int *)(param_1 + 4) + 0x1c) >> 0x10 & 1) != 0)) {
            FUN_0079e420(1,8);
          }
        }
      }
      break;
    case 0xcd:
switchD_007b1346_caseD_5:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        local_28 = FUN_0073f750(uVar1);
        if (local_28 != 0) {
          iVar2 = FUN_0073f750();
          local_2c = *(int *)(iVar2 + 0x98);
          if (local_2c != 0) {
            *(undefined4 *)(local_2c + 0x15230) = 0;
          }
        }
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
          DAT_00a106dc = 1;
          FUN_00726f80();
          FUN_007f7220();
          FUN_007b3050(0,1);
        }
        else {
          local_2d = -(*(int *)(param_1 + 800) != 0) & 2;
          FUN_0079b700(1,local_2d);
          if ((*(int *)(param_1 + 800) != 0) &&
             ((*(uint *)(*(int *)(param_1 + 4) + 0x1c) >> 0x10 & 1) != 0)) {
            FUN_0079e420(1,8);
          }
        }
      }
      break;
    case 0xce:
switchD_007b1346_caseD_9:
      if (param_3 != 0) {
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
          DAT_00a106dc = 1;
          ExceptionList = &local_10;
          FUN_00726f80();
          FUN_007f7220();
          FUN_007b3050(0,1);
        }
        else {
          ExceptionList = &local_10;
          FUN_0079f770(0,1,1);
          FUN_0073f750();
          FUN_007d02b0();
        }
        uVar4 = 6;
        FUN_0073fea0(6);
        FUN_004122a0(uVar4);
      }
      break;
    case 0xcf:
switchD_007b1346_caseD_a:
      if (param_3 != 0) {
        local_18 = 0;
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
          DAT_00a106dc = 1;
          ExceptionList = &local_10;
          FUN_00726f80();
          FUN_007f7220();
          FUN_007b3050(0,1);
        }
        else if ((*(int *)(param_1 + 0x40) != 0) && (*(int *)(*(int *)(param_1 + 0x40) + 0x30) == 0)
                ) {
          ExceptionList = &local_10;
          FUN_007cd510(0);
        }
      }
      break;
    case 0xd0:
switchD_007b1346_caseD_1:
      ExceptionList = &local_10;
      iVar2 = FUN_0078cee0();
      if ((((iVar2 != 0) && (iVar2 = FUN_0078cee0(), *(int *)(iVar2 + 0x138) == 0)) &&
          (param_3 != 0)) && (*(int *)(param_1 + 0x2c0) == 0)) {
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) != '\x05') {
          FUN_0078cee0();
          iVar2 = FUN_0077d800();
          if (iVar2 != 0) {
            FUN_0078cee0();
            FUN_0077d800();
            iVar2 = FUN_007a1780();
            if ((iVar2 != 0) && (iVar2 = FUN_00740330(), iVar2 == 0)) {
              DAT_00a106dc = 0;
              if (DAT_00a7fcb4 != 1) {
                iVar2 = FUN_007e5d80();
                uVar4 = *(undefined4 *)(iVar2 + 4);
                FUN_00726f80(uVar4);
                FUN_007f6dd0(uVar4);
                FUN_007b3050(4,1);
                ExceptionList = local_10;
                return local_14;
              }
              FUN_00726f80();
              FUN_007f70e0();
              DAT_00a7fcb4 = DAT_00a7fcb4 + 1;
              ExceptionList = local_10;
              return local_14;
            }
          }
        }
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
          DAT_00a106dc = 1;
          FUN_00726f80();
          FUN_007f7220();
          FUN_007b3050(0,1);
        }
      }
      break;
    case 0xd1:
    case 0xd2:
    case 0xd3:
    case 0xd4:
    case 0xd5:
    case 0xd6:
    case 0xd7:
    case 0xd8:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
          DAT_00a106dc = 1;
          FUN_00726f80();
          FUN_007f7220();
          FUN_007b3050(0,1);
        }
        iVar2 = FUN_0079c060();
        if (iVar2 == 0) {
          if ((*(int *)(*(int *)(param_1 + 0x40) + 0xcc) == 0) &&
             (*(int *)(*(int *)(param_1 + 0x274) + 0x98) == 0)) {
            local_34 = param_2 + -0xd1;
            if (*(int *)(*(int *)(param_1 + 0x40) + 0x34) == 3) {
              if (*(int *)(*(int *)(param_1 + 0x40) + 0x2c) == local_34) {
                iVar2 = FUN_007ca060(0);
                if (iVar2 != 0) {
                  FUN_007b3050(0,1);
                }
              }
              else {
                FUN_007ca550(local_34);
              }
            }
            else {
              iVar2 = FUN_007c9df0(local_34);
              if (iVar2 != 0) {
                FUN_007b3050(2,1);
              }
            }
          }
        }
        else if (*(int *)(param_1 + 0x2c0) != 0) {
          FUN_0079c3b0();
        }
      }
      break;
    case 0xd9:
switchD_007b1346_caseD_1a:
      if (((((param_3 != 0) && (*(int *)(param_1 + 0x18) != 0)) &&
           (*(int *)(*(int *)(param_1 + 0x18) + 0x48) != 0)) &&
          ((ExceptionList = &local_10, iVar2 = FUN_0079bf00(), iVar2 == 0 &&
           (local_3c = *(int *)(*(int *)(*(int *)(param_1 + 0x18) + 0x48) + 0x268), local_3c != 0)))
          ) && (*(int *)(local_3c + 0x24) != 0)) {
        if (*(int *)(local_3c + 0x80) == 2) {
          FUN_00838380();
        }
        else {
          FUN_00838ec0(0);
        }
      }
      break;
    case 0xda:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        FUN_0072f9f0();
        FUN_00740240();
        FUN_007a06b0();
        FUN_0072f990(0x107,1000);
      }
      break;
    case 0xdb:
    case 0xdc:
      DAT_00a7fcbc = param_3;
      break;
    case 0xdd:
    case 0xde:
      DAT_00a7fcc4 = param_3;
      break;
    case 0xdf:
switchD_007b1346_caseD_b:
      if (param_3 != 0) {
        local_1c = 0;
        ExceptionList = &local_10;
        FUN_00410aa0(param_2,param_3);
        if (DAT_00a7fcc8 == '\0') {
          if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
            DAT_00a106dc = 1;
            FUN_00726f80();
            FUN_007f7220();
            FUN_007b3050(0,1);
            local_1c = 1;
          }
          iVar2 = FUN_0079c060();
          if (iVar2 == 0) {
            if ((*(int *)(*(int *)(param_1 + 0x40) + 0xcc) == 0) &&
               (iVar2 = FUN_0073fb90(), iVar2 != 0)) {
              FUN_0073fb90();
              iVar2 = FUN_007e5d80();
              if (iVar2 != 0) {
                if (*(int *)(*(int *)(param_1 + 0x40) + 0x34) == 3) {
                  iVar2 = FUN_007ca060(0);
                  if (iVar2 != 0) {
                    FUN_007b3050(0,1);
                    local_1c = 1;
                  }
                }
                else if ((*(int *)(param_1 + 0x9c) == 0) && (iVar2 = FUN_007c9df0(7), iVar2 != 0)) {
                  FUN_007b3050(2,1);
                  FUN_00740a20(7,0x7f000000,0x7f000000,0);
                  local_1c = 1;
                }
              }
            }
          }
          else if (*(int *)(param_1 + 0x2c0) == 0) {
            local_20 = *(int **)(*(int *)(param_1 + 0x40) + 0xa0);
            FUN_0075bb00(0);
            FUN_0075ca30(0,0,0);
            (**(code **)(*local_20 + 0x70))(0xa684);
            FUN_00410530(local_20,1,1);
            local_1c = 1;
          }
          else {
            FUN_0079c3b0();
            local_1c = 1;
          }
          if (local_1c == 0) {
            FUN_00410aa0(0xb4,1);
          }
        }
        else {
          DAT_00a7fcc8 = '\0';
        }
      }
      break;
    case 0xe0:
    case 0xfd:
switchD_007b1346_caseD_2:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
          DAT_00a106dc = 1;
          FUN_00726f80();
          FUN_007f7220();
          FUN_007b3050(0,1);
        }
        local_24 = FUN_0079bf00();
        FUN_0079bf40(local_24 == 0,4,0);
        iVar2 = FUN_00740960();
        if (iVar2 != 0) {
          FUN_00740930(0,0);
        }
        if ((local_24 != 0) || (iVar2 = FUN_00740a20(6,0x7f000000,0x7f000000,0), iVar2 == 0)) {
          uVar4 = 6;
          FUN_0073fea0(6);
          FUN_004122a0(uVar4);
        }
      }
      break;
    case 0xe1:
      if (param_3 == 0) {
        ExceptionList = &local_10;
        FUN_007a0750(0);
      }
      else {
        ExceptionList = &local_10;
        FUN_007a0750(1);
      }
      break;
    case 0xe2:
      if (param_3 != 0) {
        if (DAT_00a7fcc4 == 0) {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00751710(0,1);
        }
        else {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00744260(0);
        }
      }
      break;
    case 0xe4:
      if (param_3 != 0) {
        if (DAT_00a7fcc4 == 0) {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00751710(1,1);
        }
        else {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00744260(1);
        }
      }
      break;
    case 0xe6:
      if (param_3 != 0) {
        if (DAT_00a7fcc4 == 0) {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00751710(2,1);
        }
        else {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00744260(2);
        }
      }
      break;
    case 0xe8:
      if (param_3 != 0) {
        if (DAT_00a7fcc4 == 0) {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00751750(0,1);
        }
        else {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00752450(0);
        }
      }
      break;
    case 0xea:
      if (param_3 != 0) {
        if (DAT_00a7fcc4 == 0) {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00751750(1,1);
        }
        else {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00752450(1);
        }
      }
      break;
    case 0xec:
      if (param_3 != 0) {
        if (DAT_00a7fcc4 == 0) {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00751750(3,1);
        }
        else {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00752450(3);
        }
      }
      break;
    case 0xee:
      if (param_3 != 0) {
        if (DAT_00a7fcc4 == 0) {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00751750(2,1);
        }
        else {
          ExceptionList = &local_10;
          FUN_0073f750(uVar1);
          FUN_00752450(2);
        }
      }
      break;
    case 0xef:
      if ((param_3 != 0) && (*(int *)(param_1 + 0x2b4) != 0x7f000000)) {
        ExceptionList = &local_10;
        FUN_007b40f0();
        local_44 = 0;
        if (0 < *(int *)(param_1 + 0x4d0)) {
          local_44 = *(int *)(param_1 + 0x4cc);
        }
        if (local_44 != 0) {
          FUN_0073fb90();
          local_48 = FUN_007e5d80();
          local_50 = (int *)FUN_0073f4d0(*(undefined4 *)(local_44 + 0x1c));
          if (local_50 == (int *)0x0) {
            local_318 = 0;
          }
          else {
            local_318 = (**(code **)(*local_50 + 0xc))();
          }
          local_4c = local_318;
          if (local_318 != 0) {
            (**(code **)(local_44 + 0xc))(*(undefined4 *)(local_44 + 8),local_48);
          }
        }
      }
      break;
    case 0xf0:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        local_58 = FUN_007400a0();
        local_54 = 0xffffffff;
        if ((local_58 == 0) || (iVar2 = FUN_00740330(), iVar2 == 0)) {
          iVar2 = FUN_00740860();
          if (iVar2 != 0) {
            FUN_0073fb90();
            local_5c = FUN_007e5d80();
            if (local_5c != 0) {
              FUN_0077fd20(0);
              FUN_007f24c0();
            }
          }
        }
        else {
          local_54 = *(int *)(local_58 + 0x80);
          if (local_54 == 1) {
            FUN_00733570("GBL_QUIT_SWOOP");
            local_8 = 0;
            uVar4 = 1;
            puVar3 = local_64;
            FUN_0051c890(puVar3,1);
            FUN_00654bb0(puVar3,uVar4);
            local_8 = 0xffffffff;
            FUN_00733780();
            FUN_00740350(0,4,0);
            FUN_00740930(0,0);
          }
        }
      }
      break;
    case 0xf1:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        if (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05') {
          DAT_00a106dc = 1;
          FUN_00726f80();
          FUN_007f7220();
          FUN_007b3050(0,1);
        }
        local_38 = FUN_0079bf00();
        FUN_0079bf40(local_38 == 0,4,0);
        iVar2 = FUN_00740960();
        if (iVar2 != 0) {
          FUN_00740930(0,0);
        }
        if ((local_38 != 0) || (iVar2 = FUN_00740a20(6,0x7f000000,0x7f000000,0), iVar2 == 0)) {
          uVar4 = 6;
          FUN_0073fea0(6);
          FUN_004122a0(uVar4);
        }
      }
      break;
    case 0xf2:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        FUN_007408a0();
      }
      break;
    case 0xf5:
      if (((param_3 != 0) && (*(int *)(param_1 + 0x40) != 0)) &&
         (*(int *)(*(int *)(param_1 + 0x40) + 0x98) != 0)) {
        ExceptionList = &local_10;
        FUN_00752360();
      }
      break;
    case 0xf6:
    case 0xf7:
    case 0xf8:
    case 0xf9:
      break;
    case 0xfa:
      if (((param_3 != 0) && (*(int *)(param_1 + 0x40) != 0)) &&
         (*(int *)(*(int *)(param_1 + 0x40) + 0x98) != 0)) {
        ExceptionList = &local_10;
        FUN_00623320();
      }
      break;
    case 0xfb:
      if (((param_3 != 0) && (*(int *)(param_1 + 0x40) != 0)) &&
         (*(int *)(*(int *)(param_1 + 0x40) + 0x98) != 0)) {
        ExceptionList = &local_10;
        FUN_00752360();
      }
      break;
    case 0xfc:
      break;
    case 0x107:
      if ((param_3 != 0) && (**(int **)(DAT_00a1b4a4 + 0x14) == 0)) {
        ExceptionList = &local_10;
        FUN_0072f9f0();
        DAT_00a7fcc0 = 0;
        DAT_00a7fcbc = 0;
        FUN_00740240();
        FUN_0073f750();
        FUN_007d0f50();
      }
      break;
    case 0x108:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        FUN_0073f450();
        iVar2 = FUN_0077fe50();
        if (iVar2 != 0) {
          uVar5 = 0;
          uVar4 = 0;
          FUN_007e5d80(0,0);
          FUN_0077b020(uVar4,uVar5);
        }
      }
      break;
    case 0x109:
      if (param_3 != 0) {
        ExceptionList = &local_10;
        FUN_0073f750(uVar1);
        FUN_00754db0(0);
      }
    }
  }
  ExceptionList = local_10;
  return local_14;
}

