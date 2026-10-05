; FN 005bcdc0 FUN_005bcdc0
005bcdc0  55                    PUSH EBP
005bcdc1  8bec                  MOV EBP,ESP
005bcdc3  81ec9c000000          SUB ESP,0x9c
005bcdc9  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
005bcdcf  8b856cffffff          MOV EAX,dword ptr [EBP + 0xffffff6c]
005bcdd5  8b10                  MOV EDX,dword ptr [EAX]
005bcdd7  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
005bcddd  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
005bcde3  ffd0                  CALL EAX
005bcde5  83f801                CMP EAX,0x1
005bcde8  744c                  JZ 0x005bce36
005bcdea  33c9                  XOR ECX,ECX
005bcdec  83f901                CMP ECX,0x1
005bcdef  7445                  JZ 0x005bce36
005bcdf1  8b956cffffff          MOV EDX,dword ptr [EBP + 0xffffff6c]
005bcdf7  83baac11000000        CMP dword ptr [EDX + 0x11ac],0x0
005bcdfe  7429                  JZ 0x005bce29
005bce00  6a00                  PUSH 0x0
005bce02  8b856cffffff          MOV EAX,dword ptr [EBP + 0xffffff6c]
005bce08  8b10                  MOV EDX,dword ptr [EAX]
005bce0a  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
005bce10  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
005bce16  ffd0                  CALL EAX
005bce18  0fbfc8                MOVSX ECX,AX
005bce1b  33d2                  XOR EDX,EDX
005bce1d  85c9                  TEST ECX,ECX
005bce1f  0f9ec2                SETLE DL
005bce22  8955ac                MOV dword ptr [EBP + -0x54],EDX
005bce25  eb09                  JMP 0x005bce30
005bce29  c745ac00000000        MOV dword ptr [EBP + -0x54],0x0
005bce30  837dac01              CMP dword ptr [EBP + -0x54],0x1
005bce34  750a                  JNZ 0x005bce40
005bce36  a1bc449900            MOV EAX,[0x009944bc]
005bce3b  e96a020000            JMP 0x005bd0aa
005bce40  6a04                  PUSH 0x4
005bce42  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005bce45  83c038                ADD EAX,0x38
005bce48  50                    PUSH EAX
005bce49  8d4de0                LEA ECX,[EBP + -0x20]
005bce4c  51                    PUSH ECX
005bce4d  e84e0d3600            CALL 0x0091dba0
005bce52  83c40c                ADD ESP,0xc
005bce55  6a04                  PUSH 0x4
005bce57  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005bce5a  83c23c                ADD EDX,0x3c
005bce5d  52                    PUSH EDX
005bce5e  8d45dc                LEA EAX,[EBP + -0x24]
005bce61  50                    PUSH EAX
005bce62  e8390d3600            CALL 0x0091dba0
005bce67  83c40c                ADD ESP,0xc
005bce6a  6a04                  PUSH 0x4
005bce6c  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005bce6f  83c140                ADD ECX,0x40
005bce72  51                    PUSH ECX
005bce73  8d55d8                LEA EDX,[EBP + -0x28]
005bce76  52                    PUSH EDX
005bce77  e8240d3600            CALL 0x0091dba0
005bce7c  83c40c                ADD ESP,0xc
005bce7f  6a04                  PUSH 0x4
005bce81  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005bce84  83c044                ADD EAX,0x44
005bce87  50                    PUSH EAX
005bce88  8d4dd0                LEA ECX,[EBP + -0x30]
005bce8b  51                    PUSH ECX
005bce8c  e80f0d3600            CALL 0x0091dba0
005bce91  83c40c                ADD ESP,0xc
005bce94  6a04                  PUSH 0x4
005bce96  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005bce99  83c248                ADD EDX,0x48
005bce9c  52                    PUSH EDX
005bce9d  8d45e4                LEA EAX,[EBP + -0x1c]
005bcea0  50                    PUSH EAX
005bcea1  e8fa0c3600            CALL 0x0091dba0
005bcea6  83c40c                ADD ESP,0xc
005bcea9  6a04                  PUSH 0x4
005bceab  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005bceae  83c14c                ADD ECX,0x4c
005bceb1  51                    PUSH ECX
005bceb2  8d55e8                LEA EDX,[EBP + -0x18]
005bceb5  52                    PUSH EDX
005bceb6  e8e50c3600            CALL 0x0091dba0
005bcebb  83c40c                ADD ESP,0xc
005bcebe  6a04                  PUSH 0x4
005bcec0  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005bcec3  83c050                ADD EAX,0x50
005bcec6  50                    PUSH EAX
005bcec7  8d4dd4                LEA ECX,[EBP + -0x2c]
005bceca  51                    PUSH ECX
005bcecb  e8d00c3600            CALL 0x0091dba0
005bced0  83c40c                ADD ESP,0xc
005bced3  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
005bced9  8b4208                MOV EAX,dword ptr [EDX + 0x8]
005bcedc  8945a8                MOV dword ptr [EBP + -0x58],EAX
005bcedf  8d4dec                LEA ECX,[EBP + -0x14]
005bcee2  51                    PUSH ECX
005bcee3  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
005bcee6  52                    PUSH EDX
005bcee7  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005bceea  e891f1f5ff            CALL 0x0051c080
005bceef  8bc8                  MOV ECX,EAX
005bcef1  e8ba10f8ff            CALL 0x0053dfb0
005bcef6  8845f3                MOV byte ptr [EBP + -0xd],AL
005bcef9  0fb645f3              MOVZX EAX,byte ptr [EBP + -0xd]
005bcefd  0fb60d7f449900        MOVZX ECX,byte ptr [0x0099447f]
005bcf04  3bc1                  CMP EAX,ECX
005bcf06  740a                  JZ 0x005bcf12
005bcf08  a1bc449900            MOV EAX,[0x009944bc]
005bcf0d  e998010000            JMP 0x005bd0aa
005bcf12  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
005bcf18  e8a384f8ff            CALL 0x005453c0
005bcf1d  85c0                  TEST EAX,EAX
005bcf1f  7430                  JZ 0x005bcf51
005bcf21  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
005bcf27  e89484f8ff            CALL 0x005453c0
005bcf2c  0520010000            ADD EAX,0x120
005bcf31  8945a4                MOV dword ptr [EBP + -0x5c],EAX
005bcf34  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
005bcf37  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005bcf3a  8945a0                MOV dword ptr [EBP + -0x60],EAX
005bcf3d  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
005bcf40  3b4da0                CMP ECX,dword ptr [EBP + -0x60]
005bcf43  740c                  JZ 0x005bcf51
005bcf45  c78568ffffff01000000  MOV dword ptr [EBP + 0xffffff68],0x1
005bcf4f  eb0a                  JMP 0x005bcf5b
005bcf51  c78568ffffff00000000  MOV dword ptr [EBP + 0xffffff68],0x0
005bcf5b  8b9568ffffff          MOV EDX,dword ptr [EBP + 0xffffff68]
005bcf61  8955cc                MOV dword ptr [EBP + -0x34],EDX
005bcf64  d945e0                FLD float ptr [EBP + -0x20]
005bcf67  d95d94                FSTP float ptr [EBP + -0x6c]
005bcf6a  d945dc                FLD float ptr [EBP + -0x24]
005bcf6d  d95d98                FSTP float ptr [EBP + -0x68]
005bcf70  d945d8                FLD float ptr [EBP + -0x28]
005bcf73  d95d9c                FSTP float ptr [EBP + -0x64]
005bcf76  d94594                FLD float ptr [EBP + -0x6c]
005bcf79  d95db0                FSTP float ptr [EBP + -0x50]
005bcf7c  d94598                FLD float ptr [EBP + -0x68]
005bcf7f  d95db4                FSTP float ptr [EBP + -0x4c]
005bcf82  d9459c                FLD float ptr [EBP + -0x64]
005bcf85  d95db8                FSTP float ptr [EBP + -0x48]
005bcf88  8d45b0                LEA EAX,[EBP + -0x50]
005bcf8b  50                    PUSH EAX
005bcf8c  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
005bcf92  81c194000000          ADD ECX,0x94
005bcf98  51                    PUSH ECX
005bcf99  8d55f4                LEA EDX,[EBP + -0xc]
005bcf9c  52                    PUSH EDX
005bcf9d  e80e83e8ff            CALL 0x004452b0
005bcfa2  83c40c                ADD ESP,0xc
005bcfa5  837dcc00              CMP dword ptr [EBP + -0x34],0x0
005bcfa9  753b                  JNZ 0x005bcfe6
005bcfab  d945d4                FLD float ptr [EBP + -0x2c]
005bcfae  dc0538759800          FADD double ptr [0x00987538]
005bcfb4  d95d84                FSTP float ptr [EBP + -0x7c]
005bcfb7  8d45f4                LEA EAX,[EBP + -0xc]
005bcfba  50                    PUSH EAX
005bcfbb  8d4df4                LEA ECX,[EBP + -0xc]
005bcfbe  51                    PUSH ECX
005bcfbf  e8dce8e8ff            CALL 0x0044b8a0
005bcfc4  83c408                ADD ESP,0x8
005bcfc7  d94584                FLD float ptr [EBP + -0x7c]
005bcfca  d84d84                FMUL float ptr [EBP + -0x7c]
005bcfcd  d99d64ffffff          FSTP float ptr [EBP + 0xffffff64]
005bcfd3  d98564ffffff          FLD float ptr [EBP + 0xffffff64]
005bcfd9  ded9                  FCOMPP
005bcfdb  dfe0                  FNSTSW AX
005bcfdd  f6c405                TEST AH,0x5
005bcfe0  0f8abf000000          JP 0x005bd0a5
005bcfe6  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
005bcfec  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005bcfef  8945c0                MOV dword ptr [EBP + -0x40],EAX
005bcff2  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
005bcff8  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005bcffb  895580                MOV dword ptr [EBP + -0x80],EDX
005bcffe  8b4580                MOV EAX,dword ptr [EBP + -0x80]
005bd001  50                    PUSH EAX
005bd002  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
005bd005  e8b62e1800            CALL 0x0073fec0
005bd00a  8945c8                MOV dword ptr [EBP + -0x38],EAX
005bd00d  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
005bd013  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005bd016  89957cffffff          MOV dword ptr [EBP + 0xffffff7c],EDX
005bd01c  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
005bd01f  50                    PUSH EAX
005bd020  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
005bd026  e8652b1800            CALL 0x0073fb90
005bd02b  8bc8                  MOV ECX,EAX
005bd02d  e8ae9b2200            CALL 0x007e6be0
005bd032  8945c4                MOV dword ptr [EBP + -0x3c],EAX
005bd035  837dc400              CMP dword ptr [EBP + -0x3c],0x0
005bd039  7c6a                  JL 0x005bd0a5
005bd03b  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
005bd041  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005bd044  899578ffffff          MOV dword ptr [EBP + 0xffffff78],EDX
005bd04a  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
005bd050  e83b2b1800            CALL 0x0073fb90
005bd055  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
005bd05b  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
005bd05e  69c088000000          IMUL EAX,EAX,0x88
005bd064  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
005bd06a  8d540124              LEA EDX,[ECX + EAX*0x1 + 0x24]
005bd06e  8955bc                MOV dword ptr [EBP + -0x44],EDX
005bd071  833dc4e0a70000        CMP dword ptr [0x00a7e0c4],0x0
005bd078  7421                  JZ 0x005bd09b
005bd07a  8d45f4                LEA EAX,[EBP + -0xc]
005bd07d  50                    PUSH EAX
005bd07e  8d4df4                LEA ECX,[EBP + -0xc]
005bd081  51                    PUSH ECX
005bd082  e819e8e8ff            CALL 0x0044b8a0
005bd087  ddd8                  FSTP ST0
005bd089  83c408                ADD ESP,0x8
005bd08c  d945d4                FLD float ptr [EBP + -0x2c]
005bd08f  dc0538759800          FADD double ptr [0x00987538]
005bd095  d99d70ffffff          FSTP float ptr [EBP + 0xffffff70]
005bd09b  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
005bd09e  c7420405000000        MOV dword ptr [EDX + 0x4],0x5
005bd0a5  a1b8449900            MOV EAX,[0x009944b8]
005bd0aa  8be5                  MOV ESP,EBP
005bd0ac  5d                    POP EBP
005bd0ad  c20400                RET 0x4
