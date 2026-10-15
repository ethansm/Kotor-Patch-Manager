; FN 00775990 FUN_00775990
00775990  55                    PUSH EBP
00775991  8bec                  MOV EBP,ESP
00775993  51                    PUSH ECX
00775994  894dfc                MOV dword ptr [EBP + -0x4],ECX
00775997  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0077599a  8b8824020000          MOV ECX,dword ptr [EAX + 0x224]
007759a0  e82b7a0e00            CALL 0x0085d3d0
007759a5  8be5                  MOV ESP,EBP
007759a7  5d                    POP EBP
007759a8  c3                    RET
