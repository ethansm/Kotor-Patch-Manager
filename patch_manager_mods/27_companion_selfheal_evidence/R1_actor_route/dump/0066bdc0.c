// FN 0066bdc0 FUN_0066bdc0 size=465 ret=8 callers= callees=006fdaf0,006fdab0,00623320,0051c100,009196fd,0057da90,0057d8b0

undefined4 __thiscall FUN_0066bdc0(int param_1,int param_2)

{
  int iVar1;
  int local_30;
  undefined4 local_2c;
  int local_28;
  int local_24;
  undefined1 local_1d;
  int local_1c;
  int local_18;
  int *local_14;
  undefined1 local_d;
  int local_c;
  undefined1 local_5;
  
  iVar1 = FUN_006fdab0(3,&local_14);
  if (iVar1 == 0) {
    return 0xfffff82f;
  }
  local_c = *local_14;
  if (-1 < local_c) {
    local_24 = local_14[1];
    local_5 = (undefined1)local_14[2];
    local_1c = local_14[3];
    local_18 = local_14[4];
    local_d = (undefined1)local_14[5];
    local_1d = *(undefined1 *)((int)local_14 + 0x15);
    local_28 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
    if (local_28 != 0) {
      if (param_2 == 0x135) {
        local_2c = 0x7f000000;
        iVar1 = FUN_006fdaf0(&local_2c);
        if (iVar1 == 0) {
          return 0xfffff82f;
        }
        FUN_0057d8b0(local_c,local_24,local_2c,local_5,local_1c,local_18,local_d,local_1d);
      }
      else if (param_2 == 0x136) {
        iVar1 = FUN_006fdab0(2,&local_30);
        if (iVar1 == 0) {
          return 0xfffff82f;
        }
        FUN_0057da90(local_c,local_24,local_30,local_5,local_1c,local_18,local_d,local_1d);
        if (local_30 != 0) {
          FUN_00623320();
          FUN_009196fd(local_30);
        }
      }
    }
  }
  if (local_14 != (int *)0x0) {
    FUN_00623320();
    FUN_009196fd(local_14);
  }
  return 0;
}

