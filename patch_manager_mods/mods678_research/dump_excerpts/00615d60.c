// FN 00615d60 FUN_00615d60 size=296 ret=8 callers=008ceb00,008cdb00 callees=00919723,009196fd,006041d0,006042a0,0091dba0

void __thiscall FUN_00615d60(int param_1,int param_2,short param_3)

{
  void *_Dst;
  int iVar1;
  uint uVar2;
  undefined4 local_10;
  
  if (param_2 != 0) {
    iVar1 = *(int *)(param_2 + 0x28c);
    uVar2 = iVar1 + *(int *)(param_1 + 0x28c);
    local_10 = *(int *)(param_1 + 0x28c);
    _Dst = (void *)FUN_00919723(-(uint)((int)((ulonglong)uVar2 * 0x1c >> 0x20) != 0) |
                                (uint)((ulonglong)uVar2 * 0x1c));
    _memcpy(_Dst,*(void **)(param_1 + 0x294),local_10 * 0x1c);
    FUN_006041d0(uVar2);
    _memcpy(*(void **)(param_1 + 0x294),_Dst,local_10 * 0x1c);
    FUN_009196fd(_Dst);
    _memcpy((void *)((uVar2 - iVar1) * 0x1c + *(int *)(param_1 + 0x294)),*(void **)(param_2 + 0x294)
            ,iVar1 * 0x1c);
    for (; local_10 < (int)uVar2; local_10 = local_10 + 1) {
      iVar1 = FUN_006042a0(local_10);
      *(short *)(iVar1 + 0x12) = param_3 + 1;
    }
  }
  return;
}

