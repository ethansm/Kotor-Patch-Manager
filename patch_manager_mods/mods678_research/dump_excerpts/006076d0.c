// FN 006076d0 FUN_006076d0 size=181 ret=4 callers=008c7510,008c9030,008c7f60,008c70b0,008c7aa0,008c9c40 callees=006d6e30

undefined1 __fastcall FUN_006076d0(int param_1)

{
  byte bVar1;
  int iVar2;
  undefined1 local_5;
  
  local_5 = 0;
  if (0 < *(int *)(param_1 + 0x2f0)) {
    iVar2 = FUN_006d6e30();
    bVar1 = *(byte *)(iVar2 + 0xac);
    if (((bVar1 == 0) || (bVar1 == 1)) || (bVar1 == 2)) {
      local_5 = 3;
    }
    else if (((bVar1 == 0x27) || (bVar1 == 0x28)) || (bVar1 == 0x29)) {
      local_5 = 1;
    }
    else if (((bVar1 == 3) || (bVar1 == 4)) || (bVar1 == 5)) {
      local_5 = 2;
    }
    else if ((0x1e < bVar1) && (bVar1 < 0x27)) {
      local_5 = 4;
    }
  }
  return local_5;
}

