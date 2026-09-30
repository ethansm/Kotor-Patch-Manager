; FN 0058a330 FUN_0058a330
0058a330  55                    PUSH EBP
0058a331  8bec                  MOV EBP,ESP
0058a333  6aff                  PUSH -0x1
0058a335  68ab169500            PUSH 0x9516ab
0058a33a  64a100000000          MOV EAX,FS:[0x0]
0058a340  50                    PUSH EAX
0058a341  83ec40                SUB ESP,0x40
0058a344  a1201fa100            MOV EAX,[0x00a11f20]
0058a349  33c5                  XOR EAX,EBP
0058a34b  50                    PUSH EAX
0058a34c  8d45f4                LEA EAX,[EBP + -0xc]
0058a34f  64a300000000          MOV FS:[0x0],EAX
0058a355  894db8                MOV dword ptr [EBP + -0x48],ECX
0058a358  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
0058a35b  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0058a35e  8988ac110000          MOV dword ptr [EAX + 0x11ac],ECX
0058a364  837d0c00              CMP dword ptr [EBP + 0xc],0x0
0058a368  0f84ef000000          JZ 0x0058a45d
0058a36e  837d0800              CMP dword ptr [EBP + 0x8],0x0
0058a372  0f8485000000          JZ 0x0058a3fd
0058a378  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
0058a37b  83ba0005000000        CMP dword ptr [EDX + 0x500],0x0
0058a382  7547                  JNZ 0x0058a3cb
0058a384  6a3c                  PUSH 0x3c
0058a386  e898f33800            CALL 0x00919723
0058a38b  83c404                ADD ESP,0x4
0058a38e  8945ec                MOV dword ptr [EBP + -0x14],EAX
0058a391  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0058a398  837dec00              CMP dword ptr [EBP + -0x14],0x0
0058a39c  740d                  JZ 0x0058a3ab
0058a39e  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0058a3a1  e81aaffeff            CALL 0x005752c0
0058a3a6  8945b4                MOV dword ptr [EBP + -0x4c],EAX
0058a3a9  eb07                  JMP 0x0058a3b2
0058a3ab  c745b400000000        MOV dword ptr [EBP + -0x4c],0x0
0058a3b2  8b45b4                MOV EAX,dword ptr [EBP + -0x4c]
0058a3b5  8945f0                MOV dword ptr [EBP + -0x10],EAX
0058a3b8  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0058a3bf  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
0058a3c2  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0058a3c5  899100050000          MOV dword ptr [ECX + 0x500],EDX
0058a3cb  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
0058a3ce  8b4878                MOV ECX,dword ptr [EAX + 0x78]
0058a3d1  894dc8                MOV dword ptr [EBP + -0x38],ECX
0058a3d4  837dc804              CMP dword ptr [EBP + -0x38],0x4
0058a3d8  7421                  JZ 0x0058a3fb
0058a3da  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0058a3e0  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0058a3e3  8945c4                MOV dword ptr [EBP + -0x3c],EAX
0058a3e6  6a04                  PUSH 0x4
0058a3e8  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
0058a3eb  51                    PUSH ECX
0058a3ec  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0058a3ef  e88c1ef9ff            CALL 0x0051c280
0058a3f4  8bc8                  MOV ECX,EAX
0058a3f6  e8253df9ff            CALL 0x0051e120
0058a3fb  eb60                  JMP 0x0058a45d
0058a3fd  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
0058a400  83ba0005000000        CMP dword ptr [EDX + 0x500],0x0
0058a407  7425                  JZ 0x0058a42e
0058a409  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
0058a40c  8b8800050000          MOV ECX,dword ptr [EAX + 0x500]
0058a412  894de8                MOV dword ptr [EBP + -0x18],ECX
0058a415  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0058a418  52                    PUSH EDX
0058a419  e8dff23800            CALL 0x009196fd
0058a41e  83c404                ADD ESP,0x4
0058a421  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
0058a424  c7800005000000000000  MOV dword ptr [EAX + 0x500],0x0
0058a42e  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
0058a431  8b5178                MOV EDX,dword ptr [ECX + 0x78]
0058a434  8955c0                MOV dword ptr [EBP + -0x40],EDX
0058a437  837dc002              CMP dword ptr [EBP + -0x40],0x2
0058a43b  7420                  JZ 0x0058a45d
0058a43d  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0058a442  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0058a445  894dbc                MOV dword ptr [EBP + -0x44],ECX
0058a448  6a02                  PUSH 0x2
0058a44a  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
0058a44d  52                    PUSH EDX
0058a44e  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0058a451  e82a1ef9ff            CALL 0x0051c280
0058a456  8bc8                  MOV ECX,EAX
0058a458  e8c33cf9ff            CALL 0x0051e120
0058a45d  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0058a460  64890d00000000        MOV dword ptr FS:[0x0],ECX
0058a467  59                    POP ECX
0058a468  8be5                  MOV ESP,EBP
0058a46a  5d                    POP EBP
0058a46b  c20800                RET 0x8
