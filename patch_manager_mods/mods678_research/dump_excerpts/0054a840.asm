; FN 0054a840 FUN_0054a840
0054a840  55                    PUSH EBP
0054a841  8bec                  MOV EBP,ESP
0054a843  83ec18                SUB ESP,0x18
0054a846  894de8                MOV dword ptr [EBP + -0x18],ECX
0054a849  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0054a84c  83b86402000000        CMP dword ptr [EAX + 0x264],0x0
0054a853  7577                  JNZ 0x0054a8cc
0054a855  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0054a858  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0054a85b  8955f4                MOV dword ptr [EBP + -0xc],EDX
0054a85e  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0054a863  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0054a866  894df0                MOV dword ptr [EBP + -0x10],ECX
0054a869  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0054a86c  52                    PUSH EDX
0054a86d  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0054a870  e84b561f00            CALL 0x0073fec0
0054a875  8945f8                MOV dword ptr [EBP + -0x8],EAX
0054a878  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0054a87d  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0054a880  894dec                MOV dword ptr [EBP + -0x14],ECX
0054a883  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0054a886  52                    PUSH EDX
0054a887  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0054a88a  e8414c1f00            CALL 0x0073f4d0
0054a88f  8945fc                MOV dword ptr [EBP + -0x4],EAX
0054a892  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0054a896  7434                  JZ 0x0054a8cc
0054a898  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0054a89b  8b10                  MOV EDX,dword ptr [EAX]
0054a89d  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0054a8a0  8b420c                MOV EAX,dword ptr [EDX + 0xc]
0054a8a3  ffd0                  CALL EAX
0054a8a5  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0054a8a8  898164020000          MOV dword ptr [ECX + 0x264],EAX
0054a8ae  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0054a8b1  83ba6402000000        CMP dword ptr [EDX + 0x264],0x0
0054a8b8  7412                  JZ 0x0054a8cc
0054a8ba  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0054a8bd  50                    PUSH EAX
0054a8be  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0054a8c1  8b8964020000          MOV ECX,dword ptr [ECX + 0x264]
0054a8c7  e8447d2a00            CALL 0x007f2610
0054a8cc  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0054a8cf  8b8264020000          MOV EAX,dword ptr [EDX + 0x264]
0054a8d5  8be5                  MOV ESP,EBP
0054a8d7  5d                    POP EBP
0054a8d8  c3                    RET
