// FN 008a8b30 FUN_008a8b30 size=398 ret=4 callers=008a8880 callees=00417140,0073f750

void __thiscall FUN_008a8b30(int param_1,int param_2)

{
  int iVar1;
  undefined1 local_8;
  
  if ((param_2 < 0xc) && (-1 < param_2)) {
    iVar1 = FUN_0073f750();
    local_8 = (undefined1)param_2;
    *(undefined1 *)(iVar1 + 0xfc1) = local_8;
    FUN_00417140(&DAT_00a7fe24);
    FUN_00417140(&DAT_00a7fe24);
    FUN_00417140(&DAT_00a7fe24);
    FUN_00417140(&DAT_00a7fe24);
    FUN_00417140(&DAT_00a7fe24);
    FUN_00417140(&DAT_00a7fe24);
    FUN_00417140(&DAT_00a7fe24);
    switch(param_2) {
    case 0:
      FUN_00417140(&DAT_00a7fdac);
      break;
    case 6:
      FUN_00417140(&DAT_00a7fdac);
      break;
    case 7:
      FUN_00417140(&DAT_00a7fdac);
      break;
    case 8:
      FUN_00417140(&DAT_00a7fdac);
      break;
    case 9:
      FUN_00417140(&DAT_00a7fdac);
      break;
    case 10:
      FUN_00417140(&DAT_00a7fdac);
      break;
    case 0xb:
      FUN_00417140(&DAT_00a7fdac);
    }
    *(uint *)(param_1 + 0x24d0) = *(uint *)(param_1 + 0x24d0) | 4;
  }
  return;
}

