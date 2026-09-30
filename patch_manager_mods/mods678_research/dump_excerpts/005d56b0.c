// FN 005d56b0 FUN_005d56b0 size=6534 ret=16 callers=0062c360,005f4c00,00569320,0061e360,00623330,00617980 callees=0051c280,005dda60,005cf8a0,005d1990,005d0a80,005d9690,00733540,0058c370,005ccc50,005d17d0,005bb920,005cff50,00736220,005d29f0,005bf000,005d25e0,005d9570,00593000,005d19b0,00733780,00561430,005b5360,0054aa30,005b3380,005b96f0,005dfbe0,005bf940,005f6180,00549f70,005db0a0,005be0a0,0073fb90,0073fec0,005bf7f0,0051c0a0,005bd0b0,005d29b0,0053e740,009196fd,00590180,005b2c10,005d9aa0,005dad10,00733df0,006d9500,005b8000,005d99d0,005db8f0,005cf7b0,005b43c0,00734270,005bc1d0,005210f0,005be950,005bbf40,007337c0,005dab80,005d18c0,0073fee0,0073f470,007f7220,005e0e70,005d9bf0,0072fb00,005b5ad0,006d7390,0073f430,007338d0,005bb2c0,00734010,0054aa10,005bb470,005be1f0,005bc700,0057e3d0,005d0c00,005ceed0,005d9da0,005da4d0,005d7160,007a1720,00544a10,005bfc00,0051c370,005d17f0,005df920,007f2a90,005bca60,005bcdc0,005d7480,005bbb70,0051ad20,005bb5a0,005befc0,005cf660,005d2ea0,005d75b0,005d9890,005dec20,007a1780,005d97e0

