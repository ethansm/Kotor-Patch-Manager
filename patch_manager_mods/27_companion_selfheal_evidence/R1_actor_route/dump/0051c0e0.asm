; FN 0051c0e0 FUN_0051c0e0
0051c0e0  55                    PUSH EBP
0051c0e1  8bec                  MOV EBP,ESP
0051c0e3  51                    PUSH ECX
0051c0e4  894dfc                MOV dword ptr [EBP + -0x4],ECX
0051c0e7  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0051c0ea  50                    PUSH EAX
0051c0eb  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051c0ee  8b4904                MOV ECX,dword ptr [ECX + 0x4]
0051c0f1  e80aba0100            CALL 0x00537b00
0051c0f6  8be5                  MOV ESP,EBP
0051c0f8  5d                    POP EBP
0051c0f9  c20400                RET 0x4
