; FN 0052d130 FUN_0052d130
0052d130  55                    PUSH EBP
0052d131  8bec                  MOV EBP,ESP
0052d133  83ec54                SUB ESP,0x54
0052d136  894db0                MOV dword ptr [EBP + -0x50],ECX
0052d139  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
0052d140  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
0052d143  83784800              CMP dword ptr [EAX + 0x48],0x0
0052d147  0f8e5a030000          JLE 0x0052d4a7
0052d14d  d9ee                  FLDZ
0052d14f  d95de4                FSTP float ptr [EBP + -0x1c]
0052d152  d9ee                  FLDZ
0052d154  d95de8                FSTP float ptr [EBP + -0x18]
0052d157  d9ee                  FLDZ
0052d159  d95dec                FSTP float ptr [EBP + -0x14]
0052d15c  d9ee                  FLDZ
0052d15e  d95dc0                FSTP float ptr [EBP + -0x40]
0052d161  d9ee                  FLDZ
0052d163  d95dc4                FSTP float ptr [EBP + -0x3c]
0052d166  d9ee                  FLDZ
0052d168  d95dc8                FSTP float ptr [EBP + -0x38]
0052d16b  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0052d16e  d901                  FLD float ptr [ECX]
0052d170  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0052d173  d902                  FLD float ptr [EDX]
0052d175  ded9                  FCOMPP
0052d177  dfe0                  FNSTSW AX
0052d179  f6c441                TEST AH,0x41
0052d17c  7512                  JNZ 0x0052d190
0052d17e  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052d181  d900                  FLD float ptr [EAX]
0052d183  d95de4                FSTP float ptr [EBP + -0x1c]
0052d186  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0052d189  d901                  FLD float ptr [ECX]
0052d18b  d95dc0                FSTP float ptr [EBP + -0x40]
0052d18e  eb10                  JMP 0x0052d1a0
0052d190  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0052d193  d902                  FLD float ptr [EDX]
0052d195  d95de4                FSTP float ptr [EBP + -0x1c]
0052d198  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052d19b  d900                  FLD float ptr [EAX]
0052d19d  d95dc0                FSTP float ptr [EBP + -0x40]
0052d1a0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0052d1a3  d94104                FLD float ptr [ECX + 0x4]
0052d1a6  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0052d1a9  d94204                FLD float ptr [EDX + 0x4]
0052d1ac  ded9                  FCOMPP
0052d1ae  dfe0                  FNSTSW AX
0052d1b0  f6c441                TEST AH,0x41
0052d1b3  7514                  JNZ 0x0052d1c9
0052d1b5  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052d1b8  d94004                FLD float ptr [EAX + 0x4]
0052d1bb  d95de8                FSTP float ptr [EBP + -0x18]
0052d1be  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0052d1c1  d94104                FLD float ptr [ECX + 0x4]
0052d1c4  d95dc4                FSTP float ptr [EBP + -0x3c]
0052d1c7  eb12                  JMP 0x0052d1db
0052d1c9  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0052d1cc  d94204                FLD float ptr [EDX + 0x4]
0052d1cf  d95de8                FSTP float ptr [EBP + -0x18]
0052d1d2  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052d1d5  d94004                FLD float ptr [EAX + 0x4]
0052d1d8  d95dc4                FSTP float ptr [EBP + -0x3c]
0052d1db  d94510                FLD float ptr [EBP + 0x10]
0052d1de  d95dec                FSTP float ptr [EBP + -0x14]
0052d1e1  d94514                FLD float ptr [EBP + 0x14]
0052d1e4  d95dc8                FSTP float ptr [EBP + -0x38]
0052d1e7  d945e4                FLD float ptr [EBP + -0x1c]
0052d1ea  d86518                FSUB float ptr [EBP + 0x18]
0052d1ed  d95de4                FSTP float ptr [EBP + -0x1c]
0052d1f0  d945e8                FLD float ptr [EBP + -0x18]
0052d1f3  d86518                FSUB float ptr [EBP + 0x18]
0052d1f6  d95de8                FSTP float ptr [EBP + -0x18]
0052d1f9  d945c0                FLD float ptr [EBP + -0x40]
0052d1fc  d84518                FADD float ptr [EBP + 0x18]
0052d1ff  d95dc0                FSTP float ptr [EBP + -0x40]
0052d202  d945c4                FLD float ptr [EBP + -0x3c]
0052d205  d84518                FADD float ptr [EBP + 0x18]
0052d208  d95dc4                FSTP float ptr [EBP + -0x3c]
0052d20b  d9ee                  FLDZ
0052d20d  d95df0                FSTP float ptr [EBP + -0x10]
0052d210  d9ee                  FLDZ
0052d212  d95df4                FSTP float ptr [EBP + -0xc]
0052d215  d9ee                  FLDZ
0052d217  d95df8                FSTP float ptr [EBP + -0x8]
0052d21a  d9ee                  FLDZ
0052d21c  d95dcc                FSTP float ptr [EBP + -0x34]
0052d21f  d9ee                  FLDZ
0052d221  d95dd0                FSTP float ptr [EBP + -0x30]
0052d224  d9ee                  FLDZ
0052d226  d95dd4                FSTP float ptr [EBP + -0x2c]
0052d229  c745e000000000        MOV dword ptr [EBP + -0x20],0x0
0052d230  eb09                  JMP 0x0052d23b
0052d232  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0052d235  83c101                ADD ECX,0x1
0052d238  894de0                MOV dword ptr [EBP + -0x20],ECX
0052d23b  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
0052d23e  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
0052d241  3b4248                CMP EAX,dword ptr [EDX + 0x48]
0052d244  0f8d5d020000          JGE 0x0052d4a7
0052d24a  837d3400              CMP dword ptr [EBP + 0x34],0x0
0052d24e  7456                  JZ 0x0052d2a6
0052d250  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0052d256  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0052d259  8955b8                MOV dword ptr [EBP + -0x48],EDX
0052d25c  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
0052d25f  8b4850                MOV ECX,dword ptr [EAX + 0x50]
0052d262  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0052d265  8b0491                MOV EAX,dword ptr [ECX + EDX*0x4]
0052d268  50                    PUSH EAX
0052d269  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
0052d26c  e82feefeff            CALL 0x0051c0a0
0052d271  8945dc                MOV dword ptr [EBP + -0x24],EAX
0052d274  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0052d278  742c                  JZ 0x0052d2a6
0052d27a  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0052d27d  8b11                  MOV EDX,dword ptr [ECX]
0052d27f  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0052d282  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0052d285  ffd0                  CALL EAX
0052d287  8945d8                MOV dword ptr [EBP + -0x28],EAX
0052d28a  837dd800              CMP dword ptr [EBP + -0x28],0x0
0052d28e  7414                  JZ 0x0052d2a4
0052d290  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0052d293  8a911c030000          MOV DL,byte ptr [ECX + 0x31c]
0052d299  8855b7                MOV byte ptr [EBP + -0x49],DL
0052d29c  0fb645b7              MOVZX EAX,byte ptr [EBP + -0x49]
0052d2a0  85c0                  TEST EAX,EAX
0052d2a2  7402                  JZ 0x0052d2a6
0052d2a4  eb8c                  JMP 0x0052d232
0052d2a6  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0052d2a9  8d540901              LEA EDX,[ECX + ECX*0x1 + 0x1]
0052d2ad  6bd20c                IMUL EDX,EDX,0xc
0052d2b0  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
0052d2b3  8b484c                MOV ECX,dword ptr [EAX + 0x4c]
0052d2b6  03ca                  ADD ECX,EDX
0052d2b8  8b11                  MOV EDX,dword ptr [ECX]
0052d2ba  8955f0                MOV dword ptr [EBP + -0x10],EDX
0052d2bd  8b4104                MOV EAX,dword ptr [ECX + 0x4]
0052d2c0  8945f4                MOV dword ptr [EBP + -0xc],EAX
0052d2c3  8b4908                MOV ECX,dword ptr [ECX + 0x8]
0052d2c6  894df8                MOV dword ptr [EBP + -0x8],ECX
0052d2c9  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0052d2cc  d1e2                  SHL EDX,0x1
0052d2ce  6bd20c                IMUL EDX,EDX,0xc
0052d2d1  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
0052d2d4  8b484c                MOV ECX,dword ptr [EAX + 0x4c]
0052d2d7  03ca                  ADD ECX,EDX
0052d2d9  8b11                  MOV EDX,dword ptr [ECX]
0052d2db  8955cc                MOV dword ptr [EBP + -0x34],EDX
0052d2de  8b4104                MOV EAX,dword ptr [ECX + 0x4]
0052d2e1  8945d0                MOV dword ptr [EBP + -0x30],EAX
0052d2e4  8b4908                MOV ECX,dword ptr [ECX + 0x8]
0052d2e7  894dd4                MOV dword ptr [EBP + -0x2c],ECX
0052d2ea  d945e4                FLD float ptr [EBP + -0x1c]
0052d2ed  d945cc                FLD float ptr [EBP + -0x34]
0052d2f0  ded9                  FCOMPP
0052d2f2  dfe0                  FNSTSW AX
0052d2f4  f6c441                TEST AH,0x41
0052d2f7  7513                  JNZ 0x0052d30c
0052d2f9  d945c0                FLD float ptr [EBP + -0x40]
0052d2fc  d945cc                FLD float ptr [EBP + -0x34]
0052d2ff  ded9                  FCOMPP
0052d301  dfe0                  FNSTSW AX
0052d303  f6c441                TEST AH,0x41
0052d306  0f84a3000000          JZ 0x0052d3af
0052d30c  d945c0                FLD float ptr [EBP + -0x40]
0052d30f  d945f0                FLD float ptr [EBP + -0x10]
0052d312  ded9                  FCOMPP
0052d314  dfe0                  FNSTSW AX
0052d316  f6c405                TEST AH,0x5
0052d319  7a13                  JP 0x0052d32e
0052d31b  d945e4                FLD float ptr [EBP + -0x1c]
0052d31e  d945f0                FLD float ptr [EBP + -0x10]
0052d321  ded9                  FCOMPP
0052d323  dfe0                  FNSTSW AX
0052d325  f6c405                TEST AH,0x5
0052d328  0f8b81000000          JNP 0x0052d3af
0052d32e  d945e8                FLD float ptr [EBP + -0x18]
0052d331  d945d0                FLD float ptr [EBP + -0x30]
0052d334  ded9                  FCOMPP
0052d336  dfe0                  FNSTSW AX
0052d338  f6c441                TEST AH,0x41
0052d33b  750f                  JNZ 0x0052d34c
0052d33d  d945c4                FLD float ptr [EBP + -0x3c]
0052d340  d945d0                FLD float ptr [EBP + -0x30]
0052d343  ded9                  FCOMPP
0052d345  dfe0                  FNSTSW AX
0052d347  f6c441                TEST AH,0x41
0052d34a  7463                  JZ 0x0052d3af
0052d34c  d945c4                FLD float ptr [EBP + -0x3c]
0052d34f  d945f4                FLD float ptr [EBP + -0xc]
0052d352  ded9                  FCOMPP
0052d354  dfe0                  FNSTSW AX
0052d356  f6c405                TEST AH,0x5
0052d359  7a0f                  JP 0x0052d36a
0052d35b  d945e8                FLD float ptr [EBP + -0x18]
0052d35e  d945f4                FLD float ptr [EBP + -0xc]
0052d361  ded9                  FCOMPP
0052d363  dfe0                  FNSTSW AX
0052d365  f6c405                TEST AH,0x5
0052d368  7b45                  JNP 0x0052d3af
0052d36a  d945ec                FLD float ptr [EBP + -0x14]
0052d36d  d945d4                FLD float ptr [EBP + -0x2c]
0052d370  ded9                  FCOMPP
0052d372  dfe0                  FNSTSW AX
0052d374  f6c441                TEST AH,0x41
0052d377  750f                  JNZ 0x0052d388
0052d379  d945c8                FLD float ptr [EBP + -0x38]
0052d37c  d945d4                FLD float ptr [EBP + -0x2c]
0052d37f  ded9                  FCOMPP
0052d381  dfe0                  FNSTSW AX
0052d383  f6c441                TEST AH,0x41
0052d386  7427                  JZ 0x0052d3af
0052d388  d945c8                FLD float ptr [EBP + -0x38]
0052d38b  d945f8                FLD float ptr [EBP + -0x8]
0052d38e  ded9                  FCOMPP
0052d390  dfe0                  FNSTSW AX
0052d392  f6c405                TEST AH,0x5
0052d395  7a0f                  JP 0x0052d3a6
0052d397  d945ec                FLD float ptr [EBP + -0x14]
0052d39a  d945f8                FLD float ptr [EBP + -0x8]
0052d39d  ded9                  FCOMPP
0052d39f  dfe0                  FNSTSW AX
0052d3a1  f6c405                TEST AH,0x5
0052d3a4  7b09                  JNP 0x0052d3af
0052d3a6  c745ac00000000        MOV dword ptr [EBP + -0x54],0x0
0052d3ad  eb07                  JMP 0x0052d3b6
0052d3af  c745ac01000000        MOV dword ptr [EBP + -0x54],0x1
0052d3b6  837dac00              CMP dword ptr [EBP + -0x54],0x0
0052d3ba  0f85e2000000          JNZ 0x0052d4a2
0052d3c0  8b5528                MOV EDX,dword ptr [EBP + 0x28]
0052d3c3  52                    PUSH EDX
0052d3c4  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
0052d3c7  8b4850                MOV ECX,dword ptr [EAX + 0x50]
0052d3ca  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0052d3cd  8b0491                MOV EAX,dword ptr [ECX + EDX*0x4]
0052d3d0  50                    PUSH EAX
0052d3d1  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
0052d3d4  8b11                  MOV EDX,dword ptr [ECX]
0052d3d6  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
0052d3d9  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0052d3dc  ffd0                  CALL EAX
0052d3de  85c0                  TEST EAX,EAX
0052d3e0  0f85bc000000          JNZ 0x0052d4a2
0052d3e6  8b4d30                MOV ECX,dword ptr [EBP + 0x30]
0052d3e9  51                    PUSH ECX
0052d3ea  8b5524                MOV EDX,dword ptr [EBP + 0x24]
0052d3ed  52                    PUSH EDX
0052d3ee  8b451c                MOV EAX,dword ptr [EBP + 0x1c]
0052d3f1  50                    PUSH EAX
0052d3f2  51                    PUSH ECX
0052d3f3  d94518                FLD float ptr [EBP + 0x18]
0052d3f6  d91c24                FSTP float ptr [ESP]
0052d3f9  51                    PUSH ECX
0052d3fa  d94514                FLD float ptr [EBP + 0x14]
0052d3fd  d91c24                FSTP float ptr [ESP]
0052d400  51                    PUSH ECX
0052d401  d94510                FLD float ptr [EBP + 0x10]
0052d404  d91c24                FSTP float ptr [ESP]
0052d407  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0052d40a  51                    PUSH ECX
0052d40b  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0052d40e  52                    PUSH EDX
0052d40f  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
0052d412  8b4850                MOV ECX,dword ptr [EAX + 0x50]
0052d415  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0052d418  8b0491                MOV EAX,dword ptr [ECX + EDX*0x4]
0052d41b  50                    PUSH EAX
0052d41c  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
0052d41f  e86c590200            CALL 0x00552d90
0052d424  8945bc                MOV dword ptr [EBP + -0x44],EAX
0052d427  837dbc00              CMP dword ptr [EBP + -0x44],0x0
0052d42b  7575                  JNZ 0x0052d4a2
0052d42d  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
0052d430  8b5150                MOV EDX,dword ptr [ECX + 0x50]
0052d433  8b4520                MOV EAX,dword ptr [EBP + 0x20]
0052d436  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0052d439  8b148a                MOV EDX,dword ptr [EDX + ECX*0x4]
0052d43c  8910                  MOV dword ptr [EAX],EDX
0052d43e  837d3000              CMP dword ptr [EBP + 0x30],0x0
0052d442  740b                  JZ 0x0052d44f
0052d444  8b4530                MOV EAX,dword ptr [EBP + 0x30]
0052d447  8b4d20                MOV ECX,dword ptr [EBP + 0x20]
0052d44a  8b11                  MOV EDX,dword ptr [ECX]
0052d44c  895040                MOV dword ptr [EAX + 0x40],EDX
0052d44f  837d2c01              CMP dword ptr [EBP + 0x2c],0x1
0052d453  7546                  JNZ 0x0052d49b
0052d455  8b452c                MOV EAX,dword ptr [EBP + 0x2c]
0052d458  50                    PUSH EAX
0052d459  8b4d28                MOV ECX,dword ptr [EBP + 0x28]
0052d45c  51                    PUSH ECX
0052d45d  8b5524                MOV EDX,dword ptr [EBP + 0x24]
0052d460  52                    PUSH EDX
0052d461  8b4520                MOV EAX,dword ptr [EBP + 0x20]
0052d464  50                    PUSH EAX
0052d465  8b4d1c                MOV ECX,dword ptr [EBP + 0x1c]
0052d468  51                    PUSH ECX
0052d469  51                    PUSH ECX
0052d46a  d94518                FLD float ptr [EBP + 0x18]
0052d46d  d91c24                FSTP float ptr [ESP]
0052d470  51                    PUSH ECX
0052d471  d94514                FLD float ptr [EBP + 0x14]
0052d474  d91c24                FSTP float ptr [ESP]
0052d477  51                    PUSH ECX
0052d478  d94510                FLD float ptr [EBP + 0x10]
0052d47b  d91c24                FSTP float ptr [ESP]
0052d47e  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0052d481  52                    PUSH EDX
0052d482  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052d485  50                    PUSH EAX
0052d486  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
0052d489  8b11                  MOV EDX,dword ptr [ECX]
0052d48b  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
0052d48e  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0052d491  ffd0                  CALL EAX
0052d493  2345fc                AND EAX,dword ptr [EBP + -0x4]
0052d496  8945fc                MOV dword ptr [EBP + -0x4],EAX
0052d499  eb07                  JMP 0x0052d4a2
0052d49b  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0052d4a2  e98bfdffff            JMP 0x0052d232
0052d4a7  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0052d4aa  8be5                  MOV ESP,EBP
0052d4ac  5d                    POP EBP
0052d4ad  c23000                RET 0x30
