; FN 006dac70 FUN_006dac70
006dac70  55                    PUSH EBP
006dac71  8bec                  MOV EBP,ESP
006dac73  83ec0c                SUB ESP,0xc
006dac76  894df4                MOV dword ptr [EBP + -0xc],ECX
006dac79  837d0c00              CMP dword ptr [EBP + 0xc],0x0
006dac7d  7415                  JZ 0x006dac94
006dac7f  6a04                  PUSH 0x4
006dac81  8d4508                LEA EAX,[EBP + 0x8]
006dac84  50                    PUSH EAX
006dac85  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
006dac88  83c138                ADD ECX,0x38
006dac8b  51                    PUSH ECX
006dac8c  e80f2f2400            CALL 0x0091dba0
006dac91  83c40c                ADD ESP,0xc
006dac94  6a00                  PUSH 0x0
006dac96  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006dac99  52                    PUSH EDX
006dac9a  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006dac9d  e8ce26eaff            CALL 0x0057d370
006daca2  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006daca5  8945fc                MOV dword ptr [EBP + -0x4],EAX
006daca8  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006dacab  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
006dacae  899188000000          MOV dword ptr [ECX + 0x88],EDX
006dacb4  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006dacb7  50                    PUSH EAX
006dacb8  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006dacbb  e81037eaff            CALL 0x0057e3d0
006dacc0  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006dacc3  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006dacc9  8955f8                MOV dword ptr [EBP + -0x8],EDX
006daccc  8b4508                MOV EAX,dword ptr [EBP + 0x8]
006daccf  50                    PUSH EAX
006dacd0  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
006dacd3  e80875ebff            CALL 0x005921e0
006dacd8  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006dacdb  c7812c05000001000000  MOV dword ptr [ECX + 0x52c],0x1
006dace5  8be5                  MOV ESP,EBP
006dace7  5d                    POP EBP
006dace8  c20800                RET 0x8
