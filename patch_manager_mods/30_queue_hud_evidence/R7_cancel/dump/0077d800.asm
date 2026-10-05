; FN 0077d800 FUN_0077d800
0077d800  55                    PUSH EBP
0077d801  8bec                  MOV EBP,ESP
0077d803  83ec08                SUB ESP,0x8
0077d806  894dfc                MOV dword ptr [EBP + -0x4],ECX
0077d809  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0077d80c  e82f4d0700            CALL 0x007f2540
0077d811  85c0                  TEST EAX,EAX
0077d813  741a                  JZ 0x0077d82f
0077d815  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0077d818  e8234d0700            CALL 0x007f2540
0077d81d  8945f8                MOV dword ptr [EBP + -0x8],EAX
0077d820  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0077d823  8b10                  MOV EDX,dword ptr [EAX]
0077d825  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0077d828  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0077d82b  ffd0                  CALL EAX
0077d82d  eb02                  JMP 0x0077d831
0077d82f  33c0                  XOR EAX,EAX
0077d831  8be5                  MOV ESP,EBP
0077d833  5d                    POP EBP
0077d834  c3                    RET
