// FN 0072e030 FUN_0072e030 size=1743 ret=4 callers=0072fbd0 callees=0072e730

undefined4 __thiscall FUN_0072e030(int param_1,int param_2)

{
  undefined4 uVar1;
  uint uVar2;
  undefined4 local_18;
  int local_14;
  undefined4 local_8;
  
  switch(*(undefined4 *)(param_2 + 4)) {
  case 0x200:
    if (*(int *)(param_1 + 0x3ec) != 0) {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
      return 1;
    }
    *(int *)(param_1 + 0x3e4) = (int)(short)*(undefined4 *)(param_2 + 0xc);
    *(int *)(param_1 + 1000) = (int)(short)((uint)*(undefined4 *)(param_2 + 0xc) >> 0x10);
    return 1;
  case 0x201:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 1;
    local_18 = DAT_0099c89c;
    break;
  case 0x202:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 0;
    local_18 = DAT_0099c89c;
    break;
  case 0x203:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = -1;
    local_18 = DAT_0099c89c;
    break;
  case 0x204:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 1;
    local_18 = DAT_0099c8a0;
    break;
  case 0x205:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 0;
    local_18 = DAT_0099c8a0;
    break;
  case 0x206:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = -1;
    local_18 = DAT_0099c8a0;
    break;
  case 0x207:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar2 = *(uint *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,*(uint *)(param_2 + 0xc) & 0xffff);
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,uVar2 >> 0x10);
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 1;
    local_18 = DAT_0099c8a4;
    break;
  case 0x208:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar2 = *(uint *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,*(uint *)(param_2 + 0xc) & 0xffff);
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,uVar2 >> 0x10);
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 0;
    local_18 = DAT_0099c8a4;
    break;
  default:
    return 0;
  case 0x20a:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 1000) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = (int)(short)((uint)*(undefined4 *)(param_2 + 8) >> 0x10);
    local_18 = DAT_0099c8b0;
  }
  DAT_0099c888 = local_8;
  FUN_0072e730(local_8,local_18,local_14);
  return 1;
}

