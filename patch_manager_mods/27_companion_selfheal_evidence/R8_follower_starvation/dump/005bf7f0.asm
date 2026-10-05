; FN 005bf7f0 FUN_005bf7f0
005bf7f0  55                    PUSH EBP
005bf7f1  8bec                  MOV EBP,ESP
005bf7f3  83ec24                SUB ESP,0x24
005bf7f6  894ddc                MOV dword ptr [EBP + -0x24],ECX
005bf7f9  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005bf7fc  8b88ac110000          MOV ECX,dword ptr [EAX + 0x11ac]
005bf802  894df8                MOV dword ptr [EBP + -0x8],ECX
005bf805  837df800              CMP dword ptr [EBP + -0x8],0x0
005bf809  7476                  JZ 0x005bf881
005bf80b  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
005bf80e  8b8200050000          MOV EAX,dword ptr [EDX + 0x500]
005bf814  8945f4                MOV dword ptr [EBP + -0xc],EAX
005bf817  837df400              CMP dword ptr [EBP + -0xc],0x0
005bf81b  7464                  JZ 0x005bf881
005bf81d  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
005bf820  8b11                  MOV EDX,dword ptr [ECX]
005bf822  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
005bf825  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
005bf82b  ffd0                  CALL EAX
005bf82d  83f801                CMP EAX,0x1
005bf830  744f                  JZ 0x005bf881
005bf832  33c9                  XOR ECX,ECX
005bf834  83f901                CMP ECX,0x1
005bf837  7448                  JZ 0x005bf881
005bf839  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
005bf83c  83baac11000000        CMP dword ptr [EDX + 0x11ac],0x0
005bf843  7423                  JZ 0x005bf868
005bf845  6a00                  PUSH 0x0
005bf847  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005bf84a  8b10                  MOV EDX,dword ptr [EAX]
005bf84c  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
005bf84f  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
005bf855  ffd0                  CALL EAX
005bf857  0fbfc8                MOVSX ECX,AX
005bf85a  33d2                  XOR EDX,EDX
005bf85c  85c9                  TEST ECX,ECX
005bf85e  0f9ec2                SETLE DL
005bf861  8955f0                MOV dword ptr [EBP + -0x10],EDX
005bf864  eb09                  JMP 0x005bf86f
005bf868  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
005bf86f  837df001              CMP dword ptr [EBP + -0x10],0x1
005bf873  740c                  JZ 0x005bf881
005bf875  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005bf878  83b88003000000        CMP dword ptr [EAX + 0x380],0x0
005bf87f  750a                  JNZ 0x005bf88b
005bf881  a1bc449900            MOV EAX,[0x009944bc]
005bf886  e9a9000000            JMP 0x005bf934
005bf88b  b902000000            MOV ECX,0x2
005bf890  0fb7d1                MOVZX EDX,CX
005bf893  b802000000            MOV EAX,0x2
005bf898  0fb7c8                MOVZX ECX,AX
005bf89b  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005bf89e  0fb78014110000        MOVZX EAX,word ptr [EAX + 0x1114]
005bf8a5  23c1                  AND EAX,ECX
005bf8a7  33c9                  XOR ECX,ECX
005bf8a9  3bc2                  CMP EAX,EDX
005bf8ab  0f94c1                SETZ CL
005bf8ae  85c9                  TEST ECX,ECX
005bf8b0  7507                  JNZ 0x005bf8b9
005bf8b2  a1bc449900            MOV EAX,[0x009944bc]
005bf8b7  eb7b                  JMP 0x005bf934
005bf8b9  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
005bf8bc  e8ff5af8ff            CALL 0x005453c0
005bf8c1  85c0                  TEST EAX,EAX
005bf8c3  7507                  JNZ 0x005bf8cc
005bf8c5  a1b4449900            MOV EAX,[0x009944b4]
005bf8ca  eb68                  JMP 0x005bf934
005bf8cc  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
005bf8d2  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005bf8d5  8945ec                MOV dword ptr [EBP + -0x14],EAX
005bf8d8  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
005bf8db  e8b0021800            CALL 0x0073fb90
005bf8e0  8945e8                MOV dword ptr [EBP + -0x18],EAX
005bf8e3  33c9                  XOR ECX,ECX
005bf8e5  69c988000000          IMUL ECX,ECX,0x88
005bf8eb  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005bf8ee  8b440a24              MOV EAX,dword ptr [EDX + ECX*0x1 + 0x24]
005bf8f2  8945fc                MOV dword ptr [EBP + -0x4],EAX
005bf8f5  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
005bf8fb  8b5108                MOV EDX,dword ptr [ECX + 0x8]
005bf8fe  8955e4                MOV dword ptr [EBP + -0x1c],EDX
005bf901  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005bf904  8b4804                MOV ECX,dword ptr [EAX + 0x4]
005bf907  894de0                MOV dword ptr [EBP + -0x20],ECX
005bf90a  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005bf90d  52                    PUSH EDX
005bf90e  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
005bf911  e89acff5ff            CALL 0x0051c8b0
005bf916  3b45e0                CMP EAX,dword ptr [EBP + -0x20]
005bf919  7507                  JNZ 0x005bf922
005bf91b  a1bc449900            MOV EAX,[0x009944bc]
005bf920  eb12                  JMP 0x005bf934
005bf922  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005bf925  c780b011000003000000  MOV dword ptr [EAX + 0x11b0],0x3
005bf92f  a1b4449900            MOV EAX,[0x009944b4]
005bf934  8be5                  MOV ESP,EBP
005bf936  5d                    POP EBP
005bf937  c20400                RET 0x4
