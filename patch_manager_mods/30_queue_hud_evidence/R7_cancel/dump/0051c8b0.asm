; FN 0051c8b0 FUN_0051c8b0
0051c8b0  55                    PUSH EBP
0051c8b1  8bec                  MOV EBP,ESP
0051c8b3  51                    PUSH ECX
0051c8b4  894dfc                MOV dword ptr [EBP + -0x4],ECX
0051c8b7  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0051c8ba  50                    PUSH EAX
0051c8bb  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051c8be  8b4904                MOV ECX,dword ptr [ECX + 0x4]
0051c8c1  e86ad40100            CALL 0x00539d30
0051c8c6  8be5                  MOV ESP,EBP
0051c8c8  5d                    POP EBP
0051c8c9  c20400                RET 0x4
