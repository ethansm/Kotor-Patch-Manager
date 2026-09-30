; FN 00537d30 FUN_00537d30
00537d30  55                    PUSH EBP
00537d31  8bec                  MOV EBP,ESP
00537d33  83ec0c                SUB ESP,0xc
00537d36  894df4                MOV dword ptr [EBP + -0xc],ECX
00537d39  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00537d40  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00537d43  8b885c000100          MOV ECX,dword ptr [EAX + 0x1005c]
00537d49  894df8                MOV dword ptr [EBP + -0x8],ECX
00537d4c  8d55fc                LEA EDX,[EBP + -0x4]
00537d4f  52                    PUSH EDX
00537d50  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00537d53  50                    PUSH EAX
00537d54  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00537d57  e854620000            CALL 0x0053dfb0
00537d5c  0fb6c8                MOVZX ECX,AL
00537d5f  0fb6157f449900        MOVZX EDX,byte ptr [0x0099447f]
00537d66  3bca                  CMP ECX,EDX
00537d68  7527                  JNZ 0x00537d91
00537d6a  837dfc00              CMP dword ptr [EBP + -0x4],0x0
00537d6e  7421                  JZ 0x00537d91
00537d70  0fb60578449900        MOVZX EAX,byte ptr [0x00994478]
00537d77  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00537d7a  0fb65108              MOVZX EDX,byte ptr [ECX + 0x8]
00537d7e  3bd0                  CMP EDX,EAX
00537d80  750f                  JNZ 0x00537d91
00537d82  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00537d85  8b10                  MOV EDX,dword ptr [EAX]
00537d87  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00537d8a  8b4248                MOV EAX,dword ptr [EDX + 0x48]
00537d8d  ffd0                  CALL EAX
00537d8f  eb02                  JMP 0x00537d93
00537d91  33c0                  XOR EAX,EAX
00537d93  8be5                  MOV ESP,EBP
00537d95  5d                    POP EBP
00537d96  c20400                RET 0x4
