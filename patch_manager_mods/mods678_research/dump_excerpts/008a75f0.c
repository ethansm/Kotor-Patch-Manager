// FN 008a75f0 FUN_008a75f0 size=1444 ret=24 callers=008a7ba0 callees=00848cf0,006d6e30,00418af0,0073fb90,008b1250,0083ea60,0077b670,007e5da0,00919723,0077d800,008b0a60

void __thiscall
FUN_008a75f0(int param_1,int param_2,int *param_3,undefined4 param_4,int param_5,undefined4 param_6,
            undefined4 param_7)

{
  char cVar1;
  short sVar2;
  bool bVar3;
  short sVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  int *local_b8;
  int *local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009780cb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*param_3 == *(int *)(param_1 + 0x24b4)) {
    iVar5 = FUN_00919723(0x3b4,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar5 == 0) {
      local_b8 = (int *)0x0;
    }
    else {
      local_b8 = (int *)FUN_008b0a60();
    }
    local_8 = 0xffffffff;
    local_18 = local_b8;
    FUN_0083ea60(local_b8);
    iVar5 = (**(code **)(**(int **)(param_1 + 0x864) + 0x58))();
    (**(code **)(*local_b8 + 0xa0))
              (iVar5 + 0x160,iVar5 + 0x70,iVar5 + 0xe8,
               *(int *)(param_1 + 0x82c) + (uint)*(byte *)(param_1 + 0x858) * -2,1);
    FUN_00418af0(0,param_1,FUN_008a8100);
    FUN_00418af0(1,param_1,FUN_008a83d0);
  }
  else {
    local_18 = *(int **)(*(int *)(param_1 + 0x24b0) + *param_3 * 4);
  }
  iVar5 = FUN_0077b670(param_5,param_6,0xff);
  if (iVar5 == 0) {
    iVar5 = FUN_006d6e30();
    if (*(int *)(iVar5 + 4) == 0) {
      FUN_00418af0(0x27,param_1,FUN_008a86d0);
      local_18[0x17] = 0;
    }
    else {
      FUN_00418af0(0x27,param_1,FUN_008a8640);
      local_18[0x17] = 0;
    }
  }
  else {
    iVar5 = FUN_006d6e30();
    cVar1 = *(char *)(iVar5 + 0xac);
    if (((cVar1 == '-') || (cVar1 == '\x1a')) &&
       (sVar2 = *(short *)(*(int *)(param_2 + 0x310) + 0x4c), sVar4 = FUN_00848cf0(), sVar4 <= sVar2
       )) {
      iVar5 = FUN_0077d800();
      if ((iVar5 == 0) || (*(int *)(iVar5 + 0x10f4) == 0)) {
        FUN_00418af0(0x27,param_1,FUN_008a8760);
        local_18[0x17] = 0;
      }
    }
    else if ((cVar1 == '/') && (*(short *)(*(int *)(param_2 + 0x310) + 0x24) != DAT_0099a8b8)) {
      sVar2 = *(short *)(*(int *)(param_2 + 0x310) + 0x4c);
      sVar4 = FUN_00848cf0();
      bVar3 = sVar2 < sVar4;
      uVar7 = 1;
      FUN_0073fb90(1);
      iVar5 = FUN_007e5da0(uVar7);
      if (((iVar5 != 0) && (iVar6 = FUN_0077d800(), iVar6 != 0)) &&
         (sVar2 = *(short *)(*(int *)(iVar5 + 0x310) + 0x4c), sVar4 = FUN_00848cf0(), sVar2 < sVar4)
         ) {
        bVar3 = true;
      }
      uVar7 = 2;
      FUN_0073fb90(2);
      iVar5 = FUN_007e5da0(uVar7);
      if (((iVar5 != 0) && (iVar6 = FUN_0077d800(), iVar6 != 0)) &&
         (sVar2 = *(short *)(*(int *)(iVar5 + 0x310) + 0x4c), sVar4 = FUN_00848cf0(), sVar2 < sVar4)
         ) {
        bVar3 = true;
      }
      iVar5 = FUN_0077d800();
      if ((!bVar3) || (iVar5 == 0)) {
        FUN_00418af0(0x27,param_1,FUN_008a87f0);
        local_18[0x17] = 0;
      }
    }
    else if ((cVar1 == '-') && (*(short *)(*(int *)(param_2 + 0x310) + 0x24) == DAT_0099a8b8)) {
      FUN_00418af0(0x27,param_1,FUN_008a86d0);
      local_18[0x17] = 0;
    }
    else if ((cVar1 == '\x1a') && (*(short *)(*(int *)(param_2 + 0x310) + 0x24) != DAT_0099a8b8)) {
      FUN_00418af0(0x27,param_1,FUN_008a86d0);
      local_18[0x17] = 0;
    }
    else {
      FUN_00418af0(0x27,param_1,FUN_008a8480);
      local_18[0x17] = 1;
    }
  }
  FUN_008b1250(*(undefined4 *)(param_5 + 0x14),param_7,*(uint *)(param_5 + 0x2c8) >> 7 & 1);
  (**(code **)(*local_18 + 0x40))(0);
  FUN_0083ea60(local_18);
  *param_3 = *param_3 + 1;
  ExceptionList = local_10;
  return;
}

