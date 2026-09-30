; FN 005f67b0 FUN_005f67b0
005f67b0  55                    PUSH EBP
005f67b1  8bec                  MOV EBP,ESP
005f67b3  83ec1c                SUB ESP,0x1c
005f67b6  894de4                MOV dword ptr [EBP + -0x1c],ECX
005f67b9  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
005f67bc  8b887c030000          MOV ECX,dword ptr [EAX + 0x37c]
005f67c2  894dec                MOV dword ptr [EBP + -0x14],ECX
005f67c5  837dec00              CMP dword ptr [EBP + -0x14],0x0
005f67c9  7504                  JNZ 0x005f67cf
005f67cb  33c0                  XOR EAX,EAX
005f67cd  eb78                  JMP 0x005f6847
005f67cf  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
005f67d6  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
005f67d9  8b82c4030000          MOV EAX,dword ptr [EDX + 0x3c4]
005f67df  8945fc                MOV dword ptr [EBP + -0x4],EAX
005f67e2  837dfc00              CMP dword ptr [EBP + -0x4],0x0
005f67e6  745c                  JZ 0x005f6844
005f67e8  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
005f67ef  eb09                  JMP 0x005f67fa
005f67f1  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
005f67f4  83c101                ADD ECX,0x1
005f67f7  894df0                MOV dword ptr [EBP + -0x10],ECX
005f67fa  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005f67fd  8b4210                MOV EAX,dword ptr [EDX + 0x10]
005f6800  8945e8                MOV dword ptr [EBP + -0x18],EAX
005f6803  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
005f6806  3b4de8                CMP ECX,dword ptr [EBP + -0x18]
005f6809  7339                  JNC 0x005f6844
005f680b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
005f680e  52                    PUSH EDX
005f680f  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005f6812  e8e9f80000            CALL 0x00606100
005f6817  8945f8                MOV dword ptr [EBP + -0x8],EAX
005f681a  837df800              CMP dword ptr [EBP + -0x8],0x0
005f681e  7422                  JZ 0x005f6842
005f6820  837d0800              CMP dword ptr [EBP + 0x8],0x0
005f6824  7413                  JZ 0x005f6839
005f6826  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
005f6829  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
005f682f  c1e903                SHR ECX,0x3
005f6832  83e101                AND ECX,0x1
005f6835  7502                  JNZ 0x005f6839
005f6837  ebb8                  JMP 0x005f67f1
005f6839  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
005f683c  83c201                ADD EDX,0x1
005f683f  8955f4                MOV dword ptr [EBP + -0xc],EDX
005f6842  ebad                  JMP 0x005f67f1
005f6844  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005f6847  8be5                  MOV ESP,EBP
005f6849  5d                    POP EBP
005f684a  c20400                RET 0x4
