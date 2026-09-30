; FN 0056e1e0 FUN_0056e1e0
0056e1e0  55                    PUSH EBP
0056e1e1  8bec                  MOV EBP,ESP
0056e1e3  83ec18                SUB ESP,0x18
0056e1e6  894de8                MOV dword ptr [EBP + -0x18],ECX
0056e1e9  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0056e1ec  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
0056e1f2  894df8                MOV dword ptr [EBP + -0x8],ECX
0056e1f5  6a10                  PUSH 0x10
0056e1f7  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0056e1fa  e821241600            CALL 0x006d0620
0056e1ff  8945fc                MOV dword ptr [EBP + -0x4],EAX
0056e202  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0056e206  743d                  JZ 0x0056e245
0056e208  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0056e20b  8b420c                MOV EAX,dword ptr [EDX + 0xc]
0056e20e  8945f4                MOV dword ptr [EBP + -0xc],EAX
0056e211  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
0056e217  894df0                MOV dword ptr [EBP + -0x10],ECX
0056e21a  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0056e21d  52                    PUSH EDX
0056e21e  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0056e221  8b4834                MOV ECX,dword ptr [EAX + 0x34]
0056e224  e8d76f1600            CALL 0x006d5200
0056e229  8945ec                MOV dword ptr [EBP + -0x14],EAX
0056e22c  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0056e22f  0fb6511a              MOVZX EDX,byte ptr [ECX + 0x1a]
0056e233  33c0                  XOR EAX,EAX
0056e235  85d2                  TEST EDX,EDX
0056e237  0f9fc0                SETG AL
0056e23a  33c9                  XOR ECX,ECX
0056e23c  85c0                  TEST EAX,EAX
0056e23e  0f9fc1                SETG CL
0056e241  8bc1                  MOV EAX,ECX
0056e243  eb02                  JMP 0x0056e247
0056e245  33c0                  XOR EAX,EAX
0056e247  8be5                  MOV ESP,EBP
0056e249  5d                    POP EBP
0056e24a  c3                    RET
