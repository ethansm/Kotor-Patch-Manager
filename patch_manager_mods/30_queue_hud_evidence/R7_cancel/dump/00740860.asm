; FN 00740860 FUN_00740860
00740860  55                    PUSH EBP
00740861  8bec                  MOV EBP,ESP
00740863  83ec08                SUB ESP,0x8
00740866  894df8                MOV dword ptr [EBP + -0x8],ECX
00740869  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0074086c  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0074086f  8b9120030000          MOV EDX,dword ptr [ECX + 0x320]
00740875  8955fc                MOV dword ptr [EBP + -0x4],EDX
00740878  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0074087b  8be5                  MOV ESP,EBP
0074087d  5d                    POP EBP
0074087e  c3                    RET
