; FN 0068bbe0 FUN_0068bbe0
0068bbe0  55                    PUSH EBP
0068bbe1  8bec                  MOV EBP,ESP
0068bbe3  83ec14                SUB ESP,0x14
0068bbe6  894dec                MOV dword ptr [EBP + -0x14],ECX
0068bbe9  8d45f4                LEA EAX,[EBP + -0xc]
0068bbec  50                    PUSH EAX
0068bbed  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0068bbf3  e8f81e0700            CALL 0x006fdaf0
0068bbf8  85c0                  TEST EAX,EAX
0068bbfa  7507                  JNZ 0x0068bc03
0068bbfc  b82ff8ffff            MOV EAX,0xfffff82f
0068bc01  eb50                  JMP 0x0068bc53
0068bc03  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0068bc0a  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0068bc10  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0068bc13  8955f0                MOV dword ptr [EBP + -0x10],EDX
0068bc16  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0068bc19  50                    PUSH EAX
0068bc1a  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0068bc1d  e8de04e9ff            CALL 0x0051c100
0068bc22  8945f8                MOV dword ptr [EBP + -0x8],EAX
0068bc25  837df800              CMP dword ptr [EBP + -0x8],0x0
0068bc29  740c                  JZ 0x0068bc37
0068bc2b  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0068bc2e  8b91ac110000          MOV EDX,dword ptr [ECX + 0x11ac]
0068bc34  8955fc                MOV dword ptr [EBP + -0x4],EDX
0068bc37  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0068bc3a  50                    PUSH EAX
0068bc3b  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0068bc41  e87a1d0700            CALL 0x006fd9c0
0068bc46  85c0                  TEST EAX,EAX
0068bc48  7507                  JNZ 0x0068bc51
0068bc4a  b830f8ffff            MOV EAX,0xfffff830
0068bc4f  eb02                  JMP 0x0068bc53
0068bc51  33c0                  XOR EAX,EAX
0068bc53  8be5                  MOV ESP,EBP
0068bc55  5d                    POP EBP
0068bc56  c20800                RET 0x8
