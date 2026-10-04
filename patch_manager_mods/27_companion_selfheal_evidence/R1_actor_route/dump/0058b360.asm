; FN 0058b360 FUN_0058b360
0058b360  55                    PUSH EBP
0058b361  8bec                  MOV EBP,ESP
0058b363  6aff                  PUSH -0x1
0058b365  684b179500            PUSH 0x95174b
0058b36a  64a100000000          MOV EAX,FS:[0x0]
0058b370  50                    PUSH EAX
0058b371  83ec68                SUB ESP,0x68
0058b374  a1201fa100            MOV EAX,[0x00a11f20]
0058b379  33c5                  XOR EAX,EBP
0058b37b  50                    PUSH EAX
0058b37c  8d45f4                LEA EAX,[EBP + -0xc]
0058b37f  64a300000000          MOV FS:[0x0],EAX
0058b385  894d90                MOV dword ptr [EBP + -0x70],ECX
0058b388  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0058b38d  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0058b390  894dc4                MOV dword ptr [EBP + -0x3c],ECX
0058b393  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0058b396  52                    PUSH EDX
0058b397  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0058b39a  e8410df9ff            CALL 0x0051c0e0
0058b39f  8945ec                MOV dword ptr [EBP + -0x14],EAX
0058b3a2  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
0058b3a9  eb09                  JMP 0x0058b3b4
0058b3ab  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0058b3ae  83c001                ADD EAX,0x1
0058b3b1  8945f0                MOV dword ptr [EBP + -0x10],EAX
0058b3b4  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0058b3b7  8b9188020000          MOV EDX,dword ptr [ECX + 0x288]
0058b3bd  8955c0                MOV dword ptr [EBP + -0x40],EDX
0058b3c0  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0058b3c3  3b45c0                CMP EAX,dword ptr [EBP + -0x40]
0058b3c6  0f8dab000000          JGE 0x0058b477
0058b3cc  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0058b3cf  51                    PUSH ECX
0058b3d0  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0058b3d3  e8888e0700            CALL 0x00604260
0058b3d8  8945e8                MOV dword ptr [EBP + -0x18],EAX
0058b3db  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0058b3de  668b02                MOV AX,word ptr [EDX]
0058b3e1  668945be              MOV word ptr [EBP + -0x42],AX
0058b3e5  0fb74dbe              MOVZX ECX,word ptr [EBP + -0x42]
0058b3e9  83f90a                CMP ECX,0xa
0058b3ec  0f8580000000          JNZ 0x0058b472
0058b3f2  8b5590                MOV EDX,dword ptr [EBP + -0x70]
0058b3f5  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0058b3f8  8945b8                MOV dword ptr [EBP + -0x48],EAX
0058b3fb  d9ee                  FLDZ
0058b3fd  d95dd0                FSTP float ptr [EBP + -0x30]
0058b400  d9ee                  FLDZ
0058b402  d95dd4                FSTP float ptr [EBP + -0x2c]
0058b405  d9ee                  FLDZ
0058b407  d95dd8                FSTP float ptr [EBP + -0x28]
0058b40a  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0058b40d  668b5102              MOV DX,word ptr [ECX + 0x2]
0058b411  668955b6              MOV word ptr [EBP + -0x4a],DX
0058b415  6a00                  PUSH 0x0
0058b417  6a00                  PUSH 0x0
0058b419  6a00                  PUSH 0x0
0058b41b  6a00                  PUSH 0x0
0058b41d  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0058b420  50                    PUSH EAX
0058b421  6a00                  PUSH 0x0
0058b423  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
0058b426  51                    PUSH ECX
0058b427  8d55d0                LEA EDX,[EBP + -0x30]
0058b42a  52                    PUSH EDX
0058b42b  0fb745b6              MOVZX EAX,word ptr [EBP + -0x4a]
0058b42f  50                    PUSH EAX
0058b430  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0058b433  e868cefbff            CALL 0x005482a0
0058b438  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0058b43b  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0058b43e  8955b0                MOV dword ptr [EBP + -0x50],EDX
0058b441  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
0058b444  50                    PUSH EAX
0058b445  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0058b448  51                    PUSH ECX
0058b449  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0058b44c  e84fbc0700            CALL 0x006070a0
0058b451  8b5590                MOV EDX,dword ptr [EBP + -0x70]
0058b454  8b8220050000          MOV EAX,dword ptr [EDX + 0x520]
0058b45a  8945ac                MOV dword ptr [EBP + -0x54],EAX
0058b45d  837dac00              CMP dword ptr [EBP + -0x54],0x0
0058b461  740d                  JZ 0x0058b470
0058b463  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0058b466  c781d8110000b80b0000  MOV dword ptr [ECX + 0x11d8],0xbb8
0058b470  eb05                  JMP 0x0058b477
0058b472  e934ffffff            JMP 0x0058b3ab
0058b477  8b5590                MOV EDX,dword ptr [EBP + -0x70]
0058b47a  8b82ac110000          MOV EAX,dword ptr [EDX + 0x11ac]
0058b480  8945a8                MOV dword ptr [EBP + -0x58],EAX
0058b483  837da800              CMP dword ptr [EBP + -0x58],0x0
0058b487  0f84fe000000          JZ 0x0058b58b
0058b48d  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0058b493  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0058b496  8955a4                MOV dword ptr [EBP + -0x5c],EDX
0058b499  8b4da4                MOV ECX,dword ptr [EBP + -0x5c]
0058b49c  e82f14f9ff            CALL 0x0051c8d0
0058b4a1  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0058b4a4  837de400              CMP dword ptr [EBP + -0x1c],0x0
0058b4a8  0f84dd000000          JZ 0x0058b58b
0058b4ae  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0058b4b1  8b8850020000          MOV ECX,dword ptr [EAX + 0x250]
0058b4b7  894da0                MOV dword ptr [EBP + -0x60],ECX
0058b4ba  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
0058b4bd  83e202                AND EDX,0x2
0058b4c0  0f84c5000000          JZ 0x0058b58b
0058b4c6  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0058b4c9  c7804c02000002000000  MOV dword ptr [EAX + 0x24c],0x2
0058b4d3  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0058b4d6  e8e59efbff            CALL 0x005453c0
0058b4db  8945e0                MOV dword ptr [EBP + -0x20],EAX
0058b4de  837de000              CMP dword ptr [EBP + -0x20],0x0
0058b4e2  0f84a3000000          JZ 0x0058b58b
0058b4e8  6a34                  PUSH 0x34
0058b4ea  e834e23800            CALL 0x00919723
0058b4ef  83c404                ADD ESP,0x4
0058b4f2  8945c8                MOV dword ptr [EBP + -0x38],EAX
0058b4f5  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0058b4fc  837dc800              CMP dword ptr [EBP + -0x38],0x0
0058b500  740d                  JZ 0x0058b50f
0058b502  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
0058b505  e856650500            CALL 0x005e1a60
0058b50a  89458c                MOV dword ptr [EBP + -0x74],EAX
0058b50d  eb07                  JMP 0x0058b516
0058b50f  c7458c00000000        MOV dword ptr [EBP + -0x74],0x0
0058b516  8b4d8c                MOV ECX,dword ptr [EBP + -0x74]
0058b519  894dcc                MOV dword ptr [EBP + -0x34],ECX
0058b51c  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0058b523  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
0058b526  8955dc                MOV dword ptr [EBP + -0x24],EDX
0058b529  b80b000000            MOV EAX,0xb
0058b52e  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0058b531  668901                MOV word ptr [ECX],AX
0058b534  68a10f0000            PUSH 0xfa1
0058b539  6a00                  PUSH 0x0
0058b53b  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0058b53e  e88d660500            CALL 0x005e1bd0
0058b543  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0058b546  8b8224010000          MOV EAX,dword ptr [EDX + 0x124]
0058b54c  89459c                MOV dword ptr [EBP + -0x64],EAX
0058b54f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0058b552  8b9124010000          MOV EDX,dword ptr [ECX + 0x124]
0058b558  895598                MOV dword ptr [EBP + -0x68],EDX
0058b55b  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0058b560  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0058b563  894d94                MOV dword ptr [EBP + -0x6c],ECX
0058b566  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0058b569  52                    PUSH EDX
0058b56a  a11c249900            MOV EAX,[0x0099241c]
0058b56f  50                    PUSH EAX
0058b570  8b4d9c                MOV ECX,dword ptr [EBP + -0x64]
0058b573  51                    PUSH ECX
0058b574  8b5598                MOV EDX,dword ptr [EBP + -0x68]
0058b577  52                    PUSH EDX
0058b578  6a00                  PUSH 0x0
0058b57a  6a00                  PUSH 0x0
0058b57c  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
0058b57f  e8fc0cf9ff            CALL 0x0051c280
0058b584  8bc8                  MOV ECX,EAX
0058b586  e8e52bf9ff            CALL 0x0051e170
0058b58b  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0058b58e  64890d00000000        MOV dword ptr FS:[0x0],ECX
0058b595  59                    POP ECX
0058b596  8be5                  MOV ESP,EBP
0058b598  5d                    POP EBP
0058b599  c20400                RET 0x4
