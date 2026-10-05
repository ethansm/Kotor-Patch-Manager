; FN 007a1720 FUN_007a1720
007a1720  55                    PUSH EBP
007a1721  8bec                  MOV EBP,ESP
007a1723  83ec0c                SUB ESP,0xc
007a1726  894df4                MOV dword ptr [EBP + -0xc],ECX
007a1729  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007a172c  8b08                  MOV ECX,dword ptr [EAX]
007a172e  894dfc                MOV dword ptr [EBP + -0x4],ECX
007a1731  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
007a1734  8b02                  MOV EAX,dword ptr [EDX]
007a1736  8945f8                MOV dword ptr [EBP + -0x8],EAX
007a1739  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
007a173c  8be5                  MOV ESP,EBP
007a173e  5d                    POP EBP
007a173f  c3                    RET
