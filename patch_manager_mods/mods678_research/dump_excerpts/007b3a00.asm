; FN 007b3a00 FUN_007b3a00
007b3a00  55                    PUSH EBP
007b3a01  8bec                  MOV EBP,ESP
007b3a03  83ec70                SUB ESP,0x70
007b3a06  894d90                MOV dword ptr [EBP + -0x70],ECX
007b3a09  8b450c                MOV EAX,dword ptr [EBP + 0xc]
007b3a0c  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3a0f  81c1b4040000          ADD ECX,0x4b4
007b3a15  8b10                  MOV EDX,dword ptr [EAX]
007b3a17  8911                  MOV dword ptr [ECX],EDX
007b3a19  8b5004                MOV EDX,dword ptr [EAX + 0x4]
007b3a1c  895104                MOV dword ptr [ECX + 0x4],EDX
007b3a1f  8b4008                MOV EAX,dword ptr [EAX + 0x8]
007b3a22  894108                MOV dword ptr [ECX + 0x8],EAX
007b3a25  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3a28  8b5508                MOV EDX,dword ptr [EBP + 0x8]
007b3a2b  8991b0040000          MOV dword ptr [ECX + 0x4b0],EDX
007b3a31  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3a34  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007b3a37  894de8                MOV dword ptr [EBP + -0x18],ECX
007b3a3a  837de800              CMP dword ptr [EBP + -0x18],0x0
007b3a3e  7461                  JZ 0x007b3aa1
007b3a40  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3a43  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007b3a46  8945e4                MOV dword ptr [EBP + -0x1c],EAX
007b3a49  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
007b3a4c  8b5130                MOV EDX,dword ptr [ECX + 0x30]
007b3a4f  8955e0                MOV dword ptr [EBP + -0x20],EDX
007b3a52  837de000              CMP dword ptr [EBP + -0x20],0x0
007b3a56  7536                  JNZ 0x007b3a8e
007b3a58  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3a5b  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007b3a5e  894ddc                MOV dword ptr [EBP + -0x24],ECX
007b3a61  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
007b3a64  8b82cc000000          MOV EAX,dword ptr [EDX + 0xcc]
007b3a6a  8945d8                MOV dword ptr [EBP + -0x28],EAX
007b3a6d  837dd801              CMP dword ptr [EBP + -0x28],0x1
007b3a71  741b                  JZ 0x007b3a8e
007b3a73  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3a76  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007b3a79  8955d4                MOV dword ptr [EBP + -0x2c],EDX
007b3a7c  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
007b3a7f  8a4875                MOV CL,byte ptr [EAX + 0x75]
007b3a82  884dd3                MOV byte ptr [EBP + -0x2d],CL
007b3a85  0fb655d3              MOVZX EDX,byte ptr [EBP + -0x2d]
007b3a89  83fa05                CMP EDX,0x5
007b3a8c  7513                  JNZ 0x007b3aa1
007b3a8e  6a00                  PUSH 0x0
007b3a90  6a00                  PUSH 0x0
007b3a92  6a01                  PUSH 0x1
007b3a94  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3a97  e8d4fdffff            CALL 0x007b3870
007b3a9c  e9f4010000            JMP 0x007b3c95
007b3aa1  a19cb4a100            MOV EAX,[0x00a1b49c]
007b3aa6  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007b3aa9  894dfc                MOV dword ptr [EBP + -0x4],ECX
007b3aac  837d1000              CMP dword ptr [EBP + 0x10],0x0
007b3ab0  0f8589000000          JNZ 0x007b3b3f
007b3ab6  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3ab9  8b82b0040000          MOV EAX,dword ptr [EDX + 0x4b0]
007b3abf  8945cc                MOV dword ptr [EBP + -0x34],EAX
007b3ac2  817dcc0000007f        CMP dword ptr [EBP + -0x34],0x7f000000
007b3ac9  0f845b010000          JZ 0x007b3c2a
007b3acf  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3ad2  8b91b4020000          MOV EDX,dword ptr [ECX + 0x2b4]
007b3ad8  8955c8                MOV dword ptr [EBP + -0x38],EDX
007b3adb  817dc80000007f        CMP dword ptr [EBP + -0x38],0x7f000000
007b3ae2  0f8442010000          JZ 0x007b3c2a
007b3ae8  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3aeb  8b88b0040000          MOV ECX,dword ptr [EAX + 0x4b0]
007b3af1  894dc4                MOV dword ptr [EBP + -0x3c],ECX
007b3af4  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007b3afa  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007b3afd  8945c0                MOV dword ptr [EBP + -0x40],EAX
007b3b00  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
007b3b03  e868b9f8ff            CALL 0x0073f470
007b3b08  3945c4                CMP dword ptr [EBP + -0x3c],EAX
007b3b0b  0f8419010000          JZ 0x007b3c2a
007b3b11  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007b3b15  0f850f010000          JNZ 0x007b3c2a
007b3b1b  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3b1e  8b91b4020000          MOV EDX,dword ptr [ECX + 0x2b4]
007b3b24  8955bc                MOV dword ptr [EBP + -0x44],EDX
007b3b27  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3b2a  8b88b0040000          MOV ECX,dword ptr [EAX + 0x4b0]
007b3b30  894db8                MOV dword ptr [EBP + -0x48],ECX
007b3b33  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
007b3b36  3b55b8                CMP EDX,dword ptr [EBP + -0x48]
007b3b39  0f85eb000000          JNZ 0x007b3c2a
007b3b3f  c745ec01000000        MOV dword ptr [EBP + -0x14],0x1
007b3b46  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
007b3b4d  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3b50  e89b050000            CALL 0x007b40f0
007b3b55  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
007b3b5c  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3b5f  8b88d0040000          MOV ECX,dword ptr [EAX + 0x4d0]
007b3b65  894db4                MOV dword ptr [EBP + -0x4c],ECX
007b3b68  837db400              CMP dword ptr [EBP + -0x4c],0x0
007b3b6c  7e29                  JLE 0x007b3b97
007b3b6e  33d2                  XOR EDX,EDX
007b3b70  6bd23c                IMUL EDX,EDX,0x3c
007b3b73  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3b76  0390cc040000          ADD EDX,dword ptr [EAX + 0x4cc]
007b3b7c  8955f4                MOV dword ptr [EBP + -0xc],EDX
007b3b7f  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3b82  8b91d0040000          MOV EDX,dword ptr [ECX + 0x4d0]
007b3b88  8955b0                MOV dword ptr [EBP + -0x50],EDX
007b3b8b  33c0                  XOR EAX,EAX
007b3b8d  837db001              CMP dword ptr [EBP + -0x50],0x1
007b3b91  0f9fc0                SETG AL
007b3b94  8945f8                MOV dword ptr [EBP + -0x8],EAX
007b3b97  8b0dc0fca700          MOV ECX,dword ptr [0x00a7fcc0]
007b3b9d  894df0                MOV dword ptr [EBP + -0x10],ECX
007b3ba0  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3ba3  8b82b0040000          MOV EAX,dword ptr [EDX + 0x4b0]
007b3ba9  8945ac                MOV dword ptr [EBP + -0x54],EAX
007b3bac  817dac0000007f        CMP dword ptr [EBP + -0x54],0x7f000000
007b3bb3  7445                  JZ 0x007b3bfa
007b3bb5  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3bb8  8b91b0040000          MOV EDX,dword ptr [ECX + 0x4b0]
007b3bbe  8955a8                MOV dword ptr [EBP + -0x58],EDX
007b3bc1  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3bc4  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007b3bc7  894da4                MOV dword ptr [EBP + -0x5c],ECX
007b3bca  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
007b3bcd  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
007b3bd3  8945a0                MOV dword ptr [EBP + -0x60],EAX
007b3bd6  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
007b3bd9  8b5168                MOV EDX,dword ptr [ECX + 0x68]
007b3bdc  89559c                MOV dword ptr [EBP + -0x64],EDX
007b3bdf  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
007b3be2  3b459c                CMP EAX,dword ptr [EBP + -0x64]
007b3be5  7413                  JZ 0x007b3bfa
007b3be7  6a00                  PUSH 0x0
007b3be9  6a00                  PUSH 0x0
007b3beb  6a2d                  PUSH 0x2d
007b3bed  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3bf0  e87bfcffff            CALL 0x007b3870
007b3bf5  e99b000000            JMP 0x007b3c95
007b3bfa  837df400              CMP dword ptr [EBP + -0xc],0x0
007b3bfe  7416                  JZ 0x007b3c16
007b3c00  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007b3c03  51                    PUSH ECX
007b3c04  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007b3c07  8b4208                MOV EAX,dword ptr [EDX + 0x8]
007b3c0a  50                    PUSH EAX
007b3c0b  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3c0e  e8cd020000            CALL 0x007b3ee0
007b3c13  8945ec                MOV dword ptr [EBP + -0x14],EAX
007b3c16  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007b3c19  51                    PUSH ECX
007b3c1a  6a00                  PUSH 0x0
007b3c1c  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
007b3c1f  52                    PUSH EDX
007b3c20  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3c23  e848fcffff            CALL 0x007b3870
007b3c28  eb6b                  JMP 0x007b3c95
007b3c2a  8b4590                MOV EAX,dword ptr [EBP + -0x70]
007b3c2d  8b88b0040000          MOV ECX,dword ptr [EAX + 0x4b0]
007b3c33  894d98                MOV dword ptr [EBP + -0x68],ECX
007b3c36  817d980000007f        CMP dword ptr [EBP + -0x68],0x7f000000
007b3c3d  743e                  JZ 0x007b3c7d
007b3c3f  8b5590                MOV EDX,dword ptr [EBP + -0x70]
007b3c42  8b82b0040000          MOV EAX,dword ptr [EDX + 0x4b0]
007b3c48  894594                MOV dword ptr [EBP + -0x6c],EAX
007b3c4b  68e0b67900            PUSH 0x79b6e0
007b3c50  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
007b3c53  51                    PUSH ECX
007b3c54  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3c57  81c1a8020000          ADD ECX,0x2a8
007b3c5d  e83edbfeff            CALL 0x007a17a0
007b3c62  83f8ff                CMP EAX,-0x1
007b3c65  7416                  JZ 0x007b3c7d
007b3c67  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007b3c6b  7510                  JNZ 0x007b3c7d
007b3c6d  6a00                  PUSH 0x0
007b3c6f  6a00                  PUSH 0x0
007b3c71  6a2d                  PUSH 0x2d
007b3c73  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3c76  e8f5fbffff            CALL 0x007b3870
007b3c7b  eb18                  JMP 0x007b3c95
007b3c7d  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007b3c83  e8f8c1f7ff            CALL 0x0072fe80
007b3c88  50                    PUSH EAX
007b3c89  6a00                  PUSH 0x0
007b3c8b  6a01                  PUSH 0x1
007b3c8d  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007b3c90  e8dbfbffff            CALL 0x007b3870
007b3c95  8be5                  MOV ESP,EBP
007b3c97  5d                    POP EBP
007b3c98  c20c00                RET 0xc
