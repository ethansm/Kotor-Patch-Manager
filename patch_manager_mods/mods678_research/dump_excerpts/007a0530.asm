; FN 007a0530 FUN_007a0530
007a0530  55                    PUSH EBP
007a0531  8bec                  MOV EBP,ESP
007a0533  51                    PUSH ECX
007a0534  894dfc                MOV dword ptr [EBP + -0x4],ECX
007a0537  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007a053a  8b88b4020000          MOV ECX,dword ptr [EAX + 0x2b4]
007a0540  2b4d08                SUB ECX,dword ptr [EBP + 0x8]
007a0543  f7d9                  NEG ECX
007a0545  1bc9                  SBB ECX,ECX
007a0547  234d0c                AND ECX,dword ptr [EBP + 0xc]
007a054a  83e101                AND ECX,0x1
007a054d  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
007a0550  8b827c030000          MOV EAX,dword ptr [EDX + 0x37c]
007a0556  83e0fe                AND EAX,0xfffffffe
007a0559  0bc1                  OR EAX,ECX
007a055b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007a055e  89817c030000          MOV dword ptr [ECX + 0x37c],EAX
007a0564  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
007a0567  8b4508                MOV EAX,dword ptr [EBP + 0x8]
007a056a  8982b4020000          MOV dword ptr [EDX + 0x2b4],EAX
007a0570  8be5                  MOV ESP,EBP
007a0572  5d                    POP EBP
007a0573  c20800                RET 0x8
