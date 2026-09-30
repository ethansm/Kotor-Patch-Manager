// FN 0074cfe0 FUN_0074cfe0 size=1102 ret=4 callers=007b3ca0,0074da40,0074b3f0,007ceb50 callees=0074cc90,00746950,0074da40,0077d800,0073f4d0,007ecfa0,0077e650,007469a0,0073fb90,00743540,007e5d80,007418a0,00741660

void __fastcall FUN_0074cfe0(int param_1)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  undefined4 local_6c;
  undefined4 local_68;
  int *local_64;
  int local_24;
  undefined4 local_18;
  int local_10;
  int local_8;
  
  FUN_0073fb90();
  iVar3 = FUN_007e5d80();
  if (iVar3 == 0) {
    local_64 = (int *)0x0;
  }
  else {
    local_64 = (int *)FUN_0077d800();
  }
  piVar4 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x68));
  local_18 = 0;
  local_10 = 1;
  if (iVar3 != 0) {
    if ((local_64 != (int *)0x0) && (iVar5 = (**(code **)(*local_64 + 0x94))(), iVar5 == 0)) {
      if (local_64[0x46b] == 0) {
        bVar1 = false;
      }
      else {
        sVar2 = (**(code **)(*local_64 + 0x9c))(0);
        bVar1 = sVar2 < 1;
      }
      if (!bVar1) {
        local_10 = 0;
      }
    }
    if (*(int *)(iVar3 + 4) != *(int *)(param_1 + 0x6c)) {
      *(undefined4 *)(param_1 + 0x6c) = *(undefined4 *)(iVar3 + 4);
      FUN_0074da40();
    }
    if (piVar4 != (int *)0x0) {
      local_18 = (**(code **)(*piVar4 + 0xc))();
    }
    for (local_8 = 0; local_8 < 7; local_8 = local_8 + 1) {
      for (local_24 = 0; local_24 < *(int *)(param_1 + 0x7c + local_8 * 0xc);
          local_24 = local_24 + 1) {
        FUN_007ecfa0();
      }
      *(undefined4 *)(param_1 + 0x7c + local_8 * 0xc) = 0;
      FUN_0077e650(local_8,param_1 + 0x78 + local_8 * 0xc);
    }
    FUN_00743540(iVar3,local_10,local_18);
    for (local_8 = 0; local_8 < 6; local_8 = local_8 + 1) {
      bVar1 = false;
      for (local_24 = 0; local_24 < *(int *)(param_1 + 0x7c + local_8 * 0xc);
          local_24 = local_24 + 1) {
        if (*(int *)(*(int *)(param_1 + 0x78 + local_8 * 0xc) + 8 + local_24 * 0x3c) ==
            *(int *)(param_1 + 0x1c7c + local_8 * 4)) {
          FUN_00741660(0,1 < *(int *)(param_1 + 0x7c + local_8 * 0xc));
          if (((*(uint *)(*(int *)(param_1 + 0x78 + local_8 * 0xc) + 0x30 + local_24 * 0x3c) & 1) ==
               0) || (local_10 != 0)) {
            local_68 = 0;
          }
          else {
            local_68 = 1;
          }
          FUN_007418a0(*(int *)(param_1 + 0x78 + local_8 * 0xc) + 0x20 + local_24 * 0x3c,
                       local_24 * 0x3c + *(int *)(param_1 + 0x78 + local_8 * 0xc),local_68,0);
          bVar1 = true;
          break;
        }
      }
      if (!bVar1) {
        if (*(int *)(param_1 + 0x7c + local_8 * 0xc) < 1) {
          FUN_00741660(1,0);
        }
        else {
          FUN_00741660(0,1 < *(int *)(param_1 + 0x7c + local_8 * 0xc));
          if (((*(uint *)(*(int *)(param_1 + 0x78 + local_8 * 0xc) + 0x30) & 1) == 0) ||
             (local_10 != 0)) {
            local_6c = 0;
          }
          else {
            local_6c = 1;
          }
          FUN_007418a0(*(int *)(param_1 + 0x78 + local_8 * 0xc) + 0x20,
                       *(undefined4 *)(param_1 + 0x78 + local_8 * 0xc),local_6c,0);
        }
      }
    }
    FUN_0074cc90();
    if ((-1 < DAT_00a10340) &&
       (iVar3 = FUN_007469a0(DAT_00a10340), *(char *)(iVar3 + 0x74d) != '\0')) {
      FUN_00746950();
    }
  }
  return;
}

