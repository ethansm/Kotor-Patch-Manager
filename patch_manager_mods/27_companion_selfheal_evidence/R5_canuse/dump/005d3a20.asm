; FN 005d3a20 FUN_005d3a20
005d3a20  55                    PUSH EBP
005d3a21  8bec                  MOV EBP,ESP
005d3a23  83ec34                SUB ESP,0x34
005d3a26  894dd0                MOV dword ptr [EBP + -0x30],ECX
005d3a29  6a00                  PUSH 0x0
005d3a2b  6a39                  PUSH 0x39
005d3a2d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3a30  e82b050300            CALL 0x00603f60
005d3a35  85c0                  TEST EAX,EAX
005d3a37  750a                  JNZ 0x005d3a43
005d3a39  b801000000            MOV EAX,0x1
005d3a3e  e90e010000            JMP 0x005d3b51
005d3a43  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
005d3a46  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d3a4c  894df4                MOV dword ptr [EBP + -0xc],ECX
005d3a4f  6a5d                  PUSH 0x5d
005d3a51  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005d3a54  e897490e00            CALL 0x006b83f0
005d3a59  85c0                  TEST EAX,EAX
005d3a5b  740a                  JZ 0x005d3a67
005d3a5d  b801000000            MOV EAX,0x1
005d3a62  e9ea000000            JMP 0x005d3b51
005d3a67  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005d3a6e  eb09                  JMP 0x005d3a79
005d3a70  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d3a73  83c201                ADD EDX,0x1
005d3a76  8955fc                MOV dword ptr [EBP + -0x4],EDX
005d3a79  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d3a7c  8b888c020000          MOV ECX,dword ptr [EAX + 0x28c]
005d3a82  894df0                MOV dword ptr [EBP + -0x10],ECX
005d3a85  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d3a88  3b55f0                CMP EDX,dword ptr [EBP + -0x10]
005d3a8b  0f8dbb000000          JGE 0x005d3b4c
005d3a91  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d3a94  50                    PUSH EAX
005d3a95  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3a98  e803080300            CALL 0x006042a0
005d3a9d  8945ec                MOV dword ptr [EBP + -0x14],EAX
005d3aa0  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
005d3aa3  8a5111                MOV DL,byte ptr [ECX + 0x11]
005d3aa6  8855eb                MOV byte ptr [EBP + -0x15],DL
005d3aa9  0fb645eb              MOVZX EAX,byte ptr [EBP + -0x15]
005d3aad  3dff000000            CMP EAX,0xff
005d3ab2  741f                  JZ 0x005d3ad3
005d3ab4  0fb64deb              MOVZX ECX,byte ptr [EBP + -0x15]
005d3ab8  ba01000000            MOV EDX,0x1
005d3abd  d3e2                  SHL EDX,CL
005d3abf  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d3ac2  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
005d3ac8  7509                  JNZ 0x005d3ad3
005d3aca  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
005d3ad1  eb07                  JMP 0x005d3ada
005d3ad3  c745cc01000000        MOV dword ptr [EBP + -0x34],0x1
005d3ada  837dcc00              CMP dword ptr [EBP + -0x34],0x0
005d3ade  7467                  JZ 0x005d3b47
005d3ae0  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d3ae3  51                    PUSH ECX
005d3ae4  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3ae7  e8b4070300            CALL 0x006042a0
005d3aec  8945e4                MOV dword ptr [EBP + -0x1c],EAX
005d3aef  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
005d3af2  668b02                MOV AX,word ptr [EDX]
005d3af5  668945e2              MOV word ptr [EBP + -0x1e],AX
005d3af9  0fb74de2              MOVZX ECX,word ptr [EBP + -0x1e]
005d3afd  83f939                CMP ECX,0x39
005d3b00  7545                  JNZ 0x005d3b47
005d3b02  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d3b05  52                    PUSH EDX
005d3b06  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3b09  e892070300            CALL 0x006042a0
005d3b0e  8945dc                MOV dword ptr [EBP + -0x24],EAX
005d3b11  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005d3b14  668b4802              MOV CX,word ptr [EAX + 0x2]
005d3b18  66894dda              MOV word ptr [EBP + -0x26],CX
005d3b1c  0fb755da              MOVZX EDX,word ptr [EBP + -0x26]
005d3b20  8955f8                MOV dword ptr [EBP + -0x8],EDX
005d3b23  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
005d3b26  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d3b2c  894dd4                MOV dword ptr [EBP + -0x2c],ECX
005d3b2f  0fb755f8              MOVZX EDX,word ptr [EBP + -0x8]
005d3b33  52                    PUSH EDX
005d3b34  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
005d3b37  e804490e00            CALL 0x006b8440
005d3b3c  0fb6c0                MOVZX EAX,AL
005d3b3f  85c0                  TEST EAX,EAX
005d3b41  7f04                  JG 0x005d3b47
005d3b43  33c0                  XOR EAX,EAX
005d3b45  eb0a                  JMP 0x005d3b51
005d3b47  e924ffffff            JMP 0x005d3a70
005d3b4c  b801000000            MOV EAX,0x1
005d3b51  8be5                  MOV ESP,EBP
005d3b53  5d                    POP EBP
005d3b54  c20400                RET 0x4
