// FN 005b8000 FUN_005b8000 size=5794 ret=4 callers=005d56b0 callees=006070a0,006d6e30,00603b50,007ed910,00604260,00733570,00584da0,0073fb90,0051c0e0,004452b0,00733540,005482a0,0073fec0,0058e6d0,0071b1d0,00563d20,0058fc60,0058fcd0,007e6be0,00589bf0,0057a560,00576430,0051c0a0,0058fd50,00548b10,0091dba0,00590ff0,00589c30,0051c370,0040e770,0051af60,00734f60,0058e730,0072fb00,006ad2f0,00591a70,006d0620,00590180,00733780,005912b0,007e5120,0058ae90,0051ad20,0051c830,006c1450,0056d6a0,0051c350

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005b8000(int *param_1,int param_2)

{
  bool bVar1;
  short sVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  uint uVar6;
  float10 fVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 *puVar11;
  undefined4 *puVar12;
  undefined1 *puVar13;
  uint *puVar14;
  int local_280;
  undefined1 local_dc [8];
  int local_d4;
  int local_d0;
  int local_cc;
  undefined4 local_c8;
  undefined2 local_c4 [2];
  undefined4 local_c0;
  undefined4 local_bc;
  int local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  int local_a8;
  int local_a4;
  int local_a0;
  undefined4 local_9c;
  int local_98;
  undefined4 local_94;
  undefined4 local_90;
  int *local_8c;
  int *local_88;
  int local_84;
  uint local_80;
  undefined1 local_7c [4];
  uint local_78;
  uint local_74;
  int local_70;
  undefined1 local_6c [8];
  int local_64;
  uint local_60;
  int local_5c;
  int local_58;
  undefined4 local_54;
  int local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined1 local_44 [4];
  int local_40;
  int local_3c;
  undefined2 local_38 [2];
  uint local_34;
  int local_30;
  uint local_2c;
  undefined2 local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953403;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_20 = 0;
  local_1c = 0;
  local_18 = 0;
  _memcpy(&local_58,(void *)(param_2 + 0x38),4);
  _memcpy(&local_74,(void *)(param_2 + 0x3c),4);
  _memcpy(local_7c,(void *)(param_2 + 0x40),4);
  _memcpy(&local_3c,(void *)(param_2 + 0x44),4);
  _memcpy(&local_50,(void *)(param_2 + 0x48),4);
  _memcpy(&local_5c,(void *)(param_2 + 0x4c),4);
  _memcpy(&local_70,(void *)(param_2 + 0x50),4);
  if (*(ushort *)(param_2 + 0x6e) < 9) {
    puVar12 = &local_90;
    puVar11 = &local_4c;
    FUN_0051c350(param_1[1]);
    FUN_0051ad20(puVar11,puVar12);
    _memcpy((void *)(param_2 + 0x54),&local_4c,4);
    _memcpy((void *)(param_2 + 0x58),&local_90,4);
    *(undefined2 *)(param_2 + 0x6e) = 9;
  }
  else {
    _memcpy(&local_4c,(void *)(param_2 + 0x54),4);
    _memcpy(&local_90,(void *)(param_2 + 0x58),4);
  }
  local_34 = 0;
  local_30 = 0;
  local_38[0] = 0;
  if (9 < *(ushort *)(param_2 + 0x6e)) {
    _memcpy(&local_30,(void *)(param_2 + 0x5c),4);
    _memcpy(local_38,(void *)(param_2 + 0x60),2);
    _memcpy(&local_34,(void *)(param_2 + 100),4);
  }
  if (param_1[0x46b] != 0) {
    local_94 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    uVar3 = FUN_0073fec0(param_1[1]);
    local_9c = uVar3;
    FUN_0073fb90(uVar3);
    local_a0 = FUN_007e6be0(uVar3);
    if (-1 < local_a0) {
      iVar4 = FUN_0073fb90();
      local_98 = iVar4 + 0x24 + local_a0 * 0x88;
      FUN_007e5120(0x1b2c7);
    }
  }
  param_1[0x15c] = 3000;
  iVar4 = (**(code **)(*param_1 + 0x94))();
  if (iVar4 != 1) {
    if (param_1[0x46b] == 0) {
      bVar1 = false;
    }
    else {
      sVar2 = (**(code **)(*param_1 + 0x9c))(0);
      bVar1 = sVar2 < 1;
    }
    if (!bVar1) {
      puVar11 = &local_48;
      puVar12 = &local_54;
      FUN_0051c350(param_1[1]);
      FUN_0051ad20(puVar12,puVar11);
      puVar14 = &local_60;
      puVar13 = local_44;
      uVar3 = local_54;
      uVar8 = local_48;
      uVar9 = local_4c;
      uVar10 = local_90;
      FUN_0051c370(local_54,local_48,local_4c,local_90,puVar13,puVar14);
      FUN_0051af60(uVar3,uVar8,uVar9,uVar10,puVar13,puVar14);
      if (local_30 != 0) {
        if (local_60 < local_34) {
          (**(code **)(*param_1 + 0x7c))(local_38[0]);
          ExceptionList = local_10;
          return DAT_009944b4;
        }
        uVar3 = FUN_0056d6a0();
        (**(code **)(*param_1 + 0x7c))(uVar3);
        param_1[0x41d] = 0;
        FUN_00589bf0();
        FUN_00589c30();
        ExceptionList = local_10;
        return DAT_009944b8;
      }
      local_8c = (int *)FUN_0051c0a0(local_3c);
      if ((local_3c != 0x7f000000) && (local_8c == (int *)0x0)) {
        uVar3 = FUN_0056d6a0();
        (**(code **)(*param_1 + 0x7c))(uVar3);
        param_1[0x41d] = 0;
        FUN_00589bf0();
        FUN_00589c30();
        ExceptionList = local_10;
        return DAT_009944bc;
      }
      if ((local_8c != (int *)0x0) && (iVar4 = (**(code **)(*local_8c + 0x30))(), iVar4 != 0)) {
        piVar5 = (int *)(**(code **)(*local_8c + 0x30))();
        iVar4 = (**(code **)(*piVar5 + 0x94))();
        if (iVar4 == 0) {
          piVar5 = (int *)(**(code **)(*local_8c + 0x30))();
          if (piVar5[0x46b] == 0) {
            bVar1 = false;
          }
          else {
            sVar2 = (**(code **)(*piVar5 + 0x9c))(0);
            bVar1 = sVar2 < 1;
          }
          if (!bVar1) goto LAB_005b85f3;
        }
        uVar3 = FUN_0056d6a0();
        (**(code **)(*param_1 + 0x7c))(uVar3);
        param_1[0x41d] = 0;
        FUN_00589bf0();
        FUN_00589c30();
        ExceptionList = local_10;
        return DAT_009944bc;
      }
LAB_005b85f3:
      if (local_3c == 0x7f000000) {
        param_1[0x15b] = param_1[0x24];
      }
      else {
        param_1[0x15b] = local_3c;
      }
      local_88 = (int *)0x0;
      if (local_8c != (int *)0x0) {
        local_88 = (int *)(**(code **)(*local_8c + 0x30))();
      }
      local_40 = FUN_0051c0e0(local_58);
      if (local_40 == 0) {
        uVar3 = FUN_0056d6a0();
        (**(code **)(*param_1 + 0x7c))(uVar3);
        param_1[0x41d] = 0;
        FUN_00589bf0();
        FUN_00589c30();
        ExceptionList = local_10;
        return DAT_009944bc;
      }
      local_24 = FUN_00604260(local_74);
      if (local_24 == 0) {
        uVar3 = FUN_0056d6a0();
        (**(code **)(*param_1 + 0x7c))(uVar3);
        param_1[0x41d] = 0;
        FUN_00589bf0();
        FUN_00589c30();
        ExceptionList = local_10;
        return DAT_009944bc;
      }
      local_2c = (uint)*(ushort *)(local_24 + 2);
      local_2c = FUN_00576430(local_2c);
      local_84 = FUN_006c1450(local_2c);
      if (local_84 != 0) {
        if (*(int *)(local_84 + 0x154) == 0) {
          FUN_00563d20(2);
        }
        else {
          FUN_00563d20(1);
          FUN_0057a560(1,1,0);
        }
        puVar11 = &local_48;
        puVar12 = &local_54;
        FUN_0051c350(param_1[1]);
        FUN_0051ad20(puVar12,puVar11);
        puVar14 = &local_60;
        puVar13 = local_44;
        FUN_0051c370(local_54,local_48,local_4c,local_90,puVar13,puVar14);
        FUN_0051af60(local_54,local_48,local_4c,local_90,puVar13,puVar14);
        local_d4 = local_50;
        local_d0 = local_5c;
        local_cc = local_70;
        local_20 = local_50;
        local_1c = local_5c;
        local_18 = local_70;
        FUN_00733540();
        local_8 = 0;
        FUN_00733570("MODELTYPE");
        local_8._0_1_ = 1;
        FUN_0071b1d0((short)param_1[0x461],local_dc,local_6c);
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        iVar4 = FUN_00734f60(&DAT_00991b40);
        if ((iVar4 == 0) && (iVar4 = FUN_00734f60(&DAT_00996334), iVar4 == 0)) {
          local_64 = 0;
        }
        else {
          local_64 = 1;
        }
        *(undefined4 *)(param_2 + 0x70) = 0;
        local_14 = 0;
        iVar4 = FUN_006d6e30();
        switch(*(undefined1 *)(iVar4 + 0xac)) {
        case 6:
        case 0x31:
          if (local_64 == 0) {
            local_28 = 0x2711;
            local_78 = 800;
          }
          else {
            local_a4 = 0;
            local_a8 = FUN_006d0620(0x100);
            if ((local_a8 != 0) && (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) == '0')) {
              local_a4 = 1;
            }
            if (((local_a4 == 0) && (local_a8 = FUN_006d0620(0x80), local_a8 != 0)) &&
               (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) == '0')) {
              local_a4 = 1;
            }
            if (local_a4 == 0) {
              FUN_004452b0(&local_b4,&local_20,param_1 + 0x25);
              if (local_b4 * local_b4 + local_b0 * local_b0 + local_ac * local_ac <
                  _DAT_00a0f5d0 * _DAT_00a0f5d0) {
                local_28 = 0x2792;
                local_78 = 700;
              }
              else {
                local_28 = 0x2791;
                local_78 = 800;
              }
            }
            else {
              local_28 = 0x28b1;
              local_78 = 0x3b6;
            }
          }
          local_80 = 0x5dc;
          local_14 = 1;
          break;
        default:
          local_28 = 0x2721;
          local_78 = 1;
          local_80 = *(int *)(local_84 + 0xb4) + 1;
          break;
        case 0xc:
          local_b8 = FUN_0058ae90();
          local_80 = 0x5dc;
          if (local_b8 == 0) {
            local_78 = 300;
          }
          else {
            fVar7 = (float10)FUN_007ed910(0x2af9);
            local_280 = (int)(longlong)ROUND(fVar7);
            local_78 = local_280 - 0x32;
          }
          if (local_60 < local_78) {
            local_28 = 0x2af9;
          }
          else {
            local_28 = 0x2afa;
          }
          break;
        case 0x14:
          if (local_64 == 0) {
            local_28 = 0x2711;
          }
          else {
            local_28 = 0x2798;
          }
          local_78 = 600;
          local_80 = 1000;
          break;
        case 0x19:
        case 0x1a:
        case 0x2d:
          if (local_64 == 0) {
            local_28 = 0x2711;
          }
          else if (local_88 == param_1) {
            local_28 = 0x2756;
          }
          else {
            local_28 = 0x274d;
          }
          local_78 = 0x2ee;
          local_80 = 0x5dc;
          break;
        case 0x2f:
          if (local_64 == 0) {
            local_28 = 0x2711;
          }
          else {
            local_28 = 0x2798;
          }
          local_78 = 0x2ee;
          local_80 = 0x5dc;
        }
        if (*(int *)(param_1[0x437] + 0xa84) == 0) {
          local_bc = 0;
          local_c0 = 0;
          if (local_14 == 0) {
            local_bc = FUN_00590ff0(local_3c);
            local_c0 = FUN_005912b0(local_3c,&local_bc);
          }
          else {
            local_bc = 1;
            local_c0 = 1;
          }
          FUN_0058e730(local_3c,local_bc,local_c0,0,0);
          FUN_0058e6d0(0,0);
          FUN_0058fc60(1,param_1[1]);
          FUN_0058fcd0(local_80,0);
          FUN_0058fd50(local_80,0);
        }
        if (param_1[0x41d] == 0) {
          iVar4 = FUN_006d6e30();
          if (((*(char *)(iVar4 + 0xac) == '\x19') ||
              (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) == '\x19')) &&
             ((param_1[0x43a] == 0 || (iVar4 = FUN_0051c830(2), iVar4 == 0)))) {
            FUN_0040e770(0x7f000000);
          }
          param_1[0x41d] = 1;
        }
        if (local_60 < local_78) {
          sVar2 = FUN_00603b50(local_74 & 0xff);
          if (((sVar2 == 0) && (*(short *)(local_24 + 6) != 0xd)) && (*(short *)(local_24 + 6) != 7)
             ) {
            uVar3 = FUN_0056d6a0();
            (**(code **)(*param_1 + 0x7c))(uVar3);
            param_1[0x41d] = 0;
            FUN_00589bf0();
            FUN_00589c30();
            FUN_00584da0(0x17,0);
            uVar3 = DAT_009944bc;
            local_8 = 0xffffffff;
            FUN_00733780();
            ExceptionList = local_10;
            return uVar3;
          }
          param_1[0x79] = local_2c;
          param_1[0x78] = local_3c;
          param_1[0x75] = local_20;
          param_1[0x76] = local_1c;
          param_1[0x77] = local_18;
          param_1[0x7c] = 0;
          param_1[0x419] = local_58;
          param_1[0x41b] = 0;
          FUN_00548b10(0);
          uVar6 = FUN_006ad2f0(*(byte *)(param_1[0x466] + 0x8d) - 1,0);
          param_1[0x41c] = uVar6 & 0xff;
          if ((local_88 == (int *)0x0) ||
             (((iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) != '\x19' &&
               (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) != '-')) &&
              (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) != '/')))) {
            (**(code **)(*param_1 + 0x7c))(local_28);
          }
          else {
            (**(code **)(*local_88 + 0x7c))(local_28);
          }
          uVar3 = DAT_009944b4;
          local_8 = 0xffffffff;
          FUN_00733780();
          ExceptionList = local_10;
          return uVar3;
        }
        if (param_1[0x7c] == 0) {
          param_1[0x79] = local_2c;
          param_1[0x78] = local_3c;
          param_1[0x75] = local_20;
          param_1[0x76] = local_1c;
          param_1[0x77] = local_18;
          param_1[0x7c] = 0;
          param_1[0x419] = local_58;
          param_1[0x41b] = 1;
          FUN_00548b10(0);
          uVar6 = FUN_006ad2f0(*(byte *)(param_1[0x466] + 0x8d) - 1,0);
          param_1[0x41c] = uVar6 & 0xff;
          if ((local_88 == (int *)0x0) ||
             (((iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) != '\x19' &&
               (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) != '-')) &&
              (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0xac) != '/')))) {
            (**(code **)(*param_1 + 0x7c))(local_28);
          }
          else {
            (**(code **)(*local_88 + 0x7c))(local_28);
          }
          *(int *)(param_1[0x437] + 0xab8) = *(int *)(param_1[0x437] + 0xab8) + 1;
          iVar4 = FUN_006d6e30();
          if ((*(char *)(iVar4 + 0xac) == '\x06') && ((int)local_2c < 0xf6)) {
            FUN_005482a0(local_2c,&local_20,0x7f000000,0xff,local_58,0,0,0,0);
          }
          else {
            FUN_005482a0(local_2c,&local_20,local_3c,0xff,local_58,0,0,0,0);
          }
          iVar4 = FUN_00590180();
          if (iVar4 == 1) {
            FUN_00591a70();
          }
          iVar4 = FUN_0072fb00();
          if (*(int *)(iVar4 + 0x94) == 0) {
            FUN_006070a0(local_74,param_1[1]);
          }
          iVar4 = FUN_006d6e30();
          if (*(char *)(iVar4 + 0xac) == '\f') {
            local_c4[0] = 0x2afa;
            local_c8 = 1;
            _memcpy((void *)(param_2 + 0x5c),&local_c8,4);
            _memcpy((void *)(param_2 + 0x60),local_c4,2);
            _memcpy((void *)(param_2 + 100),&local_80,4);
            *(undefined2 *)(param_2 + 0x6e) = 0xc;
          }
        }
        uVar3 = DAT_009944b4;
        if (local_60 < local_80) {
          local_8 = 0xffffffff;
          FUN_00733780();
          ExceptionList = local_10;
          return uVar3;
        }
        uVar3 = FUN_0056d6a0();
        (**(code **)(*param_1 + 0x7c))(uVar3);
        param_1[0x41d] = 0;
        FUN_00589bf0();
        FUN_00589c30();
        uVar3 = DAT_009944b8;
        local_8 = 0xffffffff;
        FUN_00733780();
        ExceptionList = local_10;
        return uVar3;
      }
      uVar3 = FUN_0056d6a0();
      (**(code **)(*param_1 + 0x7c))(uVar3);
      param_1[0x41d] = 0;
      FUN_00589bf0();
      FUN_00589c30();
      ExceptionList = local_10;
      return DAT_009944bc;
    }
  }
  (**(code **)(*param_1 + 0x7c))(10000);
  param_1[0x41d] = 0;
  FUN_00589bf0();
  FUN_00589c30();
  ExceptionList = local_10;
  return DAT_009944bc;
}

