// FN 008a6170 FUN_008a6170 size=3770 ret=4 callers=007be4c0,007d0760 callees=0041a510,00419740,00417140,0040e5a0,0073f750,0091d67e,0084a640,0040f5a0,00760ea0,00733780,0041a3d0,00733570,00416ea0,009196ee,0040f620,00418af0,0040ec00,0041ce90,00733540,00710840,0040ee40

void __thiscall FUN_008a6170(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined1 local_2b8 [8];
  undefined1 local_2b0 [8];
  undefined1 local_2a8 [8];
  undefined1 local_2a0 [8];
  undefined1 local_298 [8];
  undefined1 local_290 [8];
  undefined1 local_288 [8];
  undefined1 local_280 [8];
  undefined1 local_278 [8];
  undefined1 local_270 [8];
  undefined1 local_268 [8];
  undefined1 local_260 [8];
  undefined1 local_258 [8];
  undefined1 local_250 [8];
  undefined1 local_248 [8];
  undefined1 local_240 [8];
  undefined1 local_238 [8];
  undefined1 local_230 [8];
  undefined1 local_228 [8];
  undefined1 local_220 [8];
  undefined1 local_218 [8];
  undefined1 local_210 [484];
  uint local_2c;
  uint local_28;
  uint local_24;
  int local_20;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00977f22;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_2c = uVar1;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiInGameInventory::vftable;
  FUN_00419740(uVar1);
  local_8._0_1_ = 1;
  FUN_00419740();
  local_8._0_1_ = 2;
  FUN_00419740();
  local_8._0_1_ = 3;
  FUN_00419740();
  local_8._0_1_ = 4;
  FUN_0041ce90();
  local_8._0_1_ = 5;
  FUN_0041ce90();
  local_8._0_1_ = 6;
  FUN_0041a510();
  local_8._0_1_ = 7;
  FUN_0041a510();
  local_8._0_1_ = 8;
  _eh_vector_constructor_iterator_
            (param_1 + 0x3c2,0x148,6,FUN_00419740,
             CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._0_1_ = 9;
  FUN_00419740();
  local_8._0_1_ = 10;
  FUN_0041a510();
  local_8._0_1_ = 0xb;
  FUN_0041a510();
  local_8._0_1_ = 0xc;
  FUN_0041a510();
  local_8._0_1_ = 0xd;
  FUN_0041a510();
  local_8._0_1_ = 0xe;
  FUN_0041a510();
  local_8._0_1_ = 0xf;
  FUN_0041a510();
  local_8._0_1_ = 0x10;
  FUN_0041a510();
  local_8._0_1_ = 0x11;
  FUN_0084a640(0);
  local_8._0_1_ = 0x12;
  FUN_0084a640(0);
  local_8._0_1_ = 0x13;
  FUN_0041a510();
  local_8._0_1_ = 0x14;
  param_1[0x934] = param_1[0x934] & 0xfffffffb;
  param_1[0x934] = param_1[0x934] & 0xfffffffe;
  param_1[0x932] = 0x7f000000;
  FUN_00710840("inventory_p");
  FUN_0040ee40(local_210,1,1);
  FUN_00733570("LB_DESCRIPTION");
  local_8._0_1_ = 0x15;
  FUN_0040f620(param_1 + 0x21e,local_218,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LB_ITEMS");
  local_8._0_1_ = 0x16;
  FUN_0040f620(param_1 + 0x162,local_220,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_INV");
  local_8._0_1_ = 0x17;
  FUN_0040f620(param_1 + 0x6c,local_228,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_CREDITS");
  local_8._0_1_ = 0x18;
  FUN_0040f620(param_1 + 0xbe,local_230,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_CREDITS_VALUE");
  local_8._0_1_ = 0x19;
  FUN_0040f620(param_1 + 0x110,local_238,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_EXIT");
  local_8._0_1_ = 0x1a;
  FUN_0040f620(param_1 + 0x2da,local_240,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_USEITEM");
  local_8._0_1_ = 0x1b;
  FUN_0040f620(param_1 + 0x34e,local_248,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00760ea0(param_1 + 0x34e,0x61);
  FUN_00733570("LBL_BAR1");
  local_8._0_1_ = 0x1c;
  FUN_0040f620(param_1 + 0x3c2,local_250,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_BAR2");
  local_8._0_1_ = 0x1d;
  FUN_0040f620(param_1 + 0x414,local_258,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_BAR3");
  local_8._0_1_ = 0x1e;
  FUN_0040f620(param_1 + 0x466,local_260,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_BAR4");
  local_8._0_1_ = 0x1f;
  FUN_0040f620(param_1 + 0x4b8,local_268,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_BAR5");
  local_8._0_1_ = 0x20;
  FUN_0040f620(param_1 + 0x50a,local_270,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_BAR6");
  local_8._0_1_ = 0x21;
  FUN_0040f620(param_1 + 0x55c,local_278,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("LBL_FILTER");
  local_8._0_1_ = 0x22;
  FUN_0040f620(param_1 + 0x5ae,local_280,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_ALL");
  local_8._0_1_ = 0x23;
  FUN_0040f620(param_1 + 0x600,local_288,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_DATAPADS");
  local_8._0_1_ = 0x24;
  FUN_0040f620(param_1 + 0x674,local_290,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_WEAPONS");
  local_8._0_1_ = 0x25;
  FUN_0040f620(param_1 + 0x6e8,local_298,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_ARMOR");
  local_8._0_1_ = 0x26;
  FUN_0040f620(param_1 + 0x75c,local_2a0,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_USEABLE");
  local_8._0_1_ = 0x27;
  FUN_0040f620(param_1 + 2000,local_2a8,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_MISC");
  local_8._0_1_ = 0x28;
  FUN_0040f620(param_1 + 0x844,local_2b0,1,1);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  FUN_00733570("BTN_QUESTS");
  local_8._0_1_ = 0x29;
  FUN_0040f620(param_1 + 0x8b8,local_2b8,1,1);
  local_8 = CONCAT31(local_8._1_3_,0x14);
  FUN_00733780();
  FUN_0040f5a0();
  iVar2 = FUN_0073f750();
  local_24 = (uint)*(byte *)(iVar2 + 0xfc1);
  local_28 = 0;
  param_1[0x612] = param_1[0x612] & 0xfffffffb;
  param_1[0x617] = 0;
  FUN_00418af0(0x27,param_1,FUN_008a8cf0);
  if (local_24 == local_28) {
    FUN_00417140(&DAT_00a7fdac);
  }
  else {
    FUN_00417140(&DAT_00a7fe24);
  }
  local_28 = 6;
  param_1[0x686] = param_1[0x686] & 0xfffffffb;
  param_1[0x68b] = 6;
  FUN_00418af0(0x27,param_1,FUN_008a8cf0);
  if (local_24 == local_28) {
    FUN_00417140(&DAT_00a7fdac);
  }
  else {
    FUN_00417140(&DAT_00a7fe24);
  }
  local_28 = 7;
  param_1[0x6fa] = param_1[0x6fa] & 0xfffffffb;
  param_1[0x6ff] = 7;
  FUN_00418af0(0x27,param_1,FUN_008a8cf0);
  if (local_24 == local_28) {
    FUN_00417140(&DAT_00a7fdac);
  }
  else {
    FUN_00417140(&DAT_00a7fe24);
  }
  local_28 = 8;
  param_1[0x76e] = param_1[0x76e] & 0xfffffffb;
  param_1[0x773] = 8;
  FUN_00418af0(0x27,param_1,FUN_008a8cf0);
  if (local_24 == local_28) {
    FUN_00417140(&DAT_00a7fdac);
  }
  else {
    FUN_00417140(&DAT_00a7fe24);
  }
  local_28 = 9;
  param_1[0x7e2] = param_1[0x7e2] & 0xfffffffb;
  param_1[0x7e7] = 9;
  FUN_00418af0(0x27,param_1,FUN_008a8cf0);
  if (local_24 == local_28) {
    FUN_00417140(&DAT_00a7fdac);
  }
  else {
    FUN_00417140(&DAT_00a7fe24);
  }
  local_28 = 10;
  param_1[0x8ca] = param_1[0x8ca] & 0xfffffffb;
  param_1[0x8cf] = 10;
  FUN_00418af0(0x27,param_1,FUN_008a8cf0);
  if (local_24 == local_28) {
    FUN_00417140(&DAT_00a7fdac);
  }
  else {
    FUN_00417140(&DAT_00a7fe24);
  }
  local_28 = 0xb;
  param_1[0x856] = param_1[0x856] & 0xfffffffb;
  param_1[0x85b] = 0xb;
  FUN_00418af0(0x27,param_1,FUN_008a8cf0);
  if (local_24 == local_28) {
    FUN_00417140(&DAT_00a7fdac);
  }
  else {
    FUN_00417140(&DAT_00a7fe24);
  }
  local_20 = param_1[0x2d5];
  (**(code **)(param_1[0x1a] + 0x98))(local_20 + 4,local_20,0x3f800000);
  param_1[0x215] = param_1[0x215] & 0xffffffdf | 0x20;
  FUN_0040ec00(param_1 + 0x162,0);
  iVar2 = FUN_0073f750();
  local_14 = *(byte *)(iVar2 + 0xfc1) + 1;
  if (0xb < local_14) {
    local_14 = 0;
  }
  FUN_00733540();
  local_8._0_1_ = 0x2a;
  FUN_00416ea0(0xbded);
  param_1[0x2ec] = param_1[0x2ec] & 0xfffffffb;
  FUN_00418af0(0x27,param_1,&LAB_00762e90);
  FUN_00760ea0(param_1 + 0x2da,0x62);
  param_1[0x360] = param_1[0x360] & 0xfffffffb;
  FUN_00418af0(0x27,param_1,&LAB_00762e80);
  local_8._0_1_ = 0x14;
  FUN_00733780();
  local_8 = CONCAT31(local_8._1_3_,0x13);
  FUN_0041a3d0();
  local_8 = 0xffffffff;
  ExceptionList = local_10;
  __security_check_cookie(local_2c ^ (uint)&stack0xfffffffc);
  return;
}

