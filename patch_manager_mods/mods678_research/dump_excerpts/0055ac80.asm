; FN 0055ac80 FUN_0055ac80
0055ac80  55                    PUSH EBP
0055ac81  8bec                  MOV EBP,ESP
0055ac83  81ecb4020000          SUB ESP,0x2b4
0055ac89  56                    PUSH ESI
0055ac8a  898d58fdffff          MOV dword ptr [EBP + 0xfffffd58],ECX
0055ac90  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055ac93  d94074                FLD float ptr [EAX + 0x74]
0055ac96  d95df8                FSTP float ptr [EBP + -0x8]
0055ac99  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055ac9c  d94178                FLD float ptr [ECX + 0x78]
0055ac9f  d95dec                FSTP float ptr [EBP + -0x14]
0055aca2  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055aca5  d9425c                FLD float ptr [EDX + 0x5c]
0055aca8  d95ddc                FSTP float ptr [EBP + -0x24]
0055acab  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055acae  d94060                FLD float ptr [EAX + 0x60]
0055acb1  d95df4                FSTP float ptr [EBP + -0xc]
0055acb4  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055acb7  d94104                FLD float ptr [ECX + 0x4]
0055acba  d99d30feffff          FSTP float ptr [EBP + 0xfffffe30]
0055acc0  d98530feffff          FLD float ptr [EBP + 0xfffffe30]
0055acc6  d95df0                FSTP float ptr [EBP + -0x10]
0055acc9  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055accc  d94214                FLD float ptr [EDX + 0x14]
0055accf  d95dfc                FSTP float ptr [EBP + -0x4]
0055acd2  8b8d58fdffff          MOV ECX,dword ptr [EBP + 0xfffffd58]
0055acd8  e813d2ffff            CALL 0x00557ef0
0055acdd  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0055ace0  837de400              CMP dword ptr [EBP + -0x1c],0x0
0055ace4  742c                  JZ 0x0055ad12
0055ace6  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0055ace9  8b8824010000          MOV ECX,dword ptr [EAX + 0x124]
0055acef  898d2cfeffff          MOV dword ptr [EBP + 0xfffffe2c],ECX
0055acf5  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055acf8  8b8280000000          MOV EAX,dword ptr [EDX + 0x80]
0055acfe  898528feffff          MOV dword ptr [EBP + 0xfffffe28],EAX
0055ad04  8b8d2cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe2c]
0055ad0a  3b8d28feffff          CMP ECX,dword ptr [EBP + 0xfffffe28]
0055ad10  740a                  JZ 0x0055ad1c
0055ad12  a1bc449900            MOV EAX,[0x009944bc]
0055ad17  e9060b0000            JMP 0x0055b822
0055ad1c  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055ad1f  8b423c                MOV EAX,dword ptr [EDX + 0x3c]
0055ad22  898524feffff          MOV dword ptr [EBP + 0xfffffe24],EAX
0055ad28  8b8d24feffff          MOV ECX,dword ptr [EBP + 0xfffffe24]
0055ad2e  83c101                ADD ECX,0x1
0055ad31  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055ad34  894a3c                MOV dword ptr [EDX + 0x3c],ECX
0055ad37  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0055ad3a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055ad3d  8988b0010000          MOV dword ptr [EAX + 0x1b0],ECX
0055ad43  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055ad46  d94268                FLD float ptr [EDX + 0x68]
0055ad49  d99d20feffff          FSTP float ptr [EBP + 0xfffffe20]
0055ad4f  d98520feffff          FLD float ptr [EBP + 0xfffffe20]
0055ad55  dc1d706b9800          FCOMP double ptr [0x00986b70]
0055ad5b  dfe0                  FNSTSW AX
0055ad5d  f6c441                TEST AH,0x41
0055ad60  0f85b5010000          JNZ 0x0055af1b
0055ad66  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055ad69  8b8840020000          MOV ECX,dword ptr [EAX + 0x240]
0055ad6f  898d1cfeffff          MOV dword ptr [EBP + 0xfffffe1c],ECX
0055ad75  83bd1cfeffff00        CMP dword ptr [EBP + 0xfffffe1c],0x0
0055ad7c  0f8599010000          JNZ 0x0055af1b
0055ad82  d945f8                FLD float ptr [EBP + -0x8]
0055ad85  d95dd0                FSTP float ptr [EBP + -0x30]
0055ad88  d945ec                FLD float ptr [EBP + -0x14]
0055ad8b  d95dd4                FSTP float ptr [EBP + -0x2c]
0055ad8e  d9ee                  FLDZ
0055ad90  d95dd8                FSTP float ptr [EBP + -0x28]
0055ad93  d945dc                FLD float ptr [EBP + -0x24]
0055ad96  d95db8                FSTP float ptr [EBP + -0x48]
0055ad99  d945f4                FLD float ptr [EBP + -0xc]
0055ad9c  d95dbc                FSTP float ptr [EBP + -0x44]
0055ad9f  d9ee                  FLDZ
0055ada1  d95dc0                FSTP float ptr [EBP + -0x40]
0055ada4  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
0055ada7  8955ac                MOV dword ptr [EBP + -0x54],EDX
0055adaa  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0055adad  8945b0                MOV dword ptr [EBP + -0x50],EAX
0055adb0  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0055adb3  894db4                MOV dword ptr [EBP + -0x4c],ECX
0055adb6  8d55d0                LEA EDX,[EBP + -0x30]
0055adb9  52                    PUSH EDX
0055adba  8d45b8                LEA EAX,[EBP + -0x48]
0055adbd  50                    PUSH EAX
0055adbe  8d4dc4                LEA ECX,[EBP + -0x3c]
0055adc1  51                    PUSH ECX
0055adc2  e8e9a4eeff            CALL 0x004452b0
0055adc7  83c40c                ADD ESP,0xc
0055adca  8d55c4                LEA EDX,[EBP + -0x3c]
0055adcd  52                    PUSH EDX
0055adce  8d45c4                LEA EAX,[EBP + -0x3c]
0055add1  50                    PUSH EAX
0055add2  e8c90aefff            CALL 0x0044b8a0
0055add7  83c408                ADD ESP,0x8
0055adda  d95da4                FSTP float ptr [EBP + -0x5c]
0055addd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055ade0  d94168                FLD float ptr [ECX + 0x68]
0055ade3  d95da8                FSTP float ptr [EBP + -0x58]
0055ade6  d945a4                FLD float ptr [EBP + -0x5c]
0055ade9  dc1dc84d9900          FCOMP double ptr [0x00994dc8]
0055adef  dfe0                  FNSTSW AX
0055adf1  f6c401                TEST AH,0x1
0055adf4  0f8515010000          JNZ 0x0055af0f
0055adfa  d945a8                FLD float ptr [EBP + -0x58]
0055adfd  d84da8                FMUL float ptr [EBP + -0x58]
0055ae00  d945a4                FLD float ptr [EBP + -0x5c]
0055ae03  ded9                  FCOMPP
0055ae05  dfe0                  FNSTSW AX
0055ae07  f6c441                TEST AH,0x41
0055ae0a  757a                  JNZ 0x0055ae86
0055ae0c  8d55b8                LEA EDX,[EBP + -0x48]
0055ae0f  52                    PUSH EDX
0055ae10  8d45d0                LEA EAX,[EBP + -0x30]
0055ae13  50                    PUSH EAX
0055ae14  8d8df8feffff          LEA ECX,[EBP + 0xfffffef8]
0055ae1a  51                    PUSH ECX
0055ae1b  e890a4eeff            CALL 0x004452b0
0055ae20  83c40c                ADD ESP,0xc
0055ae23  50                    PUSH EAX
0055ae24  8d95ecfeffff          LEA EDX,[EBP + 0xfffffeec]
0055ae2a  52                    PUSH EDX
0055ae2b  e89095fbff            CALL 0x005143c0
0055ae30  83c408                ADD ESP,0x8
0055ae33  50                    PUSH EAX
0055ae34  d945a8                FLD float ptr [EBP + -0x58]
0055ae37  dc25706b9800          FSUB double ptr [0x00986b70]
0055ae3d  d99d54fdffff          FSTP float ptr [EBP + 0xfffffd54]
0055ae43  d98554fdffff          FLD float ptr [EBP + 0xfffffd54]
0055ae49  51                    PUSH ECX
0055ae4a  d91c24                FSTP float ptr [ESP]
0055ae4d  8d85e0feffff          LEA EAX,[EBP + 0xfffffee0]
0055ae53  50                    PUSH EAX
0055ae54  e88721f0ff            CALL 0x0045cfe0
0055ae59  83c40c                ADD ESP,0xc
0055ae5c  50                    PUSH EAX
0055ae5d  8d4db8                LEA ECX,[EBP + -0x48]
0055ae60  51                    PUSH ECX
0055ae61  8d95d4feffff          LEA EDX,[EBP + 0xfffffed4]
0055ae67  52                    PUSH EDX
0055ae68  e8331aefff            CALL 0x0044c8a0
0055ae6d  83c40c                ADD ESP,0xc
0055ae70  8b08                  MOV ECX,dword ptr [EAX]
0055ae72  894dac                MOV dword ptr [EBP + -0x54],ECX
0055ae75  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0055ae78  8955b0                MOV dword ptr [EBP + -0x50],EDX
0055ae7b  8b4008                MOV EAX,dword ptr [EAX + 0x8]
0055ae7e  8945b4                MOV dword ptr [EBP + -0x4c],EAX
0055ae81  e989000000            JMP 0x0055af0f
0055ae86  d945a4                FLD float ptr [EBP + -0x5c]
0055ae89  83ec08                SUB ESP,0x8
0055ae8c  dd1c24                FSTP double ptr [ESP]
0055ae8f  e8901c3c00            CALL 0x0091cb24
0055ae94  83c408                ADD ESP,0x8
0055ae97  d99d50fdffff          FSTP float ptr [EBP + 0xfffffd50]
0055ae9d  d98550fdffff          FLD float ptr [EBP + 0xfffffd50]
0055aea3  d99ddcfdffff          FSTP float ptr [EBP + 0xfffffddc]
0055aea9  8d4db8                LEA ECX,[EBP + -0x48]
0055aeac  51                    PUSH ECX
0055aead  8d55d0                LEA EDX,[EBP + -0x30]
0055aeb0  52                    PUSH EDX
0055aeb1  8d85c8feffff          LEA EAX,[EBP + 0xfffffec8]
0055aeb7  50                    PUSH EAX
0055aeb8  e8f3a3eeff            CALL 0x004452b0
0055aebd  83c40c                ADD ESP,0xc
0055aec0  50                    PUSH EAX
0055aec1  8d8dbcfeffff          LEA ECX,[EBP + 0xfffffebc]
0055aec7  51                    PUSH ECX
0055aec8  e8f394fbff            CALL 0x005143c0
0055aecd  83c408                ADD ESP,0x8
0055aed0  50                    PUSH EAX
0055aed1  51                    PUSH ECX
0055aed2  d985dcfdffff          FLD float ptr [EBP + 0xfffffddc]
0055aed8  d91c24                FSTP float ptr [ESP]
0055aedb  8d95b0feffff          LEA EDX,[EBP + 0xfffffeb0]
0055aee1  52                    PUSH EDX
0055aee2  e8f920f0ff            CALL 0x0045cfe0
0055aee7  83c40c                ADD ESP,0xc
0055aeea  50                    PUSH EAX
0055aeeb  8d45b8                LEA EAX,[EBP + -0x48]
0055aeee  50                    PUSH EAX
0055aeef  8d8da4feffff          LEA ECX,[EBP + 0xfffffea4]
0055aef5  51                    PUSH ECX
0055aef6  e8a519efff            CALL 0x0044c8a0
0055aefb  83c40c                ADD ESP,0xc
0055aefe  8b10                  MOV EDX,dword ptr [EAX]
0055af00  8955ac                MOV dword ptr [EBP + -0x54],EDX
0055af03  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0055af06  894db0                MOV dword ptr [EBP + -0x50],ECX
0055af09  8b5008                MOV EDX,dword ptr [EAX + 0x8]
0055af0c  8955b4                MOV dword ptr [EBP + -0x4c],EDX
0055af0f  d945ac                FLD float ptr [EBP + -0x54]
0055af12  d95ddc                FSTP float ptr [EBP + -0x24]
0055af15  d945b0                FLD float ptr [EBP + -0x50]
0055af18  d95df4                FSTP float ptr [EBP + -0xc]
0055af1b  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055af1e  8b483c                MOV ECX,dword ptr [EAX + 0x3c]
0055af21  898dc0fdffff          MOV dword ptr [EBP + 0xfffffdc0],ECX
0055af27  83bdc0fdffff01        CMP dword ptr [EBP + 0xfffffdc0],0x1
0055af2e  756a                  JNZ 0x0055af9a
0055af30  d945dc                FLD float ptr [EBP + -0x24]
0055af33  d99d98feffff          FSTP float ptr [EBP + 0xfffffe98]
0055af39  d945f4                FLD float ptr [EBP + -0xc]
0055af3c  d99d9cfeffff          FSTP float ptr [EBP + 0xfffffe9c]
0055af42  d9ee                  FLDZ
0055af44  d99da0feffff          FSTP float ptr [EBP + 0xfffffea0]
0055af4a  d945f8                FLD float ptr [EBP + -0x8]
0055af4d  d99d8cfeffff          FSTP float ptr [EBP + 0xfffffe8c]
0055af53  d945ec                FLD float ptr [EBP + -0x14]
0055af56  d99d90feffff          FSTP float ptr [EBP + 0xfffffe90]
0055af5c  d9ee                  FLDZ
0055af5e  d99d94feffff          FSTP float ptr [EBP + 0xfffffe94]
0055af64  6a00                  PUSH 0x0
0055af66  6a01                  PUSH 0x1
0055af68  51                    PUSH ECX
0055af69  d945fc                FLD float ptr [EBP + -0x4]
0055af6c  d91c24                FSTP float ptr [ESP]
0055af6f  8d55f0                LEA EDX,[EBP + -0x10]
0055af72  52                    PUSH EDX
0055af73  8d8598feffff          LEA EAX,[EBP + 0xfffffe98]
0055af79  50                    PUSH EAX
0055af7a  8d8d8cfeffff          LEA ECX,[EBP + 0xfffffe8c]
0055af80  51                    PUSH ECX
0055af81  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0055af84  e8f759ffff            CALL 0x00550980
0055af89  8945e0                MOV dword ptr [EBP + -0x20],EAX
0055af8c  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055af8f  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
0055af92  89829c010000          MOV dword ptr [EDX + 0x19c],EAX
0055af98  eb0c                  JMP 0x0055afa6
0055af9a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055af9d  8b919c010000          MOV EDX,dword ptr [ECX + 0x19c]
0055afa3  8955e0                MOV dword ptr [EBP + -0x20],EDX
0055afa6  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055afa9  8b8898010000          MOV ECX,dword ptr [EAX + 0x198]
0055afaf  898dbcfdffff          MOV dword ptr [EBP + 0xfffffdbc],ECX
0055afb5  83bdbcfdffff01        CMP dword ptr [EBP + 0xfffffdbc],0x1
0055afbc  0f8548010000          JNZ 0x0055b10a
0055afc2  837de001              CMP dword ptr [EBP + -0x20],0x1
0055afc6  0f8424010000          JZ 0x0055b0f0
0055afcc  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055afcf  8b8284000000          MOV EAX,dword ptr [EDX + 0x84]
0055afd5  8985b8fdffff          MOV dword ptr [EBP + 0xfffffdb8],EAX
0055afdb  83bdb8fdffff00        CMP dword ptr [EBP + 0xfffffdb8],0x0
0055afe2  0f8508010000          JNZ 0x0055b0f0
0055afe8  d90514c09800          FLD float ptr [0x0098c014]
0055afee  d95d98                FSTP float ptr [EBP + -0x68]
0055aff1  d90534be9800          FLD float ptr [0x0098be34]
0055aff7  d95d8c                FSTP float ptr [EBP + -0x74]
0055affa  d9ee                  FLDZ
0055affc  d95da0                FSTP float ptr [EBP + -0x60]
0055afff  c7459000000000        MOV dword ptr [EBP + -0x70],0x0
0055b006  d9458c                FLD float ptr [EBP + -0x74]
0055b009  dc1de8469900          FCOMP double ptr [0x009946e8]
0055b00f  dfe0                  FNSTSW AX
0055b011  f6c441                TEST AH,0x41
0055b014  0f85b4000000          JNZ 0x0055b0ce
0055b01a  d945dc                FLD float ptr [EBP + -0x24]
0055b01d  d865f8                FSUB float ptr [EBP + -0x8]
0055b020  d84d98                FMUL float ptr [EBP + -0x68]
0055b023  d845f8                FADD float ptr [EBP + -0x8]
0055b026  d95d88                FSTP float ptr [EBP + -0x78]
0055b029  d945f4                FLD float ptr [EBP + -0xc]
0055b02c  d865ec                FSUB float ptr [EBP + -0x14]
0055b02f  d84d98                FMUL float ptr [EBP + -0x68]
0055b032  d845ec                FADD float ptr [EBP + -0x14]
0055b035  d95d84                FSTP float ptr [EBP + -0x7c]
0055b038  d94588                FLD float ptr [EBP + -0x78]
0055b03b  d99d80feffff          FSTP float ptr [EBP + 0xfffffe80]
0055b041  d94584                FLD float ptr [EBP + -0x7c]
0055b044  d99d84feffff          FSTP float ptr [EBP + 0xfffffe84]
0055b04a  d9ee                  FLDZ
0055b04c  d99d88feffff          FSTP float ptr [EBP + 0xfffffe88]
0055b052  d945f8                FLD float ptr [EBP + -0x8]
0055b055  d99d74feffff          FSTP float ptr [EBP + 0xfffffe74]
0055b05b  d945ec                FLD float ptr [EBP + -0x14]
0055b05e  d99d78feffff          FSTP float ptr [EBP + 0xfffffe78]
0055b064  d9ee                  FLDZ
0055b066  d99d7cfeffff          FSTP float ptr [EBP + 0xfffffe7c]
0055b06c  6a00                  PUSH 0x0
0055b06e  6a01                  PUSH 0x1
0055b070  51                    PUSH ECX
0055b071  d945fc                FLD float ptr [EBP + -0x4]
0055b074  d91c24                FSTP float ptr [ESP]
0055b077  8d4df0                LEA ECX,[EBP + -0x10]
0055b07a  51                    PUSH ECX
0055b07b  8d9580feffff          LEA EDX,[EBP + 0xfffffe80]
0055b081  52                    PUSH EDX
0055b082  8d8574feffff          LEA EAX,[EBP + 0xfffffe74]
0055b088  50                    PUSH EAX
0055b089  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0055b08c  e8ef58ffff            CALL 0x00550980
0055b091  83f801                CMP EAX,0x1
0055b094  751e                  JNZ 0x0055b0b4
0055b096  d94598                FLD float ptr [EBP + -0x68]
0055b099  d8458c                FADD float ptr [EBP + -0x74]
0055b09c  d95d98                FSTP float ptr [EBP + -0x68]
0055b09f  d94588                FLD float ptr [EBP + -0x78]
0055b0a2  d95d94                FSTP float ptr [EBP + -0x6c]
0055b0a5  d94584                FLD float ptr [EBP + -0x7c]
0055b0a8  d95d9c                FSTP float ptr [EBP + -0x64]
0055b0ab  c7459001000000        MOV dword ptr [EBP + -0x70],0x1
0055b0b2  eb09                  JMP 0x0055b0bd
0055b0b4  d94598                FLD float ptr [EBP + -0x68]
0055b0b7  d8658c                FSUB float ptr [EBP + -0x74]
0055b0ba  d95d98                FSTP float ptr [EBP + -0x68]
0055b0bd  d9458c                FLD float ptr [EBP + -0x74]
0055b0c0  dc3590749800          FDIV double ptr [0x00987490]
0055b0c6  d95d8c                FSTP float ptr [EBP + -0x74]
0055b0c9  e938ffffff            JMP 0x0055b006
0055b0ce  837d9001              CMP dword ptr [EBP + -0x70],0x1
0055b0d2  7515                  JNZ 0x0055b0e9
0055b0d4  d94594                FLD float ptr [EBP + -0x6c]
0055b0d7  d95ddc                FSTP float ptr [EBP + -0x24]
0055b0da  d9459c                FLD float ptr [EBP + -0x64]
0055b0dd  d95df4                FSTP float ptr [EBP + -0xc]
0055b0e0  c745e001000000        MOV dword ptr [EBP + -0x20],0x1
0055b0e7  eb07                  JMP 0x0055b0f0
0055b0e9  c745e0fcffffff        MOV dword ptr [EBP + -0x20],0xfffffffc
0055b0f0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b0f3  c7819401000000000000  MOV dword ptr [ECX + 0x194],0x0
0055b0fd  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055b100  c7829801000000000000  MOV dword ptr [EDX + 0x198],0x0
0055b10a  837de001              CMP dword ptr [EBP + -0x20],0x1
0055b10e  7563                  JNZ 0x0055b173
0055b110  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b113  c7808c00000004000000  MOV dword ptr [EAX + 0x8c],0x4
0055b11d  6a10                  PUSH 0x10
0055b11f  e8ffe53b00            CALL 0x00919723
0055b124  83c404                ADD ESP,0x4
0055b127  898570feffff          MOV dword ptr [EBP + 0xfffffe70],EAX
0055b12d  8b8d70feffff          MOV ECX,dword ptr [EBP + 0xfffffe70]
0055b133  894d80                MOV dword ptr [EBP + -0x80],ECX
0055b136  8b5580                MOV EDX,dword ptr [EBP + -0x80]
0055b139  d945f8                FLD float ptr [EBP + -0x8]
0055b13c  d91a                  FSTP float ptr [EDX]
0055b13e  8b4580                MOV EAX,dword ptr [EBP + -0x80]
0055b141  d945ec                FLD float ptr [EBP + -0x14]
0055b144  d95804                FSTP float ptr [EAX + 0x4]
0055b147  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
0055b14a  d945dc                FLD float ptr [EBP + -0x24]
0055b14d  d95908                FSTP float ptr [ECX + 0x8]
0055b150  8b5580                MOV EDX,dword ptr [EBP + -0x80]
0055b153  d945f4                FLD float ptr [EBP + -0xc]
0055b156  d95a0c                FSTP float ptr [EDX + 0xc]
0055b159  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b15c  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
0055b15f  898890000000          MOV dword ptr [EAX + 0x90],ECX
0055b165  8b15b8449900          MOV EDX,dword ptr [0x009944b8]
0055b16b  8955e8                MOV dword ptr [EBP + -0x18],EDX
0055b16e  e973020000            JMP 0x0055b3e6
0055b173  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b176  8b482c                MOV ECX,dword ptr [EAX + 0x2c]
0055b179  898db4fdffff          MOV dword ptr [EBP + 0xfffffdb4],ECX
0055b17f  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0055b185  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0055b188  8985b0fdffff          MOV dword ptr [EBP + 0xfffffdb0],EAX
0055b18e  8b8db4fdffff          MOV ECX,dword ptr [EBP + 0xfffffdb4]
0055b194  51                    PUSH ECX
0055b195  8b8db0fdffff          MOV ECX,dword ptr [EBP + 0xfffffdb0]
0055b19b  e8600ffcff            CALL 0x0051c100
0055b1a0  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
0055b1a6  83bd7cffffff00        CMP dword ptr [EBP + 0xffffff7c],0x0
0055b1ad  7426                  JZ 0x0055b1d5
0055b1af  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
0055b1b5  8b4278                MOV EAX,dword ptr [EDX + 0x78]
0055b1b8  8985acfdffff          MOV dword ptr [EBP + 0xfffffdac],EAX
0055b1be  83bdacfdffff00        CMP dword ptr [EBP + 0xfffffdac],0x0
0055b1c5  750e                  JNZ 0x0055b1d5
0055b1c7  8b0dbc449900          MOV ECX,dword ptr [0x009944bc]
0055b1cd  894de8                MOV dword ptr [EBP + -0x18],ECX
0055b1d0  e911020000            JMP 0x0055b3e6
0055b1d5  c78578ffffff64000000  MOV dword ptr [EBP + 0xffffff78],0x64
0055b1df  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055b1e2  8b8294010000          MOV EAX,dword ptr [EDX + 0x194]
0055b1e8  8985a8fdffff          MOV dword ptr [EBP + 0xfffffda8],EAX
0055b1ee  83bda8fdffff00        CMP dword ptr [EBP + 0xfffffda8],0x0
0055b1f5  7456                  JZ 0x0055b24d
0055b1f7  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0055b1fa  33d2                  XOR EDX,EDX
0055b1fc  52                    PUSH EDX
0055b1fd  51                    PUSH ECX
0055b1fe  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b201  50                    PUSH EAX
0055b202  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0055b205  e8265cffff            CALL 0x00550e30
0055b20a  8945e8                MOV dword ptr [EBP + -0x18],EAX
0055b20d  833dd4e0a70000        CMP dword ptr [0x00a7e0d4],0x0
0055b214  7432                  JZ 0x0055b248
0055b216  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b219  8b512c                MOV EDX,dword ptr [ECX + 0x2c]
0055b21c  899570ffffff          MOV dword ptr [EBP + 0xffffff70],EDX
0055b222  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0055b227  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0055b22a  898da4fdffff          MOV dword ptr [EBP + 0xfffffda4],ECX
0055b230  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
0055b236  52                    PUSH EDX
0055b237  8b8da4fdffff          MOV ECX,dword ptr [EBP + 0xfffffda4]
0055b23d  e8be0efcff            CALL 0x0051c100
0055b242  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
0055b248  e9de000000            JMP 0x0055b32b
0055b24d  837de0fe              CMP dword ptr [EBP + -0x20],-0x2
0055b251  7416                  JZ 0x0055b269
0055b253  837de0ff              CMP dword ptr [EBP + -0x20],-0x1
0055b257  7410                  JZ 0x0055b269
0055b259  837de0fd              CMP dword ptr [EBP + -0x20],-0x3
0055b25d  740a                  JZ 0x0055b269
0055b25f  837de000              CMP dword ptr [EBP + -0x20],0x0
0055b263  0f85b9000000          JNZ 0x0055b322
0055b269  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0055b26c  33c9                  XOR ECX,ECX
0055b26e  3b885c020000          CMP ECX,dword ptr [EAX + 0x25c]
0055b274  1bd2                  SBB EDX,EDX
0055b276  f7da                  NEG EDX
0055b278  7454                  JZ 0x0055b2ce
0055b27a  833dd4e0a70000        CMP dword ptr [0x00a7e0d4],0x0
0055b281  7433                  JZ 0x0055b2b6
0055b283  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b286  8b482c                MOV ECX,dword ptr [EAX + 0x2c]
0055b289  898d68ffffff          MOV dword ptr [EBP + 0xffffff68],ECX
0055b28f  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0055b295  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0055b298  8985a0fdffff          MOV dword ptr [EBP + 0xfffffda0],EAX
0055b29e  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
0055b2a4  51                    PUSH ECX
0055b2a5  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
0055b2ab  e8500efcff            CALL 0x0051c100
0055b2b0  89856cffffff          MOV dword ptr [EBP + 0xffffff6c],EAX
0055b2b6  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0055b2b9  33c0                  XOR EAX,EAX
0055b2bb  50                    PUSH EAX
0055b2bc  52                    PUSH EDX
0055b2bd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b2c0  51                    PUSH ECX
0055b2c1  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0055b2c4  e82780ffff            CALL 0x005532f0
0055b2c9  8945e8                MOV dword ptr [EBP + -0x18],EAX
0055b2cc  eb52                  JMP 0x0055b320
0055b2ce  833dd4e0a70000        CMP dword ptr [0x00a7e0d4],0x0
0055b2d5  7433                  JZ 0x0055b30a
0055b2d7  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055b2da  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
0055b2dd  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
0055b2e3  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0055b2e9  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0055b2ec  89959cfdffff          MOV dword ptr [EBP + 0xfffffd9c],EDX
0055b2f2  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
0055b2f8  50                    PUSH EAX
0055b2f9  8b8d9cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd9c]
0055b2ff  e8fc0dfcff            CALL 0x0051c100
0055b304  898564ffffff          MOV dword ptr [EBP + 0xffffff64],EAX
0055b30a  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0055b30d  33d2                  XOR EDX,EDX
0055b30f  52                    PUSH EDX
0055b310  51                    PUSH ECX
0055b311  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b314  50                    PUSH EAX
0055b315  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0055b318  e8135bffff            CALL 0x00550e30
0055b31d  8945e8                MOV dword ptr [EBP + -0x18],EAX
0055b320  eb09                  JMP 0x0055b32b
0055b322  8b0dbc449900          MOV ECX,dword ptr [0x009944bc]
0055b328  894de8                MOV dword ptr [EBP + -0x18],ECX
0055b32b  833dd4e0a70000        CMP dword ptr [0x00a7e0d4],0x0
0055b332  0f8480000000          JZ 0x0055b3b8
0055b338  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055b33b  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
0055b33e  898558ffffff          MOV dword ptr [EBP + 0xffffff58],EAX
0055b344  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0055b34a  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0055b34d  899598fdffff          MOV dword ptr [EBP + 0xfffffd98],EDX
0055b353  8b8558ffffff          MOV EAX,dword ptr [EBP + 0xffffff58]
0055b359  50                    PUSH EAX
0055b35a  8b8d98fdffff          MOV ECX,dword ptr [EBP + 0xfffffd98]
0055b360  e89b0dfcff            CALL 0x0051c100
0055b365  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
0055b36b  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
0055b371  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0055b374  899594fdffff          MOV dword ptr [EBP + 0xfffffd94],EDX
0055b37a  8b8594fdffff          MOV EAX,dword ptr [EBP + 0xfffffd94]
0055b380  25ffffff0f            AND EAX,0xfffffff
0055b385  3da0860100            CMP EAX,0x186a0
0055b38a  760c                  JBE 0x0055b398
0055b38c  c7854cfdffff01000000  MOV dword ptr [EBP + 0xfffffd4c],0x1
0055b396  eb20                  JMP 0x0055b3b8
0055b398  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
0055b39e  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0055b3a1  899590fdffff          MOV dword ptr [EBP + 0xfffffd90],EDX
0055b3a7  8b8590fdffff          MOV EAX,dword ptr [EBP + 0xfffffd90]
0055b3ad  25ffffff0f            AND EAX,0xfffffff
0055b3b2  89854cfdffff          MOV dword ptr [EBP + 0xfffffd4c],EAX
0055b3b8  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b3bb  8b513c                MOV EDX,dword ptr [ECX + 0x3c]
0055b3be  89958cfdffff          MOV dword ptr [EBP + 0xfffffd8c],EDX
0055b3c4  8b858cfdffff          MOV EAX,dword ptr [EBP + 0xfffffd8c]
0055b3ca  3b8578ffffff          CMP EAX,dword ptr [EBP + 0xffffff78]
0055b3d0  7e14                  JLE 0x0055b3e6
0055b3d2  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0055b3d5  3b0db4449900          CMP ECX,dword ptr [0x009944b4]
0055b3db  7509                  JNZ 0x0055b3e6
0055b3dd  8b15bc449900          MOV EDX,dword ptr [0x009944bc]
0055b3e3  8955e8                MOV dword ptr [EBP + -0x18],EDX
0055b3e6  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0055b3e9  3b05bc449900          CMP EAX,dword ptr [0x009944bc]
0055b3ef  0f85c0020000          JNZ 0x0055b6b5
0055b3f5  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b3f8  8b11                  MOV EDX,dword ptr [ECX]
0055b3fa  899588fdffff          MOV dword ptr [EBP + 0xfffffd88],EDX
0055b400  83bd88fdffff00        CMP dword ptr [EBP + 0xfffffd88],0x0
0055b407  0f858d020000          JNZ 0x0055b69a
0055b40d  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b410  c7808400000000000000  MOV dword ptr [EAX + 0x84],0x0
0055b41a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b41d  d94174                FLD float ptr [ECX + 0x74]
0055b420  d95df8                FSTP float ptr [EBP + -0x8]
0055b423  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055b426  d94278                FLD float ptr [EDX + 0x78]
0055b429  d95dec                FSTP float ptr [EBP + -0x14]
0055b42c  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b42f  d9405c                FLD float ptr [EAX + 0x5c]
0055b432  d95ddc                FSTP float ptr [EBP + -0x24]
0055b435  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b438  d94160                FLD float ptr [ECX + 0x60]
0055b43b  d95df4                FSTP float ptr [EBP + -0xc]
0055b43e  d945dc                FLD float ptr [EBP + -0x24]
0055b441  d99d24ffffff          FSTP float ptr [EBP + 0xffffff24]
0055b447  d945f4                FLD float ptr [EBP + -0xc]
0055b44a  d99d28ffffff          FSTP float ptr [EBP + 0xffffff28]
0055b450  d9ee                  FLDZ
0055b452  d99d2cffffff          FSTP float ptr [EBP + 0xffffff2c]
0055b458  d945f8                FLD float ptr [EBP + -0x8]
0055b45b  d99d40ffffff          FSTP float ptr [EBP + 0xffffff40]
0055b461  d945ec                FLD float ptr [EBP + -0x14]
0055b464  d99d44ffffff          FSTP float ptr [EBP + 0xffffff44]
0055b46a  d9ee                  FLDZ
0055b46c  d99d48ffffff          FSTP float ptr [EBP + 0xffffff48]
0055b472  d9ee                  FLDZ
0055b474  d99d4cffffff          FSTP float ptr [EBP + 0xffffff4c]
0055b47a  d9ee                  FLDZ
0055b47c  d99d50ffffff          FSTP float ptr [EBP + 0xffffff50]
0055b482  d9ee                  FLDZ
0055b484  d99d54ffffff          FSTP float ptr [EBP + 0xffffff54]
0055b48a  d9ee                  FLDZ
0055b48c  d99d30ffffff          FSTP float ptr [EBP + 0xffffff30]
0055b492  d9ee                  FLDZ
0055b494  d99d34ffffff          FSTP float ptr [EBP + 0xffffff34]
0055b49a  d9ee                  FLDZ
0055b49c  d99d38ffffff          FSTP float ptr [EBP + 0xffffff38]
0055b4a2  c78520ffffff00000000  MOV dword ptr [EBP + 0xffffff20],0x0
0055b4ac  8d9540ffffff          LEA EDX,[EBP + 0xffffff40]
0055b4b2  52                    PUSH EDX
0055b4b3  8d8524ffffff          LEA EAX,[EBP + 0xffffff24]
0055b4b9  50                    PUSH EAX
0055b4ba  8d8d64feffff          LEA ECX,[EBP + 0xfffffe64]
0055b4c0  51                    PUSH ECX
0055b4c1  e8ea9deeff            CALL 0x004452b0
0055b4c6  83c40c                ADD ESP,0xc
0055b4c9  8b10                  MOV EDX,dword ptr [EAX]
0055b4cb  89954cffffff          MOV dword ptr [EBP + 0xffffff4c],EDX
0055b4d1  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0055b4d4  898d50ffffff          MOV dword ptr [EBP + 0xffffff50],ECX
0055b4da  8b5008                MOV EDX,dword ptr [EAX + 0x8]
0055b4dd  899554ffffff          MOV dword ptr [EBP + 0xffffff54],EDX
0055b4e3  d90514c09800          FLD float ptr [0x0098c014]
0055b4e9  d99d3cffffff          FSTP float ptr [EBP + 0xffffff3c]
0055b4ef  d90534be9800          FLD float ptr [0x0098be34]
0055b4f5  d99d1cffffff          FSTP float ptr [EBP + 0xffffff1c]
0055b4fb  837de0ff              CMP dword ptr [EBP + -0x20],-0x1
0055b4ff  0f8417010000          JZ 0x0055b61c
0055b505  d9851cffffff          FLD float ptr [EBP + 0xffffff1c]
0055b50b  dc1de8469900          FCOMP double ptr [0x009946e8]
0055b511  dfe0                  FNSTSW AX
0055b513  f6c441                TEST AH,0x41
0055b516  0f8500010000          JNZ 0x0055b61c
0055b51c  8d854cffffff          LEA EAX,[EBP + 0xffffff4c]
0055b522  50                    PUSH EAX
0055b523  51                    PUSH ECX
0055b524  d9853cffffff          FLD float ptr [EBP + 0xffffff3c]
0055b52a  d91c24                FSTP float ptr [ESP]
0055b52d  8d8d58feffff          LEA ECX,[EBP + 0xfffffe58]
0055b533  51                    PUSH ECX
0055b534  e8a71af0ff            CALL 0x0045cfe0
0055b539  83c40c                ADD ESP,0xc
0055b53c  50                    PUSH EAX
0055b53d  8d9540ffffff          LEA EDX,[EBP + 0xffffff40]
0055b543  52                    PUSH EDX
0055b544  8d854cfeffff          LEA EAX,[EBP + 0xfffffe4c]
0055b54a  50                    PUSH EAX
0055b54b  e85013efff            CALL 0x0044c8a0
0055b550  83c40c                ADD ESP,0xc
0055b553  8b08                  MOV ECX,dword ptr [EAX]
0055b555  898d24ffffff          MOV dword ptr [EBP + 0xffffff24],ECX
0055b55b  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0055b55e  899528ffffff          MOV dword ptr [EBP + 0xffffff28],EDX
0055b564  8b4008                MOV EAX,dword ptr [EAX + 0x8]
0055b567  89852cffffff          MOV dword ptr [EBP + 0xffffff2c],EAX
0055b56d  d945f8                FLD float ptr [EBP + -0x8]
0055b570  d99d40feffff          FSTP float ptr [EBP + 0xfffffe40]
0055b576  d945ec                FLD float ptr [EBP + -0x14]
0055b579  d99d44feffff          FSTP float ptr [EBP + 0xfffffe44]
0055b57f  d9ee                  FLDZ
0055b581  d99d48feffff          FSTP float ptr [EBP + 0xfffffe48]
0055b587  6a00                  PUSH 0x0
0055b589  6a00                  PUSH 0x0
0055b58b  51                    PUSH ECX
0055b58c  d945fc                FLD float ptr [EBP + -0x4]
0055b58f  d91c24                FSTP float ptr [ESP]
0055b592  8d4df0                LEA ECX,[EBP + -0x10]
0055b595  51                    PUSH ECX
0055b596  8d9524ffffff          LEA EDX,[EBP + 0xffffff24]
0055b59c  52                    PUSH EDX
0055b59d  8d8540feffff          LEA EAX,[EBP + 0xfffffe40]
0055b5a3  50                    PUSH EAX
0055b5a4  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0055b5a7  e8d453ffff            CALL 0x00550980
0055b5ac  83f801                CMP EAX,0x1
0055b5af  7542                  JNZ 0x0055b5f3
0055b5b1  8b8d24ffffff          MOV ECX,dword ptr [EBP + 0xffffff24]
0055b5b7  898d30ffffff          MOV dword ptr [EBP + 0xffffff30],ECX
0055b5bd  8b9528ffffff          MOV EDX,dword ptr [EBP + 0xffffff28]
0055b5c3  899534ffffff          MOV dword ptr [EBP + 0xffffff34],EDX
0055b5c9  8b852cffffff          MOV EAX,dword ptr [EBP + 0xffffff2c]
0055b5cf  898538ffffff          MOV dword ptr [EBP + 0xffffff38],EAX
0055b5d5  c78520ffffff01000000  MOV dword ptr [EBP + 0xffffff20],0x1
0055b5df  d9853cffffff          FLD float ptr [EBP + 0xffffff3c]
0055b5e5  d8851cffffff          FADD float ptr [EBP + 0xffffff1c]
0055b5eb  d99d3cffffff          FSTP float ptr [EBP + 0xffffff3c]
0055b5f1  eb12                  JMP 0x0055b605
0055b5f3  d9853cffffff          FLD float ptr [EBP + 0xffffff3c]
0055b5f9  d8a51cffffff          FSUB float ptr [EBP + 0xffffff1c]
0055b5ff  d99d3cffffff          FSTP float ptr [EBP + 0xffffff3c]
0055b605  d9851cffffff          FLD float ptr [EBP + 0xffffff1c]
0055b60b  dc3590749800          FDIV double ptr [0x00987490]
0055b611  d99d1cffffff          FSTP float ptr [EBP + 0xffffff1c]
0055b617  e9e9feffff            JMP 0x0055b505
0055b61c  83bd20ffffff00        CMP dword ptr [EBP + 0xffffff20],0x0
0055b623  7475                  JZ 0x0055b69a
0055b625  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b628  c7818c00000004000000  MOV dword ptr [ECX + 0x8c],0x4
0055b632  6a10                  PUSH 0x10
0055b634  e8eae03b00            CALL 0x00919723
0055b639  83c404                ADD ESP,0x4
0055b63c  89853cfeffff          MOV dword ptr [EBP + 0xfffffe3c],EAX
0055b642  8b953cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe3c]
0055b648  899518ffffff          MOV dword ptr [EBP + 0xffffff18],EDX
0055b64e  8b8518ffffff          MOV EAX,dword ptr [EBP + 0xffffff18]
0055b654  d945f8                FLD float ptr [EBP + -0x8]
0055b657  d918                  FSTP float ptr [EAX]
0055b659  8b8d18ffffff          MOV ECX,dword ptr [EBP + 0xffffff18]
0055b65f  d945ec                FLD float ptr [EBP + -0x14]
0055b662  d95904                FSTP float ptr [ECX + 0x4]
0055b665  8b9518ffffff          MOV EDX,dword ptr [EBP + 0xffffff18]
0055b66b  d98530ffffff          FLD float ptr [EBP + 0xffffff30]
0055b671  d95a08                FSTP float ptr [EDX + 0x8]
0055b674  8b8518ffffff          MOV EAX,dword ptr [EBP + 0xffffff18]
0055b67a  d98534ffffff          FLD float ptr [EBP + 0xffffff34]
0055b680  d9580c                FSTP float ptr [EAX + 0xc]
0055b683  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b686  8b9518ffffff          MOV EDX,dword ptr [EBP + 0xffffff18]
0055b68c  899190000000          MOV dword ptr [ECX + 0x90],EDX
0055b692  a1b8449900            MOV EAX,[0x009944b8]
0055b697  8945e8                MOV dword ptr [EBP + -0x18],EAX
0055b69a  8b0da0b59900          MOV ECX,dword ptr [0x0099b5a0]
0055b6a0  898d60fdffff          MOV dword ptr [EBP + 0xfffffd60],ECX
0055b6a6  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055b6a9  8b8560fdffff          MOV EAX,dword ptr [EBP + 0xfffffd60]
0055b6af  8982e0010000          MOV dword ptr [EDX + 0x1e0],EAX
0055b6b5  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0055b6b8  3b0db8449900          CMP ECX,dword ptr [0x009944b8]
0055b6be  0f8538010000          JNZ 0x0055b7fc
0055b6c4  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055b6c7  8b8284000000          MOV EAX,dword ptr [EDX + 0x84]
0055b6cd  89855cfdffff          MOV dword ptr [EBP + 0xfffffd5c],EAX
0055b6d3  83bd5cfdffff01        CMP dword ptr [EBP + 0xfffffd5c],0x1
0055b6da  0f851c010000          JNZ 0x0055b7fc
0055b6e0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b6e3  8b918c000000          MOV EDX,dword ptr [ECX + 0x8c]
0055b6e9  899508ffffff          MOV dword ptr [EBP + 0xffffff08],EDX
0055b6ef  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b6f2  8b8890000000          MOV ECX,dword ptr [EAX + 0x90]
0055b6f8  898d14ffffff          MOV dword ptr [EBP + 0xffffff14],ECX
0055b6fe  33c9                  XOR ECX,ECX
0055b700  8b8508ffffff          MOV EAX,dword ptr [EBP + 0xffffff08]
0055b706  ba04000000            MOV EDX,0x4
0055b70b  f7e2                  MUL EDX
0055b70d  0f90c1                SETO CL
0055b710  f7d9                  NEG ECX
0055b712  0bc8                  OR ECX,EAX
0055b714  51                    PUSH ECX
0055b715  e809e03b00            CALL 0x00919723
0055b71a  83c404                ADD ESP,0x4
0055b71d  898538feffff          MOV dword ptr [EBP + 0xfffffe38],EAX
0055b723  8b8538feffff          MOV EAX,dword ptr [EBP + 0xfffffe38]
0055b729  89850cffffff          MOV dword ptr [EBP + 0xffffff0c],EAX
0055b72f  c78504ffffff00000000  MOV dword ptr [EBP + 0xffffff04],0x0
0055b739  8b8d08ffffff          MOV ECX,dword ptr [EBP + 0xffffff08]
0055b73f  83e902                SUB ECX,0x2
0055b742  898d10ffffff          MOV dword ptr [EBP + 0xffffff10],ECX
0055b748  8b9504ffffff          MOV EDX,dword ptr [EBP + 0xffffff04]
0055b74e  3b9508ffffff          CMP EDX,dword ptr [EBP + 0xffffff08]
0055b754  7d5e                  JGE 0x0055b7b4
0055b756  8b8504ffffff          MOV EAX,dword ptr [EBP + 0xffffff04]
0055b75c  8b8d0cffffff          MOV ECX,dword ptr [EBP + 0xffffff0c]
0055b762  8b9510ffffff          MOV EDX,dword ptr [EBP + 0xffffff10]
0055b768  8bb514ffffff          MOV ESI,dword ptr [EBP + 0xffffff14]
0055b76e  d90496                FLD float ptr [ESI + EDX*0x4]
0055b771  d91c81                FSTP float ptr [ECX + EAX*0x4]
0055b774  8b8504ffffff          MOV EAX,dword ptr [EBP + 0xffffff04]
0055b77a  8b8d0cffffff          MOV ECX,dword ptr [EBP + 0xffffff0c]
0055b780  8b9510ffffff          MOV EDX,dword ptr [EBP + 0xffffff10]
0055b786  8bb514ffffff          MOV ESI,dword ptr [EBP + 0xffffff14]
0055b78c  d9449604              FLD float ptr [ESI + EDX*0x4 + 0x4]
0055b790  d95c8104              FSTP float ptr [ECX + EAX*0x4 + 0x4]
0055b794  8b8504ffffff          MOV EAX,dword ptr [EBP + 0xffffff04]
0055b79a  83c002                ADD EAX,0x2
0055b79d  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
0055b7a3  8b8d10ffffff          MOV ECX,dword ptr [EBP + 0xffffff10]
0055b7a9  83e902                SUB ECX,0x2
0055b7ac  898d10ffffff          MOV dword ptr [EBP + 0xffffff10],ECX
0055b7b2  eb94                  JMP 0x0055b748
0055b7b4  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0055b7b7  8b850cffffff          MOV EAX,dword ptr [EBP + 0xffffff0c]
0055b7bd  898290000000          MOV dword ptr [EDX + 0x90],EAX
0055b7c3  83bd14ffffff00        CMP dword ptr [EBP + 0xffffff14],0x0
0055b7ca  741b                  JZ 0x0055b7e7
0055b7cc  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
0055b7d2  898d34feffff          MOV dword ptr [EBP + 0xfffffe34],ECX
0055b7d8  8b9534feffff          MOV EDX,dword ptr [EBP + 0xfffffe34]
0055b7de  52                    PUSH EDX
0055b7df  e819df3b00            CALL 0x009196fd
0055b7e4  83c404                ADD ESP,0x4
0055b7e7  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b7ea  c7808400000000000000  MOV dword ptr [EAX + 0x84],0x0
0055b7f4  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0055b7f7  e8f4eb1900            CALL 0x006fa3f0
0055b7fc  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0055b7ff  3b0dbc449900          CMP ECX,dword ptr [0x009944bc]
0055b805  740b                  JZ 0x0055b812
0055b807  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0055b80a  3b15b8449900          CMP EDX,dword ptr [0x009944b8]
0055b810  750d                  JNZ 0x0055b81f
0055b812  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0055b815  c7808800000001000000  MOV dword ptr [EAX + 0x88],0x1
0055b81f  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0055b822  5e                    POP ESI
0055b823  8be5                  MOV ESP,EBP
0055b825  5d                    POP EBP
0055b826  c20800                RET 0x8
