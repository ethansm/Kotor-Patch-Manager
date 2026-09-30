; FN 005644b0 FUN_005644b0
005644b0  55                    PUSH EBP
005644b1  8bec                  MOV EBP,ESP
005644b3  83ec0c                SUB ESP,0xc
005644b6  894df4                MOV dword ptr [EBP + -0xc],ECX
005644b9  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005644c0  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005644c3  0b4518                OR EAX,dword ptr [EBP + 0x18]
005644c6  8945fc                MOV dword ptr [EBP + -0x4],EAX
005644c9  8b4d24                MOV ECX,dword ptr [EBP + 0x24]
005644cc  d1e1                  SHL ECX,0x1
005644ce  0b4dfc                OR ECX,dword ptr [EBP + -0x4]
005644d1  894dfc                MOV dword ptr [EBP + -0x4],ECX
005644d4  d94520                FLD float ptr [EBP + 0x20]
005644d7  dc1d386b9800          FCOMP double ptr [0x00986b38]
005644dd  dfe0                  FNSTSW AX
005644df  f6c444                TEST AH,0x44
005644e2  7b09                  JNP 0x005644ed
005644e4  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005644e7  83ca04                OR EDX,0x4
005644ea  8955fc                MOV dword ptr [EBP + -0x4],EDX
005644ed  8b4534                MOV EAX,dword ptr [EBP + 0x34]
005644f0  c1e003                SHL EAX,0x3
005644f3  0b45fc                OR EAX,dword ptr [EBP + -0x4]
005644f6  8945fc                MOV dword ptr [EBP + -0x4],EAX
005644f9  8b4d2c                MOV ECX,dword ptr [EBP + 0x2c]
005644fc  c1e104                SHL ECX,0x4
005644ff  83e170                AND ECX,0x70
00564502  0b4dfc                OR ECX,dword ptr [EBP + -0x4]
00564505  894dfc                MOV dword ptr [EBP + -0x4],ECX
00564508  837d3000              CMP dword ptr [EBP + 0x30],0x0
0056450c  7412                  JZ 0x00564520
0056450e  8b5530                MOV EDX,dword ptr [EBP + 0x30]
00564511  c1e207                SHL EDX,0x7
00564514  81e280010000          AND EDX,0x180
0056451a  0b55fc                OR EDX,dword ptr [EBP + -0x4]
0056451d  8955fc                MOV dword ptr [EBP + -0x4],EDX
00564520  8b4538                MOV EAX,dword ptr [EBP + 0x38]
00564523  c1e009                SHL EAX,0x9
00564526  2500020000            AND EAX,0x200
0056452b  0b45fc                OR EAX,dword ptr [EBP + -0x4]
0056452e  8945fc                MOV dword ptr [EBP + -0x4],EAX
00564531  8b4d48                MOV ECX,dword ptr [EBP + 0x48]
00564534  c1e10a                SHL ECX,0xa
00564537  81e100040000          AND ECX,0x400
0056453d  0b4dfc                OR ECX,dword ptr [EBP + -0x4]
00564540  894dfc                MOV dword ptr [EBP + -0x4],ECX
00564543  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00564546  c782b011000002000000  MOV dword ptr [EDX + 0x11b0],0x2
00564550  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00564553  8b8880030000          MOV ECX,dword ptr [EAX + 0x380]
00564559  894df8                MOV dword ptr [EBP + -0x8],ECX
0056455c  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0056455f  c7826c02000000000000  MOV dword ptr [EDX + 0x26c],0x0
00564569  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0056456c  c7807002000000000000  MOV dword ptr [EAX + 0x270],0x0
00564576  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00564579  c7817402000000000000  MOV dword ptr [ECX + 0x274],0x0
00564583  8d5544                LEA EDX,[EBP + 0x44]
00564586  52                    PUSH EDX
00564587  6a01                  PUSH 0x1
00564589  8d4540                LEA EAX,[EBP + 0x40]
0056458c  50                    PUSH EAX
0056458d  6a01                  PUSH 0x1
0056458f  8b4d3c                MOV ECX,dword ptr [EBP + 0x3c]
00564592  83c104                ADD ECX,0x4
00564595  51                    PUSH ECX
00564596  6a02                  PUSH 0x2
00564598  8b553c                MOV EDX,dword ptr [EBP + 0x3c]
0056459b  52                    PUSH EDX
0056459c  6a02                  PUSH 0x2
0056459e  8d4520                LEA EAX,[EBP + 0x20]
005645a1  50                    PUSH EAX
005645a2  6a02                  PUSH 0x2
005645a4  8d4d28                LEA ECX,[EBP + 0x28]
005645a7  51                    PUSH ECX
005645a8  6a01                  PUSH 0x1
005645aa  8d551c                LEA EDX,[EBP + 0x1c]
005645ad  52                    PUSH EDX
005645ae  6a02                  PUSH 0x2
005645b0  8d45fc                LEA EAX,[EBP + -0x4]
005645b3  50                    PUSH EAX
005645b4  6a01                  PUSH 0x1
005645b6  8d4d14                LEA ECX,[EBP + 0x14]
005645b9  51                    PUSH ECX
005645ba  6a03                  PUSH 0x3
005645bc  8d5510                LEA EDX,[EBP + 0x10]
005645bf  52                    PUSH EDX
005645c0  6a03                  PUSH 0x3
005645c2  8b450c                MOV EAX,dword ptr [EBP + 0xc]
005645c5  83c008                ADD EAX,0x8
005645c8  50                    PUSH EAX
005645c9  6a02                  PUSH 0x2
005645cb  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
005645ce  83c104                ADD ECX,0x4
005645d1  51                    PUSH ECX
005645d2  6a02                  PUSH 0x2
005645d4  8b550c                MOV EDX,dword ptr [EBP + 0xc]
005645d7  52                    PUSH EDX
005645d8  6a02                  PUSH 0x2
005645da  0fb74508              MOVZX EAX,word ptr [EBP + 0x8]
005645de  50                    PUSH EAX
005645df  6a01                  PUSH 0x1
005645e1  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005645e4  e8b7c6fdff            CALL 0x00540ca0
005645e9  6a00                  PUSH 0x0
005645eb  680000007f            PUSH 0x7f000000
005645f0  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005645f3  e8788d0100            CALL 0x0057d370
005645f8  b801000000            MOV EAX,0x1
005645fd  8be5                  MOV ESP,EBP
005645ff  5d                    POP EBP
00564600  c24400                RET 0x44
