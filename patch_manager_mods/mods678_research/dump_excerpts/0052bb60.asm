; FN 0052bb60 FUN_0052bb60
0052bb60  55                    PUSH EBP
0052bb61  8bec                  MOV EBP,ESP
0052bb63  6aff                  PUSH -0x1
0052bb65  6888e19400            PUSH 0x94e188
0052bb6a  64a100000000          MOV EAX,FS:[0x0]
0052bb70  50                    PUSH EAX
0052bb71  81ecc0000000          SUB ESP,0xc0
0052bb77  a1201fa100            MOV EAX,[0x00a11f20]
0052bb7c  33c5                  XOR EAX,EBP
0052bb7e  50                    PUSH EAX
0052bb7f  8d45f4                LEA EAX,[EBP + -0xc]
0052bb82  64a300000000          MOV FS:[0x0],EAX
0052bb88  898d34ffffff          MOV dword ptr [EBP + 0xffffff34],ECX
0052bb8e  d9ee                  FLDZ
0052bb90  d99d78ffffff          FSTP float ptr [EBP + 0xffffff78]
0052bb96  d9ee                  FLDZ
0052bb98  d99d7cffffff          FSTP float ptr [EBP + 0xffffff7c]
0052bb9e  d9ee                  FLDZ
0052bba0  d95d80                FSTP float ptr [EBP + -0x80]
0052bba3  c745d80000007f        MOV dword ptr [EBP + -0x28],0x7f000000
0052bbaa  d90500399900          FLD float ptr [0x00993900]
0052bbb0  d95df0                FSTP float ptr [EBP + -0x10]
0052bbb3  c745cc01000000        MOV dword ptr [EBP + -0x34],0x1
0052bbba  d9ee                  FLDZ
0052bbbc  d95dd0                FSTP float ptr [EBP + -0x30]
0052bbbf  d9ee                  FLDZ
0052bbc1  d95ddc                FSTP float ptr [EBP + -0x24]
0052bbc4  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052bbc7  d94008                FLD float ptr [EAX + 0x8]
0052bbca  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0052bbcd  d94108                FLD float ptr [ECX + 0x8]
0052bbd0  ded9                  FCOMPP
0052bbd2  dfe0                  FNSTSW AX
0052bbd4  f6c441                TEST AH,0x41
0052bbd7  7514                  JNZ 0x0052bbed
0052bbd9  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0052bbdc  d94208                FLD float ptr [EDX + 0x8]
0052bbdf  d95ddc                FSTP float ptr [EBP + -0x24]
0052bbe2  8b450c                MOV EAX,dword ptr [EBP + 0xc]
0052bbe5  d94008                FLD float ptr [EAX + 0x8]
0052bbe8  d95dd0                FSTP float ptr [EBP + -0x30]
0052bbeb  eb12                  JMP 0x0052bbff
0052bbed  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0052bbf0  d94108                FLD float ptr [ECX + 0x8]
0052bbf3  d95ddc                FSTP float ptr [EBP + -0x24]
0052bbf6  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0052bbf9  d94208                FLD float ptr [EDX + 0x8]
0052bbfc  d95dd0                FSTP float ptr [EBP + -0x30]
0052bbff  6a00                  PUSH 0x0
0052bc01  8d4de4                LEA ECX,[EBP + -0x1c]
0052bc04  e837ea3100            CALL 0x0084a640
0052bc09  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0052bc10  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0052bc15  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0052bc18  898d5cffffff          MOV dword ptr [EBP + 0xffffff5c],ECX
0052bc1e  8b5518                MOV EDX,dword ptr [EBP + 0x18]
0052bc21  52                    PUSH EDX
0052bc22  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
0052bc28  e87304ffff            CALL 0x0051c0a0
0052bc2d  8945e0                MOV dword ptr [EBP + -0x20],EAX
0052bc30  837de000              CMP dword ptr [EBP + -0x20],0x0
0052bc34  741d                  JZ 0x0052bc53
0052bc36  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
0052bc39  8b10                  MOV EDX,dword ptr [EAX]
0052bc3b  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0052bc3e  8b4210                MOV EAX,dword ptr [EDX + 0x10]
0052bc41  ffd0                  CALL EAX
0052bc43  85c0                  TEST EAX,EAX
0052bc45  740c                  JZ 0x0052bc53
0052bc47  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0052bc4a  51                    PUSH ECX
0052bc4b  8d4de4                LEA ECX,[EBP + -0x1c]
0052bc4e  e80d2e3100            CALL 0x0083ea60
0052bc53  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0052bc59  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0052bc5c  898558ffffff          MOV dword ptr [EBP + 0xffffff58],EAX
0052bc62  8b4d1c                MOV ECX,dword ptr [EBP + 0x1c]
0052bc65  51                    PUSH ECX
0052bc66  8b8d58ffffff          MOV ECX,dword ptr [EBP + 0xffffff58]
0052bc6c  e82f04ffff            CALL 0x0051c0a0
0052bc71  8945e0                MOV dword ptr [EBP + -0x20],EAX
0052bc74  837de000              CMP dword ptr [EBP + -0x20],0x0
0052bc78  741d                  JZ 0x0052bc97
0052bc7a  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0052bc7d  8b02                  MOV EAX,dword ptr [EDX]
0052bc7f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0052bc82  8b5010                MOV EDX,dword ptr [EAX + 0x10]
0052bc85  ffd2                  CALL EDX
0052bc87  85c0                  TEST EAX,EAX
0052bc89  740c                  JZ 0x0052bc97
0052bc8b  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
0052bc8e  50                    PUSH EAX
0052bc8f  8d4de4                LEA ECX,[EBP + -0x1c]
0052bc92  e8c92d3100            CALL 0x0083ea60
0052bc97  8d4d84                LEA ECX,[EBP + -0x7c]
0052bc9a  e8c1010000            CALL 0x0052be60
0052bc9f  6a00                  PUSH 0x0
0052bca1  8d4d84                LEA ECX,[EBP + -0x7c]
0052bca4  51                    PUSH ECX
0052bca5  8b5520                MOV EDX,dword ptr [EBP + 0x20]
0052bca8  52                    PUSH EDX
0052bca9  8d45e4                LEA EAX,[EBP + -0x1c]
0052bcac  50                    PUSH EAX
0052bcad  6a01                  PUSH 0x1
0052bcaf  8b4d14                MOV ECX,dword ptr [EBP + 0x14]
0052bcb2  51                    PUSH ECX
0052bcb3  680000007f            PUSH 0x7f000000
0052bcb8  51                    PUSH ECX
0052bcb9  d90574bd9800          FLD float ptr [0x0098bd74]
0052bcbf  d91c24                FSTP float ptr [ESP]
0052bcc2  51                    PUSH ECX
0052bcc3  d945d0                FLD float ptr [EBP + -0x30]
0052bcc6  d91c24                FSTP float ptr [ESP]
0052bcc9  51                    PUSH ECX
0052bcca  d945dc                FLD float ptr [EBP + -0x24]
0052bccd  d91c24                FSTP float ptr [ESP]
0052bcd0  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0052bcd3  52                    PUSH EDX
0052bcd4  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052bcd7  50                    PUSH EAX
0052bcd8  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
0052bcde  e84d140000            CALL 0x0052d130
0052bce3  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0052bce6  837dd400              CMP dword ptr [EBP + -0x2c],0x0
0052bcea  751b                  JNZ 0x0052bd07
0052bcec  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
0052bcef  8b5588                MOV EDX,dword ptr [EBP + -0x78]
0052bcf2  8911                  MOV dword ptr [ECX],EDX
0052bcf4  8b458c                MOV EAX,dword ptr [EBP + -0x74]
0052bcf7  894104                MOV dword ptr [ECX + 0x4],EAX
0052bcfa  8b5590                MOV EDX,dword ptr [EBP + -0x70]
0052bcfd  895108                MOV dword ptr [ECX + 0x8],EDX
0052bd00  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
0052bd07  837dd400              CMP dword ptr [EBP + -0x2c],0x0
0052bd0b  750a                  JNZ 0x0052bd17
0052bd0d  837d2400              CMP dword ptr [EBP + 0x24],0x0
0052bd11  0f8419010000          JZ 0x0052be30
0052bd17  c78574ffffff00000000  MOV dword ptr [EBP + 0xffffff74],0x0
0052bd21  eb0f                  JMP 0x0052bd32
0052bd23  8b8574ffffff          MOV EAX,dword ptr [EBP + 0xffffff74]
0052bd29  83c001                ADD EAX,0x1
0052bd2c  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
0052bd32  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
0052bd38  8b9574ffffff          MOV EDX,dword ptr [EBP + 0xffffff74]
0052bd3e  3b9150020000          CMP EDX,dword ptr [ECX + 0x250]
0052bd44  0f8de6000000          JGE 0x0052be30
0052bd4a  8d45d8                LEA EAX,[EBP + -0x28]
0052bd4d  50                    PUSH EAX
0052bd4e  8d8d78ffffff          LEA ECX,[EBP + 0xffffff78]
0052bd54  51                    PUSH ECX
0052bd55  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0052bd58  83ec0c                SUB ESP,0xc
0052bd5b  8bc4                  MOV EAX,ESP
0052bd5d  8b0a                  MOV ECX,dword ptr [EDX]
0052bd5f  8908                  MOV dword ptr [EAX],ECX
0052bd61  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
0052bd64  894804                MOV dword ptr [EAX + 0x4],ECX
0052bd67  8b5208                MOV EDX,dword ptr [EDX + 0x8]
0052bd6a  895008                MOV dword ptr [EAX + 0x8],EDX
0052bd6d  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052bd70  83ec0c                SUB ESP,0xc
0052bd73  8bcc                  MOV ECX,ESP
0052bd75  8b10                  MOV EDX,dword ptr [EAX]
0052bd77  8911                  MOV dword ptr [ECX],EDX
0052bd79  8b5004                MOV EDX,dword ptr [EAX + 0x4]
0052bd7c  895104                MOV dword ptr [ECX + 0x4],EDX
0052bd7f  8b4008                MOV EAX,dword ptr [EAX + 0x8]
0052bd82  894108                MOV dword ptr [ECX + 0x8],EAX
0052bd85  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
0052bd8b  6bc94c                IMUL ECX,ECX,0x4c
0052bd8e  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
0052bd94  038a54020000          ADD ECX,dword ptr [EDX + 0x254]
0052bd9a  e8b13b0d00            CALL 0x005ff950
0052bd9f  85c0                  TEST EAX,EAX
0052bda1  0f8584000000          JNZ 0x0052be2b
0052bda7  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0052bdaa  50                    PUSH EAX
0052bdab  8d8d78ffffff          LEA ECX,[EBP + 0xffffff78]
0052bdb1  51                    PUSH ECX
0052bdb2  8d9568ffffff          LEA EDX,[EBP + 0xffffff68]
0052bdb8  52                    PUSH EDX
0052bdb9  e8f294f1ff            CALL 0x004452b0
0052bdbe  83c40c                ADD ESP,0xc
0052bdc1  d98570ffffff          FLD float ptr [EBP + 0xffffff70]
0052bdc7  d88d70ffffff          FMUL float ptr [EBP + 0xffffff70]
0052bdcd  d9856cffffff          FLD float ptr [EBP + 0xffffff6c]
0052bdd3  d88d6cffffff          FMUL float ptr [EBP + 0xffffff6c]
0052bdd9  d98568ffffff          FLD float ptr [EBP + 0xffffff68]
0052bddf  d88d68ffffff          FMUL float ptr [EBP + 0xffffff68]
0052bde5  dec1                  FADDP
0052bde7  dec1                  FADDP
0052bde9  d99d64ffffff          FSTP float ptr [EBP + 0xffffff64]
0052bdef  d98564ffffff          FLD float ptr [EBP + 0xffffff64]
0052bdf5  d945f0                FLD float ptr [EBP + -0x10]
0052bdf8  ded9                  FCOMPP
0052bdfa  dfe0                  FNSTSW AX
0052bdfc  f6c441                TEST AH,0x41
0052bdff  752a                  JNZ 0x0052be2b
0052be01  d98564ffffff          FLD float ptr [EBP + 0xffffff64]
0052be07  d95df0                FSTP float ptr [EBP + -0x10]
0052be0a  8b4510                MOV EAX,dword ptr [EBP + 0x10]
0052be0d  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
0052be13  8908                  MOV dword ptr [EAX],ECX
0052be15  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
0052be1b  895004                MOV dword ptr [EAX + 0x4],EDX
0052be1e  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
0052be21  894808                MOV dword ptr [EAX + 0x8],ECX
0052be24  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
0052be2b  e9f3feffff            JMP 0x0052bd23
0052be30  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
0052be33  899560ffffff          MOV dword ptr [EBP + 0xffffff60],EDX
0052be39  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0052be40  8d4de4                LEA ECX,[EBP + -0x1c]
0052be43  e8681f1200            CALL 0x0064ddb0
0052be48  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
0052be4e  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0052be51  64890d00000000        MOV dword ptr FS:[0x0],ECX
0052be58  59                    POP ECX
0052be59  8be5                  MOV ESP,EBP
0052be5b  5d                    POP EBP
0052be5c  c22000                RET 0x20
