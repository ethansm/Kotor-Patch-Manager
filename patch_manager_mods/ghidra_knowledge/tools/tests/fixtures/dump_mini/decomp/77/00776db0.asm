; FN 00776db0 FUN_00776db0
00776db0  55                    PUSH EBP
00776db1  8bec                  MOV EBP,ESP
00776db3  51                    PUSH ECX
00776db4  894dfc                MOV dword ptr [EBP + -0x4],ECX
00776db7  68a0689800            PUSH 0x009868a0
00776dbc  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00776dbf  50                    PUSH EAX
00776dc0  ff1584649800          CALL dword ptr [0x00986484]
00776dc6  8be5                  MOV ESP,EBP
00776dc8  5d                    POP EBP
00776dc9  c20400                RET 0x4
