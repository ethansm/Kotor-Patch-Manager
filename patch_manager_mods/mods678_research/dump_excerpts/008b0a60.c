// NAMED 0x008b0a60 CSWGuiInGameItemEntry::Constructor evidence=ev_1a3c2b237a698cf64bae6a915368abdda04643cc45c7c3fbc5b3341f8e194051 names=3 exe=6a522e71631d

undefined4 * CSWGuiInGameItemEntry__Constructor(void)

{
  undefined4 *in_ECX;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00978ba2;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0041a510(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  *in_ECX = CSWGuiRadioButton::vftable;
  local_8 = 0;
  *in_ECX = CSWGuiInGameItemEntry::vftable;
  CSWGuiBorder__Constructor();
  local_8._0_1_ = 1;
  CSWGuiBorder__Constructor();
  local_8._0_1_ = 2;
  CSWGuiBorder__Constructor();
  local_8 = CONCAT31(local_8._1_3_,3);
  CSWGuiText__Constructor();
  in_ECX[0x74] = 0x7f000000;
  in_ECX[0xeb] = in_ECX[0xeb] & 0xfffffffe;
  in_ECX[0xeb] = in_ECX[0xeb] & 0xfffffffd;
  in_ECX[0xeb] = in_ECX[0xeb] & 0xfffffffb;
  *(undefined1 *)(in_ECX + 0xec) = 0;
  ExceptionList = local_10;
  return in_ECX;
}

