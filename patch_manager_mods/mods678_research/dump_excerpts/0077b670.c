// FN 0077b670 FUN_0077b670 size=1291 ret=12 callers=0077bcf0,008a8df0,008d34e0,008a75f0 callees=006c1450,007e8830,005d2f80,0073fb90,006d0540,006d6e30,006042e0,0077d800,007aae00,006b7a00

undefined4 FUN_0077b670(int param_1,undefined4 param_2,byte param_3)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  int local_c;
  uint local_8;
  
  if (param_1 != 0) {
    iVar2 = FUN_0077d800();
    iVar3 = FUN_007aae00();
    if (iVar2 == 0) {
      return 0;
    }
    if (*(int *)(param_1 + 0x10c) == 0) {
      if (*(int *)(iVar3 + 0x2e4) == 0) {
        if (((param_3 & 2) != 0) && (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '\x1c')) {
          iVar2 = FUN_006042e0(&local_c,0x2e,0);
          if ((iVar2 != 0) &&
             ((*(char *)(local_c + 0xc) == '\0' || (iVar2 = FUN_006b7a00(1), iVar2 == 0)))) {
            return 0;
          }
          uVar4 = FUN_005d2f80(param_1,1,1);
          return uVar4;
        }
        if (((param_3 & 1) != 0) &&
           ((((iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '\x19' ||
              (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '-')) ||
             (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '/')) ||
            (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '\x1a')))) {
          iVar2 = FUN_006042e0(&local_c,10,0);
          if ((iVar2 != 0) && (*(char *)(local_c + 0xc) == '\0')) {
            return 0;
          }
          iVar2 = FUN_006d6e30();
          if (*(char *)(iVar2 + 0xac) != '\x1a') {
            uVar5 = (uint)DAT_0099a8bc;
            FUN_0073fb90(uVar5);
            iVar2 = FUN_007e8830(uVar5);
            if (iVar2 == 0) {
              return 0;
            }
            return 1;
          }
          uVar5 = (uint)DAT_0099a8b8;
          FUN_0073fb90(uVar5);
          iVar2 = FUN_007e8830(uVar5);
          if (iVar2 == 0) {
            return 0;
          }
          return 1;
        }
        iVar2 = FUN_006042e0(&local_c,10,0);
        if (iVar2 != 0) {
          if (*(char *)(local_c + 0xc) == '\0') {
            return 0;
          }
          iVar2 = FUN_006d6e30();
          if (iVar2 != 0) {
            bVar1 = *(byte *)(iVar2 + 0xac);
            if ((bVar1 < 0xc) || ((0xd < bVar1 && (bVar1 != 0x14)))) {
              if ((param_3 & 4) == 0) {
                return 0;
              }
            }
            else {
              iVar2 = FUN_006d0540(param_1);
              if ((iVar2 == 0) || ((param_3 & 4) == 0)) {
                return 0;
              }
            }
          }
          local_8 = (uint)*(ushort *)(local_c + 2);
          iVar2 = FUN_006c1450(local_8);
          if ((iVar2 != 0) && (*(char *)(iVar2 + 0x1a4) == '\x01')) {
            uVar4 = FUN_005d2f80(param_1,1,1);
            return uVar4;
          }
        }
      }
    }
    else {
      if (((param_3 & 2) != 0) && (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '\x1c')) {
        iVar2 = FUN_006042e0(&local_c,0x2e,0);
        if ((iVar2 != 0) &&
           ((*(char *)(local_c + 0xc) == '\0' || (iVar2 = FUN_006b7a00(1), iVar2 == 0)))) {
          return 0;
        }
        uVar4 = FUN_005d2f80(param_1,1,1);
        return uVar4;
      }
      iVar2 = FUN_006042e0(&local_c,10,0);
      if (iVar2 != 0) {
        iVar2 = FUN_006d6e30();
        if (iVar2 != 0) {
          bVar1 = *(byte *)(iVar2 + 0xac);
          if ((bVar1 < 0xc) || ((0xd < bVar1 && (bVar1 != 0x14)))) {
            if ((param_3 & 4) == 0) {
              return 0;
            }
          }
          else {
            iVar2 = FUN_006d0540(param_1);
            if ((iVar2 == 0) || ((param_3 & 4) == 0)) {
              return 0;
            }
          }
        }
        local_8 = (uint)*(ushort *)(local_c + 2);
        iVar2 = FUN_006c1450(local_8);
        if ((iVar2 != 0) && (*(char *)(iVar2 + 0x1a4) == '\x01')) {
          uVar4 = FUN_005d2f80(param_1,1,1);
          return uVar4;
        }
      }
    }
  }
  return 0;
}

