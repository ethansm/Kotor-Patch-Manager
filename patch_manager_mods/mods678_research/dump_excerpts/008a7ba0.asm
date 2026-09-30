; FN 008a7ba0 FUN_008a7ba0
008a7ba0  55                    PUSH EBP
008a7ba1  8bec                  MOV EBP,ESP
008a7ba3  6aff                  PUSH -0x1
008a7ba5  6808819700            PUSH 0x978108
008a7baa  64a100000000          MOV EAX,FS:[0x0]
008a7bb0  50                    PUSH EAX
008a7bb1  81ec88000000          SUB ESP,0x88
008a7bb7  a1201fa100            MOV EAX,[0x00a11f20]
008a7bbc  33c5                  XOR EAX,EBP
008a7bbe  50                    PUSH EAX
008a7bbf  8d45f4                LEA EAX,[EBP + -0xc]
008a7bc2  64a300000000          MOV FS:[0x0],EAX
008a7bc8  898d7cffffff          MOV dword ptr [EBP + 0xffffff7c],ECX
008a7bce  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008a7bd3  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008a7bd6  894dac                MOV dword ptr [EBP + -0x54],ECX
008a7bd9  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
008a7bdc  e8af7fe9ff            CALL 0x0073fb90
008a7be1  8bc8                  MOV ECX,EAX
008a7be3  e898e1f3ff            CALL 0x007e5d80
008a7be8  8945c8                MOV dword ptr [EBP + -0x38],EAX
008a7beb  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008a7bee  e80d5cedff            CALL 0x0077d800
008a7bf3  8945c0                MOV dword ptr [EBP + -0x40],EAX
008a7bf6  6a01                  PUSH 0x1
008a7bf8  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008a7bfb  e8c026ceff            CALL 0x0058a2c0
008a7c00  8945e4                MOV dword ptr [EBP + -0x1c],EAX
008a7c03  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008a7c06  e8d596cdff            CALL 0x005812e0
008a7c0b  50                    PUSH EAX
008a7c0c  8d4db8                LEA ECX,[EBP + -0x48]
008a7c0f  e8ecbae8ff            CALL 0x00733700
008a7c14  898578ffffff          MOV dword ptr [EBP + 0xffffff78],EAX
008a7c1a  8b9578ffffff          MOV EDX,dword ptr [EBP + 0xffffff78]
008a7c20  899574ffffff          MOV dword ptr [EBP + 0xffffff74],EDX
008a7c26  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a7c2d  8b8574ffffff          MOV EAX,dword ptr [EBP + 0xffffff74]
008a7c33  50                    PUSH EAX
008a7c34  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7c3a  81c130050000          ADD ECX,0x530
008a7c40  e8ebf1b6ff            CALL 0x00416e30
008a7c45  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008a7c4c  8d4db8                LEA ECX,[EBP + -0x48]
008a7c4f  e82cbbe8ff            CALL 0x00733780
008a7c54  6a00                  PUSH 0x0
008a7c56  8d4dd4                LEA ECX,[EBP + -0x2c]
008a7c59  e8e229faff            CALL 0x0084a640
008a7c5e  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
008a7c65  c745c400000000        MOV dword ptr [EBP + -0x3c],0x0
008a7c6c  c745e0ffffffff        MOV dword ptr [EBP + -0x20],0xffffffff
008a7c73  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
008a7c7a  eb09                  JMP 0x008a7c85
008a7c7c  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7c7f  83c101                ADD ECX,0x1
008a7c82  894dec                MOV dword ptr [EBP + -0x14],ECX
008a7c85  837dec14              CMP dword ptr [EBP + -0x14],0x14
008a7c89  0f832d010000          JNC 0x008a7dbc
008a7c8f  ba01000000            MOV EDX,0x1
008a7c94  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a7c97  d3e2                  SHL EDX,CL
008a7c99  8955f0                MOV dword ptr [EBP + -0x10],EDX
008a7c9c  817df000400000        CMP dword ptr [EBP + -0x10],0x4000
008a7ca3  741b                  JZ 0x008a7cc0
008a7ca5  817df000800000        CMP dword ptr [EBP + -0x10],0x8000
008a7cac  7412                  JZ 0x008a7cc0
008a7cae  817df000000100        CMP dword ptr [EBP + -0x10],0x10000
008a7cb5  7409                  JZ 0x008a7cc0
008a7cb7  817df000000200        CMP dword ptr [EBP + -0x10],0x20000
008a7cbe  7502                  JNZ 0x008a7cc2
008a7cc0  ebba                  JMP 0x008a7c7c
008a7cc2  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008a7cc7  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008a7cca  894da8                MOV dword ptr [EBP + -0x58],ECX
008a7ccd  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
008a7cd0  52                    PUSH EDX
008a7cd1  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008a7cd4  e83769ecff            CALL 0x0076e610
008a7cd9  50                    PUSH EAX
008a7cda  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008a7cdd  e84e78e9ff            CALL 0x0073f530
008a7ce2  8945d0                MOV dword ptr [EBP + -0x30],EAX
008a7ce5  837dd000              CMP dword ptr [EBP + -0x30],0x0
008a7ce9  0f84c8000000          JZ 0x008a7db7
008a7cef  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
008a7cf2  e87971f5ff            CALL 0x007fee70
008a7cf7  8945e8                MOV dword ptr [EBP + -0x18],EAX
008a7cfa  817df080000000        CMP dword ptr [EBP + -0x10],0x80
008a7d01  741e                  JZ 0x008a7d21
008a7d03  817df000010000        CMP dword ptr [EBP + -0x10],0x100
008a7d0a  7415                  JZ 0x008a7d21
008a7d0c  817df000040000        CMP dword ptr [EBP + -0x10],0x400
008a7d13  740c                  JZ 0x008a7d21
008a7d15  c78570ffffff00000000  MOV dword ptr [EBP + 0xffffff70],0x0
008a7d1f  eb0a                  JMP 0x008a7d2b
008a7d21  c78570ffffff01000000  MOV dword ptr [EBP + 0xffffff70],0x1
008a7d2b  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008a7d31  8945cc                MOV dword ptr [EBP + -0x34],EAX
008a7d34  837de800              CMP dword ptr [EBP + -0x18],0x0
008a7d38  747d                  JZ 0x008a7db7
008a7d3a  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008a7d3d  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
008a7d43  c1ea08                SHR EDX,0x8
008a7d46  83e201                AND EDX,0x1
008a7d49  756c                  JNZ 0x008a7db7
008a7d4b  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
008a7d4e  50                    PUSH EAX
008a7d4f  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008a7d52  51                    PUSH ECX
008a7d53  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008a7d56  52                    PUSH EDX
008a7d57  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7d5d  e88e100000            CALL 0x008a8df0
008a7d62  85c0                  TEST EAX,EAX
008a7d64  7451                  JZ 0x008a7db7
008a7d66  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
008a7d69  8b4814                MOV ECX,dword ptr [EAX + 0x14]
008a7d6c  894da4                MOV dword ptr [EBP + -0x5c],ECX
008a7d6f  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
008a7d75  8b82c8240000          MOV EAX,dword ptr [EDX + 0x24c8]
008a7d7b  3b45a4                CMP EAX,dword ptr [EBP + -0x5c]
008a7d7e  7516                  JNZ 0x008a7d96
008a7d80  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
008a7d83  894de0                MOV dword ptr [EBP + -0x20],ECX
008a7d86  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
008a7d8c  c782c82400000000007f  MOV dword ptr [EDX + 0x24c8],0x7f000000
008a7d96  6a01                  PUSH 0x1
008a7d98  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
008a7d9b  50                    PUSH EAX
008a7d9c  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008a7d9f  51                    PUSH ECX
008a7da0  8d55d4                LEA EDX,[EBP + -0x2c]
008a7da3  52                    PUSH EDX
008a7da4  8d45c4                LEA EAX,[EBP + -0x3c]
008a7da7  50                    PUSH EAX
008a7da8  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
008a7dab  51                    PUSH ECX
008a7dac  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7db2  e839f8ffff            CALL 0x008a75f0
008a7db7  e9c0feffff            JMP 0x008a7c7c
008a7dbc  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
008a7dc3  eb09                  JMP 0x008a7dce
008a7dc5  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008a7dc8  83c201                ADD EDX,0x1
008a7dcb  8955ec                MOV dword ptr [EBP + -0x14],EDX
008a7dce  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
008a7dd1  8b4810                MOV ECX,dword ptr [EAX + 0x10]
008a7dd4  894da0                MOV dword ptr [EBP + -0x60],ECX
008a7dd7  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008a7dda  3b55a0                CMP EDX,dword ptr [EBP + -0x60]
008a7ddd  0f8393000000          JNC 0x008a7e76
008a7de3  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008a7de6  50                    PUSH EAX
008a7de7  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008a7dea  e811e3d5ff            CALL 0x00606100
008a7def  8945e8                MOV dword ptr [EBP + -0x18],EAX
008a7df2  837de800              CMP dword ptr [EBP + -0x18],0x0
008a7df6  7479                  JZ 0x008a7e71
008a7df8  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008a7dfb  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
008a7e01  c1ea08                SHR EDX,0x8
008a7e04  83e201                AND EDX,0x1
008a7e07  7568                  JNZ 0x008a7e71
008a7e09  6a00                  PUSH 0x0
008a7e0b  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
008a7e0e  50                    PUSH EAX
008a7e0f  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
008a7e12  51                    PUSH ECX
008a7e13  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7e19  e8d20f0000            CALL 0x008a8df0
008a7e1e  85c0                  TEST EAX,EAX
008a7e20  744f                  JZ 0x008a7e71
008a7e22  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008a7e25  8b4214                MOV EAX,dword ptr [EDX + 0x14]
008a7e28  89459c                MOV dword ptr [EBP + -0x64],EAX
008a7e2b  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7e31  8b91c8240000          MOV EDX,dword ptr [ECX + 0x24c8]
008a7e37  3b559c                CMP EDX,dword ptr [EBP + -0x64]
008a7e3a  7516                  JNZ 0x008a7e52
008a7e3c  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
008a7e3f  8945e0                MOV dword ptr [EBP + -0x20],EAX
008a7e42  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7e48  c781c82400000000007f  MOV dword ptr [ECX + 0x24c8],0x7f000000
008a7e52  6a00                  PUSH 0x0
008a7e54  6a00                  PUSH 0x0
008a7e56  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
008a7e59  52                    PUSH EDX
008a7e5a  8d45d4                LEA EAX,[EBP + -0x2c]
008a7e5d  50                    PUSH EAX
008a7e5e  8d4dc4                LEA ECX,[EBP + -0x3c]
008a7e61  51                    PUSH ECX
008a7e62  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
008a7e65  52                    PUSH EDX
008a7e66  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7e6c  e87ff7ffff            CALL 0x008a75f0
008a7e71  e94fffffff            JMP 0x008a7dc5
008a7e76  6a00                  PUSH 0x0
008a7e78  6a01                  PUSH 0x1
008a7e7a  6a00                  PUSH 0x0
008a7e7c  8d45d4                LEA EAX,[EBP + -0x2c]
008a7e7f  50                    PUSH EAX
008a7e80  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7e86  81c188050000          ADD ECX,0x588
008a7e8c  e89f7db7ff            CALL 0x0041fc30
008a7e91  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008a7e94  894d98                MOV dword ptr [EBP + -0x68],ECX
008a7e97  837d9800              CMP dword ptr [EBP + -0x68],0x0
008a7e9b  0f8ed5000000          JLE 0x008a7f76
008a7ea1  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
008a7ea7  8b82d0240000          MOV EAX,dword ptr [EDX + 0x24d0]
008a7ead  83e001                AND EAX,0x1
008a7eb0  0f849a000000          JZ 0x008a7f50
008a7eb6  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7ebc  0fb791cc240000        MOVZX EDX,word ptr [ECX + 0x24cc]
008a7ec3  52                    PUSH EDX
008a7ec4  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7eca  81c188050000          ADD ECX,0x588
008a7ed0  e86b010000            CALL 0x008a8040
008a7ed5  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
008a7ed8  894584                MOV dword ptr [EBP + -0x7c],EAX
008a7edb  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7ee1  0fbf91ce240000        MOVSX EDX,word ptr [ECX + 0x24ce]
008a7ee8  8b4584                MOV EAX,dword ptr [EBP + -0x7c]
008a7eeb  83e801                SUB EAX,0x1
008a7eee  3bd0                  CMP EDX,EAX
008a7ef0  7d15                  JGE 0x008a7f07
008a7ef2  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7ef8  0fbf91ce240000        MOVSX EDX,word ptr [ECX + 0x24ce]
008a7eff  89956cffffff          MOV dword ptr [EBP + 0xffffff6c],EDX
008a7f05  eb12                  JMP 0x008a7f19
008a7f07  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
008a7f0a  894580                MOV dword ptr [EBP + -0x80],EAX
008a7f0d  8b4d80                MOV ECX,dword ptr [EBP + -0x80]
008a7f10  83e901                SUB ECX,0x1
008a7f13  898d6cffffff          MOV dword ptr [EBP + 0xffffff6c],ECX
008a7f19  6a00                  PUSH 0x0
008a7f1b  8b956cffffff          MOV EDX,dword ptr [EBP + 0xffffff6c]
008a7f21  52                    PUSH EDX
008a7f22  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7f28  81c188050000          ADD ECX,0x588
008a7f2e  e83d69b7ff            CALL 0x0041e870
008a7f33  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
008a7f39  8b88d0240000          MOV ECX,dword ptr [EAX + 0x24d0]
008a7f3f  83e1fe                AND ECX,0xfffffffe
008a7f42  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
008a7f48  898ad0240000          MOV dword ptr [EDX + 0x24d0],ECX
008a7f4e  eb21                  JMP 0x008a7f71
008a7f50  6a00                  PUSH 0x0
008a7f52  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
008a7f55  83e8ff                SUB EAX,-0x1
008a7f58  f7d8                  NEG EAX
008a7f5a  1bc0                  SBB EAX,EAX
008a7f5c  2345e0                AND EAX,dword ptr [EBP + -0x20]
008a7f5f  50                    PUSH EAX
008a7f60  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7f66  81c188050000          ADD ECX,0x588
008a7f6c  e8ff68b7ff            CALL 0x0041e870
008a7f71  e982000000            JMP 0x008a7ff8
008a7f76  684cfda700            PUSH 0xa7fd4c
008a7f7b  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7f81  81c1b00e0000          ADD ECX,0xeb0
008a7f87  e884f0b6ff            CALL 0x00417010
008a7f8c  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7f92  81c1d00d0000          ADD ECX,0xdd0
008a7f98  8b1570fda700          MOV EDX,dword ptr [0x00a7fd70]
008a7f9e  8911                  MOV dword ptr [ECX],EDX
008a7fa0  a174fda700            MOV EAX,[0x00a7fd74]
008a7fa5  894104                MOV dword ptr [ECX + 0x4],EAX
008a7fa8  8b1578fda700          MOV EDX,dword ptr [0x00a7fd78]
008a7fae  895108                MOV dword ptr [ECX + 0x8],EDX
008a7fb1  6a00                  PUSH 0x0
008a7fb3  6aff                  PUSH -0x1
008a7fb5  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7fbb  81c188050000          ADD ECX,0x588
008a7fc1  e8aa68b7ff            CALL 0x0041e870
008a7fc6  6825699800            PUSH 0x986925
008a7fcb  8d4db0                LEA ECX,[EBP + -0x50]
008a7fce  e89db5e8ff            CALL 0x00733570
008a7fd3  c645fc02              MOV byte ptr [EBP + -0x4],0x2
008a7fd7  8d45b0                LEA EAX,[EBP + -0x50]
008a7fda  50                    PUSH EAX
008a7fdb  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7fe1  81c158010000          ADD ECX,0x158
008a7fe7  e844eeb6ff            CALL 0x00416e30
008a7fec  c645fc01              MOV byte ptr [EBP + -0x4],0x1
008a7ff0  8d4db0                LEA ECX,[EBP + -0x50]
008a7ff3  e888b7e8ff            CALL 0x00733780
008a7ff8  8b8d7cffffff          MOV ECX,dword ptr [EBP + 0xffffff7c]
008a7ffe  8b91d0240000          MOV EDX,dword ptr [ECX + 0x24d0]
008a8004  83e2fb                AND EDX,0xfffffffb
008a8007  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
008a800d  8990d0240000          MOV dword ptr [EAX + 0x24d0],EDX
008a8013  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008a801a  8d4dd4                LEA ECX,[EBP + -0x2c]
008a801d  e88e5ddaff            CALL 0x0064ddb0
008a8022  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008a8025  64890d00000000        MOV dword ptr FS:[0x0],ECX
008a802c  59                    POP ECX
008a802d  8be5                  MOV ESP,EBP
008a802f  5d                    POP EBP
008a8030  c3                    RET
