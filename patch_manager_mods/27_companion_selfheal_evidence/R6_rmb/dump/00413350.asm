; FN 00413350 FUN_00413350
00413350  55                    PUSH EBP
00413351  8bec                  MOV EBP,ESP
00413353  51                    PUSH ECX
00413354  894dfc                MOV dword ptr [EBP + -0x4],ECX
00413357  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0041335a  8b481c                MOV ECX,dword ptr [EAX + 0x1c]
0041335d  c1e903                SHR ECX,0x3
00413360  83e101                AND ECX,0x1
00413363  7408                  JZ 0x0041336d
00413365  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00413368  e8930b0000            CALL 0x00413f00
0041336d  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00413370  837a1000              CMP dword ptr [EDX + 0x10],0x0
00413374  7404                  JZ 0x0041337a
00413376  33c0                  XOR EAX,EAX
00413378  eb3a                  JMP 0x004133b4
0041337a  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0041337d  e8ce080000            CALL 0x00413c50
00413382  6a01                  PUSH 0x1
00413384  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00413387  e8540c0000            CALL 0x00413fe0
0041338c  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0041338f  83780800              CMP dword ptr [EAX + 0x8],0x0
00413393  741d                  JZ 0x004133b2
00413395  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00413398  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0041339b  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0041339e  8b4808                MOV ECX,dword ptr [EAX + 0x8]
004133a1  8b12                  MOV EDX,dword ptr [EDX]
004133a3  8b8284000000          MOV EAX,dword ptr [EDX + 0x84]
004133a9  ffd0                  CALL EAX
004133ab  b801000000            MOV EAX,0x1
004133b0  eb02                  JMP 0x004133b4
004133b2  33c0                  XOR EAX,EAX
004133b4  8be5                  MOV ESP,EBP
004133b6  5d                    POP EBP
004133b7  c3                    RET
