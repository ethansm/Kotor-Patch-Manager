; FN 00538cd0 FUN_00538cd0
00538cd0  55                    PUSH EBP
00538cd1  8bec                  MOV EBP,ESP
00538cd3  51                    PUSH ECX
00538cd4  894dfc                MOV dword ptr [EBP + -0x4],ECX
00538cd7  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00538cda  0fb68878000100        MOVZX ECX,byte ptr [EAX + 0x10078]
00538ce1  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
00538ce5  23ca                  AND ECX,EDX
00538ce7  33c0                  XOR EAX,EAX
00538ce9  85c9                  TEST ECX,ECX
00538ceb  0f9fc0                SETG AL
00538cee  8be5                  MOV ESP,EBP
00538cf0  5d                    POP EBP
00538cf1  c20400                RET 0x4
