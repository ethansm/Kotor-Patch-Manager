; FN 008ac330 FUN_008ac330
008ac330  55                    PUSH EBP
008ac331  8bec                  MOV EBP,ESP
008ac333  6aff                  PUSH -0x1
008ac335  689e899700            PUSH 0x97899e
008ac33a  64a100000000          MOV EAX,FS:[0x0]
008ac340  50                    PUSH EAX
008ac341  81ec38010000          SUB ESP,0x138
008ac347  a1201fa100            MOV EAX,[0x00a11f20]
008ac34c  33c5                  XOR EAX,EBP
008ac34e  50                    PUSH EAX
008ac34f  8d45f4                LEA EAX,[EBP + -0xc]
008ac352  64a300000000          MOV FS:[0x0],EAX
008ac358  898dc8feffff          MOV dword ptr [EBP + 0xfffffec8],ECX
008ac35e  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac364  83786800              CMP dword ptr [EAX + 0x68],0x0
008ac368  0f84500a0000          JZ 0x008acdbe
008ac36e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008ac371  8b515c                MOV EDX,dword ptr [ECX + 0x5c]
008ac374  8955ec                MOV dword ptr [EBP + -0x14],EDX
008ac377  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
008ac37e  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008ac381  6bc014                IMUL EAX,EAX,0x14
008ac384  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac38a  8b9000749a00          MOV EDX,dword ptr [EAX + 0x9a7400]
008ac390  899198500000          MOV dword ptr [ECX + 0x5098],EDX
008ac396  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac39c  83b898500000ff        CMP dword ptr [EAX + 0x5098],-0x1
008ac3a3  0f84c3090000          JZ 0x008acd6c
008ac3a9  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac3af  8b4968                MOV ECX,dword ptr [ECX + 0x68]
008ac3b2  e84914edff            CALL 0x0077d800
008ac3b7  8945b4                MOV dword ptr [EBP + -0x4c],EAX
008ac3ba  6a01                  PUSH 0x1
008ac3bc  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
008ac3bf  e8fcdecdff            CALL 0x0058a2c0
008ac3c4  8945d8                MOV dword ptr [EBP + -0x28],EAX
008ac3c7  6a00                  PUSH 0x0
008ac3c9  8d4dc8                LEA ECX,[EBP + -0x38]
008ac3cc  e86fe2f9ff            CALL 0x0084a640
008ac3d1  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008ac3d8  c745b800000000        MOV dword ptr [EBP + -0x48],0x0
008ac3df  c745c000000000        MOV dword ptr [EBP + -0x40],0x0
008ac3e6  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
008ac3e9  52                    PUSH EDX
008ac3ea  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac3f0  e8bb3d0000            CALL 0x008b01b0
008ac3f5  8945c0                MOV dword ptr [EBP + -0x40],EAX
008ac3f8  6a00                  PUSH 0x0
008ac3fa  6a00                  PUSH 0x0
008ac3fc  680000007f            PUSH 0x7f000000
008ac401  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008ac404  e8474e0000            CALL 0x008b1250
008ac409  6a00                  PUSH 0x0
008ac40b  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008ac40e  e8ed580000            CALL 0x008b1d00
008ac413  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
008ac416  c64058ff              MOV byte ptr [EAX + 0x58],0xff
008ac41a  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008ac41d  51                    PUSH ECX
008ac41e  8d4dc8                LEA ECX,[EBP + -0x38]
008ac421  e83a26f9ff            CALL 0x0083ea60
008ac426  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
008ac429  83c201                ADD EDX,0x1
008ac42c  8955b8                MOV dword ptr [EBP + -0x48],EDX
008ac42f  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008ac432  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac438  81bc819c5000000000007fCMP dword ptr [ECX + EAX*0x4 + 0x509c],0x7f000000
008ac443  0f84b4000000          JZ 0x008ac4fd
008ac449  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
008ac44c  52                    PUSH EDX
008ac44d  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac453  e8583d0000            CALL 0x008b01b0
008ac458  8945c0                MOV dword ptr [EBP + -0x40],EAX
008ac45b  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
008ac45e  8b88d0010000          MOV ECX,dword ptr [EAX + 0x1d0]
008ac464  898d68ffffff          MOV dword ptr [EBP + 0xffffff68],ECX
008ac46a  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008ac46d  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac473  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
008ac479  3b8c909c500000        CMP ECX,dword ptr [EAX + EDX*0x4 + 0x509c]
008ac480  7531                  JNZ 0x008ac4b3
008ac482  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008ac485  8b82ac030000          MOV EAX,dword ptr [EDX + 0x3ac]
008ac48b  d1e8                  SHR EAX,0x1
008ac48d  83e001                AND EAX,0x1
008ac490  0fb6c8                MOVZX ECX,AL
008ac493  83f901                CMP ECX,0x1
008ac496  751b                  JNZ 0x008ac4b3
008ac498  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008ac49b  8a82b0030000          MOV AL,byte ptr [EDX + 0x3b0]
008ac4a1  888567ffffff          MOV byte ptr [EBP + 0xffffff67],AL
008ac4a7  0fb68d67ffffff        MOVZX ECX,byte ptr [EBP + 0xffffff67]
008ac4ae  83f901                CMP ECX,0x1
008ac4b1  7407                  JZ 0x008ac4ba
008ac4b3  c745e801000000        MOV dword ptr [EBP + -0x18],0x1
008ac4ba  6a00                  PUSH 0x0
008ac4bc  6a01                  PUSH 0x1
008ac4be  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008ac4c1  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac4c7  8b8c909c500000        MOV ECX,dword ptr [EAX + EDX*0x4 + 0x509c]
008ac4ce  51                    PUSH ECX
008ac4cf  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008ac4d2  e8794d0000            CALL 0x008b1250
008ac4d7  6a01                  PUSH 0x1
008ac4d9  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008ac4dc  e81f580000            CALL 0x008b1d00
008ac4e1  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008ac4e4  c64258ff              MOV byte ptr [EDX + 0x58],0xff
008ac4e8  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
008ac4eb  50                    PUSH EAX
008ac4ec  8d4dc8                LEA ECX,[EBP + -0x38]
008ac4ef  e86c25f9ff            CALL 0x0083ea60
008ac4f4  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
008ac4f7  83c101                ADD ECX,0x1
008ac4fa  894db8                MOV dword ptr [EBP + -0x48],ECX
008ac4fd  c745e401000000        MOV dword ptr [EBP + -0x1c],0x1
008ac504  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008ac50a  83ba9850000020        CMP dword ptr [EDX + 0x5098],0x20
008ac511  0f858b000000          JNZ 0x008ac5a2
008ac517  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac51d  81b8a05000000000007f  CMP dword ptr [EAX + 0x50a0],0x7f000000
008ac527  7479                  JZ 0x008ac5a2
008ac529  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
008ac52f  8b5108                MOV EDX,dword ptr [ECX + 0x8]
008ac532  899560ffffff          MOV dword ptr [EBP + 0xffffff60],EDX
008ac538  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008ac53d  8b4808                MOV ECX,dword ptr [EAX + 0x8]
008ac540  898d5cffffff          MOV dword ptr [EBP + 0xffffff5c],ECX
008ac546  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008ac54c  8b82a0500000          MOV EAX,dword ptr [EDX + 0x50a0]
008ac552  50                    PUSH EAX
008ac553  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
008ac559  e85203c7ff            CALL 0x0051c8b0
008ac55e  50                    PUSH EAX
008ac55f  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
008ac565  e876fbc6ff            CALL 0x0051c0e0
008ac56a  8945e0                MOV dword ptr [EBP + -0x20],EAX
008ac56d  837de000              CMP dword ptr [EBP + -0x20],0x0
008ac571  742f                  JZ 0x008ac5a2
008ac573  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ac576  e8b5a8e2ff            CALL 0x006d6e30
008ac57b  898558ffffff          MOV dword ptr [EBP + 0xffffff58],EAX
008ac581  8b8d58ffffff          MOV ECX,dword ptr [EBP + 0xffffff58]
008ac587  8a511b                MOV DL,byte ptr [ECX + 0x1b]
008ac58a  889557ffffff          MOV byte ptr [EBP + 0xffffff57],DL
008ac590  0fb68557ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff57]
008ac597  33c9                  XOR ECX,ECX
008ac599  83f804                CMP EAX,0x4
008ac59c  0f95c1                SETNZ CL
008ac59f  894de4                MOV dword ptr [EBP + -0x1c],ECX
008ac5a2  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008ac5a8  8b8298500000          MOV EAX,dword ptr [EDX + 0x5098]
008ac5ae  8945bc                MOV dword ptr [EBP + -0x44],EAX
008ac5b1  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac5b7  81b99850000000000800  CMP dword ptr [ECX + 0x5098],0x80000
008ac5c1  751b                  JNZ 0x008ac5de
008ac5c3  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008ac5c9  8b8298500000          MOV EAX,dword ptr [EDX + 0x5098]
008ac5cf  83c820                OR EAX,0x20
008ac5d2  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac5d8  898198500000          MOV dword ptr [ECX + 0x5098],EAX
008ac5de  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008ac5e4  81ba9850000000000400  CMP dword ptr [EDX + 0x5098],0x40000
008ac5ee  751b                  JNZ 0x008ac60b
008ac5f0  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac5f6  8b8898500000          MOV ECX,dword ptr [EAX + 0x5098]
008ac5fc  83c910                OR ECX,0x10
008ac5ff  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008ac605  898a98500000          MOV dword ptr [EDX + 0x5098],ECX
008ac60b  837de400              CMP dword ptr [EBP + -0x1c],0x0
008ac60f  0f8420060000          JZ 0x008acc35
008ac615  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
008ac61c  eb09                  JMP 0x008ac627
008ac61e  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008ac621  83c001                ADD EAX,0x1
008ac624  8945dc                MOV dword ptr [EBP + -0x24],EAX
008ac627  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008ac62a  8b5110                MOV EDX,dword ptr [ECX + 0x10]
008ac62d  899550ffffff          MOV dword ptr [EBP + 0xffffff50],EDX
008ac633  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008ac636  3b8550ffffff          CMP EAX,dword ptr [EBP + 0xffffff50]
008ac63c  0f83f3050000          JNC 0x008acc35
008ac642  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008ac645  51                    PUSH ECX
008ac646  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008ac649  e8b29ad5ff            CALL 0x00606100
008ac64e  8945e0                MOV dword ptr [EBP + -0x20],EAX
008ac651  837de000              CMP dword ptr [EBP + -0x20],0x0
008ac655  0f84d5050000          JZ 0x008acc30
008ac65b  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008ac661  8b8298500000          MOV EAX,dword ptr [EDX + 0x5098]
008ac667  898548ffffff          MOV dword ptr [EBP + 0xffffff48],EAX
008ac66d  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ac670  e8bba7e2ff            CALL 0x006d6e30
008ac675  89854cffffff          MOV dword ptr [EBP + 0xffffff4c],EAX
008ac67b  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
008ac681  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008ac684  239548ffffff          AND EDX,dword ptr [EBP + 0xffffff48]
008ac68a  f7da                  NEG EDX
008ac68c  1bd2                  SBB EDX,EDX
008ac68e  f7da                  NEG EDX
008ac690  0f849a050000          JZ 0x008acc30
008ac696  c745b000000000        MOV dword ptr [EBP + -0x50],0x0
008ac69d  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ac6a0  e88ba7e2ff            CALL 0x006d6e30
008ac6a5  898544ffffff          MOV dword ptr [EBP + 0xffffff44],EAX
008ac6ab  8b8544ffffff          MOV EAX,dword ptr [EBP + 0xffffff44]
008ac6b1  8a88b4000000          MOV CL,byte ptr [EAX + 0xb4]
008ac6b7  884daf                MOV byte ptr [EBP + -0x51],CL
008ac6ba  0fb655af              MOVZX EDX,byte ptr [EBP + -0x51]
008ac6be  85d2                  TEST EDX,EDX
008ac6c0  0f8497000000          JZ 0x008ac75d
008ac6c6  8a45af                MOV AL,byte ptr [EBP + -0x51]
008ac6c9  8885c4feffff          MOV byte ptr [EBP + 0xfffffec4],AL
008ac6cf  80bdc4feffff01        CMP byte ptr [EBP + 0xfffffec4],0x1
008ac6d6  740b                  JZ 0x008ac6e3
008ac6d8  80bdc4feffff02        CMP byte ptr [EBP + 0xfffffec4],0x2
008ac6df  7440                  JZ 0x008ac721
008ac6e1  eb7a                  JMP 0x008ac75d
008ac6e3  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
008ac6e6  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
008ac6ec  899540ffffff          MOV dword ptr [EBP + 0xffffff40],EDX
008ac6f2  8b8540ffffff          MOV EAX,dword ptr [EBP + 0xffffff40]
008ac6f8  668b88e0000000        MOV CX,word ptr [EAX + 0xe0]
008ac6ff  66898d3effffff        MOV word ptr [EBP + 0xffffff3e],CX
008ac706  0fb7953effffff        MOVZX EDX,word ptr [EBP + 0xffffff3e]
008ac70d  0fb705bca89900        MOVZX EAX,word ptr [0x0099a8bc]
008ac714  3bd0                  CMP EDX,EAX
008ac716  7407                  JZ 0x008ac71f
008ac718  c745b001000000        MOV dword ptr [EBP + -0x50],0x1
008ac71f  eb3c                  JMP 0x008ac75d
008ac721  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
008ac724  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
008ac72a  899538ffffff          MOV dword ptr [EBP + 0xffffff38],EDX
008ac730  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
008ac736  668b88e0000000        MOV CX,word ptr [EAX + 0xe0]
008ac73d  66898d36ffffff        MOV word ptr [EBP + 0xffffff36],CX
008ac744  0fb79536ffffff        MOVZX EDX,word ptr [EBP + 0xffffff36]
008ac74b  0fb705b8a89900        MOVZX EAX,word ptr [0x0099a8b8]
008ac752  3bd0                  CMP EDX,EAX
008ac754  7407                  JZ 0x008ac75d
008ac756  c745b001000000        MOV dword ptr [EBP + -0x50],0x1
008ac75d  837db000              CMP dword ptr [EBP + -0x50],0x0
008ac761  0f85c9040000          JNZ 0x008acc30
008ac767  c645a300              MOV byte ptr [EBP + -0x5d],0x0
008ac76b  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac771  8b9198500000          MOV EDX,dword ptr [ECX + 0x5098]
008ac777  8955f0                MOV dword ptr [EBP + -0x10],EDX
008ac77a  6a00                  PUSH 0x0
008ac77c  6a00                  PUSH 0x0
008ac77e  6a00                  PUSH 0x0
008ac780  8d45f0                LEA EAX,[EBP + -0x10]
008ac783  50                    PUSH EAX
008ac784  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ac787  51                    PUSH ECX
008ac788  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
008ac78b  e8100fd1ff            CALL 0x005bd6a0
008ac790  0fb6d0                MOVZX EDX,AL
008ac793  85d2                  TEST EDX,EDX
008ac795  7504                  JNZ 0x008ac79b
008ac797  c645a302              MOV byte ptr [EBP + -0x5d],0x2
008ac79b  c745a801000000        MOV dword ptr [EBP + -0x58],0x1
008ac7a2  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008ac7a7  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008ac7aa  898d30ffffff          MOV dword ptr [EBP + 0xffffff30],ECX
008ac7b0  8b8d30ffffff          MOV ECX,dword ptr [EBP + 0xffffff30]
008ac7b6  e84533e8ff            CALL 0x0072fb00
008ac7bb  8945a4                MOV dword ptr [EBP + -0x5c],EAX
008ac7be  837da400              CMP dword ptr [EBP + -0x5c],0x0
008ac7c2  7462                  JZ 0x008ac826
008ac7c4  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
008ac7c7  8b421c                MOV EAX,dword ptr [EDX + 0x1c]
008ac7ca  83e001                AND EAX,0x1
008ac7cd  7457                  JZ 0x008ac826
008ac7cf  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac7d5  8b9198500000          MOV EDX,dword ptr [ECX + 0x5098]
008ac7db  52                    PUSH EDX
008ac7dc  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
008ac7df  50                    PUSH EAX
008ac7e0  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
008ac7e3  e85812d1ff            CALL 0x005bda40
008ac7e8  85c0                  TEST EAX,EAX
008ac7ea  7507                  JNZ 0x008ac7f3
008ac7ec  c745a800000000        MOV dword ptr [EBP + -0x58],0x0
008ac7f3  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac7f9  8b9198500000          MOV EDX,dword ptr [ECX + 0x5098]
008ac7ff  8955f0                MOV dword ptr [EBP + -0x10],EDX
008ac802  6a00                  PUSH 0x0
008ac804  6a00                  PUSH 0x0
008ac806  6a00                  PUSH 0x0
008ac808  8d45f0                LEA EAX,[EBP + -0x10]
008ac80b  50                    PUSH EAX
008ac80c  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ac80f  51                    PUSH ECX
008ac810  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
008ac813  e8880ed1ff            CALL 0x005bd6a0
008ac818  0fb6d0                MOVZX EDX,AL
008ac81b  85d2                  TEST EDX,EDX
008ac81d  7507                  JNZ 0x008ac826
008ac81f  c745a800000000        MOV dword ptr [EBP + -0x58],0x0
008ac826  837da800              CMP dword ptr [EBP + -0x58],0x0
008ac82a  0f8400040000          JZ 0x008acc30
008ac830  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ac833  e858aed5ff            CALL 0x00607690
008ac838  8945c4                MOV dword ptr [EBP + -0x3c],EAX
008ac83b  837dc400              CMP dword ptr [EBP + -0x3c],0x0
008ac83f  0f84eb030000          JZ 0x008acc30
008ac845  c7459c00000000        MOV dword ptr [EBP + -0x64],0x0
008ac84c  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac852  83b89850000010        CMP dword ptr [EAX + 0x5098],0x10
008ac859  7413                  JZ 0x008ac86e
008ac85b  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac861  83b99850000020        CMP dword ptr [ECX + 0x5098],0x20
008ac868  0f8555010000          JNZ 0x008ac9c3
008ac86e  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
008ac875  c7459800000000        MOV dword ptr [EBP + -0x68],0x0
008ac87c  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008ac882  83ba9850000010        CMP dword ptr [EDX + 0x5098],0x10
008ac889  7536                  JNZ 0x008ac8c1
008ac88b  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
008ac88e  894594                MOV dword ptr [EBP + -0x6c],EAX
008ac891  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
008ac897  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008ac89a  89952cffffff          MOV dword ptr [EBP + 0xffffff2c],EDX
008ac8a0  6a20                  PUSH 0x20
008ac8a2  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac8a8  8b4868                MOV ECX,dword ptr [EAX + 0x68]
008ac8ab  e8601decff            CALL 0x0076e610
008ac8b0  50                    PUSH EAX
008ac8b1  8b8d2cffffff          MOV ECX,dword ptr [EBP + 0xffffff2c]
008ac8b7  e8742ce9ff            CALL 0x0073f530
008ac8bc  894598                MOV dword ptr [EBP + -0x68],EAX
008ac8bf  eb34                  JMP 0x008ac8f5
008ac8c1  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
008ac8c7  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008ac8ca  899528ffffff          MOV dword ptr [EBP + 0xffffff28],EDX
008ac8d0  6a10                  PUSH 0x10
008ac8d2  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac8d8  8b4868                MOV ECX,dword ptr [EAX + 0x68]
008ac8db  e8301decff            CALL 0x0076e610
008ac8e0  50                    PUSH EAX
008ac8e1  8b8d28ffffff          MOV ECX,dword ptr [EBP + 0xffffff28]
008ac8e7  e8442ce9ff            CALL 0x0073f530
008ac8ec  894594                MOV dword ptr [EBP + -0x6c],EAX
008ac8ef  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
008ac8f2  894d98                MOV dword ptr [EBP + -0x68],ECX
008ac8f5  837d9400              CMP dword ptr [EBP + -0x6c],0x0
008ac8f9  7406                  JZ 0x008ac901
008ac8fb  837d9800              CMP dword ptr [EBP + -0x68],0x0
008ac8ff  750c                  JNZ 0x008ac90d
008ac901  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008ac908  e9a3000000            JMP 0x008ac9b0
008ac90d  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
008ac910  3b55c4                CMP EDX,dword ptr [EBP + -0x3c]
008ac913  7532                  JNZ 0x008ac947
008ac915  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ac918  e813a5e2ff            CALL 0x006d6e30
008ac91d  898524ffffff          MOV dword ptr [EBP + 0xffffff24],EAX
008ac923  8b8524ffffff          MOV EAX,dword ptr [EBP + 0xffffff24]
008ac929  8a481b                MOV CL,byte ptr [EAX + 0x1b]
008ac92c  888d23ffffff          MOV byte ptr [EBP + 0xffffff23],CL
008ac932  0fb69523ffffff        MOVZX EDX,byte ptr [EBP + 0xffffff23]
008ac939  83fa04                CMP EDX,0x4
008ac93c  7509                  JNZ 0x008ac947
008ac93e  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008ac945  eb69                  JMP 0x008ac9b0
008ac947  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
008ac94a  e8e1a4e2ff            CALL 0x006d6e30
008ac94f  89851cffffff          MOV dword ptr [EBP + 0xffffff1c],EAX
008ac955  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
008ac95b  8a4808                MOV CL,byte ptr [EAX + 0x8]
008ac95e  888d1bffffff          MOV byte ptr [EBP + 0xffffff1b],CL
008ac964  0fb6951bffffff        MOVZX EDX,byte ptr [EBP + 0xffffff1b]
008ac96b  895590                MOV dword ptr [EBP + -0x70],EDX
008ac96e  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008ac971  e8baa4e2ff            CALL 0x006d6e30
008ac976  898514ffffff          MOV dword ptr [EBP + 0xffffff14],EAX
008ac97c  8b8514ffffff          MOV EAX,dword ptr [EBP + 0xffffff14]
008ac982  8a4808                MOV CL,byte ptr [EAX + 0x8]
008ac985  888d13ffffff          MOV byte ptr [EBP + 0xffffff13],CL
008ac98b  0fb69513ffffff        MOVZX EDX,byte ptr [EBP + 0xffffff13]
008ac992  89558c                MOV dword ptr [EBP + -0x74],EDX
008ac995  8b4590                MOV EAX,dword ptr [EBP + -0x70]
008ac998  3b458c                CMP EAX,dword ptr [EBP + -0x74]
008ac99b  7513                  JNZ 0x008ac9b0
008ac99d  837d9002              CMP dword ptr [EBP + -0x70],0x2
008ac9a1  7406                  JZ 0x008ac9a9
008ac9a3  837d9004              CMP dword ptr [EBP + -0x70],0x4
008ac9a7  7507                  JNZ 0x008ac9b0
008ac9a9  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008ac9b0  837d9c00              CMP dword ptr [EBP + -0x64],0x0
008ac9b4  750b                  JNZ 0x008ac9c1
008ac9b6  c645a303              MOV byte ptr [EBP + -0x5d],0x3
008ac9ba  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008ac9c1  eb07                  JMP 0x008ac9ca
008ac9c3  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008ac9ca  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008ac9d0  8b9198500000          MOV EDX,dword ptr [ECX + 0x5098]
008ac9d6  81e200000400          AND EDX,0x40000
008ac9dc  7518                  JNZ 0x008ac9f6
008ac9de  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
008ac9e4  8b8898500000          MOV ECX,dword ptr [EAX + 0x5098]
008ac9ea  81e100000800          AND ECX,0x80000
008ac9f0  0f846c010000          JZ 0x008acb62
008ac9f6  c7459c00000000        MOV dword ptr [EBP + -0x64],0x0
008ac9fd  c7458400000000        MOV dword ptr [EBP + -0x7c],0x0
008aca04  c7458800000000        MOV dword ptr [EBP + -0x78],0x0
008aca0b  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008aca11  8b8298500000          MOV EAX,dword ptr [EDX + 0x5098]
008aca17  2500000400            AND EAX,0x40000
008aca1c  7439                  JZ 0x008aca57
008aca1e  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
008aca21  894d84                MOV dword ptr [EBP + -0x7c],ECX
008aca24  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
008aca2a  8b4204                MOV EAX,dword ptr [EDX + 0x4]
008aca2d  89850cffffff          MOV dword ptr [EBP + 0xffffff0c],EAX
008aca33  6800000800            PUSH 0x80000
008aca38  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008aca3e  8b4968                MOV ECX,dword ptr [ECX + 0x68]
008aca41  e8ca1becff            CALL 0x0076e610
008aca46  50                    PUSH EAX
008aca47  8b8d0cffffff          MOV ECX,dword ptr [EBP + 0xffffff0c]
008aca4d  e8de2ae9ff            CALL 0x0073f530
008aca52  894588                MOV dword ptr [EBP + -0x78],EAX
008aca55  eb37                  JMP 0x008aca8e
008aca57  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
008aca5d  8b4204                MOV EAX,dword ptr [EDX + 0x4]
008aca60  898508ffffff          MOV dword ptr [EBP + 0xffffff08],EAX
008aca66  6800000400            PUSH 0x40000
008aca6b  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008aca71  8b4968                MOV ECX,dword ptr [ECX + 0x68]
008aca74  e8971becff            CALL 0x0076e610
008aca79  50                    PUSH EAX
008aca7a  8b8d08ffffff          MOV ECX,dword ptr [EBP + 0xffffff08]
008aca80  e8ab2ae9ff            CALL 0x0073f530
008aca85  894584                MOV dword ptr [EBP + -0x7c],EAX
008aca88  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
008aca8b  895588                MOV dword ptr [EBP + -0x78],EDX
008aca8e  837d8400              CMP dword ptr [EBP + -0x7c],0x0
008aca92  7406                  JZ 0x008aca9a
008aca94  837d8800              CMP dword ptr [EBP + -0x78],0x0
008aca98  750c                  JNZ 0x008acaa6
008aca9a  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008acaa1  e9a9000000            JMP 0x008acb4f
008acaa6  8b4584                MOV EAX,dword ptr [EBP + -0x7c]
008acaa9  3b45c4                CMP EAX,dword ptr [EBP + -0x3c]
008acaac  7532                  JNZ 0x008acae0
008acaae  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008acab1  e87aa3e2ff            CALL 0x006d6e30
008acab6  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
008acabc  8b8d04ffffff          MOV ECX,dword ptr [EBP + 0xffffff04]
008acac2  8a511b                MOV DL,byte ptr [ECX + 0x1b]
008acac5  889503ffffff          MOV byte ptr [EBP + 0xffffff03],DL
008acacb  0fb68503ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff03]
008acad2  83f804                CMP EAX,0x4
008acad5  7509                  JNZ 0x008acae0
008acad7  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008acade  eb6f                  JMP 0x008acb4f
008acae0  8b4d84                MOV ECX,dword ptr [EBP + -0x7c]
008acae3  e848a3e2ff            CALL 0x006d6e30
008acae8  8985fcfeffff          MOV dword ptr [EBP + 0xfffffefc],EAX
008acaee  8b8dfcfeffff          MOV ECX,dword ptr [EBP + 0xfffffefc]
008acaf4  8a5108                MOV DL,byte ptr [ECX + 0x8]
008acaf7  8895fbfeffff          MOV byte ptr [EBP + 0xfffffefb],DL
008acafd  0fb685fbfeffff        MOVZX EAX,byte ptr [EBP + 0xfffffefb]
008acb04  894580                MOV dword ptr [EBP + -0x80],EAX
008acb07  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
008acb0a  e821a3e2ff            CALL 0x006d6e30
008acb0f  8985f4feffff          MOV dword ptr [EBP + 0xfffffef4],EAX
008acb15  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
008acb1b  8a5108                MOV DL,byte ptr [ECX + 0x8]
008acb1e  8895f3feffff          MOV byte ptr [EBP + 0xfffffef3],DL
008acb24  0fb685f3feffff        MOVZX EAX,byte ptr [EBP + 0xfffffef3]
008acb2b  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
008acb31  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
008acb34  3b8d7cffffff          CMP ECX,dword ptr [EBP + 0xffffff7c]
008acb3a  7513                  JNZ 0x008acb4f
008acb3c  837d8002              CMP dword ptr [EBP + -0x80],0x2
008acb40  7406                  JZ 0x008acb48
008acb42  837d8004              CMP dword ptr [EBP + -0x80],0x4
008acb46  7507                  JNZ 0x008acb4f
008acb48  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008acb4f  837d9c00              CMP dword ptr [EBP + -0x64],0x0
008acb53  750b                  JNZ 0x008acb60
008acb55  c645a303              MOV byte ptr [EBP + -0x5d],0x3
008acb59  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008acb60  eb07                  JMP 0x008acb69
008acb62  c7459c01000000        MOV dword ptr [EBP + -0x64],0x1
008acb69  837d9c00              CMP dword ptr [EBP + -0x64],0x0
008acb6d  0f84bd000000          JZ 0x008acc30
008acb73  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
008acb76  52                    PUSH EDX
008acb77  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008acb7d  e82e360000            CALL 0x008b01b0
008acb82  8945c0                MOV dword ptr [EBP + -0x40],EAX
008acb85  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
008acb88  8b88d0010000          MOV ECX,dword ptr [EAX + 0x1d0]
008acb8e  898decfeffff          MOV dword ptr [EBP + 0xfffffeec],ECX
008acb94  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
008acb97  8b4214                MOV EAX,dword ptr [EDX + 0x14]
008acb9a  8985e8feffff          MOV dword ptr [EBP + 0xfffffee8],EAX
008acba0  8b8decfeffff          MOV ECX,dword ptr [EBP + 0xfffffeec]
008acba6  3b8de8feffff          CMP ECX,dword ptr [EBP + 0xfffffee8]
008acbac  7533                  JNZ 0x008acbe1
008acbae  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008acbb1  8b82ac030000          MOV EAX,dword ptr [EDX + 0x3ac]
008acbb7  d1e8                  SHR EAX,0x1
008acbb9  83e001                AND EAX,0x1
008acbbc  0fb6c8                MOVZX ECX,AL
008acbbf  85c9                  TEST ECX,ECX
008acbc1  751e                  JNZ 0x008acbe1
008acbc3  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008acbc6  8a82b0030000          MOV AL,byte ptr [EDX + 0x3b0]
008acbcc  8885e7feffff          MOV byte ptr [EBP + 0xfffffee7],AL
008acbd2  0fb68de7feffff        MOVZX ECX,byte ptr [EBP + 0xfffffee7]
008acbd9  0fb655a3              MOVZX EDX,byte ptr [EBP + -0x5d]
008acbdd  3bca                  CMP ECX,EDX
008acbdf  7407                  JZ 0x008acbe8
008acbe1  c745e801000000        MOV dword ptr [EBP + -0x18],0x1
008acbe8  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
008acbeb  8b4814                MOV ECX,dword ptr [EAX + 0x14]
008acbee  898de0feffff          MOV dword ptr [EBP + 0xfffffee0],ECX
008acbf4  6a00                  PUSH 0x0
008acbf6  6a00                  PUSH 0x0
008acbf8  8b95e0feffff          MOV EDX,dword ptr [EBP + 0xfffffee0]
008acbfe  52                    PUSH EDX
008acbff  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008acc02  e849460000            CALL 0x008b1250
008acc07  0fb645a3              MOVZX EAX,byte ptr [EBP + -0x5d]
008acc0b  50                    PUSH EAX
008acc0c  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008acc0f  e8ec500000            CALL 0x008b1d00
008acc14  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008acc17  c64158ff              MOV byte ptr [ECX + 0x58],0xff
008acc1b  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008acc1e  52                    PUSH EDX
008acc1f  8d4dc8                LEA ECX,[EBP + -0x38]
008acc22  e8391ef9ff            CALL 0x0083ea60
008acc27  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
008acc2a  83c001                ADD EAX,0x1
008acc2d  8945b8                MOV dword ptr [EBP + -0x48],EAX
008acc30  e9e9f9ffff            JMP 0x008ac61e
008acc35  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008acc3b  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
008acc3e  899198500000          MOV dword ptr [ECX + 0x5098],EDX
008acc44  837de800              CMP dword ptr [EBP + -0x18],0x0
008acc48  7529                  JNZ 0x008acc73
008acc4a  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
008acc4d  8985dcfeffff          MOV dword ptr [EBP + 0xfffffedc],EAX
008acc53  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008acc59  8b91dc390000          MOV EDX,dword ptr [ECX + 0x39dc]
008acc5f  8995d8feffff          MOV dword ptr [EBP + 0xfffffed8],EDX
008acc65  8b85dcfeffff          MOV EAX,dword ptr [EBP + 0xfffffedc]
008acc6b  3b85d8feffff          CMP EAX,dword ptr [EBP + 0xfffffed8]
008acc71  742e                  JZ 0x008acca1
008acc73  6a00                  PUSH 0x0
008acc75  6a01                  PUSH 0x1
008acc77  6a00                  PUSH 0x0
008acc79  8d4dc8                LEA ECX,[EBP + -0x38]
008acc7c  51                    PUSH ECX
008acc7d  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008acc83  81c12c370000          ADD ECX,0x372c
008acc89  e8a22fb7ff            CALL 0x0041fc30
008acc8e  6a00                  PUSH 0x0
008acc90  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008acc96  81c138380000          ADD ECX,0x3838
008acc9c  e81ffbb6ff            CALL 0x0041c7c0
008acca1  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
008acca7  8b4268                MOV EAX,dword ptr [EDX + 0x68]
008accaa  8b8810030000          MOV ECX,dword ptr [EAX + 0x310]
008accb0  898dd4feffff          MOV dword ptr [EBP + 0xfffffed4],ECX
008accb6  8b95d4feffff          MOV EDX,dword ptr [EBP + 0xfffffed4]
008accbc  668b4224              MOV AX,word ptr [EDX + 0x24]
008accc0  668985d2feffff        MOV word ptr [EBP + 0xfffffed2],AX
008accc7  0fb78dd2feffff        MOVZX ECX,word ptr [EBP + 0xfffffed2]
008accce  0fb715b8a89900        MOVZX EDX,word ptr [0x0099a8b8]
008accd5  3bca                  CMP ECX,EDX
008accd7  7511                  JNZ 0x008accea
008accd9  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008accdc  6bc014                IMUL EAX,EAX,0x14
008accdf  8b8810749a00          MOV ECX,dword ptr [EAX + 0x9a7410]
008acce5  894dd4                MOV dword ptr [EBP + -0x2c],ECX
008acce8  eb0f                  JMP 0x008accf9
008accea  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008acced  6bd214                IMUL EDX,EDX,0x14
008accf0  8b820c749a00          MOV EAX,dword ptr [EDX + 0x9a740c]
008accf6  8945d4                MOV dword ptr [EBP + -0x2c],EAX
008accf9  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
008accff  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008acd02  8995ccfeffff          MOV dword ptr [EBP + 0xfffffecc],EDX
008acd08  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
008acd0b  50                    PUSH EAX
008acd0c  8d8d74ffffff          LEA ECX,[EBP + 0xffffff74]
008acd12  51                    PUSH ECX
008acd13  8b8dccfeffff          MOV ECX,dword ptr [EBP + 0xfffffecc]
008acd19  e8d23be9ff            CALL 0x007408f0
008acd1e  8985c0feffff          MOV dword ptr [EBP + 0xfffffec0],EAX
008acd24  8b95c0feffff          MOV EDX,dword ptr [EBP + 0xfffffec0]
008acd2a  8995bcfeffff          MOV dword ptr [EBP + 0xfffffebc],EDX
008acd30  c645fc01              MOV byte ptr [EBP + -0x4],0x1
008acd34  8b85bcfeffff          MOV EAX,dword ptr [EBP + 0xfffffebc]
008acd3a  50                    PUSH EAX
008acd3b  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008acd41  81c16c300000          ADD ECX,0x306c
008acd47  e8e4a0b6ff            CALL 0x00416e30
008acd4c  c645fc00              MOV byte ptr [EBP + -0x4],0x0
008acd50  8d8d74ffffff          LEA ECX,[EBP + 0xffffff74]
008acd56  e8256ae8ff            CALL 0x00733780
008acd5b  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008acd62  8d4dc8                LEA ECX,[EBP + -0x38]
008acd65  e84610daff            CALL 0x0064ddb0
008acd6a  eb52                  JMP 0x008acdbe
008acd6c  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008acd72  81c12c370000          ADD ECX,0x372c
008acd78  e81305b7ff            CALL 0x0041d290
008acd7d  6825699800            PUSH 0x986925
008acd82  8d8d6cffffff          LEA ECX,[EBP + 0xffffff6c]
008acd88  e8e367e8ff            CALL 0x00733570
008acd8d  c745fc02000000        MOV dword ptr [EBP + -0x4],0x2
008acd94  8d8d6cffffff          LEA ECX,[EBP + 0xffffff6c]
008acd9a  51                    PUSH ECX
008acd9b  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
008acda1  81c16c300000          ADD ECX,0x306c
008acda7  e884a0b6ff            CALL 0x00416e30
008acdac  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008acdb3  8d8d6cffffff          LEA ECX,[EBP + 0xffffff6c]
008acdb9  e8c269e8ff            CALL 0x00733780
008acdbe  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008acdc1  64890d00000000        MOV dword ptr FS:[0x0],ECX
008acdc8  59                    POP ECX
008acdc9  8be5                  MOV ESP,EBP
008acdcb  5d                    POP EBP
008acdcc  c20400                RET 0x4
