; FN 00401000 FUN_00401000
00401000  55                    PUSH EBP
00401001  8bec                  MOV EBP,ESP
00401003  8b4108                MOV EAX,dword ptr [ECX + 0x8]
00401006  8b10                  MOV EDX,dword ptr [EAX]
00401008  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
0040100b  e8f0000000            CALL 0x00401100
00401010  e8ebf0ffff            CALL 0x00401100
00401015  8b11                  MOV EDX,dword ptr [ECX]
00401017  8b4270                MOV EAX,dword ptr [EDX + 0x70]
0040101a  85c0                  TEST EAX,EAX
0040101c  90                    NOP
0040101e  90                    NOP
00401020  ffd0                  CALL EAX
00401030  e8ebf0ffff            CALL 0x00401200
00401040  ff1584649800          CALL dword ptr [0x00986484]
00401050  c3                    RET
