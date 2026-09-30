// FN 008c9c40 FUN_008c9c40 size=456 ret=12 callers=008c9030 callees=00418af0,008d0780,008cff80,0083ea60,006076d0,00919723,006d0620

void __thiscall FUN_008c9c40(int param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  char cVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *local_4c;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097af4b;
  local_10 = ExceptionList;
  uVar5 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = *(int *)(param_1 + 0x344);
  bVar1 = *(byte *)(param_1 + 0x338);
  iVar3 = *(int *)(param_1 + 0x30c);
  for (local_1c = 0; local_1c < 0x14; local_1c = local_1c + 1) {
    iVar8 = 1 << ((byte)local_1c & 0x1f);
    iVar6 = FUN_006d0620(iVar8);
    if (iVar6 != 0) {
      cVar4 = FUN_006076d0(param_4);
      if (cVar4 == *(char *)(param_1 + 0xc78)) {
        iVar7 = FUN_00919723(0x3bc,uVar5);
        local_8 = 0;
        if (iVar7 == 0) {
          local_4c = (int *)0x0;
        }
        else {
          local_4c = (int *)FUN_008cff80();
        }
        local_8 = 0xffffffff;
        (**(code **)(*local_4c + 0xa0))
                  (iVar2 + 0x160,iVar2 + 0x70,iVar2 + 0xe8,iVar3 + (uint)bVar1 * -2,0);
        FUN_00418af0(0,param_1,FUN_008c8650);
        FUN_00418af0(0x27,param_1,FUN_008c9460);
        FUN_00418af0(0x2d,param_1,FUN_008c9460);
        FUN_008d0780(*(undefined4 *)(iVar6 + 0x14),1,*(undefined4 *)(param_2 + 4),iVar8);
        FUN_0083ea60(local_4c);
      }
    }
  }
  ExceptionList = local_10;
  return;
}

