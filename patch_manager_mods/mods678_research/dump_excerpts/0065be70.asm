; FN 0065be70 FUN_0065be70
0065be70  55                    PUSH EBP
0065be71  8bec                  MOV EBP,ESP
0065be73  6aff                  PUSH -0x1
0065be75  68aa939500            PUSH 0x9593aa
0065be7a  64a100000000          MOV EAX,FS:[0x0]
0065be80  50                    PUSH EAX
0065be81  81ece8030000          SUB ESP,0x3e8
0065be87  a1201fa100            MOV EAX,[0x00a11f20]
0065be8c  33c5                  XOR EAX,EBP
0065be8e  50                    PUSH EAX
0065be8f  8d45f4                LEA EAX,[EBP + -0xc]
0065be92  64a300000000          MOV FS:[0x0],EAX
0065be98  898d24fcffff          MOV dword ptr [EBP + 0xfffffc24],ECX
0065be9e  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
0065bea5  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
0065beac  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
0065beb3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065beb6  e8656bfdff            CALL 0x00632a20
0065bebb  8945e8                MOV dword ptr [EBP + -0x18],EAX
0065bebe  837de800              CMP dword ptr [EBP + -0x18],0x0
0065bec2  7422                  JZ 0x0065bee6
0065bec4  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0065bec7  8b10                  MOV EDX,dword ptr [EAX]
0065bec9  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0065becc  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065becf  ffd0                  CALL EAX
0065bed1  8945f0                MOV dword ptr [EBP + -0x10],EAX
0065bed4  837df000              CMP dword ptr [EBP + -0x10],0x0
0065bed8  740c                  JZ 0x0065bee6
0065beda  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065bedd  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
0065bee3  8955ec                MOV dword ptr [EBP + -0x14],EDX
0065bee6  837df000              CMP dword ptr [EBP + -0x10],0x0
0065beea  741f                  JZ 0x0065bf0b
0065beec  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0065beef  8b88f0100000          MOV ECX,dword ptr [EAX + 0x10f0]
0065bef5  898da0fdffff          MOV dword ptr [EBP + 0xfffffda0],ECX
0065befb  83bda0fdffff00        CMP dword ptr [EBP + 0xfffffda0],0x0
0065bf02  7407                  JZ 0x0065bf0b
0065bf04  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
0065bf0b  6a01                  PUSH 0x1
0065bf0d  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bf13  e8c87ef8ff            CALL 0x005e3de0
0065bf18  85c0                  TEST EAX,EAX
0065bf1a  7407                  JZ 0x0065bf23
0065bf1c  33c0                  XOR EAX,EAX
0065bf1e  e967260000            JMP 0x0065e58a
0065bf23  0fb6550c              MOVZX EDX,byte ptr [EBP + 0xc]
0065bf27  899520fcffff          MOV dword ptr [EBP + 0xfffffc20],EDX
0065bf2d  8b8520fcffff          MOV EAX,dword ptr [EBP + 0xfffffc20]
0065bf33  83e801                SUB EAX,0x1
0065bf36  898520fcffff          MOV dword ptr [EBP + 0xfffffc20],EAX
0065bf3c  83bd20fcffff26        CMP dword ptr [EBP + 0xfffffc20],0x26
0065bf43  0f873f260000          JA 0x0065e588
0065bf49  8b8d20fcffff          MOV ECX,dword ptr [EBP + 0xfffffc20]
0065bf4f  ff248d9ce56500        JMP dword ptr [ECX*0x4 + 0x65e59c]
0065bf56  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0065bf59  52                    PUSH EDX
0065bf5a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bf60  e88bf5ffff            CALL 0x0065b4f0
0065bf65  e920260000            JMP 0x0065e58a
0065bf6f  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0065bf72  50                    PUSH EAX
0065bf73  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bf79  e8f2f2ffff            CALL 0x0065b270
0065bf7e  e907260000            JMP 0x0065e58a
0065bf88  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065bf8b  51                    PUSH ECX
0065bf8c  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bf92  e839f4ffff            CALL 0x0065b3d0
0065bf97  e9ee250000            JMP 0x0065e58a
0065bfa1  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0065bfa4  52                    PUSH EDX
0065bfa5  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bfab  e8e0f4ffff            CALL 0x0065b490
0065bfb0  e9d5250000            JMP 0x0065e58a
0065bfba  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bfc0  e82b6ffdff            CALL 0x00632ef0
0065bfc5  8945d8                MOV dword ptr [EBP + -0x28],EAX
0065bfc8  6a10                  PUSH 0x10
0065bfca  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bfd0  e85b78f8ff            CALL 0x005e3830
0065bfd5  668945dc              MOV word ptr [EBP + -0x24],AX
0065bfd9  6a01                  PUSH 0x1
0065bfdb  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bfe1  e8fa7df8ff            CALL 0x005e3de0
0065bfe6  85c0                  TEST EAX,EAX
0065bfe8  7517                  JNZ 0x0065c001
0065bfea  6a01                  PUSH 0x1
0065bfec  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065bff2  e8497ef8ff            CALL 0x005e3e40
0065bff7  85c0                  TEST EAX,EAX
0065bff9  7506                  JNZ 0x0065c001
0065bffb  837df000              CMP dword ptr [EBP + -0x10],0x0
0065bfff  7507                  JNZ 0x0065c008
0065c001  33c0                  XOR EAX,EAX
0065c003  e982250000            JMP 0x0065e58a
0065c008  6834899900            PUSH 0x998934
0065c00d  8d8dc8fdffff          LEA ECX,[EBP + 0xfffffdc8]
0065c013  e858750d00            CALL 0x00733570
0065c018  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0065c01f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065c024  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065c027  898d9cfdffff          MOV dword ptr [EBP + 0xfffffd9c],ECX
0065c02d  6a00                  PUSH 0x0
0065c02f  8d95c8fdffff          LEA EDX,[EBP + 0xfffffdc8]
0065c035  52                    PUSH EDX
0065c036  8b8d9cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd9c]
0065c03c  e84ffdebff            CALL 0x0051bd90
0065c041  8bc8                  MOV ECX,EAX
0065c043  e8482bf0ff            CALL 0x0055eb90
0065c048  8945e0                MOV dword ptr [EBP + -0x20],EAX
0065c04b  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0065c052  8d8dc8fdffff          LEA ECX,[EBP + 0xfffffdc8]
0065c058  e823770d00            CALL 0x00733780
0065c05d  817de00000007f        CMP dword ptr [EBP + -0x20],0x7f000000
0065c064  744f                  JZ 0x0065c0b5
0065c066  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
0065c069  3b45e0                CMP EAX,dword ptr [EBP + -0x20]
0065c06c  7547                  JNZ 0x0065c0b5
0065c06e  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065c074  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0065c077  899598fdffff          MOV dword ptr [EBP + 0xfffffd98],EDX
0065c07d  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
0065c080  50                    PUSH EAX
0065c081  8b8d98fdffff          MOV ECX,dword ptr [EBP + 0xfffffd98]
0065c087  e81400ecff            CALL 0x0051c0a0
0065c08c  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0065c08f  837dd400              CMP dword ptr [EBP + -0x2c],0x0
0065c093  7420                  JZ 0x0065c0b5
0065c095  6a34                  PUSH 0x34
0065c097  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0065c09a  81c114010000          ADD ECX,0x114
0065c0a0  e86bd1f8ff            CALL 0x005e9210
0065c0a5  8945d0                MOV dword ptr [EBP + -0x30],EAX
0065c0a8  837dd000              CMP dword ptr [EBP + -0x30],0x0
0065c0ac  7407                  JZ 0x0065c0b5
0065c0ae  33c0                  XOR EAX,EAX
0065c0b0  e9d5240000            JMP 0x0065e58a
0065c0b5  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
0065c0bc  6a00                  PUSH 0x0
0065c0be  6a04                  PUSH 0x4
0065c0c0  6a00                  PUSH 0x0
0065c0c2  6a00                  PUSH 0x0
0065c0c4  6a01                  PUSH 0x1
0065c0c6  68dc050000            PUSH 0x5dc
0065c0cb  6819270000            PUSH 0x2719
0065c0d0  6a01                  PUSH 0x1
0065c0d2  6a00                  PUSH 0x0
0065c0d4  6a00                  PUSH 0x0
0065c0d6  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0065c0d9  51                    PUSH ECX
0065c0da  6a00                  PUSH 0x0
0065c0dc  6a01                  PUSH 0x1
0065c0de  0fb755dc              MOVZX EDX,word ptr [EBP + -0x24]
0065c0e2  52                    PUSH EDX
0065c0e3  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
0065c0e6  50                    PUSH EAX
0065c0e7  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c0ea  e851c4f0ff            CALL 0x00568540
0065c0ef  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c0f2  8b91ac110000          MOV EDX,dword ptr [ECX + 0x11ac]
0065c0f8  899594fdffff          MOV dword ptr [EBP + 0xfffffd94],EDX
0065c0fe  83bd94fdffff00        CMP dword ptr [EBP + 0xfffffd94],0x0
0065c105  0f8460010000          JZ 0x0065c26b
0065c10b  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065c110  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065c113  898d90fdffff          MOV dword ptr [EBP + 0xfffffd90],ECX
0065c119  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
0065c11c  52                    PUSH EDX
0065c11d  8b8d90fdffff          MOV ECX,dword ptr [EBP + 0xfffffd90]
0065c123  e878ffebff            CALL 0x0051c0a0
0065c128  8945cc                MOV dword ptr [EBP + -0x34],EAX
0065c12b  c745c800000000        MOV dword ptr [EBP + -0x38],0x0
0065c132  837dcc00              CMP dword ptr [EBP + -0x34],0x0
0065c136  7418                  JZ 0x0065c150
0065c138  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0065c13b  8b10                  MOV EDX,dword ptr [EAX]
0065c13d  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065c140  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065c143  ffd0                  CALL EAX
0065c145  85c0                  TEST EAX,EAX
0065c147  7407                  JZ 0x0065c150
0065c149  c745c801000000        MOV dword ptr [EBP + -0x38],0x1
0065c150  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065c156  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0065c159  89958cfdffff          MOV dword ptr [EBP + 0xfffffd8c],EDX
0065c15f  8b8d8cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd8c]
0065c165  e8263a0e00            CALL 0x0073fb90
0065c16a  898588fdffff          MOV dword ptr [EBP + 0xfffffd88],EAX
0065c170  8b8588fdffff          MOV EAX,dword ptr [EBP + 0xfffffd88]
0065c176  8b08                  MOV ECX,dword ptr [EAX]
0065c178  894dc0                MOV dword ptr [EBP + -0x40],ECX
0065c17b  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0065c181  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0065c184  898584fdffff          MOV dword ptr [EBP + 0xfffffd84],EAX
0065c18a  8b8d84fdffff          MOV ECX,dword ptr [EBP + 0xfffffd84]
0065c190  e83b07ecff            CALL 0x0051c8d0
0065c195  898580fdffff          MOV dword ptr [EBP + 0xfffffd80],EAX
0065c19b  8b8d80fdffff          MOV ECX,dword ptr [EBP + 0xfffffd80]
0065c1a1  8b9138020000          MOV EDX,dword ptr [ECX + 0x238]
0065c1a7  89957cfdffff          MOV dword ptr [EBP + 0xfffffd7c],EDX
0065c1ad  83bd7cfdffff00        CMP dword ptr [EBP + 0xfffffd7c],0x0
0065c1b4  0f85b1000000          JNZ 0x0065c26b
0065c1ba  837dc800              CMP dword ptr [EBP + -0x38],0x0
0065c1be  0f84a7000000          JZ 0x0065c26b
0065c1c4  c745bc01000000        MOV dword ptr [EBP + -0x44],0x1
0065c1cb  eb09                  JMP 0x0065c1d6
0065c1cd  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0065c1d0  83c001                ADD EAX,0x1
0065c1d3  8945bc                MOV dword ptr [EBP + -0x44],EAX
0065c1d6  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0065c1d9  3b4dc0                CMP ECX,dword ptr [EBP + -0x40]
0065c1dc  0f8d89000000          JGE 0x0065c26b
0065c1e2  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0065c1e8  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0065c1eb  898578fdffff          MOV dword ptr [EBP + 0xfffffd78],EAX
0065c1f1  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0065c1f4  51                    PUSH ECX
0065c1f5  8b8d78fdffff          MOV ECX,dword ptr [EBP + 0xfffffd78]
0065c1fb  e890390e00            CALL 0x0073fb90
0065c200  8bc8                  MOV ECX,EAX
0065c202  e8999b1800            CALL 0x007e5da0
0065c207  8bc8                  MOV ECX,EAX
0065c209  e8f2151200            CALL 0x0077d800
0065c20e  8945c4                MOV dword ptr [EBP + -0x3c],EAX
0065c211  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0065c214  8b8250050000          MOV EAX,dword ptr [EDX + 0x550]
0065c21a  898574fdffff          MOV dword ptr [EBP + 0xfffffd74],EAX
0065c220  81bd74fdffff0000007f  CMP dword ptr [EBP + 0xfffffd74],0x7f000000
0065c22a  753a                  JNZ 0x0065c266
0065c22c  6a00                  PUSH 0x0
0065c22e  6a04                  PUSH 0x4
0065c230  6a00                  PUSH 0x0
0065c232  6a00                  PUSH 0x0
0065c234  6a01                  PUSH 0x1
0065c236  68dc050000            PUSH 0x5dc
0065c23b  6819270000            PUSH 0x2719
0065c240  6a01                  PUSH 0x1
0065c242  6a01                  PUSH 0x1
0065c244  6a01                  PUSH 0x1
0065c246  6a01                  PUSH 0x1
0065c248  6a00                  PUSH 0x0
0065c24a  6a00                  PUSH 0x0
0065c24c  6a00                  PUSH 0x0
0065c24e  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0065c251  51                    PUSH ECX
0065c252  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0065c255  e8e6c2f0ff            CALL 0x00568540
0065c25a  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0065c25d  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
0065c260  898254050000          MOV dword ptr [EDX + 0x554],EAX
0065c266  e962ffffff            JMP 0x0065c1cd
0065c26b  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c26e  8b9150050000          MOV EDX,dword ptr [ECX + 0x550]
0065c274  899570fdffff          MOV dword ptr [EBP + 0xfffffd70],EDX
0065c27a  8b8570fdffff          MOV EAX,dword ptr [EBP + 0xfffffd70]
0065c280  3b45d8                CMP EAX,dword ptr [EBP + -0x28]
0065c283  740c                  JZ 0x0065c291
0065c285  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c288  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
0065c28b  899154050000          MOV dword ptr [ECX + 0x554],EDX
0065c291  b801000000            MOV EAX,0x1
0065c296  e9ef220000            JMP 0x0065e58a
0065c2a0  6a10                  PUSH 0x10
0065c2a2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c2a8  e88375f8ff            CALL 0x005e3830
0065c2ad  668945b0              MOV word ptr [EBP + -0x50],AX
0065c2b1  6a10                  PUSH 0x10
0065c2b3  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c2b9  e87275f8ff            CALL 0x005e3830
0065c2be  668945b8              MOV word ptr [EBP + -0x48],AX
0065c2c2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c2c8  e8236cfdff            CALL 0x00632ef0
0065c2cd  8945b4                MOV dword ptr [EBP + -0x4c],EAX
0065c2d0  6a01                  PUSH 0x1
0065c2d2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c2d8  e8037bf8ff            CALL 0x005e3de0
0065c2dd  85c0                  TEST EAX,EAX
0065c2df  7511                  JNZ 0x0065c2f2
0065c2e1  6a01                  PUSH 0x1
0065c2e3  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c2e9  e8527bf8ff            CALL 0x005e3e40
0065c2ee  85c0                  TEST EAX,EAX
0065c2f0  7407                  JZ 0x0065c2f9
0065c2f2  33c0                  XOR EAX,EAX
0065c2f4  e991220000            JMP 0x0065e58a
0065c2f9  837df000              CMP dword ptr [EBP + -0x10],0x0
0065c2fd  7507                  JNZ 0x0065c306
0065c2ff  33c0                  XOR EAX,EAX
0065c301  e984220000            JMP 0x0065e58a
0065c306  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c309  e8b290eeff            CALL 0x005453c0
0065c30e  85c0                  TEST EAX,EAX
0065c310  7507                  JNZ 0x0065c319
0065c312  33c0                  XOR EAX,EAX
0065c314  e971220000            JMP 0x0065e58a
0065c319  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c31c  e89f90eeff            CALL 0x005453c0
0065c321  0520010000            ADD EAX,0x120
0065c326  89856cfdffff          MOV dword ptr [EBP + 0xfffffd6c],EAX
0065c32c  8b856cfdffff          MOV EAX,dword ptr [EBP + 0xfffffd6c]
0065c332  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0065c335  894dac                MOV dword ptr [EBP + -0x54],ECX
0065c338  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
0065c33b  52                    PUSH EDX
0065c33c  8b45b4                MOV EAX,dword ptr [EBP + -0x4c]
0065c33f  50                    PUSH EAX
0065c340  0fb74db8              MOVZX ECX,word ptr [EBP + -0x48]
0065c344  51                    PUSH ECX
0065c345  0fb755b0              MOVZX EDX,word ptr [EBP + -0x50]
0065c349  52                    PUSH EDX
0065c34a  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c34d  e8beecf1ff            CALL 0x0057b010
0065c352  b801000000            MOV EAX,0x1
0065c357  e92e220000            JMP 0x0065e58a
0065c361  6a08                  PUSH 0x8
0065c363  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c369  e8b273f8ff            CALL 0x005e3720
0065c36e  88459b                MOV byte ptr [EBP + -0x65],AL
0065c371  6a08                  PUSH 0x8
0065c373  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c379  e8a273f8ff            CALL 0x005e3720
0065c37e  884593                MOV byte ptr [EBP + -0x6d],AL
0065c381  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c387  e8646bfdff            CALL 0x00632ef0
0065c38c  8945a8                MOV dword ptr [EBP + -0x58],EAX
0065c38f  d9ee                  FLDZ
0065c391  d95d9c                FSTP float ptr [EBP + -0x64]
0065c394  d9ee                  FLDZ
0065c396  d95da0                FSTP float ptr [EBP + -0x60]
0065c399  d9ee                  FLDZ
0065c39b  d95da4                FSTP float ptr [EBP + -0x5c]
0065c39e  6a20                  PUSH 0x20
0065c3a0  51                    PUSH ECX
0065c3a1  d9e8                  FLD1
0065c3a3  d91c24                FSTP float ptr [ESP]
0065c3a6  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c3ac  e8af77f8ff            CALL 0x005e3b60
0065c3b1  d95d9c                FSTP float ptr [EBP + -0x64]
0065c3b4  6a20                  PUSH 0x20
0065c3b6  51                    PUSH ECX
0065c3b7  d9e8                  FLD1
0065c3b9  d91c24                FSTP float ptr [ESP]
0065c3bc  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c3c2  e89977f8ff            CALL 0x005e3b60
0065c3c7  d95da0                FSTP float ptr [EBP + -0x60]
0065c3ca  6a20                  PUSH 0x20
0065c3cc  51                    PUSH ECX
0065c3cd  d9e8                  FLD1
0065c3cf  d91c24                FSTP float ptr [ESP]
0065c3d2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c3d8  e88377f8ff            CALL 0x005e3b60
0065c3dd  d95da4                FSTP float ptr [EBP + -0x5c]
0065c3e0  6a01                  PUSH 0x1
0065c3e2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c3e8  e8f379f8ff            CALL 0x005e3de0
0065c3ed  85c0                  TEST EAX,EAX
0065c3ef  7511                  JNZ 0x0065c402
0065c3f1  6a01                  PUSH 0x1
0065c3f3  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c3f9  e8427af8ff            CALL 0x005e3e40
0065c3fe  85c0                  TEST EAX,EAX
0065c400  7407                  JZ 0x0065c409
0065c402  33c0                  XOR EAX,EAX
0065c404  e981210000            JMP 0x0065e58a
0065c409  837df000              CMP dword ptr [EBP + -0x10],0x0
0065c40d  7507                  JNZ 0x0065c416
0065c40f  33c0                  XOR EAX,EAX
0065c411  e974210000            JMP 0x0065e58a
0065c416  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c419  e8a28feeff            CALL 0x005453c0
0065c41e  0520010000            ADD EAX,0x120
0065c423  898568fdffff          MOV dword ptr [EBP + 0xfffffd68],EAX
0065c429  8b8568fdffff          MOV EAX,dword ptr [EBP + 0xfffffd68]
0065c42f  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0065c432  894d94                MOV dword ptr [EBP + -0x6c],ECX
0065c435  6a00                  PUSH 0x0
0065c437  680000007f            PUSH 0x7f000000
0065c43c  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
0065c43f  52                    PUSH EDX
0065c440  8d459c                LEA EAX,[EBP + -0x64]
0065c443  50                    PUSH EAX
0065c444  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
0065c447  51                    PUSH ECX
0065c448  0fb65593              MOVZX EDX,byte ptr [EBP + -0x6d]
0065c44c  52                    PUSH EDX
0065c44d  0fb6459b              MOVZX EAX,byte ptr [EBP + -0x65]
0065c451  50                    PUSH EAX
0065c452  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c455  e826eef1ff            CALL 0x0057b280
0065c45a  e929210000            JMP 0x0065e588
0065c45f  d9ee                  FLDZ
0065c461  d99d64ffffff          FSTP float ptr [EBP + 0xffffff64]
0065c467  d9ee                  FLDZ
0065c469  d99d68ffffff          FSTP float ptr [EBP + 0xffffff68]
0065c46f  d9ee                  FLDZ
0065c471  d99d6cffffff          FSTP float ptr [EBP + 0xffffff6c]
0065c477  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c47d  e86e6afdff            CALL 0x00632ef0
0065c482  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
0065c488  6a08                  PUSH 0x8
0065c48a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c490  e88b72f8ff            CALL 0x005e3720
0065c495  884583                MOV byte ptr [EBP + -0x7d],AL
0065c498  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c49e  e8fd71f8ff            CALL 0x005e36a0
0065c4a3  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
0065c4a9  83bd78ffffff00        CMP dword ptr [EBP + 0xffffff78],0x0
0065c4b0  7415                  JZ 0x0065c4c7
0065c4b2  6a08                  PUSH 0x8
0065c4b4  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c4ba  e86172f8ff            CALL 0x005e3720
0065c4bf  888573ffffff          MOV byte ptr [EBP + 0xffffff73],AL
0065c4c5  eb07                  JMP 0x0065c4ce
0065c4c7  c68573ffffff00        MOV byte ptr [EBP + 0xffffff73],0x0
0065c4ce  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c4d4  e8c771f8ff            CALL 0x005e36a0
0065c4d9  89458c                MOV dword ptr [EBP + -0x74],EAX
0065c4dc  837d8c01              CMP dword ptr [EBP + -0x74],0x1
0065c4e0  7510                  JNZ 0x0065c4f2
0065c4e2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c4e8  e8036afdff            CALL 0x00632ef0
0065c4ed  894584                MOV dword ptr [EBP + -0x7c],EAX
0065c4f0  eb09                  JMP 0x0065c4fb
0065c4f2  8b0d84449900          MOV ECX,dword ptr [0x00994484]
0065c4f8  894d84                MOV dword ptr [EBP + -0x7c],ECX
0065c4fb  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c501  e89a71f8ff            CALL 0x005e36a0
0065c506  894588                MOV dword ptr [EBP + -0x78],EAX
0065c509  837d8801              CMP dword ptr [EBP + -0x78],0x1
0065c50d  754b                  JNZ 0x0065c55a
0065c50f  6a20                  PUSH 0x20
0065c511  51                    PUSH ECX
0065c512  d9e8                  FLD1
0065c514  d91c24                FSTP float ptr [ESP]
0065c517  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c51d  e83e76f8ff            CALL 0x005e3b60
0065c522  d99d64ffffff          FSTP float ptr [EBP + 0xffffff64]
0065c528  6a20                  PUSH 0x20
0065c52a  51                    PUSH ECX
0065c52b  d9e8                  FLD1
0065c52d  d91c24                FSTP float ptr [ESP]
0065c530  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c536  e82576f8ff            CALL 0x005e3b60
0065c53b  d99d68ffffff          FSTP float ptr [EBP + 0xffffff68]
0065c541  6a20                  PUSH 0x20
0065c543  51                    PUSH ECX
0065c544  d9e8                  FLD1
0065c546  d91c24                FSTP float ptr [ESP]
0065c549  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c54f  e80c76f8ff            CALL 0x005e3b60
0065c554  d99d6cffffff          FSTP float ptr [EBP + 0xffffff6c]
0065c55a  6a01                  PUSH 0x1
0065c55c  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c562  e87978f8ff            CALL 0x005e3de0
0065c567  85c0                  TEST EAX,EAX
0065c569  7511                  JNZ 0x0065c57c
0065c56b  6a01                  PUSH 0x1
0065c56d  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c573  e8c878f8ff            CALL 0x005e3e40
0065c578  85c0                  TEST EAX,EAX
0065c57a  7407                  JZ 0x0065c583
0065c57c  33c0                  XOR EAX,EAX
0065c57e  e907200000            JMP 0x0065e58a
0065c583  837df000              CMP dword ptr [EBP + -0x10],0x0
0065c587  7507                  JNZ 0x0065c590
0065c589  33c0                  XOR EAX,EAX
0065c58b  e9fa1f0000            JMP 0x0065e58a
0065c590  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c593  e8288eeeff            CALL 0x005453c0
0065c598  0520010000            ADD EAX,0x120
0065c59d  898564fdffff          MOV dword ptr [EBP + 0xfffffd64],EAX
0065c5a3  8b9564fdffff          MOV EDX,dword ptr [EBP + 0xfffffd64]
0065c5a9  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0065c5ac  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
0065c5b2  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c5b5  8b9120050000          MOV EDX,dword ptr [ECX + 0x520]
0065c5bb  899560fdffff          MOV dword ptr [EBP + 0xfffffd60],EDX
0065c5c1  83bd60fdffff00        CMP dword ptr [EBP + 0xfffffd60],0x0
0065c5c8  750a                  JNZ 0x0065c5d4
0065c5ca  6a01                  PUSH 0x1
0065c5cc  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c5cf  e8bc76f0ff            CALL 0x00563c90
0065c5d4  6a01                  PUSH 0x1
0065c5d6  8b8574ffffff          MOV EAX,dword ptr [EBP + 0xffffff74]
0065c5dc  50                    PUSH EAX
0065c5dd  8d8d64ffffff          LEA ECX,[EBP + 0xffffff64]
0065c5e3  51                    PUSH ECX
0065c5e4  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
0065c5e7  52                    PUSH EDX
0065c5e8  0fb68573ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff73]
0065c5ef  50                    PUSH EAX
0065c5f0  0fb64d83              MOVZX ECX,byte ptr [EBP + -0x7d]
0065c5f4  51                    PUSH ECX
0065c5f5  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
0065c5fb  52                    PUSH EDX
0065c5fc  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c5ff  e8bceef1ff            CALL 0x0057b4c0
0065c604  85c0                  TEST EAX,EAX
0065c606  750c                  JNZ 0x0065c614
0065c608  6a00                  PUSH 0x0
0065c60a  6a17                  PUSH 0x17
0065c60c  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c60f  e88c87f2ff            CALL 0x00584da0
0065c614  e96f1f0000            JMP 0x0065e588
0065c619  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c61f  e8cc68fdff            CALL 0x00632ef0
0065c624  898548ffffff          MOV dword ptr [EBP + 0xffffff48],EAX
0065c62a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c630  e8bb68fdff            CALL 0x00632ef0
0065c635  898550ffffff          MOV dword ptr [EBP + 0xffffff50],EAX
0065c63b  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c641  e8aa68fdff            CALL 0x00632ef0
0065c646  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
0065c64c  6a20                  PUSH 0x20
0065c64e  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c654  e87773f8ff            CALL 0x005e39d0
0065c659  898558ffffff          MOV dword ptr [EBP + 0xffffff58],EAX
0065c65f  6a01                  PUSH 0x1
0065c661  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c667  e87477f8ff            CALL 0x005e3de0
0065c66c  85c0                  TEST EAX,EAX
0065c66e  7511                  JNZ 0x0065c681
0065c670  6a01                  PUSH 0x1
0065c672  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c678  e8c377f8ff            CALL 0x005e3e40
0065c67d  85c0                  TEST EAX,EAX
0065c67f  7407                  JZ 0x0065c688
0065c681  33c0                  XOR EAX,EAX
0065c683  e9021f0000            JMP 0x0065e58a
0065c688  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065c68d  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065c690  898d5cfdffff          MOV dword ptr [EBP + 0xfffffd5c],ECX
0065c696  8b9548ffffff          MOV EDX,dword ptr [EBP + 0xffffff48]
0065c69c  52                    PUSH EDX
0065c69d  8b8d5cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd5c]
0065c6a3  e858faebff            CALL 0x0051c100
0065c6a8  898554ffffff          MOV dword ptr [EBP + 0xffffff54],EAX
0065c6ae  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065c6b3  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065c6b6  898d58fdffff          MOV dword ptr [EBP + 0xfffffd58],ECX
0065c6bc  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0065c6c2  52                    PUSH EDX
0065c6c3  8b8d58fdffff          MOV ECX,dword ptr [EBP + 0xfffffd58]
0065c6c9  e812faebff            CALL 0x0051c0e0
0065c6ce  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
0065c6d4  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065c6d9  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065c6dc  898d54fdffff          MOV dword ptr [EBP + 0xfffffd54],ECX
0065c6e2  8b9560ffffff          MOV EDX,dword ptr [EBP + 0xffffff60]
0065c6e8  52                    PUSH EDX
0065c6e9  8b8d54fdffff          MOV ECX,dword ptr [EBP + 0xfffffd54]
0065c6ef  e86cfaebff            CALL 0x0051c160
0065c6f4  89854cffffff          MOV dword ptr [EBP + 0xffffff4c],EAX
0065c6fa  83bd54ffffff00        CMP dword ptr [EBP + 0xffffff54],0x0
0065c701  7412                  JZ 0x0065c715
0065c703  83bd5cffffff00        CMP dword ptr [EBP + 0xffffff5c],0x0
0065c70a  7409                  JZ 0x0065c715
0065c70c  83bd4cffffff00        CMP dword ptr [EBP + 0xffffff4c],0x0
0065c713  7507                  JNZ 0x0065c71c
0065c715  33c0                  XOR EAX,EAX
0065c717  e96e1e0000            JMP 0x0065e58a
0065c71c  6a00                  PUSH 0x0
0065c71e  8b8558ffffff          MOV EAX,dword ptr [EBP + 0xffffff58]
0065c724  50                    PUSH EAX
0065c725  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
0065c72b  51                    PUSH ECX
0065c72c  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0065c732  52                    PUSH EDX
0065c733  8b8d54ffffff          MOV ECX,dword ptr [EBP + 0xffffff54]
0065c739  e86248f8ff            CALL 0x005e0fa0
0065c73e  e9451e0000            JMP 0x0065e588
0065c743  6a08                  PUSH 0x8
0065c745  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c74b  e8d06ff8ff            CALL 0x005e3720
0065c750  888547ffffff          MOV byte ptr [EBP + 0xffffff47],AL
0065c756  6a01                  PUSH 0x1
0065c758  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c75e  e87d76f8ff            CALL 0x005e3de0
0065c763  85c0                  TEST EAX,EAX
0065c765  7407                  JZ 0x0065c76e
0065c767  33c0                  XOR EAX,EAX
0065c769  e91c1e0000            JMP 0x0065e58a
0065c76e  0fb68547ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff47]
0065c775  83f805                CMP EAX,0x5
0065c778  7511                  JNZ 0x0065c78b
0065c77a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c780  e86b67fdff            CALL 0x00632ef0
0065c785  898540ffffff          MOV dword ptr [EBP + 0xffffff40],EAX
0065c78b  6a01                  PUSH 0x1
0065c78d  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c793  e84876f8ff            CALL 0x005e3de0
0065c798  85c0                  TEST EAX,EAX
0065c79a  7511                  JNZ 0x0065c7ad
0065c79c  6a01                  PUSH 0x1
0065c79e  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c7a4  e89776f8ff            CALL 0x005e3e40
0065c7a9  85c0                  TEST EAX,EAX
0065c7ab  7407                  JZ 0x0065c7b4
0065c7ad  33c0                  XOR EAX,EAX
0065c7af  e9d61d0000            JMP 0x0065e58a
0065c7b4  837df000              CMP dword ptr [EBP + -0x10],0x0
0065c7b8  7418                  JZ 0x0065c7d2
0065c7ba  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c7bd  8b91ec000000          MOV EDX,dword ptr [ECX + 0xec]
0065c7c3  899550fdffff          MOV dword ptr [EBP + 0xfffffd50],EDX
0065c7c9  83bd50fdffff00        CMP dword ptr [EBP + 0xfffffd50],0x0
0065c7d0  7507                  JNZ 0x0065c7d9
0065c7d2  33c0                  XOR EAX,EAX
0065c7d4  e9b11d0000            JMP 0x0065e58a
0065c7d9  0fb68547ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff47]
0065c7e0  83f805                CMP EAX,0x5
0065c7e3  750f                  JNZ 0x0065c7f4
0065c7e5  8b8d40ffffff          MOV ECX,dword ptr [EBP + 0xffffff40]
0065c7eb  51                    PUSH ECX
0065c7ec  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c7ef  e8dc0cf2ff            CALL 0x0057d4d0
0065c7f4  0fb69547ffffff        MOVZX EDX,byte ptr [EBP + 0xffffff47]
0065c7fb  52                    PUSH EDX
0065c7fc  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c7ff  e83cf1f1ff            CALL 0x0057b940
0065c804  e97f1d0000            JMP 0x0065e588
0065c809  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c80f  e8dc66fdff            CALL 0x00632ef0
0065c814  89853cffffff          MOV dword ptr [EBP + 0xffffff3c],EAX
0065c81a  6a01                  PUSH 0x1
0065c81c  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c822  e8b975f8ff            CALL 0x005e3de0
0065c827  85c0                  TEST EAX,EAX
0065c829  7517                  JNZ 0x0065c842
0065c82b  6a01                  PUSH 0x1
0065c82d  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c833  e80876f8ff            CALL 0x005e3e40
0065c838  85c0                  TEST EAX,EAX
0065c83a  7506                  JNZ 0x0065c842
0065c83c  837df000              CMP dword ptr [EBP + -0x10],0x0
0065c840  7507                  JNZ 0x0065c849
0065c842  33c0                  XOR EAX,EAX
0065c844  e9411d0000            JMP 0x0065e58a
0065c849  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065c84e  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065c851  898d4cfdffff          MOV dword ptr [EBP + 0xfffffd4c],ECX
0065c857  8d9538ffffff          LEA EDX,[EBP + 0xffffff38]
0065c85d  52                    PUSH EDX
0065c85e  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
0065c864  50                    PUSH EAX
0065c865  8b8d4cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd4c]
0065c86b  e810f8ebff            CALL 0x0051c080
0065c870  8bc8                  MOV ECX,EAX
0065c872  e83917eeff            CALL 0x0053dfb0
0065c877  0fb6c8                MOVZX ECX,AL
0065c87a  0fb6157f449900        MOVZX EDX,byte ptr [0x0099447f]
0065c881  3bca                  CMP ECX,EDX
0065c883  0f85ca010000          JNZ 0x0065ca53
0065c889  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
0065c88f  0fb64808              MOVZX ECX,byte ptr [EAX + 0x8]
0065c893  898d1cfcffff          MOV dword ptr [EBP + 0xfffffc1c],ECX
0065c899  8b951cfcffff          MOV EDX,dword ptr [EBP + 0xfffffc1c]
0065c89f  83ea05                SUB EDX,0x5
0065c8a2  89951cfcffff          MOV dword ptr [EBP + 0xfffffc1c],EDX
0065c8a8  83bd1cfcffff05        CMP dword ptr [EBP + 0xfffffc1c],0x5
0065c8af  0f8709010000          JA 0x0065c9be
0065c8b5  8b851cfcffff          MOV EAX,dword ptr [EBP + 0xfffffc1c]
0065c8bb  ff248538e66500        JMP dword ptr [EAX*0x4 + 0x65e638]
0065c8c2  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
0065c8c8  51                    PUSH ECX
0065c8c9  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0065c8cc  52                    PUSH EDX
0065c8cd  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c8d3  e8888bfeff            CALL 0x00645460
0065c8d8  e9ad1c0000            JMP 0x0065e58a
0065c8e2  837df000              CMP dword ptr [EBP + -0x10],0x0
0065c8e6  740f                  JZ 0x0065c8f7
0065c8e8  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
0065c8ee  50                    PUSH EAX
0065c8ef  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c8f2  e8b9d8f2ff            CALL 0x0058a1b0
0065c8f7  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
0065c8fd  51                    PUSH ECX
0065c8fe  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0065c901  52                    PUSH EDX
0065c902  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c908  e8238ffeff            CALL 0x00645830
0065c90d  e9781c0000            JMP 0x0065e58a
0065c917  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
0065c91d  50                    PUSH EAX
0065c91e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065c921  51                    PUSH ECX
0065c922  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c928  e8d392feff            CALL 0x00645c00
0065c92d  e9581c0000            JMP 0x0065e58a
0065c937  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c93a  e8818aeeff            CALL 0x005453c0
0065c93f  0520010000            ADD EAX,0x120
0065c944  898548fdffff          MOV dword ptr [EBP + 0xfffffd48],EAX
0065c94a  8b9548fdffff          MOV EDX,dword ptr [EBP + 0xfffffd48]
0065c950  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0065c953  89852cffffff          MOV dword ptr [EBP + 0xffffff2c],EAX
0065c959  d9ee                  FLDZ
0065c95b  d99dbcfdffff          FSTP float ptr [EBP + 0xfffffdbc]
0065c961  d9ee                  FLDZ
0065c963  d99dc0fdffff          FSTP float ptr [EBP + 0xfffffdc0]
0065c969  d9ee                  FLDZ
0065c96b  d99dc4fdffff          FSTP float ptr [EBP + 0xfffffdc4]
0065c971  6a00                  PUSH 0x0
0065c973  680000007f            PUSH 0x7f000000
0065c978  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
0065c97e  51                    PUSH ECX
0065c97f  8d95bcfdffff          LEA EDX,[EBP + 0xfffffdbc]
0065c985  52                    PUSH EDX
0065c986  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
0065c98c  50                    PUSH EAX
0065c98d  6a66                  PUSH 0x66
0065c98f  6a01                  PUSH 0x1
0065c991  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065c994  e8e7e8f1ff            CALL 0x0057b280
0065c999  e9b5000000            JMP 0x0065ca53
0065c99e  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
0065c9a4  51                    PUSH ECX
0065c9a5  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0065c9a8  52                    PUSH EDX
0065c9a9  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065c9af  e87c97feff            CALL 0x00646130
0065c9b4  e9d11b0000            JMP 0x0065e58a
0065c9be  68f0889900            PUSH 0x9988f0
0065c9c3  8d8d30ffffff          LEA ECX,[EBP + 0xffffff30]
0065c9c9  e8a26b0d00            CALL 0x00733570
0065c9ce  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
0065c9d5  6825699800            PUSH 0x986925
0065c9da  8d8db4fdffff          LEA ECX,[EBP + 0xfffffdb4]
0065c9e0  e88b6b0d00            CALL 0x00733570
0065c9e5  c645fc02              MOV byte ptr [EBP + -0x4],0x2
0065c9e9  8d85b4fdffff          LEA EAX,[EBP + 0xfffffdb4]
0065c9ef  50                    PUSH EAX
0065c9f0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065c9f3  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0065c9f6  899544fdffff          MOV dword ptr [EBP + 0xfffffd44],EDX
0065c9fc  8b8544fdffff          MOV EAX,dword ptr [EBP + 0xfffffd44]
0065ca02  50                    PUSH EAX
0065ca03  83ec08                SUB ESP,0x8
0065ca06  8bcc                  MOV ECX,ESP
0065ca08  89a5b0fdffff          MOV dword ptr [EBP + 0xfffffdb0],ESP
0065ca0e  8d9530ffffff          LEA EDX,[EBP + 0xffffff30]
0065ca14  52                    PUSH EDX
0065ca15  e8d66b0d00            CALL 0x007335f0
0065ca1a  898518fcffff          MOV dword ptr [EBP + 0xfffffc18],EAX
0065ca20  680000007f            PUSH 0x7f000000
0065ca25  6a05                  PUSH 0x5
0065ca27  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065ca2d  e87edbfdff            CALL 0x0063a5b0
0065ca32  c645fc01              MOV byte ptr [EBP + -0x4],0x1
0065ca36  8d8db4fdffff          LEA ECX,[EBP + 0xfffffdb4]
0065ca3c  e83f6d0d00            CALL 0x00733780
0065ca41  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0065ca48  8d8d30ffffff          LEA ECX,[EBP + 0xffffff30]
0065ca4e  e82d6d0d00            CALL 0x00733780
0065ca53  b801000000            MOV EAX,0x1
0065ca58  e92d1b0000            JMP 0x0065e58a
0065ca62  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065ca68  e88364fdff            CALL 0x00632ef0
0065ca6d  898528ffffff          MOV dword ptr [EBP + 0xffffff28],EAX
0065ca73  6a01                  PUSH 0x1
0065ca75  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065ca7b  e86073f8ff            CALL 0x005e3de0
0065ca80  85c0                  TEST EAX,EAX
0065ca82  7511                  JNZ 0x0065ca95
0065ca84  6a01                  PUSH 0x1
0065ca86  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065ca8c  e8af73f8ff            CALL 0x005e3e40
0065ca91  85c0                  TEST EAX,EAX
0065ca93  7407                  JZ 0x0065ca9c
0065ca95  33c0                  XOR EAX,EAX
0065ca97  e9ee1a0000            JMP 0x0065e58a
0065ca9c  837df000              CMP dword ptr [EBP + -0x10],0x0
0065caa0  7507                  JNZ 0x0065caa9
0065caa2  33c0                  XOR EAX,EAX
0065caa4  e9e11a0000            JMP 0x0065e58a
0065caa9  6a02                  PUSH 0x2
0065caab  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065caae  e8dd71f0ff            CALL 0x00563c90
0065cab3  8b8528ffffff          MOV EAX,dword ptr [EBP + 0xffffff28]
0065cab9  50                    PUSH EAX
0065caba  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cabd  e86e44f8ff            CALL 0x005e0f30
0065cac2  b801000000            MOV EAX,0x1
0065cac7  e9be1a0000            JMP 0x0065e58a
0065cad1  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cad7  e81464fdff            CALL 0x00632ef0
0065cadc  898524ffffff          MOV dword ptr [EBP + 0xffffff24],EAX
0065cae2  6a10                  PUSH 0x10
0065cae4  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065caea  e8416df8ff            CALL 0x005e3830
0065caef  66898520ffffff        MOV word ptr [EBP + 0xffffff20],AX
0065caf6  6a01                  PUSH 0x1
0065caf8  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cafe  e8dd72f8ff            CALL 0x005e3de0
0065cb03  85c0                  TEST EAX,EAX
0065cb05  7511                  JNZ 0x0065cb18
0065cb07  6a01                  PUSH 0x1
0065cb09  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cb0f  e82c73f8ff            CALL 0x005e3e40
0065cb14  85c0                  TEST EAX,EAX
0065cb16  7407                  JZ 0x0065cb1f
0065cb18  33c0                  XOR EAX,EAX
0065cb1a  e96b1a0000            JMP 0x0065e58a
0065cb1f  837df000              CMP dword ptr [EBP + -0x10],0x0
0065cb23  7507                  JNZ 0x0065cb2c
0065cb25  33c0                  XOR EAX,EAX
0065cb27  e95e1a0000            JMP 0x0065e58a
0065cb2c  6a02                  PUSH 0x2
0065cb2e  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cb31  e85a71f0ff            CALL 0x00563c90
0065cb36  0fb78d20ffffff        MOVZX ECX,word ptr [EBP + 0xffffff20]
0065cb3d  81f925270000          CMP ECX,0x2725
0065cb43  7513                  JNZ 0x0065cb58
0065cb45  6a00                  PUSH 0x0
0065cb47  8b9524ffffff          MOV EDX,dword ptr [EBP + 0xffffff24]
0065cb4d  52                    PUSH EDX
0065cb4e  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cb51  e86aadeeff            CALL 0x005478c0
0065cb56  eb11                  JMP 0x0065cb69
0065cb58  6a00                  PUSH 0x0
0065cb5a  8b8524ffffff          MOV EAX,dword ptr [EBP + 0xffffff24]
0065cb60  50                    PUSH EAX
0065cb61  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cb64  e8d7adeeff            CALL 0x00547940
0065cb69  b801000000            MOV EAX,0x1
0065cb6e  e9171a0000            JMP 0x0065e58a
0065cb78  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cb7e  e86d63fdff            CALL 0x00632ef0
0065cb83  89851cffffff          MOV dword ptr [EBP + 0xffffff1c],EAX
0065cb89  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cb8f  e85c63fdff            CALL 0x00632ef0
0065cb94  89850cffffff          MOV dword ptr [EBP + 0xffffff0c],EAX
0065cb9a  6a01                  PUSH 0x1
0065cb9c  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cba2  e83972f8ff            CALL 0x005e3de0
0065cba7  85c0                  TEST EAX,EAX
0065cba9  7511                  JNZ 0x0065cbbc
0065cbab  6a01                  PUSH 0x1
0065cbad  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cbb3  e88872f8ff            CALL 0x005e3e40
0065cbb8  85c0                  TEST EAX,EAX
0065cbba  7407                  JZ 0x0065cbc3
0065cbbc  33c0                  XOR EAX,EAX
0065cbbe  e9c7190000            JMP 0x0065e58a
0065cbc3  837df000              CMP dword ptr [EBP + -0x10],0x0
0065cbc7  7418                  JZ 0x0065cbe1
0065cbc9  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cbcc  8b91ec000000          MOV EDX,dword ptr [ECX + 0xec]
0065cbd2  899540fdffff          MOV dword ptr [EBP + 0xfffffd40],EDX
0065cbd8  83bd40fdffff00        CMP dword ptr [EBP + 0xfffffd40],0x0
0065cbdf  7507                  JNZ 0x0065cbe8
0065cbe1  33c0                  XOR EAX,EAX
0065cbe3  e9a2190000            JMP 0x0065e58a
0065cbe8  6a02                  PUSH 0x2
0065cbea  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cbed  e89e70f0ff            CALL 0x00563c90
0065cbf2  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065cbf7  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065cbfa  898d3cfdffff          MOV dword ptr [EBP + 0xfffffd3c],ECX
0065cc00  8d9514ffffff          LEA EDX,[EBP + 0xffffff14]
0065cc06  52                    PUSH EDX
0065cc07  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
0065cc0d  50                    PUSH EAX
0065cc0e  8b8d3cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd3c]
0065cc14  e867f4ebff            CALL 0x0051c080
0065cc19  8bc8                  MOV ECX,EAX
0065cc1b  e89013eeff            CALL 0x0053dfb0
0065cc20  0fb6c8                MOVZX ECX,AL
0065cc23  0fb6157f449900        MOVZX EDX,byte ptr [0x0099447f]
0065cc2a  3bca                  CMP ECX,EDX
0065cc2c  0f85c6000000          JNZ 0x0065ccf8
0065cc32  8b8514ffffff          MOV EAX,dword ptr [EBP + 0xffffff14]
0065cc38  8b10                  MOV EDX,dword ptr [EAX]
0065cc3a  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
0065cc40  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0065cc43  ffd0                  CALL EAX
0065cc45  85c0                  TEST EAX,EAX
0065cc47  744c                  JZ 0x0065cc95
0065cc49  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
0065cc4f  8b11                  MOV EDX,dword ptr [ECX]
0065cc51  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
0065cc57  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0065cc5a  ffd0                  CALL EAX
0065cc5c  898518ffffff          MOV dword ptr [EBP + 0xffffff18],EAX
0065cc62  8b8d18ffffff          MOV ECX,dword ptr [EBP + 0xffffff18]
0065cc68  8b9104030000          MOV EDX,dword ptr [ECX + 0x304]
0065cc6e  899538fdffff          MOV dword ptr [EBP + 0xfffffd38],EDX
0065cc74  83bd38fdffff00        CMP dword ptr [EBP + 0xfffffd38],0x0
0065cc7b  7418                  JZ 0x0065cc95
0065cc7d  6a00                  PUSH 0x0
0065cc7f  8b850cffffff          MOV EAX,dword ptr [EBP + 0xffffff0c]
0065cc85  50                    PUSH EAX
0065cc86  8b8d1cffffff          MOV ECX,dword ptr [EBP + 0xffffff1c]
0065cc8c  51                    PUSH ECX
0065cc8d  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cc90  e82badeeff            CALL 0x005479c0
0065cc95  8b9514ffffff          MOV EDX,dword ptr [EBP + 0xffffff14]
0065cc9b  8b02                  MOV EAX,dword ptr [EDX]
0065cc9d  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
0065cca3  8b5048                MOV EDX,dword ptr [EAX + 0x48]
0065cca6  ffd2                  CALL EDX
0065cca8  85c0                  TEST EAX,EAX
0065ccaa  744c                  JZ 0x0065ccf8
0065ccac  8b8514ffffff          MOV EAX,dword ptr [EBP + 0xffffff14]
0065ccb2  8b10                  MOV EDX,dword ptr [EAX]
0065ccb4  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
0065ccba  8b4248                MOV EAX,dword ptr [EDX + 0x48]
0065ccbd  ffd0                  CALL EAX
0065ccbf  898510ffffff          MOV dword ptr [EBP + 0xffffff10],EAX
0065ccc5  8b8d10ffffff          MOV ECX,dword ptr [EBP + 0xffffff10]
0065cccb  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
0065ccd1  899534fdffff          MOV dword ptr [EBP + 0xfffffd34],EDX
0065ccd7  83bd34fdffff00        CMP dword ptr [EBP + 0xfffffd34],0x0
0065ccde  7418                  JZ 0x0065ccf8
0065cce0  6a00                  PUSH 0x0
0065cce2  8b850cffffff          MOV EAX,dword ptr [EBP + 0xffffff0c]
0065cce8  50                    PUSH EAX
0065cce9  8b8d1cffffff          MOV ECX,dword ptr [EBP + 0xffffff1c]
0065ccef  51                    PUSH ECX
0065ccf0  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065ccf3  e8c8aceeff            CALL 0x005479c0
0065ccf8  b801000000            MOV EAX,0x1
0065ccfd  e988180000            JMP 0x0065e58a
0065cd07  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cd0d  e8de61fdff            CALL 0x00632ef0
0065cd12  898508ffffff          MOV dword ptr [EBP + 0xffffff08],EAX
0065cd18  6a01                  PUSH 0x1
0065cd1a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cd20  e8bb70f8ff            CALL 0x005e3de0
0065cd25  85c0                  TEST EAX,EAX
0065cd27  7511                  JNZ 0x0065cd3a
0065cd29  6a01                  PUSH 0x1
0065cd2b  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cd31  e80a71f8ff            CALL 0x005e3e40
0065cd36  85c0                  TEST EAX,EAX
0065cd38  7407                  JZ 0x0065cd41
0065cd3a  33c0                  XOR EAX,EAX
0065cd3c  e949180000            JMP 0x0065e58a
0065cd41  837df000              CMP dword ptr [EBP + -0x10],0x0
0065cd45  7507                  JNZ 0x0065cd4e
0065cd47  33c0                  XOR EAX,EAX
0065cd49  e93c180000            JMP 0x0065e58a
0065cd4e  6a02                  PUSH 0x2
0065cd50  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cd53  e8386ff0ff            CALL 0x00563c90
0065cd58  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0065cd5e  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0065cd61  898530fdffff          MOV dword ptr [EBP + 0xfffffd30],EAX
0065cd67  8d8d00ffffff          LEA ECX,[EBP + 0xffffff00]
0065cd6d  51                    PUSH ECX
0065cd6e  8b9508ffffff          MOV EDX,dword ptr [EBP + 0xffffff08]
0065cd74  52                    PUSH EDX
0065cd75  8b8d30fdffff          MOV ECX,dword ptr [EBP + 0xfffffd30]
0065cd7b  e800f3ebff            CALL 0x0051c080
0065cd80  8bc8                  MOV ECX,EAX
0065cd82  e82912eeff            CALL 0x0053dfb0
0065cd87  0fb6c0                MOVZX EAX,AL
0065cd8a  0fb60d7f449900        MOVZX ECX,byte ptr [0x0099447f]
0065cd91  3bc1                  CMP EAX,ECX
0065cd93  0f85b4000000          JNZ 0x0065ce4d
0065cd99  8b9500ffffff          MOV EDX,dword ptr [EBP + 0xffffff00]
0065cd9f  8b02                  MOV EAX,dword ptr [EDX]
0065cda1  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0065cda7  8b5018                MOV EDX,dword ptr [EAX + 0x18]
0065cdaa  ffd2                  CALL EDX
0065cdac  85c0                  TEST EAX,EAX
0065cdae  7443                  JZ 0x0065cdf3
0065cdb0  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
0065cdb6  8b10                  MOV EDX,dword ptr [EAX]
0065cdb8  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0065cdbe  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0065cdc1  ffd0                  CALL EAX
0065cdc3  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
0065cdc9  8b8d04ffffff          MOV ECX,dword ptr [EBP + 0xffffff04]
0065cdcf  8b9104030000          MOV EDX,dword ptr [ECX + 0x304]
0065cdd5  89952cfdffff          MOV dword ptr [EBP + 0xfffffd2c],EDX
0065cddb  83bd2cfdffff00        CMP dword ptr [EBP + 0xfffffd2c],0x0
0065cde2  750f                  JNZ 0x0065cdf3
0065cde4  8b8508ffffff          MOV EAX,dword ptr [EBP + 0xffffff08]
0065cdea  50                    PUSH EAX
0065cdeb  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cdee  e89daceeff            CALL 0x00547a90
0065cdf3  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0065cdf9  8b11                  MOV EDX,dword ptr [ECX]
0065cdfb  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0065ce01  8b4248                MOV EAX,dword ptr [EDX + 0x48]
0065ce04  ffd0                  CALL EAX
0065ce06  85c0                  TEST EAX,EAX
0065ce08  7443                  JZ 0x0065ce4d
0065ce0a  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0065ce10  8b11                  MOV EDX,dword ptr [ECX]
0065ce12  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0065ce18  8b4248                MOV EAX,dword ptr [EDX + 0x48]
0065ce1b  ffd0                  CALL EAX
0065ce1d  8985fcfeffff          MOV dword ptr [EBP + 0xfffffefc],EAX
0065ce23  8b8dfcfeffff          MOV ECX,dword ptr [EBP + 0xfffffefc]
0065ce29  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
0065ce2f  899528fdffff          MOV dword ptr [EBP + 0xfffffd28],EDX
0065ce35  83bd28fdffff00        CMP dword ptr [EBP + 0xfffffd28],0x0
0065ce3c  750f                  JNZ 0x0065ce4d
0065ce3e  8b8508ffffff          MOV EAX,dword ptr [EBP + 0xffffff08]
0065ce44  50                    PUSH EAX
0065ce45  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065ce48  e843aceeff            CALL 0x00547a90
0065ce4d  b801000000            MOV EAX,0x1
0065ce52  e933170000            JMP 0x0065e58a
0065ce5c  6a10                  PUSH 0x10
0065ce5e  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065ce64  e8c769f8ff            CALL 0x005e3830
0065ce69  668985d8feffff        MOV word ptr [EBP + 0xfffffed8],AX
0065ce70  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065ce76  e87560fdff            CALL 0x00632ef0
0065ce7b  8985e8feffff          MOV dword ptr [EBP + 0xfffffee8],EAX
0065ce81  6a20                  PUSH 0x20
0065ce83  51                    PUSH ECX
0065ce84  d9e8                  FLD1
0065ce86  d91c24                FSTP float ptr [ESP]
0065ce89  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065ce8f  e8cc6cf8ff            CALL 0x005e3b60
0065ce94  d99de4feffff          FSTP float ptr [EBP + 0xfffffee4]
0065ce9a  6a20                  PUSH 0x20
0065ce9c  51                    PUSH ECX
0065ce9d  d9e8                  FLD1
0065ce9f  d91c24                FSTP float ptr [ESP]
0065cea2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cea8  e8b36cf8ff            CALL 0x005e3b60
0065cead  d99de0feffff          FSTP float ptr [EBP + 0xfffffee0]
0065ceb3  6a20                  PUSH 0x20
0065ceb5  51                    PUSH ECX
0065ceb6  d9e8                  FLD1
0065ceb8  d91c24                FSTP float ptr [ESP]
0065cebb  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cec1  e89a6cf8ff            CALL 0x005e3b60
0065cec6  d99ddcfeffff          FSTP float ptr [EBP + 0xfffffedc]
0065cecc  d9e8                  FLD1
0065cece  d99decfeffff          FSTP float ptr [EBP + 0xfffffeec]
0065ced4  d9ee                  FLDZ
0065ced6  d99df8feffff          FSTP float ptr [EBP + 0xfffffef8]
0065cedc  0fb78dd8feffff        MOVZX ECX,word ptr [EBP + 0xfffffed8]
0065cee3  898df0feffff          MOV dword ptr [EBP + 0xfffffef0],ECX
0065cee9  c785f4feffff01000000  MOV dword ptr [EBP + 0xfffffef4],0x1
0065cef3  6a01                  PUSH 0x1
0065cef5  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cefb  e8e06ef8ff            CALL 0x005e3de0
0065cf00  85c0                  TEST EAX,EAX
0065cf02  7511                  JNZ 0x0065cf15
0065cf04  6a01                  PUSH 0x1
0065cf06  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065cf0c  e82f6ff8ff            CALL 0x005e3e40
0065cf11  85c0                  TEST EAX,EAX
0065cf13  7407                  JZ 0x0065cf1c
0065cf15  33c0                  XOR EAX,EAX
0065cf17  e96e160000            JMP 0x0065e58a
0065cf1c  837df000              CMP dword ptr [EBP + -0x10],0x0
0065cf20  7418                  JZ 0x0065cf3a
0065cf22  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0065cf25  8b82ec000000          MOV EAX,dword ptr [EDX + 0xec]
0065cf2b  898524fdffff          MOV dword ptr [EBP + 0xfffffd24],EAX
0065cf31  83bd24fdffff00        CMP dword ptr [EBP + 0xfffffd24],0x0
0065cf38  7507                  JNZ 0x0065cf41
0065cf3a  33c0                  XOR EAX,EAX
0065cf3c  e949160000            JMP 0x0065e58a
0065cf41  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cf44  668b9120110000        MOV DX,word ptr [ECX + 0x1120]
0065cf4b  66899522fdffff        MOV word ptr [EBP + 0xfffffd22],DX
0065cf52  0fb78522fdffff        MOVZX EAX,word ptr [EBP + 0xfffffd22]
0065cf59  83e008                AND EAX,0x8
0065cf5c  740a                  JZ 0x0065cf68
0065cf5e  6a01                  PUSH 0x1
0065cf60  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cf63  e81841eeff            CALL 0x00541080
0065cf68  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cf6b  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0065cf6e  89951cfdffff          MOV dword ptr [EBP + 0xfffffd1c],EDX
0065cf74  8b851cfdffff          MOV EAX,dword ptr [EBP + 0xfffffd1c]
0065cf7a  3b85e8feffff          CMP EAX,dword ptr [EBP + 0xfffffee8]
0065cf80  755c                  JNZ 0x0065cfde
0065cf82  6a00                  PUSH 0x0
0065cf84  6a00                  PUSH 0x0
0065cf86  6a00                  PUSH 0x0
0065cf88  6a00                  PUSH 0x0
0065cf8a  6a00                  PUSH 0x0
0065cf8c  6a00                  PUSH 0x0
0065cf8e  6a00                  PUSH 0x0
0065cf90  6a00                  PUSH 0x0
0065cf92  6a00                  PUSH 0x0
0065cf94  6a00                  PUSH 0x0
0065cf96  6a00                  PUSH 0x0
0065cf98  6a00                  PUSH 0x0
0065cf9a  6a00                  PUSH 0x0
0065cf9c  6a00                  PUSH 0x0
0065cf9e  6a00                  PUSH 0x0
0065cfa0  6a00                  PUSH 0x0
0065cfa2  6a00                  PUSH 0x0
0065cfa4  6a00                  PUSH 0x0
0065cfa6  8d8df4feffff          LEA ECX,[EBP + 0xfffffef4]
0065cfac  51                    PUSH ECX
0065cfad  6a01                  PUSH 0x1
0065cfaf  8d95f8feffff          LEA EDX,[EBP + 0xfffffef8]
0065cfb5  52                    PUSH EDX
0065cfb6  6a02                  PUSH 0x2
0065cfb8  8d85ecfeffff          LEA EAX,[EBP + 0xfffffeec]
0065cfbe  50                    PUSH EAX
0065cfbf  6a02                  PUSH 0x2
0065cfc1  8d8df0feffff          LEA ECX,[EBP + 0xfffffef0]
0065cfc7  51                    PUSH ECX
0065cfc8  6a01                  PUSH 0x1
0065cfca  68ffff0000            PUSH 0xffff
0065cfcf  6a06                  PUSH 0x6
0065cfd1  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065cfd4  e8c73ceeff            CALL 0x00540ca0
0065cfd9  e9ff000000            JMP 0x0065d0dd
0065cfde  81bde8feffff0000007f  CMP dword ptr [EBP + 0xfffffee8],0x7f000000
0065cfe8  744a                  JZ 0x0065d034
0065cfea  6a00                  PUSH 0x0
0065cfec  6a00                  PUSH 0x0
0065cfee  6a00                  PUSH 0x0
0065cff0  6a00                  PUSH 0x0
0065cff2  6a00                  PUSH 0x0
0065cff4  6a00                  PUSH 0x0
0065cff6  6a00                  PUSH 0x0
0065cff8  6a00                  PUSH 0x0
0065cffa  6a00                  PUSH 0x0
0065cffc  6a00                  PUSH 0x0
0065cffe  6a00                  PUSH 0x0
0065d000  6a00                  PUSH 0x0
0065d002  6a00                  PUSH 0x0
0065d004  6a00                  PUSH 0x0
0065d006  6a00                  PUSH 0x0
0065d008  6a00                  PUSH 0x0
0065d00a  6a00                  PUSH 0x0
0065d00c  6a00                  PUSH 0x0
0065d00e  6a00                  PUSH 0x0
0065d010  6a00                  PUSH 0x0
0065d012  6a00                  PUSH 0x0
0065d014  6a00                  PUSH 0x0
0065d016  6a00                  PUSH 0x0
0065d018  6a00                  PUSH 0x0
0065d01a  8d95e8feffff          LEA EDX,[EBP + 0xfffffee8]
0065d020  52                    PUSH EDX
0065d021  6a03                  PUSH 0x3
0065d023  68ffff0000            PUSH 0xffff
0065d028  6a13                  PUSH 0x13
0065d02a  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d02d  e8be27eeff            CALL 0x0053f7f0
0065d032  eb52                  JMP 0x0065d086
0065d034  6a00                  PUSH 0x0
0065d036  6a00                  PUSH 0x0
0065d038  6a00                  PUSH 0x0
0065d03a  6a00                  PUSH 0x0
0065d03c  6a00                  PUSH 0x0
0065d03e  6a00                  PUSH 0x0
0065d040  6a00                  PUSH 0x0
0065d042  6a00                  PUSH 0x0
0065d044  6a00                  PUSH 0x0
0065d046  6a00                  PUSH 0x0
0065d048  6a00                  PUSH 0x0
0065d04a  6a00                  PUSH 0x0
0065d04c  6a00                  PUSH 0x0
0065d04e  6a00                  PUSH 0x0
0065d050  6a00                  PUSH 0x0
0065d052  6a00                  PUSH 0x0
0065d054  6a00                  PUSH 0x0
0065d056  6a00                  PUSH 0x0
0065d058  6a00                  PUSH 0x0
0065d05a  6a00                  PUSH 0x0
0065d05c  8d85dcfeffff          LEA EAX,[EBP + 0xfffffedc]
0065d062  50                    PUSH EAX
0065d063  6a02                  PUSH 0x2
0065d065  8d8de0feffff          LEA ECX,[EBP + 0xfffffee0]
0065d06b  51                    PUSH ECX
0065d06c  6a02                  PUSH 0x2
0065d06e  8d95e4feffff          LEA EDX,[EBP + 0xfffffee4]
0065d074  52                    PUSH EDX
0065d075  6a02                  PUSH 0x2
0065d077  68ffff0000            PUSH 0xffff
0065d07c  6a31                  PUSH 0x31
0065d07e  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d081  e86a27eeff            CALL 0x0053f7f0
0065d086  6a00                  PUSH 0x0
0065d088  6a00                  PUSH 0x0
0065d08a  6a00                  PUSH 0x0
0065d08c  6a00                  PUSH 0x0
0065d08e  6a00                  PUSH 0x0
0065d090  6a00                  PUSH 0x0
0065d092  6a00                  PUSH 0x0
0065d094  6a00                  PUSH 0x0
0065d096  6a00                  PUSH 0x0
0065d098  6a00                  PUSH 0x0
0065d09a  6a00                  PUSH 0x0
0065d09c  6a00                  PUSH 0x0
0065d09e  6a00                  PUSH 0x0
0065d0a0  6a00                  PUSH 0x0
0065d0a2  6a00                  PUSH 0x0
0065d0a4  6a00                  PUSH 0x0
0065d0a6  6a00                  PUSH 0x0
0065d0a8  6a00                  PUSH 0x0
0065d0aa  8d85f4feffff          LEA EAX,[EBP + 0xfffffef4]
0065d0b0  50                    PUSH EAX
0065d0b1  6a01                  PUSH 0x1
0065d0b3  8d8df8feffff          LEA ECX,[EBP + 0xfffffef8]
0065d0b9  51                    PUSH ECX
0065d0ba  6a02                  PUSH 0x2
0065d0bc  8d95ecfeffff          LEA EDX,[EBP + 0xfffffeec]
0065d0c2  52                    PUSH EDX
0065d0c3  6a02                  PUSH 0x2
0065d0c5  8d85f0feffff          LEA EAX,[EBP + 0xfffffef0]
0065d0cb  50                    PUSH EAX
0065d0cc  6a01                  PUSH 0x1
0065d0ce  68feff0000            PUSH 0xfffe
0065d0d3  6a06                  PUSH 0x6
0065d0d5  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d0d8  e81327eeff            CALL 0x0053f7f0
0065d0dd  b801000000            MOV EAX,0x1
0065d0e2  e9a3140000            JMP 0x0065e58a
0065d0ec  837df000              CMP dword ptr [EBP + -0x10],0x0
0065d0f0  7507                  JNZ 0x0065d0f9
0065d0f2  33c0                  XOR EAX,EAX
0065d0f4  e991140000            JMP 0x0065e58a
0065d0f9  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d0ff  e8ec5dfdff            CALL 0x00632ef0
0065d104  8985d4feffff          MOV dword ptr [EBP + 0xfffffed4],EAX
0065d10a  c785c4feffff0000007f  MOV dword ptr [EBP + 0xfffffec4],0x7f000000
0065d114  6a08                  PUSH 0x8
0065d116  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d119  e8726bf0ff            CALL 0x00563c90
0065d11e  6825699800            PUSH 0x986925
0065d123  8d8dbcfeffff          LEA ECX,[EBP + 0xfffffebc]
0065d129  e842640d00            CALL 0x00733570
0065d12e  c745fc03000000        MOV dword ptr [EBP + -0x4],0x3
0065d135  c785ccfeffff00000000  MOV dword ptr [EBP + 0xfffffecc],0x0
0065d13f  c785d0feffff01000000  MOV dword ptr [EBP + 0xfffffed0],0x1
0065d149  c785c8feffff00000000  MOV dword ptr [EBP + 0xfffffec8],0x0
0065d153  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d156  8b5178                MOV EDX,dword ptr [ECX + 0x78]
0065d159  899518fdffff          MOV dword ptr [EBP + 0xfffffd18],EDX
0065d15f  83bd18fdffff00        CMP dword ptr [EBP + 0xfffffd18],0x0
0065d166  7526                  JNZ 0x0065d18e
0065d168  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065d16d  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065d170  898d14fdffff          MOV dword ptr [EBP + 0xfffffd14],ECX
0065d176  6a01                  PUSH 0x1
0065d178  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0065d17b  52                    PUSH EDX
0065d17c  8b8d14fdffff          MOV ECX,dword ptr [EBP + 0xfffffd14]
0065d182  e8f9f0ebff            CALL 0x0051c280
0065d187  8bc8                  MOV ECX,EAX
0065d189  e8920fecff            CALL 0x0051e120
0065d18e  6a00                  PUSH 0x0
0065d190  6a00                  PUSH 0x0
0065d192  6a00                  PUSH 0x0
0065d194  6a00                  PUSH 0x0
0065d196  6a00                  PUSH 0x0
0065d198  6a00                  PUSH 0x0
0065d19a  6a00                  PUSH 0x0
0065d19c  6a00                  PUSH 0x0
0065d19e  6a00                  PUSH 0x0
0065d1a0  6a00                  PUSH 0x0
0065d1a2  6a00                  PUSH 0x0
0065d1a4  6a00                  PUSH 0x0
0065d1a6  6a00                  PUSH 0x0
0065d1a8  6a00                  PUSH 0x0
0065d1aa  8d85c4feffff          LEA EAX,[EBP + 0xfffffec4]
0065d1b0  50                    PUSH EAX
0065d1b1  6a03                  PUSH 0x3
0065d1b3  8d8dc8feffff          LEA ECX,[EBP + 0xfffffec8]
0065d1b9  51                    PUSH ECX
0065d1ba  6a01                  PUSH 0x1
0065d1bc  8d95d0feffff          LEA EDX,[EBP + 0xfffffed0]
0065d1c2  52                    PUSH EDX
0065d1c3  6a01                  PUSH 0x1
0065d1c5  8d85ccfeffff          LEA EAX,[EBP + 0xfffffecc]
0065d1cb  50                    PUSH EAX
0065d1cc  6a01                  PUSH 0x1
0065d1ce  8d8dbcfeffff          LEA ECX,[EBP + 0xfffffebc]
0065d1d4  51                    PUSH ECX
0065d1d5  6a04                  PUSH 0x4
0065d1d7  8d95d4feffff          LEA EDX,[EBP + 0xfffffed4]
0065d1dd  52                    PUSH EDX
0065d1de  6a03                  PUSH 0x3
0065d1e0  68ffff0000            PUSH 0xffff
0065d1e5  6a18                  PUSH 0x18
0065d1e7  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0065d1ea  e80126eeff            CALL 0x0053f7f0
0065d1ef  c785acfdffff01000000  MOV dword ptr [EBP + 0xfffffdac],0x1
0065d1f9  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0065d200  8d8dbcfeffff          LEA ECX,[EBP + 0xfffffebc]
0065d206  e875650d00            CALL 0x00733780
0065d20b  8b85acfdffff          MOV EAX,dword ptr [EBP + 0xfffffdac]
0065d211  e974130000            JMP 0x0065e58a
0065d21b  837df000              CMP dword ptr [EBP + -0x10],0x0
0065d21f  7418                  JZ 0x0065d239
0065d221  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0065d224  8b88ec000000          MOV ECX,dword ptr [EAX + 0xec]
0065d22a  898d10fdffff          MOV dword ptr [EBP + 0xfffffd10],ECX
0065d230  83bd10fdffff00        CMP dword ptr [EBP + 0xfffffd10],0x0
0065d237  7516                  JNZ 0x0065d24f
0065d239  6a00                  PUSH 0x0
0065d23b  68d5000000            PUSH 0xd5
0065d240  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d243  e8587bf2ff            CALL 0x00584da0
0065d248  33c0                  XOR EAX,EAX
0065d24a  e93b130000            JMP 0x0065e58a
0065d24f  6a08                  PUSH 0x8
0065d251  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d254  e8176bf0ff            CALL 0x00563d70
0065d259  85c0                  TEST EAX,EAX
0065d25b  750a                  JNZ 0x0065d267
0065d25d  6a00                  PUSH 0x0
0065d25f  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d262  e8090cf2ff            CALL 0x0057de70
0065d267  e91c130000            JMP 0x0065e588
0065d271  6a08                  PUSH 0x8
0065d273  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d279  e8a264f8ff            CALL 0x005e3720
0065d27e  8885bbfeffff          MOV byte ptr [EBP + 0xfffffebb],AL
0065d284  6a20                  PUSH 0x20
0065d286  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d28c  e8bf66f8ff            CALL 0x005e3950
0065d291  8985b4feffff          MOV dword ptr [EBP + 0xfffffeb4],EAX
0065d297  6a08                  PUSH 0x8
0065d299  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d29f  e87c64f8ff            CALL 0x005e3720
0065d2a4  8885b2feffff          MOV byte ptr [EBP + 0xfffffeb2],AL
0065d2aa  6a08                  PUSH 0x8
0065d2ac  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d2b2  e86964f8ff            CALL 0x005e3720
0065d2b7  8885bafeffff          MOV byte ptr [EBP + 0xfffffeba],AL
0065d2bd  6a08                  PUSH 0x8
0065d2bf  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d2c5  e85664f8ff            CALL 0x005e3720
0065d2ca  8885b3feffff          MOV byte ptr [EBP + 0xfffffeb3],AL
0065d2d0  6a01                  PUSH 0x1
0065d2d2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d2d8  e8036bf8ff            CALL 0x005e3de0
0065d2dd  85c0                  TEST EAX,EAX
0065d2df  7511                  JNZ 0x0065d2f2
0065d2e1  6a01                  PUSH 0x1
0065d2e3  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d2e9  e8526bf8ff            CALL 0x005e3e40
0065d2ee  85c0                  TEST EAX,EAX
0065d2f0  7407                  JZ 0x0065d2f9
0065d2f2  33c0                  XOR EAX,EAX
0065d2f4  e991120000            JMP 0x0065e58a
0065d2f9  837df000              CMP dword ptr [EBP + -0x10],0x0
0065d2fd  7507                  JNZ 0x0065d306
0065d2ff  33c0                  XOR EAX,EAX
0065d301  e984120000            JMP 0x0065e58a
0065d306  e97d120000            JMP 0x0065e588
0065d30b  6a08                  PUSH 0x8
0065d30d  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d313  e80864f8ff            CALL 0x005e3720
0065d318  8885b1feffff          MOV byte ptr [EBP + 0xfffffeb1],AL
0065d31e  6a08                  PUSH 0x8
0065d320  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d326  e8f563f8ff            CALL 0x005e3720
0065d32b  8885b0feffff          MOV byte ptr [EBP + 0xfffffeb0],AL
0065d331  6a08                  PUSH 0x8
0065d333  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d339  e8e263f8ff            CALL 0x005e3720
0065d33e  8885affeffff          MOV byte ptr [EBP + 0xfffffeaf],AL
0065d344  6a01                  PUSH 0x1
0065d346  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d34c  e88f6af8ff            CALL 0x005e3de0
0065d351  85c0                  TEST EAX,EAX
0065d353  7511                  JNZ 0x0065d366
0065d355  6a01                  PUSH 0x1
0065d357  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d35d  e8de6af8ff            CALL 0x005e3e40
0065d362  85c0                  TEST EAX,EAX
0065d364  7407                  JZ 0x0065d36d
0065d366  33c0                  XOR EAX,EAX
0065d368  e91d120000            JMP 0x0065e58a
0065d36d  837df000              CMP dword ptr [EBP + -0x10],0x0
0065d371  7507                  JNZ 0x0065d37a
0065d373  33c0                  XOR EAX,EAX
0065d375  e910120000            JMP 0x0065e58a
0065d37a  e909120000            JMP 0x0065e588
0065d37f  c78580feffff00000000  MOV dword ptr [EBP + 0xfffffe80],0x0
0065d389  d9ee                  FLDZ
0065d38b  d99da0feffff          FSTP float ptr [EBP + 0xfffffea0]
0065d391  d9ee                  FLDZ
0065d393  d99da4feffff          FSTP float ptr [EBP + 0xfffffea4]
0065d399  d9ee                  FLDZ
0065d39b  d99da8feffff          FSTP float ptr [EBP + 0xfffffea8]
0065d3a1  c78594feffff0000007f  MOV dword ptr [EBP + 0xfffffe94],0x7f000000
0065d3ab  6a20                  PUSH 0x20
0065d3ad  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d3b3  e89865f8ff            CALL 0x005e3950
0065d3b8  898598feffff          MOV dword ptr [EBP + 0xfffffe98],EAX
0065d3be  6a08                  PUSH 0x8
0065d3c0  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d3c6  e85563f8ff            CALL 0x005e3720
0065d3cb  8885aefeffff          MOV byte ptr [EBP + 0xfffffeae],AL
0065d3d1  6a08                  PUSH 0x8
0065d3d3  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d3d9  e84263f8ff            CALL 0x005e3720
0065d3de  88859ffeffff          MOV byte ptr [EBP + 0xfffffe9f],AL
0065d3e4  6a08                  PUSH 0x8
0065d3e6  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d3ec  e82f63f8ff            CALL 0x005e3720
0065d3f1  888587feffff          MOV byte ptr [EBP + 0xfffffe87],AL
0065d3f7  6a08                  PUSH 0x8
0065d3f9  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d3ff  e81c63f8ff            CALL 0x005e3720
0065d404  888593feffff          MOV byte ptr [EBP + 0xfffffe93],AL
0065d40a  0fb69593feffff        MOVZX EDX,byte ptr [EBP + 0xfffffe93]
0065d411  83e201                AND EDX,0x1
0065d414  7411                  JZ 0x0065d427
0065d416  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d41c  e8cf5afdff            CALL 0x00632ef0
0065d421  898594feffff          MOV dword ptr [EBP + 0xfffffe94],EAX
0065d427  0fb68593feffff        MOVZX EAX,byte ptr [EBP + 0xfffffe93]
0065d42e  83e002                AND EAX,0x2
0065d431  7455                  JZ 0x0065d488
0065d433  6a20                  PUSH 0x20
0065d435  51                    PUSH ECX
0065d436  d9e8                  FLD1
0065d438  d91c24                FSTP float ptr [ESP]
0065d43b  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d441  e81a67f8ff            CALL 0x005e3b60
0065d446  d99da0feffff          FSTP float ptr [EBP + 0xfffffea0]
0065d44c  6a20                  PUSH 0x20
0065d44e  51                    PUSH ECX
0065d44f  d9e8                  FLD1
0065d451  d91c24                FSTP float ptr [ESP]
0065d454  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d45a  e80167f8ff            CALL 0x005e3b60
0065d45f  d99da4feffff          FSTP float ptr [EBP + 0xfffffea4]
0065d465  6a20                  PUSH 0x20
0065d467  51                    PUSH ECX
0065d468  d9e8                  FLD1
0065d46a  d91c24                FSTP float ptr [ESP]
0065d46d  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d473  e8e866f8ff            CALL 0x005e3b60
0065d478  d99da8feffff          FSTP float ptr [EBP + 0xfffffea8]
0065d47e  c78580feffff01000000  MOV dword ptr [EBP + 0xfffffe80],0x1
0065d488  0fb68d93feffff        MOVZX ECX,byte ptr [EBP + 0xfffffe93]
0065d48f  83e104                AND ECX,0x4
0065d492  f7d9                  NEG ECX
0065d494  1bc9                  SBB ECX,ECX
0065d496  f7d9                  NEG ECX
0065d498  898d8cfeffff          MOV dword ptr [EBP + 0xfffffe8c],ECX
0065d49e  6a01                  PUSH 0x1
0065d4a0  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d4a6  e83569f8ff            CALL 0x005e3de0
0065d4ab  85c0                  TEST EAX,EAX
0065d4ad  7511                  JNZ 0x0065d4c0
0065d4af  6a01                  PUSH 0x1
0065d4b1  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d4b7  e88469f8ff            CALL 0x005e3e40
0065d4bc  85c0                  TEST EAX,EAX
0065d4be  7407                  JZ 0x0065d4c7
0065d4c0  33c0                  XOR EAX,EAX
0065d4c2  e9c3100000            JMP 0x0065e58a
0065d4c7  837df000              CMP dword ptr [EBP + -0x10],0x0
0065d4cb  7406                  JZ 0x0065d4d3
0065d4cd  837dec00              CMP dword ptr [EBP + -0x14],0x0
0065d4d1  7507                  JNZ 0x0065d4da
0065d4d3  33c0                  XOR EAX,EAX
0065d4d5  e9b0100000            JMP 0x0065e58a
0065d4da  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
0065d4e0  89950cfdffff          MOV dword ptr [EBP + 0xfffffd0c],EDX
0065d4e6  8b8598feffff          MOV EAX,dword ptr [EBP + 0xfffffe98]
0065d4ec  50                    PUSH EAX
0065d4ed  8b8d0cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd0c]
0065d4f3  8b8904010000          MOV ECX,dword ptr [ECX + 0x104]
0065d4f9  e8523f0600            CALL 0x006c1450
0065d4fe  898588feffff          MOV dword ptr [EBP + 0xfffffe88],EAX
0065d504  83bd8cfeffff00        CMP dword ptr [EBP + 0xfffffe8c],0x0
0065d50b  0f8538020000          JNZ 0x0065d749
0065d511  0fb69587feffff        MOVZX EDX,byte ptr [EBP + 0xfffffe87]
0065d518  52                    PUSH EDX
0065d519  0fb6859ffeffff        MOVZX EAX,byte ptr [EBP + 0xfffffe9f]
0065d520  50                    PUSH EAX
0065d521  0fb68daefeffff        MOVZX ECX,byte ptr [EBP + 0xfffffeae]
0065d528  51                    PUSH ECX
0065d529  8b9598feffff          MOV EDX,dword ptr [EBP + 0xfffffe98]
0065d52f  52                    PUSH EDX
0065d530  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0065d533  e808ba0500            CALL 0x006b8f40
0065d538  0fb6c0                MOVZX EAX,AL
0065d53b  85c0                  TEST EAX,EAX
0065d53d  0f8e06020000          JLE 0x0065d749
0065d543  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d546  8b9120050000          MOV EDX,dword ptr [ECX + 0x520]
0065d54c  899508fdffff          MOV dword ptr [EBP + 0xfffffd08],EDX
0065d552  83bd08fdffff01        CMP dword ptr [EBP + 0xfffffd08],0x1
0065d559  740a                  JZ 0x0065d565
0065d55b  6a01                  PUSH 0x1
0065d55d  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d560  e82b67f0ff            CALL 0x00563c90
0065d565  6a00                  PUSH 0x0
0065d567  68ff000000            PUSH 0xff
0065d56c  6aff                  PUSH -0x1
0065d56e  6a00                  PUSH 0x0
0065d570  6a00                  PUSH 0x0
0065d572  6a00                  PUSH 0x0
0065d574  6a01                  PUSH 0x1
0065d576  8b8580feffff          MOV EAX,dword ptr [EBP + 0xfffffe80]
0065d57c  50                    PUSH EAX
0065d57d  6a01                  PUSH 0x1
0065d57f  8b8d94feffff          MOV ECX,dword ptr [EBP + 0xfffffe94]
0065d585  51                    PUSH ECX
0065d586  83ec0c                SUB ESP,0xc
0065d589  8bd4                  MOV EDX,ESP
0065d58b  8b85a0feffff          MOV EAX,dword ptr [EBP + 0xfffffea0]
0065d591  8902                  MOV dword ptr [EDX],EAX
0065d593  8b8da4feffff          MOV ECX,dword ptr [EBP + 0xfffffea4]
0065d599  894a04                MOV dword ptr [EDX + 0x4],ECX
0065d59c  8b85a8feffff          MOV EAX,dword ptr [EBP + 0xfffffea8]
0065d5a2  894208                MOV dword ptr [EDX + 0x8],EAX
0065d5a5  8b8d8cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe8c]
0065d5ab  51                    PUSH ECX
0065d5ac  0fb69587feffff        MOVZX EDX,byte ptr [EBP + 0xfffffe87]
0065d5b3  52                    PUSH EDX
0065d5b4  0fb6859ffeffff        MOVZX EAX,byte ptr [EBP + 0xfffffe9f]
0065d5bb  50                    PUSH EAX
0065d5bc  0fb68daefeffff        MOVZX ECX,byte ptr [EBP + 0xfffffeae]
0065d5c3  51                    PUSH ECX
0065d5c4  8b9598feffff          MOV EDX,dword ptr [EBP + 0xfffffe98]
0065d5ca  52                    PUSH EDX
0065d5cb  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d5ce  e83d7df0ff            CALL 0x00565310
0065d5d3  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0065d5d6  8b88ac110000          MOV ECX,dword ptr [EAX + 0x11ac]
0065d5dc  898d04fdffff          MOV dword ptr [EBP + 0xfffffd04],ECX
0065d5e2  83bd04fdffff00        CMP dword ptr [EBP + 0xfffffd04],0x0
0065d5e9  0f845a010000          JZ 0x0065d749
0065d5ef  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0065d5f5  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0065d5f8  898500fdffff          MOV dword ptr [EBP + 0xfffffd00],EAX
0065d5fe  8b8d00fdffff          MOV ECX,dword ptr [EBP + 0xfffffd00]
0065d604  e8c7f2ebff            CALL 0x0051c8d0
0065d609  89857cfeffff          MOV dword ptr [EBP + 0xfffffe7c],EAX
0065d60f  83bd7cfeffff00        CMP dword ptr [EBP + 0xfffffe7c],0x0
0065d616  0f842d010000          JZ 0x0065d749
0065d61c  8b8d7cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe7c]
0065d622  8b9150020000          MOV EDX,dword ptr [ECX + 0x250]
0065d628  8995fcfcffff          MOV dword ptr [EBP + 0xfffffcfc],EDX
0065d62e  8b85fcfcffff          MOV EAX,dword ptr [EBP + 0xfffffcfc]
0065d634  83e001                AND EAX,0x1
0065d637  0f840c010000          JZ 0x0065d749
0065d63d  8b8d7cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe7c]
0065d643  c7814c02000001000000  MOV dword ptr [ECX + 0x24c],0x1
0065d64d  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d650  e86b7deeff            CALL 0x005453c0
0065d655  898578feffff          MOV dword ptr [EBP + 0xfffffe78],EAX
0065d65b  83bd78feffff00        CMP dword ptr [EBP + 0xfffffe78],0x0
0065d662  0f84e1000000          JZ 0x0065d749
0065d668  6a34                  PUSH 0x34
0065d66a  e8b4c02b00            CALL 0x00919723
0065d66f  83c404                ADD ESP,0x4
0065d672  8985a4fdffff          MOV dword ptr [EBP + 0xfffffda4],EAX
0065d678  c745fc04000000        MOV dword ptr [EBP + -0x4],0x4
0065d67f  83bda4fdffff00        CMP dword ptr [EBP + 0xfffffda4],0x0
0065d686  7413                  JZ 0x0065d69b
0065d688  8b8da4fdffff          MOV ECX,dword ptr [EBP + 0xfffffda4]
0065d68e  e8cd43f8ff            CALL 0x005e1a60
0065d693  898514fcffff          MOV dword ptr [EBP + 0xfffffc14],EAX
0065d699  eb0a                  JMP 0x0065d6a5
0065d69b  c78514fcffff00000000  MOV dword ptr [EBP + 0xfffffc14],0x0
0065d6a5  8b9514fcffff          MOV EDX,dword ptr [EBP + 0xfffffc14]
0065d6ab  8995a8fdffff          MOV dword ptr [EBP + 0xfffffda8],EDX
0065d6b1  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0065d6b8  8b85a8fdffff          MOV EAX,dword ptr [EBP + 0xfffffda8]
0065d6be  898574feffff          MOV dword ptr [EBP + 0xfffffe74],EAX
0065d6c4  b90b000000            MOV ECX,0xb
0065d6c9  8b9574feffff          MOV EDX,dword ptr [EBP + 0xfffffe74]
0065d6cf  66890a                MOV word ptr [EDX],CX
0065d6d2  68a10f0000            PUSH 0xfa1
0065d6d7  6a00                  PUSH 0x0
0065d6d9  8b8d74feffff          MOV ECX,dword ptr [EBP + 0xfffffe74]
0065d6df  e8ec44f8ff            CALL 0x005e1bd0
0065d6e4  8b8578feffff          MOV EAX,dword ptr [EBP + 0xfffffe78]
0065d6ea  8b8824010000          MOV ECX,dword ptr [EAX + 0x124]
0065d6f0  898df8fcffff          MOV dword ptr [EBP + 0xfffffcf8],ECX
0065d6f6  8b9578feffff          MOV EDX,dword ptr [EBP + 0xfffffe78]
0065d6fc  8b8224010000          MOV EAX,dword ptr [EDX + 0x124]
0065d702  8985f4fcffff          MOV dword ptr [EBP + 0xfffffcf4],EAX
0065d708  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065d70e  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0065d711  8995f0fcffff          MOV dword ptr [EBP + 0xfffffcf0],EDX
0065d717  8b8574feffff          MOV EAX,dword ptr [EBP + 0xfffffe74]
0065d71d  50                    PUSH EAX
0065d71e  8b0d1c249900          MOV ECX,dword ptr [0x0099241c]
0065d724  51                    PUSH ECX
0065d725  8b95f8fcffff          MOV EDX,dword ptr [EBP + 0xfffffcf8]
0065d72b  52                    PUSH EDX
0065d72c  8b85f4fcffff          MOV EAX,dword ptr [EBP + 0xfffffcf4]
0065d732  50                    PUSH EAX
0065d733  6a00                  PUSH 0x0
0065d735  6a00                  PUSH 0x0
0065d737  8b8df0fcffff          MOV ECX,dword ptr [EBP + 0xfffffcf0]
0065d73d  e83eebebff            CALL 0x0051c280
0065d742  8bc8                  MOV ECX,EAX
0065d744  e8270aecff            CALL 0x0051e170
0065d749  e93a0e0000            JMP 0x0065e588
0065d74e  c78550feffff00000000  MOV dword ptr [EBP + 0xfffffe50],0x0
0065d758  d9ee                  FLDZ
0065d75a  d99d68feffff          FSTP float ptr [EBP + 0xfffffe68]
0065d760  d9ee                  FLDZ
0065d762  d99d6cfeffff          FSTP float ptr [EBP + 0xfffffe6c]
0065d768  d9ee                  FLDZ
0065d76a  d99d70feffff          FSTP float ptr [EBP + 0xfffffe70]
0065d770  c7855cfeffff0000007f  MOV dword ptr [EBP + 0xfffffe5c],0x7f000000
0065d77a  6a20                  PUSH 0x20
0065d77c  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d782  e8c961f8ff            CALL 0x005e3950
0065d787  898560feffff          MOV dword ptr [EBP + 0xfffffe60],EAX
0065d78d  6a08                  PUSH 0x8
0065d78f  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d795  e8865ff8ff            CALL 0x005e3720
0065d79a  888567feffff          MOV byte ptr [EBP + 0xfffffe67],AL
0065d7a0  6a08                  PUSH 0x8
0065d7a2  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d7a8  e8735ff8ff            CALL 0x005e3720
0065d7ad  88855bfeffff          MOV byte ptr [EBP + 0xfffffe5b],AL
0065d7b3  0fb68d5bfeffff        MOVZX ECX,byte ptr [EBP + 0xfffffe5b]
0065d7ba  83e101                AND ECX,0x1
0065d7bd  7411                  JZ 0x0065d7d0
0065d7bf  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d7c5  e82657fdff            CALL 0x00632ef0
0065d7ca  89855cfeffff          MOV dword ptr [EBP + 0xfffffe5c],EAX
0065d7d0  0fb6955bfeffff        MOVZX EDX,byte ptr [EBP + 0xfffffe5b]
0065d7d7  83e202                AND EDX,0x2
0065d7da  7455                  JZ 0x0065d831
0065d7dc  6a20                  PUSH 0x20
0065d7de  51                    PUSH ECX
0065d7df  d9e8                  FLD1
0065d7e1  d91c24                FSTP float ptr [ESP]
0065d7e4  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d7ea  e87163f8ff            CALL 0x005e3b60
0065d7ef  d99d68feffff          FSTP float ptr [EBP + 0xfffffe68]
0065d7f5  6a20                  PUSH 0x20
0065d7f7  51                    PUSH ECX
0065d7f8  d9e8                  FLD1
0065d7fa  d91c24                FSTP float ptr [ESP]
0065d7fd  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d803  e85863f8ff            CALL 0x005e3b60
0065d808  d99d6cfeffff          FSTP float ptr [EBP + 0xfffffe6c]
0065d80e  6a20                  PUSH 0x20
0065d810  51                    PUSH ECX
0065d811  d9e8                  FLD1
0065d813  d91c24                FSTP float ptr [ESP]
0065d816  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d81c  e83f63f8ff            CALL 0x005e3b60
0065d821  d99d70feffff          FSTP float ptr [EBP + 0xfffffe70]
0065d827  c78550feffff01000000  MOV dword ptr [EBP + 0xfffffe50],0x1
0065d831  6a01                  PUSH 0x1
0065d833  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d839  e8a265f8ff            CALL 0x005e3de0
0065d83e  85c0                  TEST EAX,EAX
0065d840  7511                  JNZ 0x0065d853
0065d842  6a01                  PUSH 0x1
0065d844  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d84a  e8f165f8ff            CALL 0x005e3e40
0065d84f  85c0                  TEST EAX,EAX
0065d851  7407                  JZ 0x0065d85a
0065d853  33c0                  XOR EAX,EAX
0065d855  e9300d0000            JMP 0x0065e58a
0065d85a  837df000              CMP dword ptr [EBP + -0x10],0x0
0065d85e  7406                  JZ 0x0065d866
0065d860  837dec00              CMP dword ptr [EBP + -0x14],0x0
0065d864  7507                  JNZ 0x0065d86d
0065d866  33c0                  XOR EAX,EAX
0065d868  e91d0d0000            JMP 0x0065e58a
0065d86d  a1d0b4a100            MOV EAX,[0x00a1b4d0]
0065d872  8985ecfcffff          MOV dword ptr [EBP + 0xfffffcec],EAX
0065d878  8b8d60feffff          MOV ECX,dword ptr [EBP + 0xfffffe60]
0065d87e  51                    PUSH ECX
0065d87f  8b95ecfcffff          MOV EDX,dword ptr [EBP + 0xfffffcec]
0065d885  8b8a04010000          MOV ECX,dword ptr [EDX + 0x104]
0065d88b  e8c03b0600            CALL 0x006c1450
0065d890  898554feffff          MOV dword ptr [EBP + 0xfffffe54],EAX
0065d896  6a01                  PUSH 0x1
0065d898  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d89b  e8f063f0ff            CALL 0x00563c90
0065d8a0  8b855cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe5c]
0065d8a6  3b0584449900          CMP EAX,dword ptr [0x00994484]
0065d8ac  7432                  JZ 0x0065d8e0
0065d8ae  6a00                  PUSH 0x0
0065d8b0  0fb68d67feffff        MOVZX ECX,byte ptr [EBP + 0xfffffe67]
0065d8b7  51                    PUSH ECX
0065d8b8  6aff                  PUSH -0x1
0065d8ba  8b1584449900          MOV EDX,dword ptr [0x00994484]
0065d8c0  52                    PUSH EDX
0065d8c1  68fe000000            PUSH 0xfe
0065d8c6  8b855cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe5c]
0065d8cc  50                    PUSH EAX
0065d8cd  8b8d60feffff          MOV ECX,dword ptr [EBP + 0xfffffe60]
0065d8d3  51                    PUSH ECX
0065d8d4  6a00                  PUSH 0x0
0065d8d6  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d8d9  e8d2fff1ff            CALL 0x0057d8b0
0065d8de  eb2f                  JMP 0x0065d90f
0065d8e0  6a00                  PUSH 0x0
0065d8e2  0fb69567feffff        MOVZX EDX,byte ptr [EBP + 0xfffffe67]
0065d8e9  52                    PUSH EDX
0065d8ea  6aff                  PUSH -0x1
0065d8ec  a184449900            MOV EAX,[0x00994484]
0065d8f1  50                    PUSH EAX
0065d8f2  68fe000000            PUSH 0xfe
0065d8f7  8d8d68feffff          LEA ECX,[EBP + 0xfffffe68]
0065d8fd  51                    PUSH ECX
0065d8fe  8b9560feffff          MOV EDX,dword ptr [EBP + 0xfffffe60]
0065d904  52                    PUSH EDX
0065d905  6a00                  PUSH 0x0
0065d907  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d90a  e88101f2ff            CALL 0x0057da90
0065d90f  e9740c0000            JMP 0x0065e588
0065d914  6a10                  PUSH 0x10
0065d916  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d91c  e80f5ff8ff            CALL 0x005e3830
0065d921  66898548feffff        MOV word ptr [EBP + 0xfffffe48],AX
0065d928  6a10                  PUSH 0x10
0065d92a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d930  e8fb5ef8ff            CALL 0x005e3830
0065d935  6689854cfeffff        MOV word ptr [EBP + 0xfffffe4c],AX
0065d93c  6a01                  PUSH 0x1
0065d93e  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d944  e89764f8ff            CALL 0x005e3de0
0065d949  85c0                  TEST EAX,EAX
0065d94b  7407                  JZ 0x0065d954
0065d94d  33c0                  XOR EAX,EAX
0065d94f  e9360c0000            JMP 0x0065e58a
0065d954  837df000              CMP dword ptr [EBP + -0x10],0x0
0065d958  7418                  JZ 0x0065d972
0065d95a  0fb7854cfeffff        MOVZX EAX,word ptr [EBP + 0xfffffe4c]
0065d961  50                    PUSH EAX
0065d962  0fb78d48feffff        MOVZX ECX,word ptr [EBP + 0xfffffe48]
0065d969  51                    PUSH ECX
0065d96a  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065d96d  e82e93f1ff            CALL 0x00576ca0
0065d972  e9110c0000            JMP 0x0065e588
0065d977  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d97d  e86e55fdff            CALL 0x00632ef0
0065d982  898544feffff          MOV dword ptr [EBP + 0xfffffe44],EAX
0065d988  6a01                  PUSH 0x1
0065d98a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065d990  e84b64f8ff            CALL 0x005e3de0
0065d995  85c0                  TEST EAX,EAX
0065d997  7407                  JZ 0x0065d9a0
0065d999  33c0                  XOR EAX,EAX
0065d99b  e9ea0b0000            JMP 0x0065e58a
0065d9a0  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0065d9a6  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0065d9a9  8985e8fcffff          MOV dword ptr [EBP + 0xfffffce8],EAX
0065d9af  8b8d44feffff          MOV ECX,dword ptr [EBP + 0xfffffe44]
0065d9b5  51                    PUSH ECX
0065d9b6  8b8de8fcffff          MOV ECX,dword ptr [EBP + 0xfffffce8]
0065d9bc  e8dfe6ebff            CALL 0x0051c0a0
0065d9c1  898540feffff          MOV dword ptr [EBP + 0xfffffe40],EAX
0065d9c7  83bd40feffff00        CMP dword ptr [EBP + 0xfffffe40],0x0
0065d9ce  7507                  JNZ 0x0065d9d7
0065d9d0  33c0                  XOR EAX,EAX
0065d9d2  e9b30b0000            JMP 0x0065e58a
0065d9d7  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
0065d9dd  0fb64208              MOVZX EAX,byte ptr [EDX + 0x8]
0065d9e1  898510fcffff          MOV dword ptr [EBP + 0xfffffc10],EAX
0065d9e7  8b8d10fcffff          MOV ECX,dword ptr [EBP + 0xfffffc10]
0065d9ed  83e904                SUB ECX,0x4
0065d9f0  898d10fcffff          MOV dword ptr [EBP + 0xfffffc10],ECX
0065d9f6  83bd10fcffff05        CMP dword ptr [EBP + 0xfffffc10],0x5
0065d9fd  776b                  JA 0x0065da6a
0065d9ff  8b9510fcffff          MOV EDX,dword ptr [EBP + 0xfffffc10]
0065da05  ff249550e66500        JMP dword ptr [EDX*0x4 + 0x65e650]
0065da0c  8b8544feffff          MOV EAX,dword ptr [EBP + 0xfffffe44]
0065da12  50                    PUSH EAX
0065da13  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065da16  51                    PUSH ECX
0065da17  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065da1d  e8beecfeff            CALL 0x0064c6e0
0065da22  eb46                  JMP 0x0065da6a
0065da24  8b9544feffff          MOV EDX,dword ptr [EBP + 0xfffffe44]
0065da2a  52                    PUSH EDX
0065da2b  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0065da2e  50                    PUSH EAX
0065da2f  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065da35  e846f0feff            CALL 0x0064ca80
0065da3a  eb2e                  JMP 0x0065da6a
0065da3c  8b8d44feffff          MOV ECX,dword ptr [EBP + 0xfffffe44]
0065da42  51                    PUSH ECX
0065da43  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0065da46  52                    PUSH EDX
0065da47  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065da4d  e83ef1feff            CALL 0x0064cb90
0065da52  eb16                  JMP 0x0065da6a
0065da54  8b8544feffff          MOV EAX,dword ptr [EBP + 0xfffffe44]
0065da5a  50                    PUSH EAX
0065da5b  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065da5e  51                    PUSH ECX
0065da5f  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065da65  e816f3feff            CALL 0x0064cd80
0065da6a  e9190b0000            JMP 0x0065e588
0065da6f  c78538feffff00000000  MOV dword ptr [EBP + 0xfffffe38],0x0
0065da79  c7853cfeffff00000000  MOV dword ptr [EBP + 0xfffffe3c],0x0
0065da83  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065da89  e86254fdff            CALL 0x00632ef0
0065da8e  898530feffff          MOV dword ptr [EBP + 0xfffffe30],EAX
0065da94  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065da9a  e85154fdff            CALL 0x00632ef0
0065da9f  898528feffff          MOV dword ptr [EBP + 0xfffffe28],EAX
0065daa5  6a01                  PUSH 0x1
0065daa7  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065daad  e82e63f8ff            CALL 0x005e3de0
0065dab2  85c0                  TEST EAX,EAX
0065dab4  7407                  JZ 0x0065dabd
0065dab6  33c0                  XOR EAX,EAX
0065dab8  e9cd0a0000            JMP 0x0065e58a
0065dabd  837df000              CMP dword ptr [EBP + -0x10],0x0
0065dac1  7418                  JZ 0x0065dadb
0065dac3  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0065dac6  8b82ec000000          MOV EAX,dword ptr [EDX + 0xec]
0065dacc  8985e4fcffff          MOV dword ptr [EBP + 0xfffffce4],EAX
0065dad2  83bde4fcffff00        CMP dword ptr [EBP + 0xfffffce4],0x0
0065dad9  7507                  JNZ 0x0065dae2
0065dadb  33c0                  XOR EAX,EAX
0065dadd  e9a80a0000            JMP 0x0065e58a
0065dae2  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065dae8  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0065daeb  8995e0fcffff          MOV dword ptr [EBP + 0xfffffce0],EDX
0065daf1  8b8530feffff          MOV EAX,dword ptr [EBP + 0xfffffe30]
0065daf7  50                    PUSH EAX
0065daf8  8b8de0fcffff          MOV ECX,dword ptr [EBP + 0xfffffce0]
0065dafe  e8dde5ebff            CALL 0x0051c0e0
0065db03  898534feffff          MOV dword ptr [EBP + 0xfffffe34],EAX
0065db09  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065db0f  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0065db12  8995dcfcffff          MOV dword ptr [EBP + 0xfffffcdc],EDX
0065db18  8b8528feffff          MOV EAX,dword ptr [EBP + 0xfffffe28]
0065db1e  50                    PUSH EAX
0065db1f  8b8ddcfcffff          MOV ECX,dword ptr [EBP + 0xfffffcdc]
0065db25  e8b6e5ebff            CALL 0x0051c0e0
0065db2a  89852cfeffff          MOV dword ptr [EBP + 0xfffffe2c],EAX
0065db30  83bd34feffff00        CMP dword ptr [EBP + 0xfffffe34],0x0
0065db37  7409                  JZ 0x0065db42
0065db39  83bd2cfeffff00        CMP dword ptr [EBP + 0xfffffe2c],0x0
0065db40  7507                  JNZ 0x0065db49
0065db42  33c0                  XOR EAX,EAX
0065db44  e9410a0000            JMP 0x0065e58a
0065db49  8b8d34feffff          MOV ECX,dword ptr [EBP + 0xfffffe34]
0065db4f  8b91a8020000          MOV EDX,dword ptr [ECX + 0x2a8]
0065db55  8995d8fcffff          MOV dword ptr [EBP + 0xfffffcd8],EDX
0065db5b  8b852cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe2c]
0065db61  8b88a8020000          MOV ECX,dword ptr [EAX + 0x2a8]
0065db67  898dd4fcffff          MOV dword ptr [EBP + 0xfffffcd4],ECX
0065db6d  8b95d8fcffff          MOV EDX,dword ptr [EBP + 0xfffffcd8]
0065db73  3b95d4fcffff          CMP EDX,dword ptr [EBP + 0xfffffcd4]
0065db79  7536                  JNZ 0x0065dbb1
0065db7b  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0065db7e  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0065db81  898dd0fcffff          MOV dword ptr [EBP + 0xfffffcd0],ECX
0065db87  8b952cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe2c]
0065db8d  8b82a8020000          MOV EAX,dword ptr [EDX + 0x2a8]
0065db93  8985ccfcffff          MOV dword ptr [EBP + 0xfffffccc],EAX
0065db99  8b8dd0fcffff          MOV ECX,dword ptr [EBP + 0xfffffcd0]
0065db9f  3b8dccfcffff          CMP ECX,dword ptr [EBP + 0xfffffccc]
0065dba5  750a                  JNZ 0x0065dbb1
0065dba7  c7853cfeffff01000000  MOV dword ptr [EBP + 0xfffffe3c],0x1
0065dbb1  8b952cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe2c]
0065dbb7  8b82a8020000          MOV EAX,dword ptr [EDX + 0x2a8]
0065dbbd  8985c8fcffff          MOV dword ptr [EBP + 0xfffffcc8],EAX
0065dbc3  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065dbc6  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0065dbc9  8995c4fcffff          MOV dword ptr [EBP + 0xfffffcc4],EDX
0065dbcf  8b85c8fcffff          MOV EAX,dword ptr [EBP + 0xfffffcc8]
0065dbd5  3b85c4fcffff          CMP EAX,dword ptr [EBP + 0xfffffcc4]
0065dbdb  7479                  JZ 0x0065dc56
0065dbdd  8b8d2cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe2c]
0065dbe3  8b91a8020000          MOV EDX,dword ptr [ECX + 0x2a8]
0065dbe9  8995c0fcffff          MOV dword ptr [EBP + 0xfffffcc0],EDX
0065dbef  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065dbf4  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065dbf7  898dbcfcffff          MOV dword ptr [EBP + 0xfffffcbc],ECX
0065dbfd  8b95c0fcffff          MOV EDX,dword ptr [EBP + 0xfffffcc0]
0065dc03  52                    PUSH EDX
0065dc04  8b8dbcfcffff          MOV ECX,dword ptr [EBP + 0xfffffcbc]
0065dc0a  e8d1e4ebff            CALL 0x0051c0e0
0065dc0f  898538feffff          MOV dword ptr [EBP + 0xfffffe38],EAX
0065dc15  83bd38feffff00        CMP dword ptr [EBP + 0xfffffe38],0x0
0065dc1c  7436                  JZ 0x0065dc54
0065dc1e  8b8538feffff          MOV EAX,dword ptr [EBP + 0xfffffe38]
0065dc24  8b88a8020000          MOV ECX,dword ptr [EAX + 0x2a8]
0065dc2a  898db8fcffff          MOV dword ptr [EBP + 0xfffffcb8],ECX
0065dc30  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0065dc33  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0065dc36  8985b4fcffff          MOV dword ptr [EBP + 0xfffffcb4],EAX
0065dc3c  8b8db8fcffff          MOV ECX,dword ptr [EBP + 0xfffffcb8]
0065dc42  3b8db4fcffff          CMP ECX,dword ptr [EBP + 0xfffffcb4]
0065dc48  750a                  JNZ 0x0065dc54
0065dc4a  c7853cfeffff01000000  MOV dword ptr [EBP + 0xfffffe3c],0x1
0065dc54  eb0a                  JMP 0x0065dc60
0065dc56  c7853cfeffff01000000  MOV dword ptr [EBP + 0xfffffe3c],0x1
0065dc60  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0065dc63  8b4258                MOV EAX,dword ptr [EDX + 0x58]
0065dc66  8985b0fcffff          MOV dword ptr [EBP + 0xfffffcb0],EAX
0065dc6c  8b8db0fcffff          MOV ECX,dword ptr [EBP + 0xfffffcb0]
0065dc72  8b11                  MOV EDX,dword ptr [ECX]
0065dc74  8995acfcffff          MOV dword ptr [EBP + 0xfffffcac],EDX
0065dc7a  8b852cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe2c]
0065dc80  8b88a8020000          MOV ECX,dword ptr [EAX + 0x2a8]
0065dc86  898da8fcffff          MOV dword ptr [EBP + 0xfffffca8],ECX
0065dc8c  8b95acfcffff          MOV EDX,dword ptr [EBP + 0xfffffcac]
0065dc92  3b95a8fcffff          CMP EDX,dword ptr [EBP + 0xfffffca8]
0065dc98  0f855a010000          JNZ 0x0065ddf8
0065dc9e  8b8534feffff          MOV EAX,dword ptr [EBP + 0xfffffe34]
0065dca4  8b88a8020000          MOV ECX,dword ptr [EAX + 0x2a8]
0065dcaa  898da4fcffff          MOV dword ptr [EBP + 0xfffffca4],ECX
0065dcb0  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0065dcb3  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0065dcb6  8985a0fcffff          MOV dword ptr [EBP + 0xfffffca0],EAX
0065dcbc  8b8da4fcffff          MOV ECX,dword ptr [EBP + 0xfffffca4]
0065dcc2  3b8da0fcffff          CMP ECX,dword ptr [EBP + 0xfffffca0]
0065dcc8  0f852a010000          JNZ 0x0065ddf8
0065dcce  83bd38feffff00        CMP dword ptr [EBP + 0xfffffe38],0x0
0065dcd5  743e                  JZ 0x0065dd15
0065dcd7  8b9538feffff          MOV EDX,dword ptr [EBP + 0xfffffe38]
0065dcdd  8b4214                MOV EAX,dword ptr [EDX + 0x14]
0065dce0  89859cfcffff          MOV dword ptr [EBP + 0xfffffc9c],EAX
0065dce6  8b8d2cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe2c]
0065dcec  8b91a8020000          MOV EDX,dword ptr [ECX + 0x2a8]
0065dcf2  899598fcffff          MOV dword ptr [EBP + 0xfffffc98],EDX
0065dcf8  8b859cfcffff          MOV EAX,dword ptr [EBP + 0xfffffc9c]
0065dcfe  3b8598fcffff          CMP EAX,dword ptr [EBP + 0xfffffc98]
0065dd04  740f                  JZ 0x0065dd15
0065dd06  c7853cfeffff01000000  MOV dword ptr [EBP + 0xfffffe3c],0x1
0065dd10  e9e3000000            JMP 0x0065ddf8
0065dd15  83bd38feffff00        CMP dword ptr [EBP + 0xfffffe38],0x0
0065dd1c  0f85d6000000          JNZ 0x0065ddf8
0065dd22  8b8d2cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe2c]
0065dd28  8b91a8020000          MOV EDX,dword ptr [ECX + 0x2a8]
0065dd2e  899594fcffff          MOV dword ptr [EBP + 0xfffffc94],EDX
0065dd34  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065dd39  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065dd3c  898d90fcffff          MOV dword ptr [EBP + 0xfffffc90],ECX
0065dd42  8b9594fcffff          MOV EDX,dword ptr [EBP + 0xfffffc94]
0065dd48  52                    PUSH EDX
0065dd49  8b8d90fcffff          MOV ECX,dword ptr [EBP + 0xfffffc90]
0065dd4f  e80ce4ebff            CALL 0x0051c160
0065dd54  898524feffff          MOV dword ptr [EBP + 0xfffffe24],EAX
0065dd5a  83bd24feffff00        CMP dword ptr [EBP + 0xfffffe24],0x0
0065dd61  0f8491000000          JZ 0x0065ddf8
0065dd67  8b852cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe2c]
0065dd6d  50                    PUSH EAX
0065dd6e  8b8d24feffff          MOV ECX,dword ptr [EBP + 0xfffffe24]
0065dd74  e8e770f9ff            CALL 0x005f4e60
0065dd79  6a01                  PUSH 0x1
0065dd7b  6a00                  PUSH 0x0
0065dd7d  6a00                  PUSH 0x0
0065dd7f  8d8d2cfeffff          LEA ECX,[EBP + 0xfffffe2c]
0065dd85  51                    PUSH ECX
0065dd86  6a01                  PUSH 0x1
0065dd88  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065dd8b  e830c5f2ff            CALL 0x0058a2c0
0065dd90  8bc8                  MOV ECX,EAX
0065dd92  e8f972faff            CALL 0x00605090
0065dd97  85c0                  TEST EAX,EAX
0065dd99  752c                  JNZ 0x0065ddc7
0065dd9b  8b9524feffff          MOV EDX,dword ptr [EBP + 0xfffffe24]
0065dda1  8b82c4030000          MOV EAX,dword ptr [EDX + 0x3c4]
0065dda7  89858cfcffff          MOV dword ptr [EBP + 0xfffffc8c],EAX
0065ddad  6a01                  PUSH 0x1
0065ddaf  6a00                  PUSH 0x0
0065ddb1  6a00                  PUSH 0x0
0065ddb3  8d8d2cfeffff          LEA ECX,[EBP + 0xfffffe2c]
0065ddb9  51                    PUSH ECX
0065ddba  8b8d8cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc8c]
0065ddc0  e8cb72faff            CALL 0x00605090
0065ddc5  eb31                  JMP 0x0065ddf8
0065ddc7  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0065ddca  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0065ddcd  898588fcffff          MOV dword ptr [EBP + 0xfffffc88],EAX
0065ddd3  680000007f            PUSH 0x7f000000
0065ddd8  6a00                  PUSH 0x0
0065ddda  6a01                  PUSH 0x1
0065dddc  8b8d88fcffff          MOV ECX,dword ptr [EBP + 0xfffffc88]
0065dde2  51                    PUSH ECX
0065dde3  8b8d2cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe2c]
0065dde9  e89225faff            CALL 0x00600380
0065ddee  c7853cfeffff01000000  MOV dword ptr [EBP + 0xfffffe3c],0x1
0065ddf8  83bd3cfeffff01        CMP dword ptr [EBP + 0xfffffe3c],0x1
0065ddff  753d                  JNZ 0x0065de3e
0065de01  8b952cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe2c]
0065de07  52                    PUSH EDX
0065de08  8b8534feffff          MOV EAX,dword ptr [EBP + 0xfffffe34]
0065de0e  50                    PUSH EAX
0065de0f  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065de12  e869e2f0ff            CALL 0x0056c080
0065de17  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065de1a  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0065de1d  899584fcffff          MOV dword ptr [EBP + 0xfffffc84],EDX
0065de23  680000007f            PUSH 0x7f000000
0065de28  6a00                  PUSH 0x0
0065de2a  6a01                  PUSH 0x1
0065de2c  8b8584fcffff          MOV EAX,dword ptr [EBP + 0xfffffc84]
0065de32  50                    PUSH EAX
0065de33  8b8d34feffff          MOV ECX,dword ptr [EBP + 0xfffffe34]
0065de39  e84225faff            CALL 0x00600380
0065de3e  e945070000            JMP 0x0065e588
0065de43  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065de49  e8a250fdff            CALL 0x00632ef0
0065de4e  898518feffff          MOV dword ptr [EBP + 0xfffffe18],EAX
0065de54  6a10                  PUSH 0x10
0065de56  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065de5c  e8cf59f8ff            CALL 0x005e3830
0065de61  6689851cfeffff        MOV word ptr [EBP + 0xfffffe1c],AX
0065de68  6a01                  PUSH 0x1
0065de6a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065de70  e86b5ff8ff            CALL 0x005e3de0
0065de75  85c0                  TEST EAX,EAX
0065de77  7407                  JZ 0x0065de80
0065de79  33c0                  XOR EAX,EAX
0065de7b  e90a070000            JMP 0x0065e58a
0065de80  837df000              CMP dword ptr [EBP + -0x10],0x0
0065de84  7418                  JZ 0x0065de9e
0065de86  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065de89  8b91ec000000          MOV EDX,dword ptr [ECX + 0xec]
0065de8f  899580fcffff          MOV dword ptr [EBP + 0xfffffc80],EDX
0065de95  83bd80fcffff00        CMP dword ptr [EBP + 0xfffffc80],0x0
0065de9c  7507                  JNZ 0x0065dea5
0065de9e  33c0                  XOR EAX,EAX
0065dea0  e9e5060000            JMP 0x0065e58a
0065dea5  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065deaa  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065dead  898d7cfcffff          MOV dword ptr [EBP + 0xfffffc7c],ECX
0065deb3  8b9518feffff          MOV EDX,dword ptr [EBP + 0xfffffe18]
0065deb9  52                    PUSH EDX
0065deba  8b8d7cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc7c]
0065dec0  e81be2ebff            CALL 0x0051c0e0
0065dec5  898520feffff          MOV dword ptr [EBP + 0xfffffe20],EAX
0065decb  83bd20feffff00        CMP dword ptr [EBP + 0xfffffe20],0x0
0065ded2  7507                  JNZ 0x0065dedb
0065ded4  33c0                  XOR EAX,EAX
0065ded6  e9af060000            JMP 0x0065e58a
0065dedb  0fb7851cfeffff        MOVZX EAX,word ptr [EBP + 0xfffffe1c]
0065dee2  50                    PUSH EAX
0065dee3  8b8d20feffff          MOV ECX,dword ptr [EBP + 0xfffffe20]
0065dee9  51                    PUSH ECX
0065deea  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065deed  e8eee2f0ff            CALL 0x0056c1e0
0065def2  e991060000            JMP 0x0065e588
0065def7  837df000              CMP dword ptr [EBP + -0x10],0x0
0065defb  741f                  JZ 0x0065df1c
0065defd  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0065df00  8b82f0100000          MOV EAX,dword ptr [EDX + 0x10f0]
0065df06  898578fcffff          MOV dword ptr [EBP + 0xfffffc78],EAX
0065df0c  83bd78fcffff00        CMP dword ptr [EBP + 0xfffffc78],0x0
0065df13  7407                  JZ 0x0065df1c
0065df15  33c0                  XOR EAX,EAX
0065df17  e96e060000            JMP 0x0065e58a
0065df1c  837df000              CMP dword ptr [EBP + -0x10],0x0
0065df20  7507                  JNZ 0x0065df29
0065df22  33c0                  XOR EAX,EAX
0065df24  e961060000            JMP 0x0065e58a
0065df29  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065df2c  e88f74eeff            CALL 0x005453c0
0065df31  85c0                  TEST EAX,EAX
0065df33  7507                  JNZ 0x0065df3c
0065df35  33c0                  XOR EAX,EAX
0065df37  e94e060000            JMP 0x0065e58a
0065df3c  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065df42  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0065df45  899574fcffff          MOV dword ptr [EBP + 0xfffffc74],EDX
0065df4b  8b8d74fcffff          MOV ECX,dword ptr [EBP + 0xfffffc74]
0065df51  e8eaddebff            CALL 0x0051bd40
0065df56  83b8c400000000        CMP dword ptr [EAX + 0xc4],0x0
0065df5d  741b                  JZ 0x0065df7a
0065df5f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065df64  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065df67  898d70fcffff          MOV dword ptr [EBP + 0xfffffc70],ECX
0065df6d  6a02                  PUSH 0x2
0065df6f  8b8d70fcffff          MOV ECX,dword ptr [EBP + 0xfffffc70]
0065df75  e836e7ebff            CALL 0x0051c6b0
0065df7a  e909060000            JMP 0x0065e588
0065df7f  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065df85  e81657f8ff            CALL 0x005e36a0
0065df8a  898514feffff          MOV dword ptr [EBP + 0xfffffe14],EAX
0065df90  6a01                  PUSH 0x1
0065df92  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065df98  e8435ef8ff            CALL 0x005e3de0
0065df9d  85c0                  TEST EAX,EAX
0065df9f  7511                  JNZ 0x0065dfb2
0065dfa1  6a01                  PUSH 0x1
0065dfa3  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065dfa9  e8925ef8ff            CALL 0x005e3e40
0065dfae  85c0                  TEST EAX,EAX
0065dfb0  7407                  JZ 0x0065dfb9
0065dfb2  33c0                  XOR EAX,EAX
0065dfb4  e9d1050000            JMP 0x0065e58a
0065dfb9  837df000              CMP dword ptr [EBP + -0x10],0x0
0065dfbd  741f                  JZ 0x0065dfde
0065dfbf  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0065dfc2  8b82f0100000          MOV EAX,dword ptr [EDX + 0x10f0]
0065dfc8  89856cfcffff          MOV dword ptr [EBP + 0xfffffc6c],EAX
0065dfce  83bd6cfcffff00        CMP dword ptr [EBP + 0xfffffc6c],0x0
0065dfd5  7407                  JZ 0x0065dfde
0065dfd7  33c0                  XOR EAX,EAX
0065dfd9  e9ac050000            JMP 0x0065e58a
0065dfde  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065dfe4  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0065dfe7  899568fcffff          MOV dword ptr [EBP + 0xfffffc68],EDX
0065dfed  8b8514feffff          MOV EAX,dword ptr [EBP + 0xfffffe14]
0065dff3  50                    PUSH EAX
0065dff4  6a02                  PUSH 0x2
0065dff6  8b8d68fcffff          MOV ECX,dword ptr [EBP + 0xfffffc68]
0065dffc  e85fe7ebff            CALL 0x0051c760
0065e001  e982050000            JMP 0x0065e588
0065e006  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065e00c  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0065e00f  899564fcffff          MOV dword ptr [EBP + 0xfffffc64],EDX
0065e015  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0065e018  50                    PUSH EAX
0065e019  8b8d64fcffff          MOV ECX,dword ptr [EBP + 0xfffffc64]
0065e01f  e8bce2ebff            CALL 0x0051c2e0
0065e024  8bc8                  MOV ECX,EAX
0065e026  e83594fdff            CALL 0x00637460
0065e02b  e958050000            JMP 0x0065e588
0065e030  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e036  e86556f8ff            CALL 0x005e36a0
0065e03b  898510feffff          MOV dword ptr [EBP + 0xfffffe10],EAX
0065e041  6a01                  PUSH 0x1
0065e043  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e049  e8925df8ff            CALL 0x005e3de0
0065e04e  85c0                  TEST EAX,EAX
0065e050  7407                  JZ 0x0065e059
0065e052  33c0                  XOR EAX,EAX
0065e054  e931050000            JMP 0x0065e58a
0065e059  c78510feffff01000000  MOV dword ptr [EBP + 0xfffffe10],0x1
0065e063  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065e066  8b9510feffff          MOV EDX,dword ptr [EBP + 0xfffffe10]
0065e06c  895160                MOV dword ptr [ECX + 0x60],EDX
0065e06f  e914050000            JMP 0x0065e588
0065e074  d9ee                  FLDZ
0065e076  d99d00feffff          FSTP float ptr [EBP + 0xfffffe00]
0065e07c  d9ee                  FLDZ
0065e07e  d99d04feffff          FSTP float ptr [EBP + 0xfffffe04]
0065e084  d9ee                  FLDZ
0065e086  d99d08feffff          FSTP float ptr [EBP + 0xfffffe08]
0065e08c  6a20                  PUSH 0x20
0065e08e  51                    PUSH ECX
0065e08f  d9e8                  FLD1
0065e091  d91c24                FSTP float ptr [ESP]
0065e094  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e09a  e8c15af8ff            CALL 0x005e3b60
0065e09f  d99d00feffff          FSTP float ptr [EBP + 0xfffffe00]
0065e0a5  6a20                  PUSH 0x20
0065e0a7  51                    PUSH ECX
0065e0a8  d9e8                  FLD1
0065e0aa  d91c24                FSTP float ptr [ESP]
0065e0ad  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e0b3  e8a85af8ff            CALL 0x005e3b60
0065e0b8  d99d04feffff          FSTP float ptr [EBP + 0xfffffe04]
0065e0be  8d8d00feffff          LEA ECX,[EBP + 0xfffffe00]
0065e0c4  e87762ebff            CALL 0x00514340
0065e0c9  6a01                  PUSH 0x1
0065e0cb  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e0d1  e80a5df8ff            CALL 0x005e3de0
0065e0d6  85c0                  TEST EAX,EAX
0065e0d8  7407                  JZ 0x0065e0e1
0065e0da  33c0                  XOR EAX,EAX
0065e0dc  e9a9040000            JMP 0x0065e58a
0065e0e1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065e0e4  e83749fdff            CALL 0x00632a20
0065e0e9  8985fcfdffff          MOV dword ptr [EBP + 0xfffffdfc],EAX
0065e0ef  83bdfcfdffff00        CMP dword ptr [EBP + 0xfffffdfc],0x0
0065e0f6  7507                  JNZ 0x0065e0ff
0065e0f8  33c0                  XOR EAX,EAX
0065e0fa  e98b040000            JMP 0x0065e58a
0065e0ff  c7850cfeffff00000000  MOV dword ptr [EBP + 0xfffffe0c],0x0
0065e109  8b85fcfdffff          MOV EAX,dword ptr [EBP + 0xfffffdfc]
0065e10f  8b10                  MOV EDX,dword ptr [EAX]
0065e111  8b8dfcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdfc]
0065e117  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065e11a  ffd0                  CALL EAX
0065e11c  85c0                  TEST EAX,EAX
0065e11e  745c                  JZ 0x0065e17c
0065e120  8b8dfcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdfc]
0065e126  8b11                  MOV EDX,dword ptr [ECX]
0065e128  8b8dfcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdfc]
0065e12e  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065e131  ffd0                  CALL EAX
0065e133  898560fcffff          MOV dword ptr [EBP + 0xfffffc60],EAX
0065e139  8b8d60fcffff          MOV ECX,dword ptr [EBP + 0xfffffc60]
0065e13f  83b9ac11000000        CMP dword ptr [ECX + 0x11ac],0x0
0065e146  742a                  JZ 0x0065e172
0065e148  6a00                  PUSH 0x0
0065e14a  8b9560fcffff          MOV EDX,dword ptr [EBP + 0xfffffc60]
0065e150  8b02                  MOV EAX,dword ptr [EDX]
0065e152  8b8d60fcffff          MOV ECX,dword ptr [EBP + 0xfffffc60]
0065e158  8b909c000000          MOV EDX,dword ptr [EAX + 0x9c]
0065e15e  ffd2                  CALL EDX
0065e160  98                    CWDE
0065e161  33c9                  XOR ECX,ECX
0065e163  85c0                  TEST EAX,EAX
0065e165  0f9ec1                SETLE CL
0065e168  898d0cfeffff          MOV dword ptr [EBP + 0xfffffe0c],ECX
0065e16e  eb0c                  JMP 0x0065e17c
0065e172  c7850cfeffff00000000  MOV dword ptr [EBP + 0xfffffe0c],0x0
0065e17c  8b95fcfdffff          MOV EDX,dword ptr [EBP + 0xfffffdfc]
0065e182  8b82ec000000          MOV EAX,dword ptr [EDX + 0xec]
0065e188  89855cfcffff          MOV dword ptr [EBP + 0xfffffc5c],EAX
0065e18e  83bd5cfcffff00        CMP dword ptr [EBP + 0xfffffc5c],0x0
0065e195  742b                  JZ 0x0065e1c2
0065e197  33c9                  XOR ECX,ECX
0065e199  83f901                CMP ECX,0x1
0065e19c  7424                  JZ 0x0065e1c2
0065e19e  8b95fcfdffff          MOV EDX,dword ptr [EBP + 0xfffffdfc]
0065e1a4  8b02                  MOV EAX,dword ptr [EDX]
0065e1a6  8b8dfcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdfc]
0065e1ac  8b9094000000          MOV EDX,dword ptr [EAX + 0x94]
0065e1b2  ffd2                  CALL EDX
0065e1b4  83f801                CMP EAX,0x1
0065e1b7  7409                  JZ 0x0065e1c2
0065e1b9  83bd0cfeffff01        CMP dword ptr [EBP + 0xfffffe0c],0x1
0065e1c0  7507                  JNZ 0x0065e1c9
0065e1c2  33c0                  XOR EAX,EAX
0065e1c4  e9c1030000            JMP 0x0065e58a
0065e1c9  837df000              CMP dword ptr [EBP + -0x10],0x0
0065e1cd  741f                  JZ 0x0065e1ee
0065e1cf  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0065e1d2  8b88f0100000          MOV ECX,dword ptr [EAX + 0x10f0]
0065e1d8  898d58fcffff          MOV dword ptr [EBP + 0xfffffc58],ECX
0065e1de  83bd58fcffff00        CMP dword ptr [EBP + 0xfffffc58],0x0
0065e1e5  7407                  JZ 0x0065e1ee
0065e1e7  33c0                  XOR EAX,EAX
0065e1e9  e99c030000            JMP 0x0065e58a
0065e1ee  8d9500feffff          LEA EDX,[EBP + 0xfffffe00]
0065e1f4  52                    PUSH EDX
0065e1f5  8b8dfcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdfc]
0065e1fb  e8105deeff            CALL 0x00543f10
0065e200  e983030000            JMP 0x0065e588
0065e205  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e20b  e8e04cfdff            CALL 0x00632ef0
0065e210  8985ecfdffff          MOV dword ptr [EBP + 0xfffffdec],EAX
0065e216  6a01                  PUSH 0x1
0065e218  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e21e  e8bd5bf8ff            CALL 0x005e3de0
0065e223  85c0                  TEST EAX,EAX
0065e225  7407                  JZ 0x0065e22e
0065e227  33c0                  XOR EAX,EAX
0065e229  e95c030000            JMP 0x0065e58a
0065e22e  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0065e233  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0065e236  898d54fcffff          MOV dword ptr [EBP + 0xfffffc54],ECX
0065e23c  8b95ecfdffff          MOV EDX,dword ptr [EBP + 0xfffffdec]
0065e242  52                    PUSH EDX
0065e243  8b8d54fcffff          MOV ECX,dword ptr [EBP + 0xfffffc54]
0065e249  e8b2deebff            CALL 0x0051c100
0065e24e  8985f4fdffff          MOV dword ptr [EBP + 0xfffffdf4],EAX
0065e254  83bdf4fdffff00        CMP dword ptr [EBP + 0xfffffdf4],0x0
0065e25b  741b                  JZ 0x0065e278
0065e25d  8b85f4fdffff          MOV EAX,dword ptr [EBP + 0xfffffdf4]
0065e263  8b88f0100000          MOV ECX,dword ptr [EAX + 0x10f0]
0065e269  898d50fcffff          MOV dword ptr [EBP + 0xfffffc50],ECX
0065e26f  83bd50fcffff00        CMP dword ptr [EBP + 0xfffffc50],0x0
0065e276  7407                  JZ 0x0065e27f
0065e278  33c0                  XOR EAX,EAX
0065e27a  e90b030000            JMP 0x0065e58a
0065e27f  8b95f4fdffff          MOV EDX,dword ptr [EBP + 0xfffffdf4]
0065e285  81c248010000          ADD EDX,0x148
0065e28b  8995f0fdffff          MOV dword ptr [EBP + 0xfffffdf0],EDX
0065e291  7507                  JNZ 0x0065e29a
0065e293  33c0                  XOR EAX,EAX
0065e295  e9f0020000            JMP 0x0065e58a
0065e29a  c785f8fdffff00000000  MOV dword ptr [EBP + 0xfffffdf8],0x0
0065e2a4  eb0f                  JMP 0x0065e2b5
0065e2a6  8b85f8fdffff          MOV EAX,dword ptr [EBP + 0xfffffdf8]
0065e2ac  83c001                ADD EAX,0x1
0065e2af  8985f8fdffff          MOV dword ptr [EBP + 0xfffffdf8],EAX
0065e2b5  8b8df0fdffff          MOV ECX,dword ptr [EBP + 0xfffffdf0]
0065e2bb  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0065e2be  89954cfcffff          MOV dword ptr [EBP + 0xfffffc4c],EDX
0065e2c4  8b85f8fdffff          MOV EAX,dword ptr [EBP + 0xfffffdf8]
0065e2ca  3b854cfcffff          CMP EAX,dword ptr [EBP + 0xfffffc4c]
0065e2d0  0f8d95000000          JGE 0x0065e36b
0065e2d6  8b8df0fdffff          MOV ECX,dword ptr [EBP + 0xfffffdf0]
0065e2dc  8b11                  MOV EDX,dword ptr [ECX]
0065e2de  8b85f8fdffff          MOV EAX,dword ptr [EBP + 0xfffffdf8]
0065e2e4  8d0c82                LEA ECX,[EDX + EAX*0x4]
0065e2e7  898d48fcffff          MOV dword ptr [EBP + 0xfffffc48],ECX
0065e2ed  8b9548fcffff          MOV EDX,dword ptr [EBP + 0xfffffc48]
0065e2f3  8b02                  MOV EAX,dword ptr [EDX]
0065e2f5  668b4808              MOV CX,word ptr [EAX + 0x8]
0065e2f9  66898d46fcffff        MOV word ptr [EBP + 0xfffffc46],CX
0065e300  0fb79546fcffff        MOVZX EDX,word ptr [EBP + 0xfffffc46]
0065e307  3b1514609900          CMP EDX,dword ptr [0x00996014]
0065e30d  7557                  JNZ 0x0065e366
0065e30f  8b85f0fdffff          MOV EAX,dword ptr [EBP + 0xfffffdf0]
0065e315  8b08                  MOV ECX,dword ptr [EAX]
0065e317  8b95f8fdffff          MOV EDX,dword ptr [EBP + 0xfffffdf8]
0065e31d  8d0491                LEA EAX,[ECX + EDX*0x4]
0065e320  898540fcffff          MOV dword ptr [EBP + 0xfffffc40],EAX
0065e326  8b8d40fcffff          MOV ECX,dword ptr [EBP + 0xfffffc40]
0065e32c  8b11                  MOV EDX,dword ptr [ECX]
0065e32e  89953cfcffff          MOV dword ptr [EBP + 0xfffffc3c],EDX
0065e334  8b853cfcffff          MOV EAX,dword ptr [EBP + 0xfffffc3c]
0065e33a  8b08                  MOV ECX,dword ptr [EAX]
0065e33c  898d34fcffff          MOV dword ptr [EBP + 0xfffffc34],ECX
0065e342  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0065e345  899538fcffff          MOV dword ptr [EBP + 0xfffffc38],EDX
0065e34b  8b8538fcffff          MOV EAX,dword ptr [EBP + 0xfffffc38]
0065e351  50                    PUSH EAX
0065e352  8b8d34fcffff          MOV ECX,dword ptr [EBP + 0xfffffc34]
0065e358  51                    PUSH ECX
0065e359  8b8df4fdffff          MOV ECX,dword ptr [EBP + 0xfffffdf4]
0065e35f  e82c64eeff            CALL 0x00544790
0065e364  eb05                  JMP 0x0065e36b
0065e366  e93bffffff            JMP 0x0065e2a6
0065e36b  e918020000            JMP 0x0065e588
0065e370  c785e0fdffff00000000  MOV dword ptr [EBP + 0xfffffde0],0x0
0065e37a  c785e4fdffff00000000  MOV dword ptr [EBP + 0xfffffde4],0x0
0065e384  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e38a  e8614bfdff            CALL 0x00632ef0
0065e38f  8985e8fdffff          MOV dword ptr [EBP + 0xfffffde8],EAX
0065e395  6a01                  PUSH 0x1
0065e397  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e39d  e83e5af8ff            CALL 0x005e3de0
0065e3a2  85c0                  TEST EAX,EAX
0065e3a4  7511                  JNZ 0x0065e3b7
0065e3a6  6a01                  PUSH 0x1
0065e3a8  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e3ae  e88d5af8ff            CALL 0x005e3e40
0065e3b3  85c0                  TEST EAX,EAX
0065e3b5  7407                  JZ 0x0065e3be
0065e3b7  33c0                  XOR EAX,EAX
0065e3b9  e9cc010000            JMP 0x0065e58a
0065e3be  837df000              CMP dword ptr [EBP + -0x10],0x0
0065e3c2  7418                  JZ 0x0065e3dc
0065e3c4  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0065e3c7  8b82ec000000          MOV EAX,dword ptr [EDX + 0xec]
0065e3cd  898530fcffff          MOV dword ptr [EBP + 0xfffffc30],EAX
0065e3d3  83bd30fcffff00        CMP dword ptr [EBP + 0xfffffc30],0x0
0065e3da  7507                  JNZ 0x0065e3e3
0065e3dc  33c0                  XOR EAX,EAX
0065e3de  e9a7010000            JMP 0x0065e58a
0065e3e3  6a02                  PUSH 0x2
0065e3e5  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065e3e8  e8a358f0ff            CALL 0x00563c90
0065e3ed  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0065e3f3  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0065e3f6  89952cfcffff          MOV dword ptr [EBP + 0xfffffc2c],EDX
0065e3fc  8d85e0fdffff          LEA EAX,[EBP + 0xfffffde0]
0065e402  50                    PUSH EAX
0065e403  8b8de8fdffff          MOV ECX,dword ptr [EBP + 0xfffffde8]
0065e409  51                    PUSH ECX
0065e40a  8b8d2cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc2c]
0065e410  e86bdcebff            CALL 0x0051c080
0065e415  8bc8                  MOV ECX,EAX
0065e417  e894fbedff            CALL 0x0053dfb0
0065e41c  0fb6d0                MOVZX EDX,AL
0065e41f  0fb6057f449900        MOVZX EAX,byte ptr [0x0099447f]
0065e426  3bd0                  CMP EDX,EAX
0065e428  7553                  JNZ 0x0065e47d
0065e42a  8b8de0fdffff          MOV ECX,dword ptr [EBP + 0xfffffde0]
0065e430  8b11                  MOV EDX,dword ptr [ECX]
0065e432  8b8de0fdffff          MOV ECX,dword ptr [EBP + 0xfffffde0]
0065e438  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0065e43b  ffd0                  CALL EAX
0065e43d  8985e4fdffff          MOV dword ptr [EBP + 0xfffffde4],EAX
0065e443  83bde4fdffff00        CMP dword ptr [EBP + 0xfffffde4],0x0
0065e44a  7507                  JNZ 0x0065e453
0065e44c  33c0                  XOR EAX,EAX
0065e44e  e937010000            JMP 0x0065e58a
0065e453  8b8de4fdffff          MOV ECX,dword ptr [EBP + 0xfffffde4]
0065e459  8b9104030000          MOV EDX,dword ptr [ECX + 0x304]
0065e45f  899528fcffff          MOV dword ptr [EBP + 0xfffffc28],EDX
0065e465  83bd28fcffff00        CMP dword ptr [EBP + 0xfffffc28],0x0
0065e46c  740f                  JZ 0x0065e47d
0065e46e  8b85e8fdffff          MOV EAX,dword ptr [EBP + 0xfffffde8]
0065e474  50                    PUSH EAX
0065e475  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065e478  e8c396eeff            CALL 0x00547b40
0065e47d  b801000000            MOV EAX,0x1
0065e482  e903010000            JMP 0x0065e58a
0065e48c  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e492  e8594afdff            CALL 0x00632ef0
0065e497  8985dcfdffff          MOV dword ptr [EBP + 0xfffffddc],EAX
0065e49d  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e4a3  e8f851f8ff            CALL 0x005e36a0
0065e4a8  8985d8fdffff          MOV dword ptr [EBP + 0xfffffdd8],EAX
0065e4ae  6a01                  PUSH 0x1
0065e4b0  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e4b6  e82559f8ff            CALL 0x005e3de0
0065e4bb  85c0                  TEST EAX,EAX
0065e4bd  7511                  JNZ 0x0065e4d0
0065e4bf  6a01                  PUSH 0x1
0065e4c1  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e4c7  e87459f8ff            CALL 0x005e3e40
0065e4cc  85c0                  TEST EAX,EAX
0065e4ce  7407                  JZ 0x0065e4d7
0065e4d0  33c0                  XOR EAX,EAX
0065e4d2  e9b3000000            JMP 0x0065e58a
0065e4d7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065e4da  e84145fdff            CALL 0x00632a20
0065e4df  89850cfcffff          MOV dword ptr [EBP + 0xfffffc0c],EAX
0065e4e5  8b8d0cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc0c]
0065e4eb  8b11                  MOV EDX,dword ptr [ECX]
0065e4ed  8b8d0cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc0c]
0065e4f3  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065e4f6  ffd0                  CALL EAX
0065e4f8  8945f0                MOV dword ptr [EBP + -0x10],EAX
0065e4fb  837df000              CMP dword ptr [EBP + -0x10],0x0
0065e4ff  7411                  JZ 0x0065e512
0065e501  8b8dd8fdffff          MOV ECX,dword ptr [EBP + 0xfffffdd8]
0065e507  51                    PUSH ECX
0065e508  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065e50b  e8a096eeff            CALL 0x00547bb0
0065e510  eb78                  JMP 0x0065e58a
0065e512  33c0                  XOR EAX,EAX
0065e514  eb74                  JMP 0x0065e58a
0065e518  6a08                  PUSH 0x8
0065e51a  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e520  e87b52f8ff            CALL 0x005e37a0
0065e525  8885d7fdffff          MOV byte ptr [EBP + 0xfffffdd7],AL
0065e52b  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e531  e8ba49fdff            CALL 0x00632ef0
0065e536  8985d0fdffff          MOV dword ptr [EBP + 0xfffffdd0],EAX
0065e53c  6a01                  PUSH 0x1
0065e53e  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e544  e89758f8ff            CALL 0x005e3de0
0065e549  85c0                  TEST EAX,EAX
0065e54b  7511                  JNZ 0x0065e55e
0065e54d  6a01                  PUSH 0x1
0065e54f  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
0065e555  e8e658f8ff            CALL 0x005e3e40
0065e55a  85c0                  TEST EAX,EAX
0065e55c  7404                  JZ 0x0065e562
0065e55e  33c0                  XOR EAX,EAX
0065e560  eb28                  JMP 0x0065e58a
0065e562  6a00                  PUSH 0x0
0065e564  6a00                  PUSH 0x0
0065e566  6a00                  PUSH 0x0
0065e568  6a01                  PUSH 0x1
0065e56a  8b95d0fdffff          MOV EDX,dword ptr [EBP + 0xfffffdd0]
0065e570  52                    PUSH EDX
0065e571  0fbe85d7fdffff        MOVSX EAX,byte ptr [EBP + 0xfffffdd7]
0065e578  50                    PUSH EAX
0065e579  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0065e57c  e88f7ef0ff            CALL 0x00566410
0065e581  b801000000            MOV EAX,0x1
0065e586  eb02                  JMP 0x0065e58a
0065e588  33c0                  XOR EAX,EAX
0065e58a  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0065e58d  64890d00000000        MOV dword ptr FS:[0x0],ECX
0065e594  59                    POP ECX
0065e595  8be5                  MOV ESP,EBP
0065e597  5d                    POP EBP
0065e598  c20800                RET 0x8
