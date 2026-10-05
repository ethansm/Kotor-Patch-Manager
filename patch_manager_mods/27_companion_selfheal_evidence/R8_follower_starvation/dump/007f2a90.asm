; FN 007f2a90 FUN_007f2a90
007f2a90  55                    PUSH EBP
007f2a91  8bec                  MOV EBP,ESP
007f2a93  51                    PUSH ECX
007f2a94  894dfc                MOV dword ptr [EBP + -0x4],ECX
007f2a97  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007f2a9a  8b08                  MOV ECX,dword ptr [EAX]
007f2a9c  e85f77f4ff            CALL 0x0073a200
007f2aa1  8be5                  MOV ESP,EBP
007f2aa3  5d                    POP EBP
007f2aa4  c3                    RET
