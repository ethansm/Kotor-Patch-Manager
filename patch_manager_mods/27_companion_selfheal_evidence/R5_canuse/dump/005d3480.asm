; FN 005d3480 FUN_005d3480
005d3480  55                    PUSH EBP
005d3481  8bec                  MOV EBP,ESP
005d3483  83ec38                SUB ESP,0x38
005d3486  894dd0                MOV dword ptr [EBP + -0x30],ECX
005d3489  6a00                  PUSH 0x0
005d348b  6a2b                  PUSH 0x2b
005d348d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3490  e8cb0a0300            CALL 0x00603f60
005d3495  85c0                  TEST EAX,EAX
005d3497  750a                  JNZ 0x005d34a3
005d3499  b801000000            MOV EAX,0x1
005d349e  e983010000            JMP 0x005d3626
005d34a3  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
005d34a6  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d34ac  894df0                MOV dword ptr [EBP + -0x10],ECX
005d34af  6a5d                  PUSH 0x5d
005d34b1  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
005d34b4  e8374f0e00            CALL 0x006b83f0
005d34b9  85c0                  TEST EAX,EAX
005d34bb  740a                  JZ 0x005d34c7
005d34bd  b801000000            MOV EAX,0x1
005d34c2  e95f010000            JMP 0x005d3626
005d34c7  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
005d34ca  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005d34d0  8945ec                MOV dword ptr [EBP + -0x14],EAX
005d34d3  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
005d34d6  668b918a010000        MOV DX,word ptr [ECX + 0x18a]
005d34dd  668955f8              MOV word ptr [EBP + -0x8],DX
005d34e1  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005d34e8  eb09                  JMP 0x005d34f3
005d34ea  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d34ed  83c001                ADD EAX,0x1
005d34f0  8945fc                MOV dword ptr [EBP + -0x4],EAX
005d34f3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d34f6  8b918c020000          MOV EDX,dword ptr [ECX + 0x28c]
005d34fc  8955e8                MOV dword ptr [EBP + -0x18],EDX
005d34ff  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d3502  3b45e8                CMP EAX,dword ptr [EBP + -0x18]
005d3505  0f8d19010000          JGE 0x005d3624
005d350b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d350e  51                    PUSH ECX
005d350f  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3512  e8890d0300            CALL 0x006042a0
005d3517  8945e4                MOV dword ptr [EBP + -0x1c],EAX
005d351a  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
005d351d  8a4211                MOV AL,byte ptr [EDX + 0x11]
005d3520  8845e3                MOV byte ptr [EBP + -0x1d],AL
005d3523  0fb64de3              MOVZX ECX,byte ptr [EBP + -0x1d]
005d3527  81f9ff000000          CMP ECX,0xff
005d352d  741f                  JZ 0x005d354e
005d352f  0fb64de3              MOVZX ECX,byte ptr [EBP + -0x1d]
005d3533  ba01000000            MOV EDX,0x1
005d3538  d3e2                  SHL EDX,CL
005d353a  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d353d  2390d4020000          AND EDX,dword ptr [EAX + 0x2d4]
005d3543  7509                  JNZ 0x005d354e
005d3545  c745cc00000000        MOV dword ptr [EBP + -0x34],0x0
005d354c  eb07                  JMP 0x005d3555
005d354e  c745cc01000000        MOV dword ptr [EBP + -0x34],0x1
005d3555  837dcc00              CMP dword ptr [EBP + -0x34],0x0
005d3559  0f84c0000000          JZ 0x005d361f
005d355f  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d3562  51                    PUSH ECX
005d3563  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3566  e8350d0300            CALL 0x006042a0
005d356b  8945dc                MOV dword ptr [EBP + -0x24],EAX
005d356e  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
005d3571  668b02                MOV AX,word ptr [EDX]
005d3574  668945da              MOV word ptr [EBP + -0x26],AX
005d3578  0fb74dda              MOVZX ECX,word ptr [EBP + -0x26]
005d357c  83f92b                CMP ECX,0x2b
005d357f  0f859a000000          JNZ 0x005d361f
005d3585  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d3588  52                    PUSH EDX
005d3589  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d358c  e80f0d0300            CALL 0x006042a0
005d3591  8945d4                MOV dword ptr [EBP + -0x2c],EAX
005d3594  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
005d3597  668b4802              MOV CX,word ptr [EAX + 0x2]
005d359b  66894df4              MOV word ptr [EBP + -0xc],CX
005d359f  0fb755f4              MOVZX EDX,word ptr [EBP + -0xc]
005d35a3  8955c8                MOV dword ptr [EBP + -0x38],EDX
005d35a6  837dc805              CMP dword ptr [EBP + -0x38],0x5
005d35aa  7773                  JA 0x005d361f
005d35ac  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
005d35af  ff24852c365d00        JMP dword ptr [EAX*0x4 + 0x5d362c]
005d35b6  b801000000            MOV EAX,0x1
005d35bb  eb69                  JMP 0x005d3626
005d35bf  0fbf4df8              MOVSX ECX,word ptr [EBP + -0x8]
005d35c3  83f93c                CMP ECX,0x3c
005d35c6  7d10                  JGE 0x005d35d8
005d35c8  0fbf55f8              MOVSX EDX,word ptr [EBP + -0x8]
005d35cc  83fa28                CMP EDX,0x28
005d35cf  7e07                  JLE 0x005d35d8
005d35d1  b801000000            MOV EAX,0x1
005d35d6  eb4e                  JMP 0x005d3626
005d35d8  eb45                  JMP 0x005d361f
005d35da  0fbf45f8              MOVSX EAX,word ptr [EBP + -0x8]
005d35de  83f83c                CMP EAX,0x3c
005d35e1  7c07                  JL 0x005d35ea
005d35e3  b801000000            MOV EAX,0x1
005d35e8  eb3c                  JMP 0x005d3626
005d35ea  eb33                  JMP 0x005d361f
005d35ec  0fbf4df8              MOVSX ECX,word ptr [EBP + -0x8]
005d35f0  83f928                CMP ECX,0x28
005d35f3  7f07                  JG 0x005d35fc
005d35f5  b801000000            MOV EAX,0x1
005d35fa  eb2a                  JMP 0x005d3626
005d35fc  eb21                  JMP 0x005d361f
005d35fe  0fbf55f8              MOVSX EDX,word ptr [EBP + -0x8]
005d3602  85d2                  TEST EDX,EDX
005d3604  7507                  JNZ 0x005d360d
005d3606  b801000000            MOV EAX,0x1
005d360b  eb19                  JMP 0x005d3626
005d360d  eb10                  JMP 0x005d361f
005d360f  0fbf45f8              MOVSX EAX,word ptr [EBP + -0x8]
005d3613  83f864                CMP EAX,0x64
005d3616  7507                  JNZ 0x005d361f
005d3618  b801000000            MOV EAX,0x1
005d361d  eb07                  JMP 0x005d3626
005d361f  e9c6feffff            JMP 0x005d34ea
005d3624  33c0                  XOR EAX,EAX
005d3626  8be5                  MOV ESP,EBP
005d3628  5d                    POP EBP
005d3629  c20400                RET 0x4
