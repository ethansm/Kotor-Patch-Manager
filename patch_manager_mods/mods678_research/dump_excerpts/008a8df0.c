// FN 008a8df0 FUN_008a8df0 size=913 ret=12 callers=008a7ba0 callees=005d2f80,0073f750,006d6e30,0077b670,0058ae90

uint FUN_008a8df0(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  uint local_7c;
  uint local_78;
  uint local_8;
  
  local_8 = 0;
  if (param_1 == 0) {
    local_8 = 0;
  }
  else {
    iVar1 = FUN_0073f750();
    switch(*(undefined1 *)(iVar1 + 0xfc1)) {
    case 0:
      local_8 = 1;
      break;
    case 1:
      local_8 = *(uint *)(param_1 + 0x2c8) >> 7 & 1;
      break;
    case 2:
      local_8 = *(uint *)(param_1 + 0x10c);
      break;
    case 3:
      iVar1 = FUN_006d6e30();
      if (iVar1 != 0) {
        iVar1 = FUN_006d6e30();
        local_8 = (uint)(*(int *)(iVar1 + 4) != 0);
      }
      break;
    case 4:
      iVar1 = FUN_006d6e30();
      if (iVar1 != 0) {
        if ((*(int *)(param_1 + 0x10c) == 0) && (iVar1 = FUN_006d6e30(), *(int *)(iVar1 + 4) == 0))
        {
          local_78 = 1;
          local_8 = local_78;
        }
        else {
          local_78 = 0;
          local_8 = local_78;
        }
      }
      break;
    case 5:
      iVar1 = FUN_0058ae90();
      if (iVar1 != 0) {
        uVar2 = 0xff;
        iVar1 = param_1;
        FUN_0058ae90(param_1,param_3,0xff);
        iVar1 = FUN_0077b670(iVar1,param_3,uVar2);
        if ((iVar1 == 0) || (iVar1 = FUN_005d2f80(param_1,0,1), iVar1 == 0)) {
          local_7c = 0;
          local_8 = local_7c;
        }
        else {
          local_7c = 1;
          local_8 = local_7c;
        }
      }
      break;
    case 6:
      iVar1 = FUN_006d6e30();
      if (iVar1 != 0) {
        iVar1 = FUN_006d6e30();
        local_8 = (uint)(*(char *)(iVar1 + 0xac) == '\x18');
      }
      break;
    case 7:
      iVar1 = FUN_006d6e30();
      if (iVar1 != 0) {
        iVar1 = FUN_006d6e30();
        local_8 = (uint)((*(uint *)(iVar1 + 4) & 0x30) != 0);
        if (((local_8 == 0) && (iVar1 = FUN_006d6e30(), (*(uint *)(iVar1 + 4) & 0x180) != 0)) &&
           (iVar1 = FUN_006d6e30(), *(char *)(iVar1 + 0xb4) == '\x02')) {
          local_8 = 1;
        }
      }
      break;
    case 8:
      iVar1 = FUN_006d6e30();
      if (iVar1 != 0) {
        iVar1 = FUN_006d6e30();
        local_8 = (uint)((*(uint *)(iVar1 + 4) & 0x78b) != 0);
        if (((local_8 != 0) && (iVar1 = FUN_006d6e30(), (*(uint *)(iVar1 + 4) & 0x180) != 0)) &&
           (iVar1 = FUN_006d6e30(), *(char *)(iVar1 + 0xb4) == '\x02')) {
          local_8 = 0;
        }
      }
      break;
    case 9:
      iVar1 = FUN_0058ae90();
      if (iVar1 != 0) {
        if ((*(int *)(param_1 + 0xc) < 0x35) || (0x38 < *(int *)(param_1 + 0xc))) {
          local_8 = 0;
        }
        else {
          local_8 = 1;
        }
      }
      break;
    case 10:
      local_8 = *(uint *)(param_1 + 0x10c);
      break;
    case 0xb:
      local_8 = 1;
      iVar1 = FUN_006d6e30();
      if (iVar1 != 0) {
        iVar1 = FUN_006d6e30();
        local_8 = (uint)(*(char *)(iVar1 + 0xac) != '\x18');
        iVar1 = FUN_006d6e30();
        if (*(int *)(iVar1 + 4) != 0) {
          local_8 = 0;
        }
      }
      iVar1 = FUN_0058ae90();
      if (((iVar1 != 0) && (0x34 < *(int *)(param_1 + 0xc))) && (*(int *)(param_1 + 0xc) < 0x39)) {
        local_8 = 0;
      }
    }
  }
  return local_8;
}

