// FN 0069ac60 FUN_0069ac60 size=598 ret=8 callers= callees=006fdaf0,006fd9c0,0051c080,005453c0,0052bb60,0053dfb0,006fda20

undefined4 FUN_0069ac60(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int **ppiVar4;
  int extraout_var;
  int *local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  int local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  char local_11;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_38 = 0;
  local_34 = 0;
  local_30 = 0;
  local_20 = 0;
  local_1c = 0;
  local_18 = 0;
  local_3c = 0;
  iVar1 = FUN_006fda20(&local_38);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = extraout_var;
    iVar3 = FUN_006fda20(&local_20);
    if (iVar3 == 0) {
      uVar2 = 0xfffff82f;
    }
    else if (*(int *)(iVar1 + 4) == 0) {
      iVar1 = FUN_006fd9c0(local_3c);
      if (iVar1 == 0) {
        uVar2 = 0xfffff830;
      }
      else {
        uVar2 = 0;
      }
    }
    else {
      iVar3 = FUN_006fdaf0(&local_28);
      if (iVar3 == 0) {
        uVar2 = 0xfffff82f;
      }
      else {
        iVar3 = FUN_006fdaf0(&local_40);
        if (iVar3 == 0) {
          uVar2 = 0xfffff82f;
        }
        else {
          local_44 = (int *)0x0;
          ppiVar4 = &local_44;
          uVar2 = *(undefined4 *)(iVar1 + 8);
          FUN_0051c080(uVar2,ppiVar4,iVar1,*(undefined4 *)(DAT_00a1b4a4 + 8));
          local_11 = FUN_0053dfb0(uVar2,ppiVar4);
          if (local_11 == DAT_0099447f) {
            local_2c = 0;
            iVar1 = (**(code **)(*local_44 + 0x28))();
            if (iVar1 == 0) {
              iVar1 = (**(code **)(*local_44 + 0x30))();
              if (iVar1 == 0) {
                iVar1 = (**(code **)(*local_44 + 0x18))();
                if (iVar1 == 0) {
                  iVar1 = (**(code **)(*local_44 + 0x48))();
                  if (iVar1 != 0) {
                    (**(code **)(*local_44 + 0x48))();
                    local_2c = FUN_005453c0();
                  }
                }
                else {
                  (**(code **)(*local_44 + 0x18))();
                  local_2c = FUN_005453c0();
                }
              }
              else {
                (**(code **)(*local_44 + 0x30))();
                local_2c = FUN_005453c0();
              }
            }
            else {
              local_2c = (**(code **)(*local_44 + 0x28))();
            }
            if ((local_2c == 0) && (iVar1 = FUN_006fd9c0(local_3c), iVar1 == 0)) {
              return 0xfffff830;
            }
            local_24 = 0x7f000000;
            local_10 = 0;
            local_c = 0;
            local_8 = 0;
            local_3c = FUN_0052bb60(&local_38,&local_20,&local_10,&local_24,local_28,local_40,0,0);
            iVar1 = FUN_006fd9c0(local_3c);
            if (iVar1 == 0) {
              uVar2 = 0xfffff830;
            }
            else {
              uVar2 = 0;
            }
          }
          else {
            iVar1 = FUN_006fd9c0(local_3c);
            if (iVar1 == 0) {
              uVar2 = 0xfffff830;
            }
            else {
              uVar2 = 0;
            }
          }
        }
      }
    }
  }
  return uVar2;
}

