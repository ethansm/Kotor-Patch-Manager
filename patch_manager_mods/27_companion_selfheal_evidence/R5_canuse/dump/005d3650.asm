; FN 005d3650 FUN_005d3650
005d3650  55                    PUSH EBP
005d3651  8bec                  MOV EBP,ESP
005d3653  81ec9c000000          SUB ESP,0x9c
005d3659  898d70ffffff          MOV dword ptr [EBP + 0xffffff70],ECX
005d365f  c745f001000000        MOV dword ptr [EBP + -0x10],0x1
005d3666  6a00                  PUSH 0x0
005d3668  6a0a                  PUSH 0xa
005d366a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d366d  e8ee080300            CALL 0x00603f60
005d3672  85c0                  TEST EAX,EAX
005d3674  7427                  JZ 0x005d369d
005d3676  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
005d367d  6881000000            PUSH 0x81
005d3682  6a0a                  PUSH 0xa
005d3684  8d45ec                LEA EAX,[EBP + -0x14]
005d3687  50                    PUSH EAX
005d3688  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d368b  e8500c0300            CALL 0x006042e0
005d3690  837dec00              CMP dword ptr [EBP + -0x14],0x0
005d3694  7407                  JZ 0x005d369d
005d3696  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
005d369d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d36a0  e88b371000            CALL 0x006d6e30
005d36a5  8945e4                MOV dword ptr [EBP + -0x1c],EAX
005d36a8  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
005d36ab  8a91b4000000          MOV DL,byte ptr [ECX + 0xb4]
005d36b1  8855ff                MOV byte ptr [EBP + -0x1],DL
005d36b4  0fb645ff              MOVZX EAX,byte ptr [EBP + -0x1]
005d36b8  85c0                  TEST EAX,EAX
005d36ba  0f84bf000000          JZ 0x005d377f
005d36c0  837df000              CMP dword ptr [EBP + -0x10],0x0
005d36c4  0f84b5000000          JZ 0x005d377f
005d36ca  8a4dff                MOV CL,byte ptr [EBP + -0x1]
005d36cd  888d6cffffff          MOV byte ptr [EBP + 0xffffff6c],CL
005d36d3  80bd6cffffff01        CMP byte ptr [EBP + 0xffffff6c],0x1
005d36da  740e                  JZ 0x005d36ea
005d36dc  80bd6cffffff02        CMP byte ptr [EBP + 0xffffff6c],0x2
005d36e3  744c                  JZ 0x005d3731
005d36e5  e98e000000            JMP 0x005d3778
005d36ea  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
005d36f0  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005d36f6  8945e0                MOV dword ptr [EBP + -0x20],EAX
005d36f9  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005d36fc  668b91e0000000        MOV DX,word ptr [ECX + 0xe0]
005d3703  668955de              MOV word ptr [EBP + -0x22],DX
005d3707  0fb745de              MOVZX EAX,word ptr [EBP + -0x22]
005d370b  0fb70dbca89900        MOVZX ECX,word ptr [0x0099a8bc]
005d3712  3bc1                  CMP EAX,ECX
005d3714  7419                  JZ 0x005d372f
005d3716  837d0c00              CMP dword ptr [EBP + 0xc],0x0
005d371a  7409                  JZ 0x005d3725
005d371c  33c0                  XOR EAX,EAX
005d371e  e9f5020000            JMP 0x005d3a18
005d3725  b801000000            MOV EAX,0x1
005d372a  e9e9020000            JMP 0x005d3a18
005d372f  eb4e                  JMP 0x005d377f
005d3731  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
005d3737  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005d373d  8945d8                MOV dword ptr [EBP + -0x28],EAX
005d3740  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
005d3743  668b91e0000000        MOV DX,word ptr [ECX + 0xe0]
005d374a  668955d6              MOV word ptr [EBP + -0x2a],DX
005d374e  0fb745d6              MOVZX EAX,word ptr [EBP + -0x2a]
005d3752  0fb70db8a89900        MOVZX ECX,word ptr [0x0099a8b8]
005d3759  3bc1                  CMP EAX,ECX
005d375b  7419                  JZ 0x005d3776
005d375d  837d0c00              CMP dword ptr [EBP + 0xc],0x0
005d3761  7409                  JZ 0x005d376c
005d3763  33c0                  XOR EAX,EAX
005d3765  e9ae020000            JMP 0x005d3a18
005d376c  b801000000            MOV EAX,0x1
005d3771  e9a2020000            JMP 0x005d3a18
005d3776  eb07                  JMP 0x005d377f
005d3778  33c0                  XOR EAX,EAX
005d377a  e999020000            JMP 0x005d3a18
005d377f  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
005d3785  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005d378b  8945d0                MOV dword ptr [EBP + -0x30],EAX
005d378e  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
005d3791  8a91ec000000          MOV DL,byte ptr [ECX + 0xec]
005d3797  8855cf                MOV byte ptr [EBP + -0x31],DL
005d379a  0fb64dcf              MOVZX ECX,byte ptr [EBP + -0x31]
005d379e  b801000000            MOV EAX,0x1
005d37a3  d3e0                  SHL EAX,CL
005d37a5  8945f4                MOV dword ptr [EBP + -0xc],EAX
005d37a8  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d37ab  e880361000            CALL 0x006d6e30
005d37b0  8945c8                MOV dword ptr [EBP + -0x38],EAX
005d37b3  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
005d37b6  8b91b8000000          MOV EDX,dword ptr [ECX + 0xb8]
005d37bc  8955f8                MOV dword ptr [EBP + -0x8],EDX
005d37bf  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
005d37c2  2345f4                AND EAX,dword ptr [EBP + -0xc]
005d37c5  7407                  JZ 0x005d37ce
005d37c7  33c0                  XOR EAX,EAX
005d37c9  e94a020000            JMP 0x005d3a18
005d37ce  6a00                  PUSH 0x0
005d37d0  6a2d                  PUSH 0x2d
005d37d2  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d37d5  e886070300            CALL 0x00603f60
005d37da  85c0                  TEST EAX,EAX
005d37dc  751a                  JNZ 0x005d37f8
005d37de  6a00                  PUSH 0x0
005d37e0  6a3d                  PUSH 0x3d
005d37e2  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d37e5  e876070300            CALL 0x00603f60
005d37ea  85c0                  TEST EAX,EAX
005d37ec  750a                  JNZ 0x005d37f8
005d37ee  b801000000            MOV EAX,0x1
005d37f3  e920020000            JMP 0x005d3a18
005d37f8  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
005d37ff  eb09                  JMP 0x005d380a
005d3801  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d3804  83c101                ADD ECX,0x1
005d3807  894de8                MOV dword ptr [EBP + -0x18],ECX
005d380a  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d380d  8b828c020000          MOV EAX,dword ptr [EDX + 0x28c]
005d3813  8945c4                MOV dword ptr [EBP + -0x3c],EAX
005d3816  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d3819  3b4dc4                CMP ECX,dword ptr [EBP + -0x3c]
005d381c  0f8df4010000          JGE 0x005d3a16
005d3822  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005d3825  52                    PUSH EDX
005d3826  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3829  e8720a0300            CALL 0x006042a0
005d382e  8945c0                MOV dword ptr [EBP + -0x40],EAX
005d3831  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
005d3834  8a4811                MOV CL,byte ptr [EAX + 0x11]
005d3837  884dbf                MOV byte ptr [EBP + -0x41],CL
005d383a  0fb655bf              MOVZX EDX,byte ptr [EBP + -0x41]
005d383e  81faff000000          CMP EDX,0xff
005d3844  7422                  JZ 0x005d3868
005d3846  0fb64dbf              MOVZX ECX,byte ptr [EBP + -0x41]
005d384a  b801000000            MOV EAX,0x1
005d384f  d3e0                  SHL EAX,CL
005d3851  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3854  2381d4020000          AND EAX,dword ptr [ECX + 0x2d4]
005d385a  750c                  JNZ 0x005d3868
005d385c  c78568ffffff00000000  MOV dword ptr [EBP + 0xffffff68],0x0
005d3866  eb0a                  JMP 0x005d3872
005d3868  c78568ffffff01000000  MOV dword ptr [EBP + 0xffffff68],0x1
005d3872  83bd68ffffff00        CMP dword ptr [EBP + 0xffffff68],0x0
005d3879  747e                  JZ 0x005d38f9
005d387b  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005d387e  52                    PUSH EDX
005d387f  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3882  e8190a0300            CALL 0x006042a0
005d3887  8945b8                MOV dword ptr [EBP + -0x48],EAX
005d388a  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
005d388d  668b08                MOV CX,word ptr [EAX]
005d3890  66894db6              MOV word ptr [EBP + -0x4a],CX
005d3894  0fb755b6              MOVZX EDX,word ptr [EBP + -0x4a]
005d3898  83fa2d                CMP EDX,0x2d
005d389b  755c                  JNZ 0x005d38f9
005d389d  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005d38a0  50                    PUSH EAX
005d38a1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d38a4  e8f7090300            CALL 0x006042a0
005d38a9  8945b0                MOV dword ptr [EBP + -0x50],EAX
005d38ac  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
005d38b2  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005d38b8  8955ac                MOV dword ptr [EBP + -0x54],EDX
005d38bb  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
005d38be  668b88e0000000        MOV CX,word ptr [EAX + 0xe0]
005d38c5  66894daa              MOV word ptr [EBP + -0x56],CX
005d38c9  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005d38cc  52                    PUSH EDX
005d38cd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d38d0  e8cb090300            CALL 0x006042a0
005d38d5  8945a4                MOV dword ptr [EBP + -0x5c],EAX
005d38d8  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
005d38db  668b4802              MOV CX,word ptr [EAX + 0x2]
005d38df  66894da2              MOV word ptr [EBP + -0x5e],CX
005d38e3  0fb755aa              MOVZX EDX,word ptr [EBP + -0x56]
005d38e7  0fb745a2              MOVZX EAX,word ptr [EBP + -0x5e]
005d38eb  3bd0                  CMP EDX,EAX
005d38ed  750a                  JNZ 0x005d38f9
005d38ef  b801000000            MOV EAX,0x1
005d38f4  e91f010000            JMP 0x005d3a18
005d38f9  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d38fc  51                    PUSH ECX
005d38fd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3900  e89b090300            CALL 0x006042a0
005d3905  89459c                MOV dword ptr [EBP + -0x64],EAX
005d3908  8b559c                MOV EDX,dword ptr [EBP + -0x64]
005d390b  8a4211                MOV AL,byte ptr [EDX + 0x11]
005d390e  88459b                MOV byte ptr [EBP + -0x65],AL
005d3911  0fb64d9b              MOVZX ECX,byte ptr [EBP + -0x65]
005d3915  81f9ff000000          CMP ECX,0xff
005d391b  7422                  JZ 0x005d393f
005d391d  0fb64d9b              MOVZX ECX,byte ptr [EBP + -0x65]
005d3921  ba01000000            MOV EDX,0x1
005d3926  d3e2                  SHL EDX,CL
005d3928  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d392b  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
005d3931  750c                  JNZ 0x005d393f
005d3933  c78564ffffff00000000  MOV dword ptr [EBP + 0xffffff64],0x0
005d393d  eb0a                  JMP 0x005d3949
005d393f  c78564ffffff01000000  MOV dword ptr [EBP + 0xffffff64],0x1
005d3949  83bd64ffffff00        CMP dword ptr [EBP + 0xffffff64],0x0
005d3950  0f84bb000000          JZ 0x005d3a11
005d3956  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d3959  51                    PUSH ECX
005d395a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d395d  e83e090300            CALL 0x006042a0
005d3962  894594                MOV dword ptr [EBP + -0x6c],EAX
005d3965  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
005d3968  668b02                MOV AX,word ptr [EDX]
005d396b  66894592              MOV word ptr [EBP + -0x6e],AX
005d396f  0fb74d92              MOVZX ECX,word ptr [EBP + -0x6e]
005d3973  83f93d                CMP ECX,0x3d
005d3976  0f8595000000          JNZ 0x005d3a11
005d397c  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005d397f  52                    PUSH EDX
005d3980  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3983  e818090300            CALL 0x006042a0
005d3988  89458c                MOV dword ptr [EBP + -0x74],EAX
005d398b  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
005d3991  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d3997  894d88                MOV dword ptr [EBP + -0x78],ECX
005d399a  8b5588                MOV EDX,dword ptr [EBP + -0x78]
005d399d  668b82e0000000        MOV AX,word ptr [EDX + 0xe0]
005d39a4  66894586              MOV word ptr [EBP + -0x7a],AX
005d39a8  0fb74d86              MOVZX ECX,word ptr [EBP + -0x7a]
005d39ac  0fb715b8a89900        MOVZX EDX,word ptr [0x0099a8b8]
005d39b3  3bca                  CMP ECX,EDX
005d39b5  745a                  JZ 0x005d3a11
005d39b7  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
005d39bd  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d39c3  894d80                MOV dword ptr [EBP + -0x80],ECX
005d39c6  8b5580                MOV EDX,dword ptr [EBP + -0x80]
005d39c9  8a82ec000000          MOV AL,byte ptr [EDX + 0xec]
005d39cf  88857fffffff          MOV byte ptr [EBP + 0xffffff7f],AL
005d39d5  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005d39d8  51                    PUSH ECX
005d39d9  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d39dc  e8bf080300            CALL 0x006042a0
005d39e1  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
005d39e7  8b9578ffffff          MOV EDX,dword ptr [EBP + 0xffffff78]
005d39ed  668b4202              MOV AX,word ptr [EDX + 0x2]
005d39f1  66898576ffffff        MOV word ptr [EBP + 0xffffff76],AX
005d39f8  0fb68d7fffffff        MOVZX ECX,byte ptr [EBP + 0xffffff7f]
005d39ff  0fb79576ffffff        MOVZX EDX,word ptr [EBP + 0xffffff76]
005d3a06  3bca                  CMP ECX,EDX
005d3a08  7507                  JNZ 0x005d3a11
005d3a0a  b801000000            MOV EAX,0x1
005d3a0f  eb07                  JMP 0x005d3a18
005d3a11  e9ebfdffff            JMP 0x005d3801
005d3a16  33c0                  XOR EAX,EAX
005d3a18  8be5                  MOV ESP,EBP
005d3a1a  5d                    POP EBP
005d3a1b  c20800                RET 0x8
