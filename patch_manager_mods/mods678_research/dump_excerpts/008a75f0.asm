; FN 008a75f0 FUN_008a75f0
008a75f0  55                    PUSH EBP
008a75f1  8bec                  MOV EBP,ESP
008a75f3  6aff                  PUSH -0x1
008a75f5  68cb809700            PUSH 0x9780cb
008a75fa  64a100000000          MOV EAX,FS:[0x0]
008a7600  50                    PUSH EAX
008a7601  81ecac000000          SUB ESP,0xac
008a7607  56                    PUSH ESI
008a7608  a1201fa100            MOV EAX,[0x00a11f20]
008a760d  33c5                  XOR EAX,EBP
008a760f  50                    PUSH EAX
008a7610  8d45f4                LEA EAX,[EBP + -0xc]
008a7613  64a300000000          MOV FS:[0x0],EAX
008a7619  898d50ffffff          MOV dword ptr [EBP + 0xffffff50],ECX
008a761f  c745f001000000        MOV dword ptr [EBP + -0x10],0x1
008a7626  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
008a762d  8b450c                MOV EAX,dword ptr [EBP + 0xc]
008a7630  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a7636  8b10                  MOV EDX,dword ptr [EAX]
008a7638  3b91b4240000          CMP EDX,dword ptr [ECX + 0x24b4]
008a763e  0f851a010000          JNZ 0x008a775e
008a7644  68b4030000            PUSH 0x3b4
008a7649  e8d5200700            CALL 0x00919723
008a764e  83c404                ADD ESP,0x4
008a7651  8945c8                MOV dword ptr [EBP + -0x38],EAX
008a7654  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a765b  837dc800              CMP dword ptr [EBP + -0x38],0x0
008a765f  7410                  JZ 0x008a7671
008a7661  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008a7664  e8f7930000            CALL 0x008b0a60
008a7669  89854cffffff          MOV dword ptr [EBP + 0xffffff4c],EAX
008a766f  eb0a                  JMP 0x008a767b
008a7671  c7854cffffff00000000  MOV dword ptr [EBP + 0xffffff4c],0x0
008a767b  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
008a7681  8945cc                MOV dword ptr [EBP + -0x34],EAX
008a7684  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008a768b  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
008a768e  894dec                MOV dword ptr [EBP + -0x14],ECX
008a7691  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008a7694  52                    PUSH EDX
008a7695  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a769b  81c1b0240000          ADD ECX,0x24b0
008a76a1  e8ba73f9ff            CALL 0x0083ea60
008a76a6  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
008a76a9  898548ffffff          MOV dword ptr [EBP + 0xffffff48],EAX
008a76af  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a76b5  8b9164080000          MOV EDX,dword ptr [ECX + 0x864]
008a76bb  899548ffffff          MOV dword ptr [EBP + 0xffffff48],EDX
008a76c1  8b8548ffffff          MOV EAX,dword ptr [EBP + 0xffffff48]
008a76c7  8b10                  MOV EDX,dword ptr [EAX]
008a76c9  8b8d48ffffff          MOV ECX,dword ptr [EBP + 0xffffff48]
008a76cf  8b4258                MOV EAX,dword ptr [EDX + 0x58]
008a76d2  ffd0                  CALL EAX
008a76d4  8945e8                MOV dword ptr [EBP + -0x18],EAX
008a76d7  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a76dd  81c188050000          ADD ECX,0x588
008a76e3  894dc0                MOV dword ptr [EBP + -0x40],ECX
008a76e6  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008a76e9  0fb682d0020000        MOVZX EAX,byte ptr [EDX + 0x2d0]
008a76f0  d1e0                  SHL EAX,0x1
008a76f2  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008a76f5  8b91a4020000          MOV EDX,dword ptr [ECX + 0x2a4]
008a76fb  2bd0                  SUB EDX,EAX
008a76fd  8955e4                MOV dword ptr [EBP + -0x1c],EDX
008a7700  6a01                  PUSH 0x1
008a7702  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
008a7705  50                    PUSH EAX
008a7706  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008a7709  81c1e8000000          ADD ECX,0xe8
008a770f  51                    PUSH ECX
008a7710  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008a7713  83c270                ADD EDX,0x70
008a7716  52                    PUSH EDX
008a7717  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008a771a  0560010000            ADD EAX,0x160
008a771f  50                    PUSH EAX
008a7720  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7723  8b11                  MOV EDX,dword ptr [ECX]
008a7725  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7728  8b82a0000000          MOV EAX,dword ptr [EDX + 0xa0]
008a772e  ffd0                  CALL EAX
008a7730  6800818a00            PUSH 0x8a8100
008a7735  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a773b  51                    PUSH ECX
008a773c  6a00                  PUSH 0x0
008a773e  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7741  e8aa13b7ff            CALL 0x00418af0
008a7746  68d0838a00            PUSH 0x8a83d0
008a774b  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
008a7751  52                    PUSH EDX
008a7752  6a01                  PUSH 0x1
008a7754  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7757  e89413b7ff            CALL 0x00418af0
008a775c  eb17                  JMP 0x008a7775
008a775e  8b450c                MOV EAX,dword ptr [EBP + 0xc]
008a7761  8b08                  MOV ECX,dword ptr [EAX]
008a7763  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
008a7769  8b82b0240000          MOV EAX,dword ptr [EDX + 0x24b0]
008a776f  8b0c88                MOV ECX,dword ptr [EAX + ECX*0x4]
008a7772  894dec                MOV dword ptr [EBP + -0x14],ECX
008a7775  68ff000000            PUSH 0xff
008a777a  8b5518                MOV EDX,dword ptr [EBP + 0x18]
008a777d  52                    PUSH EDX
008a777e  8b4514                MOV EAX,dword ptr [EBP + 0x14]
008a7781  50                    PUSH EAX
008a7782  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a7785  e8e63eedff            CALL 0x0077b670
008a778a  85c0                  TEST EAX,EAX
008a778c  0f8425030000          JZ 0x008a7ab7
008a7792  8b4d14                MOV ECX,dword ptr [EBP + 0x14]
008a7795  e896f6e2ff            CALL 0x006d6e30
008a779a  8945bc                MOV dword ptr [EBP + -0x44],EAX
008a779d  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
008a77a0  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
008a77a6  8855bb                MOV byte ptr [EBP + -0x45],DL
008a77a9  0fb645bb              MOVZX EAX,byte ptr [EBP + -0x45]
008a77ad  8945e0                MOV dword ptr [EBP + -0x20],EAX
008a77b0  837de02d              CMP dword ptr [EBP + -0x20],0x2d
008a77b4  7406                  JZ 0x008a77bc
008a77b6  837de01a              CMP dword ptr [EBP + -0x20],0x1a
008a77ba  757e                  JNZ 0x008a783a
008a77bc  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a77bf  8b9110030000          MOV EDX,dword ptr [ECX + 0x310]
008a77c5  8955b4                MOV dword ptr [EBP + -0x4c],EDX
008a77c8  8b45b4                MOV EAX,dword ptr [EBP + -0x4c]
008a77cb  668b484c              MOV CX,word ptr [EAX + 0x4c]
008a77cf  66894db2              MOV word ptr [EBP + -0x4e],CX
008a77d3  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008a77d6  8b8210030000          MOV EAX,dword ptr [EDX + 0x310]
008a77dc  8945ac                MOV dword ptr [EBP + -0x54],EAX
008a77df  0fbf75b2              MOVSX ESI,word ptr [EBP + -0x4e]
008a77e3  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
008a77e6  e80515faff            CALL 0x00848cf0
008a77eb  0fbfc8                MOVSX ECX,AX
008a77ee  3bf1                  CMP ESI,ECX
008a77f0  7c48                  JL 0x008a783a
008a77f2  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a77f5  e80660edff            CALL 0x0077d800
008a77fa  8945dc                MOV dword ptr [EBP + -0x24],EAX
008a77fd  837ddc00              CMP dword ptr [EBP + -0x24],0x0
008a7801  7412                  JZ 0x008a7815
008a7803  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008a7806  8b82f4100000          MOV EAX,dword ptr [EDX + 0x10f4]
008a780c  8945a8                MOV dword ptr [EBP + -0x58],EAX
008a780f  837da800              CMP dword ptr [EBP + -0x58],0x0
008a7813  7520                  JNZ 0x008a7835
008a7815  6860878a00            PUSH 0x8a8760
008a781a  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a7820  51                    PUSH ECX
008a7821  6a27                  PUSH 0x27
008a7823  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7826  e8c512b7ff            CALL 0x00418af0
008a782b  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008a782e  c7425c00000000        MOV dword ptr [EDX + 0x5c],0x0
008a7835  e97b020000            JMP 0x008a7ab5
008a783a  837de02f              CMP dword ptr [EBP + -0x20],0x2f
008a783e  0f859d010000          JNZ 0x008a79e1
008a7844  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a7847  8b8810030000          MOV ECX,dword ptr [EAX + 0x310]
008a784d  894da4                MOV dword ptr [EBP + -0x5c],ECX
008a7850  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
008a7853  668b4224              MOV AX,word ptr [EDX + 0x24]
008a7857  668945a2              MOV word ptr [EBP + -0x5e],AX
008a785b  0fb74da2              MOVZX ECX,word ptr [EBP + -0x5e]
008a785f  0fb715b8a89900        MOVZX EDX,word ptr [0x0099a8b8]
008a7866  3bca                  CMP ECX,EDX
008a7868  0f8473010000          JZ 0x008a79e1
008a786e  c745d000000000        MOV dword ptr [EBP + -0x30],0x0
008a7875  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a7878  8b8810030000          MOV ECX,dword ptr [EAX + 0x310]
008a787e  894d9c                MOV dword ptr [EBP + -0x64],ECX
008a7881  8b559c                MOV EDX,dword ptr [EBP + -0x64]
008a7884  668b424c              MOV AX,word ptr [EDX + 0x4c]
008a7888  6689459a              MOV word ptr [EBP + -0x66],AX
008a788c  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a788f  8b9110030000          MOV EDX,dword ptr [ECX + 0x310]
008a7895  895594                MOV dword ptr [EBP + -0x6c],EDX
008a7898  0fbf759a              MOVSX ESI,word ptr [EBP + -0x66]
008a789c  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
008a789f  e84c14faff            CALL 0x00848cf0
008a78a4  98                    CWDE
008a78a5  3bf0                  CMP ESI,EAX
008a78a7  7d09                  JGE 0x008a78b2
008a78a9  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
008a78ac  83c901                OR ECX,0x1
008a78af  894dd0                MOV dword ptr [EBP + -0x30],ECX
008a78b2  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
008a78b8  8b4204                MOV EAX,dword ptr [EDX + 0x4]
008a78bb  894590                MOV dword ptr [EBP + -0x70],EAX
008a78be  6a01                  PUSH 0x1
008a78c0  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
008a78c3  e8c882e9ff            CALL 0x0073fb90
008a78c8  8bc8                  MOV ECX,EAX
008a78ca  e8d1e4f3ff            CALL 0x007e5da0
008a78cf  8945d8                MOV dword ptr [EBP + -0x28],EAX
008a78d2  837dd800              CMP dword ptr [EBP + -0x28],0x0
008a78d6  744b                  JZ 0x008a7923
008a78d8  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008a78db  e8205fedff            CALL 0x0077d800
008a78e0  85c0                  TEST EAX,EAX
008a78e2  743f                  JZ 0x008a7923
008a78e4  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008a78e7  8b9110030000          MOV EDX,dword ptr [ECX + 0x310]
008a78ed  89558c                MOV dword ptr [EBP + -0x74],EDX
008a78f0  8b458c                MOV EAX,dword ptr [EBP + -0x74]
008a78f3  668b484c              MOV CX,word ptr [EAX + 0x4c]
008a78f7  66894d8a              MOV word ptr [EBP + -0x76],CX
008a78fb  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
008a78fe  8b8210030000          MOV EAX,dword ptr [EDX + 0x310]
008a7904  894584                MOV dword ptr [EBP + -0x7c],EAX
008a7907  0fbf758a              MOVSX ESI,word ptr [EBP + -0x76]
008a790b  8b4d84                MOV ECX,dword ptr [EBP + -0x7c]
008a790e  e8dd13faff            CALL 0x00848cf0
008a7913  0fbfc8                MOVSX ECX,AX
008a7916  3bf1                  CMP ESI,ECX
008a7918  7d09                  JGE 0x008a7923
008a791a  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
008a791d  83ca01                OR EDX,0x1
008a7920  8955d0                MOV dword ptr [EBP + -0x30],EDX
008a7923  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008a7928  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008a792b  894d80                MOV dword ptr [EBP + -0x80],ECX
008a792e  6a02                  PUSH 0x2
008a7930  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
008a7933  e85882e9ff            CALL 0x0073fb90
008a7938  8bc8                  MOV ECX,EAX
008a793a  e861e4f3ff            CALL 0x007e5da0
008a793f  8945d8                MOV dword ptr [EBP + -0x28],EAX
008a7942  837dd800              CMP dword ptr [EBP + -0x28],0x0
008a7946  745d                  JZ 0x008a79a5
008a7948  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008a794b  e8b05eedff            CALL 0x0077d800
008a7950  85c0                  TEST EAX,EAX
008a7952  7451                  JZ 0x008a79a5
008a7954  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
008a7957  8b8210030000          MOV EAX,dword ptr [EDX + 0x310]
008a795d  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
008a7963  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7969  668b514c              MOV DX,word ptr [ECX + 0x4c]
008a796d  6689957affffff        MOV word ptr [EBP + 0xffffff7a],DX
008a7974  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
008a7977  8b8810030000          MOV ECX,dword ptr [EAX + 0x310]
008a797d  898d74ffffff          MOV dword ptr [EBP + 0xffffff74],ECX
008a7983  0fbfb57affffff        MOVSX ESI,word ptr [EBP + 0xffffff7a]
008a798a  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
008a7990  e85b13faff            CALL 0x00848cf0
008a7995  0fbfd0                MOVSX EDX,AX
008a7998  3bf2                  CMP ESI,EDX
008a799a  7d09                  JGE 0x008a79a5
008a799c  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
008a799f  83c801                OR EAX,0x1
008a79a2  8945d0                MOV dword ptr [EBP + -0x30],EAX
008a79a5  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a79a8  e8535eedff            CALL 0x0077d800
008a79ad  8945d4                MOV dword ptr [EBP + -0x2c],EAX
008a79b0  837dd000              CMP dword ptr [EBP + -0x30],0x0
008a79b4  7406                  JZ 0x008a79bc
008a79b6  837dd400              CMP dword ptr [EBP + -0x2c],0x0
008a79ba  7520                  JNZ 0x008a79dc
008a79bc  68f0878a00            PUSH 0x8a87f0
008a79c1  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a79c7  51                    PUSH ECX
008a79c8  6a27                  PUSH 0x27
008a79ca  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a79cd  e81e11b7ff            CALL 0x00418af0
008a79d2  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008a79d5  c7425c00000000        MOV dword ptr [EDX + 0x5c],0x0
008a79dc  e9d4000000            JMP 0x008a7ab5
008a79e1  837de02d              CMP dword ptr [EBP + -0x20],0x2d
008a79e5  7554                  JNZ 0x008a7a3b
008a79e7  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a79ea  8b8810030000          MOV ECX,dword ptr [EAX + 0x310]
008a79f0  898d70ffffff          MOV dword ptr [EBP + 0xffffff70],ECX
008a79f6  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008a79fc  668b4224              MOV AX,word ptr [EDX + 0x24]
008a7a00  6689856effffff        MOV word ptr [EBP + 0xffffff6e],AX
008a7a07  0fb78d6effffff        MOVZX ECX,word ptr [EBP + 0xffffff6e]
008a7a0e  0fb715b8a89900        MOVZX EDX,word ptr [0x0099a8b8]
008a7a15  3bca                  CMP ECX,EDX
008a7a17  7522                  JNZ 0x008a7a3b
008a7a19  68d0868a00            PUSH 0x8a86d0
008a7a1e  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
008a7a24  50                    PUSH EAX
008a7a25  6a27                  PUSH 0x27
008a7a27  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7a2a  e8c110b7ff            CALL 0x00418af0
008a7a2f  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7a32  c7415c00000000        MOV dword ptr [ECX + 0x5c],0x0
008a7a39  eb7a                  JMP 0x008a7ab5
008a7a3b  837de01a              CMP dword ptr [EBP + -0x20],0x1a
008a7a3f  7554                  JNZ 0x008a7a95
008a7a41  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008a7a44  8b8210030000          MOV EAX,dword ptr [EDX + 0x310]
008a7a4a  898568ffffff          MOV dword ptr [EBP + 0xffffff68],EAX
008a7a50  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
008a7a56  668b5124              MOV DX,word ptr [ECX + 0x24]
008a7a5a  66899566ffffff        MOV word ptr [EBP + 0xffffff66],DX
008a7a61  0fb78566ffffff        MOVZX EAX,word ptr [EBP + 0xffffff66]
008a7a68  0fb70db8a89900        MOVZX ECX,word ptr [0x0099a8b8]
008a7a6f  3bc1                  CMP EAX,ECX
008a7a71  7422                  JZ 0x008a7a95
008a7a73  68d0868a00            PUSH 0x8a86d0
008a7a78  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
008a7a7e  52                    PUSH EDX
008a7a7f  6a27                  PUSH 0x27
008a7a81  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7a84  e86710b7ff            CALL 0x00418af0
008a7a89  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008a7a8c  c7405c00000000        MOV dword ptr [EAX + 0x5c],0x0
008a7a93  eb20                  JMP 0x008a7ab5
008a7a95  6880848a00            PUSH 0x8a8480
008a7a9a  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a7aa0  51                    PUSH ECX
008a7aa1  6a27                  PUSH 0x27
008a7aa3  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7aa6  e84510b7ff            CALL 0x00418af0
008a7aab  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008a7aae  c7425c01000000        MOV dword ptr [EDX + 0x5c],0x1
008a7ab5  eb68                  JMP 0x008a7b1f
008a7ab7  8b4d14                MOV ECX,dword ptr [EBP + 0x14]
008a7aba  e871f3e2ff            CALL 0x006d6e30
008a7abf  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
008a7ac5  8b8560ffffff          MOV EAX,dword ptr [EBP + 0xffffff60]
008a7acb  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008a7ace  898d5cffffff          MOV dword ptr [EBP + 0xffffff5c],ECX
008a7ad4  83bd5cffffff00        CMP dword ptr [EBP + 0xffffff5c],0x0
008a7adb  7422                  JZ 0x008a7aff
008a7add  6840868a00            PUSH 0x8a8640
008a7ae2  8b9550ffffff          MOV EDX,dword ptr [EBP + 0xffffff50]
008a7ae8  52                    PUSH EDX
008a7ae9  6a27                  PUSH 0x27
008a7aeb  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7aee  e8fd0fb7ff            CALL 0x00418af0
008a7af3  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008a7af6  c7405c00000000        MOV dword ptr [EAX + 0x5c],0x0
008a7afd  eb20                  JMP 0x008a7b1f
008a7aff  68d0868a00            PUSH 0x8a86d0
008a7b04  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008a7b0a  51                    PUSH ECX
008a7b0b  6a27                  PUSH 0x27
008a7b0d  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7b10  e8db0fb7ff            CALL 0x00418af0
008a7b15  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008a7b18  c7425c00000000        MOV dword ptr [EDX + 0x5c],0x0
008a7b1f  8b4514                MOV EAX,dword ptr [EBP + 0x14]
008a7b22  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
008a7b28  c1e907                SHR ECX,0x7
008a7b2b  83e101                AND ECX,0x1
008a7b2e  898d58ffffff          MOV dword ptr [EBP + 0xffffff58],ECX
008a7b34  8b5514                MOV EDX,dword ptr [EBP + 0x14]
008a7b37  8b4214                MOV EAX,dword ptr [EDX + 0x14]
008a7b3a  898554ffffff          MOV dword ptr [EBP + 0xffffff54],EAX
008a7b40  8b8d58ffffff          MOV ECX,dword ptr [EBP + 0xffffff58]
008a7b46  51                    PUSH ECX
008a7b47  8b551c                MOV EDX,dword ptr [EBP + 0x1c]
008a7b4a  52                    PUSH EDX
008a7b4b  8b8554ffffff          MOV EAX,dword ptr [EBP + 0xffffff54]
008a7b51  50                    PUSH EAX
008a7b52  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7b55  e8f6960000            CALL 0x008b1250
008a7b5a  6a00                  PUSH 0x0
008a7b5c  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7b5f  8b11                  MOV EDX,dword ptr [ECX]
008a7b61  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7b64  8b4240                MOV EAX,dword ptr [EDX + 0x40]
008a7b67  ffd0                  CALL EAX
008a7b69  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7b6c  51                    PUSH ECX
008a7b6d  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
008a7b70  e8eb6ef9ff            CALL 0x0083ea60
008a7b75  8b550c                MOV EDX,dword ptr [EBP + 0xc]
008a7b78  8b02                  MOV EAX,dword ptr [EDX]
008a7b7a  83c001                ADD EAX,0x1
008a7b7d  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
008a7b80  8901                  MOV dword ptr [ECX],EAX
008a7b82  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008a7b85  64890d00000000        MOV dword ptr FS:[0x0],ECX
008a7b8c  59                    POP ECX
008a7b8d  5e                    POP ESI
008a7b8e  8be5                  MOV ESP,EBP
008a7b90  5d                    POP EBP
008a7b91  c21800                RET 0x18
