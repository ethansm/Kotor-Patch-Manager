; FN 00418d20 FUN_00418d20
00418d20  55                    PUSH EBP
00418d21  8bec                  MOV EBP,ESP
00418d23  83ec0c                SUB ESP,0xc
00418d26  894df4                MOV dword ptr [EBP + -0xc],ECX
00418d29  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00418d2c  8b4848                MOV ECX,dword ptr [EAX + 0x48]
00418d2f  d1e9                  SHR ECX,0x1
00418d31  83e101                AND ECX,0x1
00418d34  745c                  JZ 0x00418d92
00418d36  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00418d39  8b4248                MOV EAX,dword ptr [EDX + 0x48]
00418d3c  c1e805                SHR EAX,0x5
00418d3f  83e001                AND EAX,0x1
00418d42  754e                  JNZ 0x00418d92
00418d44  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00418d47  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00418d4a  3b5104                CMP EDX,dword ptr [ECX + 0x4]
00418d4d  7c43                  JL 0x00418d92
00418d4f  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00418d52  83c004                ADD EAX,0x4
00418d55  8945fc                MOV dword ptr [EBP + -0x4],EAX
00418d58  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00418d5b  8b11                  MOV EDX,dword ptr [ECX]
00418d5d  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00418d60  035008                ADD EDX,dword ptr [EAX + 0x8]
00418d63  395508                CMP dword ptr [EBP + 0x8],EDX
00418d66  7f2a                  JG 0x00418d92
00418d68  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00418d6b  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00418d6e  3b5108                CMP EDX,dword ptr [ECX + 0x8]
00418d71  7c1f                  JL 0x00418d92
00418d73  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00418d76  83c004                ADD EAX,0x4
00418d79  8945f8                MOV dword ptr [EBP + -0x8],EAX
00418d7c  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00418d7f  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00418d82  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00418d85  03500c                ADD EDX,dword ptr [EAX + 0xc]
00418d88  39550c                CMP dword ptr [EBP + 0xc],EDX
00418d8b  7f05                  JG 0x00418d92
00418d8d  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00418d90  eb02                  JMP 0x00418d94
00418d92  33c0                  XOR EAX,EAX
00418d94  8be5                  MOV ESP,EBP
00418d96  5d                    POP EBP
00418d97  c20800                RET 0x8
