; FN 007759b0 FUN_007759b0
007759b0  55                    PUSH EBP
007759b1  8bec                  MOV EBP,ESP
007759b3  51                    PUSH ECX
007759b4  894dfc                MOV dword ptr [EBP + -0x4],ECX
007759b7  33c0                  XOR EAX,EAX
007759b9  03818a020000          ADD EAX,dword ptr [ECX + 0x29a]
007759bf  c1e002                SHL EAX,0x2
007759c2  f7d8                  NEG EAX
007759c4  8be5                  MOV ESP,EBP
007759c6  5d                    POP EBP
007759c7  90                    NOP
007759c8  c3                    RET
