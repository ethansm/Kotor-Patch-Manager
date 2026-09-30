; FN 008c70b0 FUN_008c70b0
008c70b0  55                    PUSH EBP
008c70b1  8bec                  MOV EBP,ESP
008c70b3  6aff                  PUSH -0x1
008c70b5  6883ac9700            PUSH 0x97ac83
008c70ba  64a100000000          MOV EAX,FS:[0x0]
008c70c0  50                    PUSH EAX
008c70c1  83ec60                SUB ESP,0x60
008c70c4  a1201fa100            MOV EAX,[0x00a11f20]
008c70c9  33c5                  XOR EAX,EBP
008c70cb  50                    PUSH EAX
008c70cc  8d45f4                LEA EAX,[EBP + -0xc]
008c70cf  64a300000000          MOV FS:[0x0],EAX
008c70d5  894da0                MOV dword ptr [EBP + -0x60],ECX
008c70d8  6a54                  PUSH 0x54
008c70da  e844260500            CALL 0x00919723
008c70df  83c404                ADD ESP,0x4
008c70e2  8945dc                MOV dword ptr [EBP + -0x24],EAX
008c70e5  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008c70ec  837ddc00              CMP dword ptr [EBP + -0x24],0x0
008c70f0  7421                  JZ 0x008c7113
008c70f2  6a00                  PUSH 0x0
008c70f4  83ec10                SUB ESP,0x10
008c70f7  8bcc                  MOV ECX,ESP
008c70f9  8965d8                MOV dword ptr [EBP + -0x28],ESP
008c70fc  683c879a00            PUSH 0x9a873c
008c7101  e83a97e4ff            CALL 0x00710840
008c7106  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008c7109  e8323ce5ff            CALL 0x0071ad40
008c710e  89459c                MOV dword ptr [EBP + -0x64],EAX
008c7111  eb07                  JMP 0x008c711a
008c7113  c7459c00000000        MOV dword ptr [EBP + -0x64],0x0
008c711a  8b459c                MOV EAX,dword ptr [EBP + -0x64]
008c711d  8945e0                MOV dword ptr [EBP + -0x20],EAX
008c7120  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008c7127  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008c712a  894df0                MOV dword ptr [EBP + -0x10],ECX
008c712d  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008c7130  e89b52e5ff            CALL 0x0071c3d0
008c7135  c745ec01000000        MOV dword ptr [EBP + -0x14],0x1
008c713c  8d4dc4                LEA ECX,[EBP + -0x3c]
008c713f  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
008c7142  81c2d0180000          ADD EDX,0x18d0
008c7148  52                    PUSH EDX
008c7149  e8a2c4e6ff            CALL 0x007335f0
008c714e  894598                MOV dword ptr [EBP + -0x68],EAX
008c7151  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
008c7154  8b8868080000          MOV ECX,dword ptr [EAX + 0x868]
008c715a  894dc0                MOV dword ptr [EBP + -0x40],ECX
008c715d  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
008c7164  8d55c4                LEA EDX,[EBP + -0x3c]
008c7167  52                    PUSH EDX
008c7168  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008c716b  81c1f03e0000          ADD ECX,0x3ef0
008c7171  e84ac6e6ff            CALL 0x007337c0
008c7176  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008c717d  8d4dc4                LEA ECX,[EBP + -0x3c]
008c7180  e8fbc5e6ff            CALL 0x00733780
008c7185  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
008c7188  83b8cc18000000        CMP dword ptr [EAX + 0x18cc],0x0
008c718f  741f                  JZ 0x008c71b0
008c7191  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
008c7194  8b511c                MOV EDX,dword ptr [ECX + 0x1c]
008c7197  8955bc                MOV dword ptr [EBP + -0x44],EDX
008c719a  6a01                  PUSH 0x1
008c719c  6a03                  PUSH 0x3
008c719e  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
008c71a1  8b8868080000          MOV ECX,dword ptr [EAX + 0x868]
008c71a7  51                    PUSH ECX
008c71a8  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
008c71ab  e88093b4ff            CALL 0x00410530
008c71b0  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
008c71b3  81baa41800000000007f  CMP dword ptr [EDX + 0x18a4],0x7f000000
008c71bd  0f84ab000000          JZ 0x008c726e
008c71c3  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008c71c8  8b4808                MOV ECX,dword ptr [EAX + 0x8]
008c71cb  894db8                MOV dword ptr [EBP + -0x48],ECX
008c71ce  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
008c71d1  8b82a4180000          MOV EAX,dword ptr [EDX + 0x18a4]
008c71d7  50                    PUSH EAX
008c71d8  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
008c71db  e8004fc5ff            CALL 0x0051c0e0
008c71e0  8945e8                MOV dword ptr [EBP + -0x18],EAX
008c71e3  837de800              CMP dword ptr [EBP + -0x18],0x0
008c71e7  0f8481000000          JZ 0x008c726e
008c71ed  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
008c71f4  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008c71f7  51                    PUSH ECX
008c71f8  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008c71fb  e8d004d4ff            CALL 0x006076d0
008c7200  0fb6d0                MOVZX EDX,AL
008c7203  8955e4                MOV dword ptr [EBP + -0x1c],EDX
008c7206  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
008c7209  8b88c0180000          MOV ECX,dword ptr [EAX + 0x18c0]
008c720f  894db4                MOV dword ptr [EBP + -0x4c],ECX
008c7212  8b55b4                MOV EDX,dword ptr [EBP + -0x4c]
008c7215  8a45e4                MOV AL,byte ptr [EBP + -0x1c]
008c7218  8882780c0000          MOV byte ptr [EDX + 0xc78],AL
008c721e  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
008c7221  8b91c0180000          MOV EDX,dword ptr [ECX + 0x18c0]
008c7227  8955b0                MOV dword ptr [EBP + -0x50],EDX
008c722a  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
008c722d  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008c7230  89888c0c0000          MOV dword ptr [EAX + 0xc8c],ECX
008c7236  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
008c7239  8b82c0180000          MOV EAX,dword ptr [EDX + 0x18c0]
008c723f  8945ac                MOV dword ptr [EBP + -0x54],EAX
008c7242  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
008c7245  c781840c000001000000  MOV dword ptr [ECX + 0xc84],0x1
008c724f  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
008c7252  8b421c                MOV EAX,dword ptr [EDX + 0x1c]
008c7255  8945a8                MOV dword ptr [EBP + -0x58],EAX
008c7258  6a01                  PUSH 0x1
008c725a  6a03                  PUSH 0x3
008c725c  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
008c725f  8b91c0180000          MOV EDX,dword ptr [ECX + 0x18c0]
008c7265  52                    PUSH EDX
008c7266  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c7269  e8c292b4ff            CALL 0x00410530
008c726e  837dec00              CMP dword ptr [EBP + -0x14],0x0
008c7272  7408                  JZ 0x008c727c
008c7274  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
008c7277  e894020000            CALL 0x008c7510
008c727c  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
008c727f  8945cc                MOV dword ptr [EBP + -0x34],EAX
008c7282  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
008c7285  894dd0                MOV dword ptr [EBP + -0x30],ECX
008c7288  837dd000              CMP dword ptr [EBP + -0x30],0x0
008c728c  7413                  JZ 0x008c72a1
008c728e  6a01                  PUSH 0x1
008c7290  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
008c7293  8b02                  MOV EAX,dword ptr [EDX]
008c7295  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
008c7298  8b10                  MOV EDX,dword ptr [EAX]
008c729a  ffd2                  CALL EDX
008c729c  894594                MOV dword ptr [EBP + -0x6c],EAX
008c729f  eb07                  JMP 0x008c72a8
008c72a1  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
008c72a8  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
008c72af  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
008c72b2  83b8c818000000        CMP dword ptr [EAX + 0x18c8],0x0
008c72b9  7424                  JZ 0x008c72df
008c72bb  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
008c72be  81c198060000          ADD ECX,0x698
008c72c4  894da4                MOV dword ptr [EBP + -0x5c],ECX
008c72c7  33d2                  XOR EDX,EDX
008c72c9  83e201                AND EDX,0x1
008c72cc  d1e2                  SHL EDX,0x1
008c72ce  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
008c72d1  8b4848                MOV ECX,dword ptr [EAX + 0x48]
008c72d4  83e1fd                AND ECX,0xfffffffd
008c72d7  0bca                  OR ECX,EDX
008c72d9  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
008c72dc  894a48                MOV dword ptr [EDX + 0x48],ECX
008c72df  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
008c72e2  e8d980b4ff            CALL 0x0040f3c0
008c72e7  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c72ea  64890d00000000        MOV dword ptr FS:[0x0],ECX
008c72f1  59                    POP ECX
008c72f2  8be5                  MOV ESP,EBP
008c72f4  5d                    POP EBP
008c72f5  c3                    RET
