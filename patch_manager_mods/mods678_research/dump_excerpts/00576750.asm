; FN 00576750 FUN_00576750
00576750  55                    PUSH EBP
00576751  8bec                  MOV EBP,ESP
00576753  6aff                  PUSH -0x1
00576755  68b30f9500            PUSH 0x950fb3
0057675a  64a100000000          MOV EAX,FS:[0x0]
00576760  50                    PUSH EAX
00576761  81ecac000000          SUB ESP,0xac
00576767  a1201fa100            MOV EAX,[0x00a11f20]
0057676c  33c5                  XOR EAX,EBP
0057676e  50                    PUSH EAX
0057676f  8d45f4                LEA EAX,[EBP + -0xc]
00576772  64a300000000          MOV FS:[0x0],EAX
00576778  898d4cffffff          MOV dword ptr [EBP + 0xffffff4c],ECX
0057677e  a1a4b4a100            MOV EAX,[0x00a1b4a4]
00576783  8b4808                MOV ECX,dword ptr [EAX + 0x8]
00576786  894dbc                MOV dword ptr [EBP + -0x44],ECX
00576789  8b4dbc                MOV ECX,dword ptr [EBP + -0x44]
0057678c  e8ef5afaff            CALL 0x0051c280
00576791  8945f0                MOV dword ptr [EBP + -0x10],EAX
00576794  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
0057679a  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
005767a0  8945b8                MOV dword ptr [EBP + -0x48],EAX
005767a3  8b4db8                MOV ECX,dword ptr [EBP + -0x48]
005767a6  8b516c                MOV EDX,dword ptr [ECX + 0x6c]
005767a9  8955b4                MOV dword ptr [EBP + -0x4c],EDX
005767ac  837db401              CMP dword ptr [EBP + -0x4c],0x1
005767b0  0f85f3000000          JNZ 0x005768a9
005767b6  c745e801000000        MOV dword ptr [EBP + -0x18],0x1
005767bd  6a04                  PUSH 0x4
005767bf  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
005767c5  50                    PUSH EAX
005767c6  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
005767c9  e85279faff            CALL 0x0051e120
005767ce  c745e000000000        MOV dword ptr [EBP + -0x20],0x0
005767d5  eb09                  JMP 0x005767e0
005767d7  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
005767da  83c101                ADD ECX,0x1
005767dd  894de0                MOV dword ptr [EBP + -0x20],ECX
005767e0  837de014              CMP dword ptr [EBP + -0x20],0x14
005767e4  7351                  JNC 0x00576837
005767e6  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
005767ec  8b8250110000          MOV EAX,dword ptr [EDX + 0x1150]
005767f2  8945b0                MOV dword ptr [EBP + -0x50],EAX
005767f5  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005767f8  51                    PUSH ECX
005767f9  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
005767fc  e81f9e1500            CALL 0x006d0620
00576801  8945e4                MOV dword ptr [EBP + -0x1c],EAX
00576804  837de400              CMP dword ptr [EBP + -0x1c],0x0
00576808  7422                  JZ 0x0057682c
0057680a  ba01000000            MOV EDX,0x1
0057680f  83e201                AND EDX,0x1
00576812  c1e203                SHL EDX,0x3
00576815  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
00576818  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
0057681e  83e1f7                AND ECX,0xfffffff7
00576821  0bca                  OR ECX,EDX
00576823  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
00576826  898ac8020000          MOV dword ptr [EDX + 0x2c8],ECX
0057682c  8b45e8                MOV EAX,dword ptr [EBP + -0x18]
0057682f  0345e8                ADD EAX,dword ptr [EBP + -0x18]
00576832  8945e8                MOV dword ptr [EBP + -0x18],EAX
00576835  eba0                  JMP 0x005767d7
00576837  c745e000000000        MOV dword ptr [EBP + -0x20],0x0
0057683e  eb09                  JMP 0x00576849
00576840  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00576843  83c101                ADD ECX,0x1
00576846  894de0                MOV dword ptr [EBP + -0x20],ECX
00576849  6a01                  PUSH 0x1
0057684b  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576851  e86a3a0100            CALL 0x0058a2c0
00576856  8945a8                MOV dword ptr [EBP + -0x58],EAX
00576859  8b55a8                MOV EDX,dword ptr [EBP + -0x58]
0057685c  8b4210                MOV EAX,dword ptr [EDX + 0x10]
0057685f  8945ac                MOV dword ptr [EBP + -0x54],EAX
00576862  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
00576865  3b4dac                CMP ECX,dword ptr [EBP + -0x54]
00576868  733f                  JNC 0x005768a9
0057686a  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
0057686d  52                    PUSH EDX
0057686e  6a01                  PUSH 0x1
00576870  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576876  e8453a0100            CALL 0x0058a2c0
0057687b  8bc8                  MOV ECX,EAX
0057687d  e87ef80800            CALL 0x00606100
00576882  8945e4                MOV dword ptr [EBP + -0x1c],EAX
00576885  b801000000            MOV EAX,0x1
0057688a  83e001                AND EAX,0x1
0057688d  c1e003                SHL EAX,0x3
00576890  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
00576893  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
00576899  83e2f7                AND EDX,0xfffffff7
0057689c  0bd0                  OR EDX,EAX
0057689e  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
005768a1  8990c8020000          MOV dword ptr [EAX + 0x2c8],EDX
005768a7  eb97                  JMP 0x00576840
005768a9  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
005768af  8b5104                MOV EDX,dword ptr [ECX + 0x4]
005768b2  8955a4                MOV dword ptr [EBP + -0x5c],EDX
005768b5  a1a4b4a100            MOV EAX,[0x00a1b4a4]
005768ba  8b4808                MOV ECX,dword ptr [EAX + 0x8]
005768bd  894da0                MOV dword ptr [EBP + -0x60],ECX
005768c0  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
005768c3  52                    PUSH EDX
005768c4  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
005768ca  83c018                ADD EAX,0x18
005768cd  50                    PUSH EAX
005768ce  8b4da0                MOV ECX,dword ptr [EBP + -0x60]
005768d1  e8ba54faff            CALL 0x0051bd90
005768d6  8bc8                  MOV ECX,EAX
005768d8  e89380feff            CALL 0x0055e970
005768dd  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
005768e3  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005768e9  89559c                MOV dword ptr [EBP + -0x64],EDX
005768ec  8b459c                MOV EAX,dword ptr [EBP + -0x64]
005768ef  8b486c                MOV ECX,dword ptr [EAX + 0x6c]
005768f2  894d98                MOV dword ptr [EBP + -0x68],ECX
005768f5  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
005768fb  8b4598                MOV EAX,dword ptr [EBP + -0x68]
005768fe  8982e8100000          MOV dword ptr [EDX + 0x10e8],EAX
00576904  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
0057690a  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0057690d  895594                MOV dword ptr [EBP + -0x6c],EDX
00576910  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
00576913  e86863faff            CALL 0x0051cc80
00576918  8945ec                MOV dword ptr [EBP + -0x14],EAX
0057691b  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
00576921  8b88e8100000          MOV ECX,dword ptr [EAX + 0x10e8]
00576927  894d90                MOV dword ptr [EBP + -0x70],ECX
0057692a  837d9001              CMP dword ptr [EBP + -0x70],0x1
0057692e  7556                  JNZ 0x00576986
00576930  837dec00              CMP dword ptr [EBP + -0x14],0x0
00576934  7432                  JZ 0x00576968
00576936  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00576939  8b4218                MOV EAX,dword ptr [EDX + 0x18]
0057693c  89458c                MOV dword ptr [EBP + -0x74],EAX
0057693f  837d8c00              CMP dword ptr [EBP + -0x74],0x0
00576943  7423                  JZ 0x00576968
00576945  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
0057694b  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0057694e  895588                MOV dword ptr [EBP + -0x78],EDX
00576951  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00576954  8b4818                MOV ECX,dword ptr [EAX + 0x18]
00576957  894d84                MOV dword ptr [EBP + -0x7c],ECX
0057695a  6a00                  PUSH 0x0
0057695c  8b5588                MOV EDX,dword ptr [EBP + -0x78]
0057695f  52                    PUSH EDX
00576960  8b4d84                MOV ECX,dword ptr [EBP + -0x7c]
00576963  e8c8c91500            CALL 0x006d3330
00576968  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
0057696e  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
00576974  894d80                MOV dword ptr [EBP + -0x80],ECX
00576977  8b5580                MOV EDX,dword ptr [EBP + -0x80]
0057697a  c7427c00000000        MOV dword ptr [EDX + 0x7c],0x0
00576981  e923010000            JMP 0x00576aa9
00576986  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
0057698c  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
00576992  898d7cffffff          MOV dword ptr [EBP + 0xffffff7c],ECX
00576998  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
0057699e  8b4278                MOV EAX,dword ptr [EDX + 0x78]
005769a1  8945dc                MOV dword ptr [EBP + -0x24],EAX
005769a4  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
005769aa  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005769b0  899578ffffff          MOV dword ptr [EBP + 0xffffff78],EDX
005769b6  8b8578ffffff          MOV EAX,dword ptr [EBP + 0xffffff78]
005769bc  c74078ffffffff        MOV dword ptr [EAX + 0x78],0xffffffff
005769c3  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
005769c6  51                    PUSH ECX
005769c7  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
005769ca  e8c1da0e00            CALL 0x00664490
005769cf  8945d8                MOV dword ptr [EBP + -0x28],EAX
005769d2  837dd800              CMP dword ptr [EBP + -0x28],0x0
005769d6  7443                  JZ 0x00576a1b
005769d8  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
005769de  8b4204                MOV EAX,dword ptr [EDX + 0x4]
005769e1  898574ffffff          MOV dword ptr [EBP + 0xffffff74],EAX
005769e7  6a00                  PUSH 0x0
005769e9  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
005769ef  51                    PUSH ECX
005769f0  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
005769f3  e838c91500            CALL 0x006d3330
005769f8  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
005769fe  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
00576a04  898570ffffff          MOV dword ptr [EBP + 0xffffff70],EAX
00576a0a  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
00576a10  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
00576a13  89517c                MOV dword ptr [ECX + 0x7c],EDX
00576a16  e98e000000            JMP 0x00576aa9
00576a1b  8d4dd0                LEA ECX,[EBP + -0x30]
00576a1e  e81dcb1b00            CALL 0x00733540
00576a23  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00576a2a  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
00576a2d  50                    PUSH EAX
00576a2e  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576a34  83c118                ADD ECX,0x18
00576a37  e8d4d51b00            CALL 0x00734010
00576a3c  50                    PUSH EAX
00576a3d  68f0549900            PUSH 0x9954f0
00576a42  8d4dd0                LEA ECX,[EBP + -0x30]
00576a45  51                    PUSH ECX
00576a46  e825d81b00            CALL 0x00734270
00576a4b  83c410                ADD ESP,0x10
00576a4e  6a01                  PUSH 0x1
00576a50  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00576a53  e838da0e00            CALL 0x00664490
00576a58  8945cc                MOV dword ptr [EBP + -0x34],EAX
00576a5b  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
00576a61  8b4204                MOV EAX,dword ptr [EDX + 0x4]
00576a64  89856cffffff          MOV dword ptr [EBP + 0xffffff6c],EAX
00576a6a  6a00                  PUSH 0x0
00576a6c  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
00576a72  51                    PUSH ECX
00576a73  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
00576a76  e8b5c81500            CALL 0x006d3330
00576a7b  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
00576a81  8b8298110000          MOV EAX,dword ptr [EDX + 0x1198]
00576a87  898568ffffff          MOV dword ptr [EBP + 0xffffff68],EAX
00576a8d  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
00576a93  c7417c01000000        MOV dword ptr [ECX + 0x7c],0x1
00576a9a  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00576aa1  8d4dd0                LEA ECX,[EBP + -0x30]
00576aa4  e8d7cc1b00            CALL 0x00733780
00576aa9  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576aaf  e82c920000            CALL 0x0057fce0
00576ab4  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
00576aba  c7821402000001000000  MOV dword ptr [EDX + 0x214],0x1
00576ac4  688c000000            PUSH 0x8c
00576ac9  e8552c3a00            CALL 0x00919723
00576ace  83c404                ADD ESP,0x4
00576ad1  8945c0                MOV dword ptr [EBP + -0x40],EAX
00576ad4  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1
00576adb  837dc000              CMP dword ptr [EBP + -0x40],0x0
00576adf  7412                  JZ 0x00576af3
00576ae1  6a01                  PUSH 0x1
00576ae3  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
00576ae6  e835de0600            CALL 0x005e4920
00576aeb  898548ffffff          MOV dword ptr [EBP + 0xffffff48],EAX
00576af1  eb0a                  JMP 0x00576afd
00576af3  c78548ffffff00000000  MOV dword ptr [EBP + 0xffffff48],0x0
00576afd  8b8548ffffff          MOV EAX,dword ptr [EBP + 0xffffff48]
00576b03  8945c4                MOV dword ptr [EBP + -0x3c],EAX
00576b06  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00576b0d  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00576b10  894dc8                MOV dword ptr [EBP + -0x38],ECX
00576b13  8b152c609900          MOV EDX,dword ptr [0x0099602c]
00576b19  899564ffffff          MOV dword ptr [EBP + 0xffffff64],EDX
00576b1f  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
00576b22  668b8d64ffffff        MOV CX,word ptr [EBP + 0xffffff64]
00576b29  66894808              MOV word ptr [EAX + 0x8],CX
00576b2d  668b15bc679900        MOV DX,word ptr [0x009967bc]
00576b34  66899562ffffff        MOV word ptr [EBP + 0xffffff62],DX
00576b3b  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
00576b3e  0fb7480a              MOVZX ECX,word ptr [EAX + 0xa]
00576b42  83e1f8                AND ECX,0xfffffff8
00576b45  0fb79562ffffff        MOVZX EDX,word ptr [EBP + 0xffffff62]
00576b4c  0bca                  OR ECX,EDX
00576b4e  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
00576b51  6689480a              MOV word ptr [EAX + 0xa],CX
00576b55  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576b5b  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00576b5e  89955cffffff          MOV dword ptr [EBP + 0xffffff5c],EDX
00576b64  8b855cffffff          MOV EAX,dword ptr [EBP + 0xffffff5c]
00576b6a  50                    PUSH EAX
00576b6b  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00576b6e  e82df30600            CALL 0x005e5ea0
00576b73  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576b79  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
00576b7f  668b82e0000000        MOV AX,word ptr [EDX + 0xe0]
00576b86  6689855affffff        MOV word ptr [EBP + 0xffffff5a],AX
00576b8d  0fb78d5affffff        MOVZX ECX,word ptr [EBP + 0xffffff5a]
00576b94  51                    PUSH ECX
00576b95  6a00                  PUSH 0x0
00576b97  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
00576b9a  e8e1e20600            CALL 0x005e4e80
00576b9f  6a00                  PUSH 0x0
00576ba1  6a00                  PUSH 0x0
00576ba3  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
00576ba6  52                    PUSH EDX
00576ba7  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576bad  e85ed6fcff            CALL 0x00544210
00576bb2  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
00576bb8  c7801402000000000000  MOV dword ptr [EAX + 0x214],0x0
00576bc2  6a00                  PUSH 0x0
00576bc4  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576bca  8b11                  MOV EDX,dword ptr [ECX]
00576bcc  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576bd2  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
00576bd8  ffd0                  CALL EAX
00576bda  0fbfc8                MOVSX ECX,AX
00576bdd  83f901                CMP ECX,0x1
00576be0  7d72                  JGE 0x00576c54
00576be2  6818270000            PUSH 0x2718
00576be7  8b954cffffff          MOV EDX,dword ptr [EBP + 0xffffff4c]
00576bed  8b02                  MOV EAX,dword ptr [EDX]
00576bef  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576bf5  8b507c                MOV EDX,dword ptr [EAX + 0x7c]
00576bf8  ffd2                  CALL EDX
00576bfa  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
00576c00  c780ec00000000000000  MOV dword ptr [EAX + 0xec],0x0
00576c0a  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576c10  8b91e8100000          MOV EDX,dword ptr [ECX + 0x10e8]
00576c16  899554ffffff          MOV dword ptr [EBP + 0xffffff54],EDX
00576c1c  83bd54ffffff00        CMP dword ptr [EBP + 0xffffff54],0x0
00576c23  7420                  JZ 0x00576c45
00576c25  6a00                  PUSH 0x0
00576c27  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
00576c2d  8b10                  MOV EDX,dword ptr [EAX]
00576c2f  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576c35  8b829c000000          MOV EAX,dword ptr [EDX + 0x9c]
00576c3b  ffd0                  CALL EAX
00576c3d  0fbfc8                MOVSX ECX,AX
00576c40  83f9f6                CMP ECX,-0xa
00576c43  7f0f                  JG 0x00576c54
00576c45  33d2                  XOR EDX,EDX
00576c47  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
00576c4d  66899014110000        MOV word ptr [EAX + 0x1114],DX
00576c54  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576c5a  8b91e8100000          MOV EDX,dword ptr [ECX + 0x10e8]
00576c60  899550ffffff          MOV dword ptr [EBP + 0xffffff50],EDX
00576c66  83bd50ffffff01        CMP dword ptr [EBP + 0xffffff50],0x1
00576c6d  7510                  JNZ 0x00576c7f
00576c6f  8b854cffffff          MOV EAX,dword ptr [EBP + 0xffffff4c]
00576c75  c780f400000001000000  MOV dword ptr [EAX + 0xf4],0x1
00576c7f  6a00                  PUSH 0x0
00576c81  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00576c87  e8c4920000            CALL 0x0057ff50
00576c8c  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00576c8f  64890d00000000        MOV dword ptr FS:[0x0],ECX
00576c96  59                    POP ECX
00576c97  8be5                  MOV ESP,EBP
00576c99  5d                    POP EBP
00576c9a  c3                    RET
