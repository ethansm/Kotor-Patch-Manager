; FN 005d4190 FUN_005d4190
005d4190  55                    PUSH EBP
005d4191  8bec                  MOV EBP,ESP
005d4193  83ec24                SUB ESP,0x24
005d4196  894ddc                MOV dword ptr [EBP + -0x24],ECX
005d4199  6864519900            PUSH 0x995164
005d419e  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
005d41a1  83c118                ADD ECX,0x18
005d41a4  e8b70d1600            CALL 0x00734f60
005d41a9  85c0                  TEST EAX,EAX
005d41ab  747c                  JZ 0x005d4229
005d41ad  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d41b0  8b480c                MOV ECX,dword ptr [EAX + 0xc]
005d41b3  894dfc                MOV dword ptr [EBP + -0x4],ECX
005d41b6  837dfc23              CMP dword ptr [EBP + -0x4],0x23
005d41ba  7469                  JZ 0x005d4225
005d41bc  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d41bf  8b420c                MOV EAX,dword ptr [EDX + 0xc]
005d41c2  8945f8                MOV dword ptr [EBP + -0x8],EAX
005d41c5  837df824              CMP dword ptr [EBP + -0x8],0x24
005d41c9  745a                  JZ 0x005d4225
005d41cb  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d41ce  8b510c                MOV EDX,dword ptr [ECX + 0xc]
005d41d1  8955f4                MOV dword ptr [EBP + -0xc],EDX
005d41d4  837df425              CMP dword ptr [EBP + -0xc],0x25
005d41d8  744b                  JZ 0x005d4225
005d41da  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d41dd  8b480c                MOV ECX,dword ptr [EAX + 0xc]
005d41e0  894df0                MOV dword ptr [EBP + -0x10],ECX
005d41e3  837df061              CMP dword ptr [EBP + -0x10],0x61
005d41e7  743c                  JZ 0x005d4225
005d41e9  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d41ec  8b420c                MOV EAX,dword ptr [EDX + 0xc]
005d41ef  8945ec                MOV dword ptr [EBP + -0x14],EAX
005d41f2  837dec62              CMP dword ptr [EBP + -0x14],0x62
005d41f6  742d                  JZ 0x005d4225
005d41f8  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d41fb  8b510c                MOV EDX,dword ptr [ECX + 0xc]
005d41fe  8955e8                MOV dword ptr [EBP + -0x18],EDX
005d4201  837de863              CMP dword ptr [EBP + -0x18],0x63
005d4205  741e                  JZ 0x005d4225
005d4207  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d420a  8b480c                MOV ECX,dword ptr [EAX + 0xc]
005d420d  894de4                MOV dword ptr [EBP + -0x1c],ECX
005d4210  837de464              CMP dword ptr [EBP + -0x1c],0x64
005d4214  740f                  JZ 0x005d4225
005d4216  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d4219  8b420c                MOV EAX,dword ptr [EDX + 0xc]
005d421c  8945e0                MOV dword ptr [EBP + -0x20],EAX
005d421f  837de066              CMP dword ptr [EBP + -0x20],0x66
005d4223  7504                  JNZ 0x005d4229
005d4225  33c0                  XOR EAX,EAX
005d4227  eb05                  JMP 0x005d422e
005d4229  b801000000            MOV EAX,0x1
005d422e  8be5                  MOV ESP,EBP
005d4230  5d                    POP EBP
005d4231  c20400                RET 0x4
