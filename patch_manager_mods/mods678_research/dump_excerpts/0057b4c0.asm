; FN 0057b4c0 FUN_0057b4c0
0057b4c0  55                    PUSH EBP
0057b4c1  8bec                  MOV EBP,ESP
0057b4c3  6aff                  PUSH -0x1
0057b4c5  682b119500            PUSH 0x95112b
0057b4ca  64a100000000          MOV EAX,FS:[0x0]
0057b4d0  50                    PUSH EAX
0057b4d1  81ec94000000          SUB ESP,0x94
0057b4d7  a1201fa100            MOV EAX,[0x00a11f20]
0057b4dc  33c5                  XOR EAX,EBP
0057b4de  50                    PUSH EAX
0057b4df  8d45f4                LEA EAX,[EBP + -0xc]
0057b4e2  64a300000000          MOV FS:[0x0],EAX
0057b4e8  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
0057b4ee  c745e000000000        MOV dword ptr [EBP + -0x20],0x0
0057b4f5  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057b4fa  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057b4fd  894db8                MOV dword ptr [EBP + -0x48],ECX
0057b500  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0057b503  52                    PUSH EDX
0057b504  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
0057b507  e8d40bfaff            CALL 0x0051c0e0
0057b50c  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0057b50f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057b514  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057b517  894db4                MOV dword ptr [EBP + -0x4c],ECX
0057b51a  8b5514                MOV EDX,dword ptr [EBP + 0x14]
0057b51d  52                    PUSH EDX
0057b51e  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
0057b521  e87a0bfaff            CALL 0x0051c0a0
0057b526  8945e8                MOV dword ptr [EBP + -0x18],EAX
0057b529  837de400              CMP dword ptr [EBP + -0x1c],0x0
0057b52d  7507                  JNZ 0x0057b536
0057b52f  33c0                  XOR EAX,EAX
0057b531  e9f1030000            JMP 0x0057b927
0057b536  6a00                  PUSH 0x0
0057b538  6a00                  PUSH 0x0
0057b53a  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0057b53d  50                    PUSH EAX
0057b53e  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b544  e8377a0500            CALL 0x005d2f80
0057b549  85c0                  TEST EAX,EAX
0057b54b  7507                  JNZ 0x0057b554
0057b54d  33c0                  XOR EAX,EAX
0057b54f  e9d3030000            JMP 0x0057b927
0057b554  0fb64d0c              MOVZX ECX,byte ptr [EBP + 0xc]
0057b558  51                    PUSH ECX
0057b559  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0057b55c  e8ff8c0800            CALL 0x00604260
0057b561  8945f0                MOV dword ptr [EBP + -0x10],EAX
0057b564  837df000              CMP dword ptr [EBP + -0x10],0x0
0057b568  7507                  JNZ 0x0057b571
0057b56a  33c0                  XOR EAX,EAX
0057b56c  e9b6030000            JMP 0x0057b927
0057b571  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
0057b578  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0057b57b  8a4211                MOV AL,byte ptr [EDX + 0x11]
0057b57e  8845b3                MOV byte ptr [EBP + -0x4d],AL
0057b581  0fb64db3              MOVZX ECX,byte ptr [EBP + -0x4d]
0057b585  81f9ff000000          CMP ECX,0xff
0057b58b  7422                  JZ 0x0057b5af
0057b58d  0fb64db3              MOVZX ECX,byte ptr [EBP + -0x4d]
0057b591  ba01000000            MOV EDX,0x1
0057b596  d3e2                  SHL EDX,CL
0057b598  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0057b59b  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
0057b5a1  750c                  JNZ 0x0057b5af
0057b5a3  c78568ffffff00000000  MOV dword ptr [EBP + 0xffffff68],0x0
0057b5ad  eb0a                  JMP 0x0057b5b9
0057b5af  c78568ffffff01000000  MOV dword ptr [EBP + 0xffffff68],0x1
0057b5b9  83bd68ffffff00        CMP dword ptr [EBP + 0xffffff68],0x0
0057b5c0  0f8445010000          JZ 0x0057b70b
0057b5c6  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0057b5c9  668b11                MOV DX,word ptr [ECX]
0057b5cc  668955b0              MOV word ptr [EBP + -0x50],DX
0057b5d0  0fb745b0              MOVZX EAX,word ptr [EBP + -0x50]
0057b5d4  83f80a                CMP EAX,0xa
0057b5d7  0f852e010000          JNZ 0x0057b70b
0057b5dd  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0057b5e0  8a510c                MOV DL,byte ptr [ECX + 0xc]
0057b5e3  8855af                MOV byte ptr [EBP + -0x51],DL
0057b5e6  0fb645af              MOVZX EAX,byte ptr [EBP + -0x51]
0057b5ea  85c0                  TEST EAX,EAX
0057b5ec  0f8417010000          JZ 0x0057b709
0057b5f2  837de800              CMP dword ptr [EBP + -0x18],0x0
0057b5f6  7446                  JZ 0x0057b63e
0057b5f8  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0057b5fb  8b11                  MOV EDX,dword ptr [ECX]
0057b5fd  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0057b600  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057b603  ffd0                  CALL EAX
0057b605  85c0                  TEST EAX,EAX
0057b607  7435                  JZ 0x0057b63e
0057b609  8b4d14                MOV ECX,dword ptr [EBP + 0x14]
0057b60c  51                    PUSH ECX
0057b60d  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b613  e838610500            CALL 0x005d1750
0057b618  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0057b61b  837dd400              CMP dword ptr [EBP + -0x2c],0x0
0057b61f  741d                  JZ 0x0057b63e
0057b621  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
0057b624  8a4204                MOV AL,byte ptr [EDX + 0x4]
0057b627  c0e802                SHR AL,0x2
0057b62a  2403                  AND AL,0x3
0057b62c  0fb6c8                MOVZX ECX,AL
0057b62f  83f901                CMP ECX,0x1
0057b632  750a                  JNZ 0x0057b63e
0057b634  a1b8449900            MOV EAX,[0x009944b8]
0057b639  e9e9020000            JMP 0x0057b927
0057b63e  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
0057b641  8b420c                MOV EAX,dword ptr [EDX + 0xc]
0057b644  8945a8                MOV dword ptr [EBP + -0x58],EAX
0057b647  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
0057b64d  894da4                MOV dword ptr [EBP + -0x5c],ECX
0057b650  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
0057b653  52                    PUSH EDX
0057b654  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
0057b657  8b4834                MOV ECX,dword ptr [EAX + 0x34]
0057b65a  e8a19b1500            CALL 0x006d5200
0057b65f  8945d8                MOV dword ptr [EBP + -0x28],EAX
0057b662  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0057b665  668b5102              MOV DX,word ptr [ECX + 0x2]
0057b669  668955a2              MOV word ptr [EBP + -0x5e],DX
0057b66d  0fb745a2              MOVZX EAX,word ptr [EBP + -0x5e]
0057b671  8945dc                MOV dword ptr [EBP + -0x24],EAX
0057b674  817d140000007f        CMP dword ptr [EBP + 0x14],0x7f000000
0057b67b  7507                  JNZ 0x0057b684
0057b67d  c745e001000000        MOV dword ptr [EBP + -0x20],0x1
0057b684  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b68a  8b9120050000          MOV EDX,dword ptr [ECX + 0x520]
0057b690  89559c                MOV dword ptr [EBP + -0x64],EDX
0057b693  837d9c00              CMP dword ptr [EBP + -0x64],0x0
0057b697  741d                  JZ 0x0057b6b6
0057b699  6a0c                  PUSH 0xc
0057b69b  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b6a1  e8ba3efcff            CALL 0x0053f560
0057b6a6  85c0                  TEST EAX,EAX
0057b6a8  740c                  JZ 0x0057b6b6
0057b6aa  c78564ffffff01000000  MOV dword ptr [EBP + 0xffffff64],0x1
0057b6b4  eb0a                  JMP 0x0057b6c0
0057b6b6  c78564ffffff00000000  MOV dword ptr [EBP + 0xffffff64],0x0
0057b6c0  6a00                  PUSH 0x0
0057b6c2  8b8564ffffff          MOV EAX,dword ptr [EBP + 0xffffff64]
0057b6c8  50                    PUSH EAX
0057b6c9  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0057b6cc  51                    PUSH ECX
0057b6cd  8b5520                MOV EDX,dword ptr [EBP + 0x20]
0057b6d0  52                    PUSH EDX
0057b6d1  8b4514                MOV EAX,dword ptr [EBP + 0x14]
0057b6d4  50                    PUSH EAX
0057b6d5  8b4d18                MOV ECX,dword ptr [EBP + 0x18]
0057b6d8  83ec0c                SUB ESP,0xc
0057b6db  8bd4                  MOV EDX,ESP
0057b6dd  8b01                  MOV EAX,dword ptr [ECX]
0057b6df  8902                  MOV dword ptr [EDX],EAX
0057b6e1  8b4104                MOV EAX,dword ptr [ECX + 0x4]
0057b6e4  894204                MOV dword ptr [EDX + 0x4],EAX
0057b6e7  8b4908                MOV ECX,dword ptr [ECX + 0x8]
0057b6ea  894a08                MOV dword ptr [EDX + 0x8],ECX
0057b6ed  0fb65510              MOVZX EDX,byte ptr [EBP + 0x10]
0057b6f1  52                    PUSH EDX
0057b6f2  0fb6450c              MOVZX EAX,byte ptr [EBP + 0xc]
0057b6f6  50                    PUSH EAX
0057b6f7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0057b6fa  51                    PUSH ECX
0057b6fb  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b701  e8ca91feff            CALL 0x005648d0
0057b706  8945ec                MOV dword ptr [EBP + -0x14],EAX
0057b709  eb78                  JMP 0x0057b783
0057b70b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0057b70e  668b02                MOV AX,word ptr [EDX]
0057b711  6689459a              MOV word ptr [EBP + -0x66],AX
0057b715  0fb74d9a              MOVZX ECX,word ptr [EBP + -0x66]
0057b719  83f925                CMP ECX,0x25
0057b71c  7529                  JNZ 0x0057b747
0057b71e  0fb6550c              MOVZX EDX,byte ptr [EBP + 0xc]
0057b722  52                    PUSH EDX
0057b723  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0057b726  50                    PUSH EAX
0057b727  8b4d1c                MOV ECX,dword ptr [EBP + 0x1c]
0057b72a  51                    PUSH ECX
0057b72b  8b5518                MOV EDX,dword ptr [EBP + 0x18]
0057b72e  52                    PUSH EDX
0057b72f  8b4514                MOV EAX,dword ptr [EBP + 0x14]
0057b732  50                    PUSH EAX
0057b733  6a00                  PUSH 0x0
0057b735  6a06                  PUSH 0x6
0057b737  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b73d  e83efbffff            CALL 0x0057b280
0057b742  8945ec                MOV dword ptr [EBP + -0x14],EAX
0057b745  eb3c                  JMP 0x0057b783
0057b747  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0057b74a  668b11                MOV DX,word ptr [ECX]
0057b74d  66895598              MOV word ptr [EBP + -0x68],DX
0057b751  0fb74598              MOVZX EAX,word ptr [EBP + -0x68]
0057b755  83f82e                CMP EAX,0x2e
0057b758  7529                  JNZ 0x0057b783
0057b75a  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0057b75d  8b5114                MOV EDX,dword ptr [ECX + 0x14]
0057b760  895594                MOV dword ptr [EBP + -0x6c],EDX
0057b763  8b451c                MOV EAX,dword ptr [EBP + 0x1c]
0057b766  50                    PUSH EAX
0057b767  8b4d18                MOV ECX,dword ptr [EBP + 0x18]
0057b76a  51                    PUSH ECX
0057b76b  8b5514                MOV EDX,dword ptr [EBP + 0x14]
0057b76e  52                    PUSH EDX
0057b76f  6a1d                  PUSH 0x1d
0057b771  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
0057b774  50                    PUSH EAX
0057b775  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b77b  e850c9feff            CALL 0x005680d0
0057b780  8945ec                MOV dword ptr [EBP + -0x14],EAX
0057b783  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b789  8b91ac110000          MOV EDX,dword ptr [ECX + 0x11ac]
0057b78f  895590                MOV dword ptr [EBP + -0x70],EDX
0057b792  837d9000              CMP dword ptr [EBP + -0x70],0x0
0057b796  0f8488010000          JZ 0x0057b924
0057b79c  c745d000000000        MOV dword ptr [EBP + -0x30],0x0
0057b7a3  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057b7a8  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057b7ab  894d8c                MOV dword ptr [EBP + -0x74],ECX
0057b7ae  8b4d8c                MOV ECX,dword ptr [EBP + -0x74]
0057b7b1  e81a11faff            CALL 0x0051c8d0
0057b7b6  8945cc                MOV dword ptr [EBP + -0x34],EAX
0057b7b9  837dcc00              CMP dword ptr [EBP + -0x34],0x0
0057b7bd  7428                  JZ 0x0057b7e7
0057b7bf  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
0057b7c2  8b8250020000          MOV EAX,dword ptr [EDX + 0x250]
0057b7c8  894588                MOV dword ptr [EBP + -0x78],EAX
0057b7cb  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
0057b7ce  83e102                AND ECX,0x2
0057b7d1  7414                  JZ 0x0057b7e7
0057b7d3  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
0057b7d6  c7824c02000002000000  MOV dword ptr [EDX + 0x24c],0x2
0057b7e0  c745d001000000        MOV dword ptr [EBP + -0x30],0x1
0057b7e7  837dd000              CMP dword ptr [EBP + -0x30],0x0
0057b7eb  7557                  JNZ 0x0057b844
0057b7ed  837dcc00              CMP dword ptr [EBP + -0x34],0x0
0057b7f1  7451                  JZ 0x0057b844
0057b7f3  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0057b7f6  8b8850020000          MOV ECX,dword ptr [EAX + 0x250]
0057b7fc  894d84                MOV dword ptr [EBP + -0x7c],ECX
0057b7ff  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
0057b802  81e280000000          AND EDX,0x80
0057b808  743a                  JZ 0x0057b844
0057b80a  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0057b80d  e81eb61500            CALL 0x006d6e30
0057b812  894580                MOV dword ptr [EBP + -0x80],EAX
0057b815  8b4580                MOV EAX,dword ptr [EBP + -0x80]
0057b818  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
0057b81e  888d7fffffff          MOV byte ptr [EBP + 0xffffff7f],CL
0057b824  0fb6957fffffff        MOVZX EDX,byte ptr [EBP + 0xffffff7f]
0057b82b  83fa14                CMP EDX,0x14
0057b82e  7414                  JZ 0x0057b844
0057b830  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0057b833  c7804c02000080000000  MOV dword ptr [EAX + 0x24c],0x80
0057b83d  c745d001000000        MOV dword ptr [EBP + -0x30],0x1
0057b844  837dd000              CMP dword ptr [EBP + -0x30],0x0
0057b848  0f84d6000000          JZ 0x0057b924
0057b84e  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
0057b854  e8679bfcff            CALL 0x005453c0
0057b859  8945c8                MOV dword ptr [EBP + -0x38],EAX
0057b85c  837dc800              CMP dword ptr [EBP + -0x38],0x0
0057b860  0f84be000000          JZ 0x0057b924
0057b866  6a34                  PUSH 0x34
0057b868  e8b6de3900            CALL 0x00919723
0057b86d  83c404                ADD ESP,0x4
0057b870  8945bc                MOV dword ptr [EBP + -0x44],EAX
0057b873  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0057b87a  837dbc00              CMP dword ptr [EBP + -0x44],0x0
0057b87e  7410                  JZ 0x0057b890
0057b880  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0057b883  e8d8610600            CALL 0x005e1a60
0057b888  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
0057b88e  eb0a                  JMP 0x0057b89a
0057b890  c78560ffffff00000000  MOV dword ptr [EBP + 0xffffff60],0x0
0057b89a  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
0057b8a0  894dc0                MOV dword ptr [EBP + -0x40],ECX
0057b8a3  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0057b8aa  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
0057b8ad  8955c4                MOV dword ptr [EBP + -0x3c],EDX
0057b8b0  b80b000000            MOV EAX,0xb
0057b8b5  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057b8b8  668901                MOV word ptr [ECX],AX
0057b8bb  68a10f0000            PUSH 0xfa1
0057b8c0  6a00                  PUSH 0x0
0057b8c2  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057b8c5  e806630600            CALL 0x005e1bd0
0057b8ca  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
0057b8cd  8b8224010000          MOV EAX,dword ptr [EDX + 0x124]
0057b8d3  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
0057b8d9  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
0057b8dc  8b9124010000          MOV EDX,dword ptr [ECX + 0x124]
0057b8e2  899574ffffff          MOV dword ptr [EBP + 0xffffff74],EDX
0057b8e8  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057b8ed  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057b8f0  898d70ffffff          MOV dword ptr [EBP + 0xffffff70],ECX
0057b8f6  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0057b8f9  52                    PUSH EDX
0057b8fa  a11c249900            MOV EAX,[0x0099241c]
0057b8ff  50                    PUSH EAX
0057b900  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
0057b906  51                    PUSH ECX
0057b907  8b9574ffffff          MOV EDX,dword ptr [EBP + 0xffffff74]
0057b90d  52                    PUSH EDX
0057b90e  6a00                  PUSH 0x0
0057b910  6a00                  PUSH 0x0
0057b912  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
0057b918  e86309faff            CALL 0x0051c280
0057b91d  8bc8                  MOV ECX,EAX
0057b91f  e84c28faff            CALL 0x0051e170
0057b924  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
0057b927  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0057b92a  64890d00000000        MOV dword ptr FS:[0x0],ECX
0057b931  59                    POP ECX
0057b932  8be5                  MOV ESP,EBP
0057b934  5d                    POP EBP
0057b935  c21c00                RET 0x1c
