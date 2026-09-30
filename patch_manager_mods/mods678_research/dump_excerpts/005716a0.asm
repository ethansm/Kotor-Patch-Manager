; FN 005716a0 FUN_005716a0
005716a0  55                    PUSH EBP
005716a1  8bec                  MOV EBP,ESP
005716a3  83ec58                SUB ESP,0x58
005716a6  894da8                MOV dword ptr [EBP + -0x58],ECX
005716a9  d9ee                  FLDZ
005716ab  d95df8                FSTP float ptr [EBP + -0x8]
005716ae  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005716b3  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005716b6  894dcc                MOV dword ptr [EBP + -0x34],ECX
005716b9  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005716bc  52                    PUSH EDX
005716bd  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
005716c0  e8dba9faff            CALL 0x0051c0a0
005716c5  8945fc                MOV dword ptr [EBP + -0x4],EAX
005716c8  837dfc00              CMP dword ptr [EBP + -0x4],0x0
005716cc  0f84ba010000          JZ 0x0057188c
005716d2  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005716d5  8b10                  MOV EDX,dword ptr [EAX]
005716d7  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005716da  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005716dd  ffd0                  CALL EAX
005716df  85c0                  TEST EAX,EAX
005716e1  0f84f2000000          JZ 0x005717d9
005716e7  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005716ea  8b11                  MOV EDX,dword ptr [ECX]
005716ec  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005716ef  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005716f2  ffd0                  CALL EAX
005716f4  8945f4                MOV dword ptr [EBP + -0xc],EAX
005716f7  837d0c00              CMP dword ptr [EBP + 0xc],0x0
005716fb  744d                  JZ 0x0057174a
005716fd  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
00571700  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
00571706  8955c8                MOV dword ptr [EBP + -0x38],EDX
00571709  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
0057170c  d94008                FLD float ptr [EAX + 0x8]
0057170f  d95dc4                FSTP float ptr [EBP + -0x3c]
00571712  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00571715  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
0057171b  8955c0                MOV dword ptr [EBP + -0x40],EDX
0057171e  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
00571721  d94008                FLD float ptr [EAX + 0x8]
00571724  d95dbc                FSTP float ptr [EBP + -0x44]
00571727  d945c4                FLD float ptr [EBP + -0x3c]
0057172a  dc05087f9800          FADD double ptr [0x00987f08]
00571730  d845bc                FADD float ptr [EBP + -0x44]
00571733  dc05f07d9800          FADD double ptr [0x00987df0]
00571739  d95df8                FSTP float ptr [EBP + -0x8]
0057173c  d905e0519900          FLD float ptr [0x009951e0]
00571742  d95df8                FSTP float ptr [EBP + -0x8]
00571745  e98a000000            JMP 0x005717d4
0057174a  d905dc519900          FLD float ptr [0x009951dc]
00571750  d95df8                FSTP float ptr [EBP + -0x8]
00571753  d9ee                  FLDZ
00571755  d95df0                FSTP float ptr [EBP + -0x10]
00571758  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
0057175e  8b9130010000          MOV EDX,dword ptr [ECX + 0x130]
00571764  8955b8                MOV dword ptr [EBP + -0x48],EDX
00571767  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
0057176a  8b4810                MOV ECX,dword ptr [EAX + 0x10]
0057176d  894db4                MOV dword ptr [EBP + -0x4c],ECX
00571770  8d55f0                LEA EDX,[EBP + -0x10]
00571773  52                    PUSH EDX
00571774  a1dcfda000            MOV EAX,[0x00a0fddc]
00571779  50                    PUSH EAX
0057177a  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
0057177d  0fb79184110000        MOVZX EDX,word ptr [ECX + 0x1184]
00571784  52                    PUSH EDX
00571785  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
00571788  e8639c1a00            CALL 0x0071b3f0
0057178d  d945f8                FLD float ptr [EBP + -0x8]
00571790  d845f0                FADD float ptr [EBP + -0x10]
00571793  d95df8                FSTP float ptr [EBP + -0x8]
00571796  a1d0b4a100            MOV EAX,[0x00a1b4d0]
0057179b  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
005717a1  894db0                MOV dword ptr [EBP + -0x50],ECX
005717a4  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
005717a7  8b4210                MOV EAX,dword ptr [EDX + 0x10]
005717aa  8945ac                MOV dword ptr [EBP + -0x54],EAX
005717ad  8d4df0                LEA ECX,[EBP + -0x10]
005717b0  51                    PUSH ECX
005717b1  8b15dcfda000          MOV EDX,dword ptr [0x00a0fddc]
005717b7  52                    PUSH EDX
005717b8  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005717bb  0fb78884110000        MOVZX ECX,word ptr [EAX + 0x1184]
005717c2  51                    PUSH ECX
005717c3  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
005717c6  e8259c1a00            CALL 0x0071b3f0
005717cb  d945f8                FLD float ptr [EBP + -0x8]
005717ce  d845f0                FADD float ptr [EBP + -0x10]
005717d1  d95df8                FSTP float ptr [EBP + -0x8]
005717d4  e9b3000000            JMP 0x0057188c
005717d9  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005717dc  8b02                  MOV EAX,dword ptr [EDX]
005717de  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005717e1  8b5018                MOV EDX,dword ptr [EAX + 0x18]
005717e4  ffd2                  CALL EDX
005717e6  85c0                  TEST EAX,EAX
005717e8  7444                  JZ 0x0057182e
005717ea  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005717ed  8b10                  MOV EDX,dword ptr [EAX]
005717ef  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005717f2  8b4218                MOV EAX,dword ptr [EDX + 0x18]
005717f5  ffd0                  CALL EAX
005717f7  8945e0                MOV dword ptr [EBP + -0x20],EAX
005717fa  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005717fd  81c194000000          ADD ECX,0x94
00571803  8b11                  MOV EDX,dword ptr [ECX]
00571805  8955e4                MOV dword ptr [EBP + -0x1c],EDX
00571808  8b4104                MOV EAX,dword ptr [ECX + 0x4]
0057180b  8945e8                MOV dword ptr [EBP + -0x18],EAX
0057180e  8b4908                MOV ECX,dword ptr [ECX + 0x8]
00571811  894dec                MOV dword ptr [EBP + -0x14],ECX
00571814  6a00                  PUSH 0x0
00571816  6a00                  PUSH 0x0
00571818  8d55f8                LEA EDX,[EBP + -0x8]
0057181b  52                    PUSH EDX
0057181c  8d45e4                LEA EAX,[EBP + -0x1c]
0057181f  50                    PUSH EAX
00571820  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00571823  51                    PUSH ECX
00571824  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
00571827  e8e4460100            CALL 0x00585f10
0057182c  eb5e                  JMP 0x0057188c
0057182e  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00571831  8b02                  MOV EAX,dword ptr [EDX]
00571833  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00571836  8b5048                MOV EDX,dword ptr [EAX + 0x48]
00571839  ffd2                  CALL EDX
0057183b  85c0                  TEST EAX,EAX
0057183d  7444                  JZ 0x00571883
0057183f  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00571842  8b10                  MOV EDX,dword ptr [EAX]
00571844  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00571847  8b4248                MOV EAX,dword ptr [EDX + 0x48]
0057184a  ffd0                  CALL EAX
0057184c  8945d0                MOV dword ptr [EBP + -0x30],EAX
0057184f  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00571852  81c194000000          ADD ECX,0x94
00571858  8b11                  MOV EDX,dword ptr [ECX]
0057185a  8955d4                MOV dword ptr [EBP + -0x2c],EDX
0057185d  8b4104                MOV EAX,dword ptr [ECX + 0x4]
00571860  8945d8                MOV dword ptr [EBP + -0x28],EAX
00571863  8b4908                MOV ECX,dword ptr [ECX + 0x8]
00571866  894ddc                MOV dword ptr [EBP + -0x24],ECX
00571869  6a00                  PUSH 0x0
0057186b  6a00                  PUSH 0x0
0057186d  8d55f8                LEA EDX,[EBP + -0x8]
00571870  52                    PUSH EDX
00571871  8d45d4                LEA EAX,[EBP + -0x2c]
00571874  50                    PUSH EAX
00571875  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00571878  51                    PUSH ECX
00571879  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
0057187c  e88f460100            CALL 0x00585f10
00571881  eb09                  JMP 0x0057188c
00571883  d905c0b59800          FLD float ptr [0x0098b5c0]
00571889  d95df8                FSTP float ptr [EBP + -0x8]
0057188c  d945f8                FLD float ptr [EBP + -0x8]
0057188f  8be5                  MOV ESP,EBP
00571891  5d                    POP EBP
00571892  c20800                RET 0x8
