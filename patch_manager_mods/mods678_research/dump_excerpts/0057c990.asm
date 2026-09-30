; FN 0057c990 FUN_0057c990
0057c990  55                    PUSH EBP
0057c991  8bec                  MOV EBP,ESP
0057c993  81ec70010000          SUB ESP,0x170
0057c999  898da0feffff          MOV dword ptr [EBP + 0xfffffea0],ECX
0057c99f  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057c9a5  e8168afcff            CALL 0x005453c0
0057c9aa  8945c4                MOV dword ptr [EBP + -0x3c],EAX
0057c9ad  837dc400              CMP dword ptr [EBP + -0x3c],0x0
0057c9b1  750a                  JNZ 0x0057c9bd
0057c9b3  b80000007f            MOV EAX,0x7f000000
0057c9b8  e9ad090000            JMP 0x0057d36a
0057c9bd  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
0057c9c0  0594010000            ADD EAX,0x194
0057c9c5  8945d8                MOV dword ptr [EBP + -0x28],EAX
0057c9c8  c745bc00000000        MOV dword ptr [EBP + -0x44],0x0
0057c9cf  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0057c9d2  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0057c9d5  899548ffffff          MOV dword ptr [EBP + 0xffffff48],EDX
0057c9db  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0057c9de  3b8548ffffff          CMP EAX,dword ptr [EBP + 0xffffff48]
0057c9e4  7d3b                  JGE 0x0057ca21
0057c9e6  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0057c9e9  8b11                  MOV EDX,dword ptr [ECX]
0057c9eb  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0057c9ee  8d0c82                LEA ECX,[EDX + EAX*0x4]
0057c9f1  898d44ffffff          MOV dword ptr [EBP + 0xffffff44],ECX
0057c9f7  8b95a0feffff          MOV EDX,dword ptr [EBP + 0xfffffea0]
0057c9fd  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0057ca00  898540ffffff          MOV dword ptr [EBP + 0xffffff40],EAX
0057ca06  8b8d44ffffff          MOV ECX,dword ptr [EBP + 0xffffff44]
0057ca0c  8b11                  MOV EDX,dword ptr [ECX]
0057ca0e  3b9540ffffff          CMP EDX,dword ptr [EBP + 0xffffff40]
0057ca14  740b                  JZ 0x0057ca21
0057ca16  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0057ca19  83c001                ADD EAX,0x1
0057ca1c  8945bc                MOV dword ptr [EBP + -0x44],EAX
0057ca1f  ebae                  JMP 0x0057c9cf
0057ca21  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0057ca24  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0057ca27  89953cffffff          MOV dword ptr [EBP + 0xffffff3c],EDX
0057ca2d  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0057ca30  3b853cffffff          CMP EAX,dword ptr [EBP + 0xffffff3c]
0057ca36  7c0a                  JL 0x0057ca42
0057ca38  b80000007f            MOV EAX,0x7f000000
0057ca3d  e928090000            JMP 0x0057d36a
0057ca42  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
0057ca49  d94508                FLD float ptr [EBP + 0x8]
0057ca4c  d95de4                FSTP float ptr [EBP + -0x1c]
0057ca4f  c745e80000007f        MOV dword ptr [EBP + -0x18],0x7f000000
0057ca56  c745c001000000        MOV dword ptr [EBP + -0x40],0x1
0057ca5d  c745f801000000        MOV dword ptr [EBP + -0x8],0x1
0057ca64  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0057ca6a  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0057ca6d  8955c8                MOV dword ptr [EBP + -0x38],EDX
0057ca70  d9ee                  FLDZ
0057ca72  d95dec                FSTP float ptr [EBP + -0x14]
0057ca75  d9ee                  FLDZ
0057ca77  d95df0                FSTP float ptr [EBP + -0x10]
0057ca7a  d9ee                  FLDZ
0057ca7c  d95df4                FSTP float ptr [EBP + -0xc]
0057ca7f  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0057ca82  2b45cc                SUB EAX,dword ptr [EBP + -0x34]
0057ca85  791e                  JNS 0x0057caa5
0057ca87  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0057ca8a  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0057ca8d  899538ffffff          MOV dword ptr [EBP + 0xffffff38],EDX
0057ca93  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0057ca96  0345cc                ADD EAX,dword ptr [EBP + -0x34]
0057ca99  3b8538ffffff          CMP EAX,dword ptr [EBP + 0xffffff38]
0057ca9f  0f8dc2080000          JGE 0x0057d367
0057caa5  837dc000              CMP dword ptr [EBP + -0x40],0x0
0057caa9  0f8435040000          JZ 0x0057cee4
0057caaf  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0057cab2  2b4dcc                SUB ECX,dword ptr [EBP + -0x34]
0057cab5  0f8822040000          JS 0x0057cedd
0057cabb  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
0057cabe  2b55cc                SUB EDX,dword ptr [EBP + -0x34]
0057cac1  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
0057cac4  8b08                  MOV ECX,dword ptr [EAX]
0057cac6  8b1491                MOV EDX,dword ptr [ECX + EDX*0x4]
0057cac9  8955d0                MOV dword ptr [EBP + -0x30],EDX
0057cacc  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057cad1  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057cad4  898d34ffffff          MOV dword ptr [EBP + 0xffffff34],ECX
0057cada  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
0057cadd  52                    PUSH EDX
0057cade  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
0057cae4  e8b7f5f9ff            CALL 0x0051c0a0
0057cae9  8945dc                MOV dword ptr [EBP + -0x24],EAX
0057caec  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0057caf0  0f84e5030000          JZ 0x0057cedb
0057caf6  8b85a0feffff          MOV EAX,dword ptr [EBP + 0xfffffea0]
0057cafc  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0057caff  898d30ffffff          MOV dword ptr [EBP + 0xffffff30],ECX
0057cb05  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
0057cb08  3b9530ffffff          CMP EDX,dword ptr [EBP + 0xffffff30]
0057cb0e  0f84c7030000          JZ 0x0057cedb
0057cb14  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0057cb17  3b450c                CMP EAX,dword ptr [EBP + 0xc]
0057cb1a  0f84bb030000          JZ 0x0057cedb
0057cb20  c745b800000000        MOV dword ptr [EBP + -0x48],0x0
0057cb27  817d100000007f        CMP dword ptr [EBP + 0x10],0x7f000000
0057cb2e  7452                  JZ 0x0057cb82
0057cb30  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cb33  8b11                  MOV EDX,dword ptr [ECX]
0057cb35  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cb38  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057cb3b  ffd0                  CALL EAX
0057cb3d  85c0                  TEST EAX,EAX
0057cb3f  742c                  JZ 0x0057cb6d
0057cb41  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
0057cb44  51                    PUSH ECX
0057cb45  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057cb48  8b02                  MOV EAX,dword ptr [EDX]
0057cb4a  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cb4d  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0057cb50  ffd2                  CALL EDX
0057cb52  8bc8                  MOV ECX,EAX
0057cb54  e8274b0600            CALL 0x005e1680
0057cb59  0fb6c0                MOVZX EAX,AL
0057cb5c  83f802                CMP EAX,0x2
0057cb5f  750c                  JNZ 0x0057cb6d
0057cb61  c7859cfeffff01000000  MOV dword ptr [EBP + 0xfffffe9c],0x1
0057cb6b  eb0a                  JMP 0x0057cb77
0057cb6d  c7859cfeffff00000000  MOV dword ptr [EBP + 0xfffffe9c],0x0
0057cb77  8b8d9cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe9c]
0057cb7d  894db8                MOV dword ptr [EBP + -0x48],ECX
0057cb80  eb1d                  JMP 0x0057cb9f
0057cb82  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
0057cb85  52                    PUSH EDX
0057cb86  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057cb8c  e8ef4a0600            CALL 0x005e1680
0057cb91  0fb6c0                MOVZX EAX,AL
0057cb94  33c9                  XOR ECX,ECX
0057cb96  83f802                CMP EAX,0x2
0057cb99  0f94c1                SETZ CL
0057cb9c  894db8                MOV dword ptr [EBP + -0x48],ECX
0057cb9f  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057cba2  8b02                  MOV EAX,dword ptr [EDX]
0057cba4  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cba7  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0057cbaa  ffd2                  CALL EDX
0057cbac  85c0                  TEST EAX,EAX
0057cbae  0f84d8020000          JZ 0x0057ce8c
0057cbb4  837db800              CMP dword ptr [EBP + -0x48],0x0
0057cbb8  0f84ce020000          JZ 0x0057ce8c
0057cbbe  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0057cbc1  8b10                  MOV EDX,dword ptr [EAX]
0057cbc3  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cbc6  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057cbc9  ffd0                  CALL EAX
0057cbcb  898598feffff          MOV dword ptr [EBP + 0xfffffe98],EAX
0057cbd1  8b8d98feffff          MOV ECX,dword ptr [EBP + 0xfffffe98]
0057cbd7  8b11                  MOV EDX,dword ptr [ECX]
0057cbd9  8b8d98feffff          MOV ECX,dword ptr [EBP + 0xfffffe98]
0057cbdf  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
0057cbe5  ffd0                  CALL EAX
0057cbe7  85c0                  TEST EAX,EAX
0057cbe9  0f859d020000          JNZ 0x0057ce8c
0057cbef  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cbf2  8b11                  MOV EDX,dword ptr [ECX]
0057cbf4  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cbf7  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057cbfa  ffd0                  CALL EAX
0057cbfc  33c9                  XOR ECX,ECX
0057cbfe  0f8588020000          JNZ 0x0057ce8c
0057cc04  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057cc07  8b02                  MOV EAX,dword ptr [EDX]
0057cc09  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cc0c  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0057cc0f  ffd2                  CALL EDX
0057cc11  89852cffffff          MOV dword ptr [EBP + 0xffffff2c],EAX
0057cc17  8b852cffffff          MOV EAX,dword ptr [EBP + 0xffffff2c]
0057cc1d  83b8ac11000000        CMP dword ptr [EAX + 0x11ac],0x0
0057cc24  742c                  JZ 0x0057cc52
0057cc26  6a00                  PUSH 0x0
0057cc28  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
0057cc2e  8b11                  MOV EDX,dword ptr [ECX]
0057cc30  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
0057cc36  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
0057cc3c  ffd0                  CALL EAX
0057cc3e  0fbfc8                MOVSX ECX,AX
0057cc41  33d2                  XOR EDX,EDX
0057cc43  85c9                  TEST ECX,ECX
0057cc45  0f9ec2                SETLE DL
0057cc48  899528ffffff          MOV dword ptr [EBP + 0xffffff28],EDX
0057cc4e  eb0c                  JMP 0x0057cc5c
0057cc52  c78528ffffff00000000  MOV dword ptr [EBP + 0xffffff28],0x0
0057cc5c  83bd28ffffff00        CMP dword ptr [EBP + 0xffffff28],0x0
0057cc63  0f8523020000          JNZ 0x0057ce8c
0057cc69  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0057cc6c  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0057cc6f  898d24ffffff          MOV dword ptr [EBP + 0xffffff24],ECX
0057cc75  8b9524ffffff          MOV EDX,dword ptr [EBP + 0xffffff24]
0057cc7b  52                    PUSH EDX
0057cc7c  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057cc82  e8c94a0500            CALL 0x005d1750
0057cc87  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0057cc8a  837dd400              CMP dword ptr [EBP + -0x2c],0x0
0057cc8e  0f84f8010000          JZ 0x0057ce8c
0057cc94  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
0057cc97  8a4804                MOV CL,byte ptr [EAX + 0x4]
0057cc9a  80e101                AND CL,0x1
0057cc9d  0fb6d1                MOVZX EDX,CL
0057cca0  85d2                  TEST EDX,EDX
0057cca2  0f84e4010000          JZ 0x0057ce8c
0057cca8  d9ee                  FLDZ
0057ccaa  d95dac                FSTP float ptr [EBP + -0x54]
0057ccad  d9ee                  FLDZ
0057ccaf  d95db0                FSTP float ptr [EBP + -0x50]
0057ccb2  d9ee                  FLDZ
0057ccb4  d95db4                FSTP float ptr [EBP + -0x4c]
0057ccb7  817d100000007f        CMP dword ptr [EBP + 0x10],0x7f000000
0057ccbe  7441                  JZ 0x0057cd01
0057ccc0  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057ccc5  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057ccc8  898d20ffffff          MOV dword ptr [EBP + 0xffffff20],ECX
0057ccce  8b5510                MOV EDX,dword ptr [EBP + 0x10]
0057ccd1  52                    PUSH EDX
0057ccd2  8b8d20ffffff          MOV ECX,dword ptr [EBP + 0xffffff20]
0057ccd8  e823f4f9ff            CALL 0x0051c100
0057ccdd  8945a8                MOV dword ptr [EBP + -0x58],EAX
0057cce0  837da800              CMP dword ptr [EBP + -0x58],0x0
0057cce4  7419                  JZ 0x0057ccff
0057cce6  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
0057cce9  0594000000            ADD EAX,0x94
0057ccee  8b08                  MOV ECX,dword ptr [EAX]
0057ccf0  894dac                MOV dword ptr [EBP + -0x54],ECX
0057ccf3  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0057ccf6  8955b0                MOV dword ptr [EBP + -0x50],EDX
0057ccf9  8b4008                MOV EAX,dword ptr [EAX + 0x8]
0057ccfc  8945b4                MOV dword ptr [EBP + -0x4c],EAX
0057ccff  eb1d                  JMP 0x0057cd1e
0057cd01  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057cd07  81c194000000          ADD ECX,0x94
0057cd0d  8b11                  MOV EDX,dword ptr [ECX]
0057cd0f  8955ac                MOV dword ptr [EBP + -0x54],EDX
0057cd12  8b4104                MOV EAX,dword ptr [ECX + 0x4]
0057cd15  8945b0                MOV dword ptr [EBP + -0x50],EAX
0057cd18  8b4908                MOV ECX,dword ptr [ECX + 0x8]
0057cd1b  894db4                MOV dword ptr [EBP + -0x4c],ECX
0057cd1e  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057cd21  8b02                  MOV EAX,dword ptr [EDX]
0057cd23  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cd26  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0057cd29  ffd2                  CALL EDX
0057cd2b  89851cffffff          MOV dword ptr [EBP + 0xffffff1c],EAX
0057cd31  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
0057cd37  0594000000            ADD EAX,0x94
0057cd3c  50                    PUSH EAX
0057cd3d  8d4dac                LEA ECX,[EBP + -0x54]
0057cd40  51                    PUSH ECX
0057cd41  8d9558ffffff          LEA EDX,[EBP + 0xffffff58]
0057cd47  52                    PUSH EDX
0057cd48  e86385ecff            CALL 0x004452b0
0057cd4d  83c40c                ADD ESP,0xc
0057cd50  50                    PUSH EAX
0057cd51  e89a75f9ff            CALL 0x005142f0
0057cd56  83c404                ADD ESP,0x4
0057cd59  d95de0                FSTP float ptr [EBP + -0x20]
0057cd5c  8b85a0feffff          MOV EAX,dword ptr [EBP + 0xfffffea0]
0057cd62  8b8880030000          MOV ECX,dword ptr [EAX + 0x380]
0057cd68  898d0cffffff          MOV dword ptr [EBP + 0xffffff0c],ECX
0057cd6e  8b950cffffff          MOV EDX,dword ptr [EBP + 0xffffff0c]
0057cd74  d94208                FLD float ptr [EDX + 0x8]
0057cd77  d99d08ffffff          FSTP float ptr [EBP + 0xffffff08]
0057cd7d  d945e0                FLD float ptr [EBP + -0x20]
0057cd80  d8a508ffffff          FSUB float ptr [EBP + 0xffffff08]
0057cd86  d95de0                FSTP float ptr [EBP + -0x20]
0057cd89  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0057cd8c  8b10                  MOV EDX,dword ptr [EAX]
0057cd8e  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cd91  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057cd94  ffd0                  CALL EAX
0057cd96  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
0057cd9c  8b8d04ffffff          MOV ECX,dword ptr [EBP + 0xffffff04]
0057cda2  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
0057cda8  899500ffffff          MOV dword ptr [EBP + 0xffffff00],EDX
0057cdae  8b8500ffffff          MOV EAX,dword ptr [EBP + 0xffffff00]
0057cdb4  d94008                FLD float ptr [EAX + 0x8]
0057cdb7  d99dfcfeffff          FSTP float ptr [EBP + 0xfffffefc]
0057cdbd  d945e0                FLD float ptr [EBP + -0x20]
0057cdc0  d8a5fcfeffff          FSUB float ptr [EBP + 0xfffffefc]
0057cdc6  d95de0                FSTP float ptr [EBP + -0x20]
0057cdc9  d945e0                FLD float ptr [EBP + -0x20]
0057cdcc  d945e4                FLD float ptr [EBP + -0x1c]
0057cdcf  ded9                  FCOMPP
0057cdd1  dfe0                  FNSTSW AX
0057cdd3  f6c441                TEST AH,0x41
0057cdd6  0f85b0000000          JNZ 0x0057ce8c
0057cddc  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057cde2  81c194000000          ADD ECX,0x94
0057cde8  8b11                  MOV EDX,dword ptr [ECX]
0057cdea  89559c                MOV dword ptr [EBP + -0x64],EDX
0057cded  8b4104                MOV EAX,dword ptr [ECX + 0x4]
0057cdf0  8945a0                MOV dword ptr [EBP + -0x60],EAX
0057cdf3  8b4908                MOV ECX,dword ptr [ECX + 0x8]
0057cdf6  894da4                MOV dword ptr [EBP + -0x5c],ECX
0057cdf9  d945a4                FLD float ptr [EBP + -0x5c]
0057cdfc  dc05109f9800          FADD double ptr [0x00989f10]
0057ce02  d95da4                FSTP float ptr [EBP + -0x5c]
0057ce05  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057ce08  8b02                  MOV EAX,dword ptr [EDX]
0057ce0a  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057ce0d  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0057ce10  ffd2                  CALL EDX
0057ce12  8985f8feffff          MOV dword ptr [EBP + 0xfffffef8],EAX
0057ce18  8b85f8feffff          MOV EAX,dword ptr [EBP + 0xfffffef8]
0057ce1e  0594000000            ADD EAX,0x94
0057ce23  8b08                  MOV ECX,dword ptr [EAX]
0057ce25  894d90                MOV dword ptr [EBP + -0x70],ECX
0057ce28  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0057ce2b  895594                MOV dword ptr [EBP + -0x6c],EDX
0057ce2e  8b4008                MOV EAX,dword ptr [EAX + 0x8]
0057ce31  894598                MOV dword ptr [EBP + -0x68],EAX
0057ce34  d94598                FLD float ptr [EBP + -0x68]
0057ce37  dc05109f9800          FADD double ptr [0x00989f10]
0057ce3d  d95d98                FSTP float ptr [EBP + -0x68]
0057ce40  837dc400              CMP dword ptr [EBP + -0x3c],0x0
0057ce44  7446                  JZ 0x0057ce8c
0057ce46  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057ce4c  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0057ce4f  8995f4feffff          MOV dword ptr [EBP + 0xfffffef4],EDX
0057ce55  6a00                  PUSH 0x0
0057ce57  6a01                  PUSH 0x1
0057ce59  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0057ce5c  50                    PUSH EAX
0057ce5d  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
0057ce63  51                    PUSH ECX
0057ce64  8d55fc                LEA EDX,[EBP + -0x4]
0057ce67  52                    PUSH EDX
0057ce68  8d45ec                LEA EAX,[EBP + -0x14]
0057ce6b  50                    PUSH EAX
0057ce6c  8d4d90                LEA ECX,[EBP + -0x70]
0057ce6f  51                    PUSH ECX
0057ce70  8d559c                LEA EDX,[EBP + -0x64]
0057ce73  52                    PUSH EDX
0057ce74  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057ce77  e8e4ecfaff            CALL 0x0052bb60
0057ce7c  85c0                  TEST EAX,EAX
0057ce7e  740c                  JZ 0x0057ce8c
0057ce80  d945e0                FLD float ptr [EBP + -0x20]
0057ce83  d95de4                FSTP float ptr [EBP + -0x1c]
0057ce86  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0057ce89  8945e8                MOV dword ptr [EBP + -0x18],EAX
0057ce8c  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057ce8f  8b11                  MOV EDX,dword ptr [ECX]
0057ce91  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057ce94  8b4210                MOV EAX,dword ptr [EDX + 0x10]
0057ce97  ffd0                  CALL EAX
0057ce99  85c0                  TEST EAX,EAX
0057ce9b  743e                  JZ 0x0057cedb
0057ce9d  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cea0  8b11                  MOV EDX,dword ptr [ECX]
0057cea2  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cea5  8b4210                MOV EAX,dword ptr [EDX + 0x10]
0057cea8  ffd0                  CALL EAX
0057ceaa  8985f0feffff          MOV dword ptr [EBP + 0xfffffef0],EAX
0057ceb0  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057ceb6  d98194000000          FLD float ptr [ECX + 0x94]
0057cebc  8b95f0feffff          MOV EDX,dword ptr [EBP + 0xfffffef0]
0057cec2  d8a294000000          FSUB float ptr [EDX + 0x94]
0057cec8  d945e4                FLD float ptr [EBP + -0x1c]
0057cecb  ded9                  FCOMPP
0057cecd  dfe0                  FNSTSW AX
0057cecf  f6c405                TEST AH,0x5
0057ced2  7a07                  JP 0x0057cedb
0057ced4  c745c000000000        MOV dword ptr [EBP + -0x40],0x0
0057cedb  eb07                  JMP 0x0057cee4
0057cedd  c745c000000000        MOV dword ptr [EBP + -0x40],0x0
0057cee4  837df800              CMP dword ptr [EBP + -0x8],0x0
0057cee8  0f846b040000          JZ 0x0057d359
0057ceee  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
0057cef1  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0057cef4  898decfeffff          MOV dword ptr [EBP + 0xfffffeec],ECX
0057cefa  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
0057cefd  0355cc                ADD EDX,dword ptr [EBP + -0x34]
0057cf00  3b95ecfeffff          CMP EDX,dword ptr [EBP + 0xfffffeec]
0057cf06  0f8d46040000          JGE 0x0057d352
0057cf0c  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0057cf0f  0345cc                ADD EAX,dword ptr [EBP + -0x34]
0057cf12  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
0057cf15  8b11                  MOV EDX,dword ptr [ECX]
0057cf17  8b0482                MOV EAX,dword ptr [EDX + EAX*0x4]
0057cf1a  8945d0                MOV dword ptr [EBP + -0x30],EAX
0057cf1d  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0057cf23  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0057cf26  8995e8feffff          MOV dword ptr [EBP + 0xfffffee8],EDX
0057cf2c  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0057cf2f  50                    PUSH EAX
0057cf30  8b8de8feffff          MOV ECX,dword ptr [EBP + 0xfffffee8]
0057cf36  e865f1f9ff            CALL 0x0051c0a0
0057cf3b  8945dc                MOV dword ptr [EBP + -0x24],EAX
0057cf3e  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0057cf42  0f8408040000          JZ 0x0057d350
0057cf48  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057cf4e  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0057cf51  8995e4feffff          MOV dword ptr [EBP + 0xfffffee4],EDX
0057cf57  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0057cf5a  3b85e4feffff          CMP EAX,dword ptr [EBP + 0xfffffee4]
0057cf60  0f84ea030000          JZ 0x0057d350
0057cf66  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057cf69  3b4d0c                CMP ECX,dword ptr [EBP + 0xc]
0057cf6c  0f84de030000          JZ 0x0057d350
0057cf72  c7458c00000000        MOV dword ptr [EBP + -0x74],0x0
0057cf79  817d100000007f        CMP dword ptr [EBP + 0x10],0x7f000000
0057cf80  7452                  JZ 0x0057cfd4
0057cf82  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057cf85  8b02                  MOV EAX,dword ptr [EDX]
0057cf87  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cf8a  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0057cf8d  ffd2                  CALL EDX
0057cf8f  85c0                  TEST EAX,EAX
0057cf91  742c                  JZ 0x0057cfbf
0057cf93  8b4510                MOV EAX,dword ptr [EBP + 0x10]
0057cf96  50                    PUSH EAX
0057cf97  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cf9a  8b11                  MOV EDX,dword ptr [ECX]
0057cf9c  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cf9f  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057cfa2  ffd0                  CALL EAX
0057cfa4  8bc8                  MOV ECX,EAX
0057cfa6  e8d5460600            CALL 0x005e1680
0057cfab  0fb6c8                MOVZX ECX,AL
0057cfae  83f902                CMP ECX,0x2
0057cfb1  750c                  JNZ 0x0057cfbf
0057cfb3  c78594feffff01000000  MOV dword ptr [EBP + 0xfffffe94],0x1
0057cfbd  eb0a                  JMP 0x0057cfc9
0057cfbf  c78594feffff00000000  MOV dword ptr [EBP + 0xfffffe94],0x0
0057cfc9  8b9594feffff          MOV EDX,dword ptr [EBP + 0xfffffe94]
0057cfcf  89558c                MOV dword ptr [EBP + -0x74],EDX
0057cfd2  eb1d                  JMP 0x0057cff1
0057cfd4  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0057cfd7  50                    PUSH EAX
0057cfd8  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057cfde  e89d460600            CALL 0x005e1680
0057cfe3  0fb6c8                MOVZX ECX,AL
0057cfe6  33d2                  XOR EDX,EDX
0057cfe8  83f902                CMP ECX,0x2
0057cfeb  0f94c2                SETZ DL
0057cfee  89558c                MOV dword ptr [EBP + -0x74],EDX
0057cff1  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0057cff4  8b10                  MOV EDX,dword ptr [EAX]
0057cff6  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057cff9  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057cffc  ffd0                  CALL EAX
0057cffe  85c0                  TEST EAX,EAX
0057d000  0f84fb020000          JZ 0x0057d301
0057d006  837d8c00              CMP dword ptr [EBP + -0x74],0x0
0057d00a  0f84f1020000          JZ 0x0057d301
0057d010  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d013  8b11                  MOV EDX,dword ptr [ECX]
0057d015  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d018  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057d01b  ffd0                  CALL EAX
0057d01d  898590feffff          MOV dword ptr [EBP + 0xfffffe90],EAX
0057d023  8b8d90feffff          MOV ECX,dword ptr [EBP + 0xfffffe90]
0057d029  8b11                  MOV EDX,dword ptr [ECX]
0057d02b  8b8d90feffff          MOV ECX,dword ptr [EBP + 0xfffffe90]
0057d031  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
0057d037  ffd0                  CALL EAX
0057d039  85c0                  TEST EAX,EAX
0057d03b  0f85c0020000          JNZ 0x0057d301
0057d041  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d044  8b11                  MOV EDX,dword ptr [ECX]
0057d046  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d049  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057d04c  ffd0                  CALL EAX
0057d04e  33c9                  XOR ECX,ECX
0057d050  0f85ab020000          JNZ 0x0057d301
0057d056  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057d059  8b02                  MOV EAX,dword ptr [EDX]
0057d05b  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d05e  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0057d061  ffd2                  CALL EDX
0057d063  8985e0feffff          MOV dword ptr [EBP + 0xfffffee0],EAX
0057d069  8b85e0feffff          MOV EAX,dword ptr [EBP + 0xfffffee0]
0057d06f  83b8ac11000000        CMP dword ptr [EAX + 0x11ac],0x0
0057d076  742c                  JZ 0x0057d0a4
0057d078  6a00                  PUSH 0x0
0057d07a  8b8de0feffff          MOV ECX,dword ptr [EBP + 0xfffffee0]
0057d080  8b11                  MOV EDX,dword ptr [ECX]
0057d082  8b8de0feffff          MOV ECX,dword ptr [EBP + 0xfffffee0]
0057d088  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
0057d08e  ffd0                  CALL EAX
0057d090  0fbfc8                MOVSX ECX,AX
0057d093  33d2                  XOR EDX,EDX
0057d095  85c9                  TEST ECX,ECX
0057d097  0f9ec2                SETLE DL
0057d09a  8995dcfeffff          MOV dword ptr [EBP + 0xfffffedc],EDX
0057d0a0  eb0c                  JMP 0x0057d0ae
0057d0a4  c785dcfeffff00000000  MOV dword ptr [EBP + 0xfffffedc],0x0
0057d0ae  83bddcfeffff00        CMP dword ptr [EBP + 0xfffffedc],0x0
0057d0b5  0f8546020000          JNZ 0x0057d301
0057d0bb  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0057d0be  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0057d0c1  898dd8feffff          MOV dword ptr [EBP + 0xfffffed8],ECX
0057d0c7  8b95d8feffff          MOV EDX,dword ptr [EBP + 0xfffffed8]
0057d0cd  52                    PUSH EDX
0057d0ce  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057d0d4  e877460500            CALL 0x005d1750
0057d0d9  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0057d0dc  837dd400              CMP dword ptr [EBP + -0x2c],0x0
0057d0e0  0f841b020000          JZ 0x0057d301
0057d0e6  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
0057d0e9  8a4804                MOV CL,byte ptr [EAX + 0x4]
0057d0ec  80e101                AND CL,0x1
0057d0ef  0fb6d1                MOVZX EDX,CL
0057d0f2  85d2                  TEST EDX,EDX
0057d0f4  0f8407020000          JZ 0x0057d301
0057d0fa  d9ee                  FLDZ
0057d0fc  d95d80                FSTP float ptr [EBP + -0x80]
0057d0ff  d9ee                  FLDZ
0057d101  d95d84                FSTP float ptr [EBP + -0x7c]
0057d104  d9ee                  FLDZ
0057d106  d95d88                FSTP float ptr [EBP + -0x78]
0057d109  817d100000007f        CMP dword ptr [EBP + 0x10],0x7f000000
0057d110  744a                  JZ 0x0057d15c
0057d112  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057d117  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057d11a  898dd4feffff          MOV dword ptr [EBP + 0xfffffed4],ECX
0057d120  8b5510                MOV EDX,dword ptr [EBP + 0x10]
0057d123  52                    PUSH EDX
0057d124  8b8dd4feffff          MOV ECX,dword ptr [EBP + 0xfffffed4]
0057d12a  e8d1eff9ff            CALL 0x0051c100
0057d12f  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
0057d135  83bd7cffffff00        CMP dword ptr [EBP + 0xffffff7c],0x0
0057d13c  741c                  JZ 0x0057d15a
0057d13e  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
0057d144  0594000000            ADD EAX,0x94
0057d149  8b08                  MOV ECX,dword ptr [EAX]
0057d14b  894d80                MOV dword ptr [EBP + -0x80],ECX
0057d14e  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0057d151  895584                MOV dword ptr [EBP + -0x7c],EDX
0057d154  8b4008                MOV EAX,dword ptr [EAX + 0x8]
0057d157  894588                MOV dword ptr [EBP + -0x78],EAX
0057d15a  eb1d                  JMP 0x0057d179
0057d15c  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
0057d162  81c194000000          ADD ECX,0x94
0057d168  8b11                  MOV EDX,dword ptr [ECX]
0057d16a  895580                MOV dword ptr [EBP + -0x80],EDX
0057d16d  8b4104                MOV EAX,dword ptr [ECX + 0x4]
0057d170  894584                MOV dword ptr [EBP + -0x7c],EAX
0057d173  8b4908                MOV ECX,dword ptr [ECX + 0x8]
0057d176  894d88                MOV dword ptr [EBP + -0x78],ECX
0057d179  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057d17c  8b02                  MOV EAX,dword ptr [EDX]
0057d17e  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d181  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0057d184  ffd2                  CALL EDX
0057d186  8985d0feffff          MOV dword ptr [EBP + 0xfffffed0],EAX
0057d18c  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
0057d192  0594000000            ADD EAX,0x94
0057d197  50                    PUSH EAX
0057d198  8d4d80                LEA ECX,[EBP + -0x80]
0057d19b  51                    PUSH ECX
0057d19c  8d954cffffff          LEA EDX,[EBP + 0xffffff4c]
0057d1a2  52                    PUSH EDX
0057d1a3  e80881ecff            CALL 0x004452b0
0057d1a8  83c40c                ADD ESP,0xc
0057d1ab  50                    PUSH EAX
0057d1ac  e83f71f9ff            CALL 0x005142f0
0057d1b1  83c404                ADD ESP,0x4
0057d1b4  d95de0                FSTP float ptr [EBP + -0x20]
0057d1b7  8b85a0feffff          MOV EAX,dword ptr [EBP + 0xfffffea0]
0057d1bd  8b8880030000          MOV ECX,dword ptr [EAX + 0x380]
0057d1c3  898dc0feffff          MOV dword ptr [EBP + 0xfffffec0],ECX
0057d1c9  8b95c0feffff          MOV EDX,dword ptr [EBP + 0xfffffec0]
0057d1cf  d94208                FLD float ptr [EDX + 0x8]
0057d1d2  d99dbcfeffff          FSTP float ptr [EBP + 0xfffffebc]
0057d1d8  d945e0                FLD float ptr [EBP + -0x20]
0057d1db  d8a5bcfeffff          FSUB float ptr [EBP + 0xfffffebc]
0057d1e1  d95de0                FSTP float ptr [EBP + -0x20]
0057d1e4  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0057d1e7  8b10                  MOV EDX,dword ptr [EAX]
0057d1e9  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d1ec  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057d1ef  ffd0                  CALL EAX
0057d1f1  8985b8feffff          MOV dword ptr [EBP + 0xfffffeb8],EAX
0057d1f7  8b8db8feffff          MOV ECX,dword ptr [EBP + 0xfffffeb8]
0057d1fd  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
0057d203  8995b4feffff          MOV dword ptr [EBP + 0xfffffeb4],EDX
0057d209  8b85b4feffff          MOV EAX,dword ptr [EBP + 0xfffffeb4]
0057d20f  d94008                FLD float ptr [EAX + 0x8]
0057d212  d99db0feffff          FSTP float ptr [EBP + 0xfffffeb0]
0057d218  d945e0                FLD float ptr [EBP + -0x20]
0057d21b  d8a5b0feffff          FSUB float ptr [EBP + 0xfffffeb0]
0057d221  d95de0                FSTP float ptr [EBP + -0x20]
0057d224  d945e0                FLD float ptr [EBP + -0x20]
0057d227  d945e4                FLD float ptr [EBP + -0x1c]
0057d22a  ded9                  FCOMPP
0057d22c  dfe0                  FNSTSW AX
0057d22e  f6c441                TEST AH,0x41
0057d231  0f85ca000000          JNZ 0x0057d301
0057d237  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
0057d23a  898d70ffffff          MOV dword ptr [EBP + 0xffffff70],ECX
0057d240  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
0057d243  899574ffffff          MOV dword ptr [EBP + 0xffffff74],EDX
0057d249  8b4588                MOV EAX,dword ptr [EBP + -0x78]
0057d24c  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
0057d252  d98578ffffff          FLD float ptr [EBP + 0xffffff78]
0057d258  dc05109f9800          FADD double ptr [0x00989f10]
0057d25e  d99d78ffffff          FSTP float ptr [EBP + 0xffffff78]
0057d264  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d267  8b11                  MOV EDX,dword ptr [ECX]
0057d269  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d26c  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0057d26f  ffd0                  CALL EAX
0057d271  8985acfeffff          MOV dword ptr [EBP + 0xfffffeac],EAX
0057d277  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
0057d27d  81c194000000          ADD ECX,0x94
0057d283  8b11                  MOV EDX,dword ptr [ECX]
0057d285  899564ffffff          MOV dword ptr [EBP + 0xffffff64],EDX
0057d28b  8b4104                MOV EAX,dword ptr [ECX + 0x4]
0057d28e  898568ffffff          MOV dword ptr [EBP + 0xffffff68],EAX
0057d294  8b4908                MOV ECX,dword ptr [ECX + 0x8]
0057d297  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
0057d29d  d9856cffffff          FLD float ptr [EBP + 0xffffff6c]
0057d2a3  dc05109f9800          FADD double ptr [0x00989f10]
0057d2a9  d99d6cffffff          FSTP float ptr [EBP + 0xffffff6c]
0057d2af  837dc400              CMP dword ptr [EBP + -0x3c],0x0
0057d2b3  744c                  JZ 0x0057d301
0057d2b5  8b95a0feffff          MOV EDX,dword ptr [EBP + 0xfffffea0]
0057d2bb  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0057d2be  8985a8feffff          MOV dword ptr [EBP + 0xfffffea8],EAX
0057d2c4  6a00                  PUSH 0x0
0057d2c6  6a01                  PUSH 0x1
0057d2c8  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057d2cb  51                    PUSH ECX
0057d2cc  8b95a8feffff          MOV EDX,dword ptr [EBP + 0xfffffea8]
0057d2d2  52                    PUSH EDX
0057d2d3  8d45fc                LEA EAX,[EBP + -0x4]
0057d2d6  50                    PUSH EAX
0057d2d7  8d4dec                LEA ECX,[EBP + -0x14]
0057d2da  51                    PUSH ECX
0057d2db  8d9564ffffff          LEA EDX,[EBP + 0xffffff64]
0057d2e1  52                    PUSH EDX
0057d2e2  8d8570ffffff          LEA EAX,[EBP + 0xffffff70]
0057d2e8  50                    PUSH EAX
0057d2e9  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
0057d2ec  e86fe8faff            CALL 0x0052bb60
0057d2f1  85c0                  TEST EAX,EAX
0057d2f3  740c                  JZ 0x0057d301
0057d2f5  d945e0                FLD float ptr [EBP + -0x20]
0057d2f8  d95de4                FSTP float ptr [EBP + -0x1c]
0057d2fb  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057d2fe  894de8                MOV dword ptr [EBP + -0x18],ECX
0057d301  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
0057d304  8b02                  MOV EAX,dword ptr [EDX]
0057d306  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d309  8b5010                MOV EDX,dword ptr [EAX + 0x10]
0057d30c  ffd2                  CALL EDX
0057d30e  85c0                  TEST EAX,EAX
0057d310  743e                  JZ 0x0057d350
0057d312  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0057d315  8b10                  MOV EDX,dword ptr [EAX]
0057d317  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0057d31a  8b4210                MOV EAX,dword ptr [EDX + 0x10]
0057d31d  ffd0                  CALL EAX
0057d31f  8985a4feffff          MOV dword ptr [EBP + 0xfffffea4],EAX
0057d325  8b8da4feffff          MOV ECX,dword ptr [EBP + 0xfffffea4]
0057d32b  d98194000000          FLD float ptr [ECX + 0x94]
0057d331  8b95a0feffff          MOV EDX,dword ptr [EBP + 0xfffffea0]
0057d337  d8a294000000          FSUB float ptr [EDX + 0x94]
0057d33d  d945e4                FLD float ptr [EBP + -0x1c]
0057d340  ded9                  FCOMPP
0057d342  dfe0                  FNSTSW AX
0057d344  f6c405                TEST AH,0x5
0057d347  7a07                  JP 0x0057d350
0057d349  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
0057d350  eb07                  JMP 0x0057d359
0057d352  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
0057d359  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0057d35c  83c001                ADD EAX,0x1
0057d35f  8945cc                MOV dword ptr [EBP + -0x34],EAX
0057d362  e918f7ffff            JMP 0x0057ca7f
0057d367  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0057d36a  8be5                  MOV ESP,EBP
0057d36c  5d                    POP EBP
0057d36d  c20c00                RET 0xc
