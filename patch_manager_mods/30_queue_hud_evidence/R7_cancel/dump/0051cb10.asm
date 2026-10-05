; FN 0051cb10 FUN_0051cb10
0051cb10  55                    PUSH EBP
0051cb11  8bec                  MOV EBP,ESP
0051cb13  83ec10                SUB ESP,0x10
0051cb16  894df0                MOV dword ptr [EBP + -0x10],ECX
0051cb19  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0051cb1c  50                    PUSH EAX
0051cb1d  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0051cb20  e8dbf5ffff            CALL 0x0051c100
0051cb25  8945fc                MOV dword ptr [EBP + -0x4],EAX
0051cb28  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0051cb2c  746b                  JZ 0x0051cb99
0051cb2e  6a01                  PUSH 0x1
0051cb30  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051cb33  e848450200            CALL 0x00541080
0051cb38  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051cb3b  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
0051cb41  8955f8                MOV dword ptr [EBP + -0x8],EDX
0051cb44  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0051cb47  e8e4380700            CALL 0x00590430
0051cb4c  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0051cb4f  8b88dc100000          MOV ECX,dword ptr [EAX + 0x10dc]
0051cb55  894df4                MOV dword ptr [EBP + -0xc],ECX
0051cb58  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
0051cb5b  c782200b00000000007f  MOV dword ptr [EDX + 0xb20],0x7f000000
0051cb65  680000007f            PUSH 0x7f000000
0051cb6a  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051cb6d  e85e180600            CALL 0x0057e3d0
0051cb72  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0051cb75  c780480500000000007f  MOV dword ptr [EAX + 0x548],0x7f000000
0051cb7f  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0051cb82  c781680500000000007f  MOV dword ptr [ECX + 0x568],0x7f000000
0051cb8c  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0051cb8f  c7826c0500000000007f  MOV dword ptr [EDX + 0x56c],0x7f000000
0051cb99  8be5                  MOV ESP,EBP
0051cb9b  5d                    POP EBP
0051cb9c  c20400                RET 0x4
