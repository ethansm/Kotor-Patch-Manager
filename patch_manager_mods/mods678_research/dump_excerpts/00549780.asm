; FN 00549780 FUN_00549780
00549780  55                    PUSH EBP
00549781  8bec                  MOV EBP,ESP
00549783  6aff                  PUSH -0x1
00549785  68b9fb9400            PUSH 0x94fbb9
0054978a  64a100000000          MOV EAX,FS:[0x0]
00549790  50                    PUSH EAX
00549791  81ec14010000          SUB ESP,0x114
00549797  a1201fa100            MOV EAX,[0x00a11f20]
0054979c  33c5                  XOR EAX,EBP
0054979e  50                    PUSH EAX
0054979f  8d45f4                LEA EAX,[EBP + -0xc]
005497a2  64a300000000          MOV FS:[0x0],EAX
005497a8  898df0feffff          MOV dword ptr [EBP + 0xfffffef0],ECX
005497ae  0fb60574449900        MOVZX EAX,byte ptr [0x00994474]
005497b5  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
005497bb  0fb65108              MOVZX EDX,byte ptr [ECX + 0x8]
005497bf  3bd0                  CMP EDX,EAX
005497c1  741f                  JZ 0x005497e2
005497c3  0fb60578449900        MOVZX EAX,byte ptr [0x00994478]
005497ca  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
005497d0  0fb65108              MOVZX EDX,byte ptr [ECX + 0x8]
005497d4  3bd0                  CMP EDX,EAX
005497d6  740a                  JZ 0x005497e2
005497d8  b80000007f            MOV EAX,0x7f000000
005497dd  e9d4060000            JMP 0x00549eb6
005497e2  8b85f0feffff          MOV EAX,dword ptr [EBP + 0xfffffef0]
005497e8  8b10                  MOV EDX,dword ptr [EAX]
005497ea  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
005497f0  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005497f3  ffd0                  CALL EAX
005497f5  8945e0                MOV dword ptr [EBP + -0x20],EAX
005497f8  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
005497fe  8b11                  MOV EDX,dword ptr [ECX]
00549800  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
00549806  8b4248                MOV EAX,dword ptr [EDX + 0x48]
00549809  ffd0                  CALL EAX
0054980b  8945c4                MOV dword ptr [EBP + -0x3c],EAX
0054980e  837de000              CMP dword ptr [EBP + -0x20],0x0
00549812  7510                  JNZ 0x00549824
00549814  837dc400              CMP dword ptr [EBP + -0x3c],0x0
00549818  750a                  JNZ 0x00549824
0054981a  b80000007f            MOV EAX,0x7f000000
0054981f  e992060000            JMP 0x00549eb6
00549824  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
0054982a  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0054982d  899554ffffff          MOV dword ptr [EBP + 0xffffff54],EDX
00549833  a1a4b4a100            MOV EAX,[0x00a1b4a4]
00549838  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0054983b  898d50ffffff          MOV dword ptr [EBP + 0xffffff50],ECX
00549841  8b9554ffffff          MOV EDX,dword ptr [EBP + 0xffffff54]
00549847  52                    PUSH EDX
00549848  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0054984e  e88d2afdff            CALL 0x0051c2e0
00549853  8bc8                  MOV ECX,EAX
00549855  e8461e1000            CALL 0x0064b6a0
0054985a  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
00549860  e85bbbffff            CALL 0x005453c0
00549865  85c0                  TEST EAX,EAX
00549867  750a                  JNZ 0x00549873
00549869  b80000007f            MOV EAX,0x7f000000
0054986e  e943060000            JMP 0x00549eb6
00549873  c745d800000000        MOV dword ptr [EBP + -0x28],0x0
0054987a  837de000              CMP dword ptr [EBP + -0x20],0x0
0054987e  0f8446010000          JZ 0x005499ca
00549884  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00549887  e8c4f40300            CALL 0x00588d50
0054988c  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0054988f  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
00549892  8a88ee100000          MOV CL,byte ptr [EAX + 0x10ee]
00549898  888d4fffffff          MOV byte ptr [EBP + 0xffffff4f],CL
0054989e  0fb6954fffffff        MOVZX EDX,byte ptr [EBP + 0xffffff4f]
005498a5  8955c0                MOV dword ptr [EBP + -0x40],EDX
005498a8  837dc000              CMP dword ptr [EBP + -0x40],0x0
005498ac  757a                  JNZ 0x00549928
005498ae  68c0469900            PUSH 0x9946c0
005498b3  8d4db0                LEA ECX,[EBP + -0x50]
005498b6  e8b59c1e00            CALL 0x00733570
005498bb  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005498c2  a1d0b4a100            MOV EAX,[0x00a1b4d0]
005498c7  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
005498cd  898d48ffffff          MOV dword ptr [EBP + 0xffffff48],ECX
005498d3  8b9548ffffff          MOV EDX,dword ptr [EBP + 0xffffff48]
005498d9  8b4210                MOV EAX,dword ptr [EDX + 0x10]
005498dc  898544ffffff          MOV dword ptr [EBP + 0xffffff44],EAX
005498e2  8d4dc0                LEA ECX,[EBP + -0x40]
005498e5  51                    PUSH ECX
005498e6  8d55b0                LEA EDX,[EBP + -0x50]
005498e9  52                    PUSH EDX
005498ea  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
005498ed  0fb78884110000        MOVZX ECX,word ptr [EAX + 0x1184]
005498f4  51                    PUSH ECX
005498f5  8b8d44ffffff          MOV ECX,dword ptr [EBP + 0xffffff44]
005498fb  e8f0231d00            CALL 0x0071bcf0
00549900  f7d8                  NEG EAX
00549902  1bc0                  SBB EAX,EAX
00549904  83c001                ADD EAX,0x1
00549907  8845bb                MOV byte ptr [EBP + -0x45],AL
0054990a  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00549911  8d4db0                LEA ECX,[EBP + -0x50]
00549914  e8679e1e00            CALL 0x00733780
00549919  0fb655bb              MOVZX EDX,byte ptr [EBP + -0x45]
0054991d  85d2                  TEST EDX,EDX
0054991f  7407                  JZ 0x00549928
00549921  c745c000000000        MOV dword ptr [EBP + -0x40],0x0
00549928  68b8469900            PUSH 0x9946b8
0054992d  8d4da4                LEA ECX,[EBP + -0x5c]
00549930  e83b9c1e00            CALL 0x00733570
00549935  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
0054993c  a1d0b4a100            MOV EAX,[0x00a1b4d0]
00549941  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
00549947  898d40ffffff          MOV dword ptr [EBP + 0xffffff40],ECX
0054994d  8b9540ffffff          MOV EDX,dword ptr [EBP + 0xffffff40]
00549953  8b82d0000000          MOV EAX,dword ptr [EDX + 0xd0]
00549959  89853cffffff          MOV dword ptr [EBP + 0xffffff3c],EAX
0054995f  8d4dbc                LEA ECX,[EBP + -0x44]
00549962  51                    PUSH ECX
00549963  8d55a4                LEA EDX,[EBP + -0x5c]
00549966  52                    PUSH EDX
00549967  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
0054996a  50                    PUSH EAX
0054996b  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
00549971  e87a231d00            CALL 0x0071bcf0
00549976  8945ac                MOV dword ptr [EBP + -0x54],EAX
00549979  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00549980  8d4da4                LEA ECX,[EBP + -0x5c]
00549983  e8f89d1e00            CALL 0x00733780
00549988  837dac00              CMP dword ptr [EBP + -0x54],0x0
0054998c  740c                  JZ 0x0054999a
0054998e  33c9                  XOR ECX,ECX
00549990  837dbc00              CMP dword ptr [EBP + -0x44],0x0
00549994  0f95c1                SETNZ CL
00549997  894dd8                MOV dword ptr [EBP + -0x28],ECX
0054999a  837dd800              CMP dword ptr [EBP + -0x28],0x0
0054999e  7528                  JNZ 0x005499c8
005499a0  6a00                  PUSH 0x0
005499a2  6a00                  PUSH 0x0
005499a4  6a01                  PUSH 0x1
005499a6  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005499a9  e8424a0300            CALL 0x0057e3f0
005499ae  85c0                  TEST EAX,EAX
005499b0  7516                  JNZ 0x005499c8
005499b2  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005499b5  e826790300            CALL 0x005812e0
005499ba  85c0                  TEST EAX,EAX
005499bc  750a                  JNZ 0x005499c8
005499be  b80000007f            MOV EAX,0x7f000000
005499c3  e9ee040000            JMP 0x00549eb6
005499c8  eb29                  JMP 0x005499f3
005499ca  837dc400              CMP dword ptr [EBP + -0x3c],0x0
005499ce  7423                  JZ 0x005499f3
005499d0  6a00                  PUSH 0x0
005499d2  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005499d5  e8d6cd0a00            CALL 0x005f67b0
005499da  85c0                  TEST EAX,EAX
005499dc  750a                  JNZ 0x005499e8
005499de  b80000007f            MOV EAX,0x7f000000
005499e3  e9ce040000            JMP 0x00549eb6
005499e8  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005499eb  e840cc0a00            CALL 0x005f6630
005499f0  8945e4                MOV dword ptr [EBP + -0x1c],EAX
005499f3  68b0040000            PUSH 0x4b0
005499f8  e826fd3c00            CALL 0x00919723
005499fd  83c404                ADD ESP,0x4
00549a00  89459c                MOV dword ptr [EBP + -0x64],EAX
00549a03  c745fc02000000        MOV dword ptr [EBP + -0x4],0x2
00549a0a  837d9c00              CMP dword ptr [EBP + -0x64],0x0
00549a0e  7415                  JZ 0x00549a25
00549a10  680000007f            PUSH 0x7f000000
00549a15  8b4d9c                MOV ECX,dword ptr [EBP + -0x64]
00549a18  e823530a00            CALL 0x005eed40
00549a1d  8985ecfeffff          MOV dword ptr [EBP + 0xfffffeec],EAX
00549a23  eb0a                  JMP 0x00549a2f
00549a25  c785ecfeffff00000000  MOV dword ptr [EBP + 0xfffffeec],0x0
00549a2f  8b95ecfeffff          MOV EDX,dword ptr [EBP + 0xfffffeec]
00549a35  8955a0                MOV dword ptr [EBP + -0x60],EDX
00549a38  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00549a3f  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
00549a42  8945c8                MOV dword ptr [EBP + -0x38],EAX
00549a45  837dc800              CMP dword ptr [EBP + -0x38],0x0
00549a49  750a                  JNZ 0x00549a55
00549a4b  b80000007f            MOV EAX,0x7f000000
00549a50  e961040000            JMP 0x00549eb6
00549a55  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
00549a58  51                    PUSH ECX
00549a59  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549a5c  e8af740a00            CALL 0x005f0f10
00549a61  85c0                  TEST EAX,EAX
00549a63  753c                  JNZ 0x00549aa1
00549a65  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
00549a68  895594                MOV dword ptr [EBP + -0x6c],EDX
00549a6b  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
00549a6e  894598                MOV dword ptr [EBP + -0x68],EAX
00549a71  837d9800              CMP dword ptr [EBP + -0x68],0x0
00549a75  7416                  JZ 0x00549a8d
00549a77  6a01                  PUSH 0x1
00549a79  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00549a7c  8b11                  MOV EDX,dword ptr [ECX]
00549a7e  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00549a81  8b02                  MOV EAX,dword ptr [EDX]
00549a83  ffd0                  CALL EAX
00549a85  8985e8feffff          MOV dword ptr [EBP + 0xfffffee8],EAX
00549a8b  eb0a                  JMP 0x00549a97
00549a8d  c785e8feffff00000000  MOV dword ptr [EBP + 0xfffffee8],0x0
00549a97  b80000007f            MOV EAX,0x7f000000
00549a9c  e915040000            JMP 0x00549eb6
00549aa1  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
00549aa7  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00549aaa  899538ffffff          MOV dword ptr [EBP + 0xffffff38],EDX
00549ab0  6a01                  PUSH 0x1
00549ab2  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
00549ab8  50                    PUSH EAX
00549ab9  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549abc  e89fb60a00            CALL 0x005f5160
00549ac1  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549ac4  c7819804000001000000  MOV dword ptr [ECX + 0x498],0x1
00549ace  837de000              CMP dword ptr [EBP + -0x20],0x0
00549ad2  742b                  JZ 0x00549aff
00549ad4  8b95f0feffff          MOV EDX,dword ptr [EBP + 0xfffffef0]
00549ada  8b4204                MOV EAX,dword ptr [EDX + 0x4]
00549add  898534ffffff          MOV dword ptr [EBP + 0xffffff34],EAX
00549ae3  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549ae6  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
00549aec  8991a0040000          MOV dword ptr [ECX + 0x4a0],EDX
00549af2  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
00549af5  c7809c04000000000000  MOV dword ptr [EAX + 0x49c],0x0
00549aff  6a10                  PUSH 0x10
00549b01  e81dfc3c00            CALL 0x00919723
00549b06  83c404                ADD ESP,0x4
00549b09  89458c                MOV dword ptr [EBP + -0x74],EAX
00549b0c  837d8c00              CMP dword ptr [EBP + -0x74],0x0
00549b10  7437                  JZ 0x00549b49
00549b12  8b4d8c                MOV ECX,dword ptr [EBP + -0x74]
00549b15  83c104                ADD ECX,0x4
00549b18  898d30ffffff          MOV dword ptr [EBP + 0xffffff30],ECX
00549b1e  8b9530ffffff          MOV EDX,dword ptr [EBP + 0xffffff30]
00549b24  d9ee                  FLDZ
00549b26  d91a                  FSTP float ptr [EDX]
00549b28  8b8530ffffff          MOV EAX,dword ptr [EBP + 0xffffff30]
00549b2e  d9ee                  FLDZ
00549b30  d95804                FSTP float ptr [EAX + 0x4]
00549b33  8b8d30ffffff          MOV ECX,dword ptr [EBP + 0xffffff30]
00549b39  d9ee                  FLDZ
00549b3b  d95908                FSTP float ptr [ECX + 0x8]
00549b3e  8b558c                MOV EDX,dword ptr [EBP + -0x74]
00549b41  8995e4feffff          MOV dword ptr [EBP + 0xfffffee4],EDX
00549b47  eb0a                  JMP 0x00549b53
00549b49  c785e4feffff00000000  MOV dword ptr [EBP + 0xfffffee4],0x0
00549b53  8b85e4feffff          MOV EAX,dword ptr [EBP + 0xfffffee4]
00549b59  894590                MOV dword ptr [EBP + -0x70],EAX
00549b5c  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
00549b5f  894ddc                MOV dword ptr [EBP + -0x24],ECX
00549b62  837ddc00              CMP dword ptr [EBP + -0x24],0x0
00549b66  753c                  JNZ 0x00549ba4
00549b68  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
00549b6b  895584                MOV dword ptr [EBP + -0x7c],EDX
00549b6e  8b4584                MOV EAX,dword ptr [EBP + -0x7c]
00549b71  894588                MOV dword ptr [EBP + -0x78],EAX
00549b74  837d8800              CMP dword ptr [EBP + -0x78],0x0
00549b78  7416                  JZ 0x00549b90
00549b7a  6a01                  PUSH 0x1
00549b7c  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00549b7f  8b11                  MOV EDX,dword ptr [ECX]
00549b81  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00549b84  8b02                  MOV EAX,dword ptr [EDX]
00549b86  ffd0                  CALL EAX
00549b88  8985e0feffff          MOV dword ptr [EBP + 0xfffffee0],EAX
00549b8e  eb0a                  JMP 0x00549b9a
00549b90  c785e0feffff00000000  MOV dword ptr [EBP + 0xfffffee0],0x0
00549b9a  b80000007f            MOV EAX,0x7f000000
00549b9f  e912030000            JMP 0x00549eb6
00549ba4  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549ba7  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00549baa  89952cffffff          MOV dword ptr [EBP + 0xffffff2c],EDX
00549bb0  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
00549bb3  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
00549bb9  8908                  MOV dword ptr [EAX],ECX
00549bbb  8b95f0feffff          MOV EDX,dword ptr [EBP + 0xfffffef0]
00549bc1  81c294000000          ADD EDX,0x94
00549bc7  8b02                  MOV EAX,dword ptr [EDX]
00549bc9  8945e8                MOV dword ptr [EBP + -0x18],EAX
00549bcc  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
00549bcf  894dec                MOV dword ptr [EBP + -0x14],ECX
00549bd2  8b5208                MOV EDX,dword ptr [EDX + 0x8]
00549bd5  8955f0                MOV dword ptr [EBP + -0x10],EDX
00549bd8  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
00549bdb  83c004                ADD EAX,0x4
00549bde  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00549be1  8908                  MOV dword ptr [EAX],ECX
00549be3  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00549be6  895004                MOV dword ptr [EAX + 0x4],EDX
00549be9  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00549bec  894808                MOV dword ptr [EAX + 0x8],ECX
00549bef  8d4dd0                LEA ECX,[EBP + -0x30]
00549bf2  e849b61e00            CALL 0x00735240
00549bf7  c745fc03000000        MOV dword ptr [EBP + -0x4],0x3
00549bfe  837de000              CMP dword ptr [EBP + -0x20],0x0
00549c02  0f84a6000000          JZ 0x00549cae
00549c08  683c359900            PUSH 0x99353c
00549c0d  8d8d78ffffff          LEA ECX,[EBP + 0xffffff78]
00549c13  e858991e00            CALL 0x00733570
00549c18  c645fc04              MOV byte ptr [EBP + -0x4],0x4
00549c1c  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
00549c1f  8a82ee100000          MOV AL,byte ptr [EDX + 0x10ee]
00549c25  88852bffffff          MOV byte ptr [EBP + 0xffffff2b],AL
00549c2b  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
00549c31  8b9130010000          MOV EDX,dword ptr [ECX + 0x130]
00549c37  899524ffffff          MOV dword ptr [EBP + 0xffffff24],EDX
00549c3d  8b8524ffffff          MOV EAX,dword ptr [EBP + 0xffffff24]
00549c43  8b88d0000000          MOV ECX,dword ptr [EAX + 0xd0]
00549c49  898d20ffffff          MOV dword ptr [EBP + 0xffffff20],ECX
00549c4f  8d55cc                LEA EDX,[EBP + -0x34]
00549c52  52                    PUSH EDX
00549c53  8d8578ffffff          LEA EAX,[EBP + 0xffffff78]
00549c59  50                    PUSH EAX
00549c5a  0fb68d2bffffff        MOVZX ECX,byte ptr [EBP + 0xffffff2b]
00549c61  51                    PUSH ECX
00549c62  8b8d20ffffff          MOV ECX,dword ptr [EBP + 0xffffff20]
00549c68  e883201d00            CALL 0x0071bcf0
00549c6d  894580                MOV dword ptr [EBP + -0x80],EAX
00549c70  c645fc03              MOV byte ptr [EBP + -0x4],0x3
00549c74  8d8d78ffffff          LEA ECX,[EBP + 0xffffff78]
00549c7a  e8019b1e00            CALL 0x00733780
00549c7f  837d8000              CMP dword ptr [EBP + -0x80],0x0
00549c83  7424                  JZ 0x00549ca9
00549c85  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
00549c88  89951cffffff          MOV dword ptr [EBP + 0xffffff1c],EDX
00549c8e  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
00549c94  8945d4                MOV dword ptr [EBP + -0x2c],EAX
00549c97  8d4dd0                LEA ECX,[EBP + -0x30]
00549c9a  51                    PUSH ECX
00549c9b  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549c9e  81c168020000          ADD ECX,0x268
00549ca4  e857b71e00            CALL 0x00735400
00549ca9  e9b1000000            JMP 0x00549d5f
00549cae  837dc400              CMP dword ptr [EBP + -0x3c],0x0
00549cb2  0f84a7000000          JZ 0x00549d5f
00549cb8  683c359900            PUSH 0x99353c
00549cbd  8d8d6cffffff          LEA ECX,[EBP + 0xffffff6c]
00549cc3  e8a8981e00            CALL 0x00733570
00549cc8  c645fc05              MOV byte ptr [EBP + -0x4],0x5
00549ccc  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
00549ccf  8a82ec030000          MOV AL,byte ptr [EDX + 0x3ec]
00549cd5  88851bffffff          MOV byte ptr [EBP + 0xffffff1b],AL
00549cdb  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
00549ce1  8b9130010000          MOV EDX,dword ptr [ECX + 0x130]
00549ce7  899514ffffff          MOV dword ptr [EBP + 0xffffff14],EDX
00549ced  8b8514ffffff          MOV EAX,dword ptr [EBP + 0xffffff14]
00549cf3  8b88d0000000          MOV ECX,dword ptr [EAX + 0xd0]
00549cf9  898d10ffffff          MOV dword ptr [EBP + 0xffffff10],ECX
00549cff  8d55cc                LEA EDX,[EBP + -0x34]
00549d02  52                    PUSH EDX
00549d03  8d856cffffff          LEA EAX,[EBP + 0xffffff6c]
00549d09  50                    PUSH EAX
00549d0a  0fb68d1bffffff        MOVZX ECX,byte ptr [EBP + 0xffffff1b]
00549d11  51                    PUSH ECX
00549d12  8b8d10ffffff          MOV ECX,dword ptr [EBP + 0xffffff10]
00549d18  e8d31f1d00            CALL 0x0071bcf0
00549d1d  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
00549d23  c645fc03              MOV byte ptr [EBP + -0x4],0x3
00549d27  8d8d6cffffff          LEA ECX,[EBP + 0xffffff6c]
00549d2d  e84e9a1e00            CALL 0x00733780
00549d32  83bd74ffffff00        CMP dword ptr [EBP + 0xffffff74],0x0
00549d39  7424                  JZ 0x00549d5f
00549d3b  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
00549d3e  89950cffffff          MOV dword ptr [EBP + 0xffffff0c],EDX
00549d44  8b850cffffff          MOV EAX,dword ptr [EBP + 0xffffff0c]
00549d4a  8945d4                MOV dword ptr [EBP + -0x2c],EAX
00549d4d  8d4dd0                LEA ECX,[EBP + -0x30]
00549d50  51                    PUSH ECX
00549d51  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549d54  81c168020000          ADD ECX,0x268
00549d5a  e8a1b61e00            CALL 0x00735400
00549d5f  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
00549d62  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
00549d65  8982a4040000          MOV dword ptr [EDX + 0x4a4],EAX
00549d6b  33c9                  XOR ECX,ECX
00549d6d  837dd800              CMP dword ptr [EBP + -0x28],0x0
00549d71  0f94c1                SETZ CL
00549d74  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
00549d77  898a8c030000          MOV dword ptr [EDX + 0x38c],ECX
00549d7d  837dd800              CMP dword ptr [EBP + -0x28],0x0
00549d81  7436                  JZ 0x00549db9
00549d83  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
00549d86  8b88c4030000          MOV ECX,dword ptr [EAX + 0x3c4]
00549d8c  898d08ffffff          MOV dword ptr [EBP + 0xffffff08],ECX
00549d92  8b9508ffffff          MOV EDX,dword ptr [EBP + 0xffffff08]
00549d98  8b4210                MOV EAX,dword ptr [EDX + 0x10]
00549d9b  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
00549da1  83bd04ffffff00        CMP dword ptr [EBP + 0xffffff04],0x0
00549da8  7602                  JBE 0x00549dac
00549daa  eb0d                  JMP 0x00549db9
00549dac  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549daf  c7817c03000000000000  MOV dword ptr [ECX + 0x37c],0x0
00549db9  33d2                  XOR EDX,EDX
00549dbb  837dd800              CMP dword ptr [EBP + -0x28],0x0
00549dbf  0f94c2                SETZ DL
00549dc2  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
00549dc5  8990fc000000          MOV dword ptr [EAX + 0xfc],EDX
00549dcb  51                    PUSH ECX
00549dcc  d9ee                  FLDZ
00549dce  d91c24                FSTP float ptr [ESP]
00549dd1  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
00549dd7  81c1a0000000          ADD ECX,0xa0
00549ddd  51                    PUSH ECX
00549dde  e8edbffcff            CALL 0x00515dd0
00549de3  d91c24                FSTP float ptr [ESP]
00549de6  8b95f0feffff          MOV EDX,dword ptr [EBP + 0xfffffef0]
00549dec  81c2a0000000          ADD EDX,0xa0
00549df2  52                    PUSH EDX
00549df3  e858bffcff            CALL 0x00515d50
00549df8  d91c24                FSTP float ptr [ESP]
00549dfb  8d855cffffff          LEA EAX,[EBP + 0xffffff5c]
00549e01  50                    PUSH EAX
00549e02  e819b8fcff            CALL 0x00515620
00549e07  83c410                ADD ESP,0x10
00549e0a  50                    PUSH EAX
00549e0b  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00549e0e  e87dcb0a00            CALL 0x005f6990
00549e13  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
00549e19  e8a2b5ffff            CALL 0x005453c0
00549e1e  0520010000            ADD EAX,0x120
00549e23  898500ffffff          MOV dword ptr [EBP + 0xffffff00],EAX
00549e29  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00549e2f  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00549e32  8995fcfeffff          MOV dword ptr [EBP + 0xfffffefc],EDX
00549e38  8b85f0feffff          MOV EAX,dword ptr [EBP + 0xfffffef0]
00549e3e  8b4804                MOV ECX,dword ptr [EAX + 0x4]
00549e41  898df8feffff          MOV dword ptr [EBP + 0xfffffef8],ECX
00549e47  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
00549e4d  8b4208                MOV EAX,dword ptr [EDX + 0x8]
00549e50  8985f4feffff          MOV dword ptr [EBP + 0xfffffef4],EAX
00549e56  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
00549e59  51                    PUSH ECX
00549e5a  8b1538249900          MOV EDX,dword ptr [0x00992438]
00549e60  52                    PUSH EDX
00549e61  8b85fcfeffff          MOV EAX,dword ptr [EBP + 0xfffffefc]
00549e67  50                    PUSH EAX
00549e68  8b8df8feffff          MOV ECX,dword ptr [EBP + 0xfffffef8]
00549e6e  51                    PUSH ECX
00549e6f  68f4010000            PUSH 0x1f4
00549e74  6a00                  PUSH 0x0
00549e76  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
00549e7c  e8ff23fdff            CALL 0x0051c280
00549e81  8bc8                  MOV ECX,EAX
00549e83  e8e842fdff            CALL 0x0051e170
00549e88  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
00549e8b  c7824402000001000000  MOV dword ptr [EDX + 0x244],0x1
00549e95  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
00549e98  8b4804                MOV ECX,dword ptr [EAX + 0x4]
00549e9b  898d58ffffff          MOV dword ptr [EBP + 0xffffff58],ECX
00549ea1  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00549ea8  8d4dd0                LEA ECX,[EBP + -0x30]
00549eab  e8b0b41e00            CALL 0x00735360
00549eb0  8b8558ffffff          MOV EAX,dword ptr [EBP + 0xffffff58]
00549eb6  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00549eb9  64890d00000000        MOV dword ptr FS:[0x0],ECX
00549ec0  59                    POP ECX
00549ec1  8be5                  MOV ESP,EBP
00549ec3  5d                    POP EBP
00549ec4  c3                    RET
