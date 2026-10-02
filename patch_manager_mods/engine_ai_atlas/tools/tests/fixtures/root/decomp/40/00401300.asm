; FN 00401300 FUN_00401300
00401300  55                    PUSH EBP
00401310  e8ebf0ffff            CALL 0x00401000
00401312  8b0a                  MOV ECX,dword ptr [EDX]
00401314  e800000000            CALL 0x00401500
00401319  8b5070                MOV EDX,dword ptr [EAX + 0x70]
00401320  ffd2                  CALL EDX
00401330  c3                    RET
