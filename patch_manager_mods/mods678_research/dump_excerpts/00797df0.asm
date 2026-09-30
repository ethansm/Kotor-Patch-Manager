; FN 00797df0 FUN_00797df0
00797df0  55                    PUSH EBP
00797df1  8bec                  MOV EBP,ESP
00797df3  83ec10                SUB ESP,0x10
00797df6  894df0                MOV dword ptr [EBP + -0x10],ECX
00797df9  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00797e00  eb09                  JMP 0x00797e0b
00797e02  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00797e05  83c001                ADD EAX,0x1
00797e08  8945fc                MOV dword ptr [EBP + -0x4],EAX
00797e0b  837dfc07              CMP dword ptr [EBP + -0x4],0x7
00797e0f  7371                  JNC 0x00797e82
00797e11  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00797e14  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00797e17  83bc8a0401000000      CMP dword ptr [EDX + ECX*0x4 + 0x104],0x0
00797e1f  745f                  JZ 0x00797e80
00797e21  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00797e28  eb09                  JMP 0x00797e33
00797e2a  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00797e2d  83c001                ADD EAX,0x1
00797e30  8945fc                MOV dword ptr [EBP + -0x4],EAX
00797e33  837dfc07              CMP dword ptr [EBP + -0x4],0x7
00797e37  7313                  JNC 0x00797e4c
00797e39  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00797e3c  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00797e3f  c7848a0401000000000000MOV dword ptr [EDX + ECX*0x4 + 0x104],0x0
00797e4a  ebde                  JMP 0x00797e2a
00797e4c  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00797e4f  c7800001000001000000  MOV dword ptr [EAX + 0x100],0x1
00797e59  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00797e5c  c781f800000000000000  MOV dword ptr [ECX + 0xf8],0x0
00797e66  c781fc00000000000000  MOV dword ptr [ECX + 0xfc],0x0
00797e70  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00797e73  8b8a50010000          MOV ECX,dword ptr [EDX + 0x150]
00797e79  e8e21f0e00            CALL 0x00879e60
00797e7e  eb02                  JMP 0x00797e82
00797e80  eb80                  JMP 0x00797e02
00797e82  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00797e85  83781800              CMP dword ptr [EAX + 0x18],0x0
00797e89  7428                  JZ 0x00797eb3
00797e8b  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00797e8e  8b5118                MOV EDX,dword ptr [ECX + 0x18]
00797e91  8b4248                MOV EAX,dword ptr [EDX + 0x48]
00797e94  8945f8                MOV dword ptr [EBP + -0x8],EAX
00797e97  837df800              CMP dword ptr [EBP + -0x8],0x0
00797e9b  7416                  JZ 0x00797eb3
00797e9d  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00797ea0  8b5118                MOV EDX,dword ptr [ECX + 0x18]
00797ea3  8b4248                MOV EAX,dword ptr [EDX + 0x48]
00797ea6  8945f4                MOV dword ptr [EBP + -0xc],EAX
00797ea9  6a00                  PUSH 0x0
00797eab  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00797eae  e8ad250100            CALL 0x007aa460
00797eb3  8be5                  MOV ESP,EBP
00797eb5  5d                    POP EBP
00797eb6  c3                    RET
