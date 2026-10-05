; FN 0051c720 FUN_0051c720
0051c720  55                    PUSH EBP
0051c721  8bec                  MOV EBP,ESP
0051c723  51                    PUSH ECX
0051c724  894dfc                MOV dword ptr [EBP + -0x4],ECX
0051c727  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
0051c72b  50                    PUSH EAX
0051c72c  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051c72f  8b4904                MOV ECX,dword ptr [ECX + 0x4]
0051c732  e899c50100            CALL 0x00538cd0
0051c737  8be5                  MOV ESP,EBP
0051c739  5d                    POP EBP
0051c73a  c20400                RET 0x4
