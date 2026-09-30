; FN 005ff950 FUN_005ff950
005ff950  55                    PUSH EBP
005ff951  8bec                  MOV EBP,ESP
005ff953  83ec2c                SUB ESP,0x2c
005ff956  894dd4                MOV dword ptr [EBP + -0x2c],ECX
005ff959  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
005ff95c  8b4838                MOV ECX,dword ptr [EAX + 0x38]
005ff95f  894dd8                MOV dword ptr [EBP + -0x28],ECX
005ff962  837dd800              CMP dword ptr [EBP + -0x28],0x0
005ff966  7407                  JZ 0x005ff96f
005ff968  b801000000            MOV EAX,0x1
005ff96d  eb7b                  JMP 0x005ff9ea
005ff96f  6aff                  PUSH -0x1
005ff971  8d4ddc                LEA ECX,[EBP + -0x24]
005ff974  e807e70400            CALL 0x0064e080
005ff979  6aff                  PUSH -0x1
005ff97b  8d4ddc                LEA ECX,[EBP + -0x24]
005ff97e  e88de70400            CALL 0x0064e110
005ff983  8d55dc                LEA EDX,[EBP + -0x24]
005ff986  52                    PUSH EDX
005ff987  83ec0c                SUB ESP,0xc
005ff98a  8bc4                  MOV EAX,ESP
005ff98c  8b4d14                MOV ECX,dword ptr [EBP + 0x14]
005ff98f  8908                  MOV dword ptr [EAX],ECX
005ff991  8b5518                MOV EDX,dword ptr [EBP + 0x18]
005ff994  895004                MOV dword ptr [EAX + 0x4],EDX
005ff997  8b4d1c                MOV ECX,dword ptr [EBP + 0x1c]
005ff99a  894808                MOV dword ptr [EAX + 0x8],ECX
005ff99d  83ec0c                SUB ESP,0xc
005ff9a0  8bd4                  MOV EDX,ESP
005ff9a2  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005ff9a5  8902                  MOV dword ptr [EDX],EAX
005ff9a7  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
005ff9aa  894a04                MOV dword ptr [EDX + 0x4],ECX
005ff9ad  8b4510                MOV EAX,dword ptr [EBP + 0x10]
005ff9b0  894208                MOV dword ptr [EDX + 0x8],EAX
005ff9b3  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
005ff9b6  8b493c                MOV ECX,dword ptr [ECX + 0x3c]
005ff9b9  e862100500            CALL 0x00650a20
005ff9be  8945fc                MOV dword ptr [EBP + -0x4],EAX
005ff9c1  837dfc00              CMP dword ptr [EBP + -0x4],0x0
005ff9c5  741a                  JZ 0x005ff9e1
005ff9c7  837d2000              CMP dword ptr [EBP + 0x20],0x0
005ff9cb  7414                  JZ 0x005ff9e1
005ff9cd  8b5520                MOV EDX,dword ptr [EBP + 0x20]
005ff9d0  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
005ff9d3  8902                  MOV dword ptr [EDX],EAX
005ff9d5  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
005ff9d8  894a04                MOV dword ptr [EDX + 0x4],ECX
005ff9db  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005ff9de  894208                MOV dword ptr [EDX + 0x8],EAX
005ff9e1  33c0                  XOR EAX,EAX
005ff9e3  837dfc00              CMP dword ptr [EBP + -0x4],0x0
005ff9e7  0f94c0                SETZ AL
005ff9ea  8be5                  MOV ESP,EBP
005ff9ec  5d                    POP EBP
005ff9ed  c22000                RET 0x20
