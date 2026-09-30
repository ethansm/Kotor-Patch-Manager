; FN 0073f750 FUN_0073f750
0073f750  55                    PUSH EBP
0073f751  8bec                  MOV EBP,ESP
0073f753  83ec08                SUB ESP,0x8
0073f756  894df8                MOV dword ptr [EBP + -0x8],ECX
0073f759  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0073f75c  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0073f75f  8b5140                MOV EDX,dword ptr [ECX + 0x40]
0073f762  8955fc                MOV dword ptr [EBP + -0x4],EDX
0073f765  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0073f768  8be5                  MOV ESP,EBP
0073f76a  5d                    POP EBP
0073f76b  c3                    RET
