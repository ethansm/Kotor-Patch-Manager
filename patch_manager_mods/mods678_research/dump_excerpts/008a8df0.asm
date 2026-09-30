; FN 008a8df0 FUN_008a8df0
008a8df0  55                    PUSH EBP
008a8df1  8bec                  MOV EBP,ESP
008a8df3  81ec80000000          SUB ESP,0x80
008a8df9  894d94                MOV dword ptr [EBP + -0x6c],ECX
008a8dfc  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a8e03  837d0800              CMP dword ptr [EBP + 0x8],0x0
008a8e07  7507                  JNZ 0x008a8e10
008a8e09  33c0                  XOR EAX,EAX
008a8e0b  e96b030000            JMP 0x008a917b
008a8e10  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008a8e15  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008a8e18  894de8                MOV dword ptr [EBP + -0x18],ECX
008a8e1b  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008a8e1e  e82d69e9ff            CALL 0x0073f750
008a8e23  8945e4                MOV dword ptr [EBP + -0x1c],EAX
008a8e26  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
008a8e29  8a82c10f0000          MOV AL,byte ptr [EDX + 0xfc1]
008a8e2f  8845e3                MOV byte ptr [EBP + -0x1d],AL
008a8e32  0fb64de3              MOVZX ECX,byte ptr [EBP + -0x1d]
008a8e36  894d90                MOV dword ptr [EBP + -0x70],ECX
008a8e39  837d900b              CMP dword ptr [EBP + -0x70],0xb
008a8e3d  0f8735030000          JA 0x008a9178
008a8e43  8b5590                MOV EDX,dword ptr [EBP + -0x70]
008a8e46  ff249584918a00        JMP dword ptr [EDX*0x4 + 0x8a9184]
008a8e4d  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
008a8e54  e91f030000            JMP 0x008a9178
008a8e59  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a8e5c  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
008a8e62  c1e907                SHR ECX,0x7
008a8e65  83e101                AND ECX,0x1
008a8e68  894dfc                MOV dword ptr [EBP + -0x4],ECX
008a8e6b  e908030000            JMP 0x008a9178
008a8e70  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008a8e73  8b820c010000          MOV EAX,dword ptr [EDX + 0x10c]
008a8e79  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a8e7c  e9f7020000            JMP 0x008a9178
008a8e81  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8e84  e8a7dfe2ff            CALL 0x006d6e30
008a8e89  85c0                  TEST EAX,EAX
008a8e8b  741a                  JZ 0x008a8ea7
008a8e8d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8e90  e89bdfe2ff            CALL 0x006d6e30
008a8e95  8945dc                MOV dword ptr [EBP + -0x24],EAX
008a8e98  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008a8e9b  33d2                  XOR EDX,EDX
008a8e9d  83790400              CMP dword ptr [ECX + 0x4],0x0
008a8ea1  0f95c2                SETNZ DL
008a8ea4  8955fc                MOV dword ptr [EBP + -0x4],EDX
008a8ea7  e9cc020000            JMP 0x008a9178
008a8eac  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8eaf  e87cdfe2ff            CALL 0x006d6e30
008a8eb4  85c0                  TEST EAX,EAX
008a8eb6  7443                  JZ 0x008a8efb
008a8eb8  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a8ebb  8b880c010000          MOV ECX,dword ptr [EAX + 0x10c]
008a8ec1  894dd8                MOV dword ptr [EBP + -0x28],ECX
008a8ec4  837dd800              CMP dword ptr [EBP + -0x28],0x0
008a8ec8  7524                  JNZ 0x008a8eee
008a8eca  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8ecd  e85edfe2ff            CALL 0x006d6e30
008a8ed2  8945d4                MOV dword ptr [EBP + -0x2c],EAX
008a8ed5  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
008a8ed8  33c0                  XOR EAX,EAX
008a8eda  837a0400              CMP dword ptr [EDX + 0x4],0x0
008a8ede  0f95c0                SETNZ AL
008a8ee1  85c0                  TEST EAX,EAX
008a8ee3  7509                  JNZ 0x008a8eee
008a8ee5  c7458c01000000        MOV dword ptr [EBP + -0x74],0x1
008a8eec  eb07                  JMP 0x008a8ef5
008a8eee  c7458c00000000        MOV dword ptr [EBP + -0x74],0x0
008a8ef5  8b4d8c                MOV ECX,dword ptr [EBP + -0x74]
008a8ef8  894dfc                MOV dword ptr [EBP + -0x4],ECX
008a8efb  e978020000            JMP 0x008a9178
008a8f00  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
008a8f03  e8881fceff            CALL 0x0058ae90
008a8f08  85c0                  TEST EAX,EAX
008a8f0a  744a                  JZ 0x008a8f56
008a8f0c  68ff000000            PUSH 0xff
008a8f11  8b5510                MOV EDX,dword ptr [EBP + 0x10]
008a8f14  52                    PUSH EDX
008a8f15  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a8f18  50                    PUSH EAX
008a8f19  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
008a8f1c  e86f1fceff            CALL 0x0058ae90
008a8f21  8bc8                  MOV ECX,EAX
008a8f23  e84827edff            CALL 0x0077b670
008a8f28  85c0                  TEST EAX,EAX
008a8f2a  741d                  JZ 0x008a8f49
008a8f2c  6a01                  PUSH 0x1
008a8f2e  6a00                  PUSH 0x0
008a8f30  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8f33  51                    PUSH ECX
008a8f34  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
008a8f37  e844a0d2ff            CALL 0x005d2f80
008a8f3c  85c0                  TEST EAX,EAX
008a8f3e  7409                  JZ 0x008a8f49
008a8f40  c7458801000000        MOV dword ptr [EBP + -0x78],0x1
008a8f47  eb07                  JMP 0x008a8f50
008a8f49  c7458800000000        MOV dword ptr [EBP + -0x78],0x0
008a8f50  8b5588                MOV EDX,dword ptr [EBP + -0x78]
008a8f53  8955fc                MOV dword ptr [EBP + -0x4],EDX
008a8f56  e91d020000            JMP 0x008a9178
008a8f5b  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8f5e  e8cddee2ff            CALL 0x006d6e30
008a8f63  85c0                  TEST EAX,EAX
008a8f65  7426                  JZ 0x008a8f8d
008a8f67  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8f6a  e8c1dee2ff            CALL 0x006d6e30
008a8f6f  8945d0                MOV dword ptr [EBP + -0x30],EAX
008a8f72  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
008a8f75  8a88ac000000          MOV CL,byte ptr [EAX + 0xac]
008a8f7b  884dcf                MOV byte ptr [EBP + -0x31],CL
008a8f7e  0fb655cf              MOVZX EDX,byte ptr [EBP + -0x31]
008a8f82  33c0                  XOR EAX,EAX
008a8f84  83fa18                CMP EDX,0x18
008a8f87  0f94c0                SETZ AL
008a8f8a  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a8f8d  e9e6010000            JMP 0x008a9178
008a8f92  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8f95  e896dee2ff            CALL 0x006d6e30
008a8f9a  85c0                  TEST EAX,EAX
008a8f9c  746c                  JZ 0x008a900a
008a8f9e  c745f830000000        MOV dword ptr [EBP + -0x8],0x30
008a8fa5  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8fa8  e883dee2ff            CALL 0x006d6e30
008a8fad  8945c8                MOV dword ptr [EBP + -0x38],EAX
008a8fb0  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008a8fb3  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008a8fb6  2355f8                AND EDX,dword ptr [EBP + -0x8]
008a8fb9  f7da                  NEG EDX
008a8fbb  1bd2                  SBB EDX,EDX
008a8fbd  f7da                  NEG EDX
008a8fbf  8955fc                MOV dword ptr [EBP + -0x4],EDX
008a8fc2  7546                  JNZ 0x008a900a
008a8fc4  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8fc7  e864dee2ff            CALL 0x006d6e30
008a8fcc  8945c4                MOV dword ptr [EBP + -0x3c],EAX
008a8fcf  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
008a8fd2  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008a8fd5  81e180010000          AND ECX,0x180
008a8fdb  f7d9                  NEG ECX
008a8fdd  1bc9                  SBB ECX,ECX
008a8fdf  f7d9                  NEG ECX
008a8fe1  7427                  JZ 0x008a900a
008a8fe3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a8fe6  e845dee2ff            CALL 0x006d6e30
008a8feb  8945c0                MOV dword ptr [EBP + -0x40],EAX
008a8fee  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
008a8ff1  8a82b4000000          MOV AL,byte ptr [EDX + 0xb4]
008a8ff7  8845bf                MOV byte ptr [EBP + -0x41],AL
008a8ffa  0fb64dbf              MOVZX ECX,byte ptr [EBP + -0x41]
008a8ffe  83f902                CMP ECX,0x2
008a9001  7507                  JNZ 0x008a900a
008a9003  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
008a900a  e969010000            JMP 0x008a9178
008a900f  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a9012  e819dee2ff            CALL 0x006d6e30
008a9017  85c0                  TEST EAX,EAX
008a9019  746c                  JZ 0x008a9087
008a901b  c745f48b070000        MOV dword ptr [EBP + -0xc],0x78b
008a9022  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a9025  e806dee2ff            CALL 0x006d6e30
008a902a  8945b8                MOV dword ptr [EBP + -0x48],EAX
008a902d  8b55b8                MOV EDX,dword ptr [EBP + -0x48]
008a9030  8b4204                MOV EAX,dword ptr [EDX + 0x4]
008a9033  2345f4                AND EAX,dword ptr [EBP + -0xc]
008a9036  f7d8                  NEG EAX
008a9038  1bc0                  SBB EAX,EAX
008a903a  f7d8                  NEG EAX
008a903c  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a903f  7446                  JZ 0x008a9087
008a9041  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a9044  e8e7dde2ff            CALL 0x006d6e30
008a9049  8945b4                MOV dword ptr [EBP + -0x4c],EAX
008a904c  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
008a904f  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008a9052  81e280010000          AND EDX,0x180
008a9058  f7da                  NEG EDX
008a905a  1bd2                  SBB EDX,EDX
008a905c  f7da                  NEG EDX
008a905e  7427                  JZ 0x008a9087
008a9060  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a9063  e8c8dde2ff            CALL 0x006d6e30
008a9068  8945b0                MOV dword ptr [EBP + -0x50],EAX
008a906b  8b45b0                MOV EAX,dword ptr [EBP + -0x50]
008a906e  8a88b4000000          MOV CL,byte ptr [EAX + 0xb4]
008a9074  884daf                MOV byte ptr [EBP + -0x51],CL
008a9077  0fb655af              MOVZX EDX,byte ptr [EBP + -0x51]
008a907b  83fa02                CMP EDX,0x2
008a907e  7507                  JNZ 0x008a9087
008a9080  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a9087  e9ec000000            JMP 0x008a9178
008a908c  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
008a908f  e8fc1dceff            CALL 0x0058ae90
008a9094  85c0                  TEST EAX,EAX
008a9096  7433                  JZ 0x008a90cb
008a9098  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a909b  8b480c                MOV ECX,dword ptr [EAX + 0xc]
008a909e  894da8                MOV dword ptr [EBP + -0x58],ECX
008a90a1  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008a90a4  8955f0                MOV dword ptr [EBP + -0x10],EDX
008a90a7  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
008a90aa  894584                MOV dword ptr [EBP + -0x7c],EAX
008a90ad  837d8435              CMP dword ptr [EBP + -0x7c],0x35
008a90b1  7c11                  JL 0x008a90c4
008a90b3  837d8438              CMP dword ptr [EBP + -0x7c],0x38
008a90b7  7e02                  JLE 0x008a90bb
008a90b9  eb09                  JMP 0x008a90c4
008a90bb  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
008a90c2  eb07                  JMP 0x008a90cb
008a90c4  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a90cb  e9a8000000            JMP 0x008a9178
008a90d0  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
008a90d7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a90da  e851dde2ff            CALL 0x006d6e30
008a90df  85c0                  TEST EAX,EAX
008a90e1  7449                  JZ 0x008a912c
008a90e3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a90e6  e845dde2ff            CALL 0x006d6e30
008a90eb  8945a4                MOV dword ptr [EBP + -0x5c],EAX
008a90ee  8b4da4                MOV ECX,dword ptr [EBP + -0x5c]
008a90f1  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
008a90f7  8855a3                MOV byte ptr [EBP + -0x5d],DL
008a90fa  0fb645a3              MOVZX EAX,byte ptr [EBP + -0x5d]
008a90fe  83f818                CMP EAX,0x18
008a9101  7507                  JNZ 0x008a910a
008a9103  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a910a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
008a910d  e81edde2ff            CALL 0x006d6e30
008a9112  89459c                MOV dword ptr [EBP + -0x64],EAX
008a9115  8b4d9c                MOV ECX,dword ptr [EBP + -0x64]
008a9118  33d2                  XOR EDX,EDX
008a911a  83790400              CMP dword ptr [ECX + 0x4],0x0
008a911e  0f95c2                SETNZ DL
008a9121  85d2                  TEST EDX,EDX
008a9123  7407                  JZ 0x008a912c
008a9125  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a912c  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
008a912f  e85c1dceff            CALL 0x0058ae90
008a9134  85c0                  TEST EAX,EAX
008a9136  7432                  JZ 0x008a916a
008a9138  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a913b  8b480c                MOV ECX,dword ptr [EAX + 0xc]
008a913e  894d98                MOV dword ptr [EBP + -0x68],ECX
008a9141  8b5598                MOV EDX,dword ptr [EBP + -0x68]
008a9144  8955ec                MOV dword ptr [EBP + -0x14],EDX
008a9147  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008a914a  894580                MOV dword ptr [EBP + -0x80],EAX
008a914d  837d8035              CMP dword ptr [EBP + -0x80],0x35
008a9151  7c11                  JL 0x008a9164
008a9153  837d8038              CMP dword ptr [EBP + -0x80],0x38
008a9157  7e02                  JLE 0x008a915b
008a9159  eb09                  JMP 0x008a9164
008a915b  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a9162  eb06                  JMP 0x008a916a
008a9164  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008a9167  894dfc                MOV dword ptr [EBP + -0x4],ECX
008a916a  eb0c                  JMP 0x008a9178
008a916c  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008a916f  8b820c010000          MOV EAX,dword ptr [EDX + 0x10c]
008a9175  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a9178  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
008a917b  8be5                  MOV ESP,EBP
008a917d  5d                    POP EBP
008a917e  c20c00                RET 0xc
