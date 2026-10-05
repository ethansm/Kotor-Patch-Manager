; FN 005c7370 FUN_005c7370
005c7370  55                    PUSH EBP
005c7371  8bec                  MOV EBP,ESP
005c7373  81ec0c010000          SUB ESP,0x10c
005c7379  898df4feffff          MOV dword ptr [EBP + 0xfffffef4],ECX
005c737f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005c7384  8b4804                MOV ECX,dword ptr [EAX + 0x4]
005c7387  898d74ffffff          MOV dword ptr [EBP + 0xffffff74],ECX
005c738d  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
005c7393  e8f8871700            CALL 0x0073fb90
005c7398  8bc8                  MOV ECX,EAX
005c739a  e881132200            CALL 0x007e8720
005c739f  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005c73a2  83c228                ADD EDX,0x28
005c73a5  8b02                  MOV EAX,dword ptr [EDX]
005c73a7  8945d4                MOV dword ptr [EBP + -0x2c],EAX
005c73aa  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
005c73ad  894dd8                MOV dword ptr [EBP + -0x28],ECX
005c73b0  8b5208                MOV EDX,dword ptr [EDX + 0x8]
005c73b3  8955dc                MOV dword ptr [EBP + -0x24],EDX
005c73b6  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005c73b9  8b4834                MOV ECX,dword ptr [EAX + 0x34]
005c73bc  894dc0                MOV dword ptr [EBP + -0x40],ECX
005c73bf  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
005c73c2  52                    PUSH EDX
005c73c3  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
005c73c6  e815162200            CALL 0x007e89e0
005c73cb  898570ffffff          MOV dword ptr [EBP + 0xffffff70],EAX
005c73d1  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
005c73d7  d9400c                FLD float ptr [EAX + 0xc]
005c73da  d95db8                FSTP float ptr [EBP + -0x48]
005c73dd  d9ee                  FLDZ
005c73df  d95d9c                FSTP float ptr [EBP + -0x64]
005c73e2  d9ee                  FLDZ
005c73e4  d95da0                FSTP float ptr [EBP + -0x60]
005c73e7  d9e8                  FLD1
005c73e9  d95da4                FSTP float ptr [EBP + -0x5c]
005c73ec  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005c73ef  83c138                ADD ECX,0x38
005c73f2  898d38ffffff          MOV dword ptr [EBP + 0xffffff38],ECX
005c73f8  51                    PUSH ECX
005c73f9  d945b8                FLD float ptr [EBP + -0x48]
005c73fc  d91c24                FSTP float ptr [ESP]
005c73ff  e8bcc8f4ff            CALL 0x00513cc0
005c7404  d91c24                FSTP float ptr [ESP]
005c7407  83ec0c                SUB ESP,0xc
005c740a  8bd4                  MOV EDX,ESP
005c740c  8b459c                MOV EAX,dword ptr [EBP + -0x64]
005c740f  8902                  MOV dword ptr [EDX],EAX
005c7411  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
005c7414  894a04                MOV dword ptr [EDX + 0x4],ECX
005c7417  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
005c741a  894208                MOV dword ptr [EDX + 0x8],EAX
005c741d  8d4da8                LEA ECX,[EBP + -0x58]
005c7420  e87b21f5ff            CALL 0x005195a0
005c7425  89853cffffff          MOV dword ptr [EBP + 0xffffff3c],EAX
005c742b  833d90f1a00000        CMP dword ptr [0x00a0f190],0x0
005c7432  744f                  JZ 0x005c7483
005c7434  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
005c743a  d901                  FLD float ptr [ECX]
005c743c  dc1d506b9800          FCOMP double ptr [0x00986b50]
005c7442  dfe0                  FNSTSW AX
005c7444  f6c444                TEST AH,0x44
005c7447  7b15                  JNP 0x005c745e
005c7449  8b953cffffff          MOV EDX,dword ptr [EBP + 0xffffff3c]
005c744f  d902                  FLD float ptr [EDX]
005c7451  dc1d986d9800          FCOMP double ptr [0x00986d98]
005c7457  dfe0                  FNSTSW AX
005c7459  f6c444                TEST AH,0x44
005c745c  7a25                  JP 0x005c7483
005c745e  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
005c7464  8b08                  MOV ECX,dword ptr [EAX]
005c7466  894d90                MOV dword ptr [EBP + -0x70],ECX
005c7469  8b5004                MOV EDX,dword ptr [EAX + 0x4]
005c746c  895594                MOV dword ptr [EBP + -0x6c],EDX
005c746f  8b4008                MOV EAX,dword ptr [EAX + 0x8]
005c7472  894598                MOV dword ptr [EBP + -0x68],EAX
005c7475  8d4d90                LEA ECX,[EBP + -0x70]
005c7478  898d34ffffff          MOV dword ptr [EBP + 0xffffff34],ECX
005c747e  e925020000            JMP 0x005c76a8
005c7483  8b953cffffff          MOV EDX,dword ptr [EBP + 0xffffff3c]
005c7489  d94204                FLD float ptr [EDX + 0x4]
005c748c  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
005c7492  d84804                FMUL float ptr [EAX + 0x4]
005c7495  d99d5cffffff          FSTP float ptr [EBP + 0xffffff5c]
005c749b  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
005c74a1  d94108                FLD float ptr [ECX + 0x8]
005c74a4  8b953cffffff          MOV EDX,dword ptr [EBP + 0xffffff3c]
005c74aa  d84a08                FMUL float ptr [EDX + 0x8]
005c74ad  d99d50ffffff          FSTP float ptr [EBP + 0xffffff50]
005c74b3  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
005c74b9  d9400c                FLD float ptr [EAX + 0xc]
005c74bc  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
005c74c2  d8490c                FMUL float ptr [ECX + 0xc]
005c74c5  d99d60ffffff          FSTP float ptr [EBP + 0xffffff60]
005c74cb  8b953cffffff          MOV EDX,dword ptr [EBP + 0xffffff3c]
005c74d1  d94204                FLD float ptr [EDX + 0x4]
005c74d4  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
005c74da  d84808                FMUL float ptr [EAX + 0x8]
005c74dd  d99d6cffffff          FSTP float ptr [EBP + 0xffffff6c]
005c74e3  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
005c74e9  d94104                FLD float ptr [ECX + 0x4]
005c74ec  8b953cffffff          MOV EDX,dword ptr [EBP + 0xffffff3c]
005c74f2  d84a0c                FMUL float ptr [EDX + 0xc]
005c74f5  d99d4cffffff          FSTP float ptr [EBP + 0xffffff4c]
005c74fb  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
005c7501  d94004                FLD float ptr [EAX + 0x4]
005c7504  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
005c750a  d809                  FMUL float ptr [ECX]
005c750c  d99d58ffffff          FSTP float ptr [EBP + 0xffffff58]
005c7512  8b953cffffff          MOV EDX,dword ptr [EBP + 0xffffff3c]
005c7518  d94208                FLD float ptr [EDX + 0x8]
005c751b  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
005c7521  d8480c                FMUL float ptr [EAX + 0xc]
005c7524  d99d64ffffff          FSTP float ptr [EBP + 0xffffff64]
005c752a  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
005c7530  d94108                FLD float ptr [ECX + 0x8]
005c7533  8b953cffffff          MOV EDX,dword ptr [EBP + 0xffffff3c]
005c7539  d80a                  FMUL float ptr [EDX]
005c753b  d99d68ffffff          FSTP float ptr [EBP + 0xffffff68]
005c7541  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
005c7547  d9400c                FLD float ptr [EAX + 0xc]
005c754a  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
005c7550  d809                  FMUL float ptr [ECX]
005c7552  d99d54ffffff          FSTP float ptr [EBP + 0xffffff54]
005c7558  8b9538ffffff          MOV EDX,dword ptr [EBP + 0xffffff38]
005c755e  d94208                FLD float ptr [EDX + 0x8]
005c7561  d98568ffffff          FLD float ptr [EBP + 0xffffff68]
005c7567  d8854cffffff          FADD float ptr [EBP + 0xffffff4c]
005c756d  dc0d90749800          FMUL double ptr [0x00987490]
005c7573  dec9                  FMULP
005c7575  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
005c757b  d94004                FLD float ptr [EAX + 0x4]
005c757e  d98554ffffff          FLD float ptr [EBP + 0xffffff54]
005c7584  d8ad6cffffff          FSUBR float ptr [EBP + 0xffffff6c]
005c758a  dc0d90749800          FMUL double ptr [0x00987490]
005c7590  dec9                  FMULP
005c7592  8b8d38ffffff          MOV ECX,dword ptr [EBP + 0xffffff38]
005c7598  d901                  FLD float ptr [ECX]
005c759a  d98560ffffff          FLD float ptr [EBP + 0xffffff60]
005c75a0  d88550ffffff          FADD float ptr [EBP + 0xffffff50]
005c75a6  dc0d90749800          FMUL double ptr [0x00987490]
005c75ac  d9e8                  FLD1
005c75ae  dee1                  FSUBRP
005c75b0  dec9                  FMULP
005c75b2  dec1                  FADDP
005c75b4  dec1                  FADDP
005c75b6  d99d40ffffff          FSTP float ptr [EBP + 0xffffff40]
005c75bc  8b9538ffffff          MOV EDX,dword ptr [EBP + 0xffffff38]
005c75c2  d94208                FLD float ptr [EDX + 0x8]
005c75c5  d98558ffffff          FLD float ptr [EBP + 0xffffff58]
005c75cb  d8ad64ffffff          FSUBR float ptr [EBP + 0xffffff64]
005c75d1  dc0d90749800          FMUL double ptr [0x00987490]
005c75d7  dec9                  FMULP
005c75d9  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
005c75df  d94004                FLD float ptr [EAX + 0x4]
005c75e2  d98560ffffff          FLD float ptr [EBP + 0xffffff60]
005c75e8  d8855cffffff          FADD float ptr [EBP + 0xffffff5c]
005c75ee  dc0d90749800          FMUL double ptr [0x00987490]
005c75f4  d9e8                  FLD1
005c75f6  dee1                  FSUBRP
005c75f8  dec9                  FMULP
005c75fa  8b8d38ffffff          MOV ECX,dword ptr [EBP + 0xffffff38]
005c7600  d901                  FLD float ptr [ECX]
005c7602  d98554ffffff          FLD float ptr [EBP + 0xffffff54]
005c7608  d8856cffffff          FADD float ptr [EBP + 0xffffff6c]
005c760e  dc0d90749800          FMUL double ptr [0x00987490]
005c7614  dec9                  FMULP
005c7616  dec1                  FADDP
005c7618  dec1                  FADDP
005c761a  d99d44ffffff          FSTP float ptr [EBP + 0xffffff44]
005c7620  8b9538ffffff          MOV EDX,dword ptr [EBP + 0xffffff38]
005c7626  d94208                FLD float ptr [EDX + 0x8]
005c7629  d98550ffffff          FLD float ptr [EBP + 0xffffff50]
005c762f  d8855cffffff          FADD float ptr [EBP + 0xffffff5c]
005c7635  dc0d90749800          FMUL double ptr [0x00987490]
005c763b  d9e8                  FLD1
005c763d  dee1                  FSUBRP
005c763f  dec9                  FMULP
005c7641  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
005c7647  d94004                FLD float ptr [EAX + 0x4]
005c764a  d98558ffffff          FLD float ptr [EBP + 0xffffff58]
005c7650  d88564ffffff          FADD float ptr [EBP + 0xffffff64]
005c7656  dc0d90749800          FMUL double ptr [0x00987490]
005c765c  dec9                  FMULP
005c765e  8b8d38ffffff          MOV ECX,dword ptr [EBP + 0xffffff38]
005c7664  d901                  FLD float ptr [ECX]
005c7666  d98568ffffff          FLD float ptr [EBP + 0xffffff68]
005c766c  d8ad4cffffff          FSUBR float ptr [EBP + 0xffffff4c]
005c7672  dc0d90749800          FMUL double ptr [0x00987490]
005c7678  dec9                  FMULP
005c767a  dec1                  FADDP
005c767c  dec1                  FADDP
005c767e  d99d48ffffff          FSTP float ptr [EBP + 0xffffff48]
005c7684  d98540ffffff          FLD float ptr [EBP + 0xffffff40]
005c768a  d95d90                FSTP float ptr [EBP + -0x70]
005c768d  d98544ffffff          FLD float ptr [EBP + 0xffffff44]
005c7693  d95d94                FSTP float ptr [EBP + -0x6c]
005c7696  d98548ffffff          FLD float ptr [EBP + 0xffffff48]
005c769c  d95d98                FSTP float ptr [EBP + -0x68]
005c769f  8d5590                LEA EDX,[EBP + -0x70]
005c76a2  899534ffffff          MOV dword ptr [EBP + 0xffffff34],EDX
005c76a8  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
005c76ae  50                    PUSH EAX
005c76af  8d4dd4                LEA ECX,[EBP + -0x2c]
005c76b2  51                    PUSH ECX
005c76b3  8d55e4                LEA EDX,[EBP + -0x1c]
005c76b6  52                    PUSH EDX
005c76b7  e8e451e8ff            CALL 0x0044c8a0
005c76bc  83c40c                ADD ESP,0xc
005c76bf  6a00                  PUSH 0x0
005c76c1  6a00                  PUSH 0x0
005c76c3  6a01                  PUSH 0x1
005c76c5  8d45e4                LEA EAX,[EBP + -0x1c]
005c76c8  50                    PUSH EAX
005c76c9  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c76cf  e8ecdcf7ff            CALL 0x005453c0
005c76d4  8bc8                  MOV ECX,EAX
005c76d6  e8553af8ff            CALL 0x0054b130
005c76db  d95dec                FSTP float ptr [EBP + -0x14]
005c76de  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005c76e1  83c118                ADD ECX,0x18
005c76e4  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
005c76e7  8911                  MOV dword ptr [ECX],EDX
005c76e9  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
005c76ec  894104                MOV dword ptr [ECX + 0x4],EAX
005c76ef  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
005c76f2  895108                MOV dword ptr [ECX + 0x8],EDX
005c76f5  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005c76f8  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
005c76fb  894824                MOV dword ptr [EAX + 0x24],ECX
005c76fe  8b95f4feffff          MOV EDX,dword ptr [EBP + 0xfffffef4]
005c7704  8b8280030000          MOV EAX,dword ptr [EDX + 0x380]
005c770a  898518ffffff          MOV dword ptr [EBP + 0xffffff18],EAX
005c7710  8b8d18ffffff          MOV ECX,dword ptr [EBP + 0xffffff18]
005c7716  c781540200000000007f  MOV dword ptr [ECX + 0x254],0x7f000000
005c7720  ba0000007f            MOV EDX,0x7f000000
005c7725  81fa0000007f          CMP EDX,0x7f000000
005c772b  753d                  JNZ 0x005c776a
005c772d  d9ee                  FLDZ
005c772f  d99d1cffffff          FSTP float ptr [EBP + 0xffffff1c]
005c7735  d9ee                  FLDZ
005c7737  d99d20ffffff          FSTP float ptr [EBP + 0xffffff20]
005c773d  d9ee                  FLDZ
005c773f  d99d24ffffff          FSTP float ptr [EBP + 0xffffff24]
005c7745  8b8518ffffff          MOV EAX,dword ptr [EBP + 0xffffff18]
005c774b  0558020000            ADD EAX,0x258
005c7750  8b8d1cffffff          MOV ECX,dword ptr [EBP + 0xffffff1c]
005c7756  8908                  MOV dword ptr [EAX],ECX
005c7758  8b9520ffffff          MOV EDX,dword ptr [EBP + 0xffffff20]
005c775e  895004                MOV dword ptr [EAX + 0x4],EDX
005c7761  8b8d24ffffff          MOV ECX,dword ptr [EBP + 0xffffff24]
005c7767  894808                MOV dword ptr [EAX + 0x8],ECX
005c776a  c745f001000000        MOV dword ptr [EBP + -0x10],0x1
005c7771  d90514c09800          FLD float ptr [0x0098c014]
005c7777  d95dc4                FSTP float ptr [EBP + -0x3c]
005c777a  c745d00000007f        MOV dword ptr [EBP + -0x30],0x7f000000
005c7781  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c7787  e834dcf7ff            CALL 0x005453c0
005c778c  0520010000            ADD EAX,0x120
005c7791  898514ffffff          MOV dword ptr [EBP + 0xffffff14],EAX
005c7797  8b9514ffffff          MOV EDX,dword ptr [EBP + 0xffffff14]
005c779d  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005c77a0  898510ffffff          MOV dword ptr [EBP + 0xffffff10],EAX
005c77a6  8b8d10ffffff          MOV ECX,dword ptr [EBP + 0xffffff10]
005c77ac  894dc8                MOV dword ptr [EBP + -0x38],ECX
005c77af  8b95f4feffff          MOV EDX,dword ptr [EBP + 0xfffffef4]
005c77b5  8b82ec000000          MOV EAX,dword ptr [EDX + 0xec]
005c77bb  8945e0                MOV dword ptr [EBP + -0x20],EAX
005c77be  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c77c4  c781ec00000001000000  MOV dword ptr [ECX + 0xec],0x1
005c77ce  6a3d                  PUSH 0x3d
005c77d0  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c77d6  e845d90000            CALL 0x005d5120
005c77db  8945cc                MOV dword ptr [EBP + -0x34],EAX
005c77de  c745fcffff0000        MOV dword ptr [EBP + -0x4],0xffff
005c77e5  837dcc00              CMP dword ptr [EBP + -0x34],0x0
005c77e9  740a                  JZ 0x005c77f5
005c77eb  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
005c77ee  0fb7426c              MOVZX EAX,word ptr [EDX + 0x6c]
005c77f2  8945fc                MOV dword ptr [EBP + -0x4],EAX
005c77f5  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c77fb  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
005c7801  52                    PUSH EDX
005c7802  8d45e4                LEA EAX,[EBP + -0x1c]
005c7805  50                    PUSH EAX
005c7806  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c780c  e8afdbf7ff            CALL 0x005453c0
005c7811  8bc8                  MOV ECX,EAX
005c7813  e8383ef8ff            CALL 0x0054b650
005c7818  85c0                  TEST EAX,EAX
005c781a  0f84dc000000          JZ 0x005c78fc
005c7820  6a00                  PUSH 0x0
005c7822  6a00                  PUSH 0x0
005c7824  6a00                  PUSH 0x0
005c7826  6a00                  PUSH 0x0
005c7828  6a00                  PUSH 0x0
005c782a  6a00                  PUSH 0x0
005c782c  6a00                  PUSH 0x0
005c782e  6a00                  PUSH 0x0
005c7830  6a00                  PUSH 0x0
005c7832  6a00                  PUSH 0x0
005c7834  6a00                  PUSH 0x0
005c7836  6a00                  PUSH 0x0
005c7838  8d4dc4                LEA ECX,[EBP + -0x3c]
005c783b  51                    PUSH ECX
005c783c  6a02                  PUSH 0x2
005c783e  8d55f0                LEA EDX,[EBP + -0x10]
005c7841  52                    PUSH EDX
005c7842  6a01                  PUSH 0x1
005c7844  8d45d0                LEA EAX,[EBP + -0x30]
005c7847  50                    PUSH EAX
005c7848  6a03                  PUSH 0x3
005c784a  8d4dc8                LEA ECX,[EBP + -0x38]
005c784d  51                    PUSH ECX
005c784e  6a03                  PUSH 0x3
005c7850  8d55ec                LEA EDX,[EBP + -0x14]
005c7853  52                    PUSH EDX
005c7854  6a02                  PUSH 0x2
005c7856  8d45e8                LEA EAX,[EBP + -0x18]
005c7859  50                    PUSH EAX
005c785a  6a02                  PUSH 0x2
005c785c  8d4de4                LEA ECX,[EBP + -0x1c]
005c785f  51                    PUSH ECX
005c7860  6a02                  PUSH 0x2
005c7862  0fb755fc              MOVZX EDX,word ptr [EBP + -0x4]
005c7866  52                    PUSH EDX
005c7867  6a40                  PUSH 0x40
005c7869  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c786f  e82c94f7ff            CALL 0x00540ca0
005c7874  d9ee                  FLDZ
005c7876  d95d84                FSTP float ptr [EBP + -0x7c]
005c7879  d9ee                  FLDZ
005c787b  d95d88                FSTP float ptr [EBP + -0x78]
005c787e  d9ee                  FLDZ
005c7880  d95d8c                FSTP float ptr [EBP + -0x74]
005c7883  8b85f4feffff          MOV EAX,dword ptr [EBP + 0xfffffef4]
005c7889  8b8890000000          MOV ECX,dword ptr [EAX + 0x90]
005c788f  898d0cffffff          MOV dword ptr [EBP + 0xffffff0c],ECX
005c7895  6a00                  PUSH 0x0
005c7897  6a00                  PUSH 0x0
005c7899  6a00                  PUSH 0x0
005c789b  8d5584                LEA EDX,[EBP + -0x7c]
005c789e  52                    PUSH EDX
005c789f  6a00                  PUSH 0x0
005c78a1  6a00                  PUSH 0x0
005c78a3  6a00                  PUSH 0x0
005c78a5  6a00                  PUSH 0x0
005c78a7  6a00                  PUSH 0x0
005c78a9  6a00                  PUSH 0x0
005c78ab  51                    PUSH ECX
005c78ac  d905f0199900          FLD float ptr [0x009919f0]
005c78b2  d91c24                FSTP float ptr [ESP]
005c78b5  51                    PUSH ECX
005c78b6  d9ee                  FLDZ
005c78b8  d91c24                FSTP float ptr [ESP]
005c78bb  6a01                  PUSH 0x1
005c78bd  680000007f            PUSH 0x7f000000
005c78c2  8b850cffffff          MOV EAX,dword ptr [EBP + 0xffffff0c]
005c78c8  50                    PUSH EAX
005c78c9  8d4de4                LEA ECX,[EBP + -0x1c]
005c78cc  51                    PUSH ECX
005c78cd  68feff0000            PUSH 0xfffe
005c78d2  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c78d8  e8d3cbf9ff            CALL 0x005644b0
005c78dd  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005c78e0  c7420406000000        MOV dword ptr [EDX + 0x4],0x6
005c78e7  8b85f4feffff          MOV EAX,dword ptr [EBP + 0xfffffef4]
005c78ed  c780b0110000ffffffff  MOV dword ptr [EAX + 0x11b0],0xffffffff
005c78f7  e913010000            JMP 0x005c7a0f
005c78fc  8d4dd4                LEA ECX,[EBP + -0x2c]
005c78ff  51                    PUSH ECX
005c7900  6a00                  PUSH 0x0
005c7902  8b95f4feffff          MOV EDX,dword ptr [EBP + 0xfffffef4]
005c7908  8b8280030000          MOV EAX,dword ptr [EDX + 0x380]
005c790e  50                    PUSH EAX
005c790f  51                    PUSH ECX
005c7910  d90528be9800          FLD float ptr [0x0098be28]
005c7916  d91c24                FSTP float ptr [ESP]
005c7919  8d4dd4                LEA ECX,[EBP + -0x2c]
005c791c  51                    PUSH ECX
005c791d  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c7923  e898daf7ff            CALL 0x005453c0
005c7928  8bc8                  MOV ECX,EAX
005c792a  e89144f8ff            CALL 0x0054bdc0
005c792f  6a00                  PUSH 0x0
005c7931  6a00                  PUSH 0x0
005c7933  6a00                  PUSH 0x0
005c7935  6a00                  PUSH 0x0
005c7937  6a00                  PUSH 0x0
005c7939  6a00                  PUSH 0x0
005c793b  6a00                  PUSH 0x0
005c793d  6a00                  PUSH 0x0
005c793f  6a00                  PUSH 0x0
005c7941  6a00                  PUSH 0x0
005c7943  6a00                  PUSH 0x0
005c7945  6a00                  PUSH 0x0
005c7947  8d55c4                LEA EDX,[EBP + -0x3c]
005c794a  52                    PUSH EDX
005c794b  6a02                  PUSH 0x2
005c794d  8d45f0                LEA EAX,[EBP + -0x10]
005c7950  50                    PUSH EAX
005c7951  6a01                  PUSH 0x1
005c7953  8d4dd0                LEA ECX,[EBP + -0x30]
005c7956  51                    PUSH ECX
005c7957  6a03                  PUSH 0x3
005c7959  8d55c8                LEA EDX,[EBP + -0x38]
005c795c  52                    PUSH EDX
005c795d  6a03                  PUSH 0x3
005c795f  8d45dc                LEA EAX,[EBP + -0x24]
005c7962  50                    PUSH EAX
005c7963  6a02                  PUSH 0x2
005c7965  8d4dd8                LEA ECX,[EBP + -0x28]
005c7968  51                    PUSH ECX
005c7969  6a02                  PUSH 0x2
005c796b  8d55d4                LEA EDX,[EBP + -0x2c]
005c796e  52                    PUSH EDX
005c796f  6a02                  PUSH 0x2
005c7971  0fb745fc              MOVZX EAX,word ptr [EBP + -0x4]
005c7975  50                    PUSH EAX
005c7976  6a40                  PUSH 0x40
005c7978  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c797e  e81d93f7ff            CALL 0x00540ca0
005c7983  d9ee                  FLDZ
005c7985  d99d78ffffff          FSTP float ptr [EBP + 0xffffff78]
005c798b  d9ee                  FLDZ
005c798d  d99d7cffffff          FSTP float ptr [EBP + 0xffffff7c]
005c7993  d9ee                  FLDZ
005c7995  d95d80                FSTP float ptr [EBP + -0x80]
005c7998  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c799e  8b9190000000          MOV EDX,dword ptr [ECX + 0x90]
005c79a4  899508ffffff          MOV dword ptr [EBP + 0xffffff08],EDX
005c79aa  6a00                  PUSH 0x0
005c79ac  6a00                  PUSH 0x0
005c79ae  6a00                  PUSH 0x0
005c79b0  8d8578ffffff          LEA EAX,[EBP + 0xffffff78]
005c79b6  50                    PUSH EAX
005c79b7  6a00                  PUSH 0x0
005c79b9  6a00                  PUSH 0x0
005c79bb  6a00                  PUSH 0x0
005c79bd  6a00                  PUSH 0x0
005c79bf  6a00                  PUSH 0x0
005c79c1  6a00                  PUSH 0x0
005c79c3  51                    PUSH ECX
005c79c4  d905f0199900          FLD float ptr [0x009919f0]
005c79ca  d91c24                FSTP float ptr [ESP]
005c79cd  51                    PUSH ECX
005c79ce  d9ee                  FLDZ
005c79d0  d91c24                FSTP float ptr [ESP]
005c79d3  6a01                  PUSH 0x1
005c79d5  680000007f            PUSH 0x7f000000
005c79da  8b8d08ffffff          MOV ECX,dword ptr [EBP + 0xffffff08]
005c79e0  51                    PUSH ECX
005c79e1  8d55d4                LEA EDX,[EBP + -0x2c]
005c79e4  52                    PUSH EDX
005c79e5  68feff0000            PUSH 0xfffe
005c79ea  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c79f0  e8bbcaf9ff            CALL 0x005644b0
005c79f5  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005c79f8  c7400407000000        MOV dword ptr [EAX + 0x4],0x7
005c79ff  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005c7a05  c781b0110000ffffffff  MOV dword ptr [ECX + 0x11b0],0xffffffff
005c7a0f  8b95f4feffff          MOV EDX,dword ptr [EBP + 0xfffffef4]
005c7a15  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
005c7a18  8982ec000000          MOV dword ptr [EDX + 0xec],EAX
005c7a1e  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
005c7a24  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005c7a27  899504ffffff          MOV dword ptr [EBP + 0xffffff04],EDX
005c7a2d  8b8d04ffffff          MOV ECX,dword ptr [EBP + 0xffffff04]
005c7a33  e858811700            CALL 0x0073fb90
005c7a38  8945f8                MOV dword ptr [EBP + -0x8],EAX
005c7a3b  33c0                  XOR EAX,EAX
005c7a3d  69c088000000          IMUL EAX,EAX,0x88
005c7a43  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
005c7a46  8b540124              MOV EDX,dword ptr [ECX + EAX*0x1 + 0x24]
005c7a4a  899500ffffff          MOV dword ptr [EBP + 0xffffff00],EDX
005c7a50  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005c7a55  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005c7a58  898dfcfeffff          MOV dword ptr [EBP + 0xfffffefc],ECX
005c7a5e  8b9500ffffff          MOV EDX,dword ptr [EBP + 0xffffff00]
005c7a64  52                    PUSH EDX
005c7a65  8b8dfcfeffff          MOV ECX,dword ptr [EBP + 0xfffffefc]
005c7a6b  e8404ef5ff            CALL 0x0051c8b0
005c7a70  8945f4                MOV dword ptr [EBP + -0xc],EAX
005c7a73  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005c7a78  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005c7a7b  898df8feffff          MOV dword ptr [EBP + 0xfffffef8],ECX
005c7a81  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
005c7a84  52                    PUSH EDX
005c7a85  8b8df8feffff          MOV ECX,dword ptr [EBP + 0xfffffef8]
005c7a8b  e87046f5ff            CALL 0x0051c100
005c7a90  8945bc                MOV dword ptr [EBP + -0x44],EAX
005c7a93  837dbc00              CMP dword ptr [EBP + -0x44],0x0
005c7a97  741e                  JZ 0x005c7ab7
005c7a99  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
005c7a9c  0594000000            ADD EAX,0x94
005c7aa1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005c7aa4  83c174                ADD ECX,0x74
005c7aa7  8b10                  MOV EDX,dword ptr [EAX]
005c7aa9  8911                  MOV dword ptr [ECX],EDX
005c7aab  8b5004                MOV EDX,dword ptr [EAX + 0x4]
005c7aae  895104                MOV dword ptr [ECX + 0x4],EDX
005c7ab1  8b4008                MOV EAX,dword ptr [EAX + 0x8]
005c7ab4  894108                MOV dword ptr [ECX + 0x8],EAX
005c7ab7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005c7aba  c7416801000000        MOV dword ptr [ECX + 0x68],0x1
005c7ac1  a1b8449900            MOV EAX,[0x009944b8]
005c7ac6  8be5                  MOV ESP,EBP
005c7ac8  5d                    POP EBP
005c7ac9  c21000                RET 0x10
