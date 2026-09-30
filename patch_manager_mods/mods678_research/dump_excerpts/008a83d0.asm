; FN 008a83d0 FUN_008a83d0
008a83d0  55                    PUSH EBP
008a83d1  8bec                  MOV EBP,ESP
008a83d3  83ec18                SUB ESP,0x18
008a83d6  894de8                MOV dword ptr [EBP + -0x18],ECX
008a83d9  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a83dc  8b4850                MOV ECX,dword ptr [EAX + 0x50]
008a83df  894df0                MOV dword ptr [EBP + -0x10],ECX
008a83e2  837df000              CMP dword ptr [EBP + -0x10],0x0
008a83e6  0f848c000000          JZ 0x008a8478
008a83ec  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008a83ef  8955f4                MOV dword ptr [EBP + -0xc],EDX
008a83f2  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
008a83f5  8b88d0010000          MOV ECX,dword ptr [EAX + 0x1d0]
008a83fb  894df8                MOV dword ptr [EBP + -0x8],ECX
008a83fe  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
008a8404  8b4208                MOV EAX,dword ptr [EDX + 0x8]
008a8407  8945ec                MOV dword ptr [EBP + -0x14],EAX
008a840a  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
008a840d  51                    PUSH ECX
008a840e  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8411  e8ca3cc7ff            CALL 0x0051c0e0
008a8416  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a8419  837dfc00              CMP dword ptr [EBP + -0x4],0x0
008a841d  7459                  JZ 0x008a8478
008a841f  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008a8422  8b82c8020000          MOV EAX,dword ptr [EDX + 0x2c8]
008a8428  c1e807                SHR EAX,0x7
008a842b  83e001                AND EAX,0x1
008a842e  7448                  JZ 0x008a8478
008a8430  33c9                  XOR ECX,ECX
008a8432  83e101                AND ECX,0x1
008a8435  c1e107                SHL ECX,0x7
008a8438  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008a843b  8b82c8020000          MOV EAX,dword ptr [EDX + 0x2c8]
008a8441  257fffffff            AND EAX,0xffffff7f
008a8446  0bc1                  OR EAX,ECX
008a8448  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008a844b  8981c8020000          MOV dword ptr [ECX + 0x2c8],EAX
008a8451  33d2                  XOR EDX,EDX
008a8453  83e201                AND EDX,0x1
008a8456  c1e202                SHL EDX,0x2
008a8459  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
008a845c  8b88ac030000          MOV ECX,dword ptr [EAX + 0x3ac]
008a8462  83e1fb                AND ECX,0xfffffffb
008a8465  0bca                  OR ECX,EDX
008a8467  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
008a846a  898aac030000          MOV dword ptr [EDX + 0x3ac],ECX
008a8470  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008a8473  e848980000            CALL 0x008b1cc0
008a8478  8be5                  MOV ESP,EBP
008a847a  5d                    POP EBP
008a847b  c20400                RET 0x4
