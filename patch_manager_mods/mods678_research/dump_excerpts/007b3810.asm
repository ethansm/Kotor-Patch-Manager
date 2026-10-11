; FN 007b3810 FUN_007b3810
007b3810  55                    PUSH EBP
007b3811  8bec                  MOV EBP,ESP
007b3813  51                    PUSH ECX
007b3814  894dfc                MOV dword ptr [EBP + -0x4],ECX
007b3817  c705bcfca70001000000  MOV dword ptr [0x00a7fcbc],0x1
007b3821  8b0d9cb4a100          MOV ECX,dword ptr [0x00a1b49c]
007b3827  e824fbc5ff            CALL 0x00413350
007b382c  8be5                  MOV ESP,EBP
007b382e  5d                    POP EBP
007b382f  c3                    RET
