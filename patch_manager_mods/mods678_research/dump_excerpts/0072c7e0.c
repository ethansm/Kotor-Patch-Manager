// FN 0072c7e0 FUN_0072c7e0 size=5830 ret=12 callers=0072fa80 callees=00521060,0064dde0,00736220,009196fd,0091d220,00919bcc,0072f500,0072e730,00561430,007a1720,00732960,0058c370,00732c40,0073a560,00732aa0,0072f480,0072b3e0,0091d67e,0072f5c0,0072f540,007a1780,005936d0,00731390,0058a1b0

undefined4 __thiscall FUN_0072c7e0(int param_1,undefined4 param_2,undefined4 *param_3,int param_4)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  int *piVar7;
  undefined4 *puVar8;
  uint *puVar9;
  undefined8 uVar10;
  int local_24c;
  int local_244;
  undefined4 *local_240;
  int local_1f8;
  uint local_1f4;
  uint local_1dc;
  int local_1d8;
  uint local_1d4;
  int local_1c0;
  int local_190;
  int local_18c;
  int local_188;
  int local_184;
  int local_180;
  int local_17c;
  int local_178;
  int local_174;
  int local_170;
  int local_16c;
  int local_168;
  int local_164;
  int local_160;
  int local_15c;
  int *local_158;
  int local_154;
  int local_150;
  int *local_14c;
  int *local_148;
  int *local_144;
  int local_140;
  int local_13c;
  uint auStack_138 [10];
  undefined4 local_110;
  int local_10c;
  int *local_108;
  int local_104;
  uint local_100 [6];
  uint local_e8 [25];
  int local_84;
  int local_80;
  uint local_7c;
  int local_78 [10];
  int local_50;
  int *local_4c;
  int local_48;
  int aiStack_44 [10];
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961015;
  local_10 = ExceptionList;
  uVar5 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_50 = -1;
  _eh_vector_constructor_iterator_(local_100,0xc,10,FUN_007306e0,COleDispParams::~COleDispParams);
  local_8 = 1;
  local_110 = 0;
  local_1c = 0;
  *param_3 = 0;
  FUN_0072f5c0(0);
  if (*(int *)(param_1 + 0x1a0) == 0) {
    FUN_0072f500(0);
    local_8 = local_8 & 0xffffff00;
    _eh_vector_destructor_iterator_(local_100,0xc,10,COleDispParams::~COleDispParams);
  }
  else if (param_4 < 6) {
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1b8); local_14 = local_14 + 1) {
      iVar6 = FUN_007a1720(uVar5);
      aiStack_44[local_14] = iVar6;
    }
    if (*(int *)(param_1 + 0x410) == 0) {
      FUN_00732c40(local_100 + DAT_0099c884 * 3);
    }
    for (local_14 = 2; local_14 < *(int *)(param_1 + 0x1b8); local_14 = local_14 + 1) {
      FUN_00731390(local_14 + -2,local_100 + local_14 * 3);
    }
    local_84 = 1;
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1b8); local_14 = local_14 + 1) {
      auStack_138[local_14] = 0;
      if (auStack_138[local_14] < local_100[local_14 * 3 + 1]) {
        local_84 = 0;
      }
    }
    *(undefined4 *)(param_1 + 0x1b0) = 0;
    local_18 = 0;
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1b8); local_14 = local_14 + 1) {
      local_18 = local_18 + local_100[local_14 * 3 + 1];
    }
    if (local_18 != 0) {
      while (local_84 == 0) {
        local_13c = 0;
        for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1b8); local_14 = local_14 + 1) {
          if (((local_50 == -1) || (local_14 == local_50)) ||
             (local_100[local_50 * 3 + 1] <= auStack_138[local_50])) {
            while ((local_13c == 0 && (auStack_138[local_14] < local_100[local_14 * 3 + 1]))) {
              local_148 = (int *)(auStack_138[local_14] * 0x14 + local_100[local_14 * 3]);
              if (local_14 < 2) {
                local_24c = local_14;
              }
              else {
                local_24c = 2;
              }
              local_140 = param_1 + param_4 * 0x40 + 0x14 + local_24c * 4;
              local_144 = (int *)0x0;
              local_14c = (int *)0x0;
              iVar6 = FUN_007a1720();
              aiStack_44[local_14] = iVar6;
              if (aiStack_44[local_14] != 0) {
                local_14c = (int *)FUN_00521060();
              }
              while (aiStack_44[local_14] != 0) {
                local_144 = *(int **)(*(int *)(param_1 + 0x188) + *local_14c * 4);
                if (*(int *)(param_1 + 0x1c4 + local_144[7] * 4) == *local_148) {
                  local_13c = 1;
                  break;
                }
                iVar6 = (**(code **)(*local_144 + 4))();
                if ((iVar6 != 0) && (*(int *)(param_1 + 0x1c4 + local_144[0xc] * 4) == *local_148))
                {
                  local_13c = 1;
                  break;
                }
                local_14c = (int *)FUN_0058c370(aiStack_44 + local_14);
              }
              if (local_13c == 0) {
                auStack_138[local_14] = auStack_138[local_14] + 1;
              }
            }
          }
        }
        if (local_13c != 0) {
          local_150 = 0;
          for (local_14 = 0; (local_150 != 1 && (local_14 < *(int *)(param_1 + 0x1b8)));
              local_14 = local_14 + 1) {
            if (auStack_138[local_14] < local_100[local_14 * 3 + 1]) {
              if (local_50 == -1) {
                local_104 = *(int *)(local_100[local_14 * 3] + 0xc + auStack_138[local_14] * 0x14);
                local_50 = local_14;
                local_150 = 1;
              }
              else if (local_104 ==
                       *(int *)(local_100[local_14 * 3] + 0xc + auStack_138[local_14] * 0x14)) {
                local_104 = *(int *)(local_100[local_14 * 3] + 0xc + auStack_138[local_14] * 0x14);
                local_50 = local_14;
                local_150 = 1;
              }
            }
          }
          if (local_150 == 0) {
            local_104 = local_104 + 1;
            local_50 = -1;
          }
          else {
            local_158 = (int *)(auStack_138[local_50] * 0x14 + local_100[local_50 * 3]);
            piVar7 = (int *)FUN_00561430(aiStack_44[local_50]);
            local_154 = *(int *)(*(int *)(param_1 + 0x188) + *piVar7 * 4);
            if (*(int *)(local_154 + 0xc) == 0) {
              local_160 = 0;
              local_15c = 0;
              switch(*(undefined4 *)(local_154 + 0x14)) {
              case 0:
              case 2:
              case 5:
                *(int *)(local_154 + 4) = local_158[1];
                break;
              case 1:
                iVar6 = FUN_00732aa0(local_50,*local_158);
                if (iVar6 == 0) {
                  local_174 = FUN_0058a1b0(*local_158);
                  if (local_158[1] < -local_174) {
                    local_160 = -1;
                  }
                  else if (local_174 < local_158[1]) {
                    local_160 = 1;
                  }
                  else {
                    local_160 = 0;
                  }
                  iVar6 = FUN_00732960(local_50,*local_158);
                  if (iVar6 < -local_174) {
                    local_15c = -1;
                  }
                  else if (local_174 < iVar6) {
                    local_15c = 1;
                  }
                  else {
                    local_15c = 0;
                  }
                  if (local_160 != local_15c) {
                    local_17c = local_158[3];
                    local_180 = local_158[2];
                    local_184 = local_160;
                    local_178 = local_154;
                    *(int *)(local_154 + 4) = local_160;
                    FUN_0072f540(local_160,local_180,local_17c,local_154);
                    *(int *)(param_1 + 0x1b0) = *(int *)(param_1 + 0x1b0) + 1;
                  }
                }
                else {
                  local_168 = local_158[3];
                  local_16c = local_158[2];
                  local_170 = local_158[1];
                  local_164 = local_154;
                  *(int *)(local_154 + 4) = local_158[1];
                  FUN_0072f540(local_170,local_16c,local_168,local_154);
                  *(int *)(param_1 + 0x1b0) = *(int *)(param_1 + 0x1b0) + 1;
                }
                break;
              case 3:
                *(int *)(local_154 + 0x24) = *(int *)(local_154 + 0x24) + local_158[1];
                break;
              case 4:
                local_188 = local_154;
                if (*local_158 == *(int *)(param_1 + 0x1c4 + *(int *)(local_154 + 0x1c) * 4)) {
                  *(int *)(local_154 + 0x24) = local_158[1];
                }
                else {
                  *(int *)(local_154 + 0x34) = local_158[1];
                }
              }
            }
            auStack_138[local_50] = auStack_138[local_50] + 1;
          }
        }
        local_84 = 1;
        for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1b8); local_14 = local_14 + 1) {
          if (auStack_138[local_14] < local_100[local_14 * 3 + 1]) {
            local_84 = 0;
          }
        }
      }
    }
    if ((*(int *)(param_1 + 0x1a0) != 0) && (*(int *)(*(int *)(param_1 + 0x1a0) + 4) != 0)) {
      if (*(int *)(param_1 + 0x3e4) != -1) {
        FUN_0072e730(DAT_0099c888,DAT_0099c8a8,*(undefined4 *)(param_1 + 0x3e4));
      }
      if (*(int *)(param_1 + 1000) != -1) {
        FUN_0072e730(DAT_0099c888,DAT_0099c8ac,*(undefined4 *)(param_1 + 1000));
      }
    }
    while (iVar6 = FUN_007a1780(), iVar6 == 0) {
      puVar8 = (undefined4 *)FUN_005936d0();
      local_18c = param_1 + param_4 * 0x40 + 0x14 + *(int *)(puVar8[3] + 0x18) * 4;
      local_190 = FUN_007a1720();
      while (local_190 != 0) {
        piVar7 = (int *)FUN_00561430(local_190);
        puVar1 = *(undefined4 **)(*(int *)(param_1 + 0x188) + *piVar7 * 4);
        if ((puVar1[7] & 0xff) == *(uint *)(puVar8[3] + 0x1c)) {
          uVar2 = puVar8[2];
          uVar3 = puVar8[1];
          uVar4 = *puVar8;
          iVar6 = (**(code **)*puVar1)();
          if (iVar6 != 0) {
            puVar1[9] = uVar4;
          }
          FUN_0072f540(uVar4,uVar3,uVar2,puVar1);
          *(int *)(param_1 + 0x1b0) = *(int *)(param_1 + 0x1b0) + 1;
          break;
        }
        FUN_0058c370(&local_190);
      }
      FUN_009196fd(puVar8[3]);
      FUN_009196fd(puVar8);
    }
    *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
    *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    uVar10 = FUN_00736220();
    local_80 = __aulldiv(uVar10,1000,0);
    local_7c = local_80 - *(int *)(param_1 + 0x19c);
    *(int *)(param_1 + 0x19c) = local_80;
    local_10c = FUN_007a1720();
    while (local_10c != 0) {
      local_108 = (int *)FUN_00561430(local_10c);
      iVar6 = local_10c;
      if (*local_108 == 0) {
        FUN_0058c370(&local_10c);
      }
      else if (local_7c < *(uint *)(*local_108 + 0xc)) {
        *(uint *)(*local_108 + 0xc) = *(int *)(*local_108 + 0xc) - local_7c;
        FUN_0058c370(&local_10c);
      }
      else {
        *(undefined4 *)(*local_108 + 0xc) = 0;
        FUN_0058c370(&local_10c);
        FUN_0064dde0(iVar6);
      }
    }
    local_10c = FUN_007a1720();
    while (local_10c != 0) {
      local_4c = (int *)FUN_00561430(local_10c);
      if ((local_4c != (int *)0x0) && (iVar6 = FUN_0072b3e0(*local_4c,param_4), iVar6 != 0)) {
        iVar6 = *(int *)(*(int *)(param_1 + 0x188) + *local_4c * 4);
        if (*(int *)(iVar6 + 4) == 0) {
          local_4c[3] = 0;
          local_4c[6] = 0;
          local_4c[4] = 0;
          local_4c[5] = 0;
        }
        else if ((local_4c[3] == 0) && (local_4c[6] == 0)) {
          local_4c[3] = 1;
          local_4c[4] = *(int *)(iVar6 + 4);
          local_4c[5] = local_4c[4];
        }
        else {
          if (local_4c[4] != *(int *)(iVar6 + 4)) {
            local_4c[3] = 1;
            local_4c[4] = *(int *)(iVar6 + 4);
            local_4c[5] = local_4c[4];
            local_4c[6] = 0;
            break;
          }
          local_4c[3] = local_4c[3] + local_7c;
          if (local_4c[6] == 0) {
            if ((uint)local_4c[1] < (uint)local_4c[3]) {
              local_4c[5] = 0;
              FUN_0072f540(0,0,0,iVar6);
              *(int *)(param_1 + 0x1b0) = *(int *)(param_1 + 0x1b0) + 1;
              local_4c[6] = 1;
              local_4c[3] = 0;
            }
          }
          else if ((uint)local_4c[2] < (uint)local_4c[3]) {
            if (local_4c[5] == local_4c[4]) {
              local_1c0 = 0;
              local_4c[5] = 0;
            }
            else {
              local_1c0 = local_4c[4];
              local_4c[5] = local_4c[4];
            }
            FUN_0072f540(local_1c0,0,0,iVar6);
            *(int *)(param_1 + 0x1b0) = *(int *)(param_1 + 0x1b0) + 1;
            local_4c[3] = 0;
          }
        }
      }
      FUN_0058c370(&local_10c);
    }
    for (local_1d4 = 0; local_1d4 < 10; local_1d4 = local_1d4 + 1) {
      local_78[local_1d4] = -1;
    }
    for (local_1d8 = 2; local_1d8 < 10; local_1d8 = local_1d8 + 1) {
      for (local_1dc = 0; local_1dc < local_100[local_1d8 * 3 + 1]; local_1dc = local_1dc + 1) {
        puVar9 = (uint *)(local_100[local_1d8 * 3] + local_1dc * 0x14);
        local_1f4 = *puVar9;
        uVar5 = puVar9[1];
        if ((local_1f4 < 0x30) || (0x4f < local_1f4)) {
          if (local_1f4 == 0x20) {
            if (uVar5 == 0) {
              local_78[0] = 0x31;
              local_78[2] = 0x31;
            }
            else if (uVar5 == 0x5a) {
              local_78[0] = 0x30;
              local_78[2] = 0x30;
            }
            else if (uVar5 == 0xb4) {
              local_78[0] = 0x32;
              local_78[2] = 0x32;
            }
            else if (uVar5 == 0x10e) {
              local_78[0] = 0x2f;
              local_78[2] = 0x2f;
            }
          }
          else if (((local_1f4 != 0xc) && (local_1f4 != 0x10)) && (local_1f4 == 8)) {
            local_78[2] = 0xf2;
          }
        }
        else if (uVar5 != 0) {
          if (*(char *)(*(int *)(param_1 + 0x1a0) + 0x1c) != '\0') {
            if (local_1f4 == 0x30) {
              local_1f4 = 0x32;
            }
            else if (local_1f4 == 0x31) {
              local_1f4 = 0x30;
            }
            else if (local_1f4 == 0x32) {
              local_1f4 = 0x31;
            }
            else if (local_1f4 == 0x3a) {
              local_1f4 = 0x38;
            }
            else if (local_1f4 == 0x3b) {
              local_1f4 = 0x39;
            }
            else if (local_1f4 == 0x38) {
              local_1f4 = 0x36;
            }
            else if (local_1f4 == 0x39) {
              local_1f4 = 0x37;
            }
            else if (local_1f4 == 0x36) {
              local_1f4 = 0x2f;
            }
            else if (local_1f4 == 0x37) {
              local_1f4 = 0x2f;
            }
          }
          if (local_1f4 == 0x30) {
            local_78[0] = 0x27;
            local_78[3] = 0x27;
            local_78[4] = 0xd9;
            local_78[8] = 0x10;
          }
          else if (local_1f4 == 0x31) {
            local_78[0] = 0x28;
            local_78[2] = 0x28;
            local_78[4] = 0x16;
            local_78[8] = 0x11;
          }
          else if (local_1f4 == 0x32) {
            local_78[0] = 0x29;
            local_78[2] = 0x29;
            local_78[8] = 0x4d;
          }
          else if (local_1f4 == 0x33) {
            local_78[0] = 0x2a;
            local_78[2] = 0x2a;
            local_78[4] = 0xf0;
            local_78[8] = 0x4e;
          }
          else if (local_1f4 == 0x34) {
            local_78[0] = 0x35;
            local_78[2] = 0xcc;
          }
          else if (local_1f4 == 0x35) {
            local_78[0] = 0x36;
            local_78[2] = 0xcd;
          }
          else if (local_1f4 == 0x36) {
            local_78[0] = 0x2e;
            local_78[2] = 0xb;
          }
          else if (local_1f4 == 0x37) {
            local_78[0] = 0x2d;
            local_78[2] = 0xe0;
            local_78[4] = 0xfd;
          }
          else if (local_1f4 == 0x38) {
            local_78[2] = 0xf2;
          }
          else if (local_1f4 == 0x39) {
            local_78[2] = 1;
            local_78[8] = 1;
          }
        }
      }
    }
    local_48 = -1;
    switch(param_4) {
    case 0:
      local_48 = 1;
      break;
    case 1:
      local_48 = 2;
      break;
    case 2:
      local_48 = 0;
      break;
    case 3:
      local_48 = 0;
      break;
    case 4:
      local_48 = 4;
      break;
    case 5:
      local_48 = 4;
    }
    if (-1 < local_48) {
      for (local_1f8 = 1; -1 < local_1f8; local_1f8 = local_1f8 + -1) {
        iVar6 = local_78[local_48 * 2 + local_1f8];
        if (-1 < iVar6) {
          if ((DAT_00a7e9e0 & 1) == 0) {
            DAT_00a7e9e0 = DAT_00a7e9e0 | 1;
            local_244 = 2;
            local_240 = &DAT_00a7e9a0;
            while (local_244 = local_244 + -1, -1 < local_244) {
              *local_240 = CExoInputEventDesc::vftable;
              local_240[1] = 0;
              local_240[2] = 0;
              local_240[3] = 0;
              local_240 = local_240 + 8;
            }
          }
          *(int *)(&DAT_00a7e9b0 + local_1f8 * 0x20) = iVar6;
          FUN_0072f540(1,0,0,&DAT_00a7e9a0 + local_1f8 * 8);
          *(int *)(param_1 + 0x1b0) = *(int *)(param_1 + 0x1b0) + 1;
        }
      }
    }
    *param_3 = *(undefined4 *)(param_1 + 0x1b0);
    FUN_0072f480(param_1 + 0x1a4);
    local_8 = local_8 & 0xffffff00;
    _eh_vector_destructor_iterator_(local_100,0xc,10,COleDispParams::~COleDispParams);
  }
  else {
    FUN_0073a560(0x5f3,".\\exoinputinternal.cpp",
                 "CExoInputInternal::GetEvents() Invalid InputClass parameter");
    FUN_0072f500(0);
    local_8 = local_8 & 0xffffff00;
    _eh_vector_destructor_iterator_(local_100,0xc,10,COleDispParams::~COleDispParams);
  }
  ExceptionList = local_10;
  return param_2;
}

