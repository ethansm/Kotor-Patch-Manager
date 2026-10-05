; FN 005d3c90 FUN_005d3c90
005d3c90  55                    PUSH EBP
005d3c91  8bec                  MOV EBP,ESP
005d3c93  6aff                  PUSH -0x1
005d3c95  6870379500            PUSH 0x953770
005d3c9a  64a100000000          MOV EAX,FS:[0x0]
005d3ca0  50                    PUSH EAX
005d3ca1  83ec60                SUB ESP,0x60
005d3ca4  a1201fa100            MOV EAX,[0x00a11f20]
005d3ca9  33c5                  XOR EAX,EBP
005d3cab  50                    PUSH EAX
005d3cac  8d45f4                LEA EAX,[EBP + -0xc]
005d3caf  64a300000000          MOV FS:[0x0],EAX
005d3cb5  894d98                MOV dword ptr [EBP + -0x68],ECX
005d3cb8  8d4dec                LEA ECX,[EBP + -0x14]
005d3cbb  e880f81500            CALL 0x00733540
005d3cc0  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005d3cc7  6a00                  PUSH 0x0
005d3cc9  6a3e                  PUSH 0x3e
005d3ccb  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3cce  e88d020300            CALL 0x00603f60
005d3cd3  85c0                  TEST EAX,EAX
005d3cd5  751e                  JNZ 0x005d3cf5
005d3cd7  c745d801000000        MOV dword ptr [EBP + -0x28],0x1
005d3cde  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005d3ce5  8d4dec                LEA ECX,[EBP + -0x14]
005d3ce8  e893fa1500            CALL 0x00733780
005d3ced  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
005d3cf0  e9cb010000            JMP 0x005d3ec0
005d3cf5  c645e300              MOV byte ptr [EBP + -0x1d],0x0
005d3cf9  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
005d3d00  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
005d3d07  eb09                  JMP 0x005d3d12
005d3d09  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005d3d0c  83c001                ADD EAX,0x1
005d3d0f  8945e8                MOV dword ptr [EBP + -0x18],EAX
005d3d12  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3d15  8b918c020000          MOV EDX,dword ptr [ECX + 0x28c]
005d3d1b  8955c0                MOV dword ptr [EBP + -0x40],EDX
005d3d1e  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005d3d21  3b45c0                CMP EAX,dword ptr [EBP + -0x40]
005d3d24  0f8d37010000          JGE 0x005d3e61
005d3d2a  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d3d2d  51                    PUSH ECX
005d3d2e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3d31  e86a050300            CALL 0x006042a0
005d3d36  8945bc                MOV dword ptr [EBP + -0x44],EAX
005d3d39  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
005d3d3c  8a4211                MOV AL,byte ptr [EDX + 0x11]
005d3d3f  8845bb                MOV byte ptr [EBP + -0x45],AL
005d3d42  0fb64dbb              MOVZX ECX,byte ptr [EBP + -0x45]
005d3d46  81f9ff000000          CMP ECX,0xff
005d3d4c  741f                  JZ 0x005d3d6d
005d3d4e  0fb64dbb              MOVZX ECX,byte ptr [EBP + -0x45]
005d3d52  ba01000000            MOV EDX,0x1
005d3d57  d3e2                  SHL EDX,CL
005d3d59  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d3d5c  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
005d3d62  7509                  JNZ 0x005d3d6d
005d3d64  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
005d3d6b  eb07                  JMP 0x005d3d74
005d3d6d  c7459401000000        MOV dword ptr [EBP + -0x6c],0x1
005d3d74  837d9400              CMP dword ptr [EBP + -0x6c],0x0
005d3d78  0f84de000000          JZ 0x005d3e5c
005d3d7e  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d3d81  51                    PUSH ECX
005d3d82  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3d85  e816050300            CALL 0x006042a0
005d3d8a  8945b4                MOV dword ptr [EBP + -0x4c],EAX
005d3d8d  8b55b4                MOV EDX,dword ptr [EBP + -0x4c]
005d3d90  668b02                MOV AX,word ptr [EDX]
005d3d93  668945b2              MOV word ptr [EBP + -0x4e],AX
005d3d97  0fb74db2              MOVZX ECX,word ptr [EBP + -0x4e]
005d3d9b  83f93e                CMP ECX,0x3e
005d3d9e  0f85b8000000          JNZ 0x005d3e5c
005d3da4  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
005d3da7  83c201                ADD EDX,0x1
005d3daa  8955e4                MOV dword ptr [EBP + -0x1c],EDX
005d3dad  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005d3db0  50                    PUSH EAX
005d3db1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3db4  e8e7040300            CALL 0x006042a0
005d3db9  8945ac                MOV dword ptr [EBP + -0x54],EAX
005d3dbc  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
005d3dbf  668b5102              MOV DX,word ptr [ECX + 0x2]
005d3dc3  668955aa              MOV word ptr [EBP + -0x56],DX
005d3dc7  0fb745aa              MOVZX EAX,word ptr [EBP + -0x56]
005d3dcb  8945dc                MOV dword ptr [EBP + -0x24],EAX
005d3dce  837ddc00              CMP dword ptr [EBP + -0x24],0x0
005d3dd2  7518                  JNZ 0x005d3dec
005d3dd4  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005d3dd7  8b9100120000          MOV EDX,dword ptr [ECX + 0x1200]
005d3ddd  8955a4                MOV dword ptr [EBP + -0x5c],EDX
005d3de0  837da400              CMP dword ptr [EBP + -0x5c],0x0
005d3de4  7404                  JZ 0x005d3dea
005d3de6  c645e301              MOV byte ptr [EBP + -0x1d],0x1
005d3dea  eb70                  JMP 0x005d3e5c
005d3dec  6850669900            PUSH 0x996650
005d3df1  8d4dd0                LEA ECX,[EBP + -0x30]
005d3df4  e877f71500            CALL 0x00733570
005d3df9  c645fc01              MOV byte ptr [EBP + -0x4],0x1
005d3dfd  a1d0b4a100            MOV EAX,[0x00a1b4d0]
005d3e02  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
005d3e08  894da0                MOV dword ptr [EBP + -0x60],ECX
005d3e0b  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
005d3e0e  8b8234010000          MOV EAX,dword ptr [EDX + 0x134]
005d3e14  89459c                MOV dword ptr [EBP + -0x64],EAX
005d3e17  8d4dec                LEA ECX,[EBP + -0x14]
005d3e1a  51                    PUSH ECX
005d3e1b  8d55d0                LEA EDX,[EBP + -0x30]
005d3e1e  52                    PUSH EDX
005d3e1f  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005d3e22  50                    PUSH EAX
005d3e23  8b4d9c                MOV ECX,dword ptr [EBP + -0x64]
005d3e26  e8a5731400            CALL 0x0071b1d0
005d3e2b  c645fc00              MOV byte ptr [EBP + -0x4],0x0
005d3e2f  8d4dd0                LEA ECX,[EBP + -0x30]
005d3e32  e849f91500            CALL 0x00733780
005d3e37  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005d3e3a  83c118                ADD ECX,0x18
005d3e3d  e8ce011600            CALL 0x00734010
005d3e42  50                    PUSH EAX
005d3e43  8d4dec                LEA ECX,[EBP + -0x14]
005d3e46  e8c5011600            CALL 0x00734010
005d3e4b  50                    PUSH EAX
005d3e4c  e876323600            CALL 0x009370c7
005d3e51  83c408                ADD ESP,0x8
005d3e54  85c0                  TEST EAX,EAX
005d3e56  7504                  JNZ 0x005d3e5c
005d3e58  c645e301              MOV byte ptr [EBP + -0x1d],0x1
005d3e5c  e9a8feffff            JMP 0x005d3d09
005d3e61  837de400              CMP dword ptr [EBP + -0x1c],0x0
005d3e65  7e40                  JLE 0x005d3ea7
005d3e67  0fb64de3              MOVZX ECX,byte ptr [EBP + -0x1d]
005d3e6b  85c9                  TEST ECX,ECX
005d3e6d  741d                  JZ 0x005d3e8c
005d3e6f  c745cc01000000        MOV dword ptr [EBP + -0x34],0x1
005d3e76  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005d3e7d  8d4dec                LEA ECX,[EBP + -0x14]
005d3e80  e8fbf81500            CALL 0x00733780
005d3e85  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
005d3e88  eb36                  JMP 0x005d3ec0
005d3e8c  c745c800000000        MOV dword ptr [EBP + -0x38],0x0
005d3e93  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005d3e9a  8d4dec                LEA ECX,[EBP + -0x14]
005d3e9d  e8def81500            CALL 0x00733780
005d3ea2  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
005d3ea5  eb19                  JMP 0x005d3ec0
005d3ea7  c745c401000000        MOV dword ptr [EBP + -0x3c],0x1
005d3eae  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005d3eb5  8d4dec                LEA ECX,[EBP + -0x14]
005d3eb8  e8c3f81500            CALL 0x00733780
005d3ebd  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
005d3ec0  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005d3ec3  64890d00000000        MOV dword ptr FS:[0x0],ECX
005d3eca  59                    POP ECX
005d3ecb  8be5                  MOV ESP,EBP
005d3ecd  5d                    POP EBP
005d3ece  c20400                RET 0x4
