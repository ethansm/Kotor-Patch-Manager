// FN 0077d890 FUN_0077d890 size=417 ret=8 callers= callees=0077d800,00844e40,006f2a30,00740a20,0073f750

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0077d890(int param_1,undefined4 param_2)

{
  undefined1 uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar2 = FUN_0077d800();
  if ((iVar2 != 0) && (iVar2 = *(int *)(iVar2 + 0x1198), iVar2 != 0)) {
    local_c = 0xffffffff;
    local_8 = 0xff;
    switch(param_2) {
    case 0x410:
      local_c = 0x1e33c;
      *(undefined2 *)(iVar2 + 0x146) = 9;
      local_8 = 0x3d;
      break;
    case 0x412:
      local_c = 0x1e33e;
      *(undefined2 *)(iVar2 + 0x146) = 0xb;
      local_8 = 0x3f;
      break;
    case 0x413:
      local_c = 0x1e33f;
      *(undefined2 *)(iVar2 + 0x146) = 0xc;
      local_8 = 0x40;
      break;
    case 0x414:
      uVar1 = FUN_00844e40(*(byte *)(*(int *)(param_1 + 0x310) + 0x32) - 1);
      iVar3 = FUN_006f2a30(uVar1);
      if (iVar3 == 0) {
        local_c = 0x1ec66;
        local_8 = 0x42;
      }
      else {
        local_c = 0x1ec3e;
        local_8 = 0x41;
      }
      *(undefined2 *)(iVar2 + 0x146) = 0xd;
    }
    FUN_00740a20(local_8,0x7f000000,0x7f000000,0);
    iVar2 = FUN_0073f750();
    iVar2 = *(int *)(iVar2 + 0x98);
    *(undefined4 *)(iVar2 + 0x15410) = local_c;
    *(undefined4 *)(iVar2 + 0x1540c) = _DAT_00986b78;
  }
  return;
}

