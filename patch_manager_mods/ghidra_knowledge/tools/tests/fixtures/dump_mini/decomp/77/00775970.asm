; FN 00775970 FUN_00775970
00775970  55                    PUSH EBP
00775971  8bec                  MOV EBP,ESP
00775973  51                    PUSH ECX
00775974  894dfc                MOV dword ptr [EBP + -0x4],ECX
00775977  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0077597a  8b8824020000          MOV ECX,dword ptr [EAX + 0x224]
00775980  e89b790e00            CALL 0x0085d320
00775985  8be5                  MOV ESP,EBP
00775987  5d                    POP EBP
00775988  c3                    RET
