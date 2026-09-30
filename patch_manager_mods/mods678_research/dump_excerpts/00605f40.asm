; FN 00605f40 FUN_00605f40
00605f40  55                    PUSH EBP
00605f41  8bec                  MOV EBP,ESP
00605f43  83ec18                SUB ESP,0x18
00605f46  894de8                MOV dword ptr [EBP + -0x18],ECX
00605f49  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00605f4c  8b4804                MOV ECX,dword ptr [EAX + 0x4]
00605f4f  894df4                MOV dword ptr [EBP + -0xc],ECX
00605f52  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00605f55  83ea01                SUB EDX,0x1
00605f58  8955fc                MOV dword ptr [EBP + -0x4],EDX
00605f5b  eb09                  JMP 0x00605f66
00605f5d  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00605f60  83e801                SUB EAX,0x1
00605f63  8945fc                MOV dword ptr [EBP + -0x4],EAX
00605f66  837dfc00              CMP dword ptr [EBP + -0x4],0x0
00605f6a  7c70                  JL 0x00605fdc
00605f6c  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605f6f  8b11                  MOV EDX,dword ptr [ECX]
00605f71  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00605f74  8d0c82                LEA ECX,[EDX + EAX*0x4]
00605f77  894df0                MOV dword ptr [EBP + -0x10],ECX
00605f7a  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
00605f80  8b4208                MOV EAX,dword ptr [EDX + 0x8]
00605f83  8945ec                MOV dword ptr [EBP + -0x14],EAX
00605f86  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00605f89  8b11                  MOV EDX,dword ptr [ECX]
00605f8b  52                    PUSH EDX
00605f8c  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00605f8f  e84c61f1ff            CALL 0x0051c0e0
00605f94  8945f8                MOV dword ptr [EBP + -0x8],EAX
00605f97  837df800              CMP dword ptr [EBP + -0x8],0x0
00605f9b  743d                  JZ 0x00605fda
00605f9d  33c0                  XOR EAX,EAX
00605f9f  83e001                AND EAX,0x1
00605fa2  c1e007                SHL EAX,0x7
00605fa5  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
00605fa8  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
00605fae  81e27fffffff          AND EDX,0xffffff7f
00605fb4  0bd0                  OR EDX,EAX
00605fb6  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00605fb9  8990c8020000          MOV dword ptr [EAX + 0x2c8],EDX
00605fbf  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
00605fc2  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00605fc5  83ea01                SUB EDX,0x1
00605fc8  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
00605fcb  895004                MOV dword ptr [EAX + 0x4],EDX
00605fce  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00605fd1  51                    PUSH ECX
00605fd2  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605fd5  e86658e9ff            CALL 0x0049b840
00605fda  eb81                  JMP 0x00605f5d
00605fdc  8be5                  MOV ESP,EBP
00605fde  5d                    POP EBP
00605fdf  c20400                RET 0x4
