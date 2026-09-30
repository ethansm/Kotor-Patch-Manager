; FN 00605090 FUN_00605090
00605090  55                    PUSH EBP
00605091  8bec                  MOV EBP,ESP
00605093  6aff                  PUSH -0x1
00605095  68284f9500            PUSH 0x954f28
0060509a  64a100000000          MOV EAX,FS:[0x0]
006050a0  50                    PUSH EAX
006050a1  81ec64010000          SUB ESP,0x164
006050a7  a1201fa100            MOV EAX,[0x00a11f20]
006050ac  33c5                  XOR EAX,EBP
006050ae  50                    PUSH EAX
006050af  8d45f4                LEA EAX,[EBP + -0xc]
006050b2  64a300000000          MOV FS:[0x0],EAX
006050b8  898dacfeffff          MOV dword ptr [EBP + 0xfffffeac],ECX
006050be  a1a4b4a100            MOV EAX,[0x00a1b4a4]
006050c3  8b4808                MOV ECX,dword ptr [EAX + 0x8]
006050c6  898d60ffffff          MOV dword ptr [EBP + 0xffffff60],ECX
006050cc  8b95acfeffff          MOV EDX,dword ptr [EBP + 0xfffffeac]
006050d2  8b02                  MOV EAX,dword ptr [EDX]
006050d4  50                    PUSH EAX
006050d5  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
006050db  e8c06ff1ff            CALL 0x0051c0a0
006050e0  8945d4                MOV dword ptr [EBP + -0x2c],EAX
006050e3  837dd400              CMP dword ptr [EBP + -0x2c],0x0
006050e7  0f849c040000          JZ 0x00605589
006050ed  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006050f0  8b11                  MOV EDX,dword ptr [ECX]
006050f2  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006050f5  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006050f8  ffd0                  CALL EAX
006050fa  85c0                  TEST EAX,EAX
006050fc  0f8482020000          JZ 0x00605384
00605102  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605105  8b09                  MOV ECX,dword ptr [ECX]
00605107  e8241d0d00            CALL 0x006d6e30
0060510c  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
00605112  8b955cffffff          MOV EDX,dword ptr [EBP + 0xffffff5c]
00605118  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
0060511e  88855bffffff          MOV byte ptr [EBP + 0xffffff5b],AL
00605124  0fb68d5bffffff        MOVZX ECX,byte ptr [EBP + 0xffffff5b]
0060512b  83f917                CMP ECX,0x17
0060512e  7575                  JNZ 0x006051a5
00605130  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00605133  8b02                  MOV EAX,dword ptr [EDX]
00605135  8b88cc020000          MOV ECX,dword ptr [EAX + 0x2cc]
0060513b  898d54ffffff          MOV dword ptr [EBP + 0xffffff54],ECX
00605141  6a01                  PUSH 0x1
00605143  8b9554ffffff          MOV EDX,dword ptr [EBP + 0xffffff54]
00605149  52                    PUSH EDX
0060514a  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
0060514d  8b10                  MOV EDX,dword ptr [EAX]
0060514f  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
00605152  8b4230                MOV EAX,dword ptr [EDX + 0x30]
00605155  ffd0                  CALL EAX
00605157  8bc8                  MOV ECX,EAX
00605159  e862c2f7ff            CALL 0x005813c0
0060515e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605161  8b11                  MOV EDX,dword ptr [ECX]
00605163  89559c                MOV dword ptr [EBP + -0x64],EDX
00605166  8b459c                MOV EAX,dword ptr [EBP + -0x64]
00605169  8945a0                MOV dword ptr [EBP + -0x60],EAX
0060516c  837da000              CMP dword ptr [EBP + -0x60],0x0
00605170  7416                  JZ 0x00605188
00605172  6a01                  PUSH 0x1
00605174  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
00605177  8b11                  MOV EDX,dword ptr [ECX]
00605179  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
0060517c  8b02                  MOV EAX,dword ptr [EDX]
0060517e  ffd0                  CALL EAX
00605180  8985a8feffff          MOV dword ptr [EBP + 0xfffffea8],EAX
00605186  eb0a                  JMP 0x00605192
00605188  c785a8feffff00000000  MOV dword ptr [EBP + 0xfffffea8],0x0
00605192  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605195  c70100000000          MOV dword ptr [ECX],0x0
0060519b  b801000000            MOV EAX,0x1
006051a0  e9bf090000            JMP 0x00605b64
006051a5  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006051a8  8b0a                  MOV ECX,dword ptr [EDX]
006051aa  e8811c0d00            CALL 0x006d6e30
006051af  898550ffffff          MOV dword ptr [EBP + 0xffffff50],EAX
006051b5  8b8550ffffff          MOV EAX,dword ptr [EBP + 0xffffff50]
006051bb  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
006051c1  888d4fffffff          MOV byte ptr [EBP + 0xffffff4f],CL
006051c7  0fb6954fffffff        MOVZX EDX,byte ptr [EBP + 0xffffff4f]
006051ce  83fa32                CMP EDX,0x32
006051d1  7575                  JNZ 0x00605248
006051d3  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006051d6  8b08                  MOV ECX,dword ptr [EAX]
006051d8  8b91cc020000          MOV EDX,dword ptr [ECX + 0x2cc]
006051de  899548ffffff          MOV dword ptr [EBP + 0xffffff48],EDX
006051e4  6a01                  PUSH 0x1
006051e6  8b8548ffffff          MOV EAX,dword ptr [EBP + 0xffffff48]
006051ec  50                    PUSH EAX
006051ed  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006051f0  8b11                  MOV EDX,dword ptr [ECX]
006051f2  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006051f5  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006051f8  ffd0                  CALL EAX
006051fa  8bc8                  MOV ECX,EAX
006051fc  e80fc5f7ff            CALL 0x00581710
00605201  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605204  8b11                  MOV EDX,dword ptr [ECX]
00605206  895594                MOV dword ptr [EBP + -0x6c],EDX
00605209  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
0060520c  894598                MOV dword ptr [EBP + -0x68],EAX
0060520f  837d9800              CMP dword ptr [EBP + -0x68],0x0
00605213  7416                  JZ 0x0060522b
00605215  6a01                  PUSH 0x1
00605217  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0060521a  8b11                  MOV EDX,dword ptr [ECX]
0060521c  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
0060521f  8b02                  MOV EAX,dword ptr [EDX]
00605221  ffd0                  CALL EAX
00605223  8985a4feffff          MOV dword ptr [EBP + 0xfffffea4],EAX
00605229  eb0a                  JMP 0x00605235
0060522b  c785a4feffff00000000  MOV dword ptr [EBP + 0xfffffea4],0x0
00605235  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605238  c70100000000          MOV dword ptr [ECX],0x0
0060523e  b801000000            MOV EAX,0x1
00605243  e91c090000            JMP 0x00605b64
00605248  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0060524b  8b0a                  MOV ECX,dword ptr [EDX]
0060524d  e8de1b0d00            CALL 0x006d6e30
00605252  898544ffffff          MOV dword ptr [EBP + 0xffffff44],EAX
00605258  8b8544ffffff          MOV EAX,dword ptr [EBP + 0xffffff44]
0060525e  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
00605264  888d43ffffff          MOV byte ptr [EBP + 0xffffff43],CL
0060526a  0fb69543ffffff        MOVZX EDX,byte ptr [EBP + 0xffffff43]
00605271  83fa33                CMP EDX,0x33
00605274  757a                  JNZ 0x006052f0
00605276  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00605279  8b08                  MOV ECX,dword ptr [EAX]
0060527b  8b91cc020000          MOV EDX,dword ptr [ECX + 0x2cc]
00605281  89953cffffff          MOV dword ptr [EBP + 0xffffff3c],EDX
00605287  6a01                  PUSH 0x1
00605289  8b853cffffff          MOV EAX,dword ptr [EBP + 0xffffff3c]
0060528f  50                    PUSH EAX
00605290  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
00605293  8b11                  MOV EDX,dword ptr [ECX]
00605295  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
00605298  8b4230                MOV EAX,dword ptr [EDX + 0x30]
0060529b  ffd0                  CALL EAX
0060529d  8bc8                  MOV ECX,EAX
0060529f  e88cc3f7ff            CALL 0x00581630
006052a4  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006052a7  8b11                  MOV EDX,dword ptr [ECX]
006052a9  89558c                MOV dword ptr [EBP + -0x74],EDX
006052ac  8b458c                MOV EAX,dword ptr [EBP + -0x74]
006052af  894590                MOV dword ptr [EBP + -0x70],EAX
006052b2  837d9000              CMP dword ptr [EBP + -0x70],0x0
006052b6  7416                  JZ 0x006052ce
006052b8  6a01                  PUSH 0x1
006052ba  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
006052bd  8b11                  MOV EDX,dword ptr [ECX]
006052bf  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
006052c2  8b02                  MOV EAX,dword ptr [EDX]
006052c4  ffd0                  CALL EAX
006052c6  8985a0feffff          MOV dword ptr [EBP + 0xfffffea0],EAX
006052cc  eb0a                  JMP 0x006052d8
006052ce  c785a0feffff00000000  MOV dword ptr [EBP + 0xfffffea0],0x0
006052d8  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006052db  c70100000000          MOV dword ptr [ECX],0x0
006052e1  b801000000            MOV EAX,0x1
006052e6  e979080000            JMP 0x00605b64
006052f0  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006052f3  8b0a                  MOV ECX,dword ptr [EDX]
006052f5  e8361b0d00            CALL 0x006d6e30
006052fa  898538ffffff          MOV dword ptr [EBP + 0xffffff38],EAX
00605300  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
00605306  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
0060530c  888d37ffffff          MOV byte ptr [EBP + 0xffffff37],CL
00605312  0fb69537ffffff        MOVZX EDX,byte ptr [EBP + 0xffffff37]
00605319  83fa2a                CMP EDX,0x2a
0060531c  7561                  JNZ 0x0060537f
0060531e  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00605321  8b08                  MOV ECX,dword ptr [EAX]
00605323  51                    PUSH ECX
00605324  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
00605327  8b02                  MOV EAX,dword ptr [EDX]
00605329  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0060532c  8b5030                MOV EDX,dword ptr [EAX + 0x30]
0060532f  ffd2                  CALL EDX
00605331  8bc8                  MOV ECX,EAX
00605333  e8985bf8ff            CALL 0x0058aed0
00605338  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0060533b  8b08                  MOV ECX,dword ptr [EAX]
0060533d  894d84                MOV dword ptr [EBP + -0x7c],ECX
00605340  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
00605343  895588                MOV dword ptr [EBP + -0x78],EDX
00605346  837d8800              CMP dword ptr [EBP + -0x78],0x0
0060534a  7416                  JZ 0x00605362
0060534c  6a01                  PUSH 0x1
0060534e  8b4588                MOV EAX,dword ptr [EBP + -0x78]
00605351  8b10                  MOV EDX,dword ptr [EAX]
00605353  8b4d88                MOV ECX,dword ptr [EBP + -0x78]
00605356  8b02                  MOV EAX,dword ptr [EDX]
00605358  ffd0                  CALL EAX
0060535a  89859cfeffff          MOV dword ptr [EBP + 0xfffffe9c],EAX
00605360  eb0a                  JMP 0x0060536c
00605362  c7859cfeffff00000000  MOV dword ptr [EBP + 0xfffffe9c],0x0
0060536c  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060536f  c70100000000          MOV dword ptr [ECX],0x0
00605375  b801000000            MOV EAX,0x1
0060537a  e9e5070000            JMP 0x00605b64
0060537f  e905020000            JMP 0x00605589
00605384  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
00605387  8b02                  MOV EAX,dword ptr [EDX]
00605389  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0060538c  8b5038                MOV EDX,dword ptr [EAX + 0x38]
0060538f  ffd2                  CALL EDX
00605391  85c0                  TEST EAX,EAX
00605393  0f84f0010000          JZ 0x00605589
00605399  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
0060539c  8b10                  MOV EDX,dword ptr [EAX]
0060539e  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006053a1  8b4238                MOV EAX,dword ptr [EDX + 0x38]
006053a4  ffd0                  CALL EAX
006053a6  8945b8                MOV dword ptr [EBP + -0x48],EAX
006053a9  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
006053ac  8b510c                MOV EDX,dword ptr [ECX + 0xc]
006053af  899530ffffff          MOV dword ptr [EBP + 0xffffff30],EDX
006053b5  a1d0b4a100            MOV EAX,[0x00a1b4d0]
006053ba  89852cffffff          MOV dword ptr [EBP + 0xffffff2c],EAX
006053c0  8b8d30ffffff          MOV ECX,dword ptr [EBP + 0xffffff30]
006053c6  51                    PUSH ECX
006053c7  8b952cffffff          MOV EDX,dword ptr [EBP + 0xffffff2c]
006053cd  8b4a34                MOV ECX,dword ptr [EDX + 0x34]
006053d0  e82bfe0c00            CALL 0x006d5200
006053d5  898528ffffff          MOV dword ptr [EBP + 0xffffff28],EAX
006053db  8b8528ffffff          MOV EAX,dword ptr [EBP + 0xffffff28]
006053e1  8b4870                MOV ECX,dword ptr [EAX + 0x70]
006053e4  898d24ffffff          MOV dword ptr [EBP + 0xffffff24],ECX
006053ea  83bd24ffffff01        CMP dword ptr [EBP + 0xffffff24],0x1
006053f1  0f8592010000          JNZ 0x00605589
006053f7  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
006053fa  8b82a8020000          MOV EAX,dword ptr [EDX + 0x2a8]
00605400  898520ffffff          MOV dword ptr [EBP + 0xffffff20],EAX
00605406  81bd20ffffff0000007f  CMP dword ptr [EBP + 0xffffff20],0x7f000000
00605410  0f8473010000          JZ 0x00605589
00605416  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
00605419  8b91a8020000          MOV EDX,dword ptr [ECX + 0x2a8]
0060541f  89951cffffff          MOV dword ptr [EBP + 0xffffff1c],EDX
00605425  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0060542a  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0060542d  898d18ffffff          MOV dword ptr [EBP + 0xffffff18],ECX
00605433  8b951cffffff          MOV EDX,dword ptr [EBP + 0xffffff1c]
00605439  52                    PUSH EDX
0060543a  8b8d18ffffff          MOV ECX,dword ptr [EBP + 0xffffff18]
00605440  e8bb6cf1ff            CALL 0x0051c100
00605445  8945b4                MOV dword ptr [EBP + -0x4c],EAX
00605448  837db400              CMP dword ptr [EBP + -0x4c],0x0
0060544c  0f8437010000          JZ 0x00605589
00605452  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00605455  8b08                  MOV ECX,dword ptr [EAX]
00605457  e8d4190d00            CALL 0x006d6e30
0060545c  898514ffffff          MOV dword ptr [EBP + 0xffffff14],EAX
00605462  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
00605468  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
0060546e  889513ffffff          MOV byte ptr [EBP + 0xffffff13],DL
00605474  0fb68513ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff13]
0060547b  83f817                CMP EAX,0x17
0060547e  7574                  JNZ 0x006054f4
00605480  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605483  8b11                  MOV EDX,dword ptr [ECX]
00605485  8b82cc020000          MOV EAX,dword ptr [EDX + 0x2cc]
0060548b  89850cffffff          MOV dword ptr [EBP + 0xffffff0c],EAX
00605491  6a01                  PUSH 0x1
00605493  8b8d0cffffff          MOV ECX,dword ptr [EBP + 0xffffff0c]
00605499  51                    PUSH ECX
0060549a  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
0060549d  e81ebff7ff            CALL 0x005813c0
006054a2  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006054a5  8b02                  MOV EAX,dword ptr [EDX]
006054a7  89857cffffff          MOV dword ptr [EBP + 0xffffff7c],EAX
006054ad  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
006054b3  894d80                MOV dword ptr [EBP + -0x80],ECX
006054b6  837d8000              CMP dword ptr [EBP + -0x80],0x0
006054ba  7416                  JZ 0x006054d2
006054bc  6a01                  PUSH 0x1
006054be  8b5580                MOV EDX,dword ptr [EBP + -0x80]
006054c1  8b02                  MOV EAX,dword ptr [EDX]
006054c3  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
006054c6  8b10                  MOV EDX,dword ptr [EAX]
006054c8  ffd2                  CALL EDX
006054ca  898598feffff          MOV dword ptr [EBP + 0xfffffe98],EAX
006054d0  eb0a                  JMP 0x006054dc
006054d2  c78598feffff00000000  MOV dword ptr [EBP + 0xfffffe98],0x0
006054dc  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006054df  c70000000000          MOV dword ptr [EAX],0x0
006054e5  b801000000            MOV EAX,0x1
006054ea  e975060000            JMP 0x00605b64
006054f4  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006054f7  8b09                  MOV ECX,dword ptr [ECX]
006054f9  e832190d00            CALL 0x006d6e30
006054fe  898508ffffff          MOV dword ptr [EBP + 0xffffff08],EAX
00605504  8b9508ffffff          MOV EDX,dword ptr [EBP + 0xffffff08]
0060550a  8a82ac000000          MOV AL,byte ptr [EDX + 0xac]
00605510  888507ffffff          MOV byte ptr [EBP + 0xffffff07],AL
00605516  0fb68d07ffffff        MOVZX ECX,byte ptr [EBP + 0xffffff07]
0060551d  83f92a                CMP ECX,0x2a
00605520  7567                  JNZ 0x00605589
00605522  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00605525  8b02                  MOV EAX,dword ptr [EDX]
00605527  50                    PUSH EAX
00605528  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
0060552b  e8a059f8ff            CALL 0x0058aed0
00605530  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605533  8b11                  MOV EDX,dword ptr [ECX]
00605535  899574ffffff          MOV dword ptr [EBP + 0xffffff74],EDX
0060553b  8b8574ffffff          MOV EAX,dword ptr [EBP + 0xffffff74]
00605541  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
00605547  83bd78ffffff00        CMP dword ptr [EBP + 0xffffff78],0x0
0060554e  741c                  JZ 0x0060556c
00605550  6a01                  PUSH 0x1
00605552  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
00605558  8b11                  MOV EDX,dword ptr [ECX]
0060555a  8b8d78ffffff          MOV ECX,dword ptr [EBP + 0xffffff78]
00605560  8b02                  MOV EAX,dword ptr [EDX]
00605562  ffd0                  CALL EAX
00605564  898594feffff          MOV dword ptr [EBP + 0xfffffe94],EAX
0060556a  eb0a                  JMP 0x00605576
0060556c  c78594feffff00000000  MOV dword ptr [EBP + 0xfffffe94],0x0
00605576  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605579  c70100000000          MOV dword ptr [ECX],0x0
0060557f  b801000000            MOV EAX,0x1
00605584  e9db050000            JMP 0x00605b64
00605589  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
00605590  c745d800000000        MOV dword ptr [EBP + -0x28],0x0
00605597  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0060559d  8b4204                MOV EAX,dword ptr [EDX + 0x4]
006055a0  898500ffffff          MOV dword ptr [EBP + 0xffffff00],EAX
006055a6  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
006055ac  e89f9d1300            CALL 0x0073f350
006055b1  8945e8                MOV dword ptr [EBP + -0x18],EAX
006055b4  8d4dec                LEA ECX,[EBP + -0x14]
006055b7  e884df1200            CALL 0x00733540
006055bc  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
006055c3  8d4dc0                LEA ECX,[EBP + -0x40]
006055c6  e875df1200            CALL 0x00733540
006055cb  c645fc01              MOV byte ptr [EBP + -0x4],0x1
006055cf  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
006055d5  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006055d8  83e201                AND EDX,0x1
006055db  741a                  JZ 0x006055f7
006055dd  6a00                  PUSH 0x0
006055df  8d45ec                LEA EAX,[EBP + -0x14]
006055e2  50                    PUSH EAX
006055e3  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006055e6  51                    PUSH ECX
006055e7  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006055ea  8b0a                  MOV ECX,dword ptr [EDX]
006055ec  81c1c0020000          ADD ECX,0x2c0
006055f2  e8b9001300            CALL 0x007356b0
006055f7  837d0c01              CMP dword ptr [EBP + 0xc],0x1
006055fb  0f8506030000          JNZ 0x00605907
00605601  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
00605608  eb09                  JMP 0x00605613
0060560a  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0060560d  83c001                ADD EAX,0x1
00605610  8945e4                MOV dword ptr [EBP + -0x1c],EAX
00605613  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
00605619  8b5110                MOV EDX,dword ptr [ECX + 0x10]
0060561c  8995fcfeffff          MOV dword ptr [EBP + 0xfffffefc],EDX
00605622  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
00605625  3b85fcfeffff          CMP EAX,dword ptr [EBP + 0xfffffefc]
0060562b  0f8dd1020000          JGE 0x00605902
00605631  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
00605637  8b510c                MOV EDX,dword ptr [ECX + 0xc]
0060563a  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
0060563d  8b0c82                MOV ECX,dword ptr [EDX + EAX*0x4]
00605640  894dbc                MOV dword ptr [EBP + -0x44],ECX
00605643  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
00605649  8b4208                MOV EAX,dword ptr [EDX + 0x8]
0060564c  8985f8feffff          MOV dword ptr [EBP + 0xfffffef8],EAX
00605652  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
00605655  51                    PUSH ECX
00605656  8b8df8feffff          MOV ECX,dword ptr [EBP + 0xfffffef8]
0060565c  e87f6af1ff            CALL 0x0051c0e0
00605661  8945e0                MOV dword ptr [EBP + -0x20],EAX
00605664  837de000              CMP dword ptr [EBP + -0x20],0x0
00605668  0f848f020000          JZ 0x006058fd
0060566e  8b95acfeffff          MOV EDX,dword ptr [EBP + 0xfffffeac]
00605674  8b4208                MOV EAX,dword ptr [EDX + 0x8]
00605677  83e001                AND EAX,0x1
0060567a  745f                  JZ 0x006056db
0060567c  837dd800              CMP dword ptr [EBP + -0x28],0x0
00605680  7559                  JNZ 0x006056db
00605682  6a00                  PUSH 0x0
00605684  8d4dc0                LEA ECX,[EBP + -0x40]
00605687  51                    PUSH ECX
00605688  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
0060568b  52                    PUSH EDX
0060568c  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0060568f  81c1c0020000          ADD ECX,0x2c0
00605695  e816001300            CALL 0x007356b0
0060569a  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0060569d  8b08                  MOV ECX,dword ptr [EAX]
0060569f  8b510c                MOV EDX,dword ptr [ECX + 0xc]
006056a2  8955ac                MOV dword ptr [EBP + -0x54],EDX
006056a5  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
006056a8  8b480c                MOV ECX,dword ptr [EAX + 0xc]
006056ab  894db0                MOV dword ptr [EBP + -0x50],ECX
006056ae  8b55ac                MOV EDX,dword ptr [EBP + -0x54]
006056b1  3b55b0                CMP EDX,dword ptr [EBP + -0x50]
006056b4  7218                  JC 0x006056ce
006056b6  8b45ac                MOV EAX,dword ptr [EBP + -0x54]
006056b9  3b45b0                CMP EAX,dword ptr [EBP + -0x50]
006056bc  751d                  JNZ 0x006056db
006056be  8d4dc0                LEA ECX,[EBP + -0x40]
006056c1  51                    PUSH ECX
006056c2  8d4dec                LEA ECX,[EBP + -0x14]
006056c5  e806e51200            CALL 0x00733bd0
006056ca  85c0                  TEST EAX,EAX
006056cc  740d                  JZ 0x006056db
006056ce  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006056d1  8955dc                MOV dword ptr [EBP + -0x24],EDX
006056d4  c745d801000000        MOV dword ptr [EBP + -0x28],0x1
006056db  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
006056de  8b480c                MOV ECX,dword ptr [EAX + 0xc]
006056e1  898df4feffff          MOV dword ptr [EBP + 0xfffffef4],ECX
006056e7  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006056ea  8b02                  MOV EAX,dword ptr [EDX]
006056ec  8b480c                MOV ECX,dword ptr [EAX + 0xc]
006056ef  898df0feffff          MOV dword ptr [EBP + 0xfffffef0],ECX
006056f5  8b95f4feffff          MOV EDX,dword ptr [EBP + 0xfffffef4]
006056fb  3b95f0feffff          CMP EDX,dword ptr [EBP + 0xfffffef0]
00605701  0f85f6010000          JNZ 0x006058fd
00605707  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
0060570a  8a88a4020000          MOV CL,byte ptr [EAX + 0x2a4]
00605710  888deffeffff          MOV byte ptr [EBP + 0xfffffeef],CL
00605716  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00605719  8b02                  MOV EAX,dword ptr [EDX]
0060571b  8a88a4020000          MOV CL,byte ptr [EAX + 0x2a4]
00605721  888deefeffff          MOV byte ptr [EBP + 0xfffffeee],CL
00605727  0fb695effeffff        MOVZX EDX,byte ptr [EBP + 0xfffffeef]
0060572e  0fb685eefeffff        MOVZX EAX,byte ptr [EBP + 0xfffffeee]
00605735  3bd0                  CMP EDX,EAX
00605737  0f85c0010000          JNZ 0x006058fd
0060573d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605740  8b11                  MOV EDX,dword ptr [ECX]
00605742  52                    PUSH EDX
00605743  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00605746  e875f6ffff            CALL 0x00604dc0
0060574b  83f801                CMP EAX,0x1
0060574e  0f85a9010000          JNZ 0x006058fd
00605754  c745a800000000        MOV dword ptr [EBP + -0x58],0x0
0060575b  837dd400              CMP dword ptr [EBP + -0x2c],0x0
0060575f  0f84d2000000          JZ 0x00605837
00605765  0fb60574449900        MOVZX EAX,byte ptr [0x00994474]
0060576c  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0060576f  0fb65108              MOVZX EDX,byte ptr [ECX + 0x8]
00605773  3bd0                  CMP EDX,EAX
00605775  7527                  JNZ 0x0060579e
00605777  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0060577c  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0060577f  898de8feffff          MOV dword ptr [EBP + 0xfffffee8],ECX
00605785  8b95acfeffff          MOV EDX,dword ptr [EBP + 0xfffffeac]
0060578b  8b02                  MOV EAX,dword ptr [EDX]
0060578d  50                    PUSH EAX
0060578e  8b8de8feffff          MOV ECX,dword ptr [EBP + 0xfffffee8]
00605794  e8676ef1ff            CALL 0x0051c600
00605799  8945a8                MOV dword ptr [EBP + -0x58],EAX
0060579c  eb5a                  JMP 0x006057f8
0060579e  0fb60d75449900        MOVZX ECX,byte ptr [0x00994475]
006057a5  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
006057a8  0fb64208              MOVZX EAX,byte ptr [EDX + 0x8]
006057ac  3bc1                  CMP EAX,ECX
006057ae  7548                  JNZ 0x006057f8
006057b0  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006057b3  8b11                  MOV EDX,dword ptr [ECX]
006057b5  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
006057b8  8b4238                MOV EAX,dword ptr [EDX + 0x38]
006057bb  ffd0                  CALL EAX
006057bd  8985e4feffff          MOV dword ptr [EBP + 0xfffffee4],EAX
006057c3  8b8de4feffff          MOV ECX,dword ptr [EBP + 0xfffffee4]
006057c9  8b91a8020000          MOV EDX,dword ptr [ECX + 0x2a8]
006057cf  8995e0feffff          MOV dword ptr [EBP + 0xfffffee0],EDX
006057d5  a1a4b4a100            MOV EAX,[0x00a1b4a4]
006057da  8b4808                MOV ECX,dword ptr [EAX + 0x8]
006057dd  898ddcfeffff          MOV dword ptr [EBP + 0xfffffedc],ECX
006057e3  8b95e0feffff          MOV EDX,dword ptr [EBP + 0xfffffee0]
006057e9  52                    PUSH EDX
006057ea  8b8ddcfeffff          MOV ECX,dword ptr [EBP + 0xfffffedc]
006057f0  e80b6ef1ff            CALL 0x0051c600
006057f5  8945a8                MOV dword ptr [EBP + -0x58],EAX
006057f8  837da800              CMP dword ptr [EBP + -0x58],0x0
006057fc  7439                  JZ 0x00605837
006057fe  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00605801  8b08                  MOV ECX,dword ptr [EAX]
00605803  8b5114                MOV EDX,dword ptr [ECX + 0x14]
00605806  8995d8feffff          MOV dword ptr [EBP + 0xfffffed8],EDX
0060580c  a1a4b4a100            MOV EAX,[0x00a1b4a4]
00605811  8b4808                MOV ECX,dword ptr [EAX + 0x8]
00605814  898dd4feffff          MOV dword ptr [EBP + 0xfffffed4],ECX
0060581a  8b95d8feffff          MOV EDX,dword ptr [EBP + 0xfffffed8]
00605820  52                    PUSH EDX
00605821  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
00605824  50                    PUSH EAX
00605825  8b8dd4feffff          MOV ECX,dword ptr [EBP + 0xfffffed4]
0060582b  e8b06af1ff            CALL 0x0051c2e0
00605830  8bc8                  MOV ECX,EAX
00605832  e8295f0400            CALL 0x0064b760
00605837  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060583a  8b11                  MOV EDX,dword ptr [ECX]
0060583c  89956cffffff          MOV dword ptr [EBP + 0xffffff6c],EDX
00605842  8b856cffffff          MOV EAX,dword ptr [EBP + 0xffffff6c]
00605848  898570ffffff          MOV dword ptr [EBP + 0xffffff70],EAX
0060584e  83bd70ffffff00        CMP dword ptr [EBP + 0xffffff70],0x0
00605855  741c                  JZ 0x00605873
00605857  6a01                  PUSH 0x1
00605859  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
0060585f  8b11                  MOV EDX,dword ptr [ECX]
00605861  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
00605867  8b02                  MOV EAX,dword ptr [EDX]
00605869  ffd0                  CALL EAX
0060586b  898590feffff          MOV dword ptr [EBP + 0xfffffe90],EAX
00605871  eb0a                  JMP 0x0060587d
00605873  c78590feffff00000000  MOV dword ptr [EBP + 0xfffffe90],0x0
0060587d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605880  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
00605883  8911                  MOV dword ptr [ECX],EDX
00605885  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
00605888  8b88a8020000          MOV ECX,dword ptr [EAX + 0x2a8]
0060588e  898dd0feffff          MOV dword ptr [EBP + 0xfffffed0],ECX
00605894  680000007f            PUSH 0x7f000000
00605899  8b5510                MOV EDX,dword ptr [EBP + 0x10]
0060589c  52                    PUSH EDX
0060589d  8b4510                MOV EAX,dword ptr [EBP + 0x10]
006058a0  50                    PUSH EAX
006058a1  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
006058a7  51                    PUSH ECX
006058a8  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006058ab  8b0a                  MOV ECX,dword ptr [EDX]
006058ad  e8ceaaffff            CALL 0x00600380
006058b2  837d1400              CMP dword ptr [EBP + 0x14],0x0
006058b6  7415                  JZ 0x006058cd
006058b8  8b85acfeffff          MOV EAX,dword ptr [EBP + 0xfffffeac]
006058be  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006058c1  83c101                ADD ECX,0x1
006058c4  8b95acfeffff          MOV EDX,dword ptr [EBP + 0xfffffeac]
006058ca  894a04                MOV dword ptr [EDX + 0x4],ECX
006058cd  c78568ffffff01000000  MOV dword ptr [EBP + 0xffffff68],0x1
006058d7  c645fc00              MOV byte ptr [EBP + -0x4],0x0
006058db  8d4dc0                LEA ECX,[EBP + -0x40]
006058de  e89dde1200            CALL 0x00733780
006058e3  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
006058ea  8d4dec                LEA ECX,[EBP + -0x14]
006058ed  e88ede1200            CALL 0x00733780
006058f2  8b8568ffffff          MOV EAX,dword ptr [EBP + 0xffffff68]
006058f8  e967020000            JMP 0x00605b64
006058fd  e908fdffff            JMP 0x0060560a
00605902  e9cb000000            JMP 0x006059d2
00605907  8b85acfeffff          MOV EAX,dword ptr [EBP + 0xfffffeac]
0060590d  8b4808                MOV ECX,dword ptr [EAX + 0x8]
00605910  83e101                AND ECX,0x1
00605913  0f84b9000000          JZ 0x006059d2
00605919  c745e400000000        MOV dword ptr [EBP + -0x1c],0x0
00605920  eb09                  JMP 0x0060592b
00605922  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
00605925  83c201                ADD EDX,0x1
00605928  8955e4                MOV dword ptr [EBP + -0x1c],EDX
0060592b  8b85acfeffff          MOV EAX,dword ptr [EBP + 0xfffffeac]
00605931  8b4810                MOV ECX,dword ptr [EAX + 0x10]
00605934  898dccfeffff          MOV dword ptr [EBP + 0xfffffecc],ECX
0060593a  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
0060593d  3b95ccfeffff          CMP EDX,dword ptr [EBP + 0xfffffecc]
00605943  0f8d89000000          JGE 0x006059d2
00605949  8b85acfeffff          MOV EAX,dword ptr [EBP + 0xfffffeac]
0060594f  8b480c                MOV ECX,dword ptr [EAX + 0xc]
00605952  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
00605955  8b0491                MOV EAX,dword ptr [ECX + EDX*0x4]
00605958  8945bc                MOV dword ptr [EBP + -0x44],EAX
0060595b  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
00605961  8b5108                MOV EDX,dword ptr [ECX + 0x8]
00605964  8995c8feffff          MOV dword ptr [EBP + 0xfffffec8],EDX
0060596a  8b45bc                MOV EAX,dword ptr [EBP + -0x44]
0060596d  50                    PUSH EAX
0060596e  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
00605974  e86767f1ff            CALL 0x0051c0e0
00605979  8945e0                MOV dword ptr [EBP + -0x20],EAX
0060597c  837de000              CMP dword ptr [EBP + -0x20],0x0
00605980  744b                  JZ 0x006059cd
00605982  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
00605988  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0060598b  83e201                AND EDX,0x1
0060598e  743d                  JZ 0x006059cd
00605990  837dd800              CMP dword ptr [EBP + -0x28],0x0
00605994  7537                  JNZ 0x006059cd
00605996  6a00                  PUSH 0x0
00605998  8d45c0                LEA EAX,[EBP + -0x40]
0060599b  50                    PUSH EAX
0060599c  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0060599f  51                    PUSH ECX
006059a0  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
006059a3  81c1c0020000          ADD ECX,0x2c0
006059a9  e802fd1200            CALL 0x007356b0
006059ae  8d55c0                LEA EDX,[EBP + -0x40]
006059b1  52                    PUSH EDX
006059b2  8d4dec                LEA ECX,[EBP + -0x14]
006059b5  e816e21200            CALL 0x00733bd0
006059ba  85c0                  TEST EAX,EAX
006059bc  740f                  JZ 0x006059cd
006059be  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006059c1  8945dc                MOV dword ptr [EBP + -0x24],EAX
006059c4  c745d801000000        MOV dword ptr [EBP + -0x28],0x1
006059cb  eb05                  JMP 0x006059d2
006059cd  e950ffffff            JMP 0x00605922
006059d2  6a00                  PUSH 0x0
006059d4  8d4dc8                LEA ECX,[EBP + -0x38]
006059d7  e8644c2400            CALL 0x0084a640
006059dc  c645fc02              MOV byte ptr [EBP + -0x4],0x2
006059e0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006059e3  8b11                  MOV EDX,dword ptr [ECX]
006059e5  52                    PUSH EDX
006059e6  8d4dc8                LEA ECX,[EBP + -0x38]
006059e9  e872902300            CALL 0x0083ea60
006059ee  8b85acfeffff          MOV EAX,dword ptr [EBP + 0xfffffeac]
006059f4  8b4808                MOV ECX,dword ptr [EAX + 0x8]
006059f7  83e101                AND ECX,0x1
006059fa  0f84a5000000          JZ 0x00605aa5
00605a00  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
00605a06  8b4208                MOV EAX,dword ptr [EDX + 0x8]
00605a09  8985c4feffff          MOV dword ptr [EBP + 0xfffffec4],EAX
00605a0f  6a01                  PUSH 0x1
00605a11  6a00                  PUSH 0x0
00605a13  8b8dc4feffff          MOV ECX,dword ptr [EBP + 0xfffffec4]
00605a19  e8026cf1ff            CALL 0x0051c620
00605a1e  8945a4                MOV dword ptr [EBP + -0x5c],EAX
00605a21  837da400              CMP dword ptr [EBP + -0x5c],0x0
00605a25  742a                  JZ 0x00605a51
00605a27  8b4da4                MOV ECX,dword ptr [EBP + -0x5c]
00605a2a  8b5138                MOV EDX,dword ptr [ECX + 0x38]
00605a2d  8995c0feffff          MOV dword ptr [EBP + 0xfffffec0],EDX
00605a33  680000007f            PUSH 0x7f000000
00605a38  8b4510                MOV EAX,dword ptr [EBP + 0x10]
00605a3b  50                    PUSH EAX
00605a3c  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
00605a3f  51                    PUSH ECX
00605a40  8b95c0feffff          MOV EDX,dword ptr [EBP + 0xfffffec0]
00605a46  52                    PUSH EDX
00605a47  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00605a4a  8b08                  MOV ECX,dword ptr [EAX]
00605a4c  e82fa9ffff            CALL 0x00600380
00605a51  837dd800              CMP dword ptr [EBP + -0x28],0x0
00605a55  7429                  JZ 0x00605a80
00605a57  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605a5a  8b11                  MOV EDX,dword ptr [ECX]
00605a5c  8b4214                MOV EAX,dword ptr [EDX + 0x14]
00605a5f  8985bcfeffff          MOV dword ptr [EBP + 0xfffffebc],EAX
00605a65  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
00605a68  51                    PUSH ECX
00605a69  8b95bcfeffff          MOV EDX,dword ptr [EBP + 0xfffffebc]
00605a6f  52                    PUSH EDX
00605a70  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
00605a76  83c10c                ADD ECX,0xc
00605a79  e802e8e0ff            CALL 0x00414280
00605a7e  eb23                  JMP 0x00605aa3
00605a80  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00605a83  8b08                  MOV ECX,dword ptr [EAX]
00605a85  8b5114                MOV EDX,dword ptr [ECX + 0x14]
00605a88  8995b8feffff          MOV dword ptr [EBP + 0xfffffeb8],EDX
00605a8e  8b85b8feffff          MOV EAX,dword ptr [EBP + 0xfffffeb8]
00605a94  50                    PUSH EAX
00605a95  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
00605a9b  83c10c                ADD ECX,0xc
00605a9e  e8bd8f2300            CALL 0x0083ea60
00605aa3  eb25                  JMP 0x00605aca
00605aa5  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605aa8  8b11                  MOV EDX,dword ptr [ECX]
00605aaa  8b4214                MOV EAX,dword ptr [EDX + 0x14]
00605aad  8985b4feffff          MOV dword ptr [EBP + 0xfffffeb4],EAX
00605ab3  6a00                  PUSH 0x0
00605ab5  8b8db4feffff          MOV ECX,dword ptr [EBP + 0xfffffeb4]
00605abb  51                    PUSH ECX
00605abc  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
00605ac2  83c10c                ADD ECX,0xc
00605ac5  e8b6e7e0ff            CALL 0x00414280
00605aca  837d1400              CMP dword ptr [EBP + 0x14],0x0
00605ace  7435                  JZ 0x00605b05
00605ad0  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00605ad3  8b02                  MOV EAX,dword ptr [EDX]
00605ad5  8985b0feffff          MOV dword ptr [EBP + 0xfffffeb0],EAX
00605adb  b901000000            MOV ECX,0x1
00605ae0  83e101                AND ECX,0x1
00605ae3  c1e107                SHL ECX,0x7
00605ae6  8b95b0feffff          MOV EDX,dword ptr [EBP + 0xfffffeb0]
00605aec  8b82c8020000          MOV EAX,dword ptr [EDX + 0x2c8]
00605af2  257fffffff            AND EAX,0xffffff7f
00605af7  0bc1                  OR EAX,ECX
00605af9  8b8db0feffff          MOV ECX,dword ptr [EBP + 0xfffffeb0]
00605aff  8981c8020000          MOV dword ptr [ECX + 0x2c8],EAX
00605b05  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00605b08  8b02                  MOV EAX,dword ptr [EDX]
00605b0a  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
00605b10  c1e907                SHR ECX,0x7
00605b13  83e101                AND ECX,0x1
00605b16  7415                  JZ 0x00605b2d
00605b18  8b95acfeffff          MOV EDX,dword ptr [EBP + 0xfffffeac]
00605b1e  8b4204                MOV EAX,dword ptr [EDX + 0x4]
00605b21  83c001                ADD EAX,0x1
00605b24  8b8dacfeffff          MOV ECX,dword ptr [EBP + 0xfffffeac]
00605b2a  894104                MOV dword ptr [ECX + 0x4],EAX
00605b2d  c78564ffffff01000000  MOV dword ptr [EBP + 0xffffff64],0x1
00605b37  c645fc01              MOV byte ptr [EBP + -0x4],0x1
00605b3b  8d4dc8                LEA ECX,[EBP + -0x38]
00605b3e  e86d820400            CALL 0x0064ddb0
00605b43  c645fc00              MOV byte ptr [EBP + -0x4],0x0
00605b47  8d4dc0                LEA ECX,[EBP + -0x40]
00605b4a  e831dc1200            CALL 0x00733780
00605b4f  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00605b56  8d4dec                LEA ECX,[EBP + -0x14]
00605b59  e822dc1200            CALL 0x00733780
00605b5e  8b8564ffffff          MOV EAX,dword ptr [EBP + 0xffffff64]
00605b64  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00605b67  64890d00000000        MOV dword ptr FS:[0x0],ECX
00605b6e  59                    POP ECX
00605b6f  8be5                  MOV ESP,EBP
00605b71  5d                    POP EBP
00605b72  c21000                RET 0x10
