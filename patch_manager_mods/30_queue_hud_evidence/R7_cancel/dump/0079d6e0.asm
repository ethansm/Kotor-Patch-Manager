; FN 0079d6e0 FUN_0079d6e0
0079d6e0  55                    PUSH EBP
0079d6e1  8bec                  MOV EBP,ESP
0079d6e3  83ec08                SUB ESP,0x8
0079d6e6  894df8                MOV dword ptr [EBP + -0x8],ECX
0079d6e9  837d0800              CMP dword ptr [EBP + 0x8],0x0
0079d6ed  750d                  JNZ 0x0079d6fc
0079d6ef  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0079d6f2  c780e003000000000000  MOV dword ptr [EAX + 0x3e0],0x0
0079d6fc  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0079d6ff  8b9120030000          MOV EDX,dword ptr [ECX + 0x320]
0079d705  3b5508                CMP EDX,dword ptr [EBP + 0x8]
0079d708  7455                  JZ 0x0079d75f
0079d70a  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0079d70d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0079d710  898820030000          MOV dword ptr [EAX + 0x320],ECX
0079d716  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0079d719  8b4240                MOV EAX,dword ptr [EDX + 0x40]
0079d71c  8945fc                MOV dword ptr [EBP + -0x4],EAX
0079d71f  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0079d722  e829140300            CALL 0x007ceb50
0079d727  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0079d72a  e85198f8ff            CALL 0x00726f80
0079d72f  85c0                  TEST EAX,EAX
0079d731  742c                  JZ 0x0079d75f
0079d733  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0079d736  83b92003000000        CMP dword ptr [ECX + 0x320],0x0
0079d73d  7411                  JZ 0x0079d750
0079d73f  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0079d742  e83998f8ff            CALL 0x00726f80
0079d747  8bc8                  MOV ECX,EAX
0079d749  e862980500            CALL 0x007f6fb0
0079d74e  eb0f                  JMP 0x0079d75f
0079d750  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0079d753  e82898f8ff            CALL 0x00726f80
0079d758  8bc8                  MOV ECX,EAX
0079d75a  e8c19a0500            CALL 0x007f7220
0079d75f  8be5                  MOV ESP,EBP
0079d761  5d                    POP EBP
0079d762  c20400                RET 0x4
