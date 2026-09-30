; FN 007b3ee0 FUN_007b3ee0
007b3ee0  55                    PUSH EBP
007b3ee1  8bec                  MOV EBP,ESP
007b3ee3  83ec10                SUB ESP,0x10
007b3ee6  894df8                MOV dword ptr [EBP + -0x8],ECX
007b3ee9  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
007b3ef0  b801000000            MOV EAX,0x1
007b3ef5  85c0                  TEST EAX,EAX
007b3ef7  0f84d0000000          JZ 0x007b3fcd
007b3efd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
007b3f00  81e1ffffff0f          AND ECX,0xfffffff
007b3f06  894df4                MOV dword ptr [EBP + -0xc],ECX
007b3f09  817df4f2030000        CMP dword ptr [EBP + -0xc],0x3f2
007b3f10  7f3b                  JG 0x007b3f4d
007b3f12  817df4f2030000        CMP dword ptr [EBP + -0xc],0x3f2
007b3f19  7467                  JZ 0x007b3f82
007b3f1b  817df4ea030000        CMP dword ptr [EBP + -0xc],0x3ea
007b3f22  7f1e                  JG 0x007b3f42
007b3f24  817df4ea030000        CMP dword ptr [EBP + -0xc],0x3ea
007b3f2b  744c                  JZ 0x007b3f79
007b3f2d  837df4ff              CMP dword ptr [EBP + -0xc],-0x1
007b3f31  0f8481000000          JZ 0x007b3fb8
007b3f37  817df4e9030000        CMP dword ptr [EBP + -0xc],0x3e9
007b3f3e  746f                  JZ 0x007b3faf
007b3f40  eb7f                  JMP 0x007b3fc1
007b3f42  817df4eb030000        CMP dword ptr [EBP + -0xc],0x3eb
007b3f49  7425                  JZ 0x007b3f70
007b3f4b  eb74                  JMP 0x007b3fc1
007b3f4d  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
007b3f50  81eaf4030000          SUB EDX,0x3f4
007b3f56  8955f4                MOV dword ptr [EBP + -0xc],EDX
007b3f59  837df40e              CMP dword ptr [EBP + -0xc],0xe
007b3f5d  7762                  JA 0x007b3fc1
007b3f5f  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
007b3f62  0fb688b4407b00        MOVZX ECX,byte ptr [EAX + 0x7b40b4]
007b3f69  ff248da0407b00        JMP dword ptr [ECX*0x4 + 0x7b40a0]
007b3f70  c745fc33000000        MOV dword ptr [EBP + -0x4],0x33
007b3f77  eb4f                  JMP 0x007b3fc8
007b3f79  c745fc0b000000        MOV dword ptr [EBP + -0x4],0xb
007b3f80  eb46                  JMP 0x007b3fc8
007b3f82  c745fc17000000        MOV dword ptr [EBP + -0x4],0x17
007b3f89  eb3d                  JMP 0x007b3fc8
007b3f8b  c745fc19000000        MOV dword ptr [EBP + -0x4],0x19
007b3f92  eb34                  JMP 0x007b3fc8
007b3f94  c745fc07000000        MOV dword ptr [EBP + -0x4],0x7
007b3f9b  eb2b                  JMP 0x007b3fc8
007b3f9d  c745fc21000000        MOV dword ptr [EBP + -0x4],0x21
007b3fa4  eb22                  JMP 0x007b3fc8
007b3fa6  c745fc25000000        MOV dword ptr [EBP + -0x4],0x25
007b3fad  eb19                  JMP 0x007b3fc8
007b3faf  c745fc19000000        MOV dword ptr [EBP + -0x4],0x19
007b3fb6  eb10                  JMP 0x007b3fc8
007b3fb8  c745fc05000000        MOV dword ptr [EBP + -0x4],0x5
007b3fbf  eb07                  JMP 0x007b3fc8
007b3fc1  c745fc05000000        MOV dword ptr [EBP + -0x4],0x5
007b3fc8  e9ca000000            JMP 0x007b4097
007b3fcd  8b5508                MOV EDX,dword ptr [EBP + 0x8]
007b3fd0  81e2ffffff0f          AND EDX,0xfffffff
007b3fd6  8955f0                MOV dword ptr [EBP + -0x10],EDX
007b3fd9  817df0f2030000        CMP dword ptr [EBP + -0x10],0x3f2
007b3fe0  7f3b                  JG 0x007b401d
007b3fe2  817df0f2030000        CMP dword ptr [EBP + -0x10],0x3f2
007b3fe9  7466                  JZ 0x007b4051
007b3feb  817df0ea030000        CMP dword ptr [EBP + -0x10],0x3ea
007b3ff2  7f1e                  JG 0x007b4012
007b3ff4  817df0ea030000        CMP dword ptr [EBP + -0x10],0x3ea
007b3ffb  744b                  JZ 0x007b4048
007b3ffd  837df0ff              CMP dword ptr [EBP + -0x10],-0x1
007b4001  0f8480000000          JZ 0x007b4087
007b4007  817df0e9030000        CMP dword ptr [EBP + -0x10],0x3e9
007b400e  746e                  JZ 0x007b407e
007b4010  eb7e                  JMP 0x007b4090
007b4012  817df0eb030000        CMP dword ptr [EBP + -0x10],0x3eb
007b4019  7424                  JZ 0x007b403f
007b401b  eb73                  JMP 0x007b4090
007b401d  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
007b4020  2df4030000            SUB EAX,0x3f4
007b4025  8945f0                MOV dword ptr [EBP + -0x10],EAX
007b4028  837df00e              CMP dword ptr [EBP + -0x10],0xe
007b402c  7762                  JA 0x007b4090
007b402e  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
007b4031  0fb691d8407b00        MOVZX EDX,byte ptr [ECX + 0x7b40d8]
007b4038  ff2495c4407b00        JMP dword ptr [EDX*0x4 + 0x7b40c4]
007b403f  c745fc33000000        MOV dword ptr [EBP + -0x4],0x33
007b4046  eb4f                  JMP 0x007b4097
007b4048  c745fc0b000000        MOV dword ptr [EBP + -0x4],0xb
007b404f  eb46                  JMP 0x007b4097
007b4051  c745fc2b000000        MOV dword ptr [EBP + -0x4],0x2b
007b4058  eb3d                  JMP 0x007b4097
007b405a  c745fc1b000000        MOV dword ptr [EBP + -0x4],0x1b
007b4061  eb34                  JMP 0x007b4097
007b4063  c745fc09000000        MOV dword ptr [EBP + -0x4],0x9
007b406a  eb2b                  JMP 0x007b4097
007b406c  c745fc23000000        MOV dword ptr [EBP + -0x4],0x23
007b4073  eb22                  JMP 0x007b4097
007b4075  c745fc27000000        MOV dword ptr [EBP + -0x4],0x27
007b407c  eb19                  JMP 0x007b4097
007b407e  c745fc1b000000        MOV dword ptr [EBP + -0x4],0x1b
007b4085  eb10                  JMP 0x007b4097
007b4087  c745fc05000000        MOV dword ptr [EBP + -0x4],0x5
007b408e  eb07                  JMP 0x007b4097
007b4090  c745fc05000000        MOV dword ptr [EBP + -0x4],0x5
007b4097  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007b409a  8be5                  MOV ESP,EBP
007b409c  5d                    POP EBP
007b409d  c20800                RET 0x8
