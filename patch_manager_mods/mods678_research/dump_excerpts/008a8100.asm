; FN 008a8100 FUN_008a8100
008a8100  55                    PUSH EBP
008a8101  8bec                  MOV EBP,ESP
008a8103  6aff                  PUSH -0x1
008a8105  6840819700            PUSH 0x978140
008a810a  64a100000000          MOV EAX,FS:[0x0]
008a8110  50                    PUSH EAX
008a8111  83ec64                SUB ESP,0x64
008a8114  a1201fa100            MOV EAX,[0x00a11f20]
008a8119  33c5                  XOR EAX,EBP
008a811b  50                    PUSH EAX
008a811c  8d45f4                LEA EAX,[EBP + -0xc]
008a811f  64a300000000          MOV FS:[0x0],EAX
008a8125  894d98                MOV dword ptr [EBP + -0x68],ECX
008a8128  8b4508                MOV EAX,dword ptr [EBP + 0x8]
008a812b  8b4850                MOV ECX,dword ptr [EAX + 0x50]
008a812e  894db4                MOV dword ptr [EBP + -0x4c],ECX
008a8131  837db400              CMP dword ptr [EBP + -0x4c],0x0
008a8135  0f8475020000          JZ 0x008a83b0
008a813b  8b5508                MOV EDX,dword ptr [EBP + 0x8]
008a813e  8955dc                MOV dword ptr [EBP + -0x24],EDX
008a8141  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008a8144  8b88d0010000          MOV ECX,dword ptr [EAX + 0x1d0]
008a814a  894de0                MOV dword ptr [EBP + -0x20],ECX
008a814d  817de00000007f        CMP dword ptr [EBP + -0x20],0x7f000000
008a8154  0f842e010000          JZ 0x008a8288
008a815a  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
008a8160  8b4208                MOV EAX,dword ptr [EDX + 0x8]
008a8163  8945b0                MOV dword ptr [EBP + -0x50],EAX
008a8166  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008a8169  51                    PUSH ECX
008a816a  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
008a816d  e86e3fc7ff            CALL 0x0051c0e0
008a8172  8945d4                MOV dword ptr [EBP + -0x2c],EAX
008a8175  8d55cc                LEA EDX,[EBP + -0x34]
008a8178  52                    PUSH EDX
008a8179  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a817c  e80ff6d5ff            CALL 0x00607790
008a8181  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a8188  6a00                  PUSH 0x0
008a818a  6a3c                  PUSH 0x3c
008a818c  8d4dcc                LEA ECX,[EBP + -0x34]
008a818f  e8fcbfe8ff            CALL 0x00734190
008a8194  8945c4                MOV dword ptr [EBP + -0x3c],EAX
008a8197  6a00                  PUSH 0x0
008a8199  6a7b                  PUSH 0x7b
008a819b  8d4dcc                LEA ECX,[EBP + -0x34]
008a819e  e8edbfe8ff            CALL 0x00734190
008a81a3  8945c8                MOV dword ptr [EBP + -0x38],EAX
008a81a6  837dc4ff              CMP dword ptr [EBP + -0x3c],-0x1
008a81aa  7506                  JNZ 0x008a81b2
008a81ac  837dc8ff              CMP dword ptr [EBP + -0x38],-0x1
008a81b0  7462                  JZ 0x008a8214
008a81b2  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008a81b7  8b4804                MOV ECX,dword ptr [EAX + 0x4]
008a81ba  894dac                MOV dword ptr [EBP + -0x54],ECX
008a81bd  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
008a81c0  e8cb79e9ff            CALL 0x0073fb90
008a81c5  8bc8                  MOV ECX,EAX
008a81c7  e8b4dbf3ff            CALL 0x007e5d80
008a81cc  8945c0                MOV dword ptr [EBP + -0x40],EAX
008a81cf  8b15b4b4a100          MOV EDX,dword ptr [0x00a1b4b4]
008a81d5  8955a8                MOV dword ptr [EBP + -0x58],EDX
008a81d8  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008a81db  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008a81de  894858                MOV dword ptr [EAX + 0x58],ECX
008a81e1  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008a81e4  e8d76cf5ff            CALL 0x007feec0
008a81e9  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
008a81ec  88423c                MOV byte ptr [EDX + 0x3c],AL
008a81ef  b801000000            MOV EAX,0x1
008a81f4  83e001                AND EAX,0x1
008a81f7  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008a81fa  8b515c                MOV EDX,dword ptr [ECX + 0x5c]
008a81fd  83e2fe                AND EDX,0xfffffffe
008a8200  0bd0                  OR EDX,EAX
008a8202  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008a8205  89505c                MOV dword ptr [EAX + 0x5c],EDX
008a8208  8d4dcc                LEA ECX,[EBP + -0x34]
008a820b  51                    PUSH ECX
008a820c  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
008a820f  e8bc69e7ff            CALL 0x0071ebd0
008a8214  8d55cc                LEA EDX,[EBP + -0x34]
008a8217  52                    PUSH EDX
008a8218  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a821b  81c158010000          ADD ECX,0x158
008a8221  e80aecb6ff            CALL 0x00416e30
008a8226  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008a8229  8b485c                MOV ECX,dword ptr [EAX + 0x5c]
008a822c  894da4                MOV dword ptr [EBP + -0x5c],ECX
008a822f  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
008a8232  895594                MOV dword ptr [EBP + -0x6c],EDX
008a8235  8b4594                MOV EAX,dword ptr [EBP + -0x6c]
008a8238  83e001                AND EAX,0x1
008a823b  d1e0                  SHL EAX,0x1
008a823d  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a8240  8b91d0240000          MOV EDX,dword ptr [ECX + 0x24d0]
008a8246  83e2fd                AND EDX,0xfffffffd
008a8249  0bd0                  OR EDX,EAX
008a824b  8b4598                MOV EAX,dword ptr [EBP + -0x68]
008a824e  8990d0240000          MOV dword ptr [EAX + 0x24d0],EDX
008a8254  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008a8257  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
008a825d  c1ea07                SHR EDX,0x7
008a8260  83e201                AND EDX,0x1
008a8263  7412                  JZ 0x008a8277
008a8265  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
008a8268  50                    PUSH EAX
008a8269  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a826c  81c1bc240000          ADD ECX,0x24bc
008a8272  e8298ec7ff            CALL 0x005210a0
008a8277  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008a827e  8d4dcc                LEA ECX,[EBP + -0x34]
008a8281  e8fab4e8ff            CALL 0x00733780
008a8286  eb4a                  JMP 0x008a82d2
008a8288  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a828b  8b91d0240000          MOV EDX,dword ptr [ECX + 0x24d0]
008a8291  83e2fd                AND EDX,0xfffffffd
008a8294  8b4598                MOV EAX,dword ptr [EBP + -0x68]
008a8297  8990d0240000          MOV dword ptr [EAX + 0x24d0],EDX
008a829d  681c739a00            PUSH 0x9a731c
008a82a2  8d4db8                LEA ECX,[EBP + -0x48]
008a82a5  e8c6b2e8ff            CALL 0x00733570
008a82aa  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
008a82b1  8d4db8                LEA ECX,[EBP + -0x48]
008a82b4  51                    PUSH ECX
008a82b5  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a82b8  81c158010000          ADD ECX,0x158
008a82be  e86debb6ff            CALL 0x00416e30
008a82c3  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008a82ca  8d4db8                LEA ECX,[EBP + -0x48]
008a82cd  e8aeb4e8ff            CALL 0x00733780
008a82d2  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a82d5  81c178080000          ADD ECX,0x878
008a82db  e8b04fb7ff            CALL 0x0041d290
008a82e0  8b5598                MOV EDX,dword ptr [EBP + -0x68]
008a82e3  83c26c                ADD EDX,0x6c
008a82e6  8b02                  MOV EAX,dword ptr [EDX]
008a82e8  8945e4                MOV dword ptr [EBP + -0x1c],EAX
008a82eb  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
008a82ee  894de8                MOV dword ptr [EBP + -0x18],ECX
008a82f1  8b4208                MOV EAX,dword ptr [EDX + 0x8]
008a82f4  8945ec                MOV dword ptr [EBP + -0x14],EAX
008a82f7  8b4a0c                MOV ECX,dword ptr [EDX + 0xc]
008a82fa  894df0                MOV dword ptr [EBP + -0x10],ECX
008a82fd  8b5598                MOV EDX,dword ptr [EBP + -0x68]
008a8300  81c258010000          ADD EDX,0x158
008a8306  8955a0                MOV dword ptr [EBP + -0x60],EDX
008a8309  8b45a0                MOV EAX,dword ptr [EBP + -0x60]
008a830c  833800                CMP dword ptr [EAX],0x0
008a830f  7415                  JZ 0x008a8326
008a8311  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
008a8314  8b11                  MOV EDX,dword ptr [ECX]
008a8316  52                    PUSH EDX
008a8317  e8f41b0700            CALL 0x00919f10
008a831c  83c404                ADD ESP,0x4
008a831f  89459c                MOV dword ptr [EBP + -0x64],EAX
008a8322  eb09                  JMP 0x008a832d
008a8326  c7459c00000000        MOV dword ptr [EBP + -0x64],0x0
008a832d  837d9c00              CMP dword ptr [EBP + -0x64],0x0
008a8331  7e1d                  JLE 0x008a8350
008a8333  8b4598                MOV EAX,dword ptr [EBP + -0x68]
008a8336  0540010000            ADD EAX,0x140
008a833b  894590                MOV dword ptr [EBP + -0x70],EAX
008a833e  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
008a8341  8b11                  MOV EDX,dword ptr [ECX]
008a8343  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
008a8346  8b4208                MOV EAX,dword ptr [EDX + 0x8]
008a8349  ffd0                  CALL EAX
008a834b  8945f0                MOV dword ptr [EBP + -0x10],EAX
008a834e  eb11                  JMP 0x008a8361
008a8350  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a8353  81c140010000          ADD ECX,0x140
008a8359  e852f5b6ff            CALL 0x004178b0
008a835e  8945f0                MOV dword ptr [EBP + -0x10],EAX
008a8361  8d4de4                LEA ECX,[EBP + -0x1c]
008a8364  51                    PUSH ECX
008a8365  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a8368  83c168                ADD ECX,0x68
008a836b  8b5598                MOV EDX,dword ptr [EBP + -0x68]
008a836e  8b4268                MOV EAX,dword ptr [EDX + 0x68]
008a8371  8b5004                MOV EDX,dword ptr [EAX + 0x4]
008a8374  ffd2                  CALL EDX
008a8376  8b4598                MOV EAX,dword ptr [EBP + -0x68]
008a8379  83c068                ADD EAX,0x68
008a837c  8945d8                MOV dword ptr [EBP + -0x28],EAX
008a837f  6a00                  PUSH 0x0
008a8381  6a00                  PUSH 0x0
008a8383  6a00                  PUSH 0x0
008a8385  6a01                  PUSH 0x1
008a8387  8d4dd8                LEA ECX,[EBP + -0x28]
008a838a  51                    PUSH ECX
008a838b  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a838e  81c178080000          ADD ECX,0x878
008a8394  e8f779b7ff            CALL 0x0041fd90
008a8399  6a00                  PUSH 0x0
008a839b  8b5598                MOV EDX,dword ptr [EBP + -0x68]
008a839e  83c268                ADD EDX,0x68
008a83a1  52                    PUSH EDX
008a83a2  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008a83a5  81c178080000          ADD ECX,0x878
008a83ab  e8f065b7ff            CALL 0x0041e9a0
008a83b0  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008a83b3  64890d00000000        MOV dword ptr FS:[0x0],ECX
008a83ba  59                    POP ECX
008a83bb  8be5                  MOV ESP,EBP
008a83bd  5d                    POP EBP
008a83be  c20400                RET 0x4
