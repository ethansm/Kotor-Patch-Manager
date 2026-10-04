; FN 00564770 FUN_00564770
00564770  55                    PUSH EBP
00564771  8bec                  MOV EBP,ESP
00564773  83ec0c                SUB ESP,0xc
00564776  894df4                MOV dword ptr [EBP + -0xc],ECX
00564779  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00564780  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00564783  0b4518                OR EAX,dword ptr [EBP + 0x18]
00564786  8945fc                MOV dword ptr [EBP + -0x4],EAX
00564789  8b4d24                MOV ECX,dword ptr [EBP + 0x24]
0056478c  d1e1                  SHL ECX,0x1
0056478e  0b4dfc                OR ECX,dword ptr [EBP + -0x4]
00564791  894dfc                MOV dword ptr [EBP + -0x4],ECX
00564794  d94520                FLD float ptr [EBP + 0x20]
00564797  dc1d386b9800          FCOMP double ptr [0x00986b38]
0056479d  dfe0                  FNSTSW AX
0056479f  f6c444                TEST AH,0x44
005647a2  7b09                  JNP 0x005647ad
005647a4  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005647a7  83ca04                OR EDX,0x4
005647aa  8955fc                MOV dword ptr [EBP + -0x4],EDX
005647ad  8b4534                MOV EAX,dword ptr [EBP + 0x34]
005647b0  c1e003                SHL EAX,0x3
005647b3  0b45fc                OR EAX,dword ptr [EBP + -0x4]
005647b6  8945fc                MOV dword ptr [EBP + -0x4],EAX
005647b9  8b4d2c                MOV ECX,dword ptr [EBP + 0x2c]
005647bc  c1e104                SHL ECX,0x4
005647bf  83e170                AND ECX,0x70
005647c2  0b4dfc                OR ECX,dword ptr [EBP + -0x4]
005647c5  894dfc                MOV dword ptr [EBP + -0x4],ECX
005647c8  837d3000              CMP dword ptr [EBP + 0x30],0x0
005647cc  7412                  JZ 0x005647e0
005647ce  8b5530                MOV EDX,dword ptr [EBP + 0x30]
005647d1  c1e207                SHL EDX,0x7
005647d4  81e280010000          AND EDX,0x180
005647da  0b55fc                OR EDX,dword ptr [EBP + -0x4]
005647dd  8955fc                MOV dword ptr [EBP + -0x4],EDX
005647e0  8b4538                MOV EAX,dword ptr [EBP + 0x38]
005647e3  c1e009                SHL EAX,0x9
005647e6  2500020000            AND EAX,0x200
005647eb  0b45fc                OR EAX,dword ptr [EBP + -0x4]
005647ee  8945fc                MOV dword ptr [EBP + -0x4],EAX
005647f1  8b4d48                MOV ECX,dword ptr [EBP + 0x48]
005647f4  c1e10a                SHL ECX,0xa
005647f7  81e100040000          AND ECX,0x400
005647fd  0b4dfc                OR ECX,dword ptr [EBP + -0x4]
00564800  894dfc                MOV dword ptr [EBP + -0x4],ECX
00564803  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00564806  c782b011000002000000  MOV dword ptr [EDX + 0x11b0],0x2
00564810  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00564813  8b8880030000          MOV ECX,dword ptr [EAX + 0x380]
00564819  894df8                MOV dword ptr [EBP + -0x8],ECX
0056481c  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0056481f  c7826c02000000000000  MOV dword ptr [EDX + 0x26c],0x0
00564829  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0056482c  c7807002000000000000  MOV dword ptr [EAX + 0x270],0x0
00564836  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00564839  c7817402000000000000  MOV dword ptr [ECX + 0x274],0x0
00564843  8d5544                LEA EDX,[EBP + 0x44]
00564846  52                    PUSH EDX
00564847  6a01                  PUSH 0x1
00564849  8d4540                LEA EAX,[EBP + 0x40]
0056484c  50                    PUSH EAX
0056484d  6a01                  PUSH 0x1
0056484f  8b4d3c                MOV ECX,dword ptr [EBP + 0x3c]
00564852  83c104                ADD ECX,0x4
00564855  51                    PUSH ECX
00564856  6a02                  PUSH 0x2
00564858  8b553c                MOV EDX,dword ptr [EBP + 0x3c]
0056485b  52                    PUSH EDX
0056485c  6a02                  PUSH 0x2
0056485e  8d4520                LEA EAX,[EBP + 0x20]
00564861  50                    PUSH EAX
00564862  6a02                  PUSH 0x2
00564864  8d4d28                LEA ECX,[EBP + 0x28]
00564867  51                    PUSH ECX
00564868  6a01                  PUSH 0x1
0056486a  8d551c                LEA EDX,[EBP + 0x1c]
0056486d  52                    PUSH EDX
0056486e  6a02                  PUSH 0x2
00564870  8d45fc                LEA EAX,[EBP + -0x4]
00564873  50                    PUSH EAX
00564874  6a01                  PUSH 0x1
00564876  8d4d14                LEA ECX,[EBP + 0x14]
00564879  51                    PUSH ECX
0056487a  6a03                  PUSH 0x3
0056487c  8d5510                LEA EDX,[EBP + 0x10]
0056487f  52                    PUSH EDX
00564880  6a03                  PUSH 0x3
00564882  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00564885  83c008                ADD EAX,0x8
00564888  50                    PUSH EAX
00564889  6a02                  PUSH 0x2
0056488b  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0056488e  83c104                ADD ECX,0x4
00564891  51                    PUSH ECX
00564892  6a02                  PUSH 0x2
00564894  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00564897  52                    PUSH EDX
00564898  6a02                  PUSH 0x2
0056489a  0fb74508              MOVZX EAX,word ptr [EBP + 0x8]
0056489e  50                    PUSH EAX
0056489f  6a01                  PUSH 0x1
005648a1  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005648a4  e847affdff            CALL 0x0053f7f0
005648a9  6a00                  PUSH 0x0
005648ab  680000007f            PUSH 0x7f000000
005648b0  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005648b3  e8b88a0100            CALL 0x0057d370
005648b8  b801000000            MOV EAX,0x1
005648bd  8be5                  MOV ESP,EBP
005648bf  5d                    POP EBP
005648c0  c24400                RET 0x44
