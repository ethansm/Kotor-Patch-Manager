; FN 0051cba0 FUN_0051cba0
0051cba0  55                    PUSH EBP
0051cba1  8bec                  MOV EBP,ESP
0051cba3  83ec0c                SUB ESP,0xc
0051cba6  894df4                MOV dword ptr [EBP + -0xc],ECX
0051cba9  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0051cbac  50                    PUSH EAX
0051cbad  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0051cbb0  e84bf5ffff            CALL 0x0051c100
0051cbb5  8945fc                MOV dword ptr [EBP + -0x4],EAX
0051cbb8  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0051cbbc  7414                  JZ 0x0051cbd2
0051cbbe  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051cbc1  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
0051cbc7  8955f8                MOV dword ptr [EBP + -0x8],EDX
0051cbca  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0051cbcd  e85e380700            CALL 0x00590430
0051cbd2  8be5                  MOV ESP,EBP
0051cbd4  5d                    POP EBP
0051cbd5  c20400                RET 0x4
