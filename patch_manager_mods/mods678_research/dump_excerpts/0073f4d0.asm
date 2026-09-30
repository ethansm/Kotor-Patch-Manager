; FN 0073f4d0 FUN_0073f4d0
0073f4d0  55                    PUSH EBP
0073f4d1  8bec                  MOV EBP,ESP
0073f4d3  83ec10                SUB ESP,0x10
0073f4d6  894df0                MOV dword ptr [EBP + -0x10],ECX
0073f4d9  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0073f4dc  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0073f4df  894df4                MOV dword ptr [EBP + -0xc],ECX
0073f4e2  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0073f4e9  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0073f4ec  8b4214                MOV EAX,dword ptr [EDX + 0x14]
0073f4ef  8945f8                MOV dword ptr [EBP + -0x8],EAX
0073f4f2  8d4dfc                LEA ECX,[EBP + -0x4]
0073f4f5  51                    PUSH ECX
0073f4f6  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0073f4f9  52                    PUSH EDX
0073f4fa  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0073f4fd  e8aeeadfff            CALL 0x0053dfb0
0073f502  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0073f505  8be5                  MOV ESP,EBP
0073f507  5d                    POP EBP
0073f508  c20400                RET 0x4
