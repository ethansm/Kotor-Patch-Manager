// FN 0040a750 FUN_0040a750 size=4787 ret=16 callers= callees=0000012b,00919723,0073fe20,000000b2,00730260,0000012a,0040bcf0,0072fbd0,0040c0e0,0072fb00,0072fee0,0073fe40,00740f60,00740160,0040bc40,0040c100,0072fc30,0073f2f0,0040c090,0040a160,0040a460,00000110,00000139,0073fce0,0040bf40,0073f370,0000011d,0000012c,0072fa60,0040bdc0,0040c150,0073f390

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

LRESULT FUN_0040a750(HWND param_1,uint param_2,HWND param_3,uint param_4)

{
  undefined4 *puVar1;
  DWORD DVar2;
  int iVar3;
  uint *puVar4;
  LRESULT LVar5;
  int local_144;
  undefined1 local_128 [4];
  uint local_124;
  HWND local_120;
  uint local_11c;
  undefined1 local_10c [4];
  uint local_108;
  HWND local_104;
  uint local_100;
  undefined1 local_f0 [4];
  uint local_ec;
  HWND local_e8;
  uint local_e4;
  tagPOINT local_d4;
  undefined1 local_cc [4];
  uint local_c8;
  HWND local_c4;
  uint local_c0;
  undefined1 local_b0 [4];
  uint local_ac;
  HWND local_a8;
  uint local_a4;
  undefined1 local_94 [4];
  uint local_90;
  HWND local_8c;
  uint local_88;
  undefined1 local_78 [4];
  uint local_74;
  HWND local_70;
  uint local_6c;
  undefined1 local_5c [4];
  uint local_58;
  HWND local_54;
  uint local_50;
  undefined1 local_40 [4];
  uint local_3c;
  HWND local_38;
  uint local_34;
  undefined1 local_24 [4];
  uint local_20;
  HWND local_1c;
  uint local_18;
  HWND local_8;
  
  if (param_2 < 0x101) {
    if (param_2 == 0x100) {
      if ((((param_3 == (HWND)0x1b) && (DAT_00a1b4a4 != 0)) && (*(int *)(DAT_00a1b4a4 + 4) != 0)) &&
         (iVar3 = FUN_00740160(), iVar3 == 1)) {
        FUN_00740f60();
      }
      iVar3 = FUN_0073f2f0();
      if ((iVar3 == 0) && (iVar3 = FUN_0040a160(param_1,param_3,param_4), iVar3 == 1)) {
        return 0;
      }
      goto switchD_0040a7a7_caseD_6;
    }
    switch(param_2) {
    case 5:
      if (param_1 == DAT_00a1b484) {
        if (param_3 == (HWND)0x2) {
          DAT_00a1b4d4 = 2;
          return 0;
        }
        LVar5 = DefWindowProcA(DAT_00a1b484,param_2,(WPARAM)param_3,param_4);
        return LVar5;
      }
      break;
    case 7:
      if (DAT_009f42a0 == 1) {
        FUN_0073fe40(1,0);
      }
      break;
    case 8:
      iVar3 = FUN_00740160();
      if (iVar3 == 0) {
        FUN_0073fe40(0,0);
      }
      DAT_009f42a0 = (uint)(iVar3 == 0);
      break;
    case 0x10:
      FUN_0073fe20();
      return 0;
    case 0x1c:
      if (DAT_00a1b48c != 0) {
        if (param_3 == (HWND)0x0) {
          DAT_00a1b4bc = 0;
          DAT_00a1b4c0 = 0;
          local_144 = 0;
          if (((DAT_00a1b4a4 != 0) && (*(int *)(DAT_00a1b4a4 + 4) != 0)) &&
             ((DAT_00a1b4a0 != (int *)0x0 && (local_144 = FUN_00740160(), local_144 == 1)))) {
            iVar3 = (**(code **)(*DAT_00a1b4a0 + 4))();
            if (iVar3 != 0) {
              DAT_00a1b4bc = 1;
            }
            DAT_00a1b4c0 = 1;
          }
          if (((DAT_00a1b4bc == 0) && (DAT_00a1b4a0 != (int *)0x0)) &&
             (iVar3 = (**(code **)(*DAT_00a1b4a0 + 4))(), iVar3 == 1)) {
            ShowWindow(DAT_00a1b484,6);
            (**(code **)(*DAT_00a1b4a0 + 0x18))();
          }
          if (param_1 == DAT_00a1b484) {
            FUN_0040bc40(local_144);
            _DAT_00a1b6f4 = GetTickCount();
          }
          if (DAT_00a1b4c0 == 0) {
            FUN_0040c0e0();
            FUN_0040c150();
          }
        }
        else {
          if ((DAT_00a1b4bc == 0) && (DAT_00a1b4a0 != (int *)0x0)) {
            iVar3 = (**(code **)(*DAT_00a1b4a0 + 4))();
            if (iVar3 == 0) {
              if (((DAT_00a1b4c0 == 0) && (iVar3 = FUN_00740160(), iVar3 == 0)) &&
                 (DVar2 = GetTickCount(), 100 < DVar2 - _DAT_00a1b6f4)) {
                FUN_0073fce0();
              }
            }
            else if (DAT_00a1b4d4 == 0) {
              if (param_1 == DAT_00a1b484) {
                (**(code **)(*DAT_00a1b4a0 + 0x1c))();
              }
              FUN_0040bcf0();
            }
          }
          if ((param_1 == DAT_00a1b484) &&
             ((DVar2 = GetTickCount(), DVar2 - _DAT_00a1b6f4 < 1000 ||
              (iVar3 = (**(code **)(*DAT_00a1b4a0 + 4))(), iVar3 == 0)))) {
            FUN_0040bcf0();
          }
          if (DAT_00a1b4c0 == 0) {
            FUN_0040c090();
            FUN_0040c100();
          }
        }
      }
      break;
    case 0x1f:
      if (DAT_00a7fcbc != 0) {
        DAT_00a7fcbc = 0;
      }
      break;
    case 0x20:
      local_8 = param_3;
      if (((param_3 == DAT_00a1b484) && (DAT_00a1b48c != 0)) && ((param_4 & 0xffff) == 1)) {
        FUN_0072fc30(0);
        return 1;
      }
      break;
    case 0x46:
      iVar3 = FUN_00740160();
      if (iVar3 == 1) {
        puVar1 = (undefined4 *)FUN_00919723(0x1c);
        *puVar1 = DAT_00a1b9e8;
        puVar1[1] = 0;
        puVar1[4] = *(undefined4 *)(param_4 + 0x10);
        puVar1[5] = *(undefined4 *)(param_4 + 0x14);
        puVar1[2] = *(undefined4 *)(param_4 + 8);
        puVar1[3] = *(undefined4 *)(param_4 + 0xc);
        puVar1[6] = 0;
        PostMessageA(DAT_00a1b9e8,0x407,(WPARAM)param_3,(LPARAM)puVar1);
      }
      break;
    case 0x47:
      iVar3 = FUN_00740160();
      if (iVar3 == 1) {
        puVar1 = (undefined4 *)FUN_00919723(0x1c);
        *puVar1 = DAT_00a1b9e8;
        puVar1[1] = 0;
        puVar1[4] = *(undefined4 *)(param_4 + 0x10);
        puVar1[5] = *(undefined4 *)(param_4 + 0x14);
        puVar1[2] = *(undefined4 *)(param_4 + 8);
        puVar1[3] = *(undefined4 *)(param_4 + 0xc);
        puVar1[6] = 0;
        PostMessageA(DAT_00a1b9e8,0x408,(WPARAM)param_3,(LPARAM)puVar1);
      }
      break;
    case 0x51:
switchD_0040a7a7_caseD_51:
      iVar3 = FUN_0073f370();
      if (iVar3 != 0) {
        return 0;
      }
    }
  }
  else {
    if (param_2 < 0x201) {
      if (param_2 == 0x200) {
        if ((DAT_00a1b48c != 0) && (iVar3 = FUN_0072fa60(), iVar3 != 0)) {
          local_20 = param_2;
          local_1c = param_3;
          local_18 = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
          FUN_0072fbd0(local_24);
        }
      }
      else {
        switch(param_2) {
        case 0x102:
          iVar3 = FUN_0073f2f0();
          if ((iVar3 == 0) && (iVar3 = FUN_0040a460((uint)param_3 & 0xffff), iVar3 == 1)) {
            return 0;
          }
          break;
        case 0x104:
          switch(param_3) {
          case (HWND)0xd:
            if ((param_4 & 0x20000000) != 0) {
              return 0;
            }
            break;
          case (HWND)0x12:
            FUN_0073fe40(0,0);
            return 0;
          case (HWND)0x73:
            if ((param_4 & 0x20000000) != 0) {
              FUN_0073fe40(1,0);
            }
            SendMessageA(param_1,0x10,(WPARAM)param_3,param_4);
            break;
          case (HWND)0x79:
            return 0;
          }
          break;
        case 0x105:
          if (param_3 == (HWND)0xd) {
            if ((param_4 & 0x20000000) != 0) {
              FUN_0040bdc0(0,0);
            }
          }
          else {
            if (param_3 == (HWND)0x12) {
              FUN_0073fe40(1,0);
              return 0;
            }
            if (param_3 == (HWND)0x79) {
              return 0;
            }
            if ((param_4 & 0x20000000) != 0) {
              FUN_0073fe40(1,0);
            }
          }
          break;
        case 0x106:
          if ((param_3 == (HWND)0xd) && ((param_4 & 0x20000000) != 0)) {
            return 0;
          }
          break;
        case 0x10d:
          iVar3 = FUN_0073f370();
          if (iVar3 != 0) {
            puVar4 = (uint *)FUN_0073f390();
            *puVar4 = *puVar4 | 1;
            return 0;
          }
          break;
        case 0x10e:
          iVar3 = FUN_0073f370();
          if (iVar3 != 0) {
            puVar4 = (uint *)FUN_0073f390();
            *puVar4 = *puVar4 & 0xfffffffe;
            return 0;
          }
          break;
        case 0x10f:
          iVar3 = FUN_0073f370();
          if (iVar3 != 0) {
            FUN_0073f390(param_1,param_3,param_4);
            FUN_0072fee0(param_1,param_3,param_4);
            return 0;
          }
          break;
        case 0x112:
          if (((uint)param_3 & 0xfffffff0) == 0xf000) {
            return 0;
          }
          if ((((uint)param_3 & 0xffff) == 0xf140) || (((uint)param_3 & 0xffff) == 0xf170)) {
            return 0;
          }
        }
      }
      goto switchD_0040a7a7_caseD_6;
    }
    if (0x409 < param_2) {
      if (param_2 == 0x40a) {
        FUN_0040bf40();
        return 0;
      }
      goto switchD_0040a7a7_caseD_6;
    }
    if (param_2 == 0x409) {
      FUN_0040bdc0(1,param_3 != (HWND)0x0);
      return 0;
    }
    switch(param_2) {
    case 0x201:
      if ((DAT_00a1b48c == 0) || (iVar3 = FUN_0072fa60(), iVar3 == 0)) {
        return 0;
      }
      local_3c = param_2;
      iVar3 = FUN_0072fb00();
      if ((iVar3 != 0) && (iVar3 = FUN_0072fb00(), *(int *)(iVar3 + 0x5c) != 0)) {
        local_3c = 0x204;
        param_2 = 0x204;
      }
      local_38 = param_3;
      local_34 = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
      FUN_0072fbd0(local_40);
      SetCapture(param_1);
      break;
    case 0x202:
      if ((DAT_00a1b48c == 0) || (iVar3 = FUN_0072fa60(), iVar3 == 0)) {
        return 0;
      }
      local_74 = param_2;
      iVar3 = FUN_0072fb00();
      if ((iVar3 != 0) && (iVar3 = FUN_0072fb00(), *(int *)(iVar3 + 0x5c) != 0)) {
        local_74 = 0x205;
        param_2 = 0x205;
      }
      local_70 = param_3;
      local_6c = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
      FUN_0072fbd0(local_78);
      break;
    case 0x203:
      if ((DAT_00a1b48c == 0) || (iVar3 = FUN_0072fa60(), iVar3 == 0)) {
        return 0;
      }
      local_58 = param_2;
      iVar3 = FUN_0072fb00();
      if ((iVar3 != 0) && (iVar3 = FUN_0072fb00(), *(int *)(iVar3 + 0x5c) != 0)) {
        local_58 = 0x206;
        param_2 = 0x206;
      }
      local_54 = param_3;
      local_50 = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
      FUN_0072fbd0(local_5c);
      break;
    case 0x204:
      if ((DAT_00a1b48c != 0) && (iVar3 = FUN_0072fa60(), iVar3 != 0)) {
        local_ac = param_2;
        iVar3 = FUN_0072fb00();
        if ((iVar3 != 0) && (iVar3 = FUN_0072fb00(), *(int *)(iVar3 + 0x5c) != 0)) {
          local_ac = 0x201;
          param_2 = 0x201;
        }
        local_a8 = param_3;
        local_a4 = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
        FUN_0072fbd0(local_b0);
        SetCapture(param_1);
      }
      break;
    case 0x205:
      if ((DAT_00a1b48c != 0) && (iVar3 = FUN_0072fa60(), iVar3 != 0)) {
        local_c8 = param_2;
        iVar3 = FUN_0072fb00();
        if ((iVar3 != 0) && (iVar3 = FUN_0072fb00(), *(int *)(iVar3 + 0x5c) != 0)) {
          local_c8 = 0x202;
          param_2 = 0x202;
        }
        local_c4 = param_3;
        local_c0 = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
        FUN_0072fbd0(local_cc);
      }
      break;
    case 0x206:
      if ((DAT_00a1b48c == 0) || (iVar3 = FUN_0072fa60(), iVar3 == 0)) {
        return 0;
      }
      local_90 = param_2;
      iVar3 = FUN_0072fb00();
      if ((iVar3 != 0) && (iVar3 = FUN_0072fb00(), *(int *)(iVar3 + 0x5c) != 0)) {
        local_90 = 0x203;
        param_2 = 0x203;
      }
      local_8c = param_3;
      local_88 = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
      FUN_0072fbd0(local_94);
      SetCapture(param_1);
      break;
    case 0x207:
      if ((DAT_00a1b48c != 0) && (iVar3 = FUN_0072fa60(), iVar3 != 0)) {
        local_108 = param_2;
        local_104 = param_3;
        local_100 = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
        FUN_0072fbd0(local_10c);
        SetCapture(param_1);
      }
      break;
    case 0x208:
      if ((DAT_00a1b48c != 0) && (iVar3 = FUN_0072fa60(), iVar3 != 0)) {
        local_124 = param_2;
        local_120 = param_3;
        local_11c = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
        FUN_0072fbd0(local_128);
      }
      break;
    case 0x20a:
      if ((DAT_00a1b48c != 0) && (iVar3 = FUN_0072fa60(), iVar3 != 0)) {
        local_ec = param_2;
        local_e8 = param_3;
        local_d4.x = param_4 & 0xffff;
        local_d4.y = param_4 >> 0x10;
        ScreenToClient(param_1,&local_d4);
        local_e4 = local_d4.x & 0xffffU | ((DAT_009f42a8 + -1) - (int)(short)local_d4.y) * 0x10000;
        FUN_0072fbd0(local_f0);
      }
      break;
    case 0x281:
    case 0x283:
    case 0x284:
    case 0x290:
      goto switchD_0040a7a7_caseD_51;
    case 0x282:
      iVar3 = FUN_0073f370();
      if (iVar3 != 0) {
        FUN_0073f390(param_1,param_3,param_4);
        FUN_00730260(param_1,param_3,param_4);
        return 0;
      }
    }
  }
switchD_0040a7a7_caseD_6:
  LVar5 = DefWindowProcA(param_1,param_2,(WPARAM)param_3,param_4);
  return LVar5;
}

