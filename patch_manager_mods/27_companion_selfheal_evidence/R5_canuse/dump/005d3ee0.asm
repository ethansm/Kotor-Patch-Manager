; FN 005d3ee0 FUN_005d3ee0
005d3ee0  55                    PUSH EBP
005d3ee1  8bec                  MOV EBP,ESP
005d3ee3  6aff                  PUSH -0x1
005d3ee5  6898379500            PUSH 0x953798
005d3eea  64a100000000          MOV EAX,FS:[0x0]
005d3ef0  50                    PUSH EAX
005d3ef1  83ec7c                SUB ESP,0x7c
005d3ef4  a1201fa100            MOV EAX,[0x00a11f20]
005d3ef9  33c5                  XOR EAX,EBP
005d3efb  50                    PUSH EAX
005d3efc  8d45f4                LEA EAX,[EBP + -0xc]
005d3eff  64a300000000          MOV FS:[0x0],EAX
005d3f05  894d80                MOV dword ptr [EBP + -0x80],ECX
005d3f08  6a00                  PUSH 0x0
005d3f0a  6a45                  PUSH 0x45
005d3f0c  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3f0f  e84c000300            CALL 0x00603f60
005d3f14  85c0                  TEST EAX,EAX
005d3f16  750a                  JNZ 0x005d3f22
005d3f18  b801000000            MOV EAX,0x1
005d3f1d  e93a020000            JMP 0x005d415c
005d3f22  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
005d3f29  eb09                  JMP 0x005d3f34
005d3f2b  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005d3f2e  83c001                ADD EAX,0x1
005d3f31  8945e8                MOV dword ptr [EBP + -0x18],EAX
005d3f34  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3f37  8b918c020000          MOV EDX,dword ptr [ECX + 0x28c]
005d3f3d  8955d8                MOV dword ptr [EBP + -0x28],EDX
005d3f40  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005d3f43  3b45d8                CMP EAX,dword ptr [EBP + -0x28]
005d3f46  0f8d0b020000          JGE 0x005d4157
005d3f4c  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d3f4f  51                    PUSH ECX
005d3f50  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3f53  e848030300            CALL 0x006042a0
005d3f58  8945d4                MOV dword ptr [EBP + -0x2c],EAX
005d3f5b  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
005d3f5e  8a4211                MOV AL,byte ptr [EDX + 0x11]
005d3f61  8845d3                MOV byte ptr [EBP + -0x2d],AL
005d3f64  0fb64dd3              MOVZX ECX,byte ptr [EBP + -0x2d]
005d3f68  81f9ff000000          CMP ECX,0xff
005d3f6e  7422                  JZ 0x005d3f92
005d3f70  0fb64dd3              MOVZX ECX,byte ptr [EBP + -0x2d]
005d3f74  ba01000000            MOV EDX,0x1
005d3f79  d3e2                  SHL EDX,CL
005d3f7b  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d3f7e  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
005d3f84  750c                  JNZ 0x005d3f92
005d3f86  c7857cffffff00000000  MOV dword ptr [EBP + 0xffffff7c],0x0
005d3f90  eb0a                  JMP 0x005d3f9c
005d3f92  c7857cffffff01000000  MOV dword ptr [EBP + 0xffffff7c],0x1
005d3f9c  83bd7cffffff00        CMP dword ptr [EBP + 0xffffff7c],0x0
005d3fa3  0f84a9010000          JZ 0x005d4152
005d3fa9  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d3fac  51                    PUSH ECX
005d3fad  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3fb0  e8eb020300            CALL 0x006042a0
005d3fb5  8945cc                MOV dword ptr [EBP + -0x34],EAX
005d3fb8  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
005d3fbb  668b02                MOV AX,word ptr [EDX]
005d3fbe  668945ca              MOV word ptr [EBP + -0x36],AX
005d3fc2  0fb74dca              MOVZX ECX,word ptr [EBP + -0x36]
005d3fc6  83f945                CMP ECX,0x45
005d3fc9  0f8583010000          JNZ 0x005d4152
005d3fcf  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005d3fd2  52                    PUSH EDX
005d3fd3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3fd6  e8c5020300            CALL 0x006042a0
005d3fdb  8945c4                MOV dword ptr [EBP + -0x3c],EAX
005d3fde  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
005d3fe1  668b4802              MOV CX,word ptr [EAX + 0x2]
005d3fe5  66894dc2              MOV word ptr [EBP + -0x3e],CX
005d3fe9  0fb755c2              MOVZX EDX,word ptr [EBP + -0x3e]
005d3fed  8955f0                MOV dword ptr [EBP + -0x10],EDX
005d3ff0  688c2e9900            PUSH 0x992e8c
005d3ff5  8d4ddc                LEA ECX,[EBP + -0x24]
005d3ff8  e873f51500            CALL 0x00733570
005d3ffd  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005d4004  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005d4007  50                    PUSH EAX
005d4008  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d400b  e890020300            CALL 0x006042a0
005d4010  8945bc                MOV dword ptr [EBP + -0x44],EAX
005d4013  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
005d4016  668b5106              MOV DX,word ptr [ECX + 0x6]
005d401a  668955ba              MOV word ptr [EBP + -0x46],DX
005d401e  a1d0b4a100            MOV EAX,[0x00a1b4d0]
005d4023  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
005d4029  894db4                MOV dword ptr [EBP + -0x4c],ECX
005d402c  8d55ec                LEA EDX,[EBP + -0x14]
005d402f  52                    PUSH EDX
005d4030  8d45dc                LEA EAX,[EBP + -0x24]
005d4033  50                    PUSH EAX
005d4034  0fb74dba              MOVZX ECX,word ptr [EBP + -0x46]
005d4038  51                    PUSH ECX
005d4039  6a1a                  PUSH 0x1a
005d403b  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
005d403e  e84dc11100            CALL 0x006f0190
005d4043  8bc8                  MOV ECX,EAX
005d4045  e8a67c1400            CALL 0x0071bcf0
005d404a  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005d4051  8d4ddc                LEA ECX,[EBP + -0x24]
005d4054  e827f71500            CALL 0x00733780
005d4059  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
005d405c  899578ffffff          MOV dword ptr [EBP + 0xffffff78],EDX
005d4062  83bd78ffffff05        CMP dword ptr [EBP + 0xffffff78],0x5
005d4069  0f87d7000000          JA 0x005d4146
005d406f  8b8578ffffff          MOV EAX,dword ptr [EBP + 0xffffff78]
005d4075  ff248570415d00        JMP dword ptr [EAX*0x4 + 0x5d4170]
005d407c  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
005d407f  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005d4085  8955b0                MOV dword ptr [EBP + -0x50],EDX
005d4088  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
005d408b  8a88ed000000          MOV CL,byte ptr [EAX + 0xed]
005d4091  884daf                MOV byte ptr [EBP + -0x51],CL
005d4094  0fb655af              MOVZX EDX,byte ptr [EBP + -0x51]
005d4098  8955e4                MOV dword ptr [EBP + -0x1c],EDX
005d409b  e9a6000000            JMP 0x005d4146
005d40a0  8b4580                MOV EAX,dword ptr [EBP + -0x80]
005d40a3  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d40a9  894da8                MOV dword ptr [EBP + -0x58],ECX
005d40ac  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
005d40af  8a82ef000000          MOV AL,byte ptr [EDX + 0xef]
005d40b5  8845a7                MOV byte ptr [EBP + -0x59],AL
005d40b8  0fb64da7              MOVZX ECX,byte ptr [EBP + -0x59]
005d40bc  894de4                MOV dword ptr [EBP + -0x1c],ECX
005d40bf  e982000000            JMP 0x005d4146
005d40c4  8b5580                MOV EDX,dword ptr [EBP + -0x80]
005d40c7  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005d40cd  8945a0                MOV dword ptr [EBP + -0x60],EAX
005d40d0  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
005d40d3  8a91f1000000          MOV DL,byte ptr [ECX + 0xf1]
005d40d9  88559f                MOV byte ptr [EBP + -0x61],DL
005d40dc  0fb6459f              MOVZX EAX,byte ptr [EBP + -0x61]
005d40e0  8945e4                MOV dword ptr [EBP + -0x1c],EAX
005d40e3  eb61                  JMP 0x005d4146
005d40e5  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
005d40e8  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005d40ee  895598                MOV dword ptr [EBP + -0x68],EDX
005d40f1  8b4598                MOV EAX,dword ptr [EBP + -0x68]
005d40f4  8a88f1000000          MOV CL,byte ptr [EAX + 0xf1]
005d40fa  884d97                MOV byte ptr [EBP + -0x69],CL
005d40fd  0fb65597              MOVZX EDX,byte ptr [EBP + -0x69]
005d4101  8955e4                MOV dword ptr [EBP + -0x1c],EDX
005d4104  eb40                  JMP 0x005d4146
005d4106  8b4580                MOV EAX,dword ptr [EBP + -0x80]
005d4109  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d410f  894d90                MOV dword ptr [EBP + -0x70],ECX
005d4112  8b5590                MOV EDX,dword ptr [EBP + -0x70]
005d4115  8a82f5000000          MOV AL,byte ptr [EDX + 0xf5]
005d411b  88458f                MOV byte ptr [EBP + -0x71],AL
005d411e  0fb64d8f              MOVZX ECX,byte ptr [EBP + -0x71]
005d4122  894de4                MOV dword ptr [EBP + -0x1c],ECX
005d4125  eb1f                  JMP 0x005d4146
005d4127  8b5580                MOV EDX,dword ptr [EBP + -0x80]
005d412a  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005d4130  894588                MOV dword ptr [EBP + -0x78],EAX
005d4133  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
005d4136  8a91f7000000          MOV DL,byte ptr [ECX + 0xf7]
005d413c  885587                MOV byte ptr [EBP + -0x79],DL
005d413f  0fb64587              MOVZX EAX,byte ptr [EBP + -0x79]
005d4143  8945e4                MOV dword ptr [EBP + -0x1c],EAX
005d4146  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
005d4149  3b4dec                CMP ECX,dword ptr [EBP + -0x14]
005d414c  7d04                  JGE 0x005d4152
005d414e  33c0                  XOR EAX,EAX
005d4150  eb0a                  JMP 0x005d415c
005d4152  e9d4fdffff            JMP 0x005d3f2b
005d4157  b801000000            MOV EAX,0x1
005d415c  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005d415f  64890d00000000        MOV dword ptr FS:[0x0],ECX
005d4166  59                    POP ECX
005d4167  8be5                  MOV ESP,EBP
005d4169  5d                    POP EBP
005d416a  c20400                RET 0x4
