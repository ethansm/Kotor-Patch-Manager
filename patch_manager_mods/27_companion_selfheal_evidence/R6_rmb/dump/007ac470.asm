; FN 007ac470 FUN_007ac470
007ac470  55                    PUSH EBP
007ac471  8bec                  MOV EBP,ESP
007ac473  6aff                  PUSH -0x1
007ac475  68f7839600            PUSH 0x9683f7
007ac47a  64a100000000          MOV EAX,FS:[0x0]
007ac480  50                    PUSH EAX
007ac481  81ece8070000          SUB ESP,0x7e8
007ac487  56                    PUSH ESI
007ac488  a1201fa100            MOV EAX,[0x00a11f20]
007ac48d  33c5                  XOR EAX,EBP
007ac48f  50                    PUSH EAX
007ac490  8d45f4                LEA EAX,[EBP + -0xc]
007ac493  64a300000000          MOV FS:[0x0],EAX
007ac499  898da0f9ffff          MOV dword ptr [EBP + 0xfffff9a0],ECX
007ac49f  c78520fdffff00000000  MOV dword ptr [EBP + 0xfffffd20],0x0
007ac4a9  0fb605c9fca700        MOVZX EAX,byte ptr [0x00a7fcc9]
007ac4b0  85c0                  TEST EAX,EAX
007ac4b2  7419                  JZ 0x007ac4cd
007ac4b4  c605c9fca70000        MOV byte ptr [0x00a7fcc9],0x0
007ac4bb  6880000000            PUSH 0x80
007ac4c0  6a01                  PUSH 0x1
007ac4c2  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ac4c8  e8f34d0000            CALL 0x007b12c0
007ac4cd  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
007ac4d4  8d4dd0                LEA ECX,[EBP + -0x30]
007ac4d7  e86470f8ff            CALL 0x00733540
007ac4dc  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
007ac4e3  d905ccfca700          FLD float ptr [0x00a7fccc]
007ac4e9  dc1d386b9800          FCOMP double ptr [0x00986b38]
007ac4ef  dfe0                  FNSTSW AX
007ac4f1  f6c441                TEST AH,0x41
007ac4f4  7523                  JNZ 0x007ac519
007ac4f6  d905ccfca700          FLD float ptr [0x00a7fccc]
007ac4fc  d86508                FSUB float ptr [EBP + 0x8]
007ac4ff  d91dccfca700          FSTP float ptr [0x00a7fccc]
007ac505  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
007ac50c  8d4dd0                LEA ECX,[EBP + -0x30]
007ac50f  e86c72f8ff            CALL 0x00733780
007ac514  e9ff3c0000            JMP 0x007b0218
007ac519  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ac51f  83b9bc02000000        CMP dword ptr [ECX + 0x2bc],0x0
007ac526  7414                  JZ 0x007ac53c
007ac528  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
007ac52f  8d4dd0                LEA ECX,[EBP + -0x30]
007ac532  e84972f8ff            CALL 0x00733780
007ac537  e9dc3c0000            JMP 0x007b0218
007ac53c  8b15bcfca700          MOV EDX,dword ptr [0x00a7fcbc]
007ac542  8955d8                MOV dword ptr [EBP + -0x28],EDX
007ac545  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007ac54a  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007ac54d  898d1cfdffff          MOV dword ptr [EBP + 0xfffffd1c],ECX
007ac553  8b8d1cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd1c]
007ac559  e8a235f8ff            CALL 0x0072fb00
007ac55e  898518fdffff          MOV dword ptr [EBP + 0xfffffd18],EAX
007ac564  8b9518fdffff          MOV EDX,dword ptr [EBP + 0xfffffd18]
007ac56a  8b4208                MOV EAX,dword ptr [EDX + 0x8]
007ac56d  d1e8                  SHR EAX,0x1
007ac56f  83e001                AND EAX,0x1
007ac572  742a                  JZ 0x007ac59e
007ac574  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ac57a  83b99c00000002        CMP dword ptr [ECX + 0x9c],0x2
007ac581  741b                  JZ 0x007ac59e
007ac583  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ac589  83ba9c00000003        CMP dword ptr [EDX + 0x9c],0x3
007ac590  740c                  JZ 0x007ac59e
007ac592  33c0                  XOR EAX,EAX
007ac594  837dd800              CMP dword ptr [EBP + -0x28],0x0
007ac598  0f94c0                SETZ AL
007ac59b  8945d8                MOV dword ptr [EBP + -0x28],EAX
007ac59e  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007ac5a4  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007ac5a7  899514fdffff          MOV dword ptr [EBP + 0xfffffd14],EDX
007ac5ad  8b8d14fdffff          MOV ECX,dword ptr [EBP + 0xfffffd14]
007ac5b3  e8382df9ff            CALL 0x0073f2f0
007ac5b8  85c0                  TEST EAX,EAX
007ac5ba  741f                  JZ 0x007ac5db
007ac5bc  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ac5c2  e8c9480000            CALL 0x007b0e90
007ac5c7  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
007ac5ce  8d4dd0                LEA ECX,[EBP + -0x30]
007ac5d1  e8aa71f8ff            CALL 0x00733780
007ac5d6  e93d3c0000            JMP 0x007b0218
007ac5db  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ac5e1  8b482c                MOV ECX,dword ptr [EAX + 0x2c]
007ac5e4  e857e3d6ff            CALL 0x0051a940
007ac5e9  89859cf9ffff          MOV dword ptr [EBP + 0xfffff99c],EAX
007ac5ef  db859cf9ffff          FILD dword ptr [EBP + 0xfffff99c]
007ac5f5  dc3518659900          FDIV double ptr [0x00996518]
007ac5fb  d95d9c                FSTP float ptr [EBP + -0x64]
007ac5fe  51                    PUSH ECX
007ac5ff  d9459c                FLD float ptr [EBP + -0x64]
007ac602  d91c24                FSTP float ptr [ESP]
007ac605  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ac60b  e88037f8ff            CALL 0x0072fd90
007ac610  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
007ac617  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007ac61d  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007ac620  899510fdffff          MOV dword ptr [EBP + 0xfffffd10],EDX
007ac626  8b8d10fdffff          MOV ECX,dword ptr [EBP + 0xfffffd10]
007ac62c  e8ff2df9ff            CALL 0x0073f430
007ac631  85c0                  TEST EAX,EAX
007ac633  747b                  JZ 0x007ac6b0
007ac635  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007ac63a  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007ac63d  898d0cfdffff          MOV dword ptr [EBP + 0xfffffd0c],ECX
007ac643  8d9588fdffff          LEA EDX,[EBP + 0xfffffd88]
007ac649  52                    PUSH EDX
007ac64a  8b8d0cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd0c]
007ac650  e8db2df9ff            CALL 0x0073f430
007ac655  8bc8                  MOV ECX,EAX
007ac657  e82477fdff            CALL 0x00783d80
007ac65c  898598f9ffff          MOV dword ptr [EBP + 0xfffff998],EAX
007ac662  8b8598f9ffff          MOV EAX,dword ptr [EBP + 0xfffff998]
007ac668  898594f9ffff          MOV dword ptr [EBP + 0xfffff994],EAX
007ac66e  c645fc01              MOV byte ptr [EBP + -0x4],0x1
007ac672  8b8d20fdffff          MOV ECX,dword ptr [EBP + 0xfffffd20]
007ac678  83c901                OR ECX,0x1
007ac67b  898d20fdffff          MOV dword ptr [EBP + 0xfffffd20],ECX
007ac681  6814009a00            PUSH 0x9a0014
007ac686  8b8d94f9ffff          MOV ECX,dword ptr [EBP + 0xfffff994]
007ac68c  e8cf88f8ff            CALL 0x00734f60
007ac691  85c0                  TEST EAX,EAX
007ac693  741b                  JZ 0x007ac6b0
007ac695  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ac69b  83ba9c00000003        CMP dword ptr [EDX + 0x9c],0x3
007ac6a2  750c                  JNZ 0x007ac6b0
007ac6a4  c78590f9ffff01000000  MOV dword ptr [EBP + 0xfffff990],0x1
007ac6ae  eb0a                  JMP 0x007ac6ba
007ac6b0  c78590f9ffff00000000  MOV dword ptr [EBP + 0xfffff990],0x0
007ac6ba  8a8590f9ffff          MOV AL,byte ptr [EBP + 0xfffff990]
007ac6c0  888593fdffff          MOV byte ptr [EBP + 0xfffffd93],AL
007ac6c6  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
007ac6cd  8b8d20fdffff          MOV ECX,dword ptr [EBP + 0xfffffd20]
007ac6d3  83e101                AND ECX,0x1
007ac6d6  7412                  JZ 0x007ac6ea
007ac6d8  83a520fdfffffe        AND dword ptr [EBP + 0xfffffd20],0xfffffffe
007ac6df  8d8d88fdffff          LEA ECX,[EBP + 0xfffffd88]
007ac6e5  e89670f8ff            CALL 0x00733780
007ac6ea  0fb69593fdffff        MOVZX EDX,byte ptr [EBP + 0xfffffd93]
007ac6f1  85d2                  TEST EDX,EDX
007ac6f3  7407                  JZ 0x007ac6fc
007ac6f5  c745dc01000000        MOV dword ptr [EBP + -0x24],0x1
007ac6fc  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ac702  e85933f8ff            CALL 0x0072fa60
007ac707  85c0                  TEST EAX,EAX
007ac709  750a                  JNZ 0x007ac715
007ac70b  837ddc00              CMP dword ptr [EBP + -0x24],0x0
007ac70f  0f84921e0000          JZ 0x007ae5a7
007ac715  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ac71b  8b889c000000          MOV ECX,dword ptr [EAX + 0x9c]
007ac721  51                    PUSH ECX
007ac722  8d5598                LEA EDX,[EBP + -0x68]
007ac725  52                    PUSH EDX
007ac726  8d4588                LEA EAX,[EBP + -0x78]
007ac729  50                    PUSH EAX
007ac72a  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ac730  e84b33f8ff            CALL 0x0072fa80
007ac735  c645fc02              MOV byte ptr [EBP + -0x4],0x2
007ac739  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ac73f  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007ac742  8b4234                MOV EAX,dword ptr [EDX + 0x34]
007ac745  898508fdffff          MOV dword ptr [EBP + 0xfffffd08],EAX
007ac74b  83bd08fdffff01        CMP dword ptr [EBP + 0xfffffd08],0x1
007ac752  7422                  JZ 0x007ac776
007ac754  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ac75a  83b99c00000002        CMP dword ptr [ECX + 0x9c],0x2
007ac761  7413                  JZ 0x007ac776
007ac763  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ac769  83ba9c00000003        CMP dword ptr [EDX + 0x9c],0x3
007ac770  0f858a030000          JNZ 0x007acb00
007ac776  c7458400000000        MOV dword ptr [EBP + -0x7c],0x0
007ac77d  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
007ac784  eb09                  JMP 0x007ac78f
007ac786  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007ac789  83c001                ADD EAX,0x1
007ac78c  894594                MOV dword ptr [EBP + -0x6c],EAX
007ac78f  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
007ac792  3b4d98                CMP ECX,dword ptr [EBP + -0x68]
007ac795  0f8d65030000          JGE 0x007acb00
007ac79b  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
007ac79e  c1e204                SHL EDX,0x4
007ac7a1  035588                ADD EDX,dword ptr [EBP + -0x78]
007ac7a4  899504fdffff          MOV dword ptr [EBP + 0xfffffd04],EDX
007ac7aa  8b8504fdffff          MOV EAX,dword ptr [EBP + 0xfffffd04]
007ac7b0  8b08                  MOV ECX,dword ptr [EAX]
007ac7b2  898d00fdffff          MOV dword ptr [EBP + 0xfffffd00],ECX
007ac7b8  83bd00fdffff00        CMP dword ptr [EBP + 0xfffffd00],0x0
007ac7bf  0f8436030000          JZ 0x007acafb
007ac7c5  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ac7cb  0fb68214040000        MOVZX EAX,byte ptr [EDX + 0x414]
007ac7d2  83f80a                CMP EAX,0xa
007ac7d5  7d6d                  JGE 0x007ac844
007ac7d7  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
007ac7da  c1e104                SHL ECX,0x4
007ac7dd  034d88                ADD ECX,dword ptr [EBP + -0x78]
007ac7e0  898dfcfcffff          MOV dword ptr [EBP + 0xfffffcfc],ECX
007ac7e6  8b95fcfcffff          MOV EDX,dword ptr [EBP + 0xfffffcfc]
007ac7ec  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ac7ef  8985f8fcffff          MOV dword ptr [EBP + 0xfffffcf8],EAX
007ac7f5  8b8df8fcffff          MOV ECX,dword ptr [EBP + 0xfffffcf8]
007ac7fb  8b5110                MOV EDX,dword ptr [ECX + 0x10]
007ac7fe  8995f4fcffff          MOV dword ptr [EBP + 0xfffffcf4],EDX
007ac804  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ac80a  0fb68814040000        MOVZX ECX,byte ptr [EAX + 0x414]
007ac811  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ac817  8b85f4fcffff          MOV EAX,dword ptr [EBP + 0xfffffcf4]
007ac81d  89848aec030000        MOV dword ptr [EDX + ECX*0x4 + 0x3ec],EAX
007ac824  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ac82a  8a9114040000          MOV DL,byte ptr [ECX + 0x414]
007ac830  80c201                ADD DL,0x1
007ac833  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ac839  889014040000          MOV byte ptr [EAX + 0x414],DL
007ac83f  e98b000000            JMP 0x007ac8cf
007ac844  c7857cffffff00000000  MOV dword ptr [EBP + 0xffffff7c],0x0
007ac84e  eb0f                  JMP 0x007ac85f
007ac850  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
007ac856  83c101                ADD ECX,0x1
007ac859  898d7cffffff          MOV dword ptr [EBP + 0xffffff7c],ECX
007ac85f  83bd7cffffff09        CMP dword ptr [EBP + 0xffffff7c],0x9
007ac866  7d28                  JGE 0x007ac890
007ac868  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
007ac86e  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ac874  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
007ac87a  8bb5a0f9ffff          MOV ESI,dword ptr [EBP + 0xfffff9a0]
007ac880  8b8c8ef0030000        MOV ECX,dword ptr [ESI + ECX*0x4 + 0x3f0]
007ac887  898c90ec030000        MOV dword ptr [EAX + EDX*0x4 + 0x3ec],ECX
007ac88e  ebc0                  JMP 0x007ac850
007ac890  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
007ac893  c1e204                SHL EDX,0x4
007ac896  035588                ADD EDX,dword ptr [EBP + -0x78]
007ac899  8995f0fcffff          MOV dword ptr [EBP + 0xfffffcf0],EDX
007ac89f  8b85f0fcffff          MOV EAX,dword ptr [EBP + 0xfffffcf0]
007ac8a5  8b480c                MOV ECX,dword ptr [EAX + 0xc]
007ac8a8  898decfcffff          MOV dword ptr [EBP + 0xfffffcec],ECX
007ac8ae  8b95ecfcffff          MOV EDX,dword ptr [EBP + 0xfffffcec]
007ac8b4  8b4210                MOV EAX,dword ptr [EDX + 0x10]
007ac8b7  8985e8fcffff          MOV dword ptr [EBP + 0xfffffce8],EAX
007ac8bd  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ac8c3  8b95e8fcffff          MOV EDX,dword ptr [EBP + 0xfffffce8]
007ac8c9  899110040000          MOV dword ptr [ECX + 0x410],EDX
007ac8cf  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007ac8d2  c1e004                SHL EAX,0x4
007ac8d5  034588                ADD EAX,dword ptr [EBP + -0x78]
007ac8d8  8985e4fcffff          MOV dword ptr [EBP + 0xfffffce4],EAX
007ac8de  8b8de4fcffff          MOV ECX,dword ptr [EBP + 0xfffffce4]
007ac8e4  8b510c                MOV EDX,dword ptr [ECX + 0xc]
007ac8e7  8995e0fcffff          MOV dword ptr [EBP + 0xfffffce0],EDX
007ac8ed  8b85dcfcffff          MOV EAX,dword ptr [EBP + 0xfffffcdc]
007ac8f3  89858cf9ffff          MOV dword ptr [EBP + 0xfffff98c],EAX
007ac8f9  8b8de0fcffff          MOV ECX,dword ptr [EBP + 0xfffffce0]
007ac8ff  8b5110                MOV EDX,dword ptr [ECX + 0x10]
007ac902  89958cf9ffff          MOV dword ptr [EBP + 0xfffff98c],EDX
007ac908  8b858cf9ffff          MOV EAX,dword ptr [EBP + 0xfffff98c]
007ac90e  83e801                SUB EAX,0x1
007ac911  89858cf9ffff          MOV dword ptr [EBP + 0xfffff98c],EAX
007ac917  83bd8cf9ffff35        CMP dword ptr [EBP + 0xfffff98c],0x35
007ac91e  7746                  JA 0x007ac966
007ac920  8b8d8cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff98c]
007ac926  0fb69134027b00        MOVZX EDX,byte ptr [ECX + 0x7b0234]
007ac92d  ff24952c027b00        JMP dword ptr [EDX*0x4 + 0x7b022c]
007ac934  837d8400              CMP dword ptr [EBP + -0x7c],0x0
007ac938  7425                  JZ 0x007ac95f
007ac93a  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007ac93d  50                    PUSH EAX
007ac93e  8d4d88                LEA ECX,[EBP + -0x78]
007ac941  e87a930000            CALL 0x007b5cc0
007ac946  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
007ac949  83e901                SUB ECX,0x1
007ac94c  894d94                MOV dword ptr [EBP + -0x6c],ECX
007ac94f  8b5598                MOV EDX,dword ptr [EBP + -0x68]
007ac952  83ea01                SUB EDX,0x1
007ac955  895598                MOV dword ptr [EBP + -0x68],EDX
007ac958  e929feffff            JMP 0x007ac786
007ac95f  c7458401000000        MOV dword ptr [EBP + -0x7c],0x1
007ac966  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007ac969  c1e004                SHL EAX,0x4
007ac96c  034588                ADD EAX,dword ptr [EBP + -0x78]
007ac96f  8985d8fcffff          MOV dword ptr [EBP + 0xfffffcd8],EAX
007ac975  8b8dd8fcffff          MOV ECX,dword ptr [EBP + 0xfffffcd8]
007ac97b  8b510c                MOV EDX,dword ptr [ECX + 0xc]
007ac97e  8995d4fcffff          MOV dword ptr [EBP + 0xfffffcd4],EDX
007ac984  8b85d0fcffff          MOV EAX,dword ptr [EBP + 0xfffffcd0]
007ac98a  898588f9ffff          MOV dword ptr [EBP + 0xfffff988],EAX
007ac990  8b8dd4fcffff          MOV ECX,dword ptr [EBP + 0xfffffcd4]
007ac996  8b5110                MOV EDX,dword ptr [ECX + 0x10]
007ac999  899588f9ffff          MOV dword ptr [EBP + 0xfffff988],EDX
007ac99f  83bd88f9ffff31        CMP dword ptr [EBP + 0xfffff988],0x31
007ac9a6  0f824f010000          JC 0x007acafb
007ac9ac  83bd88f9ffff32        CMP dword ptr [EBP + 0xfffff988],0x32
007ac9b3  7605                  JBE 0x007ac9ba
007ac9b5  e941010000            JMP 0x007acafb
007ac9ba  c74580ffffffff        MOV dword ptr [EBP + -0x80],0xffffffff
007ac9c1  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007ac9c4  83c001                ADD EAX,0x1
007ac9c7  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
007ac9cd  eb0f                  JMP 0x007ac9de
007ac9cf  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
007ac9d5  83c101                ADD ECX,0x1
007ac9d8  898d7cffffff          MOV dword ptr [EBP + 0xffffff7c],ECX
007ac9de  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
007ac9e4  3b5598                CMP EDX,dword ptr [EBP + -0x68]
007ac9e7  0f8d8a000000          JGE 0x007aca77
007ac9ed  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
007ac9f3  c1e004                SHL EAX,0x4
007ac9f6  034588                ADD EAX,dword ptr [EBP + -0x78]
007ac9f9  8985ccfcffff          MOV dword ptr [EBP + 0xfffffccc],EAX
007ac9ff  8b8dccfcffff          MOV ECX,dword ptr [EBP + 0xfffffccc]
007aca05  8b11                  MOV EDX,dword ptr [ECX]
007aca07  8995c8fcffff          MOV dword ptr [EBP + 0xfffffcc8],EDX
007aca0d  83bdc8fcffff00        CMP dword ptr [EBP + 0xfffffcc8],0x0
007aca14  745c                  JZ 0x007aca72
007aca16  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
007aca1c  c1e004                SHL EAX,0x4
007aca1f  034588                ADD EAX,dword ptr [EBP + -0x78]
007aca22  8985c4fcffff          MOV dword ptr [EBP + 0xfffffcc4],EAX
007aca28  8b8dc4fcffff          MOV ECX,dword ptr [EBP + 0xfffffcc4]
007aca2e  8b510c                MOV EDX,dword ptr [ECX + 0xc]
007aca31  8995c0fcffff          MOV dword ptr [EBP + 0xfffffcc0],EDX
007aca37  8b85bcfcffff          MOV EAX,dword ptr [EBP + 0xfffffcbc]
007aca3d  898584f9ffff          MOV dword ptr [EBP + 0xfffff984],EAX
007aca43  8b8dc0fcffff          MOV ECX,dword ptr [EBP + 0xfffffcc0]
007aca49  8b5110                MOV EDX,dword ptr [ECX + 0x10]
007aca4c  899584f9ffff          MOV dword ptr [EBP + 0xfffff984],EDX
007aca52  83bd84f9ffff2f        CMP dword ptr [EBP + 0xfffff984],0x2f
007aca59  7217                  JC 0x007aca72
007aca5b  83bd84f9ffff30        CMP dword ptr [EBP + 0xfffff984],0x30
007aca62  7602                  JBE 0x007aca66
007aca64  eb0c                  JMP 0x007aca72
007aca66  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
007aca6c  83c001                ADD EAX,0x1
007aca6f  894580                MOV dword ptr [EBP + -0x80],EAX
007aca72  e958ffffff            JMP 0x007ac9cf
007aca77  837d80ff              CMP dword ptr [EBP + -0x80],-0x1
007aca7b  747e                  JZ 0x007acafb
007aca7d  8b4d8c                MOV ECX,dword ptr [EBP + -0x74]
007aca80  898db8fcffff          MOV dword ptr [EBP + 0xfffffcb8],ECX
007aca86  8b5580                MOV EDX,dword ptr [EBP + -0x80]
007aca89  3b95b8fcffff          CMP EDX,dword ptr [EBP + 0xfffffcb8]
007aca8f  7c2e                  JL 0x007acabf
007aca91  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007aca94  c1e004                SHL EAX,0x4
007aca97  034588                ADD EAX,dword ptr [EBP + -0x78]
007aca9a  83ec10                SUB ESP,0x10
007aca9d  8bcc                  MOV ECX,ESP
007aca9f  8b10                  MOV EDX,dword ptr [EAX]
007acaa1  8911                  MOV dword ptr [ECX],EDX
007acaa3  8b5004                MOV EDX,dword ptr [EAX + 0x4]
007acaa6  895104                MOV dword ptr [ECX + 0x4],EDX
007acaa9  8b5008                MOV EDX,dword ptr [EAX + 0x8]
007acaac  895108                MOV dword ptr [ECX + 0x8],EDX
007acaaf  8b400c                MOV EAX,dword ptr [EAX + 0xc]
007acab2  89410c                MOV dword ptr [ECX + 0xc],EAX
007acab5  8d4d88                LEA ECX,[EBP + -0x78]
007acab8  e8832af8ff            CALL 0x0072f540
007acabd  eb30                  JMP 0x007acaef
007acabf  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
007acac2  51                    PUSH ECX
007acac3  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
007acac6  c1e204                SHL EDX,0x4
007acac9  035588                ADD EDX,dword ptr [EBP + -0x78]
007acacc  83ec10                SUB ESP,0x10
007acacf  8bc4                  MOV EAX,ESP
007acad1  8b0a                  MOV ECX,dword ptr [EDX]
007acad3  8908                  MOV dword ptr [EAX],ECX
007acad5  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
007acad8  894804                MOV dword ptr [EAX + 0x4],ECX
007acadb  8b4a08                MOV ECX,dword ptr [EDX + 0x8]
007acade  894808                MOV dword ptr [EAX + 0x8],ECX
007acae1  8b520c                MOV EDX,dword ptr [EDX + 0xc]
007acae4  89500c                MOV dword ptr [EAX + 0xc],EDX
007acae7  8d4d88                LEA ECX,[EBP + -0x78]
007acaea  e821910000            CALL 0x007b5c10
007acaef  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007acaf2  50                    PUSH EAX
007acaf3  8d4d88                LEA ECX,[EBP + -0x78]
007acaf6  e8c5910000            CALL 0x007b5cc0
007acafb  e986fcffff            JMP 0x007ac786
007acb00  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
007acb07  eb09                  JMP 0x007acb12
007acb09  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
007acb0c  83c101                ADD ECX,0x1
007acb0f  894d94                MOV dword ptr [EBP + -0x6c],ECX
007acb12  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
007acb15  3b5598                CMP EDX,dword ptr [EBP + -0x68]
007acb18  0f8d75030000          JGE 0x007ace93
007acb1e  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007acb23  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007acb26  898db4fcffff          MOV dword ptr [EBP + 0xfffffcb4],ECX
007acb2c  8b8db4fcffff          MOV ECX,dword ptr [EBP + 0xfffffcb4]
007acb32  e89927f9ff            CALL 0x0073f2d0
007acb37  85c0                  TEST EAX,EAX
007acb39  7477                  JZ 0x007acbb2
007acb3b  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007acb41  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007acb44  8985b0fcffff          MOV dword ptr [EBP + 0xfffffcb0],EAX
007acb4a  8b8db0fcffff          MOV ECX,dword ptr [EBP + 0xfffffcb0]
007acb50  e85b27f9ff            CALL 0x0073f2b0
007acb55  85c0                  TEST EAX,EAX
007acb57  7459                  JZ 0x007acbb2
007acb59  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007acb5f  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007acb62  8995acfcffff          MOV dword ptr [EBP + 0xfffffcac],EDX
007acb68  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007acb6d  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007acb70  898da8fcffff          MOV dword ptr [EBP + 0xfffffca8],ECX
007acb76  8b8dacfcffff          MOV ECX,dword ptr [EBP + 0xfffffcac]
007acb7c  e82f27f9ff            CALL 0x0073f2b0
007acb81  50                    PUSH EAX
007acb82  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
007acb85  52                    PUSH EDX
007acb86  8b8da8fcffff          MOV ECX,dword ptr [EBP + 0xfffffca8]
007acb8c  e81f28f9ff            CALL 0x0073f3b0
007acb91  8bc8                  MOV ECX,EAX
007acb93  e888ddd6ff            CALL 0x0051a920
007acb98  6a00                  PUSH 0x0
007acb9a  68e8030000            PUSH 0x3e8
007acb9f  52                    PUSH EDX
007acba0  50                    PUSH EAX
007acba1  e87a061700            CALL 0x0091d220
007acba6  50                    PUSH EAX
007acba7  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007acbad  e85e2ef8ff            CALL 0x0072fa10
007acbb2  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007acbb5  c1e004                SHL EAX,0x4
007acbb8  034588                ADD EAX,dword ptr [EBP + -0x78]
007acbbb  8985a4fcffff          MOV dword ptr [EBP + 0xfffffca4],EAX
007acbc1  8b8da4fcffff          MOV ECX,dword ptr [EBP + 0xfffffca4]
007acbc7  8b510c                MOV EDX,dword ptr [ECX + 0xc]
007acbca  8995a0fcffff          MOV dword ptr [EBP + 0xfffffca0],EDX
007acbd0  8b85a0fcffff          MOV EAX,dword ptr [EBP + 0xfffffca0]
007acbd6  8b4810                MOV ECX,dword ptr [EAX + 0x10]
007acbd9  898d9cfcffff          MOV dword ptr [EBP + 0xfffffc9c],ECX
007acbdf  8b959cfcffff          MOV EDX,dword ptr [EBP + 0xfffffc9c]
007acbe5  899578ffffff          MOV dword ptr [EBP + 0xffffff78],EDX
007acbeb  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
007acbee  c1e004                SHL EAX,0x4
007acbf1  034588                ADD EAX,dword ptr [EBP + -0x78]
007acbf4  898598fcffff          MOV dword ptr [EBP + 0xfffffc98],EAX
007acbfa  8b8d98fcffff          MOV ECX,dword ptr [EBP + 0xfffffc98]
007acc00  8b11                  MOV EDX,dword ptr [ECX]
007acc02  899594fcffff          MOV dword ptr [EBP + 0xfffffc94],EDX
007acc08  8b8594fcffff          MOV EAX,dword ptr [EBP + 0xfffffc94]
007acc0e  898570ffffff          MOV dword ptr [EBP + 0xffffff70],EAX
007acc14  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007acc1a  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007acc1d  899590fcffff          MOV dword ptr [EBP + 0xfffffc90],EDX
007acc23  8b8d90fcffff          MOV ECX,dword ptr [EBP + 0xfffffc90]
007acc29  e8222bf9ff            CALL 0x0073f750
007acc2e  89858cfcffff          MOV dword ptr [EBP + 0xfffffc8c],EAX
007acc34  8b858cfcffff          MOV EAX,dword ptr [EBP + 0xfffffc8c]
007acc3a  8b4834                MOV ECX,dword ptr [EAX + 0x34]
007acc3d  898d74ffffff          MOV dword ptr [EBP + 0xffffff74],ECX
007acc43  83bd78ffffff27        CMP dword ptr [EBP + 0xffffff78],0x27
007acc4a  7c0d                  JL 0x007acc59
007acc4c  83bd78ffffff40        CMP dword ptr [EBP + 0xffffff78],0x40
007acc53  0f8ea4000000          JLE 0x007accfd
007acc59  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007acc5f  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007acc62  898588fcffff          MOV dword ptr [EBP + 0xfffffc88],EAX
007acc68  8b8d88fcffff          MOV ECX,dword ptr [EBP + 0xfffffc88]
007acc6e  e8dd2af9ff            CALL 0x0073f750
007acc73  898584fcffff          MOV dword ptr [EBP + 0xfffffc84],EAX
007acc79  8b8d84fcffff          MOV ECX,dword ptr [EBP + 0xfffffc84]
007acc7f  8b5134                MOV EDX,dword ptr [ECX + 0x34]
007acc82  899580fcffff          MOV dword ptr [EBP + 0xfffffc80],EDX
007acc88  83bd80fcffff01        CMP dword ptr [EBP + 0xfffffc80],0x1
007acc8f  0f8461010000          JZ 0x007acdf6
007acc95  81bd78ffffffb4000000  CMP dword ptr [EBP + 0xffffff78],0xb4
007acc9f  7c0c                  JL 0x007accad
007acca1  81bd78ffffffbb000000  CMP dword ptr [EBP + 0xffffff78],0xbb
007accab  7e50                  JLE 0x007accfd
007accad  81bd78ffffffce000000  CMP dword ptr [EBP + 0xffffff78],0xce
007accb7  7444                  JZ 0x007accfd
007accb9  81bd78ffffffdf000000  CMP dword ptr [EBP + 0xffffff78],0xdf
007accc3  7438                  JZ 0x007accfd
007accc5  81bd78fffffff3000000  CMP dword ptr [EBP + 0xffffff78],0xf3
007acccf  742c                  JZ 0x007accfd
007accd1  81bd78fffffff4000000  CMP dword ptr [EBP + 0xffffff78],0xf4
007accdb  7420                  JZ 0x007accfd
007accdd  81bd78fffffffe000000  CMP dword ptr [EBP + 0xffffff78],0xfe
007acce7  0f8c09010000          JL 0x007acdf6
007acced  81bd78ffffff06010000  CMP dword ptr [EBP + 0xffffff78],0x106
007accf7  0f8ff9000000          JG 0x007acdf6
007accfd  83bd70ffffff00        CMP dword ptr [EBP + 0xffffff70],0x0
007acd04  0f8492000000          JZ 0x007acd9c
007acd0a  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007acd10  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007acd13  898d7cfcffff          MOV dword ptr [EBP + 0xfffffc7c],ECX
007acd19  8b957cfcffff          MOV EDX,dword ptr [EBP + 0xfffffc7c]
007acd1f  8a4275                MOV AL,byte ptr [EDX + 0x75]
007acd22  88857bfcffff          MOV byte ptr [EBP + 0xfffffc7b],AL
007acd28  0fb68d7bfcffff        MOVZX ECX,byte ptr [EBP + 0xfffffc7b]
007acd2f  83f905                CMP ECX,0x5
007acd32  7568                  JNZ 0x007acd9c
007acd34  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007acd3a  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007acd3d  898574fcffff          MOV dword ptr [EBP + 0xfffffc74],EAX
007acd43  8b8d74fcffff          MOV ECX,dword ptr [EBP + 0xfffffc74]
007acd49  e8022af9ff            CALL 0x0073f750
007acd4e  898570fcffff          MOV dword ptr [EBP + 0xfffffc70],EAX
007acd54  8b8d70fcffff          MOV ECX,dword ptr [EBP + 0xfffffc70]
007acd5a  8b91f4000000          MOV EDX,dword ptr [ECX + 0xf4]
007acd60  89956cfcffff          MOV dword ptr [EBP + 0xfffffc6c],EDX
007acd66  83bd6cfcffff00        CMP dword ptr [EBP + 0xfffffc6c],0x0
007acd6d  752d                  JNZ 0x007acd9c
007acd6f  c705dc06a10001000000  MOV dword ptr [0x00a106dc],0x1
007acd79  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007acd7f  e8fca1f7ff            CALL 0x00726f80
007acd84  8bc8                  MOV ECX,EAX
007acd86  e895a40400            CALL 0x007f7220
007acd8b  6a01                  PUSH 0x1
007acd8d  6a00                  PUSH 0x0
007acd8f  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007acd95  e8b6620000            CALL 0x007b3050
007acd9a  eb58                  JMP 0x007acdf4
007acd9c  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007acda2  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007acda5  8b5134                MOV EDX,dword ptr [ECX + 0x34]
007acda8  899568fcffff          MOV dword ptr [EBP + 0xfffffc68],EDX
007acdae  83bd68fcffff01        CMP dword ptr [EBP + 0xfffffc68],0x1
007acdb5  741e                  JZ 0x007acdd5
007acdb7  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007acdbd  83b89c00000002        CMP dword ptr [EAX + 0x9c],0x2
007acdc4  740f                  JZ 0x007acdd5
007acdc6  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007acdcc  83b99c00000003        CMP dword ptr [ECX + 0x9c],0x3
007acdd3  751f                  JNZ 0x007acdf4
007acdd5  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
007acddb  52                    PUSH EDX
007acddc  8b8578ffffff          MOV EAX,dword ptr [EBP + 0xffffff78]
007acde2  50                    PUSH EAX
007acde3  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007acde9  8b8974020000          MOV ECX,dword ptr [ECX + 0x274]
007acdef  e8ac3cc6ff            CALL 0x00410aa0
007acdf4  eb1c                  JMP 0x007ace12
007acdf6  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
007acdfc  52                    PUSH EDX
007acdfd  8b8578ffffff          MOV EAX,dword ptr [EBP + 0xffffff78]
007ace03  50                    PUSH EAX
007ace04  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ace0a  e8b1440000            CALL 0x007b12c0
007ace0f  8945cc                MOV dword ptr [EBP + -0x34],EAX
007ace12  e8291af8ff            CALL 0x0072e840
007ace17  0fb6c8                MOVZX ECX,AL
007ace1a  85c9                  TEST ECX,ECX
007ace1c  7456                  JZ 0x007ace74
007ace1e  837dd800              CMP dword ptr [EBP + -0x28],0x0
007ace22  740f                  JZ 0x007ace33
007ace24  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ace2a  83ba9c00000002        CMP dword ptr [EDX + 0x9c],0x2
007ace31  7541                  JNZ 0x007ace74
007ace33  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ace39  83b89c00000004        CMP dword ptr [EAX + 0x9c],0x4
007ace40  7432                  JZ 0x007ace74
007ace42  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
007ace49  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ace4f  8b91cc030000          MOV EDX,dword ptr [ECX + 0x3cc]
007ace55  52                    PUSH EDX
007ace56  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ace5c  8b88c8030000          MOV ECX,dword ptr [EAX + 0x3c8]
007ace62  51                    PUSH ECX
007ace63  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ace69  8b8a74020000          MOV ECX,dword ptr [EDX + 0x274]
007ace6f  e88c5dc6ff            CALL 0x00412c00
007ace74  837dcc00              CMP dword ptr [EBP + -0x34],0x0
007ace78  7414                  JZ 0x007ace8e
007ace7a  833d9cb4a10000        CMP dword ptr [0x00a1b49c],0x0
007ace81  740b                  JZ 0x007ace8e
007ace83  8b0d9cb4a100          MOV ECX,dword ptr [0x00a1b49c]
007ace89  e80271c6ff            CALL 0x00413f90
007ace8e  e976fcffff            JMP 0x007acb09
007ace93  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ace99  e8e2a0f7ff            CALL 0x00726f80
007ace9e  85c0                  TEST EAX,EAX
007acea0  0f8475160000          JZ 0x007ae51b
007acea6  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007aceac  e8cfa0f7ff            CALL 0x00726f80
007aceb1  898564fcffff          MOV dword ptr [EBP + 0xfffffc64],EAX
007aceb7  8b8564fcffff          MOV EAX,dword ptr [EBP + 0xfffffc64]
007acebd  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007acec0  898d60fcffff          MOV dword ptr [EBP + 0xfffffc60],ECX
007acec6  83bd60fcffff00        CMP dword ptr [EBP + 0xfffffc60],0x0
007acecd  0f8448160000          JZ 0x007ae51b
007aced3  837dd800              CMP dword ptr [EBP + -0x28],0x0
007aced7  0f853e160000          JNZ 0x007ae51b
007acedd  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007acee3  8b8274020000          MOV EAX,dword ptr [EDX + 0x274]
007acee9  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007aceec  898d5cfcffff          MOV dword ptr [EBP + 0xfffffc5c],ECX
007acef2  83bd5cfcffff00        CMP dword ptr [EBP + 0xfffffc5c],0x0
007acef9  744d                  JZ 0x007acf48
007acefb  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007acf01  8b8274020000          MOV EAX,dword ptr [EDX + 0x274]
007acf07  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007acf0a  898d58fcffff          MOV dword ptr [EBP + 0xfffffc58],ECX
007acf10  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007acf16  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007acf19  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007acf1f  898d54fcffff          MOV dword ptr [EBP + 0xfffffc54],ECX
007acf25  8b9558fcffff          MOV EDX,dword ptr [EBP + 0xfffffc58]
007acf2b  52                    PUSH EDX
007acf2c  8b8d54fcffff          MOV ECX,dword ptr [EBP + 0xfffffc54]
007acf32  81c1cc000000          ADD ECX,0xcc
007acf38  e8b37af9ff            CALL 0x007449f0
007acf3d  0fb6c0                MOVZX EAX,AL
007acf40  898580f9ffff          MOV dword ptr [EBP + 0xfffff980],EAX
007acf46  eb0a                  JMP 0x007acf52
007acf48  c78580f9ffff00000000  MOV dword ptr [EBP + 0xfffff980],0x0
007acf52  8a8d80f9ffff          MOV CL,byte ptr [EBP + 0xfffff980]
007acf58  888d6fffffff          MOV byte ptr [EBP + 0xffffff6f],CL
007acf5e  0fb6956fffffff        MOVZX EDX,byte ptr [EBP + 0xffffff6f]
007acf65  85d2                  TEST EDX,EDX
007acf67  0f84c2000000          JZ 0x007ad02f
007acf6d  0fb6856fffffff        MOVZX EAX,byte ptr [EBP + 0xffffff6f]
007acf74  83f801                CMP EAX,0x1
007acf77  7547                  JNZ 0x007acfc0
007acf79  d9ee                  FLDZ
007acf7b  d99d7cfdffff          FSTP float ptr [EBP + 0xfffffd7c]
007acf81  d9ee                  FLDZ
007acf83  d99d80fdffff          FSTP float ptr [EBP + 0xfffffd80]
007acf89  d9ee                  FLDZ
007acf8b  d99d84fdffff          FSTP float ptr [EBP + 0xfffffd84]
007acf91  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007acf97  8b91b4020000          MOV EDX,dword ptr [ECX + 0x2b4]
007acf9d  899550fcffff          MOV dword ptr [EBP + 0xfffffc50],EDX
007acfa3  6a01                  PUSH 0x1
007acfa5  8d857cfdffff          LEA EAX,[EBP + 0xfffffd7c]
007acfab  50                    PUSH EAX
007acfac  8b8d50fcffff          MOV ECX,dword ptr [EBP + 0xfffffc50]
007acfb2  51                    PUSH ECX
007acfb3  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007acfb9  e8426a0000            CALL 0x007b3a00
007acfbe  eb31                  JMP 0x007acff1
007acfc0  d9ee                  FLDZ
007acfc2  d99d70fdffff          FSTP float ptr [EBP + 0xfffffd70]
007acfc8  d9ee                  FLDZ
007acfca  d99d74fdffff          FSTP float ptr [EBP + 0xfffffd74]
007acfd0  d9ee                  FLDZ
007acfd2  d99d78fdffff          FSTP float ptr [EBP + 0xfffffd78]
007acfd8  6a00                  PUSH 0x0
007acfda  8d9570fdffff          LEA EDX,[EBP + 0xfffffd70]
007acfe0  52                    PUSH EDX
007acfe1  680000007f            PUSH 0x7f000000
007acfe6  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007acfec  e80f6a0000            CALL 0x007b3a00
007acff1  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007acff7  8b88b4020000          MOV ECX,dword ptr [EAX + 0x2b4]
007acffd  898d4cfcffff          MOV dword ptr [EBP + 0xfffffc4c],ECX
007ad003  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ad009  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007ad00c  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007ad012  898d48fcffff          MOV dword ptr [EBP + 0xfffffc48],ECX
007ad018  8b9548fcffff          MOV EDX,dword ptr [EBP + 0xfffffc48]
007ad01e  8b854cfcffff          MOV EAX,dword ptr [EBP + 0xfffffc4c]
007ad024  898258590000          MOV dword ptr [EDX + 0x5958],EAX
007ad02a  e9e7140000            JMP 0x007ae516
007ad02f  6a00                  PUSH 0x0
007ad031  68ffffffbf            PUSH 0xbfffffff
007ad036  6a00                  PUSH 0x0
007ad038  8d8df4feffff          LEA ECX,[EBP + 0xfffffef4]
007ad03e  e81d85cbff            CALL 0x00465560
007ad043  c645fc03              MOV byte ptr [EBP + -0x4],0x3
007ad047  c68554ffffff01        MOV byte ptr [EBP + 0xffffff54],0x1
007ad04e  d9ee                  FLDZ
007ad050  d99de8feffff          FSTP float ptr [EBP + 0xfffffee8]
007ad056  d9ee                  FLDZ
007ad058  d99decfeffff          FSTP float ptr [EBP + 0xfffffeec]
007ad05e  d9ee                  FLDZ
007ad060  d99df0feffff          FSTP float ptr [EBP + 0xfffffef0]
007ad066  8b0d9cb4a100          MOV ECX,dword ptr [0x00a1b49c]
007ad06c  8b11                  MOV EDX,dword ptr [ECX]
007ad06e  89955cffffff          MOV dword ptr [EBP + 0xffffff5c],EDX
007ad074  a19cb4a100            MOV EAX,[0x00a1b49c]
007ad079  0fbf486e              MOVSX ECX,word ptr [EAX + 0x6e]
007ad07d  898d44fcffff          MOV dword ptr [EBP + 0xfffffc44],ECX
007ad083  8b159cb4a100          MOV EDX,dword ptr [0x00a1b49c]
007ad089  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007ad08c  898540fcffff          MOV dword ptr [EBP + 0xfffffc40],EAX
007ad092  8b8d44fcffff          MOV ECX,dword ptr [EBP + 0xfffffc44]
007ad098  2b8d40fcffff          SUB ECX,dword ptr [EBP + 0xfffffc40]
007ad09e  898d68ffffff          MOV dword ptr [EBP + 0xffffff68],ECX
007ad0a4  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad0aa  e87101feff            CALL 0x0078d220
007ad0af  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ad0b5  8b4220                MOV EAX,dword ptr [EDX + 0x20]
007ad0b8  89853cfcffff          MOV dword ptr [EBP + 0xfffffc3c],EAX
007ad0be  8b8d3cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc3c]
007ad0c4  51                    PUSH ECX
007ad0c5  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad0cb  e83000feff            CALL 0x0078d100
007ad0d0  c78560ffffff00000000  MOV dword ptr [EBP + 0xffffff60],0x0
007ad0da  c78564ffffff01000000  MOV dword ptr [EBP + 0xffffff64],0x1
007ad0e4  83bd64ffffff00        CMP dword ptr [EBP + 0xffffff64],0x0
007ad0eb  0f8481070000          JZ 0x007ad872
007ad0f1  c78564ffffff00000000  MOV dword ptr [EBP + 0xffffff64],0x0
007ad0fb  8d95fcfeffff          LEA EDX,[EBP + 0xfffffefc]
007ad101  52                    PUSH EDX
007ad102  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad108  e87301feff            CALL 0x0078d280
007ad10d  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad113  e8689ef7ff            CALL 0x00726f80
007ad118  898538fcffff          MOV dword ptr [EBP + 0xfffffc38],EAX
007ad11e  8b8534fcffff          MOV EAX,dword ptr [EBP + 0xfffffc34]
007ad124  89857cf9ffff          MOV dword ptr [EBP + 0xfffff97c],EAX
007ad12a  8b8d38fcffff          MOV ECX,dword ptr [EBP + 0xfffffc38]
007ad130  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007ad133  89957cf9ffff          MOV dword ptr [EBP + 0xfffff97c],EDX
007ad139  8d85f4feffff          LEA EAX,[EBP + 0xfffffef4]
007ad13f  50                    PUSH EAX
007ad140  8d8de8feffff          LEA ECX,[EBP + 0xfffffee8]
007ad146  51                    PUSH ECX
007ad147  8b9568ffffff          MOV EDX,dword ptr [EBP + 0xffffff68]
007ad14d  52                    PUSH EDX
007ad14e  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
007ad154  50                    PUSH EAX
007ad155  8b8d7cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff97c]
007ad15b  8b11                  MOV EDX,dword ptr [ECX]
007ad15d  8b8d7cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff97c]
007ad163  8b4230                MOV EAX,dword ptr [EDX + 0x30]
007ad166  ffd0                  CALL EAX
007ad168  83bd2cffffff00        CMP dword ptr [EBP + 0xffffff2c],0x0
007ad16f  0f84f8060000          JZ 0x007ad86d
007ad175  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
007ad17b  898d78f9ffff          MOV dword ptr [EBP + 0xfffff978],ECX
007ad181  8b9578f9ffff          MOV EDX,dword ptr [EBP + 0xfffff978]
007ad187  8b02                  MOV EAX,dword ptr [EDX]
007ad189  8b8d78f9ffff          MOV ECX,dword ptr [EBP + 0xfffff978]
007ad18f  8b5070                MOV EDX,dword ptr [EAX + 0x70]
007ad192  ffd2                  CALL EDX
007ad194  85c0                  TEST EAX,EAX
007ad196  0f84d1060000          JZ 0x007ad86d
007ad19c  8b852cffffff          MOV EAX,dword ptr [EBP + 0xffffff2c]
007ad1a2  898574f9ffff          MOV dword ptr [EBP + 0xfffff974],EAX
007ad1a8  8b8d74f9ffff          MOV ECX,dword ptr [EBP + 0xfffff974]
007ad1ae  8b11                  MOV EDX,dword ptr [ECX]
007ad1b0  8b8d74f9ffff          MOV ECX,dword ptr [EBP + 0xfffff974]
007ad1b6  8b4278                MOV EAX,dword ptr [EDX + 0x78]
007ad1b9  ffd0                  CALL EAX
007ad1bb  8985e4feffff          MOV dword ptr [EBP + 0xfffffee4],EAX
007ad1c1  8b8de4feffff          MOV ECX,dword ptr [EBP + 0xfffffee4]
007ad1c7  898d70f9ffff          MOV dword ptr [EBP + 0xfffff970],ECX
007ad1cd  8b9570f9ffff          MOV EDX,dword ptr [EBP + 0xfffff970]
007ad1d3  83ea05                SUB EDX,0x5
007ad1d6  899570f9ffff          MOV dword ptr [EBP + 0xfffff970],EDX
007ad1dc  83bd70f9ffff05        CMP dword ptr [EBP + 0xfffff970],0x5
007ad1e3  0f87e5030000          JA 0x007ad5ce
007ad1e9  8b8570f9ffff          MOV EAX,dword ptr [EBP + 0xfffff970]
007ad1ef  ff24856c027b00        JMP dword ptr [EAX*0x4 + 0x7b026c]
007ad1f6  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
007ad1fc  898d6cf9ffff          MOV dword ptr [EBP + 0xfffff96c],ECX
007ad202  8b956cf9ffff          MOV EDX,dword ptr [EBP + 0xfffff96c]
007ad208  8b02                  MOV EAX,dword ptr [EDX]
007ad20a  8b8d6cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff96c]
007ad210  8b5070                MOV EDX,dword ptr [EAX + 0x70]
007ad213  ffd2                  CALL EDX
007ad215  898568f9ffff          MOV dword ptr [EBP + 0xfffff968],EAX
007ad21b  8b8568f9ffff          MOV EAX,dword ptr [EBP + 0xfffff968]
007ad221  8b10                  MOV EDX,dword ptr [EAX]
007ad223  8b8d68f9ffff          MOV ECX,dword ptr [EBP + 0xfffff968]
007ad229  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ad22c  ffd0                  CALL EAX
007ad22e  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
007ad234  e995030000            JMP 0x007ad5ce
007ad239  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
007ad23f  898d64f9ffff          MOV dword ptr [EBP + 0xfffff964],ECX
007ad245  8b9564f9ffff          MOV EDX,dword ptr [EBP + 0xfffff964]
007ad24b  8b02                  MOV EAX,dword ptr [EDX]
007ad24d  8b8d64f9ffff          MOV ECX,dword ptr [EBP + 0xfffff964]
007ad253  8b5070                MOV EDX,dword ptr [EAX + 0x70]
007ad256  ffd2                  CALL EDX
007ad258  898560f9ffff          MOV dword ptr [EBP + 0xfffff960],EAX
007ad25e  8b8560f9ffff          MOV EAX,dword ptr [EBP + 0xfffff960]
007ad264  8b10                  MOV EDX,dword ptr [EAX]
007ad266  8b8d60f9ffff          MOV ECX,dword ptr [EBP + 0xfffff960]
007ad26c  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ad26f  ffd0                  CALL EAX
007ad271  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
007ad277  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
007ad27e  0f8431010000          JZ 0x007ad3b5
007ad284  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad28a  8b11                  MOV EDX,dword ptr [ECX]
007ad28c  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad292  8b424c                MOV EAX,dword ptr [EDX + 0x4c]
007ad295  ffd0                  CALL EAX
007ad297  85c0                  TEST EAX,EAX
007ad299  0f8416010000          JZ 0x007ad3b5
007ad29f  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad2a5  8b11                  MOV EDX,dword ptr [ECX]
007ad2a7  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad2ad  8b424c                MOV EAX,dword ptr [EDX + 0x4c]
007ad2b0  ffd0                  CALL EAX
007ad2b2  8985e0feffff          MOV dword ptr [EBP + 0xfffffee0],EAX
007ad2b8  8b8de0feffff          MOV ECX,dword ptr [EBP + 0xfffffee0]
007ad2be  e83d5a0900            CALL 0x00842d00
007ad2c3  8985dcfeffff          MOV dword ptr [EBP + 0xfffffedc],EAX
007ad2c9  83bddcfeffff00        CMP dword ptr [EBP + 0xfffffedc],0x0
007ad2d0  750f                  JNZ 0x007ad2e1
007ad2d2  c78560ffffff00000000  MOV dword ptr [EBP + 0xffffff60],0x0
007ad2dc  e9d4000000            JMP 0x007ad3b5
007ad2e1  8b8de0feffff          MOV ECX,dword ptr [EBP + 0xfffffee0]
007ad2e7  8b912c010000          MOV EDX,dword ptr [ECX + 0x12c]
007ad2ed  899530fcffff          MOV dword ptr [EBP + 0xfffffc30],EDX
007ad2f3  83bd30fcffff00        CMP dword ptr [EBP + 0xfffffc30],0x0
007ad2fa  741f                  JZ 0x007ad31b
007ad2fc  8b85dcfeffff          MOV EAX,dword ptr [EBP + 0xfffffedc]
007ad302  8b88f0030000          MOV ECX,dword ptr [EAX + 0x3f0]
007ad308  898d2cfcffff          MOV dword ptr [EBP + 0xfffffc2c],ECX
007ad30e  83bd2cfcffff00        CMP dword ptr [EBP + 0xfffffc2c],0x0
007ad315  0f849a000000          JZ 0x007ad3b5
007ad31b  c785d8feffff00000000  MOV dword ptr [EBP + 0xfffffed8],0x0
007ad325  8b95e0feffff          MOV EDX,dword ptr [EBP + 0xfffffee0]
007ad32b  8b820c010000          MOV EAX,dword ptr [EDX + 0x10c]
007ad331  898528fcffff          MOV dword ptr [EBP + 0xfffffc28],EAX
007ad337  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
007ad33d  8b9130010000          MOV EDX,dword ptr [ECX + 0x130]
007ad343  899524fcffff          MOV dword ptr [EBP + 0xfffffc24],EDX
007ad349  8b8524fcffff          MOV EAX,dword ptr [EBP + 0xfffffc24]
007ad34f  8b4830                MOV ECX,dword ptr [EAX + 0x30]
007ad352  898d20fcffff          MOV dword ptr [EBP + 0xfffffc20],ECX
007ad358  8d95d8feffff          LEA EDX,[EBP + 0xfffffed8]
007ad35e  52                    PUSH EDX
007ad35f  a124ffa000            MOV EAX,[0x00a0ff24]
007ad364  50                    PUSH EAX
007ad365  8b8d28fcffff          MOV ECX,dword ptr [EBP + 0xfffffc28]
007ad36b  51                    PUSH ECX
007ad36c  8b8d20fcffff          MOV ECX,dword ptr [EBP + 0xfffffc20]
007ad372  e839e5f6ff            CALL 0x0071b8b0
007ad377  83bdd8feffff00        CMP dword ptr [EBP + 0xfffffed8],0x0
007ad37e  742b                  JZ 0x007ad3ab
007ad380  c78564ffffff01000000  MOV dword ptr [EBP + 0xffffff64],0x1
007ad38a  8b95e0feffff          MOV EDX,dword ptr [EBP + 0xfffffee0]
007ad390  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007ad393  89851cfcffff          MOV dword ptr [EBP + 0xfffffc1c],EAX
007ad399  8b8d1cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc1c]
007ad39f  51                    PUSH ECX
007ad3a0  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad3a6  e855fdfdff            CALL 0x0078d100
007ad3ab  c78560ffffff00000000  MOV dword ptr [EBP + 0xffffff60],0x0
007ad3b5  e914020000            JMP 0x007ad5ce
007ad3ba  8b952cffffff          MOV EDX,dword ptr [EBP + 0xffffff2c]
007ad3c0  89955cf9ffff          MOV dword ptr [EBP + 0xfffff95c],EDX
007ad3c6  8b855cf9ffff          MOV EAX,dword ptr [EBP + 0xfffff95c]
007ad3cc  8b10                  MOV EDX,dword ptr [EAX]
007ad3ce  8b8d5cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff95c]
007ad3d4  8b4270                MOV EAX,dword ptr [EDX + 0x70]
007ad3d7  ffd0                  CALL EAX
007ad3d9  898558f9ffff          MOV dword ptr [EBP + 0xfffff958],EAX
007ad3df  8b8d58f9ffff          MOV ECX,dword ptr [EBP + 0xfffff958]
007ad3e5  8b11                  MOV EDX,dword ptr [ECX]
007ad3e7  8b8d58f9ffff          MOV ECX,dword ptr [EBP + 0xfffff958]
007ad3ed  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ad3f0  ffd0                  CALL EAX
007ad3f2  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
007ad3f8  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
007ad3ff  0f84bd000000          JZ 0x007ad4c2
007ad405  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad40b  8b11                  MOV EDX,dword ptr [ECX]
007ad40d  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad413  8b4214                MOV EAX,dword ptr [EDX + 0x14]
007ad416  ffd0                  CALL EAX
007ad418  85c0                  TEST EAX,EAX
007ad41a  0f84a2000000          JZ 0x007ad4c2
007ad420  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad426  8b11                  MOV EDX,dword ptr [ECX]
007ad428  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad42e  8b4214                MOV EAX,dword ptr [EDX + 0x14]
007ad431  ffd0                  CALL EAX
007ad433  898518fcffff          MOV dword ptr [EBP + 0xfffffc18],EAX
007ad439  8b8d18fcffff          MOV ECX,dword ptr [EBP + 0xfffffc18]
007ad43f  8b913c010000          MOV EDX,dword ptr [ECX + 0x13c]
007ad445  899514fcffff          MOV dword ptr [EBP + 0xfffffc14],EDX
007ad44b  83bd14fcffff00        CMP dword ptr [EBP + 0xfffffc14],0x0
007ad452  7564                  JNZ 0x007ad4b8
007ad454  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
007ad45a  8b10                  MOV EDX,dword ptr [EAX]
007ad45c  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad462  8b4214                MOV EAX,dword ptr [EDX + 0x14]
007ad465  ffd0                  CALL EAX
007ad467  898510fcffff          MOV dword ptr [EBP + 0xfffffc10],EAX
007ad46d  8b8d10fcffff          MOV ECX,dword ptr [EBP + 0xfffffc10]
007ad473  e858030400            CALL 0x007ed7d0
007ad478  0fb7c8                MOVZX ECX,AX
007ad47b  81f942270000          CMP ECX,0x2742
007ad481  7422                  JZ 0x007ad4a5
007ad483  8b8d10fcffff          MOV ECX,dword ptr [EBP + 0xfffffc10]
007ad489  e842030400            CALL 0x007ed7d0
007ad48e  0fb7d0                MOVZX EDX,AX
007ad491  81fa43270000          CMP EDX,0x2743
007ad497  740c                  JZ 0x007ad4a5
007ad499  c78554f9ffff00000000  MOV dword ptr [EBP + 0xfffff954],0x0
007ad4a3  eb0a                  JMP 0x007ad4af
007ad4a5  c78554f9ffff01000000  MOV dword ptr [EBP + 0xfffff954],0x1
007ad4af  83bd54f9ffff00        CMP dword ptr [EBP + 0xfffff954],0x0
007ad4b6  740a                  JZ 0x007ad4c2
007ad4b8  c78560ffffff00000000  MOV dword ptr [EBP + 0xffffff60],0x0
007ad4c2  e907010000            JMP 0x007ad5ce
007ad4c7  8b852cffffff          MOV EAX,dword ptr [EBP + 0xffffff2c]
007ad4cd  898550f9ffff          MOV dword ptr [EBP + 0xfffff950],EAX
007ad4d3  8b8d50f9ffff          MOV ECX,dword ptr [EBP + 0xfffff950]
007ad4d9  8b11                  MOV EDX,dword ptr [ECX]
007ad4db  8b8d50f9ffff          MOV ECX,dword ptr [EBP + 0xfffff950]
007ad4e1  8b4270                MOV EAX,dword ptr [EDX + 0x70]
007ad4e4  ffd0                  CALL EAX
007ad4e6  83c010                ADD EAX,0x10
007ad4e9  89854cf9ffff          MOV dword ptr [EBP + 0xfffff94c],EAX
007ad4ef  8b8d4cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff94c]
007ad4f5  8b11                  MOV EDX,dword ptr [ECX]
007ad4f7  8b8d4cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff94c]
007ad4fd  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ad500  ffd0                  CALL EAX
007ad502  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
007ad508  e9c1000000            JMP 0x007ad5ce
007ad50d  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
007ad513  898d48f9ffff          MOV dword ptr [EBP + 0xfffff948],ECX
007ad519  8b9548f9ffff          MOV EDX,dword ptr [EBP + 0xfffff948]
007ad51f  8b02                  MOV EAX,dword ptr [EDX]
007ad521  8b8d48f9ffff          MOV ECX,dword ptr [EBP + 0xfffff948]
007ad527  8b5070                MOV EDX,dword ptr [EAX + 0x70]
007ad52a  ffd2                  CALL EDX
007ad52c  898544f9ffff          MOV dword ptr [EBP + 0xfffff944],EAX
007ad532  8b8544f9ffff          MOV EAX,dword ptr [EBP + 0xfffff944]
007ad538  8b10                  MOV EDX,dword ptr [EAX]
007ad53a  8b8d44f9ffff          MOV ECX,dword ptr [EBP + 0xfffff944]
007ad540  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ad543  ffd0                  CALL EAX
007ad545  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
007ad54b  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
007ad552  747a                  JZ 0x007ad5ce
007ad554  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad55a  8b11                  MOV EDX,dword ptr [ECX]
007ad55c  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad562  8b423c                MOV EAX,dword ptr [EDX + 0x3c]
007ad565  ffd0                  CALL EAX
007ad567  85c0                  TEST EAX,EAX
007ad569  7463                  JZ 0x007ad5ce
007ad56b  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad571  8b11                  MOV EDX,dword ptr [ECX]
007ad573  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad579  8b423c                MOV EAX,dword ptr [EDX + 0x3c]
007ad57c  ffd0                  CALL EAX
007ad57e  89850cfcffff          MOV dword ptr [EBP + 0xfffffc0c],EAX
007ad584  8b8d0cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc0c]
007ad58a  8b9108010000          MOV EDX,dword ptr [ECX + 0x108]
007ad590  899508fcffff          MOV dword ptr [EBP + 0xfffffc08],EDX
007ad596  83bd08fcffff00        CMP dword ptr [EBP + 0xfffffc08],0x0
007ad59d  742f                  JZ 0x007ad5ce
007ad59f  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
007ad5a5  8b10                  MOV EDX,dword ptr [EAX]
007ad5a7  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad5ad  8b423c                MOV EAX,dword ptr [EDX + 0x3c]
007ad5b0  ffd0                  CALL EAX
007ad5b2  8bc8                  MOV ECX,EAX
007ad5b4  e817020400            CALL 0x007ed7d0
007ad5b9  0fb7c8                MOVZX ECX,AX
007ad5bc  81f9a0270000          CMP ECX,0x27a0
007ad5c2  740a                  JZ 0x007ad5ce
007ad5c4  c78560ffffff00000000  MOV dword ptr [EBP + 0xffffff60],0x0
007ad5ce  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
007ad5d5  0f8492020000          JZ 0x007ad86d
007ad5db  8b9560ffffff          MOV EDX,dword ptr [EBP + 0xffffff60]
007ad5e1  8b02                  MOV EAX,dword ptr [EDX]
007ad5e3  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad5e9  8b502c                MOV EDX,dword ptr [EAX + 0x2c]
007ad5ec  ffd2                  CALL EDX
007ad5ee  85c0                  TEST EAX,EAX
007ad5f0  0f8415020000          JZ 0x007ad80b
007ad5f6  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
007ad5fc  8b10                  MOV EDX,dword ptr [EAX]
007ad5fe  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad604  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
007ad607  ffd0                  CALL EAX
007ad609  898504fcffff          MOV dword ptr [EBP + 0xfffffc04],EAX
007ad60f  8b8d04fcffff          MOV ECX,dword ptr [EBP + 0xfffffc04]
007ad615  8b91f4030000          MOV EDX,dword ptr [ECX + 0x3f4]
007ad61b  899500fcffff          MOV dword ptr [EBP + 0xfffffc00],EDX
007ad621  81bd00fcffff0000007f  CMP dword ptr [EBP + 0xfffffc00],0x7f000000
007ad62b  0f848f000000          JZ 0x007ad6c0
007ad631  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
007ad637  8b10                  MOV EDX,dword ptr [EAX]
007ad639  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad63f  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
007ad642  ffd0                  CALL EAX
007ad644  8985fcfbffff          MOV dword ptr [EBP + 0xfffffbfc],EAX
007ad64a  8b8dfcfbffff          MOV ECX,dword ptr [EBP + 0xfffffbfc]
007ad650  8b91f4030000          MOV EDX,dword ptr [ECX + 0x3f4]
007ad656  8995f8fbffff          MOV dword ptr [EBP + 0xfffffbf8],EDX
007ad65c  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007ad661  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007ad664  898df4fbffff          MOV dword ptr [EBP + 0xfffffbf4],ECX
007ad66a  8b95f8fbffff          MOV EDX,dword ptr [EBP + 0xfffffbf8]
007ad670  52                    PUSH EDX
007ad671  8b8df4fbffff          MOV ECX,dword ptr [EBP + 0xfffffbf4]
007ad677  e8541ef9ff            CALL 0x0073f4d0
007ad67c  8985d4feffff          MOV dword ptr [EBP + 0xfffffed4],EAX
007ad682  83bdd4feffff00        CMP dword ptr [EBP + 0xfffffed4],0x0
007ad689  7430                  JZ 0x007ad6bb
007ad68b  8b85d4feffff          MOV EAX,dword ptr [EBP + 0xfffffed4]
007ad691  8b10                  MOV EDX,dword ptr [EAX]
007ad693  8b8dd4feffff          MOV ECX,dword ptr [EBP + 0xfffffed4]
007ad699  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ad69c  ffd0                  CALL EAX
007ad69e  85c0                  TEST EAX,EAX
007ad6a0  7419                  JZ 0x007ad6bb
007ad6a2  8b8dd4feffff          MOV ECX,dword ptr [EBP + 0xfffffed4]
007ad6a8  8b11                  MOV EDX,dword ptr [ECX]
007ad6aa  8b8dd4feffff          MOV ECX,dword ptr [EBP + 0xfffffed4]
007ad6b0  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ad6b3  ffd0                  CALL EAX
007ad6b5  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
007ad6bb  e94b010000            JMP 0x007ad80b
007ad6c0  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad6c6  8b11                  MOV EDX,dword ptr [ECX]
007ad6c8  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad6ce  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
007ad6d1  ffd0                  CALL EAX
007ad6d3  85c0                  TEST EAX,EAX
007ad6d5  0f8430010000          JZ 0x007ad80b
007ad6db  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad6e1  8b11                  MOV EDX,dword ptr [ECX]
007ad6e3  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad6e9  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
007ad6ec  ffd0                  CALL EAX
007ad6ee  8bc8                  MOV ECX,EAX
007ad6f0  e80b01fdff            CALL 0x0077d800
007ad6f5  85c0                  TEST EAX,EAX
007ad6f7  0f84a6000000          JZ 0x007ad7a3
007ad6fd  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad703  8b11                  MOV EDX,dword ptr [ECX]
007ad705  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad70b  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
007ad70e  ffd0                  CALL EAX
007ad710  8bc8                  MOV ECX,EAX
007ad712  e8e900fdff            CALL 0x0077d800
007ad717  898540f9ffff          MOV dword ptr [EBP + 0xfffff940],EAX
007ad71d  8b8d40f9ffff          MOV ECX,dword ptr [EBP + 0xfffff940]
007ad723  8b11                  MOV EDX,dword ptr [ECX]
007ad725  8b8d40f9ffff          MOV ECX,dword ptr [EBP + 0xfffff940]
007ad72b  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
007ad731  ffd0                  CALL EAX
007ad733  85c0                  TEST EAX,EAX
007ad735  756c                  JNZ 0x007ad7a3
007ad737  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad73d  8b11                  MOV EDX,dword ptr [ECX]
007ad73f  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad745  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
007ad748  ffd0                  CALL EAX
007ad74a  8bc8                  MOV ECX,EAX
007ad74c  e8af00fdff            CALL 0x0077d800
007ad751  8985f0fbffff          MOV dword ptr [EBP + 0xfffffbf0],EAX
007ad757  8b8df0fbffff          MOV ECX,dword ptr [EBP + 0xfffffbf0]
007ad75d  83b9ac11000000        CMP dword ptr [ECX + 0x11ac],0x0
007ad764  742a                  JZ 0x007ad790
007ad766  6a00                  PUSH 0x0
007ad768  8b95f0fbffff          MOV EDX,dword ptr [EBP + 0xfffffbf0]
007ad76e  8b02                  MOV EAX,dword ptr [EDX]
007ad770  8b8df0fbffff          MOV ECX,dword ptr [EBP + 0xfffffbf0]
007ad776  8b909c000000          MOV EDX,dword ptr [EAX + 0x9c]
007ad77c  ffd2                  CALL EDX
007ad77e  98                    CWDE
007ad77f  33c9                  XOR ECX,ECX
007ad781  85c0                  TEST EAX,EAX
007ad783  0f9ec1                SETLE CL
007ad786  898decfbffff          MOV dword ptr [EBP + 0xfffffbec],ECX
007ad78c  eb0c                  JMP 0x007ad79a
007ad790  c785ecfbffff00000000  MOV dword ptr [EBP + 0xfffffbec],0x0
007ad79a  83bdecfbffff00        CMP dword ptr [EBP + 0xfffffbec],0x0
007ad7a1  7468                  JZ 0x007ad80b
007ad7a3  8b9560ffffff          MOV EDX,dword ptr [EBP + 0xffffff60]
007ad7a9  8b02                  MOV EAX,dword ptr [EDX]
007ad7ab  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad7b1  8b502c                MOV EDX,dword ptr [EAX + 0x2c]
007ad7b4  ffd2                  CALL EDX
007ad7b6  8985e8fbffff          MOV dword ptr [EBP + 0xfffffbe8],EAX
007ad7bc  8b85e8fbffff          MOV EAX,dword ptr [EBP + 0xfffffbe8]
007ad7c2  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007ad7c5  898de4fbffff          MOV dword ptr [EBP + 0xfffffbe4],ECX
007ad7cb  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad7d1  e8aa97f7ff            CALL 0x00726f80
007ad7d6  8985e0fbffff          MOV dword ptr [EBP + 0xfffffbe0],EAX
007ad7dc  8b95e0fbffff          MOV EDX,dword ptr [EBP + 0xfffffbe0]
007ad7e2  8b4248                MOV EAX,dword ptr [EDX + 0x48]
007ad7e5  8985dcfbffff          MOV dword ptr [EBP + 0xfffffbdc],EAX
007ad7eb  8b8de4fbffff          MOV ECX,dword ptr [EBP + 0xfffffbe4]
007ad7f1  51                    PUSH ECX
007ad7f2  8b8ddcfbffff          MOV ECX,dword ptr [EBP + 0xfffffbdc]
007ad7f8  e8a3a1ffff            CALL 0x007a79a0
007ad7fd  85c0                  TEST EAX,EAX
007ad7ff  740a                  JZ 0x007ad80b
007ad801  c78560ffffff00000000  MOV dword ptr [EBP + 0xffffff60],0x0
007ad80b  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
007ad812  7459                  JZ 0x007ad86d
007ad814  8b9560ffffff          MOV EDX,dword ptr [EBP + 0xffffff60]
007ad81a  8b02                  MOV EAX,dword ptr [EDX]
007ad81c  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad822  8b5034                MOV EDX,dword ptr [EAX + 0x34]
007ad825  ffd2                  CALL EDX
007ad827  85c0                  TEST EAX,EAX
007ad829  7442                  JZ 0x007ad86d
007ad82b  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
007ad831  8b10                  MOV EDX,dword ptr [EAX]
007ad833  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad839  8b4234                MOV EAX,dword ptr [EDX + 0x34]
007ad83c  ffd0                  CALL EAX
007ad83e  8bc8                  MOV ECX,EAX
007ad840  e8fbff0400            CALL 0x007fd840
007ad845  8985d0feffff          MOV dword ptr [EBP + 0xfffffed0],EAX
007ad84b  83bdd0feffff00        CMP dword ptr [EBP + 0xfffffed0],0x0
007ad852  7419                  JZ 0x007ad86d
007ad854  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
007ad85a  8b11                  MOV EDX,dword ptr [ECX]
007ad85c  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
007ad862  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ad865  ffd0                  CALL EAX
007ad867  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
007ad86d  e972f8ffff            JMP 0x007ad0e4
007ad872  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
007ad879  0f8404040000          JZ 0x007adc83
007ad87f  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad885  e856f6fdff            CALL 0x0078cee0
007ad88a  85c0                  TEST EAX,EAX
007ad88c  0f84f1030000          JZ 0x007adc83
007ad892  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad898  e8e396f7ff            CALL 0x00726f80
007ad89d  8985d8fbffff          MOV dword ptr [EBP + 0xfffffbd8],EAX
007ad8a3  8b8dd4fbffff          MOV ECX,dword ptr [EBP + 0xfffffbd4]
007ad8a9  898d3cf9ffff          MOV dword ptr [EBP + 0xfffff93c],ECX
007ad8af  8b95d8fbffff          MOV EDX,dword ptr [EBP + 0xfffffbd8]
007ad8b5  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007ad8b8  89853cf9ffff          MOV dword ptr [EBP + 0xfffff93c],EAX
007ad8be  8d8d8cfeffff          LEA ECX,[EBP + 0xfffffe8c]
007ad8c4  51                    PUSH ECX
007ad8c5  8b953cf9ffff          MOV EDX,dword ptr [EBP + 0xfffff93c]
007ad8cb  8b02                  MOV EAX,dword ptr [EDX]
007ad8cd  8b8d3cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff93c]
007ad8d3  8b5020                MOV EDX,dword ptr [EAX + 0x20]
007ad8d6  ffd2                  CALL EDX
007ad8d8  d9ee                  FLDZ
007ad8da  d99d98feffff          FSTP float ptr [EBP + 0xfffffe98]
007ad8e0  d9ee                  FLDZ
007ad8e2  d99d9cfeffff          FSTP float ptr [EBP + 0xfffffe9c]
007ad8e8  d9ee                  FLDZ
007ad8ea  d99da0feffff          FSTP float ptr [EBP + 0xfffffea0]
007ad8f0  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad8f6  e8e5f5fdff            CALL 0x0078cee0
007ad8fb  85c0                  TEST EAX,EAX
007ad8fd  7439                  JZ 0x007ad938
007ad8ff  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad905  e8d6f5fdff            CALL 0x0078cee0
007ad90a  8985d0fbffff          MOV dword ptr [EBP + 0xfffffbd0],EAX
007ad910  8b85d0fbffff          MOV EAX,dword ptr [EBP + 0xfffffbd0]
007ad916  83c024                ADD EAX,0x24
007ad919  8b08                  MOV ECX,dword ptr [EAX]
007ad91b  898d98feffff          MOV dword ptr [EBP + 0xfffffe98],ECX
007ad921  8b5004                MOV EDX,dword ptr [EAX + 0x4]
007ad924  89959cfeffff          MOV dword ptr [EBP + 0xfffffe9c],EDX
007ad92a  8b4008                MOV EAX,dword ptr [EAX + 0x8]
007ad92d  8985a0feffff          MOV dword ptr [EBP + 0xfffffea0],EAX
007ad933  e986000000            JMP 0x007ad9be
007ad938  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ad93e  8b9170020000          MOV EDX,dword ptr [ECX + 0x270]
007ad944  8995ccfbffff          MOV dword ptr [EBP + 0xfffffbcc],EDX
007ad94a  83bdccfbffff00        CMP dword ptr [EBP + 0xfffffbcc],0x0
007ad951  746b                  JZ 0x007ad9be
007ad953  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ad959  8b8870020000          MOV ECX,dword ptr [EAX + 0x270]
007ad95f  898dc8fbffff          MOV dword ptr [EBP + 0xfffffbc8],ECX
007ad965  6a00                  PUSH 0x0
007ad967  8b8dc8fbffff          MOV ECX,dword ptr [EBP + 0xfffffbc8]
007ad96d  e82e840300            CALL 0x007e5da0
007ad972  85c0                  TEST EAX,EAX
007ad974  7448                  JZ 0x007ad9be
007ad976  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ad97c  8b8270020000          MOV EAX,dword ptr [EDX + 0x270]
007ad982  8985c4fbffff          MOV dword ptr [EBP + 0xfffffbc4],EAX
007ad988  6a00                  PUSH 0x0
007ad98a  8b8dc4fbffff          MOV ECX,dword ptr [EBP + 0xfffffbc4]
007ad990  e80b840300            CALL 0x007e5da0
007ad995  8985c0fbffff          MOV dword ptr [EBP + 0xfffffbc0],EAX
007ad99b  8b8dc0fbffff          MOV ECX,dword ptr [EBP + 0xfffffbc0]
007ad9a1  83c124                ADD ECX,0x24
007ad9a4  8b11                  MOV EDX,dword ptr [ECX]
007ad9a6  899598feffff          MOV dword ptr [EBP + 0xfffffe98],EDX
007ad9ac  8b4104                MOV EAX,dword ptr [ECX + 0x4]
007ad9af  89859cfeffff          MOV dword ptr [EBP + 0xfffffe9c],EAX
007ad9b5  8b4908                MOV ECX,dword ptr [ECX + 0x8]
007ad9b8  898da0feffff          MOV dword ptr [EBP + 0xfffffea0],ECX
007ad9be  8b9560ffffff          MOV EDX,dword ptr [EBP + 0xffffff60]
007ad9c4  8b02                  MOV EAX,dword ptr [EDX]
007ad9c6  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ad9cc  8b500c                MOV EDX,dword ptr [EAX + 0xc]
007ad9cf  ffd2                  CALL EDX
007ad9d1  8985bcfbffff          MOV dword ptr [EBP + 0xfffffbbc],EAX
007ad9d7  8b85bcfbffff          MOV EAX,dword ptr [EBP + 0xfffffbbc]
007ad9dd  83c024                ADD EAX,0x24
007ad9e0  8b08                  MOV ECX,dword ptr [EAX]
007ad9e2  898d80feffff          MOV dword ptr [EBP + 0xfffffe80],ECX
007ad9e8  8b5004                MOV EDX,dword ptr [EAX + 0x4]
007ad9eb  899584feffff          MOV dword ptr [EBP + 0xfffffe84],EDX
007ad9f1  8b4008                MOV EAX,dword ptr [EAX + 0x8]
007ad9f4  898588feffff          MOV dword ptr [EBP + 0xfffffe88],EAX
007ad9fa  8d8d8cfeffff          LEA ECX,[EBP + 0xfffffe8c]
007ada00  51                    PUSH ECX
007ada01  8d9598feffff          LEA EDX,[EBP + 0xfffffe98]
007ada07  52                    PUSH EDX
007ada08  8d85b4feffff          LEA EAX,[EBP + 0xfffffeb4]
007ada0e  50                    PUSH EAX
007ada0f  e89c78c9ff            CALL 0x004452b0
007ada14  83c40c                ADD ESP,0xc
007ada17  d9ee                  FLDZ
007ada19  d99dbcfeffff          FSTP float ptr [EBP + 0xfffffebc]
007ada1f  8d8d8cfeffff          LEA ECX,[EBP + 0xfffffe8c]
007ada25  51                    PUSH ECX
007ada26  8d9580feffff          LEA EDX,[EBP + 0xfffffe80]
007ada2c  52                    PUSH EDX
007ada2d  8d85a4feffff          LEA EAX,[EBP + 0xfffffea4]
007ada33  50                    PUSH EAX
007ada34  e87778c9ff            CALL 0x004452b0
007ada39  83c40c                ADD ESP,0xc
007ada3c  d9ee                  FLDZ
007ada3e  d99dacfeffff          FSTP float ptr [EBP + 0xfffffeac]
007ada44  8d8d98feffff          LEA ECX,[EBP + 0xfffffe98]
007ada4a  51                    PUSH ECX
007ada4b  8d9580feffff          LEA EDX,[EBP + 0xfffffe80]
007ada51  52                    PUSH EDX
007ada52  8d85c0feffff          LEA EAX,[EBP + 0xfffffec0]
007ada58  50                    PUSH EAX
007ada59  e85278c9ff            CALL 0x004452b0
007ada5e  83c40c                ADD ESP,0xc
007ada61  d9ee                  FLDZ
007ada63  d99dc8feffff          FSTP float ptr [EBP + 0xfffffec8]
007ada69  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ada6f  8b11                  MOV EDX,dword ptr [ECX]
007ada71  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007ada77  8b8238010000          MOV EAX,dword ptr [EDX + 0x138]
007ada7d  ffd0                  CALL EAX
007ada7f  85c0                  TEST EAX,EAX
007ada81  740e                  JZ 0x007ada91
007ada83  d905f0199900          FLD float ptr [0x009919f0]
007ada89  d99d90fbffff          FSTP float ptr [EBP + 0xfffffb90]
007ada8f  eb0c                  JMP 0x007ada9d
007ada91  d90504779800          FLD float ptr [0x00987704]
007ada97  d99d90fbffff          FSTP float ptr [EBP + 0xfffffb90]
007ada9d  d985c0feffff          FLD float ptr [EBP + 0xfffffec0]
007adaa3  d88dc0feffff          FMUL float ptr [EBP + 0xfffffec0]
007adaa9  d985c4feffff          FLD float ptr [EBP + 0xfffffec4]
007adaaf  d88dc4feffff          FMUL float ptr [EBP + 0xfffffec4]
007adab5  dec1                  FADDP
007adab7  d985c8feffff          FLD float ptr [EBP + 0xfffffec8]
007adabd  d88dc8feffff          FMUL float ptr [EBP + 0xfffffec8]
007adac3  dec1                  FADDP
007adac5  d99d94fbffff          FSTP float ptr [EBP + 0xfffffb94]
007adacb  d98594fbffff          FLD float ptr [EBP + 0xfffffb94]
007adad1  d98590fbffff          FLD float ptr [EBP + 0xfffffb90]
007adad7  d88d90fbffff          FMUL float ptr [EBP + 0xfffffb90]
007adadd  d99d38f9ffff          FSTP float ptr [EBP + 0xfffff938]
007adae3  d98538f9ffff          FLD float ptr [EBP + 0xfffff938]
007adae9  ded9                  FCOMPP
007adaeb  dfe0                  FNSTSW AX
007adaed  f6c405                TEST AH,0x5
007adaf0  7a0c                  JP 0x007adafe
007adaf2  c78534f9ffff01000000  MOV dword ptr [EBP + 0xfffff934],0x1
007adafc  eb0a                  JMP 0x007adb08
007adafe  c78534f9ffff00000000  MOV dword ptr [EBP + 0xfffff934],0x0
007adb08  8b8d34f9ffff          MOV ECX,dword ptr [EBP + 0xfffff934]
007adb0e  898dccfeffff          MOV dword ptr [EBP + 0xfffffecc],ECX
007adb14  d985acfeffff          FLD float ptr [EBP + 0xfffffeac]
007adb1a  d88dbcfeffff          FMUL float ptr [EBP + 0xfffffebc]
007adb20  d985a8feffff          FLD float ptr [EBP + 0xfffffea8]
007adb26  d88db8feffff          FMUL float ptr [EBP + 0xfffffeb8]
007adb2c  d985a4feffff          FLD float ptr [EBP + 0xfffffea4]
007adb32  d88db4feffff          FMUL float ptr [EBP + 0xfffffeb4]
007adb38  dec1                  FADDP
007adb3a  dec1                  FADDP
007adb3c  d99d30f9ffff          FSTP float ptr [EBP + 0xfffff930]
007adb42  d98530f9ffff          FLD float ptr [EBP + 0xfffff930]
007adb48  dc1d386b9800          FCOMP double ptr [0x00986b38]
007adb4e  dfe0                  FNSTSW AX
007adb50  f6c401                TEST AH,0x1
007adb53  750c                  JNZ 0x007adb61
007adb55  c7852cf9ffff01000000  MOV dword ptr [EBP + 0xfffff92c],0x1
007adb5f  eb0a                  JMP 0x007adb6b
007adb61  c7852cf9ffff00000000  MOV dword ptr [EBP + 0xfffff92c],0x0
007adb6b  8b952cf9ffff          MOV EDX,dword ptr [EBP + 0xfffff92c]
007adb71  8995b0feffff          MOV dword ptr [EBP + 0xfffffeb0],EDX
007adb77  83bdb0feffff00        CMP dword ptr [EBP + 0xfffffeb0],0x0
007adb7e  0f84ab000000          JZ 0x007adc2f
007adb84  83bdccfeffff00        CMP dword ptr [EBP + 0xfffffecc],0x0
007adb8b  0f859e000000          JNZ 0x007adc2f
007adb91  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007adb97  8b8874020000          MOV ECX,dword ptr [EAX + 0x274]
007adb9d  8b5108                MOV EDX,dword ptr [ECX + 0x8]
007adba0  89958cfbffff          MOV dword ptr [EBP + 0xfffffb8c],EDX
007adba6  83bd8cfbffff00        CMP dword ptr [EBP + 0xfffffb8c],0x0
007adbad  7427                  JZ 0x007adbd6
007adbaf  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007adbb5  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007adbb8  8b9198000000          MOV EDX,dword ptr [ECX + 0x98]
007adbbe  899588fbffff          MOV dword ptr [EBP + 0xfffffb88],EDX
007adbc4  8b8588fbffff          MOV EAX,dword ptr [EBP + 0xfffffb88]
007adbca  c780585900000000007f  MOV dword ptr [EAX + 0x5958],0x7f000000
007adbd4  eb57                  JMP 0x007adc2d
007adbd6  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
007adbdd  741d                  JZ 0x007adbfc
007adbdf  8b8d84fbffff          MOV ECX,dword ptr [EBP + 0xfffffb84]
007adbe5  898d7cfbffff          MOV dword ptr [EBP + 0xfffffb7c],ECX
007adbeb  8b9560ffffff          MOV EDX,dword ptr [EBP + 0xffffff60]
007adbf1  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007adbf4  89857cfbffff          MOV dword ptr [EBP + 0xfffffb7c],EAX
007adbfa  eb0a                  JMP 0x007adc06
007adbfc  c7857cfbffff0000007f  MOV dword ptr [EBP + 0xfffffb7c],0x7f000000
007adc06  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007adc0c  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007adc0f  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
007adc15  898580fbffff          MOV dword ptr [EBP + 0xfffffb80],EAX
007adc1b  8b8d80fbffff          MOV ECX,dword ptr [EBP + 0xfffffb80]
007adc21  8b957cfbffff          MOV EDX,dword ptr [EBP + 0xfffffb7c]
007adc27  899158590000          MOV dword ptr [ECX + 0x5958],EDX
007adc2d  eb25                  JMP 0x007adc54
007adc2f  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007adc35  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007adc38  8b9198000000          MOV EDX,dword ptr [ECX + 0x98]
007adc3e  899578fbffff          MOV dword ptr [EBP + 0xfffffb78],EDX
007adc44  8b8578fbffff          MOV EAX,dword ptr [EBP + 0xfffffb78]
007adc4a  c780585900000000007f  MOV dword ptr [EAX + 0x5958],0x7f000000
007adc54  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
007adc5a  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007adc5d  899574fbffff          MOV dword ptr [EBP + 0xfffffb74],EDX
007adc63  6a00                  PUSH 0x0
007adc65  8d85e8feffff          LEA EAX,[EBP + 0xfffffee8]
007adc6b  50                    PUSH EAX
007adc6c  8b8d74fbffff          MOV ECX,dword ptr [EBP + 0xfffffb74]
007adc72  51                    PUSH ECX
007adc73  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007adc79  e8825d0000            CALL 0x007b3a00
007adc7e  e975080000            JMP 0x007ae4f8
007adc83  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007adc89  e8f292f7ff            CALL 0x00726f80
007adc8e  8bc8                  MOV ECX,EAX
007adc90  e89bd10400            CALL 0x007fae30
007adc95  85c0                  TEST EAX,EAX
007adc97  0f851d080000          JNZ 0x007ae4ba
007adc9d  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007adca3  e838f2fdff            CALL 0x0078cee0
007adca8  85c0                  TEST EAX,EAX
007adcaa  0f840a080000          JZ 0x007ae4ba
007adcb0  d9ee                  FLDZ
007adcb2  d99d54feffff          FSTP float ptr [EBP + 0xfffffe54]
007adcb8  d9ee                  FLDZ
007adcba  d99d58feffff          FSTP float ptr [EBP + 0xfffffe58]
007adcc0  d9ee                  FLDZ
007adcc2  d99d5cfeffff          FSTP float ptr [EBP + 0xfffffe5c]
007adcc8  d9ee                  FLDZ
007adcca  d99d70feffff          FSTP float ptr [EBP + 0xfffffe70]
007adcd0  d9ee                  FLDZ
007adcd2  d99d74feffff          FSTP float ptr [EBP + 0xfffffe74]
007adcd8  d9ee                  FLDZ
007adcda  d99d78feffff          FSTP float ptr [EBP + 0xfffffe78]
007adce0  8b159cb4a100          MOV EDX,dword ptr [0x00a1b49c]
007adce6  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007adce9  898570fbffff          MOV dword ptr [EBP + 0xfffffb70],EAX
007adcef  8b0d9cb4a100          MOV ECX,dword ptr [0x00a1b49c]
007adcf5  8b11                  MOV EDX,dword ptr [ECX]
007adcf7  89956cfbffff          MOV dword ptr [EBP + 0xfffffb6c],EDX
007adcfd  db856cfbffff          FILD dword ptr [EBP + 0xfffffb6c]
007add03  d99d60feffff          FSTP float ptr [EBP + 0xfffffe60]
007add09  db8570fbffff          FILD dword ptr [EBP + 0xfffffb70]
007add0f  d99d64feffff          FSTP float ptr [EBP + 0xfffffe64]
007add15  d9ee                  FLDZ
007add17  d99d68feffff          FSTP float ptr [EBP + 0xfffffe68]
007add1d  d90500399900          FLD float ptr [0x00993900]
007add23  d99d7cfeffff          FSTP float ptr [EBP + 0xfffffe7c]
007add29  c78544feffff00000000  MOV dword ptr [EBP + 0xfffffe44],0x0
007add33  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007add39  e8a2f1fdff            CALL 0x0078cee0
007add3e  898548feffff          MOV dword ptr [EBP + 0xfffffe48],EAX
007add44  c7856cfeffff00000000  MOV dword ptr [EBP + 0xfffffe6c],0x0
007add4e  eb0f                  JMP 0x007add5f
007add50  8b856cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe6c]
007add56  83c001                ADD EAX,0x1
007add59  89856cfeffff          MOV dword ptr [EBP + 0xfffffe6c],EAX
007add5f  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007add65  8b91ac020000          MOV EDX,dword ptr [ECX + 0x2ac]
007add6b  899568fbffff          MOV dword ptr [EBP + 0xfffffb68],EDX
007add71  8b856cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe6c]
007add77  3b8568fbffff          CMP EAX,dword ptr [EBP + 0xfffffb68]
007add7d  0f8dbf040000          JGE 0x007ae242
007add83  8b8d6cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe6c]
007add89  6bc90c                IMUL ECX,ECX,0xc
007add8c  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007add92  038aa8020000          ADD ECX,dword ptr [EDX + 0x2a8]
007add98  898d64fbffff          MOV dword ptr [EBP + 0xfffffb64],ECX
007add9e  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
007adda4  8b08                  MOV ECX,dword ptr [EAX]
007adda6  898d58fbffff          MOV dword ptr [EBP + 0xfffffb58],ECX
007addac  c78560fbffff00000000  MOV dword ptr [EBP + 0xfffffb60],0x0
007addb6  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007addbc  8b4214                MOV EAX,dword ptr [EDX + 0x14]
007addbf  89855cfbffff          MOV dword ptr [EBP + 0xfffffb5c],EAX
007addc5  8d8d60fbffff          LEA ECX,[EBP + 0xfffffb60]
007addcb  51                    PUSH ECX
007addcc  8b9558fbffff          MOV EDX,dword ptr [EBP + 0xfffffb58]
007addd2  52                    PUSH EDX
007addd3  8b8d5cfbffff          MOV ECX,dword ptr [EBP + 0xfffffb5c]
007addd9  e8d201d9ff            CALL 0x0053dfb0
007addde  8b8560fbffff          MOV EAX,dword ptr [EBP + 0xfffffb60]
007adde4  898550feffff          MOV dword ptr [EBP + 0xfffffe50],EAX
007addea  83bd50feffff00        CMP dword ptr [EBP + 0xfffffe50],0x0
007addf1  0f8446040000          JZ 0x007ae23d
007addf7  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007addfd  8b11                  MOV EDX,dword ptr [ECX]
007addff  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007ade05  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ade08  ffd0                  CALL EAX
007ade0a  85c0                  TEST EAX,EAX
007ade0c  0f842b040000          JZ 0x007ae23d
007ade12  8b8d6cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe6c]
007ade18  6bc90c                IMUL ECX,ECX,0xc
007ade1b  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ade21  8b82a8020000          MOV EAX,dword ptr [EDX + 0x2a8]
007ade27  0fbe4c0804            MOVSX ECX,byte ptr [EAX + ECX*0x1 + 0x4]
007ade2c  83f9ff                CMP ECX,-0x1
007ade2f  7543                  JNZ 0x007ade74
007ade31  8b956cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe6c]
007ade37  6bd20c                IMUL EDX,EDX,0xc
007ade3a  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ade40  0390a8020000          ADD EDX,dword ptr [EAX + 0x2a8]
007ade46  899554fbffff          MOV dword ptr [EBP + 0xfffffb54],EDX
007ade4c  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007ade52  8b11                  MOV EDX,dword ptr [ECX]
007ade54  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007ade5a  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ade5d  ffd0                  CALL EAX
007ade5f  50                    PUSH EAX
007ade60  8b8d48feffff          MOV ECX,dword ptr [EBP + 0xfffffe48]
007ade66  e8a5a6fcff            CALL 0x00778510
007ade6b  8b8d54fbffff          MOV ECX,dword ptr [EBP + 0xfffffb54]
007ade71  884104                MOV byte ptr [ECX + 0x4],AL
007ade74  8b956cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe6c]
007ade7a  6bd20c                IMUL EDX,EDX,0xc
007ade7d  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ade83  8b88a8020000          MOV ECX,dword ptr [EAX + 0x2a8]
007ade89  0fbe541104            MOVSX EDX,byte ptr [ECX + EDX*0x1 + 0x4]
007ade8e  85d2                  TEST EDX,EDX
007ade90  0f84a7030000          JZ 0x007ae23d
007ade96  8b8550feffff          MOV EAX,dword ptr [EBP + 0xfffffe50]
007ade9c  8b10                  MOV EDX,dword ptr [EAX]
007ade9e  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007adea4  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007adea7  ffd0                  CALL EAX
007adea9  898528f9ffff          MOV dword ptr [EBP + 0xfffff928],EAX
007adeaf  8d8d54feffff          LEA ECX,[EBP + 0xfffffe54]
007adeb5  51                    PUSH ECX
007adeb6  8b9528f9ffff          MOV EDX,dword ptr [EBP + 0xfffff928]
007adebc  8b02                  MOV EAX,dword ptr [EDX]
007adebe  8b8d28f9ffff          MOV ECX,dword ptr [EBP + 0xfffff928]
007adec4  8b903c010000          MOV EDX,dword ptr [EAX + 0x13c]
007adeca  ffd2                  CALL EDX
007adecc  d98560feffff          FLD float ptr [EBP + 0xfffffe60]
007aded2  d8ad54feffff          FSUBR float ptr [EBP + 0xfffffe54]
007aded8  d99d48fbffff          FSTP float ptr [EBP + 0xfffffb48]
007adede  d98564feffff          FLD float ptr [EBP + 0xfffffe64]
007adee4  d8ad58feffff          FSUBR float ptr [EBP + 0xfffffe58]
007adeea  d99d4cfbffff          FSTP float ptr [EBP + 0xfffffb4c]
007adef0  d98568feffff          FLD float ptr [EBP + 0xfffffe68]
007adef6  d8ad5cfeffff          FSUBR float ptr [EBP + 0xfffffe5c]
007adefc  d99d50fbffff          FSTP float ptr [EBP + 0xfffffb50]
007adf02  d98548fbffff          FLD float ptr [EBP + 0xfffffb48]
007adf08  d99d64fdffff          FSTP float ptr [EBP + 0xfffffd64]
007adf0e  d9854cfbffff          FLD float ptr [EBP + 0xfffffb4c]
007adf14  d99d68fdffff          FSTP float ptr [EBP + 0xfffffd68]
007adf1a  d98550fbffff          FLD float ptr [EBP + 0xfffffb50]
007adf20  d99d6cfdffff          FSTP float ptr [EBP + 0xfffffd6c]
007adf26  8b8564fdffff          MOV EAX,dword ptr [EBP + 0xfffffd64]
007adf2c  898570feffff          MOV dword ptr [EBP + 0xfffffe70],EAX
007adf32  8b8d68fdffff          MOV ECX,dword ptr [EBP + 0xfffffd68]
007adf38  898d74feffff          MOV dword ptr [EBP + 0xfffffe74],ECX
007adf3e  8b956cfdffff          MOV EDX,dword ptr [EBP + 0xfffffd6c]
007adf44  899578feffff          MOV dword ptr [EBP + 0xfffffe78],EDX
007adf4a  d98578feffff          FLD float ptr [EBP + 0xfffffe78]
007adf50  d88d78feffff          FMUL float ptr [EBP + 0xfffffe78]
007adf56  d98574feffff          FLD float ptr [EBP + 0xfffffe74]
007adf5c  d88d74feffff          FMUL float ptr [EBP + 0xfffffe74]
007adf62  d98570feffff          FLD float ptr [EBP + 0xfffffe70]
007adf68  d88d70feffff          FMUL float ptr [EBP + 0xfffffe70]
007adf6e  dec1                  FADDP
007adf70  dec1                  FADDP
007adf72  d99d4cfeffff          FSTP float ptr [EBP + 0xfffffe4c]
007adf78  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007adf7e  e8fd8ff7ff            CALL 0x00726f80
007adf83  898544fbffff          MOV dword ptr [EBP + 0xfffffb44],EAX
007adf89  8b8540fbffff          MOV EAX,dword ptr [EBP + 0xfffffb40]
007adf8f  898524f9ffff          MOV dword ptr [EBP + 0xfffff924],EAX
007adf95  8b8d44fbffff          MOV ECX,dword ptr [EBP + 0xfffffb44]
007adf9b  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007adf9e  899524f9ffff          MOV dword ptr [EBP + 0xfffff924],EDX
007adfa4  8d8500feffff          LEA EAX,[EBP + 0xfffffe00]
007adfaa  50                    PUSH EAX
007adfab  8b8d24f9ffff          MOV ECX,dword ptr [EBP + 0xfffff924]
007adfb1  8b11                  MOV EDX,dword ptr [ECX]
007adfb3  8b8d24f9ffff          MOV ECX,dword ptr [EBP + 0xfffff924]
007adfb9  8b4220                MOV EAX,dword ptr [EDX + 0x20]
007adfbc  ffd0                  CALL EAX
007adfbe  8b8d48feffff          MOV ECX,dword ptr [EBP + 0xfffffe48]
007adfc4  83c124                ADD ECX,0x24
007adfc7  8b11                  MOV EDX,dword ptr [ECX]
007adfc9  89950cfeffff          MOV dword ptr [EBP + 0xfffffe0c],EDX
007adfcf  8b4104                MOV EAX,dword ptr [ECX + 0x4]
007adfd2  898510feffff          MOV dword ptr [EBP + 0xfffffe10],EAX
007adfd8  8b4908                MOV ECX,dword ptr [ECX + 0x8]
007adfdb  898d14feffff          MOV dword ptr [EBP + 0xfffffe14],ECX
007adfe1  8b9550feffff          MOV EDX,dword ptr [EBP + 0xfffffe50]
007adfe7  8b02                  MOV EAX,dword ptr [EDX]
007adfe9  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007adfef  8b500c                MOV EDX,dword ptr [EAX + 0xc]
007adff2  ffd2                  CALL EDX
007adff4  89853cfbffff          MOV dword ptr [EBP + 0xfffffb3c],EAX
007adffa  8b853cfbffff          MOV EAX,dword ptr [EBP + 0xfffffb3c]
007ae000  83c024                ADD EAX,0x24
007ae003  8b08                  MOV ECX,dword ptr [EAX]
007ae005  898df4fdffff          MOV dword ptr [EBP + 0xfffffdf4],ECX
007ae00b  8b5004                MOV EDX,dword ptr [EAX + 0x4]
007ae00e  8995f8fdffff          MOV dword ptr [EBP + 0xfffffdf8],EDX
007ae014  8b4008                MOV EAX,dword ptr [EAX + 0x8]
007ae017  8985fcfdffff          MOV dword ptr [EBP + 0xfffffdfc],EAX
007ae01d  8d8d00feffff          LEA ECX,[EBP + 0xfffffe00]
007ae023  51                    PUSH ECX
007ae024  8d950cfeffff          LEA EDX,[EBP + 0xfffffe0c]
007ae02a  52                    PUSH EDX
007ae02b  8d8528feffff          LEA EAX,[EBP + 0xfffffe28]
007ae031  50                    PUSH EAX
007ae032  e87972c9ff            CALL 0x004452b0
007ae037  83c40c                ADD ESP,0xc
007ae03a  d9ee                  FLDZ
007ae03c  d99d30feffff          FSTP float ptr [EBP + 0xfffffe30]
007ae042  8d8d00feffff          LEA ECX,[EBP + 0xfffffe00]
007ae048  51                    PUSH ECX
007ae049  8d95f4fdffff          LEA EDX,[EBP + 0xfffffdf4]
007ae04f  52                    PUSH EDX
007ae050  8d8518feffff          LEA EAX,[EBP + 0xfffffe18]
007ae056  50                    PUSH EAX
007ae057  e85472c9ff            CALL 0x004452b0
007ae05c  83c40c                ADD ESP,0xc
007ae05f  d9ee                  FLDZ
007ae061  d99d20feffff          FSTP float ptr [EBP + 0xfffffe20]
007ae067  8d8d0cfeffff          LEA ECX,[EBP + 0xfffffe0c]
007ae06d  51                    PUSH ECX
007ae06e  8d95f4fdffff          LEA EDX,[EBP + 0xfffffdf4]
007ae074  52                    PUSH EDX
007ae075  8d8534feffff          LEA EAX,[EBP + 0xfffffe34]
007ae07b  50                    PUSH EAX
007ae07c  e82f72c9ff            CALL 0x004452b0
007ae081  83c40c                ADD ESP,0xc
007ae084  d9ee                  FLDZ
007ae086  d99d3cfeffff          FSTP float ptr [EBP + 0xfffffe3c]
007ae08c  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007ae092  8b11                  MOV EDX,dword ptr [ECX]
007ae094  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007ae09a  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ae09d  ffd0                  CALL EAX
007ae09f  898520f9ffff          MOV dword ptr [EBP + 0xfffff920],EAX
007ae0a5  8b8d20f9ffff          MOV ECX,dword ptr [EBP + 0xfffff920]
007ae0ab  8b11                  MOV EDX,dword ptr [ECX]
007ae0ad  8b8d20f9ffff          MOV ECX,dword ptr [EBP + 0xfffff920]
007ae0b3  8b8238010000          MOV EAX,dword ptr [EDX + 0x138]
007ae0b9  ffd0                  CALL EAX
007ae0bb  85c0                  TEST EAX,EAX
007ae0bd  740e                  JZ 0x007ae0cd
007ae0bf  d905f0199900          FLD float ptr [0x009919f0]
007ae0c5  d99d10fbffff          FSTP float ptr [EBP + 0xfffffb10]
007ae0cb  eb0c                  JMP 0x007ae0d9
007ae0cd  d90504779800          FLD float ptr [0x00987704]
007ae0d3  d99d10fbffff          FSTP float ptr [EBP + 0xfffffb10]
007ae0d9  d98534feffff          FLD float ptr [EBP + 0xfffffe34]
007ae0df  d88d34feffff          FMUL float ptr [EBP + 0xfffffe34]
007ae0e5  d98538feffff          FLD float ptr [EBP + 0xfffffe38]
007ae0eb  d88d38feffff          FMUL float ptr [EBP + 0xfffffe38]
007ae0f1  dec1                  FADDP
007ae0f3  d9853cfeffff          FLD float ptr [EBP + 0xfffffe3c]
007ae0f9  d88d3cfeffff          FMUL float ptr [EBP + 0xfffffe3c]
007ae0ff  dec1                  FADDP
007ae101  d99d14fbffff          FSTP float ptr [EBP + 0xfffffb14]
007ae107  d98514fbffff          FLD float ptr [EBP + 0xfffffb14]
007ae10d  d98510fbffff          FLD float ptr [EBP + 0xfffffb10]
007ae113  d88d10fbffff          FMUL float ptr [EBP + 0xfffffb10]
007ae119  d99d1cf9ffff          FSTP float ptr [EBP + 0xfffff91c]
007ae11f  d9851cf9ffff          FLD float ptr [EBP + 0xfffff91c]
007ae125  ded9                  FCOMPP
007ae127  dfe0                  FNSTSW AX
007ae129  f6c405                TEST AH,0x5
007ae12c  7a0c                  JP 0x007ae13a
007ae12e  c78518f9ffff01000000  MOV dword ptr [EBP + 0xfffff918],0x1
007ae138  eb0a                  JMP 0x007ae144
007ae13a  c78518f9ffff00000000  MOV dword ptr [EBP + 0xfffff918],0x0
007ae144  8b8d18f9ffff          MOV ECX,dword ptr [EBP + 0xfffff918]
007ae14a  898d40feffff          MOV dword ptr [EBP + 0xfffffe40],ECX
007ae150  d98520feffff          FLD float ptr [EBP + 0xfffffe20]
007ae156  d88d30feffff          FMUL float ptr [EBP + 0xfffffe30]
007ae15c  d9851cfeffff          FLD float ptr [EBP + 0xfffffe1c]
007ae162  d88d2cfeffff          FMUL float ptr [EBP + 0xfffffe2c]
007ae168  d98518feffff          FLD float ptr [EBP + 0xfffffe18]
007ae16e  d88d28feffff          FMUL float ptr [EBP + 0xfffffe28]
007ae174  dec1                  FADDP
007ae176  dec1                  FADDP
007ae178  d99d14f9ffff          FSTP float ptr [EBP + 0xfffff914]
007ae17e  d98514f9ffff          FLD float ptr [EBP + 0xfffff914]
007ae184  dc1d386b9800          FCOMP double ptr [0x00986b38]
007ae18a  dfe0                  FNSTSW AX
007ae18c  f6c401                TEST AH,0x1
007ae18f  750c                  JNZ 0x007ae19d
007ae191  c78510f9ffff01000000  MOV dword ptr [EBP + 0xfffff910],0x1
007ae19b  eb0a                  JMP 0x007ae1a7
007ae19d  c78510f9ffff00000000  MOV dword ptr [EBP + 0xfffff910],0x0
007ae1a7  8b9510f9ffff          MOV EDX,dword ptr [EBP + 0xfffff910]
007ae1ad  899524feffff          MOV dword ptr [EBP + 0xfffffe24],EDX
007ae1b3  8b8550feffff          MOV EAX,dword ptr [EBP + 0xfffffe50]
007ae1b9  8b10                  MOV EDX,dword ptr [EAX]
007ae1bb  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007ae1c1  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ae1c4  ffd0                  CALL EAX
007ae1c6  89850cf9ffff          MOV dword ptr [EBP + 0xfffff90c],EAX
007ae1cc  8b8d0cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff90c]
007ae1d2  8b11                  MOV EDX,dword ptr [ECX]
007ae1d4  8b8d0cf9ffff          MOV ECX,dword ptr [EBP + 0xfffff90c]
007ae1da  8b8244010000          MOV EAX,dword ptr [EDX + 0x144]
007ae1e0  ffd0                  CALL EAX
007ae1e2  d9854cfeffff          FLD float ptr [EBP + 0xfffffe4c]
007ae1e8  ded9                  FCOMPP
007ae1ea  dfe0                  FNSTSW AX
007ae1ec  f6c441                TEST AH,0x41
007ae1ef  7a4c                  JP 0x007ae23d
007ae1f1  d9854cfeffff          FLD float ptr [EBP + 0xfffffe4c]
007ae1f7  d9857cfeffff          FLD float ptr [EBP + 0xfffffe7c]
007ae1fd  ded9                  FCOMPP
007ae1ff  dfe0                  FNSTSW AX
007ae201  f6c441                TEST AH,0x41
007ae204  7537                  JNZ 0x007ae23d
007ae206  83bd24feffff00        CMP dword ptr [EBP + 0xfffffe24],0x0
007ae20d  742e                  JZ 0x007ae23d
007ae20f  83bd40feffff00        CMP dword ptr [EBP + 0xfffffe40],0x0
007ae216  7525                  JNZ 0x007ae23d
007ae218  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007ae21e  8b11                  MOV EDX,dword ptr [ECX]
007ae220  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
007ae226  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ae229  ffd0                  CALL EAX
007ae22b  898544feffff          MOV dword ptr [EBP + 0xfffffe44],EAX
007ae231  d9854cfeffff          FLD float ptr [EBP + 0xfffffe4c]
007ae237  d99d7cfeffff          FSTP float ptr [EBP + 0xfffffe7c]
007ae23d  e90efbffff            JMP 0x007add50
007ae242  83bd44feffff00        CMP dword ptr [EBP + 0xfffffe44],0x0
007ae249  0f8482010000          JZ 0x007ae3d1
007ae24f  8b8d44feffff          MOV ECX,dword ptr [EBP + 0xfffffe44]
007ae255  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007ae258  89950cfbffff          MOV dword ptr [EBP + 0xfffffb0c],EDX
007ae25e  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ae264  8b88b4020000          MOV ECX,dword ptr [EAX + 0x2b4]
007ae26a  898d08fbffff          MOV dword ptr [EBP + 0xfffffb08],ECX
007ae270  8b950cfbffff          MOV EDX,dword ptr [EBP + 0xfffffb0c]
007ae276  3b9508fbffff          CMP EDX,dword ptr [EBP + 0xfffffb08]
007ae27c  0f854f010000          JNZ 0x007ae3d1
007ae282  c785f0fdffff00000000  MOV dword ptr [EBP + 0xfffffdf0],0x0
007ae28c  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007ae291  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007ae294  898d04fbffff          MOV dword ptr [EBP + 0xfffffb04],ECX
007ae29a  8b8d04fbffff          MOV ECX,dword ptr [EBP + 0xfffffb04]
007ae2a0  e8ab14f9ff            CALL 0x0073f750
007ae2a5  898500fbffff          MOV dword ptr [EBP + 0xfffffb00],EAX
007ae2ab  8b9500fbffff          MOV EDX,dword ptr [EBP + 0xfffffb00]
007ae2b1  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
007ae2b7  8985fcfaffff          MOV dword ptr [EBP + 0xfffffafc],EAX
007ae2bd  33c9                  XOR ECX,ECX
007ae2bf  6bc90c                IMUL ECX,ECX,0xc
007ae2c2  8b95fcfaffff          MOV EDX,dword ptr [EBP + 0xfffffafc]
007ae2c8  8b440a7c              MOV EAX,dword ptr [EDX + ECX*0x1 + 0x7c]
007ae2cc  8985f8faffff          MOV dword ptr [EBP + 0xfffffaf8],EAX
007ae2d2  83bdf8faffff00        CMP dword ptr [EBP + 0xfffffaf8],0x0
007ae2d9  7e4c                  JLE 0x007ae327
007ae2db  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007ae2e1  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007ae2e4  8995f4faffff          MOV dword ptr [EBP + 0xfffffaf4],EDX
007ae2ea  8b8df4faffff          MOV ECX,dword ptr [EBP + 0xfffffaf4]
007ae2f0  e85b14f9ff            CALL 0x0073f750
007ae2f5  8985f0faffff          MOV dword ptr [EBP + 0xfffffaf0],EAX
007ae2fb  8b85f0faffff          MOV EAX,dword ptr [EBP + 0xfffffaf0]
007ae301  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007ae307  898decfaffff          MOV dword ptr [EBP + 0xfffffaec],ECX
007ae30d  33d2                  XOR EDX,EDX
007ae30f  6bd20c                IMUL EDX,EDX,0xc
007ae312  33c0                  XOR EAX,EAX
007ae314  6bc03c                IMUL EAX,EAX,0x3c
007ae317  8b8decfaffff          MOV ECX,dword ptr [EBP + 0xfffffaec]
007ae31d  03441178              ADD EAX,dword ptr [ECX + EDX*0x1 + 0x78]
007ae321  8985f0fdffff          MOV dword ptr [EBP + 0xfffffdf0],EAX
007ae327  83bdf0fdffff00        CMP dword ptr [EBP + 0xfffffdf0],0x0
007ae32e  7477                  JZ 0x007ae3a7
007ae330  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ae336  8b8274020000          MOV EAX,dword ptr [EDX + 0x274]
007ae33c  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007ae33f  898de8faffff          MOV dword ptr [EBP + 0xfffffae8],ECX
007ae345  83bde8faffff00        CMP dword ptr [EBP + 0xfffffae8],0x0
007ae34c  7559                  JNZ 0x007ae3a7
007ae34e  83bd44feffff00        CMP dword ptr [EBP + 0xfffffe44],0x0
007ae355  741d                  JZ 0x007ae374
007ae357  8b95e4faffff          MOV EDX,dword ptr [EBP + 0xfffffae4]
007ae35d  8995dcfaffff          MOV dword ptr [EBP + 0xfffffadc],EDX
007ae363  8b8544feffff          MOV EAX,dword ptr [EBP + 0xfffffe44]
007ae369  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007ae36c  898ddcfaffff          MOV dword ptr [EBP + 0xfffffadc],ECX
007ae372  eb0a                  JMP 0x007ae37e
007ae374  c785dcfaffff0000007f  MOV dword ptr [EBP + 0xfffffadc],0x7f000000
007ae37e  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ae384  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007ae387  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007ae38d  898de0faffff          MOV dword ptr [EBP + 0xfffffae0],ECX
007ae393  8b95e0faffff          MOV EDX,dword ptr [EBP + 0xfffffae0]
007ae399  8b85dcfaffff          MOV EAX,dword ptr [EBP + 0xfffffadc]
007ae39f  898258590000          MOV dword ptr [EDX + 0x5958],EAX
007ae3a5  eb25                  JMP 0x007ae3cc
007ae3a7  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ae3ad  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007ae3b0  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
007ae3b6  8985d8faffff          MOV dword ptr [EBP + 0xfffffad8],EAX
007ae3bc  8b8dd8faffff          MOV ECX,dword ptr [EBP + 0xfffffad8]
007ae3c2  c781585900000000007f  MOV dword ptr [ECX + 0x5958],0x7f000000
007ae3cc  e99c000000            JMP 0x007ae46d
007ae3d1  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ae3d7  8b8274020000          MOV EAX,dword ptr [EDX + 0x274]
007ae3dd  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007ae3e0  898dd4faffff          MOV dword ptr [EBP + 0xfffffad4],ECX
007ae3e6  83bdd4faffff00        CMP dword ptr [EBP + 0xfffffad4],0x0
007ae3ed  7427                  JZ 0x007ae416
007ae3ef  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ae3f5  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007ae3f8  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007ae3fe  898dd0faffff          MOV dword ptr [EBP + 0xfffffad0],ECX
007ae404  8b95d0faffff          MOV EDX,dword ptr [EBP + 0xfffffad0]
007ae40a  c782585900000000007f  MOV dword ptr [EDX + 0x5958],0x7f000000
007ae414  eb57                  JMP 0x007ae46d
007ae416  83bd44feffff00        CMP dword ptr [EBP + 0xfffffe44],0x0
007ae41d  741d                  JZ 0x007ae43c
007ae41f  8b85ccfaffff          MOV EAX,dword ptr [EBP + 0xfffffacc]
007ae425  8985c4faffff          MOV dword ptr [EBP + 0xfffffac4],EAX
007ae42b  8b8d44feffff          MOV ECX,dword ptr [EBP + 0xfffffe44]
007ae431  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007ae434  8995c4faffff          MOV dword ptr [EBP + 0xfffffac4],EDX
007ae43a  eb0a                  JMP 0x007ae446
007ae43c  c785c4faffff0000007f  MOV dword ptr [EBP + 0xfffffac4],0x7f000000
007ae446  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ae44c  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007ae44f  8b9198000000          MOV EDX,dword ptr [ECX + 0x98]
007ae455  8995c8faffff          MOV dword ptr [EBP + 0xfffffac8],EDX
007ae45b  8b85c8faffff          MOV EAX,dword ptr [EBP + 0xfffffac8]
007ae461  8b8dc4faffff          MOV ECX,dword ptr [EBP + 0xfffffac4]
007ae467  898858590000          MOV dword ptr [EAX + 0x5958],ECX
007ae46d  83bd44feffff00        CMP dword ptr [EBP + 0xfffffe44],0x0
007ae474  741d                  JZ 0x007ae493
007ae476  8b95c0faffff          MOV EDX,dword ptr [EBP + 0xfffffac0]
007ae47c  899508f9ffff          MOV dword ptr [EBP + 0xfffff908],EDX
007ae482  8b8544feffff          MOV EAX,dword ptr [EBP + 0xfffffe44]
007ae488  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007ae48b  898d08f9ffff          MOV dword ptr [EBP + 0xfffff908],ECX
007ae491  eb0a                  JMP 0x007ae49d
007ae493  c78508f9ffff0000007f  MOV dword ptr [EBP + 0xfffff908],0x7f000000
007ae49d  6a00                  PUSH 0x0
007ae49f  8d95e8feffff          LEA EDX,[EBP + 0xfffffee8]
007ae4a5  52                    PUSH EDX
007ae4a6  8b8508f9ffff          MOV EAX,dword ptr [EBP + 0xfffff908]
007ae4ac  50                    PUSH EAX
007ae4ad  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ae4b3  e848550000            CALL 0x007b3a00
007ae4b8  eb3e                  JMP 0x007ae4f8
007ae4ba  6a00                  PUSH 0x0
007ae4bc  8d8de8feffff          LEA ECX,[EBP + 0xfffffee8]
007ae4c2  51                    PUSH ECX
007ae4c3  680000007f            PUSH 0x7f000000
007ae4c8  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ae4ce  e82d550000            CALL 0x007b3a00
007ae4d3  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ae4d9  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007ae4dc  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007ae4e2  898dbcfaffff          MOV dword ptr [EBP + 0xfffffabc],ECX
007ae4e8  8b95bcfaffff          MOV EDX,dword ptr [EBP + 0xfffffabc]
007ae4ee  c782585900000000007f  MOV dword ptr [EDX + 0x5958],0x7f000000
007ae4f8  c645fc04              MOV byte ptr [EBP + -0x4],0x4
007ae4fc  8d8d2cffffff          LEA ECX,[EBP + 0xffffff2c]
007ae502  e87959ccff            CALL 0x00473e80
007ae507  c645fc02              MOV byte ptr [EBP + -0x4],0x2
007ae50b  8d8dfcfeffff          LEA ECX,[EBP + 0xfffffefc]
007ae511  e83a59ccff            CALL 0x00473e50
007ae516  e980000000            JMP 0x007ae59b
007ae51b  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ae521  83784000              CMP dword ptr [EAX + 0x40],0x0
007ae525  7474                  JZ 0x007ae59b
007ae527  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ae52d  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007ae530  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
007ae536  8985b8faffff          MOV dword ptr [EBP + 0xfffffab8],EAX
007ae53c  83bdb8faffff00        CMP dword ptr [EBP + 0xfffffab8],0x0
007ae543  7456                  JZ 0x007ae59b
007ae545  d9ee                  FLDZ
007ae547  d99d58fdffff          FSTP float ptr [EBP + 0xfffffd58]
007ae54d  d9ee                  FLDZ
007ae54f  d99d5cfdffff          FSTP float ptr [EBP + 0xfffffd5c]
007ae555  d9ee                  FLDZ
007ae557  d99d60fdffff          FSTP float ptr [EBP + 0xfffffd60]
007ae55d  6a00                  PUSH 0x0
007ae55f  8d8d58fdffff          LEA ECX,[EBP + 0xfffffd58]
007ae565  51                    PUSH ECX
007ae566  680000007f            PUSH 0x7f000000
007ae56b  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ae571  e88a540000            CALL 0x007b3a00
007ae576  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ae57c  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007ae57f  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007ae585  898db4faffff          MOV dword ptr [EBP + 0xfffffab4],ECX
007ae58b  8b95b4faffff          MOV EDX,dword ptr [EBP + 0xfffffab4]
007ae591  c782585900000000007f  MOV dword ptr [EDX + 0x5958],0x7f000000
007ae59b  c645fc00              MOV byte ptr [EBP + -0x4],0x0
007ae59f  8d4d88                LEA ECX,[EBP + -0x78]
007ae5a2  e809f8e9ff            CALL 0x0064ddb0
007ae5a7  837dd800              CMP dword ptr [EBP + -0x28],0x0
007ae5ab  0f84af000000          JZ 0x007ae660
007ae5b1  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ae5b7  8b8874020000          MOV ECX,dword ptr [EAX + 0x274]
007ae5bd  8b5108                MOV EDX,dword ptr [ECX + 0x8]
007ae5c0  8995ecfdffff          MOV dword ptr [EBP + 0xfffffdec],EDX
007ae5c6  83bdecfdffff00        CMP dword ptr [EBP + 0xfffffdec],0x0
007ae5cd  0f848d000000          JZ 0x007ae660
007ae5d3  8b8decfdffff          MOV ECX,dword ptr [EBP + 0xfffffdec]
007ae5d9  e8c2adc6ff            CALL 0x004193a0
007ae5de  8985e8fdffff          MOV dword ptr [EBP + 0xfffffde8],EAX
007ae5e4  6a00                  PUSH 0x0
007ae5e6  8b85ecfdffff          MOV EAX,dword ptr [EBP + 0xfffffdec]
007ae5ec  8b10                  MOV EDX,dword ptr [EAX]
007ae5ee  8b8decfdffff          MOV ECX,dword ptr [EBP + 0xfffffdec]
007ae5f4  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ae5f7  ffd0                  CALL EAX
007ae5f9  6a00                  PUSH 0x0
007ae5fb  8b8decfdffff          MOV ECX,dword ptr [EBP + 0xfffffdec]
007ae601  8b11                  MOV EDX,dword ptr [ECX]
007ae603  8b8decfdffff          MOV ECX,dword ptr [EBP + 0xfffffdec]
007ae609  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
007ae60c  ffd0                  CALL EAX
007ae60e  83bde8fdffff00        CMP dword ptr [EBP + 0xfffffde8],0x0
007ae615  742a                  JZ 0x007ae641
007ae617  6a00                  PUSH 0x0
007ae619  8b8de8fdffff          MOV ECX,dword ptr [EBP + 0xfffffde8]
007ae61f  8b11                  MOV EDX,dword ptr [ECX]
007ae621  8b8de8fdffff          MOV ECX,dword ptr [EBP + 0xfffffde8]
007ae627  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007ae62a  ffd0                  CALL EAX
007ae62c  6a00                  PUSH 0x0
007ae62e  8b8de8fdffff          MOV ECX,dword ptr [EBP + 0xfffffde8]
007ae634  8b11                  MOV EDX,dword ptr [ECX]
007ae636  8b8de8fdffff          MOV ECX,dword ptr [EBP + 0xfffffde8]
007ae63c  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
007ae63f  ffd0                  CALL EAX
007ae641  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ae647  8b9174020000          MOV EDX,dword ptr [ECX + 0x274]
007ae64d  8995b0faffff          MOV dword ptr [EBP + 0xfffffab0],EDX
007ae653  8b85b0faffff          MOV EAX,dword ptr [EBP + 0xfffffab0]
007ae659  c7400800000000        MOV dword ptr [EAX + 0x8],0x0
007ae660  d9ee                  FLDZ
007ae662  d95da0                FSTP float ptr [EBP + -0x60]
007ae665  d9ee                  FLDZ
007ae667  d95da4                FSTP float ptr [EBP + -0x5c]
007ae66a  d9ee                  FLDZ
007ae66c  d95da8                FSTP float ptr [EBP + -0x58]
007ae66f  8d4dc4                LEA ECX,[EBP + -0x3c]
007ae672  e8c94ef8ff            CALL 0x00733540
007ae677  c645fc05              MOV byte ptr [EBP + -0x4],0x5
007ae67b  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007ae681  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007ae684  8995acfaffff          MOV dword ptr [EBP + 0xfffffaac],EDX
007ae68a  8b8dacfaffff          MOV ECX,dword ptr [EBP + 0xfffffaac]
007ae690  e8bb0df9ff            CALL 0x0073f450
007ae695  8945ac                MOV dword ptr [EBP + -0x54],EAX
007ae698  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007ae69d  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007ae6a0  898da8faffff          MOV dword ptr [EBP + 0xfffffaa8],ECX
007ae6a6  8b8da8faffff          MOV ECX,dword ptr [EBP + 0xfffffaa8]
007ae6ac  e8ff0bf9ff            CALL 0x0073f2b0
007ae6b1  85c0                  TEST EAX,EAX
007ae6b3  0f84ca000000          JZ 0x007ae783
007ae6b9  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007ae6bf  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007ae6c2  8985a4faffff          MOV dword ptr [EBP + 0xfffffaa4],EAX
007ae6c8  8b8da4faffff          MOV ECX,dword ptr [EBP + 0xfffffaa4]
007ae6ce  e87d0df9ff            CALL 0x0073f450
007ae6d3  8985e4fdffff          MOV dword ptr [EBP + 0xfffffde4],EAX
007ae6d9  83bde4fdffff00        CMP dword ptr [EBP + 0xfffffde4],0x0
007ae6e0  0f849d000000          JZ 0x007ae783
007ae6e6  8b8de4fdffff          MOV ECX,dword ptr [EBP + 0xfffffde4]
007ae6ec  83c124                ADD ECX,0x24
007ae6ef  8b11                  MOV EDX,dword ptr [ECX]
007ae6f1  8955a0                MOV dword ptr [EBP + -0x60],EDX
007ae6f4  8b4104                MOV EAX,dword ptr [ECX + 0x4]
007ae6f7  8945a4                MOV dword ptr [EBP + -0x5c],EAX
007ae6fa  8b4908                MOV ECX,dword ptr [ECX + 0x8]
007ae6fd  894da8                MOV dword ptr [EBP + -0x58],ECX
007ae700  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007ae706  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007ae709  8985a0faffff          MOV dword ptr [EBP + 0xfffffaa0],EAX
007ae70f  d945a8                FLD float ptr [EBP + -0x58]
007ae712  83ec08                SUB ESP,0x8
007ae715  dd1c24                FSTP double ptr [ESP]
007ae718  d945a4                FLD float ptr [EBP + -0x5c]
007ae71b  83ec08                SUB ESP,0x8
007ae71e  dd1c24                FSTP double ptr [ESP]
007ae721  d945a0                FLD float ptr [EBP + -0x60]
007ae724  83ec08                SUB ESP,0x8
007ae727  dd1c24                FSTP double ptr [ESP]
007ae72a  6a58                  PUSH 0x58
007ae72c  8b8da0faffff          MOV ECX,dword ptr [EBP + 0xfffffaa0]
007ae732  e8790cf9ff            CALL 0x0073f3b0
007ae737  8bc8                  MOV ECX,EAX
007ae739  e8e2c1d6ff            CALL 0x0051a920
007ae73e  6a00                  PUSH 0x0
007ae740  68e8030000            PUSH 0x3e8
007ae745  52                    PUSH EDX
007ae746  50                    PUSH EAX
007ae747  e8d4ea1600            CALL 0x0091d220
007ae74c  50                    PUSH EAX
007ae74d  6834f79900            PUSH 0x99f734
007ae752  8d4dc4                LEA ECX,[EBP + -0x3c]
007ae755  51                    PUSH ECX
007ae756  e8155bf8ff            CALL 0x00734270
007ae75b  83c428                ADD ESP,0x28
007ae75e  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007ae764  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007ae767  89859cfaffff          MOV dword ptr [EBP + 0xfffffa9c],EAX
007ae76d  8d4dc4                LEA ECX,[EBP + -0x3c]
007ae770  51                    PUSH ECX
007ae771  8b8d9cfaffff          MOV ECX,dword ptr [EBP + 0xfffffa9c]
007ae777  e8340bf9ff            CALL 0x0073f2b0
007ae77c  8bc8                  MOV ECX,EAX
007ae77e  e81db3f8ff            CALL 0x00739aa0
007ae783  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007ae789  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007ae78c  898598faffff          MOV dword ptr [EBP + 0xfffffa98],EAX
007ae792  6a00                  PUSH 0x0
007ae794  8b8d98faffff          MOV ECX,dword ptr [EBP + 0xfffffa98]
007ae79a  e8f113f9ff            CALL 0x0073fb90
007ae79f  8bc8                  MOV ECX,EAX
007ae7a1  e8fa750300            CALL 0x007e5da0
007ae7a6  8945bc                MOV dword ptr [EBP + -0x44],EAX
007ae7a9  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ae7af  e8acd8feff            CALL 0x0079c060
007ae7b4  85c0                  TEST EAX,EAX
007ae7b6  0f850c100000          JNZ 0x007af7c8
007ae7bc  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007ae7c2  83b9a002000000        CMP dword ptr [ECX + 0x2a0],0x0
007ae7c9  0f84f90f0000          JZ 0x007af7c8
007ae7cf  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ae7d5  8b82a0020000          MOV EAX,dword ptr [EDX + 0x2a0]
007ae7db  8b480c                MOV ECX,dword ptr [EAX + 0xc]
007ae7de  898d94faffff          MOV dword ptr [EBP + 0xfffffa94],ECX
007ae7e4  83bd94faffff00        CMP dword ptr [EBP + 0xfffffa94],0x0
007ae7eb  0f84a30c0000          JZ 0x007af494
007ae7f1  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007ae7f7  83ba9c00000000        CMP dword ptr [EDX + 0x9c],0x0
007ae7fe  0f85900c0000          JNZ 0x007af494
007ae804  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007ae80a  8b8888020000          MOV ECX,dword ptr [EAX + 0x288]
007ae810  898d90faffff          MOV dword ptr [EBP + 0xfffffa90],ECX
007ae816  83bd90faffff00        CMP dword ptr [EBP + 0xfffffa90],0x0
007ae81d  0f85710c0000          JNZ 0x007af494
007ae823  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007ae829  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007ae82c  89858cfaffff          MOV dword ptr [EBP + 0xfffffa8c],EAX
007ae832  8b8d8cfaffff          MOV ECX,dword ptr [EBP + 0xfffffa8c]
007ae838  e8c312f8ff            CALL 0x0072fb00
007ae83d  898588faffff          MOV dword ptr [EBP + 0xfffffa88],EAX
007ae843  8b8d88faffff          MOV ECX,dword ptr [EBP + 0xfffffa88]
007ae849  8a5175                MOV DL,byte ptr [ECX + 0x75]
007ae84c  889587faffff          MOV byte ptr [EBP + 0xfffffa87],DL
007ae852  0fb68587faffff        MOVZX EAX,byte ptr [EBP + 0xfffffa87]
007ae859  83f807                CMP EAX,0x7
007ae85c  0f84320c0000          JZ 0x007af494
007ae862  6a00                  PUSH 0x0
007ae864  6a07                  PUSH 0x7
007ae866  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae86c  e85f11f8ff            CALL 0x0072f9d0
007ae871  d95db8                FSTP float ptr [EBP + -0x48]
007ae874  6a00                  PUSH 0x0
007ae876  6a08                  PUSH 0x8
007ae878  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae87e  e84d11f8ff            CALL 0x0072f9d0
007ae883  d95db0                FSTP float ptr [EBP + -0x50]
007ae886  6a00                  PUSH 0x0
007ae888  6a0d                  PUSH 0xd
007ae88a  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae890  e83b11f8ff            CALL 0x0072f9d0
007ae895  d99ddcfdffff          FSTP float ptr [EBP + 0xfffffddc]
007ae89b  d945b8                FLD float ptr [EBP + -0x48]
007ae89e  dc1d386b9800          FCOMP double ptr [0x00986b38]
007ae8a4  dfe0                  FNSTSW AX
007ae8a6  f6c444                TEST AH,0x44
007ae8a9  0f8a8c010000          JP 0x007aea3b
007ae8af  d945b0                FLD float ptr [EBP + -0x50]
007ae8b2  dc1d386b9800          FCOMP double ptr [0x00986b38]
007ae8b8  dfe0                  FNSTSW AX
007ae8ba  f6c444                TEST AH,0x44
007ae8bd  0f8a78010000          JP 0x007aea3b
007ae8c3  6a00                  PUSH 0x0
007ae8c5  6819010000            PUSH 0x119
007ae8ca  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae8d0  e8fb10f8ff            CALL 0x0072f9d0
007ae8d5  dc1d986d9800          FCOMP double ptr [0x00986d98]
007ae8db  dfe0                  FNSTSW AX
007ae8dd  f6c405                TEST AH,0x5
007ae8e0  7a0e                  JP 0x007ae8f0
007ae8e2  d90574749800          FLD float ptr [0x00987474]
007ae8e8  d99d04f9ffff          FSTP float ptr [EBP + 0xfffff904]
007ae8ee  eb18                  JMP 0x007ae908
007ae8f0  6a00                  PUSH 0x0
007ae8f2  6819010000            PUSH 0x119
007ae8f7  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae8fd  e8ce10f8ff            CALL 0x0072f9d0
007ae902  d99d04f9ffff          FSTP float ptr [EBP + 0xfffff904]
007ae908  d98504f9ffff          FLD float ptr [EBP + 0xfffff904]
007ae90e  dc1d506b9800          FCOMP double ptr [0x00986b50]
007ae914  dfe0                  FNSTSW AX
007ae916  f6c441                TEST AH,0x41
007ae919  750a                  JNZ 0x007ae925
007ae91b  d9e8                  FLD1
007ae91d  d99d00f9ffff          FSTP float ptr [EBP + 0xfffff900]
007ae923  eb51                  JMP 0x007ae976
007ae925  6a00                  PUSH 0x0
007ae927  6819010000            PUSH 0x119
007ae92c  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae932  e89910f8ff            CALL 0x0072f9d0
007ae937  dc1d986d9800          FCOMP double ptr [0x00986d98]
007ae93d  dfe0                  FNSTSW AX
007ae93f  f6c405                TEST AH,0x5
007ae942  7a0e                  JP 0x007ae952
007ae944  d90574749800          FLD float ptr [0x00987474]
007ae94a  d99dfcf8ffff          FSTP float ptr [EBP + 0xfffff8fc]
007ae950  eb18                  JMP 0x007ae96a
007ae952  6a00                  PUSH 0x0
007ae954  6819010000            PUSH 0x119
007ae959  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae95f  e86c10f8ff            CALL 0x0072f9d0
007ae964  d99dfcf8ffff          FSTP float ptr [EBP + 0xfffff8fc]
007ae96a  d985fcf8ffff          FLD float ptr [EBP + 0xfffff8fc]
007ae970  d99d00f9ffff          FSTP float ptr [EBP + 0xfffff900]
007ae976  d98500f9ffff          FLD float ptr [EBP + 0xfffff900]
007ae97c  d95db0                FSTP float ptr [EBP + -0x50]
007ae97f  6a00                  PUSH 0x0
007ae981  6818010000            PUSH 0x118
007ae986  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae98c  e83f10f8ff            CALL 0x0072f9d0
007ae991  dc1d986d9800          FCOMP double ptr [0x00986d98]
007ae997  dfe0                  FNSTSW AX
007ae999  f6c405                TEST AH,0x5
007ae99c  7a0e                  JP 0x007ae9ac
007ae99e  d90574749800          FLD float ptr [0x00987474]
007ae9a4  d99df8f8ffff          FSTP float ptr [EBP + 0xfffff8f8]
007ae9aa  eb18                  JMP 0x007ae9c4
007ae9ac  6a00                  PUSH 0x0
007ae9ae  6818010000            PUSH 0x118
007ae9b3  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae9b9  e81210f8ff            CALL 0x0072f9d0
007ae9be  d99df8f8ffff          FSTP float ptr [EBP + 0xfffff8f8]
007ae9c4  d985f8f8ffff          FLD float ptr [EBP + 0xfffff8f8]
007ae9ca  dc1d506b9800          FCOMP double ptr [0x00986b50]
007ae9d0  dfe0                  FNSTSW AX
007ae9d2  f6c441                TEST AH,0x41
007ae9d5  750a                  JNZ 0x007ae9e1
007ae9d7  d9e8                  FLD1
007ae9d9  d99df4f8ffff          FSTP float ptr [EBP + 0xfffff8f4]
007ae9df  eb51                  JMP 0x007aea32
007ae9e1  6a00                  PUSH 0x0
007ae9e3  6818010000            PUSH 0x118
007ae9e8  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007ae9ee  e8dd0ff8ff            CALL 0x0072f9d0
007ae9f3  dc1d986d9800          FCOMP double ptr [0x00986d98]
007ae9f9  dfe0                  FNSTSW AX
007ae9fb  f6c405                TEST AH,0x5
007ae9fe  7a0e                  JP 0x007aea0e
007aea00  d90574749800          FLD float ptr [0x00987474]
007aea06  d99df0f8ffff          FSTP float ptr [EBP + 0xfffff8f0]
007aea0c  eb18                  JMP 0x007aea26
007aea0e  6a00                  PUSH 0x0
007aea10  6818010000            PUSH 0x118
007aea15  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007aea1b  e8b00ff8ff            CALL 0x0072f9d0
007aea20  d99df0f8ffff          FSTP float ptr [EBP + 0xfffff8f0]
007aea26  d985f0f8ffff          FLD float ptr [EBP + 0xfffff8f0]
007aea2c  d99df4f8ffff          FSTP float ptr [EBP + 0xfffff8f4]
007aea32  d985f4f8ffff          FLD float ptr [EBP + 0xfffff8f4]
007aea38  d95db8                FSTP float ptr [EBP + -0x48]
007aea3b  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007aea41  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007aea44  899580faffff          MOV dword ptr [EBP + 0xfffffa80],EDX
007aea4a  8b8d80faffff          MOV ECX,dword ptr [EBP + 0xfffffa80]
007aea50  e8fb0cf9ff            CALL 0x0073f750
007aea55  8bc8                  MOV ECX,EAX
007aea57  e824380200            CALL 0x007d2280
007aea5c  8985e0fdffff          MOV dword ptr [EBP + 0xfffffde0],EAX
007aea62  83bde0fdffff00        CMP dword ptr [EBP + 0xfffffde0],0x0
007aea69  740a                  JZ 0x007aea75
007aea6b  d9ee                  FLDZ
007aea6d  d95db8                FSTP float ptr [EBP + -0x48]
007aea70  d9ee                  FLDZ
007aea72  d95db0                FSTP float ptr [EBP + -0x50]
007aea75  a1bcfca700            MOV EAX,[0x00a7fcbc]
007aea7a  8985d4fdffff          MOV dword ptr [EBP + 0xfffffdd4],EAX
007aea80  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007aea86  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007aea89  89957cfaffff          MOV dword ptr [EBP + 0xfffffa7c],EDX
007aea8f  8b8d7cfaffff          MOV ECX,dword ptr [EBP + 0xfffffa7c]
007aea95  e86610f8ff            CALL 0x0072fb00
007aea9a  898578faffff          MOV dword ptr [EBP + 0xfffffa78],EAX
007aeaa0  8b8578faffff          MOV EAX,dword ptr [EBP + 0xfffffa78]
007aeaa6  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007aeaa9  d1e9                  SHR ECX,0x1
007aeaab  83e101                AND ECX,0x1
007aeaae  7412                  JZ 0x007aeac2
007aeab0  33d2                  XOR EDX,EDX
007aeab2  83bdd4fdffff00        CMP dword ptr [EBP + 0xfffffdd4],0x0
007aeab9  0f94c2                SETZ DL
007aeabc  8995d4fdffff          MOV dword ptr [EBP + 0xfffffdd4],EDX
007aeac2  8d85d0fdffff          LEA EAX,[EBP + 0xfffffdd0]
007aeac8  50                    PUSH EAX
007aeac9  8d8dd8fdffff          LEA ECX,[EBP + 0xfffffdd8]
007aeacf  51                    PUSH ECX
007aead0  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007aead6  e89512f8ff            CALL 0x0072fd70
007aeadb  e80004f9ff            CALL 0x0073eee0
007aeae0  0fb6d0                MOVZX EDX,AL
007aeae3  85d2                  TEST EDX,EDX
007aeae5  0f8581000000          JNZ 0x007aeb6c
007aeaeb  83bdd4fdffff00        CMP dword ptr [EBP + 0xfffffdd4],0x0
007aeaf2  0f84a0010000          JZ 0x007aec98
007aeaf8  833dc0fca70000        CMP dword ptr [0x00a7fcc0],0x0
007aeaff  0f8593010000          JNZ 0x007aec98
007aeb05  d985d0fdffff          FLD float ptr [EBP + 0xfffffdd0]
007aeb0b  dc1d30bd9800          FCOMP double ptr [0x0098bd30]
007aeb11  dfe0                  FNSTSW AX
007aeb13  f6c441                TEST AH,0x41
007aeb16  742a                  JZ 0x007aeb42
007aeb18  d985d0fdffff          FLD float ptr [EBP + 0xfffffdd0]
007aeb1e  dc1d10fd9900          FCOMP double ptr [0x0099fd10]
007aeb24  dfe0                  FNSTSW AX
007aeb26  f6c405                TEST AH,0x5
007aeb29  7b17                  JNP 0x007aeb42
007aeb2b  d905ec08a100          FLD float ptr [0x00a108ec]
007aeb31  dc1d386b9800          FCOMP double ptr [0x00986b38]
007aeb37  dfe0                  FNSTSW AX
007aeb39  f6c441                TEST AH,0x41
007aeb3c  0f8556010000          JNZ 0x007aec98
007aeb42  d985d8fdffff          FLD float ptr [EBP + 0xfffffdd8]
007aeb48  dc1d30bd9800          FCOMP double ptr [0x0098bd30]
007aeb4e  dfe0                  FNSTSW AX
007aeb50  f6c405                TEST AH,0x5
007aeb53  7b17                  JNP 0x007aeb6c
007aeb55  d985d8fdffff          FLD float ptr [EBP + 0xfffffdd8]
007aeb5b  dc1d10fd9900          FCOMP double ptr [0x0099fd10]
007aeb61  dfe0                  FNSTSW AX
007aeb63  f6c441                TEST AH,0x41
007aeb66  0f852c010000          JNZ 0x007aec98
007aeb6c  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007aeb71  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007aeb74  898d74faffff          MOV dword ptr [EBP + 0xfffffa74],ECX
007aeb7a  8b8d74faffff          MOV ECX,dword ptr [EBP + 0xfffffa74]
007aeb80  e8ab08f9ff            CALL 0x0073f430
007aeb85  898570faffff          MOV dword ptr [EBP + 0xfffffa70],EAX
007aeb8b  8b956cfaffff          MOV EDX,dword ptr [EBP + 0xfffffa6c]
007aeb91  8995ecf8ffff          MOV dword ptr [EBP + 0xfffff8ec],EDX
007aeb97  8b8570faffff          MOV EAX,dword ptr [EBP + 0xfffffa70]
007aeb9d  8b4840                MOV ECX,dword ptr [EAX + 0x40]
007aeba0  898decf8ffff          MOV dword ptr [EBP + 0xfffff8ec],ECX
007aeba6  686a100000            PUSH 0x106a
007aebab  8b95ecf8ffff          MOV EDX,dword ptr [EBP + 0xfffff8ec]
007aebb1  8b02                  MOV EAX,dword ptr [EDX]
007aebb3  8b8decf8ffff          MOV ECX,dword ptr [EBP + 0xfffff8ec]
007aebb9  8b9080000000          MOV EDX,dword ptr [EAX + 0x80]
007aebbf  ffd2                  CALL EDX
007aebc1  8945e4                MOV dword ptr [EBP + -0x1c],EAX
007aebc4  837de400              CMP dword ptr [EBP + -0x1c],0x0
007aebc8  0f84c5000000          JZ 0x007aec93
007aebce  a1d0b4a100            MOV EAX,[0x00a1b4d0]
007aebd3  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
007aebd9  898d68faffff          MOV dword ptr [EBP + 0xfffffa68],ECX
007aebdf  8b9568faffff          MOV EDX,dword ptr [EBP + 0xfffffa68]
007aebe5  8b4278                MOV EAX,dword ptr [EDX + 0x78]
007aebe8  8985ccfdffff          MOV dword ptr [EBP + 0xfffffdcc],EAX
007aebee  6808009a00            PUSH 0x9a0008
007aebf3  8d8d50fdffff          LEA ECX,[EBP + 0xfffffd50]
007aebf9  e87249f8ff            CALL 0x00733570
007aebfe  c645fc06              MOV byte ptr [EBP + -0x4],0x6
007aec02  68ec08a100            PUSH 0xa108ec
007aec07  8d8d50fdffff          LEA ECX,[EBP + 0xfffffd50]
007aec0d  51                    PUSH ECX
007aec0e  6a00                  PUSH 0x0
007aec10  8b8dccfdffff          MOV ECX,dword ptr [EBP + 0xfffffdcc]
007aec16  e855caf6ff            CALL 0x0071b670
007aec1b  c645fc05              MOV byte ptr [EBP + -0x4],0x5
007aec1f  8d8d50fdffff          LEA ECX,[EBP + 0xfffffd50]
007aec25  e8564bf8ff            CALL 0x00733780
007aec2a  68fcff9900            PUSH 0x99fffc
007aec2f  8d8d48fdffff          LEA ECX,[EBP + 0xfffffd48]
007aec35  e83649f8ff            CALL 0x00733570
007aec3a  c645fc07              MOV byte ptr [EBP + -0x4],0x7
007aec3e  68f008a100            PUSH 0xa108f0
007aec43  8d9548fdffff          LEA EDX,[EBP + 0xfffffd48]
007aec49  52                    PUSH EDX
007aec4a  6a00                  PUSH 0x0
007aec4c  8b8dccfdffff          MOV ECX,dword ptr [EBP + 0xfffffdcc]
007aec52  e819caf6ff            CALL 0x0071b670
007aec57  c645fc05              MOV byte ptr [EBP + -0x4],0x5
007aec5b  8d8d48fdffff          LEA ECX,[EBP + 0xfffffd48]
007aec61  e81a4bf8ff            CALL 0x00733780
007aec66  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
007aec69  d98090010000          FLD float ptr [EAX + 0x190]
007aec6f  d99d64faffff          FSTP float ptr [EBP + 0xfffffa64]
007aec75  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
007aec78  d98564faffff          FLD float ptr [EBP + 0xfffffa64]
007aec7e  d99994010000          FSTP float ptr [ECX + 0x194]
007aec84  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
007aec87  d905f8ff9900          FLD float ptr [0x0099fff8]
007aec8d  d99a90010000          FSTP float ptr [EDX + 0x190]
007aec93  e90c020000            JMP 0x007aeea4
007aec98  a1d0b4a100            MOV EAX,[0x00a1b4d0]
007aec9d  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
007aeca3  898d60faffff          MOV dword ptr [EBP + 0xfffffa60],ECX
007aeca9  8b9560faffff          MOV EDX,dword ptr [EBP + 0xfffffa60]
007aecaf  8b4278                MOV EAX,dword ptr [EDX + 0x78]
007aecb2  8985bcfdffff          MOV dword ptr [EBP + 0xfffffdbc],EAX
007aecb8  68f0ff9900            PUSH 0x99fff0
007aecbd  8d8d40fdffff          LEA ECX,[EBP + 0xfffffd40]
007aecc3  e8a848f8ff            CALL 0x00733570
007aecc8  c645fc08              MOV byte ptr [EBP + -0x4],0x8
007aeccc  8d8dc0fdffff          LEA ECX,[EBP + 0xfffffdc0]
007aecd2  51                    PUSH ECX
007aecd3  8d9540fdffff          LEA EDX,[EBP + 0xfffffd40]
007aecd9  52                    PUSH EDX
007aecda  6a00                  PUSH 0x0
007aecdc  8b8dbcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdbc]
007aece2  e889c9f6ff            CALL 0x0071b670
007aece7  c645fc05              MOV byte ptr [EBP + -0x4],0x5
007aeceb  8d8d40fdffff          LEA ECX,[EBP + 0xfffffd40]
007aecf1  e88a4af8ff            CALL 0x00733780
007aecf6  68e4ff9900            PUSH 0x99ffe4
007aecfb  8d8d38fdffff          LEA ECX,[EBP + 0xfffffd38]
007aed01  e86a48f8ff            CALL 0x00733570
007aed06  c645fc09              MOV byte ptr [EBP + -0x4],0x9
007aed0a  8d85c4fdffff          LEA EAX,[EBP + 0xfffffdc4]
007aed10  50                    PUSH EAX
007aed11  8d8d38fdffff          LEA ECX,[EBP + 0xfffffd38]
007aed17  51                    PUSH ECX
007aed18  6a00                  PUSH 0x0
007aed1a  8b8dbcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdbc]
007aed20  e84bc9f6ff            CALL 0x0071b670
007aed25  c645fc05              MOV byte ptr [EBP + -0x4],0x5
007aed29  8d8d38fdffff          LEA ECX,[EBP + 0xfffffd38]
007aed2f  e84c4af8ff            CALL 0x00733780
007aed34  68d8ff9900            PUSH 0x99ffd8
007aed39  8d8d30fdffff          LEA ECX,[EBP + 0xfffffd30]
007aed3f  e82c48f8ff            CALL 0x00733570
007aed44  c645fc0a              MOV byte ptr [EBP + -0x4],0xa
007aed48  8d95c8fdffff          LEA EDX,[EBP + 0xfffffdc8]
007aed4e  52                    PUSH EDX
007aed4f  8d8530fdffff          LEA EAX,[EBP + 0xfffffd30]
007aed55  50                    PUSH EAX
007aed56  6a00                  PUSH 0x0
007aed58  8b8dbcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdbc]
007aed5e  e80dc9f6ff            CALL 0x0071b670
007aed63  c645fc05              MOV byte ptr [EBP + -0x4],0x5
007aed67  8d8d30fdffff          LEA ECX,[EBP + 0xfffffd30]
007aed6d  e80e4af8ff            CALL 0x00733780
007aed72  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007aed78  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007aed7b  89955cfaffff          MOV dword ptr [EBP + 0xfffffa5c],EDX
007aed81  8b8d5cfaffff          MOV ECX,dword ptr [EBP + 0xfffffa5c]
007aed87  e8a406f9ff            CALL 0x0073f430
007aed8c  898558faffff          MOV dword ptr [EBP + 0xfffffa58],EAX
007aed92  8b8554faffff          MOV EAX,dword ptr [EBP + 0xfffffa54]
007aed98  8985e8f8ffff          MOV dword ptr [EBP + 0xfffff8e8],EAX
007aed9e  8b8d58faffff          MOV ECX,dword ptr [EBP + 0xfffffa58]
007aeda4  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007aeda7  8995e8f8ffff          MOV dword ptr [EBP + 0xfffff8e8],EDX
007aedad  686a100000            PUSH 0x106a
007aedb2  8b85e8f8ffff          MOV EAX,dword ptr [EBP + 0xfffff8e8]
007aedb8  8b10                  MOV EDX,dword ptr [EAX]
007aedba  8b8de8f8ffff          MOV ECX,dword ptr [EBP + 0xfffff8e8]
007aedc0  8b8280000000          MOV EAX,dword ptr [EDX + 0x80]
007aedc6  ffd0                  CALL EAX
007aedc8  8945e4                MOV dword ptr [EBP + -0x1c],EAX
007aedcb  d905ec08a100          FLD float ptr [0x00a108ec]
007aedd1  d985c0fdffff          FLD float ptr [EBP + 0xfffffdc0]
007aedd7  ded9                  FCOMPP
007aedd9  dfe0                  FNSTSW AX
007aeddb  f6c405                TEST AH,0x5
007aedde  7a35                  JP 0x007aee15
007aede0  d905ec08a100          FLD float ptr [0x00a108ec]
007aede6  dc2568b99800          FSUB double ptr [0x0098b968]
007aedec  d91dec08a100          FSTP float ptr [0x00a108ec]
007aedf2  d905ec08a100          FLD float ptr [0x00a108ec]
007aedf8  d985c0fdffff          FLD float ptr [EBP + 0xfffffdc0]
007aedfe  ded9                  FCOMPP
007aee00  dfe0                  FNSTSW AX
007aee02  f6c441                TEST AH,0x41
007aee05  750c                  JNZ 0x007aee13
007aee07  d985c0fdffff          FLD float ptr [EBP + 0xfffffdc0]
007aee0d  d91dec08a100          FSTP float ptr [0x00a108ec]
007aee13  eb24                  JMP 0x007aee39
007aee15  837de400              CMP dword ptr [EBP + -0x1c],0x0
007aee19  741e                  JZ 0x007aee39
007aee1b  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
007aee1e  d98194010000          FLD float ptr [ECX + 0x194]
007aee24  d99d50faffff          FSTP float ptr [EBP + 0xfffffa50]
007aee2a  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
007aee2d  d98550faffff          FLD float ptr [EBP + 0xfffffa50]
007aee33  d99a90010000          FSTP float ptr [EDX + 0x190]
007aee39  d905f008a100          FLD float ptr [0x00a108f0]
007aee3f  d985c4fdffff          FLD float ptr [EBP + 0xfffffdc4]
007aee45  ded9                  FCOMPP
007aee47  dfe0                  FNSTSW AX
007aee49  f6c405                TEST AH,0x5
007aee4c  7a35                  JP 0x007aee83
007aee4e  d905f008a100          FLD float ptr [0x00a108f0]
007aee54  dc2568b99800          FSUB double ptr [0x0098b968]
007aee5a  d91df008a100          FSTP float ptr [0x00a108f0]
007aee60  d905f008a100          FLD float ptr [0x00a108f0]
007aee66  d985c4fdffff          FLD float ptr [EBP + 0xfffffdc4]
007aee6c  ded9                  FCOMPP
007aee6e  dfe0                  FNSTSW AX
007aee70  f6c441                TEST AH,0x41
007aee73  750c                  JNZ 0x007aee81
007aee75  d985c4fdffff          FLD float ptr [EBP + 0xfffffdc4]
007aee7b  d91df008a100          FSTP float ptr [0x00a108f0]
007aee81  eb21                  JMP 0x007aeea4
007aee83  837de400              CMP dword ptr [EBP + -0x1c],0x0
007aee87  741b                  JZ 0x007aeea4
007aee89  d985c8fdffff          FLD float ptr [EBP + 0xfffffdc8]
007aee8f  d99d4cfaffff          FSTP float ptr [EBP + 0xfffffa4c]
007aee95  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
007aee98  d9854cfaffff          FLD float ptr [EBP + 0xfffffa4c]
007aee9e  d99890010000          FSTP float ptr [EAX + 0x190]
007aeea4  83bdd4fdffff00        CMP dword ptr [EBP + 0xfffffdd4],0x0
007aeeab  0f844f010000          JZ 0x007af000
007aeeb1  833dc0fca70000        CMP dword ptr [0x00a7fcc0],0x0
007aeeb8  0f8442010000          JZ 0x007af000
007aeebe  d945b8                FLD float ptr [EBP + -0x48]
007aeec1  dc1d386b9800          FCOMP double ptr [0x00986b38]
007aeec7  dfe0                  FNSTSW AX
007aeec9  f6c441                TEST AH,0x41
007aeecc  0f8a2e010000          JP 0x007af000
007aeed2  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007aeed8  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007aeedb  899548faffff          MOV dword ptr [EBP + 0xfffffa48],EDX
007aeee1  8b8d48faffff          MOV ECX,dword ptr [EBP + 0xfffffa48]
007aeee7  e84405f9ff            CALL 0x0073f430
007aeeec  898544faffff          MOV dword ptr [EBP + 0xfffffa44],EAX
007aeef2  8b8540faffff          MOV EAX,dword ptr [EBP + 0xfffffa40]
007aeef8  8985e4f8ffff          MOV dword ptr [EBP + 0xfffff8e4],EAX
007aeefe  8b8d44faffff          MOV ECX,dword ptr [EBP + 0xfffffa44]
007aef04  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007aef07  8995e4f8ffff          MOV dword ptr [EBP + 0xfffff8e4],EDX
007aef0d  686a100000            PUSH 0x106a
007aef12  8b85e4f8ffff          MOV EAX,dword ptr [EBP + 0xfffff8e4]
007aef18  8b10                  MOV EDX,dword ptr [EAX]
007aef1a  8b8de4f8ffff          MOV ECX,dword ptr [EBP + 0xfffff8e4]
007aef20  8b8280000000          MOV EAX,dword ptr [EDX + 0x80]
007aef26  ffd0                  CALL EAX
007aef28  8945e4                MOV dword ptr [EBP + -0x1c],EAX
007aef2b  d905ec08a100          FLD float ptr [0x00a108ec]
007aef31  dc1d386b9800          FCOMP double ptr [0x00986b38]
007aef37  dfe0                  FNSTSW AX
007aef39  f6c441                TEST AH,0x41
007aef3c  752f                  JNZ 0x007aef6d
007aef3e  d905ec08a100          FLD float ptr [0x00a108ec]
007aef44  dc2568b99800          FSUB double ptr [0x0098b968]
007aef4a  d91dec08a100          FSTP float ptr [0x00a108ec]
007aef50  d905ec08a100          FLD float ptr [0x00a108ec]
007aef56  dc1d386b9800          FCOMP double ptr [0x00986b38]
007aef5c  dfe0                  FNSTSW AX
007aef5e  f6c405                TEST AH,0x5
007aef61  7a08                  JP 0x007aef6b
007aef63  d9ee                  FLDZ
007aef65  d91dec08a100          FSTP float ptr [0x00a108ec]
007aef6b  eb24                  JMP 0x007aef91
007aef6d  837de400              CMP dword ptr [EBP + -0x1c],0x0
007aef71  741e                  JZ 0x007aef91
007aef73  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
007aef76  d98194010000          FLD float ptr [ECX + 0x194]
007aef7c  d99d3cfaffff          FSTP float ptr [EBP + 0xfffffa3c]
007aef82  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
007aef85  d9853cfaffff          FLD float ptr [EBP + 0xfffffa3c]
007aef8b  d99a90010000          FSTP float ptr [EDX + 0x190]
007aef91  d905f008a100          FLD float ptr [0x00a108f0]
007aef97  dc1d386b9800          FCOMP double ptr [0x00986b38]
007aef9d  dfe0                  FNSTSW AX
007aef9f  f6c441                TEST AH,0x41
007aefa2  752f                  JNZ 0x007aefd3
007aefa4  d905f008a100          FLD float ptr [0x00a108f0]
007aefaa  dc2568b99800          FSUB double ptr [0x0098b968]
007aefb0  d91df008a100          FSTP float ptr [0x00a108f0]
007aefb6  d905f008a100          FLD float ptr [0x00a108f0]
007aefbc  dc1d386b9800          FCOMP double ptr [0x00986b38]
007aefc2  dfe0                  FNSTSW AX
007aefc4  f6c405                TEST AH,0x5
007aefc7  7a08                  JP 0x007aefd1
007aefc9  d9ee                  FLDZ
007aefcb  d91df008a100          FSTP float ptr [0x00a108f0]
007aefd1  eb24                  JMP 0x007aeff7
007aefd3  837de400              CMP dword ptr [EBP + -0x1c],0x0
007aefd7  741e                  JZ 0x007aeff7
007aefd9  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
007aefdc  d98094010000          FLD float ptr [EAX + 0x194]
007aefe2  d99d38faffff          FSTP float ptr [EBP + 0xfffffa38]
007aefe8  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
007aefeb  d98538faffff          FLD float ptr [EBP + 0xfffffa38]
007aeff1  d99990010000          FSTP float ptr [ECX + 0x190]
007aeff7  d90574749800          FLD float ptr [0x00987474]
007aeffd  d95db8                FSTP float ptr [EBP + -0x48]
007af000  6a00                  PUSH 0x0
007af002  680c010000            PUSH 0x10c
007af007  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af00d  e8be09f8ff            CALL 0x0072f9d0
007af012  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af018  dfe0                  FNSTSW AX
007af01a  f6c444                TEST AH,0x44
007af01d  7b23                  JNP 0x007af042
007af01f  6a01                  PUSH 0x1
007af021  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af027  8b82a0020000          MOV EAX,dword ptr [EDX + 0x2a0]
007af02d  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af033  8b89a0020000          MOV ECX,dword ptr [ECX + 0x2a0]
007af039  8b10                  MOV EDX,dword ptr [EAX]
007af03b  8b4210                MOV EAX,dword ptr [EDX + 0x10]
007af03e  ffd0                  CALL EAX
007af040  eb21                  JMP 0x007af063
007af042  6a00                  PUSH 0x0
007af044  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af04a  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
007af050  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af056  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af05c  8b12                  MOV EDX,dword ptr [EDX]
007af05e  8b4210                MOV EAX,dword ptr [EDX + 0x10]
007af061  ffd0                  CALL EAX
007af063  837dbc00              CMP dword ptr [EBP + -0x44],0x0
007af067  0f8419020000          JZ 0x007af286
007af06d  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af070  e8cb7dfcff            CALL 0x00776e40
007af075  83f801                CMP EAX,0x1
007af078  0f8508020000          JNZ 0x007af286
007af07e  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af081  8b9158040000          MOV EDX,dword ptr [ECX + 0x458]
007af087  83e201                AND EDX,0x1
007af08a  742a                  JZ 0x007af0b6
007af08c  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af092  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007af095  898d34faffff          MOV dword ptr [EBP + 0xfffffa34],ECX
007af09b  8b9534faffff          MOV EDX,dword ptr [EBP + 0xfffffa34]
007af0a1  8b4208                MOV EAX,dword ptr [EDX + 0x8]
007af0a4  c1e805                SHR EAX,0x5
007af0a7  83e001                AND EAX,0x1
007af0aa  750a                  JNZ 0x007af0b6
007af0ac  d9ee                  FLDZ
007af0ae  d95db8                FSTP float ptr [EBP + -0x48]
007af0b1  d9ee                  FLDZ
007af0b3  d95db0                FSTP float ptr [EBP + -0x50]
007af0b6  d945b8                FLD float ptr [EBP + -0x48]
007af0b9  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af0bf  dfe0                  FNSTSW AX
007af0c1  f6c444                TEST AH,0x44
007af0c4  7a10                  JP 0x007af0d6
007af0c6  d945b0                FLD float ptr [EBP + -0x50]
007af0c9  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af0cf  dfe0                  FNSTSW AX
007af0d1  f6c444                TEST AH,0x44
007af0d4  7b2d                  JNP 0x007af103
007af0d6  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af0dc  d981a4020000          FLD float ptr [ECX + 0x2a4]
007af0e2  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af0e8  dfe0                  FNSTSW AX
007af0ea  f6c441                TEST AH,0x41
007af0ed  7a12                  JP 0x007af101
007af0ef  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af0f5  d905606b9800          FLD float ptr [0x00986b60]
007af0fb  d99aa4020000          FSTP float ptr [EDX + 0x2a4]
007af101  eb12                  JMP 0x007af115
007af103  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af109  d90574749800          FLD float ptr [0x00987474]
007af10f  d998a4020000          FSTP float ptr [EAX + 0x2a4]
007af115  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af11b  d981a4020000          FLD float ptr [ECX + 0x2a4]
007af121  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af127  dfe0                  FNSTSW AX
007af129  f6c441                TEST AH,0x41
007af12c  0f8554010000          JNZ 0x007af286
007af132  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af138  d982a4020000          FLD float ptr [EDX + 0x2a4]
007af13e  d86508                FSUB float ptr [EBP + 0x8]
007af141  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af147  d998a4020000          FSTP float ptr [EAX + 0x2a4]
007af14d  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af150  e8abe6fcff            CALL 0x0077d800
007af155  8945ec                MOV dword ptr [EBP + -0x14],EAX
007af158  c745e801000000        MOV dword ptr [EBP + -0x18],0x1
007af15f  837dec00              CMP dword ptr [EBP + -0x14],0x0
007af163  740c                  JZ 0x007af171
007af165  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
007af168  8b91ec000000          MOV EDX,dword ptr [ECX + 0xec]
007af16e  8955e8                MOV dword ptr [EBP + -0x18],EDX
007af171  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af177  d980a4020000          FLD float ptr [EAX + 0x2a4]
007af17d  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af183  dfe0                  FNSTSW AX
007af185  f6c441                TEST AH,0x41
007af188  0f8aee000000          JP 0x007af27c
007af18e  837de800              CMP dword ptr [EBP + -0x18],0x0
007af192  0f84e2000000          JZ 0x007af27a
007af198  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af19e  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
007af1a4  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af1aa  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af1b0  8b12                  MOV EDX,dword ptr [EDX]
007af1b2  8b421c                MOV EAX,dword ptr [EDX + 0x1c]
007af1b5  ffd0                  CALL EAX
007af1b7  dc1dd8c19800          FCOMP double ptr [0x0098c1d8]
007af1bd  dfe0                  FNSTSW AX
007af1bf  f6c441                TEST AH,0x41
007af1c2  0f85a0000000          JNZ 0x007af268
007af1c8  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af1cb  e8f0320400            CALL 0x007f24c0
007af1d0  6a00                  PUSH 0x0
007af1d2  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af1d5  e826110400            CALL 0x007f0300
007af1da  6a00                  PUSH 0x0
007af1dc  680000007f            PUSH 0x7f000000
007af1e1  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af1e4  e8270afdff            CALL 0x0077fc10
007af1e9  6a00                  PUSH 0x0
007af1eb  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af1ee  e8cd7cfcff            CALL 0x00776ec0
007af1f3  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007af1f9  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007af1fc  899530faffff          MOV dword ptr [EBP + 0xfffffa30],EDX
007af202  8b8d30faffff          MOV ECX,dword ptr [EBP + 0xfffffa30]
007af208  e82302f9ff            CALL 0x0073f430
007af20d  89852cfaffff          MOV dword ptr [EBP + 0xfffffa2c],EAX
007af213  8b8528faffff          MOV EAX,dword ptr [EBP + 0xfffffa28]
007af219  8985e0f8ffff          MOV dword ptr [EBP + 0xfffff8e0],EAX
007af21f  8b8d2cfaffff          MOV ECX,dword ptr [EBP + 0xfffffa2c]
007af225  8b5140                MOV EDX,dword ptr [ECX + 0x40]
007af228  8995e0f8ffff          MOV dword ptr [EBP + 0xfffff8e0],EDX
007af22e  686a100000            PUSH 0x106a
007af233  8b85e0f8ffff          MOV EAX,dword ptr [EBP + 0xfffff8e0]
007af239  8b10                  MOV EDX,dword ptr [EAX]
007af23b  8b8de0f8ffff          MOV ECX,dword ptr [EBP + 0xfffff8e0]
007af241  8b8280000000          MOV EAX,dword ptr [EDX + 0x80]
007af247  ffd0                  CALL EAX
007af249  8945e4                MOV dword ptr [EBP + -0x1c],EAX
007af24c  837de400              CMP dword ptr [EBP + -0x1c],0x0
007af250  7408                  JZ 0x007af25a
007af252  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
007af255  e8d6230300            CALL 0x007e1630
007af25a  6a00                  PUSH 0x0
007af25c  6a00                  PUSH 0x0
007af25e  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af261  e88addfbff            CALL 0x0076cff0
007af266  eb12                  JMP 0x007af27a
007af268  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af26e  d905fc769800          FLD float ptr [0x009876fc]
007af274  d999a4020000          FSTP float ptr [ECX + 0x2a4]
007af27a  eb0a                  JMP 0x007af286
007af27c  d9ee                  FLDZ
007af27e  d95db8                FSTP float ptr [EBP + -0x48]
007af281  d9ee                  FLDZ
007af283  d95db0                FSTP float ptr [EBP + -0x50]
007af286  837dbc00              CMP dword ptr [EBP + -0x44],0x0
007af28a  0f84de000000          JZ 0x007af36e
007af290  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
007af293  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007af296  898524faffff          MOV dword ptr [EBP + 0xfffffa24],EAX
007af29c  8b8d24faffff          MOV ECX,dword ptr [EBP + 0xfffffa24]
007af2a2  51                    PUSH ECX
007af2a3  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af2a9  8b8a70020000          MOV ECX,dword ptr [EDX + 0x270]
007af2af  e82c790300            CALL 0x007e6be0
007af2b4  8985b8fdffff          MOV dword ptr [EBP + 0xfffffdb8],EAX
007af2ba  83bdb8fdffff00        CMP dword ptr [EBP + 0xfffffdb8],0x0
007af2c1  0f8ca7000000          JL 0x007af36e
007af2c7  8b85b8fdffff          MOV EAX,dword ptr [EBP + 0xfffffdb8]
007af2cd  69c088000000          IMUL EAX,EAX,0x88
007af2d3  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af2d9  8b9170020000          MOV EDX,dword ptr [ECX + 0x270]
007af2df  8d440224              LEA EAX,[EDX + EAX*0x1 + 0x24]
007af2e3  8985b4fdffff          MOV dword ptr [EBP + 0xfffffdb4],EAX
007af2e9  83bdb4fdffff00        CMP dword ptr [EBP + 0xfffffdb4],0x0
007af2f0  747c                  JZ 0x007af36e
007af2f2  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
007af2f5  8b9158040000          MOV EDX,dword ptr [ECX + 0x458]
007af2fb  83e201                AND EDX,0x1
007af2fe  746e                  JZ 0x007af36e
007af300  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af306  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af30c  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af312  8b82a0020000          MOV EAX,dword ptr [EDX + 0x2a0]
007af318  8b11                  MOV EDX,dword ptr [ECX]
007af31a  8bc8                  MOV ECX,EAX
007af31c  8b421c                MOV EAX,dword ptr [EDX + 0x1c]
007af31f  ffd0                  CALL EAX
007af321  dc1dd8c19800          FCOMP double ptr [0x0098c1d8]
007af327  dfe0                  FNSTSW AX
007af329  f6c405                TEST AH,0x5
007af32c  7a30                  JP 0x007af35e
007af32e  8b8db4fdffff          MOV ECX,dword ptr [EBP + 0xfffffdb4]
007af334  8b9184000000          MOV EDX,dword ptr [ECX + 0x84]
007af33a  899520faffff          MOV dword ptr [EBP + 0xfffffa20],EDX
007af340  81bd20faffffeca50000  CMP dword ptr [EBP + 0xfffffa20],0xa5ec
007af34a  7510                  JNZ 0x007af35c
007af34c  68c7b20100            PUSH 0x1b2c7
007af351  8b8db4fdffff          MOV ECX,dword ptr [EBP + 0xfffffdb4]
007af357  e8c45d0300            CALL 0x007e5120
007af35c  eb10                  JMP 0x007af36e
007af35e  68eca50000            PUSH 0xa5ec
007af363  8b8db4fdffff          MOV ECX,dword ptr [EBP + 0xfffffdb4]
007af369  e8b25d0300            CALL 0x007e5120
007af36e  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af374  83b82003000000        CMP dword ptr [EAX + 0x320],0x0
007af37b  0f85b5000000          JNZ 0x007af436
007af381  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af387  d98190030000          FLD float ptr [ECX + 0x390]
007af38d  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af393  dfe0                  FNSTSW AX
007af395  f6c441                TEST AH,0x41
007af398  0f8a98000000          JP 0x007af436
007af39e  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af3a4  81bab40200000000007f  CMP dword ptr [EDX + 0x2b4],0x7f000000
007af3ae  0f8482000000          JZ 0x007af436
007af3b4  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af3ba  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af3c0  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af3c6  8b82a0020000          MOV EAX,dword ptr [EDX + 0x2a0]
007af3cc  8b11                  MOV EDX,dword ptr [ECX]
007af3ce  8bc8                  MOV ECX,EAX
007af3d0  8b421c                MOV EAX,dword ptr [EDX + 0x1c]
007af3d3  ffd0                  CALL EAX
007af3d5  dc1dd8c19800          FCOMP double ptr [0x0098c1d8]
007af3db  dfe0                  FNSTSW AX
007af3dd  f6c401                TEST AH,0x1
007af3e0  7554                  JNZ 0x007af436
007af3e2  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af3e8  d9816c030000          FLD float ptr [ECX + 0x36c]
007af3ee  d84508                FADD float ptr [EBP + 0x8]
007af3f1  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af3f7  d99a6c030000          FSTP float ptr [EDX + 0x36c]
007af3fd  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af403  d9806c030000          FLD float ptr [EAX + 0x36c]
007af409  dc1d087f9800          FCOMP double ptr [0x00987f08]
007af40f  dfe0                  FNSTSW AX
007af411  f6c401                TEST AH,0x1
007af414  751e                  JNZ 0x007af434
007af416  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af41c  d9ee                  FLDZ
007af41e  d9996c030000          FSTP float ptr [ECX + 0x36c]
007af424  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af42a  c782b40200000000007f  MOV dword ptr [EDX + 0x2b4],0x7f000000
007af434  eb0e                  JMP 0x007af444
007af436  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af43c  d9ee                  FLDZ
007af43e  d9986c030000          FSTP float ptr [EAX + 0x36c]
007af444  51                    PUSH ECX
007af445  d945b8                FLD float ptr [EBP + -0x48]
007af448  d91c24                FSTP float ptr [ESP]
007af44b  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af451  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
007af457  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af45d  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af463  8b12                  MOV EDX,dword ptr [EDX]
007af465  8b02                  MOV EAX,dword ptr [EDX]
007af467  ffd0                  CALL EAX
007af469  51                    PUSH ECX
007af46a  d945b0                FLD float ptr [EBP + -0x50]
007af46d  d91c24                FSTP float ptr [ESP]
007af470  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af476  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
007af47c  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af482  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af488  8b12                  MOV EDX,dword ptr [EDX]
007af48a  8b4208                MOV EAX,dword ptr [EDX + 0x8]
007af48d  ffd0                  CALL EAX
007af48f  e934030000            JMP 0x007af7c8
007af494  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007af49a  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007af49d  89951cfaffff          MOV dword ptr [EBP + 0xfffffa1c],EDX
007af4a3  8b8d1cfaffff          MOV ECX,dword ptr [EBP + 0xfffffa1c]
007af4a9  e85206f8ff            CALL 0x0072fb00
007af4ae  898518faffff          MOV dword ptr [EBP + 0xfffffa18],EAX
007af4b4  8b8518faffff          MOV EAX,dword ptr [EBP + 0xfffffa18]
007af4ba  8a4875                MOV CL,byte ptr [EAX + 0x75]
007af4bd  888d17faffff          MOV byte ptr [EBP + 0xfffffa17],CL
007af4c3  0fb69517faffff        MOVZX EDX,byte ptr [EBP + 0xfffffa17]
007af4ca  83fa07                CMP EDX,0x7
007af4cd  0f85b8010000          JNZ 0x007af68b
007af4d3  6a00                  PUSH 0x0
007af4d5  6a07                  PUSH 0x7
007af4d7  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af4dd  e8ee04f8ff            CALL 0x0072f9d0
007af4e2  d95db8                FSTP float ptr [EBP + -0x48]
007af4e5  6a00                  PUSH 0x0
007af4e7  6a08                  PUSH 0x8
007af4e9  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af4ef  e8dc04f8ff            CALL 0x0072f9d0
007af4f4  d95db0                FSTP float ptr [EBP + -0x50]
007af4f7  d945b8                FLD float ptr [EBP + -0x48]
007af4fa  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af500  dfe0                  FNSTSW AX
007af502  f6c444                TEST AH,0x44
007af505  7a3a                  JP 0x007af541
007af507  d945b0                FLD float ptr [EBP + -0x50]
007af50a  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af510  dfe0                  FNSTSW AX
007af512  f6c444                TEST AH,0x44
007af515  7a2a                  JP 0x007af541
007af517  6a00                  PUSH 0x0
007af519  6819010000            PUSH 0x119
007af51e  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af524  e8a704f8ff            CALL 0x0072f9d0
007af529  d95db0                FSTP float ptr [EBP + -0x50]
007af52c  6a00                  PUSH 0x0
007af52e  6818010000            PUSH 0x118
007af533  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af539  e89204f8ff            CALL 0x0072f9d0
007af53e  d95db8                FSTP float ptr [EBP + -0x48]
007af541  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007af546  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007af549  898d10faffff          MOV dword ptr [EBP + 0xfffffa10],ECX
007af54f  8b8d10faffff          MOV ECX,dword ptr [EBP + 0xfffffa10]
007af555  e8f601f9ff            CALL 0x0073f750
007af55a  8bc8                  MOV ECX,EAX
007af55c  e81f2d0200            CALL 0x007d2280
007af561  8985b0fdffff          MOV dword ptr [EBP + 0xfffffdb0],EAX
007af567  83bdb0fdffff00        CMP dword ptr [EBP + 0xfffffdb0],0x0
007af56e  740a                  JZ 0x007af57a
007af570  d9ee                  FLDZ
007af572  d95db8                FSTP float ptr [EBP + -0x48]
007af575  d9ee                  FLDZ
007af577  d95db0                FSTP float ptr [EBP + -0x50]
007af57a  ba01000000            MOV EDX,0x1
007af57f  85d2                  TEST EDX,EDX
007af581  7426                  JZ 0x007af5a9
007af583  6a00                  PUSH 0x0
007af585  6a0c                  PUSH 0xc
007af587  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af58d  e83e04f8ff            CALL 0x0072f9d0
007af592  d95db4                FSTP float ptr [EBP + -0x4c]
007af595  6a00                  PUSH 0x0
007af597  6a0d                  PUSH 0xd
007af599  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af59f  e82c04f8ff            CALL 0x0072f9d0
007af5a4  d95de0                FSTP float ptr [EBP + -0x20]
007af5a7  eb1e                  JMP 0x007af5c7
007af5a9  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af5af  db80c8030000          FILD dword ptr [EAX + 0x3c8]
007af5b5  d95db4                FSTP float ptr [EBP + -0x4c]
007af5b8  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af5be  db81cc030000          FILD dword ptr [ECX + 0x3cc]
007af5c4  d95de0                FSTP float ptr [EBP + -0x20]
007af5c7  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007af5cd  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007af5d0  89850cfaffff          MOV dword ptr [EBP + 0xfffffa0c],EAX
007af5d6  8b8d0cfaffff          MOV ECX,dword ptr [EBP + 0xfffffa0c]
007af5dc  e84ffef8ff            CALL 0x0073f430
007af5e1  898508faffff          MOV dword ptr [EBP + 0xfffffa08],EAX
007af5e7  8b8d04faffff          MOV ECX,dword ptr [EBP + 0xfffffa04]
007af5ed  898ddcf8ffff          MOV dword ptr [EBP + 0xfffff8dc],ECX
007af5f3  8b9508faffff          MOV EDX,dword ptr [EBP + 0xfffffa08]
007af5f9  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007af5fc  8985dcf8ffff          MOV dword ptr [EBP + 0xfffff8dc],EAX
007af602  6aff                  PUSH -0x1
007af604  8b8ddcf8ffff          MOV ECX,dword ptr [EBP + 0xfffff8dc]
007af60a  8b11                  MOV EDX,dword ptr [ECX]
007af60c  8b8ddcf8ffff          MOV ECX,dword ptr [EBP + 0xfffff8dc]
007af612  8b8280000000          MOV EAX,dword ptr [EDX + 0x80]
007af618  ffd0                  CALL EAX
007af61a  8945f0                MOV dword ptr [EBP + -0x10],EAX
007af61d  837df000              CMP dword ptr [EBP + -0x10],0x0
007af621  7468                  JZ 0x007af68b
007af623  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007af626  8b11                  MOV EDX,dword ptr [ECX]
007af628  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007af62b  8b4210                MOV EAX,dword ptr [EDX + 0x10]
007af62e  ffd0                  CALL EAX
007af630  3d71100000            CMP EAX,0x1071
007af635  7554                  JNZ 0x007af68b
007af637  d945b8                FLD float ptr [EBP + -0x48]
007af63a  dc0de09e9800          FMUL double ptr [0x00989ee0]
007af640  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007af643  d9591c                FSTP float ptr [ECX + 0x1c]
007af646  d945b0                FLD float ptr [EBP + -0x50]
007af649  dc0de09e9800          FMUL double ptr [0x00989ee0]
007af64f  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
007af652  d95a24                FSTP float ptr [EDX + 0x24]
007af655  d945b4                FLD float ptr [EBP + -0x4c]
007af658  d9e0                  FCHS
007af65a  dc0df8469900          FMUL double ptr [0x009946f8]
007af660  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
007af663  d95818                FSTP float ptr [EAX + 0x18]
007af666  d945e0                FLD float ptr [EBP + -0x20]
007af669  d9e0                  FCHS
007af66b  dc0d58139900          FMUL double ptr [0x00991358]
007af671  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007af674  d95914                FSTP float ptr [ECX + 0x14]
007af677  d945b8                FLD float ptr [EBP + -0x48]
007af67a  d9e0                  FCHS
007af67c  d84de0                FMUL float ptr [EBP + -0x20]
007af67f  dc0d90bd9800          FMUL double ptr [0x0098bd90]
007af685  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
007af688  d95a28                FSTP float ptr [EDX + 0x28]
007af68b  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007af690  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007af693  898d00faffff          MOV dword ptr [EBP + 0xfffffa00],ECX
007af699  8b8d00faffff          MOV ECX,dword ptr [EBP + 0xfffffa00]
007af69f  e85c04f8ff            CALL 0x0072fb00
007af6a4  8985fcf9ffff          MOV dword ptr [EBP + 0xfffff9fc],EAX
007af6aa  8b95fcf9ffff          MOV EDX,dword ptr [EBP + 0xfffff9fc]
007af6b0  8a4275                MOV AL,byte ptr [EDX + 0x75]
007af6b3  8885fbf9ffff          MOV byte ptr [EBP + 0xfffff9fb],AL
007af6b9  0fb68dfbf9ffff        MOVZX ECX,byte ptr [EBP + 0xfffff9fb]
007af6c0  83f905                CMP ECX,0x5
007af6c3  0f85b6000000          JNZ 0x007af77f
007af6c9  6a00                  PUSH 0x0
007af6cb  6a07                  PUSH 0x7
007af6cd  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af6d3  e8f802f8ff            CALL 0x0072f9d0
007af6d8  d95db8                FSTP float ptr [EBP + -0x48]
007af6db  6a00                  PUSH 0x0
007af6dd  6a08                  PUSH 0x8
007af6df  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af6e5  e8e602f8ff            CALL 0x0072f9d0
007af6ea  d95db0                FSTP float ptr [EBP + -0x50]
007af6ed  d945b8                FLD float ptr [EBP + -0x48]
007af6f0  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af6f6  dfe0                  FNSTSW AX
007af6f8  f6c444                TEST AH,0x44
007af6fb  7a3a                  JP 0x007af737
007af6fd  d945b0                FLD float ptr [EBP + -0x50]
007af700  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af706  dfe0                  FNSTSW AX
007af708  f6c444                TEST AH,0x44
007af70b  7a2a                  JP 0x007af737
007af70d  6a04                  PUSH 0x4
007af70f  6819010000            PUSH 0x119
007af714  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af71a  e8b102f8ff            CALL 0x0072f9d0
007af71f  d95db0                FSTP float ptr [EBP + -0x50]
007af722  6a04                  PUSH 0x4
007af724  6818010000            PUSH 0x118
007af729  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af72f  e89c02f8ff            CALL 0x0072f9d0
007af734  d95db8                FSTP float ptr [EBP + -0x48]
007af737  d945b8                FLD float ptr [EBP + -0x48]
007af73a  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af740  dfe0                  FNSTSW AX
007af742  f6c444                TEST AH,0x44
007af745  7a10                  JP 0x007af757
007af747  d945b0                FLD float ptr [EBP + -0x50]
007af74a  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af750  dfe0                  FNSTSW AX
007af752  f6c444                TEST AH,0x44
007af755  7b28                  JNP 0x007af77f
007af757  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007af75d  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007af760  8985f4f9ffff          MOV dword ptr [EBP + 0xfffff9f4],EAX
007af766  51                    PUSH ECX
007af767  d945b8                FLD float ptr [EBP + -0x48]
007af76a  d91c24                FSTP float ptr [ESP]
007af76d  8b8df4f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9f4]
007af773  e8b8fcf8ff            CALL 0x0073f430
007af778  8bc8                  MOV ECX,EAX
007af77a  e861460400            CALL 0x007f3de0
007af77f  51                    PUSH ECX
007af780  d9ee                  FLDZ
007af782  d91c24                FSTP float ptr [ESP]
007af785  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af78b  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
007af791  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af797  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af79d  8b12                  MOV EDX,dword ptr [EDX]
007af79f  8b02                  MOV EAX,dword ptr [EDX]
007af7a1  ffd0                  CALL EAX
007af7a3  51                    PUSH ECX
007af7a4  d9ee                  FLDZ
007af7a6  d91c24                FSTP float ptr [ESP]
007af7a9  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af7af  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
007af7b5  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af7bb  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af7c1  8b12                  MOV EDX,dword ptr [EDX]
007af7c3  8b4208                MOV EAX,dword ptr [EDX + 0x8]
007af7c6  ffd0                  CALL EAX
007af7c8  c745c000000000        MOV dword ptr [EBP + -0x40],0x0
007af7cf  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af7d5  e806d7fdff            CALL 0x0078cee0
007af7da  85c0                  TEST EAX,EAX
007af7dc  0f8488000000          JZ 0x007af86a
007af7e2  51                    PUSH ECX
007af7e3  d94508                FLD float ptr [EBP + 0x8]
007af7e6  d91c24                FSTP float ptr [ESP]
007af7e9  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af7ef  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
007af7f5  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007af7fb  8b88a0020000          MOV ECX,dword ptr [EAX + 0x2a0]
007af801  8b12                  MOV EDX,dword ptr [EDX]
007af803  8b4228                MOV EAX,dword ptr [EDX + 0x28]
007af806  ffd0                  CALL EAX
007af808  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af80e  83791800              CMP dword ptr [ECX + 0x18],0x0
007af812  7451                  JZ 0x007af865
007af814  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af81a  8b4218                MOV EAX,dword ptr [EDX + 0x18]
007af81d  8b4848                MOV ECX,dword ptr [EAX + 0x48]
007af820  898df0f9ffff          MOV dword ptr [EBP + 0xfffff9f0],ECX
007af826  83bdf0f9ffff00        CMP dword ptr [EBP + 0xfffff9f0],0x0
007af82d  7436                  JZ 0x007af865
007af82f  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af835  8b4218                MOV EAX,dword ptr [EDX + 0x18]
007af838  8b4848                MOV ECX,dword ptr [EAX + 0x48]
007af83b  898decf9ffff          MOV dword ptr [EBP + 0xfffff9ec],ECX
007af841  8b95ecf9ffff          MOV EDX,dword ptr [EBP + 0xfffff9ec]
007af847  8b8268020000          MOV EAX,dword ptr [EDX + 0x268]
007af84d  8945c0                MOV dword ptr [EBP + -0x40],EAX
007af850  837dc000              CMP dword ptr [EBP + -0x40],0x0
007af854  740f                  JZ 0x007af865
007af856  51                    PUSH ECX
007af857  d94508                FLD float ptr [EBP + 0x8]
007af85a  d91c24                FSTP float ptr [ESP]
007af85d  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
007af860  e8ebdd0800            CALL 0x0083d650
007af865  e993090000            JMP 0x007b01fd
007af86a  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007af870  83791800              CMP dword ptr [ECX + 0x18],0x0
007af874  0f8483090000          JZ 0x007b01fd
007af87a  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af880  8b4218                MOV EAX,dword ptr [EDX + 0x18]
007af883  8b4848                MOV ECX,dword ptr [EAX + 0x48]
007af886  898de8f9ffff          MOV dword ptr [EBP + 0xfffff9e8],ECX
007af88c  83bde8f9ffff00        CMP dword ptr [EBP + 0xfffff9e8],0x0
007af893  0f8464090000          JZ 0x007b01fd
007af899  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007af89f  8b4218                MOV EAX,dword ptr [EDX + 0x18]
007af8a2  8b4848                MOV ECX,dword ptr [EAX + 0x48]
007af8a5  898de4f9ffff          MOV dword ptr [EBP + 0xfffff9e4],ECX
007af8ab  8b95e4f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9e4]
007af8b1  8b8268020000          MOV EAX,dword ptr [EDX + 0x268]
007af8b7  8945c0                MOV dword ptr [EBP + -0x40],EAX
007af8ba  837dc000              CMP dword ptr [EBP + -0x40],0x0
007af8be  0f8439090000          JZ 0x007b01fd
007af8c4  837ddc00              CMP dword ptr [EBP + -0x24],0x0
007af8c8  0f852f090000          JNZ 0x007b01fd
007af8ce  8b8de0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9e0]
007af8d4  898dd8f8ffff          MOV dword ptr [EBP + 0xfffff8d8],ECX
007af8da  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
007af8dd  8b8280000000          MOV EAX,dword ptr [EDX + 0x80]
007af8e3  8985d8f8ffff          MOV dword ptr [EBP + 0xfffff8d8],EAX
007af8e9  83bdd8f8ffff01        CMP dword ptr [EBP + 0xfffff8d8],0x1
007af8f0  7412                  JZ 0x007af904
007af8f2  83bdd8f8ffff02        CMP dword ptr [EBP + 0xfffff8d8],0x2
007af8f9  0f8409030000          JZ 0x007afc08
007af8ff  e9ea080000            JMP 0x007b01ee
007af904  d9ee                  FLDZ
007af906  d99da8fdffff          FSTP float ptr [EBP + 0xfffffda8]
007af90c  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007af912  d99dacfdffff          FSTP float ptr [EBP + 0xfffffdac]
007af918  8d8dacfdffff          LEA ECX,[EBP + 0xfffffdac]
007af91e  51                    PUSH ECX
007af91f  8d95a8fdffff          LEA EDX,[EBP + 0xfffffda8]
007af925  52                    PUSH EDX
007af926  6a00                  PUSH 0x0
007af928  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af92e  e87d04f8ff            CALL 0x0072fdb0
007af933  6a01                  PUSH 0x1
007af935  6a17                  PUSH 0x17
007af937  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007af93d  e88e00f8ff            CALL 0x0072f9d0
007af942  d8352c07a100          FDIV float ptr [0x00a1072c]
007af948  d885a8fdffff          FADD float ptr [EBP + 0xfffffda8]
007af94e  d99da8fdffff          FSTP float ptr [EBP + 0xfffffda8]
007af954  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007af95a  dc1d986d9800          FCOMP double ptr [0x00986d98]
007af960  dfe0                  FNSTSW AX
007af962  f6c405                TEST AH,0x5
007af965  7a0e                  JP 0x007af975
007af967  d90574749800          FLD float ptr [0x00987474]
007af96d  d99dd4f8ffff          FSTP float ptr [EBP + 0xfffff8d4]
007af973  eb0c                  JMP 0x007af981
007af975  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007af97b  d99dd4f8ffff          FSTP float ptr [EBP + 0xfffff8d4]
007af981  d985d4f8ffff          FLD float ptr [EBP + 0xfffff8d4]
007af987  dc1d506b9800          FCOMP double ptr [0x00986b50]
007af98d  dfe0                  FNSTSW AX
007af98f  f6c441                TEST AH,0x41
007af992  750a                  JNZ 0x007af99e
007af994  d9e8                  FLD1
007af996  d99dd0f8ffff          FSTP float ptr [EBP + 0xfffff8d0]
007af99c  eb39                  JMP 0x007af9d7
007af99e  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007af9a4  dc1d986d9800          FCOMP double ptr [0x00986d98]
007af9aa  dfe0                  FNSTSW AX
007af9ac  f6c405                TEST AH,0x5
007af9af  7a0e                  JP 0x007af9bf
007af9b1  d90574749800          FLD float ptr [0x00987474]
007af9b7  d99dccf8ffff          FSTP float ptr [EBP + 0xfffff8cc]
007af9bd  eb0c                  JMP 0x007af9cb
007af9bf  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007af9c5  d99dccf8ffff          FSTP float ptr [EBP + 0xfffff8cc]
007af9cb  d985ccf8ffff          FLD float ptr [EBP + 0xfffff8cc]
007af9d1  d99dd0f8ffff          FSTP float ptr [EBP + 0xfffff8d0]
007af9d7  d985d0f8ffff          FLD float ptr [EBP + 0xfffff8d0]
007af9dd  d99da8fdffff          FSTP float ptr [EBP + 0xfffffda8]
007af9e3  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007af9e9  dc1d386b9800          FCOMP double ptr [0x00986b38]
007af9ef  dfe0                  FNSTSW AX
007af9f1  f6c444                TEST AH,0x44
007af9f4  0f8a3a010000          JP 0x007afb34
007af9fa  6a01                  PUSH 0x1
007af9fc  681e010000            PUSH 0x11e
007afa01  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afa07  e8c4fff7ff            CALL 0x0072f9d0
007afa0c  6a01                  PUSH 0x1
007afa0e  681b010000            PUSH 0x11b
007afa13  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afa19  dd9dc4f8ffff          FSTP double ptr [EBP + 0xfffff8c4]
007afa1f  e8acfff7ff            CALL 0x0072f9d0
007afa24  dc85c4f8ffff          FADD double ptr [EBP + 0xfffff8c4]
007afa2a  dc1d986d9800          FCOMP double ptr [0x00986d98]
007afa30  dfe0                  FNSTSW AX
007afa32  f6c405                TEST AH,0x5
007afa35  7a0e                  JP 0x007afa45
007afa37  dd05986d9800          FLD double ptr [0x00986d98]
007afa3d  dd9dbcf8ffff          FSTP double ptr [EBP + 0xfffff8bc]
007afa43  eb36                  JMP 0x007afa7b
007afa45  6a01                  PUSH 0x1
007afa47  681e010000            PUSH 0x11e
007afa4c  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afa52  e879fff7ff            CALL 0x0072f9d0
007afa57  6a01                  PUSH 0x1
007afa59  681b010000            PUSH 0x11b
007afa5e  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afa64  dd9db4f8ffff          FSTP double ptr [EBP + 0xfffff8b4]
007afa6a  e861fff7ff            CALL 0x0072f9d0
007afa6f  dc85b4f8ffff          FADD double ptr [EBP + 0xfffff8b4]
007afa75  dd9dbcf8ffff          FSTP double ptr [EBP + 0xfffff8bc]
007afa7b  dd85bcf8ffff          FLD double ptr [EBP + 0xfffff8bc]
007afa81  dc1d506b9800          FCOMP double ptr [0x00986b50]
007afa87  dfe0                  FNSTSW AX
007afa89  f6c441                TEST AH,0x41
007afa8c  750d                  JNZ 0x007afa9b
007afa8e  d9e8                  FLD1
007afa90  dd9dacf8ffff          FSTP double ptr [EBP + 0xfffff8ac]
007afa96  e98d000000            JMP 0x007afb28
007afa9b  6a01                  PUSH 0x1
007afa9d  681e010000            PUSH 0x11e
007afaa2  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afaa8  e823fff7ff            CALL 0x0072f9d0
007afaad  6a01                  PUSH 0x1
007afaaf  681b010000            PUSH 0x11b
007afab4  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afaba  dd9da4f8ffff          FSTP double ptr [EBP + 0xfffff8a4]
007afac0  e80bfff7ff            CALL 0x0072f9d0
007afac5  dc85a4f8ffff          FADD double ptr [EBP + 0xfffff8a4]
007afacb  dc1d986d9800          FCOMP double ptr [0x00986d98]
007afad1  dfe0                  FNSTSW AX
007afad3  f6c405                TEST AH,0x5
007afad6  7a0e                  JP 0x007afae6
007afad8  dd05986d9800          FLD double ptr [0x00986d98]
007afade  dd9d9cf8ffff          FSTP double ptr [EBP + 0xfffff89c]
007afae4  eb36                  JMP 0x007afb1c
007afae6  6a01                  PUSH 0x1
007afae8  681e010000            PUSH 0x11e
007afaed  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afaf3  e8d8fef7ff            CALL 0x0072f9d0
007afaf8  6a01                  PUSH 0x1
007afafa  681b010000            PUSH 0x11b
007afaff  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afb05  dd9d94f8ffff          FSTP double ptr [EBP + 0xfffff894]
007afb0b  e8c0fef7ff            CALL 0x0072f9d0
007afb10  dc8594f8ffff          FADD double ptr [EBP + 0xfffff894]
007afb16  dd9d9cf8ffff          FSTP double ptr [EBP + 0xfffff89c]
007afb1c  dd859cf8ffff          FLD double ptr [EBP + 0xfffff89c]
007afb22  dd9dacf8ffff          FSTP double ptr [EBP + 0xfffff8ac]
007afb28  dd85acf8ffff          FLD double ptr [EBP + 0xfffff8ac]
007afb2e  d99da8fdffff          FSTP float ptr [EBP + 0xfffffda8]
007afb34  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007afb3a  d99ddcf9ffff          FSTP float ptr [EBP + 0xfffff9dc]
007afb40  d985dcf9ffff          FLD float ptr [EBP + 0xfffff9dc]
007afb46  d99d9cfdffff          FSTP float ptr [EBP + 0xfffffd9c]
007afb4c  d9ee                  FLDZ
007afb4e  d99da0fdffff          FSTP float ptr [EBP + 0xfffffda0]
007afb54  d9ee                  FLDZ
007afb56  d99da4fdffff          FSTP float ptr [EBP + 0xfffffda4]
007afb5c  51                    PUSH ECX
007afb5d  d94508                FLD float ptr [EBP + 0x8]
007afb60  d91c24                FSTP float ptr [ESP]
007afb63  8d859cfdffff          LEA EAX,[EBP + 0xfffffd9c]
007afb69  50                    PUSH EAX
007afb6a  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
007afb6d  e83ed80800            CALL 0x0083d3b0
007afb72  51                    PUSH ECX
007afb73  d94508                FLD float ptr [EBP + 0x8]
007afb76  d91c24                FSTP float ptr [ESP]
007afb79  51                    PUSH ECX
007afb7a  d9ee                  FLDZ
007afb7c  d91c24                FSTP float ptr [ESP]
007afb7f  51                    PUSH ECX
007afb80  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007afb86  d91c24                FSTP float ptr [ESP]
007afb89  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
007afb8c  e88fd90800            CALL 0x0083d520
007afb91  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007afb97  e864c3feff            CALL 0x0079bf00
007afb9c  85c0                  TEST EAX,EAX
007afb9e  7563                  JNZ 0x007afc03
007afba0  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007afba6  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007afba9  8995d8f9ffff          MOV dword ptr [EBP + 0xfffff9d8],EDX
007afbaf  8b8dd8f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9d8]
007afbb5  e84603f9ff            CALL 0x0073ff00
007afbba  85c0                  TEST EAX,EAX
007afbbc  7445                  JZ 0x007afc03
007afbbe  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007afbc4  8b8874020000          MOV ECX,dword ptr [EAX + 0x274]
007afbca  0fbf516e              MOVSX EDX,word ptr [ECX + 0x6e]
007afbce  8995d4f9ffff          MOV dword ptr [EBP + 0xfffff9d4],EDX
007afbd4  6a01                  PUSH 0x1
007afbd6  8b85d4f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9d4]
007afbdc  99                    CDQ
007afbdd  2bc2                  SUB EAX,EDX
007afbdf  d1f8                  SAR EAX,0x1
007afbe1  50                    PUSH EAX
007afbe2  8b85a0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a0]
007afbe8  8b8874020000          MOV ECX,dword ptr [EAX + 0x274]
007afbee  0fbf416c              MOVSX EAX,word ptr [ECX + 0x6c]
007afbf2  99                    CDQ
007afbf3  2bc2                  SUB EAX,EDX
007afbf5  d1f8                  SAR EAX,0x1
007afbf7  50                    PUSH EAX
007afbf8  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afbfe  e87d00f8ff            CALL 0x0072fc80
007afc03  e9e6050000            JMP 0x007b01ee
007afc08  8d9598fdffff          LEA EDX,[EBP + 0xfffffd98]
007afc0e  52                    PUSH EDX
007afc0f  8d8594fdffff          LEA EAX,[EBP + 0xfffffd94]
007afc15  50                    PUSH EAX
007afc16  6a00                  PUSH 0x0
007afc18  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afc1e  e88d01f8ff            CALL 0x0072fdb0
007afc23  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007afc29  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007afc2c  8995d0f9ffff          MOV dword ptr [EBP + 0xfffff9d0],EDX
007afc32  8b8dd0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9d0]
007afc38  e8c3fef7ff            CALL 0x0072fb00
007afc3d  8985ccf9ffff          MOV dword ptr [EBP + 0xfffff9cc],EAX
007afc43  8b85ccf9ffff          MOV EAX,dword ptr [EBP + 0xfffff9cc]
007afc49  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007afc4c  c1e903                SHR ECX,0x3
007afc4f  83e101                AND ECX,0x1
007afc52  7412                  JZ 0x007afc66
007afc54  d98598fdffff          FLD float ptr [EBP + 0xfffffd98]
007afc5a  dc0d986d9800          FMUL double ptr [0x00986d98]
007afc60  d99d98fdffff          FSTP float ptr [EBP + 0xfffffd98]
007afc66  51                    PUSH ECX
007afc67  d94508                FLD float ptr [EBP + 0x8]
007afc6a  d91c24                FSTP float ptr [ESP]
007afc6d  51                    PUSH ECX
007afc6e  d98598fdffff          FLD float ptr [EBP + 0xfffffd98]
007afc74  d91c24                FSTP float ptr [ESP]
007afc77  51                    PUSH ECX
007afc78  d98594fdffff          FLD float ptr [EBP + 0xfffffd94]
007afc7e  d91c24                FSTP float ptr [ESP]
007afc81  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
007afc84  e857d80800            CALL 0x0083d4e0
007afc89  6a01                  PUSH 0x1
007afc8b  681e010000            PUSH 0x11e
007afc90  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afc96  e835fdf7ff            CALL 0x0072f9d0
007afc9b  6a01                  PUSH 0x1
007afc9d  681b010000            PUSH 0x11b
007afca2  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afca8  dd9d8cf8ffff          FSTP double ptr [EBP + 0xfffff88c]
007afcae  e81dfdf7ff            CALL 0x0072f9d0
007afcb3  dc858cf8ffff          FADD double ptr [EBP + 0xfffff88c]
007afcb9  dc1d986d9800          FCOMP double ptr [0x00986d98]
007afcbf  dfe0                  FNSTSW AX
007afcc1  f6c405                TEST AH,0x5
007afcc4  7a0e                  JP 0x007afcd4
007afcc6  dd05986d9800          FLD double ptr [0x00986d98]
007afccc  dd9d84f8ffff          FSTP double ptr [EBP + 0xfffff884]
007afcd2  eb36                  JMP 0x007afd0a
007afcd4  6a01                  PUSH 0x1
007afcd6  681e010000            PUSH 0x11e
007afcdb  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afce1  e8eafcf7ff            CALL 0x0072f9d0
007afce6  6a01                  PUSH 0x1
007afce8  681b010000            PUSH 0x11b
007afced  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afcf3  dd9d7cf8ffff          FSTP double ptr [EBP + 0xfffff87c]
007afcf9  e8d2fcf7ff            CALL 0x0072f9d0
007afcfe  dc857cf8ffff          FADD double ptr [EBP + 0xfffff87c]
007afd04  dd9d84f8ffff          FSTP double ptr [EBP + 0xfffff884]
007afd0a  dd8584f8ffff          FLD double ptr [EBP + 0xfffff884]
007afd10  dc1d506b9800          FCOMP double ptr [0x00986b50]
007afd16  dfe0                  FNSTSW AX
007afd18  f6c441                TEST AH,0x41
007afd1b  750d                  JNZ 0x007afd2a
007afd1d  d9e8                  FLD1
007afd1f  dd9d74f8ffff          FSTP double ptr [EBP + 0xfffff874]
007afd25  e98d000000            JMP 0x007afdb7
007afd2a  6a01                  PUSH 0x1
007afd2c  681e010000            PUSH 0x11e
007afd31  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afd37  e894fcf7ff            CALL 0x0072f9d0
007afd3c  6a01                  PUSH 0x1
007afd3e  681b010000            PUSH 0x11b
007afd43  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afd49  dd9d6cf8ffff          FSTP double ptr [EBP + 0xfffff86c]
007afd4f  e87cfcf7ff            CALL 0x0072f9d0
007afd54  dc856cf8ffff          FADD double ptr [EBP + 0xfffff86c]
007afd5a  dc1d986d9800          FCOMP double ptr [0x00986d98]
007afd60  dfe0                  FNSTSW AX
007afd62  f6c405                TEST AH,0x5
007afd65  7a0e                  JP 0x007afd75
007afd67  dd05986d9800          FLD double ptr [0x00986d98]
007afd6d  dd9d64f8ffff          FSTP double ptr [EBP + 0xfffff864]
007afd73  eb36                  JMP 0x007afdab
007afd75  6a01                  PUSH 0x1
007afd77  681e010000            PUSH 0x11e
007afd7c  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afd82  e849fcf7ff            CALL 0x0072f9d0
007afd87  6a01                  PUSH 0x1
007afd89  681b010000            PUSH 0x11b
007afd8e  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afd94  dd9d5cf8ffff          FSTP double ptr [EBP + 0xfffff85c]
007afd9a  e831fcf7ff            CALL 0x0072f9d0
007afd9f  dc855cf8ffff          FADD double ptr [EBP + 0xfffff85c]
007afda5  dd9d64f8ffff          FSTP double ptr [EBP + 0xfffff864]
007afdab  dd8564f8ffff          FLD double ptr [EBP + 0xfffff864]
007afdb1  dd9d74f8ffff          FSTP double ptr [EBP + 0xfffff874]
007afdb7  dd8574f8ffff          FLD double ptr [EBP + 0xfffff874]
007afdbd  d99da8fdffff          FSTP float ptr [EBP + 0xfffffda8]
007afdc3  6a01                  PUSH 0x1
007afdc5  681d010000            PUSH 0x11d
007afdca  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afdd0  e8fbfbf7ff            CALL 0x0072f9d0
007afdd5  6a01                  PUSH 0x1
007afdd7  681a010000            PUSH 0x11a
007afddc  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afde2  dd9d54f8ffff          FSTP double ptr [EBP + 0xfffff854]
007afde8  e8e3fbf7ff            CALL 0x0072f9d0
007afded  dc8554f8ffff          FADD double ptr [EBP + 0xfffff854]
007afdf3  dc1d986d9800          FCOMP double ptr [0x00986d98]
007afdf9  dfe0                  FNSTSW AX
007afdfb  f6c405                TEST AH,0x5
007afdfe  7a0e                  JP 0x007afe0e
007afe00  dd05986d9800          FLD double ptr [0x00986d98]
007afe06  dd9d4cf8ffff          FSTP double ptr [EBP + 0xfffff84c]
007afe0c  eb36                  JMP 0x007afe44
007afe0e  6a01                  PUSH 0x1
007afe10  681d010000            PUSH 0x11d
007afe15  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afe1b  e8b0fbf7ff            CALL 0x0072f9d0
007afe20  6a01                  PUSH 0x1
007afe22  681a010000            PUSH 0x11a
007afe27  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afe2d  dd9d44f8ffff          FSTP double ptr [EBP + 0xfffff844]
007afe33  e898fbf7ff            CALL 0x0072f9d0
007afe38  dc8544f8ffff          FADD double ptr [EBP + 0xfffff844]
007afe3e  dd9d4cf8ffff          FSTP double ptr [EBP + 0xfffff84c]
007afe44  dd854cf8ffff          FLD double ptr [EBP + 0xfffff84c]
007afe4a  dc1d506b9800          FCOMP double ptr [0x00986b50]
007afe50  dfe0                  FNSTSW AX
007afe52  f6c441                TEST AH,0x41
007afe55  750d                  JNZ 0x007afe64
007afe57  d9e8                  FLD1
007afe59  dd9d3cf8ffff          FSTP double ptr [EBP + 0xfffff83c]
007afe5f  e98d000000            JMP 0x007afef1
007afe64  6a01                  PUSH 0x1
007afe66  681d010000            PUSH 0x11d
007afe6b  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afe71  e85afbf7ff            CALL 0x0072f9d0
007afe76  6a01                  PUSH 0x1
007afe78  681a010000            PUSH 0x11a
007afe7d  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afe83  dd9d34f8ffff          FSTP double ptr [EBP + 0xfffff834]
007afe89  e842fbf7ff            CALL 0x0072f9d0
007afe8e  dc8534f8ffff          FADD double ptr [EBP + 0xfffff834]
007afe94  dc1d986d9800          FCOMP double ptr [0x00986d98]
007afe9a  dfe0                  FNSTSW AX
007afe9c  f6c405                TEST AH,0x5
007afe9f  7a0e                  JP 0x007afeaf
007afea1  dd05986d9800          FLD double ptr [0x00986d98]
007afea7  dd9d2cf8ffff          FSTP double ptr [EBP + 0xfffff82c]
007afead  eb36                  JMP 0x007afee5
007afeaf  6a01                  PUSH 0x1
007afeb1  681d010000            PUSH 0x11d
007afeb6  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afebc  e80ffbf7ff            CALL 0x0072f9d0
007afec1  6a01                  PUSH 0x1
007afec3  681a010000            PUSH 0x11a
007afec8  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007afece  dd9d24f8ffff          FSTP double ptr [EBP + 0xfffff824]
007afed4  e8f7faf7ff            CALL 0x0072f9d0
007afed9  dc8524f8ffff          FADD double ptr [EBP + 0xfffff824]
007afedf  dd9d2cf8ffff          FSTP double ptr [EBP + 0xfffff82c]
007afee5  dd852cf8ffff          FLD double ptr [EBP + 0xfffff82c]
007afeeb  dd9d3cf8ffff          FSTP double ptr [EBP + 0xfffff83c]
007afef1  dd853cf8ffff          FLD double ptr [EBP + 0xfffff83c]
007afef7  d99dacfdffff          FSTP float ptr [EBP + 0xfffffdac]
007afefd  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007aff03  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007aff06  8985c8f9ffff          MOV dword ptr [EBP + 0xfffff9c8],EAX
007aff0c  8b8dc8f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9c8]
007aff12  e8e9fbf7ff            CALL 0x0072fb00
007aff17  85c0                  TEST EAX,EAX
007aff19  743f                  JZ 0x007aff5a
007aff1b  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007aff21  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007aff24  8995c4f9ffff          MOV dword ptr [EBP + 0xfffff9c4],EDX
007aff2a  8b8dc4f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9c4]
007aff30  e8cbfbf7ff            CALL 0x0072fb00
007aff35  8985c0f9ffff          MOV dword ptr [EBP + 0xfffff9c0],EAX
007aff3b  8b85c0f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9c0]
007aff41  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007aff44  c1e903                SHR ECX,0x3
007aff47  83e101                AND ECX,0x1
007aff4a  740e                  JZ 0x007aff5a
007aff4c  d985acfdffff          FLD float ptr [EBP + 0xfffffdac]
007aff52  d9e0                  FCHS
007aff54  d99dacfdffff          FSTP float ptr [EBP + 0xfffffdac]
007aff5a  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007aff60  d99db8f9ffff          FSTP float ptr [EBP + 0xfffff9b8]
007aff66  d985acfdffff          FLD float ptr [EBP + 0xfffffdac]
007aff6c  d99dbcf9ffff          FSTP float ptr [EBP + 0xfffff9bc]
007aff72  d985b8f9ffff          FLD float ptr [EBP + 0xfffff9b8]
007aff78  d99d24fdffff          FSTP float ptr [EBP + 0xfffffd24]
007aff7e  d9ee                  FLDZ
007aff80  d99d28fdffff          FSTP float ptr [EBP + 0xfffffd28]
007aff86  d985bcf9ffff          FLD float ptr [EBP + 0xfffff9bc]
007aff8c  d99d2cfdffff          FSTP float ptr [EBP + 0xfffffd2c]
007aff92  51                    PUSH ECX
007aff93  d94508                FLD float ptr [EBP + 0x8]
007aff96  d91c24                FSTP float ptr [ESP]
007aff99  8d9524fdffff          LEA EDX,[EBP + 0xfffffd24]
007aff9f  52                    PUSH EDX
007affa0  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
007affa3  e808d40800            CALL 0x0083d3b0
007affa8  6a01                  PUSH 0x1
007affaa  6a17                  PUSH 0x17
007affac  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007affb2  e819faf7ff            CALL 0x0072f9d0
007affb7  d8352c07a100          FDIV float ptr [0x00a1072c]
007affbd  d99da8fdffff          FSTP float ptr [EBP + 0xfffffda8]
007affc3  6a01                  PUSH 0x1
007affc5  6a18                  PUSH 0x18
007affc7  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007affcd  e8fef9f7ff            CALL 0x0072f9d0
007affd2  d9e0                  FCHS
007affd4  d8352c07a100          FDIV float ptr [0x00a1072c]
007affda  d99dacfdffff          FSTP float ptr [EBP + 0xfffffdac]
007affe0  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007affe6  dc1d986d9800          FCOMP double ptr [0x00986d98]
007affec  dfe0                  FNSTSW AX
007affee  f6c405                TEST AH,0x5
007afff1  7a0e                  JP 0x007b0001
007afff3  d90574749800          FLD float ptr [0x00987474]
007afff9  d99d20f8ffff          FSTP float ptr [EBP + 0xfffff820]
007affff  eb0c                  JMP 0x007b000d
007b0001  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007b0007  d99d20f8ffff          FSTP float ptr [EBP + 0xfffff820]
007b000d  d98520f8ffff          FLD float ptr [EBP + 0xfffff820]
007b0013  dc1d506b9800          FCOMP double ptr [0x00986b50]
007b0019  dfe0                  FNSTSW AX
007b001b  f6c441                TEST AH,0x41
007b001e  750a                  JNZ 0x007b002a
007b0020  d9e8                  FLD1
007b0022  d99d1cf8ffff          FSTP float ptr [EBP + 0xfffff81c]
007b0028  eb39                  JMP 0x007b0063
007b002a  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007b0030  dc1d986d9800          FCOMP double ptr [0x00986d98]
007b0036  dfe0                  FNSTSW AX
007b0038  f6c405                TEST AH,0x5
007b003b  7a0e                  JP 0x007b004b
007b003d  d90574749800          FLD float ptr [0x00987474]
007b0043  d99d18f8ffff          FSTP float ptr [EBP + 0xfffff818]
007b0049  eb0c                  JMP 0x007b0057
007b004b  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007b0051  d99d18f8ffff          FSTP float ptr [EBP + 0xfffff818]
007b0057  d98518f8ffff          FLD float ptr [EBP + 0xfffff818]
007b005d  d99d1cf8ffff          FSTP float ptr [EBP + 0xfffff81c]
007b0063  d9851cf8ffff          FLD float ptr [EBP + 0xfffff81c]
007b0069  d99da8fdffff          FSTP float ptr [EBP + 0xfffffda8]
007b006f  d985acfdffff          FLD float ptr [EBP + 0xfffffdac]
007b0075  dc1d986d9800          FCOMP double ptr [0x00986d98]
007b007b  dfe0                  FNSTSW AX
007b007d  f6c405                TEST AH,0x5
007b0080  7a0e                  JP 0x007b0090
007b0082  d90574749800          FLD float ptr [0x00987474]
007b0088  d99d14f8ffff          FSTP float ptr [EBP + 0xfffff814]
007b008e  eb0c                  JMP 0x007b009c
007b0090  d985acfdffff          FLD float ptr [EBP + 0xfffffdac]
007b0096  d99d14f8ffff          FSTP float ptr [EBP + 0xfffff814]
007b009c  d98514f8ffff          FLD float ptr [EBP + 0xfffff814]
007b00a2  dc1d506b9800          FCOMP double ptr [0x00986b50]
007b00a8  dfe0                  FNSTSW AX
007b00aa  f6c441                TEST AH,0x41
007b00ad  750a                  JNZ 0x007b00b9
007b00af  d9e8                  FLD1
007b00b1  d99d10f8ffff          FSTP float ptr [EBP + 0xfffff810]
007b00b7  eb39                  JMP 0x007b00f2
007b00b9  d985acfdffff          FLD float ptr [EBP + 0xfffffdac]
007b00bf  dc1d986d9800          FCOMP double ptr [0x00986d98]
007b00c5  dfe0                  FNSTSW AX
007b00c7  f6c405                TEST AH,0x5
007b00ca  7a0e                  JP 0x007b00da
007b00cc  d90574749800          FLD float ptr [0x00987474]
007b00d2  d99d0cf8ffff          FSTP float ptr [EBP + 0xfffff80c]
007b00d8  eb0c                  JMP 0x007b00e6
007b00da  d985acfdffff          FLD float ptr [EBP + 0xfffffdac]
007b00e0  d99d0cf8ffff          FSTP float ptr [EBP + 0xfffff80c]
007b00e6  d9850cf8ffff          FLD float ptr [EBP + 0xfffff80c]
007b00ec  d99d10f8ffff          FSTP float ptr [EBP + 0xfffff810]
007b00f2  d98510f8ffff          FLD float ptr [EBP + 0xfffff810]
007b00f8  d99dacfdffff          FSTP float ptr [EBP + 0xfffffdac]
007b00fe  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007b0103  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007b0106  898db4f9ffff          MOV dword ptr [EBP + 0xfffff9b4],ECX
007b010c  8b8db4f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9b4]
007b0112  e8e9f9f7ff            CALL 0x0072fb00
007b0117  85c0                  TEST EAX,EAX
007b0119  743f                  JZ 0x007b015a
007b011b  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007b0121  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007b0124  8985b0f9ffff          MOV dword ptr [EBP + 0xfffff9b0],EAX
007b012a  8b8db0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9b0]
007b0130  e8cbf9f7ff            CALL 0x0072fb00
007b0135  8985acf9ffff          MOV dword ptr [EBP + 0xfffff9ac],EAX
007b013b  8b8dacf9ffff          MOV ECX,dword ptr [EBP + 0xfffff9ac]
007b0141  8b5108                MOV EDX,dword ptr [ECX + 0x8]
007b0144  c1ea03                SHR EDX,0x3
007b0147  83e201                AND EDX,0x1
007b014a  740e                  JZ 0x007b015a
007b014c  d985acfdffff          FLD float ptr [EBP + 0xfffffdac]
007b0152  d9e0                  FCHS
007b0154  d99dacfdffff          FSTP float ptr [EBP + 0xfffffdac]
007b015a  51                    PUSH ECX
007b015b  d94508                FLD float ptr [EBP + 0x8]
007b015e  d91c24                FSTP float ptr [ESP]
007b0161  51                    PUSH ECX
007b0162  d985acfdffff          FLD float ptr [EBP + 0xfffffdac]
007b0168  d91c24                FSTP float ptr [ESP]
007b016b  51                    PUSH ECX
007b016c  d985a8fdffff          FLD float ptr [EBP + 0xfffffda8]
007b0172  d91c24                FSTP float ptr [ESP]
007b0175  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
007b0178  e8a3d30800            CALL 0x0083d520
007b017d  8b8da0f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a0]
007b0183  e878bdfeff            CALL 0x0079bf00
007b0188  85c0                  TEST EAX,EAX
007b018a  7562                  JNZ 0x007b01ee
007b018c  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007b0191  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007b0194  898da8f9ffff          MOV dword ptr [EBP + 0xfffff9a8],ECX
007b019a  8b8da8f9ffff          MOV ECX,dword ptr [EBP + 0xfffff9a8]
007b01a0  e85bfdf8ff            CALL 0x0073ff00
007b01a5  85c0                  TEST EAX,EAX
007b01a7  7445                  JZ 0x007b01ee
007b01a9  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007b01af  8b8274020000          MOV EAX,dword ptr [EDX + 0x274]
007b01b5  0fbf486e              MOVSX ECX,word ptr [EAX + 0x6e]
007b01b9  898da4f9ffff          MOV dword ptr [EBP + 0xfffff9a4],ECX
007b01bf  6a01                  PUSH 0x1
007b01c1  8b85a4f9ffff          MOV EAX,dword ptr [EBP + 0xfffff9a4]
007b01c7  99                    CDQ
007b01c8  2bc2                  SUB EAX,EDX
007b01ca  d1f8                  SAR EAX,0x1
007b01cc  50                    PUSH EAX
007b01cd  8b95a0f9ffff          MOV EDX,dword ptr [EBP + 0xfffff9a0]
007b01d3  8b8274020000          MOV EAX,dword ptr [EDX + 0x274]
007b01d9  0fbf406c              MOVSX EAX,word ptr [EAX + 0x6c]
007b01dd  99                    CDQ
007b01de  2bc2                  SUB EAX,EDX
007b01e0  d1f8                  SAR EAX,0x1
007b01e2  50                    PUSH EAX
007b01e3  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007b01e9  e892faf7ff            CALL 0x0072fc80
007b01ee  51                    PUSH ECX
007b01ef  d94508                FLD float ptr [EBP + 0x8]
007b01f2  d91c24                FSTP float ptr [ESP]
007b01f5  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
007b01f8  e853d40800            CALL 0x0083d650
007b01fd  c645fc00              MOV byte ptr [EBP + -0x4],0x0
007b0201  8d4dc4                LEA ECX,[EBP + -0x3c]
007b0204  e87735f8ff            CALL 0x00733780
007b0209  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
007b0210  8d4dd0                LEA ECX,[EBP + -0x30]
007b0213  e86835f8ff            CALL 0x00733780
007b0218  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007b021b  64890d00000000        MOV dword ptr FS:[0x0],ECX
007b0222  59                    POP ECX
007b0223  5e                    POP ESI
007b0224  8be5                  MOV ESP,EBP
007b0226  5d                    POP EBP
007b0227  c20400                RET 0x4
