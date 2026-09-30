; FN 008c9460 FUN_008c9460
008c9460  55                    PUSH EBP
008c9461  8bec                  MOV EBP,ESP
008c9463  83ec58                SUB ESP,0x58
008c9466  894da8                MOV dword ptr [EBP + -0x58],ECX
008c9469  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008c946c  8b4850                MOV ECX,dword ptr [EAX + 0x50]
008c946f  894de8                MOV dword ptr [EBP + -0x18],ECX
008c9472  837de800              CMP dword ptr [EBP + -0x18],0x0
008c9476  0f84c0020000          JZ 0x008c973c
008c947c  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008c947f  8b424c                MOV EAX,dword ptr [EDX + 0x4c]
008c9482  8945e4                MOV dword ptr [EBP + -0x1c],EAX
008c9485  837de42d              CMP dword ptr [EBP + -0x1c],0x2d
008c9489  7513                  JNZ 0x008c949e
008c948b  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c948e  8b511c                MOV EDX,dword ptr [ECX + 0x1c]
008c9491  8955e0                MOV dword ptr [EBP + -0x20],EDX
008c9494  6a00                  PUSH 0x0
008c9496  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008c9499  e8028eb4ff            CALL 0x004122a0
008c949e  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008c94a3  8b4808                MOV ECX,dword ptr [EAX + 0x8]
008c94a6  894dfc                MOV dword ptr [EBP + -0x4],ECX
008c94a9  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008c94ac  81c2d8080000          ADD EDX,0x8d8
008c94b2  395508                CMP dword ptr [EBP + 0x8],EDX
008c94b5  7527                  JNZ 0x008c94de
008c94b7  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c94ba  83c168                ADD ECX,0x68
008c94bd  e8ee6ab5ff            CALL 0x0041ffb0
008c94c2  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c94c5  8981900c0000          MOV dword ptr [ECX + 0xc90],EAX
008c94cb  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008c94ce  83ba900c000000        CMP dword ptr [EDX + 0xc90],0x0
008c94d5  7505                  JNZ 0x008c94dc
008c94d7  e960020000            JMP 0x008c973c
008c94dc  eb0c                  JMP 0x008c94ea
008c94de  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c94e1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008c94e4  8988900c0000          MOV dword ptr [EAX + 0xc90],ECX
008c94ea  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008c94ed  8b82900c0000          MOV EAX,dword ptr [EDX + 0xc90]
008c94f3  8b88d0010000          MOV ECX,dword ptr [EAX + 0x1d0]
008c94f9  894ddc                MOV dword ptr [EBP + -0x24],ECX
008c94fc  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008c94ff  52                    PUSH EDX
008c9500  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008c9503  e8d82bc5ff            CALL 0x0051c0e0
008c9508  8945f8                MOV dword ptr [EBP + -0x8],EAX
008c950b  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c950e  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c9514  83e1fe                AND ECX,0xfffffffe
008c9517  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008c951a  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c9520  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c9523  8b88900c0000          MOV ECX,dword ptr [EAX + 0xc90]
008c9529  8b91dc010000          MOV EDX,dword ptr [ECX + 0x1dc]
008c952f  83e201                AND EDX,0x1
008c9532  0f846e010000          JZ 0x008c96a6
008c9538  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c953b  8b88900c0000          MOV ECX,dword ptr [EAX + 0xc90]
008c9541  8b91d4010000          MOV EDX,dword ptr [ECX + 0x1d4]
008c9547  8955d8                MOV dword ptr [EBP + -0x28],EDX
008c954a  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
008c954d  50                    PUSH EAX
008c954e  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008c9551  e8aa2bc5ff            CALL 0x0051c100
008c9556  8945f4                MOV dword ptr [EBP + -0xc],EAX
008c9559  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c955c  8b91900c0000          MOV EDX,dword ptr [ECX + 0xc90]
008c9562  8b82d8010000          MOV EAX,dword ptr [EDX + 0x1d8]
008c9568  8945d4                MOV dword ptr [EBP + -0x2c],EAX
008c956b  837dd410              CMP dword ptr [EBP + -0x2c],0x10
008c956f  741c                  JZ 0x008c958d
008c9571  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c9574  8b91900c0000          MOV EDX,dword ptr [ECX + 0xc90]
008c957a  8b82d8010000          MOV EAX,dword ptr [EDX + 0x1d8]
008c9580  8945d0                MOV dword ptr [EBP + -0x30],EAX
008c9583  837dd020              CMP dword ptr [EBP + -0x30],0x20
008c9587  0f850b010000          JNZ 0x008c9698
008c958d  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c9590  8b9150110000          MOV EDX,dword ptr [ECX + 0x1150]
008c9596  8955cc                MOV dword ptr [EBP + -0x34],EDX
008c9599  6a10                  PUSH 0x10
008c959b  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
008c959e  e87d70e0ff            CALL 0x006d0620
008c95a3  8945ec                MOV dword ptr [EBP + -0x14],EAX
008c95a6  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
008c95a9  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
008c95af  894dc8                MOV dword ptr [EBP + -0x38],ECX
008c95b2  6a20                  PUSH 0x20
008c95b4  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008c95b7  e86470e0ff            CALL 0x006d0620
008c95bc  8945f0                MOV dword ptr [EBP + -0x10],EAX
008c95bf  837df000              CMP dword ptr [EBP + -0x10],0x0
008c95c3  7511                  JNZ 0x008c95d6
008c95c5  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
008c95c8  52                    PUSH EDX
008c95c9  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c95cc  e87f49caff            CALL 0x0056df50
008c95d1  e9c0000000            JMP 0x008c9696
008c95d6  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008c95d9  3b45f8                CMP EAX,dword ptr [EBP + -0x8]
008c95dc  7559                  JNZ 0x008c9637
008c95de  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c95e1  8b917c0c0000          MOV EDX,dword ptr [ECX + 0xc7c]
008c95e7  83ca01                OR EDX,0x1
008c95ea  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c95ed  89907c0c0000          MOV dword ptr [EAX + 0xc7c],EDX
008c95f3  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c95f6  8b917c0c0000          MOV EDX,dword ptr [ECX + 0xc7c]
008c95fc  83e2fd                AND EDX,0xfffffffd
008c95ff  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c9602  89907c0c0000          MOV dword ptr [EAX + 0xc7c],EDX
008c9608  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008c960b  8b5114                MOV EDX,dword ptr [ECX + 0x14]
008c960e  8955c4                MOV dword ptr [EBP + -0x3c],EDX
008c9611  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c9614  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
008c9617  8988800c0000          MOV dword ptr [EAX + 0xc80],ECX
008c961d  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
008c9620  52                    PUSH EDX
008c9621  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c9624  e82749caff            CALL 0x0056df50
008c9629  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
008c962c  50                    PUSH EAX
008c962d  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c9630  e81b49caff            CALL 0x0056df50
008c9635  eb5f                  JMP 0x008c9696
008c9637  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008c963a  3b4df8                CMP ECX,dword ptr [EBP + -0x8]
008c963d  7557                  JNZ 0x008c9696
008c963f  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008c9642  8b827c0c0000          MOV EAX,dword ptr [EDX + 0xc7c]
008c9648  83c801                OR EAX,0x1
008c964b  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c964e  89817c0c0000          MOV dword ptr [ECX + 0xc7c],EAX
008c9654  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008c9657  8b827c0c0000          MOV EAX,dword ptr [EDX + 0xc7c]
008c965d  83c802                OR EAX,0x2
008c9660  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c9663  89817c0c0000          MOV dword ptr [ECX + 0xc7c],EAX
008c9669  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008c966c  8b4214                MOV EAX,dword ptr [EDX + 0x14]
008c966f  8945c0                MOV dword ptr [EBP + -0x40],EAX
008c9672  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c9675  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008c9678  8991800c0000          MOV dword ptr [ECX + 0xc80],EDX
008c967e  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
008c9681  50                    PUSH EAX
008c9682  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c9685  e8c648caff            CALL 0x0056df50
008c968a  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008c968d  51                    PUSH ECX
008c968e  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c9691  e8ba48caff            CALL 0x0056df50
008c9696  eb0c                  JMP 0x008c96a4
008c9698  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
008c969b  52                    PUSH EDX
008c969c  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c969f  e8ac48caff            CALL 0x0056df50
008c96a4  eb3b                  JMP 0x008c96e1
008c96a6  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
008c96a9  8b88cc020000          MOV ECX,dword ptr [EAX + 0x2cc]
008c96af  894dbc                MOV dword ptr [EBP + -0x44],ECX
008c96b2  837dbc01              CMP dword ptr [EBP + -0x44],0x1
008c96b6  7e0f                  JLE 0x008c96c7
008c96b8  6a01                  PUSH 0x1
008c96ba  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
008c96bd  e8feb7d3ff            CALL 0x00604ec0
008c96c2  8945f8                MOV dword ptr [EBP + -0x8],EAX
008c96c5  eb1a                  JMP 0x008c96e1
008c96c7  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
008c96ca  52                    PUSH EDX
008c96cb  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008c96ce  e8fd31c5ff            CALL 0x0051c8d0
008c96d3  8bc8                  MOV ECX,EAX
008c96d5  e856e3d2ff            CALL 0x005f7a30
008c96da  8bc8                  MOV ECX,EAX
008c96dc  e89fc7d3ff            CALL 0x00605e80
008c96e1  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c96e4  8a88780c0000          MOV CL,byte ptr [EAX + 0xc78]
008c96ea  884db7                MOV byte ptr [EBP + -0x49],CL
008c96ed  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008c96f0  8b82940c0000          MOV EAX,dword ptr [EDX + 0xc94]
008c96f6  8945b8                MOV dword ptr [EBP + -0x48],EAX
008c96f9  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
008c96fc  8a55b7                MOV DL,byte ptr [EBP + -0x49]
008c96ff  88912c3d0000          MOV byte ptr [ECX + 0x3d2c],DL
008c9705  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c9708  8b88940c0000          MOV ECX,dword ptr [EAX + 0xc94]
008c970e  894db0                MOV dword ptr [EBP + -0x50],ECX
008c9711  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
008c9714  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
008c9717  8982343d0000          MOV dword ptr [EDX + 0x3d34],EAX
008c971d  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008c9720  8b511c                MOV EDX,dword ptr [ECX + 0x1c]
008c9723  8955ac                MOV dword ptr [EBP + -0x54],EDX
008c9726  6a01                  PUSH 0x1
008c9728  6a03                  PUSH 0x3
008c972a  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c972d  8b88940c0000          MOV ECX,dword ptr [EAX + 0xc94]
008c9733  51                    PUSH ECX
008c9734  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
008c9737  e8f46db4ff            CALL 0x00410530
008c973c  8be5                  MOV ESP,EBP
008c973e  5d                    POP EBP
008c973f  c20400                RET 0x4
