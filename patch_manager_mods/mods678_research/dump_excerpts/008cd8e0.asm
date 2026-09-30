; FN 008cd8e0 FUN_008cd8e0
008cd8e0  55                    PUSH EBP
008cd8e1  8bec                  MOV EBP,ESP
008cd8e3  6aff                  PUSH -0x1
008cd8e5  68a0b69700            PUSH 0x97b6a0
008cd8ea  64a100000000          MOV EAX,FS:[0x0]
008cd8f0  50                    PUSH EAX
008cd8f1  81ec80000000          SUB ESP,0x80
008cd8f7  a1201fa100            MOV EAX,[0x00a11f20]
008cd8fc  33c5                  XOR EAX,EBP
008cd8fe  8945e0                MOV dword ptr [EBP + -0x20],EAX
008cd901  50                    PUSH EAX
008cd902  8d45f4                LEA EAX,[EBP + -0xc]
008cd905  64a300000000          MOV FS:[0x0],EAX
008cd90b  894d80                MOV dword ptr [EBP + -0x80],ECX
008cd90e  8d4dd0                LEA ECX,[EBP + -0x30]
008cd911  e8da2ee4ff            CALL 0x007107f0
008cd916  8d4dec                LEA ECX,[EBP + -0x14]
008cd919  e8225ce6ff            CALL 0x00733540
008cd91e  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008cd925  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
008cd92c  8b4580                MOV EAX,dword ptr [EBP + -0x80]
008cd92f  0fb6882c3d0000        MOVZX ECX,byte ptr [EAX + 0x3d2c]
008cd936  898d7cffffff          MOV dword ptr [EBP + 0xffffff7c],ECX
008cd93c  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
008cd942  83ea01                SUB EDX,0x1
008cd945  89957cffffff          MOV dword ptr [EBP + 0xffffff7c],EDX
008cd94b  83bd7cffffff03        CMP dword ptr [EBP + 0xffffff7c],0x3
008cd952  773b                  JA 0x008cd98f
008cd954  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
008cd95a  ff2485ecda8c00        JMP dword ptr [EAX*0x4 + 0x8cdaec]
008cd961  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
008cd964  81c1f8170000          ADD ECX,0x17f8
008cd96a  894dcc                MOV dword ptr [EBP + -0x34],ECX
008cd96d  eb20                  JMP 0x008cd98f
008cd96f  8b5580                MOV EDX,dword ptr [EBP + -0x80]
008cd972  81c2a81f0000          ADD EDX,0x1fa8
008cd978  8955cc                MOV dword ptr [EBP + -0x34],EDX
008cd97b  eb12                  JMP 0x008cd98f
008cd97d  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
008cd984  8b4580                MOV EAX,dword ptr [EBP + -0x80]
008cd987  05a81f0000            ADD EAX,0x1fa8
008cd98c  8945cc                MOV dword ptr [EBP + -0x34],EAX
008cd98f  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
008cd992  e899d8ffff            CALL 0x008cb230
008cd997  0fb6c8                MOVZX ECX,AL
008cd99a  034de4                ADD ECX,dword ptr [EBP + -0x1c]
008cd99d  894dc8                MOV dword ptr [EBP + -0x38],ECX
008cd9a0  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
008cd9a3  8955e8                MOV dword ptr [EBP + -0x18],EDX
008cd9a6  eb09                  JMP 0x008cd9b1
008cd9a8  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008cd9ab  83c001                ADD EAX,0x1
008cd9ae  8945e8                MOV dword ptr [EBP + -0x18],EAX
008cd9b1  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008cd9b4  3b4dc8                CMP ECX,dword ptr [EBP + -0x38]
008cd9b7  0f8dc2000000          JGE 0x008cda7f
008cd9bd  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008cd9c0  8b4580                MOV EAX,dword ptr [EBP + -0x80]
008cd9c3  83bc90543d000000      CMP dword ptr [EAX + EDX*0x4 + 0x3d54],0x0
008cd9cb  7468                  JZ 0x008cda35
008cd9cd  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008cd9d0  8b5580                MOV EDX,dword ptr [EBP + -0x80]
008cd9d3  8b848a543d0000        MOV EAX,dword ptr [EDX + ECX*0x4 + 0x3d54]
008cd9da  8b480c                MOV ECX,dword ptr [EAX + 0xc]
008cd9dd  894d88                MOV dword ptr [EBP + -0x78],ECX
008cd9e0  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
008cd9e6  895584                MOV dword ptr [EBP + -0x7c],EDX
008cd9e9  8b4588                MOV EAX,dword ptr [EBP + -0x78]
008cd9ec  50                    PUSH EAX
008cd9ed  8b4d84                MOV ECX,dword ptr [EBP + -0x7c]
008cd9f0  8b4934                MOV ECX,dword ptr [ECX + 0x34]
008cd9f3  e80878e0ff            CALL 0x006d5200
008cd9f8  8945c4                MOV dword ptr [EBP + -0x3c],EAX
008cd9fb  8d55b4                LEA EDX,[EBP + -0x4c]
008cd9fe  52                    PUSH EDX
008cd9ff  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008cda02  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
008cda05  8b8c81543d0000        MOV ECX,dword ptr [ECX + EAX*0x4 + 0x3d54]
008cda0c  e8af82d4ff            CALL 0x00615cc0
008cda11  50                    PUSH EAX
008cda12  8d55a4                LEA EDX,[EBP + -0x5c]
008cda15  52                    PUSH EDX
008cda16  8d4dd0                LEA ECX,[EBP + -0x30]
008cda19  e88232e4ff            CALL 0x00710ca0
008cda1e  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008cda21  69c048010000          IMUL EAX,EAX,0x148
008cda27  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
008cda2a  d9e8                  FLD1
008cda2c  d99c0184000000        FSTP float ptr [ECX + EAX*0x1 + 0x84]
008cda33  eb2a                  JMP 0x008cda5f
008cda35  8b5580                MOV EDX,dword ptr [EBP + -0x80]
008cda38  0fb6822c3d0000        MOVZX EAX,byte ptr [EDX + 0x3d2c]
008cda3f  83e801                SUB EAX,0x1
008cda42  6bc048                IMUL EAX,EAX,0x48
008cda45  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008cda48  6bc90c                IMUL ECX,ECX,0xc
008cda4b  8b9408ec849a00        MOV EDX,dword ptr [EAX + ECX*0x1 + 0x9a84ec]
008cda52  52                    PUSH EDX
008cda53  8d4594                LEA EAX,[EBP + -0x6c]
008cda56  50                    PUSH EAX
008cda57  8d4dd0                LEA ECX,[EBP + -0x30]
008cda5a  e85134e4ff            CALL 0x00710eb0
008cda5f  6a00                  PUSH 0x0
008cda61  8d4dd0                LEA ECX,[EBP + -0x30]
008cda64  51                    PUSH ECX
008cda65  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008cda68  69d248010000          IMUL EDX,EDX,0x148
008cda6e  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
008cda71  8d4c1074              LEA ECX,[EAX + EDX*0x1 + 0x74]
008cda75  e8c66db4ff            CALL 0x00414840
008cda7a  e929ffffff            JMP 0x008cd9a8
008cda7f  8d4d8c                LEA ECX,[EBP + -0x74]
008cda82  51                    PUSH ECX
008cda83  8b5580                MOV EDX,dword ptr [EBP + -0x80]
008cda86  8b8a343d0000          MOV ECX,dword ptr [EDX + 0x3d34]
008cda8c  e8ff9cd3ff            CALL 0x00607790
008cda91  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
008cda97  8b8578ffffff          MOV EAX,dword ptr [EBP + 0xffffff78]
008cda9d  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
008cdaa3  c645fc01              MOV byte ptr [EBP + -0x4],0x1
008cdaa7  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
008cdaad  51                    PUSH ECX
008cdaae  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
008cdab1  e8baf2ffff            CALL 0x008ccd70
008cdab6  c645fc00              MOV byte ptr [EBP + -0x4],0x0
008cdaba  8d4d8c                LEA ECX,[EBP + -0x74]
008cdabd  e8be5ce6ff            CALL 0x00733780
008cdac2  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008cdac9  8d4dec                LEA ECX,[EBP + -0x14]
008cdacc  e8af5ce6ff            CALL 0x00733780
008cdad1  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008cdad4  64890d00000000        MOV dword ptr FS:[0x0],ECX
008cdadb  59                    POP ECX
008cdadc  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008cdadf  33cd                  XOR ECX,EBP
008cdae1  e808bc0400            CALL 0x009196ee
008cdae6  8be5                  MOV ESP,EBP
008cdae8  5d                    POP EBP
008cdae9  c3                    RET
