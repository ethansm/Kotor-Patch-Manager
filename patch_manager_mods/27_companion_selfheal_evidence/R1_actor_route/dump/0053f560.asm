; FN 0053f560 FUN_0053f560
0053f560  55                    PUSH EBP
0053f561  8bec                  MOV EBP,ESP
0053f563  83ec0c                SUB ESP,0xc
0053f566  894df4                MOV dword ptr [EBP + -0xc],ECX
0053f569  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0053f56c  81c100010000          ADD ECX,0x100
0053f572  e809222600            CALL 0x007a1780
0053f577  83f801                CMP EAX,0x1
0053f57a  7504                  JNZ 0x0053f580
0053f57c  33c0                  XOR EAX,EAX
0053f57e  eb55                  JMP 0x0053f5d5
0053f580  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0053f583  81c100010000          ADD ECX,0x100
0053f589  e892212600            CALL 0x007a1720
0053f58e  8945fc                MOV dword ptr [EBP + -0x4],EAX
0053f591  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0053f594  50                    PUSH EAX
0053f595  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0053f598  81c100010000          ADD ECX,0x100
0053f59e  e88d1e0200            CALL 0x00561430
0053f5a3  8945f8                MOV dword ptr [EBP + -0x8],EAX
0053f5a6  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0053f5aa  7427                  JZ 0x0053f5d3
0053f5ac  0fb74d08              MOVZX ECX,word ptr [EBP + 0x8]
0053f5b0  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0053f5b3  390a                  CMP dword ptr [EDX],ECX
0053f5b5  7505                  JNZ 0x0053f5bc
0053f5b7  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0053f5ba  eb19                  JMP 0x0053f5d5
0053f5bc  8d45fc                LEA EAX,[EBP + -0x4]
0053f5bf  50                    PUSH EAX
0053f5c0  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0053f5c3  81c100010000          ADD ECX,0x100
0053f5c9  e8a2cd0400            CALL 0x0058c370
0053f5ce  8945f8                MOV dword ptr [EBP + -0x8],EAX
0053f5d1  ebd3                  JMP 0x0053f5a6
0053f5d3  33c0                  XOR EAX,EAX
0053f5d5  8be5                  MOV ESP,EBP
0053f5d7  5d                    POP EBP
0053f5d8  c20400                RET 0x4
