; FN 008cdb00 FUN_008cdb00
008cdb00  55                    PUSH EBP
008cdb01  8bec                  MOV EBP,ESP
008cdb03  6aff                  PUSH -0x1
008cdb05  68fbb69700            PUSH 0x97b6fb
008cdb0a  64a100000000          MOV EAX,FS:[0x0]
008cdb10  50                    PUSH EAX
008cdb11  81eccc000000          SUB ESP,0xcc
008cdb17  a1201fa100            MOV EAX,[0x00a11f20]
008cdb1c  33c5                  XOR EAX,EBP
008cdb1e  50                    PUSH EAX
008cdb1f  8d45f4                LEA EAX,[EBP + -0xc]
008cdb22  64a300000000          MOV FS:[0x0],EAX
008cdb28  898d34ffffff          MOV dword ptr [EBP + 0xffffff34],ECX
008cdb2e  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008cdb31  8b4850                MOV ECX,dword ptr [EAX + 0x50]
008cdb34  898d7cffffff          MOV dword ptr [EBP + 0xffffff7c],ECX
008cdb3a  83bd7cffffff00        CMP dword ptr [EBP + 0xffffff7c],0x0
008cdb41  0f84a3070000          JZ 0x008ce2ea
008cdb47  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008cdb4a  8b424c                MOV EAX,dword ptr [EDX + 0x4c]
008cdb4d  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
008cdb53  83bd78ffffff2d        CMP dword ptr [EBP + 0xffffff78],0x2d
008cdb5a  751c                  JNZ 0x008cdb78
008cdb5c  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdb62  8b511c                MOV EDX,dword ptr [ECX + 0x1c]
008cdb65  899574ffffff          MOV dword ptr [EBP + 0xffffff74],EDX
008cdb6b  6a00                  PUSH 0x0
008cdb6d  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
008cdb73  e82847b4ff            CALL 0x004122a0
008cdb78  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
008cdb7f  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008cdb85  0560380000            ADD EAX,0x3860
008cdb8a  394508                CMP dword ptr [EBP + 0x8],EAX
008cdb8d  7516                  JNZ 0x008cdba5
008cdb8f  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdb95  81c180230000          ADD ECX,0x2380
008cdb9b  e81024b5ff            CALL 0x0041ffb0
008cdba0  8945e4                MOV dword ptr [EBP + -0x1c],EAX
008cdba3  eb06                  JMP 0x008cdbab
008cdba5  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008cdba8  894de4                MOV dword ptr [EBP + -0x1c],ECX
008cdbab  837de400              CMP dword ptr [EBP + -0x1c],0x0
008cdbaf  7505                  JNZ 0x008cdbb6
008cdbb1  e934070000            JMP 0x008ce2ea
008cdbb6  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
008cdbb9  8b82dc010000          MOV EAX,dword ptr [EDX + 0x1dc]
008cdbbf  d1e8                  SHR EAX,0x1
008cdbc1  83e001                AND EAX,0x1
008cdbc4  7412                  JZ 0x008cdbd8
008cdbc6  6a00                  PUSH 0x0
008cdbc8  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdbce  e82d070000            CALL 0x008ce300
008cdbd3  e912070000            JMP 0x008ce2ea
008cdbd8  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
008cdbde  8b5108                MOV EDX,dword ptr [ECX + 0x8]
008cdbe1  8955f0                MOV dword ptr [EBP + -0x10],EDX
008cdbe4  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008cdbe7  e8e4ecc4ff            CALL 0x0051c8d0
008cdbec  8bc8                  MOV ECX,EAX
008cdbee  e83d9ed2ff            CALL 0x005f7a30
008cdbf3  8945e0                MOV dword ptr [EBP + -0x20],EAX
008cdbf6  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008cdbfc  8b88bc3d0000          MOV ECX,dword ptr [EAX + 0x3dbc]
008cdc02  8b515c                MOV EDX,dword ptr [ECX + 0x5c]
008cdc05  8955e8                MOV dword ptr [EBP + -0x18],EDX
008cdc08  c745ec01000000        MOV dword ptr [EBP + -0x14],0x1
008cdc0f  837de801              CMP dword ptr [EBP + -0x18],0x1
008cdc13  0f85d1020000          JNZ 0x008cdeea
008cdc19  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008cdc1f  0fb6882c3d0000        MOVZX ECX,byte ptr [EAX + 0x3d2c]
008cdc26  83f901                CMP ECX,0x1
008cdc29  0f85bb020000          JNZ 0x008cdeea
008cdc2f  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
008cdc32  8b82e0010000          MOV EAX,dword ptr [EDX + 0x1e0]
008cdc38  898570ffffff          MOV dword ptr [EBP + 0xffffff70],EAX
008cdc3e  83bd70ffffffff        CMP dword ptr [EBP + 0xffffff70],-0x1
008cdc45  750c                  JNZ 0x008cdc53
008cdc47  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
008cdc4e  e995020000            JMP 0x008cdee8
008cdc53  8d4dd4                LEA ECX,[EBP + -0x2c]
008cdc56  e8e558e6ff            CALL 0x00733540
008cdc5b  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008cdc62  8d4dcc                LEA ECX,[EBP + -0x34]
008cdc65  e8d658e6ff            CALL 0x00733540
008cdc6a  c645fc01              MOV byte ptr [EBP + -0x4],0x1
008cdc6e  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdc74  8b89343d0000          MOV ECX,dword ptr [ECX + 0x3d34]
008cdc7a  e8b191e0ff            CALL 0x006d6e30
008cdc7f  89856cffffff          MOV dword ptr [EBP + 0xffffff6c],EAX
008cdc85  8b956cffffff          MOV EDX,dword ptr [EBP + 0xffffff6c]
008cdc8b  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
008cdc91  88856bffffff          MOV byte ptr [EBP + 0xffffff6b],AL
008cdc97  8a8d6bffffff          MOV CL,byte ptr [EBP + 0xffffff6b]
008cdc9d  888d30ffffff          MOV byte ptr [EBP + 0xffffff30],CL
008cdca3  80bd30ffffff27        CMP byte ptr [EBP + 0xffffff30],0x27
008cdcaa  7432                  JZ 0x008cdcde
008cdcac  80bd30ffffff28        CMP byte ptr [EBP + 0xffffff30],0x28
008cdcb3  741a                  JZ 0x008cdccf
008cdcb5  80bd30ffffff29        CMP byte ptr [EBP + 0xffffff30],0x29
008cdcbc  7402                  JZ 0x008cdcc0
008cdcbe  eb2b                  JMP 0x008cdceb
008cdcc0  683c899a00            PUSH 0x9a893c
008cdcc5  8d4dcc                LEA ECX,[EBP + -0x34]
008cdcc8  e8035ce6ff            CALL 0x007338d0
008cdccd  eb1c                  JMP 0x008cdceb
008cdccf  6830899a00            PUSH 0x9a8930
008cdcd4  8d4dcc                LEA ECX,[EBP + -0x34]
008cdcd7  e8f45be6ff            CALL 0x007338d0
008cdcdc  eb0d                  JMP 0x008cdceb
008cdcde  6820899a00            PUSH 0x9a8920
008cdce3  8d4dcc                LEA ECX,[EBP + -0x34]
008cdce6  e8e55be6ff            CALL 0x007338d0
008cdceb  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
008cdcee  8b82e0010000          MOV EAX,dword ptr [EDX + 0x1e0]
008cdcf4  898564ffffff          MOV dword ptr [EBP + 0xffffff64],EAX
008cdcfa  8d4dd4                LEA ECX,[EBP + -0x2c]
008cdcfd  51                    PUSH ECX
008cdcfe  8d55cc                LEA EDX,[EBP + -0x34]
008cdd01  52                    PUSH EDX
008cdd02  8b8564ffffff          MOV EAX,dword ptr [EBP + 0xffffff64]
008cdd08  50                    PUSH EAX
008cdd09  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdd0f  8b89b83d0000          MOV ECX,dword ptr [ECX + 0x3db8]
008cdd15  e8b6d4e4ff            CALL 0x0071b1d0
008cdd1a  680c030000            PUSH 0x30c
008cdd1f  e8ffb90400            CALL 0x00919723
008cdd24  83c404                ADD ESP,0x4
008cdd27  8945a0                MOV dword ptr [EBP + -0x60],EAX
008cdd2a  c645fc02              MOV byte ptr [EBP + -0x4],0x2
008cdd2e  837da000              CMP dword ptr [EBP + -0x60],0x0
008cdd32  7415                  JZ 0x008cdd49
008cdd34  680000007f            PUSH 0x7f000000
008cdd39  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
008cdd3c  e81f20d3ff            CALL 0x005ffd60
008cdd41  89852cffffff          MOV dword ptr [EBP + 0xffffff2c],EAX
008cdd47  eb0a                  JMP 0x008cdd53
008cdd49  c7852cffffff00000000  MOV dword ptr [EBP + 0xffffff2c],0x0
008cdd53  8b952cffffff          MOV EDX,dword ptr [EBP + 0xffffff2c]
008cdd59  8955a4                MOV dword ptr [EBP + -0x5c],EDX
008cdd5c  c645fc01              MOV byte ptr [EBP + -0x4],0x1
008cdd60  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
008cdd63  8945c8                MOV dword ptr [EBP + -0x38],EAX
008cdd66  8d4dd4                LEA ECX,[EBP + -0x2c]
008cdd69  51                    PUSH ECX
008cdd6a  8d4d90                LEA ECX,[EBP + -0x70]
008cdd6d  e89e2ae4ff            CALL 0x00710810
008cdd72  8d5590                LEA EDX,[EBP + -0x70]
008cdd75  52                    PUSH EDX
008cdd76  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008cdd79  e89238d3ff            CALL 0x00601610
008cdd7e  680000007f            PUSH 0x7f000000
008cdd83  6a00                  PUSH 0x0
008cdd85  6a00                  PUSH 0x0
008cdd87  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008cdd8a  e861ebc4ff            CALL 0x0051c8f0
008cdd8f  50                    PUSH EAX
008cdd90  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008cdd93  e8e825d3ff            CALL 0x00600380
008cdd98  6a01                  PUSH 0x1
008cdd9a  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008cdd9d  e85e8fd3ff            CALL 0x00606d00
008cdda2  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008cdda8  8b88343d0000          MOV ECX,dword ptr [EAX + 0x3d34]
008cddae  8b91d4020000          MOV EDX,dword ptr [ECX + 0x2d4]
008cddb4  899560ffffff          MOV dword ptr [EBP + 0xffffff60],EDX
008cddba  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
008cddbd  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
008cddc3  8988d4020000          MOV dword ptr [EAX + 0x2d4],ECX
008cddc9  c745c400000000        MOV dword ptr [EBP + -0x3c],0x0
008cddd0  eb09                  JMP 0x008cdddb
008cddd2  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
008cddd5  83c201                ADD EDX,0x1
008cddd8  8955c4                MOV dword ptr [EBP + -0x3c],EDX
008cdddb  837dc406              CMP dword ptr [EBP + -0x3c],0x6
008cdddf  7d56                  JGE 0x008cde37
008cdde1  837dc401              CMP dword ptr [EBP + -0x3c],0x1
008cdde5  7502                  JNZ 0x008cdde9
008cdde7  ebe9                  JMP 0x008cddd2
008cdde9  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
008cddec  50                    PUSH EAX
008cdded  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
008cddf0  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
008cddf6  8b848a543d0000        MOV EAX,dword ptr [EDX + ECX*0x4 + 0x3d54]
008cddfd  50                    PUSH EAX
008cddfe  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008cde01  e85a7fd4ff            CALL 0x00615d60
008cde06  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cde0c  8b91343d0000          MOV EDX,dword ptr [ECX + 0x3d34]
008cde12  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
008cde15  8b8c82f4020000        MOV ECX,dword ptr [EDX + EAX*0x4 + 0x2f4]
008cde1c  898d5cffffff          MOV dword ptr [EBP + 0xffffff5c],ECX
008cde22  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
008cde25  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
008cde28  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
008cde2e  898c90f4020000        MOV dword ptr [EAX + EDX*0x4 + 0x2f4],ECX
008cde35  eb9b                  JMP 0x008cddd2
008cde37  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
008cde3d  8b82343d0000          MOV EAX,dword ptr [EDX + 0x3d34]
008cde43  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
008cde49  c1e905                SHR ECX,0x5
008cde4c  83e101                AND ECX,0x1
008cde4f  898d58ffffff          MOV dword ptr [EBP + 0xffffff58],ECX
008cde55  8b9558ffffff          MOV EDX,dword ptr [EBP + 0xffffff58]
008cde5b  83e201                AND EDX,0x1
008cde5e  c1e205                SHL EDX,0x5
008cde61  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
008cde64  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
008cde6a  83e1df                AND ECX,0xffffffdf
008cde6d  0bca                  OR ECX,EDX
008cde6f  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
008cde72  898ac8020000          MOV dword ptr [EDX + 0x2c8],ECX
008cde78  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008cde7e  8b88343d0000          MOV ECX,dword ptr [EAX + 0x3d34]
008cde84  894d88                MOV dword ptr [EBP + -0x78],ECX
008cde87  8b5588                MOV EDX,dword ptr [EBP + -0x78]
008cde8a  89558c                MOV dword ptr [EBP + -0x74],EDX
008cde8d  837d8c00              CMP dword ptr [EBP + -0x74],0x0
008cde91  7416                  JZ 0x008cdea9
008cde93  6a01                  PUSH 0x1
008cde95  8b458c                MOV EAX,dword ptr [EBP + -0x74]
008cde98  8b10                  MOV EDX,dword ptr [EAX]
008cde9a  8b4d8c                MOV ECX,dword ptr [EBP + -0x74]
008cde9d  8b02                  MOV EAX,dword ptr [EDX]
008cde9f  ffd0                  CALL EAX
008cdea1  898528ffffff          MOV dword ptr [EBP + 0xffffff28],EAX
008cdea7  eb0a                  JMP 0x008cdeb3
008cdea9  c78528ffffff00000000  MOV dword ptr [EBP + 0xffffff28],0x0
008cdeb3  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdeb9  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
008cdebc  8991343d0000          MOV dword ptr [ECX + 0x3d34],EDX
008cdec2  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdec8  e8f3f3ffff            CALL 0x008cd2c0
008cdecd  c645fc00              MOV byte ptr [EBP + -0x4],0x0
008cded1  8d4dcc                LEA ECX,[EBP + -0x34]
008cded4  e8a758e6ff            CALL 0x00733780
008cded9  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008cdee0  8d4dd4                LEA ECX,[EBP + -0x2c]
008cdee3  e89858e6ff            CALL 0x00733780
008cdee8  eb34                  JMP 0x008cdf1e
008cdeea  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
008cdeed  8b88e0010000          MOV ECX,dword ptr [EAX + 0x1e0]
008cdef3  898d54ffffff          MOV dword ptr [EBP + 0xffffff54],ECX
008cdef9  83bd54ffffffff        CMP dword ptr [EBP + 0xffffff54],-0x1
008cdf00  751c                  JNZ 0x008cdf1e
008cdf02  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
008cdf05  8b82d0010000          MOV EAX,dword ptr [EDX + 0x1d0]
008cdf0b  8945c0                MOV dword ptr [EBP + -0x40],EAX
008cdf0e  817dc00000007f        CMP dword ptr [EBP + -0x40],0x7f000000
008cdf15  7407                  JZ 0x008cdf1e
008cdf17  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
008cdf1e  837dec00              CMP dword ptr [EBP + -0x14],0x0
008cdf22  0f84b7030000          JZ 0x008ce2df
008cdf28  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008cdf2b  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
008cdf31  83bc8a543d000000      CMP dword ptr [EDX + ECX*0x4 + 0x3d54],0x0
008cdf39  0f846b010000          JZ 0x008ce0aa
008cdf3f  680000007f            PUSH 0x7f000000
008cdf44  6a00                  PUSH 0x0
008cdf46  6a00                  PUSH 0x0
008cdf48  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008cdf4b  e8a0e9c4ff            CALL 0x0051c8f0
008cdf50  50                    PUSH EAX
008cdf51  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008cdf54  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdf5a  8b8c81543d0000        MOV ECX,dword ptr [ECX + EAX*0x4 + 0x3d54]
008cdf61  e81a24d3ff            CALL 0x00600380
008cdf66  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008cdf69  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008cdf6f  8b8c90543d0000        MOV ECX,dword ptr [EAX + EDX*0x4 + 0x3d54]
008cdf76  51                    PUSH ECX
008cdf77  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdf7d  81c1483d0000          ADD ECX,0x3d48
008cdf83  e85863b4ff            CALL 0x004142e0
008cdf88  8945bc                MOV dword ptr [EBP + -0x44],EAX
008cdf8b  837dbcff              CMP dword ptr [EBP + -0x44],-0x1
008cdf8f  7436                  JZ 0x008cdfc7
008cdf91  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
008cdf94  52                    PUSH EDX
008cdf95  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdf9b  81c1483d0000          ADD ECX,0x3d48
008cdfa1  e89ad8bcff            CALL 0x0049b840
008cdfa6  6a00                  PUSH 0x0
008cdfa8  6a00                  PUSH 0x0
008cdfaa  6a01                  PUSH 0x1
008cdfac  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008cdfaf  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdfb5  8d9481543d0000        LEA EDX,[ECX + EAX*0x4 + 0x3d54]
008cdfbc  52                    PUSH EDX
008cdfbd  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008cdfc0  e8cb70d3ff            CALL 0x00605090
008cdfc5  eb41                  JMP 0x008ce008
008cdfc7  6a00                  PUSH 0x0
008cdfc9  6a00                  PUSH 0x0
008cdfcb  6a01                  PUSH 0x1
008cdfcd  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008cdfd0  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdfd6  8d9481543d0000        LEA EDX,[ECX + EAX*0x4 + 0x3d54]
008cdfdd  52                    PUSH EDX
008cdfde  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008cdfe1  e8aa70d3ff            CALL 0x00605090
008cdfe6  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008cdfe9  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdfef  8b9481543d0000        MOV EDX,dword ptr [ECX + EAX*0x4 + 0x3d54]
008cdff6  52                    PUSH EDX
008cdff7  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008cdffd  81c13c3d0000          ADD ECX,0x3d3c
008ce003  e8580af7ff            CALL 0x0083ea60
008ce008  837de801              CMP dword ptr [EBP + -0x18],0x1
008ce00c  7512                  JNZ 0x008ce020
008ce00e  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008ce014  0fb6882c3d0000        MOVZX ECX,byte ptr [EAX + 0x3d2c]
008ce01b  83f901                CMP ECX,0x1
008ce01e  7476                  JZ 0x008ce096
008ce020  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008ce023  52                    PUSH EDX
008ce024  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008ce027  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008ce02d  8b9481543d0000        MOV EDX,dword ptr [ECX + EAX*0x4 + 0x3d54]
008ce034  52                    PUSH EDX
008ce035  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008ce03b  8b88343d0000          MOV ECX,dword ptr [EAX + 0x3d34]
008ce041  e84a7ed4ff            CALL 0x00615e90
008ce046  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008ce04c  8b91343d0000          MOV EDX,dword ptr [ECX + 0x3d34]
008ce052  899550ffffff          MOV dword ptr [EBP + 0xffffff50],EDX
008ce058  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008ce05b  8b8d50ffffff          MOV ECX,dword ptr [EBP + 0xffffff50]
008ce061  c78481f4020000ffffffffMOV dword ptr [ECX + EAX*0x4 + 0x2f4],0xffffffff
008ce06c  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008ce06f  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008ce075  8b8c90543d0000        MOV ECX,dword ptr [EAX + EDX*0x4 + 0x3d54]
008ce07c  898d4cffffff          MOV dword ptr [EBP + 0xffffff4c],ECX
008ce082  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008ce085  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
008ce08b  c78490f4020000ffffffffMOV dword ptr [EAX + EDX*0x4 + 0x2f4],0xffffffff
008ce096  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008ce099  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
008ce09f  c7848a543d000000000000MOV dword ptr [EDX + ECX*0x4 + 0x3d54],0x0
008ce0aa  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
008ce0ad  8b88d0010000          MOV ECX,dword ptr [EAX + 0x1d0]
008ce0b3  898d48ffffff          MOV dword ptr [EBP + 0xffffff48],ECX
008ce0b9  8b9548ffffff          MOV EDX,dword ptr [EBP + 0xffffff48]
008ce0bf  52                    PUSH EDX
008ce0c0  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008ce0c3  e818e0c4ff            CALL 0x0051c0e0
008ce0c8  8945dc                MOV dword ptr [EBP + -0x24],EAX
008ce0cb  837ddc00              CMP dword ptr [EBP + -0x24],0x0
008ce0cf  0f840a020000          JZ 0x008ce2df
008ce0d5  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008ce0d8  50                    PUSH EAX
008ce0d9  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008ce0df  81c13c3d0000          ADD ECX,0x3d3c
008ce0e5  e8f661b4ff            CALL 0x004142e0
008ce0ea  8945bc                MOV dword ptr [EBP + -0x44],EAX
008ce0ed  837dbcff              CMP dword ptr [EBP + -0x44],-0x1
008ce0f1  744a                  JZ 0x008ce13d
008ce0f3  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
008ce0f6  51                    PUSH ECX
008ce0f7  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008ce0fd  81c13c3d0000          ADD ECX,0x3d3c
008ce103  e838d7bcff            CALL 0x0049b840
008ce108  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008ce10b  8b82cc020000          MOV EAX,dword ptr [EDX + 0x2cc]
008ce111  898544ffffff          MOV dword ptr [EBP + 0xffffff44],EAX
008ce117  83bd44ffffff01        CMP dword ptr [EBP + 0xffffff44],0x1
008ce11e  7e0f                  JLE 0x008ce12f
008ce120  6a01                  PUSH 0x1
008ce122  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008ce125  e8966dd3ff            CALL 0x00604ec0
008ce12a  8945dc                MOV dword ptr [EBP + -0x24],EAX
008ce12d  eb0c                  JMP 0x008ce13b
008ce12f  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008ce132  51                    PUSH ECX
008ce133  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ce136  e8457dd3ff            CALL 0x00605e80
008ce13b  eb48                  JMP 0x008ce185
008ce13d  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008ce140  8b82cc020000          MOV EAX,dword ptr [EDX + 0x2cc]
008ce146  898540ffffff          MOV dword ptr [EBP + 0xffffff40],EAX
008ce14c  83bd40ffffff01        CMP dword ptr [EBP + 0xffffff40],0x1
008ce153  7e0f                  JLE 0x008ce164
008ce155  6a01                  PUSH 0x1
008ce157  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008ce15a  e8616dd3ff            CALL 0x00604ec0
008ce15f  8945dc                MOV dword ptr [EBP + -0x24],EAX
008ce162  eb0c                  JMP 0x008ce170
008ce164  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008ce167  51                    PUSH ECX
008ce168  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008ce16b  e8107dd3ff            CALL 0x00605e80
008ce170  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008ce173  52                    PUSH EDX
008ce174  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008ce17a  81c1483d0000          ADD ECX,0x3d48
008ce180  e8db08f7ff            CALL 0x0083ea60
008ce185  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008ce18b  8b88b43d0000          MOV ECX,dword ptr [EAX + 0x3db4]
008ce191  8b5124                MOV EDX,dword ptr [ECX + 0x24]
008ce194  8955b4                MOV dword ptr [EBP + -0x4c],EDX
008ce197  c745b000000000        MOV dword ptr [EBP + -0x50],0x0
008ce19e  c745b800000000        MOV dword ptr [EBP + -0x48],0x0
008ce1a5  eb09                  JMP 0x008ce1b0
008ce1a7  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
008ce1aa  83c001                ADD EAX,0x1
008ce1ad  8945b8                MOV dword ptr [EBP + -0x48],EAX
008ce1b0  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
008ce1b3  3b4db4                CMP ECX,dword ptr [EBP + -0x4c]
008ce1b6  0f8d95000000          JGE 0x008ce251
008ce1bc  8d4da8                LEA ECX,[EBP + -0x58]
008ce1bf  e87c53e6ff            CALL 0x00733540
008ce1c4  c745fc03000000        MOV dword ptr [EBP + -0x4],0x3
008ce1cb  6814899a00            PUSH 0x9a8914
008ce1d0  8d4d80                LEA ECX,[EBP + -0x80]
008ce1d3  e89853e6ff            CALL 0x00733570
008ce1d8  c645fc04              MOV byte ptr [EBP + -0x4],0x4
008ce1dc  8d55a8                LEA EDX,[EBP + -0x58]
008ce1df  52                    PUSH EDX
008ce1e0  8d4580                LEA EAX,[EBP + -0x80]
008ce1e3  50                    PUSH EAX
008ce1e4  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
008ce1e7  51                    PUSH ECX
008ce1e8  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
008ce1ee  8b8ab43d0000          MOV ECX,dword ptr [EDX + 0x3db4]
008ce1f4  e8d7cfe4ff            CALL 0x0071b1d0
008ce1f9  c645fc03              MOV byte ptr [EBP + -0x4],0x3
008ce1fd  8d4d80                LEA ECX,[EBP + -0x80]
008ce200  e87b55e6ff            CALL 0x00733780
008ce205  8d4da8                LEA ECX,[EBP + -0x58]
008ce208  e8035ee6ff            CALL 0x00734010
008ce20d  50                    PUSH EAX
008ce20e  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008ce211  83c128                ADD ECX,0x28
008ce214  e8f75de6ff            CALL 0x00734010
008ce219  50                    PUSH EAX
008ce21a  e8a88e0600            CALL 0x009370c7
008ce21f  83c408                ADD ESP,0x8
008ce222  85c0                  TEST EAX,EAX
008ce224  7517                  JNZ 0x008ce23d
008ce226  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
008ce229  8945b0                MOV dword ptr [EBP + -0x50],EAX
008ce22c  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008ce233  8d4da8                LEA ECX,[EBP + -0x58]
008ce236  e84555e6ff            CALL 0x00733780
008ce23b  eb14                  JMP 0x008ce251
008ce23d  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008ce244  8d4da8                LEA ECX,[EBP + -0x58]
008ce247  e83455e6ff            CALL 0x00733780
008ce24c  e956ffffff            JMP 0x008ce1a7
008ce251  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008ce254  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
008ce25a  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008ce25d  89848a543d0000        MOV dword ptr [EDX + ECX*0x4 + 0x3d54],EAX
008ce264  837de801              CMP dword ptr [EBP + -0x18],0x1
008ce268  7512                  JNZ 0x008ce27c
008ce26a  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008ce270  0fb6912c3d0000        MOVZX EDX,byte ptr [ECX + 0x3d2c]
008ce277  83fa01                CMP EDX,0x1
008ce27a  7463                  JZ 0x008ce2df
008ce27c  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
008ce27f  8b88e0010000          MOV ECX,dword ptr [EAX + 0x1e0]
008ce285  898d3cffffff          MOV dword ptr [EBP + 0xffffff3c],ECX
008ce28b  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008ce28e  8b8534ffffff          MOV EAX,dword ptr [EBP + 0xffffff34]
008ce294  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
008ce29a  898c909c3d0000        MOV dword ptr [EAX + EDX*0x4 + 0x3d9c],ECX
008ce2a1  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008ce2a4  52                    PUSH EDX
008ce2a5  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008ce2a8  50                    PUSH EAX
008ce2a9  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008ce2af  8b89343d0000          MOV ECX,dword ptr [ECX + 0x3d34]
008ce2b5  e8a67ad4ff            CALL 0x00615d60
008ce2ba  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
008ce2c0  8b82343d0000          MOV EAX,dword ptr [EDX + 0x3d34]
008ce2c6  898538ffffff          MOV dword ptr [EBP + 0xffffff38],EAX
008ce2cc  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008ce2cf  8b9538ffffff          MOV EDX,dword ptr [EBP + 0xffffff38]
008ce2d5  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
008ce2d8  89848af4020000        MOV dword ptr [EDX + ECX*0x4 + 0x2f4],EAX
008ce2df  8b8d34ffffff          MOV ECX,dword ptr [EBP + 0xffffff34]
008ce2e5  e8a6cfffff            CALL 0x008cb290
008ce2ea  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008ce2ed  64890d00000000        MOV dword ptr FS:[0x0],ECX
008ce2f4  59                    POP ECX
008ce2f5  8be5                  MOV ESP,EBP
008ce2f7  5d                    POP EBP
008ce2f8  c20400                RET 0x4
