// FN 00418af0 FUN_00418af0 size=177 ret=12 callers=0075b370,008c7aa0,00880740,008d8c50,00893950,008a1170,00892090,00918b10,008c17f0,007d3750,0088aac0,009074e0,007d6170,00747210,008b4270,0084c3a0,008d1020,00882c00,0089c5f0,007d8540,0075ae40,008bba80,007d23e0,00895cc0,008b9fd0,00760210,008c7510,007d9800,008c80f0,008b7da0,008c9e10,00850770,007d4620,007dc410,007fc880,008fb7a0,00916ef0,007dacb0,008b7ed0,00886fc0,008a92d0,009153d0,00757c40,008e1810,008df7f0,008586d0,008db630,008f92b0,008fd950,008b01b0,00910490,008b91f0,008fc8e0,008bfe30,008b1ea0,007450e0,008973d0,00916640,008a75f0,008557c0,008b2f70,008b5db0,00902f60,008519f0,008d3900,008fff20,008c9030,0089cf30,008df930,007fae60,008c9c40,00754ed0,008c6650,008bc620,00901be0,00914000,008fbef0,008a25c0,008d6b90,007d31d0,0090c6a0,0075cec0,008ce930,00741940,008a6170,00887da0,00909e00 callees=007a1940,008a5fb0

void __thiscall FUN_00418af0(int param_1,int param_2,undefined4 param_3,int param_4)

{
  undefined4 *puVar1;
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x40) <= local_c) {
      if (param_4 != 0) {
        FUN_008a5fb0(param_3,param_4,param_2);
      }
      return;
    }
    puVar1 = (undefined4 *)(local_c * 0xc + *(int *)(param_1 + 0x3c));
    if (puVar1[2] == param_2) break;
    local_c = local_c + 1;
  }
  if (param_4 != 0) {
    *puVar1 = param_3;
    puVar1[1] = param_4;
    return;
  }
  FUN_007a1940(local_c);
  return;
}

