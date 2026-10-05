; FN 007523e0 FUN_007523e0
007523e0  55                    PUSH EBP
007523e1  8bec                  MOV EBP,ESP
007523e3  83ec10                SUB ESP,0x10
007523e6  894df0                MOV dword ptr [EBP + -0x10],ECX
007523e9  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007523ee  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007523f1  894dfc                MOV dword ptr [EBP + -0x4],ECX
007523f4  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007523f7  e894d7feff            CALL 0x0073fb90
007523fc  8bc8                  MOV ECX,EAX
007523fe  e87d390900            CALL 0x007e5d80
00752403  8945f8                MOV dword ptr [EBP + -0x8],EAX
00752406  837df800              CMP dword ptr [EBP + -0x8],0x0
0075240a  7440                  JZ 0x0075244c
0075240c  6a00                  PUSH 0x0
0075240e  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00752411  e80ad90200            CALL 0x0077fd20
00752416  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00752419  e8a2000a00            CALL 0x007f24c0
0075241e  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00752421  8b421c                MOV EAX,dword ptr [EDX + 0x1c]
00752424  8945f4                MOV dword ptr [EBP + -0xc],EAX
00752427  6a00                  PUSH 0x0
00752429  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0075242c  e86ffecbff            CALL 0x004122a0
00752431  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00752434  d90574749800          FLD float ptr [0x00987474]
0075243a  d9995c590000          FSTP float ptr [ECX + 0x595c]
00752440  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00752443  d90574749800          FLD float ptr [0x00987474]
00752449  d95a70                FSTP float ptr [EDX + 0x70]
0075244c  8be5                  MOV ESP,EBP
0075244e  5d                    POP EBP
0075244f  c3                    RET
