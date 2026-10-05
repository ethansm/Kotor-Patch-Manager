; FN 004133c0 FUN_004133c0
004133c0  55                    PUSH EBP
004133c1  8bec                  MOV EBP,ESP
004133c3  83ec0c                SUB ESP,0xc
004133c6  894df4                MOV dword ptr [EBP + -0xc],ECX
004133c9  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
004133d0  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
004133d3  83781000              CMP dword ptr [EAX + 0x10],0x0
004133d7  7434                  JZ 0x0041340d
004133d9  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
004133dc  8b510c                MOV EDX,dword ptr [ECX + 0xc]
004133df  8955f8                MOV dword ptr [EBP + -0x8],EDX
004133e2  837df804              CMP dword ptr [EBP + -0x8],0x4
004133e6  751e                  JNZ 0x00413406
004133e8  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
004133eb  8b4810                MOV ECX,dword ptr [EAX + 0x10]
004133ee  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
004133f1  8b4210                MOV EAX,dword ptr [EDX + 0x10]
004133f4  8b11                  MOV EDX,dword ptr [ECX]
004133f6  8bc8                  MOV ECX,EAX
004133f8  8b4224                MOV EAX,dword ptr [EDX + 0x24]
004133fb  ffd0                  CALL EAX
004133fd  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
00413404  eb07                  JMP 0x0041340d
00413406  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0041340d  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00413410  8be5                  MOV ESP,EBP
00413412  5d                    POP EBP
00413413  c3                    RET
