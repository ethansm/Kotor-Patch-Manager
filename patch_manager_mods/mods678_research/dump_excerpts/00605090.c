// FN 00605090 FUN_00605090 size=2779 ret=16 callers=008ddf20,008df680,008cdb00,00606460,0065be70,005676d0,00580900,005f8960,005f5770,00580330,008ceb00,006029f0,00573950,0056c1e0,0058aed0,00566650,0057fa40,00606e20,006949f0,005b3380,0059bf80,005f4ed0,005bdb60,0062bde0,008c9750,005f9220,008cfd90 callees=005813c0,0058aed0,00600380,00581630,006d6e30,007356b0,00733bd0,0084a640,0064b760,00733780,0083ea60,00414280,0051c0e0,00733540,0051c620,00604dc0,0051c100,00581710,0051c600,0051c2e0,0051c0a0,006d5200,0073f350,0064ddb0

undefined4 __thiscall
FUN_00605090(undefined4 *param_1,int *param_2,int param_3,undefined4 param_4,int param_5)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int local_5c;
  undefined1 local_44 [20];
  int *local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  undefined4 local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954f28;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_30 = (int *)FUN_0051c0a0(*param_1);
  if (local_30 != (int *)0x0) {
    iVar2 = (**(code **)(*local_30 + 0x30))(uVar1);
    if (iVar2 == 0) {
      iVar2 = (**(code **)(*local_30 + 0x38))();
      if (iVar2 != 0) {
        iVar2 = (**(code **)(*local_30 + 0x38))();
        iVar3 = FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
        if (((*(int *)(iVar3 + 0x70) == 1) && (*(int *)(iVar2 + 0x2a8) != 0x7f000000)) &&
           (iVar2 = FUN_0051c100(*(undefined4 *)(iVar2 + 0x2a8)), iVar2 != 0)) {
          iVar2 = FUN_006d6e30();
          if (*(char *)(iVar2 + 0xac) == '\x17') {
            FUN_005813c0(*(undefined4 *)(*param_2 + 0x2cc),1);
            if ((undefined4 *)*param_2 != (undefined4 *)0x0) {
              (*(code *)**(undefined4 **)*param_2)(1);
            }
            *param_2 = 0;
            ExceptionList = local_10;
            return 1;
          }
          iVar2 = FUN_006d6e30();
          if (*(char *)(iVar2 + 0xac) == '*') {
            FUN_0058aed0(*param_2);
            if ((undefined4 *)*param_2 != (undefined4 *)0x0) {
              (*(code *)**(undefined4 **)*param_2)(1);
            }
            *param_2 = 0;
            ExceptionList = local_10;
            return 1;
          }
        }
      }
    }
    else {
      iVar2 = FUN_006d6e30();
      if (*(char *)(iVar2 + 0xac) == '\x17') {
        uVar4 = *(undefined4 *)(*param_2 + 0x2cc);
        uVar5 = 1;
        (**(code **)(*local_30 + 0x30))(uVar4,1);
        FUN_005813c0(uVar4,uVar5);
        if ((undefined4 *)*param_2 != (undefined4 *)0x0) {
          (*(code *)**(undefined4 **)*param_2)(1);
        }
        *param_2 = 0;
        ExceptionList = local_10;
        return 1;
      }
      iVar2 = FUN_006d6e30();
      if (*(char *)(iVar2 + 0xac) == '2') {
        uVar4 = *(undefined4 *)(*param_2 + 0x2cc);
        uVar5 = 1;
        (**(code **)(*local_30 + 0x30))(uVar4,1);
        FUN_00581710(uVar4,uVar5);
        if ((undefined4 *)*param_2 != (undefined4 *)0x0) {
          (*(code *)**(undefined4 **)*param_2)(1);
        }
        *param_2 = 0;
        ExceptionList = local_10;
        return 1;
      }
      iVar2 = FUN_006d6e30();
      if (*(char *)(iVar2 + 0xac) == '3') {
        uVar4 = *(undefined4 *)(*param_2 + 0x2cc);
        uVar5 = 1;
        (**(code **)(*local_30 + 0x30))(uVar4,1);
        FUN_00581630(uVar4,uVar5);
        if ((undefined4 *)*param_2 != (undefined4 *)0x0) {
          (*(code *)**(undefined4 **)*param_2)(1);
        }
        *param_2 = 0;
        ExceptionList = local_10;
        return 1;
      }
      iVar2 = FUN_006d6e30();
      if (*(char *)(iVar2 + 0xac) == '*') {
        iVar2 = *param_2;
        (**(code **)(*local_30 + 0x30))(iVar2);
        FUN_0058aed0(iVar2);
        if ((undefined4 *)*param_2 != (undefined4 *)0x0) {
          (*(code *)**(undefined4 **)*param_2)(1);
        }
        *param_2 = 0;
        ExceptionList = local_10;
        return 1;
      }
    }
  }
  local_28 = 0;
  local_2c = 0;
  local_1c = FUN_0073f350();
  FUN_00733540();
  local_8 = 0;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,1);
  if ((param_1[2] & 1) != 0) {
    FUN_007356b0(local_1c,local_18,0);
  }
  if (param_3 == 1) {
    for (local_20 = 0; local_20 < (int)param_1[4]; local_20 = local_20 + 1) {
      local_24 = FUN_0051c0e0(*(undefined4 *)(param_1[3] + local_20 * 4));
      if (local_24 != 0) {
        if (((param_1[2] & 1) != 0) && (local_2c == 0)) {
          FUN_007356b0(local_1c,local_44,0);
          if ((*(uint *)(*param_2 + 0xc) < *(uint *)(local_24 + 0xc)) ||
             ((*(uint *)(*param_2 + 0xc) == *(uint *)(local_24 + 0xc) &&
              (iVar2 = FUN_00733bd0(local_44), iVar2 != 0)))) {
            local_28 = local_20;
            local_2c = 1;
          }
        }
        if (((*(int *)(local_24 + 0xc) == *(int *)(*param_2 + 0xc)) &&
            (*(char *)(local_24 + 0x2a4) == *(char *)(*param_2 + 0x2a4))) &&
           (iVar2 = FUN_00604dc0(*param_2), iVar2 == 1)) {
          local_5c = 0;
          if (local_30 != (int *)0x0) {
            if ((char)local_30[2] == DAT_00994474) {
              local_5c = FUN_0051c600(*param_1);
            }
            else if ((char)local_30[2] == DAT_00994475) {
              iVar2 = (**(code **)(*local_30 + 0x38))();
              local_5c = FUN_0051c600(*(undefined4 *)(iVar2 + 0x2a8));
            }
            if (local_5c != 0) {
              uVar4 = *(undefined4 *)(*param_2 + 0x14);
              FUN_0051c2e0(local_5c,uVar4);
              FUN_0064b760(local_5c,uVar4);
            }
          }
          if ((undefined4 *)*param_2 == (undefined4 *)0x0) {
            uVar4 = 0;
          }
          else {
            uVar4 = (*(code *)**(undefined4 **)*param_2)(1);
          }
          *param_2 = local_24;
          FUN_00600380(*(undefined4 *)(local_24 + 0x2a8),param_4,param_4,0x7f000000);
          if (param_5 != 0) {
            param_1[1] = param_1[1] + 1;
          }
          local_8 = local_8 & 0xffffff00;
          FUN_00733780(uVar1,uVar4);
          local_8 = 0xffffffff;
          FUN_00733780();
          ExceptionList = local_10;
          return 1;
        }
      }
    }
  }
  else if ((param_1[2] & 1) != 0) {
    for (local_20 = 0; local_20 < (int)param_1[4]; local_20 = local_20 + 1) {
      local_24 = FUN_0051c0e0(*(undefined4 *)(param_1[3] + local_20 * 4));
      if (((local_24 != 0) && ((param_1[2] & 1) != 0)) && (local_2c == 0)) {
        FUN_007356b0(local_1c,local_44,0);
        iVar2 = FUN_00733bd0(local_44);
        if (iVar2 != 0) {
          local_28 = local_20;
          local_2c = 1;
          break;
        }
      }
    }
  }
  FUN_0084a640(0);
  local_8 = CONCAT31(local_8._1_3_,2);
  FUN_0083ea60(*param_2);
  if ((param_1[2] & 1) == 0) {
    FUN_00414280(*(undefined4 *)(*param_2 + 0x14),0);
  }
  else {
    iVar2 = FUN_0051c620(0,1);
    if (iVar2 != 0) {
      FUN_00600380(*(undefined4 *)(iVar2 + 0x38),param_4,param_4,0x7f000000);
    }
    if (local_2c == 0) {
      FUN_0083ea60(*(undefined4 *)(*param_2 + 0x14));
    }
    else {
      FUN_00414280(*(undefined4 *)(*param_2 + 0x14),local_28);
    }
  }
  if (param_5 != 0) {
    *(uint *)(*param_2 + 0x2c8) = *(uint *)(*param_2 + 0x2c8) & 0xffffff7f | 0x80;
  }
  if ((*(uint *)(*param_2 + 0x2c8) >> 7 & 1) != 0) {
    param_1[1] = param_1[1] + 1;
  }
  local_8._0_1_ = 1;
  Free();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return 1;
}

