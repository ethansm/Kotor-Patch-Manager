; FN 007680a0 FUN_007680a0
007680a0  55                    PUSH EBP
007680a1  8bec                  MOV EBP,ESP
007680a3  8b4108                MOV EAX,dword ptr [ECX + 0x8]
007680a6  ff5014                CALL dword ptr [EAX + 0x14]
007680a9  8b4508                MOV EAX,dword ptr [EBP + 0x8]
007680ac  51                    PUSH ECX
007680ad  e8ee8e0000            CALL 0x007679a0
007680b2  83c404                ADD ESP,0x4
007680b5  8b5508                MOV EDX,dword ptr [EBP + 0xc]
007680b8  52                    PUSH EDX
007680b9  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007680bc  83c110                ADD ECX,0x10
007680bf  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007680c2  52                    PUSH EDX
007680c3  51                    PUSH ECX
007680c4  8b4510                MOV EAX,dword ptr [EBP + 0x10]
007680c7  50                    PUSH EAX
007680c8  83c40c                ADD ESP,0xc
007680cb  85c0                  TEST EAX,EAX
007680cd  0f94c0                SETZ AL
007680d0  0fb6c0                MOVZX EAX,AL
007680d3  50                    PUSH EAX
007680d4  51                    PUSH ECX
007680d5  e8c68e0000            CALL 0x007679a0
007680da  83c408                ADD ESP,0x8
007680dd  90                    NOP
007680de  8b4108                MOV EAX,dword ptr [ECX + 0x8]
007680e1  ffd0                  CALL EAX
007680e3  8be5                  MOV ESP,EBP
007680e5  5d                    POP EBP
007680e6  c3                    RET
