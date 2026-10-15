; FN 00775930 FUN_00775930
00775930  55                    PUSH EBP
00775931  8bec                  MOV EBP,ESP
00775933  51                    PUSH ECX
00775934  894dfc                MOV dword ptr [EBP + -0x4],ECX
00775937  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0077593a  8b8824020000          MOV ECX,dword ptr [EAX + 0x224]
00775940  e81b790e00            CALL 0x0085d260
00775945  8be5                  MOV ESP,EBP
00775947  5d                    POP EBP
00775948  c3                    RET
