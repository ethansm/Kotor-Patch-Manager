; FN 007b3560 FUN_007b3560
007b3560  55                    PUSH EBP
007b3561  8bec                  MOV EBP,ESP
007b3563  83ec18                SUB ESP,0x18
007b3566  894de8                MOV dword ptr [EBP + -0x18],ECX
007b3569  c705c0fca70001000000  MOV dword ptr [0x00a7fcc0],0x1
007b3573  833d8cb4a10000        CMP dword ptr [0x00a1b48c],0x0
007b357a  740d                  JZ 0x007b3589
007b357c  6a01                  PUSH 0x1
007b357e  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007b3584  e8a7c6f7ff            CALL 0x0072fc30
007b3589  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
007b358c  83b87402000000        CMP dword ptr [EAX + 0x274],0x0
007b3593  7436                  JZ 0x007b35cb
007b3595  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
007b3598  8b9174020000          MOV EDX,dword ptr [ECX + 0x274]
007b359e  8b4238                MOV EAX,dword ptr [EDX + 0x38]
007b35a1  8945f8                MOV dword ptr [EBP + -0x8],EAX
007b35a4  6a01                  PUSH 0x1
007b35a6  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
007b35a9  8b9174020000          MOV EDX,dword ptr [ECX + 0x274]
007b35af  8b421c                MOV EAX,dword ptr [EDX + 0x1c]
007b35b2  c1e802                SHR EAX,0x2
007b35b5  83e001                AND EAX,0x1
007b35b8  50                    PUSH EAX
007b35b9  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007b35bc  51                    PUSH ECX
007b35bd  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
007b35c0  8b8a74020000          MOV ECX,dword ptr [EDX + 0x274]
007b35c6  e8b504c6ff            CALL 0x00413a80
007b35cb  a1bcfca700            MOV EAX,[0x00a7fcbc]
007b35d0  8945fc                MOV dword ptr [EBP + -0x4],EAX
007b35d3  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007b35d9  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007b35dc  8955f4                MOV dword ptr [EBP + -0xc],EDX
007b35df  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007b35e2  e819c5f7ff            CALL 0x0072fb00
007b35e7  8945f0                MOV dword ptr [EBP + -0x10],EAX
007b35ea  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
007b35ed  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007b35f0  d1e9                  SHR ECX,0x1
007b35f2  83e101                AND ECX,0x1
007b35f5  7424                  JZ 0x007b361b
007b35f7  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
007b35fa  83ba9c00000002        CMP dword ptr [EDX + 0x9c],0x2
007b3601  7418                  JZ 0x007b361b
007b3603  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
007b3606  83b89c00000003        CMP dword ptr [EAX + 0x9c],0x3
007b360d  740c                  JZ 0x007b361b
007b360f  33c9                  XOR ECX,ECX
007b3611  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007b3615  0f94c1                SETZ CL
007b3618  894dfc                MOV dword ptr [EBP + -0x4],ECX
007b361b  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007b361f  750c                  JNZ 0x007b362d
007b3621  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
007b3624  83ba9c00000004        CMP dword ptr [EDX + 0x9c],0x4
007b362b  7518                  JNZ 0x007b3645
007b362d  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
007b3630  83b89c00000002        CMP dword ptr [EAX + 0x9c],0x2
007b3637  740c                  JZ 0x007b3645
007b3639  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
007b363c  83b99c00000003        CMP dword ptr [ECX + 0x9c],0x3
007b3643  7530                  JNZ 0x007b3675
007b3645  837d0800              CMP dword ptr [EBP + 0x8],0x0
007b3649  7518                  JNZ 0x007b3663
007b364b  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
007b364e  8b82b4020000          MOV EAX,dword ptr [EDX + 0x2b4]
007b3654  8945ec                MOV dword ptr [EBP + -0x14],EAX
007b3657  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
007b365a  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
007b365d  8991b8020000          MOV dword ptr [ECX + 0x2b8],EDX
007b3663  8b4508                MOV EAX,dword ptr [EBP + 0x8]
007b3666  50                    PUSH EAX
007b3667  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
007b366a  8b8974020000          MOV ECX,dword ptr [ECX + 0x274]
007b3670  e86bfbc5ff            CALL 0x004131e0
007b3675  8be5                  MOV ESP,EBP
007b3677  5d                    POP EBP
007b3678  c20400                RET 0x4
