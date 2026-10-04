; FN 007b3ca0 FUN_007b3ca0
007b3ca0  55                    PUSH EBP
007b3ca1  8bec                  MOV EBP,ESP
007b3ca3  83ec74                SUB ESP,0x74
007b3ca6  894d90                MOV dword ptr [EBP + -0x70],ECX
007b3ca9  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007b3cae  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007b3cb1  894dec                MOV dword ptr [EBP + -0x14],ECX
007b3cb4  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
007b3cb7  e894baf8ff            CALL 0x0073f750
007b3cbc  8945e8                MOV dword ptr [EBP + -0x18],EAX
007b3cbf  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
007b3cc2  8b82cc000000          MOV EAX,dword ptr [EDX + 0xcc]
007b3cc8  8945e4                MOV dword ptr [EBP + -0x1c],EAX
007b3ccb  837de400              CMP dword ptr [EBP + -0x1c],0x0
007b3ccf  0f85ff010000          JNZ 0x007b3ed4
007b3cd5  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3cd8  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007b3cdb  8955e0                MOV dword ptr [EBP + -0x20],EDX
007b3cde  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
007b3ce1  8b4830                MOV ECX,dword ptr [EAX + 0x30]
007b3ce4  894ddc                MOV dword ptr [EBP + -0x24],ECX
007b3ce7  837ddc00              CMP dword ptr [EBP + -0x24],0x0
007b3ceb  0f85e3010000          JNZ 0x007b3ed4
007b3cf1  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3cf4  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007b3cf7  8945d8                MOV dword ptr [EBP + -0x28],EAX
007b3cfa  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
007b3cfd  8a5175                MOV DL,byte ptr [ECX + 0x75]
007b3d00  8855d7                MOV byte ptr [EBP + -0x29],DL
007b3d03  0fb645d7              MOVZX EAX,byte ptr [EBP + -0x29]
007b3d07  83f805                CMP EAX,0x5
007b3d0a  0f84c4010000          JZ 0x007b3ed4
007b3d10  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3d13  8b91b8020000          MOV EDX,dword ptr [ECX + 0x2b8]
007b3d19  8955d0                MOV dword ptr [EBP + -0x30],EDX
007b3d1c  817dd00000007f        CMP dword ptr [EBP + -0x30],0x7f000000
007b3d23  0f8427010000          JZ 0x007b3e50
007b3d29  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3d2c  8b88b4020000          MOV ECX,dword ptr [EAX + 0x2b4]
007b3d32  894dcc                MOV dword ptr [EBP + -0x34],ECX
007b3d35  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3d38  8b82b0040000          MOV EAX,dword ptr [EDX + 0x4b0]
007b3d3e  8945c8                MOV dword ptr [EBP + -0x38],EAX
007b3d41  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
007b3d44  3b4dc8                CMP ECX,dword ptr [EBP + -0x38]
007b3d47  0f8503010000          JNZ 0x007b3e50
007b3d4d  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3d50  8b82b8020000          MOV EAX,dword ptr [EDX + 0x2b8]
007b3d56  8945c4                MOV dword ptr [EBP + -0x3c],EAX
007b3d59  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3d5c  8b91b4020000          MOV EDX,dword ptr [ECX + 0x2b4]
007b3d62  8955c0                MOV dword ptr [EBP + -0x40],EDX
007b3d65  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
007b3d68  3b45c0                CMP EAX,dword ptr [EBP + -0x40]
007b3d6b  0f85df000000          JNZ 0x007b3e50
007b3d71  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3d74  c781b80200000000007f  MOV dword ptr [ECX + 0x2b8],0x7f000000
007b3d7e  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
007b3d85  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3d88  8b82d0040000          MOV EAX,dword ptr [EDX + 0x4d0]
007b3d8e  8945bc                MOV dword ptr [EBP + -0x44],EAX
007b3d91  837dbc00              CMP dword ptr [EBP + -0x44],0x0
007b3d95  7e11                  JLE 0x007b3da8
007b3d97  33c9                  XOR ECX,ECX
007b3d99  6bc93c                IMUL ECX,ECX,0x3c
007b3d9c  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3d9f  038acc040000          ADD ECX,dword ptr [EDX + 0x4cc]
007b3da5  894dfc                MOV dword ptr [EBP + -0x4],ECX
007b3da8  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007b3dac  0f8499000000          JZ 0x007b3e4b
007b3db2  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007b3db7  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007b3dba  894db8                MOV dword ptr [EBP + -0x48],ECX
007b3dbd  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
007b3dc0  e8cbbdf8ff            CALL 0x0073fb90
007b3dc5  8bc8                  MOV ECX,EAX
007b3dc7  e8b41f0300            CALL 0x007e5d80
007b3dcc  8945f8                MOV dword ptr [EBP + -0x8],EAX
007b3dcf  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007b3dd5  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007b3dd8  8945b4                MOV dword ptr [EBP + -0x4c],EAX
007b3ddb  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007b3dde  8b511c                MOV EDX,dword ptr [ECX + 0x1c]
007b3de1  52                    PUSH EDX
007b3de2  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
007b3de5  e8e6b6f8ff            CALL 0x0073f4d0
007b3dea  8945f0                MOV dword ptr [EBP + -0x10],EAX
007b3ded  837df000              CMP dword ptr [EBP + -0x10],0x0
007b3df1  7412                  JZ 0x007b3e05
007b3df3  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
007b3df6  8b10                  MOV EDX,dword ptr [EAX]
007b3df8  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007b3dfb  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007b3dfe  ffd0                  CALL EAX
007b3e00  89458c                MOV dword ptr [EBP + -0x74],EAX
007b3e03  eb07                  JMP 0x007b3e0c
007b3e05  c7458c00000000        MOV dword ptr [EBP + -0x74],0x0
007b3e0c  8b4d8c                MOV ECX,dword ptr [EBP + -0x74]
007b3e0f  894df4                MOV dword ptr [EBP + -0xc],ECX
007b3e12  837df400              CMP dword ptr [EBP + -0xc],0x0
007b3e16  742c                  JZ 0x007b3e44
007b3e18  6a06                  PUSH 0x6
007b3e1a  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3e1d  8b8a74020000          MOV ECX,dword ptr [EDX + 0x274]
007b3e23  e878e4c5ff            CALL 0x004122a0
007b3e28  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
007b3e2b  50                    PUSH EAX
007b3e2c  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007b3e2f  8b5108                MOV EDX,dword ptr [ECX + 0x8]
007b3e32  52                    PUSH EDX
007b3e33  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007b3e36  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007b3e39  034810                ADD ECX,dword ptr [EAX + 0x10]
007b3e3c  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
007b3e3f  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007b3e42  ffd0                  CALL EAX
007b3e44  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
007b3e4b  e984000000            JMP 0x007b3ed4
007b3e50  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3e53  8b91b0040000          MOV EDX,dword ptr [ECX + 0x4b0]
007b3e59  8955b0                MOV dword ptr [EBP + -0x50],EDX
007b3e5c  817db00000007f        CMP dword ptr [EBP + -0x50],0x7f000000
007b3e63  741a                  JZ 0x007b3e7f
007b3e65  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3e68  8b88b0040000          MOV ECX,dword ptr [EAX + 0x4b0]
007b3e6e  894dac                MOV dword ptr [EBP + -0x54],ECX
007b3e71  6a00                  PUSH 0x0
007b3e73  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
007b3e76  52                    PUSH EDX
007b3e77  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3e7a  e8b1c6feff            CALL 0x007a0530
007b3e7f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007b3e84  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007b3e87  894da8                MOV dword ptr [EBP + -0x58],ECX
007b3e8a  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
007b3e8d  e8beb8f8ff            CALL 0x0073f750
007b3e92  8945a4                MOV dword ptr [EBP + -0x5c],EAX
007b3e95  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
007b3e98  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
007b3e9e  8945a0                MOV dword ptr [EBP + -0x60],EAX
007b3ea1  837da000              CMP dword ptr [EBP + -0x60],0x0
007b3ea5  742d                  JZ 0x007b3ed4
007b3ea7  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007b3ead  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007b3eb0  89559c                MOV dword ptr [EBP + -0x64],EDX
007b3eb3  8b4d9c                MOV ECX,dword ptr [EBP + -0x64]
007b3eb6  e895b8f8ff            CALL 0x0073f750
007b3ebb  894598                MOV dword ptr [EBP + -0x68],EAX
007b3ebe  8b4598                MOV EAX,dword ptr [EBP + -0x68]
007b3ec1  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007b3ec7  894d94                MOV dword ptr [EBP + -0x6c],ECX
007b3eca  6a01                  PUSH 0x1
007b3ecc  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
007b3ecf  e80c91f9ff            CALL 0x0074cfe0
007b3ed4  8be5                  MOV ESP,EBP
007b3ed6  5d                    POP EBP
007b3ed7  c3                    RET
