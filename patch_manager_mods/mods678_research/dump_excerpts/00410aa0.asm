; FN 00410aa0 FUN_00410aa0
00410aa0  55                    PUSH EBP
00410aa1  8bec                  MOV EBP,ESP
00410aa3  6aff                  PUSH -0x1
00410aa5  6858849400            PUSH 0x948458
00410aaa  64a100000000          MOV EAX,FS:[0x0]
00410ab0  50                    PUSH EAX
00410ab1  83ec7c                SUB ESP,0x7c
00410ab4  a1201fa100            MOV EAX,[0x00a11f20]
00410ab9  33c5                  XOR EAX,EBP
00410abb  50                    PUSH EAX
00410abc  8d45f4                LEA EAX,[EBP + -0xc]
00410abf  64a300000000          MOV FS:[0x0],EAX
00410ac5  894d98                MOV dword ptr [EBP + -0x68],ECX
00410ac8  8b4598                MOV EAX,dword ptr [EBP + -0x68]
00410acb  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00410ace  894868                MOV dword ptr [EAX + 0x68],ECX
00410ad1  837d0c00              CMP dword ptr [EBP + 0xc],0x0
00410ad5  0f84a0000000          JZ 0x00410b7b
00410adb  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00410ade  895594                MOV dword ptr [EBP + -0x6c],EDX
00410ae1  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
00410ae4  83e833                SUB EAX,0x33
00410ae7  894594                MOV dword ptr [EBP + -0x6c],EAX
00410aea  837d9405              CMP dword ptr [EBP + -0x6c],0x5
00410aee  0f8780000000          JA 0x00410b74
00410af4  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
00410af7  ff248dbc0f4100        JMP dword ptr [ECX*0x4 + 0x410fbc]
00410afe  837d0c01              CMP dword ptr [EBP + 0xc],0x1
00410b02  7509                  JNZ 0x00410b0d
00410b04  c745083e000000        MOV dword ptr [EBP + 0x8],0x3e
00410b0b  eb0d                  JMP 0x00410b1a
00410b0d  837d0cff              CMP dword ptr [EBP + 0xc],-0x1
00410b11  7507                  JNZ 0x00410b1a
00410b13  c745083d000000        MOV dword ptr [EBP + 0x8],0x3d
00410b1a  eb58                  JMP 0x00410b74
00410b1c  837d0c01              CMP dword ptr [EBP + 0xc],0x1
00410b20  7509                  JNZ 0x00410b2b
00410b22  c7450840000000        MOV dword ptr [EBP + 0x8],0x40
00410b29  eb0d                  JMP 0x00410b38
00410b2b  837d0cff              CMP dword ptr [EBP + 0xc],-0x1
00410b2f  7507                  JNZ 0x00410b38
00410b31  c745083f000000        MOV dword ptr [EBP + 0x8],0x3f
00410b38  eb3a                  JMP 0x00410b74
00410b3a  837d0c01              CMP dword ptr [EBP + 0xc],0x1
00410b3e  7509                  JNZ 0x00410b49
00410b40  c745083a000000        MOV dword ptr [EBP + 0x8],0x3a
00410b47  eb0d                  JMP 0x00410b56
00410b49  837d0cff              CMP dword ptr [EBP + 0xc],-0x1
00410b4d  7507                  JNZ 0x00410b56
00410b4f  c7450839000000        MOV dword ptr [EBP + 0x8],0x39
00410b56  eb1c                  JMP 0x00410b74
00410b58  837d0c01              CMP dword ptr [EBP + 0xc],0x1
00410b5c  7509                  JNZ 0x00410b67
00410b5e  c745083c000000        MOV dword ptr [EBP + 0x8],0x3c
00410b65  eb0d                  JMP 0x00410b74
00410b67  837d0cff              CMP dword ptr [EBP + 0xc],-0x1
00410b6b  7507                  JNZ 0x00410b74
00410b6d  c745083b000000        MOV dword ptr [EBP + 0x8],0x3b
00410b74  c7450c01000000        MOV dword ptr [EBP + 0xc],0x1
00410b7b  837d0c00              CMP dword ptr [EBP + 0xc],0x0
00410b7f  0f84d9000000          JZ 0x00410c5e
00410b85  8b1588b4a100          MOV EDX,dword ptr [0x00a1b488]
00410b8b  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
00410b8e  e86d563200            CALL 0x00736200
00410b93  8945e8                MOV dword ptr [EBP + -0x18],EAX
00410b96  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00410b99  894590                MOV dword ptr [EBP + -0x70],EAX
00410b9c  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
00410b9f  83e92f                SUB ECX,0x2f
00410ba2  894d90                MOV dword ptr [EBP + -0x70],ECX
00410ba5  837d9003              CMP dword ptr [EBP + -0x70],0x3
00410ba9  0f87af000000          JA 0x00410c5e
00410baf  8b5590                MOV EDX,dword ptr [EBP + -0x70]
00410bb2  ff2495d40f4100        JMP dword ptr [EDX*0x4 + 0x410fd4]
00410bb9  8b4598                MOV EAX,dword ptr [EBP + -0x68]
00410bbc  8a4872                MOV CL,byte ptr [EAX + 0x72]
00410bbf  884d8c                MOV byte ptr [EBP + -0x74],CL
00410bc2  807d8c00              CMP byte ptr [EBP + -0x74],0x0
00410bc6  7430                  JZ 0x00410bf8
00410bc8  807d8c2e              CMP byte ptr [EBP + -0x74],0x2e
00410bcc  7633                  JBE 0x00410c01
00410bce  807d8c30              CMP byte ptr [EBP + -0x74],0x30
00410bd2  7602                  JBE 0x00410bd6
00410bd4  eb2b                  JMP 0x00410c01
00410bd6  8b5598                MOV EDX,dword ptr [EBP + -0x68]
00410bd9  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
00410bdc  2b4264                SUB EAX,dword ptr [EDX + 0x64]
00410bdf  3d96000000            CMP EAX,0x96
00410be4  720b                  JC 0x00410bf1
00410be6  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410be9  8a5508                MOV DL,byte ptr [EBP + 0x8]
00410bec  885172                MOV byte ptr [ECX + 0x72],DL
00410bef  eb05                  JMP 0x00410bf6
00410bf1  e9b3030000            JMP 0x00410fa9
00410bf6  eb09                  JMP 0x00410c01
00410bf8  8b4598                MOV EAX,dword ptr [EBP + -0x68]
00410bfb  8a4d08                MOV CL,byte ptr [EBP + 0x8]
00410bfe  884872                MOV byte ptr [EAX + 0x72],CL
00410c01  8b5598                MOV EDX,dword ptr [EBP + -0x68]
00410c04  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
00410c07  894264                MOV dword ptr [EDX + 0x64],EAX
00410c0a  eb52                  JMP 0x00410c5e
00410c0c  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410c0f  8a5172                MOV DL,byte ptr [ECX + 0x72]
00410c12  885588                MOV byte ptr [EBP + -0x78],DL
00410c15  807d8800              CMP byte ptr [EBP + -0x78],0x0
00410c19  7431                  JZ 0x00410c4c
00410c1b  807d8830              CMP byte ptr [EBP + -0x78],0x30
00410c1f  7634                  JBE 0x00410c55
00410c21  807d8832              CMP byte ptr [EBP + -0x78],0x32
00410c25  7602                  JBE 0x00410c29
00410c27  eb2c                  JMP 0x00410c55
00410c29  8b4598                MOV EAX,dword ptr [EBP + -0x68]
00410c2c  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00410c2f  2b4864                SUB ECX,dword ptr [EAX + 0x64]
00410c32  81f996000000          CMP ECX,0x96
00410c38  720b                  JC 0x00410c45
00410c3a  8b5598                MOV EDX,dword ptr [EBP + -0x68]
00410c3d  8a4508                MOV AL,byte ptr [EBP + 0x8]
00410c40  884272                MOV byte ptr [EDX + 0x72],AL
00410c43  eb05                  JMP 0x00410c4a
00410c45  e95f030000            JMP 0x00410fa9
00410c4a  eb09                  JMP 0x00410c55
00410c4c  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410c4f  8a5508                MOV DL,byte ptr [EBP + 0x8]
00410c52  885172                MOV byte ptr [ECX + 0x72],DL
00410c55  8b4598                MOV EAX,dword ptr [EBP + -0x68]
00410c58  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00410c5b  894864                MOV dword ptr [EAX + 0x64],ECX
00410c5e  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00410c61  895584                MOV dword ptr [EBP + -0x7c],EDX
00410c64  8b4584                MOV EAX,dword ptr [EBP + -0x7c]
00410c67  2db4000000            SUB EAX,0xb4
00410c6c  894584                MOV dword ptr [EBP + -0x7c],EAX
00410c6f  837d842b              CMP dword ptr [EBP + -0x7c],0x2b
00410c73  774b                  JA 0x00410cc0
00410c75  8b4d84                MOV ECX,dword ptr [EBP + -0x7c]
00410c78  0fb69100104100        MOVZX EDX,byte ptr [ECX + 0x411000]
00410c7f  ff2495e40f4100        JMP dword ptr [EDX*0x4 + 0x410fe4]
00410c86  837d0c00              CMP dword ptr [EBP + 0xc],0x0
00410c8a  7407                  JZ 0x00410c93
00410c8c  c7450828000000        MOV dword ptr [EBP + 0x8],0x28
00410c93  eb2b                  JMP 0x00410cc0
00410c95  c7450827000000        MOV dword ptr [EBP + 0x8],0x27
00410c9c  eb22                  JMP 0x00410cc0
00410c9e  c745083d000000        MOV dword ptr [EBP + 0x8],0x3d
00410ca5  eb19                  JMP 0x00410cc0
00410ca7  c745083e000000        MOV dword ptr [EBP + 0x8],0x3e
00410cae  eb10                  JMP 0x00410cc0
00410cb0  c745083f000000        MOV dword ptr [EBP + 0x8],0x3f
00410cb7  eb07                  JMP 0x00410cc0
00410cb9  c7450840000000        MOV dword ptr [EBP + 0x8],0x40
00410cc0  8b4598                MOV EAX,dword ptr [EBP + -0x68]
00410cc3  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
00410cc9  894dc0                MOV dword ptr [EBP + -0x40],ECX
00410ccc  837dc000              CMP dword ptr [EBP + -0x40],0x0
00410cd0  7439                  JZ 0x00410d0b
00410cd2  8b5598                MOV EDX,dword ptr [EBP + -0x68]
00410cd5  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
00410cdb  8945bc                MOV dword ptr [EBP + -0x44],EAX
00410cde  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410ce1  8b9194000000          MOV EDX,dword ptr [ECX + 0x94]
00410ce7  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
00410cea  8b4c82fc              MOV ECX,dword ptr [EDX + EAX*0x4 + -0x4]
00410cee  894d80                MOV dword ptr [EBP + -0x80],ECX
00410cf1  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00410cf4  52                    PUSH EDX
00410cf5  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00410cf8  50                    PUSH EAX
00410cf9  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
00410cfc  8b11                  MOV EDX,dword ptr [ECX]
00410cfe  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
00410d01  8b423c                MOV EAX,dword ptr [EDX + 0x3c]
00410d04  ffd0                  CALL EAX
00410d06  e9f1000000            JMP 0x00410dfc
00410d0b  6a00                  PUSH 0x0
00410d0d  8d4ddc                LEA ECX,[EBP + -0x24]
00410d10  e82b994300            CALL 0x0084a640
00410d15  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00410d1c  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410d1f  8b918c000000          MOV EDX,dword ptr [ECX + 0x8c]
00410d25  8955b8                MOV dword ptr [EBP + -0x48],EDX
00410d28  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
00410d2b  50                    PUSH EAX
00410d2c  8d4ddc                LEA ECX,[EBP + -0x24]
00410d2f  e8bc014300            CALL 0x00840ef0
00410d34  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
00410d3b  eb09                  JMP 0x00410d46
00410d3d  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00410d40  83c101                ADD ECX,0x1
00410d43  894df0                MOV dword ptr [EBP + -0x10],ECX
00410d46  8b5598                MOV EDX,dword ptr [EBP + -0x68]
00410d49  8b828c000000          MOV EAX,dword ptr [EDX + 0x8c]
00410d4f  8945b4                MOV dword ptr [EBP + -0x4c],EAX
00410d52  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00410d55  3b4db4                CMP ECX,dword ptr [EBP + -0x4c]
00410d58  7d22                  JGE 0x00410d7c
00410d5a  8b5598                MOV EDX,dword ptr [EBP + -0x68]
00410d5d  8b8288000000          MOV EAX,dword ptr [EDX + 0x88]
00410d63  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00410d66  8d1488                LEA EDX,[EAX + ECX*0x4]
00410d69  8955b0                MOV dword ptr [EBP + -0x50],EDX
00410d6c  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00410d6f  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
00410d72  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
00410d75  8b12                  MOV EDX,dword ptr [EDX]
00410d77  891481                MOV dword ptr [ECX + EAX*0x4],EDX
00410d7a  ebc1                  JMP 0x00410d3d
00410d7c  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
00410d83  eb09                  JMP 0x00410d8e
00410d85  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00410d88  83c001                ADD EAX,0x1
00410d8b  8945f0                MOV dword ptr [EBP + -0x10],EAX
00410d8e  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00410d91  894dac                MOV dword ptr [EBP + -0x54],ECX
00410d94  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00410d97  3b55ac                CMP EDX,dword ptr [EBP + -0x54]
00410d9a  7d51                  JGE 0x00410ded
00410d9c  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00410d9f  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
00410da2  8d1481                LEA EDX,[ECX + EAX*0x4]
00410da5  8955a8                MOV dword ptr [EBP + -0x58],EDX
00410da8  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
00410dab  8b08                  MOV ECX,dword ptr [EAX]
00410dad  51                    PUSH ECX
00410dae  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410db1  81c188000000          ADD ECX,0x88
00410db7  e824350000            CALL 0x004142e0
00410dbc  83f8ff                CMP EAX,-0x1
00410dbf  742a                  JZ 0x00410deb
00410dc1  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00410dc4  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
00410dc7  8b0c90                MOV ECX,dword ptr [EAX + EDX*0x4]
00410dca  898d7cffffff          MOV dword ptr [EBP + 0xffffff7c],ECX
00410dd0  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00410dd3  52                    PUSH EDX
00410dd4  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00410dd7  50                    PUSH EAX
00410dd8  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
00410dde  8b11                  MOV EDX,dword ptr [ECX]
00410de0  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
00410de6  8b423c                MOV EAX,dword ptr [EDX + 0x3c]
00410de9  ffd0                  CALL EAX
00410deb  eb98                  JMP 0x00410d85
00410ded  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00410df4  8d4ddc                LEA ECX,[EBP + -0x24]
00410df7  e8b4cf2300            CALL 0x0064ddb0
00410dfc  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410dff  8b918c000000          MOV EDX,dword ptr [ECX + 0x8c]
00410e05  8955a4                MOV dword ptr [EBP + -0x5c],EDX
00410e08  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
00410e0b  83e801                SUB EAX,0x1
00410e0e  8945f0                MOV dword ptr [EBP + -0x10],EAX
00410e11  eb09                  JMP 0x00410e1c
00410e13  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00410e16  83e901                SUB ECX,0x1
00410e19  894df0                MOV dword ptr [EBP + -0x10],ECX
00410e1c  837df000              CMP dword ptr [EBP + -0x10],0x0
00410e20  0f8cb3000000          JL 0x00410ed9
00410e26  8b5598                MOV EDX,dword ptr [EBP + -0x68]
00410e29  8b8288000000          MOV EAX,dword ptr [EDX + 0x88]
00410e2f  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00410e32  8b1488                MOV EDX,dword ptr [EAX + ECX*0x4]
00410e35  8955ec                MOV dword ptr [EBP + -0x14],EDX
00410e38  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00410e3b  8b4848                MOV ECX,dword ptr [EAX + 0x48]
00410e3e  c1e908                SHR ECX,0x8
00410e41  83e103                AND ECX,0x3
00410e44  33d2                  XOR EDX,EDX
00410e46  83f902                CMP ECX,0x2
00410e49  0f94c2                SETZ DL
00410e4c  85d2                  TEST EDX,EDX
00410e4e  7440                  JZ 0x00410e90
00410e50  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00410e53  50                    PUSH EAX
00410e54  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410e57  e814f8ffff            CALL 0x00410670
00410e5c  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00410e5f  894dd4                MOV dword ptr [EBP + -0x2c],ECX
00410e62  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
00410e65  8955d8                MOV dword ptr [EBP + -0x28],EDX
00410e68  837dd800              CMP dword ptr [EBP + -0x28],0x0
00410e6c  7416                  JZ 0x00410e84
00410e6e  6a01                  PUSH 0x1
00410e70  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
00410e73  8b10                  MOV EDX,dword ptr [EAX]
00410e75  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
00410e78  8b02                  MOV EAX,dword ptr [EDX]
00410e7a  ffd0                  CALL EAX
00410e7c  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
00410e82  eb0a                  JMP 0x00410e8e
00410e84  c78578ffffff00000000  MOV dword ptr [EBP + 0xffffff78],0x0
00410e8e  eb44                  JMP 0x00410ed4
00410e90  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00410e93  8b5148                MOV EDX,dword ptr [ECX + 0x48]
00410e96  c1ea08                SHR EDX,0x8
00410e99  83e203                AND EDX,0x3
00410e9c  f7da                  NEG EDX
00410e9e  1bd2                  SBB EDX,EDX
00410ea0  f7da                  NEG EDX
00410ea2  7430                  JZ 0x00410ed4
00410ea4  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00410ea7  8b4848                MOV ECX,dword ptr [EAX + 0x48]
00410eaa  81e17fffffff          AND ECX,0xffffff7f
00410eb0  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00410eb3  894a48                MOV dword ptr [EDX + 0x48],ECX
00410eb6  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00410eb9  8b4848                MOV ECX,dword ptr [EAX + 0x48]
00410ebc  81e1fffcffff          AND ECX,0xfffffcff
00410ec2  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00410ec5  894a48                MOV dword ptr [EDX + 0x48],ECX
00410ec8  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00410ecb  50                    PUSH EAX
00410ecc  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
00410ecf  e89cf7ffff            CALL 0x00410670
00410ed4  e93affffff            JMP 0x00410e13
00410ed9  b902000000            MOV ECX,0x2
00410ede  8b1590b4a100          MOV EDX,dword ptr [0x00a1b490]
00410ee4  0fb64234              MOVZX EAX,byte ptr [EDX + 0x34]
00410ee8  23c1                  AND EAX,ECX
00410eea  0fb6c8                MOVZX ECX,AL
00410eed  85c9                  TEST ECX,ECX
00410eef  7450                  JZ 0x00410f41
00410ef1  68f0749800            PUSH 0x9874f0
00410ef6  8d4dcc                LEA ECX,[EBP + -0x34]
00410ef9  e872263200            CALL 0x00733570
00410efe  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
00410f05  8d55cc                LEA EDX,[EBP + -0x34]
00410f08  52                    PUSH EDX
00410f09  8b0d90b4a100          MOV ECX,dword ptr [0x00a1b490]
00410f0f  e8bc073000            CALL 0x007116d0
00410f14  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00410f1b  8d4dcc                LEA ECX,[EBP + -0x34]
00410f1e  e85d283200            CALL 0x00733780
00410f23  a190b4a100            MOV EAX,[0x00a1b490]
00410f28  8945a0                MOV dword ptr [EBP + -0x60],EAX
00410f2b  b902000000            MOV ECX,0x2
00410f30  f7d1                  NOT ECX
00410f32  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
00410f35  0fb64234              MOVZX EAX,byte ptr [EDX + 0x34]
00410f39  23c1                  AND EAX,ECX
00410f3b  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
00410f3e  884134                MOV byte ptr [ECX + 0x34],AL
00410f41  ba01000000            MOV EDX,0x1
00410f46  a190b4a100            MOV EAX,[0x00a1b490]
00410f4b  0fb64834              MOVZX ECX,byte ptr [EAX + 0x34]
00410f4f  23ca                  AND ECX,EDX
00410f51  0fb6d1                MOVZX EDX,CL
00410f54  85d2                  TEST EDX,EDX
00410f56  7451                  JZ 0x00410fa9
00410f58  68e0749800            PUSH 0x9874e0
00410f5d  8d4dc4                LEA ECX,[EBP + -0x3c]
00410f60  e80b263200            CALL 0x00733570
00410f65  c745fc02000000        MOV dword ptr [EBP + -0x4],0x2
00410f6c  8d45c4                LEA EAX,[EBP + -0x3c]
00410f6f  50                    PUSH EAX
00410f70  8b0d90b4a100          MOV ECX,dword ptr [0x00a1b490]
00410f76  e855073000            CALL 0x007116d0
00410f7b  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00410f82  8d4dc4                LEA ECX,[EBP + -0x3c]
00410f85  e8f6273200            CALL 0x00733780
00410f8a  8b0d90b4a100          MOV ECX,dword ptr [0x00a1b490]
00410f90  894d9c                MOV dword ptr [EBP + -0x64],ECX
00410f93  ba01000000            MOV EDX,0x1
00410f98  f7d2                  NOT EDX
00410f9a  8b459c                MOV EAX,dword ptr [EBP + -0x64]
00410f9d  0fb64834              MOVZX ECX,byte ptr [EAX + 0x34]
00410fa1  23ca                  AND ECX,EDX
00410fa3  8b559c                MOV EDX,dword ptr [EBP + -0x64]
00410fa6  884a34                MOV byte ptr [EDX + 0x34],CL
00410fa9  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00410fac  64890d00000000        MOV dword ptr FS:[0x0],ECX
00410fb3  59                    POP ECX
00410fb4  8be5                  MOV ESP,EBP
00410fb6  5d                    POP EBP
00410fb7  c20800                RET 0x8
