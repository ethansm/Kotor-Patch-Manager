; FN 00842d00 FUN_00842d00
00842d00  55                    PUSH EBP
00842d01  8bec                  MOV EBP,ESP
00842d03  83ec08                SUB ESP,0x8
00842d06  894dfc                MOV dword ptr [EBP + -0x4],ECX
00842d09  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00842d0c  e82ff8faff            CALL 0x007f2540
00842d11  85c0                  TEST EAX,EAX
00842d13  741a                  JZ 0x00842d2f
00842d15  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00842d18  e823f8faff            CALL 0x007f2540
00842d1d  8945f8                MOV dword ptr [EBP + -0x8],EAX
00842d20  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00842d23  8b10                  MOV EDX,dword ptr [EAX]
00842d25  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00842d28  8b4248                MOV EAX,dword ptr [EDX + 0x48]
00842d2b  ffd0                  CALL EAX
00842d2d  eb02                  JMP 0x00842d31
00842d2f  33c0                  XOR EAX,EAX
00842d31  8be5                  MOV ESP,EBP
00842d33  5d                    POP EBP
00842d34  c3                    RET
