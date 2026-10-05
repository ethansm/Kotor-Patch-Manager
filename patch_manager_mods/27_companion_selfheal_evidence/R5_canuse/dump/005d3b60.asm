; FN 005d3b60 FUN_005d3b60
005d3b60  55                    PUSH EBP
005d3b61  8bec                  MOV EBP,ESP
005d3b63  83ec30                SUB ESP,0x30
005d3b66  894dd4                MOV dword ptr [EBP + -0x2c],ECX
005d3b69  6a00                  PUSH 0x0
005d3b6b  6a3c                  PUSH 0x3c
005d3b6d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3b70  e8eb030300            CALL 0x00603f60
005d3b75  85c0                  TEST EAX,EAX
005d3b77  750a                  JNZ 0x005d3b83
005d3b79  b801000000            MOV EAX,0x1
005d3b7e  e901010000            JMP 0x005d3c84
005d3b83  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
005d3b8a  eb09                  JMP 0x005d3b95
005d3b8c  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
005d3b8f  83c001                ADD EAX,0x1
005d3b92  8945f8                MOV dword ptr [EBP + -0x8],EAX
005d3b95  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3b98  8b918c020000          MOV EDX,dword ptr [ECX + 0x28c]
005d3b9e  8955f0                MOV dword ptr [EBP + -0x10],EDX
005d3ba1  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
005d3ba4  3b45f0                CMP EAX,dword ptr [EBP + -0x10]
005d3ba7  0f8dd2000000          JGE 0x005d3c7f
005d3bad  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
005d3bb0  51                    PUSH ECX
005d3bb1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3bb4  e8e7060300            CALL 0x006042a0
005d3bb9  8945ec                MOV dword ptr [EBP + -0x14],EAX
005d3bbc  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
005d3bbf  8a4211                MOV AL,byte ptr [EDX + 0x11]
005d3bc2  8845eb                MOV byte ptr [EBP + -0x15],AL
005d3bc5  0fb64deb              MOVZX ECX,byte ptr [EBP + -0x15]
005d3bc9  81f9ff000000          CMP ECX,0xff
005d3bcf  741f                  JZ 0x005d3bf0
005d3bd1  0fb64deb              MOVZX ECX,byte ptr [EBP + -0x15]
005d3bd5  ba01000000            MOV EDX,0x1
005d3bda  d3e2                  SHL EDX,CL
005d3bdc  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d3bdf  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
005d3be5  7509                  JNZ 0x005d3bf0
005d3be7  c745d000000000        MOV dword ptr [EBP + -0x30],0x0
005d3bee  eb07                  JMP 0x005d3bf7
005d3bf0  c745d001000000        MOV dword ptr [EBP + -0x30],0x1
005d3bf7  837dd000              CMP dword ptr [EBP + -0x30],0x0
005d3bfb  747d                  JZ 0x005d3c7a
005d3bfd  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
005d3c00  51                    PUSH ECX
005d3c01  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3c04  e897060300            CALL 0x006042a0
005d3c09  8945e4                MOV dword ptr [EBP + -0x1c],EAX
005d3c0c  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
005d3c0f  668b02                MOV AX,word ptr [EDX]
005d3c12  668945e2              MOV word ptr [EBP + -0x1e],AX
005d3c16  0fb74de2              MOVZX ECX,word ptr [EBP + -0x1e]
005d3c1a  83f93c                CMP ECX,0x3c
005d3c1d  755b                  JNZ 0x005d3c7a
005d3c1f  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
005d3c22  52                    PUSH EDX
005d3c23  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3c26  e875060300            CALL 0x006042a0
005d3c2b  8945dc                MOV dword ptr [EBP + -0x24],EAX
005d3c2e  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005d3c31  668b4802              MOV CX,word ptr [EAX + 0x2]
005d3c35  66894dda              MOV word ptr [EBP + -0x26],CX
005d3c39  0fb755da              MOVZX EDX,word ptr [EBP + -0x26]
005d3c3d  8955fc                MOV dword ptr [EBP + -0x4],EDX
005d3c40  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
005d3c43  8b10                  MOV EDX,dword ptr [EAX]
005d3c45  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
005d3c48  8b8288000000          MOV EAX,dword ptr [EDX + 0x88]
005d3c4e  ffd0                  CALL EAX
005d3c50  0fb6c8                MOVZX ECX,AL
005d3c53  894df4                MOV dword ptr [EBP + -0xc],ECX
005d3c56  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d3c59  3b55f4                CMP EDX,dword ptr [EBP + -0xc]
005d3c5c  741c                  JZ 0x005d3c7a
005d3c5e  837dfc00              CMP dword ptr [EBP + -0x4],0x0
005d3c62  7406                  JZ 0x005d3c6a
005d3c64  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d3c68  750c                  JNZ 0x005d3c76
005d3c6a  837df402              CMP dword ptr [EBP + -0xc],0x2
005d3c6e  7404                  JZ 0x005d3c74
005d3c70  33c0                  XOR EAX,EAX
005d3c72  eb10                  JMP 0x005d3c84
005d3c74  eb04                  JMP 0x005d3c7a
005d3c76  33c0                  XOR EAX,EAX
005d3c78  eb0a                  JMP 0x005d3c84
005d3c7a  e90dffffff            JMP 0x005d3b8c
005d3c7f  b801000000            MOV EAX,0x1
005d3c84  8be5                  MOV ESP,EBP
005d3c86  5d                    POP EBP
005d3c87  c20400                RET 0x4
