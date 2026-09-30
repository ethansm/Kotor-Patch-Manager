; FN 007aa460 FUN_007aa460
007aa460  55                    PUSH EBP
007aa461  8bec                  MOV EBP,ESP
007aa463  81ec84000000          SUB ESP,0x84
007aa469  894d80                MOV dword ptr [EBP + -0x80],ECX
007aa46c  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
007aa473  eb09                  JMP 0x007aa47e
007aa475  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007aa478  83c001                ADD EAX,0x1
007aa47b  8945fc                MOV dword ptr [EBP + -0x4],EAX
007aa47e  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
007aa481  8b9114020000          MOV EDX,dword ptr [ECX + 0x214]
007aa487  895598                MOV dword ptr [EBP + -0x68],EDX
007aa48a  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007aa48d  3b4598                CMP EAX,dword ptr [EBP + -0x68]
007aa490  0f8de8020000          JGE 0x007aa77e
007aa496  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
007aa499  8b9110020000          MOV EDX,dword ptr [ECX + 0x210]
007aa49f  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007aa4a2  8d0c82                LEA ECX,[EDX + EAX*0x4]
007aa4a5  894d94                MOV dword ptr [EBP + -0x6c],ECX
007aa4a8  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
007aa4ae  8b4204                MOV EAX,dword ptr [EDX + 0x4]
007aa4b1  894590                MOV dword ptr [EBP + -0x70],EAX
007aa4b4  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
007aa4b7  8b11                  MOV EDX,dword ptr [ECX]
007aa4b9  52                    PUSH EDX
007aa4ba  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
007aa4bd  e80e50f9ff            CALL 0x0073f4d0
007aa4c2  8945f8                MOV dword ptr [EBP + -0x8],EAX
007aa4c5  837df800              CMP dword ptr [EBP + -0x8],0x0
007aa4c9  0f84aa020000          JZ 0x007aa779
007aa4cf  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
007aa4d2  8b10                  MOV EDX,dword ptr [EAX]
007aa4d4  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007aa4d7  8b420c                MOV EAX,dword ptr [EDX + 0xc]
007aa4da  ffd0                  CALL EAX
007aa4dc  8945f4                MOV dword ptr [EBP + -0xc],EAX
007aa4df  837df400              CMP dword ptr [EBP + -0xc],0x0
007aa4e3  0f8490020000          JZ 0x007aa779
007aa4e9  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa4ec  8a9190000000          MOV DL,byte ptr [ECX + 0x90]
007aa4f2  88558f                MOV byte ptr [EBP + -0x71],DL
007aa4f5  0fb6458f              MOVZX EAX,byte ptr [EBP + -0x71]
007aa4f9  85c0                  TEST EAX,EAX
007aa4fb  0f8478020000          JZ 0x007aa779
007aa501  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa504  0fb65108              MOVZX EDX,byte ptr [ECX + 0x8]
007aa508  89957cffffff          MOV dword ptr [EBP + 0xffffff7c],EDX
007aa50e  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
007aa514  83e805                SUB EAX,0x5
007aa517  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
007aa51d  83bd7cffffff05        CMP dword ptr [EBP + 0xffffff7c],0x5
007aa524  0f874f020000          JA 0x007aa779
007aa52a  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
007aa530  ff248d84a77a00        JMP dword ptr [ECX*0x4 + 0x7aa784]
007aa537  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007aa53a  8b02                  MOV EAX,dword ptr [EDX]
007aa53c  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa53f  8b502c                MOV EDX,dword ptr [EAX + 0x2c]
007aa542  ffd2                  CALL EDX
007aa544  8945f0                MOV dword ptr [EBP + -0x10],EAX
007aa547  837d0800              CMP dword ptr [EBP + 0x8],0x0
007aa54b  0f841b010000          JZ 0x007aa66c
007aa551  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007aa556  8b4804                MOV ECX,dword ptr [EAX + 0x4]
007aa559  894d88                MOV dword ptr [EBP + -0x78],ECX
007aa55c  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
007aa55f  e8ec4ef9ff            CALL 0x0073f450
007aa564  3945f0                CMP dword ptr [EBP + -0x10],EAX
007aa567  7532                  JNZ 0x007aa59b
007aa569  d9e8                  FLD1
007aa56b  d95de4                FSTP float ptr [EBP + -0x1c]
007aa56e  d9e8                  FLD1
007aa570  d95de8                FSTP float ptr [EBP + -0x18]
007aa573  d9e8                  FLD1
007aa575  d95dec                FSTP float ptr [EBP + -0x14]
007aa578  6a01                  PUSH 0x1
007aa57a  51                    PUSH ECX
007aa57b  d9ee                  FLDZ
007aa57d  d91c24                FSTP float ptr [ESP]
007aa580  6a02                  PUSH 0x2
007aa582  8d55e4                LEA EDX,[EBP + -0x1c]
007aa585  52                    PUSH EDX
007aa586  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007aa589  8b10                  MOV EDX,dword ptr [EAX]
007aa58b  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa58e  8b82a4000000          MOV EAX,dword ptr [EDX + 0xa4]
007aa594  ffd0                  CALL EAX
007aa596  e9cf000000            JMP 0x007aa66a
007aa59b  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007aa59e  8a91f4020000          MOV DL,byte ptr [ECX + 0x2f4]
007aa5a4  885587                MOV byte ptr [EBP + -0x79],DL
007aa5a7  0fb64587              MOVZX EAX,byte ptr [EBP + -0x79]
007aa5ab  83f801                CMP EAX,0x1
007aa5ae  753a                  JNZ 0x007aa5ea
007aa5b0  d905606b9800          FLD float ptr [0x00986b60]
007aa5b6  d95dd8                FSTP float ptr [EBP + -0x28]
007aa5b9  d9e8                  FLD1
007aa5bb  d95ddc                FSTP float ptr [EBP + -0x24]
007aa5be  d905606b9800          FLD float ptr [0x00986b60]
007aa5c4  d95de0                FSTP float ptr [EBP + -0x20]
007aa5c7  6a01                  PUSH 0x1
007aa5c9  51                    PUSH ECX
007aa5ca  d9ee                  FLDZ
007aa5cc  d91c24                FSTP float ptr [ESP]
007aa5cf  6a02                  PUSH 0x2
007aa5d1  8d4dd8                LEA ECX,[EBP + -0x28]
007aa5d4  51                    PUSH ECX
007aa5d5  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007aa5d8  8b02                  MOV EAX,dword ptr [EDX]
007aa5da  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa5dd  8b90a4000000          MOV EDX,dword ptr [EAX + 0xa4]
007aa5e3  ffd2                  CALL EDX
007aa5e5  e980000000            JMP 0x007aa66a
007aa5ea  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
007aa5ed  8a88f4020000          MOV CL,byte ptr [EAX + 0x2f4]
007aa5f3  884d86                MOV byte ptr [EBP + -0x7a],CL
007aa5f6  0fb65586              MOVZX EDX,byte ptr [EBP + -0x7a]
007aa5fa  85d2                  TEST EDX,EDX
007aa5fc  7537                  JNZ 0x007aa635
007aa5fe  d905b0759800          FLD float ptr [0x009875b0]
007aa604  d95dcc                FSTP float ptr [EBP + -0x34]
007aa607  d905b0759800          FLD float ptr [0x009875b0]
007aa60d  d95dd0                FSTP float ptr [EBP + -0x30]
007aa610  d9e8                  FLD1
007aa612  d95dd4                FSTP float ptr [EBP + -0x2c]
007aa615  6a01                  PUSH 0x1
007aa617  51                    PUSH ECX
007aa618  d9ee                  FLDZ
007aa61a  d91c24                FSTP float ptr [ESP]
007aa61d  6a02                  PUSH 0x2
007aa61f  8d45cc                LEA EAX,[EBP + -0x34]
007aa622  50                    PUSH EAX
007aa623  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa626  8b11                  MOV EDX,dword ptr [ECX]
007aa628  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa62b  8b82a4000000          MOV EAX,dword ptr [EDX + 0xa4]
007aa631  ffd0                  CALL EAX
007aa633  eb35                  JMP 0x007aa66a
007aa635  d9e8                  FLD1
007aa637  d95dc0                FSTP float ptr [EBP + -0x40]
007aa63a  d905fc769800          FLD float ptr [0x009876fc]
007aa640  d95dc4                FSTP float ptr [EBP + -0x3c]
007aa643  d905fc769800          FLD float ptr [0x009876fc]
007aa649  d95dc8                FSTP float ptr [EBP + -0x38]
007aa64c  6a01                  PUSH 0x1
007aa64e  51                    PUSH ECX
007aa64f  d9ee                  FLDZ
007aa651  d91c24                FSTP float ptr [ESP]
007aa654  6a02                  PUSH 0x2
007aa656  8d4dc0                LEA ECX,[EBP + -0x40]
007aa659  51                    PUSH ECX
007aa65a  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007aa65d  8b02                  MOV EAX,dword ptr [EDX]
007aa65f  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa662  8b90a4000000          MOV EDX,dword ptr [EAX + 0xa4]
007aa668  ffd2                  CALL EDX
007aa66a  eb20                  JMP 0x007aa68c
007aa66c  51                    PUSH ECX
007aa66d  d9ee                  FLDZ
007aa66f  d91c24                FSTP float ptr [ESP]
007aa672  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007aa675  e8f640c6ff            CALL 0x0040e770
007aa67a  6a02                  PUSH 0x2
007aa67c  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007aa67f  8b10                  MOV EDX,dword ptr [EAX]
007aa681  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa684  8b82a8000000          MOV EAX,dword ptr [EDX + 0xa8]
007aa68a  ffd0                  CALL EAX
007aa68c  e9e8000000            JMP 0x007aa779
007aa691  837d0800              CMP dword ptr [EBP + 0x8],0x0
007aa695  7433                  JZ 0x007aa6ca
007aa697  d905606b9800          FLD float ptr [0x00986b60]
007aa69d  d95db4                FSTP float ptr [EBP + -0x4c]
007aa6a0  d9e8                  FLD1
007aa6a2  d95db8                FSTP float ptr [EBP + -0x48]
007aa6a5  d9e8                  FLD1
007aa6a7  d95dbc                FSTP float ptr [EBP + -0x44]
007aa6aa  6a01                  PUSH 0x1
007aa6ac  51                    PUSH ECX
007aa6ad  d9ee                  FLDZ
007aa6af  d91c24                FSTP float ptr [ESP]
007aa6b2  6a02                  PUSH 0x2
007aa6b4  8d4db4                LEA ECX,[EBP + -0x4c]
007aa6b7  51                    PUSH ECX
007aa6b8  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007aa6bb  8b02                  MOV EAX,dword ptr [EDX]
007aa6bd  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa6c0  8b90a4000000          MOV EDX,dword ptr [EAX + 0xa4]
007aa6c6  ffd2                  CALL EDX
007aa6c8  eb12                  JMP 0x007aa6dc
007aa6ca  6a02                  PUSH 0x2
007aa6cc  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007aa6cf  8b10                  MOV EDX,dword ptr [EAX]
007aa6d1  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa6d4  8b82a8000000          MOV EAX,dword ptr [EDX + 0xa8]
007aa6da  ffd0                  CALL EAX
007aa6dc  e998000000            JMP 0x007aa779
007aa6e1  837d0800              CMP dword ptr [EBP + 0x8],0x0
007aa6e5  7433                  JZ 0x007aa71a
007aa6e7  d905606b9800          FLD float ptr [0x00986b60]
007aa6ed  d95da8                FSTP float ptr [EBP + -0x58]
007aa6f0  d9e8                  FLD1
007aa6f2  d95dac                FSTP float ptr [EBP + -0x54]
007aa6f5  d9e8                  FLD1
007aa6f7  d95db0                FSTP float ptr [EBP + -0x50]
007aa6fa  6a01                  PUSH 0x1
007aa6fc  51                    PUSH ECX
007aa6fd  d9ee                  FLDZ
007aa6ff  d91c24                FSTP float ptr [ESP]
007aa702  6a02                  PUSH 0x2
007aa704  8d4da8                LEA ECX,[EBP + -0x58]
007aa707  51                    PUSH ECX
007aa708  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007aa70b  8b02                  MOV EAX,dword ptr [EDX]
007aa70d  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa710  8b90a4000000          MOV EDX,dword ptr [EAX + 0xa4]
007aa716  ffd2                  CALL EDX
007aa718  eb12                  JMP 0x007aa72c
007aa71a  6a02                  PUSH 0x2
007aa71c  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007aa71f  8b10                  MOV EDX,dword ptr [EAX]
007aa721  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa724  8b82a8000000          MOV EAX,dword ptr [EDX + 0xa8]
007aa72a  ffd0                  CALL EAX
007aa72c  eb4b                  JMP 0x007aa779
007aa72e  837d0800              CMP dword ptr [EBP + 0x8],0x0
007aa732  7433                  JZ 0x007aa767
007aa734  d905606b9800          FLD float ptr [0x00986b60]
007aa73a  d95d9c                FSTP float ptr [EBP + -0x64]
007aa73d  d9e8                  FLD1
007aa73f  d95da0                FSTP float ptr [EBP + -0x60]
007aa742  d9e8                  FLD1
007aa744  d95da4                FSTP float ptr [EBP + -0x5c]
007aa747  6a01                  PUSH 0x1
007aa749  51                    PUSH ECX
007aa74a  d9ee                  FLDZ
007aa74c  d91c24                FSTP float ptr [ESP]
007aa74f  6a02                  PUSH 0x2
007aa751  8d4d9c                LEA ECX,[EBP + -0x64]
007aa754  51                    PUSH ECX
007aa755  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007aa758  8b02                  MOV EAX,dword ptr [EDX]
007aa75a  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa75d  8b90a4000000          MOV EDX,dword ptr [EAX + 0xa4]
007aa763  ffd2                  CALL EDX
007aa765  eb12                  JMP 0x007aa779
007aa767  6a02                  PUSH 0x2
007aa769  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007aa76c  8b10                  MOV EDX,dword ptr [EAX]
007aa76e  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007aa771  8b82a8000000          MOV EAX,dword ptr [EDX + 0xa8]
007aa777  ffd0                  CALL EAX
007aa779  e9f7fcffff            JMP 0x007aa475
007aa77e  8be5                  MOV ESP,EBP
007aa780  5d                    POP EBP
007aa781  c20400                RET 0x4
