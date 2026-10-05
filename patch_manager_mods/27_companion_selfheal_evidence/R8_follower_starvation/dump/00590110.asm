; FN 00590110 FUN_00590110
00590110  55                    PUSH EBP
00590111  8bec                  MOV EBP,ESP
00590113  83ec0c                SUB ESP,0xc
00590116  894df4                MOV dword ptr [EBP + -0xc],ECX
00590119  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0059011c  83b8000b000000        CMP dword ptr [EAX + 0xb00],0x0
00590123  7504                  JNZ 0x00590129
00590125  33c0                  XOR EAX,EAX
00590127  eb45                  JMP 0x0059016e
00590129  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0059012c  8b89000b0000          MOV ECX,dword ptr [ECX + 0xb00]
00590132  e8e9152100            CALL 0x007a1720
00590137  8945fc                MOV dword ptr [EBP + -0x4],EAX
0059013a  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0059013e  742c                  JZ 0x0059016c
00590140  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00590143  52                    PUSH EDX
00590144  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00590147  8b88000b0000          MOV ECX,dword ptr [EAX + 0xb00]
0059014d  e8de12fdff            CALL 0x00561430
00590152  8945f8                MOV dword ptr [EBP + -0x8],EAX
00590155  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00590158  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0059015b  8b81940a0000          MOV EAX,dword ptr [ECX + 0xa94]
00590161  3b02                  CMP EAX,dword ptr [EDX]
00590163  7c07                  JL 0x0059016c
00590165  b801000000            MOV EAX,0x1
0059016a  eb02                  JMP 0x0059016e
0059016c  33c0                  XOR EAX,EAX
0059016e  8be5                  MOV ESP,EBP
00590170  5d                    POP EBP
00590171  c3                    RET
