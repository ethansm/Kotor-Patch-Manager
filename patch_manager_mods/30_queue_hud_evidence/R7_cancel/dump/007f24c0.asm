; FN 007f24c0 FUN_007f24c0
007f24c0  55                    PUSH EBP
007f24c1  8bec                  MOV EBP,ESP
007f24c3  83ec0c                SUB ESP,0xc
007f24c6  894df4                MOV dword ptr [EBP + -0xc],ECX
007f24c9  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007f24ce  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007f24d1  894dfc                MOV dword ptr [EBP + -0x4],ECX
007f24d4  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007f24d7  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007f24da  8945f8                MOV dword ptr [EBP + -0x8],EAX
007f24dd  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007f24e0  51                    PUSH ECX
007f24e1  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007f24e4  e8c7a3d2ff            CALL 0x0051c8b0
007f24e9  50                    PUSH EAX
007f24ea  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007f24ed  e81ea6d2ff            CALL 0x0051cb10
007f24f2  8be5                  MOV ESP,EBP
007f24f4  5d                    POP EBP
007f24f5  c3                    RET
