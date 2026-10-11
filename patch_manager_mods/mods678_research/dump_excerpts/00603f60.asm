; FN 00603f60 FUN_00603f60
00603f60  55                    PUSH EBP
00603f61  8bec                  MOV EBP,ESP
00603f63  83ec48                SUB ESP,0x48
00603f66  894dc0                MOV dword ptr [EBP + -0x40],ECX
00603f69  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00603f70  eb09                  JMP 0x00603f7b
00603f72  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00603f75  83c001                ADD EAX,0x1
00603f78  8945fc                MOV dword ptr [EBP + -0x4],EAX
00603f7b  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
00603f7e  8b9188020000          MOV EDX,dword ptr [ECX + 0x288]
00603f84  8955f8                MOV dword ptr [EBP + -0x8],EDX
00603f87  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00603f8a  3b45f8                CMP EAX,dword ptr [EBP + -0x8]
00603f8d  0f8dbe000000          JGE 0x00604051
00603f93  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00603f96  51                    PUSH ECX
00603f97  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
00603f9a  e8c1020000            CALL 0x00604260
00603f9f  8945f4                MOV dword ptr [EBP + -0xc],EAX
00603fa2  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00603fa5  8a4211                MOV AL,byte ptr [EDX + 0x11]
00603fa8  8845f3                MOV byte ptr [EBP + -0xd],AL
00603fab  0fb64df3              MOVZX ECX,byte ptr [EBP + -0xd]
00603faf  81f9ff000000          CMP ECX,0xff
00603fb5  741f                  JZ 0x00603fd6
00603fb7  0fb64df3              MOVZX ECX,byte ptr [EBP + -0xd]
00603fbb  ba01000000            MOV EDX,0x1
00603fc0  d3e2                  SHL EDX,CL
00603fc2  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
00603fc5  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
00603fcb  7509                  JNZ 0x00603fd6
00603fcd  c745bc00000000        MOV dword ptr [EBP + -0x44],0x0
00603fd4  eb07                  JMP 0x00603fdd
00603fd6  c745bc01000000        MOV dword ptr [EBP + -0x44],0x1
00603fdd  837dbc00              CMP dword ptr [EBP + -0x44],0x0
00603fe1  7469                  JZ 0x0060404c
00603fe3  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00603fe6  51                    PUSH ECX
00603fe7  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
00603fea  e871020000            CALL 0x00604260
00603fef  8945ec                MOV dword ptr [EBP + -0x14],EAX
00603ff2  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00603ff5  668b02                MOV AX,word ptr [EDX]
00603ff8  668945ea              MOV word ptr [EBP + -0x16],AX
00603ffc  0fb74dea              MOVZX ECX,word ptr [EBP + -0x16]
00604000  0fb75508              MOVZX EDX,word ptr [EBP + 0x8]
00604004  3bca                  CMP ECX,EDX
00604006  7544                  JNZ 0x0060404c
00604008  0fb7450c              MOVZX EAX,word ptr [EBP + 0xc]
0060400c  85c0                  TEST EAX,EAX
0060400e  7432                  JZ 0x00604042
00604010  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00604013  51                    PUSH ECX
00604014  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
00604017  e844020000            CALL 0x00604260
0060401c  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0060401f  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
00604022  668b4202              MOV AX,word ptr [EDX + 0x2]
00604026  668945e2              MOV word ptr [EBP + -0x1e],AX
0060402a  0fb74de2              MOVZX ECX,word ptr [EBP + -0x1e]
0060402e  0fb7550c              MOVZX EDX,word ptr [EBP + 0xc]
00604032  3bca                  CMP ECX,EDX
00604034  750a                  JNZ 0x00604040
00604036  b801000000            MOV EAX,0x1
0060403b  e9f5000000            JMP 0x00604135
00604040  eb0a                  JMP 0x0060404c
00604042  b801000000            MOV EAX,0x1
00604047  e9e9000000            JMP 0x00604135
0060404c  e921ffffff            JMP 0x00603f72
00604051  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00604058  eb09                  JMP 0x00604063
0060405a  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0060405d  83c001                ADD EAX,0x1
00604060  8945fc                MOV dword ptr [EBP + -0x4],EAX
00604063  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
00604066  8b918c020000          MOV EDX,dword ptr [ECX + 0x28c]
0060406c  8955dc                MOV dword ptr [EBP + -0x24],EDX
0060406f  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00604072  3b45dc                CMP EAX,dword ptr [EBP + -0x24]
00604075  0f8db8000000          JGE 0x00604133
0060407b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0060407e  51                    PUSH ECX
0060407f  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
00604082  e819020000            CALL 0x006042a0
00604087  8945d8                MOV dword ptr [EBP + -0x28],EAX
0060408a  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
0060408d  8a4211                MOV AL,byte ptr [EDX + 0x11]
00604090  8845d7                MOV byte ptr [EBP + -0x29],AL
00604093  0fb64dd7              MOVZX ECX,byte ptr [EBP + -0x29]
00604097  81f9ff000000          CMP ECX,0xff
0060409d  741f                  JZ 0x006040be
0060409f  0fb64dd7              MOVZX ECX,byte ptr [EBP + -0x29]
006040a3  ba01000000            MOV EDX,0x1
006040a8  d3e2                  SHL EDX,CL
006040aa  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
006040ad  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
006040b3  7509                  JNZ 0x006040be
006040b5  c745b800000000        MOV dword ptr [EBP + -0x48],0x0
006040bc  eb07                  JMP 0x006040c5
006040be  c745b801000000        MOV dword ptr [EBP + -0x48],0x1
006040c5  837db800              CMP dword ptr [EBP + -0x48],0x0
006040c9  7463                  JZ 0x0060412e
006040cb  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
006040ce  51                    PUSH ECX
006040cf  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
006040d2  e8c9010000            CALL 0x006042a0
006040d7  8945d0                MOV dword ptr [EBP + -0x30],EAX
006040da  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
006040dd  668b02                MOV AX,word ptr [EDX]
006040e0  668945ce              MOV word ptr [EBP + -0x32],AX
006040e4  0fb74dce              MOVZX ECX,word ptr [EBP + -0x32]
006040e8  0fb75508              MOVZX EDX,word ptr [EBP + 0x8]
006040ec  3bca                  CMP ECX,EDX
006040ee  753e                  JNZ 0x0060412e
006040f0  0fb7450c              MOVZX EAX,word ptr [EBP + 0xc]
006040f4  85c0                  TEST EAX,EAX
006040f6  742f                  JZ 0x00604127
006040f8  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
006040fb  51                    PUSH ECX
006040fc  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
006040ff  e89c010000            CALL 0x006042a0
00604104  8945c8                MOV dword ptr [EBP + -0x38],EAX
00604107  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
0060410a  668b4202              MOV AX,word ptr [EDX + 0x2]
0060410e  668945c6              MOV word ptr [EBP + -0x3a],AX
00604112  0fb74dc6              MOVZX ECX,word ptr [EBP + -0x3a]
00604116  0fb7550c              MOVZX EDX,word ptr [EBP + 0xc]
0060411a  3bca                  CMP ECX,EDX
0060411c  7507                  JNZ 0x00604125
0060411e  b801000000            MOV EAX,0x1
00604123  eb10                  JMP 0x00604135
00604125  eb07                  JMP 0x0060412e
00604127  b801000000            MOV EAX,0x1
0060412c  eb07                  JMP 0x00604135
0060412e  e927ffffff            JMP 0x0060405a
00604133  33c0                  XOR EAX,EAX
00604135  8be5                  MOV ESP,EBP
00604137  5d                    POP EBP
00604138  c20800                RET 0x8
