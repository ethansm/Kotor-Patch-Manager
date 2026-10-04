// FN 0087a350 FUN_0087a350 size=182 ret=16 callers=00879cc0,008798a0,0087a410,0087aec0,0087a330,00879e80,0087a770,0087a2a0,0087a530,00879e00,0087a550,0087a5c0,00879e60,0087a430,00879a20,00879910,00879930,00879d90,008798c0,00879c50,00879af0,008799a0,0087aea0,0087a4b0,00879d20,00879f10,008074e0 callees=009196fd,00919723,00704060

undefined4 __thiscall
FUN_0087a350(undefined4 param_1,undefined1 param_2,undefined1 param_3,undefined1 *param_4,
            int param_5)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined4 uVar3;
  undefined1 *local_10;
  int local_8;
  
  if (param_5 == 0) {
    local_8 = 3;
    local_10 = (undefined1 *)FUN_00919723(3);
  }
  else {
    local_8 = param_5;
    local_10 = param_4;
  }
  if (local_10 == (undefined1 *)0x0) {
    uVar1 = 0;
  }
  else {
    *local_10 = 0x70;
    local_10[1] = param_2;
    local_10[2] = param_3;
    uVar3 = 0;
    uVar1 = DAT_0099b6c0;
    puVar2 = local_10;
    (**(code **)(**(int **)(DAT_00a1b4a4 + 4) + 0x10))
              (DAT_0099b6c0,local_10,local_8,0,*(int **)(DAT_00a1b4a4 + 4),param_1);
    uVar1 = FUN_00704060(uVar1,puVar2,local_8,uVar3);
    if (param_5 == 0) {
      FUN_009196fd(local_10);
    }
  }
  return uVar1;
}

