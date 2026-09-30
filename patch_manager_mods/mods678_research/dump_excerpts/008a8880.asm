; FN 008a8880 FUN_008a8880
008a8880  55                    PUSH EBP
008a8881  8bec                  MOV EBP,ESP
008a8883  83ec30                SUB ESP,0x30
008a8886  894dd4                MOV dword ptr [EBP + -0x2c],ECX
008a8889  837d0c00              CMP dword ptr [EBP + 0xc],0x0
008a888d  0f84a7010000          JZ 0x008a8a3a
008a8893  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a8896  8945d0                MOV dword ptr [EBP + -0x30],EAX
008a8899  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
008a889c  83e928                SUB ECX,0x28
008a889f  894dd0                MOV dword ptr [EBP + -0x30],ECX
008a88a2  817dd0b7000000        CMP dword ptr [EBP + -0x30],0xb7
008a88a9  0f878b010000          JA 0x008a8a3a
008a88af  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
008a88b2  0fb6826c8a8a00        MOVZX EAX,byte ptr [EDX + 0x8a8a6c]
008a88b9  ff2485508a8a00        JMP dword ptr [EAX*0x4 + 0x8a8a50]
008a88c0  6a01                  PUSH 0x1
008a88c2  6a32                  PUSH 0x32
008a88c4  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a88c7  81c178080000          ADD ECX,0x878
008a88cd  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
008a88d0  8b8278080000          MOV EAX,dword ptr [EDX + 0x878]
008a88d6  8b503c                MOV EDX,dword ptr [EAX + 0x3c]
008a88d9  ffd2                  CALL EDX
008a88db  e95a010000            JMP 0x008a8a3a
008a88e0  6a01                  PUSH 0x1
008a88e2  6a31                  PUSH 0x31
008a88e4  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a88e7  81c178080000          ADD ECX,0x878
008a88ed  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
008a88f0  8b9078080000          MOV EDX,dword ptr [EAX + 0x878]
008a88f6  8b423c                MOV EAX,dword ptr [EDX + 0x3c]
008a88f9  ffd0                  CALL EAX
008a88fb  e93a010000            JMP 0x008a8a3a
008a8900  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
008a8906  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008a8909  8955f8                MOV dword ptr [EBP + -0x8],EDX
008a890c  6a01                  PUSH 0x1
008a890e  6a00                  PUSH 0x0
008a8910  6a00                  PUSH 0x0
008a8912  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
008a8915  e86682e9ff            CALL 0x00740b80
008a891a  6a01                  PUSH 0x1
008a891c  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
008a891f  8b481c                MOV ECX,dword ptr [EAX + 0x1c]
008a8922  e87999b6ff            CALL 0x004122a0
008a8927  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a892a  8b91d0240000          MOV EDX,dword ptr [ECX + 0x24d0]
008a8930  83ca04                OR EDX,0x4
008a8933  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
008a8936  8990d0240000          MOV dword ptr [EAX + 0x24d0],EDX
008a893c  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a893f  e8dca9d7ff            CALL 0x00623320
008a8944  e9f1000000            JMP 0x008a8a3a
008a8949  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
008a894f  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008a8952  8955f4                MOV dword ptr [EBP + -0xc],EDX
008a8955  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008a8958  e8f36de9ff            CALL 0x0073f750
008a895d  8945f0                MOV dword ptr [EBP + -0x10],EAX
008a8960  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
008a8963  8a88c10f0000          MOV CL,byte ptr [EAX + 0xfc1]
008a8969  884def                MOV byte ptr [EBP + -0x11],CL
008a896c  0fb655ef              MOVZX EDX,byte ptr [EBP + -0x11]
008a8970  8955fc                MOV dword ptr [EBP + -0x4],EDX
008a8973  837dfc06              CMP dword ptr [EBP + -0x4],0x6
008a8977  7509                  JNZ 0x008a8982
008a8979  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a8980  eb12                  JMP 0x008a8994
008a8982  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
008a8985  83e801                SUB EAX,0x1
008a8988  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a898b  7907                  JNS 0x008a8994
008a898d  c745fc0b000000        MOV dword ptr [EBP + -0x4],0xb
008a8994  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008a8997  51                    PUSH ECX
008a8998  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a899b  e890010000            CALL 0x008a8b30
008a89a0  e995000000            JMP 0x008a8a3a
008a89a5  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
008a89ab  8b4204                MOV EAX,dword ptr [EDX + 0x4]
008a89ae  8945e8                MOV dword ptr [EBP + -0x18],EAX
008a89b1  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008a89b4  e8976de9ff            CALL 0x0073f750
008a89b9  8945e4                MOV dword ptr [EBP + -0x1c],EAX
008a89bc  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008a89bf  8a91c10f0000          MOV DL,byte ptr [ECX + 0xfc1]
008a89c5  8855e3                MOV byte ptr [EBP + -0x1d],DL
008a89c8  0fb645e3              MOVZX EAX,byte ptr [EBP + -0x1d]
008a89cc  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a89cf  837dfc00              CMP dword ptr [EBP + -0x4],0x0
008a89d3  7509                  JNZ 0x008a89de
008a89d5  c745fc06000000        MOV dword ptr [EBP + -0x4],0x6
008a89dc  eb16                  JMP 0x008a89f4
008a89de  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008a89e1  83c101                ADD ECX,0x1
008a89e4  894dfc                MOV dword ptr [EBP + -0x4],ECX
008a89e7  837dfc0b              CMP dword ptr [EBP + -0x4],0xb
008a89eb  7e07                  JLE 0x008a89f4
008a89ed  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a89f4  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008a89f7  52                    PUSH EDX
008a89f8  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a89fb  e830010000            CALL 0x008a8b30
008a8a00  eb38                  JMP 0x008a8a3a
008a8a02  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008a8a07  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008a8a0a  894ddc                MOV dword ptr [EBP + -0x24],ECX
008a8a0d  6a00                  PUSH 0x0
008a8a0f  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008a8a12  e8396de9ff            CALL 0x0073f750
008a8a17  8bc8                  MOV ECX,EAX
008a8a19  e84216f2ff            CALL 0x007ca060
008a8a1e  85c0                  TEST EAX,EAX
008a8a20  7418                  JZ 0x008a8a3a
008a8a22  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
008a8a28  8b4204                MOV EAX,dword ptr [EDX + 0x4]
008a8a2b  8945d8                MOV dword ptr [EBP + -0x28],EAX
008a8a2e  6a01                  PUSH 0x1
008a8a30  6a00                  PUSH 0x0
008a8a32  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008a8a35  e8a674e9ff            CALL 0x0073fee0
008a8a3a  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
008a8a3d  51                    PUSH ECX
008a8a3e  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008a8a41  52                    PUSH EDX
008a8a42  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a8a45  e85660b6ff            CALL 0x0040eaa0
008a8a4a  8be5                  MOV ESP,EBP
008a8a4c  5d                    POP EBP
008a8a4d  c20800                RET 0x8
