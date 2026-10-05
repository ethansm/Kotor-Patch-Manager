; FN 0073fb90 FUN_0073fb90
0073fb90  55                    PUSH EBP
0073fb91  8bec                  MOV EBP,ESP
0073fb93  83ec08                SUB ESP,0x8
0073fb96  894df8                MOV dword ptr [EBP + -0x8],ECX
0073fb99  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0073fb9c  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0073fb9f  8b9170020000          MOV EDX,dword ptr [ECX + 0x270]
0073fba5  8955fc                MOV dword ptr [EBP + -0x4],EDX
0073fba8  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0073fbab  8be5                  MOV ESP,EBP
0073fbad  5d                    POP EBP
0073fbae  c3                    RET
