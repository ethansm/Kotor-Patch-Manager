; FN 00752360 FUN_00752360
00752360  55                    PUSH EBP
00752361  8bec                  MOV EBP,ESP
00752363  83ec08                SUB ESP,0x8
00752366  894df8                MOV dword ptr [EBP + -0x8],ECX
00752369  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0075236c  8b88e4680000          MOV ECX,dword ptr [EAX + 0x68e4]
00752372  d1e9                  SHR ECX,0x1
00752374  83e101                AND ECX,0x1
00752377  742e                  JZ 0x007523a7
00752379  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0075237f  8b4204                MOV EAX,dword ptr [EDX + 0x4]
00752382  8945fc                MOV dword ptr [EBP + -0x4],EAX
00752385  6a00                  PUSH 0x0
00752387  680000007f            PUSH 0x7f000000
0075238c  680000007f            PUSH 0x7f000000
00752391  6a20                  PUSH 0x20
00752393  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00752396  e885e6feff            CALL 0x00740a20
0075239b  85c0                  TEST EAX,EAX
0075239d  7508                  JNZ 0x007523a7
0075239f  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007523a2  e809eeffff            CALL 0x007511b0
007523a7  8be5                  MOV ESP,EBP
007523a9  5d                    POP EBP
007523aa  c3                    RET
