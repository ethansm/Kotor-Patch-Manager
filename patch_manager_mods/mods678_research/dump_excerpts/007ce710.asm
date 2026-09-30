; FN 007ce710 FUN_007ce710
007ce710  55                    PUSH EBP
007ce711  8bec                  MOV EBP,ESP
007ce713  51                    PUSH ECX
007ce714  894dfc                MOV dword ptr [EBP + -0x4],ECX
007ce717  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007ce71a  83b89800000000        CMP dword ptr [EAX + 0x98],0x0
007ce721  7416                  JZ 0x007ce739
007ce723  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
007ce726  51                    PUSH ECX
007ce727  8b5508                MOV EDX,dword ptr [EBP + 0x8]
007ce72a  52                    PUSH EDX
007ce72b  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007ce72e  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007ce734  e877e8f7ff            CALL 0x0074cfb0
007ce739  8be5                  MOV ESP,EBP
007ce73b  5d                    POP EBP
007ce73c  c20800                RET 0x8
