; FN 006d7390 FUN_006d7390
006d7390  55                    PUSH EBP
006d7391  8bec                  MOV EBP,ESP
006d7393  81ece0040000          SUB ESP,0x4e0
006d7399  898d64fbffff          MOV dword ptr [EBP + 0xfffffb64],ECX
006d739f  c745a80000007f        MOV dword ptr [EBP + -0x58],0x7f000000
006d73a6  c745c800000000        MOV dword ptr [EBP + -0x38],0x0
006d73ad  d9ee                  FLDZ
006d73af  d95df4                FSTP float ptr [EBP + -0xc]
006d73b2  d9ee                  FLDZ
006d73b4  d95df8                FSTP float ptr [EBP + -0x8]
006d73b7  d9ee                  FLDZ
006d73b9  d95dfc                FSTP float ptr [EBP + -0x4]
006d73bc  d9ee                  FLDZ
006d73be  d95dd4                FSTP float ptr [EBP + -0x2c]
006d73c1  d9ee                  FLDZ
006d73c3  d95db4                FSTP float ptr [EBP + -0x4c]
006d73c6  c745bc01000000        MOV dword ptr [EBP + -0x44],0x1
006d73cd  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d73d3  8b8880030000          MOV ECX,dword ptr [EAX + 0x380]
006d73d9  898d94fdffff          MOV dword ptr [EBP + 0xfffffd94],ECX
006d73df  8b9594fdffff          MOV EDX,dword ptr [EBP + 0xfffffd94]
006d73e5  d9ee                  FLDZ
006d73e7  d95a0c                FSTP float ptr [EDX + 0xc]
006d73ea  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d73f0  8b8880030000          MOV ECX,dword ptr [EAX + 0x380]
006d73f6  898d90fdffff          MOV dword ptr [EBP + 0xfffffd90],ECX
006d73fc  8b9590fdffff          MOV EDX,dword ptr [EBP + 0xfffffd90]
006d7402  c742380000007f        MOV dword ptr [EDX + 0x38],0x7f000000
006d7409  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d740f  c78070050000b80b0000  MOV dword ptr [EAX + 0x570],0xbb8
006d7419  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
006d7420  c745ec0000007f        MOV dword ptr [EBP + -0x14],0x7f000000
006d7427  33c9                  XOR ECX,ECX
006d7429  66894dcc              MOV word ptr [EBP + -0x34],CX
006d742d  c745c000000000        MOV dword ptr [EBP + -0x40],0x0
006d7434  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
006d743b  c745b800000000        MOV dword ptr [EBP + -0x48],0x0
006d7442  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
006d7449  c745e004000000        MOV dword ptr [EBP + -0x20],0x4
006d7450  c745ac00000000        MOV dword ptr [EBP + -0x54],0x0
006d7457  c745a400000000        MOV dword ptr [EBP + -0x5c],0x0
006d745e  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
006d7465  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d7468  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d746b  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d746f  898d60fbffff          MOV dword ptr [EBP + 0xfffffb60],ECX
006d7475  6a04                  PUSH 0x4
006d7477  8b9560fbffff          MOV EDX,dword ptr [EBP + 0xfffffb60]
006d747d  52                    PUSH EDX
006d747e  8d45e4                LEA EAX,[EBP + -0x1c]
006d7481  50                    PUSH EAX
006d7482  e819672400            CALL 0x0091dba0
006d7487  83c40c                ADD ESP,0xc
006d748a  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d748d  83c101                ADD ECX,0x1
006d7490  894de8                MOV dword ptr [EBP + -0x18],ECX
006d7493  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d7496  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d7499  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d749d  898d5cfbffff          MOV dword ptr [EBP + 0xfffffb5c],ECX
006d74a3  6a04                  PUSH 0x4
006d74a5  8b955cfbffff          MOV EDX,dword ptr [EBP + 0xfffffb5c]
006d74ab  52                    PUSH EDX
006d74ac  8d45ec                LEA EAX,[EBP + -0x14]
006d74af  50                    PUSH EAX
006d74b0  e8eb662400            CALL 0x0091dba0
006d74b5  83c40c                ADD ESP,0xc
006d74b8  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d74bb  83c101                ADD ECX,0x1
006d74be  894de8                MOV dword ptr [EBP + -0x18],ECX
006d74c1  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d74c4  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d74c7  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d74cb  898d58fbffff          MOV dword ptr [EBP + 0xfffffb58],ECX
006d74d1  6a04                  PUSH 0x4
006d74d3  8b9558fbffff          MOV EDX,dword ptr [EBP + 0xfffffb58]
006d74d9  52                    PUSH EDX
006d74da  8d45c0                LEA EAX,[EBP + -0x40]
006d74dd  50                    PUSH EAX
006d74de  e8bd662400            CALL 0x0091dba0
006d74e3  83c40c                ADD ESP,0xc
006d74e6  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d74e9  83c101                ADD ECX,0x1
006d74ec  894de8                MOV dword ptr [EBP + -0x18],ECX
006d74ef  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d74f2  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d74f5  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d74f9  898d54fbffff          MOV dword ptr [EBP + 0xfffffb54],ECX
006d74ff  6a04                  PUSH 0x4
006d7501  8b9554fbffff          MOV EDX,dword ptr [EBP + 0xfffffb54]
006d7507  52                    PUSH EDX
006d7508  8d45f0                LEA EAX,[EBP + -0x10]
006d750b  50                    PUSH EAX
006d750c  e88f662400            CALL 0x0091dba0
006d7511  83c40c                ADD ESP,0xc
006d7514  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d7517  83c101                ADD ECX,0x1
006d751a  894de8                MOV dword ptr [EBP + -0x18],ECX
006d751d  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d7520  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d7523  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d7527  898d50fbffff          MOV dword ptr [EBP + 0xfffffb50],ECX
006d752d  6a04                  PUSH 0x4
006d752f  8b9550fbffff          MOV EDX,dword ptr [EBP + 0xfffffb50]
006d7535  52                    PUSH EDX
006d7536  8d45b8                LEA EAX,[EBP + -0x48]
006d7539  50                    PUSH EAX
006d753a  e861662400            CALL 0x0091dba0
006d753f  83c40c                ADD ESP,0xc
006d7542  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d7545  83c101                ADD ECX,0x1
006d7548  894de8                MOV dword ptr [EBP + -0x18],ECX
006d754b  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d754e  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d7551  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d7555  898d4cfbffff          MOV dword ptr [EBP + 0xfffffb4c],ECX
006d755b  6a04                  PUSH 0x4
006d755d  8b954cfbffff          MOV EDX,dword ptr [EBP + 0xfffffb4c]
006d7563  52                    PUSH EDX
006d7564  8d45dc                LEA EAX,[EBP + -0x24]
006d7567  50                    PUSH EAX
006d7568  e833662400            CALL 0x0091dba0
006d756d  83c40c                ADD ESP,0xc
006d7570  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d7573  83c101                ADD ECX,0x1
006d7576  894de8                MOV dword ptr [EBP + -0x18],ECX
006d7579  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d757c  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d757f  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d7583  898d48fbffff          MOV dword ptr [EBP + 0xfffffb48],ECX
006d7589  6a02                  PUSH 0x2
006d758b  8b9548fbffff          MOV EDX,dword ptr [EBP + 0xfffffb48]
006d7591  52                    PUSH EDX
006d7592  8d45cc                LEA EAX,[EBP + -0x34]
006d7595  50                    PUSH EAX
006d7596  e805662400            CALL 0x0091dba0
006d759b  83c40c                ADD ESP,0xc
006d759e  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d75a1  83c101                ADD ECX,0x1
006d75a4  894de8                MOV dword ptr [EBP + -0x18],ECX
006d75a7  837de400              CMP dword ptr [EBP + -0x1c],0x0
006d75ab  0f848a000000          JZ 0x006d763b
006d75b1  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d75b4  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d75b7  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d75bb  898d44fbffff          MOV dword ptr [EBP + 0xfffffb44],ECX
006d75c1  6a04                  PUSH 0x4
006d75c3  8b9544fbffff          MOV EDX,dword ptr [EBP + 0xfffffb44]
006d75c9  52                    PUSH EDX
006d75ca  8d45ac                LEA EAX,[EBP + -0x54]
006d75cd  50                    PUSH EAX
006d75ce  e8cd652400            CALL 0x0091dba0
006d75d3  83c40c                ADD ESP,0xc
006d75d6  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d75d9  83c101                ADD ECX,0x1
006d75dc  894de8                MOV dword ptr [EBP + -0x18],ECX
006d75df  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d75e2  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d75e5  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d75e9  898d40fbffff          MOV dword ptr [EBP + 0xfffffb40],ECX
006d75ef  6a04                  PUSH 0x4
006d75f1  8b9540fbffff          MOV EDX,dword ptr [EBP + 0xfffffb40]
006d75f7  52                    PUSH EDX
006d75f8  8d45e0                LEA EAX,[EBP + -0x20]
006d75fb  50                    PUSH EAX
006d75fc  e89f652400            CALL 0x0091dba0
006d7601  83c40c                ADD ESP,0xc
006d7604  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d7607  83c101                ADD ECX,0x1
006d760a  894de8                MOV dword ptr [EBP + -0x18],ECX
006d760d  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
006d7610  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d7613  8d4c9038              LEA ECX,[EAX + EDX*0x4 + 0x38]
006d7617  898d3cfbffff          MOV dword ptr [EBP + 0xfffffb3c],ECX
006d761d  6a04                  PUSH 0x4
006d761f  8b953cfbffff          MOV EDX,dword ptr [EBP + 0xfffffb3c]
006d7625  52                    PUSH EDX
006d7626  8d45a4                LEA EAX,[EBP + -0x5c]
006d7629  50                    PUSH EAX
006d762a  e871652400            CALL 0x0091dba0
006d762f  83c40c                ADD ESP,0xc
006d7632  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006d7635  83c101                ADD ECX,0x1
006d7638  894de8                MOV dword ptr [EBP + -0x18],ECX
006d763b  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d7641  8b82ac110000          MOV EAX,dword ptr [EDX + 0x11ac]
006d7647  89858cfdffff          MOV dword ptr [EBP + 0xfffffd8c],EAX
006d764d  83bd8cfdffff00        CMP dword ptr [EBP + 0xfffffd8c],0x0
006d7654  7474                  JZ 0x006d76ca
006d7656  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006d765c  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d765f  8955a0                MOV dword ptr [EBP + -0x60],EDX
006d7662  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d7668  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006d766b  898d88fdffff          MOV dword ptr [EBP + 0xfffffd88],ECX
006d7671  8b9588fdffff          MOV EDX,dword ptr [EBP + 0xfffffd88]
006d7677  52                    PUSH EDX
006d7678  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
006d767b  e840880600            CALL 0x0073fec0
006d7680  50                    PUSH EAX
006d7681  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
006d7684  e807850600            CALL 0x0073fb90
006d7689  8bc8                  MOV ECX,EAX
006d768b  e850f51000            CALL 0x007e6be0
006d7690  89459c                MOV dword ptr [EBP + -0x64],EAX
006d7693  837d9c00              CMP dword ptr [EBP + -0x64],0x0
006d7697  7c31                  JL 0x006d76ca
006d7699  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
006d769c  e8ef840600            CALL 0x0073fb90
006d76a1  898584fdffff          MOV dword ptr [EBP + 0xfffffd84],EAX
006d76a7  8b459c                MOV EAX,dword ptr [EBP + -0x64]
006d76aa  69c088000000          IMUL EAX,EAX,0x88
006d76b0  8b8d84fdffff          MOV ECX,dword ptr [EBP + 0xfffffd84]
006d76b6  8d540124              LEA EDX,[ECX + EAX*0x1 + 0x24]
006d76ba  895598                MOV dword ptr [EBP + -0x68],EDX
006d76bd  68c7b20100            PUSH 0x1b2c7
006d76c2  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
006d76c5  e856da1000            CALL 0x007e5120
006d76ca  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d76d0  8b10                  MOV EDX,dword ptr [EAX]
006d76d2  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d76d8  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
006d76de  ffd0                  CALL EAX
006d76e0  85c0                  TEST EAX,EAX
006d76e2  0f85c4000000          JNZ 0x006d77ac
006d76e8  33c9                  XOR ECX,ECX
006d76ea  0f85bc000000          JNZ 0x006d77ac
006d76f0  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d76f6  83baac11000000        CMP dword ptr [EDX + 0x11ac],0x0
006d76fd  742c                  JZ 0x006d772b
006d76ff  6a00                  PUSH 0x0
006d7701  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d7707  8b10                  MOV EDX,dword ptr [EAX]
006d7709  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d770f  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
006d7715  ffd0                  CALL EAX
006d7717  0fbfc8                MOVSX ECX,AX
006d771a  33d2                  XOR EDX,EDX
006d771c  85c9                  TEST ECX,ECX
006d771e  0f9ec2                SETLE DL
006d7721  899580fdffff          MOV dword ptr [EBP + 0xfffffd80],EDX
006d7727  eb0c                  JMP 0x006d7735
006d772b  c78580fdffff00000000  MOV dword ptr [EBP + 0xfffffd80],0x0
006d7735  83bd80fdffff00        CMP dword ptr [EBP + 0xfffffd80],0x0
006d773c  756e                  JNZ 0x006d77ac
006d773e  b880000000            MOV EAX,0x80
006d7743  0fb7c8                MOVZX ECX,AX
006d7746  ba80000000            MOV EDX,0x80
006d774b  0fb7c2                MOVZX EAX,DX
006d774e  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d7754  0fb79214110000        MOVZX EDX,word ptr [EDX + 0x1114]
006d775b  23d0                  AND EDX,EAX
006d775d  33c0                  XOR EAX,EAX
006d775f  3bd1                  CMP EDX,ECX
006d7761  0f94c0                SETZ AL
006d7764  85c0                  TEST EAX,EAX
006d7766  7444                  JZ 0x006d77ac
006d7768  b904000000            MOV ECX,0x4
006d776d  0fb7d1                MOVZX EDX,CX
006d7770  b804000000            MOV EAX,0x4
006d7775  0fb7c8                MOVZX ECX,AX
006d7778  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d777e  0fb78014110000        MOVZX EAX,word ptr [EAX + 0x1114]
006d7785  23c1                  AND EAX,ECX
006d7787  33c9                  XOR ECX,ECX
006d7789  3bc2                  CMP EAX,EDX
006d778b  0f94c1                SETZ CL
006d778e  85c9                  TEST ECX,ECX
006d7790  741a                  JZ 0x006d77ac
006d7792  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d7798  8b4204                MOV EAX,dword ptr [EDX + 0x4]
006d779b  89857cfdffff          MOV dword ptr [EBP + 0xfffffd7c],EAX
006d77a1  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d77a4  3b8d7cfdffff          CMP ECX,dword ptr [EBP + 0xfffffd7c]
006d77aa  7517                  JNZ 0x006d77c3
006d77ac  6a01                  PUSH 0x1
006d77ae  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d77b4  e867330000            CALL 0x006dab20
006d77b9  a1bc449900            MOV EAX,[0x009944bc]
006d77be  e9381c0000            JMP 0x006d93fb
006d77c3  c745d000000000        MOV dword ptr [EBP + -0x30],0x0
006d77ca  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
006d77d0  8b4208                MOV EAX,dword ptr [EDX + 0x8]
006d77d3  898578fdffff          MOV dword ptr [EBP + 0xfffffd78],EAX
006d77d9  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d77dc  51                    PUSH ECX
006d77dd  8b8d78fdffff          MOV ECX,dword ptr [EBP + 0xfffffd78]
006d77e3  e8b848e4ff            CALL 0x0051c0a0
006d77e8  8945d8                MOV dword ptr [EBP + -0x28],EAX
006d77eb  c745b000000000        MOV dword ptr [EBP + -0x50],0x0
006d77f2  837dd800              CMP dword ptr [EBP + -0x28],0x0
006d77f6  0f84bb000000          JZ 0x006d78b7
006d77fc  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
006d77ff  8b02                  MOV EAX,dword ptr [EDX]
006d7801  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006d7804  8b5010                MOV EDX,dword ptr [EAX + 0x10]
006d7807  ffd2                  CALL EDX
006d7809  85c0                  TEST EAX,EAX
006d780b  0f84a6000000          JZ 0x006d78b7
006d7811  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
006d7814  8b10                  MOV EDX,dword ptr [EAX]
006d7816  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006d7819  8b4210                MOV EAX,dword ptr [EDX + 0x10]
006d781c  ffd0                  CALL EAX
006d781e  8945b0                MOV dword ptr [EBP + -0x50],EAX
006d7821  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
006d7828  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d782b  8b11                  MOV EDX,dword ptr [ECX]
006d782d  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7830  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006d7833  ffd0                  CALL EAX
006d7835  85c0                  TEST EAX,EAX
006d7837  7450                  JZ 0x006d7889
006d7839  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d783c  8b11                  MOV EDX,dword ptr [ECX]
006d783e  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7841  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006d7844  ffd0                  CALL EAX
006d7846  898574fdffff          MOV dword ptr [EBP + 0xfffffd74],EAX
006d784c  8b8d74fdffff          MOV ECX,dword ptr [EBP + 0xfffffd74]
006d7852  83b9ac11000000        CMP dword ptr [ECX + 0x11ac],0x0
006d7859  7427                  JZ 0x006d7882
006d785b  6a00                  PUSH 0x0
006d785d  8b9574fdffff          MOV EDX,dword ptr [EBP + 0xfffffd74]
006d7863  8b02                  MOV EAX,dword ptr [EDX]
006d7865  8b8d74fdffff          MOV ECX,dword ptr [EBP + 0xfffffd74]
006d786b  8b909c000000          MOV EDX,dword ptr [EAX + 0x9c]
006d7871  ffd2                  CALL EDX
006d7873  98                    CWDE
006d7874  33c9                  XOR ECX,ECX
006d7876  85c0                  TEST EAX,EAX
006d7878  0f9ec1                SETLE CL
006d787b  894d94                MOV dword ptr [EBP + -0x6c],ECX
006d787e  eb09                  JMP 0x006d7889
006d7882  c7459400000000        MOV dword ptr [EBP + -0x6c],0x0
006d7889  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d788c  8b02                  MOV EAX,dword ptr [EDX]
006d788e  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7891  8b9094000000          MOV EDX,dword ptr [EAX + 0x94]
006d7897  ffd2                  CALL EDX
006d7899  85c0                  TEST EAX,EAX
006d789b  751a                  JNZ 0x006d78b7
006d789d  837d9400              CMP dword ptr [EBP + -0x6c],0x0
006d78a1  7514                  JNZ 0x006d78b7
006d78a3  6a01                  PUSH 0x1
006d78a5  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d78a8  50                    PUSH EAX
006d78a9  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d78af  e8cc320000            CALL 0x006dab80
006d78b4  8945d0                MOV dword ptr [EBP + -0x30],EAX
006d78b7  c745c400000000        MOV dword ptr [EBP + -0x3c],0x0
006d78be  837dd000              CMP dword ptr [EBP + -0x30],0x0
006d78c2  0f842e1b0000          JZ 0x006d93f6
006d78c8  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d78cb  81c194000000          ADD ECX,0x94
006d78d1  8b11                  MOV EDX,dword ptr [ECX]
006d78d3  8955f4                MOV dword ptr [EBP + -0xc],EDX
006d78d6  8b4104                MOV EAX,dword ptr [ECX + 0x4]
006d78d9  8945f8                MOV dword ptr [EBP + -0x8],EAX
006d78dc  8b4908                MOV ECX,dword ptr [ECX + 0x8]
006d78df  894dfc                MOV dword ptr [EBP + -0x4],ECX
006d78e2  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d78e5  e8d6dae6ff            CALL 0x005453c0
006d78ea  8945c8                MOV dword ptr [EBP + -0x38],EAX
006d78ed  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d78f3  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d78f9  898570fdffff          MOV dword ptr [EBP + 0xfffffd70],EAX
006d78ff  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d7902  51                    PUSH ECX
006d7903  8b8d70fdffff          MOV ECX,dword ptr [EBP + 0xfffffd70]
006d7909  e8d294ebff            CALL 0x00590de0
006d790e  898564ffffff          MOV dword ptr [EBP + 0xffffff64],EAX
006d7914  83bd64ffffff00        CMP dword ptr [EBP + 0xffffff64],0x0
006d791b  7407                  JZ 0x006d7924
006d791d  c745f07d270000        MOV dword ptr [EBP + -0x10],0x277d
006d7924  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d7927  8b02                  MOV EAX,dword ptr [EDX]
006d7929  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d792c  8b5018                MOV EDX,dword ptr [EAX + 0x18]
006d792f  ffd2                  CALL EDX
006d7931  85c0                  TEST EAX,EAX
006d7933  747d                  JZ 0x006d79b2
006d7935  d9ee                  FLDZ
006d7937  d99d2cffffff          FSTP float ptr [EBP + 0xffffff2c]
006d793d  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d7943  8b8880030000          MOV ECX,dword ptr [EAX + 0x380]
006d7949  898d6cfdffff          MOV dword ptr [EBP + 0xfffffd6c],ECX
006d794f  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d7952  899568fdffff          MOV dword ptr [EBP + 0xfffffd68],EDX
006d7958  8b856cfdffff          MOV EAX,dword ptr [EBP + 0xfffffd6c]
006d795e  8b8d68fdffff          MOV ECX,dword ptr [EBP + 0xfffffd68]
006d7964  894838                MOV dword ptr [EAX + 0x38],ECX
006d7967  6a00                  PUSH 0x0
006d7969  6a01                  PUSH 0x1
006d796b  8d952cffffff          LEA EDX,[EBP + 0xffffff2c]
006d7971  52                    PUSH EDX
006d7972  8d45f4                LEA EAX,[EBP + -0xc]
006d7975  50                    PUSH EAX
006d7976  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d7979  51                    PUSH ECX
006d797a  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7980  e88be5eaff            CALL 0x00585f10
006d7985  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d798b  8b8280030000          MOV EAX,dword ptr [EDX + 0x380]
006d7991  898564fdffff          MOV dword ptr [EBP + 0xfffffd64],EAX
006d7997  d9852cffffff          FLD float ptr [EBP + 0xffffff2c]
006d799d  d99d60fdffff          FSTP float ptr [EBP + 0xfffffd60]
006d79a3  8b8d64fdffff          MOV ECX,dword ptr [EBP + 0xfffffd64]
006d79a9  d98560fdffff          FLD float ptr [EBP + 0xfffffd60]
006d79af  d9590c                FSTP float ptr [ECX + 0xc]
006d79b2  6a00                  PUSH 0x0
006d79b4  8b9564ffffff          MOV EDX,dword ptr [EBP + 0xffffff64]
006d79ba  52                    PUSH EDX
006d79bb  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d79be  50                    PUSH EAX
006d79bf  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d79c5  e8a69be9ff            CALL 0x00571570
006d79ca  d95dd4                FSTP float ptr [EBP + -0x2c]
006d79cd  8b8d64ffffff          MOV ECX,dword ptr [EBP + 0xffffff64]
006d79d3  51                    PUSH ECX
006d79d4  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d79d7  52                    PUSH EDX
006d79d8  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d79de  e8bd9ce9ff            CALL 0x005716a0
006d79e3  d95db4                FSTP float ptr [EBP + -0x4c]
006d79e6  c78530ffffff00000000  MOV dword ptr [EBP + 0xffffff30],0x0
006d79f0  c745900000007f        MOV dword ptr [EBP + -0x70],0x7f000000
006d79f7  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d79fd  0594000000            ADD EAX,0x94
006d7a02  8b08                  MOV ECX,dword ptr [EAX]
006d7a04  898d54ffffff          MOV dword ptr [EBP + 0xffffff54],ECX
006d7a0a  8b5004                MOV EDX,dword ptr [EAX + 0x4]
006d7a0d  899558ffffff          MOV dword ptr [EBP + 0xffffff58],EDX
006d7a13  8b4008                MOV EAX,dword ptr [EAX + 0x8]
006d7a16  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
006d7a1c  d9855cffffff          FLD float ptr [EBP + 0xffffff5c]
006d7a22  dc05109f9800          FADD double ptr [0x00989f10]
006d7a28  d99d5cffffff          FSTP float ptr [EBP + 0xffffff5c]
006d7a2e  d9ee                  FLDZ
006d7a30  d99d34ffffff          FSTP float ptr [EBP + 0xffffff34]
006d7a36  d9ee                  FLDZ
006d7a38  d99d38ffffff          FSTP float ptr [EBP + 0xffffff38]
006d7a3e  d9ee                  FLDZ
006d7a40  d99d3cffffff          FSTP float ptr [EBP + 0xffffff3c]
006d7a46  0fb60d79449900        MOVZX ECX,byte ptr [0x00994479]
006d7a4d  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d7a50  0fb64208              MOVZX EAX,byte ptr [EDX + 0x8]
006d7a54  3bc1                  CMP EAX,ECX
006d7a56  7412                  JZ 0x006d7a6a
006d7a58  0fb60d78449900        MOVZX ECX,byte ptr [0x00994478]
006d7a5f  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d7a62  0fb64208              MOVZX EAX,byte ptr [EDX + 0x8]
006d7a66  3bc1                  CMP EAX,ECX
006d7a68  754d                  JNZ 0x006d7ab7
006d7a6a  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7a6d  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d7a70  89955cfdffff          MOV dword ptr [EBP + 0xfffffd5c],EDX
006d7a76  6a00                  PUSH 0x0
006d7a78  6a00                  PUSH 0x0
006d7a7a  8d8528ffffff          LEA EAX,[EBP + 0xffffff28]
006d7a80  50                    PUSH EAX
006d7a81  8d8d34ffffff          LEA ECX,[EBP + 0xffffff34]
006d7a87  51                    PUSH ECX
006d7a88  8b955cfdffff          MOV EDX,dword ptr [EBP + 0xfffffd5c]
006d7a8e  52                    PUSH EDX
006d7a8f  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7a95  e876e4eaff            CALL 0x00585f10
006d7a9a  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
006d7aa0  8945f4                MOV dword ptr [EBP + -0xc],EAX
006d7aa3  8b8d38ffffff          MOV ECX,dword ptr [EBP + 0xffffff38]
006d7aa9  894df8                MOV dword ptr [EBP + -0x8],ECX
006d7aac  8b953cffffff          MOV EDX,dword ptr [EBP + 0xffffff3c]
006d7ab2  8955fc                MOV dword ptr [EBP + -0x4],EDX
006d7ab5  eb22                  JMP 0x006d7ad9
006d7ab7  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
006d7aba  0594000000            ADD EAX,0x94
006d7abf  8b08                  MOV ECX,dword ptr [EAX]
006d7ac1  898d34ffffff          MOV dword ptr [EBP + 0xffffff34],ECX
006d7ac7  8b5004                MOV EDX,dword ptr [EAX + 0x4]
006d7aca  899538ffffff          MOV dword ptr [EBP + 0xffffff38],EDX
006d7ad0  8b4008                MOV EAX,dword ptr [EAX + 0x8]
006d7ad3  89853cffffff          MOV dword ptr [EBP + 0xffffff3c],EAX
006d7ad9  d9853cffffff          FLD float ptr [EBP + 0xffffff3c]
006d7adf  dc05109f9800          FADD double ptr [0x00989f10]
006d7ae5  d99d3cffffff          FSTP float ptr [EBP + 0xffffff3c]
006d7aeb  d9ee                  FLDZ
006d7aed  d99d78ffffff          FSTP float ptr [EBP + 0xffffff78]
006d7af3  d9ee                  FLDZ
006d7af5  d99d7cffffff          FSTP float ptr [EBP + 0xffffff7c]
006d7afb  d9ee                  FLDZ
006d7afd  d95d80                FSTP float ptr [EBP + -0x80]
006d7b00  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7b06  e8b5d8e6ff            CALL 0x005453c0
006d7b0b  85c0                  TEST EAX,EAX
006d7b0d  0f841b030000          JZ 0x006d7e2e
006d7b13  d9855cffffff          FLD float ptr [EBP + 0xffffff5c]
006d7b19  dc0538759800          FADD double ptr [0x00987538]
006d7b1f  d99d5cffffff          FSTP float ptr [EBP + 0xffffff5c]
006d7b25  d9853cffffff          FLD float ptr [EBP + 0xffffff3c]
006d7b2b  dc2538759800          FSUB double ptr [0x00987538]
006d7b31  d99d3cffffff          FSTP float ptr [EBP + 0xffffff3c]
006d7b37  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7b3d  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d7b40  899558fdffff          MOV dword ptr [EBP + 0xfffffd58],EDX
006d7b46  6a00                  PUSH 0x0
006d7b48  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7b4e  e88d66e9ff            CALL 0x0056e1e0
006d7b53  50                    PUSH EAX
006d7b54  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d7b57  50                    PUSH EAX
006d7b58  8b8d58fdffff          MOV ECX,dword ptr [EBP + 0xfffffd58]
006d7b5e  51                    PUSH ECX
006d7b5f  8d5590                LEA EDX,[EBP + -0x70]
006d7b62  52                    PUSH EDX
006d7b63  8d8578ffffff          LEA EAX,[EBP + 0xffffff78]
006d7b69  50                    PUSH EAX
006d7b6a  8d8d34ffffff          LEA ECX,[EBP + 0xffffff34]
006d7b70  51                    PUSH ECX
006d7b71  8d9554ffffff          LEA EDX,[EBP + 0xffffff54]
006d7b77  52                    PUSH EDX
006d7b78  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7b7e  e83dd8e6ff            CALL 0x005453c0
006d7b83  8bc8                  MOV ECX,EAX
006d7b85  e8d63fe5ff            CALL 0x0052bb60
006d7b8a  898530ffffff          MOV dword ptr [EBP + 0xffffff30],EAX
006d7b90  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7b96  e84566e9ff            CALL 0x0056e1e0
006d7b9b  85c0                  TEST EAX,EAX
006d7b9d  0f8593000000          JNZ 0x006d7c36
006d7ba3  83bd30ffffff00        CMP dword ptr [EBP + 0xffffff30],0x0
006d7baa  0f8486000000          JZ 0x006d7c36
006d7bb0  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
006d7bb3  8b10                  MOV EDX,dword ptr [EAX]
006d7bb5  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7bb8  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006d7bbb  ffd0                  CALL EAX
006d7bbd  85c0                  TEST EAX,EAX
006d7bbf  7475                  JZ 0x006d7c36
006d7bc1  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7bc7  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
006d7bcd  89951cffffff          MOV dword ptr [EBP + 0xffffff1c],EDX
006d7bd3  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
006d7bd9  d94008                FLD float ptr [EAX + 0x8]
006d7bdc  d99d20ffffff          FSTP float ptr [EBP + 0xffffff20]
006d7be2  8b8d1cffffff          MOV ECX,dword ptr [EBP + 0xffffff1c]
006d7be8  8b5130                MOV EDX,dword ptr [ECX + 0x30]
006d7beb  899524ffffff          MOV dword ptr [EBP + 0xffffff24],EDX
006d7bf1  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d7bf4  898554fdffff          MOV dword ptr [EBP + 0xfffffd54],EAX
006d7bfa  8b8d1cffffff          MOV ECX,dword ptr [EBP + 0xffffff1c]
006d7c00  8b9554fdffff          MOV EDX,dword ptr [EBP + 0xfffffd54]
006d7c06  895130                MOV dword ptr [ECX + 0x30],EDX
006d7c09  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
006d7c0f  d90528c69800          FLD float ptr [0x0098c628]
006d7c15  d95808                FSTP float ptr [EAX + 0x8]
006d7c18  8b8d1cffffff          MOV ECX,dword ptr [EBP + 0xffffff1c]
006d7c1e  8b9524ffffff          MOV EDX,dword ptr [EBP + 0xffffff24]
006d7c24  895130                MOV dword ptr [ECX + 0x30],EDX
006d7c27  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
006d7c2d  d98520ffffff          FLD float ptr [EBP + 0xffffff20]
006d7c33  d95808                FSTP float ptr [EAX + 0x8]
006d7c36  83bd30ffffff00        CMP dword ptr [EBP + 0xffffff30],0x0
006d7c3d  0f85ae010000          JNZ 0x006d7df1
006d7c43  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7c49  81c194000000          ADD ECX,0x94
006d7c4f  51                    PUSH ECX
006d7c50  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d7c56  81c258050000          ADD EDX,0x558
006d7c5c  52                    PUSH EDX
006d7c5d  e81efde3ff            CALL 0x00517980
006d7c62  83c408                ADD ESP,0x8
006d7c65  85c0                  TEST EAX,EAX
006d7c67  0f845b010000          JZ 0x006d7dc8
006d7c6d  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d7c73  8b88e8100000          MOV ECX,dword ptr [EAX + 0x10e8]
006d7c79  898d50fdffff          MOV dword ptr [EBP + 0xfffffd50],ECX
006d7c7f  83bd50fdffff00        CMP dword ptr [EBP + 0xfffffd50],0x0
006d7c86  0f843c010000          JZ 0x006d7dc8
006d7c8c  6a00                  PUSH 0x0
006d7c8e  68da000000            PUSH 0xda
006d7c93  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7c99  e802d1eaff            CALL 0x00584da0
006d7c9e  d9ee                  FLDZ
006d7ca0  d99d20feffff          FSTP float ptr [EBP + 0xfffffe20]
006d7ca6  d9ee                  FLDZ
006d7ca8  d99d24feffff          FSTP float ptr [EBP + 0xfffffe24]
006d7cae  d9ee                  FLDZ
006d7cb0  d99d28feffff          FSTP float ptr [EBP + 0xfffffe28]
006d7cb6  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d7cbc  81c258050000          ADD EDX,0x558
006d7cc2  8b8520feffff          MOV EAX,dword ptr [EBP + 0xfffffe20]
006d7cc8  8902                  MOV dword ptr [EDX],EAX
006d7cca  8b8d24feffff          MOV ECX,dword ptr [EBP + 0xfffffe24]
006d7cd0  894a04                MOV dword ptr [EDX + 0x4],ECX
006d7cd3  8b8528feffff          MOV EAX,dword ptr [EBP + 0xfffffe28]
006d7cd9  894208                MOV dword ptr [EDX + 0x8],EAX
006d7cdc  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7ce2  c781480500000000007f  MOV dword ptr [ECX + 0x548],0x7f000000
006d7cec  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d7cf2  8b4204                MOV EAX,dword ptr [EDX + 0x4]
006d7cf5  89854cfdffff          MOV dword ptr [EBP + 0xfffffd4c],EAX
006d7cfb  680000007f            PUSH 0x7f000000
006d7d00  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d7d03  51                    PUSH ECX
006d7d04  6a01                  PUSH 0x1
006d7d06  6a00                  PUSH 0x0
006d7d08  8b954cfdffff          MOV EDX,dword ptr [EBP + 0xfffffd4c]
006d7d0e  52                    PUSH EDX
006d7d0f  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7d15  e85698e9ff            CALL 0x00571570
006d7d1a  51                    PUSH ECX
006d7d1b  d91c24                FSTP float ptr [ESP]
006d7d1e  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7d24  e8674ceaff            CALL 0x0057c990
006d7d29  898518ffffff          MOV dword ptr [EBP + 0xffffff18],EAX
006d7d2f  81bd18ffffff0000007f  CMP dword ptr [EBP + 0xffffff18],0x7f000000
006d7d39  7474                  JZ 0x006d7daf
006d7d3b  a1a4b4a100            MOV EAX,[0x00a1b4a4]
006d7d40  8b4808                MOV ECX,dword ptr [EAX + 0x8]
006d7d43  898d48fdffff          MOV dword ptr [EBP + 0xfffffd48],ECX
006d7d49  8b9518ffffff          MOV EDX,dword ptr [EBP + 0xffffff18]
006d7d4f  52                    PUSH EDX
006d7d50  8b8d48fdffff          MOV ECX,dword ptr [EBP + 0xfffffd48]
006d7d56  e84543e4ff            CALL 0x0051c0a0
006d7d5b  8945d8                MOV dword ptr [EBP + -0x28],EAX
006d7d5e  837dd800              CMP dword ptr [EBP + -0x28],0x0
006d7d62  7412                  JZ 0x006d7d76
006d7d64  0fb60574449900        MOVZX EAX,byte ptr [0x00994474]
006d7d6b  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006d7d6e  0fb65108              MOVZX EDX,byte ptr [ECX + 0x8]
006d7d72  3bd0                  CMP EDX,EAX
006d7d74  7417                  JZ 0x006d7d8d
006d7d76  6a01                  PUSH 0x1
006d7d78  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7d7e  e89d2d0000            CALL 0x006dab20
006d7d83  a1b8449900            MOV EAX,[0x009944b8]
006d7d88  e96e160000            JMP 0x006d93fb
006d7d8d  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d7d90  50                    PUSH EAX
006d7d91  8b8d18ffffff          MOV ECX,dword ptr [EBP + 0xffffff18]
006d7d97  51                    PUSH ECX
006d7d98  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7d9e  e8cd2e0000            CALL 0x006dac70
006d7da3  a1b4449900            MOV EAX,[0x009944b4]
006d7da8  e94e160000            JMP 0x006d93fb
006d7daf  6a01                  PUSH 0x1
006d7db1  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d7db7  e8642d0000            CALL 0x006dab20
006d7dbc  a1b8449900            MOV EAX,[0x009944b8]
006d7dc1  e935160000            JMP 0x006d93fb
006d7dc8  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d7dce  81c294000000          ADD EDX,0x94
006d7dd4  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d7dda  0558050000            ADD EAX,0x558
006d7ddf  8b0a                  MOV ECX,dword ptr [EDX]
006d7de1  8908                  MOV dword ptr [EAX],ECX
006d7de3  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
006d7de6  894804                MOV dword ptr [EAX + 0x4],ECX
006d7de9  8b5208                MOV EDX,dword ptr [EDX + 0x8]
006d7dec  895008                MOV dword ptr [EAX + 0x8],EDX
006d7def  eb3d                  JMP 0x006d7e2e
006d7df1  d9ee                  FLDZ
006d7df3  d99d14feffff          FSTP float ptr [EBP + 0xfffffe14]
006d7df9  d9ee                  FLDZ
006d7dfb  d99d18feffff          FSTP float ptr [EBP + 0xfffffe18]
006d7e01  d9ee                  FLDZ
006d7e03  d99d1cfeffff          FSTP float ptr [EBP + 0xfffffe1c]
006d7e09  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d7e0f  0558050000            ADD EAX,0x558
006d7e14  8b8d14feffff          MOV ECX,dword ptr [EBP + 0xfffffe14]
006d7e1a  8908                  MOV dword ptr [EAX],ECX
006d7e1c  8b9518feffff          MOV EDX,dword ptr [EBP + 0xfffffe18]
006d7e22  895004                MOV dword ptr [EAX + 0x4],EDX
006d7e25  8b8d1cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe1c]
006d7e2b  894808                MOV dword ptr [EAX + 0x8],ECX
006d7e2e  8d55f4                LEA EDX,[EBP + -0xc]
006d7e31  52                    PUSH EDX
006d7e32  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d7e38  0594000000            ADD EAX,0x94
006d7e3d  50                    PUSH EAX
006d7e3e  8d4d84                LEA ECX,[EBP + -0x7c]
006d7e41  51                    PUSH ECX
006d7e42  e869d4d6ff            CALL 0x004452b0
006d7e47  83c40c                ADD ESP,0xc
006d7e4a  d9ee                  FLDZ
006d7e4c  d95d8c                FSTP float ptr [EBP + -0x74]
006d7e4f  8d5584                LEA EDX,[EBP + -0x7c]
006d7e52  52                    PUSH EDX
006d7e53  8d4584                LEA EAX,[EBP + -0x7c]
006d7e56  50                    PUSH EAX
006d7e57  e8443ad7ff            CALL 0x0044b8a0
006d7e5c  83c408                ADD ESP,0x8
006d7e5f  d99d44ffffff          FSTP float ptr [EBP + 0xffffff44]
006d7e65  d9ee                  FLDZ
006d7e67  d99d6cffffff          FSTP float ptr [EBP + 0xffffff6c]
006d7e6d  d9ee                  FLDZ
006d7e6f  d99d70ffffff          FSTP float ptr [EBP + 0xffffff70]
006d7e75  d9ee                  FLDZ
006d7e77  d99d74ffffff          FSTP float ptr [EBP + 0xffffff74]
006d7e7d  c7854cffffff00000000  MOV dword ptr [EBP + 0xffffff4c],0x0
006d7e87  d945b4                FLD float ptr [EBP + -0x4c]
006d7e8a  d84db4                FMUL float ptr [EBP + -0x4c]
006d7e8d  d99d38fbffff          FSTP float ptr [EBP + 0xfffffb38]
006d7e93  d98538fbffff          FLD float ptr [EBP + 0xfffffb38]
006d7e99  dc2598d89800          FSUB double ptr [0x0098d898]
006d7e9f  d98544ffffff          FLD float ptr [EBP + 0xffffff44]
006d7ea5  ded9                  FCOMPP
006d7ea7  dfe0                  FNSTSW AX
006d7ea9  f6c405                TEST AH,0x5
006d7eac  7b1f                  JNP 0x006d7ecd
006d7eae  837db000              CMP dword ptr [EBP + -0x50],0x0
006d7eb2  0f84cb020000          JZ 0x006d8183
006d7eb8  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7ebb  8b11                  MOV EDX,dword ptr [ECX]
006d7ebd  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7ec0  8b4218                MOV EAX,dword ptr [EDX + 0x18]
006d7ec3  ffd0                  CALL EAX
006d7ec5  85c0                  TEST EAX,EAX
006d7ec7  0f84b6020000          JZ 0x006d8183
006d7ecd  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7ed0  8b11                  MOV EDX,dword ptr [ECX]
006d7ed2  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7ed5  8b4218                MOV EAX,dword ptr [EDX + 0x18]
006d7ed8  ffd0                  CALL EAX
006d7eda  85c0                  TEST EAX,EAX
006d7edc  0f84b7000000          JZ 0x006d7f99
006d7ee2  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d7ee5  81c194000000          ADD ECX,0x94
006d7eeb  51                    PUSH ECX
006d7eec  8d55f4                LEA EDX,[EBP + -0xc]
006d7eef  52                    PUSH EDX
006d7ef0  8d85bcfeffff          LEA EAX,[EBP + 0xfffffebc]
006d7ef6  50                    PUSH EAX
006d7ef7  e8b4d3d6ff            CALL 0x004452b0
006d7efc  83c40c                ADD ESP,0xc
006d7eff  8d8dbcfeffff          LEA ECX,[EBP + 0xfffffebc]
006d7f05  51                    PUSH ECX
006d7f06  8d9508feffff          LEA EDX,[EBP + 0xfffffe08]
006d7f0c  52                    PUSH EDX
006d7f0d  e8aec4e3ff            CALL 0x005143c0
006d7f12  83c408                ADD ESP,0x8
006d7f15  8b08                  MOV ECX,dword ptr [EAX]
006d7f17  898dbcfeffff          MOV dword ptr [EBP + 0xfffffebc],ECX
006d7f1d  8b5004                MOV EDX,dword ptr [EAX + 0x4]
006d7f20  8995c0feffff          MOV dword ptr [EBP + 0xfffffec0],EDX
006d7f26  8b4008                MOV EAX,dword ptr [EAX + 0x8]
006d7f29  8985c4feffff          MOV dword ptr [EBP + 0xfffffec4],EAX
006d7f2f  51                    PUSH ECX
006d7f30  d90568bd9800          FLD float ptr [0x0098bd68]
006d7f36  d91c24                FSTP float ptr [ESP]
006d7f39  51                    PUSH ECX
006d7f3a  d945b4                FLD float ptr [EBP + -0x4c]
006d7f3d  d91c24                FSTP float ptr [ESP]
006d7f40  8d8dbcfeffff          LEA ECX,[EBP + 0xfffffebc]
006d7f46  51                    PUSH ECX
006d7f47  8d95fcfdffff          LEA EDX,[EBP + 0xfffffdfc]
006d7f4d  52                    PUSH EDX
006d7f4e  e8ade2d5ff            CALL 0x00436200
006d7f53  83c40c                ADD ESP,0xc
006d7f56  50                    PUSH EAX
006d7f57  8d85f0fdffff          LEA EAX,[EBP + 0xfffffdf0]
006d7f5d  50                    PUSH EAX
006d7f5e  e89de2d5ff            CALL 0x00436200
006d7f63  83c40c                ADD ESP,0xc
006d7f66  50                    PUSH EAX
006d7f67  8d4df4                LEA ECX,[EBP + -0xc]
006d7f6a  51                    PUSH ECX
006d7f6b  8d95e4fdffff          LEA EDX,[EBP + 0xfffffde4]
006d7f71  52                    PUSH EDX
006d7f72  e82949d7ff            CALL 0x0044c8a0
006d7f77  83c40c                ADD ESP,0xc
006d7f7a  8b08                  MOV ECX,dword ptr [EAX]
006d7f7c  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
006d7f82  8b5004                MOV EDX,dword ptr [EAX + 0x4]
006d7f85  899570ffffff          MOV dword ptr [EBP + 0xffffff70],EDX
006d7f8b  8b4008                MOV EAX,dword ptr [EAX + 0x8]
006d7f8e  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
006d7f94  e9ac000000            JMP 0x006d8045
006d7f99  d94584                FLD float ptr [EBP + -0x7c]
006d7f9c  d9e0                  FCHS
006d7f9e  d99d00fdffff          FSTP float ptr [EBP + 0xfffffd00]
006d7fa4  d94588                FLD float ptr [EBP + -0x78]
006d7fa7  d9e0                  FCHS
006d7fa9  d99d04fdffff          FSTP float ptr [EBP + 0xfffffd04]
006d7faf  d9458c                FLD float ptr [EBP + -0x74]
006d7fb2  d9e0                  FCHS
006d7fb4  d99d08fdffff          FSTP float ptr [EBP + 0xfffffd08]
006d7fba  d98500fdffff          FLD float ptr [EBP + 0xfffffd00]
006d7fc0  d99dd8fdffff          FSTP float ptr [EBP + 0xfffffdd8]
006d7fc6  d98504fdffff          FLD float ptr [EBP + 0xfffffd04]
006d7fcc  d99ddcfdffff          FSTP float ptr [EBP + 0xfffffddc]
006d7fd2  d98508fdffff          FLD float ptr [EBP + 0xfffffd08]
006d7fd8  d99de0fdffff          FSTP float ptr [EBP + 0xfffffde0]
006d7fde  8d8dd8fdffff          LEA ECX,[EBP + 0xfffffdd8]
006d7fe4  51                    PUSH ECX
006d7fe5  8d95b0feffff          LEA EDX,[EBP + 0xfffffeb0]
006d7feb  52                    PUSH EDX
006d7fec  e8cfc3e3ff            CALL 0x005143c0
006d7ff1  83c408                ADD ESP,0x8
006d7ff4  51                    PUSH ECX
006d7ff5  d945b4                FLD float ptr [EBP + -0x4c]
006d7ff8  d91c24                FSTP float ptr [ESP]
006d7ffb  8d85b0feffff          LEA EAX,[EBP + 0xfffffeb0]
006d8001  50                    PUSH EAX
006d8002  8d8dccfdffff          LEA ECX,[EBP + 0xfffffdcc]
006d8008  51                    PUSH ECX
006d8009  e8f2e1d5ff            CALL 0x00436200
006d800e  83c40c                ADD ESP,0xc
006d8011  50                    PUSH EAX
006d8012  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d8015  81c294000000          ADD EDX,0x94
006d801b  52                    PUSH EDX
006d801c  8d85c0fdffff          LEA EAX,[EBP + 0xfffffdc0]
006d8022  50                    PUSH EAX
006d8023  e888d2d6ff            CALL 0x004452b0
006d8028  83c40c                ADD ESP,0xc
006d802b  8b08                  MOV ECX,dword ptr [EAX]
006d802d  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
006d8033  8b5004                MOV EDX,dword ptr [EAX + 0x4]
006d8036  899570ffffff          MOV dword ptr [EBP + 0xffffff70],EDX
006d803c  8b4008                MOV EAX,dword ptr [EAX + 0x8]
006d803f  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
006d8045  8d8dc8feffff          LEA ECX,[EBP + 0xfffffec8]
006d804b  e8103ee5ff            CALL 0x0052be60
006d8050  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8056  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
006d805c  8995d0fcffff          MOV dword ptr [EBP + 0xfffffcd0],EDX
006d8062  8b85d0fcffff          MOV EAX,dword ptr [EBP + 0xfffffcd0]
006d8068  d94004                FLD float ptr [EAX + 0x4]
006d806b  d99dccfcffff          FSTP float ptr [EBP + 0xfffffccc]
006d8071  d985ccfcffff          FLD float ptr [EBP + 0xfffffccc]
006d8077  d99dbcfdffff          FSTP float ptr [EBP + 0xfffffdbc]
006d807d  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8083  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
006d8089  8995c8fcffff          MOV dword ptr [EBP + 0xfffffcc8],EDX
006d808f  8b85c8fcffff          MOV EAX,dword ptr [EBP + 0xfffffcc8]
006d8095  d94014                FLD float ptr [EAX + 0x14]
006d8098  d99dc4fcffff          FSTP float ptr [EBP + 0xfffffcc4]
006d809e  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d80a4  e817d3e6ff            CALL 0x005453c0
006d80a9  0504010000            ADD EAX,0x104
006d80ae  8985c0fcffff          MOV dword ptr [EBP + 0xfffffcc0],EAX
006d80b4  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d80ba  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d80bd  8995bcfcffff          MOV dword ptr [EBP + 0xfffffcbc],EDX
006d80c3  a1a4b4a100            MOV EAX,[0x00a1b4a4]
006d80c8  8b4808                MOV ECX,dword ptr [EAX + 0x8]
006d80cb  898db8fcffff          MOV dword ptr [EBP + 0xfffffcb8],ECX
006d80d1  8d95c8feffff          LEA EDX,[EBP + 0xfffffec8]
006d80d7  52                    PUSH EDX
006d80d8  6a01                  PUSH 0x1
006d80da  51                    PUSH ECX
006d80db  d985c4fcffff          FLD float ptr [EBP + 0xfffffcc4]
006d80e1  d91c24                FSTP float ptr [ESP]
006d80e4  8d85bcfdffff          LEA EAX,[EBP + 0xfffffdbc]
006d80ea  50                    PUSH EAX
006d80eb  8d8d6cffffff          LEA ECX,[EBP + 0xffffff6c]
006d80f1  51                    PUSH ECX
006d80f2  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d80f8  81c294000000          ADD EDX,0x94
006d80fe  52                    PUSH EDX
006d80ff  8b85c0fcffff          MOV EAX,dword ptr [EBP + 0xfffffcc0]
006d8105  83c00c                ADD EAX,0xc
006d8108  50                    PUSH EAX
006d8109  8b8dbcfcffff          MOV ECX,dword ptr [EBP + 0xfffffcbc]
006d810f  51                    PUSH ECX
006d8110  8b8db8fcffff          MOV ECX,dword ptr [EBP + 0xfffffcb8]
006d8116  e8f547e4ff            CALL 0x0051c910
006d811b  898514ffffff          MOV dword ptr [EBP + 0xffffff14],EAX
006d8121  83bd14ffffff01        CMP dword ptr [EBP + 0xffffff14],0x1
006d8128  7459                  JZ 0x006d8183
006d812a  8b95ccfeffff          MOV EDX,dword ptr [EBP + 0xfffffecc]
006d8130  89956cffffff          MOV dword ptr [EBP + 0xffffff6c],EDX
006d8136  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
006d813c  898570ffffff          MOV dword ptr [EBP + 0xffffff70],EAX
006d8142  8b8dd4feffff          MOV ECX,dword ptr [EBP + 0xfffffed4]
006d8148  898d74ffffff          MOV dword ptr [EBP + 0xffffff74],ECX
006d814e  8d956cffffff          LEA EDX,[EBP + 0xffffff6c]
006d8154  52                    PUSH EDX
006d8155  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
006d8158  0594000000            ADD EAX,0x94
006d815d  50                    PUSH EAX
006d815e  8d8db0fdffff          LEA ECX,[EBP + 0xfffffdb0]
006d8164  51                    PUSH ECX
006d8165  e846d1d6ff            CALL 0x004452b0
006d816a  83c40c                ADD ESP,0xc
006d816d  50                    PUSH EAX
006d816e  e87dc1e3ff            CALL 0x005142f0
006d8173  83c404                ADD ESP,0x4
006d8176  d95db4                FSTP float ptr [EBP + -0x4c]
006d8179  c7854cffffff01000000  MOV dword ptr [EBP + 0xffffff4c],0x1
006d8183  d9ee                  FLDZ
006d8185  d99d50ffffff          FSTP float ptr [EBP + 0xffffff50]
006d818b  837db000              CMP dword ptr [EBP + -0x50],0x0
006d818f  7458                  JZ 0x006d81e9
006d8191  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d8194  8b02                  MOV EAX,dword ptr [EDX]
006d8196  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8199  8b5018                MOV EDX,dword ptr [EAX + 0x18]
006d819c  ffd2                  CALL EDX
006d819e  85c0                  TEST EAX,EAX
006d81a0  7447                  JZ 0x006d81e9
006d81a2  8d856cffffff          LEA EAX,[EBP + 0xffffff6c]
006d81a8  50                    PUSH EAX
006d81a9  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d81af  81c194000000          ADD ECX,0x94
006d81b5  51                    PUSH ECX
006d81b6  8d95a4feffff          LEA EDX,[EBP + 0xfffffea4]
006d81bc  52                    PUSH EDX
006d81bd  e8eed0d6ff            CALL 0x004452b0
006d81c2  83c40c                ADD ESP,0xc
006d81c5  d9ee                  FLDZ
006d81c7  d99dacfeffff          FSTP float ptr [EBP + 0xfffffeac]
006d81cd  8d85a4feffff          LEA EAX,[EBP + 0xfffffea4]
006d81d3  50                    PUSH EAX
006d81d4  8d8da4feffff          LEA ECX,[EBP + 0xfffffea4]
006d81da  51                    PUSH ECX
006d81db  e8c036d7ff            CALL 0x0044b8a0
006d81e0  83c408                ADD ESP,0x8
006d81e3  d99d50ffffff          FSTP float ptr [EBP + 0xffffff50]
006d81e9  c78568ffffff01000000  MOV dword ptr [EBP + 0xffffff68],0x1
006d81f3  837db000              CMP dword ptr [EBP + -0x50],0x0
006d81f7  747d                  JZ 0x006d8276
006d81f9  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d81fc  8b02                  MOV EAX,dword ptr [EDX]
006d81fe  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8201  8b5030                MOV EDX,dword ptr [EAX + 0x30]
006d8204  ffd2                  CALL EDX
006d8206  85c0                  TEST EAX,EAX
006d8208  746c                  JZ 0x006d8276
006d820a  d9ee                  FLDZ
006d820c  d99da0feffff          FSTP float ptr [EBP + 0xfffffea0]
006d8212  a1d0b4a100            MOV EAX,[0x00a1b4d0]
006d8217  8b8830010000          MOV ECX,dword ptr [EAX + 0x130]
006d821d  898d9cfcffff          MOV dword ptr [EBP + 0xfffffc9c],ECX
006d8223  8b959cfcffff          MOV EDX,dword ptr [EBP + 0xfffffc9c]
006d8229  8b4210                MOV EAX,dword ptr [EDX + 0x10]
006d822c  898598fcffff          MOV dword ptr [EBP + 0xfffffc98],EAX
006d8232  8d8da0feffff          LEA ECX,[EBP + 0xfffffea0]
006d8238  51                    PUSH ECX
006d8239  8b15f0fda000          MOV EDX,dword ptr [0x00a0fdf0]
006d823f  52                    PUSH EDX
006d8240  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8246  0fb78884110000        MOVZX ECX,word ptr [EAX + 0x1184]
006d824d  51                    PUSH ECX
006d824e  8b8d98fcffff          MOV ECX,dword ptr [EBP + 0xfffffc98]
006d8254  e897310400            CALL 0x0071b3f0
006d8259  d985a0feffff          FLD float ptr [EBP + 0xfffffea0]
006d825f  dc1de8c19800          FCOMP double ptr [0x0098c1e8]
006d8265  dfe0                  FNSTSW AX
006d8267  f6c405                TEST AH,0x5
006d826a  7a0a                  JP 0x006d8276
006d826c  c78568ffffff00000000  MOV dword ptr [EBP + 0xffffff68],0x0
006d8276  83bd68ffffff00        CMP dword ptr [EBP + 0xffffff68],0x0
006d827d  0f8469090000          JZ 0x006d8bec
006d8283  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8289  e832d1e6ff            CALL 0x005453c0
006d828e  3945c8                CMP dword ptr [EBP + -0x38],EAX
006d8291  0f85c9000000          JNZ 0x006d8360
006d8297  d945d4                FLD float ptr [EBP + -0x2c]
006d829a  dc05f07d9800          FADD double ptr [0x00987df0]
006d82a0  d99d94fcffff          FSTP float ptr [EBP + 0xfffffc94]
006d82a6  d98594fcffff          FLD float ptr [EBP + 0xfffffc94]
006d82ac  d88d94fcffff          FMUL float ptr [EBP + 0xfffffc94]
006d82b2  d99d34fbffff          FSTP float ptr [EBP + 0xfffffb34]
006d82b8  d98534fbffff          FLD float ptr [EBP + 0xfffffb34]
006d82be  d98544ffffff          FLD float ptr [EBP + 0xffffff44]
006d82c4  ded9                  FCOMPP
006d82c6  dfe0                  FNSTSW AX
006d82c8  f6c441                TEST AH,0x41
006d82cb  0f848f000000          JZ 0x006d8360
006d82d1  d945b4                FLD float ptr [EBP + -0x4c]
006d82d4  d84db4                FMUL float ptr [EBP + -0x4c]
006d82d7  d99d30fbffff          FSTP float ptr [EBP + 0xfffffb30]
006d82dd  d98530fbffff          FLD float ptr [EBP + 0xfffffb30]
006d82e3  dc2598d89800          FSUB double ptr [0x0098d898]
006d82e9  d98544ffffff          FLD float ptr [EBP + 0xffffff44]
006d82ef  ded9                  FCOMPP
006d82f1  dfe0                  FNSTSW AX
006d82f3  f6c405                TEST AH,0x5
006d82f6  7a1b                  JP 0x006d8313
006d82f8  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d82fe  8b82ac110000          MOV EAX,dword ptr [EDX + 0x11ac]
006d8304  898590fcffff          MOV dword ptr [EBP + 0xfffffc90],EAX
006d830a  83bd90fcffff00        CMP dword ptr [EBP + 0xfffffc90],0x0
006d8311  744d                  JZ 0x006d8360
006d8313  83bd30ffffff00        CMP dword ptr [EBP + 0xffffff30],0x0
006d831a  7444                  JZ 0x006d8360
006d831c  837db000              CMP dword ptr [EBP + -0x50],0x0
006d8320  0f84c6080000          JZ 0x006d8bec
006d8326  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8329  8b11                  MOV EDX,dword ptr [ECX]
006d832b  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d832e  8b4218                MOV EAX,dword ptr [EDX + 0x18]
006d8331  ffd0                  CALL EAX
006d8333  85c0                  TEST EAX,EAX
006d8335  0f84b1080000          JZ 0x006d8bec
006d833b  d945b4                FLD float ptr [EBP + -0x4c]
006d833e  d84db4                FMUL float ptr [EBP + -0x4c]
006d8341  d99d2cfbffff          FSTP float ptr [EBP + 0xfffffb2c]
006d8347  d9852cfbffff          FLD float ptr [EBP + 0xfffffb2c]
006d834d  d98550ffffff          FLD float ptr [EBP + 0xffffff50]
006d8353  ded9                  FCOMPP
006d8355  dfe0                  FNSTSW AX
006d8357  f6c441                TEST AH,0x41
006d835a  0f858c080000          JNZ 0x006d8bec
006d8360  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8366  e8755ee9ff            CALL 0x0056e1e0
006d836b  89859cfeffff          MOV dword ptr [EBP + 0xfffffe9c],EAX
006d8371  c78598feffff00000000  MOV dword ptr [EBP + 0xfffffe98],0x0
006d837b  d945b4                FLD float ptr [EBP + -0x4c]
006d837e  d84db4                FMUL float ptr [EBP + -0x4c]
006d8381  d99d28fbffff          FSTP float ptr [EBP + 0xfffffb28]
006d8387  d98528fbffff          FLD float ptr [EBP + 0xfffffb28]
006d838d  dc2598d89800          FSUB double ptr [0x0098d898]
006d8393  d98544ffffff          FLD float ptr [EBP + 0xffffff44]
006d8399  ded9                  FCOMPP
006d839b  dfe0                  FNSTSW AX
006d839d  f6c405                TEST AH,0x5
006d83a0  7b38                  JNP 0x006d83da
006d83a2  837db000              CMP dword ptr [EBP + -0x50],0x0
006d83a6  743c                  JZ 0x006d83e4
006d83a8  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d83ab  8b11                  MOV EDX,dword ptr [ECX]
006d83ad  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d83b0  8b4218                MOV EAX,dword ptr [EDX + 0x18]
006d83b3  ffd0                  CALL EAX
006d83b5  85c0                  TEST EAX,EAX
006d83b7  742b                  JZ 0x006d83e4
006d83b9  d945b4                FLD float ptr [EBP + -0x4c]
006d83bc  d84db4                FMUL float ptr [EBP + -0x4c]
006d83bf  d99d24fbffff          FSTP float ptr [EBP + 0xfffffb24]
006d83c5  d98524fbffff          FLD float ptr [EBP + 0xfffffb24]
006d83cb  d98550ffffff          FLD float ptr [EBP + 0xffffff50]
006d83d1  ded9                  FCOMPP
006d83d3  dfe0                  FNSTSW AX
006d83d5  f6c441                TEST AH,0x41
006d83d8  750a                  JNZ 0x006d83e4
006d83da  c78598feffff01000000  MOV dword ptr [EBP + 0xfffffe98],0x1
006d83e4  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d83ea  8b912c050000          MOV EDX,dword ptr [ECX + 0x52c]
006d83f0  89958cfcffff          MOV dword ptr [EBP + 0xfffffc8c],EDX
006d83f6  83bd8cfcffff00        CMP dword ptr [EBP + 0xfffffc8c],0x0
006d83fd  7417                  JZ 0x006d8416
006d83ff  6a01                  PUSH 0x1
006d8401  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8407  e814270000            CALL 0x006dab20
006d840c  a1bc449900            MOV EAX,[0x009944bc]
006d8411  e9e50f0000            JMP 0x006d93fb
006d8416  837dc800              CMP dword ptr [EBP + -0x38],0x0
006d841a  7411                  JZ 0x006d842d
006d841c  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
006d841f  8b8824010000          MOV ECX,dword ptr [EAX + 0x124]
006d8425  894da8                MOV dword ptr [EBP + -0x58],ECX
006d8428  e999000000            JMP 0x006d84c6
006d842d  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d8430  8b02                  MOV EAX,dword ptr [EDX]
006d8432  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8435  8b5030                MOV EDX,dword ptr [EAX + 0x30]
006d8438  ffd2                  CALL EDX
006d843a  85c0                  TEST EAX,EAX
006d843c  7471                  JZ 0x006d84af
006d843e  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8444  8b8848110000          MOV ECX,dword ptr [EAX + 0x1148]
006d844a  898d88fcffff          MOV dword ptr [EBP + 0xfffffc88],ECX
006d8450  81bd88fcffff0000007f  CMP dword ptr [EBP + 0xfffffc88],0x7f000000
006d845a  7553                  JNZ 0x006d84af
006d845c  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d845f  8b02                  MOV EAX,dword ptr [EDX]
006d8461  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8464  8b5030                MOV EDX,dword ptr [EAX + 0x30]
006d8467  ffd2                  CALL EDX
006d8469  898584fcffff          MOV dword ptr [EBP + 0xfffffc84],EAX
006d846f  8b8584fcffff          MOV EAX,dword ptr [EBP + 0xfffffc84]
006d8475  8b8850030000          MOV ECX,dword ptr [EAX + 0x350]
006d847b  894da8                MOV dword ptr [EBP + -0x58],ECX
006d847e  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d8481  8b02                  MOV EAX,dword ptr [EDX]
006d8483  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8486  8b5030                MOV EDX,dword ptr [EAX + 0x30]
006d8489  ffd2                  CALL EDX
006d848b  898580fcffff          MOV dword ptr [EBP + 0xfffffc80],EAX
006d8491  8b8580fcffff          MOV EAX,dword ptr [EBP + 0xfffffc80]
006d8497  0554030000            ADD EAX,0x354
006d849c  8b08                  MOV ECX,dword ptr [EAX]
006d849e  894df4                MOV dword ptr [EBP + -0xc],ECX
006d84a1  8b5004                MOV EDX,dword ptr [EAX + 0x4]
006d84a4  8955f8                MOV dword ptr [EBP + -0x8],EDX
006d84a7  8b4008                MOV EAX,dword ptr [EAX + 0x8]
006d84aa  8945fc                MOV dword ptr [EBP + -0x4],EAX
006d84ad  eb17                  JMP 0x006d84c6
006d84af  6a01                  PUSH 0x1
006d84b1  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d84b7  e864260000            CALL 0x006dab20
006d84bc  a1bc449900            MOV EAX,[0x009944bc]
006d84c1  e9350f0000            JMP 0x006d93fb
006d84c6  83bd30ffffff00        CMP dword ptr [EBP + 0xffffff30],0x0
006d84cd  7416                  JZ 0x006d84e5
006d84cf  8d4dcc                LEA ECX,[EBP + -0x34]
006d84d2  51                    PUSH ECX
006d84d3  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d84d6  52                    PUSH EDX
006d84d7  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d84dd  e8cea40000            CALL 0x006e29b0
006d84e2  8945c4                MOV dword ptr [EBP + -0x3c],EAX
006d84e5  837dc400              CMP dword ptr [EBP + -0x3c],0x0
006d84e9  7405                  JZ 0x006d84f0
006d84eb  e9fc060000            JMP 0x006d8bec
006d84f0  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d84f6  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
006d84fc  898d7cfcffff          MOV dword ptr [EBP + 0xfffffc7c],ECX
006d8502  8b957cfcffff          MOV EDX,dword ptr [EBP + 0xfffffc7c]
006d8508  0fb78246010000        MOVZX EAX,word ptr [EDX + 0x146]
006d850f  89858cfeffff          MOV dword ptr [EBP + 0xfffffe8c],EAX
006d8515  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006d851b  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d851e  899578fcffff          MOV dword ptr [EBP + 0xfffffc78],EDX
006d8524  6a00                  PUSH 0x0
006d8526  8b8d78fcffff          MOV ECX,dword ptr [EBP + 0xfffffc78]
006d852c  e85f760600            CALL 0x0073fb90
006d8531  8bc8                  MOV ECX,EAX
006d8533  e868d81000            CALL 0x007e5da0
006d8538  898588feffff          MOV dword ptr [EBP + 0xfffffe88],EAX
006d853e  c78594feffff0000007f  MOV dword ptr [EBP + 0xfffffe94],0x7f000000
006d8548  83bd88feffff00        CMP dword ptr [EBP + 0xfffffe88],0x0
006d854f  7420                  JZ 0x006d8571
006d8551  8b8d88feffff          MOV ECX,dword ptr [EBP + 0xfffffe88]
006d8557  e8a4520a00            CALL 0x0077d800
006d855c  898574fcffff          MOV dword ptr [EBP + 0xfffffc74],EAX
006d8562  8b8574fcffff          MOV EAX,dword ptr [EBP + 0xfffffc74]
006d8568  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006d856b  898d94feffff          MOV dword ptr [EBP + 0xfffffe94],ECX
006d8571  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
006d8574  8b02                  MOV EAX,dword ptr [EDX]
006d8576  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006d8579  8b5018                MOV EDX,dword ptr [EAX + 0x18]
006d857c  ffd2                  CALL EDX
006d857e  85c0                  TEST EAX,EAX
006d8580  0f84c7000000          JZ 0x006d864d
006d8586  6a00                  PUSH 0x0
006d8588  6a01                  PUSH 0x1
006d858a  8d8578feffff          LEA EAX,[EBP + 0xfffffe78]
006d8590  50                    PUSH EAX
006d8591  8d4df4                LEA ECX,[EBP + -0xc]
006d8594  51                    PUSH ECX
006d8595  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d8598  52                    PUSH EDX
006d8599  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d859f  e86cd9eaff            CALL 0x00585f10
006d85a4  d98578feffff          FLD float ptr [EBP + 0xfffffe78]
006d85aa  d95db4                FSTP float ptr [EBP + -0x4c]
006d85ad  6a00                  PUSH 0x0
006d85af  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d85b5  0594000000            ADD EAX,0x94
006d85ba  50                    PUSH EAX
006d85bb  8d8d7cfeffff          LEA ECX,[EBP + 0xfffffe7c]
006d85c1  51                    PUSH ECX
006d85c2  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
006d85c5  8b02                  MOV EAX,dword ptr [EDX]
006d85c7  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006d85ca  8b5018                MOV EDX,dword ptr [EAX + 0x18]
006d85cd  ffd2                  CALL EDX
006d85cf  8bc8                  MOV ECX,EAX
006d85d1  e84a4df4ff            CALL 0x0061d320
006d85d6  8b857cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe7c]
006d85dc  8945f4                MOV dword ptr [EBP + -0xc],EAX
006d85df  8b8d80feffff          MOV ECX,dword ptr [EBP + 0xfffffe80]
006d85e5  894df8                MOV dword ptr [EBP + -0x8],ECX
006d85e8  8b9584feffff          MOV EDX,dword ptr [EBP + 0xfffffe84]
006d85ee  8955fc                MOV dword ptr [EBP + -0x4],EDX
006d85f1  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d85f7  8b8880030000          MOV ECX,dword ptr [EAX + 0x380]
006d85fd  898d70fcffff          MOV dword ptr [EBP + 0xfffffc70],ECX
006d8603  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d8606  89956cfcffff          MOV dword ptr [EBP + 0xfffffc6c],EDX
006d860c  8b8570fcffff          MOV EAX,dword ptr [EBP + 0xfffffc70]
006d8612  8b8d6cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc6c]
006d8618  894838                MOV dword ptr [EAX + 0x38],ECX
006d861b  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8621  8b8280030000          MOV EAX,dword ptr [EDX + 0x380]
006d8627  898568fcffff          MOV dword ptr [EBP + 0xfffffc68],EAX
006d862d  d98578feffff          FLD float ptr [EBP + 0xfffffe78]
006d8633  d99d64fcffff          FSTP float ptr [EBP + 0xfffffc64]
006d8639  8b8d68fcffff          MOV ECX,dword ptr [EBP + 0xfffffc68]
006d863f  d98564fcffff          FLD float ptr [EBP + 0xfffffc64]
006d8645  d9590c                FSTP float ptr [ECX + 0xc]
006d8648  e984000000            JMP 0x006d86d1
006d864d  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
006d8650  8b02                  MOV EAX,dword ptr [EDX]
006d8652  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006d8655  8b5048                MOV EDX,dword ptr [EAX + 0x48]
006d8658  ffd2                  CALL EDX
006d865a  85c0                  TEST EAX,EAX
006d865c  7473                  JZ 0x006d86d1
006d865e  6a00                  PUSH 0x0
006d8660  6a01                  PUSH 0x1
006d8662  8d8568feffff          LEA EAX,[EBP + 0xfffffe68]
006d8668  50                    PUSH EAX
006d8669  8d4df4                LEA ECX,[EBP + -0xc]
006d866c  51                    PUSH ECX
006d866d  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d8670  52                    PUSH EDX
006d8671  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8677  e894d8eaff            CALL 0x00585f10
006d867c  d98568feffff          FLD float ptr [EBP + 0xfffffe68]
006d8682  d95db4                FSTP float ptr [EBP + -0x4c]
006d8685  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d868b  0594000000            ADD EAX,0x94
006d8690  50                    PUSH EAX
006d8691  8d8d6cfeffff          LEA ECX,[EBP + 0xfffffe6c]
006d8697  51                    PUSH ECX
006d8698  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
006d869b  8b02                  MOV EAX,dword ptr [EDX]
006d869d  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006d86a0  8b5048                MOV EDX,dword ptr [EAX + 0x48]
006d86a3  ffd2                  CALL EDX
006d86a5  8bc8                  MOV ECX,EAX
006d86a7  e824d7f1ff            CALL 0x005f5dd0
006d86ac  8b856cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe6c]
006d86b2  8945f4                MOV dword ptr [EBP + -0xc],EAX
006d86b5  8b8d70feffff          MOV ECX,dword ptr [EBP + 0xfffffe70]
006d86bb  894df8                MOV dword ptr [EBP + -0x8],ECX
006d86be  8b9574feffff          MOV EDX,dword ptr [EBP + 0xfffffe74]
006d86c4  8955fc                MOV dword ptr [EBP + -0x4],EDX
006d86c7  c78598feffff00000000  MOV dword ptr [EBP + 0xfffffe98],0x0
006d86d1  6a00                  PUSH 0x0
006d86d3  6a00                  PUSH 0x0
006d86d5  6a00                  PUSH 0x0
006d86d7  6a00                  PUSH 0x0
006d86d9  6a00                  PUSH 0x0
006d86db  6a00                  PUSH 0x0
006d86dd  8d45a4                LEA EAX,[EBP + -0x5c]
006d86e0  50                    PUSH EAX
006d86e1  6a01                  PUSH 0x1
006d86e3  8d4de0                LEA ECX,[EBP + -0x20]
006d86e6  51                    PUSH ECX
006d86e7  6a01                  PUSH 0x1
006d86e9  8d55ac                LEA EDX,[EBP + -0x54]
006d86ec  52                    PUSH EDX
006d86ed  6a01                  PUSH 0x1
006d86ef  8d45cc                LEA EAX,[EBP + -0x34]
006d86f2  50                    PUSH EAX
006d86f3  6a01                  PUSH 0x1
006d86f5  8d4ddc                LEA ECX,[EBP + -0x24]
006d86f8  51                    PUSH ECX
006d86f9  6a01                  PUSH 0x1
006d86fb  8d55b8                LEA EDX,[EBP + -0x48]
006d86fe  52                    PUSH EDX
006d86ff  6a01                  PUSH 0x1
006d8701  8d45f0                LEA EAX,[EBP + -0x10]
006d8704  50                    PUSH EAX
006d8705  6a01                  PUSH 0x1
006d8707  8d4dc0                LEA ECX,[EBP + -0x40]
006d870a  51                    PUSH ECX
006d870b  6a01                  PUSH 0x1
006d870d  8d55ec                LEA EDX,[EBP + -0x14]
006d8710  52                    PUSH EDX
006d8711  6a03                  PUSH 0x3
006d8713  8d45e4                LEA EAX,[EBP + -0x1c]
006d8716  50                    PUSH EAX
006d8717  6a01                  PUSH 0x1
006d8719  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006d871c  0fb7516c              MOVZX EDX,word ptr [ECX + 0x6c]
006d8720  52                    PUSH EDX
006d8721  6a0c                  PUSH 0xc
006d8723  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8729  e87285e6ff            CALL 0x00540ca0
006d872e  c78590feffff01000000  MOV dword ptr [EBP + 0xfffffe90],0x1
006d8738  83bd98feffff00        CMP dword ptr [EBP + 0xfffffe98],0x0
006d873f  0f8502010000          JNZ 0x006d8847
006d8745  c78564feffff00000000  MOV dword ptr [EBP + 0xfffffe64],0x0
006d874f  d945b4                FLD float ptr [EBP + -0x4c]
006d8752  d99d60feffff          FSTP float ptr [EBP + 0xfffffe60]
006d8758  83bd9cfeffff00        CMP dword ptr [EBP + 0xfffffe9c],0x0
006d875f  7409                  JZ 0x006d876a
006d8761  d945d4                FLD float ptr [EBP + -0x2c]
006d8764  d99d60feffff          FSTP float ptr [EBP + 0xfffffe60]
006d876a  83bd8cfeffff0b        CMP dword ptr [EBP + 0xfffffe8c],0xb
006d8771  7409                  JZ 0x006d877c
006d8773  83bd8cfeffff0c        CMP dword ptr [EBP + 0xfffffe8c],0xc
006d877a  751d                  JNZ 0x006d8799
006d877c  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8782  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006d8785  898d60fcffff          MOV dword ptr [EBP + 0xfffffc60],ECX
006d878b  8b9594feffff          MOV EDX,dword ptr [EBP + 0xfffffe94]
006d8791  3b9560fcffff          CMP EDX,dword ptr [EBP + 0xfffffc60]
006d8797  7563                  JNZ 0x006d87fc
006d8799  6a00                  PUSH 0x0
006d879b  6a00                  PUSH 0x0
006d879d  6a00                  PUSH 0x0
006d879f  6a00                  PUSH 0x0
006d87a1  6a00                  PUSH 0x0
006d87a3  6a00                  PUSH 0x0
006d87a5  6a00                  PUSH 0x0
006d87a7  6a00                  PUSH 0x0
006d87a9  6a00                  PUSH 0x0
006d87ab  6a00                  PUSH 0x0
006d87ad  6a00                  PUSH 0x0
006d87af  6a00                  PUSH 0x0
006d87b1  8d859cfeffff          LEA EAX,[EBP + 0xfffffe9c]
006d87b7  50                    PUSH EAX
006d87b8  6a01                  PUSH 0x1
006d87ba  8d8d64feffff          LEA ECX,[EBP + 0xfffffe64]
006d87c0  51                    PUSH ECX
006d87c1  6a01                  PUSH 0x1
006d87c3  8d55bc                LEA EDX,[EBP + -0x44]
006d87c6  52                    PUSH EDX
006d87c7  6a01                  PUSH 0x1
006d87c9  8d45d4                LEA EAX,[EBP + -0x2c]
006d87cc  50                    PUSH EAX
006d87cd  6a02                  PUSH 0x2
006d87cf  8d8d60feffff          LEA ECX,[EBP + 0xfffffe60]
006d87d5  51                    PUSH ECX
006d87d6  6a02                  PUSH 0x2
006d87d8  8d9590feffff          LEA EDX,[EBP + 0xfffffe90]
006d87de  52                    PUSH EDX
006d87df  6a01                  PUSH 0x1
006d87e1  8d45ec                LEA EAX,[EBP + -0x14]
006d87e4  50                    PUSH EAX
006d87e5  6a03                  PUSH 0x3
006d87e7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006d87ea  0fb7516c              MOVZX EDX,word ptr [ECX + 0x6c]
006d87ee  52                    PUSH EDX
006d87ef  6a11                  PUSH 0x11
006d87f1  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d87f7  e8a484e6ff            CALL 0x00540ca0
006d87fc  6a00                  PUSH 0x0
006d87fe  6a00                  PUSH 0x0
006d8800  6a00                  PUSH 0x0
006d8802  6a00                  PUSH 0x0
006d8804  6a00                  PUSH 0x0
006d8806  6a00                  PUSH 0x0
006d8808  6a00                  PUSH 0x0
006d880a  6a00                  PUSH 0x0
006d880c  6a00                  PUSH 0x0
006d880e  6a00                  PUSH 0x0
006d8810  6a00                  PUSH 0x0
006d8812  6a00                  PUSH 0x0
006d8814  6a00                  PUSH 0x0
006d8816  6a00                  PUSH 0x0
006d8818  6a00                  PUSH 0x0
006d881a  6a00                  PUSH 0x0
006d881c  6a00                  PUSH 0x0
006d881e  6a00                  PUSH 0x0
006d8820  6a00                  PUSH 0x0
006d8822  6a00                  PUSH 0x0
006d8824  6a00                  PUSH 0x0
006d8826  6a00                  PUSH 0x0
006d8828  6a00                  PUSH 0x0
006d882a  6a00                  PUSH 0x0
006d882c  8d45ec                LEA EAX,[EBP + -0x14]
006d882f  50                    PUSH EAX
006d8830  6a03                  PUSH 0x3
006d8832  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006d8835  0fb7516c              MOVZX EDX,word ptr [ECX + 0x6c]
006d8839  52                    PUSH EDX
006d883a  6a13                  PUSH 0x13
006d883c  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8842  e85984e6ff            CALL 0x00540ca0
006d8847  83bd98feffff00        CMP dword ptr [EBP + 0xfffffe98],0x0
006d884e  0f846e010000          JZ 0x006d89c2
006d8854  c7855cfeffff00000000  MOV dword ptr [EBP + 0xfffffe5c],0x0
006d885e  837db000              CMP dword ptr [EBP + -0x50],0x0
006d8862  742e                  JZ 0x006d8892
006d8864  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
006d8867  8b10                  MOV EDX,dword ptr [EAX]
006d8869  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d886c  8b4218                MOV EAX,dword ptr [EDX + 0x18]
006d886f  ffd0                  CALL EAX
006d8871  85c0                  TEST EAX,EAX
006d8873  741d                  JZ 0x006d8892
006d8875  d98550ffffff          FLD float ptr [EBP + 0xffffff50]
006d887b  dc1d68b99800          FCOMP double ptr [0x0098b968]
006d8881  dfe0                  FNSTSW AX
006d8883  f6c441                TEST AH,0x41
006d8886  750a                  JNZ 0x006d8892
006d8888  c7855cfeffff01000000  MOV dword ptr [EBP + 0xfffffe5c],0x1
006d8892  d9ee                  FLDZ
006d8894  d99da4fdffff          FSTP float ptr [EBP + 0xfffffda4]
006d889a  d9ee                  FLDZ
006d889c  d99da8fdffff          FSTP float ptr [EBP + 0xfffffda8]
006d88a2  d9ee                  FLDZ
006d88a4  d99dacfdffff          FSTP float ptr [EBP + 0xfffffdac]
006d88aa  6a00                  PUSH 0x0
006d88ac  6a00                  PUSH 0x0
006d88ae  6a00                  PUSH 0x0
006d88b0  8d8da4fdffff          LEA ECX,[EBP + 0xfffffda4]
006d88b6  51                    PUSH ECX
006d88b7  6a00                  PUSH 0x0
006d88b9  6a00                  PUSH 0x0
006d88bb  6a00                  PUSH 0x0
006d88bd  6a00                  PUSH 0x0
006d88bf  6a00                  PUSH 0x0
006d88c1  6a00                  PUSH 0x0
006d88c3  51                    PUSH ECX
006d88c4  d9ee                  FLDZ
006d88c6  d91c24                FSTP float ptr [ESP]
006d88c9  51                    PUSH ECX
006d88ca  d9ee                  FLDZ
006d88cc  d91c24                FSTP float ptr [ESP]
006d88cf  8b955cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe5c]
006d88d5  52                    PUSH EDX
006d88d6  680000007f            PUSH 0x7f000000
006d88db  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
006d88de  50                    PUSH EAX
006d88df  8d8d6cffffff          LEA ECX,[EBP + 0xffffff6c]
006d88e5  51                    PUSH ECX
006d88e6  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006d88e9  0fb7426c              MOVZX EAX,word ptr [EDX + 0x6c]
006d88ed  50                    PUSH EAX
006d88ee  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d88f4  e8b7bbe8ff            CALL 0x005644b0
006d88f9  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d88ff  8b91ac110000          MOV EDX,dword ptr [ECX + 0x11ac]
006d8905  89955cfcffff          MOV dword ptr [EBP + 0xfffffc5c],EDX
006d890b  83bd5cfcffff00        CMP dword ptr [EBP + 0xfffffc5c],0x0
006d8912  0f84a5000000          JZ 0x006d89bd
006d8918  d98544ffffff          FLD float ptr [EBP + 0xffffff44]
006d891e  dc1dd8c19800          FCOMP double ptr [0x0098c1d8]
006d8924  dfe0                  FNSTSW AX
006d8926  f6c441                TEST AH,0x41
006d8929  0f858e000000          JNZ 0x006d89bd
006d892f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
006d8934  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006d8937  898d58feffff          MOV dword ptr [EBP + 0xfffffe58],ECX
006d893d  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8943  8b4204                MOV EAX,dword ptr [EDX + 0x4]
006d8946  898558fcffff          MOV dword ptr [EBP + 0xfffffc58],EAX
006d894c  8b8d58fcffff          MOV ECX,dword ptr [EBP + 0xfffffc58]
006d8952  51                    PUSH ECX
006d8953  8b8d58feffff          MOV ECX,dword ptr [EBP + 0xfffffe58]
006d8959  e862750600            CALL 0x0073fec0
006d895e  50                    PUSH EAX
006d895f  8b8d58feffff          MOV ECX,dword ptr [EBP + 0xfffffe58]
006d8965  e826720600            CALL 0x0073fb90
006d896a  8bc8                  MOV ECX,EAX
006d896c  e86fe21000            CALL 0x007e6be0
006d8971  898554feffff          MOV dword ptr [EBP + 0xfffffe54],EAX
006d8977  83bd54feffff00        CMP dword ptr [EBP + 0xfffffe54],0x0
006d897e  7c3d                  JL 0x006d89bd
006d8980  8b8d58feffff          MOV ECX,dword ptr [EBP + 0xfffffe58]
006d8986  e805720600            CALL 0x0073fb90
006d898b  898554fcffff          MOV dword ptr [EBP + 0xfffffc54],EAX
006d8991  8b9554feffff          MOV EDX,dword ptr [EBP + 0xfffffe54]
006d8997  69d288000000          IMUL EDX,EDX,0x88
006d899d  8b8554fcffff          MOV EAX,dword ptr [EBP + 0xfffffc54]
006d89a3  8d4c1024              LEA ECX,[EAX + EDX*0x1 + 0x24]
006d89a7  898d50feffff          MOV dword ptr [EBP + 0xfffffe50],ECX
006d89ad  68eda50000            PUSH 0xa5ed
006d89b2  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
006d89b8  e863c71000            CALL 0x007e5120
006d89bd  e920020000            JMP 0x006d8be2
006d89c2  837dc400              CMP dword ptr [EBP + -0x3c],0x0
006d89c6  0f8516020000          JNZ 0x006d8be2
006d89cc  d945b4                FLD float ptr [EBP + -0x4c]
006d89cf  d99d4cfeffff          FSTP float ptr [EBP + 0xfffffe4c]
006d89d5  83bd9cfeffff00        CMP dword ptr [EBP + 0xfffffe9c],0x0
006d89dc  7409                  JZ 0x006d89e7
006d89de  d945d4                FLD float ptr [EBP + -0x2c]
006d89e1  d99d4cfeffff          FSTP float ptr [EBP + 0xfffffe4c]
006d89e7  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
006d89ea  8b02                  MOV EAX,dword ptr [EDX]
006d89ec  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d89ef  8b5030                MOV EDX,dword ptr [EAX + 0x30]
006d89f2  ffd2                  CALL EDX
006d89f4  85c0                  TEST EAX,EAX
006d89f6  0f8482000000          JZ 0x006d8a7e
006d89fc  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
006d89ff  8b10                  MOV EDX,dword ptr [EAX]
006d8a01  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8a04  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006d8a07  ffd0                  CALL EAX
006d8a09  898550fcffff          MOV dword ptr [EBP + 0xfffffc50],EAX
006d8a0f  8b8d50fcffff          MOV ECX,dword ptr [EBP + 0xfffffc50]
006d8a15  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
006d8a1b  89954cfcffff          MOV dword ptr [EBP + 0xfffffc4c],EDX
006d8a21  8b854cfcffff          MOV EAX,dword ptr [EBP + 0xfffffc4c]
006d8a27  d94008                FLD float ptr [EAX + 0x8]
006d8a2a  d99d48fcffff          FSTP float ptr [EBP + 0xfffffc48]
006d8a30  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8a36  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
006d8a3c  899544fcffff          MOV dword ptr [EBP + 0xfffffc44],EDX
006d8a42  d98548fcffff          FLD float ptr [EBP + 0xfffffc48]
006d8a48  d86db4                FSUBR float ptr [EBP + -0x4c]
006d8a4b  8b8544fcffff          MOV EAX,dword ptr [EBP + 0xfffffc44]
006d8a51  d9580c                FSTP float ptr [EAX + 0xc]
006d8a54  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8a5a  8b9180030000          MOV EDX,dword ptr [ECX + 0x380]
006d8a60  899540fcffff          MOV dword ptr [EBP + 0xfffffc40],EDX
006d8a66  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d8a69  89853cfcffff          MOV dword ptr [EBP + 0xfffffc3c],EAX
006d8a6f  8b8d40fcffff          MOV ECX,dword ptr [EBP + 0xfffffc40]
006d8a75  8b953cfcffff          MOV EDX,dword ptr [EBP + 0xfffffc3c]
006d8a7b  895138                MOV dword ptr [ECX + 0x38],EDX
006d8a7e  83bd8cfeffff0b        CMP dword ptr [EBP + 0xfffffe8c],0xb
006d8a85  7409                  JZ 0x006d8a90
006d8a87  83bd8cfeffff0c        CMP dword ptr [EBP + 0xfffffe8c],0xc
006d8a8e  7521                  JNZ 0x006d8ab1
006d8a90  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8a96  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006d8a99  898d38fcffff          MOV dword ptr [EBP + 0xfffffc38],ECX
006d8a9f  8b9594feffff          MOV EDX,dword ptr [EBP + 0xfffffe94]
006d8aa5  3b9538fcffff          CMP EDX,dword ptr [EBP + 0xfffffc38]
006d8aab  0f8531010000          JNZ 0x006d8be2
006d8ab1  d9ee                  FLDZ
006d8ab3  d99d98fdffff          FSTP float ptr [EBP + 0xfffffd98]
006d8ab9  d9ee                  FLDZ
006d8abb  d99d9cfdffff          FSTP float ptr [EBP + 0xfffffd9c]
006d8ac1  d9ee                  FLDZ
006d8ac3  d99da0fdffff          FSTP float ptr [EBP + 0xfffffda0]
006d8ac9  8b859cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe9c]
006d8acf  50                    PUSH EAX
006d8ad0  6a00                  PUSH 0x0
006d8ad2  6a00                  PUSH 0x0
006d8ad4  8d8d98fdffff          LEA ECX,[EBP + 0xfffffd98]
006d8ada  51                    PUSH ECX
006d8adb  6a00                  PUSH 0x0
006d8add  6a00                  PUSH 0x0
006d8adf  6a00                  PUSH 0x0
006d8ae1  6a00                  PUSH 0x0
006d8ae3  6a00                  PUSH 0x0
006d8ae5  6a00                  PUSH 0x0
006d8ae7  51                    PUSH ECX
006d8ae8  d9ee                  FLDZ
006d8aea  d91c24                FSTP float ptr [ESP]
006d8aed  51                    PUSH ECX
006d8aee  d9854cfeffff          FLD float ptr [EBP + 0xfffffe4c]
006d8af4  d91c24                FSTP float ptr [ESP]
006d8af7  8b9590feffff          MOV EDX,dword ptr [EBP + 0xfffffe90]
006d8afd  52                    PUSH EDX
006d8afe  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d8b01  50                    PUSH EAX
006d8b02  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
006d8b05  51                    PUSH ECX
006d8b06  8d55f4                LEA EDX,[EBP + -0xc]
006d8b09  52                    PUSH EDX
006d8b0a  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d8b0d  0fb7486c              MOVZX ECX,word ptr [EAX + 0x6c]
006d8b11  51                    PUSH ECX
006d8b12  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8b18  e893b9e8ff            CALL 0x005644b0
006d8b1d  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8b23  8b82ac110000          MOV EAX,dword ptr [EDX + 0x11ac]
006d8b29  898534fcffff          MOV dword ptr [EBP + 0xfffffc34],EAX
006d8b2f  83bd34fcffff00        CMP dword ptr [EBP + 0xfffffc34],0x0
006d8b36  0f84a6000000          JZ 0x006d8be2
006d8b3c  d98544ffffff          FLD float ptr [EBP + 0xffffff44]
006d8b42  dc1dd8c19800          FCOMP double ptr [0x0098c1d8]
006d8b48  dfe0                  FNSTSW AX
006d8b4a  f6c441                TEST AH,0x41
006d8b4d  0f858f000000          JNZ 0x006d8be2
006d8b53  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006d8b59  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d8b5c  899548feffff          MOV dword ptr [EBP + 0xfffffe48],EDX
006d8b62  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8b68  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006d8b6b  898d30fcffff          MOV dword ptr [EBP + 0xfffffc30],ECX
006d8b71  8b9530fcffff          MOV EDX,dword ptr [EBP + 0xfffffc30]
006d8b77  52                    PUSH EDX
006d8b78  8b8d48feffff          MOV ECX,dword ptr [EBP + 0xfffffe48]
006d8b7e  e83d730600            CALL 0x0073fec0
006d8b83  50                    PUSH EAX
006d8b84  8b8d48feffff          MOV ECX,dword ptr [EBP + 0xfffffe48]
006d8b8a  e801700600            CALL 0x0073fb90
006d8b8f  8bc8                  MOV ECX,EAX
006d8b91  e84ae01000            CALL 0x007e6be0
006d8b96  898544feffff          MOV dword ptr [EBP + 0xfffffe44],EAX
006d8b9c  83bd44feffff00        CMP dword ptr [EBP + 0xfffffe44],0x0
006d8ba3  7c3d                  JL 0x006d8be2
006d8ba5  8b8d48feffff          MOV ECX,dword ptr [EBP + 0xfffffe48]
006d8bab  e8e06f0600            CALL 0x0073fb90
006d8bb0  89852cfcffff          MOV dword ptr [EBP + 0xfffffc2c],EAX
006d8bb6  8b8544feffff          MOV EAX,dword ptr [EBP + 0xfffffe44]
006d8bbc  69c088000000          IMUL EAX,EAX,0x88
006d8bc2  8b8d2cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc2c]
006d8bc8  8d540124              LEA EDX,[ECX + EAX*0x1 + 0x24]
006d8bcc  899540feffff          MOV dword ptr [EBP + 0xfffffe40],EDX
006d8bd2  68eda50000            PUSH 0xa5ed
006d8bd7  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006d8bdd  e83ec51000            CALL 0x007e5120
006d8be2  a1b8449900            MOV EAX,[0x009944b8]
006d8be7  e90f080000            JMP 0x006d93fb
006d8bec  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8bf2  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d8bf8  898d28fcffff          MOV dword ptr [EBP + 0xfffffc28],ECX
006d8bfe  83bd28fcffff00        CMP dword ptr [EBP + 0xfffffc28],0x0
006d8c05  7517                  JNZ 0x006d8c1e
006d8c07  6a01                  PUSH 0x1
006d8c09  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8c0f  e80c1f0000            CALL 0x006dab20
006d8c14  a1bc449900            MOV EAX,[0x009944bc]
006d8c19  e9dd070000            JMP 0x006d93fb
006d8c1e  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
006d8c24  8b4208                MOV EAX,dword ptr [EDX + 0x8]
006d8c27  898524fcffff          MOV dword ptr [EBP + 0xfffffc24],EAX
006d8c2d  6a02                  PUSH 0x2
006d8c2f  8b8d24fcffff          MOV ECX,dword ptr [EBP + 0xfffffc24]
006d8c35  e8e63ae4ff            CALL 0x0051c720
006d8c3a  85c0                  TEST EAX,EAX
006d8c3c  752d                  JNZ 0x006d8c6b
006d8c3e  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8c44  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d8c4a  899520fcffff          MOV dword ptr [EBP + 0xfffffc20],EDX
006d8c50  8b8520fcffff          MOV EAX,dword ptr [EBP + 0xfffffc20]
006d8c56  8b88a80a0000          MOV ECX,dword ptr [EAX + 0xaa8]
006d8c5c  898d1cfcffff          MOV dword ptr [EBP + 0xfffffc1c],ECX
006d8c62  83bd1cfcffff00        CMP dword ptr [EBP + 0xfffffc1c],0x0
006d8c69  740a                  JZ 0x006d8c75
006d8c6b  a1b4449900            MOV EAX,[0x009944b4]
006d8c70  e986070000            JMP 0x006d93fb
006d8c75  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8c7b  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d8c81  898518fcffff          MOV dword ptr [EBP + 0xfffffc18],EAX
006d8c87  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d8c8a  51                    PUSH ECX
006d8c8b  8b8d18fcffff          MOV ECX,dword ptr [EBP + 0xfffffc18]
006d8c91  e85a83ebff            CALL 0x00590ff0
006d8c96  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
006d8c9c  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8ca2  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d8ca8  898514fcffff          MOV dword ptr [EBP + 0xfffffc14],EAX
006d8cae  8d8d60ffffff          LEA ECX,[EBP + 0xffffff60]
006d8cb4  51                    PUSH ECX
006d8cb5  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d8cb8  52                    PUSH EDX
006d8cb9  8b8d14fcffff          MOV ECX,dword ptr [EBP + 0xfffffc14]
006d8cbf  e8ec85ebff            CALL 0x005912b0
006d8cc4  898548ffffff          MOV dword ptr [EBP + 0xffffff48],EAX
006d8cca  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
006d8cd1  0f8410010000          JZ 0x006d8de7
006d8cd7  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8cdd  e82e070000            CALL 0x006d9410
006d8ce2  85c0                  TEST EAX,EAX
006d8ce4  7529                  JNZ 0x006d8d0f
006d8ce6  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
006d8ce9  8b10                  MOV EDX,dword ptr [EAX]
006d8ceb  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8cee  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006d8cf1  ffd0                  CALL EAX
006d8cf3  85c0                  TEST EAX,EAX
006d8cf5  741f                  JZ 0x006d8d16
006d8cf7  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8cfa  8b11                  MOV EDX,dword ptr [ECX]
006d8cfc  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d8cff  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006d8d02  ffd0                  CALL EAX
006d8d04  8bc8                  MOV ECX,EAX
006d8d06  e805070000            CALL 0x006d9410
006d8d0b  85c0                  TEST EAX,EAX
006d8d0d  7407                  JZ 0x006d8d16
006d8d0f  c745f07d270000        MOV dword ptr [EBP + -0x10],0x277d
006d8d16  83bd48ffffff00        CMP dword ptr [EBP + 0xffffff48],0x0
006d8d1d  0f84c2000000          JZ 0x006d8de5
006d8d23  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8d29  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d8d2f  899510fcffff          MOV dword ptr [EBP + 0xfffffc10],EDX
006d8d35  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d8d38  50                    PUSH EAX
006d8d39  0fb74dcc              MOVZX ECX,word ptr [EBP + -0x34]
006d8d3d  51                    PUSH ECX
006d8d3e  8b9548ffffff          MOV EDX,dword ptr [EBP + 0xffffff48]
006d8d44  52                    PUSH EDX
006d8d45  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
006d8d4b  50                    PUSH EAX
006d8d4c  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d8d4f  51                    PUSH ECX
006d8d50  8b8d10fcffff          MOV ECX,dword ptr [EBP + 0xfffffc10]
006d8d56  e8d559ebff            CALL 0x0058e730
006d8d5b  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
006d8d61  8b4208                MOV EAX,dword ptr [EDX + 0x8]
006d8d64  89850cfcffff          MOV dword ptr [EBP + 0xfffffc0c],EAX
006d8d6a  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d8d6d  51                    PUSH ECX
006d8d6e  8b8d0cfcffff          MOV ECX,dword ptr [EBP + 0xfffffc0c]
006d8d74  e88733e4ff            CALL 0x0051c100
006d8d79  89853cfeffff          MOV dword ptr [EBP + 0xfffffe3c],EAX
006d8d7f  83bd3cfeffff00        CMP dword ptr [EBP + 0xfffffe3c],0x0
006d8d86  745d                  JZ 0x006d8de5
006d8d88  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8d8e  8b4204                MOV EAX,dword ptr [EDX + 0x4]
006d8d91  898508fcffff          MOV dword ptr [EBP + 0xfffffc08],EAX
006d8d97  8b8d3cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe3c]
006d8d9d  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d8da3  899504fcffff          MOV dword ptr [EBP + 0xfffffc04],EDX
006d8da9  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d8dac  50                    PUSH EAX
006d8dad  6a00                  PUSH 0x0
006d8daf  33c9                  XOR ECX,ECX
006d8db1  83bd48ffffff00        CMP dword ptr [EBP + 0xffffff48],0x0
006d8db8  0f94c1                SETZ CL
006d8dbb  51                    PUSH ECX
006d8dbc  8b9560ffffff          MOV EDX,dword ptr [EBP + 0xffffff60]
006d8dc2  52                    PUSH EDX
006d8dc3  8b8508fcffff          MOV EAX,dword ptr [EBP + 0xfffffc08]
006d8dc9  50                    PUSH EAX
006d8dca  8b8d04fcffff          MOV ECX,dword ptr [EBP + 0xfffffc04]
006d8dd0  e85b59ebff            CALL 0x0058e730
006d8dd5  8b8d3cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe3c]
006d8ddb  c7811c05000001000000  MOV dword ptr [ECX + 0x51c],0x1
006d8de5  eb3f                  JMP 0x006d8e26
006d8de7  c745f019270000        MOV dword ptr [EBP + -0x10],0x2719
006d8dee  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8df4  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d8dfa  898500fcffff          MOV dword ptr [EBP + 0xfffffc00],EAX
006d8e00  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d8e03  51                    PUSH ECX
006d8e04  0fb755cc              MOVZX EDX,word ptr [EBP + -0x34]
006d8e08  52                    PUSH EDX
006d8e09  8b8548ffffff          MOV EAX,dword ptr [EBP + 0xffffff48]
006d8e0f  50                    PUSH EAX
006d8e10  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
006d8e16  51                    PUSH ECX
006d8e17  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d8e1a  52                    PUSH EDX
006d8e1b  8b8d00fcffff          MOV ECX,dword ptr [EBP + 0xfffffc00]
006d8e21  e80a59ebff            CALL 0x0058e730
006d8e26  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8e2c  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d8e32  898dfcfbffff          MOV dword ptr [EBP + 0xfffffbfc],ECX
006d8e38  8b95fcfbffff          MOV EDX,dword ptr [EBP + 0xfffffbfc]
006d8e3e  8b82840a0000          MOV EAX,dword ptr [EDX + 0xa84]
006d8e44  8985f8fbffff          MOV dword ptr [EBP + 0xfffffbf8],EAX
006d8e4a  83bdf8fbffff00        CMP dword ptr [EBP + 0xfffffbf8],0x0
006d8e51  750a                  JNZ 0x006d8e5d
006d8e53  a1b4449900            MOV EAX,[0x009944b4]
006d8e58  e99e050000            JMP 0x006d93fb
006d8e5d  837dc00b              CMP dword ptr [EBP + -0x40],0xb
006d8e61  740a                  JZ 0x006d8e6d
006d8e63  837dc400              CMP dword ptr [EBP + -0x3c],0x0
006d8e67  0f8492010000          JZ 0x006d8fff
006d8e6d  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8e73  668b91e80f0000        MOV DX,word ptr [ECX + 0xfe8]
006d8e7a  668995f6fbffff        MOV word ptr [EBP + 0xfffffbf6],DX
006d8e81  0fb785f6fbffff        MOVZX EAX,word ptr [EBP + 0xfffffbf6]
006d8e88  85c0                  TEST EAX,EAX
006d8e8a  0f856f010000          JNZ 0x006d8fff
006d8e90  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8e96  8b9150110000          MOV EDX,dword ptr [ECX + 0x1150]
006d8e9c  8995f0fbffff          MOV dword ptr [EBP + 0xfffffbf0],EDX
006d8ea2  6a10                  PUSH 0x10
006d8ea4  8b8df0fbffff          MOV ECX,dword ptr [EBP + 0xfffffbf0]
006d8eaa  e87177ffff            CALL 0x006d0620
006d8eaf  898538feffff          MOV dword ptr [EBP + 0xfffffe38],EAX
006d8eb5  83bd38feffff00        CMP dword ptr [EBP + 0xfffffe38],0x0
006d8ebc  7456                  JZ 0x006d8f14
006d8ebe  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8ec4  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d8eca  898decfbffff          MOV dword ptr [EBP + 0xfffffbec],ECX
006d8ed0  8b95ecfbffff          MOV EDX,dword ptr [EBP + 0xfffffbec]
006d8ed6  8a82bc0a0000          MOV AL,byte ptr [EDX + 0xabc]
006d8edc  8885ebfbffff          MOV byte ptr [EBP + 0xfffffbeb],AL
006d8ee2  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8ee8  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d8eee  8995e4fbffff          MOV dword ptr [EBP + 0xfffffbe4],EDX
006d8ef4  0fb685ebfbffff        MOVZX EAX,byte ptr [EBP + 0xfffffbeb]
006d8efb  50                    PUSH EAX
006d8efc  8b8de4fbffff          MOV ECX,dword ptr [EBP + 0xfffffbe4]
006d8f02  e8b979ebff            CALL 0x005908c0
006d8f07  668b4dcc              MOV CX,word ptr [EBP + -0x34]
006d8f0b  66894864              MOV word ptr [EAX + 0x64],CX
006d8f0f  e9eb000000            JMP 0x006d8fff
006d8f14  837dc00b              CMP dword ptr [EBP + -0x40],0xb
006d8f18  7562                  JNZ 0x006d8f7c
006d8f1a  0fb755cc              MOVZX EDX,word ptr [EBP + -0x34]
006d8f1e  81fadc000000          CMP EDX,0xdc
006d8f24  7556                  JNZ 0x006d8f7c
006d8f26  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d8f2c  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d8f32  898de0fbffff          MOV dword ptr [EBP + 0xfffffbe0],ECX
006d8f38  8b95e0fbffff          MOV EDX,dword ptr [EBP + 0xfffffbe0]
006d8f3e  8a82bc0a0000          MOV AL,byte ptr [EDX + 0xabc]
006d8f44  8885dffbffff          MOV byte ptr [EBP + 0xfffffbdf],AL
006d8f4a  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8f50  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d8f56  8995d8fbffff          MOV dword ptr [EBP + 0xfffffbd8],EDX
006d8f5c  0fb685dffbffff        MOVZX EAX,byte ptr [EBP + 0xfffffbdf]
006d8f63  50                    PUSH EAX
006d8f64  8b8dd8fbffff          MOV ECX,dword ptr [EBP + 0xfffffbd8]
006d8f6a  e85179ebff            CALL 0x005908c0
006d8f6f  668b4dcc              MOV CX,word ptr [EBP + -0x34]
006d8f73  66894864              MOV word ptr [EAX + 0x64],CX
006d8f77  e983000000            JMP 0x006d8fff
006d8f7c  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8f82  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
006d8f88  8985d4fbffff          MOV dword ptr [EBP + 0xfffffbd4],EAX
006d8f8e  68cf000000            PUSH 0xcf
006d8f93  8b8dd4fbffff          MOV ECX,dword ptr [EBP + 0xfffffbd4]
006d8f99  e852f4fdff            CALL 0x006b83f0
006d8f9e  85c0                  TEST EAX,EAX
006d8fa0  745d                  JZ 0x006d8fff
006d8fa2  837dc00b              CMP dword ptr [EBP + -0x40],0xb
006d8fa6  7557                  JNZ 0x006d8fff
006d8fa8  837dc400              CMP dword ptr [EBP + -0x3c],0x0
006d8fac  7551                  JNZ 0x006d8fff
006d8fae  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d8fb4  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d8fba  8995d0fbffff          MOV dword ptr [EBP + 0xfffffbd0],EDX
006d8fc0  8b85d0fbffff          MOV EAX,dword ptr [EBP + 0xfffffbd0]
006d8fc6  8a88bc0a0000          MOV CL,byte ptr [EAX + 0xabc]
006d8fcc  888dcffbffff          MOV byte ptr [EBP + 0xfffffbcf],CL
006d8fd2  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d8fd8  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d8fde  8985c8fbffff          MOV dword ptr [EBP + 0xfffffbc8],EAX
006d8fe4  0fb68dcffbffff        MOVZX ECX,byte ptr [EBP + 0xfffffbcf]
006d8feb  51                    PUSH ECX
006d8fec  8b8dc8fbffff          MOV ECX,dword ptr [EBP + 0xfffffbc8]
006d8ff2  e8c978ebff            CALL 0x005908c0
006d8ff7  668b55cc              MOV DX,word ptr [EBP + -0x34]
006d8ffb  66895064              MOV word ptr [EAX + 0x64],DX
006d8fff  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
006d9002  50                    PUSH EAX
006d9003  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d9009  8b11                  MOV EDX,dword ptr [ECX]
006d900b  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d9011  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
006d9014  ffd0                  CALL EAX
006d9016  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d9019  51                    PUSH ECX
006d901a  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d9020  e8ab53eaff            CALL 0x0057e3d0
006d9025  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d902b  8b4204                MOV EAX,dword ptr [EDX + 0x4]
006d902e  8985c4fbffff          MOV dword ptr [EBP + 0xfffffbc4],EAX
006d9034  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d903a  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d9040  8995c0fbffff          MOV dword ptr [EBP + 0xfffffbc0],EDX
006d9046  8b85c4fbffff          MOV EAX,dword ptr [EBP + 0xfffffbc4]
006d904c  50                    PUSH EAX
006d904d  6a01                  PUSH 0x1
006d904f  8b8dc0fbffff          MOV ECX,dword ptr [EBP + 0xfffffbc0]
006d9055  e8066cebff            CALL 0x0058fc60
006d905a  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d9060  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d9066  8995bcfbffff          MOV dword ptr [EBP + 0xfffffbbc],EDX
006d906c  6a00                  PUSH 0x0
006d906e  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
006d9071  50                    PUSH EAX
006d9072  8b8dbcfbffff          MOV ECX,dword ptr [EBP + 0xfffffbbc]
006d9078  e8536cebff            CALL 0x0058fcd0
006d907d  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d9083  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d9089  8995b8fbffff          MOV dword ptr [EBP + 0xfffffbb8],EDX
006d908f  8b85b8fbffff          MOV EAX,dword ptr [EBP + 0xfffffbb8]
006d9095  8b88b80a0000          MOV ECX,dword ptr [EAX + 0xab8]
006d909b  898db4fbffff          MOV dword ptr [EBP + 0xfffffbb4],ECX
006d90a1  8b95b4fbffff          MOV EDX,dword ptr [EBP + 0xfffffbb4]
006d90a7  83c201                ADD EDX,0x1
006d90aa  8b85b8fbffff          MOV EAX,dword ptr [EBP + 0xfffffbb8]
006d90b0  8990b80a0000          MOV dword ptr [EAX + 0xab8],EDX
006d90b6  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
006d90bd  0f846c010000          JZ 0x006d922f
006d90c3  83bd48ffffff00        CMP dword ptr [EBP + 0xffffff48],0x0
006d90ca  0f84c6000000          JZ 0x006d9196
006d90d0  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d90d6  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d90dc  8995b0fbffff          MOV dword ptr [EBP + 0xfffffbb0],EDX
006d90e2  8b85b0fbffff          MOV EAX,dword ptr [EBP + 0xfffffbb0]
006d90e8  8b88140b0000          MOV ECX,dword ptr [EAX + 0xb14]
006d90ee  898dacfbffff          MOV dword ptr [EBP + 0xfffffbac],ECX
006d90f4  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
006d90fa  8b4208                MOV EAX,dword ptr [EDX + 0x8]
006d90fd  8985a8fbffff          MOV dword ptr [EBP + 0xfffffba8],EAX
006d9103  8b8dacfbffff          MOV ECX,dword ptr [EBP + 0xfffffbac]
006d9109  51                    PUSH ECX
006d910a  8b8da8fbffff          MOV ECX,dword ptr [EBP + 0xfffffba8]
006d9110  e8eb2fe4ff            CALL 0x0051c100
006d9115  898534feffff          MOV dword ptr [EBP + 0xfffffe34],EAX
006d911b  83bd34feffff00        CMP dword ptr [EBP + 0xfffffe34],0x0
006d9122  746d                  JZ 0x006d9191
006d9124  8b9534feffff          MOV EDX,dword ptr [EBP + 0xfffffe34]
006d912a  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d9130  8985a4fbffff          MOV dword ptr [EBP + 0xfffffba4],EAX
006d9136  8b8da4fbffff          MOV ECX,dword ptr [EBP + 0xfffffba4]
006d913c  8b91100b0000          MOV EDX,dword ptr [ECX + 0xb10]
006d9142  8995a0fbffff          MOV dword ptr [EBP + 0xfffffba0],EDX
006d9148  8b8534feffff          MOV EAX,dword ptr [EBP + 0xfffffe34]
006d914e  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d9154  898d9cfbffff          MOV dword ptr [EBP + 0xfffffb9c],ECX
006d915a  8b95a0fbffff          MOV EDX,dword ptr [EBP + 0xfffffba0]
006d9160  52                    PUSH EDX
006d9161  6a01                  PUSH 0x1
006d9163  8b8d9cfbffff          MOV ECX,dword ptr [EBP + 0xfffffb9c]
006d9169  e8f26aebff            CALL 0x0058fc60
006d916e  8b8534feffff          MOV EAX,dword ptr [EBP + 0xfffffe34]
006d9174  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d917a  898d98fbffff          MOV dword ptr [EBP + 0xfffffb98],ECX
006d9180  6a01                  PUSH 0x1
006d9182  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
006d9185  52                    PUSH EDX
006d9186  8b8d98fbffff          MOV ECX,dword ptr [EBP + 0xfffffb98]
006d918c  e83f6bebff            CALL 0x0058fcd0
006d9191  e999000000            JMP 0x006d922f
006d9196  a1a4b4a100            MOV EAX,[0x00a1b4a4]
006d919b  8b4808                MOV ECX,dword ptr [EAX + 0x8]
006d919e  898d94fbffff          MOV dword ptr [EBP + 0xfffffb94],ECX
006d91a4  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d91a7  52                    PUSH EDX
006d91a8  8b8d94fbffff          MOV ECX,dword ptr [EBP + 0xfffffb94]
006d91ae  e84d2fe4ff            CALL 0x0051c100
006d91b3  898530feffff          MOV dword ptr [EBP + 0xfffffe30],EAX
006d91b9  83bd30feffff00        CMP dword ptr [EBP + 0xfffffe30],0x0
006d91c0  746d                  JZ 0x006d922f
006d91c2  8b8530feffff          MOV EAX,dword ptr [EBP + 0xfffffe30]
006d91c8  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d91ce  898d90fbffff          MOV dword ptr [EBP + 0xfffffb90],ECX
006d91d4  8b9590fbffff          MOV EDX,dword ptr [EBP + 0xfffffb90]
006d91da  8b82140b0000          MOV EAX,dword ptr [EDX + 0xb14]
006d91e0  89858cfbffff          MOV dword ptr [EBP + 0xfffffb8c],EAX
006d91e6  8b8d30feffff          MOV ECX,dword ptr [EBP + 0xfffffe30]
006d91ec  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d91f2  899588fbffff          MOV dword ptr [EBP + 0xfffffb88],EDX
006d91f8  8b858cfbffff          MOV EAX,dword ptr [EBP + 0xfffffb8c]
006d91fe  50                    PUSH EAX
006d91ff  6a01                  PUSH 0x1
006d9201  8b8d88fbffff          MOV ECX,dword ptr [EBP + 0xfffffb88]
006d9207  e8546aebff            CALL 0x0058fc60
006d920c  8b8d30feffff          MOV ECX,dword ptr [EBP + 0xfffffe30]
006d9212  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d9218  899584fbffff          MOV dword ptr [EBP + 0xfffffb84],EDX
006d921e  6a00                  PUSH 0x0
006d9220  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
006d9223  50                    PUSH EAX
006d9224  8b8d84fbffff          MOV ECX,dword ptr [EBP + 0xfffffb84]
006d922a  e8a16aebff            CALL 0x0058fcd0
006d922f  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d9235  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d923b  899580fbffff          MOV dword ptr [EBP + 0xfffffb80],EDX
006d9241  8b8580fbffff          MOV EAX,dword ptr [EBP + 0xfffffb80]
006d9247  8b88dc0a0000          MOV ECX,dword ptr [EAX + 0xadc]
006d924d  898d7cfbffff          MOV dword ptr [EBP + 0xfffffb7c],ECX
006d9253  81bd7cfbffff0000007f  CMP dword ptr [EBP + 0xfffffb7c],0x7f000000
006d925d  0f84d6000000          JZ 0x006d9339
006d9263  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d9269  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d926f  898578fbffff          MOV dword ptr [EBP + 0xfffffb78],EAX
006d9275  8b8d78fbffff          MOV ECX,dword ptr [EBP + 0xfffffb78]
006d927b  8b91dc0a0000          MOV EDX,dword ptr [ECX + 0xadc]
006d9281  89952cfeffff          MOV dword ptr [EBP + 0xfffffe2c],EDX
006d9287  81bd2cfeffff0000007f  CMP dword ptr [EBP + 0xfffffe2c],0x7f000000
006d9291  744a                  JZ 0x006d92dd
006d9293  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d9296  50                    PUSH EAX
006d9297  8b8d2cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe2c]
006d929d  51                    PUSH ECX
006d929e  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d92a4  e8c7190000            CALL 0x006dac70
006d92a9  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d92af  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d92b5  898574fbffff          MOV dword ptr [EBP + 0xfffffb74],EAX
006d92bb  8b8d74fbffff          MOV ECX,dword ptr [EBP + 0xfffffb74]
006d92c1  c781dc0a00000000007f  MOV dword ptr [ECX + 0xadc],0x7f000000
006d92cb  8b952cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe2c]
006d92d1  8955ec                MOV dword ptr [EBP + -0x14],EDX
006d92d4  c745d001000000        MOV dword ptr [EBP + -0x30],0x1
006d92db  eb5c                  JMP 0x006d9339
006d92dd  8b8564fbffff          MOV EAX,dword ptr [EBP + 0xfffffb64]
006d92e3  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d92e9  898d70fbffff          MOV dword ptr [EBP + 0xfffffb70],ECX
006d92ef  680000007f            PUSH 0x7f000000
006d92f4  6a00                  PUSH 0x0
006d92f6  8b8d70fbffff          MOV ECX,dword ptr [EBP + 0xfffffb70]
006d92fc  e85f69ebff            CALL 0x0058fc60
006d9301  8b9564fbffff          MOV EDX,dword ptr [EBP + 0xfffffb64]
006d9307  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d930d  89856cfbffff          MOV dword ptr [EBP + 0xfffffb6c],EAX
006d9313  6a00                  PUSH 0x0
006d9315  6a00                  PUSH 0x0
006d9317  8b8d6cfbffff          MOV ECX,dword ptr [EBP + 0xfffffb6c]
006d931d  e8ae69ebff            CALL 0x0058fcd0
006d9322  6a01                  PUSH 0x1
006d9324  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d932a  e8f1170000            CALL 0x006dab20
006d932f  a1b8449900            MOV EAX,[0x009944b8]
006d9334  e9c2000000            JMP 0x006d93fb
006d9339  6a00                  PUSH 0x0
006d933b  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d933e  51                    PUSH ECX
006d933f  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d9345  e82640eaff            CALL 0x0057d370
006d934a  c78540ffffff00000000  MOV dword ptr [EBP + 0xffffff40],0x0
006d9354  83bd68ffffff00        CMP dword ptr [EBP + 0xffffff68],0x0
006d935b  7521                  JNZ 0x006d937e
006d935d  682ca99900            PUSH 0x99a92c
006d9362  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d9368  83c118                ADD ECX,0x18
006d936b  e8f0bb0500            CALL 0x00734f60
006d9370  85c0                  TEST EAX,EAX
006d9372  740a                  JZ 0x006d937e
006d9374  c78540ffffff01000000  MOV dword ptr [EBP + 0xffffff40],0x1
006d937e  83bd68ffffff00        CMP dword ptr [EBP + 0xffffff68],0x0
006d9385  7548                  JNZ 0x006d93cf
006d9387  83bd68ffffff00        CMP dword ptr [EBP + 0xffffff68],0x0
006d938e  7566                  JNZ 0x006d93f6
006d9390  83bd40ffffff00        CMP dword ptr [EBP + 0xffffff40],0x0
006d9397  755d                  JNZ 0x006d93f6
006d9399  d945d4                FLD float ptr [EBP + -0x2c]
006d939c  dc05f07d9800          FADD double ptr [0x00987df0]
006d93a2  d99d68fbffff          FSTP float ptr [EBP + 0xfffffb68]
006d93a8  d98544ffffff          FLD float ptr [EBP + 0xffffff44]
006d93ae  d98568fbffff          FLD float ptr [EBP + 0xfffffb68]
006d93b4  d88d68fbffff          FMUL float ptr [EBP + 0xfffffb68]
006d93ba  d99d20fbffff          FSTP float ptr [EBP + 0xfffffb20]
006d93c0  d98520fbffff          FLD float ptr [EBP + 0xfffffb20]
006d93c6  ded9                  FCOMPP
006d93c8  dfe0                  FNSTSW AX
006d93ca  f6c441                TEST AH,0x41
006d93cd  7527                  JNZ 0x006d93f6
006d93cf  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
006d93d2  52                    PUSH EDX
006d93d3  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
006d93d6  50                    PUSH EAX
006d93d7  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
006d93da  51                    PUSH ECX
006d93db  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d93de  52                    PUSH EDX
006d93df  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
006d93e2  50                    PUSH EAX
006d93e3  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
006d93e6  51                    PUSH ECX
006d93e7  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006d93ea  52                    PUSH EDX
006d93eb  8b8d64fbffff          MOV ECX,dword ptr [EBP + 0xfffffb64]
006d93f1  e8fa180000            CALL 0x006dacf0
006d93f6  a1b8449900            MOV EAX,[0x009944b8]
006d93fb  8be5                  MOV ESP,EBP
006d93fd  5d                    POP EBP
006d93fe  c20400                RET 0x4
