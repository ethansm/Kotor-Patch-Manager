; FN 00775950 FUN_00775950
00775950  55                    PUSH EBP
00775951  8bec                  MOV EBP,ESP
00775953  51                    PUSH ECX
00775954  894dfc                MOV dword ptr [EBP + -0x4],ECX
00775957  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0077595a  8b8824020000          MOV ECX,dword ptr [EAX + 0x224]
00775960  e86b790e00            CALL 0x0085d2d0
00775965  8be5                  MOV ESP,EBP
00775967  5d                    POP EBP
00775968  c3                    RET
