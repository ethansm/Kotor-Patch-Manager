// FN 007b3ee0 FUN_007b3ee0 size=448 ret=8 callers=007b3a00 callees=

/* WARNING: Removing unreachable block (ram,0x007b3fcd) */
/* WARNING: Removing unreachable block (ram,0x007b401d) */
/* WARNING: Removing unreachable block (ram,0x007b402e) */
/* WARNING: Removing unreachable block (ram,0x007b4075) */
/* WARNING: Removing unreachable block (ram,0x007b405a) */
/* WARNING: Removing unreachable block (ram,0x007b4063) */
/* WARNING: Removing unreachable block (ram,0x007b406c) */
/* WARNING: Removing unreachable block (ram,0x007b3fe2) */
/* WARNING: Removing unreachable block (ram,0x007b4051) */
/* WARNING: Removing unreachable block (ram,0x007b3feb) */
/* WARNING: Removing unreachable block (ram,0x007b4012) */
/* WARNING: Removing unreachable block (ram,0x007b403f) */
/* WARNING: Removing unreachable block (ram,0x007b401b) */
/* WARNING: Removing unreachable block (ram,0x007b3ff4) */
/* WARNING: Removing unreachable block (ram,0x007b4048) */
/* WARNING: Removing unreachable block (ram,0x007b3ffd) */
/* WARNING: Removing unreachable block (ram,0x007b4087) */
/* WARNING: Removing unreachable block (ram,0x007b4007) */
/* WARNING: Removing unreachable block (ram,0x007b407e) */
/* WARNING: Removing unreachable block (ram,0x007b4010) */
/* WARNING: Removing unreachable block (ram,0x007b4090) */
/* WARNING: Recovered jumptable eliminated as dead code */

undefined4 FUN_007b3ee0(uint param_1)

{
  undefined4 local_8;
  
  param_1 = param_1 & 0xfffffff;
  if (param_1 < 0x3f3) {
    if (param_1 == 0x3f2) {
      return 0x17;
    }
    if (param_1 < 0x3eb) {
      if (param_1 == 0x3ea) {
        return 0xb;
      }
      if (param_1 == 0xffffffff) {
        return 5;
      }
      if (param_1 == 0x3e9) {
        return 0x19;
      }
    }
    else if (param_1 == 0x3eb) {
      return 0x33;
    }
switchD_007b3f69_caseD_3f6:
    local_8 = 5;
  }
  else {
    switch(param_1) {
    case 0x3f4:
      local_8 = 0x21;
      break;
    case 0x3f5:
      local_8 = 7;
      break;
    default:
      goto switchD_007b3f69_caseD_3f6;
    case 0x3f7:
      local_8 = 0x19;
      break;
    case 0x402:
      local_8 = 0x25;
    }
  }
  return local_8;
}

