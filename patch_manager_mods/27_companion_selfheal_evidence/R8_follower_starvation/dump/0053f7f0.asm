; FN 0053f7f0 FUN_0053f7f0
0053f7f0  55                    PUSH EBP
0053f7f1  8bec                  MOV EBP,ESP
0053f7f3  6aff                  PUSH -0x1
0053f7f5  68bb219800            PUSH 0x9821bb
0053f7fa  64a100000000          MOV EAX,FS:[0x0]
0053f800  50                    PUSH EAX
0053f801  83ec18                SUB ESP,0x18
0053f804  a1201fa100            MOV EAX,[0x00a11f20]
0053f809  33c5                  XOR EAX,EBP
0053f80b  50                    PUSH EAX
0053f80c  8d45f4                LEA EAX,[EBP + -0xc]
0053f80f  64a300000000          MOV FS:[0x0],EAX
0053f815  894de0                MOV dword ptr [EBP + -0x20],ECX
0053f818  6a74                  PUSH 0x74
0053f81a  e8049f3d00            CALL 0x00919723
0053f81f  83c404                ADD ESP,0x4
0053f822  8945e8                MOV dword ptr [EBP + -0x18],EAX
0053f825  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
0053f82c  837de800              CMP dword ptr [EBP + -0x18],0x0
0053f830  740d                  JZ 0x0053f83f
0053f832  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
0053f835  e8b6eeffff            CALL 0x0053e6f0
0053f83a  8945dc                MOV dword ptr [EBP + -0x24],EAX
0053f83d  eb07                  JMP 0x0053f846
0053f83f  c745dc00000000        MOV dword ptr [EBP + -0x24],0x0
0053f846  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
0053f849  8945ec                MOV dword ptr [EBP + -0x14],EAX
0053f84c  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
0053f853  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0053f856  894df0                MOV dword ptr [EBP + -0x10],ECX
0053f859  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f85c  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0053f85f  8902                  MOV dword ptr [EDX],EAX
0053f861  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0053f864  c7417001000000        MOV dword ptr [ECX + 0x70],0x1
0053f86b  0fb7550c              MOVZX EDX,word ptr [EBP + 0xc]
0053f86f  81faffff0000          CMP EDX,0xffff
0053f875  7511                  JNZ 0x0053f888
0053f877  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f87a  e821ffffff            CALL 0x0053f7a0
0053f87f  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0053f882  6689416c              MOV word ptr [ECX + 0x6c],AX
0053f886  eb2f                  JMP 0x0053f8b7
0053f888  0fb7550c              MOVZX EDX,word ptr [EBP + 0xc]
0053f88c  81fafeff0000          CMP EDX,0xfffe
0053f892  7518                  JNZ 0x0053f8ac
0053f894  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
0053f897  668b4816              MOV CX,word ptr [EAX + 0x16]
0053f89b  66894de6              MOV word ptr [EBP + -0x1a],CX
0053f89f  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f8a2  668b45e6              MOV AX,word ptr [EBP + -0x1a]
0053f8a6  6689426c              MOV word ptr [EDX + 0x6c],AX
0053f8aa  eb0b                  JMP 0x0053f8b7
0053f8ac  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0053f8af  668b550c              MOV DX,word ptr [EBP + 0xc]
0053f8b3  6689516c              MOV word ptr [ECX + 0x6c],DX
0053f8b7  837d1000              CMP dword ptr [EBP + 0x10],0x0
0053f8bb  0f8486010000          JZ 0x0053fa47
0053f8c1  8b4514                MOV EAX,dword ptr [EBP + 0x14]
0053f8c4  50                    PUSH EAX
0053f8c5  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
0053f8c8  51                    PUSH ECX
0053f8c9  6a00                  PUSH 0x0
0053f8cb  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f8ce  52                    PUSH EDX
0053f8cf  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f8d2  e8a9010000            CALL 0x0053fa80
0053f8d7  837d1800              CMP dword ptr [EBP + 0x18],0x0
0053f8db  0f8466010000          JZ 0x0053fa47
0053f8e1  8b451c                MOV EAX,dword ptr [EBP + 0x1c]
0053f8e4  50                    PUSH EAX
0053f8e5  8b4d18                MOV ECX,dword ptr [EBP + 0x18]
0053f8e8  51                    PUSH ECX
0053f8e9  6a01                  PUSH 0x1
0053f8eb  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f8ee  52                    PUSH EDX
0053f8ef  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f8f2  e889010000            CALL 0x0053fa80
0053f8f7  837d2000              CMP dword ptr [EBP + 0x20],0x0
0053f8fb  0f8446010000          JZ 0x0053fa47
0053f901  8b4524                MOV EAX,dword ptr [EBP + 0x24]
0053f904  50                    PUSH EAX
0053f905  8b4d20                MOV ECX,dword ptr [EBP + 0x20]
0053f908  51                    PUSH ECX
0053f909  6a02                  PUSH 0x2
0053f90b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f90e  52                    PUSH EDX
0053f90f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f912  e869010000            CALL 0x0053fa80
0053f917  837d2800              CMP dword ptr [EBP + 0x28],0x0
0053f91b  0f8426010000          JZ 0x0053fa47
0053f921  8b452c                MOV EAX,dword ptr [EBP + 0x2c]
0053f924  50                    PUSH EAX
0053f925  8b4d28                MOV ECX,dword ptr [EBP + 0x28]
0053f928  51                    PUSH ECX
0053f929  6a03                  PUSH 0x3
0053f92b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f92e  52                    PUSH EDX
0053f92f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f932  e849010000            CALL 0x0053fa80
0053f937  837d3000              CMP dword ptr [EBP + 0x30],0x0
0053f93b  0f8406010000          JZ 0x0053fa47
0053f941  8b4534                MOV EAX,dword ptr [EBP + 0x34]
0053f944  50                    PUSH EAX
0053f945  8b4d30                MOV ECX,dword ptr [EBP + 0x30]
0053f948  51                    PUSH ECX
0053f949  6a04                  PUSH 0x4
0053f94b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f94e  52                    PUSH EDX
0053f94f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f952  e829010000            CALL 0x0053fa80
0053f957  837d3800              CMP dword ptr [EBP + 0x38],0x0
0053f95b  0f84e6000000          JZ 0x0053fa47
0053f961  8b453c                MOV EAX,dword ptr [EBP + 0x3c]
0053f964  50                    PUSH EAX
0053f965  8b4d38                MOV ECX,dword ptr [EBP + 0x38]
0053f968  51                    PUSH ECX
0053f969  6a05                  PUSH 0x5
0053f96b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f96e  52                    PUSH EDX
0053f96f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f972  e809010000            CALL 0x0053fa80
0053f977  837d4000              CMP dword ptr [EBP + 0x40],0x0
0053f97b  0f84c6000000          JZ 0x0053fa47
0053f981  8b4544                MOV EAX,dword ptr [EBP + 0x44]
0053f984  50                    PUSH EAX
0053f985  8b4d40                MOV ECX,dword ptr [EBP + 0x40]
0053f988  51                    PUSH ECX
0053f989  6a06                  PUSH 0x6
0053f98b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f98e  52                    PUSH EDX
0053f98f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f992  e8e9000000            CALL 0x0053fa80
0053f997  837d4800              CMP dword ptr [EBP + 0x48],0x0
0053f99b  0f84a6000000          JZ 0x0053fa47
0053f9a1  8b454c                MOV EAX,dword ptr [EBP + 0x4c]
0053f9a4  50                    PUSH EAX
0053f9a5  8b4d48                MOV ECX,dword ptr [EBP + 0x48]
0053f9a8  51                    PUSH ECX
0053f9a9  6a07                  PUSH 0x7
0053f9ab  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f9ae  52                    PUSH EDX
0053f9af  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f9b2  e8c9000000            CALL 0x0053fa80
0053f9b7  837d5000              CMP dword ptr [EBP + 0x50],0x0
0053f9bb  0f8486000000          JZ 0x0053fa47
0053f9c1  8b4554                MOV EAX,dword ptr [EBP + 0x54]
0053f9c4  50                    PUSH EAX
0053f9c5  8b4d50                MOV ECX,dword ptr [EBP + 0x50]
0053f9c8  51                    PUSH ECX
0053f9c9  6a08                  PUSH 0x8
0053f9cb  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f9ce  52                    PUSH EDX
0053f9cf  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f9d2  e8a9000000            CALL 0x0053fa80
0053f9d7  837d5800              CMP dword ptr [EBP + 0x58],0x0
0053f9db  746a                  JZ 0x0053fa47
0053f9dd  8b455c                MOV EAX,dword ptr [EBP + 0x5c]
0053f9e0  50                    PUSH EAX
0053f9e1  8b4d58                MOV ECX,dword ptr [EBP + 0x58]
0053f9e4  51                    PUSH ECX
0053f9e5  6a09                  PUSH 0x9
0053f9e7  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053f9ea  52                    PUSH EDX
0053f9eb  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053f9ee  e88d000000            CALL 0x0053fa80
0053f9f3  837d6000              CMP dword ptr [EBP + 0x60],0x0
0053f9f7  744e                  JZ 0x0053fa47
0053f9f9  8b4564                MOV EAX,dword ptr [EBP + 0x64]
0053f9fc  50                    PUSH EAX
0053f9fd  8b4d60                MOV ECX,dword ptr [EBP + 0x60]
0053fa00  51                    PUSH ECX
0053fa01  6a0a                  PUSH 0xa
0053fa03  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053fa06  52                    PUSH EDX
0053fa07  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053fa0a  e871000000            CALL 0x0053fa80
0053fa0f  837d6800              CMP dword ptr [EBP + 0x68],0x0
0053fa13  7432                  JZ 0x0053fa47
0053fa15  8b456c                MOV EAX,dword ptr [EBP + 0x6c]
0053fa18  50                    PUSH EAX
0053fa19  8b4d68                MOV ECX,dword ptr [EBP + 0x68]
0053fa1c  51                    PUSH ECX
0053fa1d  6a0b                  PUSH 0xb
0053fa1f  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053fa22  52                    PUSH EDX
0053fa23  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053fa26  e855000000            CALL 0x0053fa80
0053fa2b  837d7000              CMP dword ptr [EBP + 0x70],0x0
0053fa2f  7416                  JZ 0x0053fa47
0053fa31  8b4574                MOV EAX,dword ptr [EBP + 0x74]
0053fa34  50                    PUSH EAX
0053fa35  8b4d70                MOV ECX,dword ptr [EBP + 0x70]
0053fa38  51                    PUSH ECX
0053fa39  6a0c                  PUSH 0xc
0053fa3b  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0053fa3e  52                    PUSH EDX
0053fa3f  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053fa42  e839000000            CALL 0x0053fa80
0053fa47  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0053fa4a  50                    PUSH EAX
0053fa4b  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053fa4e  81c100010000          ADD ECX,0x100
0053fa54  e8d7af0000            CALL 0x0054aa30
0053fa59  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0053fa5c  e8af010000            CALL 0x0053fc10
0053fa61  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0053fa64  64890d00000000        MOV dword ptr FS:[0x0],ECX
0053fa6b  59                    POP ECX
0053fa6c  8be5                  MOV ESP,EBP
0053fa6e  5d                    POP EBP
0053fa6f  c27000                RET 0x70
