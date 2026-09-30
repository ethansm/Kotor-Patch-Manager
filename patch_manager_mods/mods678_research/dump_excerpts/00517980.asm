; FN 00517980 FUN_00517980
00517980  55                    PUSH EBP
00517981  8bec                  MOV EBP,ESP
00517983  51                    PUSH ECX
00517984  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00517987  d900                  FLD float ptr [EAX]
00517989  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0051798c  d901                  FLD float ptr [ECX]
0051798e  dae9                  FUCOMPP
00517990  dfe0                  FNSTSW AX
00517992  f6c444                TEST AH,0x44
00517995  7a33                  JP 0x005179ca
00517997  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0051799a  d94204                FLD float ptr [EDX + 0x4]
0051799d  8b450c                MOV EAX,dword ptr [EBP + 0xc]
005179a0  d94004                FLD float ptr [EAX + 0x4]
005179a3  dae9                  FUCOMPP
005179a5  dfe0                  FNSTSW AX
005179a7  f6c444                TEST AH,0x44
005179aa  7a1e                  JP 0x005179ca
005179ac  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005179af  d94108                FLD float ptr [ECX + 0x8]
005179b2  8b550c                MOV EDX,dword ptr [EBP + 0xc]
005179b5  d94208                FLD float ptr [EDX + 0x8]
005179b8  dae9                  FUCOMPP
005179ba  dfe0                  FNSTSW AX
005179bc  f6c444                TEST AH,0x44
005179bf  7a09                  JP 0x005179ca
005179c1  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
005179c8  eb07                  JMP 0x005179d1
005179ca  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005179d1  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005179d4  8be5                  MOV ESP,EBP
005179d6  5d                    POP EBP
005179d7  c3                    RET
