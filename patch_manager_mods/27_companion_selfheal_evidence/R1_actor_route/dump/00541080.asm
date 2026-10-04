; FN 00541080 FUN_00541080
00541080  55                    PUSH EBP
00541081  8bec                  MOV EBP,ESP
00541083  83ec7c                SUB ESP,0x7c
00541086  894d88                MOV dword ptr [EBP + -0x78],ECX
00541089  8b4588                MOV EAX,dword ptr [EBP + -0x78]
0054108c  8b10                  MOV EDX,dword ptr [EAX]
0054108e  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541091  8b4210                MOV EAX,dword ptr [EDX + 0x10]
00541094  ffd0                  CALL EAX
00541096  85c0                  TEST EAX,EAX
00541098  7505                  JNZ 0x0054109f
0054109a  e9e6020000            JMP 0x00541385
0054109f  833dc8e0a70000        CMP dword ptr [0x00a7e0c8],0x0
005410a6  743e                  JZ 0x005410e6
005410a8  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005410ab  8b11                  MOV EDX,dword ptr [ECX]
005410ad  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005410b0  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005410b3  ffd0                  CALL EAX
005410b5  85c0                  TEST EAX,EAX
005410b7  742d                  JZ 0x005410e6
005410b9  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005410bc  8b11                  MOV EDX,dword ptr [ECX]
005410be  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005410c1  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005410c4  ffd0                  CALL EAX
005410c6  8945e0                MOV dword ptr [EBP + -0x20],EAX
005410c9  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005410cc  8b91ac110000          MOV EDX,dword ptr [ECX + 0x11ac]
005410d2  8955dc                MOV dword ptr [EBP + -0x24],EDX
005410d5  837ddc00              CMP dword ptr [EBP + -0x24],0x0
005410d9  740b                  JZ 0x005410e6
005410db  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005410de  83c118                ADD ECX,0x18
005410e1  e82a2f1f00            CALL 0x00734010
005410e6  8b4588                MOV EAX,dword ptr [EBP + -0x78]
005410e9  83b8ec00000001        CMP dword ptr [EAX + 0xec],0x1
005410f0  0f858f020000          JNZ 0x00541385
005410f6  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005410f9  81c100010000          ADD ECX,0x100
005410ff  e81c062600            CALL 0x007a1720
00541104  8945fc                MOV dword ptr [EBP + -0x4],EAX
00541107  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0054110b  0f84b3000000          JZ 0x005411c4
00541111  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00541114  51                    PUSH ECX
00541115  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541118  81c100010000          ADD ECX,0x100
0054111e  e80d030200            CALL 0x00561430
00541123  8945f4                MOV dword ptr [EBP + -0xc],EAX
00541126  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00541129  8955f8                MOV dword ptr [EBP + -0x8],EDX
0054112c  8d45fc                LEA EAX,[EBP + -0x4]
0054112f  50                    PUSH EAX
00541130  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541133  81c100010000          ADD ECX,0x100
00541139  e832b20400            CALL 0x0058c370
0054113e  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541141  81c100010000          ADD ECX,0x100
00541147  e814fffdff            CALL 0x00521060
0054114c  33c9                  XOR ECX,ECX
0054114e  3945f4                CMP dword ptr [EBP + -0xc],EAX
00541151  0f94c1                SETZ CL
00541154  51                    PUSH ECX
00541155  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00541158  52                    PUSH EDX
00541159  8b4588                MOV EAX,dword ptr [EBP + -0x78]
0054115c  8b10                  MOV EDX,dword ptr [EAX]
0054115e  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541161  8b4274                MOV EAX,dword ptr [EDX + 0x74]
00541164  ffd0                  CALL EAX
00541166  83f801                CMP EAX,0x1
00541169  7554                  JNZ 0x005411bf
0054116b  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0054116e  51                    PUSH ECX
0054116f  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541172  81c100010000          ADD ECX,0x100
00541178  e863cc1000            CALL 0x0064dde0
0054117d  8945f4                MOV dword ptr [EBP + -0xc],EAX
00541180  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00541183  8955e4                MOV dword ptr [EBP + -0x1c],EDX
00541186  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
00541189  8945e8                MOV dword ptr [EBP + -0x18],EAX
0054118c  837de800              CMP dword ptr [EBP + -0x18],0x0
00541190  7426                  JZ 0x005411b8
00541192  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00541195  e8a6d5ffff            CALL 0x0053e740
0054119a  b901000000            MOV ECX,0x1
0054119f  83e101                AND ECX,0x1
005411a2  740c                  JZ 0x005411b0
005411a4  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005411a7  52                    PUSH EDX
005411a8  e850853d00            CALL 0x009196fd
005411ad  83c404                ADD ESP,0x4
005411b0  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005411b3  894584                MOV dword ptr [EBP + -0x7c],EAX
005411b6  eb07                  JMP 0x005411bf
005411b8  c7458400000000        MOV dword ptr [EBP + -0x7c],0x0
005411bf  e943ffffff            JMP 0x00541107
005411c4  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005411c7  8b11                  MOV EDX,dword ptr [ECX]
005411c9  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005411cc  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005411cf  ffd0                  CALL EAX
005411d1  85c0                  TEST EAX,EAX
005411d3  0f84ac010000          JZ 0x00541385
005411d9  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005411dc  8b11                  MOV EDX,dword ptr [ECX]
005411de  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005411e1  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005411e4  ffd0                  CALL EAX
005411e6  8945d8                MOV dword ptr [EBP + -0x28],EAX
005411e9  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
005411ec  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
005411f2  8955d4                MOV dword ptr [EBP + -0x2c],EDX
005411f5  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
005411f8  e8938a1b00            CALL 0x006f9c90
005411fd  8b4588                MOV EAX,dword ptr [EBP + -0x78]
00541200  8b10                  MOV EDX,dword ptr [EAX]
00541202  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541205  8b4230                MOV EAX,dword ptr [EDX + 0x30]
00541208  ffd0                  CALL EAX
0054120a  8945d0                MOV dword ptr [EBP + -0x30],EAX
0054120d  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00541210  c781b011000001000000  MOV dword ptr [ECX + 0x11b0],0x1
0054121a  8b5588                MOV EDX,dword ptr [EBP + -0x78]
0054121d  8b02                  MOV EAX,dword ptr [EDX]
0054121f  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541222  8b5030                MOV EDX,dword ptr [EAX + 0x30]
00541225  ffd2                  CALL EDX
00541227  8945cc                MOV dword ptr [EBP + -0x34],EAX
0054122a  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0054122d  c7804c0500000000007f  MOV dword ptr [EAX + 0x54c],0x7f000000
00541237  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
0054123a  8b11                  MOV EDX,dword ptr [ECX]
0054123c  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
0054123f  8b4230                MOV EAX,dword ptr [EDX + 0x30]
00541242  ffd0                  CALL EAX
00541244  8945c8                MOV dword ptr [EBP + -0x38],EAX
00541247  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
0054124a  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
00541250  8955c4                MOV dword ptr [EBP + -0x3c],EDX
00541253  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
00541256  c780540200000000007f  MOV dword ptr [EAX + 0x254],0x7f000000
00541260  b90000007f            MOV ECX,0x7f000000
00541265  81f90000007f          CMP ECX,0x7f000000
0054126b  7529                  JNZ 0x00541296
0054126d  d9ee                  FLDZ
0054126f  d95db8                FSTP float ptr [EBP + -0x48]
00541272  d9ee                  FLDZ
00541274  d95dbc                FSTP float ptr [EBP + -0x44]
00541277  d9ee                  FLDZ
00541279  d95dc0                FSTP float ptr [EBP + -0x40]
0054127c  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0054127f  81c258020000          ADD EDX,0x258
00541285  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
00541288  8902                  MOV dword ptr [EDX],EAX
0054128a  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0054128d  894a04                MOV dword ptr [EDX + 0x4],ECX
00541290  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
00541293  894208                MOV dword ptr [EDX + 0x8],EAX
00541296  837d0800              CMP dword ptr [EBP + 0x8],0x0
0054129a  7424                  JZ 0x005412c0
0054129c  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
0054129f  8b11                  MOV EDX,dword ptr [ECX]
005412a1  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005412a4  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005412a7  ffd0                  CALL EAX
005412a9  8945b4                MOV dword ptr [EBP + -0x4c],EAX
005412ac  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
005412af  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
005412b5  8955b0                MOV dword ptr [EBP + -0x50],EDX
005412b8  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
005412bb  e830f60400            CALL 0x005908f0
005412c0  8b4588                MOV EAX,dword ptr [EBP + -0x78]
005412c3  8b10                  MOV EDX,dword ptr [EAX]
005412c5  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005412c8  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005412cb  ffd0                  CALL EAX
005412cd  8945ac                MOV dword ptr [EBP + -0x54],EAX
005412d0  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
005412d3  83c118                ADD ECX,0x18
005412d6  e8352d1f00            CALL 0x00734010
005412db  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005412de  8b11                  MOV EDX,dword ptr [ECX]
005412e0  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005412e3  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005412e6  ffd0                  CALL EAX
005412e8  8945a8                MOV dword ptr [EBP + -0x58],EAX
005412eb  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005412ee  8b91ac110000          MOV EDX,dword ptr [ECX + 0x11ac]
005412f4  8955a4                MOV dword ptr [EBP + -0x5c],EDX
005412f7  837da400              CMP dword ptr [EBP + -0x5c],0x0
005412fb  0f8484000000          JZ 0x00541385
00541301  8b4588                MOV EAX,dword ptr [EBP + -0x78]
00541304  8b10                  MOV EDX,dword ptr [EAX]
00541306  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00541309  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0054130c  ffd0                  CALL EAX
0054130e  8945a0                MOV dword ptr [EBP + -0x60],EAX
00541311  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
00541314  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00541317  89559c                MOV dword ptr [EBP + -0x64],EDX
0054131a  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0054131f  8b4804                MOV ECX,dword ptr [EAX + 0x4]
00541322  894d98                MOV dword ptr [EBP + -0x68],ECX
00541325  8b559c                MOV EDX,dword ptr [EBP + -0x64]
00541328  52                    PUSH EDX
00541329  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0054132c  e88feb1f00            CALL 0x0073fec0
00541331  8945f0                MOV dword ptr [EBP + -0x10],EAX
00541334  a1a4b4a100            MOV EAX,[0x00a1b4a4]
00541339  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0054133c  894d94                MOV dword ptr [EBP + -0x6c],ECX
0054133f  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00541342  52                    PUSH EDX
00541343  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
00541346  e845e81f00            CALL 0x0073fb90
0054134b  8bc8                  MOV ECX,EAX
0054134d  e88e582a00            CALL 0x007e6be0
00541352  8945ec                MOV dword ptr [EBP + -0x14],EAX
00541355  837decff              CMP dword ptr [EBP + -0x14],-0x1
00541359  742a                  JZ 0x00541385
0054135b  a1a4b4a100            MOV EAX,[0x00a1b4a4]
00541360  8b4804                MOV ECX,dword ptr [EAX + 0x4]
00541363  894d90                MOV dword ptr [EBP + -0x70],ECX
00541366  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
00541369  e822e81f00            CALL 0x0073fb90
0054136e  89458c                MOV dword ptr [EBP + -0x74],EAX
00541371  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00541374  69d288000000          IMUL EDX,EDX,0x88
0054137a  8b458c                MOV EAX,dword ptr [EBP + -0x74]
0054137d  c7441028ffffffff      MOV dword ptr [EAX + EDX*0x1 + 0x28],0xffffffff
00541385  8be5                  MOV ESP,EBP
00541387  5d                    POP EBP
00541388  c20400                RET 0x4
