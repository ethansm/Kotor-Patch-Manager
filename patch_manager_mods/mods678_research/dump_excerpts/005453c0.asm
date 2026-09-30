; FN 005453c0 FUN_005453c0
005453c0  55                    PUSH EBP
005453c1  8bec                  MOV EBP,ESP
005453c3  83ec10                SUB ESP,0x10
005453c6  894df0                MOV dword ptr [EBP + -0x10],ECX
005453c9  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
005453d0  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005453d7  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005453dc  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005453df  894df4                MOV dword ptr [EBP + -0xc],ECX
005453e2  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005453e5  e8966cfdff            CALL 0x0051c080
005453ea  8945fc                MOV dword ptr [EBP + -0x4],EAX
005453ed  8d55f8                LEA EDX,[EBP + -0x8]
005453f0  52                    PUSH EDX
005453f1  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
005453f4  8b8890000000          MOV ECX,dword ptr [EAX + 0x90]
005453fa  51                    PUSH ECX
005453fb  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005453fe  e8ad8bffff            CALL 0x0053dfb0
00545403  837df800              CMP dword ptr [EBP + -0x8],0x0
00545407  7411                  JZ 0x0054541a
00545409  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0054540c  8b02                  MOV EAX,dword ptr [EDX]
0054540e  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00545411  8b5028                MOV EDX,dword ptr [EAX + 0x28]
00545414  ffd2                  CALL EDX
00545416  eb04                  JMP 0x0054541c
0054541a  33c0                  XOR EAX,EAX
0054541c  8be5                  MOV ESP,EBP
0054541e  5d                    POP EBP
0054541f  c3                    RET
