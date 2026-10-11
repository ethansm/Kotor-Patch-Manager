// FN 006a4e00 FUN_006a4e00 size=540 ret=4 callers=006bedf0,005db8f0,0057e8c0,005b43c0,00661c30,008c0bd0,005bd6a0,006df2a0,006afed0,0089eff0,0067e730,0063edc0,006d3a80,005a8860,008d98f0,008d4680,005b3380,006be820,005a65d0,00645460,005a3390,006dea90,006b6a20,008d95e0,006900f0,007ac1e0,00645830,0057eca0,008b6930,006b6bd0,005f9ff0,006b72e0,00656af0,005fe0c0,006db4a0,006b4ea0,006d3920,008d4ac0,0058a1c0,005a2ee0,005d2f80,0074ee90,008d26b0,006b9790,006d4220,006beb10,006415e0,006b7310,008b61c0 callees=0071b670,0051c890,006ad2f0,0091c860,00733570,00733780,00654860

undefined1 __thiscall FUN_006a4e00(int param_1,undefined4 param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  undefined1 *puVar4;
  char *pcVar5;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined4 local_30;
  char local_2a;
  char local_29;
  undefined1 local_28 [4];
  uint local_24;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ba00;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_20 = 0;
  for (local_14 = 0; (int)local_14 < (int)(uint)*(byte *)(param_1 + 0x8d); local_14 = local_14 + 1)
  {
    uVar3 = FUN_006ad2f0(local_14 & 0xff,param_2);
    local_20 = (uVar3 & 0xff) + local_20;
  }
  local_1c = 0;
  iVar1 = local_1c;
  local_18 = *(int *)(param_1 + 0x24);
  local_1c._0_1_ = (undefined1)local_20;
  if ((((local_18 != 0) && (*(int *)(*(int *)(local_18 + 0x1198) + 0x6c) == 0)) &&
      (*(int *)(local_18 + 0x11ac) == 0)) &&
     ((*(char *)(local_18 + 0x250) != '\0' &&
      (local_24 = (uint)*(byte *)(local_18 + 0x250), local_24 != 0)))) {
    local_30 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x154);
    local_1c = iVar1;
    FUN_00733570("LevelMult");
    local_8 = 0;
    FUN_0071b670(local_24,local_38,local_28);
    local_8 = 0xffffffff;
    FUN_00733780();
    local_2a = *(char *)(local_18 + 0x251);
    if (local_2a == '\0') {
      FUN_00733570("G_PC_LEVEL");
      local_8 = 1;
      pcVar5 = &local_2a;
      puVar4 = local_40;
      FUN_0051c890(puVar4,pcVar5);
      FUN_00654860(puVar4,pcVar5);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    local_29 = local_2a;
    if (local_2a < '\x01') {
      local_29 = '\x01';
    }
    local_1c = FUN_0091c860(uVar2,(int)local_29);
    local_1c = local_20 + -1 + local_1c;
    if (local_1c < 0) {
      local_1c = 1;
    }
  }
  ExceptionList = local_10;
  return (undefined1)local_1c;
}

