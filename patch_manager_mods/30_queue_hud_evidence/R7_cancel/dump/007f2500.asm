; FN 007f2500 FUN_007f2500
007f2500  55                    PUSH EBP
007f2501  8bec                  MOV EBP,ESP
007f2503  83ec0c                SUB ESP,0xc
007f2506  894df4                MOV dword ptr [EBP + -0xc],ECX
007f2509  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007f250e  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007f2511  894dfc                MOV dword ptr [EBP + -0x4],ECX
007f2514  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007f2517  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007f251a  8945f8                MOV dword ptr [EBP + -0x8],EAX
007f251d  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007f2520  51                    PUSH ECX
007f2521  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007f2524  e887a3d2ff            CALL 0x0051c8b0
007f2529  50                    PUSH EAX
007f252a  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007f252d  e86ea6d2ff            CALL 0x0051cba0
007f2532  8be5                  MOV ESP,EBP
007f2534  5d                    POP EBP
007f2535  c3                    RET
