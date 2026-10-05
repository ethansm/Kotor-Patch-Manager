; FN 006d9500 FUN_006d9500
006d9500  55                    PUSH EBP
006d9501  8bec                  MOV EBP,ESP
006d9503  6aff                  PUSH -0x1
006d9505  686bd79500            PUSH 0x95d76b
006d950a  64a100000000          MOV EAX,FS:[0x0]
006d9510  50                    PUSH EAX
006d9511  81ecbc010000          SUB ESP,0x1bc
006d9517  a1201fa100            MOV EAX,[0x00a11f20]
006d951c  33c5                  XOR EAX,EBP
006d951e  50                    PUSH EAX
006d951f  8d45f4                LEA EAX,[EBP + -0xc]
006d9522  64a300000000          MOV FS:[0x0],EAX
006d9528  898d54feffff          MOV dword ptr [EBP + 0xfffffe54],ECX
006d952e  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
006d9535  c745ec0000007f        MOV dword ptr [EBP + -0x14],0x7f000000
006d953c  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d9542  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d9548  898d28ffffff          MOV dword ptr [EBP + 0xffffff28],ECX
006d954e  8b9528ffffff          MOV EDX,dword ptr [EBP + 0xffffff28]
006d9554  8b82a80a0000          MOV EAX,dword ptr [EDX + 0xaa8]
006d955a  898524ffffff          MOV dword ptr [EBP + 0xffffff24],EAX
006d9560  83bd24ffffff00        CMP dword ptr [EBP + 0xffffff24],0x0
006d9567  0f852d0f0000          JNZ 0x006da49a
006d956d  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006d9573  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006d9576  899520ffffff          MOV dword ptr [EBP + 0xffffff20],EDX
006d957c  6a02                  PUSH 0x2
006d957e  8b8d20ffffff          MOV ECX,dword ptr [EBP + 0xffffff20]
006d9584  e89731e4ff            CALL 0x0051c720
006d9589  85c0                  TEST EAX,EAX
006d958b  0f85090f0000          JNZ 0x006da49a
006d9591  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d9597  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d959d  898d1cffffff          MOV dword ptr [EBP + 0xffffff1c],ECX
006d95a3  8b951cffffff          MOV EDX,dword ptr [EBP + 0xffffff1c]
006d95a9  8b82180b0000          MOV EAX,dword ptr [EDX + 0xb18]
006d95af  898518ffffff          MOV dword ptr [EBP + 0xffffff18],EAX
006d95b5  83bd18ffffff00        CMP dword ptr [EBP + 0xffffff18],0x0
006d95bc  0f84c1000000          JZ 0x006d9683
006d95c2  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d95c8  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d95ce  899514ffffff          MOV dword ptr [EBP + 0xffffff14],EDX
006d95d4  8b8514ffffff          MOV EAX,dword ptr [EBP + 0xffffff14]
006d95da  8b88180b0000          MOV ECX,dword ptr [EAX + 0xb18]
006d95e0  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
006d95e6  8b956cffffff          MOV EDX,dword ptr [EBP + 0xffffff6c]
006d95ec  899570ffffff          MOV dword ptr [EBP + 0xffffff70],EDX
006d95f2  83bd70ffffff00        CMP dword ptr [EBP + 0xffffff70],0x0
006d95f9  7432                  JZ 0x006d962d
006d95fb  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
006d9601  e81a9df4ff            CALL 0x00623320
006d9606  b801000000            MOV EAX,0x1
006d960b  83e001                AND EAX,0x1
006d960e  740f                  JZ 0x006d961f
006d9610  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
006d9616  51                    PUSH ECX
006d9617  e8e1002400            CALL 0x009196fd
006d961c  83c404                ADD ESP,0x4
006d961f  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
006d9625  899550feffff          MOV dword ptr [EBP + 0xfffffe50],EDX
006d962b  eb0a                  JMP 0x006d9637
006d962d  c78550feffff00000000  MOV dword ptr [EBP + 0xfffffe50],0x0
006d9637  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d963d  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d9643  898d10ffffff          MOV dword ptr [EBP + 0xffffff10],ECX
006d9649  8b9510ffffff          MOV EDX,dword ptr [EBP + 0xffffff10]
006d964f  c782180b000000000000  MOV dword ptr [EDX + 0xb18],0x0
006d9659  8b8510ffffff          MOV EAX,dword ptr [EBP + 0xffffff10]
006d965f  83b8180b000000        CMP dword ptr [EAX + 0xb18],0x0
006d9666  741b                  JZ 0x006d9683
006d9668  8b8d10ffffff          MOV ECX,dword ptr [EBP + 0xffffff10]
006d966e  8b91180b0000          MOV EDX,dword ptr [ECX + 0xb18]
006d9674  8b8510ffffff          MOV EAX,dword ptr [EBP + 0xffffff10]
006d967a  8a4a10                MOV CL,byte ptr [EDX + 0x10]
006d967d  88881c0b0000          MOV byte ptr [EAX + 0xb1c],CL
006d9683  8b9554feffff          MOV EDX,dword ptr [EBP + 0xfffffe54]
006d9689  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d968f  89850cffffff          MOV dword ptr [EBP + 0xffffff0c],EAX
006d9695  8b8d0cffffff          MOV ECX,dword ptr [EBP + 0xffffff0c]
006d969b  e8706aebff            CALL 0x00590110
006d96a0  85c0                  TEST EAX,EAX
006d96a2  0f847d0d0000          JZ 0x006da425
006d96a8  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d96ae  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d96b4  899508ffffff          MOV dword ptr [EBP + 0xffffff08],EDX
006d96ba  8b8508ffffff          MOV EAX,dword ptr [EBP + 0xffffff08]
006d96c0  8b88b80a0000          MOV ECX,dword ptr [EAX + 0xab8]
006d96c6  898d04ffffff          MOV dword ptr [EBP + 0xffffff04],ECX
006d96cc  83bd04ffffff01        CMP dword ptr [EBP + 0xffffff04],0x1
006d96d3  0f8d4c0d0000          JGE 0x006da425
006d96d9  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
006d96e0  6a04                  PUSH 0x4
006d96e2  8d55f0                LEA EDX,[EBP + -0x10]
006d96e5  52                    PUSH EDX
006d96e6  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006d96e9  83c038                ADD EAX,0x38
006d96ec  50                    PUSH EAX
006d96ed  e8ae442400            CALL 0x0091dba0
006d96f2  83c40c                ADD ESP,0xc
006d96f5  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d96fb  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d9701  899500ffffff          MOV dword ptr [EBP + 0xffffff00],EDX
006d9707  8b8d00ffffff          MOV ECX,dword ptr [EBP + 0xffffff00]
006d970d  e80e6bebff            CALL 0x00590220
006d9712  8945e4                MOV dword ptr [EBP + -0x1c],EAX
006d9715  837de400              CMP dword ptr [EBP + -0x1c],0x0
006d9719  0f84040d0000          JZ 0x006da423
006d971f  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9722  668b4804              MOV CX,word ptr [EAX + 0x4]
006d9726  66894de0              MOV word ptr [EBP + -0x20],CX
006d972a  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d972d  8b4214                MOV EAX,dword ptr [EDX + 0x14]
006d9730  8945ec                MOV dword ptr [EBP + -0x14],EAX
006d9733  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9736  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006d9739  8955cc                MOV dword ptr [EBP + -0x34],EDX
006d973c  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d973f  8b480c                MOV ECX,dword ptr [EAX + 0xc]
006d9742  894dd8                MOV dword ptr [EBP + -0x28],ECX
006d9745  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9748  8a4210                MOV AL,byte ptr [EDX + 0x10]
006d974b  8845d3                MOV byte ptr [EBP + -0x2d],AL
006d974e  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9751  668b515c              MOV DX,word ptr [ECX + 0x5c]
006d9755  668955d4              MOV word ptr [EBP + -0x2c],DX
006d9759  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d975c  8b4874                MOV ECX,dword ptr [EAX + 0x74]
006d975f  894ddc                MOV dword ptr [EBP + -0x24],ECX
006d9762  8b9554feffff          MOV EDX,dword ptr [EBP + 0xfffffe54]
006d9768  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d976e  8985fcfeffff          MOV dword ptr [EBP + 0xfffffefc],EAX
006d9774  8b8dfcfeffff          MOV ECX,dword ptr [EBP + 0xfffffefc]
006d977a  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d977d  8991180b0000          MOV dword ptr [ECX + 0xb18],EDX
006d9783  8b85fcfeffff          MOV EAX,dword ptr [EBP + 0xfffffefc]
006d9789  83b8180b000000        CMP dword ptr [EAX + 0xb18],0x0
006d9790  741b                  JZ 0x006d97ad
006d9792  8b8dfcfeffff          MOV ECX,dword ptr [EBP + 0xfffffefc]
006d9798  8b91180b0000          MOV EDX,dword ptr [ECX + 0xb18]
006d979e  8b85fcfeffff          MOV EAX,dword ptr [EBP + 0xfffffefc]
006d97a4  8a4a10                MOV CL,byte ptr [EDX + 0x10]
006d97a7  88881c0b0000          MOV byte ptr [EAX + 0xb1c],CL
006d97ad  8b9554feffff          MOV EDX,dword ptr [EBP + 0xfffffe54]
006d97b3  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d97b9  8985f8feffff          MOV dword ptr [EBP + 0xfffffef8],EAX
006d97bf  0fb64dd3              MOVZX ECX,byte ptr [EBP + -0x2d]
006d97c3  51                    PUSH ECX
006d97c4  8b8df8feffff          MOV ECX,dword ptr [EBP + 0xfffffef8]
006d97ca  e8d170ebff            CALL 0x005908a0
006d97cf  0fb655d3              MOVZX EDX,byte ptr [EBP + -0x2d]
006d97d3  83fa0a                CMP EDX,0xa
006d97d6  746a                  JZ 0x006d9842
006d97d8  0fb645d3              MOVZX EAX,byte ptr [EBP + -0x2d]
006d97dc  83f80d                CMP EAX,0xd
006d97df  7461                  JZ 0x006d9842
006d97e1  837de400              CMP dword ptr [EBP + -0x1c],0x0
006d97e5  740c                  JZ 0x006d97f3
006d97e7  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d97ea  817924b5000000        CMP dword ptr [ECX + 0x24],0xb5
006d97f1  743c                  JZ 0x006d982f
006d97f3  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d97f6  817a24c8000000        CMP dword ptr [EDX + 0x24],0xc8
006d97fd  7430                  JZ 0x006d982f
006d97ff  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9802  817824b6000000        CMP dword ptr [EAX + 0x24],0xb6
006d9809  7424                  JZ 0x006d982f
006d980b  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d980e  817924b8000000        CMP dword ptr [ECX + 0x24],0xb8
006d9815  7418                  JZ 0x006d982f
006d9817  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d981a  817a24c9000000        CMP dword ptr [EDX + 0x24],0xc9
006d9821  740c                  JZ 0x006d982f
006d9823  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9826  8178240d010000        CMP dword ptr [EAX + 0x24],0x10d
006d982d  7502                  JNZ 0x006d9831
006d982f  eb11                  JMP 0x006d9842
006d9831  6a00                  PUSH 0x0
006d9833  6a00                  PUSH 0x0
006d9835  6a01                  PUSH 0x1
006d9837  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d983d  e85ea5e8ff            CALL 0x00563da0
006d9842  0fb64dd3              MOVZX ECX,byte ptr [EBP + -0x2d]
006d9846  898d4cfeffff          MOV dword ptr [EBP + 0xfffffe4c],ECX
006d984c  8b954cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe4c]
006d9852  83ea01                SUB EDX,0x1
006d9855  89954cfeffff          MOV dword ptr [EBP + 0xfffffe4c],EDX
006d985b  83bd4cfeffff0d        CMP dword ptr [EBP + 0xfffffe4c],0xd
006d9862  0f87bb0b0000          JA 0x006da423
006d9868  8b854cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe4c]
006d986e  ff2485b0a46d00        JMP dword ptr [EAX*0x4 + 0x6da4b0]
006d9875  6a01                  PUSH 0x1
006d9877  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d987d  8b11                  MOV EDX,dword ptr [ECX]
006d987f  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9885  8b4210                MOV EAX,dword ptr [EDX + 0x10]
006d9888  ffd0                  CALL EAX
006d988a  8bc8                  MOV ECX,EAX
006d988c  e81fe3e6ff            CALL 0x00547bb0
006d9891  a1c0449900            MOV EAX,[0x009944c0]
006d9896  e9040c0000            JMP 0x006da49f
006d98a0  c745c800000000        MOV dword ptr [EBP + -0x38],0x0
006d98a7  c745c400000000        MOV dword ptr [EBP + -0x3c],0x0
006d98ae  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006d98b4  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006d98b7  8995f4feffff          MOV dword ptr [EBP + 0xfffffef4],EDX
006d98bd  8d45c4                LEA EAX,[EBP + -0x3c]
006d98c0  50                    PUSH EAX
006d98c1  8d4dc8                LEA ECX,[EBP + -0x38]
006d98c4  51                    PUSH ECX
006d98c5  8b8df4feffff          MOV ECX,dword ptr [EBP + 0xfffffef4]
006d98cb  e8a02ae4ff            CALL 0x0051c370
006d98d0  8bc8                  MOV ECX,EAX
006d98d2  e84914e4ff            CALL 0x0051ad20
006d98d7  837ddc00              CMP dword ptr [EBP + -0x24],0x0
006d98db  745b                  JZ 0x006d9938
006d98dd  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d98e0  8b4278                MOV EAX,dword ptr [EDX + 0x78]
006d98e3  8945bc                MOV dword ptr [EBP + -0x44],EAX
006d98e6  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d98e9  8b517c                MOV EDX,dword ptr [ECX + 0x7c]
006d98ec  8955c0                MOV dword ptr [EBP + -0x40],EDX
006d98ef  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d98f2  8b8880000000          MOV ECX,dword ptr [EAX + 0x80]
006d98f8  894db8                MOV dword ptr [EBP + -0x48],ECX
006d98fb  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
006d98fe  52                    PUSH EDX
006d98ff  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
006d9902  50                    PUSH EAX
006d9903  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
006d9906  51                    PUSH ECX
006d9907  6a01                  PUSH 0x1
006d9909  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
006d990c  52                    PUSH EDX
006d990d  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
006d9910  50                    PUSH EAX
006d9911  0fb74de0              MOVZX ECX,word ptr [EBP + -0x20]
006d9915  51                    PUSH ECX
006d9916  0fb655d3              MOVZX EDX,byte ptr [EBP + -0x2d]
006d991a  52                    PUSH EDX
006d991b  6a01                  PUSH 0x1
006d991d  6a01                  PUSH 0x1
006d991f  6a00                  PUSH 0x0
006d9921  6a00                  PUSH 0x0
006d9923  6a00                  PUSH 0x0
006d9925  6a00                  PUSH 0x0
006d9927  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d992a  50                    PUSH EAX
006d992b  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9931  e80aece8ff            CALL 0x00568540
006d9936  eb38                  JMP 0x006d9970
006d9938  6a00                  PUSH 0x0
006d993a  6a04                  PUSH 0x4
006d993c  6a00                  PUSH 0x0
006d993e  6a00                  PUSH 0x0
006d9940  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
006d9943  51                    PUSH ECX
006d9944  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
006d9947  52                    PUSH EDX
006d9948  0fb745e0              MOVZX EAX,word ptr [EBP + -0x20]
006d994c  50                    PUSH EAX
006d994d  0fb64dd3              MOVZX ECX,byte ptr [EBP + -0x2d]
006d9951  51                    PUSH ECX
006d9952  6a01                  PUSH 0x1
006d9954  6a01                  PUSH 0x1
006d9956  6a00                  PUSH 0x0
006d9958  6a00                  PUSH 0x0
006d995a  6a00                  PUSH 0x0
006d995c  0fb755d4              MOVZX EDX,word ptr [EBP + -0x2c]
006d9960  52                    PUSH EDX
006d9961  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d9964  50                    PUSH EAX
006d9965  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d996b  e8d0ebe8ff            CALL 0x00568540
006d9970  a1c0449900            MOV EAX,[0x009944c0]
006d9975  e9250b0000            JMP 0x006da49f
006d997f  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9985  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d9988  8995f0feffff          MOV dword ptr [EBP + 0xfffffef0],EDX
006d998e  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d9994  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d999a  898decfeffff          MOV dword ptr [EBP + 0xfffffeec],ECX
006d99a0  8b95f0feffff          MOV EDX,dword ptr [EBP + 0xfffffef0]
006d99a6  52                    PUSH EDX
006d99a7  6a01                  PUSH 0x1
006d99a9  8b8decfeffff          MOV ECX,dword ptr [EBP + 0xfffffeec]
006d99af  e8ac62ebff            CALL 0x0058fc60
006d99b4  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d99ba  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d99c0  898de8feffff          MOV dword ptr [EBP + 0xfffffee8],ECX
006d99c6  6a00                  PUSH 0x0
006d99c8  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
006d99cb  52                    PUSH EDX
006d99cc  8b8de8feffff          MOV ECX,dword ptr [EBP + 0xfffffee8]
006d99d2  e8f962ebff            CALL 0x0058fcd0
006d99d7  e9470a0000            JMP 0x006da423
006d99dc  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d99e2  8b4804                MOV ECX,dword ptr [EAX + 0x4]
006d99e5  898de4feffff          MOV dword ptr [EBP + 0xfffffee4],ECX
006d99eb  8b9554feffff          MOV EDX,dword ptr [EBP + 0xfffffe54]
006d99f1  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d99f7  8985e0feffff          MOV dword ptr [EBP + 0xfffffee0],EAX
006d99fd  8b8de4feffff          MOV ECX,dword ptr [EBP + 0xfffffee4]
006d9a03  51                    PUSH ECX
006d9a04  6a01                  PUSH 0x1
006d9a06  8b8de0feffff          MOV ECX,dword ptr [EBP + 0xfffffee0]
006d9a0c  e84f62ebff            CALL 0x0058fc60
006d9a11  8b9554feffff          MOV EDX,dword ptr [EBP + 0xfffffe54]
006d9a17  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d9a1d  8985dcfeffff          MOV dword ptr [EBP + 0xfffffedc],EAX
006d9a23  6a00                  PUSH 0x0
006d9a25  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
006d9a28  51                    PUSH ECX
006d9a29  8b8ddcfeffff          MOV ECX,dword ptr [EBP + 0xfffffedc]
006d9a2f  e89c62ebff            CALL 0x0058fcd0
006d9a34  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9a37  8b4254                MOV EAX,dword ptr [EDX + 0x54]
006d9a3a  50                    PUSH EAX
006d9a3b  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9a3e  8b511c                MOV EDX,dword ptr [ECX + 0x1c]
006d9a41  52                    PUSH EDX
006d9a42  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
006d9a45  50                    PUSH EAX
006d9a46  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9a4c  e8ffcbe8ff            CALL 0x00566650
006d9a51  e9cd090000            JMP 0x006da423
006d9a56  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9a5c  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d9a5f  8995d8feffff          MOV dword ptr [EBP + 0xfffffed8],EDX
006d9a65  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d9a6b  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d9a71  898dd4feffff          MOV dword ptr [EBP + 0xfffffed4],ECX
006d9a77  8b95d8feffff          MOV EDX,dword ptr [EBP + 0xfffffed8]
006d9a7d  52                    PUSH EDX
006d9a7e  6a01                  PUSH 0x1
006d9a80  8b8dd4feffff          MOV ECX,dword ptr [EBP + 0xfffffed4]
006d9a86  e8d561ebff            CALL 0x0058fc60
006d9a8b  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d9a91  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d9a97  898dd0feffff          MOV dword ptr [EBP + 0xfffffed0],ECX
006d9a9d  6a00                  PUSH 0x0
006d9a9f  8b55cc                MOV EDX,dword ptr [EBP + -0x34]
006d9aa2  52                    PUSH EDX
006d9aa3  8b8dd0feffff          MOV ECX,dword ptr [EBP + 0xfffffed0]
006d9aa9  e82262ebff            CALL 0x0058fcd0
006d9aae  6a00                  PUSH 0x0
006d9ab0  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9ab3  8b4854                MOV ECX,dword ptr [EAX + 0x54]
006d9ab6  51                    PUSH ECX
006d9ab7  6a00                  PUSH 0x0
006d9ab9  6a01                  PUSH 0x1
006d9abb  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9abe  8b4220                MOV EAX,dword ptr [EDX + 0x20]
006d9ac1  50                    PUSH EAX
006d9ac2  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
006d9ac5  51                    PUSH ECX
006d9ac6  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9acc  e8ffdbe8ff            CALL 0x005676d0
006d9ad1  e94d090000            JMP 0x006da423
006d9adb  6a01                  PUSH 0x1
006d9add  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9ae0  0fb64260              MOVZX EAX,byte ptr [EDX + 0x60]
006d9ae4  50                    PUSH EAX
006d9ae5  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9ae8  8b515c                MOV EDX,dword ptr [ECX + 0x5c]
006d9aeb  52                    PUSH EDX
006d9aec  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9aef  8b4854                MOV ECX,dword ptr [EAX + 0x54]
006d9af2  51                    PUSH ECX
006d9af3  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9af6  0fb64250              MOVZX EAX,byte ptr [EDX + 0x50]
006d9afa  50                    PUSH EAX
006d9afb  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9afe  8b514c                MOV EDX,dword ptr [ECX + 0x4c]
006d9b01  52                    PUSH EDX
006d9b02  6a01                  PUSH 0x1
006d9b04  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9b07  8b4848                MOV ECX,dword ptr [EAX + 0x48]
006d9b0a  51                    PUSH ECX
006d9b0b  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9b0e  8b8284000000          MOV EAX,dword ptr [EDX + 0x84]
006d9b14  50                    PUSH EAX
006d9b15  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9b18  8b5144                MOV EDX,dword ptr [ECX + 0x44]
006d9b1b  52                    PUSH EDX
006d9b1c  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9b1f  83c038                ADD EAX,0x38
006d9b22  83ec0c                SUB ESP,0xc
006d9b25  8bcc                  MOV ECX,ESP
006d9b27  8b10                  MOV EDX,dword ptr [EAX]
006d9b29  8911                  MOV dword ptr [ECX],EDX
006d9b2b  8b5004                MOV EDX,dword ptr [EAX + 0x4]
006d9b2e  895104                MOV dword ptr [ECX + 0x4],EDX
006d9b31  8b4008                MOV EAX,dword ptr [EAX + 0x8]
006d9b34  894108                MOV dword ptr [ECX + 0x8],EAX
006d9b37  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9b3a  8b5134                MOV EDX,dword ptr [ECX + 0x34]
006d9b3d  52                    PUSH EDX
006d9b3e  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9b41  8b4830                MOV ECX,dword ptr [EAX + 0x30]
006d9b44  51                    PUSH ECX
006d9b45  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9b48  8b422c                MOV EAX,dword ptr [EDX + 0x2c]
006d9b4b  50                    PUSH EAX
006d9b4c  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9b4f  8b5128                MOV EDX,dword ptr [ECX + 0x28]
006d9b52  52                    PUSH EDX
006d9b53  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9b56  8b4824                MOV ECX,dword ptr [EAX + 0x24]
006d9b59  51                    PUSH ECX
006d9b5a  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9b60  e8abb7e8ff            CALL 0x00565310
006d9b65  8b9554feffff          MOV EDX,dword ptr [EBP + 0xfffffe54]
006d9b6b  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d9b71  8985ccfeffff          MOV dword ptr [EBP + 0xfffffecc],EAX
006d9b77  8b8dccfeffff          MOV ECX,dword ptr [EBP + 0xfffffecc]
006d9b7d  8b91180b0000          MOV EDX,dword ptr [ECX + 0xb18]
006d9b83  899564ffffff          MOV dword ptr [EBP + 0xffffff64],EDX
006d9b89  8b8564ffffff          MOV EAX,dword ptr [EBP + 0xffffff64]
006d9b8f  898568ffffff          MOV dword ptr [EBP + 0xffffff68],EAX
006d9b95  83bd68ffffff00        CMP dword ptr [EBP + 0xffffff68],0x0
006d9b9c  7432                  JZ 0x006d9bd0
006d9b9e  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
006d9ba4  e87797f4ff            CALL 0x00623320
006d9ba9  b901000000            MOV ECX,0x1
006d9bae  83e101                AND ECX,0x1
006d9bb1  740f                  JZ 0x006d9bc2
006d9bb3  8b9568ffffff          MOV EDX,dword ptr [EBP + 0xffffff68]
006d9bb9  52                    PUSH EDX
006d9bba  e83efb2300            CALL 0x009196fd
006d9bbf  83c404                ADD ESP,0x4
006d9bc2  8b8568ffffff          MOV EAX,dword ptr [EBP + 0xffffff68]
006d9bc8  898548feffff          MOV dword ptr [EBP + 0xfffffe48],EAX
006d9bce  eb0a                  JMP 0x006d9bda
006d9bd0  c78548feffff00000000  MOV dword ptr [EBP + 0xfffffe48],0x0
006d9bda  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9be0  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006d9be6  8995c8feffff          MOV dword ptr [EBP + 0xfffffec8],EDX
006d9bec  8b85c8feffff          MOV EAX,dword ptr [EBP + 0xfffffec8]
006d9bf2  c780180b000000000000  MOV dword ptr [EAX + 0xb18],0x0
006d9bfc  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
006d9c02  83b9180b000000        CMP dword ptr [ECX + 0xb18],0x0
006d9c09  741b                  JZ 0x006d9c26
006d9c0b  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
006d9c11  8b82180b0000          MOV EAX,dword ptr [EDX + 0xb18]
006d9c17  8b8dc8feffff          MOV ECX,dword ptr [EBP + 0xfffffec8]
006d9c1d  8a5010                MOV DL,byte ptr [EAX + 0x10]
006d9c20  88911c0b0000          MOV byte ptr [ECX + 0xb1c],DL
006d9c26  a1c0449900            MOV EAX,[0x009944c0]
006d9c2b  e96f080000            JMP 0x006da49f
006d9c35  a1a4b4a100            MOV EAX,[0x00a1b4a4]
006d9c3a  8b4808                MOV ECX,dword ptr [EAX + 0x8]
006d9c3d  898dc4feffff          MOV dword ptr [EBP + 0xfffffec4],ECX
006d9c43  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9c46  8b4244                MOV EAX,dword ptr [EDX + 0x44]
006d9c49  50                    PUSH EAX
006d9c4a  8b8dc4feffff          MOV ECX,dword ptr [EBP + 0xfffffec4]
006d9c50  e84b24e4ff            CALL 0x0051c0a0
006d9c55  8945b4                MOV dword ptr [EBP + -0x4c],EAX
006d9c58  837db400              CMP dword ptr [EBP + -0x4c],0x0
006d9c5c  7454                  JZ 0x006d9cb2
006d9c5e  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
006d9c61  8b11                  MOV EDX,dword ptr [ECX]
006d9c63  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
006d9c66  8b4210                MOV EAX,dword ptr [EDX + 0x10]
006d9c69  ffd0                  CALL EAX
006d9c6b  85c0                  TEST EAX,EAX
006d9c6d  7443                  JZ 0x006d9cb2
006d9c6f  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
006d9c72  8b11                  MOV EDX,dword ptr [ECX]
006d9c74  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
006d9c77  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006d9c7a  ffd0                  CALL EAX
006d9c7c  85c0                  TEST EAX,EAX
006d9c7e  7432                  JZ 0x006d9cb2
006d9c80  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
006d9c83  8b11                  MOV EDX,dword ptr [ECX]
006d9c85  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
006d9c88  8b4230                MOV EAX,dword ptr [EDX + 0x30]
006d9c8b  ffd0                  CALL EAX
006d9c8d  898544feffff          MOV dword ptr [EBP + 0xfffffe44],EAX
006d9c93  8b8d44feffff          MOV ECX,dword ptr [EBP + 0xfffffe44]
006d9c99  8b11                  MOV EDX,dword ptr [ECX]
006d9c9b  8b8d44feffff          MOV ECX,dword ptr [EBP + 0xfffffe44]
006d9ca1  8b8294000000          MOV EAX,dword ptr [EDX + 0x94]
006d9ca7  ffd0                  CALL EAX
006d9ca9  85c0                  TEST EAX,EAX
006d9cab  7405                  JZ 0x006d9cb2
006d9cad  e971070000            JMP 0x006da423
006d9cb2  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006d9cb8  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006d9cbb  8995c0feffff          MOV dword ptr [EBP + 0xfffffec0],EDX
006d9cc1  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9cc4  8b4864                MOV ECX,dword ptr [EAX + 0x64]
006d9cc7  51                    PUSH ECX
006d9cc8  8b8dc0feffff          MOV ECX,dword ptr [EBP + 0xfffffec0]
006d9cce  e80d24e4ff            CALL 0x0051c0e0
006d9cd3  8945b0                MOV dword ptr [EBP + -0x50],EAX
006d9cd6  837db000              CMP dword ptr [EBP + -0x50],0x0
006d9cda  0f84c4010000          JZ 0x006d9ea4
006d9ce0  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9ce3  8b4268                MOV EAX,dword ptr [EDX + 0x68]
006d9ce6  50                    PUSH EAX
006d9ce7  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d9cea  e871a5f2ff            CALL 0x00604260
006d9cef  8945ac                MOV dword ptr [EBP + -0x54],EAX
006d9cf2  837dac00              CMP dword ptr [EBP + -0x54],0x0
006d9cf6  0f84a8010000          JZ 0x006d9ea4
006d9cfc  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
006d9cff  668b5102              MOV DX,word ptr [ECX + 0x2]
006d9d03  668995befeffff        MOV word ptr [EBP + 0xfffffebe],DX
006d9d0a  0fb785befeffff        MOVZX EAX,word ptr [EBP + 0xfffffebe]
006d9d11  8945a8                MOV dword ptr [EBP + -0x58],EAX
006d9d14  c7459c00000000        MOV dword ptr [EBP + -0x64],0x0
006d9d1b  68449e9900            PUSH 0x999e44
006d9d20  8d8d5cffffff          LEA ECX,[EBP + 0xffffff5c]
006d9d26  e845980500            CALL 0x00733570
006d9d2b  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
006d9d32  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
006d9d38  8b9130010000          MOV EDX,dword ptr [ECX + 0x130]
006d9d3e  8995b8feffff          MOV dword ptr [EBP + 0xfffffeb8],EDX
006d9d44  8b85b8feffff          MOV EAX,dword ptr [EBP + 0xfffffeb8]
006d9d4a  8b88f8000000          MOV ECX,dword ptr [EAX + 0xf8]
006d9d50  898db4feffff          MOV dword ptr [EBP + 0xfffffeb4],ECX
006d9d56  8d559c                LEA EDX,[EBP + -0x64]
006d9d59  52                    PUSH EDX
006d9d5a  8d855cffffff          LEA EAX,[EBP + 0xffffff5c]
006d9d60  50                    PUSH EAX
006d9d61  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
006d9d64  51                    PUSH ECX
006d9d65  8b8db4feffff          MOV ECX,dword ptr [EBP + 0xfffffeb4]
006d9d6b  e8801f0400            CALL 0x0071bcf0
006d9d70  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
006d9d77  8d8d5cffffff          LEA ECX,[EBP + 0xffffff5c]
006d9d7d  e8fe990500            CALL 0x00733780
006d9d82  c745a400000000        MOV dword ptr [EBP + -0x5c],0x0
006d9d89  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
006d9d8c  52                    PUSH EDX
006d9d8d  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9d93  e898c6e9ff            CALL 0x00576430
006d9d98  8945a0                MOV dword ptr [EBP + -0x60],EAX
006d9d9b  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
006d9d9e  3b45a8                CMP EAX,dword ptr [EBP + -0x58]
006d9da1  740d                  JZ 0x006d9db0
006d9da3  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
006d9da6  894da8                MOV dword ptr [EBP + -0x58],ECX
006d9da9  c745a401000000        MOV dword ptr [EBP + -0x5c],0x1
006d9db0  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
006d9db6  8995b0feffff          MOV dword ptr [EBP + 0xfffffeb0],EDX
006d9dbc  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
006d9dbf  50                    PUSH EAX
006d9dc0  8b8db0feffff          MOV ECX,dword ptr [EBP + 0xfffffeb0]
006d9dc6  8b8904010000          MOV ECX,dword ptr [ECX + 0x104]
006d9dcc  e87f76feff            CALL 0x006c1450
006d9dd1  894598                MOV dword ptr [EBP + -0x68],EAX
006d9dd4  837d9800              CMP dword ptr [EBP + -0x68],0x0
006d9dd8  7429                  JZ 0x006d9e03
006d9dda  8b5598                MOV EDX,dword ptr [EBP + -0x68]
006d9ddd  8b8254010000          MOV EAX,dword ptr [EDX + 0x154]
006d9de3  8985acfeffff          MOV dword ptr [EBP + 0xfffffeac],EAX
006d9de9  83bdacfeffff00        CMP dword ptr [EBP + 0xfffffeac],0x0
006d9df0  7411                  JZ 0x006d9e03
006d9df2  6a00                  PUSH 0x0
006d9df4  6a00                  PUSH 0x0
006d9df6  6a01                  PUSH 0x1
006d9df8  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9dfe  e89d9fe8ff            CALL 0x00563da0
006d9e03  837d9c00              CMP dword ptr [EBP + -0x64],0x0
006d9e07  0f8f97000000          JG 0x006d9ea4
006d9e0d  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9e13  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d9e16  8995a8feffff          MOV dword ptr [EBP + 0xfffffea8],EDX
006d9e1c  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9e1f  8b4844                MOV ECX,dword ptr [EAX + 0x44]
006d9e22  3b8da8feffff          CMP ECX,dword ptr [EBP + 0xfffffea8]
006d9e28  747a                  JZ 0x006d9ea4
006d9e2a  8b5598                MOV EDX,dword ptr [EBP + -0x68]
006d9e2d  8b8254010000          MOV EAX,dword ptr [EDX + 0x154]
006d9e33  8985a4feffff          MOV dword ptr [EBP + 0xfffffea4],EAX
006d9e39  83bda4feffff00        CMP dword ptr [EBP + 0xfffffea4],0x0
006d9e40  7462                  JZ 0x006d9ea4
006d9e42  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d9e45  e8e6cfffff            CALL 0x006d6e30
006d9e4a  8985a0feffff          MOV dword ptr [EBP + 0xfffffea0],EAX
006d9e50  8b8da0feffff          MOV ECX,dword ptr [EBP + 0xfffffea0]
006d9e56  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
006d9e5c  88959ffeffff          MOV byte ptr [EBP + 0xfffffe9f],DL
006d9e62  0fb6859ffeffff        MOVZX EAX,byte ptr [EBP + 0xfffffe9f]
006d9e69  83f831                CMP EAX,0x31
006d9e6c  7436                  JZ 0x006d9ea4
006d9e6e  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
006d9e71  e8bacfffff            CALL 0x006d6e30
006d9e76  898598feffff          MOV dword ptr [EBP + 0xfffffe98],EAX
006d9e7c  8b8d98feffff          MOV ECX,dword ptr [EBP + 0xfffffe98]
006d9e82  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
006d9e88  889597feffff          MOV byte ptr [EBP + 0xfffffe97],DL
006d9e8e  0fb68597feffff        MOVZX EAX,byte ptr [EBP + 0xfffffe97]
006d9e95  83f806                CMP EAX,0x6
006d9e98  740a                  JZ 0x006d9ea4
006d9e9a  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9e9d  c741440000007f        MOV dword ptr [ECX + 0x44],0x7f000000
006d9ea4  6a01                  PUSH 0x1
006d9ea6  6a01                  PUSH 0x1
006d9ea8  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9eab  8b4248                MOV EAX,dword ptr [EDX + 0x48]
006d9eae  50                    PUSH EAX
006d9eaf  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9eb2  8b9184000000          MOV EDX,dword ptr [ECX + 0x84]
006d9eb8  52                    PUSH EDX
006d9eb9  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9ebc  8b4844                MOV ECX,dword ptr [EAX + 0x44]
006d9ebf  51                    PUSH ECX
006d9ec0  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9ec3  83c238                ADD EDX,0x38
006d9ec6  83ec0c                SUB ESP,0xc
006d9ec9  8bc4                  MOV EAX,ESP
006d9ecb  8b0a                  MOV ECX,dword ptr [EDX]
006d9ecd  8908                  MOV dword ptr [EAX],ECX
006d9ecf  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
006d9ed2  894804                MOV dword ptr [EAX + 0x4],ECX
006d9ed5  8b5208                MOV EDX,dword ptr [EDX + 0x8]
006d9ed8  895008                MOV dword ptr [EAX + 0x8],EDX
006d9edb  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006d9ede  8b486c                MOV ECX,dword ptr [EAX + 0x6c]
006d9ee1  51                    PUSH ECX
006d9ee2  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006d9ee5  8b4268                MOV EAX,dword ptr [EDX + 0x68]
006d9ee8  50                    PUSH EAX
006d9ee9  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006d9eec  8b5164                MOV EDX,dword ptr [ECX + 0x64]
006d9eef  52                    PUSH EDX
006d9ef0  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9ef6  e8d5a9e8ff            CALL 0x005648d0
006d9efb  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006d9f01  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006d9f07  898d90feffff          MOV dword ptr [EBP + 0xfffffe90],ECX
006d9f0d  8b9590feffff          MOV EDX,dword ptr [EBP + 0xfffffe90]
006d9f13  8b82180b0000          MOV EAX,dword ptr [EDX + 0xb18]
006d9f19  898554ffffff          MOV dword ptr [EBP + 0xffffff54],EAX
006d9f1f  8b8d54ffffff          MOV ECX,dword ptr [EBP + 0xffffff54]
006d9f25  898d58ffffff          MOV dword ptr [EBP + 0xffffff58],ECX
006d9f2b  83bd58ffffff00        CMP dword ptr [EBP + 0xffffff58],0x0
006d9f32  7432                  JZ 0x006d9f66
006d9f34  8b8d58ffffff          MOV ECX,dword ptr [EBP + 0xffffff58]
006d9f3a  e8e193f4ff            CALL 0x00623320
006d9f3f  ba01000000            MOV EDX,0x1
006d9f44  83e201                AND EDX,0x1
006d9f47  740f                  JZ 0x006d9f58
006d9f49  8b8558ffffff          MOV EAX,dword ptr [EBP + 0xffffff58]
006d9f4f  50                    PUSH EAX
006d9f50  e8a8f72300            CALL 0x009196fd
006d9f55  83c404                ADD ESP,0x4
006d9f58  8b8d58ffffff          MOV ECX,dword ptr [EBP + 0xffffff58]
006d9f5e  898d40feffff          MOV dword ptr [EBP + 0xfffffe40],ECX
006d9f64  eb0a                  JMP 0x006d9f70
006d9f66  c78540feffff00000000  MOV dword ptr [EBP + 0xfffffe40],0x0
006d9f70  8b9554feffff          MOV EDX,dword ptr [EBP + 0xfffffe54]
006d9f76  8b82dc100000          MOV EAX,dword ptr [EDX + 0x10dc]
006d9f7c  89858cfeffff          MOV dword ptr [EBP + 0xfffffe8c],EAX
006d9f82  8b8d8cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe8c]
006d9f88  c781180b000000000000  MOV dword ptr [ECX + 0xb18],0x0
006d9f92  8b958cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe8c]
006d9f98  83ba180b000000        CMP dword ptr [EDX + 0xb18],0x0
006d9f9f  741b                  JZ 0x006d9fbc
006d9fa1  8b858cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe8c]
006d9fa7  8b88180b0000          MOV ECX,dword ptr [EAX + 0xb18]
006d9fad  8b958cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe8c]
006d9fb3  8a4110                MOV AL,byte ptr [ECX + 0x10]
006d9fb6  88821c0b0000          MOV byte ptr [EDX + 0xb1c],AL
006d9fbc  a1c0449900            MOV EAX,[0x009944c0]
006d9fc1  e9d9040000            JMP 0x006da49f
006d9fcb  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006d9fd1  e8eab3e6ff            CALL 0x005453c0
006d9fd6  0520010000            ADD EAX,0x120
006d9fdb  898588feffff          MOV dword ptr [EBP + 0xfffffe88],EAX
006d9fe1  8b8d88feffff          MOV ECX,dword ptr [EBP + 0xfffffe88]
006d9fe7  8b5104                MOV EDX,dword ptr [ECX + 0x4]
006d9fea  899584feffff          MOV dword ptr [EBP + 0xfffffe84],EDX
006d9ff0  8b8584feffff          MOV EAX,dword ptr [EBP + 0xfffffe84]
006d9ff6  894590                MOV dword ptr [EBP + -0x70],EAX
006d9ff9  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
006d9fff  8b5108                MOV EDX,dword ptr [ECX + 0x8]
006da002  899580feffff          MOV dword ptr [EBP + 0xfffffe80],EDX
006da008  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006da00b  8b4814                MOV ECX,dword ptr [EAX + 0x14]
006da00e  51                    PUSH ECX
006da00f  8b8d80feffff          MOV ECX,dword ptr [EBP + 0xfffffe80]
006da015  e88620e4ff            CALL 0x0051c0a0
006da01a  894594                MOV dword ptr [EBP + -0x6c],EAX
006da01d  837d9400              CMP dword ptr [EBP + -0x6c],0x0
006da021  0f84fa010000          JZ 0x006da221
006da027  8b5594                MOV EDX,dword ptr [EBP + -0x6c]
006da02a  8b02                  MOV EAX,dword ptr [EDX]
006da02c  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
006da02f  8b5010                MOV EDX,dword ptr [EAX + 0x10]
006da032  ffd2                  CALL EDX
006da034  85c0                  TEST EAX,EAX
006da036  0f84e5010000          JZ 0x006da221
006da03c  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006da03f  8178140000007f        CMP dword ptr [EAX + 0x14],0x7f000000
006da046  0f84d5010000          JZ 0x006da221
006da04c  c7458c00000000        MOV dword ptr [EBP + -0x74],0x0
006da053  d90574519900          FLD float ptr [0x00995174]
006da059  d95d80                FSTP float ptr [EBP + -0x80]
006da05c  c7458401000000        MOV dword ptr [EBP + -0x7c],0x1
006da063  c7458800000000        MOV dword ptr [EBP + -0x78],0x0
006da06a  6a00                  PUSH 0x0
006da06c  6a00                  PUSH 0x0
006da06e  6a00                  PUSH 0x0
006da070  6a00                  PUSH 0x0
006da072  6a00                  PUSH 0x0
006da074  6a00                  PUSH 0x0
006da076  6a00                  PUSH 0x0
006da078  6a00                  PUSH 0x0
006da07a  6a00                  PUSH 0x0
006da07c  6a00                  PUSH 0x0
006da07e  6a00                  PUSH 0x0
006da080  6a00                  PUSH 0x0
006da082  8d4d8c                LEA ECX,[EBP + -0x74]
006da085  51                    PUSH ECX
006da086  6a01                  PUSH 0x1
006da088  8d5588                LEA EDX,[EBP + -0x78]
006da08b  52                    PUSH EDX
006da08c  6a01                  PUSH 0x1
006da08e  8d4584                LEA EAX,[EBP + -0x7c]
006da091  50                    PUSH EAX
006da092  6a01                  PUSH 0x1
006da094  8d4d80                LEA ECX,[EBP + -0x80]
006da097  51                    PUSH ECX
006da098  6a02                  PUSH 0x2
006da09a  8d5580                LEA EDX,[EBP + -0x80]
006da09d  52                    PUSH EDX
006da09e  6a02                  PUSH 0x2
006da0a0  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
006da0a3  83c054                ADD EAX,0x54
006da0a6  50                    PUSH EAX
006da0a7  6a01                  PUSH 0x1
006da0a9  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006da0ac  83c114                ADD ECX,0x14
006da0af  51                    PUSH ECX
006da0b0  6a03                  PUSH 0x3
006da0b2  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006da0b5  0fb7426c              MOVZX EAX,word ptr [EDX + 0x6c]
006da0b9  50                    PUSH EAX
006da0ba  6a11                  PUSH 0x11
006da0bc  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da0c2  e8d96be6ff            CALL 0x00540ca0
006da0c7  d9ee                  FLDZ
006da0c9  d99d48ffffff          FSTP float ptr [EBP + 0xffffff48]
006da0cf  d9ee                  FLDZ
006da0d1  d99d4cffffff          FSTP float ptr [EBP + 0xffffff4c]
006da0d7  d9ee                  FLDZ
006da0d9  d99d50ffffff          FSTP float ptr [EBP + 0xffffff50]
006da0df  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
006da0e2  8b11                  MOV EDX,dword ptr [ECX]
006da0e4  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
006da0e7  8b4210                MOV EAX,dword ptr [EDX + 0x10]
006da0ea  ffd0                  CALL EAX
006da0ec  89857cfeffff          MOV dword ptr [EBP + 0xfffffe7c],EAX
006da0f2  6a00                  PUSH 0x0
006da0f4  6a00                  PUSH 0x0
006da0f6  6a00                  PUSH 0x0
006da0f8  8d8d48ffffff          LEA ECX,[EBP + 0xffffff48]
006da0fe  51                    PUSH ECX
006da0ff  6a00                  PUSH 0x0
006da101  6a00                  PUSH 0x0
006da103  6a00                  PUSH 0x0
006da105  6a00                  PUSH 0x0
006da107  6a00                  PUSH 0x0
006da109  6a00                  PUSH 0x0
006da10b  51                    PUSH ECX
006da10c  d905fc769800          FLD float ptr [0x009876fc]
006da112  d91c24                FSTP float ptr [ESP]
006da115  51                    PUSH ECX
006da116  d90574519900          FLD float ptr [0x00995174]
006da11c  d91c24                FSTP float ptr [ESP]
006da11f  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006da122  8b4254                MOV EAX,dword ptr [EDX + 0x54]
006da125  50                    PUSH EAX
006da126  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006da129  8b5114                MOV EDX,dword ptr [ECX + 0x14]
006da12c  52                    PUSH EDX
006da12d  8b4590                MOV EAX,dword ptr [EBP + -0x70]
006da130  50                    PUSH EAX
006da131  8b8d7cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe7c]
006da137  81c194000000          ADD ECX,0x94
006da13d  51                    PUSH ECX
006da13e  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006da141  0fb7426c              MOVZX EAX,word ptr [EDX + 0x6c]
006da145  50                    PUSH EAX
006da146  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da14c  e85fa3e8ff            CALL 0x005644b0
006da151  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da157  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006da15d  899578feffff          MOV dword ptr [EBP + 0xfffffe78],EDX
006da163  8b8578feffff          MOV EAX,dword ptr [EBP + 0xfffffe78]
006da169  8b88180b0000          MOV ECX,dword ptr [EAX + 0xb18]
006da16f  898d40ffffff          MOV dword ptr [EBP + 0xffffff40],ECX
006da175  8b9540ffffff          MOV EDX,dword ptr [EBP + 0xffffff40]
006da17b  899544ffffff          MOV dword ptr [EBP + 0xffffff44],EDX
006da181  83bd44ffffff00        CMP dword ptr [EBP + 0xffffff44],0x0
006da188  7432                  JZ 0x006da1bc
006da18a  8b8d44ffffff          MOV ECX,dword ptr [EBP + 0xffffff44]
006da190  e88b91f4ff            CALL 0x00623320
006da195  b801000000            MOV EAX,0x1
006da19a  83e001                AND EAX,0x1
006da19d  740f                  JZ 0x006da1ae
006da19f  8b8d44ffffff          MOV ECX,dword ptr [EBP + 0xffffff44]
006da1a5  51                    PUSH ECX
006da1a6  e852f52300            CALL 0x009196fd
006da1ab  83c404                ADD ESP,0x4
006da1ae  8b9544ffffff          MOV EDX,dword ptr [EBP + 0xffffff44]
006da1b4  89953cfeffff          MOV dword ptr [EBP + 0xfffffe3c],EDX
006da1ba  eb0a                  JMP 0x006da1c6
006da1bc  c7853cfeffff00000000  MOV dword ptr [EBP + 0xfffffe3c],0x0
006da1c6  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006da1cc  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006da1d2  898d74feffff          MOV dword ptr [EBP + 0xfffffe74],ECX
006da1d8  8b9574feffff          MOV EDX,dword ptr [EBP + 0xfffffe74]
006da1de  c782180b000000000000  MOV dword ptr [EDX + 0xb18],0x0
006da1e8  8b8574feffff          MOV EAX,dword ptr [EBP + 0xfffffe74]
006da1ee  83b8180b000000        CMP dword ptr [EAX + 0xb18],0x0
006da1f5  741b                  JZ 0x006da212
006da1f7  8b8d74feffff          MOV ECX,dword ptr [EBP + 0xfffffe74]
006da1fd  8b91180b0000          MOV EDX,dword ptr [ECX + 0xb18]
006da203  8b8574feffff          MOV EAX,dword ptr [EBP + 0xfffffe74]
006da209  8a4a10                MOV CL,byte ptr [EDX + 0x10]
006da20c  88881c0b0000          MOV byte ptr [EAX + 0xb1c],CL
006da212  a1c0449900            MOV EAX,[0x009944c0]
006da217  e983020000            JMP 0x006da49f
006da221  c7857cffffff00000000  MOV dword ptr [EBP + 0xffffff7c],0x0
006da22b  d90574519900          FLD float ptr [0x00995174]
006da231  d99d74ffffff          FSTP float ptr [EBP + 0xffffff74]
006da237  c78578ffffff0000007f  MOV dword ptr [EBP + 0xffffff78],0x7f000000
006da241  6a00                  PUSH 0x0
006da243  6a00                  PUSH 0x0
006da245  6a00                  PUSH 0x0
006da247  6a00                  PUSH 0x0
006da249  6a00                  PUSH 0x0
006da24b  6a00                  PUSH 0x0
006da24d  6a00                  PUSH 0x0
006da24f  6a00                  PUSH 0x0
006da251  6a00                  PUSH 0x0
006da253  6a00                  PUSH 0x0
006da255  8d957cffffff          LEA EDX,[EBP + 0xffffff7c]
006da25b  52                    PUSH EDX
006da25c  6a01                  PUSH 0x1
006da25e  8d8574ffffff          LEA EAX,[EBP + 0xffffff74]
006da264  50                    PUSH EAX
006da265  6a02                  PUSH 0x2
006da267  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006da26a  83c154                ADD ECX,0x54
006da26d  51                    PUSH ECX
006da26e  6a01                  PUSH 0x1
006da270  8d9578ffffff          LEA EDX,[EBP + 0xffffff78]
006da276  52                    PUSH EDX
006da277  6a03                  PUSH 0x3
006da279  8d4590                LEA EAX,[EBP + -0x70]
006da27c  50                    PUSH EAX
006da27d  6a03                  PUSH 0x3
006da27f  b902000000            MOV ECX,0x2
006da284  c1e102                SHL ECX,0x2
006da287  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006da28a  8d440a38              LEA EAX,[EDX + ECX*0x1 + 0x38]
006da28e  50                    PUSH EAX
006da28f  6a02                  PUSH 0x2
006da291  b901000000            MOV ECX,0x1
006da296  c1e102                SHL ECX,0x2
006da299  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006da29c  8d440a38              LEA EAX,[EDX + ECX*0x1 + 0x38]
006da2a0  50                    PUSH EAX
006da2a1  6a02                  PUSH 0x2
006da2a3  33c9                  XOR ECX,ECX
006da2a5  c1e102                SHL ECX,0x2
006da2a8  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
006da2ab  8d440a38              LEA EAX,[EDX + ECX*0x1 + 0x38]
006da2af  50                    PUSH EAX
006da2b0  6a02                  PUSH 0x2
006da2b2  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006da2b5  0fb7516c              MOVZX EDX,word ptr [ECX + 0x6c]
006da2b9  52                    PUSH EDX
006da2ba  6a12                  PUSH 0x12
006da2bc  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da2c2  e8d969e6ff            CALL 0x00540ca0
006da2c7  d9ee                  FLDZ
006da2c9  d99d34ffffff          FSTP float ptr [EBP + 0xffffff34]
006da2cf  d9ee                  FLDZ
006da2d1  d99d38ffffff          FSTP float ptr [EBP + 0xffffff38]
006da2d7  d9ee                  FLDZ
006da2d9  d99d3cffffff          FSTP float ptr [EBP + 0xffffff3c]
006da2df  6a00                  PUSH 0x0
006da2e1  6a00                  PUSH 0x0
006da2e3  6a00                  PUSH 0x0
006da2e5  8d8534ffffff          LEA EAX,[EBP + 0xffffff34]
006da2eb  50                    PUSH EAX
006da2ec  6a00                  PUSH 0x0
006da2ee  6a00                  PUSH 0x0
006da2f0  6a00                  PUSH 0x0
006da2f2  6a00                  PUSH 0x0
006da2f4  6a00                  PUSH 0x0
006da2f6  6a00                  PUSH 0x0
006da2f8  51                    PUSH ECX
006da2f9  d905fc769800          FLD float ptr [0x009876fc]
006da2ff  d91c24                FSTP float ptr [ESP]
006da302  51                    PUSH ECX
006da303  d90574519900          FLD float ptr [0x00995174]
006da309  d91c24                FSTP float ptr [ESP]
006da30c  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006da30f  8b5154                MOV EDX,dword ptr [ECX + 0x54]
006da312  52                    PUSH EDX
006da313  680000007f            PUSH 0x7f000000
006da318  8b4590                MOV EAX,dword ptr [EBP + -0x70]
006da31b  50                    PUSH EAX
006da31c  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
006da31f  83c138                ADD ECX,0x38
006da322  51                    PUSH ECX
006da323  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006da326  0fb7426c              MOVZX EAX,word ptr [EDX + 0x6c]
006da32a  50                    PUSH EAX
006da32b  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da331  e87aa1e8ff            CALL 0x005644b0
006da336  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da33c  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006da342  899570feffff          MOV dword ptr [EBP + 0xfffffe70],EDX
006da348  8b8570feffff          MOV EAX,dword ptr [EBP + 0xfffffe70]
006da34e  8b88180b0000          MOV ECX,dword ptr [EAX + 0xb18]
006da354  898d2cffffff          MOV dword ptr [EBP + 0xffffff2c],ECX
006da35a  8b952cffffff          MOV EDX,dword ptr [EBP + 0xffffff2c]
006da360  899530ffffff          MOV dword ptr [EBP + 0xffffff30],EDX
006da366  83bd30ffffff00        CMP dword ptr [EBP + 0xffffff30],0x0
006da36d  7432                  JZ 0x006da3a1
006da36f  8b8d30ffffff          MOV ECX,dword ptr [EBP + 0xffffff30]
006da375  e8a68ff4ff            CALL 0x00623320
006da37a  b801000000            MOV EAX,0x1
006da37f  83e001                AND EAX,0x1
006da382  740f                  JZ 0x006da393
006da384  8b8d30ffffff          MOV ECX,dword ptr [EBP + 0xffffff30]
006da38a  51                    PUSH ECX
006da38b  e86df32300            CALL 0x009196fd
006da390  83c404                ADD ESP,0x4
006da393  8b9530ffffff          MOV EDX,dword ptr [EBP + 0xffffff30]
006da399  899538feffff          MOV dword ptr [EBP + 0xfffffe38],EDX
006da39f  eb0a                  JMP 0x006da3ab
006da3a1  c78538feffff00000000  MOV dword ptr [EBP + 0xfffffe38],0x0
006da3ab  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006da3b1  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006da3b7  898d6cfeffff          MOV dword ptr [EBP + 0xfffffe6c],ECX
006da3bd  8b956cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe6c]
006da3c3  c782180b000000000000  MOV dword ptr [EDX + 0xb18],0x0
006da3cd  8b856cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe6c]
006da3d3  83b8180b000000        CMP dword ptr [EAX + 0xb18],0x0
006da3da  741b                  JZ 0x006da3f7
006da3dc  8b8d6cfeffff          MOV ECX,dword ptr [EBP + 0xfffffe6c]
006da3e2  8b91180b0000          MOV EDX,dword ptr [ECX + 0xb18]
006da3e8  8b856cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe6c]
006da3ee  8a4a10                MOV CL,byte ptr [EDX + 0x10]
006da3f1  88881c0b0000          MOV byte ptr [EAX + 0xb1c],CL
006da3f7  a1c0449900            MOV EAX,[0x009944c0]
006da3fc  e99e000000            JMP 0x006da49f
006da401  6a01                  PUSH 0x1
006da403  6a01                  PUSH 0x1
006da405  6a00                  PUSH 0x0
006da407  6a00                  PUSH 0x0
006da409  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006da40c  52                    PUSH EDX
006da40d  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
006da410  50                    PUSH EAX
006da411  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da417  e8f4bfe8ff            CALL 0x00566410
006da41c  a1c0449900            MOV EAX,[0x009944c0]
006da421  eb7c                  JMP 0x006da49f
006da423  eb75                  JMP 0x006da49a
006da425  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da42b  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006da431  899568feffff          MOV dword ptr [EBP + 0xfffffe68],EDX
006da437  8b8d68feffff          MOV ECX,dword ptr [EBP + 0xfffffe68]
006da43d  e8ce5cebff            CALL 0x00590110
006da442  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
006da448  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
006da44e  898d64feffff          MOV dword ptr [EBP + 0xfffffe64],ECX
006da454  8b9564feffff          MOV EDX,dword ptr [EBP + 0xfffffe64]
006da45a  8b82b80a0000          MOV EAX,dword ptr [EDX + 0xab8]
006da460  898560feffff          MOV dword ptr [EBP + 0xfffffe60],EAX
006da466  8b8d54feffff          MOV ECX,dword ptr [EBP + 0xfffffe54]
006da46c  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006da472  89955cfeffff          MOV dword ptr [EBP + 0xfffffe5c],EDX
006da478  8b855cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe5c]
006da47e  8b88840a0000          MOV ECX,dword ptr [EAX + 0xa84]
006da484  898d58feffff          MOV dword ptr [EBP + 0xfffffe58],ECX
006da48a  83bd58feffff00        CMP dword ptr [EBP + 0xfffffe58],0x0
006da491  7507                  JNZ 0x006da49a
006da493  a1b8449900            MOV EAX,[0x009944b8]
006da498  eb05                  JMP 0x006da49f
006da49a  a1b4449900            MOV EAX,[0x009944b4]
006da49f  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006da4a2  64890d00000000        MOV dword ptr FS:[0x0],ECX
006da4a9  59                    POP ECX
006da4aa  8be5                  MOV ESP,EBP
006da4ac  5d                    POP EBP
006da4ad  c20400                RET 0x4
