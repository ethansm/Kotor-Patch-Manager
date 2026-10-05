// FN 0077fd20 FUN_0077fd20 size=110 ret=4 callers=007523e0,007794e0,0077ce00,00586750,0079d840,007b12c0,0077cdd0,0077f660,007445d0 callees=0073f470,00740880

void __thiscall FUN_0077fd20(int param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  
  *(uint *)(param_1 + 0x458) = *(uint *)(param_1 + 0x458) & 0xfffffffe | param_2 & 1;
  iVar1 = *(int *)(param_1 + 4);
  iVar2 = FUN_0073f470();
  if (iVar1 == iVar2) {
    FUN_00740880(*(uint *)(param_1 + 0x458) & 1);
  }
  return;
}

