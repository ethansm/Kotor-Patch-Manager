; FN 00740880 FUN_00740880
00740880  55                    PUSH EBP
00740881  8bec                  MOV EBP,ESP
00740883  51                    PUSH ECX
00740884  894dfc                MOV dword ptr [EBP + -0x4],ECX
00740887  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0074088a  50                    PUSH EAX
0074088b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0074088e  8b4904                MOV ECX,dword ptr [ECX + 0x4]
00740891  e84ace0500            CALL 0x0079d6e0
00740896  8be5                  MOV ESP,EBP
00740898  5d                    POP EBP
00740899  c20400                RET 0x4
