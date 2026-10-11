// FN 004133c0 FUN_004133c0 size=84 ret=0 callers=007b3830 callees=

undefined4 __fastcall FUN_004133c0(int param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x10) != 0) {
    if (*(int *)(param_1 + 0xc) == 4) {
      (**(code **)(**(int **)(param_1 + 0x10) + 0x24))();
      local_8 = 1;
    }
    else {
      local_8 = 0;
    }
  }
  return local_8;
}

