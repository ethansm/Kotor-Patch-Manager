; FN 0073a200 FUN_0073a200
0073a200  55                    PUSH EBP
0073a201  8bec                  MOV EBP,ESP
0073a203  83ec10                SUB ESP,0x10
0073a206  894df0                MOV dword ptr [EBP + -0x10],ECX
0073a209  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0073a20c  833800                CMP dword ptr [EAX],0x0
0073a20f  7521                  JNZ 0x0073a232
0073a211  68d8d39900            PUSH 0x99d3d8
0073a216  687cd29900            PUSH 0x99d27c
0073a21b  6865010000            PUSH 0x165
0073a220  8b0d88b4a100          MOV ECX,dword ptr [0x00a1b488]
0073a226  8b4908                MOV ECX,dword ptr [ECX + 0x8]
0073a229  e832030000            CALL 0x0073a560
0073a22e  33c0                  XOR EAX,EAX
0073a230  eb61                  JMP 0x0073a293
0073a232  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0073a235  8b02                  MOV EAX,dword ptr [EDX]
0073a237  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0073a23a  894df8                MOV dword ptr [EBP + -0x8],ECX
0073a23d  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0073a240  8b02                  MOV EAX,dword ptr [EDX]
0073a242  8945fc                MOV dword ptr [EBP + -0x4],EAX
0073a245  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0073a248  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0073a24b  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0073a24e  8901                  MOV dword ptr [ECX],EAX
0073a250  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0073a253  833900                CMP dword ptr [ECX],0x0
0073a256  750c                  JNZ 0x0073a264
0073a258  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0073a25b  c7420400000000        MOV dword ptr [EDX + 0x4],0x0
0073a262  eb0b                  JMP 0x0073a26f
0073a264  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0073a267  8b08                  MOV ECX,dword ptr [EAX]
0073a269  c70100000000          MOV dword ptr [ECX],0x0
0073a26f  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0073a272  8955f4                MOV dword ptr [EBP + -0xc],EDX
0073a275  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0073a278  50                    PUSH EAX
0073a279  e87ff41d00            CALL 0x009196fd
0073a27e  83c404                ADD ESP,0x4
0073a281  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0073a284  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0073a287  83ea01                SUB EDX,0x1
0073a28a  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0073a28d  895008                MOV dword ptr [EAX + 0x8],EDX
0073a290  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0073a293  8be5                  MOV ESP,EBP
0073a295  5d                    POP EBP
0073a296  c3                    RET
