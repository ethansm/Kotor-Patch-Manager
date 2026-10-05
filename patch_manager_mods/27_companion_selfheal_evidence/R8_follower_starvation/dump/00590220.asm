; FN 00590220 FUN_00590220
00590220  55                    PUSH EBP
00590221  8bec                  MOV EBP,ESP
00590223  83ec08                SUB ESP,0x8
00590226  894df8                MOV dword ptr [EBP + -0x8],ECX
00590229  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0059022c  83b8000b000000        CMP dword ptr [EAX + 0xb00],0x0
00590233  7504                  JNZ 0x00590239
00590235  33c0                  XOR EAX,EAX
00590237  eb2e                  JMP 0x00590267
00590239  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0059023c  8b89000b0000          MOV ECX,dword ptr [ECX + 0xb00]
00590242  e849282600            CALL 0x007f2a90
00590247  8945fc                MOV dword ptr [EBP + -0x4],EAX
0059024a  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0059024e  7415                  JZ 0x00590265
00590250  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
00590253  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00590256  8b8a940a0000          MOV ECX,dword ptr [EDX + 0xa94]
0059025c  3b08                  CMP ECX,dword ptr [EAX]
0059025e  7c05                  JL 0x00590265
00590260  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00590263  eb02                  JMP 0x00590267
00590265  33c0                  XOR EAX,EAX
00590267  8be5                  MOV ESP,EBP
00590269  5d                    POP EBP
0059026a  c3                    RET
