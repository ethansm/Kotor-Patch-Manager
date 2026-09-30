; FN 006d5200 FUN_006d5200
006d5200  55                    PUSH EBP
006d5201  8bec                  MOV EBP,ESP
006d5203  51                    PUSH ECX
006d5204  894dfc                MOV dword ptr [EBP + -0x4],ECX
006d5207  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
006d520a  83780400              CMP dword ptr [EAX + 0x4],0x0
006d520e  7421                  JZ 0x006d5231
006d5210  837d0800              CMP dword ptr [EBP + 0x8],0x0
006d5214  7c1b                  JL 0x006d5231
006d5216  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
006d5219  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006d521c  3b11                  CMP EDX,dword ptr [ECX]
006d521e  7d11                  JGE 0x006d5231
006d5220  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d5223  69c0c8000000          IMUL EAX,EAX,0xc8
006d5229  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
006d522c  034104                ADD EAX,dword ptr [ECX + 0x4]
006d522f  eb02                  JMP 0x006d5233
006d5231  33c0                  XOR EAX,EAX
006d5233  8be5                  MOV ESP,EBP
006d5235  5d                    POP EBP
006d5236  c20400                RET 0x4
