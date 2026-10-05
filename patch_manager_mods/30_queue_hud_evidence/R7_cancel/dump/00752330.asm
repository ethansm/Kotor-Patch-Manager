; FN 00752330 FUN_00752330
00752330  55                    PUSH EBP
00752331  8bec                  MOV EBP,ESP
00752333  83ec08                SUB ESP,0x8
00752336  894df8                MOV dword ptr [EBP + -0x8],ECX
00752339  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0075233c  8b4850                MOV ECX,dword ptr [EAX + 0x50]
0075233f  894dfc                MOV dword ptr [EBP + -0x4],ECX
00752342  837dfc00              CMP dword ptr [EBP + -0x4],0x0
00752346  7408                  JZ 0x00752350
00752348  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0075234b  e810000000            CALL 0x00752360
00752350  8be5                  MOV ESP,EBP
00752352  5d                    POP EBP
00752353  c20400                RET 0x4
