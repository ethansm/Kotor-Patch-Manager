; FN 0087a350 FUN_0087a350
0087a350  55                    PUSH EBP
0087a351  8bec                  MOV EBP,ESP
0087a353  83ec20                SUB ESP,0x20
0087a356  894de4                MOV dword ptr [EBP + -0x1c],ECX
0087a359  837d1400              CMP dword ptr [EBP + 0x14],0x0
0087a35d  740e                  JZ 0x0087a36d
0087a35f  8b4514                MOV EAX,dword ptr [EBP + 0x14]
0087a362  8945fc                MOV dword ptr [EBP + -0x4],EAX
0087a365  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
0087a368  894df4                MOV dword ptr [EBP + -0xc],ECX
0087a36b  eb1c                  JMP 0x0087a389
0087a36d  c745fc03000000        MOV dword ptr [EBP + -0x4],0x3
0087a374  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0087a377  52                    PUSH EDX
0087a378  e8a6f30900            CALL 0x00919723
0087a37d  83c404                ADD ESP,0x4
0087a380  8945f0                MOV dword ptr [EBP + -0x10],EAX
0087a383  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0087a386  8945f4                MOV dword ptr [EBP + -0xc],EAX
0087a389  837df400              CMP dword ptr [EBP + -0xc],0x0
0087a38d  7504                  JNZ 0x0087a393
0087a38f  33c0                  XOR EAX,EAX
0087a391  eb6d                  JMP 0x0087a400
0087a393  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0087a396  c60170                MOV byte ptr [ECX],0x70
0087a399  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0087a39c  8a4508                MOV AL,byte ptr [EBP + 0x8]
0087a39f  884201                MOV byte ptr [EDX + 0x1],AL
0087a3a2  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0087a3a5  8a550c                MOV DL,byte ptr [EBP + 0xc]
0087a3a8  885102                MOV byte ptr [ECX + 0x2],DL
0087a3ab  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0087a3ae  8945e0                MOV dword ptr [EBP + -0x20],EAX
0087a3b1  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0087a3b7  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0087a3ba  8955e0                MOV dword ptr [EBP + -0x20],EDX
0087a3bd  6a00                  PUSH 0x0
0087a3bf  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0087a3c2  50                    PUSH EAX
0087a3c3  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0087a3c6  51                    PUSH ECX
0087a3c7  8b15c0b69900          MOV EDX,dword ptr [0x0099b6c0]
0087a3cd  52                    PUSH EDX
0087a3ce  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
0087a3d1  8b10                  MOV EDX,dword ptr [EAX]
0087a3d3  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0087a3d6  8b4210                MOV EAX,dword ptr [EDX + 0x10]
0087a3d9  ffd0                  CALL EAX
0087a3db  8bc8                  MOV ECX,EAX
0087a3dd  e87e9ce8ff            CALL 0x00704060
0087a3e2  8945f8                MOV dword ptr [EBP + -0x8],EAX
0087a3e5  837d1400              CMP dword ptr [EBP + 0x14],0x0
0087a3e9  7512                  JNZ 0x0087a3fd
0087a3eb  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0087a3ee  894dec                MOV dword ptr [EBP + -0x14],ECX
0087a3f1  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
0087a3f4  52                    PUSH EDX
0087a3f5  e803f30900            CALL 0x009196fd
0087a3fa  83c404                ADD ESP,0x4
0087a3fd  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0087a400  8be5                  MOV ESP,EBP
0087a402  5d                    POP EBP
0087a403  c21000                RET 0x10
