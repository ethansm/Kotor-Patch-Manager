; FN 0066bdc0 FUN_0066bdc0
0066bdc0  55                    PUSH EBP
0066bdc1  8bec                  MOV EBP,ESP
0066bdc3  83ec4c                SUB ESP,0x4c
0066bdc6  894dbc                MOV dword ptr [EBP + -0x44],ECX
0066bdc9  8d45f0                LEA EAX,[EBP + -0x10]
0066bdcc  50                    PUSH EAX
0066bdcd  6a03                  PUSH 0x3
0066bdcf  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0066bdd5  e8d61c0900            CALL 0x006fdab0
0066bdda  85c0                  TEST EAX,EAX
0066bddc  750a                  JNZ 0x0066bde8
0066bdde  b82ff8ffff            MOV EAX,0xfffff82f
0066bde3  e9a3010000            JMP 0x0066bf8b
0066bde8  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0066bdeb  8b11                  MOV EDX,dword ptr [ECX]
0066bded  8955f8                MOV dword ptr [EBP + -0x8],EDX
0066bdf0  837df8ff              CMP dword ptr [EBP + -0x8],-0x1
0066bdf4  0f8e50010000          JLE 0x0066bf4a
0066bdfa  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0066bdfd  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0066be00  894de0                MOV dword ptr [EBP + -0x20],ECX
0066be03  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0066be06  8a4208                MOV AL,byte ptr [EDX + 0x8]
0066be09  8845ff                MOV byte ptr [EBP + -0x1],AL
0066be0c  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0066be0f  8b510c                MOV EDX,dword ptr [ECX + 0xc]
0066be12  8955e8                MOV dword ptr [EBP + -0x18],EDX
0066be15  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0066be18  8b4810                MOV ECX,dword ptr [EAX + 0x10]
0066be1b  894dec                MOV dword ptr [EBP + -0x14],ECX
0066be1e  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0066be21  8a4214                MOV AL,byte ptr [EDX + 0x14]
0066be24  8845f7                MOV byte ptr [EBP + -0x9],AL
0066be27  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0066be2a  8a5115                MOV DL,byte ptr [ECX + 0x15]
0066be2d  8855e7                MOV byte ptr [EBP + -0x19],DL
0066be30  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0066be35  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0066be38  894dc0                MOV dword ptr [EBP + -0x40],ECX
0066be3b  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
0066be3e  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0066be41  50                    PUSH EAX
0066be42  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0066be45  e8b602ebff            CALL 0x0051c100
0066be4a  8945dc                MOV dword ptr [EBP + -0x24],EAX
0066be4d  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0066be51  0f84f3000000          JZ 0x0066bf4a
0066be57  817d0835010000        CMP dword ptr [EBP + 0x8],0x135
0066be5e  7554                  JNZ 0x0066beb4
0066be60  c745d80000007f        MOV dword ptr [EBP + -0x28],0x7f000000
0066be67  8d4dd8                LEA ECX,[EBP + -0x28]
0066be6a  51                    PUSH ECX
0066be6b  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0066be71  e87a1c0900            CALL 0x006fdaf0
0066be76  85c0                  TEST EAX,EAX
0066be78  750a                  JNZ 0x0066be84
0066be7a  b82ff8ffff            MOV EAX,0xfffff82f
0066be7f  e907010000            JMP 0x0066bf8b
0066be84  0fb655e7              MOVZX EDX,byte ptr [EBP + -0x19]
0066be88  52                    PUSH EDX
0066be89  0fb645f7              MOVZX EAX,byte ptr [EBP + -0x9]
0066be8d  50                    PUSH EAX
0066be8e  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0066be91  51                    PUSH ECX
0066be92  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0066be95  52                    PUSH EDX
0066be96  0fb645ff              MOVZX EAX,byte ptr [EBP + -0x1]
0066be9a  50                    PUSH EAX
0066be9b  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0066be9e  51                    PUSH ECX
0066be9f  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0066bea2  52                    PUSH EDX
0066bea3  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0066bea6  50                    PUSH EAX
0066bea7  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0066beaa  e8011af1ff            CALL 0x0057d8b0
0066beaf  e996000000            JMP 0x0066bf4a
0066beb4  817d0836010000        CMP dword ptr [EBP + 0x8],0x136
0066bebb  0f8589000000          JNZ 0x0066bf4a
0066bec1  8d4dd4                LEA ECX,[EBP + -0x2c]
0066bec4  51                    PUSH ECX
0066bec5  6a02                  PUSH 0x2
0066bec7  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0066becd  e8de1b0900            CALL 0x006fdab0
0066bed2  85c0                  TEST EAX,EAX
0066bed4  750a                  JNZ 0x0066bee0
0066bed6  b82ff8ffff            MOV EAX,0xfffff82f
0066bedb  e9ab000000            JMP 0x0066bf8b
0066bee0  0fb655e7              MOVZX EDX,byte ptr [EBP + -0x19]
0066bee4  52                    PUSH EDX
0066bee5  0fb645f7              MOVZX EAX,byte ptr [EBP + -0x9]
0066bee9  50                    PUSH EAX
0066beea  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0066beed  51                    PUSH ECX
0066beee  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0066bef1  52                    PUSH EDX
0066bef2  0fb645ff              MOVZX EAX,byte ptr [EBP + -0x1]
0066bef6  50                    PUSH EAX
0066bef7  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0066befa  51                    PUSH ECX
0066befb  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0066befe  52                    PUSH EDX
0066beff  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0066bf02  50                    PUSH EAX
0066bf03  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0066bf06  e8851bf1ff            CALL 0x0057da90
0066bf0b  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0066bf0e  894dcc                MOV dword ptr [EBP + -0x34],ECX
0066bf11  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
0066bf14  8955d0                MOV dword ptr [EBP + -0x30],EDX
0066bf17  837dd000              CMP dword ptr [EBP + -0x30],0x0
0066bf1b  7426                  JZ 0x0066bf43
0066bf1d  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0066bf20  e8fb73fbff            CALL 0x00623320
0066bf25  b801000000            MOV EAX,0x1
0066bf2a  83e001                AND EAX,0x1
0066bf2d  740c                  JZ 0x0066bf3b
0066bf2f  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0066bf32  51                    PUSH ECX
0066bf33  e8c5d72a00            CALL 0x009196fd
0066bf38  83c404                ADD ESP,0x4
0066bf3b  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
0066bf3e  8955b8                MOV dword ptr [EBP + -0x48],EDX
0066bf41  eb07                  JMP 0x0066bf4a
0066bf43  c745b800000000        MOV dword ptr [EBP + -0x48],0x0
0066bf4a  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0066bf4d  8945c4                MOV dword ptr [EBP + -0x3c],EAX
0066bf50  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0066bf53  894dc8                MOV dword ptr [EBP + -0x38],ECX
0066bf56  837dc800              CMP dword ptr [EBP + -0x38],0x0
0066bf5a  7426                  JZ 0x0066bf82
0066bf5c  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
0066bf5f  e8bc73fbff            CALL 0x00623320
0066bf64  ba01000000            MOV EDX,0x1
0066bf69  83e201                AND EDX,0x1
0066bf6c  740c                  JZ 0x0066bf7a
0066bf6e  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
0066bf71  50                    PUSH EAX
0066bf72  e886d72a00            CALL 0x009196fd
0066bf77  83c404                ADD ESP,0x4
0066bf7a  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
0066bf7d  894db4                MOV dword ptr [EBP + -0x4c],ECX
0066bf80  eb07                  JMP 0x0066bf89
0066bf82  c745b400000000        MOV dword ptr [EBP + -0x4c],0x0
0066bf89  33c0                  XOR EAX,EAX
0066bf8b  8be5                  MOV ESP,EBP
0066bf8d  5d                    POP EBP
0066bf8e  c20800                RET 0x8
