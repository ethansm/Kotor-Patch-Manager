; FN 007b3830 FUN_007b3830
007b3830  55                    PUSH EBP
007b3831  8bec                  MOV EBP,ESP
007b3833  51                    PUSH ECX
007b3834  894dfc                MOV dword ptr [EBP + -0x4],ECX
007b3837  c705bcfca70000000000  MOV dword ptr [0x00a7fcbc],0x0
007b3841  8b0d9cb4a100          MOV ECX,dword ptr [0x00a1b49c]
007b3847  e874fbc5ff            CALL 0x004133c0
007b384c  8be5                  MOV ESP,EBP
007b384e  5d                    POP EBP
007b384f  c3                    RET
