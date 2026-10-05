; FN 007e5da0 FUN_007e5da0
007e5da0  55                    PUSH EBP
007e5da1  8bec                  MOV EBP,ESP
007e5da3  83ec0c                SUB ESP,0xc
007e5da6  894df4                MOV dword ptr [EBP + -0xc],ECX
007e5da9  837d0800              CMP dword ptr [EBP + 0x8],0x0
007e5dad  7c37                  JL 0x007e5de6
007e5daf  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007e5db2  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
007e5db5  3b08                  CMP ECX,dword ptr [EAX]
007e5db7  7d2d                  JGE 0x007e5de6
007e5db9  8b5508                MOV EDX,dword ptr [EBP + 0x8]
007e5dbc  69d288000000          IMUL EDX,EDX,0x88
007e5dc2  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007e5dc5  8b4c1024              MOV ECX,dword ptr [EAX + EDX*0x1 + 0x24]
007e5dc9  894dfc                MOV dword ptr [EBP + -0x4],ECX
007e5dcc  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007e5dd2  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007e5dd5  8945f8                MOV dword ptr [EBP + -0x8],EAX
007e5dd8  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007e5ddb  51                    PUSH ECX
007e5ddc  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007e5ddf  e86c97f5ff            CALL 0x0073f550
007e5de4  eb02                  JMP 0x007e5de8
007e5de6  33c0                  XOR EAX,EAX
007e5de8  8be5                  MOV ESP,EBP
007e5dea  5d                    POP EBP
007e5deb  c20400                RET 0x4
