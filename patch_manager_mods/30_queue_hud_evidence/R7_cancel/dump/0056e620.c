// FN 0056e620 FUN_0056e620 size=2219 ret=8 callers= callees=00589c30,00646fc0,00591a70,005b7ae0,0073f750,0057d370,0057e3d0,0073fb90,00646df0,0057a990,00646d90,0073fec0,00646ee0,0056eed0,007e6be0,007ceb80,00589e60,0051c2e0,0051c160,0052f330,0057e140,0091dba0

undefined4 __thiscall FUN_0056e620(int *param_1,int *param_2,int param_3)

{
  undefined4 uVar1;
  int *piVar2;
  int iVar3;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  if (param_2[0x1c] == 0) {
    uVar1 = 0;
  }
  else {
    local_c = FUN_0051c2e0();
    local_8 = FUN_0052f330(param_1[1]);
    if ((local_8 != 0) && (local_c != 0)) {
      if (*param_2 == 8) {
        _memcpy(&local_10,param_2 + 0xe,4);
        _memcpy(&local_14,param_2 + 0xf,4);
        FUN_00646df0(*(undefined4 *)(local_8 + 4),local_10,local_14);
      }
      else if (*param_2 == 7) {
        _memcpy(&local_10,param_2 + 0xe,4);
        FUN_00646ee0(*(undefined4 *)(local_8 + 4),local_10);
      }
      else if (*param_2 == 9) {
        _memcpy(&local_10,param_2 + 0xe,4);
        FUN_00646d90(*(undefined4 *)(local_8 + 4),local_10);
      }
      else if (*param_2 == 0xb) {
        _memcpy(&local_10,param_2 + 0xe,4);
        FUN_00646fc0(*(undefined4 *)(local_8 + 4),local_10);
      }
    }
    if (*param_2 == 0x2a) {
      FUN_0057e140(0x14);
    }
    else if (*param_2 == 0xc) {
      param_1[0x152] = 0x7f000000;
      FUN_0057e3d0(0x7f000000);
      (**(code **)(*param_1 + 0x7c))(10000);
      FUN_00589c30();
    }
    else if (*param_2 == 1) {
      *(undefined4 *)(param_1[0xe0] + 0x30) = 0x7f000000;
      *(undefined4 *)(param_1[0xe0] + 0x38) = 0x7f000000;
      param_1[0x46c] = 1;
    }
    else if (*param_2 == 0xf) {
      param_1[0x15a] = 0x7f000000;
      param_1[0x15b] = 0x7f000000;
      FUN_00591a70();
      param_1[0x41a] = 0;
      (**(code **)(*param_1 + 0x7c))(10000);
      FUN_00589c30();
    }
    else if (*param_2 == 0x2e) {
      param_1[0x15a] = 0x7f000000;
      param_1[0x15b] = 0x7f000000;
      FUN_00591a70();
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x41d] = 0;
      FUN_00589c30();
    }
    else if (*param_2 == 0x32) {
      FUN_005b7ae0(0,1);
    }
    else if (*param_2 == 9) {
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x424] = 0;
    }
    else if (*param_2 == 7) {
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x425] = 0;
    }
    else if ((((*param_2 == 0x1d) || (*param_2 == 0x19)) || (*param_2 == 0x1a)) ||
            ((*param_2 == 0x1c || (*param_2 == 0x1b)))) {
      (**(code **)(*param_1 + 0x7c))(10000);
      FUN_00589e60();
      param_1[0x421] = 0;
      param_1[0x42b] = 0;
    }
    else if (*param_2 == 0x36) {
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x41e] = 0;
      param_1[0x41f] = 0;
    }
    else if (*param_2 == 0x38) {
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x420] = 0;
    }
    else if (*param_2 == 0x26) {
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x422] = 0;
      FUN_00589e60();
    }
    else if (*param_2 == 0x43) {
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x42c] = 0;
      FUN_00589e60();
    }
    else if (*param_2 == 0x47) {
      (**(code **)(*param_1 + 0x7c))(10000);
    }
    else if (*param_2 == 0x44) {
      *(undefined1 *)(param_1 + 0x443) = 0xff;
      (**(code **)(*param_1 + 0x7c))(0x2711);
      FUN_00589c30();
    }
    else if (*param_2 == 0x45) {
      *(undefined1 *)(param_1 + 0x443) = 0xff;
      FUN_0057a990();
      (**(code **)(*param_1 + 0x7c))(10000);
    }
    else if (*param_2 == 0x27) {
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x423] = 0;
      FUN_00589e60();
    }
    else if (*param_2 == 0x14) {
      (**(code **)(*param_1 + 0x7c))(10000);
      param_1[0x86] = 0;
    }
    else if (*param_2 == 0x3d) {
      if (param_1[0x46b] != 0) {
        uVar1 = FUN_0073fec0(param_1[1]);
        local_18 = uVar1;
        FUN_0073fb90(uVar1);
        local_1c = FUN_007e6be0(uVar1);
        if ((-1 < local_1c) && (local_1c < 3)) {
          FUN_0073fb90();
          FUN_0056eed0();
        }
      }
    }
    else if (*param_2 == 0x46) {
      *(undefined4 *)(param_1[0xe0] + 0x30) = 0x7f000000;
      *(undefined4 *)(param_1[0xe0] + 0x38) = 0x7f000000;
      param_1[0x46c] = 1;
    }
    else if (*param_2 == 0x18) {
      uVar1 = 0;
      FUN_0073f750(0);
      FUN_007ceb80(uVar1);
    }
    else if (*param_2 == 0x20) {
      param_1[0x14] = 0;
    }
    else if (*param_2 == 0x28) {
      _memcpy(&local_20,param_2 + 0xe,4);
      piVar2 = (int *)FUN_0051c160(local_20);
      if ((piVar2 != (int *)0x0) && (piVar2[0x12a] != 0)) {
        piVar2[0x12a] = 0;
        if (piVar2[0x36] == 0x275b) {
          (**(code **)(*piVar2 + 0x7c))(0x275c);
        }
        piVar2[0x37] = 0x3f800000;
      }
    }
    if (((local_c != 0) && (*param_2 == 1)) && (param_3 != 0)) {
      (**(code **)(*param_1 + 0x7c))(10000);
    }
    iVar3 = (**(code **)(*param_1 + 0xc4))();
    if (iVar3 != DAT_00994484) {
      (**(code **)(*param_1 + 0xc4))();
      FUN_0057d370(DAT_00994484,0);
    }
    if (param_3 == 1) {
      param_1[0x30] = 0;
      param_1[0x31] = 0;
    }
    uVar1 = 1;
  }
  return uVar1;
}

