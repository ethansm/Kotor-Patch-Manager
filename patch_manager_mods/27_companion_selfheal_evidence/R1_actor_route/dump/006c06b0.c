// FN 006c06b0 FUN_006c06b0 size=947 ret=12 callers=0057da90,0057d8b0 callees=006d0620,0058a2c0,00604260,00606100,006d5200,00603f60

undefined4 __thiscall FUN_006c06b0(int param_1,uint param_2,undefined4 *param_3,int *param_4)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  short *psVar4;
  int iVar5;
  int local_20;
  int local_18;
  uint local_8;
  
  if (*(int *)(*(int *)(param_1 + 0x24) + 0x1150) != 0) {
    for (local_8 = 0; local_8 < 0x14; local_8 = local_8 + 1) {
      iVar2 = FUN_006d0620(1 << ((byte)local_8 & 0x1f));
      if ((iVar2 != 0) && (iVar3 = FUN_00603f60(10,0), iVar3 != 0)) {
        for (local_18 = 0; local_18 < *(int *)(iVar2 + 0x288); local_18 = local_18 + 1) {
          psVar4 = (short *)FUN_00604260(local_18);
          if ((*psVar4 == 10) && (iVar3 = FUN_00604260(local_18), *(char *)(iVar3 + 0xc) != '\0')) {
            iVar3 = FUN_00604260(local_18);
            if ((*(byte *)(iVar3 + 0x11) == 0xff) ||
               ((1 << (*(byte *)(iVar3 + 0x11) & 0x1f) & *(uint *)(iVar2 + 0x2d4)) != 0)) {
              bVar1 = true;
            }
            else {
              bVar1 = false;
            }
            if ((bVar1) && (iVar3 = FUN_00604260(local_18), *(ushort *)(iVar3 + 2) == param_2)) {
              *param_3 = *(undefined4 *)(iVar2 + 0x14);
              *param_4 = local_18;
              return 1;
            }
          }
        }
      }
    }
  }
  iVar2 = FUN_0058a2c0(1);
  if (iVar2 != 0) {
    for (local_8 = 0; local_8 < *(uint *)(iVar2 + 0x10); local_8 = local_8 + 1) {
      iVar3 = FUN_00606100(local_8);
      if (((iVar3 != 0) &&
          (iVar5 = FUN_006d5200(*(undefined4 *)(iVar3 + 0xc)), *(int *)(iVar5 + 4) == 0)) &&
         (iVar5 = FUN_00603f60(10,0), iVar5 != 0)) {
        for (local_20 = 0; local_20 < *(int *)(iVar3 + 0x288); local_20 = local_20 + 1) {
          psVar4 = (short *)FUN_00604260(local_20);
          if ((*psVar4 == 10) &&
             ((iVar5 = FUN_00604260(local_20), *(char *)(iVar5 + 0xc) != '\0' ||
              (0 < *(int *)(iVar3 + 0x298))))) {
            iVar5 = FUN_00604260(local_20);
            if ((*(byte *)(iVar5 + 0x11) == 0xff) ||
               ((1 << (*(byte *)(iVar5 + 0x11) & 0x1f) & *(uint *)(iVar3 + 0x2d4)) != 0)) {
              bVar1 = true;
            }
            else {
              bVar1 = false;
            }
            if ((bVar1) && (iVar5 = FUN_00604260(local_20), *(ushort *)(iVar5 + 2) == param_2)) {
              *param_3 = *(undefined4 *)(iVar3 + 0x14);
              *param_4 = local_20;
              return 1;
            }
          }
        }
      }
    }
  }
  return 0;
}

