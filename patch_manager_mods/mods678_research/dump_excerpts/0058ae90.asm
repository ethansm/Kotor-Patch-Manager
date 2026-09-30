; FN 0058ae90 FUN_0058ae90
0058ae90  55                    PUSH EBP
0058ae91  8bec                  MOV EBP,ESP
0058ae93  83ec08                SUB ESP,0x8
0058ae96  894dfc                MOV dword ptr [EBP + -0x4],ECX
0058ae99  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0058ae9c  e89ff9fbff            CALL 0x0054a840
0058aea1  85c0                  TEST EAX,EAX
0058aea3  741a                  JZ 0x0058aebf
0058aea5  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0058aea8  e893f9fbff            CALL 0x0054a840
0058aead  8945f8                MOV dword ptr [EBP + -0x8],EAX
0058aeb0  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0058aeb3  8b10                  MOV EDX,dword ptr [EAX]
0058aeb5  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0058aeb8  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
0058aebb  ffd0                  CALL EAX
0058aebd  eb02                  JMP 0x0058aec1
0058aebf  33c0                  XOR EAX,EAX
0058aec1  8be5                  MOV ESP,EBP
0058aec3  5d                    POP EBP
0058aec4  c3                    RET
