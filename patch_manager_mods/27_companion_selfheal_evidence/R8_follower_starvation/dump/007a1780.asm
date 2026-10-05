; FN 007a1780 FUN_007a1780
007a1780  55                    PUSH EBP
007a1781  8bec                  MOV EBP,ESP
007a1783  83ec08                SUB ESP,0x8
007a1786  894df8                MOV dword ptr [EBP + -0x8],ECX
007a1789  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
007a178c  8b08                  MOV ECX,dword ptr [EAX]
007a178e  894dfc                MOV dword ptr [EBP + -0x4],ECX
007a1791  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
007a1794  33c0                  XOR EAX,EAX
007a1796  833a00                CMP dword ptr [EDX],0x0
007a1799  0f94c0                SETZ AL
007a179c  8be5                  MOV ESP,EBP
007a179e  5d                    POP EBP
007a179f  c3                    RET
