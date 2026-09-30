; FN 005b7ae0 FUN_005b7ae0
005b7ae0  55                    PUSH EBP
005b7ae1  8bec                  MOV EBP,ESP
005b7ae3  83ec68                SUB ESP,0x68
005b7ae6  894d98                MOV dword ptr [EBP + -0x68],ECX
005b7ae9  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
005b7af0  c645f300              MOV byte ptr [EBP + -0xd],0x0
005b7af4  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
005b7afb  837d0c01              CMP dword ptr [EBP + 0xc],0x1
005b7aff  0f85b0010000          JNZ 0x005b7cb5
005b7b05  8b4598                MOV EAX,dword ptr [EBP + -0x68]
005b7b08  8a8812050000          MOV CL,byte ptr [EAX + 0x512]
005b7b0e  884df3                MOV byte ptr [EBP + -0xd],CL
005b7b11  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7b15  83fa05                CMP EDX,0x5
005b7b18  756d                  JNZ 0x005b7b87
005b7b1a  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005b7b21  eb09                  JMP 0x005b7b2c
005b7b23  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005b7b26  83c001                ADD EAX,0x1
005b7b29  8945fc                MOV dword ptr [EBP + -0x4],EAX
005b7b2c  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7b2f  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005b7b35  8955e8                MOV dword ptr [EBP + -0x18],EDX
005b7b38  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005b7b3b  8a888d000000          MOV CL,byte ptr [EAX + 0x8d]
005b7b41  884de7                MOV byte ptr [EBP + -0x19],CL
005b7b44  0fb655e7              MOVZX EDX,byte ptr [EBP + -0x19]
005b7b48  3955fc                CMP dword ptr [EBP + -0x4],EDX
005b7b4b  7d02                  JGE 0x005b7b4f
005b7b4d  ebd4                  JMP 0x005b7b23
005b7b4f  8b4598                MOV EAX,dword ptr [EBP + -0x68]
005b7b52  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
005b7b58  894de0                MOV dword ptr [EBP + -0x20],ECX
005b7b5b  6a10                  PUSH 0x10
005b7b5d  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005b7b60  e8bb8a1100            CALL 0x006d0620
005b7b65  8945f4                MOV dword ptr [EBP + -0xc],EAX
005b7b68  837df400              CMP dword ptr [EBP + -0xc],0x0
005b7b6c  7414                  JZ 0x005b7b82
005b7b6e  6a00                  PUSH 0x0
005b7b70  68bc000000            PUSH 0xbc
005b7b75  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7b78  e823d2fcff            CALL 0x00584da0
005b7b7d  e96e040000            JMP 0x005b7ff0
005b7b82  e916010000            JMP 0x005b7c9d
005b7b87  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7b8b  83fa06                CMP EDX,0x6
005b7b8e  0f8584000000          JNZ 0x005b7c18
005b7b94  8b4598                MOV EAX,dword ptr [EBP + -0x68]
005b7b97  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
005b7b9d  894ddc                MOV dword ptr [EBP + -0x24],ECX
005b7ba0  6a10                  PUSH 0x10
005b7ba2  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
005b7ba5  e8768a1100            CALL 0x006d0620
005b7baa  8945f4                MOV dword ptr [EBP + -0xc],EAX
005b7bad  837df400              CMP dword ptr [EBP + -0xc],0x0
005b7bb1  744c                  JZ 0x005b7bff
005b7bb3  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
005b7bb6  8b420c                MOV EAX,dword ptr [EDX + 0xc]
005b7bb9  8945d8                MOV dword ptr [EBP + -0x28],EAX
005b7bbc  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
005b7bc2  894dd4                MOV dword ptr [EBP + -0x2c],ECX
005b7bc5  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
005b7bc8  52                    PUSH EDX
005b7bc9  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
005b7bcc  8b4834                MOV ECX,dword ptr [EAX + 0x34]
005b7bcf  e82cd61100            CALL 0x006d5200
005b7bd4  8945d0                MOV dword ptr [EBP + -0x30],EAX
005b7bd7  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
005b7bda  0fb6511a              MOVZX EDX,byte ptr [ECX + 0x1a]
005b7bde  33c0                  XOR EAX,EAX
005b7be0  85d2                  TEST EDX,EDX
005b7be2  0f9fc0                SETG AL
005b7be5  85c0                  TEST EAX,EAX
005b7be7  7514                  JNZ 0x005b7bfd
005b7be9  6a00                  PUSH 0x0
005b7beb  68bc000000            PUSH 0xbc
005b7bf0  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7bf3  e8a8d1fcff            CALL 0x00584da0
005b7bf8  e9f3030000            JMP 0x005b7ff0
005b7bfd  eb14                  JMP 0x005b7c13
005b7bff  6a00                  PUSH 0x0
005b7c01  68bc000000            PUSH 0xbc
005b7c06  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7c09  e892d1fcff            CALL 0x00584da0
005b7c0e  e9dd030000            JMP 0x005b7ff0
005b7c13  e985000000            JMP 0x005b7c9d
005b7c18  0fb64d08              MOVZX ECX,byte ptr [EBP + 0x8]
005b7c1c  83f902                CMP ECX,0x2
005b7c1f  7412                  JZ 0x005b7c33
005b7c21  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7c25  83fa03                CMP EDX,0x3
005b7c28  7409                  JZ 0x005b7c33
005b7c2a  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7c2e  83f801                CMP EAX,0x1
005b7c31  756a                  JNZ 0x005b7c9d
005b7c33  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7c36  8b9150110000          MOV EDX,dword ptr [ECX + 0x1150]
005b7c3c  8955cc                MOV dword ptr [EBP + -0x34],EDX
005b7c3f  6a10                  PUSH 0x10
005b7c41  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
005b7c44  e8d7891100            CALL 0x006d0620
005b7c49  8945f4                MOV dword ptr [EBP + -0xc],EAX
005b7c4c  837df400              CMP dword ptr [EBP + -0xc],0x0
005b7c50  744b                  JZ 0x005b7c9d
005b7c52  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005b7c55  8b480c                MOV ECX,dword ptr [EAX + 0xc]
005b7c58  894dc8                MOV dword ptr [EBP + -0x38],ECX
005b7c5b  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
005b7c61  8955c4                MOV dword ptr [EBP + -0x3c],EDX
005b7c64  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
005b7c67  50                    PUSH EAX
005b7c68  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b7c6b  8b4934                MOV ECX,dword ptr [ECX + 0x34]
005b7c6e  e88dd51100            CALL 0x006d5200
005b7c73  8945c0                MOV dword ptr [EBP + -0x40],EAX
005b7c76  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
005b7c79  0fb6421a              MOVZX EAX,byte ptr [EDX + 0x1a]
005b7c7d  33c9                  XOR ECX,ECX
005b7c7f  85c0                  TEST EAX,EAX
005b7c81  0f9fc1                SETG CL
005b7c84  83f901                CMP ECX,0x1
005b7c87  7514                  JNZ 0x005b7c9d
005b7c89  6a00                  PUSH 0x0
005b7c8b  68bc000000            PUSH 0xbc
005b7c90  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7c93  e808d1fcff            CALL 0x00584da0
005b7c98  e953030000            JMP 0x005b7ff0
005b7c9d  8b5598                MOV EDX,dword ptr [EBP + -0x68]
005b7ca0  8a4508                MOV AL,byte ptr [EBP + 0x8]
005b7ca3  888212050000          MOV byte ptr [EDX + 0x512],AL
005b7ca9  c745ec01000000        MOV dword ptr [EBP + -0x14],0x1
005b7cb0  e9a1010000            JMP 0x005b7e56
005b7cb5  0fb64d08              MOVZX ECX,byte ptr [EBP + 0x8]
005b7cb9  83f904                CMP ECX,0x4
005b7cbc  7536                  JNZ 0x005b7cf4
005b7cbe  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7cc2  8b4598                MOV EAX,dword ptr [EBP + -0x68]
005b7cc5  0fb68812050000        MOVZX ECX,byte ptr [EAX + 0x512]
005b7ccc  3bd1                  CMP EDX,ECX
005b7cce  7424                  JZ 0x005b7cf4
005b7cd0  8b5598                MOV EDX,dword ptr [EBP + -0x68]
005b7cd3  8a8212050000          MOV AL,byte ptr [EDX + 0x512]
005b7cd9  8845f3                MOV byte ptr [EBP + -0xd],AL
005b7cdc  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7cdf  8a5508                MOV DL,byte ptr [EBP + 0x8]
005b7ce2  889112050000          MOV byte ptr [ECX + 0x512],DL
005b7ce8  c745ec01000000        MOV dword ptr [EBP + -0x14],0x1
005b7cef  e962010000            JMP 0x005b7e56
005b7cf4  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7cf8  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7cfb  0fb69112050000        MOVZX EDX,byte ptr [ECX + 0x512]
005b7d02  3bc2                  CMP EAX,EDX
005b7d04  0f844c010000          JZ 0x005b7e56
005b7d0a  8b4598                MOV EAX,dword ptr [EBP + -0x68]
005b7d0d  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
005b7d13  894dbc                MOV dword ptr [EBP + -0x44],ECX
005b7d16  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
005b7d19  8b82840a0000          MOV EAX,dword ptr [EDX + 0xa84]
005b7d1f  8945b8                MOV dword ptr [EBP + -0x48],EAX
005b7d22  837db801              CMP dword ptr [EBP + -0x48],0x1
005b7d26  7511                  JNZ 0x005b7d39
005b7d28  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7d2b  8a5508                MOV DL,byte ptr [EBP + 0x8]
005b7d2e  889113050000          MOV byte ptr [ECX + 0x513],DL
005b7d34  e91d010000            JMP 0x005b7e56
005b7d39  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7d3d  83f805                CMP EAX,0x5
005b7d40  756d                  JNZ 0x005b7daf
005b7d42  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005b7d49  eb09                  JMP 0x005b7d54
005b7d4b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005b7d4e  83c101                ADD ECX,0x1
005b7d51  894dfc                MOV dword ptr [EBP + -0x4],ECX
005b7d54  8b5598                MOV EDX,dword ptr [EBP + -0x68]
005b7d57  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005b7d5d  8945b4                MOV dword ptr [EBP + -0x4c],EAX
005b7d60  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
005b7d63  8a918d000000          MOV DL,byte ptr [ECX + 0x8d]
005b7d69  8855b3                MOV byte ptr [EBP + -0x4d],DL
005b7d6c  0fb645b3              MOVZX EAX,byte ptr [EBP + -0x4d]
005b7d70  3945fc                CMP dword ptr [EBP + -0x4],EAX
005b7d73  7d02                  JGE 0x005b7d77
005b7d75  ebd4                  JMP 0x005b7d4b
005b7d77  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7d7a  8b9150110000          MOV EDX,dword ptr [ECX + 0x1150]
005b7d80  8955ac                MOV dword ptr [EBP + -0x54],EDX
005b7d83  6a10                  PUSH 0x10
005b7d85  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
005b7d88  e893881100            CALL 0x006d0620
005b7d8d  8945f4                MOV dword ptr [EBP + -0xc],EAX
005b7d90  837df400              CMP dword ptr [EBP + -0xc],0x0
005b7d94  7414                  JZ 0x005b7daa
005b7d96  6a00                  PUSH 0x0
005b7d98  68bc000000            PUSH 0xbc
005b7d9d  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7da0  e8fbcffcff            CALL 0x00584da0
005b7da5  e946020000            JMP 0x005b7ff0
005b7daa  e988000000            JMP 0x005b7e37
005b7daf  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7db3  83f806                CMP EAX,0x6
005b7db6  757f                  JNZ 0x005b7e37
005b7db8  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7dbb  8b9150110000          MOV EDX,dword ptr [ECX + 0x1150]
005b7dc1  8955a8                MOV dword ptr [EBP + -0x58],EDX
005b7dc4  6a10                  PUSH 0x10
005b7dc6  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005b7dc9  e852881100            CALL 0x006d0620
005b7dce  8945f4                MOV dword ptr [EBP + -0xc],EAX
005b7dd1  837df400              CMP dword ptr [EBP + -0xc],0x0
005b7dd5  744c                  JZ 0x005b7e23
005b7dd7  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005b7dda  8b480c                MOV ECX,dword ptr [EAX + 0xc]
005b7ddd  894da4                MOV dword ptr [EBP + -0x5c],ECX
005b7de0  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
005b7de6  8955a0                MOV dword ptr [EBP + -0x60],EDX
005b7de9  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
005b7dec  50                    PUSH EAX
005b7ded  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
005b7df0  8b4934                MOV ECX,dword ptr [ECX + 0x34]
005b7df3  e808d41100            CALL 0x006d5200
005b7df8  89459c                MOV dword ptr [EBP + -0x64],EAX
005b7dfb  8b559c                MOV EDX,dword ptr [EBP + -0x64]
005b7dfe  0fb6421a              MOVZX EAX,byte ptr [EDX + 0x1a]
005b7e02  33c9                  XOR ECX,ECX
005b7e04  85c0                  TEST EAX,EAX
005b7e06  0f9fc1                SETG CL
005b7e09  85c9                  TEST ECX,ECX
005b7e0b  7514                  JNZ 0x005b7e21
005b7e0d  6a00                  PUSH 0x0
005b7e0f  68bc000000            PUSH 0xbc
005b7e14  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7e17  e884cffcff            CALL 0x00584da0
005b7e1c  e9cf010000            JMP 0x005b7ff0
005b7e21  eb14                  JMP 0x005b7e37
005b7e23  6a00                  PUSH 0x0
005b7e25  68bc000000            PUSH 0xbc
005b7e2a  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7e2d  e86ecffcff            CALL 0x00584da0
005b7e32  e9b9010000            JMP 0x005b7ff0
005b7e37  8b5598                MOV EDX,dword ptr [EBP + -0x68]
005b7e3a  8a8212050000          MOV AL,byte ptr [EDX + 0x512]
005b7e40  8845f3                MOV byte ptr [EBP + -0xd],AL
005b7e43  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7e46  8a5508                MOV DL,byte ptr [EBP + 0x8]
005b7e49  889112050000          MOV byte ptr [ECX + 0x512],DL
005b7e4f  c745ec01000000        MOV dword ptr [EBP + -0x14],0x1
005b7e56  837dec01              CMP dword ptr [EBP + -0x14],0x1
005b7e5a  0f8590010000          JNZ 0x005b7ff0
005b7e60  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7e64  83f801                CMP EAX,0x1
005b7e67  7409                  JZ 0x005b7e72
005b7e69  0fb64df3              MOVZX ECX,byte ptr [EBP + -0xd]
005b7e6d  83f901                CMP ECX,0x1
005b7e70  7532                  JNZ 0x005b7ea4
005b7e72  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7e76  83fa01                CMP EDX,0x1
005b7e79  7513                  JNZ 0x005b7e8e
005b7e7b  6a00                  PUSH 0x0
005b7e7d  6a01                  PUSH 0x1
005b7e7f  6800040000            PUSH 0x400
005b7e84  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7e87  e814bffaff            CALL 0x00563da0
005b7e8c  eb11                  JMP 0x005b7e9f
005b7e8e  6a00                  PUSH 0x0
005b7e90  6a00                  PUSH 0x0
005b7e92  6800040000            PUSH 0x400
005b7e97  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7e9a  e801bffaff            CALL 0x00563da0
005b7e9f  e94c010000            JMP 0x005b7ff0
005b7ea4  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7ea8  83f802                CMP EAX,0x2
005b7eab  7409                  JZ 0x005b7eb6
005b7ead  0fb64df3              MOVZX ECX,byte ptr [EBP + -0xd]
005b7eb1  83f902                CMP ECX,0x2
005b7eb4  7532                  JNZ 0x005b7ee8
005b7eb6  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7eba  83fa02                CMP EDX,0x2
005b7ebd  7513                  JNZ 0x005b7ed2
005b7ebf  6a00                  PUSH 0x0
005b7ec1  6a01                  PUSH 0x1
005b7ec3  6800010000            PUSH 0x100
005b7ec8  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7ecb  e8d0befaff            CALL 0x00563da0
005b7ed0  eb11                  JMP 0x005b7ee3
005b7ed2  6a00                  PUSH 0x0
005b7ed4  6a00                  PUSH 0x0
005b7ed6  6800010000            PUSH 0x100
005b7edb  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7ede  e8bdbefaff            CALL 0x00563da0
005b7ee3  e908010000            JMP 0x005b7ff0
005b7ee8  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7eec  83f803                CMP EAX,0x3
005b7eef  7409                  JZ 0x005b7efa
005b7ef1  0fb64df3              MOVZX ECX,byte ptr [EBP + -0xd]
005b7ef5  83f903                CMP ECX,0x3
005b7ef8  7532                  JNZ 0x005b7f2c
005b7efa  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7efe  83fa03                CMP EDX,0x3
005b7f01  7513                  JNZ 0x005b7f16
005b7f03  6a00                  PUSH 0x0
005b7f05  6a01                  PUSH 0x1
005b7f07  6800020000            PUSH 0x200
005b7f0c  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7f0f  e88cbefaff            CALL 0x00563da0
005b7f14  eb11                  JMP 0x005b7f27
005b7f16  6a00                  PUSH 0x0
005b7f18  6a00                  PUSH 0x0
005b7f1a  6800020000            PUSH 0x200
005b7f1f  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7f22  e879befaff            CALL 0x00563da0
005b7f27  e9c4000000            JMP 0x005b7ff0
005b7f2c  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7f30  83f804                CMP EAX,0x4
005b7f33  7409                  JZ 0x005b7f3e
005b7f35  0fb64df3              MOVZX ECX,byte ptr [EBP + -0xd]
005b7f39  83f904                CMP ECX,0x4
005b7f3c  7532                  JNZ 0x005b7f70
005b7f3e  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7f42  83fa04                CMP EDX,0x4
005b7f45  7513                  JNZ 0x005b7f5a
005b7f47  6a00                  PUSH 0x0
005b7f49  6a01                  PUSH 0x1
005b7f4b  6800080000            PUSH 0x800
005b7f50  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7f53  e848befaff            CALL 0x00563da0
005b7f58  eb11                  JMP 0x005b7f6b
005b7f5a  6a00                  PUSH 0x0
005b7f5c  6a00                  PUSH 0x0
005b7f5e  6800080000            PUSH 0x800
005b7f63  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7f66  e835befaff            CALL 0x00563da0
005b7f6b  e980000000            JMP 0x005b7ff0
005b7f70  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7f74  83f805                CMP EAX,0x5
005b7f77  7409                  JZ 0x005b7f82
005b7f79  0fb64df3              MOVZX ECX,byte ptr [EBP + -0xd]
005b7f7d  83f905                CMP ECX,0x5
005b7f80  752f                  JNZ 0x005b7fb1
005b7f82  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7f86  83fa05                CMP EDX,0x5
005b7f89  7513                  JNZ 0x005b7f9e
005b7f8b  6a00                  PUSH 0x0
005b7f8d  6a01                  PUSH 0x1
005b7f8f  6800100000            PUSH 0x1000
005b7f94  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7f97  e804befaff            CALL 0x00563da0
005b7f9c  eb11                  JMP 0x005b7faf
005b7f9e  6a00                  PUSH 0x0
005b7fa0  6a00                  PUSH 0x0
005b7fa2  6800100000            PUSH 0x1000
005b7fa7  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7faa  e8f1bdfaff            CALL 0x00563da0
005b7faf  eb3f                  JMP 0x005b7ff0
005b7fb1  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
005b7fb5  83f806                CMP EAX,0x6
005b7fb8  7409                  JZ 0x005b7fc3
005b7fba  0fb64df3              MOVZX ECX,byte ptr [EBP + -0xd]
005b7fbe  83f906                CMP ECX,0x6
005b7fc1  752d                  JNZ 0x005b7ff0
005b7fc3  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
005b7fc7  83fa06                CMP EDX,0x6
005b7fca  7513                  JNZ 0x005b7fdf
005b7fcc  6a00                  PUSH 0x0
005b7fce  6a01                  PUSH 0x1
005b7fd0  6800200000            PUSH 0x2000
005b7fd5  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7fd8  e8c3bdfaff            CALL 0x00563da0
005b7fdd  eb11                  JMP 0x005b7ff0
005b7fdf  6a00                  PUSH 0x0
005b7fe1  6a00                  PUSH 0x0
005b7fe3  6800200000            PUSH 0x2000
005b7fe8  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
005b7feb  e8b0bdfaff            CALL 0x00563da0
005b7ff0  8be5                  MOV ESP,EBP
005b7ff2  5d                    POP EBP
005b7ff3  c20800                RET 0x8
