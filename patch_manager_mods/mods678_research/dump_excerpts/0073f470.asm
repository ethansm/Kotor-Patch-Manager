; FN 0073f470 FUN_0073f470
0073f470  55                    PUSH EBP
0073f471  8bec                  MOV EBP,ESP
0073f473  83ec08                SUB ESP,0x8
0073f476  894df8                MOV dword ptr [EBP + -0x8],ECX
0073f479  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0073f47c  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0073f47f  8b5120                MOV EDX,dword ptr [ECX + 0x20]
0073f482  8955fc                MOV dword ptr [EBP + -0x4],EDX
0073f485  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0073f488  8be5                  MOV ESP,EBP
0073f48a  5d                    POP EBP
0073f48b  c3                    RET
