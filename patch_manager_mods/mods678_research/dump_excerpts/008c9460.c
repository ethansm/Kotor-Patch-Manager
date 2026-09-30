// FN 008c9460 FUN_008c9460 size=738 ret=4 callers= callees=0051c100,004122a0,0051c8d0,00604ec0,0056df50,005f7a30,0051c0e0,00605e80,0041ffb0,006d0620,00410530

void __thiscall FUN_008c9460(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int local_c;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    if (*(int *)(param_2 + 0x4c) == 0x2d) {
      FUN_004122a0(0);
    }
    if (param_2 == param_1 + 0x8d8) {
      uVar1 = FUN_0041ffb0();
      *(undefined4 *)(param_1 + 0xc90) = uVar1;
      if (*(int *)(param_1 + 0xc90) == 0) {
        return;
      }
    }
    else {
      *(int *)(param_1 + 0xc90) = param_2;
    }
    local_c = FUN_0051c0e0(*(undefined4 *)(*(int *)(param_1 + 0xc90) + 0x1d0));
    *(uint *)(param_1 + 0xc7c) = *(uint *)(param_1 + 0xc7c) & 0xfffffffe;
    if ((*(uint *)(*(int *)(param_1 + 0xc90) + 0x1dc) & 1) == 0) {
      if (*(int *)(local_c + 0x2cc) < 2) {
        iVar2 = local_c;
        FUN_0051c8d0(local_c);
        FUN_005f7a30();
        FUN_00605e80(iVar2);
      }
      else {
        local_c = FUN_00604ec0(1);
      }
    }
    else {
      FUN_0051c100(*(undefined4 *)(*(int *)(param_1 + 0xc90) + 0x1d4));
      if ((*(int *)(*(int *)(param_1 + 0xc90) + 0x1d8) == 0x10) ||
         (*(int *)(*(int *)(param_1 + 0xc90) + 0x1d8) == 0x20)) {
        iVar2 = FUN_006d0620(0x10);
        iVar3 = FUN_006d0620(0x20);
        if (iVar3 == 0) {
          FUN_0056df50(local_c);
        }
        else if (iVar2 == local_c) {
          *(uint *)(param_1 + 0xc7c) = *(uint *)(param_1 + 0xc7c) | 1;
          *(uint *)(param_1 + 0xc7c) = *(uint *)(param_1 + 0xc7c) & 0xfffffffd;
          *(undefined4 *)(param_1 + 0xc80) = *(undefined4 *)(iVar3 + 0x14);
          FUN_0056df50(iVar3);
          FUN_0056df50(local_c);
        }
        else if (iVar3 == local_c) {
          *(uint *)(param_1 + 0xc7c) = *(uint *)(param_1 + 0xc7c) | 1;
          *(uint *)(param_1 + 0xc7c) = *(uint *)(param_1 + 0xc7c) | 2;
          *(undefined4 *)(param_1 + 0xc80) = *(undefined4 *)(iVar2 + 0x14);
          FUN_0056df50(local_c);
          FUN_0056df50(iVar2);
        }
      }
      else {
        FUN_0056df50(local_c);
      }
    }
    *(undefined1 *)(*(int *)(param_1 + 0xc94) + 0x3d2c) = *(undefined1 *)(param_1 + 0xc78);
    *(int *)(*(int *)(param_1 + 0xc94) + 0x3d34) = local_c;
    FUN_00410530(*(undefined4 *)(param_1 + 0xc94),3,1);
  }
  return;
}

