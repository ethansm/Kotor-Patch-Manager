; FN 005d2f80 FUN_005d2f80
005d2f80  55                    PUSH EBP
005d2f81  8bec                  MOV EBP,ESP
005d2f83  83ec58                SUB ESP,0x58
005d2f86  56                    PUSH ESI
005d2f87  894da8                MOV dword ptr [EBP + -0x58],ECX
005d2f8a  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
005d2f91  837d0800              CMP dword ptr [EBP + 0x8],0x0
005d2f95  7507                  JNZ 0x005d2f9e
005d2f97  33c0                  XOR EAX,EAX
005d2f99  e910030000            JMP 0x005d32ae
005d2f9e  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d2fa2  7519                  JNZ 0x005d2fbd
005d2fa4  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d2fa7  50                    PUSH EAX
005d2fa8  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d2fab  e8d0040000            CALL 0x005d3480
005d2fb0  85c0                  TEST EAX,EAX
005d2fb2  7509                  JNZ 0x005d2fbd
005d2fb4  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d2fb7  83e100                AND ECX,0x0
005d2fba  894dfc                MOV dword ptr [EBP + -0x4],ECX
005d2fbd  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d2fc1  7519                  JNZ 0x005d2fdc
005d2fc3  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d2fc6  52                    PUSH EDX
005d2fc7  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d2fca  e8f1020000            CALL 0x005d32c0
005d2fcf  85c0                  TEST EAX,EAX
005d2fd1  7509                  JNZ 0x005d2fdc
005d2fd3  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d2fd6  83e000                AND EAX,0x0
005d2fd9  8945fc                MOV dword ptr [EBP + -0x4],EAX
005d2fdc  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d2fe0  0f8586000000          JNZ 0x005d306c
005d2fe6  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
005d2fe9  51                    PUSH ECX
005d2fea  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d2fed  52                    PUSH EDX
005d2fee  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d2ff1  e85a060000            CALL 0x005d3650
005d2ff6  85c0                  TEST EAX,EAX
005d2ff8  750b                  JNZ 0x005d3005
005d2ffa  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d2ffd  83e000                AND EAX,0x0
005d3000  8945fc                MOV dword ptr [EBP + -0x4],EAX
005d3003  eb67                  JMP 0x005d306c
005d3005  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3008  e8233e1000            CALL 0x006d6e30
005d300d  8945d8                MOV dword ptr [EBP + -0x28],EAX
005d3010  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
005d3013  8a91ac000000          MOV DL,byte ptr [ECX + 0xac]
005d3019  8855d7                MOV byte ptr [EBP + -0x29],DL
005d301c  0fb645d7              MOVZX EAX,byte ptr [EBP + -0x29]
005d3020  8945dc                MOV dword ptr [EBP + -0x24],EAX
005d3023  837d1000              CMP dword ptr [EBP + 0x10],0x0
005d3027  7443                  JZ 0x005d306c
005d3029  837ddc2d              CMP dword ptr [EBP + -0x24],0x2d
005d302d  7406                  JZ 0x005d3035
005d302f  837ddc1a              CMP dword ptr [EBP + -0x24],0x1a
005d3033  7537                  JNZ 0x005d306c
005d3035  6a00                  PUSH 0x0
005d3037  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d303a  8b11                  MOV EDX,dword ptr [ECX]
005d303c  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d303f  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
005d3045  ffd0                  CALL EAX
005d3047  0fbff0                MOVSX ESI,AX
005d304a  6a01                  PUSH 0x1
005d304c  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d304f  8b11                  MOV EDX,dword ptr [ECX]
005d3051  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d3054  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
005d305a  ffd0                  CALL EAX
005d305c  0fbfc8                MOVSX ECX,AX
005d305f  3bf1                  CMP ESI,ECX
005d3061  7509                  JNZ 0x005d306c
005d3063  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d3066  83e200                AND EDX,0x0
005d3069  8955fc                MOV dword ptr [EBP + -0x4],EDX
005d306c  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d3070  7519                  JNZ 0x005d308b
005d3072  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d3075  50                    PUSH EAX
005d3076  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d3079  e8a2090000            CALL 0x005d3a20
005d307e  85c0                  TEST EAX,EAX
005d3080  7509                  JNZ 0x005d308b
005d3082  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d3085  83e100                AND ECX,0x0
005d3088  894dfc                MOV dword ptr [EBP + -0x4],ECX
005d308b  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d308f  7519                  JNZ 0x005d30aa
005d3091  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d3094  52                    PUSH EDX
005d3095  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d3098  e8c30a0000            CALL 0x005d3b60
005d309d  85c0                  TEST EAX,EAX
005d309f  7509                  JNZ 0x005d30aa
005d30a1  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d30a4  83e000                AND EAX,0x0
005d30a7  8945fc                MOV dword ptr [EBP + -0x4],EAX
005d30aa  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d30ae  7519                  JNZ 0x005d30c9
005d30b0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d30b3  51                    PUSH ECX
005d30b4  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d30b7  e8d40b0000            CALL 0x005d3c90
005d30bc  85c0                  TEST EAX,EAX
005d30be  7509                  JNZ 0x005d30c9
005d30c0  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d30c3  83e200                AND EDX,0x0
005d30c6  8955fc                MOV dword ptr [EBP + -0x4],EDX
005d30c9  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d30cd  7519                  JNZ 0x005d30e8
005d30cf  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d30d2  50                    PUSH EAX
005d30d3  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d30d6  e8050e0000            CALL 0x005d3ee0
005d30db  85c0                  TEST EAX,EAX
005d30dd  7509                  JNZ 0x005d30e8
005d30df  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d30e2  83e100                AND ECX,0x0
005d30e5  894dfc                MOV dword ptr [EBP + -0x4],ECX
005d30e8  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d30ec  7519                  JNZ 0x005d3107
005d30ee  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d30f1  52                    PUSH EDX
005d30f2  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d30f5  e896100000            CALL 0x005d4190
005d30fa  85c0                  TEST EAX,EAX
005d30fc  7509                  JNZ 0x005d3107
005d30fe  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d3101  83e000                AND EAX,0x0
005d3104  8945fc                MOV dword ptr [EBP + -0x4],EAX
005d3107  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d310a  8b91e8100000          MOV EDX,dword ptr [ECX + 0x10e8]
005d3110  8955d0                MOV dword ptr [EBP + -0x30],EDX
005d3113  837dd001              CMP dword ptr [EBP + -0x30],0x1
005d3117  7567                  JNZ 0x005d3180
005d3119  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
005d311c  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005d3122  894dcc                MOV dword ptr [EBP + -0x34],ECX
005d3125  6a00                  PUSH 0x0
005d3127  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
005d312a  e8d11c0d00            CALL 0x006a4e00
005d312f  0fb6f0                MOVZX ESI,AL
005d3132  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3135  e8f63a0300            CALL 0x00606c30
005d313a  0fb6d0                MOVZX EDX,AL
005d313d  3bf2                  CMP ESI,EDX
005d313f  7d27                  JGE 0x005d3168
005d3141  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005d3146  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005d3149  894dc8                MOV dword ptr [EBP + -0x38],ECX
005d314c  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
005d314f  e8ec8bf4ff            CALL 0x0051bd40
005d3154  83b8d000000000        CMP dword ptr [EAX + 0xd0],0x0
005d315b  740b                  JZ 0x005d3168
005d315d  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d3160  83e200                AND EDX,0x0
005d3163  8955fc                MOV dword ptr [EBP + -0x4],EDX
005d3166  eb18                  JMP 0x005d3180
005d3168  837d0c00              CMP dword ptr [EBP + 0xc],0x0
005d316c  7512                  JNZ 0x005d3180
005d316e  b801000000            MOV EAX,0x1
005d3173  85c0                  TEST EAX,EAX
005d3175  7509                  JNZ 0x005d3180
005d3177  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005d317a  83e100                AND ECX,0x0
005d317d  894dfc                MOV dword ptr [EBP + -0x4],ECX
005d3180  837dfc01              CMP dword ptr [EBP + -0x4],0x1
005d3184  0f8521010000          JNZ 0x005d32ab
005d318a  8b5508                MOV EDX,dword ptr [EBP + 0x8]
005d318d  8b420c                MOV EAX,dword ptr [EDX + 0xc]
005d3190  8945c4                MOV dword ptr [EBP + -0x3c],EAX
005d3193  8b0dd0b4a100          MOV ECX,dword ptr [0x00a1b4d0]
005d3199  894dc0                MOV dword ptr [EBP + -0x40],ECX
005d319c  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
005d319f  52                    PUSH EDX
005d31a0  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
005d31a3  8b4834                MOV ECX,dword ptr [EAX + 0x34]
005d31a6  e855201000            CALL 0x006d5200
005d31ab  8945e0                MOV dword ptr [EBP + -0x20],EAX
005d31ae  837de000              CMP dword ptr [EBP + -0x20],0x0
005d31b2  747b                  JZ 0x005d322f
005d31b4  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005d31b7  8a5108                MOV DL,byte ptr [ECX + 0x8]
005d31ba  8855bf                MOV byte ptr [EBP + -0x41],DL
005d31bd  0fb645bf              MOVZX EAX,byte ptr [EBP + -0x41]
005d31c1  83f808                CMP EAX,0x8
005d31c4  7469                  JZ 0x005d322f
005d31c6  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005d31c9  8a5109                MOV DL,byte ptr [ECX + 0x9]
005d31cc  8855be                MOV byte ptr [EBP + -0x42],DL
005d31cf  0fb645be              MOVZX EAX,byte ptr [EBP + -0x42]
005d31d3  85c0                  TEST EAX,EAX
005d31d5  7458                  JZ 0x005d322f
005d31d7  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d31da  8b913c050000          MOV EDX,dword ptr [ECX + 0x53c]
005d31e0  8955e4                MOV dword ptr [EBP + -0x1c],EDX
005d31e3  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005d31e6  8b480c                MOV ECX,dword ptr [EAX + 0xc]
005d31e9  894db8                MOV dword ptr [EBP + -0x48],ECX
005d31ec  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
005d31f2  8955b4                MOV dword ptr [EBP + -0x4c],EDX
005d31f5  8b45b8                MOV EAX,dword ptr [EBP + -0x48]
005d31f8  50                    PUSH EAX
005d31f9  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
005d31fc  8b4934                MOV ECX,dword ptr [ECX + 0x34]
005d31ff  e8fc1f1000            CALL 0x006d5200
005d3204  8945b0                MOV dword ptr [EBP + -0x50],EAX
005d3207  8b55b0                MOV EDX,dword ptr [EBP + -0x50]
005d320a  8a421b                MOV AL,byte ptr [EDX + 0x1b]
005d320d  8845af                MOV byte ptr [EBP + -0x51],AL
005d3210  0fb64daf              MOVZX ECX,byte ptr [EBP + -0x51]
005d3214  894de8                MOV dword ptr [EBP + -0x18],ECX
005d3217  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
005d321a  2b55e4                SUB EDX,dword ptr [EBP + -0x1c]
005d321d  8955ec                MOV dword ptr [EBP + -0x14],EDX
005d3220  837dec01              CMP dword ptr [EBP + -0x14],0x1
005d3224  7e09                  JLE 0x005d322f
005d3226  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d3229  83e000                AND EAX,0x0
005d322c  8945fc                MOV dword ptr [EBP + -0x4],EAX
005d322f  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
005d3236  c745f800000000        MOV dword ptr [EBP + -0x8],0x0
005d323d  eb09                  JMP 0x005d3248
005d323f  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
005d3242  83c101                ADD ECX,0x1
005d3245  894df8                MOV dword ptr [EBP + -0x8],ECX
005d3248  837df814              CMP dword ptr [EBP + -0x8],0x14
005d324c  7d33                  JGE 0x005d3281
005d324e  ba01000000            MOV EDX,0x1
005d3253  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
005d3256  d3e2                  SHL EDX,CL
005d3258  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
005d325b  235004                AND EDX,dword ptr [EAX + 0x4]
005d325e  f7da                  NEG EDX
005d3260  1bd2                  SBB EDX,EDX
005d3262  f7da                  NEG EDX
005d3264  83fa01                CMP EDX,0x1
005d3267  7516                  JNZ 0x005d327f
005d3269  ba01000000            MOV EDX,0x1
005d326e  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
005d3271  d3e2                  SHL EDX,CL
005d3273  8955f4                MOV dword ptr [EBP + -0xc],EDX
005d3276  c745f001000000        MOV dword ptr [EBP + -0x10],0x1
005d327d  eb02                  JMP 0x005d3281
005d327f  ebbe                  JMP 0x005d323f
005d3281  837df001              CMP dword ptr [EBP + -0x10],0x1
005d3285  7524                  JNZ 0x005d32ab
005d3287  b801000000            MOV EAX,0x1
005d328c  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
005d328f  d3e0                  SHL EAX,CL
005d3291  50                    PUSH EAX
005d3292  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005d3295  51                    PUSH ECX
005d3296  8b4da8                MOV ECX,dword ptr [EBP + -0x58]
005d3299  e8a2a7feff            CALL 0x005bda40
005d329e  85c0                  TEST EAX,EAX
005d32a0  7509                  JNZ 0x005d32ab
005d32a2  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
005d32a5  83e200                AND EDX,0x0
005d32a8  8955fc                MOV dword ptr [EBP + -0x4],EDX
005d32ab  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005d32ae  5e                    POP ESI
005d32af  8be5                  MOV ESP,EBP
005d32b1  5d                    POP EBP
005d32b2  c20c00                RET 0xc
