; FN 0065b4f0 FUN_0065b4f0
0065b4f0  55                    PUSH EBP
0065b4f1  8bec                  MOV EBP,ESP
0065b4f3  81ecbc010000          SUB ESP,0x1bc
0065b4f9  898d50feffff          MOV dword ptr [EBP + 0xfffffe50],ECX
0065b4ff  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
0065b506  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b50c  e8df79fdff            CALL 0x00632ef0
0065b511  8945d0                MOV dword ptr [EBP + -0x30],EAX
0065b514  6a20                  PUSH 0x20
0065b516  51                    PUSH ECX
0065b517  d9e8                  FLD1
0065b519  d91c24                FSTP float ptr [ESP]
0065b51c  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b522  e83986f8ff            CALL 0x005e3b60
0065b527  d95de4                FSTP float ptr [EBP + -0x1c]
0065b52a  6a20                  PUSH 0x20
0065b52c  51                    PUSH ECX
0065b52d  d9e8                  FLD1
0065b52f  d91c24                FSTP float ptr [ESP]
0065b532  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b538  e82386f8ff            CALL 0x005e3b60
0065b53d  d95de0                FSTP float ptr [EBP + -0x20]
0065b540  6a20                  PUSH 0x20
0065b542  51                    PUSH ECX
0065b543  d9e8                  FLD1
0065b545  d91c24                FSTP float ptr [ESP]
0065b548  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b54e  e80d86f8ff            CALL 0x005e3b60
0065b553  d95ddc                FSTP float ptr [EBP + -0x24]
0065b556  6a08                  PUSH 0x8
0065b558  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b55e  e8bd81f8ff            CALL 0x005e3720
0065b563  0fb6c0                MOVZX EAX,AL
0065b566  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0065b569  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b56f  e82c81f8ff            CALL 0x005e36a0
0065b574  8945e8                MOV dword ptr [EBP + -0x18],EAX
0065b577  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b57d  e81e81f8ff            CALL 0x005e36a0
0065b582  8945ec                MOV dword ptr [EBP + -0x14],EAX
0065b585  6a08                  PUSH 0x8
0065b587  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b58d  e88e81f8ff            CALL 0x005e3720
0065b592  8845fb                MOV byte ptr [EBP + -0x5],AL
0065b595  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b59b  e85079fdff            CALL 0x00632ef0
0065b5a0  8945fc                MOV dword ptr [EBP + -0x4],EAX
0065b5a3  6a01                  PUSH 0x1
0065b5a5  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b5ab  e83088f8ff            CALL 0x005e3de0
0065b5b0  85c0                  TEST EAX,EAX
0065b5b2  7511                  JNZ 0x0065b5c5
0065b5b4  6a01                  PUSH 0x1
0065b5b6  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
0065b5bc  e87f88f8ff            CALL 0x005e3e40
0065b5c1  85c0                  TEST EAX,EAX
0065b5c3  7407                  JZ 0x0065b5cc
0065b5c5  33c0                  XOR EAX,EAX
0065b5c7  e978070000            JMP 0x0065bd44
0065b5cc  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065b5cf  e84c74fdff            CALL 0x00632a20
0065b5d4  8945cc                MOV dword ptr [EBP + -0x34],EAX
0065b5d7  8b0d84449900          MOV ECX,dword ptr [0x00994484]
0065b5dd  894df0                MOV dword ptr [EBP + -0x10],ECX
0065b5e0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065b5e3  e83874fdff            CALL 0x00632a20
0065b5e8  85c0                  TEST EAX,EAX
0065b5ea  7507                  JNZ 0x0065b5f3
0065b5ec  33c0                  XOR EAX,EAX
0065b5ee  e951070000            JMP 0x0065bd44
0065b5f3  837dcc00              CMP dword ptr [EBP + -0x34],0x0
0065b5f7  740c                  JZ 0x0065b605
0065b5f9  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065b5fc  e8bf9deeff            CALL 0x005453c0
0065b601  85c0                  TEST EAX,EAX
0065b603  7507                  JNZ 0x0065b60c
0065b605  33c0                  XOR EAX,EAX
0065b607  e938070000            JMP 0x0065bd44
0065b60c  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065b60f  e80c74fdff            CALL 0x00632a20
0065b614  89854cfeffff          MOV dword ptr [EBP + 0xfffffe4c],EAX
0065b61a  8b954cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe4c]
0065b620  8b02                  MOV EAX,dword ptr [EDX]
0065b622  8b8d4cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe4c]
0065b628  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0065b62b  ffd2                  CALL EDX
0065b62d  85c0                  TEST EAX,EAX
0065b62f  0f840c020000          JZ 0x0065b841
0065b635  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065b638  e8e373fdff            CALL 0x00632a20
0065b63d  898548feffff          MOV dword ptr [EBP + 0xfffffe48],EAX
0065b643  8b8548feffff          MOV EAX,dword ptr [EBP + 0xfffffe48]
0065b649  8b10                  MOV EDX,dword ptr [EAX]
0065b64b  8b8d48feffff          MOV ECX,dword ptr [EBP + 0xfffffe48]
0065b651  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065b654  ffd0                  CALL EAX
0065b656  8985d8feffff          MOV dword ptr [EBP + 0xfffffed8],EAX
0065b65c  8b8dd8feffff          MOV ECX,dword ptr [EBP + 0xfffffed8]
0065b662  8b91f0100000          MOV EDX,dword ptr [ECX + 0x10f0]
0065b668  8995d4feffff          MOV dword ptr [EBP + 0xfffffed4],EDX
0065b66e  83bdd4feffff00        CMP dword ptr [EBP + 0xfffffed4],0x0
0065b675  7407                  JZ 0x0065b67e
0065b677  33c0                  XOR EAX,EAX
0065b679  e9c6060000            JMP 0x0065bd44
0065b67e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0065b681  e89a73fdff            CALL 0x00632a20
0065b686  898544feffff          MOV dword ptr [EBP + 0xfffffe44],EAX
0065b68c  8b8544feffff          MOV EAX,dword ptr [EBP + 0xfffffe44]
0065b692  8b10                  MOV EDX,dword ptr [EAX]
0065b694  8b8d44feffff          MOV ECX,dword ptr [EBP + 0xfffffe44]
0065b69a  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065b69d  ffd0                  CALL EAX
0065b69f  8985d0feffff          MOV dword ptr [EBP + 0xfffffed0],EAX
0065b6a5  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0065b6ab  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
0065b6b1  8955ac                MOV dword ptr [EBP + -0x54],EDX
0065b6b4  d9ee                  FLDZ
0065b6b6  d95db0                FSTP float ptr [EBP + -0x50]
0065b6b9  d9ee                  FLDZ
0065b6bb  d95db4                FSTP float ptr [EBP + -0x4c]
0065b6be  d9ee                  FLDZ
0065b6c0  d95db8                FSTP float ptr [EBP + -0x48]
0065b6c3  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
0065b6c6  8b8884000000          MOV ECX,dword ptr [EAX + 0x84]
0065b6cc  898dccfeffff          MOV dword ptr [EBP + 0xfffffecc],ECX
0065b6d2  83bdccfeffff00        CMP dword ptr [EBP + 0xfffffecc],0x0
0065b6d9  756e                  JNZ 0x0065b749
0065b6db  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
0065b6de  d94264                FLD float ptr [EDX + 0x64]
0065b6e1  d99dc8feffff          FSTP float ptr [EBP + 0xfffffec8]
0065b6e7  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
0065b6ea  d94060                FLD float ptr [EAX + 0x60]
0065b6ed  d99dc4feffff          FSTP float ptr [EBP + 0xfffffec4]
0065b6f3  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
0065b6f6  d9415c                FLD float ptr [ECX + 0x5c]
0065b6f9  d99dc0feffff          FSTP float ptr [EBP + 0xfffffec0]
0065b6ff  d985c0feffff          FLD float ptr [EBP + 0xfffffec0]
0065b705  d99d48ffffff          FSTP float ptr [EBP + 0xffffff48]
0065b70b  d985c4feffff          FLD float ptr [EBP + 0xfffffec4]
0065b711  d99d4cffffff          FSTP float ptr [EBP + 0xffffff4c]
0065b717  d985c8feffff          FLD float ptr [EBP + 0xfffffec8]
0065b71d  d99d50ffffff          FSTP float ptr [EBP + 0xffffff50]
0065b723  8b9548ffffff          MOV EDX,dword ptr [EBP + 0xffffff48]
0065b729  8955b0                MOV dword ptr [EBP + -0x50],EDX
0065b72c  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
0065b732  8945b4                MOV dword ptr [EBP + -0x4c],EAX
0065b735  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0065b73b  894db8                MOV dword ptr [EBP + -0x48],ECX
0065b73e  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
0065b741  8b4270                MOV EAX,dword ptr [EDX + 0x70]
0065b744  8945bc                MOV dword ptr [EBP + -0x44],EAX
0065b747  eb6f                  JMP 0x0065b7b8
0065b749  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
0065b74c  d9417c                FLD float ptr [ECX + 0x7c]
0065b74f  d99dbcfeffff          FSTP float ptr [EBP + 0xfffffebc]
0065b755  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
0065b758  d94278                FLD float ptr [EDX + 0x78]
0065b75b  d99db8feffff          FSTP float ptr [EBP + 0xfffffeb8]
0065b761  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
0065b764  d94074                FLD float ptr [EAX + 0x74]
0065b767  d99db4feffff          FSTP float ptr [EBP + 0xfffffeb4]
0065b76d  d985b4feffff          FLD float ptr [EBP + 0xfffffeb4]
0065b773  d99d3cffffff          FSTP float ptr [EBP + 0xffffff3c]
0065b779  d985b8feffff          FLD float ptr [EBP + 0xfffffeb8]
0065b77f  d99d40ffffff          FSTP float ptr [EBP + 0xffffff40]
0065b785  d985bcfeffff          FLD float ptr [EBP + 0xfffffebc]
0065b78b  d99d44ffffff          FSTP float ptr [EBP + 0xffffff44]
0065b791  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
0065b797  894db0                MOV dword ptr [EBP + -0x50],ECX
0065b79a  8b9540ffffff          MOV EDX,dword ptr [EBP + 0xffffff40]
0065b7a0  8955b4                MOV dword ptr [EBP + -0x4c],EDX
0065b7a3  8b8544ffffff          MOV EAX,dword ptr [EBP + 0xffffff44]
0065b7a9  8945b8                MOV dword ptr [EBP + -0x48],EAX
0065b7ac  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
0065b7af  8b9180000000          MOV EDX,dword ptr [ECX + 0x80]
0065b7b5  8955bc                MOV dword ptr [EBP + -0x44],EDX
0065b7b8  d945e4                FLD float ptr [EBP + -0x1c]
0065b7bb  d99d30ffffff          FSTP float ptr [EBP + 0xffffff30]
0065b7c1  d945e0                FLD float ptr [EBP + -0x20]
0065b7c4  d99d34ffffff          FSTP float ptr [EBP + 0xffffff34]
0065b7ca  d945dc                FLD float ptr [EBP + -0x24]
0065b7cd  d99d38ffffff          FSTP float ptr [EBP + 0xffffff38]
0065b7d3  8d45b0                LEA EAX,[EBP + -0x50]
0065b7d6  50                    PUSH EAX
0065b7d7  8d8d30ffffff          LEA ECX,[EBP + 0xffffff30]
0065b7dd  51                    PUSH ECX
0065b7de  8d55c0                LEA EDX,[EBP + -0x40]
0065b7e1  52                    PUSH EDX
0065b7e2  e8c99adeff            CALL 0x004452b0
0065b7e7  83c40c                ADD ESP,0xc
0065b7ea  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0065b7ed  3b45d0                CMP EAX,dword ptr [EBP + -0x30]
0065b7f0  7531                  JNZ 0x0065b823
0065b7f2  8d4dc0                LEA ECX,[EBP + -0x40]
0065b7f5  51                    PUSH ECX
0065b7f6  8d55c0                LEA EDX,[EBP + -0x40]
0065b7f9  52                    PUSH EDX
0065b7fa  e8a100dfff            CALL 0x0044b8a0
0065b7ff  83c408                ADD ESP,0x8
0065b802  dc1d38759800          FCOMP double ptr [0x00987538]
0065b808  dfe0                  FNSTSW AX
0065b80a  f6c405                TEST AH,0x5
0065b80d  7a14                  JP 0x0065b823
0065b80f  0fb645d4              MOVZX EAX,byte ptr [EBP + -0x2c]
0065b813  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
0065b816  894128                MOV dword ptr [ECX + 0x28],EAX
0065b819  b801000000            MOV EAX,0x1
0065b81e  e921050000            JMP 0x0065bd44
0065b823  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
0065b826  e865e40900            CALL 0x006f9c90
0065b82b  0fb655fb              MOVZX EDX,byte ptr [EBP + -0x5]
0065b82f  83fa01                CMP EDX,0x1
0065b832  750d                  JNZ 0x0065b841
0065b834  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
0065b837  c7809401000001000000  MOV dword ptr [EAX + 0x194],0x1
0065b841  d9ee                  FLDZ
0065b843  d95dd8                FSTP float ptr [EBP + -0x28]
0065b846  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065b849  8b11                  MOV EDX,dword ptr [ECX]
0065b84b  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065b84e  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065b851  ffd0                  CALL EAX
0065b853  85c0                  TEST EAX,EAX
0065b855  0f8486030000          JZ 0x0065bbe1
0065b85b  d945e4                FLD float ptr [EBP + -0x1c]
0065b85e  d95d88                FSTP float ptr [EBP + -0x78]
0065b861  d945e0                FLD float ptr [EBP + -0x20]
0065b864  d95d8c                FSTP float ptr [EBP + -0x74]
0065b867  d945dc                FLD float ptr [EBP + -0x24]
0065b86a  d95d90                FSTP float ptr [EBP + -0x70]
0065b86d  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065b870  8b11                  MOV EDX,dword ptr [ECX]
0065b872  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065b875  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065b878  ffd0                  CALL EAX
0065b87a  894598                MOV dword ptr [EBP + -0x68],EAX
0065b87d  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
0065b884  8d4d88                LEA ECX,[EBP + -0x78]
0065b887  51                    PUSH ECX
0065b888  8b5598                MOV EDX,dword ptr [EBP + -0x68]
0065b88b  81c294000000          ADD EDX,0x94
0065b891  52                    PUSH EDX
0065b892  8d45a0                LEA EAX,[EBP + -0x60]
0065b895  50                    PUSH EAX
0065b896  e8159adeff            CALL 0x004452b0
0065b89b  83c40c                ADD ESP,0xc
0065b89e  8d4da0                LEA ECX,[EBP + -0x60]
0065b8a1  51                    PUSH ECX
0065b8a2  8d55a0                LEA EDX,[EBP + -0x60]
0065b8a5  52                    PUSH EDX
0065b8a6  e8f5ffdeff            CALL 0x0044b8a0
0065b8ab  83c408                ADD ESP,0x8
0065b8ae  dc1d30649900          FCOMP double ptr [0x00996430]
0065b8b4  dfe0                  FNSTSW AX
0065b8b6  f6c405                TEST AH,0x5
0065b8b9  0f8a22030000          JP 0x0065bbe1
0065b8bf  8b4598                MOV EAX,dword ptr [EBP + -0x68]
0065b8c2  8a8816110000          MOV CL,byte ptr [EAX + 0x1116]
0065b8c8  888d9bfeffff          MOV byte ptr [EBP + 0xfffffe9b],CL
0065b8ce  0fb6959bfeffff        MOVZX EDX,byte ptr [EBP + 0xfffffe9b]
0065b8d5  83fa02                CMP EDX,0x2
0065b8d8  0f8503030000          JNZ 0x0065bbe1
0065b8de  d905e8889900          FLD float ptr [0x009988e8]
0065b8e4  d95d84                FSTP float ptr [EBP + -0x7c]
0065b8e7  d9ee                  FLDZ
0065b8e9  d99d70ffffff          FSTP float ptr [EBP + 0xffffff70]
0065b8ef  d9ee                  FLDZ
0065b8f1  d99d74ffffff          FSTP float ptr [EBP + 0xffffff74]
0065b8f7  d9ee                  FLDZ
0065b8f9  d99d78ffffff          FSTP float ptr [EBP + 0xffffff78]
0065b8ff  d9ee                  FLDZ
0065b901  d99d58ffffff          FSTP float ptr [EBP + 0xffffff58]
0065b907  d9ee                  FLDZ
0065b909  d99d5cffffff          FSTP float ptr [EBP + 0xffffff5c]
0065b90f  d9ee                  FLDZ
0065b911  d99d60ffffff          FSTP float ptr [EBP + 0xffffff60]
0065b917  d9ee                  FLDZ
0065b919  d99d64ffffff          FSTP float ptr [EBP + 0xffffff64]
0065b91f  d9ee                  FLDZ
0065b921  d99d68ffffff          FSTP float ptr [EBP + 0xffffff68]
0065b927  d9ee                  FLDZ
0065b929  d99d6cffffff          FSTP float ptr [EBP + 0xffffff6c]
0065b92f  8b4598                MOV EAX,dword ptr [EBP + -0x68]
0065b932  8b8818110000          MOV ECX,dword ptr [EAX + 0x1118]
0065b938  894d80                MOV dword ptr [EBP + -0x80],ECX
0065b93b  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0065b941  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0065b944  898594feffff          MOV dword ptr [EBP + 0xfffffe94],EAX
0065b94a  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
0065b94d  51                    PUSH ECX
0065b94e  8b8d94feffff          MOV ECX,dword ptr [EBP + 0xfffffe94]
0065b954  e84707ecff            CALL 0x0051c0a0
0065b959  89459c                MOV dword ptr [EBP + -0x64],EAX
0065b95c  837d9c00              CMP dword ptr [EBP + -0x64],0x0
0065b960  7421                  JZ 0x0065b983
0065b962  8b559c                MOV EDX,dword ptr [EBP + -0x64]
0065b965  8b02                  MOV EAX,dword ptr [EDX]
0065b967  8b4d9c                MOV ECX,dword ptr [EBP + -0x64]
0065b96a  8b5010                MOV EDX,dword ptr [EAX + 0x10]
0065b96d  ffd2                  CALL EDX
0065b96f  85c0                  TEST EAX,EAX
0065b971  7410                  JZ 0x0065b983
0065b973  8b459c                MOV EAX,dword ptr [EBP + -0x64]
0065b976  8b10                  MOV EDX,dword ptr [EAX]
0065b978  8b4d9c                MOV ECX,dword ptr [EBP + -0x64]
0065b97b  8b4210                MOV EAX,dword ptr [EDX + 0x10]
0065b97e  ffd0                  CALL EAX
0065b980  894594                MOV dword ptr [EBP + -0x6c],EAX
0065b983  837d9400              CMP dword ptr [EBP + -0x6c],0x0
0065b987  750a                  JNZ 0x0065b993
0065b989  b801000000            MOV EAX,0x1
0065b98e  e9b1030000            JMP 0x0065bd44
0065b993  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0065b996  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0065b999  899590feffff          MOV dword ptr [EBP + 0xfffffe90],EDX
0065b99f  8b4598                MOV EAX,dword ptr [EBP + -0x68]
0065b9a2  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
0065b9a8  898d8cfeffff          MOV dword ptr [EBP + 0xfffffe8c],ECX
0065b9ae  8b9590feffff          MOV EDX,dword ptr [EBP + 0xfffffe90]
0065b9b4  52                    PUSH EDX
0065b9b5  6a01                  PUSH 0x1
0065b9b7  8b8d8cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe8c]
0065b9bd  e89e42f3ff            CALL 0x0058fc60
0065b9c2  8b4598                MOV EAX,dword ptr [EBP + -0x68]
0065b9c5  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
0065b9cb  898d88feffff          MOV dword ptr [EBP + 0xfffffe88],ECX
0065b9d1  6a00                  PUSH 0x0
0065b9d3  68f4010000            PUSH 0x1f4
0065b9d8  8b8d88feffff          MOV ECX,dword ptr [EBP + 0xfffffe88]
0065b9de  e8ed42f3ff            CALL 0x0058fcd0
0065b9e3  8b5598                MOV EDX,dword ptr [EBP + -0x68]
0065b9e6  81c294000000          ADD EDX,0x94
0065b9ec  52                    PUSH EDX
0065b9ed  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
0065b9f0  0594000000            ADD EAX,0x94
0065b9f5  50                    PUSH EAX
0065b9f6  8d8d24ffffff          LEA ECX,[EBP + 0xffffff24]
0065b9fc  51                    PUSH ECX
0065b9fd  e8ae98deff            CALL 0x004452b0
0065ba02  83c40c                ADD ESP,0xc
0065ba05  50                    PUSH EAX
0065ba06  8d9518ffffff          LEA EDX,[EBP + 0xffffff18]
0065ba0c  52                    PUSH EDX
0065ba0d  e8ae89ebff            CALL 0x005143c0
0065ba12  83c408                ADD ESP,0x8
0065ba15  8b08                  MOV ECX,dword ptr [EAX]
0065ba17  898d64ffffff          MOV dword ptr [EBP + 0xffffff64],ECX
0065ba1d  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0065ba20  899568ffffff          MOV dword ptr [EBP + 0xffffff68],EDX
0065ba26  8b4008                MOV EAX,dword ptr [EAX + 0x8]
0065ba29  89856cffffff          MOV dword ptr [EBP + 0xffffff6c],EAX
0065ba2f  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0065ba32  81c194000000          ADD ECX,0x94
0065ba38  51                    PUSH ECX
0065ba39  8d5588                LEA EDX,[EBP + -0x78]
0065ba3c  52                    PUSH EDX
0065ba3d  8d850cffffff          LEA EAX,[EBP + 0xffffff0c]
0065ba43  50                    PUSH EAX
0065ba44  e86798deff            CALL 0x004452b0
0065ba49  83c40c                ADD ESP,0xc
0065ba4c  50                    PUSH EAX
0065ba4d  8d8d00ffffff          LEA ECX,[EBP + 0xffffff00]
0065ba53  51                    PUSH ECX
0065ba54  e86789ebff            CALL 0x005143c0
0065ba59  83c408                ADD ESP,0x8
0065ba5c  8b10                  MOV EDX,dword ptr [EAX]
0065ba5e  899570ffffff          MOV dword ptr [EBP + 0xffffff70],EDX
0065ba64  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0065ba67  898d74ffffff          MOV dword ptr [EBP + 0xffffff74],ECX
0065ba6d  8b5008                MOV EDX,dword ptr [EAX + 0x8]
0065ba70  899578ffffff          MOV dword ptr [EBP + 0xffffff78],EDX
0065ba76  d9856cffffff          FLD float ptr [EBP + 0xffffff6c]
0065ba7c  d99d78ffffff          FSTP float ptr [EBP + 0xffffff78]
0065ba82  8d8570ffffff          LEA EAX,[EBP + 0xffffff70]
0065ba88  50                    PUSH EAX
0065ba89  8d8d64ffffff          LEA ECX,[EBP + 0xffffff64]
0065ba8f  51                    PUSH ECX
0065ba90  e80bfedeff            CALL 0x0044b8a0
0065ba95  83c408                ADD ESP,0x8
0065ba98  d99d7cffffff          FSTP float ptr [EBP + 0xffffff7c]
0065ba9e  d9857cffffff          FLD float ptr [EBP + 0xffffff7c]
0065baa4  d94584                FLD float ptr [EBP + -0x7c]
0065baa7  ded9                  FCOMPP
0065baa9  dfe0                  FNSTSW AX
0065baab  f6c441                TEST AH,0x41
0065baae  7a18                  JP 0x0065bac8
0065bab0  8b5580                MOV EDX,dword ptr [EBP + -0x80]
0065bab3  52                    PUSH EDX
0065bab4  68f4010000            PUSH 0x1f4
0065bab9  6a02                  PUSH 0x2
0065babb  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0065babe  e87d7f0800            CALL 0x006e3a40
0065bac3  e90f010000            JMP 0x0065bbd7
0065bac8  d94584                FLD float ptr [EBP + -0x7c]
0065bacb  d9e0                  FCHS
0065bacd  d9857cffffff          FLD float ptr [EBP + 0xffffff7c]
0065bad3  ded9                  FCOMPP
0065bad5  dfe0                  FNSTSW AX
0065bad7  f6c441                TEST AH,0x41
0065bada  7a18                  JP 0x0065baf4
0065badc  8b4580                MOV EAX,dword ptr [EBP + -0x80]
0065badf  50                    PUSH EAX
0065bae0  68f4010000            PUSH 0x1f4
0065bae5  6a03                  PUSH 0x3
0065bae7  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0065baea  e8517f0800            CALL 0x006e3a40
0065baef  e9e3000000            JMP 0x0065bbd7
0065baf4  d98574ffffff          FLD float ptr [EBP + 0xffffff74]
0065bafa  d88d6cffffff          FMUL float ptr [EBP + 0xffffff6c]
0065bb00  d98578ffffff          FLD float ptr [EBP + 0xffffff78]
0065bb06  d88d68ffffff          FMUL float ptr [EBP + 0xffffff68]
0065bb0c  dee1                  FSUBRP
0065bb0e  d99d64feffff          FSTP float ptr [EBP + 0xfffffe64]
0065bb14  d98578ffffff          FLD float ptr [EBP + 0xffffff78]
0065bb1a  d88d64ffffff          FMUL float ptr [EBP + 0xffffff64]
0065bb20  d98570ffffff          FLD float ptr [EBP + 0xffffff70]
0065bb26  d88d6cffffff          FMUL float ptr [EBP + 0xffffff6c]
0065bb2c  dee1                  FSUBRP
0065bb2e  d99d68feffff          FSTP float ptr [EBP + 0xfffffe68]
0065bb34  d98570ffffff          FLD float ptr [EBP + 0xffffff70]
0065bb3a  d88d68ffffff          FMUL float ptr [EBP + 0xffffff68]
0065bb40  d98574ffffff          FLD float ptr [EBP + 0xffffff74]
0065bb46  d88d64ffffff          FMUL float ptr [EBP + 0xffffff64]
0065bb4c  dee1                  FSUBRP
0065bb4e  d99d6cfeffff          FSTP float ptr [EBP + 0xfffffe6c]
0065bb54  d98564feffff          FLD float ptr [EBP + 0xfffffe64]
0065bb5a  d99df4feffff          FSTP float ptr [EBP + 0xfffffef4]
0065bb60  d98568feffff          FLD float ptr [EBP + 0xfffffe68]
0065bb66  d99df8feffff          FSTP float ptr [EBP + 0xfffffef8]
0065bb6c  d9856cfeffff          FLD float ptr [EBP + 0xfffffe6c]
0065bb72  d99dfcfeffff          FSTP float ptr [EBP + 0xfffffefc]
0065bb78  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
0065bb7e  898d58ffffff          MOV dword ptr [EBP + 0xffffff58],ECX
0065bb84  8b95f8feffff          MOV EDX,dword ptr [EBP + 0xfffffef8]
0065bb8a  89955cffffff          MOV dword ptr [EBP + 0xffffff5c],EDX
0065bb90  8b85fcfeffff          MOV EAX,dword ptr [EBP + 0xfffffefc]
0065bb96  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
0065bb9c  d98560ffffff          FLD float ptr [EBP + 0xffffff60]
0065bba2  dc1d386b9800          FCOMP double ptr [0x00986b38]
0065bba8  dfe0                  FNSTSW AX
0065bbaa  f6c401                TEST AH,0x1
0065bbad  7515                  JNZ 0x0065bbc4
0065bbaf  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
0065bbb2  51                    PUSH ECX
0065bbb3  68f4010000            PUSH 0x1f4
0065bbb8  6a04                  PUSH 0x4
0065bbba  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0065bbbd  e87e7e0800            CALL 0x006e3a40
0065bbc2  eb13                  JMP 0x0065bbd7
0065bbc4  8b5580                MOV EDX,dword ptr [EBP + -0x80]
0065bbc7  52                    PUSH EDX
0065bbc8  68f4010000            PUSH 0x1f4
0065bbcd  6a05                  PUSH 0x5
0065bbcf  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0065bbd2  e8697e0800            CALL 0x006e3a40
0065bbd7  b801000000            MOV EAX,0x1
0065bbdc  e963010000            JMP 0x0065bd44
0065bbe1  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0065bbe4  8b10                  MOV EDX,dword ptr [EAX]
0065bbe6  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065bbe9  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065bbec  ffd0                  CALL EAX
0065bbee  8945f4                MOV dword ptr [EBP + -0xc],EAX
0065bbf1  837df400              CMP dword ptr [EBP + -0xc],0x0
0065bbf5  0f8444010000          JZ 0x0065bd3f
0065bbfb  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0065bbfe  8b91ec000000          MOV EDX,dword ptr [ECX + 0xec]
0065bc04  899560feffff          MOV dword ptr [EBP + 0xfffffe60],EDX
0065bc0a  83bd60feffff01        CMP dword ptr [EBP + 0xfffffe60],0x1
0065bc11  0f8528010000          JNZ 0x0065bd3f
0065bc17  b8ffff0000            MOV EAX,0xffff
0065bc1c  66898554ffffff        MOV word ptr [EBP + 0xffffff54],AX
0065bc23  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0065bc26  e8755cf1ff            CALL 0x005718a0
0065bc2b  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0065bc2e  8a9116110000          MOV DL,byte ptr [ECX + 0x1116]
0065bc34  88955ffeffff          MOV byte ptr [EBP + 0xfffffe5f],DL
0065bc3a  0fb6855ffeffff        MOVZX EAX,byte ptr [EBP + 0xfffffe5f]
0065bc41  83f801                CMP EAX,0x1
0065bc44  750c                  JNZ 0x0065bc52
0065bc46  b9feff0000            MOV ECX,0xfffe
0065bc4b  66898d54ffffff        MOV word ptr [EBP + 0xffffff54],CX
0065bc52  6a01                  PUSH 0x1
0065bc54  6a00                  PUSH 0x0
0065bc56  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
0065bc59  8b02                  MOV EAX,dword ptr [EDX]
0065bc5b  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065bc5e  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0065bc61  ffd2                  CALL EDX
0065bc63  8bc8                  MOV ECX,EAX
0065bc65  e876bef5ff            CALL 0x005b7ae0
0065bc6a  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0065bc6d  8b10                  MOV EDX,dword ptr [EAX]
0065bc6f  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065bc72  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065bc75  ffd0                  CALL EAX
0065bc77  898558feffff          MOV dword ptr [EBP + 0xfffffe58],EAX
0065bc7d  8b8d58feffff          MOV ECX,dword ptr [EBP + 0xfffffe58]
0065bc83  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
0065bc89  899554feffff          MOV dword ptr [EBP + 0xfffffe54],EDX
0065bc8f  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
0065bc95  e85659f3ff            CALL 0x005915f0
0065bc9a  6a08                  PUSH 0x8
0065bc9c  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0065bc9f  8b10                  MOV EDX,dword ptr [EAX]
0065bca1  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065bca4  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065bca7  ffd0                  CALL EAX
0065bca9  8bc8                  MOV ECX,EAX
0065bcab  e8e07ff0ff            CALL 0x00563c90
0065bcb0  d9ee                  FLDZ
0065bcb2  d99de8feffff          FSTP float ptr [EBP + 0xfffffee8]
0065bcb8  d9ee                  FLDZ
0065bcba  d99decfeffff          FSTP float ptr [EBP + 0xfffffeec]
0065bcc0  d9ee                  FLDZ
0065bcc2  d99df0feffff          FSTP float ptr [EBP + 0xfffffef0]
0065bcc8  d945e4                FLD float ptr [EBP + -0x1c]
0065bccb  d99ddcfeffff          FSTP float ptr [EBP + 0xfffffedc]
0065bcd1  d945e0                FLD float ptr [EBP + -0x20]
0065bcd4  d99de0feffff          FSTP float ptr [EBP + 0xfffffee0]
0065bcda  d945dc                FLD float ptr [EBP + -0x24]
0065bcdd  d99de4feffff          FSTP float ptr [EBP + 0xfffffee4]
0065bce3  6a00                  PUSH 0x0
0065bce5  6a00                  PUSH 0x0
0065bce7  6a00                  PUSH 0x0
0065bce9  8d8de8feffff          LEA ECX,[EBP + 0xfffffee8]
0065bcef  51                    PUSH ECX
0065bcf0  6a00                  PUSH 0x0
0065bcf2  6a00                  PUSH 0x0
0065bcf4  0fb655fb              MOVZX EDX,byte ptr [EBP + -0x5]
0065bcf8  52                    PUSH EDX
0065bcf9  6a00                  PUSH 0x0
0065bcfb  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
0065bcfe  50                    PUSH EAX
0065bcff  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0065bd02  51                    PUSH ECX
0065bd03  51                    PUSH ECX
0065bd04  d9ee                  FLDZ
0065bd06  d91c24                FSTP float ptr [ESP]
0065bd09  51                    PUSH ECX
0065bd0a  d945d8                FLD float ptr [EBP + -0x28]
0065bd0d  d91c24                FSTP float ptr [ESP]
0065bd10  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0065bd13  52                    PUSH EDX
0065bd14  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0065bd17  50                    PUSH EAX
0065bd18  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0065bd1b  51                    PUSH ECX
0065bd1c  8d95dcfeffff          LEA EDX,[EBP + 0xfffffedc]
0065bd22  52                    PUSH EDX
0065bd23  0fb78554ffffff        MOVZX EAX,word ptr [EBP + 0xffffff54]
0065bd2a  50                    PUSH EAX
0065bd2b  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065bd2e  8b11                  MOV EDX,dword ptr [ECX]
0065bd30  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0065bd33  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0065bd36  ffd0                  CALL EAX
0065bd38  8bc8                  MOV ECX,EAX
0065bd3a  e8318af0ff            CALL 0x00564770
0065bd3f  b801000000            MOV EAX,0x1
0065bd44  8be5                  MOV ESP,EBP
0065bd46  5d                    POP EBP
0065bd47  c20400                RET 0x4
