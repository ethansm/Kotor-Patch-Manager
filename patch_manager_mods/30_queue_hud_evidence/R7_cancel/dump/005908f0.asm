; FN 005908f0 FUN_005908f0
005908f0  55                    PUSH EBP
005908f1  8bec                  MOV EBP,ESP
005908f3  83ec08                SUB ESP,0x8
005908f6  894df8                MOV dword ptr [EBP + -0x8],ECX
005908f9  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00590900  eb09                  JMP 0x0059090b
00590902  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00590905  83c001                ADD EAX,0x1
00590908  8945fc                MOV dword ptr [EBP + -0x4],EAX
0059090b  837dfc05              CMP dword ptr [EBP + -0x4],0x5
0059090f  7d15                  JGE 0x00590926
00590911  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00590914  51                    PUSH ECX
00590915  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00590918  e8a3ffffff            CALL 0x005908c0
0059091d  8bc8                  MOV ECX,EAX
0059091f  e85cc4ffff            CALL 0x0058cd80
00590924  ebdc                  JMP 0x00590902
00590926  8be5                  MOV ESP,EBP
00590928  5d                    POP EBP
00590929  c3                    RET
