// FN 006da4f0 FUN_006da4f0 size=1290 ret=12 callers=0058e990 callees=0044b8a0,00590a50,0057c990,00740640,006dac70,004452b0,0073fec0,0051c100,00571540,00590d10,006dab20,0051c0a0,007a1780,005e13c0

void __thiscall FUN_006da4f0(int *param_1,int param_2,int param_3,undefined4 param_4)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  int *piVar4;
  float10 fVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  int local_30;
  undefined1 local_28 [12];
  float local_1c;
  int *local_18;
  int *local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_14 = (int *)0x0;
  local_10 = 0;
  local_c = 0;
  if (param_3 == 0) {
    local_8 = *(int *)(param_1[0x437] + 0xb20);
  }
  else {
    local_8 = param_1[0x15a];
  }
  if ((local_8 == 0x7f000000) || (iVar3 = FUN_007a1780(), iVar3 == 0)) {
    if ((local_8 == 0x7f000000) && (iVar3 = FUN_007a1780(), iVar3 != 0)) {
      param_2 = 1;
    }
  }
  else {
    local_10 = 1;
    local_18 = (int *)FUN_0051c0a0(local_8);
    if ((local_18 == (int *)0x0) || (iVar3 = (**(code **)(*local_18 + 0x30))(), iVar3 == 0)) {
      if (local_18 == (int *)0x0) {
        *(undefined4 *)(param_1[0x437] + 0xb20) = 0x7f000000;
        local_10 = 0;
      }
    }
    else {
      local_14 = (int *)(**(code **)(*local_18 + 0x30))();
      iVar3 = (**(code **)(*local_14 + 0x94))();
      if (iVar3 == 0) {
        if (local_14[0x46b] == 0) {
          bVar1 = false;
        }
        else {
          sVar2 = (**(code **)(*local_14 + 0x9c))(0);
          bVar1 = sVar2 < 1;
        }
        if ((!bVar1) && (iVar3 = (**(code **)(*param_1 + 0x94))(), iVar3 == 0)) {
          if (param_1[0x46b] == 0) {
            bVar1 = false;
          }
          else {
            sVar2 = (**(code **)(*param_1 + 0x9c))(0);
            bVar1 = sVar2 < 1;
          }
          if (!bVar1) {
            iVar3 = FUN_005e13c0(local_8,0);
            if (iVar3 < 0xb) {
              if (param_3 != 0) {
                FUN_004452b0(local_28,local_14 + 0x25,param_1 + 0x25);
                fVar5 = (float10)FUN_00571540(local_14[1],0,1);
                local_1c = (float)fVar5;
                fVar5 = (float10)FUN_0044b8a0(local_28,local_28);
                if ((float10)local_1c * (float10)local_1c < fVar5) {
                  local_10 = 0;
                }
              }
            }
            else if (local_8 == param_1[1]) {
              local_10 = 1;
              local_c = 1;
            }
            goto LAB_006da79f;
          }
        }
      }
      local_10 = 0;
    }
  }
LAB_006da79f:
  if (param_2 != 0) {
    local_c = 1;
    local_10 = 1;
  }
  if (local_c != 0) {
    iVar3 = FUN_0051c100(param_4);
    local_30 = 0x7f000000;
    if (iVar3 != 0) {
      iVar3 = param_1[1];
      iVar6 = param_1[1];
      fVar5 = (float10)FUN_00571540(param_1[1],0,1);
      local_30 = FUN_0057c990((float)fVar5,iVar6,iVar3);
    }
    if (local_30 == 0x7f000000) {
      iVar3 = param_1[1];
      uVar7 = 0x7f000000;
      fVar5 = (float10)FUN_00571540(param_1[1],0,1);
      local_30 = FUN_0057c990((float)fVar5,iVar3,uVar7);
    }
    if (local_30 == 0x7f000000) {
      FUN_006dab20(1);
      local_10 = 0;
    }
    else {
      piVar4 = (int *)FUN_0051c0a0(local_30);
      iVar3 = (**(code **)(*piVar4 + 0x30))();
      if (iVar3 == 0) {
        FUN_006dab20(1);
        local_10 = 0;
      }
      else {
        FUN_006dac70(local_30,0);
        local_8 = local_30;
        uVar8 = 1;
        uVar7 = FUN_0073fec0(local_30);
        FUN_00740640(uVar7,uVar8);
      }
    }
  }
  if (local_10 != 0) {
    if (*(char *)(param_1[0x437] + 0xb1c) == '\r') {
      FUN_00590d10(0,local_8,0);
    }
    else {
      FUN_00590a50(local_8,0,0,0x5dc,1);
      if (param_1[0x155] == local_8) {
        param_1[0x155] = 0x7f000000;
      }
    }
  }
  return;
}

