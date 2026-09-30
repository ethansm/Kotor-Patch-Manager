; FN 00696190 FUN_00696190
00696190  55                    PUSH EBP
00696191  8bec                  MOV EBP,ESP
00696193  83ec20                SUB ESP,0x20
00696196  894de0                MOV dword ptr [EBP + -0x20],ECX
00696199  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
006961a0  8d45f8                LEA EAX,[EBP + -0x8]
006961a3  50                    PUSH EAX
006961a4  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
006961aa  e841790600            CALL 0x006fdaf0
006961af  85c0                  TEST EAX,EAX
006961b1  750a                  JNZ 0x006961bd
006961b3  b82ff8ffff            MOV EAX,0xfffff82f
006961b8  e98b000000            JMP 0x00696248
006961bd  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006961c3  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006961c6  8955ec                MOV dword ptr [EBP + -0x14],EDX
006961c9  8d45f4                LEA EAX,[EBP + -0xc]
006961cc  50                    PUSH EAX
006961cd  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
006961d0  51                    PUSH ECX
006961d1  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006961d4  e8a75ee8ff            CALL 0x0051c080
006961d9  8bc8                  MOV ECX,EAX
006961db  e8d07deaff            CALL 0x0053dfb0
006961e0  8845ff                MOV byte ptr [EBP + -0x1],AL
006961e3  0fb655ff              MOVZX EDX,byte ptr [EBP + -0x1]
006961e7  0fb6057f449900        MOVZX EAX,byte ptr [0x0099447f]
006961ee  3bd0                  CMP EDX,EAX
006961f0  753a                  JNZ 0x0069622c
006961f2  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006961f5  8b11                  MOV EDX,dword ptr [ECX]
006961f7  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006961fa  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006961fd  ffd0                  CALL EAX
006961ff  85c0                  TEST EAX,EAX
00696201  7429                  JZ 0x0069622c
00696203  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00696206  8b11                  MOV EDX,dword ptr [ECX]
00696208  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0069620b  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0069620e  ffd0                  CALL EAX
00696210  8945e8                MOV dword ptr [EBP + -0x18],EAX
00696213  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00696216  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
0069621c  8955e4                MOV dword ptr [EBP + -0x1c],EDX
0069621f  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
00696222  0fb78846010000        MOVZX ECX,word ptr [EAX + 0x146]
00696229  894df0                MOV dword ptr [EBP + -0x10],ECX
0069622c  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0069622f  52                    PUSH EDX
00696230  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
00696236  e885770600            CALL 0x006fd9c0
0069623b  85c0                  TEST EAX,EAX
0069623d  7507                  JNZ 0x00696246
0069623f  b830f8ffff            MOV EAX,0xfffff830
00696244  eb02                  JMP 0x00696248
00696246  33c0                  XOR EAX,EAX
00696248  8be5                  MOV ESP,EBP
0069624a  5d                    POP EBP
0069624b  c20800                RET 0x8
