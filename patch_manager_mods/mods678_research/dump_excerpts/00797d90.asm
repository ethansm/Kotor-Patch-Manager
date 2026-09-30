; FN 00797d90 FUN_00797d90
00797d90  55                    PUSH EBP
00797d91  8bec                  MOV EBP,ESP
00797d93  51                    PUSH ECX
00797d94  894dfc                MOV dword ptr [EBP + -0x4],ECX
00797d97  833d8cb4a10000        CMP dword ptr [0x00a1b48c],0x0
00797d9e  740f                  JZ 0x00797daf
00797da0  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00797da3  50                    PUSH EAX
00797da4  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
00797daa  e8717df9ff            CALL 0x0072fb20
00797daf  837d0800              CMP dword ptr [EBP + 0x8],0x0
00797db3  7508                  JNZ 0x00797dbd
00797db5  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00797db8  e833000000            CALL 0x00797df0
00797dbd  837d0c00              CMP dword ptr [EBP + 0xc],0x0
00797dc1  740b                  JZ 0x00797dce
00797dc3  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
00797dc9  e8227cf9ff            CALL 0x0072f9f0
00797dce  837d0800              CMP dword ptr [EBP + 0x8],0x0
00797dd2  740a                  JZ 0x00797dde
00797dd4  6a00                  PUSH 0x0
00797dd6  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00797dd9  e852bb0100            CALL 0x007b3930
00797dde  8be5                  MOV ESP,EBP
00797de0  5d                    POP EBP
00797de1  c20800                RET 0x8
