; FN 006da4f0 FUN_006da4f0
006da4f0  55                    PUSH EBP
006da4f1  8bec                  MOV EBP,ESP
006da4f3  81eca4000000          SUB ESP,0xa4
006da4f9  898d5cffffff          MOV dword ptr [EBP + 0xffffff5c],ECX
006da4ff  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
006da506  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
006da50d  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
006da514  c745fc0000007f        MOV dword ptr [EBP + -0x4],0x7f000000
006da51b  837d0c00              CMP dword ptr [EBP + 0xc],0x0
006da51f  7411                  JZ 0x006da532
006da521  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
006da527  8b8868050000          MOV ECX,dword ptr [EAX + 0x568]
006da52d  894dfc                MOV dword ptr [EBP + -0x4],ECX
006da530  eb1b                  JMP 0x006da54d
006da532  8b955cffffff          MOV EDX,dword ptr [EBP + 0xffffff5c]
006da538  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006da53e  8945cc                MOV dword ptr [EBP + -0x34],EAX
006da541  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
006da544  8b91200b0000          MOV EDX,dword ptr [ECX + 0xb20]
006da54a  8955fc                MOV dword ptr [EBP + -0x4],EDX
006da54d  817dfc0000007f        CMP dword ptr [EBP + -0x4],0x7f000000
006da554  0f840e020000          JZ 0x006da768
006da55a  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
006da560  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006da566  894dc8                MOV dword ptr [EBP + -0x38],ECX
006da569  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
006da56c  8b82000b0000          MOV EAX,dword ptr [EDX + 0xb00]
006da572  8945c4                MOV dword ptr [EBP + -0x3c],EAX
006da575  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
006da578  e803720c00            CALL 0x007a1780
006da57d  85c0                  TEST EAX,EAX
006da57f  0f84e3010000          JZ 0x006da768
006da585  c745f401000000        MOV dword ptr [EBP + -0xc],0x1
006da58c  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006da592  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006da595  8955c0                MOV dword ptr [EBP + -0x40],EDX
006da598  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
006da59b  50                    PUSH EAX
006da59c  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
006da59f  e8fc1ae4ff            CALL 0x0051c0a0
006da5a4  8945ec                MOV dword ptr [EBP + -0x14],EAX
006da5a7  837dec00              CMP dword ptr [EBP + -0x14],0x0
006da5ab  0f848c010000          JZ 0x006da73d
006da5b1  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006da5b4  8b11                  MOV EDX,dword ptr [ECX]
006da5b6  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006da5b9  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006da5bc  ffd0                  CALL EAX
006da5be  85c0                  TEST EAX,EAX
006da5c0  0f8477010000          JZ 0x006da73d
006da5c6  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006da5c9  8b11                  MOV EDX,dword ptr [ECX]
006da5cb  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006da5ce  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006da5d1  ffd0                  CALL EAX
006da5d3  8945f0                MOV dword ptr [EBP + -0x10],EAX
006da5d6  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
006da5d9  8b11                  MOV EDX,dword ptr [ECX]
006da5db  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
006da5de  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
006da5e4  ffd0                  CALL EAX
006da5e6  85c0                  TEST EAX,EAX
006da5e8  0f8599000000          JNZ 0x006da687
006da5ee  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
006da5f1  83b9ac11000000        CMP dword ptr [ECX + 0x11ac],0x0
006da5f8  7421                  JZ 0x006da61b
006da5fa  6a00                  PUSH 0x0
006da5fc  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
006da5ff  8b02                  MOV EAX,dword ptr [EDX]
006da601  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
006da604  8b909c000000          MOV EDX,dword ptr [EAX + 0x9c]
006da60a  ffd2                  CALL EDX
006da60c  98                    CWDE
006da60d  33c9                  XOR ECX,ECX
006da60f  85c0                  TEST EAX,EAX
006da611  0f9ec1                SETLE CL
006da614  894dbc                MOV dword ptr [EBP + -0x44],ECX
006da617  eb09                  JMP 0x006da622
006da61b  c745bc00000000        MOV dword ptr [EBP + -0x44],0x0
006da622  837dbc00              CMP dword ptr [EBP + -0x44],0x0
006da626  755f                  JNZ 0x006da687
006da628  8b955cffffff          MOV EDX,dword ptr [EBP + 0xffffff5c]
006da62e  8b02                  MOV EAX,dword ptr [EDX]
006da630  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da636  8b9094000000          MOV EDX,dword ptr [EAX + 0x94]
006da63c  ffd2                  CALL EDX
006da63e  85c0                  TEST EAX,EAX
006da640  7545                  JNZ 0x006da687
006da642  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
006da648  83b8ac11000000        CMP dword ptr [EAX + 0x11ac],0x0
006da64f  7429                  JZ 0x006da67a
006da651  6a00                  PUSH 0x0
006da653  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da659  8b11                  MOV EDX,dword ptr [ECX]
006da65b  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da661  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
006da667  ffd0                  CALL EAX
006da669  0fbfc8                MOVSX ECX,AX
006da66c  33d2                  XOR EDX,EDX
006da66e  85c9                  TEST ECX,ECX
006da670  0f9ec2                SETLE DL
006da673  8955b8                MOV dword ptr [EBP + -0x48],EDX
006da676  eb09                  JMP 0x006da681
006da67a  c745b800000000        MOV dword ptr [EBP + -0x48],0x0
006da681  837db800              CMP dword ptr [EBP + -0x48],0x0
006da685  740c                  JZ 0x006da693
006da687  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
006da68e  e9a8000000            JMP 0x006da73b
006da693  6a00                  PUSH 0x0
006da695  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
006da698  50                    PUSH EAX
006da699  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da69f  e81c6df0ff            CALL 0x005e13c0
006da6a4  83f80a                CMP EAX,0xa
006da6a7  7e24                  JLE 0x006da6cd
006da6a9  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da6af  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006da6b2  8955b4                MOV dword ptr [EBP + -0x4c],EDX
006da6b5  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
006da6b8  3b45b4                CMP EAX,dword ptr [EBP + -0x4c]
006da6bb  750e                  JNZ 0x006da6cb
006da6bd  c745f401000000        MOV dword ptr [EBP + -0xc],0x1
006da6c4  c745f801000000        MOV dword ptr [EBP + -0x8],0x1
006da6cb  eb6e                  JMP 0x006da73b
006da6cd  837d0c00              CMP dword ptr [EBP + 0xc],0x0
006da6d1  7468                  JZ 0x006da73b
006da6d3  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da6d9  81c194000000          ADD ECX,0x94
006da6df  51                    PUSH ECX
006da6e0  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
006da6e3  81c294000000          ADD EDX,0x94
006da6e9  52                    PUSH EDX
006da6ea  8d45dc                LEA EAX,[EBP + -0x24]
006da6ed  50                    PUSH EAX
006da6ee  e8bdabd6ff            CALL 0x004452b0
006da6f3  83c40c                ADD ESP,0xc
006da6f6  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
006da6f9  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006da6fc  8955a4                MOV dword ptr [EBP + -0x5c],EDX
006da6ff  6a01                  PUSH 0x1
006da701  6a00                  PUSH 0x0
006da703  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
006da706  50                    PUSH EAX
006da707  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da70d  e82e6ee9ff            CALL 0x00571540
006da712  d95de8                FSTP float ptr [EBP + -0x18]
006da715  8d4ddc                LEA ECX,[EBP + -0x24]
006da718  51                    PUSH ECX
006da719  8d55dc                LEA EDX,[EBP + -0x24]
006da71c  52                    PUSH EDX
006da71d  e87e11d7ff            CALL 0x0044b8a0
006da722  83c408                ADD ESP,0x8
006da725  d945e8                FLD float ptr [EBP + -0x18]
006da728  d84de8                FMUL float ptr [EBP + -0x18]
006da72b  ded9                  FCOMPP
006da72d  dfe0                  FNSTSW AX
006da72f  f6c405                TEST AH,0x5
006da732  7a07                  JP 0x006da73b
006da734  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
006da73b  eb29                  JMP 0x006da766
006da73d  837dec00              CMP dword ptr [EBP + -0x14],0x0
006da741  7523                  JNZ 0x006da766
006da743  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
006da749  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006da74f  894da0                MOV dword ptr [EBP + -0x60],ECX
006da752  8b55a0                MOV EDX,dword ptr [EBP + -0x60]
006da755  c782200b00000000007f  MOV dword ptr [EDX + 0xb20],0x7f000000
006da75f  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
006da766  eb37                  JMP 0x006da79f
006da768  817dfc0000007f        CMP dword ptr [EBP + -0x4],0x7f000000
006da76f  752e                  JNZ 0x006da79f
006da771  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
006da777  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006da77d  894d9c                MOV dword ptr [EBP + -0x64],ECX
006da780  8b559c                MOV EDX,dword ptr [EBP + -0x64]
006da783  8b82000b0000          MOV EAX,dword ptr [EDX + 0xb00]
006da789  894598                MOV dword ptr [EBP + -0x68],EAX
006da78c  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
006da78f  e8ec6f0c00            CALL 0x007a1780
006da794  85c0                  TEST EAX,EAX
006da796  7407                  JZ 0x006da79f
006da798  c7450801000000        MOV dword ptr [EBP + 0x8],0x1
006da79f  837d0800              CMP dword ptr [EBP + 0x8],0x0
006da7a3  740e                  JZ 0x006da7b3
006da7a5  c745f801000000        MOV dword ptr [EBP + -0x8],0x1
006da7ac  c745f401000000        MOV dword ptr [EBP + -0xc],0x1
006da7b3  837df800              CMP dword ptr [EBP + -0x8],0x0
006da7b7  0f8481010000          JZ 0x006da93e
006da7bd  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006da7c3  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006da7c6  895594                MOV dword ptr [EBP + -0x6c],EDX
006da7c9  8b4510                MOV EAX,dword ptr [EBP + 0x10]
006da7cc  50                    PUSH EAX
006da7cd  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
006da7d0  e82b19e4ff            CALL 0x0051c100
006da7d5  8945d8                MOV dword ptr [EBP + -0x28],EAX
006da7d8  c745d40000007f        MOV dword ptr [EBP + -0x2c],0x7f000000
006da7df  837dd800              CMP dword ptr [EBP + -0x28],0x0
006da7e3  744e                  JZ 0x006da833
006da7e5  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da7eb  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006da7ee  895590                MOV dword ptr [EBP + -0x70],EDX
006da7f1  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
006da7f7  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006da7fa  894d8c                MOV dword ptr [EBP + -0x74],ECX
006da7fd  8b955cffffff          MOV EDX,dword ptr [EBP + 0xffffff5c]
006da803  8b4204                MOV EAX,dword ptr [EDX + 0x4]
006da806  894588                MOV dword ptr [EBP + -0x78],EAX
006da809  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
006da80c  51                    PUSH ECX
006da80d  8b558c                MOV EDX,dword ptr [EBP + -0x74]
006da810  52                    PUSH EDX
006da811  6a01                  PUSH 0x1
006da813  6a00                  PUSH 0x0
006da815  8b4588                MOV EAX,dword ptr [EBP + -0x78]
006da818  50                    PUSH EAX
006da819  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da81f  e81c6de9ff            CALL 0x00571540
006da824  51                    PUSH ECX
006da825  d91c24                FSTP float ptr [ESP]
006da828  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006da82b  e86021eaff            CALL 0x0057c990
006da830  8945d4                MOV dword ptr [EBP + -0x2c],EAX
006da833  817dd40000007f        CMP dword ptr [EBP + -0x2c],0x7f000000
006da83a  7546                  JNZ 0x006da882
006da83c  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da842  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006da845  895584                MOV dword ptr [EBP + -0x7c],EDX
006da848  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
006da84e  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006da851  894d80                MOV dword ptr [EBP + -0x80],ECX
006da854  680000007f            PUSH 0x7f000000
006da859  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
006da85c  52                    PUSH EDX
006da85d  6a01                  PUSH 0x1
006da85f  6a00                  PUSH 0x0
006da861  8b4580                MOV EAX,dword ptr [EBP + -0x80]
006da864  50                    PUSH EAX
006da865  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da86b  e8d06ce9ff            CALL 0x00571540
006da870  51                    PUSH ECX
006da871  d91c24                FSTP float ptr [ESP]
006da874  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da87a  e81121eaff            CALL 0x0057c990
006da87f  8945d4                MOV dword ptr [EBP + -0x2c],EAX
006da882  817dd40000007f        CMP dword ptr [EBP + -0x2c],0x7f000000
006da889  0f849b000000          JZ 0x006da92a
006da88f  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006da895  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006da898  89957cffffff          MOV dword ptr [EBP + 0xffffff7c],EDX
006da89e  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
006da8a1  50                    PUSH EAX
006da8a2  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
006da8a8  e8f317e4ff            CALL 0x0051c0a0
006da8ad  8945d0                MOV dword ptr [EBP + -0x30],EAX
006da8b0  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
006da8b3  8b11                  MOV EDX,dword ptr [ECX]
006da8b5  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
006da8b8  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006da8bb  ffd0                  CALL EAX
006da8bd  85c0                  TEST EAX,EAX
006da8bf  7516                  JNZ 0x006da8d7
006da8c1  6a01                  PUSH 0x1
006da8c3  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da8c9  e852020000            CALL 0x006dab20
006da8ce  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
006da8d5  eb51                  JMP 0x006da928
006da8d7  6a00                  PUSH 0x0
006da8d9  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006da8dc  51                    PUSH ECX
006da8dd  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da8e3  e888030000            CALL 0x006dac70
006da8e8  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
006da8eb  8955fc                MOV dword ptr [EBP + -0x4],EDX
006da8ee  a1a4b4a100            MOV EAX,[0x00a1b4a4]
006da8f3  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006da8f6  898d78ffffff          MOV dword ptr [EBP + 0xffffff78],ECX
006da8fc  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
006da902  8b4204                MOV EAX,dword ptr [EDX + 0x4]
006da905  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
006da90b  6a01                  PUSH 0x1
006da90d  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006da910  51                    PUSH ECX
006da911  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
006da917  e8a4550600            CALL 0x0073fec0
006da91c  50                    PUSH EAX
006da91d  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
006da923  e8185d0600            CALL 0x00740640
006da928  eb14                  JMP 0x006da93e
006da92a  6a01                  PUSH 0x1
006da92c  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da932  e8e9010000            CALL 0x006dab20
006da937  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
006da93e  837df400              CMP dword ptr [EBP + -0xc],0x0
006da942  0f84b0000000          JZ 0x006da9f8
006da948  8b955cffffff          MOV EDX,dword ptr [EBP + 0xffffff5c]
006da94e  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006da954  898570ffffff          MOV dword ptr [EBP + 0xffffff70],EAX
006da95a  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
006da960  8a911c0b0000          MOV DL,byte ptr [ECX + 0xb1c]
006da966  88956fffffff          MOV byte ptr [EBP + 0xffffff6f],DL
006da96c  0fb6856fffffff        MOVZX EAX,byte ptr [EBP + 0xffffff6f]
006da973  83f80d                CMP EAX,0xd
006da976  7527                  JNZ 0x006da99f
006da978  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da97e  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006da984  899568ffffff          MOV dword ptr [EBP + 0xffffff68],EDX
006da98a  6a00                  PUSH 0x0
006da98c  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
006da98f  50                    PUSH EAX
006da990  6a00                  PUSH 0x0
006da992  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
006da998  e87363ebff            CALL 0x00590d10
006da99d  eb59                  JMP 0x006da9f8
006da99f  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da9a5  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006da9ab  899564ffffff          MOV dword ptr [EBP + 0xffffff64],EDX
006da9b1  6a01                  PUSH 0x1
006da9b3  68dc050000            PUSH 0x5dc
006da9b8  6a00                  PUSH 0x0
006da9ba  6a00                  PUSH 0x0
006da9bc  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
006da9bf  50                    PUSH EAX
006da9c0  8b8d64ffffff          MOV ECX,dword ptr [EBP + 0xffffff64]
006da9c6  e88560ebff            CALL 0x00590a50
006da9cb  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da9d1  8b9154050000          MOV EDX,dword ptr [ECX + 0x554]
006da9d7  899560ffffff          MOV dword ptr [EBP + 0xffffff60],EDX
006da9dd  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
006da9e3  3b45fc                CMP EAX,dword ptr [EBP + -0x4]
006da9e6  7510                  JNZ 0x006da9f8
006da9e8  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006da9ee  c781540500000000007f  MOV dword ptr [ECX + 0x554],0x7f000000
006da9f8  8be5                  MOV ESP,EBP
006da9fa  5d                    POP EBP
006da9fb  c20c00                RET 0xc
