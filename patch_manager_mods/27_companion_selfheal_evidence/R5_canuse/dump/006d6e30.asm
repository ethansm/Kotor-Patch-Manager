; FN 006d6e30 FUN_006d6e30
006d6e30  55                    PUSH EBP
006d6e31  8bec                  MOV EBP,ESP
006d6e33  83ec0c                SUB ESP,0xc
006d6e36  894df4                MOV dword ptr [EBP + -0xc],ECX
006d6e39  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
006d6e3c  8b480c                MOV ECX,dword ptr [EAX + 0xc]
006d6e3f  894df8                MOV dword ptr [EBP + -0x8],ECX
006d6e42  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
006d6e48  8955fc                MOV dword ptr [EBP + -0x4],EDX
006d6e4b  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
006d6e4e  50                    PUSH EAX
006d6e4f  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
006d6e52  8b4934                MOV ECX,dword ptr [ECX + 0x34]
006d6e55  e8a6e3ffff            CALL 0x006d5200
006d6e5a  8be5                  MOV ESP,EBP
006d6e5c  5d                    POP EBP
006d6e5d  c3                    RET
