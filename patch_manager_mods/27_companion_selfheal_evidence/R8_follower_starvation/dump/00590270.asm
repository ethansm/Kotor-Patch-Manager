; FN 00590270 FUN_00590270
00590270  55                    PUSH EBP
00590271  8bec                  MOV EBP,ESP
00590273  83ec28                SUB ESP,0x28
00590276  894de0                MOV dword ptr [EBP + -0x20],ECX
00590279  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
0059027c  83b8000b000000        CMP dword ptr [EAX + 0xb00],0x0
00590283  7544                  JNZ 0x005902c9
00590285  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00590288  894dec                MOV dword ptr [EBP + -0x14],ECX
0059028b  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
0059028e  8955f0                MOV dword ptr [EBP + -0x10],EDX
00590291  837df000              CMP dword ptr [EBP + -0x10],0x0
00590295  7426                  JZ 0x005902bd
00590297  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0059029a  e881300900            CALL 0x00623320
0059029f  b801000000            MOV EAX,0x1
005902a4  83e001                AND EAX,0x1
005902a7  740c                  JZ 0x005902b5
005902a9  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
005902ac  51                    PUSH ECX
005902ad  e84b943800            CALL 0x009196fd
005902b2  83c404                ADD ESP,0x4
005902b5  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
005902b8  8955dc                MOV dword ptr [EBP + -0x24],EDX
005902bb  eb07                  JMP 0x005902c4
005902bd  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
005902c4  e955010000            JMP 0x0059041e
005902c9  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
005902cc  8b88000b0000          MOV ECX,dword ptr [EAX + 0xb00]
005902d2  e8190ef9ff            CALL 0x005210f0
005902d7  83f804                CMP EAX,0x4
005902da  0f83f8000000          JNC 0x005903d8
005902e0  837d0cff              CMP dword ptr [EBP + 0xc],-0x1
005902e4  757d                  JNZ 0x00590363
005902e6  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005902e9  8b89000b0000          MOV ECX,dword ptr [ECX + 0xb00]
005902ef  e82c142100            CALL 0x007a1720
005902f4  8945fc                MOV dword ptr [EBP + -0x4],EAX
005902f7  837dfc00              CMP dword ptr [EBP + -0x4],0x0
005902fb  7452                  JZ 0x0059034f
005902fd  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00590300  52                    PUSH EDX
00590301  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
00590304  8b88000b0000          MOV ECX,dword ptr [EAX + 0xb00]
0059030a  e82111fdff            CALL 0x00561430
0059030f  8945f8                MOV dword ptr [EBP + -0x8],EAX
00590312  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00590315  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
00590318  8b01                  MOV EAX,dword ptr [ECX]
0059031a  3b02                  CMP EAX,dword ptr [EDX]
0059031c  7d1d                  JGE 0x0059033b
0059031e  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00590321  51                    PUSH ECX
00590322  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00590325  52                    PUSH EDX
00590326  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
00590329  8b88000b0000          MOV ECX,dword ptr [EAX + 0xb00]
0059032f  e89c0df9ff            CALL 0x005210d0
00590334  e9e5000000            JMP 0x0059041e
0059033b  8d4dfc                LEA ECX,[EBP + -0x4]
0059033e  51                    PUSH ECX
0059033f  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
00590342  8b8a000b0000          MOV ECX,dword ptr [EDX + 0xb00]
00590348  e823c0ffff            CALL 0x0058c370
0059034d  eba8                  JMP 0x005902f7
0059034f  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00590352  50                    PUSH EAX
00590353  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00590356  8b89000b0000          MOV ECX,dword ptr [ECX + 0xb00]
0059035c  e8cfa6fbff            CALL 0x0054aa30
00590361  eb73                  JMP 0x005903d6
00590363  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
00590366  8b8a000b0000          MOV ECX,dword ptr [EDX + 0xb00]
0059036c  e8af132100            CALL 0x007a1720
00590371  8945fc                MOV dword ptr [EBP + -0x4],EAX
00590374  c745f400000000        MOV dword ptr [EBP + -0xc],0x0
0059037b  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0059037f  7425                  JZ 0x005903a6
00590381  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00590384  3b450c                CMP EAX,dword ptr [EBP + 0xc]
00590387  7d1d                  JGE 0x005903a6
00590389  8d4dfc                LEA ECX,[EBP + -0x4]
0059038c  51                    PUSH ECX
0059038d  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
00590390  8b8a000b0000          MOV ECX,dword ptr [EDX + 0xb00]
00590396  e8d5bfffff            CALL 0x0058c370
0059039b  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0059039e  83c001                ADD EAX,0x1
005903a1  8945f4                MOV dword ptr [EBP + -0xc],EAX
005903a4  ebd5                  JMP 0x0059037b
005903a6  837dfc00              CMP dword ptr [EBP + -0x4],0x0
005903aa  7418                  JZ 0x005903c4
005903ac  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005903af  51                    PUSH ECX
005903b0  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005903b3  52                    PUSH EDX
005903b4  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
005903b7  8b88000b0000          MOV ECX,dword ptr [EAX + 0xb00]
005903bd  e80e0df9ff            CALL 0x005210d0
005903c2  eb12                  JMP 0x005903d6
005903c4  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005903c7  51                    PUSH ECX
005903c8  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
005903cb  8b8a000b0000          MOV ECX,dword ptr [EDX + 0xb00]
005903d1  e85aa6fbff            CALL 0x0054aa30
005903d6  eb46                  JMP 0x0059041e
005903d8  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005903db  8945e4                MOV dword ptr [EBP + -0x1c],EAX
005903de  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
005903e1  894de8                MOV dword ptr [EBP + -0x18],ECX
005903e4  837de800              CMP dword ptr [EBP + -0x18],0x0
005903e8  7426                  JZ 0x00590410
005903ea  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005903ed  e82e2f0900            CALL 0x00623320
005903f2  ba01000000            MOV EDX,0x1
005903f7  83e201                AND EDX,0x1
005903fa  740c                  JZ 0x00590408
005903fc  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
005903ff  50                    PUSH EAX
00590400  e8f8923800            CALL 0x009196fd
00590405  83c404                ADD ESP,0x4
00590408  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0059040b  894dd8                MOV dword ptr [EBP + -0x28],ECX
0059040e  eb07                  JMP 0x00590417
00590410  c745d800000000        MOV dword ptr [EBP + -0x28],0x0
00590417  c7450800000000        MOV dword ptr [EBP + 0x8],0x0
0059041e  8be5                  MOV ESP,EBP
00590420  5d                    POP EBP
00590421  c20800                RET 0x8
