// FN 0073a200 FUN_0073a200 size=151 ret=0 callers=007f2a90 callees=0073a560,009196fd

undefined4 __fastcall FUN_0073a200(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if (*param_1 == 0) {
    FUN_0073a560(0x165,".\\exolinkedlistinternal.cpp",
                 "CExoLinkedListInternal::RemoveHead() Removing from empty List");
    uVar2 = 0;
  }
  else {
    uVar2 = *(undefined4 *)(*param_1 + 8);
    iVar1 = *param_1;
    *param_1 = *(int *)(iVar1 + 4);
    if (*param_1 == 0) {
      param_1[1] = 0;
    }
    else {
      *(undefined4 *)*param_1 = 0;
    }
    FUN_009196fd(iVar1);
    param_1[2] = param_1[2] + -1;
  }
  return uVar2;
}

