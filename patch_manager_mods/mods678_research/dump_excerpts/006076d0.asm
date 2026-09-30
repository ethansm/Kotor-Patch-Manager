; FN 006076d0 FUN_006076d0
006076d0  55                    PUSH EBP
006076d1  8bec                  MOV EBP,ESP
006076d3  83ec0c                SUB ESP,0xc
006076d6  894df4                MOV dword ptr [EBP + -0xc],ECX
006076d9  c645ff00              MOV byte ptr [EBP + -0x1],0x0
006076dd  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
006076e0  83b8f002000000        CMP dword ptr [EAX + 0x2f0],0x0
006076e7  0f8e8f000000          JLE 0x0060777c
006076ed  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006076f0  e83bf70c00            CALL 0x006d6e30
006076f5  8945f8                MOV dword ptr [EBP + -0x8],EAX
006076f8  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
006076fb  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
00607701  8855fe                MOV byte ptr [EBP + -0x2],DL
00607704  0fb645fe              MOVZX EAX,byte ptr [EBP + -0x2]
00607708  85c0                  TEST EAX,EAX
0060770a  7412                  JZ 0x0060771e
0060770c  0fb64dfe              MOVZX ECX,byte ptr [EBP + -0x2]
00607710  83f901                CMP ECX,0x1
00607713  7409                  JZ 0x0060771e
00607715  0fb655fe              MOVZX EDX,byte ptr [EBP + -0x2]
00607719  83fa02                CMP EDX,0x2
0060771c  7506                  JNZ 0x00607724
0060771e  c645ff03              MOV byte ptr [EBP + -0x1],0x3
00607722  eb58                  JMP 0x0060777c
00607724  0fb645fe              MOVZX EAX,byte ptr [EBP + -0x2]
00607728  83f827                CMP EAX,0x27
0060772b  7412                  JZ 0x0060773f
0060772d  0fb64dfe              MOVZX ECX,byte ptr [EBP + -0x2]
00607731  83f928                CMP ECX,0x28
00607734  7409                  JZ 0x0060773f
00607736  0fb655fe              MOVZX EDX,byte ptr [EBP + -0x2]
0060773a  83fa29                CMP EDX,0x29
0060773d  7506                  JNZ 0x00607745
0060773f  c645ff01              MOV byte ptr [EBP + -0x1],0x1
00607743  eb37                  JMP 0x0060777c
00607745  0fb645fe              MOVZX EAX,byte ptr [EBP + -0x2]
00607749  83f803                CMP EAX,0x3
0060774c  7412                  JZ 0x00607760
0060774e  0fb64dfe              MOVZX ECX,byte ptr [EBP + -0x2]
00607752  83f904                CMP ECX,0x4
00607755  7409                  JZ 0x00607760
00607757  0fb655fe              MOVZX EDX,byte ptr [EBP + -0x2]
0060775b  83fa05                CMP EDX,0x5
0060775e  7506                  JNZ 0x00607766
00607760  c645ff02              MOV byte ptr [EBP + -0x1],0x2
00607764  eb16                  JMP 0x0060777c
00607766  0fb645fe              MOVZX EAX,byte ptr [EBP + -0x2]
0060776a  83f81f                CMP EAX,0x1f
0060776d  7c0d                  JL 0x0060777c
0060776f  0fb64dfe              MOVZX ECX,byte ptr [EBP + -0x2]
00607773  83f926                CMP ECX,0x26
00607776  7f04                  JG 0x0060777c
00607778  c645ff04              MOV byte ptr [EBP + -0x1],0x4
0060777c  8a45ff                MOV AL,byte ptr [EBP + -0x1]
0060777f  8be5                  MOV ESP,EBP
00607781  5d                    POP EBP
00607782  c20400                RET 0x4
