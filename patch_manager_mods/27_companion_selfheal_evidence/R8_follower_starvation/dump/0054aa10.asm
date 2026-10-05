; FN 0054aa10 FUN_0054aa10
0054aa10  55                    PUSH EBP
0054aa11  8bec                  MOV EBP,ESP
0054aa13  51                    PUSH ECX
0054aa14  894dfc                MOV dword ptr [EBP + -0x4],ECX
0054aa17  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0054aa1a  50                    PUSH EAX
0054aa1b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0054aa1e  8b09                  MOV ECX,dword ptr [ECX]
0054aa20  e87bf31e00            CALL 0x00739da0
0054aa25  8be5                  MOV ESP,EBP
0054aa27  5d                    POP EBP
0054aa28  c20400                RET 0x4
