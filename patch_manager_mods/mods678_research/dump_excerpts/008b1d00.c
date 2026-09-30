// FN 008b1d00 FUN_008b1d00 size=390 ret=4 callers=008ac330 callees=004149f0

void __thiscall FUN_008b1d00(int param_1,undefined1 param_2)

{
  switch(param_2) {
  case 0:
  case 1:
    *(undefined4 *)(param_1 + 0x98) = DAT_00a7fde8;
    *(undefined4 *)(param_1 + 0x9c) = DAT_00a7fdec;
    *(undefined4 *)(param_1 + 0xa0) = DAT_00a7fdf0;
    *(undefined4 *)(param_1 + 0x1fc) = DAT_00a7fde8;
    *(undefined4 *)(param_1 + 0x200) = DAT_00a7fdec;
    *(undefined4 *)(param_1 + 0x204) = DAT_00a7fdf0;
    *(undefined4 *)(param_1 + 0x110) = DAT_00a7fdac;
    *(undefined4 *)(param_1 + 0x114) = DAT_00a7fdb0;
    *(undefined4 *)(param_1 + 0x118) = DAT_00a7fdb4;
    *(undefined4 *)(param_1 + 0x274) = DAT_00a7fdac;
    *(undefined4 *)(param_1 + 0x278) = DAT_00a7fdb0;
    *(undefined4 *)(param_1 + 0x27c) = DAT_00a7fdb4;
    break;
  case 2:
  case 3:
    *(undefined4 *)(param_1 + 0x98) = DAT_00a7fdd0;
    *(undefined4 *)(param_1 + 0x9c) = DAT_00a7fdd4;
    *(undefined4 *)(param_1 + 0xa0) = DAT_00a7fdd8;
    *(undefined4 *)(param_1 + 0x1fc) = DAT_00a7fdd0;
    *(undefined4 *)(param_1 + 0x200) = DAT_00a7fdd4;
    *(undefined4 *)(param_1 + 0x204) = DAT_00a7fdd8;
    *(undefined4 *)(param_1 + 0x110) = DAT_00a7fdd0;
    *(undefined4 *)(param_1 + 0x114) = DAT_00a7fdd4;
    *(undefined4 *)(param_1 + 0x118) = DAT_00a7fdd8;
    *(undefined4 *)(param_1 + 0x274) = DAT_00a7fdd0;
    *(undefined4 *)(param_1 + 0x278) = DAT_00a7fdd4;
    *(undefined4 *)(param_1 + 0x27c) = DAT_00a7fdd8;
  }
  FUN_004149f0(*(uint *)(param_1 + 0x3ac) & 1,1,0);
  FUN_004149f0(*(uint *)(param_1 + 0x3ac) & 1,0,1);
  *(undefined1 *)(param_1 + 0x3b0) = param_2;
  return;
}

