; FN 007523b0 FUN_007523b0
007523b0  55                    PUSH EBP
007523b1  8bec                  MOV EBP,ESP
007523b3  83ec08                SUB ESP,0x8
007523b6  894df8                MOV dword ptr [EBP + -0x8],ECX
007523b9  8b4508                MOV EAX,dword ptr [EBP + 0x8]
007523bc  8b4850                MOV ECX,dword ptr [EAX + 0x50]
007523bf  894dfc                MOV dword ptr [EBP + -0x4],ECX
007523c2  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007523c6  7408                  JZ 0x007523d0
007523c8  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007523cb  e810000000            CALL 0x007523e0
007523d0  8be5                  MOV ESP,EBP
007523d2  5d                    POP EBP
007523d3  c20400                RET 0x4
