; FN 005d32c0 FUN_005d32c0
005d32c0  55                    PUSH EBP
005d32c1  8bec                  MOV EBP,ESP
005d32c3  83ec40                SUB ESP,0x40
005d32c6  894dc4                MOV dword ptr [EBP + -0x3c],ECX
005d32c9  6a00                  PUSH 0x0
005d32cb  6a2c                  PUSH 0x2c
005d32cd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d32d0  e88b0c0300            CALL 0x00603f60
005d32d5  85c0                  TEST EAX,EAX
005d32d7  750a                  JNZ 0x005d32e3
005d32d9  b801000000            MOV EAX,0x1
005d32de  e98c010000            JMP 0x005d346f
005d32e3  6a00                  PUSH 0x0
005d32e5  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d32e8  50                    PUSH EAX
005d32e9  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005d32ec  e84f0f0000            CALL 0x005d4240
005d32f1  83f801                CMP EAX,0x1
005d32f4  750a                  JNZ 0x005d3300
005d32f6  b801000000            MOV EAX,0x1
005d32fb  e96f010000            JMP 0x005d346f
005d3300  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005d3303  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005d3309  8955f0                MOV dword ptr [EBP + -0x10],EDX
005d330c  6a5d                  PUSH 0x5d
005d330e  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
005d3311  e8da500e00            CALL 0x006b83f0
005d3316  85c0                  TEST EAX,EAX
005d3318  740a                  JZ 0x005d3324
005d331a  b801000000            MOV EAX,0x1
005d331f  e94b010000            JMP 0x005d346f
005d3324  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005d332b  eb09                  JMP 0x005d3336
005d332d  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d3330  83c001                ADD EAX,0x1
005d3333  8945fc                MOV dword ptr [EBP + -0x4],EAX
005d3336  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3339  8b918c020000          MOV EDX,dword ptr [ECX + 0x28c]
005d333f  8955ec                MOV dword ptr [EBP + -0x14],EDX
005d3342  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d3345  3b45ec                CMP EAX,dword ptr [EBP + -0x14]
005d3348  0f8d1f010000          JGE 0x005d346d
005d334e  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d3351  51                    PUSH ECX
005d3352  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3355  e8460f0300            CALL 0x006042a0
005d335a  8945e8                MOV dword ptr [EBP + -0x18],EAX
005d335d  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005d3360  8a4211                MOV AL,byte ptr [EDX + 0x11]
005d3363  8845e7                MOV byte ptr [EBP + -0x19],AL
005d3366  0fb64de7              MOVZX ECX,byte ptr [EBP + -0x19]
005d336a  81f9ff000000          CMP ECX,0xff
005d3370  741f                  JZ 0x005d3391
005d3372  0fb64de7              MOVZX ECX,byte ptr [EBP + -0x19]
005d3376  ba01000000            MOV EDX,0x1
005d337b  d3e2                  SHL EDX,CL
005d337d  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d3380  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
005d3386  7509                  JNZ 0x005d3391
005d3388  c745c000000000        MOV dword ptr [EBP + -0x40],0x0
005d338f  eb07                  JMP 0x005d3398
005d3391  c745c001000000        MOV dword ptr [EBP + -0x40],0x1
005d3398  837dc000              CMP dword ptr [EBP + -0x40],0x0
005d339c  0f84c6000000          JZ 0x005d3468
005d33a2  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d33a5  51                    PUSH ECX
005d33a6  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d33a9  e8f20e0300            CALL 0x006042a0
005d33ae  8945e0                MOV dword ptr [EBP + -0x20],EAX
005d33b1  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
005d33b4  668b02                MOV AX,word ptr [EDX]
005d33b7  668945de              MOV word ptr [EBP + -0x22],AX
005d33bb  0fb74dde              MOVZX ECX,word ptr [EBP + -0x22]
005d33bf  83f92c                CMP ECX,0x2c
005d33c2  0f85a0000000          JNZ 0x005d3468
005d33c8  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d33cb  52                    PUSH EDX
005d33cc  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d33cf  e8cc0e0300            CALL 0x006042a0
005d33d4  8945d8                MOV dword ptr [EBP + -0x28],EAX
005d33d7  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
005d33da  668b4802              MOV CX,word ptr [EAX + 0x2]
005d33de  66894df8              MOV word ptr [EBP + -0x8],CX
005d33e2  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
005d33e9  eb09                  JMP 0x005d33f4
005d33eb  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
005d33ee  83c201                ADD EDX,0x1
005d33f1  8955f4                MOV dword ptr [EBP + -0xc],EDX
005d33f4  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
005d33f7  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d33fd  894dd4                MOV dword ptr [EBP + -0x2c],ECX
005d3400  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
005d3403  8a828d000000          MOV AL,byte ptr [EDX + 0x8d]
005d3409  8845d3                MOV byte ptr [EBP + -0x2d],AL
005d340c  0fb64dd3              MOVZX ECX,byte ptr [EBP + -0x2d]
005d3410  394df4                CMP dword ptr [EBP + -0xc],ECX
005d3413  7d53                  JGE 0x005d3468
005d3415  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
005d3418  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005d341e  8945cc                MOV dword ptr [EBP + -0x34],EAX
005d3421  0fb64df4              MOVZX ECX,byte ptr [EBP + -0xc]
005d3425  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
005d3428  0fb6828d000000        MOVZX EAX,byte ptr [EDX + 0x8d]
005d342f  3bc8                  CMP ECX,EAX
005d3431  7c06                  JL 0x005d3439
005d3433  c645caff              MOV byte ptr [EBP + -0x36],0xff
005d3437  eb1a                  JMP 0x005d3453
005d3439  0fb64df4              MOVZX ECX,byte ptr [EBP + -0xc]
005d343d  6bc928                IMUL ECX,ECX,0x28
005d3440  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
005d3443  8a840aab000000        MOV AL,byte ptr [EDX + ECX*0x1 + 0xab]
005d344a  8845cb                MOV byte ptr [EBP + -0x35],AL
005d344d  8a4dcb                MOV CL,byte ptr [EBP + -0x35]
005d3450  884dca                MOV byte ptr [EBP + -0x36],CL
005d3453  0fb755f8              MOVZX EDX,word ptr [EBP + -0x8]
005d3457  0fb645ca              MOVZX EAX,byte ptr [EBP + -0x36]
005d345b  3bd0                  CMP EDX,EAX
005d345d  7507                  JNZ 0x005d3466
005d345f  b801000000            MOV EAX,0x1
005d3464  eb09                  JMP 0x005d346f
005d3466  eb83                  JMP 0x005d33eb
005d3468  e9c0feffff            JMP 0x005d332d
005d346d  33c0                  XOR EAX,EAX
005d346f  8be5                  MOV ESP,EBP
005d3471  5d                    POP EBP
005d3472  c20400                RET 0x4
