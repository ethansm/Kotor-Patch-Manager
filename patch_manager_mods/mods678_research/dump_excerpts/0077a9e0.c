// FN 0077a9e0 FUN_0077a9e0 size=1206 ret=4 callers=0077e650 callees=00733570,0071bcf0,007408f0,0058c3b0,00710eb0,007337c0,0077d800,00733780,00844e40,006f2a30

void __thiscall FUN_0077a9e0(int param_1,int *param_2)

{
  undefined4 *puVar1;
  undefined1 uVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined1 local_a0 [16];
  undefined1 local_90 [16];
  undefined1 local_80 [16];
  undefined1 local_70 [16];
  undefined1 local_60 [16];
  code *local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  int local_28;
  undefined4 local_24;
  int local_20;
  uint local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00965298;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = *(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x158);
  iVar4 = FUN_0077d800(uVar3);
  local_1c = (uint)*(ushort *)(*(int *)(iVar4 + 0x1198) + 0x146);
  if ((((local_1c != 9) && (local_1c != 0xb)) && (local_1c != 0xc)) && (local_1c != 0xd)) {
    local_1c = 9;
    iVar4 = FUN_0077d800(uVar3);
    *(undefined2 *)(*(int *)(iVar4 + 0x1198) + 0x146) = (undefined2)local_1c;
  }
  local_20 = param_2[1];
  local_24 = 0xffffffff;
  local_18 = -1;
  for (local_28 = 0; local_28 < *(int *)(local_14 + 0x24); local_28 = local_28 + 1) {
    FUN_00733570("NAME_STRREF");
    local_8 = 0;
    FUN_0071bcf0(local_28,local_30,&local_24);
    local_8 = 0xffffffff;
    FUN_00733780();
    FUN_00733570("AISTATE");
    local_8 = 1;
    FUN_0071bcf0(local_28,local_38,&local_18);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (local_18 != 10) {
      if (local_28 == 4) {
        uVar2 = FUN_00844e40(*(byte *)(*(int *)(param_1 + 0x310) + 0x32) - 1);
        iVar4 = FUN_006f2a30(uVar2);
        if (iVar4 == 0) {
          local_24 = 0x1ec65;
        }
      }
      FUN_0058c3b0(local_20 + 1);
      uVar5 = FUN_007408f0(local_40,local_24);
      local_8 = 2;
      FUN_007337c0(uVar5);
      local_8 = 0xffffffff;
      FUN_00733780();
      local_50 = FUN_0077d890;
      local_4c = 0;
      local_48 = 0;
      local_44 = 0;
      puVar1 = (undefined4 *)(*param_2 + 0xc + local_20 * 0x3c);
      *puVar1 = FUN_0077d890;
      puVar1[1] = 0;
      puVar1[2] = 0;
      puVar1[3] = 0;
      *(undefined1 *)(*param_2 + 0x38 + local_20 * 0x3c) = 1;
      *(undefined4 *)(*param_2 + 0x1c + local_20 * 0x3c) = *(undefined4 *)(param_1 + 4);
      switch(local_18) {
      case 9:
        *(undefined4 *)(*param_2 + 8 + local_20 * 0x3c) = 0x410;
        FUN_00710eb0(local_60,"ib_aggressive");
        break;
      case 0xb:
        *(undefined4 *)(*param_2 + 8 + local_20 * 0x3c) = 0x412;
        FUN_00710eb0(local_70,"ib_ranged");
        break;
      case 0xc:
        *(undefined4 *)(*param_2 + 8 + local_20 * 0x3c) = 0x413;
        FUN_00710eb0(local_80,"ib_stationary");
        break;
      case 0xd:
        *(undefined4 *)(*param_2 + 8 + local_20 * 0x3c) = 0x414;
        uVar2 = FUN_00844e40(*(byte *)(*(int *)(param_1 + 0x310) + 0x32) - 1);
        iVar4 = FUN_006f2a30(uVar2);
        if (iVar4 == 0) {
          FUN_00710eb0(local_a0,"ib_grenadier");
        }
        else {
          FUN_00710eb0(local_90,"ib_jedisupport");
        }
      }
      local_20 = local_20 + 1;
    }
  }
  ExceptionList = local_10;
  return;
}

