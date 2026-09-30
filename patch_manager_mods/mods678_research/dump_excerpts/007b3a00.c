// FN 007b3a00 FUN_007b3a00 size=667 ret=12 callers=007ac470 callees=0072fe80,007b40f0,007b3870,007b3ee0,0073f470,007a17a0

void __thiscall FUN_007b3a00(int param_1,undefined4 param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 local_18;
  int local_10;
  
  *(undefined4 *)(param_1 + 0x4b4) = *param_3;
  *(undefined4 *)(param_1 + 0x4b8) = param_3[1];
  *(undefined4 *)(param_1 + 0x4bc) = param_3[2];
  *(undefined4 *)(param_1 + 0x4b0) = param_2;
  if ((*(int *)(param_1 + 0x40) != 0) &&
     (((*(int *)(*(int *)(param_1 + 0x40) + 0x30) != 0 ||
       (*(int *)(*(int *)(param_1 + 0x40) + 0xcc) == 1)) ||
      (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05')))) {
    FUN_007b3870(1,0,0);
    return;
  }
  iVar1 = *(int *)(DAT_00a1b49c + 8);
  if ((param_4 == 0) &&
     (((*(int *)(param_1 + 0x4b0) == 0x7f000000 || (*(int *)(param_1 + 0x2b4) == 0x7f000000)) ||
      ((iVar4 = *(int *)(param_1 + 0x4b0), iVar3 = FUN_0073f470(), iVar4 == iVar3 ||
       ((iVar1 != 0 || (*(int *)(param_1 + 0x2b4) != *(int *)(param_1 + 0x4b0))))))))) {
    if (((*(int *)(param_1 + 0x4b0) != 0x7f000000) &&
        (iVar4 = FUN_007a17a0(*(undefined4 *)(param_1 + 0x4b0),FUN_0079b6e0), iVar4 != -1)) &&
       (iVar1 == 0)) {
      FUN_007b3870(0x2d,0,0);
      return;
    }
    uVar5 = FUN_0072fe80();
    FUN_007b3870(1,0,uVar5);
  }
  else {
    local_18 = 1;
    local_10 = 0;
    FUN_007b40f0();
    uVar5 = DAT_00a7fcc0;
    bVar2 = false;
    if (0 < *(int *)(param_1 + 0x4d0)) {
      local_10 = *(int *)(param_1 + 0x4cc);
      bVar2 = 1 < *(int *)(param_1 + 0x4d0);
    }
    if ((*(int *)(param_1 + 0x4b0) != 0x7f000000) &&
       (*(int *)(param_1 + 0x4b0) != *(int *)(*(int *)(*(int *)(param_1 + 0x40) + 0x98) + 0x68))) {
      FUN_007b3870(0x2d,0,0);
      return;
    }
    if (local_10 != 0) {
      local_18 = FUN_007b3ee0(*(undefined4 *)(local_10 + 8),bVar2);
    }
    FUN_007b3870(local_18,0,uVar5);
  }
  return;
}

