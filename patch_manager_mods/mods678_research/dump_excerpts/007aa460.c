// FN 007aa460 FUN_007aa460 size=804 ret=4 callers=00797df0 callees=0073f450,0040e770,0073f4d0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007aa460(int param_1,int param_2)

{
  int iVar1;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  int *local_10;
  int *local_c;
  int local_8;
  
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x214); local_8 = local_8 + 1) {
    local_c = (int *)FUN_0073f4d0(*(undefined4 *)(*(int *)(param_1 + 0x210) + local_8 * 4));
    if (((local_c != (int *)0x0) &&
        (local_10 = (int *)(**(code **)(*local_c + 0xc))(), local_10 != (int *)0x0)) &&
       ((char)local_10[0x24] != '\0')) {
      switch((char)local_10[2]) {
      case '\x05':
        local_14 = (**(code **)(*local_10 + 0x2c))();
        if (param_2 == 0) {
          FUN_0040e770(0);
          (**(code **)(*local_10 + 0xa8))(2);
        }
        else {
          iVar1 = FUN_0073f450();
          if (local_14 == iVar1) {
            local_20 = 0x3f800000;
            local_1c = 0x3f800000;
            local_18 = 0x3f800000;
            (**(code **)(*local_10 + 0xa4))(&local_20,2,0,1);
          }
          else if (*(char *)(local_14 + 0x2f4) == '\x01') {
            local_2c = _DAT_00986b60;
            local_28 = 0x3f800000;
            local_24 = _DAT_00986b60;
            (**(code **)(*local_10 + 0xa4))(&local_2c,2,0,1);
          }
          else if (*(char *)(local_14 + 0x2f4) == '\0') {
            local_38 = _DAT_009875b0;
            local_34 = _DAT_009875b0;
            local_30 = 0x3f800000;
            (**(code **)(*local_10 + 0xa4))(&local_38,2,0,1);
          }
          else {
            local_44 = 0x3f800000;
            local_40 = _DAT_009876fc;
            local_3c = _DAT_009876fc;
            (**(code **)(*local_10 + 0xa4))(&local_44,2,0,1);
          }
        }
        break;
      case '\x06':
        if (param_2 == 0) {
          (**(code **)(*local_10 + 0xa8))(2);
        }
        else {
          local_50 = _DAT_00986b60;
          local_4c = 0x3f800000;
          local_48 = 0x3f800000;
          (**(code **)(*local_10 + 0xa4))(&local_50,2,0,1);
        }
        break;
      case '\t':
        if (param_2 == 0) {
          (**(code **)(*local_10 + 0xa8))(2);
        }
        else {
          local_68 = _DAT_00986b60;
          local_64 = 0x3f800000;
          local_60 = 0x3f800000;
          (**(code **)(*local_10 + 0xa4))(&local_68,2,0,1);
        }
        break;
      case '\n':
        if (param_2 == 0) {
          (**(code **)(*local_10 + 0xa8))(2);
        }
        else {
          local_5c = _DAT_00986b60;
          local_58 = 0x3f800000;
          local_54 = 0x3f800000;
          (**(code **)(*local_10 + 0xa4))(&local_5c,2,0,1);
        }
      }
    }
  }
  return;
}

