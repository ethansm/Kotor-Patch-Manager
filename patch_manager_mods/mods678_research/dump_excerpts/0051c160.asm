; FN 0051c160 FUN_0051c160
0051c160  55                    PUSH EBP
0051c161  8bec                  MOV EBP,ESP
0051c163  51                    PUSH ECX
0051c164  894dfc                MOV dword ptr [EBP + -0x4],ECX
0051c167  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0051c16a  50                    PUSH EAX
0051c16b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051c16e  8b4904                MOV ECX,dword ptr [ECX + 0x4]
0051c171  e8babb0100            CALL 0x00537d30
0051c176  8be5                  MOV ESP,EBP
0051c178  5d                    POP EBP
0051c179  c20400                RET 0x4
