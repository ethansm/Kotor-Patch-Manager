; FN 005b8000 FUN_005b8000
005b8000  55                    PUSH EBP
005b8001  8bec                  MOV EBP,ESP
005b8003  6aff                  PUSH -0x1
005b8005  6803349500            PUSH 0x953403
005b800a  64a100000000          MOV EAX,FS:[0x0]
005b8010  50                    PUSH EAX
005b8011  81ec70020000          SUB ESP,0x270
005b8017  a1201fa100            MOV EAX,[0x00a11f20]
005b801c  33c5                  XOR EAX,EBP
005b801e  50                    PUSH EAX
005b801f  8d45f4                LEA EAX,[EBP + -0xc]
005b8022  64a300000000          MOV FS:[0x0],EAX
005b8028  898da0fdffff          MOV dword ptr [EBP + 0xfffffda0],ECX
005b802e  d9ee                  FLDZ
005b8030  d95de4                FSTP float ptr [EBP + -0x1c]
005b8033  d9ee                  FLDZ
005b8035  d95de8                FSTP float ptr [EBP + -0x18]
005b8038  d9ee                  FLDZ
005b803a  d95dec                FSTP float ptr [EBP + -0x14]
005b803d  6a04                  PUSH 0x4
005b803f  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005b8042  83c038                ADD EAX,0x38
005b8045  50                    PUSH EAX
005b8046  8d4dac                LEA ECX,[EBP + -0x54]
005b8049  51                    PUSH ECX
005b804a  e8515b3600            CALL 0x0091dba0
005b804f  83c40c                ADD ESP,0xc
005b8052  6a04                  PUSH 0x4
005b8054  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005b8057  83c23c                ADD EDX,0x3c
005b805a  52                    PUSH EDX
005b805b  8d4590                LEA EAX,[EBP + -0x70]
005b805e  50                    PUSH EAX
005b805f  e83c5b3600            CALL 0x0091dba0
005b8064  83c40c                ADD ESP,0xc
005b8067  6a04                  PUSH 0x4
005b8069  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005b806c  83c140                ADD ECX,0x40
005b806f  51                    PUSH ECX
005b8070  8d5588                LEA EDX,[EBP + -0x78]
005b8073  52                    PUSH EDX
005b8074  e8275b3600            CALL 0x0091dba0
005b8079  83c40c                ADD ESP,0xc
005b807c  6a04                  PUSH 0x4
005b807e  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005b8081  83c044                ADD EAX,0x44
005b8084  50                    PUSH EAX
005b8085  8d4dc8                LEA ECX,[EBP + -0x38]
005b8088  51                    PUSH ECX
005b8089  e8125b3600            CALL 0x0091dba0
005b808e  83c40c                ADD ESP,0xc
005b8091  6a04                  PUSH 0x4
005b8093  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005b8096  83c248                ADD EDX,0x48
005b8099  52                    PUSH EDX
005b809a  8d45b4                LEA EAX,[EBP + -0x4c]
005b809d  50                    PUSH EAX
005b809e  e8fd5a3600            CALL 0x0091dba0
005b80a3  83c40c                ADD ESP,0xc
005b80a6  6a04                  PUSH 0x4
005b80a8  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005b80ab  83c14c                ADD ECX,0x4c
005b80ae  51                    PUSH ECX
005b80af  8d55a8                LEA EDX,[EBP + -0x58]
005b80b2  52                    PUSH EDX
005b80b3  e8e85a3600            CALL 0x0091dba0
005b80b8  83c40c                ADD ESP,0xc
005b80bb  6a04                  PUSH 0x4
005b80bd  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005b80c0  83c050                ADD EAX,0x50
005b80c3  50                    PUSH EAX
005b80c4  8d4d94                LEA ECX,[EBP + -0x6c]
005b80c7  51                    PUSH ECX
005b80c8  e8d35a3600            CALL 0x0091dba0
005b80cd  83c40c                ADD ESP,0xc
005b80d0  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005b80d3  0fb7426e              MOVZX EAX,word ptr [EDX + 0x6e]
005b80d7  83f809                CMP EAX,0x9
005b80da  7d7c                  JGE 0x005b8158
005b80dc  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b80e2  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005b80e5  899514ffffff          MOV dword ptr [EBP + 0xffffff14],EDX
005b80eb  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005b80f0  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005b80f3  898d10ffffff          MOV dword ptr [EBP + 0xffffff10],ECX
005b80f9  8d9574ffffff          LEA EDX,[EBP + 0xffffff74]
005b80ff  52                    PUSH EDX
005b8100  8d45b8                LEA EAX,[EBP + -0x48]
005b8103  50                    PUSH EAX
005b8104  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
005b810a  51                    PUSH ECX
005b810b  8b8d10ffffff          MOV ECX,dword ptr [EBP + 0xffffff10]
005b8111  e83a42f6ff            CALL 0x0051c350
005b8116  8bc8                  MOV ECX,EAX
005b8118  e8032cf6ff            CALL 0x0051ad20
005b811d  6a04                  PUSH 0x4
005b811f  8d55b8                LEA EDX,[EBP + -0x48]
005b8122  52                    PUSH EDX
005b8123  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005b8126  83c054                ADD EAX,0x54
005b8129  50                    PUSH EAX
005b812a  e8715a3600            CALL 0x0091dba0
005b812f  83c40c                ADD ESP,0xc
005b8132  6a04                  PUSH 0x4
005b8134  8d8d74ffffff          LEA ECX,[EBP + 0xffffff74]
005b813a  51                    PUSH ECX
005b813b  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005b813e  83c258                ADD EDX,0x58
005b8141  52                    PUSH EDX
005b8142  e8595a3600            CALL 0x0091dba0
005b8147  83c40c                ADD ESP,0xc
005b814a  b809000000            MOV EAX,0x9
005b814f  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005b8152  6689416e              MOV word ptr [ECX + 0x6e],AX
005b8156  eb2d                  JMP 0x005b8185
005b8158  6a04                  PUSH 0x4
005b815a  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005b815d  83c254                ADD EDX,0x54
005b8160  52                    PUSH EDX
005b8161  8d45b8                LEA EAX,[EBP + -0x48]
005b8164  50                    PUSH EAX
005b8165  e8365a3600            CALL 0x0091dba0
005b816a  83c40c                ADD ESP,0xc
005b816d  6a04                  PUSH 0x4
005b816f  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005b8172  83c158                ADD ECX,0x58
005b8175  51                    PUSH ECX
005b8176  8d9574ffffff          LEA EDX,[EBP + 0xffffff74]
005b817c  52                    PUSH EDX
005b817d  e81e5a3600            CALL 0x0091dba0
005b8182  83c40c                ADD ESP,0xc
005b8185  c745d000000000        MOV dword ptr [EBP + -0x30],0x0
005b818c  c745d400000000        MOV dword ptr [EBP + -0x2c],0x0
005b8193  33c0                  XOR EAX,EAX
005b8195  668945cc              MOV word ptr [EBP + -0x34],AX
005b8199  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005b819c  0fb7516e              MOVZX EDX,word ptr [ECX + 0x6e]
005b81a0  83fa09                CMP EDX,0x9
005b81a3  7e3f                  JLE 0x005b81e4
005b81a5  6a04                  PUSH 0x4
005b81a7  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005b81aa  83c05c                ADD EAX,0x5c
005b81ad  50                    PUSH EAX
005b81ae  8d4dd4                LEA ECX,[EBP + -0x2c]
005b81b1  51                    PUSH ECX
005b81b2  e8e9593600            CALL 0x0091dba0
005b81b7  83c40c                ADD ESP,0xc
005b81ba  6a02                  PUSH 0x2
005b81bc  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005b81bf  83c260                ADD EDX,0x60
005b81c2  52                    PUSH EDX
005b81c3  8d45cc                LEA EAX,[EBP + -0x34]
005b81c6  50                    PUSH EAX
005b81c7  e8d4593600            CALL 0x0091dba0
005b81cc  83c40c                ADD ESP,0xc
005b81cf  6a04                  PUSH 0x4
005b81d1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005b81d4  83c164                ADD ECX,0x64
005b81d7  51                    PUSH ECX
005b81d8  8d55d0                LEA EDX,[EBP + -0x30]
005b81db  52                    PUSH EDX
005b81dc  e8bf593600            CALL 0x0091dba0
005b81e1  83c40c                ADD ESP,0xc
005b81e4  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b81ea  8b88ac110000          MOV ECX,dword ptr [EAX + 0x11ac]
005b81f0  898d0cffffff          MOV dword ptr [EBP + 0xffffff0c],ECX
005b81f6  83bd0cffffff00        CMP dword ptr [EBP + 0xffffff0c],0x0
005b81fd  0f849b000000          JZ 0x005b829e
005b8203  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
005b8209  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005b820c  898570ffffff          MOV dword ptr [EBP + 0xffffff70],EAX
005b8212  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8218  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005b821b  899508ffffff          MOV dword ptr [EBP + 0xffffff08],EDX
005b8221  8b8508ffffff          MOV EAX,dword ptr [EBP + 0xffffff08]
005b8227  50                    PUSH EAX
005b8228  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
005b822e  e88d7c1800            CALL 0x0073fec0
005b8233  898568ffffff          MOV dword ptr [EBP + 0xffffff68],EAX
005b8239  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
005b823f  51                    PUSH ECX
005b8240  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
005b8246  e845791800            CALL 0x0073fb90
005b824b  8bc8                  MOV ECX,EAX
005b824d  e88ee92200            CALL 0x007e6be0
005b8252  898564ffffff          MOV dword ptr [EBP + 0xffffff64],EAX
005b8258  83bd64ffffff00        CMP dword ptr [EBP + 0xffffff64],0x0
005b825f  7c3d                  JL 0x005b829e
005b8261  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
005b8267  e824791800            CALL 0x0073fb90
005b826c  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
005b8272  8b9564ffffff          MOV EDX,dword ptr [EBP + 0xffffff64]
005b8278  69d288000000          IMUL EDX,EDX,0x88
005b827e  8b8504ffffff          MOV EAX,dword ptr [EBP + 0xffffff04]
005b8284  8d4c1024              LEA ECX,[EAX + EDX*0x1 + 0x24]
005b8288  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
005b828e  68c7b20100            PUSH 0x1b2c7
005b8293  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
005b8299  e882ce2200            CALL 0x007e5120
005b829e  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b82a4  c78270050000b80b0000  MOV dword ptr [EDX + 0x570],0xbb8
005b82ae  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b82b4  8b10                  MOV EDX,dword ptr [EAX]
005b82b6  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b82bc  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
005b82c2  ffd0                  CALL EAX
005b82c4  83f801                CMP EAX,0x1
005b82c7  7455                  JZ 0x005b831e
005b82c9  33c9                  XOR ECX,ECX
005b82cb  83f901                CMP ECX,0x1
005b82ce  744e                  JZ 0x005b831e
005b82d0  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b82d6  83baac11000000        CMP dword ptr [EDX + 0x11ac],0x0
005b82dd  742c                  JZ 0x005b830b
005b82df  6a00                  PUSH 0x0
005b82e1  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b82e7  8b10                  MOV EDX,dword ptr [EAX]
005b82e9  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b82ef  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
005b82f5  ffd0                  CALL EAX
005b82f7  0fbfc8                MOVSX ECX,AX
005b82fa  33d2                  XOR EDX,EDX
005b82fc  85c9                  TEST ECX,ECX
005b82fe  0f9ec2                SETLE DL
005b8301  899500ffffff          MOV dword ptr [EBP + 0xffffff00],EDX
005b8307  eb0c                  JMP 0x005b8315
005b830b  c78500ffffff00000000  MOV dword ptr [EBP + 0xffffff00],0x0
005b8315  83bd00ffffff01        CMP dword ptr [EBP + 0xffffff00],0x1
005b831c  7548                  JNZ 0x005b8366
005b831e  6810270000            PUSH 0x2710
005b8323  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8329  8b10                  MOV EDX,dword ptr [EAX]
005b832b  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8331  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b8334  ffd0                  CALL EAX
005b8336  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b833c  c7817410000000000000  MOV dword ptr [ECX + 0x1074],0x0
005b8346  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b834c  e89f18fdff            CALL 0x00589bf0
005b8351  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8357  e8d418fdff            CALL 0x00589c30
005b835c  a1bc449900            MOV EAX,[0x009944bc]
005b8361  e92f130000            JMP 0x005b9695
005b8366  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b836c  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005b836f  8985fcfeffff          MOV dword ptr [EBP + 0xfffffefc],EAX
005b8375  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
005b837b  8b5108                MOV EDX,dword ptr [ECX + 0x8]
005b837e  8995f8feffff          MOV dword ptr [EBP + 0xfffffef8],EDX
005b8384  8d45bc                LEA EAX,[EBP + -0x44]
005b8387  50                    PUSH EAX
005b8388  8d4db0                LEA ECX,[EBP + -0x50]
005b838b  51                    PUSH ECX
005b838c  8b95fcfeffff          MOV EDX,dword ptr [EBP + 0xfffffefc]
005b8392  52                    PUSH EDX
005b8393  8b8df8feffff          MOV ECX,dword ptr [EBP + 0xfffffef8]
005b8399  e8b23ff6ff            CALL 0x0051c350
005b839e  8bc8                  MOV ECX,EAX
005b83a0  e87b29f6ff            CALL 0x0051ad20
005b83a5  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005b83aa  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005b83ad  898df4feffff          MOV dword ptr [EBP + 0xfffffef4],ECX
005b83b3  8d55a4                LEA EDX,[EBP + -0x5c]
005b83b6  52                    PUSH EDX
005b83b7  8d45c0                LEA EAX,[EBP + -0x40]
005b83ba  50                    PUSH EAX
005b83bb  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
005b83c1  51                    PUSH ECX
005b83c2  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
005b83c5  52                    PUSH EDX
005b83c6  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
005b83c9  50                    PUSH EAX
005b83ca  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
005b83cd  51                    PUSH ECX
005b83ce  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
005b83d4  e8973ff6ff            CALL 0x0051c370
005b83d9  8bc8                  MOV ECX,EAX
005b83db  e8802bf6ff            CALL 0x0051af60
005b83e0  837dd400              CMP dword ptr [EBP + -0x2c],0x0
005b83e4  7479                  JZ 0x005b845f
005b83e6  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
005b83e9  3b55d0                CMP EDX,dword ptr [EBP + -0x30]
005b83ec  7322                  JNC 0x005b8410
005b83ee  0fb745cc              MOVZX EAX,word ptr [EBP + -0x34]
005b83f2  50                    PUSH EAX
005b83f3  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b83f9  8b11                  MOV EDX,dword ptr [ECX]
005b83fb  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8401  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b8404  ffd0                  CALL EAX
005b8406  a1b4449900            MOV EAX,[0x009944b4]
005b840b  e985120000            JMP 0x005b9695
005b8410  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8416  e88552fbff            CALL 0x0056d6a0
005b841b  50                    PUSH EAX
005b841c  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8422  8b11                  MOV EDX,dword ptr [ECX]
005b8424  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b842a  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b842d  ffd0                  CALL EAX
005b842f  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8435  c7817410000000000000  MOV dword ptr [ECX + 0x1074],0x0
005b843f  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8445  e8a617fdff            CALL 0x00589bf0
005b844a  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8450  e8db17fdff            CALL 0x00589c30
005b8455  a1b8449900            MOV EAX,[0x009944b8]
005b845a  e936120000            JMP 0x005b9695
005b845f  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
005b8465  8b4208                MOV EAX,dword ptr [EDX + 0x8]
005b8468  8985f0feffff          MOV dword ptr [EBP + 0xfffffef0],EAX
005b846e  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
005b8471  51                    PUSH ECX
005b8472  8b8df0feffff          MOV ECX,dword ptr [EBP + 0xfffffef0]
005b8478  e8233cf6ff            CALL 0x0051c0a0
005b847d  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
005b8483  817dc80000007f        CMP dword ptr [EBP + -0x38],0x7f000000
005b848a  7458                  JZ 0x005b84e4
005b848c  83bd78ffffff00        CMP dword ptr [EBP + 0xffffff78],0x0
005b8493  754f                  JNZ 0x005b84e4
005b8495  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b849b  e80052fbff            CALL 0x0056d6a0
005b84a0  50                    PUSH EAX
005b84a1  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b84a7  8b02                  MOV EAX,dword ptr [EDX]
005b84a9  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b84af  8b507c                MOV EDX,dword ptr [EAX + 0x7c]
005b84b2  ffd2                  CALL EDX
005b84b4  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b84ba  c7807410000000000000  MOV dword ptr [EAX + 0x1074],0x0
005b84c4  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b84ca  e82117fdff            CALL 0x00589bf0
005b84cf  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b84d5  e85617fdff            CALL 0x00589c30
005b84da  a1bc449900            MOV EAX,[0x009944bc]
005b84df  e9b1110000            JMP 0x005b9695
005b84e4  83bd78ffffff00        CMP dword ptr [EBP + 0xffffff78],0x0
005b84eb  0f8402010000          JZ 0x005b85f3
005b84f1  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
005b84f7  8b11                  MOV EDX,dword ptr [ECX]
005b84f9  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
005b84ff  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005b8502  ffd0                  CALL EAX
005b8504  85c0                  TEST EAX,EAX
005b8506  0f84e7000000          JZ 0x005b85f3
005b850c  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
005b8512  8b11                  MOV EDX,dword ptr [ECX]
005b8514  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
005b851a  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005b851d  ffd0                  CALL EAX
005b851f  89859cfdffff          MOV dword ptr [EBP + 0xfffffd9c],EAX
005b8525  8b8d9cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd9c]
005b852b  8b11                  MOV EDX,dword ptr [ECX]
005b852d  8b8d9cfdffff          MOV ECX,dword ptr [EBP + 0xfffffd9c]
005b8533  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
005b8539  ffd0                  CALL EAX
005b853b  85c0                  TEST EAX,EAX
005b853d  7565                  JNZ 0x005b85a4
005b853f  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
005b8545  8b11                  MOV EDX,dword ptr [ECX]
005b8547  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
005b854d  8b4230                MOV EAX,dword ptr [EDX + 0x30]
005b8550  ffd0                  CALL EAX
005b8552  8985ecfeffff          MOV dword ptr [EBP + 0xfffffeec],EAX
005b8558  8b8decfeffff          MOV ECX,dword ptr [EBP + 0xfffffeec]
005b855e  83b9ac11000000        CMP dword ptr [ECX + 0x11ac],0x0
005b8565  742a                  JZ 0x005b8591
005b8567  6a00                  PUSH 0x0
005b8569  8b95ecfeffff          MOV EDX,dword ptr [EBP + 0xfffffeec]
005b856f  8b02                  MOV EAX,dword ptr [EDX]
005b8571  8b8decfeffff          MOV ECX,dword ptr [EBP + 0xfffffeec]
005b8577  8b909c000000          MOV EDX,dword ptr [EAX + 0x9c]
005b857d  ffd2                  CALL EDX
005b857f  98                    CWDE
005b8580  33c9                  XOR ECX,ECX
005b8582  85c0                  TEST EAX,EAX
005b8584  0f9ec1                SETLE CL
005b8587  898de8feffff          MOV dword ptr [EBP + 0xfffffee8],ECX
005b858d  eb0c                  JMP 0x005b859b
005b8591  c785e8feffff00000000  MOV dword ptr [EBP + 0xfffffee8],0x0
005b859b  83bde8feffff00        CMP dword ptr [EBP + 0xfffffee8],0x0
005b85a2  744f                  JZ 0x005b85f3
005b85a4  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b85aa  e8f150fbff            CALL 0x0056d6a0
005b85af  50                    PUSH EAX
005b85b0  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b85b6  8b02                  MOV EAX,dword ptr [EDX]
005b85b8  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b85be  8b507c                MOV EDX,dword ptr [EAX + 0x7c]
005b85c1  ffd2                  CALL EDX
005b85c3  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b85c9  c7807410000000000000  MOV dword ptr [EAX + 0x1074],0x0
005b85d3  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b85d9  e81216fdff            CALL 0x00589bf0
005b85de  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b85e4  e84716fdff            CALL 0x00589c30
005b85e9  a1bc449900            MOV EAX,[0x009944bc]
005b85ee  e9a2100000            JMP 0x005b9695
005b85f3  817dc80000007f        CMP dword ptr [EBP + -0x38],0x7f000000
005b85fa  7526                  JNZ 0x005b8622
005b85fc  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8602  8b9190000000          MOV EDX,dword ptr [ECX + 0x90]
005b8608  8995e4feffff          MOV dword ptr [EBP + 0xfffffee4],EDX
005b860e  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8614  8b8de4feffff          MOV ECX,dword ptr [EBP + 0xfffffee4]
005b861a  89886c050000          MOV dword ptr [EAX + 0x56c],ECX
005b8620  eb1b                  JMP 0x005b863d
005b8622  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
005b8625  8995e0feffff          MOV dword ptr [EBP + 0xfffffee0],EDX
005b862b  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8631  8b8de0feffff          MOV ECX,dword ptr [EBP + 0xfffffee0]
005b8637  89886c050000          MOV dword ptr [EAX + 0x56c],ECX
005b863d  c7857cffffff00000000  MOV dword ptr [EBP + 0xffffff7c],0x0
005b8647  83bd78ffffff00        CMP dword ptr [EBP + 0xffffff78],0x0
005b864e  7419                  JZ 0x005b8669
005b8650  8b9578ffffff          MOV EDX,dword ptr [EBP + 0xffffff78]
005b8656  8b02                  MOV EAX,dword ptr [EDX]
005b8658  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
005b865e  8b5030                MOV EDX,dword ptr [EAX + 0x30]
005b8661  ffd2                  CALL EDX
005b8663  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
005b8669  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005b866e  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005b8671  898ddcfeffff          MOV dword ptr [EBP + 0xfffffedc],ECX
005b8677  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
005b867a  52                    PUSH EDX
005b867b  8b8ddcfeffff          MOV ECX,dword ptr [EBP + 0xfffffedc]
005b8681  e85a3af6ff            CALL 0x0051c0e0
005b8686  8945c4                MOV dword ptr [EBP + -0x3c],EAX
005b8689  837dc400              CMP dword ptr [EBP + -0x3c],0x0
005b868d  754f                  JNZ 0x005b86de
005b868f  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8695  e80650fbff            CALL 0x0056d6a0
005b869a  50                    PUSH EAX
005b869b  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b86a1  8b10                  MOV EDX,dword ptr [EAX]
005b86a3  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b86a9  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b86ac  ffd0                  CALL EAX
005b86ae  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b86b4  c7817410000000000000  MOV dword ptr [ECX + 0x1074],0x0
005b86be  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b86c4  e82715fdff            CALL 0x00589bf0
005b86c9  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b86cf  e85c15fdff            CALL 0x00589c30
005b86d4  a1bc449900            MOV EAX,[0x009944bc]
005b86d9  e9b70f0000            JMP 0x005b9695
005b86de  8b5590                MOV EDX,dword ptr [EBP + -0x70]
005b86e1  52                    PUSH EDX
005b86e2  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b86e5  e876bb0400            CALL 0x00604260
005b86ea  8945e0                MOV dword ptr [EBP + -0x20],EAX
005b86ed  837de000              CMP dword ptr [EBP + -0x20],0x0
005b86f1  754f                  JNZ 0x005b8742
005b86f3  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b86f9  e8a24ffbff            CALL 0x0056d6a0
005b86fe  50                    PUSH EAX
005b86ff  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8705  8b10                  MOV EDX,dword ptr [EAX]
005b8707  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b870d  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b8710  ffd0                  CALL EAX
005b8712  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8718  c7817410000000000000  MOV dword ptr [ECX + 0x1074],0x0
005b8722  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8728  e8c314fdff            CALL 0x00589bf0
005b872d  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8733  e8f814fdff            CALL 0x00589c30
005b8738  a1bc449900            MOV EAX,[0x009944bc]
005b873d  e9530f0000            JMP 0x005b9695
005b8742  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
005b8745  668b4202              MOV AX,word ptr [EDX + 0x2]
005b8749  668985dafeffff        MOV word ptr [EBP + 0xfffffeda],AX
005b8750  0fb78ddafeffff        MOVZX ECX,word ptr [EBP + 0xfffffeda]
005b8757  894dd8                MOV dword ptr [EBP + -0x28],ECX
005b875a  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
005b875d  52                    PUSH EDX
005b875e  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8764  e8c7dcfbff            CALL 0x00576430
005b8769  8945d8                MOV dword ptr [EBP + -0x28],EAX
005b876c  a1d0b4a100            MOV EAX,[0x00a1b4d0]
005b8771  8985d4feffff          MOV dword ptr [EBP + 0xfffffed4],EAX
005b8777  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
005b877a  51                    PUSH ECX
005b877b  8b95d4feffff          MOV EDX,dword ptr [EBP + 0xfffffed4]
005b8781  8b8a04010000          MOV ECX,dword ptr [EDX + 0x104]
005b8787  e8c48c1000            CALL 0x006c1450
005b878c  894580                MOV dword ptr [EBP + -0x80],EAX
005b878f  837d8000              CMP dword ptr [EBP + -0x80],0x0
005b8793  754f                  JNZ 0x005b87e4
005b8795  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b879b  e8004ffbff            CALL 0x0056d6a0
005b87a0  50                    PUSH EAX
005b87a1  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b87a7  8b10                  MOV EDX,dword ptr [EAX]
005b87a9  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b87af  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b87b2  ffd0                  CALL EAX
005b87b4  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b87ba  c7817410000000000000  MOV dword ptr [ECX + 0x1074],0x0
005b87c4  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b87ca  e82114fdff            CALL 0x00589bf0
005b87cf  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b87d5  e85614fdff            CALL 0x00589c30
005b87da  a1bc449900            MOV EAX,[0x009944bc]
005b87df  e9b10e0000            JMP 0x005b9695
005b87e4  8b5580                MOV EDX,dword ptr [EBP + -0x80]
005b87e7  8b8254010000          MOV EAX,dword ptr [EDX + 0x154]
005b87ed  8985d0feffff          MOV dword ptr [EBP + 0xfffffed0],EAX
005b87f3  83bdd0feffff00        CMP dword ptr [EBP + 0xfffffed0],0x0
005b87fa  7420                  JZ 0x005b881c
005b87fc  6a01                  PUSH 0x1
005b87fe  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8804  e817b5faff            CALL 0x00563d20
005b8809  6a00                  PUSH 0x0
005b880b  6a01                  PUSH 0x1
005b880d  6a01                  PUSH 0x1
005b880f  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8815  e8461dfcff            CALL 0x0057a560
005b881a  eb0d                  JMP 0x005b8829
005b881c  6a02                  PUSH 0x2
005b881e  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8824  e8f7b4faff            CALL 0x00563d20
005b8829  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b882f  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005b8832  8995ccfeffff          MOV dword ptr [EBP + 0xfffffecc],EDX
005b8838  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005b883d  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005b8840  898dc8feffff          MOV dword ptr [EBP + 0xfffffec8],ECX
005b8846  8d55bc                LEA EDX,[EBP + -0x44]
005b8849  52                    PUSH EDX
005b884a  8d45b0                LEA EAX,[EBP + -0x50]
005b884d  50                    PUSH EAX
005b884e  8b8dccfeffff          MOV ECX,dword ptr [EBP + 0xfffffecc]
005b8854  51                    PUSH ECX
005b8855  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
005b885b  e8f03af6ff            CALL 0x0051c350
005b8860  8bc8                  MOV ECX,EAX
005b8862  e8b924f6ff            CALL 0x0051ad20
005b8867  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
005b886d  8b4208                MOV EAX,dword ptr [EDX + 0x8]
005b8870  8985c4feffff          MOV dword ptr [EBP + 0xfffffec4],EAX
005b8876  8d4da4                LEA ECX,[EBP + -0x5c]
005b8879  51                    PUSH ECX
005b887a  8d55c0                LEA EDX,[EBP + -0x40]
005b887d  52                    PUSH EDX
005b887e  8b8574ffffff          MOV EAX,dword ptr [EBP + 0xffffff74]
005b8884  50                    PUSH EAX
005b8885  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
005b8888  51                    PUSH ECX
005b8889  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
005b888c  52                    PUSH EDX
005b888d  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
005b8890  50                    PUSH EAX
005b8891  8b8dc4feffff          MOV ECX,dword ptr [EBP + 0xfffffec4]
005b8897  e8d43af6ff            CALL 0x0051c370
005b889c  8bc8                  MOV ECX,EAX
005b889e  e8bd26f6ff            CALL 0x0051af60
005b88a3  d945b4                FLD float ptr [EBP + -0x4c]
005b88a6  d99db8feffff          FSTP float ptr [EBP + 0xfffffeb8]
005b88ac  d945a8                FLD float ptr [EBP + -0x58]
005b88af  d99dbcfeffff          FSTP float ptr [EBP + 0xfffffebc]
005b88b5  d94594                FLD float ptr [EBP + -0x6c]
005b88b8  d99dc0feffff          FSTP float ptr [EBP + 0xfffffec0]
005b88be  d985b8feffff          FLD float ptr [EBP + 0xfffffeb8]
005b88c4  d99d30ffffff          FSTP float ptr [EBP + 0xffffff30]
005b88ca  d985bcfeffff          FLD float ptr [EBP + 0xfffffebc]
005b88d0  d99d34ffffff          FSTP float ptr [EBP + 0xffffff34]
005b88d6  d985c0feffff          FLD float ptr [EBP + 0xfffffec0]
005b88dc  d99d38ffffff          FSTP float ptr [EBP + 0xffffff38]
005b88e2  8b8d30ffffff          MOV ECX,dword ptr [EBP + 0xffffff30]
005b88e8  894de4                MOV dword ptr [EBP + -0x1c],ECX
005b88eb  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
005b88f1  8955e8                MOV dword ptr [EBP + -0x18],EDX
005b88f4  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
005b88fa  8945ec                MOV dword ptr [EBP + -0x14],EAX
005b88fd  8d4d98                LEA ECX,[EBP + -0x68]
005b8900  e83bac1700            CALL 0x00733540
005b8905  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005b890c  6838649900            PUSH 0x996438
005b8911  8d8d28ffffff          LEA ECX,[EBP + 0xffffff28]
005b8917  e854ac1700            CALL 0x00733570
005b891c  c645fc01              MOV byte ptr [EBP + -0x4],0x1
005b8920  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
005b8926  8b9130010000          MOV EDX,dword ptr [ECX + 0x130]
005b892c  8995b4feffff          MOV dword ptr [EBP + 0xfffffeb4],EDX
005b8932  8b85b4feffff          MOV EAX,dword ptr [EBP + 0xfffffeb4]
005b8938  8b4810                MOV ECX,dword ptr [EAX + 0x10]
005b893b  898db0feffff          MOV dword ptr [EBP + 0xfffffeb0],ECX
005b8941  8d5598                LEA EDX,[EBP + -0x68]
005b8944  52                    PUSH EDX
005b8945  8d8528ffffff          LEA EAX,[EBP + 0xffffff28]
005b894b  50                    PUSH EAX
005b894c  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8952  0fb79184110000        MOVZX EDX,word ptr [ECX + 0x1184]
005b8959  52                    PUSH EDX
005b895a  8b8db0feffff          MOV ECX,dword ptr [EBP + 0xfffffeb0]
005b8960  e86b281600            CALL 0x0071b1d0
005b8965  c645fc00              MOV byte ptr [EBP + -0x4],0x0
005b8969  8d8d28ffffff          LEA ECX,[EBP + 0xffffff28]
005b896f  e80cae1700            CALL 0x00733780
005b8974  68401b9900            PUSH 0x991b40
005b8979  8d4d98                LEA ECX,[EBP + -0x68]
005b897c  e8dfc51700            CALL 0x00734f60
005b8981  85c0                  TEST EAX,EAX
005b8983  7511                  JNZ 0x005b8996
005b8985  6834639900            PUSH 0x996334
005b898a  8d4d98                LEA ECX,[EBP + -0x68]
005b898d  e8cec51700            CALL 0x00734f60
005b8992  85c0                  TEST EAX,EAX
005b8994  7409                  JZ 0x005b899f
005b8996  c745a001000000        MOV dword ptr [EBP + -0x60],0x1
005b899d  eb07                  JMP 0x005b89a6
005b899f  c745a000000000        MOV dword ptr [EBP + -0x60],0x0
005b89a6  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005b89a9  c7407000000000        MOV dword ptr [EAX + 0x70],0x0
005b89b0  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
005b89b7  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b89ba  e871e41100            CALL 0x006d6e30
005b89bf  8985acfeffff          MOV dword ptr [EBP + 0xfffffeac],EAX
005b89c5  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
005b89cb  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
005b89d1  8895abfeffff          MOV byte ptr [EBP + 0xfffffeab],DL
005b89d7  0fb685abfeffff        MOVZX EAX,byte ptr [EBP + 0xfffffeab]
005b89de  898598fdffff          MOV dword ptr [EBP + 0xfffffd98],EAX
005b89e4  8b8d98fdffff          MOV ECX,dword ptr [EBP + 0xfffffd98]
005b89ea  83e906                SUB ECX,0x6
005b89ed  898d98fdffff          MOV dword ptr [EBP + 0xfffffd98],ECX
005b89f3  83bd98fdffff2b        CMP dword ptr [EBP + 0xfffffd98],0x2b
005b89fa  0f8701030000          JA 0x005b8d01
005b8a00  8b9598fdffff          MOV EDX,dword ptr [EBP + 0xfffffd98]
005b8a06  0fb682c0965b00        MOVZX EAX,byte ptr [EDX + 0x5b96c0]
005b8a0d  ff2485a8965b00        JMP dword ptr [EAX*0x4 + 0x5b96a8]
005b8a14  837da000              CMP dword ptr [EBP + -0x60],0x0
005b8a18  7424                  JZ 0x005b8a3e
005b8a1a  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
005b8a20  3b8da0fdffff          CMP ECX,dword ptr [EBP + 0xfffffda0]
005b8a26  750b                  JNZ 0x005b8a33
005b8a28  ba56270000            MOV EDX,0x2756
005b8a2d  668955dc              MOV word ptr [EBP + -0x24],DX
005b8a31  eb09                  JMP 0x005b8a3c
005b8a33  b84d270000            MOV EAX,0x274d
005b8a38  668945dc              MOV word ptr [EBP + -0x24],AX
005b8a3c  eb09                  JMP 0x005b8a47
005b8a3e  b911270000            MOV ECX,0x2711
005b8a43  66894ddc              MOV word ptr [EBP + -0x24],CX
005b8a47  c7458cee020000        MOV dword ptr [EBP + -0x74],0x2ee
005b8a4e  c74584dc050000        MOV dword ptr [EBP + -0x7c],0x5dc
005b8a55  e9d2020000            JMP 0x005b8d2c
005b8a5a  837da000              CMP dword ptr [EBP + -0x60],0x0
005b8a5e  0f8492010000          JZ 0x005b8bf6
005b8a64  c78560ffffff00000000  MOV dword ptr [EBP + 0xffffff60],0x0
005b8a6e  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b8a74  8b8250110000          MOV EAX,dword ptr [EDX + 0x1150]
005b8a7a  8985a4feffff          MOV dword ptr [EBP + 0xfffffea4],EAX
005b8a80  6800010000            PUSH 0x100
005b8a85  8b8da4feffff          MOV ECX,dword ptr [EBP + 0xfffffea4]
005b8a8b  e8907b1100            CALL 0x006d0620
005b8a90  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
005b8a96  83bd5cffffff00        CMP dword ptr [EBP + 0xffffff5c],0x0
005b8a9d  7439                  JZ 0x005b8ad8
005b8a9f  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
005b8aa5  e886e31100            CALL 0x006d6e30
005b8aaa  8985a0feffff          MOV dword ptr [EBP + 0xfffffea0],EAX
005b8ab0  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
005b8ab6  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
005b8abc  88959ffeffff          MOV byte ptr [EBP + 0xfffffe9f],DL
005b8ac2  0fb6859ffeffff        MOVZX EAX,byte ptr [EBP + 0xfffffe9f]
005b8ac9  83f830                CMP EAX,0x30
005b8acc  750a                  JNZ 0x005b8ad8
005b8ace  c78560ffffff01000000  MOV dword ptr [EBP + 0xffffff60],0x1
005b8ad8  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
005b8adf  756a                  JNZ 0x005b8b4b
005b8ae1  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8ae7  8b9150110000          MOV EDX,dword ptr [ECX + 0x1150]
005b8aed  899598feffff          MOV dword ptr [EBP + 0xfffffe98],EDX
005b8af3  6880000000            PUSH 0x80
005b8af8  8b8d98feffff          MOV ECX,dword ptr [EBP + 0xfffffe98]
005b8afe  e81d7b1100            CALL 0x006d0620
005b8b03  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
005b8b09  83bd5cffffff00        CMP dword ptr [EBP + 0xffffff5c],0x0
005b8b10  7439                  JZ 0x005b8b4b
005b8b12  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
005b8b18  e813e31100            CALL 0x006d6e30
005b8b1d  898594feffff          MOV dword ptr [EBP + 0xfffffe94],EAX
005b8b23  8b8594feffff          MOV EAX,dword ptr [EBP + 0xfffffe94]
005b8b29  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
005b8b2f  888d93feffff          MOV byte ptr [EBP + 0xfffffe93],CL
005b8b35  0fb69593feffff        MOVZX EDX,byte ptr [EBP + 0xfffffe93]
005b8b3c  83fa30                CMP EDX,0x30
005b8b3f  750a                  JNZ 0x005b8b4b
005b8b41  c78560ffffff01000000  MOV dword ptr [EBP + 0xffffff60],0x1
005b8b4b  83bd60ffffff00        CMP dword ptr [EBP + 0xffffff60],0x0
005b8b52  7415                  JZ 0x005b8b69
005b8b54  b8b1280000            MOV EAX,0x28b1
005b8b59  668945dc              MOV word ptr [EBP + -0x24],AX
005b8b5d  c7458cb6030000        MOV dword ptr [EBP + -0x74],0x3b6
005b8b64  e98b000000            JMP 0x005b8bf4
005b8b69  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8b6f  81c194000000          ADD ECX,0x94
005b8b75  51                    PUSH ECX
005b8b76  8d55e4                LEA EDX,[EBP + -0x1c]
005b8b79  52                    PUSH EDX
005b8b7a  8d8550ffffff          LEA EAX,[EBP + 0xffffff50]
005b8b80  50                    PUSH EAX
005b8b81  e82ac7e8ff            CALL 0x004452b0
005b8b86  83c40c                ADD ESP,0xc
005b8b89  d905d0f5a000          FLD float ptr [0x00a0f5d0]
005b8b8f  d80dd0f5a000          FMUL float ptr [0x00a0f5d0]
005b8b95  d98558ffffff          FLD float ptr [EBP + 0xffffff58]
005b8b9b  d88d58ffffff          FMUL float ptr [EBP + 0xffffff58]
005b8ba1  d98554ffffff          FLD float ptr [EBP + 0xffffff54]
005b8ba7  d88d54ffffff          FMUL float ptr [EBP + 0xffffff54]
005b8bad  d98550ffffff          FLD float ptr [EBP + 0xffffff50]
005b8bb3  d88d50ffffff          FMUL float ptr [EBP + 0xffffff50]
005b8bb9  dec1                  FADDP
005b8bbb  dec1                  FADDP
005b8bbd  d99d94fdffff          FSTP float ptr [EBP + 0xfffffd94]
005b8bc3  d98594fdffff          FLD float ptr [EBP + 0xfffffd94]
005b8bc9  ded9                  FCOMPP
005b8bcb  dfe0                  FNSTSW AX
005b8bcd  f6c401                TEST AH,0x1
005b8bd0  7512                  JNZ 0x005b8be4
005b8bd2  b991270000            MOV ECX,0x2791
005b8bd7  66894ddc              MOV word ptr [EBP + -0x24],CX
005b8bdb  c7458c20030000        MOV dword ptr [EBP + -0x74],0x320
005b8be2  eb10                  JMP 0x005b8bf4
005b8be4  ba92270000            MOV EDX,0x2792
005b8be9  668955dc              MOV word ptr [EBP + -0x24],DX
005b8bed  c7458cbc020000        MOV dword ptr [EBP + -0x74],0x2bc
005b8bf4  eb10                  JMP 0x005b8c06
005b8bf6  b811270000            MOV EAX,0x2711
005b8bfb  668945dc              MOV word ptr [EBP + -0x24],AX
005b8bff  c7458c20030000        MOV dword ptr [EBP + -0x74],0x320
005b8c06  c74584dc050000        MOV dword ptr [EBP + -0x7c],0x5dc
005b8c0d  c745f001000000        MOV dword ptr [EBP + -0x10],0x1
005b8c14  e913010000            JMP 0x005b8d2c
005b8c19  837da000              CMP dword ptr [EBP + -0x60],0x0
005b8c1d  740b                  JZ 0x005b8c2a
005b8c1f  b998270000            MOV ECX,0x2798
005b8c24  66894ddc              MOV word ptr [EBP + -0x24],CX
005b8c28  eb09                  JMP 0x005b8c33
005b8c2a  ba11270000            MOV EDX,0x2711
005b8c2f  668955dc              MOV word ptr [EBP + -0x24],DX
005b8c33  c7458cee020000        MOV dword ptr [EBP + -0x74],0x2ee
005b8c3a  c74584dc050000        MOV dword ptr [EBP + -0x7c],0x5dc
005b8c41  e9e6000000            JMP 0x005b8d2c
005b8c46  837da000              CMP dword ptr [EBP + -0x60],0x0
005b8c4a  740b                  JZ 0x005b8c57
005b8c4c  b898270000            MOV EAX,0x2798
005b8c51  668945dc              MOV word ptr [EBP + -0x24],AX
005b8c55  eb09                  JMP 0x005b8c60
005b8c57  b911270000            MOV ECX,0x2711
005b8c5c  66894ddc              MOV word ptr [EBP + -0x24],CX
005b8c60  c7458c58020000        MOV dword ptr [EBP + -0x74],0x258
005b8c67  c74584e8030000        MOV dword ptr [EBP + -0x7c],0x3e8
005b8c6e  e9b9000000            JMP 0x005b8d2c
005b8c73  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8c79  e81222fdff            CALL 0x0058ae90
005b8c7e  89854cffffff          MOV dword ptr [EBP + 0xffffff4c],EAX
005b8c84  c74584dc050000        MOV dword ptr [EBP + -0x7c],0x5dc
005b8c8b  83bd4cffffff00        CMP dword ptr [EBP + 0xffffff4c],0x0
005b8c92  7448                  JZ 0x005b8cdc
005b8c94  68f92a0000            PUSH 0x2af9
005b8c99  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
005b8c9f  e86c4c2300            CALL 0x007ed910
005b8ca4  d9bd92fdffff          FNSTCW word ptr [EBP + 0xfffffd92]
005b8caa  0fb78592fdffff        MOVZX EAX,word ptr [EBP + 0xfffffd92]
005b8cb1  0d000c0000            OR EAX,0xc00
005b8cb6  89858cfdffff          MOV dword ptr [EBP + 0xfffffd8c],EAX
005b8cbc  d9ad8cfdffff          FLDCW word ptr [EBP + 0xfffffd8c]
005b8cc2  dfbd84fdffff          FISTP qword ptr [EBP + 0xfffffd84]
005b8cc8  d9ad92fdffff          FLDCW word ptr [EBP + 0xfffffd92]
005b8cce  8b9584fdffff          MOV EDX,dword ptr [EBP + 0xfffffd84]
005b8cd4  83ea32                SUB EDX,0x32
005b8cd7  89558c                MOV dword ptr [EBP + -0x74],EDX
005b8cda  eb07                  JMP 0x005b8ce3
005b8cdc  c7458c2c010000        MOV dword ptr [EBP + -0x74],0x12c
005b8ce3  8b45a4                MOV EAX,dword ptr [EBP + -0x5c]
005b8ce6  3b458c                CMP EAX,dword ptr [EBP + -0x74]
005b8ce9  730b                  JNC 0x005b8cf6
005b8ceb  b9f92a0000            MOV ECX,0x2af9
005b8cf0  66894ddc              MOV word ptr [EBP + -0x24],CX
005b8cf4  eb09                  JMP 0x005b8cff
005b8cf6  bafa2a0000            MOV EDX,0x2afa
005b8cfb  668955dc              MOV word ptr [EBP + -0x24],DX
005b8cff  eb2b                  JMP 0x005b8d2c
005b8d01  b821270000            MOV EAX,0x2721
005b8d06  668945dc              MOV word ptr [EBP + -0x24],AX
005b8d0a  c7458c01000000        MOV dword ptr [EBP + -0x74],0x1
005b8d11  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
005b8d14  8b91b4000000          MOV EDX,dword ptr [ECX + 0xb4]
005b8d1a  899580feffff          MOV dword ptr [EBP + 0xfffffe80],EDX
005b8d20  8b458c                MOV EAX,dword ptr [EBP + -0x74]
005b8d23  038580feffff          ADD EAX,dword ptr [EBP + 0xfffffe80]
005b8d29  894584                MOV dword ptr [EBP + -0x7c],EAX
005b8d2c  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8d32  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
005b8d38  89957cfeffff          MOV dword ptr [EBP + 0xfffffe7c],EDX
005b8d3e  8b857cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe7c]
005b8d44  8b88840a0000          MOV ECX,dword ptr [EAX + 0xa84]
005b8d4a  898d78feffff          MOV dword ptr [EBP + 0xfffffe78],ECX
005b8d50  83bd78feffff00        CMP dword ptr [EBP + 0xfffffe78],0x0
005b8d57  0f8554010000          JNZ 0x005b8eb1
005b8d5d  c78548ffffff00000000  MOV dword ptr [EBP + 0xffffff48],0x0
005b8d67  c78544ffffff00000000  MOV dword ptr [EBP + 0xffffff44],0x0
005b8d71  837df000              CMP dword ptr [EBP + -0x10],0x0
005b8d75  7416                  JZ 0x005b8d8d
005b8d77  c78548ffffff01000000  MOV dword ptr [EBP + 0xffffff48],0x1
005b8d81  c78544ffffff01000000  MOV dword ptr [EBP + 0xffffff44],0x1
005b8d8b  eb55                  JMP 0x005b8de2
005b8d8d  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b8d93  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
005b8d99  898574feffff          MOV dword ptr [EBP + 0xfffffe74],EAX
005b8d9f  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
005b8da2  51                    PUSH ECX
005b8da3  8b8d74feffff          MOV ECX,dword ptr [EBP + 0xfffffe74]
005b8da9  e84282fdff            CALL 0x00590ff0
005b8dae  898548ffffff          MOV dword ptr [EBP + 0xffffff48],EAX
005b8db4  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b8dba  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
005b8dc0  898570feffff          MOV dword ptr [EBP + 0xfffffe70],EAX
005b8dc6  8d8d48ffffff          LEA ECX,[EBP + 0xffffff48]
005b8dcc  51                    PUSH ECX
005b8dcd  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
005b8dd0  52                    PUSH EDX
005b8dd1  8b8d70feffff          MOV ECX,dword ptr [EBP + 0xfffffe70]
005b8dd7  e8d484fdff            CALL 0x005912b0
005b8ddc  898544ffffff          MOV dword ptr [EBP + 0xffffff44],EAX
005b8de2  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8de8  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
005b8dee  898d6cfeffff          MOV dword ptr [EBP + 0xfffffe6c],ECX
005b8df4  6a00                  PUSH 0x0
005b8df6  6a00                  PUSH 0x0
005b8df8  8b9544ffffff          MOV EDX,dword ptr [EBP + 0xffffff44]
005b8dfe  52                    PUSH EDX
005b8dff  8b8548ffffff          MOV EAX,dword ptr [EBP + 0xffffff48]
005b8e05  50                    PUSH EAX
005b8e06  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
005b8e09  51                    PUSH ECX
005b8e0a  8b8d6cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe6c]
005b8e10  e81b59fdff            CALL 0x0058e730
005b8e15  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b8e1b  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
005b8e21  898568feffff          MOV dword ptr [EBP + 0xfffffe68],EAX
005b8e27  6a00                  PUSH 0x0
005b8e29  6a00                  PUSH 0x0
005b8e2b  8b8d68feffff          MOV ECX,dword ptr [EBP + 0xfffffe68]
005b8e31  e89a58fdff            CALL 0x0058e6d0
005b8e36  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8e3c  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005b8e3f  899564feffff          MOV dword ptr [EBP + 0xfffffe64],EDX
005b8e45  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8e4b  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
005b8e51  898d60feffff          MOV dword ptr [EBP + 0xfffffe60],ECX
005b8e57  8b9564feffff          MOV EDX,dword ptr [EBP + 0xfffffe64]
005b8e5d  52                    PUSH EDX
005b8e5e  6a01                  PUSH 0x1
005b8e60  8b8d60feffff          MOV ECX,dword ptr [EBP + 0xfffffe60]
005b8e66  e8f56dfdff            CALL 0x0058fc60
005b8e6b  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8e71  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
005b8e77  898d5cfeffff          MOV dword ptr [EBP + 0xfffffe5c],ECX
005b8e7d  6a00                  PUSH 0x0
005b8e7f  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
005b8e82  52                    PUSH EDX
005b8e83  8b8d5cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe5c]
005b8e89  e8426efdff            CALL 0x0058fcd0
005b8e8e  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8e94  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
005b8e9a  898d58feffff          MOV dword ptr [EBP + 0xfffffe58],ECX
005b8ea0  6a00                  PUSH 0x0
005b8ea2  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
005b8ea5  52                    PUSH EDX
005b8ea6  8b8d58feffff          MOV ECX,dword ptr [EBP + 0xfffffe58]
005b8eac  e89f6efdff            CALL 0x0058fd50
005b8eb1  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8eb7  8b8874100000          MOV ECX,dword ptr [EAX + 0x1074]
005b8ebd  898d54feffff          MOV dword ptr [EBP + 0xfffffe54],ECX
005b8ec3  83bd54feffff00        CMP dword ptr [EBP + 0xfffffe54],0x0
005b8eca  0f85b3000000          JNZ 0x005b8f83
005b8ed0  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b8ed3  e858df1100            CALL 0x006d6e30
005b8ed8  898550feffff          MOV dword ptr [EBP + 0xfffffe50],EAX
005b8ede  8b9550feffff          MOV EDX,dword ptr [EBP + 0xfffffe50]
005b8ee4  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
005b8eea  88854ffeffff          MOV byte ptr [EBP + 0xfffffe4f],AL
005b8ef0  0fb68d4ffeffff        MOVZX ECX,byte ptr [EBP + 0xfffffe4f]
005b8ef7  83f919                CMP ECX,0x19
005b8efa  742c                  JZ 0x005b8f28
005b8efc  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b8eff  e82cdf1100            CALL 0x006d6e30
005b8f04  898548feffff          MOV dword ptr [EBP + 0xfffffe48],EAX
005b8f0a  8b9548feffff          MOV EDX,dword ptr [EBP + 0xfffffe48]
005b8f10  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
005b8f16  888547feffff          MOV byte ptr [EBP + 0xfffffe47],AL
005b8f1c  0fb68d47feffff        MOVZX ECX,byte ptr [EBP + 0xfffffe47]
005b8f23  83f919                CMP ECX,0x19
005b8f26  754b                  JNZ 0x005b8f73
005b8f28  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b8f2e  8b82e8100000          MOV EAX,dword ptr [EDX + 0x10e8]
005b8f34  898540feffff          MOV dword ptr [EBP + 0xfffffe40],EAX
005b8f3a  83bd40feffff00        CMP dword ptr [EBP + 0xfffffe40],0x0
005b8f41  7420                  JZ 0x005b8f63
005b8f43  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
005b8f49  8b5108                MOV EDX,dword ptr [ECX + 0x8]
005b8f4c  89953cfeffff          MOV dword ptr [EBP + 0xfffffe3c],EDX
005b8f52  6a02                  PUSH 0x2
005b8f54  8b8d3cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe3c]
005b8f5a  e8d138f6ff            CALL 0x0051c830
005b8f5f  85c0                  TEST EAX,EAX
005b8f61  7510                  JNZ 0x005b8f73
005b8f63  680000007f            PUSH 0x7f000000
005b8f68  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8f6e  e8fd57e5ff            CALL 0x0040e770
005b8f73  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b8f79  c7807410000001000000  MOV dword ptr [EAX + 0x1074],0x1
005b8f83  8b4da4                MOV ECX,dword ptr [EBP + -0x5c]
005b8f86  3b4d8c                CMP ECX,dword ptr [EBP + -0x74]
005b8f89  0f83ac020000          JNC 0x005b923b
005b8f8f  0fb65590              MOVZX EDX,byte ptr [EBP + -0x70]
005b8f93  52                    PUSH EDX
005b8f94  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b8f97  e8b4ab0400            CALL 0x00603b50
005b8f9c  0fb7c0                MOVZX EAX,AX
005b8f9f  83f801                CMP EAX,0x1
005b8fa2  0f8db2000000          JGE 0x005b905a
005b8fa8  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005b8fab  668b5106              MOV DX,word ptr [ECX + 0x6]
005b8faf  6689953afeffff        MOV word ptr [EBP + 0xfffffe3a],DX
005b8fb6  0fb7853afeffff        MOVZX EAX,word ptr [EBP + 0xfffffe3a]
005b8fbd  83f80d                CMP EAX,0xd
005b8fc0  0f8494000000          JZ 0x005b905a
005b8fc6  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005b8fc9  668b5106              MOV DX,word ptr [ECX + 0x6]
005b8fcd  66899538feffff        MOV word ptr [EBP + 0xfffffe38],DX
005b8fd4  0fb78538feffff        MOVZX EAX,word ptr [EBP + 0xfffffe38]
005b8fdb  83f807                CMP EAX,0x7
005b8fde  747a                  JZ 0x005b905a
005b8fe0  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8fe6  e8b546fbff            CALL 0x0056d6a0
005b8feb  50                    PUSH EAX
005b8fec  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8ff2  8b11                  MOV EDX,dword ptr [ECX]
005b8ff4  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b8ffa  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b8ffd  ffd0                  CALL EAX
005b8fff  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9005  c7817410000000000000  MOV dword ptr [ECX + 0x1074],0x0
005b900f  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9015  e8d60bfdff            CALL 0x00589bf0
005b901a  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9020  e80b0cfdff            CALL 0x00589c30
005b9025  6a00                  PUSH 0x0
005b9027  6a17                  PUSH 0x17
005b9029  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b902f  e86cbdfcff            CALL 0x00584da0
005b9034  8b15bc449900          MOV EDX,dword ptr [0x009944bc]
005b903a  899524ffffff          MOV dword ptr [EBP + 0xffffff24],EDX
005b9040  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005b9047  8d4d98                LEA ECX,[EBP + -0x68]
005b904a  e831a71700            CALL 0x00733780
005b904f  8b8524ffffff          MOV EAX,dword ptr [EBP + 0xffffff24]
005b9055  e93b060000            JMP 0x005b9695
005b905a  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b9060  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
005b9063  8988e4010000          MOV dword ptr [EAX + 0x1e4],ECX
005b9069  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
005b906c  899534feffff          MOV dword ptr [EBP + 0xfffffe34],EDX
005b9072  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b9078  8b8d34feffff          MOV ECX,dword ptr [EBP + 0xfffffe34]
005b907e  8988e0010000          MOV dword ptr [EAX + 0x1e0],ECX
005b9084  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b908a  81c2d4010000          ADD EDX,0x1d4
005b9090  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
005b9093  8902                  MOV dword ptr [EDX],EAX
005b9095  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005b9098  894a04                MOV dword ptr [EDX + 0x4],ECX
005b909b  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
005b909e  894208                MOV dword ptr [EDX + 0x8],EAX
005b90a1  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b90a7  c781f001000000000000  MOV dword ptr [ECX + 0x1f0],0x0
005b90b1  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
005b90b4  899530feffff          MOV dword ptr [EBP + 0xfffffe30],EDX
005b90ba  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b90c0  8b8d30feffff          MOV ECX,dword ptr [EBP + 0xfffffe30]
005b90c6  898864100000          MOV dword ptr [EAX + 0x1064],ECX
005b90cc  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b90d2  c7826c10000000000000  MOV dword ptr [EDX + 0x106c],0x0
005b90dc  6a00                  PUSH 0x0
005b90de  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b90e4  e827faf8ff            CALL 0x00548b10
005b90e9  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b90ef  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005b90f5  898d2cfeffff          MOV dword ptr [EBP + 0xfffffe2c],ECX
005b90fb  8b952cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe2c]
005b9101  8a828d000000          MOV AL,byte ptr [EDX + 0x8d]
005b9107  88852bfeffff          MOV byte ptr [EBP + 0xfffffe2b],AL
005b910d  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9113  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005b9119  899524feffff          MOV dword ptr [EBP + 0xfffffe24],EDX
005b911f  6a00                  PUSH 0x0
005b9121  0fb6852bfeffff        MOVZX EAX,byte ptr [EBP + 0xfffffe2b]
005b9128  83e801                SUB EAX,0x1
005b912b  50                    PUSH EAX
005b912c  8b8d24feffff          MOV ECX,dword ptr [EBP + 0xfffffe24]
005b9132  e8b9410f00            CALL 0x006ad2f0
005b9137  0fb6c8                MOVZX ECX,AL
005b913a  898d20feffff          MOV dword ptr [EBP + 0xfffffe20],ECX
005b9140  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b9146  8b8520feffff          MOV EAX,dword ptr [EBP + 0xfffffe20]
005b914c  898270100000          MOV dword ptr [EDX + 0x1070],EAX
005b9152  83bd7cffffff00        CMP dword ptr [EBP + 0xffffff7c],0x0
005b9159  0f849e000000          JZ 0x005b91fd
005b915f  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b9162  e8c9dc1100            CALL 0x006d6e30
005b9167  89851cfeffff          MOV dword ptr [EBP + 0xfffffe1c],EAX
005b916d  8b8d1cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe1c]
005b9173  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
005b9179  88951bfeffff          MOV byte ptr [EBP + 0xfffffe1b],DL
005b917f  0fb6851bfeffff        MOVZX EAX,byte ptr [EBP + 0xfffffe1b]
005b9186  83f819                CMP EAX,0x19
005b9189  7458                  JZ 0x005b91e3
005b918b  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b918e  e89ddc1100            CALL 0x006d6e30
005b9193  898514feffff          MOV dword ptr [EBP + 0xfffffe14],EAX
005b9199  8b8d14feffff          MOV ECX,dword ptr [EBP + 0xfffffe14]
005b919f  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
005b91a5  889513feffff          MOV byte ptr [EBP + 0xfffffe13],DL
005b91ab  0fb68513feffff        MOVZX EAX,byte ptr [EBP + 0xfffffe13]
005b91b2  83f82d                CMP EAX,0x2d
005b91b5  742c                  JZ 0x005b91e3
005b91b7  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b91ba  e871dc1100            CALL 0x006d6e30
005b91bf  89850cfeffff          MOV dword ptr [EBP + 0xfffffe0c],EAX
005b91c5  8b8d0cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe0c]
005b91cb  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
005b91d1  88950bfeffff          MOV byte ptr [EBP + 0xfffffe0b],DL
005b91d7  0fb6850bfeffff        MOVZX EAX,byte ptr [EBP + 0xfffffe0b]
005b91de  83f82f                CMP EAX,0x2f
005b91e1  751a                  JNZ 0x005b91fd
005b91e3  0fb74ddc              MOVZX ECX,word ptr [EBP + -0x24]
005b91e7  51                    PUSH ECX
005b91e8  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
005b91ee  8b02                  MOV EAX,dword ptr [EDX]
005b91f0  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
005b91f6  8b507c                MOV EDX,dword ptr [EAX + 0x7c]
005b91f9  ffd2                  CALL EDX
005b91fb  eb18                  JMP 0x005b9215
005b91fd  0fb745dc              MOVZX EAX,word ptr [EBP + -0x24]
005b9201  50                    PUSH EAX
005b9202  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9208  8b11                  MOV EDX,dword ptr [ECX]
005b920a  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9210  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b9213  ffd0                  CALL EAX
005b9215  8b0db4449900          MOV ECX,dword ptr [0x009944b4]
005b921b  898d20ffffff          MOV dword ptr [EBP + 0xffffff20],ECX
005b9221  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005b9228  8d4d98                LEA ECX,[EBP + -0x68]
005b922b  e850a51700            CALL 0x00733780
005b9230  8b8520ffffff          MOV EAX,dword ptr [EBP + 0xffffff20]
005b9236  e95a040000            JMP 0x005b9695
005b923b  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b9241  8b82f0010000          MOV EAX,dword ptr [EDX + 0x1f0]
005b9247  898504feffff          MOV dword ptr [EBP + 0xfffffe04],EAX
005b924d  83bd04feffff00        CMP dword ptr [EBP + 0xfffffe04],0x0
005b9254  0f85aa030000          JNZ 0x005b9604
005b925a  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9260  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
005b9263  8991e4010000          MOV dword ptr [ECX + 0x1e4],EDX
005b9269  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
005b926c  898500feffff          MOV dword ptr [EBP + 0xfffffe00],EAX
005b9272  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9278  8b9500feffff          MOV EDX,dword ptr [EBP + 0xfffffe00]
005b927e  8991e0010000          MOV dword ptr [ECX + 0x1e0],EDX
005b9284  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b928a  05d4010000            ADD EAX,0x1d4
005b928f  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
005b9292  8908                  MOV dword ptr [EAX],ECX
005b9294  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005b9297  895004                MOV dword ptr [EAX + 0x4],EDX
005b929a  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
005b929d  894808                MOV dword ptr [EAX + 0x8],ECX
005b92a0  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b92a6  c782f001000000000000  MOV dword ptr [EDX + 0x1f0],0x0
005b92b0  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
005b92b3  8985fcfdffff          MOV dword ptr [EBP + 0xfffffdfc],EAX
005b92b9  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b92bf  8b95fcfdffff          MOV EDX,dword ptr [EBP + 0xfffffdfc]
005b92c5  899164100000          MOV dword ptr [ECX + 0x1064],EDX
005b92cb  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b92d1  c7806c10000001000000  MOV dword ptr [EAX + 0x106c],0x1
005b92db  6a00                  PUSH 0x0
005b92dd  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b92e3  e828f8f8ff            CALL 0x00548b10
005b92e8  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b92ee  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005b92f4  8995f8fdffff          MOV dword ptr [EBP + 0xfffffdf8],EDX
005b92fa  8b85f8fdffff          MOV EAX,dword ptr [EBP + 0xfffffdf8]
005b9300  8a888d000000          MOV CL,byte ptr [EAX + 0x8d]
005b9306  888df7fdffff          MOV byte ptr [EBP + 0xfffffdf7],CL
005b930c  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b9312  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005b9318  8985f0fdffff          MOV dword ptr [EBP + 0xfffffdf0],EAX
005b931e  6a00                  PUSH 0x0
005b9320  0fb68df7fdffff        MOVZX ECX,byte ptr [EBP + 0xfffffdf7]
005b9327  83e901                SUB ECX,0x1
005b932a  51                    PUSH ECX
005b932b  8b8df0fdffff          MOV ECX,dword ptr [EBP + 0xfffffdf0]
005b9331  e8ba3f0f00            CALL 0x006ad2f0
005b9336  0fb6d0                MOVZX EDX,AL
005b9339  8995ecfdffff          MOV dword ptr [EBP + 0xfffffdec],EDX
005b933f  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b9345  8b8decfdffff          MOV ECX,dword ptr [EBP + 0xfffffdec]
005b934b  898870100000          MOV dword ptr [EAX + 0x1070],ECX
005b9351  83bd7cffffff00        CMP dword ptr [EBP + 0xffffff7c],0x0
005b9358  0f849e000000          JZ 0x005b93fc
005b935e  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b9361  e8cada1100            CALL 0x006d6e30
005b9366  8985e8fdffff          MOV dword ptr [EBP + 0xfffffde8],EAX
005b936c  8b95e8fdffff          MOV EDX,dword ptr [EBP + 0xfffffde8]
005b9372  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
005b9378  8885e7fdffff          MOV byte ptr [EBP + 0xfffffde7],AL
005b937e  0fb68de7fdffff        MOVZX ECX,byte ptr [EBP + 0xfffffde7]
005b9385  83f919                CMP ECX,0x19
005b9388  7458                  JZ 0x005b93e2
005b938a  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b938d  e89eda1100            CALL 0x006d6e30
005b9392  8985e0fdffff          MOV dword ptr [EBP + 0xfffffde0],EAX
005b9398  8b95e0fdffff          MOV EDX,dword ptr [EBP + 0xfffffde0]
005b939e  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
005b93a4  8885dffdffff          MOV byte ptr [EBP + 0xfffffddf],AL
005b93aa  0fb68ddffdffff        MOVZX ECX,byte ptr [EBP + 0xfffffddf]
005b93b1  83f92d                CMP ECX,0x2d
005b93b4  742c                  JZ 0x005b93e2
005b93b6  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b93b9  e872da1100            CALL 0x006d6e30
005b93be  8985d8fdffff          MOV dword ptr [EBP + 0xfffffdd8],EAX
005b93c4  8b95d8fdffff          MOV EDX,dword ptr [EBP + 0xfffffdd8]
005b93ca  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
005b93d0  8885d7fdffff          MOV byte ptr [EBP + 0xfffffdd7],AL
005b93d6  0fb68dd7fdffff        MOVZX ECX,byte ptr [EBP + 0xfffffdd7]
005b93dd  83f92f                CMP ECX,0x2f
005b93e0  751a                  JNZ 0x005b93fc
005b93e2  0fb755dc              MOVZX EDX,word ptr [EBP + -0x24]
005b93e6  52                    PUSH EDX
005b93e7  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
005b93ed  8b10                  MOV EDX,dword ptr [EAX]
005b93ef  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
005b93f5  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b93f8  ffd0                  CALL EAX
005b93fa  eb18                  JMP 0x005b9414
005b93fc  0fb74ddc              MOVZX ECX,word ptr [EBP + -0x24]
005b9400  51                    PUSH ECX
005b9401  8b95a0fdffff          MOV EDX,dword ptr [EBP + 0xfffffda0]
005b9407  8b02                  MOV EAX,dword ptr [EDX]
005b9409  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b940f  8b507c                MOV EDX,dword ptr [EAX + 0x7c]
005b9412  ffd2                  CALL EDX
005b9414  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b941a  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
005b9420  898dd0fdffff          MOV dword ptr [EBP + 0xfffffdd0],ECX
005b9426  8b95d0fdffff          MOV EDX,dword ptr [EBP + 0xfffffdd0]
005b942c  8b82b80a0000          MOV EAX,dword ptr [EDX + 0xab8]
005b9432  8985ccfdffff          MOV dword ptr [EBP + 0xfffffdcc],EAX
005b9438  8b8dccfdffff          MOV ECX,dword ptr [EBP + 0xfffffdcc]
005b943e  83c101                ADD ECX,0x1
005b9441  8b95d0fdffff          MOV EDX,dword ptr [EBP + 0xfffffdd0]
005b9447  898ab80a0000          MOV dword ptr [EDX + 0xab8],ECX
005b944d  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b9450  e8dbd91100            CALL 0x006d6e30
005b9455  8985c8fdffff          MOV dword ptr [EBP + 0xfffffdc8],EAX
005b945b  8b85c8fdffff          MOV EAX,dword ptr [EBP + 0xfffffdc8]
005b9461  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
005b9467  888dc7fdffff          MOV byte ptr [EBP + 0xfffffdc7],CL
005b946d  0fb695c7fdffff        MOVZX EDX,byte ptr [EBP + 0xfffffdc7]
005b9474  83fa06                CMP EDX,0x6
005b9477  7534                  JNZ 0x005b94ad
005b9479  817dd8f6000000        CMP dword ptr [EBP + -0x28],0xf6
005b9480  7d2b                  JGE 0x005b94ad
005b9482  6a00                  PUSH 0x0
005b9484  6a00                  PUSH 0x0
005b9486  6a00                  PUSH 0x0
005b9488  6a00                  PUSH 0x0
005b948a  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
005b948d  50                    PUSH EAX
005b948e  68ff000000            PUSH 0xff
005b9493  680000007f            PUSH 0x7f000000
005b9498  8d4de4                LEA ECX,[EBP + -0x1c]
005b949b  51                    PUSH ECX
005b949c  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
005b949f  52                    PUSH EDX
005b94a0  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b94a6  e8f5edf8ff            CALL 0x005482a0
005b94ab  eb28                  JMP 0x005b94d5
005b94ad  6a00                  PUSH 0x0
005b94af  6a00                  PUSH 0x0
005b94b1  6a00                  PUSH 0x0
005b94b3  6a00                  PUSH 0x0
005b94b5  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
005b94b8  50                    PUSH EAX
005b94b9  68ff000000            PUSH 0xff
005b94be  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
005b94c1  51                    PUSH ECX
005b94c2  8d55e4                LEA EDX,[EBP + -0x1c]
005b94c5  52                    PUSH EDX
005b94c6  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
005b94c9  50                    PUSH EAX
005b94ca  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b94d0  e8cbedf8ff            CALL 0x005482a0
005b94d5  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b94db  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
005b94e1  8995c0fdffff          MOV dword ptr [EBP + 0xfffffdc0],EDX
005b94e7  8b8dc0fdffff          MOV ECX,dword ptr [EBP + 0xfffffdc0]
005b94ed  e88e6cfdff            CALL 0x00590180
005b94f2  83f801                CMP EAX,0x1
005b94f5  751d                  JNZ 0x005b9514
005b94f7  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b94fd  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
005b9503  898dbcfdffff          MOV dword ptr [EBP + 0xfffffdbc],ECX
005b9509  8b8dbcfdffff          MOV ECX,dword ptr [EBP + 0xfffffdbc]
005b950f  e85c85fdff            CALL 0x00591a70
005b9514  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
005b951a  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005b951d  8985b8fdffff          MOV dword ptr [EBP + 0xfffffdb8],EAX
005b9523  8b8db8fdffff          MOV ECX,dword ptr [EBP + 0xfffffdb8]
005b9529  e8d2651700            CALL 0x0072fb00
005b952e  8985b4fdffff          MOV dword ptr [EBP + 0xfffffdb4],EAX
005b9534  8b8db4fdffff          MOV ECX,dword ptr [EBP + 0xfffffdb4]
005b953a  8b9194000000          MOV EDX,dword ptr [ECX + 0x94]
005b9540  8995b0fdffff          MOV dword ptr [EBP + 0xfffffdb0],EDX
005b9546  83bdb0fdffff00        CMP dword ptr [EBP + 0xfffffdb0],0x0
005b954d  7522                  JNZ 0x005b9571
005b954f  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b9555  8b4804                MOV ECX,dword ptr [EAX + 0x4]
005b9558  898dacfdffff          MOV dword ptr [EBP + 0xfffffdac],ECX
005b955e  8b95acfdffff          MOV EDX,dword ptr [EBP + 0xfffffdac]
005b9564  52                    PUSH EDX
005b9565  8b4590                MOV EAX,dword ptr [EBP + -0x70]
005b9568  50                    PUSH EAX
005b9569  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b956c  e82fdb0400            CALL 0x006070a0
005b9571  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
005b9574  e8b7d81100            CALL 0x006d6e30
005b9579  8985a8fdffff          MOV dword ptr [EBP + 0xfffffda8],EAX
005b957f  8b8da8fdffff          MOV ECX,dword ptr [EBP + 0xfffffda8]
005b9585  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
005b958b  8895a7fdffff          MOV byte ptr [EBP + 0xfffffda7],DL
005b9591  0fb685a7fdffff        MOVZX EAX,byte ptr [EBP + 0xfffffda7]
005b9598  83f80c                CMP EAX,0xc
005b959b  7567                  JNZ 0x005b9604
005b959d  b9fa2a0000            MOV ECX,0x2afa
005b95a2  66898d40ffffff        MOV word ptr [EBP + 0xffffff40],CX
005b95a9  c7853cffffff01000000  MOV dword ptr [EBP + 0xffffff3c],0x1
005b95b3  6a04                  PUSH 0x4
005b95b5  8d953cffffff          LEA EDX,[EBP + 0xffffff3c]
005b95bb  52                    PUSH EDX
005b95bc  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005b95bf  83c05c                ADD EAX,0x5c
005b95c2  50                    PUSH EAX
005b95c3  e8d8453600            CALL 0x0091dba0
005b95c8  83c40c                ADD ESP,0xc
005b95cb  6a02                  PUSH 0x2
005b95cd  8d8d40ffffff          LEA ECX,[EBP + 0xffffff40]
005b95d3  51                    PUSH ECX
005b95d4  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005b95d7  83c260                ADD EDX,0x60
005b95da  52                    PUSH EDX
005b95db  e8c0453600            CALL 0x0091dba0
005b95e0  83c40c                ADD ESP,0xc
005b95e3  6a04                  PUSH 0x4
005b95e5  8d4584                LEA EAX,[EBP + -0x7c]
005b95e8  50                    PUSH EAX
005b95e9  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005b95ec  83c164                ADD ECX,0x64
005b95ef  51                    PUSH ECX
005b95f0  e8ab453600            CALL 0x0091dba0
005b95f5  83c40c                ADD ESP,0xc
005b95f8  ba0c000000            MOV EDX,0xc
005b95fd  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005b9600  6689506e              MOV word ptr [EAX + 0x6e],DX
005b9604  8b4da4                MOV ECX,dword ptr [EBP + -0x5c]
005b9607  3b4d84                CMP ECX,dword ptr [EBP + -0x7c]
005b960a  7323                  JNC 0x005b962f
005b960c  8b15b4449900          MOV EDX,dword ptr [0x009944b4]
005b9612  89951cffffff          MOV dword ptr [EBP + 0xffffff1c],EDX
005b9618  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005b961f  8d4d98                LEA ECX,[EBP + -0x68]
005b9622  e859a11700            CALL 0x00733780
005b9627  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
005b962d  eb66                  JMP 0x005b9695
005b962f  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9635  e86640fbff            CALL 0x0056d6a0
005b963a  50                    PUSH EAX
005b963b  8b85a0fdffff          MOV EAX,dword ptr [EBP + 0xfffffda0]
005b9641  8b10                  MOV EDX,dword ptr [EAX]
005b9643  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9649  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
005b964c  ffd0                  CALL EAX
005b964e  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9654  c7817410000000000000  MOV dword ptr [ECX + 0x1074],0x0
005b965e  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b9664  e88705fdff            CALL 0x00589bf0
005b9669  8b8da0fdffff          MOV ECX,dword ptr [EBP + 0xfffffda0]
005b966f  e8bc05fdff            CALL 0x00589c30
005b9674  8b15b8449900          MOV EDX,dword ptr [0x009944b8]
005b967a  899518ffffff          MOV dword ptr [EBP + 0xffffff18],EDX
005b9680  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005b9687  8d4d98                LEA ECX,[EBP + -0x68]
005b968a  e8f1a01700            CALL 0x00733780
005b968f  8b8518ffffff          MOV EAX,dword ptr [EBP + 0xffffff18]
005b9695  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
005b9698  64890d00000000        MOV dword ptr FS:[0x0],ECX
005b969f  59                    POP ECX
005b96a0  8be5                  MOV ESP,EBP
005b96a2  5d                    POP EBP
005b96a3  c20400                RET 0x4
