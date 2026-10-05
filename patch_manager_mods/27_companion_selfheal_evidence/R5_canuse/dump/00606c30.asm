; FN 00606c30 FUN_00606c30
00606c30  55                    PUSH EBP
00606c31  8bec                  MOV EBP,ESP
00606c33  6aff                  PUSH -0x1
00606c35  68b84f9500            PUSH 0x954fb8
00606c3a  64a100000000          MOV EAX,FS:[0x0]
00606c40  50                    PUSH EAX
00606c41  83ec24                SUB ESP,0x24
00606c44  a1201fa100            MOV EAX,[0x00a11f20]
00606c49  33c5                  XOR EAX,EBP
00606c4b  50                    PUSH EAX
00606c4c  8d45f4                LEA EAX,[EBP + -0xc]
00606c4f  64a300000000          MOV FS:[0x0],EAX
00606c55  894dd0                MOV dword ptr [EBP + -0x30],ECX
00606c58  a1d0b4a100            MOV EAX,[0x00a1b4d0]
00606c5d  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
00606c63  894dd8                MOV dword ptr [EBP + -0x28],ECX
00606c66  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
00606c69  8b8218010000          MOV EAX,dword ptr [EDX + 0x118]
00606c6f  8945e8                MOV dword ptr [EBP + -0x18],EAX
00606c72  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
00606c79  eb09                  JMP 0x00606c84
00606c7b  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00606c7e  83c101                ADD ECX,0x1
00606c81  894dec                MOV dword ptr [EBP + -0x14],ECX
00606c84  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
00606c87  8b4224                MOV EAX,dword ptr [EDX + 0x24]
00606c8a  8945d4                MOV dword ptr [EBP + -0x2c],EAX
00606c8d  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00606c90  3b4dd4                CMP ECX,dword ptr [EBP + -0x2c]
00606c93  7d48                  JGE 0x00606cdd
00606c95  68b4759900            PUSH 0x9975b4
00606c9a  8d4ddc                LEA ECX,[EBP + -0x24]
00606c9d  e8cec81200            CALL 0x00733570
00606ca2  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00606ca9  8d55e4                LEA EDX,[EBP + -0x1c]
00606cac  52                    PUSH EDX
00606cad  8d45dc                LEA EAX,[EBP + -0x24]
00606cb0  50                    PUSH EAX
00606cb1  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00606cb4  51                    PUSH ECX
00606cb5  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00606cb8  e833501100            CALL 0x0071bcf0
00606cbd  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00606cc4  8d4ddc                LEA ECX,[EBP + -0x24]
00606cc7  e8b4ca1200            CALL 0x00733780
00606ccc  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00606ccf  e8acfdffff            CALL 0x00606a80
00606cd4  3b45e4                CMP EAX,dword ptr [EBP + -0x1c]
00606cd7  7702                  JA 0x00606cdb
00606cd9  eb02                  JMP 0x00606cdd
00606cdb  eb9e                  JMP 0x00606c7b
00606cdd  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00606ce0  83c201                ADD EDX,0x1
00606ce3  8855f3                MOV byte ptr [EBP + -0xd],DL
00606ce6  8a45f3                MOV AL,byte ptr [EBP + -0xd]
00606ce9  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00606cec  64890d00000000        MOV dword ptr FS:[0x0],ECX
00606cf3  59                    POP ECX
00606cf4  8be5                  MOV ESP,EBP
00606cf6  5d                    POP EBP
00606cf7  c3                    RET
