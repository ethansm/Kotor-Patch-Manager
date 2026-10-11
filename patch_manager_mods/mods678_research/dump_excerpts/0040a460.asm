; FN 0040a460 FUN_0040a460
0040a460  55                    PUSH EBP
0040a461  8bec                  MOV EBP,ESP
0040a463  6aff                  PUSH -0x1
0040a465  68e0809400            PUSH 0x9480e0
0040a46a  64a100000000          MOV EAX,FS:[0x0]
0040a470  50                    PUSH EAX
0040a471  83ec54                SUB ESP,0x54
0040a474  a1201fa100            MOV EAX,[0x00a11f20]
0040a479  33c5                  XOR EAX,EBP
0040a47b  50                    PUSH EAX
0040a47c  8d45f4                LEA EAX,[EBP + -0xc]
0040a47f  64a300000000          MOV FS:[0x0],EAX
0040a485  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0040a48a  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0040a48d  894db8                MOV dword ptr [EBP + -0x48],ECX
0040a490  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
0040a493  e8384e3300            CALL 0x0073f2d0
0040a498  85c0                  TEST EAX,EAX
0040a49a  0f8497000000          JZ 0x0040a537
0040a4a0  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0040a4a6  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0040a4a9  8945b4                MOV dword ptr [EBP + -0x4c],EAX
0040a4ac  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
0040a4af  e8fc4d3300            CALL 0x0073f2b0
0040a4b4  85c0                  TEST EAX,EAX
0040a4b6  747f                  JZ 0x0040a537
0040a4b8  8d4de8                LEA ECX,[EBP + -0x18]
0040a4bb  e880903200            CALL 0x00733540
0040a4c0  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0040a4c7  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0040a4cd  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0040a4d0  8955b0                MOV dword ptr [EBP + -0x50],EDX
0040a4d3  0fb74508              MOVZX EAX,word ptr [EBP + 0x8]
0040a4d7  50                    PUSH EAX
0040a4d8  6a43                  PUSH 0x43
0040a4da  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
0040a4dd  e8ce4e3300            CALL 0x0073f3b0
0040a4e2  8bc8                  MOV ECX,EAX
0040a4e4  e837041100            CALL 0x0051a920
0040a4e9  6a00                  PUSH 0x0
0040a4eb  68e8030000            PUSH 0x3e8
0040a4f0  52                    PUSH EDX
0040a4f1  50                    PUSH EAX
0040a4f2  e8292d5100            CALL 0x0091d220
0040a4f7  50                    PUSH EAX
0040a4f8  68c86c9800            PUSH 0x986cc8
0040a4fd  8d4de8                LEA ECX,[EBP + -0x18]
0040a500  51                    PUSH ECX
0040a501  e86a9d3200            CALL 0x00734270
0040a506  83c414                ADD ESP,0x14
0040a509  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0040a50f  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0040a512  8945ac                MOV dword ptr [EBP + -0x54],EAX
0040a515  8d4de8                LEA ECX,[EBP + -0x18]
0040a518  51                    PUSH ECX
0040a519  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
0040a51c  e88f4d3300            CALL 0x0073f2b0
0040a521  8bc8                  MOV ECX,EAX
0040a523  e878f53200            CALL 0x00739aa0
0040a528  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0040a52f  8d4de8                LEA ECX,[EBP + -0x18]
0040a532  e849923200            CALL 0x00733780
0040a537  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0040a53d  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0040a540  8945a8                MOV dword ptr [EBP + -0x58],EAX
0040a543  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
0040a546  e8054e3300            CALL 0x0073f350
0040a54b  8945a0                MOV dword ptr [EBP + -0x60],EAX
0040a54e  837da002              CMP dword ptr [EBP + -0x60],0x2
0040a552  7413                  JZ 0x0040a567
0040a554  837da005              CMP dword ptr [EBP + -0x60],0x5
0040a558  7402                  JZ 0x0040a55c
0040a55a  eb16                  JMP 0x0040a572
0040a55c  b903000000            MOV ECX,0x3
0040a561  66894df0              MOV word ptr [EBP + -0x10],CX
0040a565  eb14                  JMP 0x0040a57b
0040a567  bab0000000            MOV EDX,0xb0
0040a56c  668955f0              MOV word ptr [EBP + -0x10],DX
0040a570  eb09                  JMP 0x0040a57b
0040a572  b860000000            MOV EAX,0x60
0040a577  668945f0              MOV word ptr [EBP + -0x10],AX
0040a57b  0fb74d08              MOVZX ECX,word ptr [EBP + 0x8]
0040a57f  0fb755f0              MOVZX EDX,word ptr [EBP + -0x10]
0040a583  3bca                  CMP ECX,EDX
0040a585  0f855d010000          JNZ 0x0040a6e8
0040a58b  0fb745f0              MOVZX EAX,word ptr [EBP + -0x10]
0040a58f  83f803                CMP EAX,0x3
0040a592  750a                  JNZ 0x0040a59e
0040a594  c705bcfca70000000000  MOV dword ptr [0x00a7fcbc],0x0
0040a59e  8d4dd8                LEA ECX,[EBP + -0x28]
0040a5a1  e89abc3200            CALL 0x00736240
0040a5a6  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
0040a5ad  8d4de0                LEA ECX,[EBP + -0x20]
0040a5b0  e88b8f3200            CALL 0x00733540
0040a5b5  c645fc02              MOV byte ptr [EBP + -0x4],0x2
0040a5b9  68e46c9800            PUSH 0x986ce4
0040a5be  8d4dd0                LEA ECX,[EBP + -0x30]
0040a5c1  e8aa8f3200            CALL 0x00733570
0040a5c6  c645fc03              MOV byte ptr [EBP + -0x4],0x3
0040a5ca  68d46c9800            PUSH 0x986cd4
0040a5cf  8d4dc8                LEA ECX,[EBP + -0x38]
0040a5d2  e8998f3200            CALL 0x00733570
0040a5d7  c645fc04              MOV byte ptr [EBP + -0x4],0x4
0040a5db  68986b9800            PUSH 0x986b98
0040a5e0  8d4dc0                LEA ECX,[EBP + -0x40]
0040a5e3  e8888f3200            CALL 0x00733570
0040a5e8  c645fc05              MOV byte ptr [EBP + -0x4],0x5
0040a5ec  8d4dd0                LEA ECX,[EBP + -0x30]
0040a5ef  51                    PUSH ECX
0040a5f0  8d55c8                LEA EDX,[EBP + -0x38]
0040a5f3  52                    PUSH EDX
0040a5f4  8d45c0                LEA EAX,[EBP + -0x40]
0040a5f7  50                    PUSH EAX
0040a5f8  8d4de0                LEA ECX,[EBP + -0x20]
0040a5fb  51                    PUSH ECX
0040a5fc  8d4dd8                LEA ECX,[EBP + -0x28]
0040a5ff  e81cbd3200            CALL 0x00736320
0040a604  c645fc04              MOV byte ptr [EBP + -0x4],0x4
0040a608  8d4dc0                LEA ECX,[EBP + -0x40]
0040a60b  e870913200            CALL 0x00733780
0040a610  c645fc03              MOV byte ptr [EBP + -0x4],0x3
0040a614  8d4dc8                LEA ECX,[EBP + -0x38]
0040a617  e864913200            CALL 0x00733780
0040a61c  c645fc02              MOV byte ptr [EBP + -0x4],0x2
0040a620  8d4dd0                LEA ECX,[EBP + -0x30]
0040a623  e858913200            CALL 0x00733780
0040a628  837de000              CMP dword ptr [EBP + -0x20],0x0
0040a62c  7413                  JZ 0x0040a641
0040a62e  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0040a631  52                    PUSH EDX
0040a632  e8e1285100            CALL 0x0091cf18
0040a637  83c404                ADD ESP,0x4
0040a63a  8945dc                MOV dword ptr [EBP + -0x24],EAX
0040a63d  eb09                  JMP 0x0040a648
0040a641  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
0040a648  837ddc01              CMP dword ptr [EBP + -0x24],0x1
0040a64c  7573                  JNZ 0x0040a6c1
0040a64e  833de8b4a10000        CMP dword ptr [0x00a1b4e8],0x0
0040a655  7420                  JZ 0x0040a677
0040a657  833d60b7a10000        CMP dword ptr [0x00a1b760],0x0
0040a65e  740b                  JZ 0x0040a66b
0040a660  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0040a666  e875543200            CALL 0x0072fae0
0040a66b  c705e8b4a10000000000  MOV dword ptr [0x00a1b4e8],0x0
0040a675  eb4a                  JMP 0x0040a6c1
0040a677  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0040a67d  e87e543200            CALL 0x0072fb00
0040a682  f7d8                  NEG EAX
0040a684  1bc0                  SBB EAX,EAX
0040a686  83c001                ADD EAX,0x1
0040a689  a360b7a100            MOV [0x00a1b760],EAX
0040a68e  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0040a694  e827543200            CALL 0x0072fac0
0040a699  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0040a69f  e84c533200            CALL 0x0072f9f0
0040a6a4  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0040a6a9  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0040a6ac  894da4                MOV dword ptr [EBP + -0x5c],ECX
0040a6af  8b4da4                MOV ECX,dword ptr [EBP + -0x5c]
0040a6b2  e8a9573300            CALL 0x0073fe60
0040a6b7  c705e8b4a10001000000  MOV dword ptr [0x00a1b4e8],0x1
0040a6c1  c745bc01000000        MOV dword ptr [EBP + -0x44],0x1
0040a6c8  c645fc01              MOV byte ptr [EBP + -0x4],0x1
0040a6cc  8d4de0                LEA ECX,[EBP + -0x20]
0040a6cf  e8ac903200            CALL 0x00733780
0040a6d4  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0040a6db  8d4dd8                LEA ECX,[EBP + -0x28]
0040a6de  e8ddbb3200            CALL 0x007362c0
0040a6e3  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0040a6e6  eb52                  JMP 0x0040a73a
0040a6e8  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
0040a6ee  e80d543200            CALL 0x0072fb00
0040a6f3  85c0                  TEST EAX,EAX
0040a6f5  7441                  JZ 0x0040a738
0040a6f7  833de8b4a10000        CMP dword ptr [0x00a1b4e8],0x0
0040a6fe  7414                  JZ 0x0040a714
0040a700  0fb65508              MOVZX EDX,byte ptr [EBP + 0x8]
0040a704  52                    PUSH EDX
0040a705  e8d6e7ffff            CALL 0x00408ee0
0040a70a  83c404                ADD ESP,0x4
0040a70d  b801000000            MOV EAX,0x1
0040a712  eb26                  JMP 0x0040a73a
0040a714  833d9cb4a10000        CMP dword ptr [0x00a1b49c],0x0
0040a71b  741b                  JZ 0x0040a738
0040a71d  0fb74508              MOVZX EAX,word ptr [EBP + 0x8]
0040a721  50                    PUSH EAX
0040a722  8b0d9cb4a100          MOV ECX,dword ptr [0x00a1b49c]
0040a728  e873940000            CALL 0x00413ba0
0040a72d  85c0                  TEST EAX,EAX
0040a72f  7407                  JZ 0x0040a738
0040a731  b801000000            MOV EAX,0x1
0040a736  eb02                  JMP 0x0040a73a
0040a738  33c0                  XOR EAX,EAX
0040a73a  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0040a73d  64890d00000000        MOV dword ptr FS:[0x0],ECX
0040a744  59                    POP ECX
0040a745  8be5                  MOV ESP,EBP
0040a747  5d                    POP EBP
0040a748  c3                    RET
