from hookcheck import *
F = [("ExoString default ctor",0x4EAED0,0x733540),("ExoString(char*) ctor",0x605680,0x733570),("ExoString dtor",0x605890,0x733780),
("C2DA ctor",0x6207E0,0x71AD40),("C2DA dtor",0x620920,0x71AE80),("C2DA Load",0x621E70,0x71C3D0),("C2DA GetString",0x620C70,0x71B1D0)]
def ret(read, va):
    b = read(va, 1200)
    # first c3 / c2 nn 00 after an epilogue (5d or 8b e5 5d)
    for i in range(len(b)-3):
        if b[i] == 0xC2 and b[i+2] == 0 and b[i-1] in (0x5D,0x5E,0x5B,0x5F,0x59) : return f"ret {b[i+1]:#x} at +{i:#x}"
        if b[i] == 0xC3 and b[i-1] in (0x5D,0x5E,0x5B,0x5F,0x59): return f"ret at +{i:#x}"
for n,g,s in F:
    gb = gog_read(g, 10); sb = steam_read(s, 10)
    print(f"{n}: GOG {g:#x} align16={g%16==0} [{gb.hex(' ')}] {ret(gog_read,g)} | Steam {s:#x} align16={s%16==0} [{sb.hex(' ')}] {ret(steam_read,s)}")
