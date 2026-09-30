; FN 008a8b30 FUN_008a8b30
008a8b30  55                    PUSH EBP
008a8b31  8bec                  MOV EBP,ESP
008a8b33  83ec14                SUB ESP,0x14
008a8b36  894df0                MOV dword ptr [EBP + -0x10],ECX
008a8b39  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a8b3c  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a8b3f  837dfc0c              CMP dword ptr [EBP + -0x4],0xc
008a8b43  7d06                  JGE 0x008a8b4b
008a8b45  837dfc00              CMP dword ptr [EBP + -0x4],0x0
008a8b49  7d05                  JGE 0x008a8b50
008a8b4b  e968010000            JMP 0x008a8cb8
008a8b50  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
008a8b56  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008a8b59  8955f8                MOV dword ptr [EBP + -0x8],EDX
008a8b5c  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
008a8b5f  e8ec6be9ff            CALL 0x0073f750
008a8b64  8945f4                MOV dword ptr [EBP + -0xc],EAX
008a8b67  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
008a8b6a  8a4dfc                MOV CL,byte ptr [EBP + -0x4]
008a8b6d  8888c10f0000          MOV byte ptr [EAX + 0xfc1],CL
008a8b73  6824fea700            PUSH 0xa7fe24
008a8b78  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8b7b  81c178190000          ADD ECX,0x1978
008a8b81  e8bae5b6ff            CALL 0x00417140
008a8b86  6824fea700            PUSH 0xa7fe24
008a8b8b  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8b8e  81c1481b0000          ADD ECX,0x1b48
008a8b94  e8a7e5b6ff            CALL 0x00417140
008a8b99  6824fea700            PUSH 0xa7fe24
008a8b9e  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8ba1  81c1181d0000          ADD ECX,0x1d18
008a8ba7  e894e5b6ff            CALL 0x00417140
008a8bac  6824fea700            PUSH 0xa7fe24
008a8bb1  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8bb4  81c1e81e0000          ADD ECX,0x1ee8
008a8bba  e881e5b6ff            CALL 0x00417140
008a8bbf  6824fea700            PUSH 0xa7fe24
008a8bc4  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8bc7  81c1b8200000          ADD ECX,0x20b8
008a8bcd  e86ee5b6ff            CALL 0x00417140
008a8bd2  6824fea700            PUSH 0xa7fe24
008a8bd7  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8bda  81c158240000          ADD ECX,0x2458
008a8be0  e85be5b6ff            CALL 0x00417140
008a8be5  6824fea700            PUSH 0xa7fe24
008a8bea  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8bed  81c188220000          ADD ECX,0x2288
008a8bf3  e848e5b6ff            CALL 0x00417140
008a8bf8  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008a8bfb  8955ec                MOV dword ptr [EBP + -0x14],EDX
008a8bfe  837dec0b              CMP dword ptr [EBP + -0x14],0xb
008a8c02  0f879b000000          JA 0x008a8ca3
008a8c08  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008a8c0b  ff2485c08c8a00        JMP dword ptr [EAX*0x4 + 0x8a8cc0]
008a8c12  68acfda700            PUSH 0xa7fdac
008a8c17  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8c1a  81c178190000          ADD ECX,0x1978
008a8c20  e81be5b6ff            CALL 0x00417140
008a8c25  eb7c                  JMP 0x008a8ca3
008a8c27  68acfda700            PUSH 0xa7fdac
008a8c2c  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8c2f  81c1481b0000          ADD ECX,0x1b48
008a8c35  e806e5b6ff            CALL 0x00417140
008a8c3a  eb67                  JMP 0x008a8ca3
008a8c3c  68acfda700            PUSH 0xa7fdac
008a8c41  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8c44  81c1181d0000          ADD ECX,0x1d18
008a8c4a  e8f1e4b6ff            CALL 0x00417140
008a8c4f  eb52                  JMP 0x008a8ca3
008a8c51  68acfda700            PUSH 0xa7fdac
008a8c56  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8c59  81c1e81e0000          ADD ECX,0x1ee8
008a8c5f  e8dce4b6ff            CALL 0x00417140
008a8c64  eb3d                  JMP 0x008a8ca3
008a8c66  68acfda700            PUSH 0xa7fdac
008a8c6b  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8c6e  81c1b8200000          ADD ECX,0x20b8
008a8c74  e8c7e4b6ff            CALL 0x00417140
008a8c79  eb28                  JMP 0x008a8ca3
008a8c7b  68acfda700            PUSH 0xa7fdac
008a8c80  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8c83  81c158240000          ADD ECX,0x2458
008a8c89  e8b2e4b6ff            CALL 0x00417140
008a8c8e  eb13                  JMP 0x008a8ca3
008a8c90  68acfda700            PUSH 0xa7fdac
008a8c95  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8c98  81c188220000          ADD ECX,0x2288
008a8c9e  e89de4b6ff            CALL 0x00417140
008a8ca3  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a8ca6  8b91d0240000          MOV EDX,dword ptr [ECX + 0x24d0]
008a8cac  83ca04                OR EDX,0x4
008a8caf  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
008a8cb2  8990d0240000          MOV dword ptr [EAX + 0x24d0],EDX
008a8cb8  8be5                  MOV ESP,EBP
008a8cba  5d                    POP EBP
008a8cbb  c20400                RET 0x4
