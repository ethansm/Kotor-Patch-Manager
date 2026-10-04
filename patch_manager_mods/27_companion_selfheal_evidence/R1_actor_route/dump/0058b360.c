// FN 0058b360 FUN_0058b360 size=572 ret=4 callers=008a8480 callees=0051c8d0,0051c280,005e1a60,006070a0,00919723,005e1bd0,00604260,005453c0,0051e170,0051c0e0,005482a0

void __thiscall FUN_0058b360(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined2 *puVar8;
  undefined2 *local_78;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined2 *local_28;
  int local_24;
  int local_20;
  short *local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095174b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = FUN_0051c0e0(param_2);
  local_14 = 0;
  do {
    if (*(int *)(local_18 + 0x288) <= local_14) {
LAB_0058b477:
      if (((*(int *)(param_1 + 0x11ac) != 0) && (local_20 = FUN_0051c8d0(uVar1), local_20 != 0)) &&
         ((*(uint *)(local_20 + 0x250) & 2) != 0)) {
        *(undefined4 *)(local_20 + 0x24c) = 2;
        local_24 = FUN_005453c0();
        if (local_24 != 0) {
          iVar2 = FUN_00919723(0x34);
          local_8 = 0;
          if (iVar2 == 0) {
            local_78 = (undefined2 *)0x0;
          }
          else {
            local_78 = (undefined2 *)FUN_005e1a60();
          }
          local_8 = 0xffffffff;
          local_28 = local_78;
          *local_78 = 0xb;
          FUN_005e1bd0(0,0xfa1);
          uVar6 = *(undefined4 *)(local_24 + 0x124);
          uVar5 = *(undefined4 *)(local_24 + 0x124);
          uVar4 = 0;
          uVar3 = 0;
          uVar7 = DAT_0099241c;
          puVar8 = local_28;
          FUN_0051c280(0,0,uVar5,uVar6,DAT_0099241c,local_28);
          FUN_0051e170(uVar3,uVar4,uVar5,uVar6,uVar7,puVar8);
        }
      }
      ExceptionList = local_10;
      return;
    }
    local_1c = (short *)FUN_00604260(local_14);
    if (*local_1c == 10) {
      local_34 = 0;
      local_30 = 0;
      local_2c = 0;
      FUN_005482a0(local_1c[1],&local_34,*(undefined4 *)(param_1 + 4),0,param_2,0,0,0,0);
      FUN_006070a0(local_14,*(undefined4 *)(param_1 + 4));
      if (*(int *)(param_1 + 0x520) != 0) {
        *(undefined4 *)(param_1 + 0x11d8) = 3000;
      }
      goto LAB_0058b477;
    }
    local_14 = local_14 + 1;
  } while( true );
}

