// FN 0057d8b0 FUN_0057d8b0 size=476 ret=32 callers=0066bdc0,0065be70 callees=006c06b0,0051c100,0057b4c0,00565310,00568540,005453c0,0057b280

undefined4 __thiscall
FUN_0057d8b0(int param_1,int param_2,uint param_3,undefined4 param_4,undefined1 param_5,int param_6,
            uint param_7,undefined1 param_8,undefined1 param_9)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined1 local_8 [4];
  
  iVar1 = FUN_0051c100(param_4);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else if (*(int *)(param_1 + 0xec) == 0) {
    uVar2 = 0;
  }
  else {
    iVar3 = FUN_005453c0();
    if (iVar3 == 0) {
      uVar2 = 0;
    }
    else {
      local_1c = *(undefined4 *)(iVar1 + 0x94);
      local_18 = *(undefined4 *)(iVar1 + 0x98);
      local_14 = *(undefined4 *)(iVar1 + 0x9c);
      iVar1 = FUN_005453c0();
      local_10 = *(undefined4 *)(iVar1 + 0x124);
      if (param_2 == 0) {
        if ((param_6 == 0x7f000000) || (param_7 == 0xffffffff)) {
          iVar1 = FUN_006c06b0(param_3,&local_c,local_8);
          if (iVar1 == 1) {
            FUN_0057b4c0(local_c,local_8[0],0,param_4,&local_1c,local_10,0);
          }
          else {
            FUN_00565310(param_3,param_5,0,param_9,0,local_1c,local_18,local_14,param_4,0,0,0,0,0,0,
                         0xffffffff,param_8,0);
          }
        }
        else {
          FUN_0057b4c0(param_6,param_7 & 0xff,0,param_4,&local_1c,local_10,0);
        }
      }
      else if (param_2 == 1) {
        FUN_00568540(param_4,param_3 & 0xffff,0,0,1,0,0,1,0x2719,0x5dc,1,0,0,4,0);
      }
      else if (param_2 == 2) {
        FUN_0057b280(param_3 & 0xff,0,param_4,&local_1c,local_10,0x7f000000,0);
      }
      uVar2 = 1;
    }
  }
  return uVar2;
}

