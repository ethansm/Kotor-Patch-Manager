; FN 0078c350 FUN_0078c350
0078c350  55                    PUSH EBP
0078c351  8bec                  MOV EBP,ESP
0078c353  6aff                  PUSH -0x1
0078c355  685b659600            PUSH 0x96655b
0078c35a  64a100000000          MOV EAX,FS:[0x0]
0078c360  50                    PUSH EAX
0078c361  81ec7c010000          SUB ESP,0x17c
0078c367  a1201fa100            MOV EAX,[0x00a11f20]
0078c36c  33c5                  XOR EAX,EBP
0078c36e  50                    PUSH EAX
0078c36f  8d45f4                LEA EAX,[EBP + -0xc]
0078c372  64a300000000          MOV FS:[0x0],EAX
0078c378  898dd0feffff          MOV dword ptr [EBP + 0xfffffed0],ECX
0078c37e  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078c384  83781800              CMP dword ptr [EAX + 0x18],0x0
0078c388  7505                  JNZ 0x0078c38f
0078c38a  e9f30a0000            JMP 0x0078ce82
0078c38f  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0078c395  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0078c398  89954cffffff          MOV dword ptr [EBP + 0xffffff4c],EDX
0078c39e  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
0078c3a4  e82730fbff            CALL 0x0073f3d0
0078c3a9  8bc8                  MOV ECX,EAX
0078c3ab  e890e5d8ff            CALL 0x0051a940
0078c3b0  8985c8feffff          MOV dword ptr [EBP + 0xfffffec8],EAX
0078c3b6  8995ccfeffff          MOV dword ptr [EBP + 0xfffffecc],EDX
0078c3bc  dfadc8feffff          FILD qword ptr [EBP + 0xfffffec8]
0078c3c2  dc3518659900          FDIV double ptr [0x00996518]
0078c3c8  d95dd4                FSTP float ptr [EBP + -0x2c]
0078c3cb  a02808a300            MOV AL,[0x00a30828]
0078c3d0  8845eb                MOV byte ptr [EBP + -0x15],AL
0078c3d3  c6052808a30000        MOV byte ptr [0x00a30828],0x0
0078c3da  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078c3e0  8b5118                MOV EDX,dword ptr [ECX + 0x18]
0078c3e3  8b4240                MOV EAX,dword ptr [EDX + 0x40]
0078c3e6  894590                MOV dword ptr [EBP + -0x70],EAX
0078c3e9  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078c3ef  8b5118                MOV EDX,dword ptr [ECX + 0x18]
0078c3f2  8b4248                MOV EAX,dword ptr [EDX + 0x48]
0078c3f5  898548ffffff          MOV dword ptr [EBP + 0xffffff48],EAX
0078c3fb  8b8d48ffffff          MOV ECX,dword ptr [EBP + 0xffffff48]
0078c401  8b9188010000          MOV EDX,dword ptr [ECX + 0x188]
0078c407  895580                MOV dword ptr [EBP + -0x80],EDX
0078c40a  d9ee                  FLDZ
0078c40c  d95dcc                FSTP float ptr [EBP + -0x34]
0078c40f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0078c414  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0078c417  898d44ffffff          MOV dword ptr [EBP + 0xffffff44],ECX
0078c41d  8b8d44ffffff          MOV ECX,dword ptr [EBP + 0xffffff44]
0078c423  e8d836faff            CALL 0x0072fb00
0078c428  8bc8                  MOV ECX,EAX
0078c42a  e8c1fb0200            CALL 0x007bbff0
0078c42f  d99d7cffffff          FSTP float ptr [EBP + 0xffffff7c]
0078c435  d9ee                  FLDZ
0078c437  d95dd0                FSTP float ptr [EBP + -0x30]
0078c43a  d9ee                  FLDZ
0078c43c  d95db0                FSTP float ptr [EBP + -0x50]
0078c43f  8d55b0                LEA EDX,[EBP + -0x50]
0078c442  52                    PUSH EDX
0078c443  8d45d0                LEA EAX,[EBP + -0x30]
0078c446  50                    PUSH EAX
0078c447  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0078c44d  e81e39faff            CALL 0x0072fd70
0078c452  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0078c458  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0078c45b  899540ffffff          MOV dword ptr [EBP + 0xffffff40],EDX
0078c461  8b8d40ffffff          MOV ECX,dword ptr [EBP + 0xffffff40]
0078c467  e89436faff            CALL 0x0072fb00
0078c46c  89853cffffff          MOV dword ptr [EBP + 0xffffff3c],EAX
0078c472  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
0078c478  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0078c47b  c1e904                SHR ECX,0x4
0078c47e  83e101                AND ECX,0x1
0078c481  740c                  JZ 0x0078c48f
0078c483  d945b0                FLD float ptr [EBP + -0x50]
0078c486  dc0d986d9800          FMUL double ptr [0x00986d98]
0078c48c  d95db0                FSTP float ptr [EBP + -0x50]
0078c48f  6a00                  PUSH 0x0
0078c491  681c010000            PUSH 0x11c
0078c496  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0078c49c  e82f35faff            CALL 0x0072f9d0
0078c4a1  dc1d986d9800          FCOMP double ptr [0x00986d98]
0078c4a7  dfe0                  FNSTSW AX
0078c4a9  f6c405                TEST AH,0x5
0078c4ac  7a0e                  JP 0x0078c4bc
0078c4ae  d90574749800          FLD float ptr [0x00987474]
0078c4b4  d99dc4feffff          FSTP float ptr [EBP + 0xfffffec4]
0078c4ba  eb18                  JMP 0x0078c4d4
0078c4bc  6a00                  PUSH 0x0
0078c4be  681c010000            PUSH 0x11c
0078c4c3  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0078c4c9  e80235faff            CALL 0x0072f9d0
0078c4ce  d99dc4feffff          FSTP float ptr [EBP + 0xfffffec4]
0078c4d4  d985c4feffff          FLD float ptr [EBP + 0xfffffec4]
0078c4da  dc1d506b9800          FCOMP double ptr [0x00986b50]
0078c4e0  dfe0                  FNSTSW AX
0078c4e2  f6c441                TEST AH,0x41
0078c4e5  750a                  JNZ 0x0078c4f1
0078c4e7  d9e8                  FLD1
0078c4e9  d99dc0feffff          FSTP float ptr [EBP + 0xfffffec0]
0078c4ef  eb51                  JMP 0x0078c542
0078c4f1  6a00                  PUSH 0x0
0078c4f3  681c010000            PUSH 0x11c
0078c4f8  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0078c4fe  e8cd34faff            CALL 0x0072f9d0
0078c503  dc1d986d9800          FCOMP double ptr [0x00986d98]
0078c509  dfe0                  FNSTSW AX
0078c50b  f6c405                TEST AH,0x5
0078c50e  7a0e                  JP 0x0078c51e
0078c510  d90574749800          FLD float ptr [0x00987474]
0078c516  d99dbcfeffff          FSTP float ptr [EBP + 0xfffffebc]
0078c51c  eb18                  JMP 0x0078c536
0078c51e  6a00                  PUSH 0x0
0078c520  681c010000            PUSH 0x11c
0078c525  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0078c52b  e8a034faff            CALL 0x0072f9d0
0078c530  d99dbcfeffff          FSTP float ptr [EBP + 0xfffffebc]
0078c536  d985bcfeffff          FLD float ptr [EBP + 0xfffffebc]
0078c53c  d99dc0feffff          FSTP float ptr [EBP + 0xfffffec0]
0078c542  d985c0feffff          FLD float ptr [EBP + 0xfffffec0]
0078c548  d95d84                FSTP float ptr [EBP + -0x7c]
0078c54b  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0078c551  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0078c554  898538ffffff          MOV dword ptr [EBP + 0xffffff38],EAX
0078c55a  8b8d38ffffff          MOV ECX,dword ptr [EBP + 0xffffff38]
0078c560  e83b39fbff            CALL 0x0073fea0
0078c565  898534ffffff          MOV dword ptr [EBP + 0xffffff34],EAX
0078c56b  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
0078c571  0fbf516c              MOVSX EDX,word ptr [ECX + 0x6c]
0078c575  8955a8                MOV dword ptr [EBP + -0x58],EDX
0078c578  db45a8                FILD dword ptr [EBP + -0x58]
0078c57b  d80d1009a100          FMUL float ptr [0x00a10910]
0078c581  e8da021900            CALL 0x0091c860
0078c586  83f802                CMP EAX,0x2
0078c589  7d0c                  JGE 0x0078c597
0078c58b  c785b8feffff02000000  MOV dword ptr [EBP + 0xfffffeb8],0x2
0078c595  eb14                  JMP 0x0078c5ab
0078c597  db45a8                FILD dword ptr [EBP + -0x58]
0078c59a  d80d1009a100          FMUL float ptr [0x00a10910]
0078c5a0  e8bb021900            CALL 0x0091c860
0078c5a5  8985b8feffff          MOV dword ptr [EBP + 0xfffffeb8],EAX
0078c5ab  8b85b8feffff          MOV EAX,dword ptr [EBP + 0xfffffeb8]
0078c5b1  8945ec                MOV dword ptr [EBP + -0x14],EAX
0078c5b4  8b0d9cb4a100          MOV ECX,dword ptr [0x00a1b49c]
0078c5ba  8b11                  MOV EDX,dword ptr [ECX]
0078c5bc  899530ffffff          MOV dword ptr [EBP + 0xffffff30],EDX
0078c5c2  8b8530ffffff          MOV EAX,dword ptr [EBP + 0xffffff30]
0078c5c8  33c9                  XOR ECX,ECX
0078c5ca  3b45ec                CMP EAX,dword ptr [EBP + -0x14]
0078c5cd  0f9ec1                SETLE CL
0078c5d0  894dd8                MOV dword ptr [EBP + -0x28],ECX
0078c5d3  8b159cb4a100          MOV EDX,dword ptr [0x00a1b49c]
0078c5d9  8b02                  MOV EAX,dword ptr [EDX]
0078c5db  89852cffffff          MOV dword ptr [EBP + 0xffffff2c],EAX
0078c5e1  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
0078c5e4  2b4dec                SUB ECX,dword ptr [EBP + -0x14]
0078c5e7  33d2                  XOR EDX,EDX
0078c5e9  398d2cffffff          CMP dword ptr [EBP + 0xffffff2c],ECX
0078c5ef  0f9dc2                SETGE DL
0078c5f2  8955b4                MOV dword ptr [EBP + -0x4c],EDX
0078c5f5  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0078c5fa  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0078c5fd  898d28ffffff          MOV dword ptr [EBP + 0xffffff28],ECX
0078c603  8b8d28ffffff          MOV ECX,dword ptr [EBP + 0xffffff28]
0078c609  e89238fbff            CALL 0x0073fea0
0078c60e  898524ffffff          MOV dword ptr [EBP + 0xffffff24],EAX
0078c614  8b9524ffffff          MOV EDX,dword ptr [EBP + 0xffffff24]
0078c61a  0fbf426e              MOVSX EAX,word ptr [EDX + 0x6e]
0078c61e  894588                MOV dword ptr [EBP + -0x78],EAX
0078c621  db4588                FILD dword ptr [EBP + -0x78]
0078c624  d80d1009a100          FMUL float ptr [0x00a10910]
0078c62a  e831021900            CALL 0x0091c860
0078c62f  83f802                CMP EAX,0x2
0078c632  7d0c                  JGE 0x0078c640
0078c634  c785b4feffff02000000  MOV dword ptr [EBP + 0xfffffeb4],0x2
0078c63e  eb14                  JMP 0x0078c654
0078c640  db4588                FILD dword ptr [EBP + -0x78]
0078c643  d80d1009a100          FMUL float ptr [0x00a10910]
0078c649  e812021900            CALL 0x0091c860
0078c64e  8985b4feffff          MOV dword ptr [EBP + 0xfffffeb4],EAX
0078c654  8b8db4feffff          MOV ECX,dword ptr [EBP + 0xfffffeb4]
0078c65a  894dec                MOV dword ptr [EBP + -0x14],ECX
0078c65d  8b159cb4a100          MOV EDX,dword ptr [0x00a1b49c]
0078c663  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0078c666  898520ffffff          MOV dword ptr [EBP + 0xffffff20],EAX
0078c66c  8b8d20ffffff          MOV ECX,dword ptr [EBP + 0xffffff20]
0078c672  33d2                  XOR EDX,EDX
0078c674  3b4dec                CMP ECX,dword ptr [EBP + -0x14]
0078c677  0f9ec2                SETLE DL
0078c67a  8955e4                MOV dword ptr [EBP + -0x1c],EDX
0078c67d  a19cb4a100            MOV EAX,[0x00a1b49c]
0078c682  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0078c685  898d1cffffff          MOV dword ptr [EBP + 0xffffff1c],ECX
0078c68b  8b5588                MOV EDX,dword ptr [EBP + -0x78]
0078c68e  2b55ec                SUB EDX,dword ptr [EBP + -0x14]
0078c691  33c0                  XOR EAX,EAX
0078c693  39951cffffff          CMP dword ptr [EBP + 0xffffff1c],EDX
0078c699  0f9dc0                SETGE AL
0078c69c  894594                MOV dword ptr [EBP + -0x6c],EAX
0078c69f  e89c21faff            CALL 0x0072e840
0078c6a4  0fb6c8                MOVZX ECX,AL
0078c6a7  85c9                  TEST ECX,ECX
0078c6a9  740c                  JZ 0x0078c6b7
0078c6ab  e8a021faff            CALL 0x0072e850
0078c6b0  0fb6d0                MOVZX EDX,AL
0078c6b3  85d2                  TEST EDX,EDX
0078c6b5  7519                  JNZ 0x0078c6d0
0078c6b7  c745b400000000        MOV dword ptr [EBP + -0x4c],0x0
0078c6be  8b45b4                MOV EAX,dword ptr [EBP + -0x4c]
0078c6c1  8945d8                MOV dword ptr [EBP + -0x28],EAX
0078c6c4  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0078c6c7  894d94                MOV dword ptr [EBP + -0x6c],ECX
0078c6ca  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
0078c6cd  8955e4                MOV dword ptr [EBP + -0x1c],EDX
0078c6d0  e80b28fbff            CALL 0x0073eee0
0078c6d5  0fb6c0                MOVZX EAX,AL
0078c6d8  85c0                  TEST EAX,EAX
0078c6da  740d                  JZ 0x0078c6e9
0078c6dc  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
0078c6e3  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
0078c6e6  894de4                MOV dword ptr [EBP + -0x1c],ECX
0078c6e9  8b15bcfca700          MOV EDX,dword ptr [0x00a7fcbc]
0078c6ef  8955dc                MOV dword ptr [EBP + -0x24],EDX
0078c6f2  d9ee                  FLDZ
0078c6f4  d95d8c                FSTP float ptr [EBP + -0x74]
0078c6f7  d9ee                  FLDZ
0078c6f9  d95dac                FSTP float ptr [EBP + -0x54]
0078c6fc  8d45ac                LEA EAX,[EBP + -0x54]
0078c6ff  50                    PUSH EAX
0078c700  8d4d8c                LEA ECX,[EBP + -0x74]
0078c703  51                    PUSH ECX
0078c704  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0078c70a  e8d136faff            CALL 0x0072fde0
0078c70f  d9ee                  FLDZ
0078c711  d95df0                FSTP float ptr [EBP + -0x10]
0078c714  d9458c                FLD float ptr [EBP + -0x74]
0078c717  dc1d506b9800          FCOMP double ptr [0x00986b50]
0078c71d  dfe0                  FNSTSW AX
0078c71f  f6c405                TEST AH,0x5
0078c722  7a0b                  JP 0x0078c72f
0078c724  d9458c                FLD float ptr [EBP + -0x74]
0078c727  d99db0feffff          FSTP float ptr [EBP + 0xfffffeb0]
0078c72d  eb08                  JMP 0x0078c737
0078c72f  d9e8                  FLD1
0078c731  d99db0feffff          FSTP float ptr [EBP + 0xfffffeb0]
0078c737  d985b0feffff          FLD float ptr [EBP + 0xfffffeb0]
0078c73d  d95df0                FSTP float ptr [EBP + -0x10]
0078c740  d9458c                FLD float ptr [EBP + -0x74]
0078c743  dc1d986d9800          FCOMP double ptr [0x00986d98]
0078c749  dfe0                  FNSTSW AX
0078c74b  f6c441                TEST AH,0x41
0078c74e  750b                  JNZ 0x0078c75b
0078c750  d9458c                FLD float ptr [EBP + -0x74]
0078c753  d99dacfeffff          FSTP float ptr [EBP + 0xfffffeac]
0078c759  eb0c                  JMP 0x0078c767
0078c75b  d90574749800          FLD float ptr [0x00987474]
0078c761  d99dacfeffff          FSTP float ptr [EBP + 0xfffffeac]
0078c767  d985acfeffff          FLD float ptr [EBP + 0xfffffeac]
0078c76d  d95df0                FSTP float ptr [EBP + -0x10]
0078c770  d945f0                FLD float ptr [EBP + -0x10]
0078c773  dc1d386b9800          FCOMP double ptr [0x00986b38]
0078c779  dfe0                  FNSTSW AX
0078c77b  f6c444                TEST AH,0x44
0078c77e  7b06                  JNP 0x0078c786
0078c780  d945f0                FLD float ptr [EBP + -0x10]
0078c783  d95d84                FSTP float ptr [EBP + -0x7c]
0078c786  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0078c78c  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0078c78f  898518ffffff          MOV dword ptr [EBP + 0xffffff18],EAX
0078c795  8b8d18ffffff          MOV ECX,dword ptr [EBP + 0xffffff18]
0078c79b  e86033faff            CALL 0x0072fb00
0078c7a0  898514ffffff          MOV dword ptr [EBP + 0xffffff14],EAX
0078c7a6  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
0078c7ac  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0078c7af  c1ea04                SHR EDX,0x4
0078c7b2  83e201                AND EDX,0x1
0078c7b5  f7da                  NEG EDX
0078c7b7  1bd2                  SBB EDX,EDX
0078c7b9  83e202                AND EDX,0x2
0078c7bc  83c2ff                ADD EDX,-0x1
0078c7bf  8995a8feffff          MOV dword ptr [EBP + 0xfffffea8],EDX
0078c7c5  db85a8feffff          FILD dword ptr [EBP + 0xfffffea8]
0078c7cb  d84dac                FMUL float ptr [EBP + -0x54]
0078c7ce  d99da4feffff          FSTP float ptr [EBP + 0xfffffea4]
0078c7d4  d985a4feffff          FLD float ptr [EBP + 0xfffffea4]
0078c7da  51                    PUSH ECX
0078c7db  d91c24                FSTP float ptr [ESP]
0078c7de  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078c7e4  8b4818                MOV ECX,dword ptr [EAX + 0x18]
0078c7e7  e8f4750600            CALL 0x007f3de0
0078c7ec  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0078c7f2  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0078c7f5  899510ffffff          MOV dword ptr [EBP + 0xffffff10],EDX
0078c7fb  8b8d10ffffff          MOV ECX,dword ptr [EBP + 0xffffff10]
0078c801  e8fa32faff            CALL 0x0072fb00
0078c806  89850cffffff          MOV dword ptr [EBP + 0xffffff0c],EAX
0078c80c  8b850cffffff          MOV EAX,dword ptr [EBP + 0xffffff0c]
0078c812  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0078c815  d1e9                  SHR ECX,0x1
0078c817  83e101                AND ECX,0x1
0078c81a  740c                  JZ 0x0078c828
0078c81c  33d2                  XOR EDX,EDX
0078c81e  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0078c822  0f94c2                SETZ DL
0078c825  8955dc                MOV dword ptr [EBP + -0x24],EDX
0078c828  d94584                FLD float ptr [EBP + -0x7c]
0078c82b  dc1d386b9800          FCOMP double ptr [0x00986b38]
0078c831  dfe0                  FNSTSW AX
0078c833  f6c444                TEST AH,0x44
0078c836  7b4d                  JNP 0x0078c885
0078c838  51                    PUSH ECX
0078c839  d94508                FLD float ptr [EBP + 0x8]
0078c83c  d91c24                FSTP float ptr [ESP]
0078c83f  33c0                  XOR EAX,EAX
0078c841  833dbceba70000        CMP dword ptr [0x00a7ebbc],0x0
0078c848  0f95c0                SETNZ AL
0078c84b  8d4400ff              LEA EAX,[EAX + EAX*0x1 + -0x1]
0078c84f  8985a0feffff          MOV dword ptr [EBP + 0xfffffea0],EAX
0078c855  db85a0feffff          FILD dword ptr [EBP + 0xfffffea0]
0078c85b  d94584                FLD float ptr [EBP + -0x7c]
0078c85e  d9e0                  FCHS
0078c860  dec9                  FMULP
0078c862  d99d9cfeffff          FSTP float ptr [EBP + 0xfffffe9c]
0078c868  d9859cfeffff          FLD float ptr [EBP + 0xfffffe9c]
0078c86e  51                    PUSH ECX
0078c86f  d91c24                FSTP float ptr [ESP]
0078c872  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078c878  8b4918                MOV ECX,dword ptr [ECX + 0x18]
0078c87b  e8a0790600            CALL 0x007f4220
0078c880  e922020000            JMP 0x0078caa7
0078c885  837dd800              CMP dword ptr [EBP + -0x28],0x0
0078c889  7516                  JNZ 0x0078c8a1
0078c88b  837db400              CMP dword ptr [EBP + -0x4c],0x0
0078c88f  7510                  JNZ 0x0078c8a1
0078c891  837de400              CMP dword ptr [EBP + -0x1c],0x0
0078c895  750a                  JNZ 0x0078c8a1
0078c897  837d9400              CMP dword ptr [EBP + -0x6c],0x0
0078c89b  0f840d010000          JZ 0x0078c9ae
0078c8a1  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0078c8a5  0f8503010000          JNZ 0x0078c9ae
0078c8ab  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0078c8b1  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0078c8b4  898508ffffff          MOV dword ptr [EBP + 0xffffff08],EAX
0078c8ba  8b8d08ffffff          MOV ECX,dword ptr [EBP + 0xffffff08]
0078c8c0  e83b32faff            CALL 0x0072fb00
0078c8c5  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
0078c8cb  8b8d04ffffff          MOV ECX,dword ptr [EBP + 0xffffff04]
0078c8d1  8a5175                MOV DL,byte ptr [ECX + 0x75]
0078c8d4  889503ffffff          MOV byte ptr [EBP + 0xffffff03],DL
0078c8da  0fb68503ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff03]
0078c8e1  83f805                CMP EAX,0x5
0078c8e4  0f84c4000000          JZ 0x0078c9ae
0078c8ea  837dd800              CMP dword ptr [EBP + -0x28],0x0
0078c8ee  7506                  JNZ 0x0078c8f6
0078c8f0  837db400              CMP dword ptr [EBP + -0x4c],0x0
0078c8f4  7457                  JZ 0x0078c94d
0078c8f6  51                    PUSH ECX
0078c8f7  d94508                FLD float ptr [EBP + 0x8]
0078c8fa  d91c24                FSTP float ptr [ESP]
0078c8fd  33c9                  XOR ECX,ECX
0078c8ff  833dbceba70000        CMP dword ptr [0x00a7ebbc],0x0
0078c906  0f95c1                SETNZ CL
0078c909  8d4c09ff              LEA ECX,[ECX + ECX*0x1 + -0x1]
0078c90d  33d2                  XOR EDX,EDX
0078c90f  837dd800              CMP dword ptr [EBP + -0x28],0x0
0078c913  0f95c2                SETNZ DL
0078c916  8d5412ff              LEA EDX,[EDX + EDX*0x1 + -0x1]
0078c91a  0fafca                IMUL ECX,EDX
0078c91d  898d98feffff          MOV dword ptr [EBP + 0xfffffe98],ECX
0078c923  db8598feffff          FILD dword ptr [EBP + 0xfffffe98]
0078c929  dc0d506b9800          FMUL double ptr [0x00986b50]
0078c92f  d99d94feffff          FSTP float ptr [EBP + 0xfffffe94]
0078c935  d98594feffff          FLD float ptr [EBP + 0xfffffe94]
0078c93b  51                    PUSH ECX
0078c93c  d91c24                FSTP float ptr [ESP]
0078c93f  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078c945  8b4818                MOV ECX,dword ptr [EAX + 0x18]
0078c948  e8d3780600            CALL 0x007f4220
0078c94d  837de400              CMP dword ptr [EBP + -0x1c],0x0
0078c951  7506                  JNZ 0x0078c959
0078c953  837d9400              CMP dword ptr [EBP + -0x6c],0x0
0078c957  7450                  JZ 0x0078c9a9
0078c959  33c9                  XOR ECX,ECX
0078c95b  833dd806a10000        CMP dword ptr [0x00a106d8],0x0
0078c962  0f95c1                SETNZ CL
0078c965  8d4c09ff              LEA ECX,[ECX + ECX*0x1 + -0x1]
0078c969  33d2                  XOR EDX,EDX
0078c96b  837de400              CMP dword ptr [EBP + -0x1c],0x0
0078c96f  0f95c2                SETNZ DL
0078c972  8d5412ff              LEA EDX,[EDX + EDX*0x1 + -0x1]
0078c976  0fafca                IMUL ECX,EDX
0078c979  898d90feffff          MOV dword ptr [EBP + 0xfffffe90],ECX
0078c97f  db8590feffff          FILD dword ptr [EBP + 0xfffffe90]
0078c985  dc0d506b9800          FMUL double ptr [0x00986b50]
0078c98b  d99d8cfeffff          FSTP float ptr [EBP + 0xfffffe8c]
0078c991  d9858cfeffff          FLD float ptr [EBP + 0xfffffe8c]
0078c997  51                    PUSH ECX
0078c998  d91c24                FSTP float ptr [ESP]
0078c99b  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078c9a1  8b4818                MOV ECX,dword ptr [EAX + 0x18]
0078c9a4  e837740600            CALL 0x007f3de0
0078c9a9  e9f9000000            JMP 0x0078caa7
0078c9ae  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0078c9b2  753f                  JNZ 0x0078c9f3
0078c9b4  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0078c9ba  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0078c9bd  8995fcfeffff          MOV dword ptr [EBP + 0xfffffefc],EDX
0078c9c3  8b8dfcfeffff          MOV ECX,dword ptr [EBP + 0xfffffefc]
0078c9c9  e83231faff            CALL 0x0072fb00
0078c9ce  8985f8feffff          MOV dword ptr [EBP + 0xfffffef8],EAX
0078c9d4  8b85f8feffff          MOV EAX,dword ptr [EBP + 0xfffffef8]
0078c9da  8a4875                MOV CL,byte ptr [EAX + 0x75]
0078c9dd  888df7feffff          MOV byte ptr [EBP + 0xfffffef7],CL
0078c9e3  0fb695f7feffff        MOVZX EDX,byte ptr [EBP + 0xfffffef7]
0078c9ea  83fa05                CMP EDX,0x5
0078c9ed  0f8599000000          JNZ 0x0078ca8c
0078c9f3  51                    PUSH ECX
0078c9f4  d94508                FLD float ptr [EBP + 0x8]
0078c9f7  d91c24                FSTP float ptr [ESP]
0078c9fa  33c0                  XOR EAX,EAX
0078c9fc  833dbceba70000        CMP dword ptr [0x00a7ebbc],0x0
0078ca03  0f95c0                SETNZ AL
0078ca06  8d4400ff              LEA EAX,[EAX + EAX*0x1 + -0x1]
0078ca0a  898588feffff          MOV dword ptr [EBP + 0xfffffe88],EAX
0078ca10  db8588feffff          FILD dword ptr [EBP + 0xfffffe88]
0078ca16  d9857cffffff          FLD float ptr [EBP + 0xffffff7c]
0078ca1c  d84dd0                FMUL float ptr [EBP + -0x30]
0078ca1f  dec9                  FMULP
0078ca21  d99d84feffff          FSTP float ptr [EBP + 0xfffffe84]
0078ca27  d98584feffff          FLD float ptr [EBP + 0xfffffe84]
0078ca2d  51                    PUSH ECX
0078ca2e  d91c24                FSTP float ptr [ESP]
0078ca31  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078ca37  8b4918                MOV ECX,dword ptr [ECX + 0x18]
0078ca3a  e861740600            CALL 0x007f3ea0
0078ca3f  33d2                  XOR EDX,EDX
0078ca41  833dbceba70000        CMP dword ptr [0x00a7ebbc],0x0
0078ca48  0f95c2                SETNZ DL
0078ca4b  8d5412ff              LEA EDX,[EDX + EDX*0x1 + -0x1]
0078ca4f  899580feffff          MOV dword ptr [EBP + 0xfffffe80],EDX
0078ca55  db8580feffff          FILD dword ptr [EBP + 0xfffffe80]
0078ca5b  d9857cffffff          FLD float ptr [EBP + 0xffffff7c]
0078ca61  d84db0                FMUL float ptr [EBP + -0x50]
0078ca64  dc35e09e9800          FDIV double ptr [0x00989ee0]
0078ca6a  dec9                  FMULP
0078ca6c  d99d7cfeffff          FSTP float ptr [EBP + 0xfffffe7c]
0078ca72  d9857cfeffff          FLD float ptr [EBP + 0xfffffe7c]
0078ca78  51                    PUSH ECX
0078ca79  d91c24                FSTP float ptr [ESP]
0078ca7c  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078ca82  8b4818                MOV ECX,dword ptr [EAX + 0x18]
0078ca85  e8e6710600            CALL 0x007f3c70
0078ca8a  eb1b                  JMP 0x0078caa7
0078ca8c  51                    PUSH ECX
0078ca8d  d94508                FLD float ptr [EBP + 0x8]
0078ca90  d91c24                FSTP float ptr [ESP]
0078ca93  51                    PUSH ECX
0078ca94  d9ee                  FLDZ
0078ca96  d91c24                FSTP float ptr [ESP]
0078ca99  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078ca9f  8b4918                MOV ECX,dword ptr [ECX + 0x18]
0078caa2  e8f9730600            CALL 0x007f3ea0
0078caa7  8b95d0feffff          MOV EDX,dword ptr [EBP + 0xfffffed0]
0078caad  d982ac000000          FLD float ptr [EDX + 0xac]
0078cab3  dc1d386b9800          FCOMP double ptr [0x00986b38]
0078cab9  dfe0                  FNSTSW AX
0078cabb  f6c444                TEST AH,0x44
0078cabe  7b1f                  JNP 0x0078cadf
0078cac0  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078cac6  8b4818                MOV ECX,dword ptr [EAX + 0x18]
0078cac9  898df0feffff          MOV dword ptr [EBP + 0xfffffef0],ECX
0078cacf  8b95f0feffff          MOV EDX,dword ptr [EBP + 0xfffffef0]
0078cad5  c7820401000001000000  MOV dword ptr [EDX + 0x104],0x1
0078cadf  837d9000              CMP dword ptr [EBP + -0x70],0x0
0078cae3  7437                  JZ 0x0078cb1c
0078cae5  8b4590                MOV EAX,dword ptr [EBP + -0x70]
0078cae8  8b10                  MOV EDX,dword ptr [EAX]
0078caea  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078caed  8b426c                MOV EAX,dword ptr [EDX + 0x6c]
0078caf0  ffd0                  CALL EAX
0078caf2  dc0d98d89800          FMUL double ptr [0x0098d898]
0078caf8  dc3508d89800          FDIV double ptr [0x0098d808]
0078cafe  d91dc0ffa100          FSTP float ptr [0x00a1ffc0]
0078cb04  d905c0ffa100          FLD float ptr [0x00a1ffc0]
0078cb0a  d91dc4ffa100          FSTP float ptr [0x00a1ffc4]
0078cb10  d905c0ffa100          FLD float ptr [0x00a1ffc0]
0078cb16  d91dc8ffa100          FSTP float ptr [0x00a1ffc8]
0078cb1c  8d4db8                LEA ECX,[EBP + -0x48]
0078cb1f  51                    PUSH ECX
0078cb20  8b5590                MOV EDX,dword ptr [EBP + -0x70]
0078cb23  8b02                  MOV EAX,dword ptr [EDX]
0078cb25  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cb28  8b5020                MOV EDX,dword ptr [EAX + 0x20]
0078cb2b  ffd2                  CALL EDX
0078cb2d  d9ee                  FLDZ
0078cb2f  d95d98                FSTP float ptr [EBP + -0x68]
0078cb32  d9ee                  FLDZ
0078cb34  d95d9c                FSTP float ptr [EBP + -0x64]
0078cb37  d9ee                  FLDZ
0078cb39  d95da0                FSTP float ptr [EBP + -0x60]
0078cb3c  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078cb42  83b8d000000000        CMP dword ptr [EAX + 0xd0],0x0
0078cb49  7429                  JZ 0x0078cb74
0078cb4b  6a00                  PUSH 0x0
0078cb4d  6a28                  PUSH 0x28
0078cb4f  8d8d5cffffff          LEA ECX,[EBP + 0xffffff5c]
0078cb55  51                    PUSH ECX
0078cb56  8b5590                MOV EDX,dword ptr [EBP + -0x70]
0078cb59  8b02                  MOV EAX,dword ptr [EDX]
0078cb5b  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cb5e  8b502c                MOV EDX,dword ptr [EAX + 0x2c]
0078cb61  ffd2                  CALL EDX
0078cb63  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078cb69  db81d0000000          FILD dword ptr [ECX + 0xd0]
0078cb6f  d808                  FMUL float ptr [EAX]
0078cb71  d95d98                FSTP float ptr [EBP + -0x68]
0078cb74  8b95d0feffff          MOV EDX,dword ptr [EBP + 0xfffffed0]
0078cb7a  83bad400000000        CMP dword ptr [EDX + 0xd4],0x0
0078cb81  742a                  JZ 0x0078cbad
0078cb83  6a28                  PUSH 0x28
0078cb85  6a00                  PUSH 0x0
0078cb87  8d8550ffffff          LEA EAX,[EBP + 0xffffff50]
0078cb8d  50                    PUSH EAX
0078cb8e  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cb91  8b11                  MOV EDX,dword ptr [ECX]
0078cb93  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cb96  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
0078cb99  ffd0                  CALL EAX
0078cb9b  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078cba1  db81d4000000          FILD dword ptr [ECX + 0xd4]
0078cba7  d84804                FMUL float ptr [EAX + 0x4]
0078cbaa  d95d9c                FSTP float ptr [EBP + -0x64]
0078cbad  83ec0c                SUB ESP,0xc
0078cbb0  8bd4                  MOV EDX,ESP
0078cbb2  8b4598                MOV EAX,dword ptr [EBP + -0x68]
0078cbb5  8902                  MOV dword ptr [EDX],EAX
0078cbb7  8b4d9c                MOV ECX,dword ptr [EBP + -0x64]
0078cbba  894a04                MOV dword ptr [EDX + 0x4],ECX
0078cbbd  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
0078cbc0  894208                MOV dword ptr [EDX + 0x8],EAX
0078cbc3  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078cbc9  8b4918                MOV ECX,dword ptr [ECX + 0x18]
0078cbcc  e87f790600            CALL 0x007f4550
0078cbd1  8a55eb                MOV DL,byte ptr [EBP + -0x15]
0078cbd4  88152808a300          MOV byte ptr [0x00a30828],DL
0078cbda  c745c400000000        MOV dword ptr [EBP + -0x3c],0x0
0078cbe1  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078cbe7  8b4818                MOV ECX,dword ptr [EAX + 0x18]
0078cbea  8b5140                MOV EDX,dword ptr [ECX + 0x40]
0078cbed  8955e0                MOV dword ptr [EBP + -0x20],EDX
0078cbf0  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078cbf6  8b4818                MOV ECX,dword ptr [EAX + 0x18]
0078cbf9  8b5148                MOV EDX,dword ptr [ECX + 0x48]
0078cbfc  8955a4                MOV dword ptr [EBP + -0x5c],EDX
0078cbff  837da400              CMP dword ptr [EBP + -0x5c],0x0
0078cc03  740c                  JZ 0x0078cc11
0078cc05  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
0078cc08  8b8888010000          MOV ECX,dword ptr [EAX + 0x188]
0078cc0e  894dc4                MOV dword ptr [EBP + -0x3c],ECX
0078cc11  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078cc17  e8c4020000            CALL 0x0078cee0
0078cc1c  8945c8                MOV dword ptr [EBP + -0x38],EAX
0078cc1f  51                    PUSH ECX
0078cc20  d945d4                FLD float ptr [EBP + -0x2c]
0078cc23  d91c24                FSTP float ptr [ESP]
0078cc26  8b95d0feffff          MOV EDX,dword ptr [EBP + 0xfffffed0]
0078cc2c  8b4a18                MOV ECX,dword ptr [EDX + 0x18]
0078cc2f  e8acde0600            CALL 0x007faae0
0078cc34  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078cc3a  83b82003000000        CMP dword ptr [EAX + 0x320],0x0
0078cc41  7515                  JNZ 0x0078cc58
0078cc43  51                    PUSH ECX
0078cc44  d945d4                FLD float ptr [EBP + -0x2c]
0078cc47  d91c24                FSTP float ptr [ESP]
0078cc4a  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078cc50  8b4918                MOV ECX,dword ptr [ECX + 0x18]
0078cc53  e8b8790600            CALL 0x007f4610
0078cc58  8b95d0feffff          MOV EDX,dword ptr [EBP + 0xfffffed0]
0078cc5e  51                    PUSH ECX
0078cc5f  d982ac000000          FLD float ptr [EDX + 0xac]
0078cc65  d91c24                FSTP float ptr [ESP]
0078cc68  51                    PUSH ECX
0078cc69  d945d4                FLD float ptr [EBP + -0x2c]
0078cc6c  d91c24                FSTP float ptr [ESP]
0078cc6f  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078cc75  8b4818                MOV ECX,dword ptr [EAX + 0x18]
0078cc78  e8b37f0600            CALL 0x007f4c30
0078cc7d  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078cc83  d999ac000000          FSTP float ptr [ECX + 0xac]
0078cc89  8b95d0feffff          MOV EDX,dword ptr [EBP + 0xfffffed0]
0078cc8f  d982ac000000          FLD float ptr [EDX + 0xac]
0078cc95  d99decfeffff          FSTP float ptr [EBP + 0xfffffeec]
0078cc9b  d985ecfeffff          FLD float ptr [EBP + 0xfffffeec]
0078cca1  83ec08                SUB ESP,0x8
0078cca4  dd1c24                FSTP double ptr [ESP]
0078cca7  e8011f1900            CALL 0x0091ebad
0078ccac  83c408                ADD ESP,0x8
0078ccaf  d99d78feffff          FSTP float ptr [EBP + 0xfffffe78]
0078ccb5  d98578feffff          FLD float ptr [EBP + 0xfffffe78]
0078ccbb  dc1de8c19800          FCOMP double ptr [0x0098c1e8]
0078ccc1  dfe0                  FNSTSW AX
0078ccc3  f6c441                TEST AH,0x41
0078ccc6  751e                  JNZ 0x0078cce6
0078ccc8  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078ccce  51                    PUSH ECX
0078cccf  d980ac000000          FLD float ptr [EAX + 0xac]
0078ccd5  d91c24                FSTP float ptr [ESP]
0078ccd8  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078ccde  8b4918                MOV ECX,dword ptr [ECX + 0x18]
0078cce1  e8ca770600            CALL 0x007f44b0
0078cce6  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0078ccec  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0078ccef  8985e8feffff          MOV dword ptr [EBP + 0xfffffee8],EAX
0078ccf5  8b8de8feffff          MOV ECX,dword ptr [EBP + 0xfffffee8]
0078ccfb  e8d025fbff            CALL 0x0073f2d0
0078cd00  83f801                CMP EAX,0x1
0078cd03  0f8530010000          JNZ 0x0078ce39
0078cd09  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078cd0f  8b5118                MOV EDX,dword ptr [ECX + 0x18]
0078cd12  8b8280000000          MOV EAX,dword ptr [EDX + 0x80]
0078cd18  8985e4feffff          MOV dword ptr [EBP + 0xfffffee4],EAX
0078cd1e  83bde4feffff01        CMP dword ptr [EBP + 0xfffffee4],0x1
0078cd25  0f850e010000          JNZ 0x0078ce39
0078cd2b  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cd2e  8b11                  MOV EDX,dword ptr [ECX]
0078cd30  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cd33  8b426c                MOV EAX,dword ptr [EDX + 0x6c]
0078cd36  ffd0                  CALL EAX
0078cd38  d99d70ffffff          FSTP float ptr [EBP + 0xffffff70]
0078cd3e  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cd41  8b11                  MOV EDX,dword ptr [ECX]
0078cd43  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cd46  8b4268                MOV EAX,dword ptr [EDX + 0x68]
0078cd49  ffd0                  CALL EAX
0078cd4b  d99d68ffffff          FSTP float ptr [EBP + 0xffffff68]
0078cd51  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cd54  8b11                  MOV EDX,dword ptr [ECX]
0078cd56  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
0078cd59  8b4264                MOV EAX,dword ptr [EDX + 0x64]
0078cd5c  ffd0                  CALL EAX
0078cd5e  d99d6cffffff          FSTP float ptr [EBP + 0xffffff6c]
0078cd64  8d8d74ffffff          LEA ECX,[EBP + 0xffffff74]
0078cd6a  e8d167faff            CALL 0x00733540
0078cd6f  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0078cd76  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0078cd7c  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0078cd7f  8995e0feffff          MOV dword ptr [EBP + 0xfffffee0],EDX
0078cd85  d9856cffffff          FLD float ptr [EBP + 0xffffff6c]
0078cd8b  83ec08                SUB ESP,0x8
0078cd8e  dd1c24                FSTP double ptr [ESP]
0078cd91  d98568ffffff          FLD float ptr [EBP + 0xffffff68]
0078cd97  83ec08                SUB ESP,0x8
0078cd9a  dd1c24                FSTP double ptr [ESP]
0078cd9d  d98570ffffff          FLD float ptr [EBP + 0xffffff70]
0078cda3  83ec08                SUB ESP,0x8
0078cda6  dd1c24                FSTP double ptr [ESP]
0078cda9  6a56                  PUSH 0x56
0078cdab  8b8de0feffff          MOV ECX,dword ptr [EBP + 0xfffffee0]
0078cdb1  e8fa25fbff            CALL 0x0073f3b0
0078cdb6  8bc8                  MOV ECX,EAX
0078cdb8  e863dbd8ff            CALL 0x0051a920
0078cdbd  6a00                  PUSH 0x0
0078cdbf  68e8030000            PUSH 0x3e8
0078cdc4  52                    PUSH EDX
0078cdc5  50                    PUSH EAX
0078cdc6  e855041900            CALL 0x0091d220
0078cdcb  50                    PUSH EAX
0078cdcc  6834f79900            PUSH 0x99f734
0078cdd1  8d8574ffffff          LEA EAX,[EBP + 0xffffff74]
0078cdd7  50                    PUSH EAX
0078cdd8  e89374faff            CALL 0x00734270
0078cddd  83c428                ADD ESP,0x28
0078cde0  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0078cde6  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0078cde9  8995dcfeffff          MOV dword ptr [EBP + 0xfffffedc],EDX
0078cdef  8d8574ffffff          LEA EAX,[EBP + 0xffffff74]
0078cdf5  50                    PUSH EAX
0078cdf6  8b8ddcfeffff          MOV ECX,dword ptr [EBP + 0xfffffedc]
0078cdfc  e8af24fbff            CALL 0x0073f2b0
0078ce01  8bc8                  MOV ECX,EAX
0078ce03  e898ccfaff            CALL 0x00739aa0
0078ce08  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078ce0e  8b5118                MOV EDX,dword ptr [ECX + 0x18]
0078ce11  8995d8feffff          MOV dword ptr [EBP + 0xfffffed8],EDX
0078ce17  8b85d8feffff          MOV EAX,dword ptr [EBP + 0xfffffed8]
0078ce1d  c7808000000000000000  MOV dword ptr [EAX + 0x80],0x0
0078ce27  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0078ce2e  8d8d74ffffff          LEA ECX,[EBP + 0xffffff74]
0078ce34  e84769faff            CALL 0x00733780
0078ce39  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078ce3f  d9ee                  FLDZ
0078ce41  d999ac000000          FSTP float ptr [ECX + 0xac]
0078ce47  8b95d0feffff          MOV EDX,dword ptr [EBP + 0xfffffed0]
0078ce4d  d9ee                  FLDZ
0078ce4f  d99ab0000000          FSTP float ptr [EDX + 0xb0]
0078ce55  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0078ce5b  d9ee                  FLDZ
0078ce5d  d998b4000000          FSTP float ptr [EAX + 0xb4]
0078ce63  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
0078ce69  8b5118                MOV EDX,dword ptr [ECX + 0x18]
0078ce6c  8995d4feffff          MOV dword ptr [EBP + 0xfffffed4],EDX
0078ce72  8b85d4feffff          MOV EAX,dword ptr [EBP + 0xfffffed4]
0078ce78  c7808000000000000000  MOV dword ptr [EAX + 0x80],0x0
0078ce82  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0078ce85  64890d00000000        MOV dword ptr FS:[0x0],ECX
0078ce8c  59                    POP ECX
0078ce8d  8be5                  MOV ESP,EBP
0078ce8f  5d                    POP EBP
0078ce90  c20400                RET 0x4
