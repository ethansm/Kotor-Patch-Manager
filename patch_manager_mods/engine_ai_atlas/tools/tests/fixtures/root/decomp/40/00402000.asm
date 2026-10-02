; FN 00402000 FUN_00402000
00402000  55                    PUSH EBP
00402001  8bec                  MOV EBP,ESP
00402003  898d6cfeffff          MOV dword ptr [EBP + 0xfffffe6c],ECX
00402009  d95df8                FSTP float ptr [EBP + -0x8]
0040200c  8b45f8                MOV EAX,dword ptr [EBP + 0xfffffff8]
0040200f  8b856cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe6c]
00402015  c78024050000401f0000  MOV dword ptr [EAX + 0x524],0x1f40
0040201f  8b956cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe6c]
00402025  8b8a24050000          MOV ECX,dword ptr [EDX + 0x524]
0040202b  018a24050000          ADD dword ptr [EDX + 0x524],ECX
00402031  8b410c                MOV EAX,dword ptr [ECX + 0xc]
00402034  c78490bc10000000000000MOV dword ptr [EAX + EDX*0x4 + 0x10bc],0x0
0040203b  c7849024050000000000  MOV dword ptr [EAX + EDX*0x4 + 0x524],0x0
00402045  8d8124050000          LEA EAX,[ECX + 0x524]
0040204b  e8b0efffff            CALL 0x00403000
00402050  8bf0                  MOV ESI,EAX
00402052  83be2405000000        CMP dword ptr [ESI + 0x524],0x0
00402059  0fb68624050000        MOVZX EAX,byte ptr [ESI + 0x524]
00402060  8b8e24050000          MOV ECX,dword ptr [ESI + 0x1524]
00402066  8a8624050000          MOV AL,byte ptr [ESI + 0x524]
0040206c  0f94862405000000      SETZ byte ptr [ESI + 0x524]
00402070  8b0d24050000          MOV ECX,dword ptr [0x00000524]
00402076  c3                    RET
