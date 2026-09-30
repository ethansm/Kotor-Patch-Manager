; FN 00605e80 FUN_00605e80
00605e80  55                    PUSH EBP
00605e81  8bec                  MOV EBP,ESP
00605e83  83ec10                SUB ESP,0x10
00605e86  894df0                MOV dword ptr [EBP + -0x10],ECX
00605e89  837d0800              CMP dword ptr [EBP + 0x8],0x0
00605e8d  0f8499000000          JZ 0x00605f2c
00605e93  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00605e9a  eb09                  JMP 0x00605ea5
00605e9c  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00605e9f  83c001                ADD EAX,0x1
00605ea2  8945fc                MOV dword ptr [EBP + -0x4],EAX
00605ea5  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00605ea8  8b5110                MOV EDX,dword ptr [ECX + 0x10]
00605eab  8955f8                MOV dword ptr [EBP + -0x8],EDX
00605eae  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00605eb1  3b45f8                CMP EAX,dword ptr [EBP + -0x8]
00605eb4  7d76                  JGE 0x00605f2c
00605eb6  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605eb9  8b5114                MOV EDX,dword ptr [ECX + 0x14]
00605ebc  8955f4                MOV dword ptr [EBP + -0xc],EDX
00605ebf  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00605ec2  8b480c                MOV ECX,dword ptr [EAX + 0xc]
00605ec5  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00605ec8  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00605ecb  3b0491                CMP EAX,dword ptr [ECX + EDX*0x4]
00605ece  7557                  JNZ 0x00605f27
00605ed0  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00605ed3  51                    PUSH ECX
00605ed4  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00605ed7  83c10c                ADD ECX,0xc
00605eda  e86159e9ff            CALL 0x0049b840
00605edf  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00605ee2  8b82c8020000          MOV EAX,dword ptr [EDX + 0x2c8]
00605ee8  c1e807                SHR EAX,0x7
00605eeb  83e001                AND EAX,0x1
00605eee  7430                  JZ 0x00605f20
00605ef0  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00605ef3  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00605ef6  83ea01                SUB EDX,0x1
00605ef9  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00605efc  895004                MOV dword ptr [EAX + 0x4],EDX
00605eff  33c9                  XOR ECX,ECX
00605f01  83e101                AND ECX,0x1
00605f04  c1e107                SHL ECX,0x7
00605f07  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00605f0a  8b82c8020000          MOV EAX,dword ptr [EDX + 0x2c8]
00605f10  257fffffff            AND EAX,0xffffff7f
00605f15  0bc1                  OR EAX,ECX
00605f17  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00605f1a  8981c8020000          MOV dword ptr [ECX + 0x2c8],EAX
00605f20  b801000000            MOV EAX,0x1
00605f25  eb07                  JMP 0x00605f2e
00605f27  e970ffffff            JMP 0x00605e9c
00605f2c  33c0                  XOR EAX,EAX
00605f2e  8be5                  MOV ESP,EBP
00605f30  5d                    POP EBP
00605f31  c20400                RET 0x4
