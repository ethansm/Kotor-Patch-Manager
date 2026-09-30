; FN 00743540 FUN_00743540
00743540  55                    PUSH EBP
00743541  8bec                  MOV EBP,ESP
00743543  83ec38                SUB ESP,0x38
00743546  56                    PUSH ESI
00743547  894dd0                MOV dword ptr [EBP + -0x30],ECX
0074354a  8b4510                MOV EAX,dword ptr [EBP + 0x10]
0074354d  50                    PUSH EAX
0074354e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00743551  e83aaa0300            CALL 0x0077df90
00743556  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00743559  8881aa1b0000          MOV byte ptr [ECX + 0x1baa],AL
0074355f  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00743566  eb09                  JMP 0x00743571
00743568  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0074356b  83c201                ADD EDX,0x1
0074356e  8955fc                MOV dword ptr [EBP + -0x4],EDX
00743571  837dfc03              CMP dword ptr [EBP + -0x4],0x3
00743575  7d70                  JGE 0x007435e7
00743577  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
0074357e  eb09                  JMP 0x00743589
00743580  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00743583  83c001                ADD EAX,0x1
00743586  8945f0                MOV dword ptr [EBP + -0x10],EAX
00743589  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0074358c  6bc90c                IMUL ECX,ECX,0xc
0074358f  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
00743592  8b440a04              MOV EAX,dword ptr [EDX + ECX*0x1 + 0x4]
00743596  8945ec                MOV dword ptr [EBP + -0x14],EAX
00743599  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0074359c  3b4dec                CMP ECX,dword ptr [EBP + -0x14]
0074359f  7d19                  JGE 0x007435ba
007435a1  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
007435a4  6bd20c                IMUL EDX,EDX,0xc
007435a7  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007435aa  6bc93c                IMUL ECX,ECX,0x3c
007435ad  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
007435b0  030c10                ADD ECX,dword ptr [EAX + EDX*0x1]
007435b3  e8e8990a00            CALL 0x007ecfa0
007435b8  ebc6                  JMP 0x00743580
007435ba  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007435bd  6bc90c                IMUL ECX,ECX,0xc
007435c0  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
007435c3  c7440a0400000000      MOV dword ptr [EDX + ECX*0x1 + 0x4],0x0
007435cb  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007435ce  6bc00c                IMUL EAX,EAX,0xc
007435d1  0345d0                ADD EAX,dword ptr [EBP + -0x30]
007435d4  50                    PUSH EAX
007435d5  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007435d8  51                    PUSH ECX
007435d9  8b5510                MOV EDX,dword ptr [EBP + 0x10]
007435dc  52                    PUSH EDX
007435dd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
007435e0  e88bad0300            CALL 0x0077e370
007435e5  eb81                  JMP 0x00743568
007435e7  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
007435ee  eb09                  JMP 0x007435f9
007435f0  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007435f3  83c001                ADD EAX,0x1
007435f6  8945fc                MOV dword ptr [EBP + -0x4],EAX
007435f9  837dfc03              CMP dword ptr [EBP + -0x4],0x3
007435fd  0f8d28020000          JGE 0x0074382b
00743603  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
0074360a  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
00743611  eb09                  JMP 0x0074361c
00743613  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00743616  83c101                ADD ECX,0x1
00743619  894df0                MOV dword ptr [EBP + -0x10],ECX
0074361c  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0074361f  6bd20c                IMUL EDX,EDX,0xc
00743622  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
00743625  8b4c1004              MOV ECX,dword ptr [EAX + EDX*0x1 + 0x4]
00743629  894de8                MOV dword ptr [EBP + -0x18],ECX
0074362c  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0074362f  3b55e8                CMP EDX,dword ptr [EBP + -0x18]
00743632  0f8df7000000          JGE 0x0074372f
00743638  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0074363b  6bc00c                IMUL EAX,EAX,0xc
0074363e  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00743641  6bc93c                IMUL ECX,ECX,0x3c
00743644  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
00743647  8b0402                MOV EAX,dword ptr [EDX + EAX*0x1]
0074364a  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
0074364d  0fbe92aa1b0000        MOVSX EDX,byte ptr [EDX + 0x1baa]
00743654  6bd20c                IMUL EDX,EDX,0xc
00743657  8b75d0                MOV ESI,dword ptr [EBP + -0x30]
0074365a  8d541624              LEA EDX,[ESI + EDX*0x1 + 0x24]
0074365e  8b75fc                MOV ESI,dword ptr [EBP + -0x4]
00743661  8b440808              MOV EAX,dword ptr [EAX + ECX*0x1 + 0x8]
00743665  3b04b2                CMP EAX,dword ptr [EDX + ESI*0x4]
00743668  0f85bc000000          JNZ 0x0074372a
0074366e  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00743671  6bc90c                IMUL ECX,ECX,0xc
00743674  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
00743677  8b440a04              MOV EAX,dword ptr [EDX + ECX*0x1 + 0x4]
0074367b  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0074367e  33c9                  XOR ECX,ECX
00743680  837de401              CMP dword ptr [EBP + -0x1c],0x1
00743684  0f9fc1                SETG CL
00743687  51                    PUSH ECX
00743688  6a00                  PUSH 0x0
0074368a  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0074368d  69d250070000          IMUL EDX,EDX,0x750
00743693  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
00743696  8d4c1054              LEA ECX,[EAX + EDX*0x1 + 0x54]
0074369a  e8c1dfffff            CALL 0x00741660
0074369f  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007436a2  6bc90c                IMUL ECX,ECX,0xc
007436a5  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
007436a8  6bd23c                IMUL EDX,EDX,0x3c
007436ab  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
007436ae  8b0c08                MOV ECX,dword ptr [EAX + ECX*0x1]
007436b1  8b541130              MOV EDX,dword ptr [ECX + EDX*0x1 + 0x30]
007436b5  83e201                AND EDX,0x1
007436b8  740f                  JZ 0x007436c9
007436ba  837d0c00              CMP dword ptr [EBP + 0xc],0x0
007436be  7509                  JNZ 0x007436c9
007436c0  c745cc01000000        MOV dword ptr [EBP + -0x34],0x1
007436c7  eb07                  JMP 0x007436d0
007436c9  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
007436d0  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
007436d3  6bc03c                IMUL EAX,EAX,0x3c
007436d6  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007436d9  6bc90c                IMUL ECX,ECX,0xc
007436dc  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
007436df  03040a                ADD EAX,dword ptr [EDX + ECX*0x1]
007436e2  8945e0                MOV dword ptr [EBP + -0x20],EAX
007436e5  6a00                  PUSH 0x0
007436e7  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
007436ea  50                    PUSH EAX
007436eb  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
007436ee  51                    PUSH ECX
007436ef  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
007436f2  6bd20c                IMUL EDX,EDX,0xc
007436f5  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
007436f8  6bc03c                IMUL EAX,EAX,0x3c
007436fb  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
007436fe  8b1411                MOV EDX,dword ptr [ECX + EDX*0x1]
00743701  8d440220              LEA EAX,[EDX + EAX*0x1 + 0x20]
00743705  50                    PUSH EAX
00743706  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00743709  69c950070000          IMUL ECX,ECX,0x750
0074370f  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
00743712  8d4c0a54              LEA ECX,[EDX + ECX*0x1 + 0x54]
00743716  e885e1ffff            CALL 0x007418a0
0074371b  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0074371e  8945f4                MOV dword ptr [EBP + -0xc],EAX
00743721  c745f801000000        MOV dword ptr [EBP + -0x8],0x1
00743728  eb05                  JMP 0x0074372f
0074372a  e9e4feffff            JMP 0x00743613
0074372f  837df800              CMP dword ptr [EBP + -0x8],0x0
00743733  0f85ed000000          JNZ 0x00743826
00743739  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0074373c  6bc90c                IMUL ECX,ECX,0xc
0074373f  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
00743742  8b440a04              MOV EAX,dword ptr [EDX + ECX*0x1 + 0x4]
00743746  8945dc                MOV dword ptr [EBP + -0x24],EAX
00743749  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0074374d  0f8eb3000000          JLE 0x00743806
00743753  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00743756  6bc90c                IMUL ECX,ECX,0xc
00743759  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
0074375c  8b440a04              MOV EAX,dword ptr [EDX + ECX*0x1 + 0x4]
00743760  8945d8                MOV dword ptr [EBP + -0x28],EAX
00743763  33c9                  XOR ECX,ECX
00743765  837dd801              CMP dword ptr [EBP + -0x28],0x1
00743769  0f9fc1                SETG CL
0074376c  51                    PUSH ECX
0074376d  6a00                  PUSH 0x0
0074376f  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00743772  69d250070000          IMUL EDX,EDX,0x750
00743778  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0074377b  8d4c1054              LEA ECX,[EAX + EDX*0x1 + 0x54]
0074377f  e8dcdeffff            CALL 0x00741660
00743784  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00743787  6bc90c                IMUL ECX,ECX,0xc
0074378a  33d2                  XOR EDX,EDX
0074378c  6bd23c                IMUL EDX,EDX,0x3c
0074378f  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
00743792  8b0c08                MOV ECX,dword ptr [EAX + ECX*0x1]
00743795  8b541130              MOV EDX,dword ptr [ECX + EDX*0x1 + 0x30]
00743799  83e201                AND EDX,0x1
0074379c  740f                  JZ 0x007437ad
0074379e  837d0c00              CMP dword ptr [EBP + 0xc],0x0
007437a2  7509                  JNZ 0x007437ad
007437a4  c745c801000000        MOV dword ptr [EBP + -0x38],0x1
007437ab  eb07                  JMP 0x007437b4
007437ad  c745c800000000        MOV dword ptr [EBP + -0x38],0x0
007437b4  33c0                  XOR EAX,EAX
007437b6  6bc03c                IMUL EAX,EAX,0x3c
007437b9  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007437bc  6bc90c                IMUL ECX,ECX,0xc
007437bf  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
007437c2  03040a                ADD EAX,dword ptr [EDX + ECX*0x1]
007437c5  8945d4                MOV dword ptr [EBP + -0x2c],EAX
007437c8  6a00                  PUSH 0x0
007437ca  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
007437cd  50                    PUSH EAX
007437ce  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
007437d1  51                    PUSH ECX
007437d2  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
007437d5  6bd20c                IMUL EDX,EDX,0xc
007437d8  33c0                  XOR EAX,EAX
007437da  6bc03c                IMUL EAX,EAX,0x3c
007437dd  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
007437e0  8b1411                MOV EDX,dword ptr [ECX + EDX*0x1]
007437e3  8d440220              LEA EAX,[EDX + EAX*0x1 + 0x20]
007437e7  50                    PUSH EAX
007437e8  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007437eb  69c950070000          IMUL ECX,ECX,0x750
007437f1  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
007437f4  8d4c0a54              LEA ECX,[EDX + ECX*0x1 + 0x54]
007437f8  e8a3e0ffff            CALL 0x007418a0
007437fd  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
00743804  eb20                  JMP 0x00743826
00743806  6a00                  PUSH 0x0
00743808  6a01                  PUSH 0x1
0074380a  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0074380d  69c050070000          IMUL EAX,EAX,0x750
00743813  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00743816  8d4c0154              LEA ECX,[ECX + EAX*0x1 + 0x54]
0074381a  e841deffff            CALL 0x00741660
0074381f  c745f4ffffffff        MOV dword ptr [EBP + -0xc],0xffffffff
00743826  e9c5fdffff            JMP 0x007435f0
0074382b  8b5510                MOV EDX,dword ptr [EBP + 0x10]
0074382e  52                    PUSH EDX
0074382f  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00743832  e8c9f1ffff            CALL 0x00742a00
00743837  8b4510                MOV EAX,dword ptr [EBP + 0x10]
0074383a  50                    PUSH EAX
0074383b  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0074383e  51                    PUSH ECX
0074383f  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00743842  e809000000            CALL 0x00743850
00743847  5e                    POP ESI
00743848  8be5                  MOV ESP,EBP
0074384a  5d                    POP EBP
0074384b  c20c00                RET 0xc
