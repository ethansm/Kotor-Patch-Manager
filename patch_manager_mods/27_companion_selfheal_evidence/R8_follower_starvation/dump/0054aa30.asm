; FN 0054aa30 FUN_0054aa30
0054aa30  55                    PUSH EBP
0054aa31  8bec                  MOV EBP,ESP
0054aa33  51                    PUSH ECX
0054aa34  894dfc                MOV dword ptr [EBP + -0x4],ECX
0054aa37  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0054aa3a  50                    PUSH EAX
0054aa3b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0054aa3e  8b09                  MOV ECX,dword ptr [ECX]
0054aa40  e81bf41e00            CALL 0x00739e60
0054aa45  8be5                  MOV ESP,EBP
0054aa47  5d                    POP EBP
0054aa48  c20400                RET 0x4
