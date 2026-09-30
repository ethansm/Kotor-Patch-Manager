// FN 005b7ae0 FUN_005b7ae0 size=1302 ret=8 callers=0065b4f0,00566650,007469f0,006dab20,005d25e0,0057b940,0065b270,0058e990,0056e620 callees=00563da0,006d0620,00584da0,006d5200

void __thiscall FUN_005b7ae0(int param_1,char param_2,int param_3)

{
  bool bVar1;
  int iVar2;
  char local_11;
  int local_8;
  
  bVar1 = false;
  local_11 = '\0';
  if (param_3 == 1) {
    local_11 = *(char *)(param_1 + 0x512);
    if (param_2 == '\x05') {
      for (local_8 = 0; local_8 < (int)(uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0x8d);
          local_8 = local_8 + 1) {
      }
      iVar2 = FUN_006d0620(0x10);
      if (iVar2 != 0) {
        FUN_00584da0(0xbc,0);
        return;
      }
    }
    else if (param_2 == '\x06') {
      iVar2 = FUN_006d0620(0x10);
      if (iVar2 == 0) {
        FUN_00584da0(0xbc,0);
        return;
      }
      iVar2 = FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
      if (*(char *)(iVar2 + 0x1a) == '\0') {
        FUN_00584da0(0xbc,0);
        return;
      }
    }
    else if ((((param_2 == '\x02') || (param_2 == '\x03')) || (param_2 == '\x01')) &&
            ((iVar2 = FUN_006d0620(0x10), iVar2 != 0 &&
             (iVar2 = FUN_006d5200(*(undefined4 *)(iVar2 + 0xc)), *(char *)(iVar2 + 0x1a) != '\0')))
            ) {
      FUN_00584da0(0xbc,0);
      return;
    }
    *(char *)(param_1 + 0x512) = param_2;
    bVar1 = true;
  }
  else if ((param_2 == '\x04') && (*(char *)(param_1 + 0x512) != '\x04')) {
    local_11 = *(char *)(param_1 + 0x512);
    *(undefined1 *)(param_1 + 0x512) = 4;
    bVar1 = true;
  }
  else if (param_2 != *(char *)(param_1 + 0x512)) {
    if (*(int *)(*(int *)(param_1 + 0x10dc) + 0xa84) == 1) {
      *(char *)(param_1 + 0x513) = param_2;
    }
    else {
      if (param_2 == '\x05') {
        for (local_8 = 0; local_8 < (int)(uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0x8d);
            local_8 = local_8 + 1) {
        }
        iVar2 = FUN_006d0620(0x10);
        if (iVar2 != 0) {
          FUN_00584da0(0xbc,0);
          return;
        }
      }
      else if (param_2 == '\x06') {
        iVar2 = FUN_006d0620(0x10);
        if (iVar2 == 0) {
          FUN_00584da0(0xbc,0);
          return;
        }
        iVar2 = FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
        if (*(char *)(iVar2 + 0x1a) == '\0') {
          FUN_00584da0(0xbc,0);
          return;
        }
      }
      local_11 = *(char *)(param_1 + 0x512);
      *(char *)(param_1 + 0x512) = param_2;
      bVar1 = true;
    }
  }
  if (bVar1) {
    if ((param_2 == '\x01') || (local_11 == '\x01')) {
      if (param_2 == '\x01') {
        FUN_00563da0(0x400,1,0);
      }
      else {
        FUN_00563da0(0x400,0,0);
      }
    }
    else if ((param_2 == '\x02') || (local_11 == '\x02')) {
      if (param_2 == '\x02') {
        FUN_00563da0(0x100,1,0);
      }
      else {
        FUN_00563da0(0x100,0,0);
      }
    }
    else if ((param_2 == '\x03') || (local_11 == '\x03')) {
      if (param_2 == '\x03') {
        FUN_00563da0(0x200,1,0);
      }
      else {
        FUN_00563da0(0x200,0,0);
      }
    }
    else if ((param_2 == '\x04') || (local_11 == '\x04')) {
      if (param_2 == '\x04') {
        FUN_00563da0(0x800,1,0);
      }
      else {
        FUN_00563da0(0x800,0,0);
      }
    }
    else if ((param_2 == '\x05') || (local_11 == '\x05')) {
      if (param_2 == '\x05') {
        FUN_00563da0(0x1000,1,0);
      }
      else {
        FUN_00563da0(0x1000,0,0);
      }
    }
    else if ((param_2 == '\x06') || (local_11 == '\x06')) {
      if (param_2 == '\x06') {
        FUN_00563da0(0x2000,1,0);
      }
      else {
        FUN_00563da0(0x2000,0,0);
      }
    }
  }
  return;
}

