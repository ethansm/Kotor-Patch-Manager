// FN 008ac330 FUN_008ac330 size=2719 ret=4 callers=008b0140,008aed10,008ad930,008ab060,008abb30 callees=0051c8b0,0051c0e0,0041d290,0084a640,0041c7c0,0073f530,0058a2c0,0072fb00,007408f0,005bda40,008b1d00,006d6e30,008b1250,00416e30,005bd6a0,0077d800,00733780,0041fc30,008b01b0,00607690,00733570,00606100,0083ea60,0076e610,0064ddb0

void __thiscall FUN_008ac330(int param_1,int param_2)

{
  char cVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  uint local_88;
  uint local_84;
  int local_80;
  int local_7c;
  uint local_78;
  uint local_74;
  int local_70;
  int local_6c;
  int local_68;
  char local_61;
  int local_60;
  int local_5c;
  char local_55;
  int local_54;
  int local_50;
  int local_4c;
  undefined4 local_48;
  int local_44;
  int local_40;
  undefined1 local_3c [4];
  int local_38;
  undefined4 local_30;
  int local_2c;
  uint local_28;
  int local_24;
  uint local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097899e;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x68) != 0) {
    local_18 = *(int *)(param_2 + 0x5c);
    local_1c = 0;
    *(undefined4 *)(param_1 + 0x5098) = *(undefined4 *)(&DAT_009a7400 + local_18 * 0x14);
    if (*(int *)(param_1 + 0x5098) == -1) {
      FUN_0041d290(uVar2);
      FUN_00733570(&DAT_00986925);
      local_8 = 2;
      FUN_00416e30(local_98);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    else {
      local_50 = FUN_0077d800();
      local_2c = FUN_0058a2c0(1);
      FUN_0084a640(0);
      local_8 = 0;
      local_4c = 0;
      local_44 = 0;
      local_44 = FUN_008b01b0(0);
      FUN_008b1250(0x7f000000,0,0);
      FUN_008b1d00(0);
      *(undefined1 *)(local_44 + 0x58) = 0xff;
      FUN_0083ea60(local_44);
      local_4c = local_4c + 1;
      if (*(int *)(param_1 + 0x509c + local_18 * 4) != 0x7f000000) {
        local_44 = FUN_008b01b0(local_4c);
        if (((*(int *)(local_44 + 0x1d0) != *(int *)(param_1 + 0x509c + local_18 * 4)) ||
            ((*(uint *)(local_44 + 0x3ac) >> 1 & 1) != 1)) ||
           (*(char *)(local_44 + 0x3b0) != '\x01')) {
          local_1c = 1;
        }
        FUN_008b1250(*(undefined4 *)(param_1 + 0x509c + local_18 * 4),1,0);
        FUN_008b1d00(1);
        *(undefined1 *)(local_44 + 0x58) = 0xff;
        FUN_0083ea60(local_44);
        local_4c = local_4c + 1;
      }
      local_20 = 1;
      if ((*(int *)(param_1 + 0x5098) == 0x20) && (*(int *)(param_1 + 0x50a0) != 0x7f000000)) {
        uVar3 = FUN_0051c8b0(*(undefined4 *)(param_1 + 0x50a0));
        local_24 = FUN_0051c0e0(uVar3);
        if (local_24 != 0) {
          iVar4 = FUN_006d6e30();
          local_20 = (uint)(*(char *)(iVar4 + 0x1b) != '\x04');
        }
      }
      local_48 = *(undefined4 *)(param_1 + 0x5098);
      if (*(int *)(param_1 + 0x5098) == 0x80000) {
        *(uint *)(param_1 + 0x5098) = *(uint *)(param_1 + 0x5098) | 0x20;
      }
      if (*(int *)(param_1 + 0x5098) == 0x40000) {
        *(uint *)(param_1 + 0x5098) = *(uint *)(param_1 + 0x5098) | 0x10;
      }
      if (local_20 != 0) {
        for (local_28 = 0; local_28 < *(uint *)(local_2c + 0x10); local_28 = local_28 + 1) {
          local_24 = FUN_00606100(local_28);
          if ((local_24 != 0) &&
             (uVar2 = *(uint *)(param_1 + 0x5098), iVar4 = FUN_006d6e30(),
             (*(uint *)(iVar4 + 4) & uVar2) != 0)) {
            local_54 = 0;
            iVar4 = FUN_006d6e30();
            local_55 = *(char *)(iVar4 + 0xb4);
            if (local_55 != '\0') {
              if (local_55 == '\x01') {
                if (*(short *)(*(int *)(local_50 + 0x1198) + 0xe0) != DAT_0099a8bc) {
                  local_54 = 1;
                }
              }
              else if ((local_55 == '\x02') &&
                      (*(short *)(*(int *)(local_50 + 0x1198) + 0xe0) != DAT_0099a8b8)) {
                local_54 = 1;
              }
            }
            if (local_54 == 0) {
              local_61 = '\0';
              local_14 = *(undefined4 *)(param_1 + 0x5098);
              cVar1 = FUN_005bd6a0(local_24,&local_14,0,0,0);
              if (cVar1 == '\0') {
                local_61 = '\x02';
              }
              local_5c = 1;
              local_60 = FUN_0072fb00();
              if ((local_60 != 0) && ((*(uint *)(local_60 + 0x1c) & 1) != 0)) {
                iVar4 = FUN_005bda40(local_24,*(undefined4 *)(param_1 + 0x5098));
                if (iVar4 == 0) {
                  local_5c = 0;
                }
                local_14 = *(undefined4 *)(param_1 + 0x5098);
                cVar1 = FUN_005bd6a0(local_24,&local_14,0,0,0);
                if (cVar1 == '\0') {
                  local_5c = 0;
                }
              }
              if ((local_5c != 0) && (local_40 = FUN_00607690(), local_40 != 0)) {
                local_68 = 0;
                if ((*(int *)(param_1 + 0x5098) == 0x10) || (*(int *)(param_1 + 0x5098) == 0x20)) {
                  local_70 = 0;
                  local_6c = 0;
                  if (*(int *)(param_1 + 0x5098) == 0x10) {
                    local_70 = local_40;
                    uVar3 = FUN_0076e610(0x20);
                    local_6c = FUN_0073f530(uVar3);
                  }
                  else {
                    uVar3 = FUN_0076e610(0x10);
                    local_70 = FUN_0073f530(uVar3);
                    local_6c = local_40;
                  }
                  if ((local_70 == 0) || (local_6c == 0)) {
                    local_68 = 1;
                  }
                  else if ((local_70 == local_40) &&
                          (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0x1b) == '\x04')) {
                    local_68 = 1;
                  }
                  else {
                    iVar4 = FUN_006d6e30();
                    local_74 = (uint)*(byte *)(iVar4 + 8);
                    iVar4 = FUN_006d6e30();
                    local_78 = (uint)*(byte *)(iVar4 + 8);
                    if ((local_74 == local_78) && ((local_74 == 2 || (local_74 == 4)))) {
                      local_68 = 1;
                    }
                  }
                  if (local_68 == 0) {
                    local_61 = '\x03';
                  }
                }
                if (((*(uint *)(param_1 + 0x5098) & 0x40000) == 0) &&
                   ((*(uint *)(param_1 + 0x5098) & 0x80000) == 0)) {
                  local_68 = 1;
                }
                else {
                  local_68 = 0;
                  local_80 = 0;
                  local_7c = 0;
                  if ((*(uint *)(param_1 + 0x5098) & 0x40000) == 0) {
                    uVar3 = FUN_0076e610(0x40000);
                    local_80 = FUN_0073f530(uVar3);
                    local_7c = local_40;
                  }
                  else {
                    local_80 = local_40;
                    uVar3 = FUN_0076e610(0x80000);
                    local_7c = FUN_0073f530(uVar3);
                  }
                  if ((local_80 == 0) || (local_7c == 0)) {
                    local_68 = 1;
                  }
                  else if ((local_80 == local_40) &&
                          (iVar4 = FUN_006d6e30(), *(char *)(iVar4 + 0x1b) == '\x04')) {
                    local_68 = 1;
                  }
                  else {
                    iVar4 = FUN_006d6e30();
                    local_84 = (uint)*(byte *)(iVar4 + 8);
                    iVar4 = FUN_006d6e30();
                    local_88 = (uint)*(byte *)(iVar4 + 8);
                    if ((local_84 == local_88) && ((local_84 == 2 || (local_84 == 4)))) {
                      local_68 = 1;
                    }
                  }
                  if (local_68 == 0) {
                    local_61 = '\x03';
                    local_68 = 1;
                  }
                }
                if (local_68 != 0) {
                  local_44 = FUN_008b01b0(local_4c);
                  if (((*(int *)(local_44 + 0x1d0) != *(int *)(local_40 + 0x14)) ||
                      ((*(uint *)(local_44 + 0x3ac) >> 1 & 1) != 0)) ||
                     (*(char *)(local_44 + 0x3b0) != local_61)) {
                    local_1c = 1;
                  }
                  FUN_008b1250(*(undefined4 *)(local_40 + 0x14),0,0);
                  FUN_008b1d00(local_61);
                  *(undefined1 *)(local_44 + 0x58) = 0xff;
                  FUN_0083ea60(local_44);
                  local_4c = local_4c + 1;
                }
              }
            }
          }
        }
      }
      *(undefined4 *)(param_1 + 0x5098) = local_48;
      if ((local_1c != 0) || (local_38 != *(int *)(param_1 + 0x39dc))) {
        FUN_0041fc30(local_3c,0,1,0);
        FUN_0041c7c0(0);
      }
      if (*(short *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0x24) == DAT_0099a8b8) {
        local_30 = *(undefined4 *)(&DAT_009a7410 + local_18 * 0x14);
      }
      else {
        local_30 = *(undefined4 *)(&DAT_009a740c + local_18 * 0x14);
      }
      uVar3 = FUN_007408f0(local_90,local_30);
      local_8._0_1_ = 1;
      FUN_00416e30(uVar3);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      Free();
    }
  }
  ExceptionList = local_10;
  return;
}

