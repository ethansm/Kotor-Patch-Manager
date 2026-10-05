; FN 00590430 FUN_00590430
00590430  55                    PUSH EBP
00590431  8bec                  MOV EBP,ESP
00590433  83ec14                SUB ESP,0x14
00590436  894df0                MOV dword ptr [EBP + -0x10],ECX
00590439  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0059043c  83b8000b000000        CMP dword ptr [EAX + 0xb00],0x0
00590443  746b                  JZ 0x005904b0
00590445  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00590448  8b89000b0000          MOV ECX,dword ptr [ECX + 0xb00]
0059044e  e82d132100            CALL 0x007a1780
00590453  85c0                  TEST EAX,EAX
00590455  7559                  JNZ 0x005904b0
00590457  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0059045a  8b8a000b0000          MOV ECX,dword ptr [EDX + 0xb00]
00590460  e82b262600            CALL 0x007f2a90
00590465  8945fc                MOV dword ptr [EBP + -0x4],EAX
00590468  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0059046b  8945f4                MOV dword ptr [EBP + -0xc],EAX
0059046e  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00590471  894df8                MOV dword ptr [EBP + -0x8],ECX
00590474  837df800              CMP dword ptr [EBP + -0x8],0x0
00590478  7426                  JZ 0x005904a0
0059047a  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0059047d  e89e2e0900            CALL 0x00623320
00590482  ba01000000            MOV EDX,0x1
00590487  83e201                AND EDX,0x1
0059048a  740c                  JZ 0x00590498
0059048c  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0059048f  50                    PUSH EAX
00590490  e868923800            CALL 0x009196fd
00590495  83c404                ADD ESP,0x4
00590498  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0059049b  894dec                MOV dword ptr [EBP + -0x14],ECX
0059049e  eb07                  JMP 0x005904a7
005904a0  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
005904a7  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005904ae  eb95                  JMP 0x00590445
005904b0  8be5                  MOV ESP,EBP
005904b2  5d                    POP EBP
005904b3  c3                    RET
