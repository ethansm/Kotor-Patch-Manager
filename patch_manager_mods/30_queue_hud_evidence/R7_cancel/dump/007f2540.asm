; FN 007f2540 FUN_007f2540
007f2540  55                    PUSH EBP
007f2541  8bec                  MOV EBP,ESP
007f2543  83ec20                SUB ESP,0x20
007f2546  894de0                MOV dword ptr [EBP + -0x20],ECX
007f2549  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
007f254c  83b8f800000000        CMP dword ptr [EAX + 0xf8],0x0
007f2553  0f85aa000000          JNZ 0x007f2603
007f2559  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
007f255c  8b91e4000000          MOV EDX,dword ptr [ECX + 0xe4]
007f2562  8955f4                MOV dword ptr [EBP + -0xc],EDX
007f2565  837df400              CMP dword ptr [EBP + -0xc],0x0
007f2569  7407                  JZ 0x007f2572
007f256b  33c0                  XOR EAX,EAX
007f256d  e99a000000            JMP 0x007f260c
007f2572  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007f2577  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007f257a  894df0                MOV dword ptr [EBP + -0x10],ECX
007f257d  837df000              CMP dword ptr [EBP + -0x10],0x0
007f2581  7507                  JNZ 0x007f258a
007f2583  33c0                  XOR EAX,EAX
007f2585  e982000000            JMP 0x007f260c
007f258a  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
007f258d  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007f2590  8945ec                MOV dword ptr [EBP + -0x14],EAX
007f2593  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007f2599  8b5108                MOV EDX,dword ptr [ECX + 0x8]
007f259c  8955e8                MOV dword ptr [EBP + -0x18],EDX
007f259f  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
007f25a2  50                    PUSH EAX
007f25a3  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
007f25a6  e805a3d2ff            CALL 0x0051c8b0
007f25ab  8945f8                MOV dword ptr [EBP + -0x8],EAX
007f25ae  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007f25b4  8b5108                MOV EDX,dword ptr [ECX + 0x8]
007f25b7  8955e4                MOV dword ptr [EBP + -0x1c],EDX
007f25ba  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
007f25bd  50                    PUSH EAX
007f25be  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
007f25c1  e8da9ad2ff            CALL 0x0051c0a0
007f25c6  8945fc                MOV dword ptr [EBP + -0x4],EAX
007f25c9  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007f25cd  7434                  JZ 0x007f2603
007f25cf  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007f25d2  8b11                  MOV EDX,dword ptr [ECX]
007f25d4  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007f25d7  8b4210                MOV EAX,dword ptr [EDX + 0x10]
007f25da  ffd0                  CALL EAX
007f25dc  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
007f25df  8981f8000000          MOV dword ptr [ECX + 0xf8],EAX
007f25e5  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
007f25e8  83baf800000000        CMP dword ptr [EDX + 0xf8],0x0
007f25ef  7412                  JZ 0x007f2603
007f25f1  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
007f25f4  50                    PUSH EAX
007f25f5  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
007f25f8  8b89f8000000          MOV ECX,dword ptr [ECX + 0xf8]
007f25fe  e8dd82d5ff            CALL 0x0054a8e0
007f2603  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
007f2606  8b82f8000000          MOV EAX,dword ptr [EDX + 0xf8]
007f260c  8be5                  MOV ESP,EBP
007f260e  5d                    POP EBP
007f260f  c3                    RET
