; FN 00418af0 FUN_00418af0
00418af0  55                    PUSH EBP
00418af1  8bec                  MOV EBP,ESP
00418af3  83ec1c                SUB ESP,0x1c
00418af6  894de4                MOV dword ptr [EBP + -0x1c],ECX
00418af9  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
00418afc  8b4840                MOV ECX,dword ptr [EAX + 0x40]
00418aff  894dfc                MOV dword ptr [EBP + -0x4],ECX
00418b02  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
00418b09  eb09                  JMP 0x00418b14
00418b0b  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
00418b0e  83c201                ADD EDX,0x1
00418b11  8955f8                MOV dword ptr [EBP + -0x8],EDX
00418b14  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00418b17  3b45fc                CMP EAX,dword ptr [EBP + -0x4]
00418b1a  7d46                  JGE 0x00418b62
00418b1c  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00418b1f  6bc90c                IMUL ECX,ECX,0xc
00418b22  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
00418b25  034a3c                ADD ECX,dword ptr [EDX + 0x3c]
00418b28  894de8                MOV dword ptr [EBP + -0x18],ECX
00418b2b  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
00418b2e  8b4808                MOV ECX,dword ptr [EAX + 0x8]
00418b31  3b4d08                CMP ECX,dword ptr [EBP + 0x8]
00418b34  752a                  JNZ 0x00418b60
00418b36  837d1000              CMP dword ptr [EBP + 0x10],0x0
00418b3a  7413                  JZ 0x00418b4f
00418b3c  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
00418b3f  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00418b42  8902                  MOV dword ptr [EDX],EAX
00418b44  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00418b47  8b5510                MOV EDX,dword ptr [EBP + 0x10]
00418b4a  895104                MOV dword ptr [ECX + 0x4],EDX
00418b4d  eb0f                  JMP 0x00418b5e
00418b4f  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00418b52  50                    PUSH EAX
00418b53  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
00418b56  83c13c                ADD ECX,0x3c
00418b59  e8e28d3800            CALL 0x007a1940
00418b5e  eb3b                  JMP 0x00418b9b
00418b60  eba9                  JMP 0x00418b0b
00418b62  837d1000              CMP dword ptr [EBP + 0x10],0x0
00418b66  7433                  JZ 0x00418b9b
00418b68  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00418b6b  894df4                MOV dword ptr [EBP + -0xc],ECX
00418b6e  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00418b71  8955ec                MOV dword ptr [EBP + -0x14],EDX
00418b74  8b4510                MOV EAX,dword ptr [EBP + 0x10]
00418b77  8945f0                MOV dword ptr [EBP + -0x10],EAX
00418b7a  83ec0c                SUB ESP,0xc
00418b7d  8bcc                  MOV ECX,ESP
00418b7f  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00418b82  8911                  MOV dword ptr [ECX],EDX
00418b84  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00418b87  894104                MOV dword ptr [ECX + 0x4],EAX
00418b8a  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00418b8d  895108                MOV dword ptr [ECX + 0x8],EDX
00418b90  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
00418b93  83c13c                ADD ECX,0x3c
00418b96  e815d44800            CALL 0x008a5fb0
00418b9b  8be5                  MOV ESP,EBP
00418b9d  5d                    POP EBP
00418b9e  c20c00                RET 0xc
