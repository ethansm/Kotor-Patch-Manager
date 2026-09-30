// FN 007a0530 FUN_007a0530 size=70 ret=8 callers=007b3ca0,00740640 callees=

void __thiscall FUN_007a0530(int param_1,int param_2,uint param_3)

{
  *(uint *)(param_1 + 0x37c) =
       *(uint *)(param_1 + 0x37c) & 0xfffffffe |
       -(uint)(*(int *)(param_1 + 0x2b4) != param_2) & param_3 & 1;
  *(int *)(param_1 + 0x2b4) = param_2;
  return;
}

