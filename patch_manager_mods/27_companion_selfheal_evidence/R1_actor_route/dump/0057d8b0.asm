; FN 0057d8b0 FUN_0057d8b0
0057d8b0  55                    PUSH EBP
0057d8b1  8bec                  MOV EBP,ESP
0057d8b3  83ec34                SUB ESP,0x34
0057d8b6  894dd0                MOV dword ptr [EBP + -0x30],ECX
0057d8b9  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0057d8be  8b4808                MOV ECX,dword ptr [EAX + 0x8]
0057d8c1  894de0                MOV dword ptr [EBP + -0x20],ECX
0057d8c4  8b5510                MOV EDX,dword ptr [EBP + 0x10]
0057d8c7  52                    PUSH EDX
0057d8c8  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0057d8cb  e830e8f9ff            CALL 0x0051c100
0057d8d0  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0057d8d3  837de400              CMP dword ptr [EBP + -0x1c],0x0
0057d8d7  7507                  JNZ 0x0057d8e0
0057d8d9  33c0                  XOR EAX,EAX
0057d8db  e9a6010000            JMP 0x0057da86
0057d8e0  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0057d8e3  8b88ec000000          MOV ECX,dword ptr [EAX + 0xec]
0057d8e9  894ddc                MOV dword ptr [EBP + -0x24],ECX
0057d8ec  837ddc00              CMP dword ptr [EBP + -0x24],0x0
0057d8f0  7507                  JNZ 0x0057d8f9
0057d8f2  33c0                  XOR EAX,EAX
0057d8f4  e98d010000            JMP 0x0057da86
0057d8f9  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057d8fc  e8bf7afcff            CALL 0x005453c0
0057d901  85c0                  TEST EAX,EAX
0057d903  7507                  JNZ 0x0057d90c
0057d905  33c0                  XOR EAX,EAX
0057d907  e97a010000            JMP 0x0057da86
0057d90c  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
0057d90f  81c294000000          ADD EDX,0x94
0057d915  8b02                  MOV EAX,dword ptr [EDX]
0057d917  8945e8                MOV dword ptr [EBP + -0x18],EAX
0057d91a  8b4a04                MOV ECX,dword ptr [EDX + 0x4]
0057d91d  894dec                MOV dword ptr [EBP + -0x14],ECX
0057d920  8b5208                MOV EDX,dword ptr [EDX + 0x8]
0057d923  8955f0                MOV dword ptr [EBP + -0x10],EDX
0057d926  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057d929  e8927afcff            CALL 0x005453c0
0057d92e  0520010000            ADD EAX,0x120
0057d933  8945d8                MOV dword ptr [EBP + -0x28],EAX
0057d936  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
0057d939  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0057d93c  894df4                MOV dword ptr [EBP + -0xc],ECX
0057d93f  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0057d942  8955cc                MOV dword ptr [EBP + -0x34],EDX
0057d945  837dcc00              CMP dword ptr [EBP + -0x34],0x0
0057d949  7419                  JZ 0x0057d964
0057d94b  837dcc01              CMP dword ptr [EBP + -0x34],0x1
0057d94f  0f84d7000000          JZ 0x0057da2c
0057d955  837dcc02              CMP dword ptr [EBP + -0x34],0x2
0057d959  0f8400010000          JZ 0x0057da5f
0057d95f  e91d010000            JMP 0x0057da81
0057d964  817d180000007f        CMP dword ptr [EBP + 0x18],0x7f000000
0057d96b  742c                  JZ 0x0057d999
0057d96d  837d1cff              CMP dword ptr [EBP + 0x1c],-0x1
0057d971  7426                  JZ 0x0057d999
0057d973  6a00                  PUSH 0x0
0057d975  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0057d978  50                    PUSH EAX
0057d979  8d4de8                LEA ECX,[EBP + -0x18]
0057d97c  51                    PUSH ECX
0057d97d  8b5510                MOV EDX,dword ptr [EBP + 0x10]
0057d980  52                    PUSH EDX
0057d981  6a00                  PUSH 0x0
0057d983  0fb6451c              MOVZX EAX,byte ptr [EBP + 0x1c]
0057d987  50                    PUSH EAX
0057d988  8b4d18                MOV ECX,dword ptr [EBP + 0x18]
0057d98b  51                    PUSH ECX
0057d98c  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057d98f  e82cdbffff            CALL 0x0057b4c0
0057d994  e991000000            JMP 0x0057da2a
0057d999  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
0057d99c  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
0057d9a2  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0057d9a5  8d4dfc                LEA ECX,[EBP + -0x4]
0057d9a8  51                    PUSH ECX
0057d9a9  8d55f8                LEA EDX,[EBP + -0x8]
0057d9ac  52                    PUSH EDX
0057d9ad  8b450c                MOV EAX,dword ptr [EBP + 0xc]
0057d9b0  50                    PUSH EAX
0057d9b1  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0057d9b4  e8f72c1400            CALL 0x006c06b0
0057d9b9  83f801                CMP EAX,0x1
0057d9bc  7523                  JNZ 0x0057d9e1
0057d9be  6a00                  PUSH 0x0
0057d9c0  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0057d9c3  51                    PUSH ECX
0057d9c4  8d55e8                LEA EDX,[EBP + -0x18]
0057d9c7  52                    PUSH EDX
0057d9c8  8b4510                MOV EAX,dword ptr [EBP + 0x10]
0057d9cb  50                    PUSH EAX
0057d9cc  6a00                  PUSH 0x0
0057d9ce  0fb64dfc              MOVZX ECX,byte ptr [EBP + -0x4]
0057d9d2  51                    PUSH ECX
0057d9d3  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0057d9d6  52                    PUSH EDX
0057d9d7  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057d9da  e8e1daffff            CALL 0x0057b4c0
0057d9df  eb49                  JMP 0x0057da2a
0057d9e1  6a00                  PUSH 0x0
0057d9e3  0fb64520              MOVZX EAX,byte ptr [EBP + 0x20]
0057d9e7  50                    PUSH EAX
0057d9e8  6aff                  PUSH -0x1
0057d9ea  6a00                  PUSH 0x0
0057d9ec  6a00                  PUSH 0x0
0057d9ee  6a00                  PUSH 0x0
0057d9f0  6a00                  PUSH 0x0
0057d9f2  6a00                  PUSH 0x0
0057d9f4  6a00                  PUSH 0x0
0057d9f6  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
0057d9f9  51                    PUSH ECX
0057d9fa  83ec0c                SUB ESP,0xc
0057d9fd  8bd4                  MOV EDX,ESP
0057d9ff  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0057da02  8902                  MOV dword ptr [EDX],EAX
0057da04  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0057da07  894a04                MOV dword ptr [EDX + 0x4],ECX
0057da0a  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0057da0d  894208                MOV dword ptr [EDX + 0x8],EAX
0057da10  6a00                  PUSH 0x0
0057da12  0fb64d24              MOVZX ECX,byte ptr [EBP + 0x24]
0057da16  51                    PUSH ECX
0057da17  6a00                  PUSH 0x0
0057da19  0fb65514              MOVZX EDX,byte ptr [EBP + 0x14]
0057da1d  52                    PUSH EDX
0057da1e  8b450c                MOV EAX,dword ptr [EBP + 0xc]
0057da21  50                    PUSH EAX
0057da22  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057da25  e8e678feff            CALL 0x00565310
0057da2a  eb55                  JMP 0x0057da81
0057da2c  6a00                  PUSH 0x0
0057da2e  6a04                  PUSH 0x4
0057da30  6a00                  PUSH 0x0
0057da32  6a00                  PUSH 0x0
0057da34  6a01                  PUSH 0x1
0057da36  68dc050000            PUSH 0x5dc
0057da3b  6819270000            PUSH 0x2719
0057da40  6a01                  PUSH 0x1
0057da42  6a00                  PUSH 0x0
0057da44  6a00                  PUSH 0x0
0057da46  6a01                  PUSH 0x1
0057da48  6a00                  PUSH 0x0
0057da4a  6a00                  PUSH 0x0
0057da4c  0fb74d0c              MOVZX ECX,word ptr [EBP + 0xc]
0057da50  51                    PUSH ECX
0057da51  8b5510                MOV EDX,dword ptr [EBP + 0x10]
0057da54  52                    PUSH EDX
0057da55  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057da58  e8e3aafeff            CALL 0x00568540
0057da5d  eb22                  JMP 0x0057da81
0057da5f  6a00                  PUSH 0x0
0057da61  680000007f            PUSH 0x7f000000
0057da66  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0057da69  50                    PUSH EAX
0057da6a  8d4de8                LEA ECX,[EBP + -0x18]
0057da6d  51                    PUSH ECX
0057da6e  8b5510                MOV EDX,dword ptr [EBP + 0x10]
0057da71  52                    PUSH EDX
0057da72  6a00                  PUSH 0x0
0057da74  0fb6450c              MOVZX EAX,byte ptr [EBP + 0xc]
0057da78  50                    PUSH EAX
0057da79  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057da7c  e8ffd7ffff            CALL 0x0057b280
0057da81  b801000000            MOV EAX,0x1
0057da86  8be5                  MOV ESP,EBP
0057da88  5d                    POP EBP
0057da89  c22000                RET 0x20
