// FN 00541080 FUN_00541080 size=779 ret=4 callers=00566410,005a60b0,007445d0,005f8520,00696e10,0066df70,00656af0,0066ad10,0065b490,007469f0,005951a0,005f8130,0059de30,0066f330,00599050,005d5550,005302e0,005fd810,00568540,0057da90,008b05e0,0065be70,007511b0,005df920,005953e0,0065b270,008b0320,005959e0,00751750,0051cb10,0089fbd0,005a6570,00547bb0,0068db00,005fd480,00597d90,005f7f30,00586750,00586b70,0059b520,0077cf70,005a6280,00563c90,006cf5d0,0065e670,0057de70,005f8320,005d75b0,005a64c0 callees=007e6be0,0073fec0,0064dde0,0053e740,00734010,00561430,005908f0,0073fb90,0058c370,006f9c90,009196fd,00521060,007a1720

void __thiscall FUN_00541080(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_8;
  
  iVar1 = (**(code **)(*param_1 + 0x10))();
  if (iVar1 != 0) {
    if (((DAT_00a7e0c8 != 0) && (iVar1 = (**(code **)(*param_1 + 0x30))(), iVar1 != 0)) &&
       (iVar1 = (**(code **)(*param_1 + 0x30))(), *(int *)(iVar1 + 0x11ac) != 0)) {
      FUN_00734010();
    }
    if (param_1[0x3b] == 1) {
      local_8 = FUN_007a1720();
      while (local_8 != 0) {
        iVar2 = FUN_00561430(local_8);
        iVar1 = local_8;
        FUN_0058c370(&local_8);
        iVar3 = FUN_00521060();
        iVar2 = (**(code **)(*param_1 + 0x74))(iVar2,iVar2 == iVar3);
        if ((iVar2 == 1) && (iVar1 = FUN_0064dde0(iVar1), iVar1 != 0)) {
          FUN_0053e740();
          FUN_009196fd(iVar1);
        }
      }
      iVar1 = (**(code **)(*param_1 + 0x30))();
      if (iVar1 != 0) {
        (**(code **)(*param_1 + 0x30))();
        FUN_006f9c90();
        iVar1 = (**(code **)(*param_1 + 0x30))();
        *(undefined4 *)(iVar1 + 0x11b0) = 1;
        iVar1 = (**(code **)(*param_1 + 0x30))();
        *(undefined4 *)(iVar1 + 0x54c) = 0x7f000000;
        iVar1 = (**(code **)(*param_1 + 0x30))();
        iVar1 = *(int *)(iVar1 + 0x380);
        *(undefined4 *)(iVar1 + 0x254) = 0x7f000000;
        *(undefined4 *)(iVar1 + 600) = 0;
        *(undefined4 *)(iVar1 + 0x25c) = 0;
        *(undefined4 *)(iVar1 + 0x260) = 0;
        if (param_2 != 0) {
          (**(code **)(*param_1 + 0x30))();
          FUN_005908f0();
        }
        (**(code **)(*param_1 + 0x30))();
        FUN_00734010();
        iVar1 = (**(code **)(*param_1 + 0x30))();
        if (*(int *)(iVar1 + 0x11ac) != 0) {
          iVar1 = (**(code **)(*param_1 + 0x30))();
          uVar4 = FUN_0073fec0(*(undefined4 *)(iVar1 + 4));
          FUN_0073fb90(uVar4);
          iVar1 = FUN_007e6be0(uVar4);
          if (iVar1 != -1) {
            iVar2 = FUN_0073fb90();
            *(undefined4 *)(iVar2 + 0x28 + iVar1 * 0x88) = 0xffffffff;
          }
        }
      }
    }
  }
  return;
}