void __thiscall FUN_005d56b0(int *param_1,int param_2,int param_3,uint param_4,int param_5)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  undefined8 uVar6;
  undefined1 *puVar7;
  undefined4 uVar8;
  undefined1 *puVar9;
  undefined4 uVar10;
  undefined1 local_70 [8];
  int local_68;
  int local_64;
  int local_60;
  int local_5c;
  int local_58;
  int *local_54;
  int local_50;
  undefined1 local_4c [4];
  undefined1 local_48 [4];
  int *local_44;
  int local_40;
  undefined1 local_3c [8];
  int local_34;
  undefined4 *local_30;
  undefined4 local_24;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953898;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_18 = 0;
  local_20 = DAT_009944bc;
  local_1c = 0;
  local_14 = (uint)*(byte *)(param_1 + 2);
  if (DAT_00a7e09c != 0) {
    FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    local_24 = 0x48;
    FUN_00734010();
    iVar1 = (**(code **)(*param_1 + 0x30))();
    if (iVar1 != 0) {
      (**(code **)(*param_1 + 0x30))();
      (**(code **)(*param_1 + 0x30))();
      (**(code **)(*param_1 + 0x30))();
      (**(code **)(*param_1 + 0x30))();
      (**(code **)(*param_1 + 0x30))();
      (**(code **)(*param_1 + 0x30))();
      (**(code **)(*param_1 + 0x30))();
      (**(code **)(*param_1 + 0x30))();
      iVar1 = (**(code **)(*param_1 + 0x30))();
      if (*(int *)(*(int *)(iVar1 + 0x10dc) + 0xaa8) != 0) {
        (**(code **)(*param_1 + 0x30))();
      }
      (**(code **)(*param_1 + 0x30))();
    }
    iVar1 = FUN_007a1780();
    if (iVar1 == 0) {
      local_34 = FUN_007a1720();
      FUN_007338d0(&DAT_00986bfc);
      while (local_34 != 0) {
        FUN_00733540();
        local_8._0_1_ = 1;
        local_30 = (undefined4 *)FUN_00561430(local_34);
        FUN_00734270(local_3c," %d(%d)",*local_30,*(undefined2 *)(local_30 + 0x1b));
        uVar2 = FUN_00733df0(local_70,local_3c);
        local_8._0_1_ = 2;
        FUN_007337c0(uVar2);
        local_8._0_1_ = 1;
        FUN_00733780();
        FUN_0058c370(&local_34);
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
      }
      FUN_00734010();
    }
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  iVar1 = FUN_007a1780();
  if (iVar1 == 0) {
    iVar1 = param_1[1];
    iVar3 = FUN_0073fb90();
    iVar3 = *(int *)(iVar3 + 0x24);
    iVar1 = FUN_0073fec0(iVar1);
    if ((iVar1 == iVar3) && (iVar1 = FUN_0072fb00(), *(char *)(iVar1 + 0x75) == '\x05')) {
      DAT_00a106dc = 1;
      FUN_0073f430();
      FUN_007f7220();
      FUN_0073fee0(0,1);
    }
  }
  do {
    if (local_18 != 0) {
      uVar5 = FUN_005210f0();
      if (0x4b < uVar5) {
        uVar10 = 0;
        uVar8 = 0x4b;
        uVar2 = FUN_005210f0(0x4b,0);
        FUN_00549f70(0xa1,uVar2,uVar8,uVar10);
        uVar5 = FUN_005210f0();
        if (500 < uVar5) {
          uVar10 = 1;
          uVar8 = 500;
          uVar2 = FUN_005210f0(500,1);
          FUN_00549f70(0xa4,uVar2,uVar8,uVar10);
          local_60 = 1;
          while ((iVar1 = FUN_007a1780(), iVar1 == 0 && (local_60 == 1))) {
            local_64 = FUN_007f2a90();
            iVar3 = (**(code **)(*param_1 + 0x74))(local_64,1);
            iVar1 = local_64;
            if (iVar3 == 1) {
              if (local_64 != 0) {
                FUN_0053e740();
                FUN_009196fd(iVar1);
              }
            }
            else {
              local_60 = 0;
              FUN_0054aa10(local_64);
            }
          }
          uVar5 = FUN_005210f0();
          if (1000 < uVar5) {
            while (iVar1 = FUN_007a1780(), iVar1 == 0) {
              iVar1 = FUN_007f2a90();
              local_68 = iVar1;
              if (iVar1 != 0) {
                FUN_0053e740();
                FUN_009196fd(iVar1);
              }
            }
          }
        }
      }
      if ((500 < param_1[0x53]) && (FUN_00549f70(0xa2,param_1[0x53],500,0), 1000 < param_1[0x53])) {
        FUN_00549f70(0xa5,param_1[0x53],1000,1);
        FUN_00544a10(DAT_009967b4);
        FUN_00544a10(DAT_009967b8);
      }
      ExceptionList = local_10;
      return;
    }
    iVar1 = FUN_007a1780();
    if (iVar1 == 0) {
      piVar4 = (int *)FUN_007f2a90();
      if ((param_1[0x30] == 0) && (param_1[0x31] == 0)) {
        param_1[0x30] = param_2;
        param_1[0x31] = param_3;
      }
      param_1[0x83] = (int)piVar4;
      local_1c = *piVar4;
      *(short *)(param_1 + 0x21) = (short)*piVar4;
      local_20 = DAT_009944bc;
      local_44 = piVar4;
      switch(local_1c) {
      case 1:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bfc00();
        }
        break;
      case 2:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bbf40(piVar4);
        }
        break;
      case 3:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bb5a0(piVar4);
        }
        break;
      case 4:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bbb70(piVar4);
        }
        break;
      case 5:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005be950(piVar4);
        }
        break;
      case 6:
        local_20 = FUN_005d9da0(piVar4);
        break;
      case 7:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005cf8a0(piVar4);
        }
        break;
      case 8:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005be0a0(piVar4);
        }
        break;
      case 9:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005cff50(piVar4);
        }
        break;
      case 10:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bc700(piVar4);
        }
        break;
      case 0xb:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d0a80(piVar4);
        }
        break;
      case 0xc:
        if (local_14 == 5) {
          param_1[0x22] = piVar4[0xe];
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_006d7390(piVar4);
          param_1[0x22] = 0x7f000000;
          if (local_20 != DAT_009944b4) {
            puVar9 = local_4c;
            puVar7 = local_48;
            FUN_0051c370(puVar7,puVar9);
            FUN_0051ad20(puVar7,puVar9);
          }
        }
        break;
      case 0xe:
        local_20 = FUN_005d9890(piVar4);
        break;
      case 0xf:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005b96f0(piVar4);
          if ((local_20 == DAT_009944bc) || (local_20 == DAT_009944b8)) {
            (**(code **)(*param_1 + 0x30))();
            FUN_00590180();
          }
        }
        else if (local_14 == 9) {
          (**(code **)(*param_1 + 0x48))(piVar4);
          local_20 = FUN_005f6180(piVar4);
        }
        break;
      case 0x10:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d0c00(piVar4);
        }
        break;
      case 0x11:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bc1d0(piVar4);
        }
        break;
      case 0x12:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bca60(piVar4);
        }
        break;
      case 0x13:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bb2c0(piVar4);
        }
        break;
      case 0x14:
        local_20 = FUN_005db0a0(piVar4);
        break;
      case 0x15:
        local_20 = FUN_005dad10(piVar4);
        break;
      case 0x16:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005cf7b0(piVar4);
        }
        break;
      case 0x17:
        local_20 = FUN_005d9bf0(piVar4);
        break;
      case 0x18:
        local_20 = FUN_005d75b0(piVar4);
        break;
      case 0x19:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005b43c0(piVar4);
        }
        break;
      case 0x1a:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005b3380(piVar4);
        }
        break;
      case 0x1b:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005b2c10(piVar4);
        }
        break;
      case 0x1c:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005b5360(piVar4);
        }
        break;
      case 0x1d:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005b5ad0(piVar4);
        }
        break;
      case 0x1e:
        local_20 = FUN_005d9aa0(piVar4);
        break;
      case 0x1f:
        local_20 = FUN_005d9570(piVar4);
        break;
      case 0x20:
        local_20 = FUN_005d9690(piVar4);
        break;
      case 0x21:
        local_20 = FUN_005d97e0(piVar4);
        break;
      case 0x22:
        local_20 = FUN_005da4d0(piVar4);
        break;
      case 0x23:
        if ((local_14 == 5) || (local_14 == 9)) {
          local_20 = FUN_005dab80(piVar4);
        }
        break;
      case 0x24:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))();
          local_20 = FUN_005bd0b0();
        }
        break;
      case 0x25:
        local_20 = FUN_005d99d0(piVar4);
        break;
      case 0x26:
        local_20 = FUN_005db8f0(piVar4);
        break;
      case 0x27:
        local_20 = FUN_005dda60(piVar4);
        break;
      case 0x28:
        local_20 = FUN_005dfbe0(piVar4);
        break;
      case 0x29:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d1990(piVar4);
        }
        break;
      case 0x2a:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d17d0(piVar4);
        }
        break;
      case 0x2b:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d1990(piVar4);
        }
        break;
      case 0x2c:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bb920(piVar4);
        }
        break;
      case 0x2d:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bf000(piVar4);
        }
        break;
      case 0x2e:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005b8000(piVar4);
          if (local_20 != DAT_009944b4) {
            iVar1 = (**(code **)(*param_1 + 0x30))();
            *(undefined4 *)(iVar1 + 0x568) = 0x7f000000;
            iVar1 = (**(code **)(*param_1 + 0x30))();
            *(undefined4 *)(iVar1 + 0x56c) = 0x7f000000;
          }
        }
        break;
      case 0x2f:
        local_20 = FUN_005e0e70(piVar4);
        break;
      case 0x30:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005be1f0(piVar4);
        }
        break;
      case 0x31:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005bb470(piVar4);
        }
        break;
      case 0x32:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d25e0(piVar4);
        }
        break;
      case 0x33:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005ccc50(piVar4);
        }
        break;
      case 0x34:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d17f0(piVar4);
        }
        break;
      case 0x35:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d18c0(piVar4);
        }
        break;
      case 0x36:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d1990(piVar4);
        }
        break;
      case 0x37:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005ceed0(piVar4);
        }
        break;
      case 0x38:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d19b0(piVar4);
        }
        break;
      case 0x3a:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005cf660(piVar4);
        }
        break;
      case 0x3c:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005befc0(piVar4);
        }
        break;
      case 0x3d:
        iVar1 = FUN_0073f470();
        if (iVar1 == 0x7f000000) {
          local_20 = DAT_009944b4;
        }
        else {
          iVar1 = (**(code **)(*param_1 + 0x30))();
          if (iVar1 != 0) {
            (**(code **)(*param_1 + 0x30))();
          }
          if (((local_14 == 5) && (iVar1 = (**(code **)(*param_1 + 0x30))(), iVar1 != 0)) &&
             (iVar1 = (**(code **)(*param_1 + 0x30))(), *(int *)(iVar1 + 0x11ac) != 0)) {
            piVar4 = local_44;
            (**(code **)(*param_1 + 0x30))(local_44);
            local_20 = FUN_005bf7f0(piVar4);
          }
        }
        break;
      case 0x3e:
        local_20 = FUN_005d7480(piVar4);
        break;
      case 0x3f:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_006d9500(piVar4);
        }
        break;
      case 0x40:
        if (((local_14 == 5) && (iVar1 = (**(code **)(*param_1 + 0x30))(), iVar1 != 0)) &&
           (iVar1 = (**(code **)(*param_1 + 0x30))(), *(int *)(iVar1 + 0x11ac) != 0)) {
          piVar4 = local_44;
          (**(code **)(*param_1 + 0x30))(local_44);
          local_20 = FUN_005bcdc0(piVar4);
        }
        break;
      case 0x41:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d29b0(piVar4);
        }
        break;
      case 0x43:
        local_20 = FUN_005dec20(piVar4);
        break;
      case 0x44:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d29f0(piVar4);
        }
        break;
      case 0x45:
        if (local_14 == 5) {
          (**(code **)(*param_1 + 0x30))(piVar4);
          local_20 = FUN_005d2ea0(piVar4);
        }
        break;
      case 0x46:
        if (((local_14 == 5) && (iVar1 = (**(code **)(*param_1 + 0x30))(), iVar1 != 0)) &&
           (iVar1 = (**(code **)(*param_1 + 0x30))(), *(int *)(iVar1 + 0x11fc) != 0)) {
          piVar4 = local_44;
          (**(code **)(*param_1 + 0x30))(local_44);
          local_20 = FUN_005bf940(piVar4);
        }
        break;
      case 0x47:
        local_20 = FUN_005df920(piVar4);
      }
      if (((DAT_00a7e09c != 0) && (iVar1 = (**(code **)(*param_1 + 0x30))(), iVar1 != 0)) &&
         (iVar1 = (**(code **)(*param_1 + 0x30))(), *(int *)(iVar1 + 0x11ac) != 0)) {
        iVar1 = (**(code **)(*param_1 + 0x30))();
        (**(code **)(*param_1 + 0x30))();
        FUN_00593000(10,iVar1 + 0x18);
        iVar1 = (**(code **)(*param_1 + 0x30))();
        local_50 = *(int *)(iVar1 + 0x550);
        if (((local_50 != 0x7f000000) &&
            (local_54 = (int *)FUN_0051c0a0(local_50), local_54 != (int *)0x0)) &&
           (iVar1 = (**(code **)(*local_54 + 0x30))(), iVar1 != 0)) {
          local_58 = (**(code **)(*local_54 + 0x30))();
          FUN_00593000(0x1e,local_58 + 0x18);
        }
      }
      param_1[0x83] = 0;
      local_40 = FUN_0051c280();
      *(uint *)(local_40 + 0x7c) = (uint)*(ushort *)(param_1 + 0x21);
      *(undefined2 *)(param_1 + 0x21) = 0xffff;
      param_1[0x2b] = param_2;
      param_1[0x2c] = param_3;
      if (local_20 == DAT_009944c0) {
        FUN_0054aa30(local_44);
      }
      else if (local_20 == DAT_009944b4) {
        FUN_0054aa10(local_44);
      }
      else {
        iVar1 = (**(code **)(*param_1 + 0xc4))();
        if ((iVar1 != 0x7f000000) && (local_1c != 0xc)) {
          (**(code **)(*param_1 + 0xc4))();
          (**(code **)(*param_1 + 0xc0))(0x7f000000);
        }
        piVar4 = local_44;
        if (local_44 != (int *)0x0) {
          FUN_0053e740();
          FUN_009196fd(piVar4);
        }
        local_44 = (int *)0x0;
        param_1[0x30] = 0;
        param_1[0x31] = 0;
      }
    }
    else {
      iVar1 = (**(code **)(*param_1 + 0x30))();
      if (iVar1 != 0) {
        local_5c = (**(code **)(*param_1 + 0x30))();
        if (*(int *)(local_5c + 0x550) != 0x7f000000) {
          FUN_0057e3d0(0x7f000000);
        }
        if (*(int *)(local_5c + 0x548) != 0x7f000000) {
          *(undefined4 *)(local_5c + 0x548) = 0x7f000000;
        }
        if (*(int *)(local_5c + 0x568) != 0x7f000000) {
          *(undefined4 *)(local_5c + 0x568) = 0x7f000000;
        }
        if (*(int *)(local_5c + 0x56c) != 0x7f000000) {
          *(undefined4 *)(local_5c + 0x56c) = 0x7f000000;
        }
        iVar1 = (**(code **)(*param_1 + 0x30))();
        *(undefined4 *)(iVar1 + 0x54c) = 0x7f000000;
        iVar1 = (**(code **)(*param_1 + 0x30))();
        iVar1 = *(int *)(iVar1 + 0x380);
        *(undefined4 *)(iVar1 + 0x254) = 0x7f000000;
        *(undefined4 *)(iVar1 + 600) = 0;
        *(undefined4 *)(iVar1 + 0x25c) = 0;
        *(undefined4 *)(iVar1 + 0x260) = 0;
      }
    }
    if (local_20 == DAT_009944b4) {
LAB_005d6d5f:
      local_18 = 1;
    }
    else {
      uVar6 = FUN_00736220();
      if ((((int)((ulonglong)uVar6 >> 0x20) - param_5 != (uint)((uint)uVar6 < param_4)) ||
          (DAT_009944c4 < (uint)uVar6 - param_4)) || (iVar1 = FUN_005d7160(local_1c), iVar1 == 1))
      goto LAB_005d6d5f;
    }
    iVar1 = FUN_007a1780();
    if (iVar1 != 0) {
      local_18 = 1;
    }
  } while( true );
}

