; FN 008a8cf0 FUN_008a8cf0
008a8cf0  55                    PUSH EBP
008a8cf1  8bec                  MOV EBP,ESP
008a8cf3  83ec14                SUB ESP,0x14
008a8cf6  894dec                MOV dword ptr [EBP + -0x14],ECX
008a8cf9  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a8cfc  8b485c                MOV ECX,dword ptr [EAX + 0x5c]
008a8cff  894dfc                MOV dword ptr [EBP + -0x4],ECX
008a8d02  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
008a8d08  8b4204                MOV EAX,dword ptr [EDX + 0x4]
008a8d0b  8945f8                MOV dword ptr [EBP + -0x8],EAX
008a8d0e  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
008a8d11  e83a6ae9ff            CALL 0x0073f750
008a8d16  8945f4                MOV dword ptr [EBP + -0xc],EAX
008a8d19  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008a8d1c  8a55fc                MOV DL,byte ptr [EBP + -0x4]
008a8d1f  8891c10f0000          MOV byte ptr [ECX + 0xfc1],DL
008a8d25  6824fea700            PUSH 0xa7fe24
008a8d2a  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8d2d  81c178190000          ADD ECX,0x1978
008a8d33  e808e4b6ff            CALL 0x00417140
008a8d38  6824fea700            PUSH 0xa7fe24
008a8d3d  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8d40  81c1481b0000          ADD ECX,0x1b48
008a8d46  e8f5e3b6ff            CALL 0x00417140
008a8d4b  6824fea700            PUSH 0xa7fe24
008a8d50  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8d53  81c1181d0000          ADD ECX,0x1d18
008a8d59  e8e2e3b6ff            CALL 0x00417140
008a8d5e  6824fea700            PUSH 0xa7fe24
008a8d63  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8d66  81c1e81e0000          ADD ECX,0x1ee8
008a8d6c  e8cfe3b6ff            CALL 0x00417140
008a8d71  6824fea700            PUSH 0xa7fe24
008a8d76  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8d79  81c1b8200000          ADD ECX,0x20b8
008a8d7f  e8bce3b6ff            CALL 0x00417140
008a8d84  6824fea700            PUSH 0xa7fe24
008a8d89  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8d8c  81c158240000          ADD ECX,0x2458
008a8d92  e8a9e3b6ff            CALL 0x00417140
008a8d97  6824fea700            PUSH 0xa7fe24
008a8d9c  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8d9f  81c188220000          ADD ECX,0x2288
008a8da5  e896e3b6ff            CALL 0x00417140
008a8daa  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a8dad  8b10                  MOV EDX,dword ptr [EAX]
008a8daf  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8db2  8b4258                MOV EAX,dword ptr [EDX + 0x58]
008a8db5  ffd0                  CALL EAX
008a8db7  8945f0                MOV dword ptr [EBP + -0x10],EAX
008a8dba  68acfda700            PUSH 0xa7fdac
008a8dbf  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8dc2  81c178010000          ADD ECX,0x178
008a8dc8  e873e3b6ff            CALL 0x00417140
008a8dcd  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a8dd0  8b91d0240000          MOV EDX,dword ptr [ECX + 0x24d0]
008a8dd6  83ca04                OR EDX,0x4
008a8dd9  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008a8ddc  8990d0240000          MOV dword ptr [EAX + 0x24d0],EDX
008a8de2  8be5                  MOV ESP,EBP
008a8de4  5d                    POP EBP
008a8de5  c20400                RET 0x4
