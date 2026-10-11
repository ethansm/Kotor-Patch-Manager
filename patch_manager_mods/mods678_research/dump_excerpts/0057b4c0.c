// FN 0057b4c0 FUN_0057b4c0 size=1144 ret=28 callers=0057da90,0065be70,0057d8b0 callees=005d1750,0051c280,005680d0,005e1a60,006d6e30,00604260,0051e170,0057b280,005d2f80,0051c0e0,0051c8d0,005648d0,00919723,005e1bd0,0053f560,005453c0,0051c0a0,006d5200

undefined4 __thiscall
FUN_0057b4c0(int param_1,undefined4 param_2,undefined1 param_3,undefined1 param_4,int param_5,
            undefined4 *param_6,undefined4 param_7,undefined4 param_8)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  short *psVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined2 *local_a4;
  undefined4 local_a0;
  undefined4 local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095112b;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar3 = FUN_0051c0e0(param_2);
  piVar4 = (int *)FUN_0051c0a0(param_5);
  if (iVar3 == 0) {
    local_18 = 0;
  }
  else {
    iVar5 = FUN_005d2f80(iVar3,0,0);
    if (iVar5 == 0) {
      local_18 = 0;
    }
    else {
      psVar6 = (short *)FUN_00604260(param_3);
      if (psVar6 == (short *)0x0) {
        local_18 = 0;
      }
      else {
        local_18 = 0;
        if ((*(byte *)((int)psVar6 + 0x11) == 0xff) ||
           ((1 << (*(byte *)((int)psVar6 + 0x11) & 0x1f) & *(uint *)(iVar3 + 0x2d4)) != 0)) {
          bVar1 = true;
        }
        else {
          bVar1 = false;
        }
        if ((bVar1) && (*psVar6 == 10)) {
          if ((char)psVar6[6] != '\0') {
            if ((((piVar4 != (int *)0x0) &&
                 (iVar5 = (**(code **)(*piVar4 + 0x30))(uVar2), iVar5 != 0)) &&
                (iVar5 = FUN_005d1750(param_5), iVar5 != 0)) &&
               ((*(byte *)(iVar5 + 4) >> 2 & 3) == 1)) {
              ExceptionList = local_10;
              return DAT_009944b8;
            }
            FUN_006d5200(*(undefined4 *)(iVar3 + 0xc));
            if ((*(int *)(param_1 + 0x520) == 0) || (iVar3 = FUN_0053f560(0xc), iVar3 == 0)) {
              local_a0 = 0;
            }
            else {
              local_a0 = 1;
            }
            local_18 = FUN_005648d0(param_2,param_3,param_4,*param_6,param_6[1],param_6[2],param_5,
                                    param_8,param_5 == 0x7f000000,local_a0,0);
          }
        }
        else if (*psVar6 == 0x25) {
          local_18 = FUN_0057b280(6,0,param_5,param_6,param_7,param_2,param_3);
        }
        else if (*psVar6 == 0x2e) {
          local_18 = FUN_005680d0(*(undefined4 *)(iVar3 + 0x14),0x1d,param_5,param_6,param_7);
        }
        if (*(int *)(param_1 + 0x11ac) != 0) {
          bVar1 = false;
          iVar3 = FUN_0051c8d0();
          if ((iVar3 != 0) && ((*(uint *)(iVar3 + 0x250) & 2) != 0)) {
            *(undefined4 *)(iVar3 + 0x24c) = 2;
            bVar1 = true;
          }
          if ((((!bVar1) && (iVar3 != 0)) && ((*(uint *)(iVar3 + 0x250) & 0x80) != 0)) &&
             (iVar5 = FUN_006d6e30(), *(char *)(iVar5 + 0xac) != '\x14')) {
            *(undefined4 *)(iVar3 + 0x24c) = 0x80;
            bVar1 = true;
          }
          if ((bVar1) && (iVar3 = FUN_005453c0(), iVar3 != 0)) {
            iVar5 = FUN_00919723(0x34);
            local_8 = 0;
            if (iVar5 == 0) {
              local_a4 = (undefined2 *)0x0;
            }
            else {
              local_a4 = (undefined2 *)FUN_005e1a60();
            }
            local_8 = 0xffffffff;
            *local_a4 = 0xb;
            FUN_005e1bd0(0,0xfa1);
            uVar10 = *(undefined4 *)(iVar3 + 0x124);
            uVar9 = *(undefined4 *)(iVar3 + 0x124);
            uVar8 = 0;
            uVar7 = 0;
            uVar11 = DAT_0099241c;
            FUN_0051c280(0,0,uVar9,uVar10,DAT_0099241c,local_a4);
            FUN_0051e170(uVar7,uVar8,uVar9,uVar10,uVar11,local_a4);
          }
        }
      }
    }
  }
  ExceptionList = local_10;
  return local_18;
}

