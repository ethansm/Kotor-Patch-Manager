; FN 005648d0 FUN_005648d0
005648d0  55                    PUSH EBP
005648d1  8bec                  MOV EBP,ESP
005648d3  81ec10010000          SUB ESP,0x110
005648d9  898d00ffffff          MOV dword ptr [EBP + 0xffffff00],ECX
005648df  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
005648e6  c745d800000000        MOV dword ptr [EBP + -0x28],0x0
005648ed  c745f401000000        MOV dword ptr [EBP + -0xc],0x1
005648f4  837d3000              CMP dword ptr [EBP + 0x30],0x0
005648f8  0f85fe000000          JNZ 0x005649fc
005648fe  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
00564904  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
0056490a  898d68ffffff          MOV dword ptr [EBP + 0xffffff68],ECX
00564910  8b5524                MOV EDX,dword ptr [EBP + 0x24]
00564913  52                    PUSH EDX
00564914  8b4528                MOV EAX,dword ptr [EBP + 0x28]
00564917  50                    PUSH EAX
00564918  8b4d20                MOV ECX,dword ptr [EBP + 0x20]
0056491b  51                    PUSH ECX
0056491c  83ec0c                SUB ESP,0xc
0056491f  8bd4                  MOV EDX,ESP
00564921  8b4514                MOV EAX,dword ptr [EBP + 0x14]
00564924  8902                  MOV dword ptr [EDX],EAX
00564926  8b4d18                MOV ECX,dword ptr [EBP + 0x18]
00564929  894a04                MOV dword ptr [EDX + 0x4],ECX
0056492c  8b451c                MOV EAX,dword ptr [EBP + 0x1c]
0056492f  894208                MOV dword ptr [EDX + 0x8],EAX
00564932  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
00564935  51                    PUSH ECX
00564936  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00564939  52                    PUSH EDX
0056493a  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056493d  50                    PUSH EAX
0056493e  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
00564944  e847cf0200            CALL 0x00591890
00564949  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0056494f  8b9120050000          MOV EDX,dword ptr [ECX + 0x520]
00564955  899564ffffff          MOV dword ptr [EBP + 0xffffff64],EDX
0056495b  83bd64ffffff00        CMP dword ptr [EBP + 0xffffff64],0x0
00564962  752e                  JNZ 0x00564992
00564964  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
0056496a  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
00564970  898d60ffffff          MOV dword ptr [EBP + 0xffffff60],ECX
00564976  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
0056497c  e88fb70200            CALL 0x00590110
00564981  85c0                  TEST EAX,EAX
00564983  740d                  JZ 0x00564992
00564985  6a04                  PUSH 0x4
00564987  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0056498d  e8fef2ffff            CALL 0x00563c90
00564992  6a3f                  PUSH 0x3f
00564994  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0056499a  e8c1abfdff            CALL 0x0053f560
0056499f  85c0                  TEST EAX,EAX
005649a1  754f                  JNZ 0x005649f2
005649a3  c745c001000000        MOV dword ptr [EBP + -0x40],0x1
005649aa  6a00                  PUSH 0x0
005649ac  6a00                  PUSH 0x0
005649ae  6a00                  PUSH 0x0
005649b0  6a00                  PUSH 0x0
005649b2  6a00                  PUSH 0x0
005649b4  6a00                  PUSH 0x0
005649b6  6a00                  PUSH 0x0
005649b8  6a00                  PUSH 0x0
005649ba  6a00                  PUSH 0x0
005649bc  6a00                  PUSH 0x0
005649be  6a00                  PUSH 0x0
005649c0  6a00                  PUSH 0x0
005649c2  6a00                  PUSH 0x0
005649c4  6a00                  PUSH 0x0
005649c6  6a00                  PUSH 0x0
005649c8  6a00                  PUSH 0x0
005649ca  6a00                  PUSH 0x0
005649cc  6a00                  PUSH 0x0
005649ce  6a00                  PUSH 0x0
005649d0  6a00                  PUSH 0x0
005649d2  6a00                  PUSH 0x0
005649d4  6a00                  PUSH 0x0
005649d6  6a00                  PUSH 0x0
005649d8  6a00                  PUSH 0x0
005649da  8d55c0                LEA EDX,[EBP + -0x40]
005649dd  52                    PUSH EDX
005649de  6a01                  PUSH 0x1
005649e0  68ffff0000            PUSH 0xffff
005649e5  6a3f                  PUSH 0x3f
005649e7  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
005649ed  e8feadfdff            CALL 0x0053f7f0
005649f2  b801000000            MOV EAX,0x1
005649f7  e902090000            JMP 0x005652fe
005649fc  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
00564a02  c780680500000000007f  MOV dword ptr [EAX + 0x568],0x7f000000
00564a0c  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564a12  c7816c0500000000007f  MOV dword ptr [ECX + 0x56c],0x7f000000
00564a1c  d94514                FLD float ptr [EBP + 0x14]
00564a1f  d95dd4                FSTP float ptr [EBP + -0x2c]
00564a22  d94518                FLD float ptr [EBP + 0x18]
00564a25  d95dd0                FSTP float ptr [EBP + -0x30]
00564a28  d9451c                FLD float ptr [EBP + 0x1c]
00564a2b  d95dcc                FSTP float ptr [EBP + -0x34]
00564a2e  817d200000007f        CMP dword ptr [EBP + 0x20],0x7f000000
00564a35  746c                  JZ 0x00564aa3
00564a37  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
00564a3d  8b4208                MOV EAX,dword ptr [EDX + 0x8]
00564a40  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
00564a46  8b4d20                MOV ECX,dword ptr [EBP + 0x20]
00564a49  51                    PUSH ECX
00564a4a  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
00564a50  e84b76fbff            CALL 0x0051c0a0
00564a55  8945d8                MOV dword ptr [EBP + -0x28],EAX
00564a58  837dd800              CMP dword ptr [EBP + -0x28],0x0
00564a5c  743e                  JZ 0x00564a9c
00564a5e  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
00564a61  e85a09feff            CALL 0x005453c0
00564a66  8945e8                MOV dword ptr [EBP + -0x18],EAX
00564a69  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564a6f  e84c09feff            CALL 0x005453c0
00564a74  3945e8                CMP dword ptr [EBP + -0x18],EAX
00564a77  7407                  JZ 0x00564a80
00564a79  33c0                  XOR EAX,EAX
00564a7b  e97e080000            JMP 0x005652fe
00564a80  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
00564a83  81c294000000          ADD EDX,0x94
00564a89  8b02                  MOV EAX,dword ptr [EDX]
00564a8b  894514                MOV dword ptr [EBP + 0x14],EAX
00564a8e  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
00564a91  894d18                MOV dword ptr [EBP + 0x18],ECX
00564a94  8b5208                MOV EDX,dword ptr [EDX + 0x8]
00564a97  89551c                MOV dword ptr [EBP + 0x1c],EDX
00564a9a  eb07                  JMP 0x00564aa3
00564a9c  33c0                  XOR EAX,EAX
00564a9e  e95b080000            JMP 0x005652fe
00564aa3  837dd800              CMP dword ptr [EBP + -0x28],0x0
00564aa7  0f84b9000000          JZ 0x00564b66
00564aad  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
00564ab0  8b10                  MOV EDX,dword ptr [EAX]
00564ab2  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
00564ab5  8b4230                MOV EAX,dword ptr [EDX + 0x30]
00564ab8  ffd0                  CALL EAX
00564aba  85c0                  TEST EAX,EAX
00564abc  0f84a4000000          JZ 0x00564b66
00564ac2  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
00564ac5  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00564ac8  899558ffffff          MOV dword ptr [EBP + 0xffffff58],EDX
00564ace  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
00564ad4  8b4804                MOV ECX,dword ptr [EAX + 0x4]
00564ad7  898d54ffffff          MOV dword ptr [EBP + 0xffffff54],ECX
00564add  8b9558ffffff          MOV EDX,dword ptr [EBP + 0xffffff58]
00564ae3  3b9554ffffff          CMP EDX,dword ptr [EBP + 0xffffff54]
00564ae9  747b                  JZ 0x00564b66
00564aeb  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
00564af1  8b88ac110000          MOV ECX,dword ptr [EAX + 0x11ac]
00564af7  898d50ffffff          MOV dword ptr [EBP + 0xffffff50],ECX
00564afd  83bd50ffffff00        CMP dword ptr [EBP + 0xffffff50],0x0
00564b04  7560                  JNZ 0x00564b66
00564b06  8b5520                MOV EDX,dword ptr [EBP + 0x20]
00564b09  52                    PUSH EDX
00564b0a  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564b10  e83bcc0600            CALL 0x005d1750
00564b15  8945bc                MOV dword ptr [EBP + -0x44],EAX
00564b18  837dbc00              CMP dword ptr [EBP + -0x44],0x0
00564b1c  7441                  JZ 0x00564b5f
00564b1e  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
00564b21  8a4804                MOV CL,byte ptr [EAX + 0x4]
00564b24  c0e902                SHR CL,0x2
00564b27  80e103                AND CL,0x3
00564b2a  0fb6d1                MOVZX EDX,CL
00564b2d  83fa01                CMP EDX,0x1
00564b30  7424                  JZ 0x00564b56
00564b32  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
00564b35  8a4804                MOV CL,byte ptr [EAX + 0x4]
00564b38  c0e904                SHR CL,0x4
00564b3b  80e101                AND CL,0x1
00564b3e  0fb6d1                MOVZX EDX,CL
00564b41  83fa01                CMP EDX,0x1
00564b44  7517                  JNZ 0x00564b5d
00564b46  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
00564b49  8a4804                MOV CL,byte ptr [EAX + 0x4]
00564b4c  80e101                AND CL,0x1
00564b4f  0fb6d1                MOVZX EDX,CL
00564b52  85d2                  TEST EDX,EDX
00564b54  7507                  JNZ 0x00564b5d
00564b56  33c0                  XOR EAX,EAX
00564b58  e9a1070000            JMP 0x005652fe
00564b5d  eb07                  JMP 0x00564b66
00564b5f  33c0                  XOR EAX,EAX
00564b61  e998070000            JMP 0x005652fe
00564b66  a1a4b4a100            MOV EAX,[0x00a1b4a4]
00564b6b  8b4808                MOV ECX,dword ptr [EAX + 0x8]
00564b6e  898d4cffffff          MOV dword ptr [EBP + 0xffffff4c],ECX
00564b74  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00564b77  52                    PUSH EDX
00564b78  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00564b7e  e85d75fbff            CALL 0x0051c0e0
00564b83  8945ec                MOV dword ptr [EBP + -0x14],EAX
00564b86  837dec00              CMP dword ptr [EBP + -0x14],0x0
00564b8a  742f                  JZ 0x00564bbb
00564b8c  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00564b8f  50                    PUSH EAX
00564b90  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00564b93  e8c8f60900            CALL 0x00604260
00564b98  8945b8                MOV dword ptr [EBP + -0x48],EAX
00564b9b  837db800              CMP dword ptr [EBP + -0x48],0x0
00564b9f  7418                  JZ 0x00564bb9
00564ba1  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
00564ba4  668b5102              MOV DX,word ptr [ECX + 0x2]
00564ba8  6689954affffff        MOV word ptr [EBP + 0xffffff4a],DX
00564baf  0fb7854affffff        MOVZX EAX,word ptr [EBP + 0xffffff4a]
00564bb6  8945f8                MOV dword ptr [EBP + -0x8],EAX
00564bb9  eb07                  JMP 0x00564bc2
00564bbb  33c0                  XOR EAX,EAX
00564bbd  e93c070000            JMP 0x005652fe
00564bc2  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00564bc5  51                    PUSH ECX
00564bc6  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564bcc  e85f180100            CALL 0x00576430
00564bd1  8945f8                MOV dword ptr [EBP + -0x8],EAX
00564bd4  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
00564bda  899544ffffff          MOV dword ptr [EBP + 0xffffff44],EDX
00564be0  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00564be3  50                    PUSH EAX
00564be4  8b8d44ffffff          MOV ECX,dword ptr [EBP + 0xffffff44]
00564bea  8b8904010000          MOV ECX,dword ptr [ECX + 0x104]
00564bf0  e85bc81500            CALL 0x006c1450
00564bf5  8945c4                MOV dword ptr [EBP + -0x3c],EAX
00564bf8  837dc400              CMP dword ptr [EBP + -0x3c],0x0
00564bfc  7507                  JNZ 0x00564c05
00564bfe  33c0                  XOR EAX,EAX
00564c00  e9f9060000            JMP 0x005652fe
00564c05  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564c0b  e8b007feff            CALL 0x005453c0
00564c10  85c0                  TEST EAX,EAX
00564c12  7507                  JNZ 0x00564c1b
00564c14  33c0                  XOR EAX,EAX
00564c16  e9e3060000            JMP 0x005652fe
00564c1b  8b9500ffffff          MOV EDX,dword ptr [EBP + 0xffffff00]
00564c21  8b82ec000000          MOV EAX,dword ptr [EDX + 0xec]
00564c27  898540ffffff          MOV dword ptr [EBP + 0xffffff40],EAX
00564c2d  83bd40ffffff00        CMP dword ptr [EBP + 0xffffff40],0x0
00564c34  7507                  JNZ 0x00564c3d
00564c36  33c0                  XOR EAX,EAX
00564c38  e9c1060000            JMP 0x005652fe
00564c3d  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564c43  e87807feff            CALL 0x005453c0
00564c48  0520010000            ADD EAX,0x120
00564c4d  89853cffffff          MOV dword ptr [EBP + 0xffffff3c],EAX
00564c53  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
00564c59  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00564c5c  899538ffffff          MOV dword ptr [EBP + 0xffffff38],EDX
00564c62  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
00564c68  8945c8                MOV dword ptr [EBP + -0x38],EAX
00564c6b  8b4d20                MOV ECX,dword ptr [EBP + 0x20]
00564c6e  51                    PUSH ECX
00564c6f  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
00564c72  83c224                ADD EDX,0x24
00564c75  83ec08                SUB ESP,0x8
00564c78  8bcc                  MOV ECX,ESP
00564c7a  896584                MOV dword ptr [EBP + -0x7c],ESP
00564c7d  52                    PUSH EDX
00564c7e  e86de91c00            CALL 0x007335f0
00564c83  8985fcfeffff          MOV dword ptr [EBP + 0xfffffefc],EAX
00564c89  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564c8f  e83cf6ffff            CALL 0x005642d0
00564c94  d99df8feffff          FSTP float ptr [EBP + 0xfffffef8]
00564c9a  d985f8feffff          FLD float ptr [EBP + 0xfffffef8]
00564ca0  d95df0                FSTP float ptr [EBP + -0x10]
00564ca3  c745e401000000        MOV dword ptr [EBP + -0x1c],0x1
00564caa  c745fcffff0000        MOV dword ptr [EBP + -0x4],0xffff
00564cb1  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
00564cb8  c745e001000000        MOV dword ptr [EBP + -0x20],0x1
00564cbf  837d2c00              CMP dword ptr [EBP + 0x2c],0x0
00564cc3  0f859c020000          JNZ 0x00564f65
00564cc9  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
00564ccf  0594000000            ADD EAX,0x94
00564cd4  50                    PUSH EAX
00564cd5  8d4d14                LEA ECX,[EBP + 0x14]
00564cd8  51                    PUSH ECX
00564cd9  8d55ac                LEA EDX,[EBP + -0x54]
00564cdc  52                    PUSH EDX
00564cdd  e8ce05eeff            CALL 0x004452b0
00564ce2  83c40c                ADD ESP,0xc
00564ce5  d945ac                FLD float ptr [EBP + -0x54]
00564ce8  d84dac                FMUL float ptr [EBP + -0x54]
00564ceb  d945b0                FLD float ptr [EBP + -0x50]
00564cee  d84db0                FMUL float ptr [EBP + -0x50]
00564cf1  dec1                  FADDP
00564cf3  d945b4                FLD float ptr [EBP + -0x4c]
00564cf6  d84db4                FMUL float ptr [EBP + -0x4c]
00564cf9  dec1                  FADDP
00564cfb  d99d28ffffff          FSTP float ptr [EBP + 0xffffff28]
00564d01  d98528ffffff          FLD float ptr [EBP + 0xffffff28]
00564d07  d945f0                FLD float ptr [EBP + -0x10]
00564d0a  d84df0                FMUL float ptr [EBP + -0x10]
00564d0d  d99df4feffff          FSTP float ptr [EBP + 0xfffffef4]
00564d13  d985f4feffff          FLD float ptr [EBP + 0xfffffef4]
00564d19  ded9                  FCOMPP
00564d1b  dfe0                  FNSTSW AX
00564d1d  f6c405                TEST AH,0x5
00564d20  7a6b                  JP 0x00564d8d
00564d22  d9ee                  FLDZ
00564d24  d99d78ffffff          FSTP float ptr [EBP + 0xffffff78]
00564d2a  d9ee                  FLDZ
00564d2c  d99d7cffffff          FSTP float ptr [EBP + 0xffffff7c]
00564d32  d9ee                  FLDZ
00564d34  d95d80                FSTP float ptr [EBP + -0x80]
00564d37  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00564d3a  50                    PUSH EAX
00564d3b  6a00                  PUSH 0x0
00564d3d  6a00                  PUSH 0x0
00564d3f  8d8d78ffffff          LEA ECX,[EBP + 0xffffff78]
00564d45  51                    PUSH ECX
00564d46  6a00                  PUSH 0x0
00564d48  6a00                  PUSH 0x0
00564d4a  6a00                  PUSH 0x0
00564d4c  6a00                  PUSH 0x0
00564d4e  6a00                  PUSH 0x0
00564d50  6a00                  PUSH 0x0
00564d52  51                    PUSH ECX
00564d53  d9ee                  FLDZ
00564d55  d91c24                FSTP float ptr [ESP]
00564d58  51                    PUSH ECX
00564d59  d945f0                FLD float ptr [EBP + -0x10]
00564d5c  d91c24                FSTP float ptr [ESP]
00564d5f  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
00564d62  52                    PUSH EDX
00564d63  8b4520                MOV EAX,dword ptr [EBP + 0x20]
00564d66  50                    PUSH EAX
00564d67  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00564d6a  51                    PUSH ECX
00564d6b  8d5514                LEA EDX,[EBP + 0x14]
00564d6e  52                    PUSH EDX
00564d6f  0fb745fc              MOVZX EAX,word ptr [EBP + -0x4]
00564d73  50                    PUSH EAX
00564d74  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564d7a  e8f1f9ffff            CALL 0x00564770
00564d7f  c745dc01000000        MOV dword ptr [EBP + -0x24],0x1
00564d86  c745fcfeff0000        MOV dword ptr [EBP + -0x4],0xfffe
00564d8d  817d200000007f        CMP dword ptr [EBP + 0x20],0x7f000000
00564d94  0f84bc000000          JZ 0x00564e56
00564d9a  c745a800000000        MOV dword ptr [EBP + -0x58],0x0
00564da1  6a00                  PUSH 0x0
00564da3  6a00                  PUSH 0x0
00564da5  6a00                  PUSH 0x0
00564da7  6a00                  PUSH 0x0
00564da9  6a00                  PUSH 0x0
00564dab  6a00                  PUSH 0x0
00564dad  6a00                  PUSH 0x0
00564daf  6a00                  PUSH 0x0
00564db1  6a00                  PUSH 0x0
00564db3  6a00                  PUSH 0x0
00564db5  6a00                  PUSH 0x0
00564db7  6a00                  PUSH 0x0
00564db9  8d4df4                LEA ECX,[EBP + -0xc]
00564dbc  51                    PUSH ECX
00564dbd  6a01                  PUSH 0x1
00564dbf  8d55a8                LEA EDX,[EBP + -0x58]
00564dc2  52                    PUSH EDX
00564dc3  6a01                  PUSH 0x1
00564dc5  8d45e0                LEA EAX,[EBP + -0x20]
00564dc8  50                    PUSH EAX
00564dc9  6a01                  PUSH 0x1
00564dcb  8d4df0                LEA ECX,[EBP + -0x10]
00564dce  51                    PUSH ECX
00564dcf  6a02                  PUSH 0x2
00564dd1  8d55f0                LEA EDX,[EBP + -0x10]
00564dd4  52                    PUSH EDX
00564dd5  6a02                  PUSH 0x2
00564dd7  8d45e4                LEA EAX,[EBP + -0x1c]
00564dda  50                    PUSH EAX
00564ddb  6a01                  PUSH 0x1
00564ddd  8d4d20                LEA ECX,[EBP + 0x20]
00564de0  51                    PUSH ECX
00564de1  6a03                  PUSH 0x3
00564de3  0fb755fc              MOVZX EDX,word ptr [EBP + -0x4]
00564de7  52                    PUSH EDX
00564de8  6a11                  PUSH 0x11
00564dea  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564df0  e8fba9fdff            CALL 0x0053f7f0
00564df5  837ddc00              CMP dword ptr [EBP + -0x24],0x0
00564df9  750e                  JNZ 0x00564e09
00564dfb  c745dc01000000        MOV dword ptr [EBP + -0x24],0x1
00564e02  c745fcfeff0000        MOV dword ptr [EBP + -0x4],0xfffe
00564e09  6a00                  PUSH 0x0
00564e0b  6a00                  PUSH 0x0
00564e0d  6a00                  PUSH 0x0
00564e0f  6a00                  PUSH 0x0
00564e11  6a00                  PUSH 0x0
00564e13  6a00                  PUSH 0x0
00564e15  6a00                  PUSH 0x0
00564e17  6a00                  PUSH 0x0
00564e19  6a00                  PUSH 0x0
00564e1b  6a00                  PUSH 0x0
00564e1d  6a00                  PUSH 0x0
00564e1f  6a00                  PUSH 0x0
00564e21  6a00                  PUSH 0x0
00564e23  6a00                  PUSH 0x0
00564e25  6a00                  PUSH 0x0
00564e27  6a00                  PUSH 0x0
00564e29  6a00                  PUSH 0x0
00564e2b  6a00                  PUSH 0x0
00564e2d  6a00                  PUSH 0x0
00564e2f  6a00                  PUSH 0x0
00564e31  6a00                  PUSH 0x0
00564e33  6a00                  PUSH 0x0
00564e35  6a00                  PUSH 0x0
00564e37  6a00                  PUSH 0x0
00564e39  8d4520                LEA EAX,[EBP + 0x20]
00564e3c  50                    PUSH EAX
00564e3d  6a03                  PUSH 0x3
00564e3f  0fb74dfc              MOVZX ECX,word ptr [EBP + -0x4]
00564e43  51                    PUSH ECX
00564e44  6a13                  PUSH 0x13
00564e46  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564e4c  e89fa9fdff            CALL 0x0053f7f0
00564e51  e9b6000000            JMP 0x00564f0c
00564e56  6a00                  PUSH 0x0
00564e58  6a00                  PUSH 0x0
00564e5a  6a00                  PUSH 0x0
00564e5c  6a00                  PUSH 0x0
00564e5e  6a00                  PUSH 0x0
00564e60  6a00                  PUSH 0x0
00564e62  6a00                  PUSH 0x0
00564e64  6a00                  PUSH 0x0
00564e66  6a00                  PUSH 0x0
00564e68  6a00                  PUSH 0x0
00564e6a  8d55f4                LEA EDX,[EBP + -0xc]
00564e6d  52                    PUSH EDX
00564e6e  6a01                  PUSH 0x1
00564e70  8d45f0                LEA EAX,[EBP + -0x10]
00564e73  50                    PUSH EAX
00564e74  6a02                  PUSH 0x2
00564e76  8d4de4                LEA ECX,[EBP + -0x1c]
00564e79  51                    PUSH ECX
00564e7a  6a01                  PUSH 0x1
00564e7c  8d5520                LEA EDX,[EBP + 0x20]
00564e7f  52                    PUSH EDX
00564e80  6a03                  PUSH 0x3
00564e82  8d45c8                LEA EAX,[EBP + -0x38]
00564e85  50                    PUSH EAX
00564e86  6a03                  PUSH 0x3
00564e88  8d4dcc                LEA ECX,[EBP + -0x34]
00564e8b  51                    PUSH ECX
00564e8c  6a02                  PUSH 0x2
00564e8e  8d55d0                LEA EDX,[EBP + -0x30]
00564e91  52                    PUSH EDX
00564e92  6a02                  PUSH 0x2
00564e94  8d45d4                LEA EAX,[EBP + -0x2c]
00564e97  50                    PUSH EAX
00564e98  6a02                  PUSH 0x2
00564e9a  0fb74dfc              MOVZX ECX,word ptr [EBP + -0x4]
00564e9e  51                    PUSH ECX
00564e9f  6a12                  PUSH 0x12
00564ea1  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564ea7  e844a9fdff            CALL 0x0053f7f0
00564eac  837ddc00              CMP dword ptr [EBP + -0x24],0x0
00564eb0  750e                  JNZ 0x00564ec0
00564eb2  c745dc01000000        MOV dword ptr [EBP + -0x24],0x1
00564eb9  c745fcfeff0000        MOV dword ptr [EBP + -0x4],0xfffe
00564ec0  6a00                  PUSH 0x0
00564ec2  6a00                  PUSH 0x0
00564ec4  6a00                  PUSH 0x0
00564ec6  6a00                  PUSH 0x0
00564ec8  6a00                  PUSH 0x0
00564eca  6a00                  PUSH 0x0
00564ecc  6a00                  PUSH 0x0
00564ece  6a00                  PUSH 0x0
00564ed0  6a00                  PUSH 0x0
00564ed2  6a00                  PUSH 0x0
00564ed4  6a00                  PUSH 0x0
00564ed6  6a00                  PUSH 0x0
00564ed8  6a00                  PUSH 0x0
00564eda  6a00                  PUSH 0x0
00564edc  6a00                  PUSH 0x0
00564ede  6a00                  PUSH 0x0
00564ee0  6a00                  PUSH 0x0
00564ee2  6a00                  PUSH 0x0
00564ee4  6a00                  PUSH 0x0
00564ee6  6a00                  PUSH 0x0
00564ee8  8d55cc                LEA EDX,[EBP + -0x34]
00564eeb  52                    PUSH EDX
00564eec  6a02                  PUSH 0x2
00564eee  8d45d0                LEA EAX,[EBP + -0x30]
00564ef1  50                    PUSH EAX
00564ef2  6a02                  PUSH 0x2
00564ef4  8d4dd4                LEA ECX,[EBP + -0x2c]
00564ef7  51                    PUSH ECX
00564ef8  6a02                  PUSH 0x2
00564efa  0fb755fc              MOVZX EDX,word ptr [EBP + -0x4]
00564efe  52                    PUSH EDX
00564eff  6a31                  PUSH 0x31
00564f01  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564f07  e8e4a8fdff            CALL 0x0053f7f0
00564f0c  6a00                  PUSH 0x0
00564f0e  6a00                  PUSH 0x0
00564f10  6a00                  PUSH 0x0
00564f12  6a00                  PUSH 0x0
00564f14  6a00                  PUSH 0x0
00564f16  6a00                  PUSH 0x0
00564f18  6a00                  PUSH 0x0
00564f1a  6a00                  PUSH 0x0
00564f1c  6a00                  PUSH 0x0
00564f1e  6a00                  PUSH 0x0
00564f20  6a00                  PUSH 0x0
00564f22  6a00                  PUSH 0x0
00564f24  8d45cc                LEA EAX,[EBP + -0x34]
00564f27  50                    PUSH EAX
00564f28  6a02                  PUSH 0x2
00564f2a  8d4dd0                LEA ECX,[EBP + -0x30]
00564f2d  51                    PUSH ECX
00564f2e  6a02                  PUSH 0x2
00564f30  8d55d4                LEA EDX,[EBP + -0x2c]
00564f33  52                    PUSH EDX
00564f34  6a02                  PUSH 0x2
00564f36  8d4520                LEA EAX,[EBP + 0x20]
00564f39  50                    PUSH EAX
00564f3a  6a03                  PUSH 0x3
00564f3c  8d4d10                LEA ECX,[EBP + 0x10]
00564f3f  51                    PUSH ECX
00564f40  6a01                  PUSH 0x1
00564f42  8d550c                LEA EDX,[EBP + 0xc]
00564f45  52                    PUSH EDX
00564f46  6a01                  PUSH 0x1
00564f48  8d4508                LEA EAX,[EBP + 0x8]
00564f4b  50                    PUSH EAX
00564f4c  6a03                  PUSH 0x3
00564f4e  0fb74dfc              MOVZX ECX,word ptr [EBP + -0x4]
00564f52  51                    PUSH ECX
00564f53  6a2e                  PUSH 0x2e
00564f55  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564f5b  e890a8fdff            CALL 0x0053f7f0
00564f60  e93a030000            JMP 0x0056529f
00564f65  6a00                  PUSH 0x0
00564f67  6a00                  PUSH 0x0
00564f69  6a00                  PUSH 0x0
00564f6b  6a00                  PUSH 0x0
00564f6d  6a00                  PUSH 0x0
00564f6f  6a00                  PUSH 0x0
00564f71  6a00                  PUSH 0x0
00564f73  6a00                  PUSH 0x0
00564f75  6a00                  PUSH 0x0
00564f77  6a00                  PUSH 0x0
00564f79  6a00                  PUSH 0x0
00564f7b  6a00                  PUSH 0x0
00564f7d  8d55cc                LEA EDX,[EBP + -0x34]
00564f80  52                    PUSH EDX
00564f81  6a02                  PUSH 0x2
00564f83  8d45d0                LEA EAX,[EBP + -0x30]
00564f86  50                    PUSH EAX
00564f87  6a02                  PUSH 0x2
00564f89  8d4dd4                LEA ECX,[EBP + -0x2c]
00564f8c  51                    PUSH ECX
00564f8d  6a02                  PUSH 0x2
00564f8f  8d5520                LEA EDX,[EBP + 0x20]
00564f92  52                    PUSH EDX
00564f93  6a03                  PUSH 0x3
00564f95  8d4510                LEA EAX,[EBP + 0x10]
00564f98  50                    PUSH EAX
00564f99  6a01                  PUSH 0x1
00564f9b  8d4d0c                LEA ECX,[EBP + 0xc]
00564f9e  51                    PUSH ECX
00564f9f  6a01                  PUSH 0x1
00564fa1  8d5508                LEA EDX,[EBP + 0x8]
00564fa4  52                    PUSH EDX
00564fa5  6a03                  PUSH 0x3
00564fa7  0fb745fc              MOVZX EAX,word ptr [EBP + -0x4]
00564fab  50                    PUSH EAX
00564fac  6a2e                  PUSH 0x2e
00564fae  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00564fb4  e8e7bcfdff            CALL 0x00540ca0
00564fb9  817d200000007f        CMP dword ptr [EBP + 0x20],0x7f000000
00564fc0  0f84bc000000          JZ 0x00565082
00564fc6  837ddc00              CMP dword ptr [EBP + -0x24],0x0
00564fca  750e                  JNZ 0x00564fda
00564fcc  c745dc01000000        MOV dword ptr [EBP + -0x24],0x1
00564fd3  c745fcfeff0000        MOV dword ptr [EBP + -0x4],0xfffe
00564fda  6a00                  PUSH 0x0
00564fdc  6a00                  PUSH 0x0
00564fde  6a00                  PUSH 0x0
00564fe0  6a00                  PUSH 0x0
00564fe2  6a00                  PUSH 0x0
00564fe4  6a00                  PUSH 0x0
00564fe6  6a00                  PUSH 0x0
00564fe8  6a00                  PUSH 0x0
00564fea  6a00                  PUSH 0x0
00564fec  6a00                  PUSH 0x0
00564fee  6a00                  PUSH 0x0
00564ff0  6a00                  PUSH 0x0
00564ff2  6a00                  PUSH 0x0
00564ff4  6a00                  PUSH 0x0
00564ff6  6a00                  PUSH 0x0
00564ff8  6a00                  PUSH 0x0
00564ffa  6a00                  PUSH 0x0
00564ffc  6a00                  PUSH 0x0
00564ffe  6a00                  PUSH 0x0
00565000  6a00                  PUSH 0x0
00565002  6a00                  PUSH 0x0
00565004  6a00                  PUSH 0x0
00565006  6a00                  PUSH 0x0
00565008  6a00                  PUSH 0x0
0056500a  8d4d20                LEA ECX,[EBP + 0x20]
0056500d  51                    PUSH ECX
0056500e  6a03                  PUSH 0x3
00565010  0fb755fc              MOVZX EDX,word ptr [EBP + -0x4]
00565014  52                    PUSH EDX
00565015  6a13                  PUSH 0x13
00565017  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
0056501d  e87ebcfdff            CALL 0x00540ca0
00565022  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
00565029  6a00                  PUSH 0x0
0056502b  6a00                  PUSH 0x0
0056502d  6a00                  PUSH 0x0
0056502f  6a00                  PUSH 0x0
00565031  6a00                  PUSH 0x0
00565033  6a00                  PUSH 0x0
00565035  6a00                  PUSH 0x0
00565037  6a00                  PUSH 0x0
00565039  6a00                  PUSH 0x0
0056503b  6a00                  PUSH 0x0
0056503d  6a00                  PUSH 0x0
0056503f  6a00                  PUSH 0x0
00565041  8d45f4                LEA EAX,[EBP + -0xc]
00565044  50                    PUSH EAX
00565045  6a01                  PUSH 0x1
00565047  8d4d94                LEA ECX,[EBP + -0x6c]
0056504a  51                    PUSH ECX
0056504b  6a01                  PUSH 0x1
0056504d  8d55e0                LEA EDX,[EBP + -0x20]
00565050  52                    PUSH EDX
00565051  6a01                  PUSH 0x1
00565053  8d45f0                LEA EAX,[EBP + -0x10]
00565056  50                    PUSH EAX
00565057  6a02                  PUSH 0x2
00565059  8d4df0                LEA ECX,[EBP + -0x10]
0056505c  51                    PUSH ECX
0056505d  6a02                  PUSH 0x2
0056505f  8d55e4                LEA EDX,[EBP + -0x1c]
00565062  52                    PUSH EDX
00565063  6a01                  PUSH 0x1
00565065  8d4520                LEA EAX,[EBP + 0x20]
00565068  50                    PUSH EAX
00565069  6a03                  PUSH 0x3
0056506b  0fb74dfc              MOVZX ECX,word ptr [EBP + -0x4]
0056506f  51                    PUSH ECX
00565070  6a11                  PUSH 0x11
00565072  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00565078  e823bcfdff            CALL 0x00540ca0
0056507d  e9b6000000            JMP 0x00565138
00565082  837ddc00              CMP dword ptr [EBP + -0x24],0x0
00565086  750e                  JNZ 0x00565096
00565088  c745dc01000000        MOV dword ptr [EBP + -0x24],0x1
0056508f  c745fcfeff0000        MOV dword ptr [EBP + -0x4],0xfffe
00565096  6a00                  PUSH 0x0
00565098  6a00                  PUSH 0x0
0056509a  6a00                  PUSH 0x0
0056509c  6a00                  PUSH 0x0
0056509e  6a00                  PUSH 0x0
005650a0  6a00                  PUSH 0x0
005650a2  6a00                  PUSH 0x0
005650a4  6a00                  PUSH 0x0
005650a6  6a00                  PUSH 0x0
005650a8  6a00                  PUSH 0x0
005650aa  6a00                  PUSH 0x0
005650ac  6a00                  PUSH 0x0
005650ae  6a00                  PUSH 0x0
005650b0  6a00                  PUSH 0x0
005650b2  6a00                  PUSH 0x0
005650b4  6a00                  PUSH 0x0
005650b6  6a00                  PUSH 0x0
005650b8  6a00                  PUSH 0x0
005650ba  6a00                  PUSH 0x0
005650bc  6a00                  PUSH 0x0
005650be  8d55cc                LEA EDX,[EBP + -0x34]
005650c1  52                    PUSH EDX
005650c2  6a02                  PUSH 0x2
005650c4  8d45d0                LEA EAX,[EBP + -0x30]
005650c7  50                    PUSH EAX
005650c8  6a02                  PUSH 0x2
005650ca  8d4dd4                LEA ECX,[EBP + -0x2c]
005650cd  51                    PUSH ECX
005650ce  6a02                  PUSH 0x2
005650d0  0fb755fc              MOVZX EDX,word ptr [EBP + -0x4]
005650d4  52                    PUSH EDX
005650d5  6a31                  PUSH 0x31
005650d7  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
005650dd  e8bebbfdff            CALL 0x00540ca0
005650e2  6a00                  PUSH 0x0
005650e4  6a00                  PUSH 0x0
005650e6  6a00                  PUSH 0x0
005650e8  6a00                  PUSH 0x0
005650ea  6a00                  PUSH 0x0
005650ec  6a00                  PUSH 0x0
005650ee  6a00                  PUSH 0x0
005650f0  6a00                  PUSH 0x0
005650f2  6a00                  PUSH 0x0
005650f4  6a00                  PUSH 0x0
005650f6  8d45f4                LEA EAX,[EBP + -0xc]
005650f9  50                    PUSH EAX
005650fa  6a01                  PUSH 0x1
005650fc  8d4df0                LEA ECX,[EBP + -0x10]
005650ff  51                    PUSH ECX
00565100  6a02                  PUSH 0x2
00565102  8d55e4                LEA EDX,[EBP + -0x1c]
00565105  52                    PUSH EDX
00565106  6a01                  PUSH 0x1
00565108  8d4520                LEA EAX,[EBP + 0x20]
0056510b  50                    PUSH EAX
0056510c  6a03                  PUSH 0x3
0056510e  8d4dc8                LEA ECX,[EBP + -0x38]
00565111  51                    PUSH ECX
00565112  6a03                  PUSH 0x3
00565114  8d55cc                LEA EDX,[EBP + -0x34]
00565117  52                    PUSH EDX
00565118  6a02                  PUSH 0x2
0056511a  8d45d0                LEA EAX,[EBP + -0x30]
0056511d  50                    PUSH EAX
0056511e  6a02                  PUSH 0x2
00565120  8d4dd4                LEA ECX,[EBP + -0x2c]
00565123  51                    PUSH ECX
00565124  6a02                  PUSH 0x2
00565126  0fb755fc              MOVZX EDX,word ptr [EBP + -0x4]
0056512a  52                    PUSH EDX
0056512b  6a12                  PUSH 0x12
0056512d  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
00565133  e868bbfdff            CALL 0x00540ca0
00565138  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
0056513e  0594000000            ADD EAX,0x94
00565143  50                    PUSH EAX
00565144  8d4d14                LEA ECX,[EBP + 0x14]
00565147  51                    PUSH ECX
00565148  8d559c                LEA EDX,[EBP + -0x64]
0056514b  52                    PUSH EDX
0056514c  e85f01eeff            CALL 0x004452b0
00565151  83c40c                ADD ESP,0xc
00565154  d945a4                FLD float ptr [EBP + -0x5c]
00565157  d84da4                FMUL float ptr [EBP + -0x5c]
0056515a  d945a0                FLD float ptr [EBP + -0x60]
0056515d  d84da0                FMUL float ptr [EBP + -0x60]
00565160  d9459c                FLD float ptr [EBP + -0x64]
00565163  d84d9c                FMUL float ptr [EBP + -0x64]
00565166  dec1                  FADDP
00565168  dec1                  FADDP
0056516a  d95d98                FSTP float ptr [EBP + -0x68]
0056516d  d94598                FLD float ptr [EBP + -0x68]
00565170  d945f0                FLD float ptr [EBP + -0x10]
00565173  d84df0                FMUL float ptr [EBP + -0x10]
00565176  d99df0feffff          FSTP float ptr [EBP + 0xfffffef0]
0056517c  d985f0feffff          FLD float ptr [EBP + 0xfffffef0]
00565182  ded9                  FCOMPP
00565184  dfe0                  FNSTSW AX
00565186  f6c405                TEST AH,0x5
00565189  0f8a10010000          JP 0x0056529f
0056518f  d9ee                  FLDZ
00565191  d99d6cffffff          FSTP float ptr [EBP + 0xffffff6c]
00565197  d9ee                  FLDZ
00565199  d99d70ffffff          FSTP float ptr [EBP + 0xffffff70]
0056519f  d9ee                  FLDZ
005651a1  d99d74ffffff          FSTP float ptr [EBP + 0xffffff74]
005651a7  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005651aa  50                    PUSH EAX
005651ab  6a00                  PUSH 0x0
005651ad  6a00                  PUSH 0x0
005651af  8d8d6cffffff          LEA ECX,[EBP + 0xffffff6c]
005651b5  51                    PUSH ECX
005651b6  6a00                  PUSH 0x0
005651b8  6a00                  PUSH 0x0
005651ba  6a00                  PUSH 0x0
005651bc  6a00                  PUSH 0x0
005651be  6a00                  PUSH 0x0
005651c0  6a00                  PUSH 0x0
005651c2  51                    PUSH ECX
005651c3  d9ee                  FLDZ
005651c5  d91c24                FSTP float ptr [ESP]
005651c8  51                    PUSH ECX
005651c9  d945f0                FLD float ptr [EBP + -0x10]
005651cc  d91c24                FSTP float ptr [ESP]
005651cf  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
005651d2  52                    PUSH EDX
005651d3  8b4520                MOV EAX,dword ptr [EBP + 0x20]
005651d6  50                    PUSH EAX
005651d7  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
005651da  51                    PUSH ECX
005651db  8d5514                LEA EDX,[EBP + 0x14]
005651de  52                    PUSH EDX
005651df  0fb745fc              MOVZX EAX,word ptr [EBP + -0x4]
005651e3  50                    PUSH EAX
005651e4  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
005651ea  e8c1f2ffff            CALL 0x005644b0
005651ef  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
005651f5  8b91ac110000          MOV EDX,dword ptr [ECX + 0x11ac]
005651fb  899518ffffff          MOV dword ptr [EBP + 0xffffff18],EDX
00565201  83bd18ffffff00        CMP dword ptr [EBP + 0xffffff18],0x0
00565208  0f8483000000          JZ 0x00565291
0056520e  d94598                FLD float ptr [EBP + -0x68]
00565211  dc1dd8c19800          FCOMP double ptr [0x0098c1d8]
00565217  dfe0                  FNSTSW AX
00565219  f6c441                TEST AH,0x41
0056521c  7573                  JNZ 0x00565291
0056521e  a1a4b4a100            MOV EAX,[0x00a1b4a4]
00565223  8b4804                MOV ECX,dword ptr [EAX + 0x4]
00565226  894d90                MOV dword ptr [EBP + -0x70],ECX
00565229  8b9500ffffff          MOV EDX,dword ptr [EBP + 0xffffff00]
0056522f  8b4204                MOV EAX,dword ptr [EDX + 0x4]
00565232  898514ffffff          MOV dword ptr [EBP + 0xffffff14],EAX
00565238  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
0056523e  51                    PUSH ECX
0056523f  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
00565242  e879ac1d00            CALL 0x0073fec0
00565247  50                    PUSH EAX
00565248  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0056524b  e840a91d00            CALL 0x0073fb90
00565250  8bc8                  MOV ECX,EAX
00565252  e889192800            CALL 0x007e6be0
00565257  89458c                MOV dword ptr [EBP + -0x74],EAX
0056525a  837d8c00              CMP dword ptr [EBP + -0x74],0x0
0056525e  7c31                  JL 0x00565291
00565260  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
00565263  e828a91d00            CALL 0x0073fb90
00565268  898510ffffff          MOV dword ptr [EBP + 0xffffff10],EAX
0056526e  8b558c                MOV EDX,dword ptr [EBP + -0x74]
00565271  69d288000000          IMUL EDX,EDX,0x88
00565277  8b8510ffffff          MOV EAX,dword ptr [EBP + 0xffffff10]
0056527d  8d4c1024              LEA ECX,[EAX + EDX*0x1 + 0x24]
00565281  894d88                MOV dword ptr [EBP + -0x78],ECX
00565284  68eda50000            PUSH 0xa5ed
00565289  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
0056528c  e88ffe2700            CALL 0x007e5120
00565291  c745dc01000000        MOV dword ptr [EBP + -0x24],0x1
00565298  c745fcfeff0000        MOV dword ptr [EBP + -0x4],0xfffe
0056529f  837d2800              CMP dword ptr [EBP + 0x28],0x0
005652a3  751d                  JNZ 0x005652c2
005652a5  8b5520                MOV EDX,dword ptr [EBP + 0x20]
005652a8  89950cffffff          MOV dword ptr [EBP + 0xffffff0c],EDX
005652ae  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
005652b4  8b8d0cffffff          MOV ECX,dword ptr [EBP + 0xffffff0c]
005652ba  898868050000          MOV dword ptr [EAX + 0x568],ECX
005652c0  eb37                  JMP 0x005652f9
005652c2  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
005652c8  e8f300feff            CALL 0x005453c0
005652cd  0520010000            ADD EAX,0x120
005652d2  898508ffffff          MOV dword ptr [EBP + 0xffffff08],EAX
005652d8  8b9508ffffff          MOV EDX,dword ptr [EBP + 0xffffff08]
005652de  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005652e1  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
005652e7  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
005652ed  8b9504ffffff          MOV EDX,dword ptr [EBP + 0xffffff04]
005652f3  899168050000          MOV dword ptr [ECX + 0x568],EDX
005652f9  b801000000            MOV EAX,0x1
005652fe  8be5                  MOV ESP,EBP
00565300  5d                    POP EBP
00565301  c22c00                RET 0x2c
