; FN 0069ac60 FUN_0069ac60
0069ac60  55                    PUSH EBP
0069ac61  8bec                  MOV EBP,ESP
0069ac63  83ec48                SUB ESP,0x48
0069ac66  894db8                MOV dword ptr [EBP + -0x48],ECX
0069ac69  d9ee                  FLDZ
0069ac6b  d95dcc                FSTP float ptr [EBP + -0x34]
0069ac6e  d9ee                  FLDZ
0069ac70  d95dd0                FSTP float ptr [EBP + -0x30]
0069ac73  d9ee                  FLDZ
0069ac75  d95dd4                FSTP float ptr [EBP + -0x2c]
0069ac78  d9ee                  FLDZ
0069ac7a  d95de4                FSTP float ptr [EBP + -0x1c]
0069ac7d  d9ee                  FLDZ
0069ac7f  d95de8                FSTP float ptr [EBP + -0x18]
0069ac82  d9ee                  FLDZ
0069ac84  d95dec                FSTP float ptr [EBP + -0x14]
0069ac87  c745c800000000        MOV dword ptr [EBP + -0x38],0x0
0069ac8e  8d45cc                LEA EAX,[EBP + -0x34]
0069ac91  50                    PUSH EAX
0069ac92  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069ac98  e8832d0600            CALL 0x006fda20
0069ac9d  85c0                  TEST EAX,EAX
0069ac9f  750a                  JNZ 0x0069acab
0069aca1  b82ff8ffff            MOV EAX,0xfffff82f
0069aca6  e905020000            JMP 0x0069aeb0
0069acab  8d4de4                LEA ECX,[EBP + -0x1c]
0069acae  51                    PUSH ECX
0069acaf  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069acb5  e8662d0600            CALL 0x006fda20
0069acba  85c0                  TEST EAX,EAX
0069acbc  750a                  JNZ 0x0069acc8
0069acbe  b82ff8ffff            MOV EAX,0xfffff82f
0069acc3  e9e8010000            JMP 0x0069aeb0
0069acc8  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
0069accb  837a0400              CMP dword ptr [EDX + 0x4],0x0
0069accf  7524                  JNZ 0x0069acf5
0069acd1  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
0069acd4  50                    PUSH EAX
0069acd5  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069acdb  e8e02c0600            CALL 0x006fd9c0
0069ace0  85c0                  TEST EAX,EAX
0069ace2  750a                  JNZ 0x0069acee
0069ace4  b830f8ffff            MOV EAX,0xfffff830
0069ace9  e9c2010000            JMP 0x0069aeb0
0069acee  33c0                  XOR EAX,EAX
0069acf0  e9bb010000            JMP 0x0069aeb0
0069acf5  8d4ddc                LEA ECX,[EBP + -0x24]
0069acf8  51                    PUSH ECX
0069acf9  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069acff  e8ec2d0600            CALL 0x006fdaf0
0069ad04  85c0                  TEST EAX,EAX
0069ad06  750a                  JNZ 0x0069ad12
0069ad08  b82ff8ffff            MOV EAX,0xfffff82f
0069ad0d  e99e010000            JMP 0x0069aeb0
0069ad12  8d55c4                LEA EDX,[EBP + -0x3c]
0069ad15  52                    PUSH EDX
0069ad16  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069ad1c  e8cf2d0600            CALL 0x006fdaf0
0069ad21  85c0                  TEST EAX,EAX
0069ad23  750a                  JNZ 0x0069ad2f
0069ad25  b82ff8ffff            MOV EAX,0xfffff82f
0069ad2a  e981010000            JMP 0x0069aeb0
0069ad2f  c745c000000000        MOV dword ptr [EBP + -0x40],0x0
0069ad36  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0069ad3b  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0069ad3e  894dbc                MOV dword ptr [EBP + -0x44],ECX
0069ad41  8d55c0                LEA EDX,[EBP + -0x40]
0069ad44  52                    PUSH EDX
0069ad45  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
0069ad48  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0069ad4b  51                    PUSH ECX
0069ad4c  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0069ad4f  e82c13e8ff            CALL 0x0051c080
0069ad54  8bc8                  MOV ECX,EAX
0069ad56  e85532eaff            CALL 0x0053dfb0
0069ad5b  8845f3                MOV byte ptr [EBP + -0xd],AL
0069ad5e  0fb655f3              MOVZX EDX,byte ptr [EBP + -0xd]
0069ad62  0fb6057f449900        MOVZX EAX,byte ptr [0x0099447f]
0069ad69  3bd0                  CMP EDX,EAX
0069ad6b  7424                  JZ 0x0069ad91
0069ad6d  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
0069ad70  51                    PUSH ECX
0069ad71  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069ad77  e8442c0600            CALL 0x006fd9c0
0069ad7c  85c0                  TEST EAX,EAX
0069ad7e  750a                  JNZ 0x0069ad8a
0069ad80  b830f8ffff            MOV EAX,0xfffff830
0069ad85  e926010000            JMP 0x0069aeb0
0069ad8a  33c0                  XOR EAX,EAX
0069ad8c  e91f010000            JMP 0x0069aeb0
0069ad91  c745d800000000        MOV dword ptr [EBP + -0x28],0x0
0069ad98  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
0069ad9b  8b02                  MOV EAX,dword ptr [EDX]
0069ad9d  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069ada0  8b5028                MOV EDX,dword ptr [EAX + 0x28]
0069ada3  ffd2                  CALL EDX
0069ada5  85c0                  TEST EAX,EAX
0069ada7  7412                  JZ 0x0069adbb
0069ada9  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
0069adac  8b10                  MOV EDX,dword ptr [EAX]
0069adae  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069adb1  8b4228                MOV EAX,dword ptr [EDX + 0x28]
0069adb4  ffd0                  CALL EAX
0069adb6  8945d8                MOV dword ptr [EBP + -0x28],EAX
0069adb9  eb7c                  JMP 0x0069ae37
0069adbb  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069adbe  8b11                  MOV EDX,dword ptr [ECX]
0069adc0  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069adc3  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0069adc6  ffd0                  CALL EAX
0069adc8  85c0                  TEST EAX,EAX
0069adca  7419                  JZ 0x0069ade5
0069adcc  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069adcf  8b11                  MOV EDX,dword ptr [ECX]
0069add1  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069add4  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0069add7  ffd0                  CALL EAX
0069add9  8bc8                  MOV ECX,EAX
0069addb  e8e0a5eaff            CALL 0x005453c0
0069ade0  8945d8                MOV dword ptr [EBP + -0x28],EAX
0069ade3  eb52                  JMP 0x0069ae37
0069ade5  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069ade8  8b11                  MOV EDX,dword ptr [ECX]
0069adea  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069aded  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0069adf0  ffd0                  CALL EAX
0069adf2  85c0                  TEST EAX,EAX
0069adf4  7419                  JZ 0x0069ae0f
0069adf6  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069adf9  8b11                  MOV EDX,dword ptr [ECX]
0069adfb  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069adfe  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0069ae01  ffd0                  CALL EAX
0069ae03  8bc8                  MOV ECX,EAX
0069ae05  e8b6a5eaff            CALL 0x005453c0
0069ae0a  8945d8                MOV dword ptr [EBP + -0x28],EAX
0069ae0d  eb28                  JMP 0x0069ae37
0069ae0f  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069ae12  8b11                  MOV EDX,dword ptr [ECX]
0069ae14  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069ae17  8b4248                MOV EAX,dword ptr [EDX + 0x48]
0069ae1a  ffd0                  CALL EAX
0069ae1c  85c0                  TEST EAX,EAX
0069ae1e  7417                  JZ 0x0069ae37
0069ae20  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069ae23  8b11                  MOV EDX,dword ptr [ECX]
0069ae25  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0069ae28  8b4248                MOV EAX,dword ptr [EDX + 0x48]
0069ae2b  ffd0                  CALL EAX
0069ae2d  8bc8                  MOV ECX,EAX
0069ae2f  e88ca5eaff            CALL 0x005453c0
0069ae34  8945d8                MOV dword ptr [EBP + -0x28],EAX
0069ae37  837dd800              CMP dword ptr [EBP + -0x28],0x0
0069ae3b  751a                  JNZ 0x0069ae57
0069ae3d  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
0069ae40  51                    PUSH ECX
0069ae41  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069ae47  e8742b0600            CALL 0x006fd9c0
0069ae4c  85c0                  TEST EAX,EAX
0069ae4e  7507                  JNZ 0x0069ae57
0069ae50  b830f8ffff            MOV EAX,0xfffff830
0069ae55  eb59                  JMP 0x0069aeb0
0069ae57  c745e00000007f        MOV dword ptr [EBP + -0x20],0x7f000000
0069ae5e  d9ee                  FLDZ
0069ae60  d95df4                FSTP float ptr [EBP + -0xc]
0069ae63  d9ee                  FLDZ
0069ae65  d95df8                FSTP float ptr [EBP + -0x8]
0069ae68  d9ee                  FLDZ
0069ae6a  d95dfc                FSTP float ptr [EBP + -0x4]
0069ae6d  6a00                  PUSH 0x0
0069ae6f  6a00                  PUSH 0x0
0069ae71  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0069ae74  52                    PUSH EDX
0069ae75  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0069ae78  50                    PUSH EAX
0069ae79  8d4de0                LEA ECX,[EBP + -0x20]
0069ae7c  51                    PUSH ECX
0069ae7d  8d55f4                LEA EDX,[EBP + -0xc]
0069ae80  52                    PUSH EDX
0069ae81  8d45e4                LEA EAX,[EBP + -0x1c]
0069ae84  50                    PUSH EAX
0069ae85  8d4dcc                LEA ECX,[EBP + -0x34]
0069ae88  51                    PUSH ECX
0069ae89  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0069ae8c  e8cf0ce9ff            CALL 0x0052bb60
0069ae91  8945c8                MOV dword ptr [EBP + -0x38],EAX
0069ae94  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
0069ae97  52                    PUSH EDX
0069ae98  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
0069ae9e  e81d2b0600            CALL 0x006fd9c0
0069aea3  85c0                  TEST EAX,EAX
0069aea5  7507                  JNZ 0x0069aeae
0069aea7  b830f8ffff            MOV EAX,0xfffff830
0069aeac  eb02                  JMP 0x0069aeb0
0069aeae  33c0                  XOR EAX,EAX
0069aeb0  8be5                  MOV ESP,EBP
0069aeb2  5d                    POP EBP
0069aeb3  c20800                RET 0x8
