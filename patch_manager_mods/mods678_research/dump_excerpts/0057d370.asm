; FN 0057d370 FUN_0057d370
0057d370  55                    PUSH EBP
0057d371  8bec                  MOV EBP,ESP
0057d373  83ec3c                SUB ESP,0x3c
0057d376  894dc4                MOV dword ptr [EBP + -0x3c],ECX
0057d379  837d0c00              CMP dword ptr [EBP + 0xc],0x0
0057d37d  7553                  JNZ 0x0057d3d2
0057d37f  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
0057d382  8b10                  MOV EDX,dword ptr [EAX]
0057d384  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057d387  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
0057d38d  ffd0                  CALL EAX
0057d38f  85c0                  TEST EAX,EAX
0057d391  753a                  JNZ 0x0057d3cd
0057d393  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057d396  83b9ac11000000        CMP dword ptr [ECX + 0x11ac],0x0
0057d39d  7421                  JZ 0x0057d3c0
0057d39f  6a00                  PUSH 0x0
0057d3a1  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0057d3a4  8b02                  MOV EAX,dword ptr [EDX]
0057d3a6  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057d3a9  8b909c000000          MOV EDX,dword ptr [EAX + 0x9c]
0057d3af  ffd2                  CALL EDX
0057d3b1  98                    CWDE
0057d3b2  33c9                  XOR ECX,ECX
0057d3b4  85c0                  TEST EAX,EAX
0057d3b6  0f9ec1                SETLE CL
0057d3b9  894de0                MOV dword ptr [EBP + -0x20],ECX
0057d3bc  eb09                  JMP 0x0057d3c7
0057d3c0  c745e000000000        MOV dword ptr [EBP + -0x20],0x0
0057d3c7  837de000              CMP dword ptr [EBP + -0x20],0x0
0057d3cb  7405                  JZ 0x0057d3d2
0057d3cd  e9f6000000            JMP 0x0057d4c8
0057d3d2  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0057d3d5  3b1584449900          CMP EDX,dword ptr [0x00994484]
0057d3db  0f85db000000          JNZ 0x0057d4bc
0057d3e1  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
0057d3e4  8b884c100000          MOV ECX,dword ptr [EAX + 0x104c]
0057d3ea  3b0d84449900          CMP ECX,dword ptr [0x00994484]
0057d3f0  0f84c6000000          JZ 0x0057d4bc
0057d3f6  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0057d3f9  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0057d3fc  8945dc                MOV dword ptr [EBP + -0x24],EAX
0057d3ff  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057d402  8b914c100000          MOV EDX,dword ptr [ECX + 0x104c]
0057d408  3b55dc                CMP EDX,dword ptr [EBP + -0x24]
0057d40b  0f84ab000000          JZ 0x0057d4bc
0057d411  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057d416  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057d419  894dd8                MOV dword ptr [EBP + -0x28],ECX
0057d41c  8d55fc                LEA EDX,[EBP + -0x4]
0057d41f  52                    PUSH EDX
0057d420  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
0057d423  8b884c100000          MOV ECX,dword ptr [EAX + 0x104c]
0057d429  51                    PUSH ECX
0057d42a  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0057d42d  e84eecf9ff            CALL 0x0051c080
0057d432  8bc8                  MOV ECX,EAX
0057d434  e8770bfcff            CALL 0x0053dfb0
0057d439  0fb6d0                MOVZX EDX,AL
0057d43c  0fb6057f449900        MOVZX EAX,byte ptr [0x0099447f]
0057d443  3bd0                  CMP EDX,EAX
0057d445  7575                  JNZ 0x0057d4bc
0057d447  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0057d44a  0fb65108              MOVZX EDX,byte ptr [ECX + 0x8]
0057d44e  0fb60573449900        MOVZX EAX,byte ptr [0x00994473]
0057d455  3bd0                  CMP EDX,EAX
0057d457  7f09                  JG 0x0057d462
0057d459  c745d400000000        MOV dword ptr [EBP + -0x2c],0x0
0057d460  eb07                  JMP 0x0057d469
0057d462  c745d401000000        MOV dword ptr [EBP + -0x2c],0x1
0057d469  837dd400              CMP dword ptr [EBP + -0x2c],0x0
0057d46d  744d                  JZ 0x0057d4bc
0057d46f  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057d472  81c194000000          ADD ECX,0x94
0057d478  51                    PUSH ECX
0057d479  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0057d47c  81c294000000          ADD EDX,0x94
0057d482  52                    PUSH EDX
0057d483  8d45f0                LEA EAX,[EBP + -0x10]
0057d486  50                    PUSH EAX
0057d487  e8247eecff            CALL 0x004452b0
0057d48c  83c40c                ADD ESP,0xc
0057d48f  8d4df0                LEA ECX,[EBP + -0x10]
0057d492  51                    PUSH ECX
0057d493  8d55e4                LEA EDX,[EBP + -0x1c]
0057d496  52                    PUSH EDX
0057d497  e8246ff9ff            CALL 0x005143c0
0057d49c  83c408                ADD ESP,0x8
0057d49f  8b08                  MOV ECX,dword ptr [EAX]
0057d4a1  894df0                MOV dword ptr [EBP + -0x10],ECX
0057d4a4  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0057d4a7  8955f4                MOV dword ptr [EBP + -0xc],EDX
0057d4aa  8b4008                MOV EAX,dword ptr [EAX + 0x8]
0057d4ad  8945f8                MOV dword ptr [EBP + -0x8],EAX
0057d4b0  8d4df0                LEA ECX,[EBP + -0x10]
0057d4b3  51                    PUSH ECX
0057d4b4  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057d4b7  e8546afcff            CALL 0x00543f10
0057d4bc  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0057d4bf  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0057d4c2  89824c100000          MOV dword ptr [EDX + 0x104c],EAX
0057d4c8  8be5                  MOV ESP,EBP
0057d4ca  5d                    POP EBP
0057d4cb  c20800                RET 0x8
