// NAMED 0x008a7350 CSWGuiInGameInventory::Draw evidence=ev_1a3c2b237a698cf64bae6a915368abdda04643cc45c7c3fbc5b3341f8e194051 names=7 exe=6a522e71631d

void CSWGuiInGameInventory__Draw(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  int in_ECX;
  
  if ((*(uint *)(in_ECX + 0x24d0) >> 2 & 1) != 0) {
    CSWGuiInGameInventory__PopulateItemListBox();
  }
  CSWSEncounter__AIUpdate();
  if ((*(uint *)(in_ECX + 0x24d0) >> 1 & 1) == 0) {
    CSWGuiTextParams__SetColor(&DAT_00a7fde8);
    *(undefined4 *)(in_ECX + 0xdd0) = DAT_00a7fd4c;
    *(undefined4 *)(in_ECX + 0xdd4) = DAT_00a7fd50;
    *(undefined4 *)(in_ECX + 0xdd8) = DAT_00a7fd54;
  }
  else {
    uVar1 = CServerExoApp__GetPlayerCreatureId();
    iVar2 = CServerExoApp__GetCreatureByGameObjectID(uVar1);
    if (*(int *)(iVar2 + 0x11d8) == 0) {
      if ((*(uint *)(in_ECX + 0xd80) & 1) == 0) {
        CSWGuiTextParams__SetColor(&DAT_00a7fe24);
        *(undefined4 *)(in_ECX + 0xdd0) = DAT_00a7fde8;
        *(undefined4 *)(in_ECX + 0xdd4) = DAT_00a7fdec;
        *(undefined4 *)(in_ECX + 0xdd8) = DAT_00a7fdf0;
      }
      else {
        CSWGuiTextParams__SetColor(&DAT_00a7fdac);
        *(undefined4 *)(in_ECX + 0xdd0) = DAT_00a7fde8;
        *(undefined4 *)(in_ECX + 0xdd4) = DAT_00a7fdec;
        *(undefined4 *)(in_ECX + 0xdd8) = DAT_00a7fdf0;
      }
    }
    else {
      CSWGuiTextParams__SetColor(&DAT_00a7fde8);
      *(undefined4 *)(in_ECX + 0xdd0) = DAT_00a7fd4c;
      *(undefined4 *)(in_ECX + 0xdd4) = DAT_00a7fd50;
      *(undefined4 *)(in_ECX + 0xdd8) = DAT_00a7fd54;
    }
  }
  CSWGuiPanel__Draw(param_1);
  return;
}

