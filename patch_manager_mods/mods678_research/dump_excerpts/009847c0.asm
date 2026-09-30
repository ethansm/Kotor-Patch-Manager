; FN 009847c0 FUN_009847c0
009847c0  55                    PUSH EBP
009847c1  8bec                  MOV EBP,ESP
009847c3  d905a0179a00          FLD float ptr [0x009a17a0]
009847c9  d91dd0fda700          FSTP float ptr [0x00a7fdd0]
009847cf  d9059cba9800          FLD float ptr [0x0098ba9c]
009847d5  d91dd4fda700          FSTP float ptr [0x00a7fdd4]
009847db  d9ee                  FLDZ
009847dd  d91dd8fda700          FSTP float ptr [0x00a7fdd8]
009847e3  5d                    POP EBP
009847e4  c3                    RET
