// FN 005915f0 FUN_005915f0 size=115 ret=0 callers=0065b4f0,006dab20,0065b270 callees=00592260,005908c0,005933f0

void __fastcall FUN_005915f0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 local_8;
  
  local_8 = (uint)*(byte *)(param_1 + 0xabc);
  while( true ) {
    uVar1 = FUN_00592260();
    if ((int)(uVar1 & 0xff) <= (int)local_8) break;
    iVar2 = FUN_005908c0(local_8);
    if (*(short *)(iVar2 + 100) != 0) {
      iVar2 = FUN_005908c0(local_8);
      *(undefined2 *)(iVar2 + 100) = 0;
    }
    local_8 = local_8 + 1;
  }
  FUN_005933f0(0);
  return;
}

