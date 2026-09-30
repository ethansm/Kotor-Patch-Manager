; FN 00615d60 FUN_00615d60
00615d60  55                    PUSH EBP
00615d61  8bec                  MOV EBP,ESP
00615d63  83ec28                SUB ESP,0x28
00615d66  894dd8                MOV dword ptr [EBP + -0x28],ECX
00615d69  837d0800              CMP dword ptr [EBP + 0x8],0x0
00615d6d  7505                  JNZ 0x00615d74
00615d6f  e90e010000            JMP 0x00615e82
00615d74  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00615d77  8b888c020000          MOV ECX,dword ptr [EAX + 0x28c]
00615d7d  894df8                MOV dword ptr [EBP + -0x8],ECX
00615d80  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
00615d83  8b828c020000          MOV EAX,dword ptr [EDX + 0x28c]
00615d89  8945e0                MOV dword ptr [EBP + -0x20],EAX
00615d8c  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00615d8f  034de0                ADD ECX,dword ptr [EBP + -0x20]
00615d92  894dec                MOV dword ptr [EBP + -0x14],ECX
00615d95  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
00615d98  8b828c020000          MOV EAX,dword ptr [EDX + 0x28c]
00615d9e  8945fc                MOV dword ptr [EBP + -0x4],EAX
00615da1  33c9                  XOR ECX,ECX
00615da3  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00615da6  ba1c000000            MOV EDX,0x1c
00615dab  f7e2                  MUL EDX
00615dad  0f90c1                SETO CL
00615db0  f7d9                  NEG ECX
00615db2  0bc8                  OR ECX,EAX
00615db4  51                    PUSH ECX
00615db5  e869393000            CALL 0x00919723
00615dba  83c404                ADD ESP,0x4
00615dbd  8945e8                MOV dword ptr [EBP + -0x18],EAX
00615dc0  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
00615dc3  8945f0                MOV dword ptr [EBP + -0x10],EAX
00615dc6  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00615dc9  6bc91c                IMUL ECX,ECX,0x1c
00615dcc  51                    PUSH ECX
00615dcd  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
00615dd0  8b8294020000          MOV EAX,dword ptr [EDX + 0x294]
00615dd6  50                    PUSH EAX
00615dd7  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00615dda  51                    PUSH ECX
00615ddb  e8c07d3000            CALL 0x0091dba0
00615de0  83c40c                ADD ESP,0xc
00615de3  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00615de6  52                    PUSH EDX
00615de7  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
00615dea  e8e1e3feff            CALL 0x006041d0
00615def  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00615df2  6bc01c                IMUL EAX,EAX,0x1c
00615df5  50                    PUSH EAX
00615df6  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00615df9  51                    PUSH ECX
00615dfa  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
00615dfd  8b8294020000          MOV EAX,dword ptr [EDX + 0x294]
00615e03  50                    PUSH EAX
00615e04  e8977d3000            CALL 0x0091dba0
00615e09  83c40c                ADD ESP,0xc
00615e0c  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00615e0f  894de4                MOV dword ptr [EBP + -0x1c],ECX
00615e12  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
00615e15  52                    PUSH EDX
00615e16  e8e2383000            CALL 0x009196fd
00615e1b  83c404                ADD ESP,0x4
00615e1e  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00615e21  6bc01c                IMUL EAX,EAX,0x1c
00615e24  50                    PUSH EAX
00615e25  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00615e28  8b9194020000          MOV EDX,dword ptr [ECX + 0x294]
00615e2e  52                    PUSH EDX
00615e2f  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00615e32  2b45f8                SUB EAX,dword ptr [EBP + -0x8]
00615e35  6bc01c                IMUL EAX,EAX,0x1c
00615e38  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
00615e3b  038194020000          ADD EAX,dword ptr [ECX + 0x294]
00615e41  50                    PUSH EAX
00615e42  e8597d3000            CALL 0x0091dba0
00615e47  83c40c                ADD ESP,0xc
00615e4a  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00615e4d  8955f4                MOV dword ptr [EBP + -0xc],EDX
00615e50  eb09                  JMP 0x00615e5b
00615e52  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00615e55  83c001                ADD EAX,0x1
00615e58  8945f4                MOV dword ptr [EBP + -0xc],EAX
00615e5b  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00615e5e  3b4dec                CMP ECX,dword ptr [EBP + -0x14]
00615e61  7d1f                  JGE 0x00615e82
00615e63  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00615e66  52                    PUSH EDX
00615e67  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
00615e6a  e831e4feff            CALL 0x006042a0
00615e6f  8945dc                MOV dword ptr [EBP + -0x24],EAX
00615e72  0fb7450c              MOVZX EAX,word ptr [EBP + 0xc]
00615e76  83c001                ADD EAX,0x1
00615e79  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
00615e7c  66894112              MOV word ptr [ECX + 0x12],AX
00615e80  ebd0                  JMP 0x00615e52
00615e82  8be5                  MOV ESP,EBP
00615e84  5d                    POP EBP
00615e85  c20800                RET 0x8
