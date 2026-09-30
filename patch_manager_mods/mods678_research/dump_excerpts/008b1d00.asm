; FN 008b1d00 FUN_008b1d00
008b1d00  55                    PUSH EBP
008b1d01  8bec                  MOV EBP,ESP
008b1d03  83ec08                SUB ESP,0x8
008b1d06  894dfc                MOV dword ptr [EBP + -0x4],ECX
008b1d09  0fb64508              MOVZX EAX,byte ptr [EBP + 0x8]
008b1d0d  8945f8                MOV dword ptr [EBP + -0x8],EAX
008b1d10  837df803              CMP dword ptr [EBP + -0x8],0x3
008b1d14  0f871c010000          JA 0x008b1e36
008b1d1a  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
008b1d1d  ff248d881e8b00        JMP dword ptr [ECX*0x4 + 0x8b1e88]
008b1d24  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008b1d27  81c298000000          ADD EDX,0x98
008b1d2d  a1e8fda700            MOV EAX,[0x00a7fde8]
008b1d32  8902                  MOV dword ptr [EDX],EAX
008b1d34  8b0decfda700          MOV ECX,dword ptr [0x00a7fdec]
008b1d3a  894a04                MOV dword ptr [EDX + 0x4],ECX
008b1d3d  a1f0fda700            MOV EAX,[0x00a7fdf0]
008b1d42  894208                MOV dword ptr [EDX + 0x8],EAX
008b1d45  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008b1d48  81c1fc010000          ADD ECX,0x1fc
008b1d4e  8b15e8fda700          MOV EDX,dword ptr [0x00a7fde8]
008b1d54  8911                  MOV dword ptr [ECX],EDX
008b1d56  a1ecfda700            MOV EAX,[0x00a7fdec]
008b1d5b  894104                MOV dword ptr [ECX + 0x4],EAX
008b1d5e  8b15f0fda700          MOV EDX,dword ptr [0x00a7fdf0]
008b1d64  895108                MOV dword ptr [ECX + 0x8],EDX
008b1d67  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
008b1d6a  0510010000            ADD EAX,0x110
008b1d6f  8b0dacfda700          MOV ECX,dword ptr [0x00a7fdac]
008b1d75  8908                  MOV dword ptr [EAX],ECX
008b1d77  8b15b0fda700          MOV EDX,dword ptr [0x00a7fdb0]
008b1d7d  895004                MOV dword ptr [EAX + 0x4],EDX
008b1d80  8b0db4fda700          MOV ECX,dword ptr [0x00a7fdb4]
008b1d86  894808                MOV dword ptr [EAX + 0x8],ECX
008b1d89  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008b1d8c  81c274020000          ADD EDX,0x274
008b1d92  a1acfda700            MOV EAX,[0x00a7fdac]
008b1d97  8902                  MOV dword ptr [EDX],EAX
008b1d99  8b0db0fda700          MOV ECX,dword ptr [0x00a7fdb0]
008b1d9f  894a04                MOV dword ptr [EDX + 0x4],ECX
008b1da2  a1b4fda700            MOV EAX,[0x00a7fdb4]
008b1da7  894208                MOV dword ptr [EDX + 0x8],EAX
008b1daa  e987000000            JMP 0x008b1e36
008b1daf  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008b1db2  81c198000000          ADD ECX,0x98
008b1db8  8b15d0fda700          MOV EDX,dword ptr [0x00a7fdd0]
008b1dbe  8911                  MOV dword ptr [ECX],EDX
008b1dc0  a1d4fda700            MOV EAX,[0x00a7fdd4]
008b1dc5  894104                MOV dword ptr [ECX + 0x4],EAX
008b1dc8  8b15d8fda700          MOV EDX,dword ptr [0x00a7fdd8]
008b1dce  895108                MOV dword ptr [ECX + 0x8],EDX
008b1dd1  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
008b1dd4  05fc010000            ADD EAX,0x1fc
008b1dd9  8b0dd0fda700          MOV ECX,dword ptr [0x00a7fdd0]
008b1ddf  8908                  MOV dword ptr [EAX],ECX
008b1de1  8b15d4fda700          MOV EDX,dword ptr [0x00a7fdd4]
008b1de7  895004                MOV dword ptr [EAX + 0x4],EDX
008b1dea  8b0dd8fda700          MOV ECX,dword ptr [0x00a7fdd8]
008b1df0  894808                MOV dword ptr [EAX + 0x8],ECX
008b1df3  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008b1df6  81c210010000          ADD EDX,0x110
008b1dfc  a1d0fda700            MOV EAX,[0x00a7fdd0]
008b1e01  8902                  MOV dword ptr [EDX],EAX
008b1e03  8b0dd4fda700          MOV ECX,dword ptr [0x00a7fdd4]
008b1e09  894a04                MOV dword ptr [EDX + 0x4],ECX
008b1e0c  a1d8fda700            MOV EAX,[0x00a7fdd8]
008b1e11  894208                MOV dword ptr [EDX + 0x8],EAX
008b1e14  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008b1e17  81c174020000          ADD ECX,0x274
008b1e1d  8b15d0fda700          MOV EDX,dword ptr [0x00a7fdd0]
008b1e23  8911                  MOV dword ptr [ECX],EDX
008b1e25  a1d4fda700            MOV EAX,[0x00a7fdd4]
008b1e2a  894104                MOV dword ptr [ECX + 0x4],EAX
008b1e2d  8b15d8fda700          MOV EDX,dword ptr [0x00a7fdd8]
008b1e33  895108                MOV dword ptr [ECX + 0x8],EDX
008b1e36  6a00                  PUSH 0x0
008b1e38  6a01                  PUSH 0x1
008b1e3a  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
008b1e3d  8b88ac030000          MOV ECX,dword ptr [EAX + 0x3ac]
008b1e43  83e101                AND ECX,0x1
008b1e46  51                    PUSH ECX
008b1e47  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008b1e4a  81c1fc000000          ADD ECX,0xfc
008b1e50  e89b2bb6ff            CALL 0x004149f0
008b1e55  6a01                  PUSH 0x1
008b1e57  6a00                  PUSH 0x0
008b1e59  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008b1e5c  8b82ac030000          MOV EAX,dword ptr [EDX + 0x3ac]
008b1e62  83e001                AND EAX,0x1
008b1e65  50                    PUSH EAX
008b1e66  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008b1e69  81c160020000          ADD ECX,0x260
008b1e6f  e87c2bb6ff            CALL 0x004149f0
008b1e74  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008b1e77  8a5508                MOV DL,byte ptr [EBP + 0x8]
008b1e7a  8891b0030000          MOV byte ptr [ECX + 0x3b0],DL
008b1e80  8be5                  MOV ESP,EBP
008b1e82  5d                    POP EBP
008b1e83  c20400                RET 0x4
