// FN 00602dd0 FUN_00602dd0 size=1458 ret=8 callers=0052a730,005f8710,005f18c0,0062b5f0,00603740,00574500 callees=00719bf0,007199d0,00606a80,00719e50,00719b90,00719f30,00603740,00719ce0,00603390,00719ab0,006d5200

undefined4 __thiscall FUN_00602dd0(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined *puVar3;
  
  FUN_00719bf0(param_3,*(undefined4 *)(param_1 + 0xc),"BaseItem");
  FUN_00719e50(param_3,param_1 + 0x28,&DAT_00993538);
  FUN_007199d0(param_3,1,"Identified");
  FUN_00719f30(param_3,param_1 + 0x2b8,"Description");
  FUN_00719f30(param_3,param_1 + 0x2b0,"DescIdentified");
  FUN_00719f30(param_3,param_1 + 0x2c0,"LocalizedName");
  FUN_00719ab0(param_3,*(uint *)(param_1 + 0x2cc) & 0xffff,"StackSize");
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x2c8) >> 5 & 1,"Stolen");
  FUN_00719b90(param_3,*(undefined4 *)(param_1 + 0x2d4),"Upgrades");
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x2f0) & 0xff,"UpgradeLevel");
  if (*(int *)(param_1 + 0x2f4) != 0) {
    FUN_00719bf0(param_3,*(undefined4 *)(param_1 + 0x2f4),"UpgradeSlot0");
  }
  if (*(int *)(param_1 + 0x2f8) != 0) {
    FUN_00719bf0(param_3,*(undefined4 *)(param_1 + 0x2f8),"UpgradeSlot1");
  }
  if (*(int *)(param_1 + 0x2fc) != 0) {
    FUN_00719bf0(param_3,*(undefined4 *)(param_1 + 0x2fc),"UpgradeSlot2");
  }
  if (*(int *)(param_1 + 0x300) != 0) {
    FUN_00719bf0(param_3,*(undefined4 *)(param_1 + 0x300),"UpgradeSlot3");
  }
  if (*(int *)(param_1 + 0x304) != 0) {
    FUN_00719bf0(param_3,*(undefined4 *)(param_1 + 0x304),"UpgradeSlot4");
  }
  if (*(int *)(param_1 + 0x308) != 0) {
    FUN_00719bf0(param_3,*(undefined4 *)(param_1 + 0x308),"UpgradeSlot5");
  }
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x2c8) >> 3 & 1,"Dropable");
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x2c8) >> 4 & 1,"Pickpocketable");
  FUN_007199d0(param_3,*(undefined1 *)(param_1 + 0x2a4),"ModelVariation");
  iVar1 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  if (*(char *)(iVar1 + 0x18) == '\x01') {
    FUN_007199d0(param_3,*(undefined1 *)(param_1 + 0x2a5),"BodyVariation");
    FUN_007199d0(param_3,*(undefined1 *)(param_1 + 0x2a6),"TextureVar");
  }
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x298) & 0xff,"Charges");
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x29c) & 0xff,"MaxCharges");
  puVar3 = &DAT_0099759c;
  uVar2 = FUN_00606a80(&DAT_0099759c);
  FUN_00719b90(param_3,uVar2,puVar3);
  FUN_00719b90(param_3,*(undefined4 *)(param_1 + 0x2a0),"AddCost");
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x10c) & 0xff,&DAT_00996df4);
  iVar1 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  if (*(int *)(iVar1 + 0x70) != 0) {
    FUN_00603740(param_2,param_3);
  }
  FUN_00603390(param_2,param_3);
  FUN_00719ce0(param_3,*(undefined4 *)(param_1 + 0xa4),"XPosition");
  FUN_00719ce0(param_3,*(undefined4 *)(param_1 + 0xa8),"YPosition");
  FUN_00719ce0(param_3,*(undefined4 *)(param_1 + 0xac),"ZPosition");
  FUN_00719ce0(param_3,*(undefined4 *)(param_1 + 0xb0),"XOrientation");
  FUN_00719ce0(param_3,*(undefined4 *)(param_1 + 0xb4),"YOrientation");
  FUN_00719ce0(param_3,*(undefined4 *)(param_1 + 0xb8),"ZOrientation");
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x2c8) >> 6 & 1,"NonEquippable");
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x2c8) >> 7 & 1,"NewItem");
  FUN_007199d0(param_3,*(uint *)(param_1 + 0x2c8) >> 8 & 1,"DELETING");
  return 1;
}

