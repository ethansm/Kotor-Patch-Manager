; FN 007e5d80 FUN_007e5d80
007e5d80  55                    PUSH EBP
007e5d81  8bec                  MOV EBP,ESP
007e5d83  51                    PUSH ECX
007e5d84  894dfc                MOV dword ptr [EBP + -0x4],ECX
007e5d87  6a00                  PUSH 0x0
007e5d89  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007e5d8c  e80f000000            CALL 0x007e5da0
007e5d91  8be5                  MOV ESP,EBP
007e5d93  5d                    POP EBP
007e5d94  c3                    RET
