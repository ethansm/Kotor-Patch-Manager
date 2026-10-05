; FN 00540ca0 FUN_00540ca0
00540ca0  55                    PUSH EBP
00540ca1  8bec                  MOV EBP,ESP
00540ca3  6aff                  PUSH -0x1
00540ca5  68bb219800            PUSH 0x9821bb
00540caa  64a100000000          MOV EAX,FS:[0x0]
00540cb0  50                    PUSH EAX
00540cb1  83ec18                SUB ESP,0x18
00540cb4  a1201fa100            MOV EAX,[0x00a11f20]
00540cb9  33c5                  XOR EAX,EBP
00540cbb  50                    PUSH EAX
00540cbc  8d45f4                LEA EAX,[EBP + -0xc]
00540cbf  64a300000000          MOV FS:[0x0],EAX
00540cc5  894de0                MOV dword ptr [EBP + -0x20],ECX
00540cc8  6a74                  PUSH 0x74
00540cca  e8548a3d00            CALL 0x00919723
00540ccf  83c404                ADD ESP,0x4
00540cd2  8945e8                MOV dword ptr [EBP + -0x18],EAX
00540cd5  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00540cdc  837de800              CMP dword ptr [EBP + -0x18],0x0
00540ce0  740d                  JZ 0x00540cef
00540ce2  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00540ce5  e806daffff            CALL 0x0053e6f0
00540cea  8945dc                MOV dword ptr [EBP + -0x24],EAX
00540ced  eb07                  JMP 0x00540cf6
00540cef  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
00540cf6  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
00540cf9  8945ec                MOV dword ptr [EBP + -0x14],EAX
00540cfc  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00540d03  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00540d06  894df0                MOV dword ptr [EBP + -0x10],ECX
00540d09  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540d0c  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00540d0f  8902                  MOV dword ptr [EDX],EAX
00540d11  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00540d14  c7417001000000        MOV dword ptr [ECX + 0x70],0x1
00540d1b  0fb7550c              MOVZX EDX,word ptr [EBP + 0xc]
00540d1f  81faffff0000          CMP EDX,0xffff
00540d25  7511                  JNZ 0x00540d38
00540d27  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540d2a  e871eaffff            CALL 0x0053f7a0
00540d2f  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00540d32  6689416c              MOV word ptr [ECX + 0x6c],AX
00540d36  eb2f                  JMP 0x00540d67
00540d38  0fb7550c              MOVZX EDX,word ptr [EBP + 0xc]
00540d3c  81fafeff0000          CMP EDX,0xfffe
00540d42  7518                  JNZ 0x00540d5c
00540d44  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
00540d47  668b4816              MOV CX,word ptr [EAX + 0x16]
00540d4b  66894de6              MOV word ptr [EBP + -0x1a],CX
00540d4f  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540d52  668b45e6              MOV AX,word ptr [EBP + -0x1a]
00540d56  6689426c              MOV word ptr [EDX + 0x6c],AX
00540d5a  eb0b                  JMP 0x00540d67
00540d5c  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00540d5f  668b550c              MOV DX,word ptr [EBP + 0xc]
00540d63  6689516c              MOV word ptr [ECX + 0x6c],DX
00540d67  837d1000              CMP dword ptr [EBP + 0x10],0x0
00540d6b  0f8486010000          JZ 0x00540ef7
00540d71  8b4514                MOV EAX,dword ptr [EBP + 0x14]
00540d74  50                    PUSH EAX
00540d75  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
00540d78  51                    PUSH ECX
00540d79  6a00                  PUSH 0x0
00540d7b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540d7e  52                    PUSH EDX
00540d7f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540d82  e8f9ecffff            CALL 0x0053fa80
00540d87  837d1800              CMP dword ptr [EBP + 0x18],0x0
00540d8b  0f8466010000          JZ 0x00540ef7
00540d91  8b451c                MOV EAX,dword ptr [EBP + 0x1c]
00540d94  50                    PUSH EAX
00540d95  8b4d18                MOV ECX,dword ptr [EBP + 0x18]
00540d98  51                    PUSH ECX
00540d99  6a01                  PUSH 0x1
00540d9b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540d9e  52                    PUSH EDX
00540d9f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540da2  e8d9ecffff            CALL 0x0053fa80
00540da7  837d2000              CMP dword ptr [EBP + 0x20],0x0
00540dab  0f8446010000          JZ 0x00540ef7
00540db1  8b4524                MOV EAX,dword ptr [EBP + 0x24]
00540db4  50                    PUSH EAX
00540db5  8b4d20                MOV ECX,dword ptr [EBP + 0x20]
00540db8  51                    PUSH ECX
00540db9  6a02                  PUSH 0x2
00540dbb  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540dbe  52                    PUSH EDX
00540dbf  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540dc2  e8b9ecffff            CALL 0x0053fa80
00540dc7  837d2800              CMP dword ptr [EBP + 0x28],0x0
00540dcb  0f8426010000          JZ 0x00540ef7
00540dd1  8b452c                MOV EAX,dword ptr [EBP + 0x2c]
00540dd4  50                    PUSH EAX
00540dd5  8b4d28                MOV ECX,dword ptr [EBP + 0x28]
00540dd8  51                    PUSH ECX
00540dd9  6a03                  PUSH 0x3
00540ddb  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540dde  52                    PUSH EDX
00540ddf  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540de2  e899ecffff            CALL 0x0053fa80
00540de7  837d3000              CMP dword ptr [EBP + 0x30],0x0
00540deb  0f8406010000          JZ 0x00540ef7
00540df1  8b4534                MOV EAX,dword ptr [EBP + 0x34]
00540df4  50                    PUSH EAX
00540df5  8b4d30                MOV ECX,dword ptr [EBP + 0x30]
00540df8  51                    PUSH ECX
00540df9  6a04                  PUSH 0x4
00540dfb  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540dfe  52                    PUSH EDX
00540dff  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540e02  e879ecffff            CALL 0x0053fa80
00540e07  837d3800              CMP dword ptr [EBP + 0x38],0x0
00540e0b  0f84e6000000          JZ 0x00540ef7
00540e11  8b453c                MOV EAX,dword ptr [EBP + 0x3c]
00540e14  50                    PUSH EAX
00540e15  8b4d38                MOV ECX,dword ptr [EBP + 0x38]
00540e18  51                    PUSH ECX
00540e19  6a05                  PUSH 0x5
00540e1b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540e1e  52                    PUSH EDX
00540e1f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540e22  e859ecffff            CALL 0x0053fa80
00540e27  837d4000              CMP dword ptr [EBP + 0x40],0x0
00540e2b  0f84c6000000          JZ 0x00540ef7
00540e31  8b4544                MOV EAX,dword ptr [EBP + 0x44]
00540e34  50                    PUSH EAX
00540e35  8b4d40                MOV ECX,dword ptr [EBP + 0x40]
00540e38  51                    PUSH ECX
00540e39  6a06                  PUSH 0x6
00540e3b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540e3e  52                    PUSH EDX
00540e3f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540e42  e839ecffff            CALL 0x0053fa80
00540e47  837d4800              CMP dword ptr [EBP + 0x48],0x0
00540e4b  0f84a6000000          JZ 0x00540ef7
00540e51  8b454c                MOV EAX,dword ptr [EBP + 0x4c]
00540e54  50                    PUSH EAX
00540e55  8b4d48                MOV ECX,dword ptr [EBP + 0x48]
00540e58  51                    PUSH ECX
00540e59  6a07                  PUSH 0x7
00540e5b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540e5e  52                    PUSH EDX
00540e5f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540e62  e819ecffff            CALL 0x0053fa80
00540e67  837d5000              CMP dword ptr [EBP + 0x50],0x0
00540e6b  0f8486000000          JZ 0x00540ef7
00540e71  8b4554                MOV EAX,dword ptr [EBP + 0x54]
00540e74  50                    PUSH EAX
00540e75  8b4d50                MOV ECX,dword ptr [EBP + 0x50]
00540e78  51                    PUSH ECX
00540e79  6a08                  PUSH 0x8
00540e7b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540e7e  52                    PUSH EDX
00540e7f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540e82  e8f9ebffff            CALL 0x0053fa80
00540e87  837d5800              CMP dword ptr [EBP + 0x58],0x0
00540e8b  746a                  JZ 0x00540ef7
00540e8d  8b455c                MOV EAX,dword ptr [EBP + 0x5c]
00540e90  50                    PUSH EAX
00540e91  8b4d58                MOV ECX,dword ptr [EBP + 0x58]
00540e94  51                    PUSH ECX
00540e95  6a09                  PUSH 0x9
00540e97  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540e9a  52                    PUSH EDX
00540e9b  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540e9e  e8ddebffff            CALL 0x0053fa80
00540ea3  837d6000              CMP dword ptr [EBP + 0x60],0x0
00540ea7  744e                  JZ 0x00540ef7
00540ea9  8b4564                MOV EAX,dword ptr [EBP + 0x64]
00540eac  50                    PUSH EAX
00540ead  8b4d60                MOV ECX,dword ptr [EBP + 0x60]
00540eb0  51                    PUSH ECX
00540eb1  6a0a                  PUSH 0xa
00540eb3  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540eb6  52                    PUSH EDX
00540eb7  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540eba  e8c1ebffff            CALL 0x0053fa80
00540ebf  837d6800              CMP dword ptr [EBP + 0x68],0x0
00540ec3  7432                  JZ 0x00540ef7
00540ec5  8b456c                MOV EAX,dword ptr [EBP + 0x6c]
00540ec8  50                    PUSH EAX
00540ec9  8b4d68                MOV ECX,dword ptr [EBP + 0x68]
00540ecc  51                    PUSH ECX
00540ecd  6a0b                  PUSH 0xb
00540ecf  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540ed2  52                    PUSH EDX
00540ed3  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540ed6  e8a5ebffff            CALL 0x0053fa80
00540edb  837d7000              CMP dword ptr [EBP + 0x70],0x0
00540edf  7416                  JZ 0x00540ef7
00540ee1  8b4574                MOV EAX,dword ptr [EBP + 0x74]
00540ee4  50                    PUSH EAX
00540ee5  8b4d70                MOV ECX,dword ptr [EBP + 0x70]
00540ee8  51                    PUSH ECX
00540ee9  6a0c                  PUSH 0xc
00540eeb  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00540eee  52                    PUSH EDX
00540eef  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540ef2  e889ebffff            CALL 0x0053fa80
00540ef7  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00540efa  50                    PUSH EAX
00540efb  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540efe  81c100010000          ADD ECX,0x100
00540f04  e8079b0000            CALL 0x0054aa10
00540f09  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00540f0c  e8ffecffff            CALL 0x0053fc10
00540f11  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00540f14  64890d00000000        MOV dword ptr FS:[0x0],ECX
00540f1b  59                    POP ECX
00540f1c  8be5                  MOV ESP,EBP
00540f1e  5d                    POP EBP
00540f1f  c27000                RET 0x70
