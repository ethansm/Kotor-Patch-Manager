; FN 00739e60 FUN_00739e60
00739e60  55                    PUSH EBP
00739e61  8bec                  MOV EBP,ESP
00739e63  83ec14                SUB ESP,0x14
00739e66  894df0                MOV dword ptr [EBP + -0x10],ECX
00739e69  6a0c                  PUSH 0xc
00739e6b  e8b3f81d00            CALL 0x00919723
00739e70  83c404                ADD ESP,0x4
00739e73  8945f4                MOV dword ptr [EBP + -0xc],EAX
00739e76  837df400              CMP dword ptr [EBP + -0xc],0x0
00739e7a  7425                  JZ 0x00739ea1
00739e7c  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00739e7f  c7400800000000        MOV dword ptr [EAX + 0x8],0x0
00739e86  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00739e89  c7410400000000        MOV dword ptr [ECX + 0x4],0x0
00739e90  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00739e93  c70200000000          MOV dword ptr [EDX],0x0
00739e99  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00739e9c  8945ec                MOV dword ptr [EBP + -0x14],EAX
00739e9f  eb07                  JMP 0x00739ea8
00739ea1  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
00739ea8  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00739eab  894df8                MOV dword ptr [EBP + -0x8],ECX
00739eae  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
00739eb1  8955fc                MOV dword ptr [EBP + -0x4],EDX
00739eb4  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00739eb7  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00739eba  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00739ebd  8910                  MOV dword ptr [EAX],EDX
00739ebf  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00739ec2  c7400400000000        MOV dword ptr [EAX + 0x4],0x0
00739ec9  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00739ecc  83790400              CMP dword ptr [ECX + 0x4],0x0
00739ed0  740c                  JZ 0x00739ede
00739ed2  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00739ed5  8b4204                MOV EAX,dword ptr [EDX + 0x4]
00739ed8  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00739edb  894804                MOV dword ptr [EAX + 0x4],ECX
00739ede  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00739ee1  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00739ee4  894204                MOV dword ptr [EDX + 0x4],EAX
00739ee7  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00739eea  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00739eed  895108                MOV dword ptr [ECX + 0x8],EDX
00739ef0  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00739ef3  833800                CMP dword ptr [EAX],0x0
00739ef6  7508                  JNZ 0x00739f00
00739ef8  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00739efb  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00739efe  8911                  MOV dword ptr [ECX],EDX
00739f00  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00739f03  8b4808                MOV ECX,dword ptr [EAX + 0x8]
00739f06  83c101                ADD ECX,0x1
00739f09  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00739f0c  894a08                MOV dword ptr [EDX + 0x8],ECX
00739f0f  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00739f12  8be5                  MOV ESP,EBP
00739f14  5d                    POP EBP
00739f15  c20400                RET 0x4
