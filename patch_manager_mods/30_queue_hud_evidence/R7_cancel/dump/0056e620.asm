; FN 0056e620 FUN_0056e620
0056e620  55                    PUSH EBP
0056e621  8bec                  MOV EBP,ESP
0056e623  81ecb0000000          SUB ESP,0xb0
0056e629  898d50ffffff          MOV dword ptr [EBP + 0xffffff50],ECX
0056e62f  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e632  83787000              CMP dword ptr [EAX + 0x70],0x0
0056e636  7507                  JNZ 0x0056e63f
0056e638  33c0                  XOR EAX,EAX
0056e63a  e986080000            JMP 0x0056eec5
0056e63f  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0056e645  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0056e648  8955dc                MOV dword ptr [EBP + -0x24],EDX
0056e64b  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0056e64e  e88ddcfaff            CALL 0x0051c2e0
0056e653  8945f8                MOV dword ptr [EBP + -0x8],EAX
0056e656  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0056e65b  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0056e65e  894dd8                MOV dword ptr [EBP + -0x28],ECX
0056e661  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
0056e664  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0056e667  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0056e66a  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e670  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0056e673  52                    PUSH EDX
0056e674  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0056e677  e8b40cfcff            CALL 0x0052f330
0056e67c  8945fc                MOV dword ptr [EBP + -0x4],EAX
0056e67f  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0056e683  0f8404010000          JZ 0x0056e78d
0056e689  837df800              CMP dword ptr [EBP + -0x8],0x0
0056e68d  0f84fa000000          JZ 0x0056e78d
0056e693  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e696  833808                CMP dword ptr [EAX],0x8
0056e699  754c                  JNZ 0x0056e6e7
0056e69b  6a04                  PUSH 0x4
0056e69d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056e6a0  83c138                ADD ECX,0x38
0056e6a3  51                    PUSH ECX
0056e6a4  8d55f4                LEA EDX,[EBP + -0xc]
0056e6a7  52                    PUSH EDX
0056e6a8  e8f3f43a00            CALL 0x0091dba0
0056e6ad  83c40c                ADD ESP,0xc
0056e6b0  6a04                  PUSH 0x4
0056e6b2  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e6b5  83c03c                ADD EAX,0x3c
0056e6b8  50                    PUSH EAX
0056e6b9  8d4df0                LEA ECX,[EBP + -0x10]
0056e6bc  51                    PUSH ECX
0056e6bd  e8def43a00            CALL 0x0091dba0
0056e6c2  83c40c                ADD ESP,0xc
0056e6c5  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0056e6c8  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0056e6cb  8945d0                MOV dword ptr [EBP + -0x30],EAX
0056e6ce  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0056e6d1  51                    PUSH ECX
0056e6d2  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0056e6d5  52                    PUSH EDX
0056e6d6  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0056e6d9  50                    PUSH EAX
0056e6da  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0056e6dd  e80e870d00            CALL 0x00646df0
0056e6e2  e9a6000000            JMP 0x0056e78d
0056e6e7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056e6ea  833907                CMP dword ptr [ECX],0x7
0056e6ed  7530                  JNZ 0x0056e71f
0056e6ef  6a04                  PUSH 0x4
0056e6f1  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056e6f4  83c238                ADD EDX,0x38
0056e6f7  52                    PUSH EDX
0056e6f8  8d45f4                LEA EAX,[EBP + -0xc]
0056e6fb  50                    PUSH EAX
0056e6fc  e89ff43a00            CALL 0x0091dba0
0056e701  83c40c                ADD ESP,0xc
0056e704  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0056e707  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0056e70a  8955cc                MOV dword ptr [EBP + -0x34],EDX
0056e70d  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0056e710  50                    PUSH EAX
0056e711  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0056e714  51                    PUSH ECX
0056e715  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0056e718  e8c3870d00            CALL 0x00646ee0
0056e71d  eb6e                  JMP 0x0056e78d
0056e71f  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056e722  833a09                CMP dword ptr [EDX],0x9
0056e725  7530                  JNZ 0x0056e757
0056e727  6a04                  PUSH 0x4
0056e729  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e72c  83c038                ADD EAX,0x38
0056e72f  50                    PUSH EAX
0056e730  8d4df4                LEA ECX,[EBP + -0xc]
0056e733  51                    PUSH ECX
0056e734  e867f43a00            CALL 0x0091dba0
0056e739  83c40c                ADD ESP,0xc
0056e73c  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0056e73f  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0056e742  8945c8                MOV dword ptr [EBP + -0x38],EAX
0056e745  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0056e748  51                    PUSH ECX
0056e749  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
0056e74c  52                    PUSH EDX
0056e74d  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0056e750  e83b860d00            CALL 0x00646d90
0056e755  eb36                  JMP 0x0056e78d
0056e757  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e75a  83380b                CMP dword ptr [EAX],0xb
0056e75d  752e                  JNZ 0x0056e78d
0056e75f  6a04                  PUSH 0x4
0056e761  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056e764  83c138                ADD ECX,0x38
0056e767  51                    PUSH ECX
0056e768  8d55f4                LEA EDX,[EBP + -0xc]
0056e76b  52                    PUSH EDX
0056e76c  e82ff43a00            CALL 0x0091dba0
0056e771  83c40c                ADD ESP,0xc
0056e774  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0056e777  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0056e77a  894dc4                MOV dword ptr [EBP + -0x3c],ECX
0056e77d  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0056e780  52                    PUSH EDX
0056e781  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
0056e784  50                    PUSH EAX
0056e785  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0056e788  e833880d00            CALL 0x00646fc0
0056e78d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056e790  83392a                CMP dword ptr [ECX],0x2a
0056e793  7512                  JNZ 0x0056e7a7
0056e795  6a14                  PUSH 0x14
0056e797  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e79d  e89ef90000            CALL 0x0057e140
0056e7a2  e97f060000            JMP 0x0056ee26
0056e7a7  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056e7aa  833a0c                CMP dword ptr [EDX],0xc
0056e7ad  7548                  JNZ 0x0056e7f7
0056e7af  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056e7b5  c780480500000000007f  MOV dword ptr [EAX + 0x548],0x7f000000
0056e7bf  680000007f            PUSH 0x7f000000
0056e7c4  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e7ca  e801fc0000            CALL 0x0057e3d0
0056e7cf  6810270000            PUSH 0x2710
0056e7d4  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e7da  8b11                  MOV EDX,dword ptr [ECX]
0056e7dc  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e7e2  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056e7e5  ffd0                  CALL EAX
0056e7e7  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e7ed  e83eb40100            CALL 0x00589c30
0056e7f2  e92f060000            JMP 0x0056ee26
0056e7f7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056e7fa  833901                CMP dword ptr [ECX],0x1
0056e7fd  7547                  JNZ 0x0056e846
0056e7ff  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056e805  8b8280030000          MOV EAX,dword ptr [EDX + 0x380]
0056e80b  8945c0                MOV dword ptr [EBP + -0x40],EAX
0056e80e  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
0056e811  c741300000007f        MOV dword ptr [ECX + 0x30],0x7f000000
0056e818  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056e81e  8b8280030000          MOV EAX,dword ptr [EDX + 0x380]
0056e824  8945bc                MOV dword ptr [EBP + -0x44],EAX
0056e827  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0056e82a  c741380000007f        MOV dword ptr [ECX + 0x38],0x7f000000
0056e831  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056e837  c782b011000001000000  MOV dword ptr [EDX + 0x11b0],0x1
0056e841  e9e0050000            JMP 0x0056ee26
0056e846  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e849  83380f                CMP dword ptr [EAX],0xf
0056e84c  756f                  JNZ 0x0056e8bd
0056e84e  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e854  c781680500000000007f  MOV dword ptr [ECX + 0x568],0x7f000000
0056e85e  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056e864  c7826c0500000000007f  MOV dword ptr [EDX + 0x56c],0x7f000000
0056e86e  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056e874  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
0056e87a  894db8                MOV dword ptr [EBP + -0x48],ECX
0056e87d  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
0056e880  e8eb310200            CALL 0x00591a70
0056e885  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056e88b  c7826810000000000000  MOV dword ptr [EDX + 0x1068],0x0
0056e895  6810270000            PUSH 0x2710
0056e89a  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056e8a0  8b10                  MOV EDX,dword ptr [EAX]
0056e8a2  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e8a8  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056e8ab  ffd0                  CALL EAX
0056e8ad  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e8b3  e878b30100            CALL 0x00589c30
0056e8b8  e969050000            JMP 0x0056ee26
0056e8bd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056e8c0  83392e                CMP dword ptr [ECX],0x2e
0056e8c3  756f                  JNZ 0x0056e934
0056e8c5  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056e8cb  c782680500000000007f  MOV dword ptr [EDX + 0x568],0x7f000000
0056e8d5  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056e8db  c7806c0500000000007f  MOV dword ptr [EAX + 0x56c],0x7f000000
0056e8e5  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e8eb  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
0056e8f1  8955b4                MOV dword ptr [EBP + -0x4c],EDX
0056e8f4  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
0056e8f7  e874310200            CALL 0x00591a70
0056e8fc  6810270000            PUSH 0x2710
0056e901  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056e907  8b10                  MOV EDX,dword ptr [EAX]
0056e909  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e90f  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056e912  ffd0                  CALL EAX
0056e914  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e91a  c7817410000000000000  MOV dword ptr [ECX + 0x1074],0x0
0056e924  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e92a  e801b30100            CALL 0x00589c30
0056e92f  e9f2040000            JMP 0x0056ee26
0056e934  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056e937  833a32                CMP dword ptr [EDX],0x32
0056e93a  7514                  JNZ 0x0056e950
0056e93c  6a01                  PUSH 0x1
0056e93e  6a00                  PUSH 0x0
0056e940  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e946  e895910400            CALL 0x005b7ae0
0056e94b  e9d6040000            JMP 0x0056ee26
0056e950  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e953  833809                CMP dword ptr [EAX],0x9
0056e956  752d                  JNZ 0x0056e985
0056e958  6810270000            PUSH 0x2710
0056e95d  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e963  8b11                  MOV EDX,dword ptr [ECX]
0056e965  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e96b  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056e96e  ffd0                  CALL EAX
0056e970  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e976  c7819010000000000000  MOV dword ptr [ECX + 0x1090],0x0
0056e980  e9a1040000            JMP 0x0056ee26
0056e985  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056e988  833a07                CMP dword ptr [EDX],0x7
0056e98b  752d                  JNZ 0x0056e9ba
0056e98d  6810270000            PUSH 0x2710
0056e992  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056e998  8b10                  MOV EDX,dword ptr [EAX]
0056e99a  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e9a0  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056e9a3  ffd0                  CALL EAX
0056e9a5  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e9ab  c7819410000000000000  MOV dword ptr [ECX + 0x1094],0x0
0056e9b5  e96c040000            JMP 0x0056ee26
0056e9ba  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056e9bd  833a1d                CMP dword ptr [EDX],0x1d
0056e9c0  7420                  JZ 0x0056e9e2
0056e9c2  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e9c5  833819                CMP dword ptr [EAX],0x19
0056e9c8  7418                  JZ 0x0056e9e2
0056e9ca  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056e9cd  83391a                CMP dword ptr [ECX],0x1a
0056e9d0  7410                  JZ 0x0056e9e2
0056e9d2  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056e9d5  833a1c                CMP dword ptr [EDX],0x1c
0056e9d8  7408                  JZ 0x0056e9e2
0056e9da  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056e9dd  83381b                CMP dword ptr [EAX],0x1b
0056e9e0  7548                  JNZ 0x0056ea2a
0056e9e2  6810270000            PUSH 0x2710
0056e9e7  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e9ed  8b11                  MOV EDX,dword ptr [ECX]
0056e9ef  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056e9f5  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056e9f8  ffd0                  CALL EAX
0056e9fa  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ea00  e85bb40100            CALL 0x00589e60
0056ea05  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ea0b  c7818410000000000000  MOV dword ptr [ECX + 0x1084],0x0
0056ea15  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056ea1b  c782ac10000000000000  MOV dword ptr [EDX + 0x10ac],0x0
0056ea25  e9fc030000            JMP 0x0056ee26
0056ea2a  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056ea2d  833836                CMP dword ptr [EAX],0x36
0056ea30  753d                  JNZ 0x0056ea6f
0056ea32  6810270000            PUSH 0x2710
0056ea37  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ea3d  8b11                  MOV EDX,dword ptr [ECX]
0056ea3f  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ea45  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056ea48  ffd0                  CALL EAX
0056ea4a  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ea50  c7817810000000000000  MOV dword ptr [ECX + 0x1078],0x0
0056ea5a  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056ea60  c7827c10000000000000  MOV dword ptr [EDX + 0x107c],0x0
0056ea6a  e9b7030000            JMP 0x0056ee26
0056ea6f  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056ea72  833838                CMP dword ptr [EAX],0x38
0056ea75  752d                  JNZ 0x0056eaa4
0056ea77  6810270000            PUSH 0x2710
0056ea7c  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ea82  8b11                  MOV EDX,dword ptr [ECX]
0056ea84  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ea8a  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056ea8d  ffd0                  CALL EAX
0056ea8f  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ea95  c7818010000000000000  MOV dword ptr [ECX + 0x1080],0x0
0056ea9f  e982030000            JMP 0x0056ee26
0056eaa4  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056eaa7  833a26                CMP dword ptr [EDX],0x26
0056eaaa  7538                  JNZ 0x0056eae4
0056eaac  6810270000            PUSH 0x2710
0056eab1  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056eab7  8b10                  MOV EDX,dword ptr [EAX]
0056eab9  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eabf  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056eac2  ffd0                  CALL EAX
0056eac4  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eaca  c7818810000000000000  MOV dword ptr [ECX + 0x1088],0x0
0056ead4  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eada  e881b30100            CALL 0x00589e60
0056eadf  e942030000            JMP 0x0056ee26
0056eae4  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056eae7  833a43                CMP dword ptr [EDX],0x43
0056eaea  7538                  JNZ 0x0056eb24
0056eaec  6810270000            PUSH 0x2710
0056eaf1  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056eaf7  8b10                  MOV EDX,dword ptr [EAX]
0056eaf9  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eaff  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056eb02  ffd0                  CALL EAX
0056eb04  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eb0a  c781b010000000000000  MOV dword ptr [ECX + 0x10b0],0x0
0056eb14  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eb1a  e841b30100            CALL 0x00589e60
0056eb1f  e902030000            JMP 0x0056ee26
0056eb24  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056eb27  833a47                CMP dword ptr [EDX],0x47
0056eb2a  751d                  JNZ 0x0056eb49
0056eb2c  6810270000            PUSH 0x2710
0056eb31  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056eb37  8b10                  MOV EDX,dword ptr [EAX]
0056eb39  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eb3f  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056eb42  ffd0                  CALL EAX
0056eb44  e9dd020000            JMP 0x0056ee26
0056eb49  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056eb4c  833944                CMP dword ptr [ECX],0x44
0056eb4f  7535                  JNZ 0x0056eb86
0056eb51  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056eb57  c6820c110000ff        MOV byte ptr [EDX + 0x110c],0xff
0056eb5e  6811270000            PUSH 0x2711
0056eb63  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056eb69  8b10                  MOV EDX,dword ptr [EAX]
0056eb6b  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eb71  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056eb74  ffd0                  CALL EAX
0056eb76  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eb7c  e8afb00100            CALL 0x00589c30
0056eb81  e9a0020000            JMP 0x0056ee26
0056eb86  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056eb89  833945                CMP dword ptr [ECX],0x45
0056eb8c  7535                  JNZ 0x0056ebc3
0056eb8e  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056eb94  c6820c110000ff        MOV byte ptr [EDX + 0x110c],0xff
0056eb9b  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056eba1  e8eabd0000            CALL 0x0057a990
0056eba6  6810270000            PUSH 0x2710
0056ebab  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056ebb1  8b10                  MOV EDX,dword ptr [EAX]
0056ebb3  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ebb9  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056ebbc  ffd0                  CALL EAX
0056ebbe  e963020000            JMP 0x0056ee26
0056ebc3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056ebc6  833927                CMP dword ptr [ECX],0x27
0056ebc9  7538                  JNZ 0x0056ec03
0056ebcb  6810270000            PUSH 0x2710
0056ebd0  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056ebd6  8b02                  MOV EAX,dword ptr [EDX]
0056ebd8  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ebde  8b507c                MOV EDX,dword ptr [EAX + 0x7c]
0056ebe1  ffd2                  CALL EDX
0056ebe3  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056ebe9  c7808c10000000000000  MOV dword ptr [EAX + 0x108c],0x0
0056ebf3  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ebf9  e862b20100            CALL 0x00589e60
0056ebfe  e923020000            JMP 0x0056ee26
0056ec03  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056ec06  833914                CMP dword ptr [ECX],0x14
0056ec09  752d                  JNZ 0x0056ec38
0056ec0b  6810270000            PUSH 0x2710
0056ec10  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056ec16  8b02                  MOV EAX,dword ptr [EDX]
0056ec18  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ec1e  8b507c                MOV EDX,dword ptr [EAX + 0x7c]
0056ec21  ffd2                  CALL EDX
0056ec23  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056ec29  c7801802000000000000  MOV dword ptr [EAX + 0x218],0x0
0056ec33  e9ee010000            JMP 0x0056ee26
0056ec38  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056ec3b  83393d                CMP dword ptr [ECX],0x3d
0056ec3e  0f8598000000          JNZ 0x0056ecdc
0056ec44  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056ec4a  8b82ac110000          MOV EAX,dword ptr [EDX + 0x11ac]
0056ec50  8945b0                MOV dword ptr [EBP + -0x50],EAX
0056ec53  837db000              CMP dword ptr [EBP + -0x50],0x0
0056ec57  747e                  JZ 0x0056ecd7
0056ec59  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ec5f  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0056ec62  8955ac                MOV dword ptr [EBP + -0x54],EDX
0056ec65  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0056ec6a  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0056ec6d  894da8                MOV dword ptr [EBP + -0x58],ECX
0056ec70  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
0056ec73  52                    PUSH EDX
0056ec74  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
0056ec77  e844121d00            CALL 0x0073fec0
0056ec7c  8945ec                MOV dword ptr [EBP + -0x14],EAX
0056ec7f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0056ec84  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0056ec87  894da4                MOV dword ptr [EBP + -0x5c],ECX
0056ec8a  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
0056ec8d  52                    PUSH EDX
0056ec8e  8b4da4                MOV ECX,dword ptr [EBP + -0x5c]
0056ec91  e8fa0e1d00            CALL 0x0073fb90
0056ec96  8bc8                  MOV ECX,EAX
0056ec98  e8437f2700            CALL 0x007e6be0
0056ec9d  8945e8                MOV dword ptr [EBP + -0x18],EAX
0056eca0  837de800              CMP dword ptr [EBP + -0x18],0x0
0056eca4  7c31                  JL 0x0056ecd7
0056eca6  837de802              CMP dword ptr [EBP + -0x18],0x2
0056ecaa  7f2b                  JG 0x0056ecd7
0056ecac  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0056ecb1  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0056ecb4  894da0                MOV dword ptr [EBP + -0x60],ECX
0056ecb7  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
0056ecba  e8d10e1d00            CALL 0x0073fb90
0056ecbf  89459c                MOV dword ptr [EBP + -0x64],EAX
0056ecc2  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0056ecc5  69d288000000          IMUL EDX,EDX,0x88
0056eccb  8b459c                MOV EAX,dword ptr [EBP + -0x64]
0056ecce  8d4c1024              LEA ECX,[EAX + EDX*0x1 + 0x24]
0056ecd2  e8f9010000            CALL 0x0056eed0
0056ecd7  e94a010000            JMP 0x0056ee26
0056ecdc  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0056ecdf  833946                CMP dword ptr [ECX],0x46
0056ece2  7553                  JNZ 0x0056ed37
0056ece4  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056ecea  8b8280030000          MOV EAX,dword ptr [EDX + 0x380]
0056ecf0  898568ffffff          MOV dword ptr [EBP + 0xffffff68],EAX
0056ecf6  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
0056ecfc  c741300000007f        MOV dword ptr [ECX + 0x30],0x7f000000
0056ed03  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056ed09  8b8280030000          MOV EAX,dword ptr [EDX + 0x380]
0056ed0f  898564ffffff          MOV dword ptr [EBP + 0xffffff64],EAX
0056ed15  8b8d64ffffff          MOV ECX,dword ptr [EBP + 0xffffff64]
0056ed1b  c741380000007f        MOV dword ptr [ECX + 0x38],0x7f000000
0056ed22  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056ed28  c782b011000001000000  MOV dword ptr [EDX + 0x11b0],0x1
0056ed32  e9ef000000            JMP 0x0056ee26
0056ed37  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056ed3a  833818                CMP dword ptr [EAX],0x18
0056ed3d  7528                  JNZ 0x0056ed67
0056ed3f  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0056ed45  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0056ed48  899560ffffff          MOV dword ptr [EBP + 0xffffff60],EDX
0056ed4e  6a00                  PUSH 0x0
0056ed50  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
0056ed56  e8f5091d00            CALL 0x0073f750
0056ed5b  8bc8                  MOV ECX,EAX
0056ed5d  e81efe2500            CALL 0x007ceb80
0056ed62  e9bf000000            JMP 0x0056ee26
0056ed67  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056ed6a  833820                CMP dword ptr [EAX],0x20
0056ed6d  7512                  JNZ 0x0056ed81
0056ed6f  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ed75  c7415000000000        MOV dword ptr [ECX + 0x50],0x0
0056ed7c  e9a5000000            JMP 0x0056ee26
0056ed81  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056ed84  833a28                CMP dword ptr [EDX],0x28
0056ed87  0f8599000000          JNZ 0x0056ee26
0056ed8d  6a04                  PUSH 0x4
0056ed8f  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0056ed92  83c038                ADD EAX,0x38
0056ed95  50                    PUSH EAX
0056ed96  8d4de4                LEA ECX,[EBP + -0x1c]
0056ed99  51                    PUSH ECX
0056ed9a  e801ee3a00            CALL 0x0091dba0
0056ed9f  83c40c                ADD ESP,0xc
0056eda2  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0056eda8  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0056edab  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
0056edb1  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0056edb4  51                    PUSH ECX
0056edb5  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
0056edbb  e8a0d3faff            CALL 0x0051c160
0056edc0  8945e0                MOV dword ptr [EBP + -0x20],EAX
0056edc3  837de000              CMP dword ptr [EBP + -0x20],0x0
0056edc7  745d                  JZ 0x0056ee26
0056edc9  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0056edcc  8b82a8040000          MOV EAX,dword ptr [EDX + 0x4a8]
0056edd2  898558ffffff          MOV dword ptr [EBP + 0xffffff58],EAX
0056edd8  83bd58ffffff00        CMP dword ptr [EBP + 0xffffff58],0x0
0056eddf  7445                  JZ 0x0056ee26
0056ede1  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0056ede4  c781a804000000000000  MOV dword ptr [ECX + 0x4a8],0x0
0056edee  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0056edf1  8b82d8000000          MOV EAX,dword ptr [EDX + 0xd8]
0056edf7  898554ffffff          MOV dword ptr [EBP + 0xffffff54],EAX
0056edfd  81bd54ffffff5b270000  CMP dword ptr [EBP + 0xffffff54],0x275b
0056ee07  7512                  JNZ 0x0056ee1b
0056ee09  685c270000            PUSH 0x275c
0056ee0e  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0056ee11  8b11                  MOV EDX,dword ptr [ECX]
0056ee13  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0056ee16  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056ee19  ffd0                  CALL EAX
0056ee1b  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0056ee1e  d9e8                  FLD1
0056ee20  d999dc000000          FSTP float ptr [ECX + 0xdc]
0056ee26  837df800              CMP dword ptr [EBP + -0x8],0x0
0056ee2a  7426                  JZ 0x0056ee52
0056ee2c  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0056ee2f  833a01                CMP dword ptr [EDX],0x1
0056ee32  751e                  JNZ 0x0056ee52
0056ee34  837d0c00              CMP dword ptr [EBP + 0xc],0x0
0056ee38  7418                  JZ 0x0056ee52
0056ee3a  6810270000            PUSH 0x2710
0056ee3f  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056ee45  8b10                  MOV EDX,dword ptr [EAX]
0056ee47  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ee4d  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
0056ee50  ffd0                  CALL EAX
0056ee52  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ee58  8b11                  MOV EDX,dword ptr [ECX]
0056ee5a  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ee60  8b82c4000000          MOV EAX,dword ptr [EDX + 0xc4]
0056ee66  ffd0                  CALL EAX
0056ee68  3b0584449900          CMP EAX,dword ptr [0x00994484]
0056ee6e  742a                  JZ 0x0056ee9a
0056ee70  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ee76  8b11                  MOV EDX,dword ptr [ECX]
0056ee78  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ee7e  8b82c4000000          MOV EAX,dword ptr [EDX + 0xc4]
0056ee84  ffd0                  CALL EAX
0056ee86  6a00                  PUSH 0x0
0056ee88  8b0d84449900          MOV ECX,dword ptr [0x00994484]
0056ee8e  51                    PUSH ECX
0056ee8f  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
0056ee95  e8d6e40000            CALL 0x0057d370
0056ee9a  837d0c01              CMP dword ptr [EBP + 0xc],0x1
0056ee9e  7520                  JNZ 0x0056eec0
0056eea0  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
0056eea6  c782c000000000000000  MOV dword ptr [EDX + 0xc0],0x0
0056eeb0  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
0056eeb6  c780c400000000000000  MOV dword ptr [EAX + 0xc4],0x0
0056eec0  b801000000            MOV EAX,0x1
0056eec5  8be5                  MOV ESP,EBP
0056eec7  5d                    POP EBP
0056eec8  c20800                RET 0x8
