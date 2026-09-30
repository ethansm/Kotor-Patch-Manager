; FN 00537b00 FUN_00537b00
00537b00  55                    PUSH EBP
00537b01  8bec                  MOV EBP,ESP
00537b03  83ec0c                SUB ESP,0xc
00537b06  894df4                MOV dword ptr [EBP + -0xc],ECX
00537b09  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00537b10  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00537b13  8b885c000100          MOV ECX,dword ptr [EAX + 0x1005c]
00537b19  894df8                MOV dword ptr [EBP + -0x8],ECX
00537b1c  8d55fc                LEA EDX,[EBP + -0x4]
00537b1f  52                    PUSH EDX
00537b20  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00537b23  50                    PUSH EAX
00537b24  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00537b27  e884640000            CALL 0x0053dfb0
00537b2c  0fb6c8                MOVZX ECX,AL
00537b2f  0fb6157f449900        MOVZX EDX,byte ptr [0x0099447f]
00537b36  3bca                  CMP ECX,EDX
00537b38  7527                  JNZ 0x00537b61
00537b3a  837dfc00              CMP dword ptr [EBP + -0x4],0x0
00537b3e  7421                  JZ 0x00537b61
00537b40  0fb60575449900        MOVZX EAX,byte ptr [0x00994475]
00537b47  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00537b4a  0fb65108              MOVZX EDX,byte ptr [ECX + 0x8]
00537b4e  3bd0                  CMP EDX,EAX
00537b50  750f                  JNZ 0x00537b61
00537b52  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00537b55  8b10                  MOV EDX,dword ptr [EAX]
00537b57  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00537b5a  8b4238                MOV EAX,dword ptr [EDX + 0x38]
00537b5d  ffd0                  CALL EAX
00537b5f  eb02                  JMP 0x00537b63
00537b61  33c0                  XOR EAX,EAX
00537b63  8be5                  MOV ESP,EBP
00537b65  5d                    POP EBP
00537b66  c20400                RET 0x4
