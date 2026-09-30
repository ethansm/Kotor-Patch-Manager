// FN 00615e90 FUN_00615e90 size=349 ret=8 callers=008cdb00 callees=00919723,009196fd,006041d0,006042a0,0091dba0

void __thiscall FUN_00615e90(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  bool bVar3;
  void *_Src;
  void *_Src_00;
  uint uVar4;
  undefined4 local_24;
  undefined4 local_1c;
  undefined4 local_10;
  
  iVar1 = *(int *)(param_2 + 0x28c);
  uVar4 = *(int *)(param_1 + 0x28c) - iVar1;
  iVar2 = *(int *)(param_1 + 0x28c);
  _Src = (void *)FUN_00919723(-(uint)((int)((ulonglong)uVar4 * 0x1c >> 0x20) != 0) |
                              (uint)((ulonglong)uVar4 * 0x1c));
  local_24 = 0;
  for (local_10 = 0; local_10 < iVar2; local_10 = local_10 + 1) {
    bVar3 = false;
    _Src_00 = (void *)(local_10 * 0x1c + *(int *)(param_1 + 0x294));
    for (local_1c = 0; local_1c < iVar1; local_1c = local_1c + 1) {
      FUN_006042a0(local_1c);
      if ((uint)*(ushort *)((int)_Src_00 + 0x12) == param_3 + 1U) {
        bVar3 = true;
      }
    }
    if (!bVar3) {
      _memcpy((void *)(local_24 * 0x1c + (int)_Src),_Src_00,0x1c);
      local_24 = local_24 + 1;
    }
  }
  FUN_006041d0(uVar4);
  if (*(int *)(param_1 + 0x294) != 0) {
    _memcpy(*(void **)(param_1 + 0x294),_Src,uVar4 * 0x1c);
  }
  FUN_009196fd(_Src);
  return;
}

