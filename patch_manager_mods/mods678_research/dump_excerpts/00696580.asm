; FN 00696580 FUN_00696580
00696580  55                    PUSH EBP
00696581  8bec                  MOV EBP,ESP
00696583  83ec24                SUB ESP,0x24
00696586  894ddc                MOV dword ptr [EBP + -0x24],ECX
00696589  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
00696590  8d45f8                LEA EAX,[EBP + -0x8]
00696593  50                    PUSH EAX
00696594  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069659a  e851750600            CALL 0x006fdaf0
0069659f  85c0                  TEST EAX,EAX
006965a1  750a                  JNZ 0x006965ad
006965a3  b82ff8ffff            MOV EAX,0xfffff82f
006965a8  e992000000            JMP 0x0069663f
006965ad  8d4df4                LEA ECX,[EBP + -0xc]
006965b0  51                    PUSH ECX
006965b1  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
006965b7  e8e4730600            CALL 0x006fd9a0
006965bc  85c0                  TEST EAX,EAX
006965be  7507                  JNZ 0x006965c7
006965c0  b82ff8ffff            MOV EAX,0xfffff82f
006965c5  eb78                  JMP 0x0069663f
006965c7  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
006965cd  8b4208                MOV EAX,dword ptr [EDX + 0x8]
006965d0  8945ec                MOV dword ptr [EBP + -0x14],EAX
006965d3  8d4df0                LEA ECX,[EBP + -0x10]
006965d6  51                    PUSH ECX
006965d7  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
006965da  52                    PUSH EDX
006965db  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006965de  e89d5ae8ff            CALL 0x0051c080
006965e3  8bc8                  MOV ECX,EAX
006965e5  e8c679eaff            CALL 0x0053dfb0
006965ea  8845ff                MOV byte ptr [EBP + -0x1],AL
006965ed  0fb645ff              MOVZX EAX,byte ptr [EBP + -0x1]
006965f1  0fb60d7f449900        MOVZX ECX,byte ptr [0x0099447f]
006965f8  3bc1                  CMP EAX,ECX
006965fa  7541                  JNZ 0x0069663d
006965fc  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
006965ff  8b02                  MOV EAX,dword ptr [EDX]
00696601  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00696604  8b5030                MOV EDX,dword ptr [EAX + 0x30]
00696607  ffd2                  CALL EDX
00696609  85c0                  TEST EAX,EAX
0069660b  7430                  JZ 0x0069663d
0069660d  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00696610  8b10                  MOV EDX,dword ptr [EAX]
00696612  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00696615  8b4230                MOV EAX,dword ptr [EDX + 0x30]
00696618  ffd0                  CALL EAX
0069661a  8945e8                MOV dword ptr [EBP + -0x18],EAX
0069661d  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00696620  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
00696626  8955e4                MOV dword ptr [EBP + -0x1c],EDX
00696629  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0069662c  8945e0                MOV dword ptr [EBP + -0x20],EAX
0069662f  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
00696632  668b55e0              MOV DX,word ptr [EBP + -0x20]
00696636  66899146010000        MOV word ptr [ECX + 0x146],DX
0069663d  33c0                  XOR EAX,EAX
0069663f  8be5                  MOV ESP,EBP
00696641  5d                    POP EBP
00696642  c20800                RET 0x8
