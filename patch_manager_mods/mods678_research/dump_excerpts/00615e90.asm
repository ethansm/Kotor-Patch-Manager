; FN 00615e90 FUN_00615e90
00615e90  55                    PUSH EBP
00615e91  8bec                  MOV EBP,ESP
00615e93  83ec3c                SUB ESP,0x3c
00615e96  894dc4                MOV dword ptr [EBP + -0x3c],ECX
00615e99  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00615e9c  8b888c020000          MOV ECX,dword ptr [EAX + 0x28c]
00615ea2  894df8                MOV dword ptr [EBP + -0x8],ECX
00615ea5  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
00615ea8  8b828c020000          MOV EAX,dword ptr [EDX + 0x28c]
00615eae  8945cc                MOV dword ptr [EBP + -0x34],EAX
00615eb1  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
00615eb4  2b4df8                SUB ECX,dword ptr [EBP + -0x8]
00615eb7  894dec                MOV dword ptr [EBP + -0x14],ECX
00615eba  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
00615ebd  8b828c020000          MOV EAX,dword ptr [EDX + 0x28c]
00615ec3  8945fc                MOV dword ptr [EBP + -0x4],EAX
00615ec6  33c9                  XOR ECX,ECX
00615ec8  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00615ecb  ba1c000000            MOV EDX,0x1c
00615ed0  f7e2                  MUL EDX
00615ed2  0f90c1                SETO CL
00615ed5  f7d9                  NEG ECX
00615ed7  0bc8                  OR ECX,EAX
00615ed9  51                    PUSH ECX
00615eda  e844383000            CALL 0x00919723
00615edf  83c404                ADD ESP,0x4
00615ee2  8945d4                MOV dword ptr [EBP + -0x2c],EAX
00615ee5  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
00615ee8  8945f0                MOV dword ptr [EBP + -0x10],EAX
00615eeb  c745e000000000        MOV dword ptr [EBP + -0x20],0x0
00615ef2  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
00615ef9  eb09                  JMP 0x00615f04
00615efb  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00615efe  83c101                ADD ECX,0x1
00615f01  894df4                MOV dword ptr [EBP + -0xc],ECX
00615f04  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00615f07  3b55fc                CMP EDX,dword ptr [EBP + -0x4]
00615f0a  0f8d90000000          JGE 0x00615fa0
00615f10  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
00615f17  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00615f1a  6bc01c                IMUL EAX,EAX,0x1c
00615f1d  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00615f20  038194020000          ADD EAX,dword ptr [ECX + 0x294]
00615f26  8945dc                MOV dword ptr [EBP + -0x24],EAX
00615f29  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
00615f30  eb09                  JMP 0x00615f3b
00615f32  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
00615f35  83c201                ADD EDX,0x1
00615f38  8955e8                MOV dword ptr [EBP + -0x18],EDX
00615f3b  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
00615f3e  3b45f8                CMP EAX,dword ptr [EBP + -0x8]
00615f41  7d31                  JGE 0x00615f74
00615f43  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00615f46  51                    PUSH ECX
00615f47  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00615f4a  e851e3feff            CALL 0x006042a0
00615f4f  8945d8                MOV dword ptr [EBP + -0x28],EAX
00615f52  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
00615f55  668b4212              MOV AX,word ptr [EDX + 0x12]
00615f59  668945ca              MOV word ptr [EBP + -0x36],AX
00615f5d  0fb74dca              MOVZX ECX,word ptr [EBP + -0x36]
00615f61  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00615f64  83c201                ADD EDX,0x1
00615f67  3bca                  CMP ECX,EDX
00615f69  7507                  JNZ 0x00615f72
00615f6b  c745e401000000        MOV dword ptr [EBP + -0x1c],0x1
00615f72  ebbe                  JMP 0x00615f32
00615f74  837de400              CMP dword ptr [EBP + -0x1c],0x0
00615f78  7521                  JNZ 0x00615f9b
00615f7a  6a1c                  PUSH 0x1c
00615f7c  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
00615f7f  50                    PUSH EAX
00615f80  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00615f83  6bc91c                IMUL ECX,ECX,0x1c
00615f86  034df0                ADD ECX,dword ptr [EBP + -0x10]
00615f89  51                    PUSH ECX
00615f8a  e8117c3000            CALL 0x0091dba0
00615f8f  83c40c                ADD ESP,0xc
00615f92  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
00615f95  83c201                ADD EDX,0x1
00615f98  8955e0                MOV dword ptr [EBP + -0x20],EDX
00615f9b  e95bffffff            JMP 0x00615efb
00615fa0  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00615fa3  50                    PUSH EAX
00615fa4  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00615fa7  e824e2feff            CALL 0x006041d0
00615fac  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00615faf  83b99402000000        CMP dword ptr [ECX + 0x294],0x0
00615fb6  741d                  JZ 0x00615fd5
00615fb8  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00615fbb  6bd21c                IMUL EDX,EDX,0x1c
00615fbe  52                    PUSH EDX
00615fbf  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00615fc2  50                    PUSH EAX
00615fc3  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00615fc6  8b9194020000          MOV EDX,dword ptr [ECX + 0x294]
00615fcc  52                    PUSH EDX
00615fcd  e8ce7b3000            CALL 0x0091dba0
00615fd2  83c40c                ADD ESP,0xc
00615fd5  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00615fd8  8945d0                MOV dword ptr [EBP + -0x30],EAX
00615fdb  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00615fde  51                    PUSH ECX
00615fdf  e819373000            CALL 0x009196fd
00615fe4  83c404                ADD ESP,0x4
00615fe7  8be5                  MOV ESP,EBP
00615fe9  5d                    POP EBP
00615fea  c20800                RET 0x8
