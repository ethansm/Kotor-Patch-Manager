// FN 005ff950 FUN_005ff950 size=160 ret=32 callers=0052bb60 callees=00650a20,0064e080,0064e110

bool __thiscall
FUN_005ff950(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6,undefined4 param_7,undefined4 *param_8)

{
  int iVar1;
  bool bVar2;
  undefined1 local_28 [16];
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  
  if (*(int *)(param_1 + 0x38) == 0) {
    FUN_0064e080(0xffffffff);
    FUN_0064e110(0xffffffff);
    iVar1 = FUN_00650a20(param_2,param_3,param_4,param_5,param_6,param_7,local_28);
    if ((iVar1 != 0) && (param_8 != (undefined4 *)0x0)) {
      *param_8 = local_18;
      param_8[1] = local_14;
      param_8[2] = local_10;
    }
    bVar2 = iVar1 == 0;
  }
  else {
    bVar2 = true;
  }
  return bVar2;
}

