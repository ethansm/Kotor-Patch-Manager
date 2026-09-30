; FN 008c9c40 FUN_008c9c40
008c9c40  55                    PUSH EBP
008c9c41  8bec                  MOV EBP,ESP
008c9c43  6aff                  PUSH -0x1
008c9c45  684baf9700            PUSH 0x97af4b
008c9c4a  64a100000000          MOV EAX,FS:[0x0]
008c9c50  50                    PUSH EAX
008c9c51  83ec3c                SUB ESP,0x3c
008c9c54  a1201fa100            MOV EAX,[0x00a11f20]
008c9c59  33c5                  XOR EAX,EBP
008c9c5b  50                    PUSH EAX
008c9c5c  8d45f4                LEA EAX,[EBP + -0xc]
008c9c5f  64a300000000          MOV FS:[0x0],EAX
008c9c65  894dbc                MOV dword ptr [EBP + -0x44],ECX
008c9c68  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008c9c6d  8b4808                MOV ECX,dword ptr [EAX + 0x8]
008c9c70  894df0                MOV dword ptr [EBP + -0x10],ECX
008c9c73  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
008c9c76  8b8244030000          MOV EAX,dword ptr [EDX + 0x344]
008c9c7c  8945dc                MOV dword ptr [EBP + -0x24],EAX
008c9c7f  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
008c9c82  83c168                ADD ECX,0x68
008c9c85  894dcc                MOV dword ptr [EBP + -0x34],ECX
008c9c88  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
008c9c8b  0fb682d0020000        MOVZX EAX,byte ptr [EDX + 0x2d0]
008c9c92  d1e0                  SHL EAX,0x1
008c9c94  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
008c9c97  8b91a4020000          MOV EDX,dword ptr [ECX + 0x2a4]
008c9c9d  2bd0                  SUB EDX,EAX
008c9c9f  8955d8                MOV dword ptr [EBP + -0x28],EDX
008c9ca2  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
008c9ca9  eb09                  JMP 0x008c9cb4
008c9cab  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008c9cae  83c001                ADD EAX,0x1
008c9cb1  8945e8                MOV dword ptr [EBP + -0x18],EAX
008c9cb4  837de814              CMP dword ptr [EBP + -0x18],0x14
008c9cb8  0f8d39010000          JGE 0x008c9df7
008c9cbe  ba01000000            MOV EDX,0x1
008c9cc3  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008c9cc6  d3e2                  SHL EDX,CL
008c9cc8  8955ec                MOV dword ptr [EBP + -0x14],EDX
008c9ccb  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008c9cce  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
008c9cd4  894dc8                MOV dword ptr [EBP + -0x38],ECX
008c9cd7  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008c9cda  52                    PUSH EDX
008c9cdb  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008c9cde  e83d69e0ff            CALL 0x006d0620
008c9ce3  8945e0                MOV dword ptr [EBP + -0x20],EAX
008c9ce6  837de000              CMP dword ptr [EBP + -0x20],0x0
008c9cea  0f8402010000          JZ 0x008c9df2
008c9cf0  8b4510                MOV EAX,dword ptr [EBP + 0x10]
008c9cf3  50                    PUSH EAX
008c9cf4  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008c9cf7  e8d4d9d3ff            CALL 0x006076d0
008c9cfc  0fb6c8                MOVZX ECX,AL
008c9cff  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
008c9d02  0fb682780c0000        MOVZX EAX,byte ptr [EDX + 0xc78]
008c9d09  3bc8                  CMP ECX,EAX
008c9d0b  0f85e1000000          JNZ 0x008c9df2
008c9d11  68bc030000            PUSH 0x3bc
008c9d16  e808fa0400            CALL 0x00919723
008c9d1b  83c404                ADD ESP,0x4
008c9d1e  8945d0                MOV dword ptr [EBP + -0x30],EAX
008c9d21  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008c9d28  837dd000              CMP dword ptr [EBP + -0x30],0x0
008c9d2c  740d                  JZ 0x008c9d3b
008c9d2e  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
008c9d31  e84a620000            CALL 0x008cff80
008c9d36  8945b8                MOV dword ptr [EBP + -0x48],EAX
008c9d39  eb07                  JMP 0x008c9d42
008c9d3b  c745b800000000        MOV dword ptr [EBP + -0x48],0x0
008c9d42  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
008c9d45  894dd4                MOV dword ptr [EBP + -0x2c],ECX
008c9d48  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008c9d4f  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
008c9d52  8955e4                MOV dword ptr [EBP + -0x1c],EDX
008c9d55  6a00                  PUSH 0x0
008c9d57  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
008c9d5a  50                    PUSH EAX
008c9d5b  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008c9d5e  81c1e8000000          ADD ECX,0xe8
008c9d64  51                    PUSH ECX
008c9d65  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008c9d68  83c270                ADD EDX,0x70
008c9d6b  52                    PUSH EDX
008c9d6c  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008c9d6f  0560010000            ADD EAX,0x160
008c9d74  50                    PUSH EAX
008c9d75  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c9d78  8b11                  MOV EDX,dword ptr [ECX]
008c9d7a  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c9d7d  8b82a0000000          MOV EAX,dword ptr [EDX + 0xa0]
008c9d83  ffd0                  CALL EAX
008c9d85  6850868c00            PUSH 0x8c8650
008c9d8a  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
008c9d8d  51                    PUSH ECX
008c9d8e  6a00                  PUSH 0x0
008c9d90  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c9d93  e858edb4ff            CALL 0x00418af0
008c9d98  6860948c00            PUSH 0x8c9460
008c9d9d  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
008c9da0  52                    PUSH EDX
008c9da1  6a27                  PUSH 0x27
008c9da3  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c9da6  e845edb4ff            CALL 0x00418af0
008c9dab  6860948c00            PUSH 0x8c9460
008c9db0  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
008c9db3  50                    PUSH EAX
008c9db4  6a2d                  PUSH 0x2d
008c9db6  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c9db9  e832edb4ff            CALL 0x00418af0
008c9dbe  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008c9dc1  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008c9dc4  8955c4                MOV dword ptr [EBP + -0x3c],EDX
008c9dc7  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
008c9dca  8b4814                MOV ECX,dword ptr [EAX + 0x14]
008c9dcd  894dc0                MOV dword ptr [EBP + -0x40],ECX
008c9dd0  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008c9dd3  52                    PUSH EDX
008c9dd4  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
008c9dd7  50                    PUSH EAX
008c9dd8  6a01                  PUSH 0x1
008c9dda  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008c9ddd  51                    PUSH ECX
008c9dde  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c9de1  e89a690000            CALL 0x008d0780
008c9de6  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
008c9de9  52                    PUSH EDX
008c9dea  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
008c9ded  e86e4cf7ff            CALL 0x0083ea60
008c9df2  e9b4feffff            JMP 0x008c9cab
008c9df7  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c9dfa  64890d00000000        MOV dword ptr FS:[0x0],ECX
008c9e01  59                    POP ECX
008c9e02  8be5                  MOV ESP,EBP
008c9e04  5d                    POP EBP
008c9e05  c20c00                RET 0xc
