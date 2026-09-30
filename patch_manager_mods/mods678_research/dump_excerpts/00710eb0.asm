; FN 00710eb0 FUN_00710eb0
00710eb0  55                    PUSH EBP
00710eb1  8bec                  MOV EBP,ESP
00710eb3  83ec08                SUB ESP,0x8
00710eb6  894df8                MOV dword ptr [EBP + -0x8],ECX
00710eb9  6a10                  PUSH 0x10
00710ebb  6a00                  PUSH 0x0
00710ebd  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00710ec0  50                    PUSH EAX
00710ec1  e86ab72000            CALL 0x0091c630
00710ec6  83c40c                ADD ESP,0xc
00710ec9  837d0c00              CMP dword ptr [EBP + 0xc],0x0
00710ecd  7441                  JZ 0x00710f10
00710ecf  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00710ed6  eb09                  JMP 0x00710ee1
00710ed8  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00710edb  83c101                ADD ECX,0x1
00710ede  894dfc                MOV dword ptr [EBP + -0x4],ECX
00710ee1  837dfc10              CMP dword ptr [EBP + -0x4],0x10
00710ee5  7329                  JNC 0x00710f10
00710ee7  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00710eea  0355fc                ADD EDX,dword ptr [EBP + -0x4]
00710eed  0fbe02                MOVSX EAX,byte ptr [EDX]
00710ef0  85c0                  TEST EAX,EAX
00710ef2  741c                  JZ 0x00710f10
00710ef4  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00710ef7  034dfc                ADD ECX,dword ptr [EBP + -0x4]
00710efa  0fbe11                MOVSX EDX,byte ptr [ECX]
00710efd  52                    PUSH EDX
00710efe  e817d12000            CALL 0x0091e01a
00710f03  83c404                ADD ESP,0x4
00710f06  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00710f09  034dfc                ADD ECX,dword ptr [EBP + -0x4]
00710f0c  8801                  MOV byte ptr [ECX],AL
00710f0e  ebc8                  JMP 0x00710ed8
00710f10  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
00710f13  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00710f16  8b0a                  MOV ECX,dword ptr [EDX]
00710f18  8908                  MOV dword ptr [EAX],ECX
00710f1a  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
00710f1d  894804                MOV dword ptr [EAX + 0x4],ECX
00710f20  8b4a08                MOV ECX,dword ptr [EDX + 0x8]
00710f23  894808                MOV dword ptr [EAX + 0x8],ECX
00710f26  8b520c                MOV EDX,dword ptr [EDX + 0xc]
00710f29  89500c                MOV dword ptr [EAX + 0xc],EDX
00710f2c  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00710f2f  8be5                  MOV ESP,EBP
00710f31  5d                    POP EBP
00710f32  c20800                RET 0x8
