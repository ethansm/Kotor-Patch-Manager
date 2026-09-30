; FN 0077b670 FUN_0077b670
0077b670  55                    PUSH EBP
0077b671  8bec                  MOV EBP,ESP
0077b673  81eca4000000          SUB ESP,0xa4
0077b679  898d64ffffff          MOV dword ptr [EBP + 0xffffff64],ECX
0077b67f  837d0800              CMP dword ptr [EBP + 0x8],0x0
0077b683  0f84fa040000          JZ 0x0077bb83
0077b689  8b8d64ffffff          MOV ECX,dword ptr [EBP + 0xffffff64]
0077b68f  e86c210000            CALL 0x0077d800
0077b694  8945f4                MOV dword ptr [EBP + -0xc],EAX
0077b697  8b8564ffffff          MOV EAX,dword ptr [EBP + 0xffffff64]
0077b69d  8b4820                MOV ECX,dword ptr [EAX + 0x20]
0077b6a0  894de0                MOV dword ptr [EBP + -0x20],ECX
0077b6a3  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0077b6a6  e855f70200            CALL 0x007aae00
0077b6ab  8945dc                MOV dword ptr [EBP + -0x24],EAX
0077b6ae  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0077b6b1  8b82e4020000          MOV EAX,dword ptr [EDX + 0x2e4]
0077b6b7  8945f0                MOV dword ptr [EBP + -0x10],EAX
0077b6ba  837df400              CMP dword ptr [EBP + -0xc],0x0
0077b6be  7507                  JNZ 0x0077b6c7
0077b6c0  33c0                  XOR EAX,EAX
0077b6c2  e9be040000            JMP 0x0077bb85
0077b6c7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b6ca  8b910c010000          MOV EDX,dword ptr [ECX + 0x10c]
0077b6d0  8955d8                MOV dword ptr [EBP + -0x28],EDX
0077b6d3  837dd800              CMP dword ptr [EBP + -0x28],0x0
0077b6d7  0f848b010000          JZ 0x0077b868
0077b6dd  0fb64510              MOVZX EAX,byte ptr [EBP + 0x10]
0077b6e1  83e002                AND EAX,0x2
0077b6e4  0f8480000000          JZ 0x0077b76a
0077b6ea  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b6ed  e83eb7f5ff            CALL 0x006d6e30
0077b6f2  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0077b6f5  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0077b6f8  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
0077b6fe  8855d3                MOV byte ptr [EBP + -0x2d],DL
0077b701  0fb645d3              MOVZX EAX,byte ptr [EBP + -0x2d]
0077b705  83f81c                CMP EAX,0x1c
0077b708  7560                  JNZ 0x0077b76a
0077b70a  6a00                  PUSH 0x0
0077b70c  6a2e                  PUSH 0x2e
0077b70e  8d4df8                LEA ECX,[EBP + -0x8]
0077b711  51                    PUSH ECX
0077b712  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b715  e8c68be8ff            CALL 0x006042e0
0077b71a  85c0                  TEST EAX,EAX
0077b71c  7432                  JZ 0x0077b750
0077b71e  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0077b721  8a420c                MOV AL,byte ptr [EDX + 0xc]
0077b724  8845d2                MOV byte ptr [EBP + -0x2e],AL
0077b727  0fb64dd2              MOVZX ECX,byte ptr [EBP + -0x2e]
0077b72b  85c9                  TEST ECX,ECX
0077b72d  741a                  JZ 0x0077b749
0077b72f  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0077b732  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
0077b738  8945cc                MOV dword ptr [EBP + -0x34],EAX
0077b73b  6a01                  PUSH 0x1
0077b73d  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0077b740  e8bbc2f3ff            CALL 0x006b7a00
0077b745  85c0                  TEST EAX,EAX
0077b747  7507                  JNZ 0x0077b750
0077b749  33c0                  XOR EAX,EAX
0077b74b  e935040000            JMP 0x0077bb85
0077b750  6a01                  PUSH 0x1
0077b752  6a01                  PUSH 0x1
0077b754  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b757  51                    PUSH ECX
0077b758  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0077b75b  e82078e5ff            CALL 0x005d2f80
0077b760  e920040000            JMP 0x0077bb85
0077b76a  6a00                  PUSH 0x0
0077b76c  6a0a                  PUSH 0xa
0077b76e  8d55f8                LEA EDX,[EBP + -0x8]
0077b771  52                    PUSH EDX
0077b772  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b775  e8668be8ff            CALL 0x006042e0
0077b77a  85c0                  TEST EAX,EAX
0077b77c  0f84e1000000          JZ 0x0077b863
0077b782  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b785  e8a6b6f5ff            CALL 0x006d6e30
0077b78a  8945e8                MOV dword ptr [EBP + -0x18],EAX
0077b78d  837de800              CMP dword ptr [EBP + -0x18],0x0
0077b791  7470                  JZ 0x0077b803
0077b793  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0077b796  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
0077b79c  884dcb                MOV byte ptr [EBP + -0x35],CL
0077b79f  8a55cb                MOV DL,byte ptr [EBP + -0x35]
0077b7a2  889560ffffff          MOV byte ptr [EBP + 0xffffff60],DL
0077b7a8  80bd60ffffff0c        CMP byte ptr [EBP + 0xffffff60],0xc
0077b7af  7242                  JC 0x0077b7f3
0077b7b1  80bd60ffffff0d        CMP byte ptr [EBP + 0xffffff60],0xd
0077b7b8  760b                  JBE 0x0077b7c5
0077b7ba  80bd60ffffff14        CMP byte ptr [EBP + 0xffffff60],0x14
0077b7c1  7402                  JZ 0x0077b7c5
0077b7c3  eb2e                  JMP 0x0077b7f3
0077b7c5  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0077b7c8  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
0077b7ce  894dc4                MOV dword ptr [EBP + -0x3c],ECX
0077b7d1  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0077b7d4  52                    PUSH EDX
0077b7d5  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0077b7d8  e8634df5ff            CALL 0x006d0540
0077b7dd  85c0                  TEST EAX,EAX
0077b7df  7409                  JZ 0x0077b7ea
0077b7e1  0fb64510              MOVZX EAX,byte ptr [EBP + 0x10]
0077b7e5  83e004                AND EAX,0x4
0077b7e8  7507                  JNZ 0x0077b7f1
0077b7ea  33c0                  XOR EAX,EAX
0077b7ec  e994030000            JMP 0x0077bb85
0077b7f1  eb10                  JMP 0x0077b803
0077b7f3  0fb64d10              MOVZX ECX,byte ptr [EBP + 0x10]
0077b7f7  83e104                AND ECX,0x4
0077b7fa  7507                  JNZ 0x0077b803
0077b7fc  33c0                  XOR EAX,EAX
0077b7fe  e982030000            JMP 0x0077bb85
0077b803  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0077b806  668b4202              MOV AX,word ptr [EDX + 0x2]
0077b80a  668945c2              MOV word ptr [EBP + -0x3e],AX
0077b80e  0fb74dc2              MOVZX ECX,word ptr [EBP + -0x3e]
0077b812  894dfc                MOV dword ptr [EBP + -0x4],ECX
0077b815  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
0077b81b  8955bc                MOV dword ptr [EBP + -0x44],EDX
0077b81e  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0077b821  50                    PUSH EAX
0077b822  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0077b825  8b8904010000          MOV ECX,dword ptr [ECX + 0x104]
0077b82b  e8205cf4ff            CALL 0x006c1450
0077b830  8945ec                MOV dword ptr [EBP + -0x14],EAX
0077b833  837dec00              CMP dword ptr [EBP + -0x14],0x0
0077b837  742a                  JZ 0x0077b863
0077b839  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
0077b83c  8a82a4010000          MOV AL,byte ptr [EDX + 0x1a4]
0077b842  8845bb                MOV byte ptr [EBP + -0x45],AL
0077b845  0fb64dbb              MOVZX ECX,byte ptr [EBP + -0x45]
0077b849  83f901                CMP ECX,0x1
0077b84c  7515                  JNZ 0x0077b863
0077b84e  6a01                  PUSH 0x1
0077b850  6a01                  PUSH 0x1
0077b852  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0077b855  52                    PUSH EDX
0077b856  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0077b859  e82277e5ff            CALL 0x005d2f80
0077b85e  e922030000            JMP 0x0077bb85
0077b863  e91b030000            JMP 0x0077bb83
0077b868  837df000              CMP dword ptr [EBP + -0x10],0x0
0077b86c  0f8511030000          JNZ 0x0077bb83
0077b872  0fb64510              MOVZX EAX,byte ptr [EBP + 0x10]
0077b876  83e002                AND EAX,0x2
0077b879  747b                  JZ 0x0077b8f6
0077b87b  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b87e  e8adb5f5ff            CALL 0x006d6e30
0077b883  8945b4                MOV dword ptr [EBP + -0x4c],EAX
0077b886  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
0077b889  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
0077b88f  8855b3                MOV byte ptr [EBP + -0x4d],DL
0077b892  0fb645b3              MOVZX EAX,byte ptr [EBP + -0x4d]
0077b896  83f81c                CMP EAX,0x1c
0077b899  755b                  JNZ 0x0077b8f6
0077b89b  6a00                  PUSH 0x0
0077b89d  6a2e                  PUSH 0x2e
0077b89f  8d4df8                LEA ECX,[EBP + -0x8]
0077b8a2  51                    PUSH ECX
0077b8a3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b8a6  e8358ae8ff            CALL 0x006042e0
0077b8ab  85c0                  TEST EAX,EAX
0077b8ad  7432                  JZ 0x0077b8e1
0077b8af  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0077b8b2  8a420c                MOV AL,byte ptr [EDX + 0xc]
0077b8b5  8845b2                MOV byte ptr [EBP + -0x4e],AL
0077b8b8  0fb64db2              MOVZX ECX,byte ptr [EBP + -0x4e]
0077b8bc  85c9                  TEST ECX,ECX
0077b8be  741a                  JZ 0x0077b8da
0077b8c0  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0077b8c3  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
0077b8c9  8945ac                MOV dword ptr [EBP + -0x54],EAX
0077b8cc  6a01                  PUSH 0x1
0077b8ce  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
0077b8d1  e82ac1f3ff            CALL 0x006b7a00
0077b8d6  85c0                  TEST EAX,EAX
0077b8d8  7507                  JNZ 0x0077b8e1
0077b8da  33c0                  XOR EAX,EAX
0077b8dc  e9a4020000            JMP 0x0077bb85
0077b8e1  6a01                  PUSH 0x1
0077b8e3  6a01                  PUSH 0x1
0077b8e5  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b8e8  51                    PUSH ECX
0077b8e9  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0077b8ec  e88f76e5ff            CALL 0x005d2f80
0077b8f1  e98f020000            JMP 0x0077bb85
0077b8f6  0fb65510              MOVZX EDX,byte ptr [EBP + 0x10]
0077b8fa  83e201                AND EDX,0x1
0077b8fd  0f8451010000          JZ 0x0077ba54
0077b903  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b906  e825b5f5ff            CALL 0x006d6e30
0077b90b  8945a8                MOV dword ptr [EBP + -0x58],EAX
0077b90e  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
0077b911  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
0077b917  884da7                MOV byte ptr [EBP + -0x59],CL
0077b91a  0fb655a7              MOVZX EDX,byte ptr [EBP + -0x59]
0077b91e  83fa19                CMP EDX,0x19
0077b921  7464                  JZ 0x0077b987
0077b923  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b926  e805b5f5ff            CALL 0x006d6e30
0077b92b  8945a0                MOV dword ptr [EBP + -0x60],EAX
0077b92e  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
0077b931  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
0077b937  884d9f                MOV byte ptr [EBP + -0x61],CL
0077b93a  0fb6559f              MOVZX EDX,byte ptr [EBP + -0x61]
0077b93e  83fa2d                CMP EDX,0x2d
0077b941  7444                  JZ 0x0077b987
0077b943  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b946  e8e5b4f5ff            CALL 0x006d6e30
0077b94b  894598                MOV dword ptr [EBP + -0x68],EAX
0077b94e  8b4598                MOV EAX,dword ptr [EBP + -0x68]
0077b951  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
0077b957  884d97                MOV byte ptr [EBP + -0x69],CL
0077b95a  0fb65597              MOVZX EDX,byte ptr [EBP + -0x69]
0077b95e  83fa2f                CMP EDX,0x2f
0077b961  7424                  JZ 0x0077b987
0077b963  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b966  e8c5b4f5ff            CALL 0x006d6e30
0077b96b  894590                MOV dword ptr [EBP + -0x70],EAX
0077b96e  8b4590                MOV EAX,dword ptr [EBP + -0x70]
0077b971  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
0077b977  884d8f                MOV byte ptr [EBP + -0x71],CL
0077b97a  0fb6558f              MOVZX EDX,byte ptr [EBP + -0x71]
0077b97e  83fa1a                CMP EDX,0x1a
0077b981  0f85cd000000          JNZ 0x0077ba54
0077b987  6a00                  PUSH 0x0
0077b989  6a0a                  PUSH 0xa
0077b98b  8d45f8                LEA EAX,[EBP + -0x8]
0077b98e  50                    PUSH EAX
0077b98f  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b992  e84989e8ff            CALL 0x006042e0
0077b997  85c0                  TEST EAX,EAX
0077b999  7418                  JZ 0x0077b9b3
0077b99b  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0077b99e  8a510c                MOV DL,byte ptr [ECX + 0xc]
0077b9a1  88558e                MOV byte ptr [EBP + -0x72],DL
0077b9a4  0fb6458e              MOVZX EAX,byte ptr [EBP + -0x72]
0077b9a8  85c0                  TEST EAX,EAX
0077b9aa  7507                  JNZ 0x0077b9b3
0077b9ac  33c0                  XOR EAX,EAX
0077b9ae  e9d2010000            JMP 0x0077bb85
0077b9b3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077b9b6  e875b4f5ff            CALL 0x006d6e30
0077b9bb  894588                MOV dword ptr [EBP + -0x78],EAX
0077b9be  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
0077b9c1  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
0077b9c7  885587                MOV byte ptr [EBP + -0x79],DL
0077b9ca  0fb64587              MOVZX EAX,byte ptr [EBP + -0x79]
0077b9ce  83f81a                CMP EAX,0x1a
0077b9d1  753c                  JNZ 0x0077ba0f
0077b9d3  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0077b9d9  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0077b9dc  895580                MOV dword ptr [EBP + -0x80],EDX
0077b9df  0fb705b8a89900        MOVZX EAX,word ptr [0x0099a8b8]
0077b9e6  50                    PUSH EAX
0077b9e7  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
0077b9ea  e8a141fcff            CALL 0x0073fb90
0077b9ef  8bc8                  MOV ECX,EAX
0077b9f1  e83ace0600            CALL 0x007e8830
0077b9f6  85c0                  TEST EAX,EAX
0077b9f8  7509                  JNZ 0x0077ba03
0077b9fa  33c0                  XOR EAX,EAX
0077b9fc  e984010000            JMP 0x0077bb85
0077ba03  b801000000            MOV EAX,0x1
0077ba08  e978010000            JMP 0x0077bb85
0077ba0f  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0077ba15  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0077ba18  89957cffffff          MOV dword ptr [EBP + 0xffffff7c],EDX
0077ba1e  0fb705bca89900        MOVZX EAX,word ptr [0x0099a8bc]
0077ba25  50                    PUSH EAX
0077ba26  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
0077ba2c  e85f41fcff            CALL 0x0073fb90
0077ba31  8bc8                  MOV ECX,EAX
0077ba33  e8f8cd0600            CALL 0x007e8830
0077ba38  85c0                  TEST EAX,EAX
0077ba3a  7509                  JNZ 0x0077ba45
0077ba3c  33c0                  XOR EAX,EAX
0077ba3e  e942010000            JMP 0x0077bb85
0077ba45  b801000000            MOV EAX,0x1
0077ba4a  e936010000            JMP 0x0077bb85
0077ba54  6a00                  PUSH 0x0
0077ba56  6a0a                  PUSH 0xa
0077ba58  8d4df8                LEA ECX,[EBP + -0x8]
0077ba5b  51                    PUSH ECX
0077ba5c  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077ba5f  e87c88e8ff            CALL 0x006042e0
0077ba64  85c0                  TEST EAX,EAX
0077ba66  0f8417010000          JZ 0x0077bb83
0077ba6c  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0077ba6f  8a420c                MOV AL,byte ptr [EDX + 0xc]
0077ba72  88857bffffff          MOV byte ptr [EBP + 0xffffff7b],AL
0077ba78  0fb68d7bffffff        MOVZX ECX,byte ptr [EBP + 0xffffff7b]
0077ba7f  85c9                  TEST ECX,ECX
0077ba81  7507                  JNZ 0x0077ba8a
0077ba83  33c0                  XOR EAX,EAX
0077ba85  e9fb000000            JMP 0x0077bb85
0077ba8a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077ba8d  e89eb3f5ff            CALL 0x006d6e30
0077ba92  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0077ba95  837de400              CMP dword ptr [EBP + -0x1c],0x0
0077ba99  7479                  JZ 0x0077bb14
0077ba9b  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
0077ba9e  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
0077baa4  88857affffff          MOV byte ptr [EBP + 0xffffff7a],AL
0077baaa  8a8d7affffff          MOV CL,byte ptr [EBP + 0xffffff7a]
0077bab0  888d5cffffff          MOV byte ptr [EBP + 0xffffff5c],CL
0077bab6  80bd5cffffff0c        CMP byte ptr [EBP + 0xffffff5c],0xc
0077babd  7248                  JC 0x0077bb07
0077babf  80bd5cffffff0d        CMP byte ptr [EBP + 0xffffff5c],0xd
0077bac6  760b                  JBE 0x0077bad3
0077bac8  80bd5cffffff14        CMP byte ptr [EBP + 0xffffff5c],0x14
0077bacf  7402                  JZ 0x0077bad3
0077bad1  eb34                  JMP 0x0077bb07
0077bad3  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0077bad6  8b8250110000          MOV EAX,dword ptr [EDX + 0x1150]
0077badc  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
0077bae2  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077bae5  51                    PUSH ECX
0077bae6  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
0077baec  e84f4af5ff            CALL 0x006d0540
0077baf1  85c0                  TEST EAX,EAX
0077baf3  7409                  JZ 0x0077bafe
0077baf5  0fb65510              MOVZX EDX,byte ptr [EBP + 0x10]
0077baf9  83e204                AND EDX,0x4
0077bafc  7507                  JNZ 0x0077bb05
0077bafe  33c0                  XOR EAX,EAX
0077bb00  e980000000            JMP 0x0077bb85
0077bb05  eb0d                  JMP 0x0077bb14
0077bb07  0fb64510              MOVZX EAX,byte ptr [EBP + 0x10]
0077bb0b  83e004                AND EAX,0x4
0077bb0e  7504                  JNZ 0x0077bb14
0077bb10  33c0                  XOR EAX,EAX
0077bb12  eb71                  JMP 0x0077bb85
0077bb14  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0077bb17  668b5102              MOV DX,word ptr [ECX + 0x2]
0077bb1b  66899572ffffff        MOV word ptr [EBP + 0xffffff72],DX
0077bb22  0fb78572ffffff        MOVZX EAX,word ptr [EBP + 0xffffff72]
0077bb29  8945fc                MOV dword ptr [EBP + -0x4],EAX
0077bb2c  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
0077bb32  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
0077bb38  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0077bb3b  52                    PUSH EDX
0077bb3c  8b856cffffff          MOV EAX,dword ptr [EBP + 0xffffff6c]
0077bb42  8b8804010000          MOV ECX,dword ptr [EAX + 0x104]
0077bb48  e80359f4ff            CALL 0x006c1450
0077bb4d  8945ec                MOV dword ptr [EBP + -0x14],EAX
0077bb50  837dec00              CMP dword ptr [EBP + -0x14],0x0
0077bb54  742d                  JZ 0x0077bb83
0077bb56  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0077bb59  8a91a4010000          MOV DL,byte ptr [ECX + 0x1a4]
0077bb5f  88956bffffff          MOV byte ptr [EBP + 0xffffff6b],DL
0077bb65  0fb6856bffffff        MOVZX EAX,byte ptr [EBP + 0xffffff6b]
0077bb6c  83f801                CMP EAX,0x1
0077bb6f  7512                  JNZ 0x0077bb83
0077bb71  6a01                  PUSH 0x1
0077bb73  6a01                  PUSH 0x1
0077bb75  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0077bb78  51                    PUSH ECX
0077bb79  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0077bb7c  e8ff73e5ff            CALL 0x005d2f80
0077bb81  eb02                  JMP 0x0077bb85
0077bb83  33c0                  XOR EAX,EAX
0077bb85  8be5                  MOV ESP,EBP
0077bb87  5d                    POP EBP
0077bb88  c20c00                RET 0xc
