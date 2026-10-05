; FN 005904c0 FUN_005904c0
005904c0  55                    PUSH EBP
005904c1  8bec                  MOV EBP,ESP
005904c3  83ec18                SUB ESP,0x18
005904c6  894dec                MOV dword ptr [EBP + -0x14],ECX
005904c9  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005904d0  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
005904d3  83b8000b000000        CMP dword ptr [EAX + 0xb00],0x0
005904da  7470                  JZ 0x0059054c
005904dc  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
005904df  8b89000b0000          MOV ECX,dword ptr [ECX + 0xb00]
005904e5  e896122100            CALL 0x007a1780
005904ea  85c0                  TEST EAX,EAX
005904ec  755e                  JNZ 0x0059054c
005904ee  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
005904f1  8b8a000b0000          MOV ECX,dword ptr [EDX + 0xb00]
005904f7  e8d4310000            CALL 0x005936d0
005904fc  8945f8                MOV dword ptr [EBP + -0x8],EAX
005904ff  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00590502  8945f0                MOV dword ptr [EBP + -0x10],EAX
00590505  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00590508  894df4                MOV dword ptr [EBP + -0xc],ECX
0059050b  837df400              CMP dword ptr [EBP + -0xc],0x0
0059050f  7426                  JZ 0x00590537
00590511  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00590514  e8072e0900            CALL 0x00623320
00590519  ba01000000            MOV EDX,0x1
0059051e  83e201                AND EDX,0x1
00590521  740c                  JZ 0x0059052f
00590523  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00590526  50                    PUSH EAX
00590527  e8d1913800            CALL 0x009196fd
0059052c  83c404                ADD ESP,0x4
0059052f  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00590532  894de8                MOV dword ptr [EBP + -0x18],ECX
00590535  eb07                  JMP 0x0059053e
00590537  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
0059053e  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
00590545  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
0059054c  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0059054f  8be5                  MOV ESP,EBP
00590551  5d                    POP EBP
00590552  c3                    RET
