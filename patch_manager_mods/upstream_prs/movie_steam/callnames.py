import re, subprocess, sys, struct
from hookcheck import *
def names(which, start, size):
    out = subprocess.run(["python3","/home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/disasm_va.py",which,f"{start:X}","--len",str(size)],capture_output=True,text=True).stdout
    res = []
    for l in out.splitlines():
        m = re.search(r"call\s+DWORD PTR ds:0x([0-9a-f]+)", l)
        if m:
            slot = int(m.group(1),16); rd = gog_read if which=="gog" else steam_read
            v = struct.unpack("<I", rd(slot,4))[0]
            nm = rd(base+v+2, 48).split(b"\0")[0].decode()
            res.append((slot, nm))
    return res
a = names("gog", int(sys.argv[1],16), int(sys.argv[3])); b = names("steam", int(sys.argv[2],16), int(sys.argv[3]))
print("gog  :", [n for _,n in a]); print("steam:", [n for _,n in b]); print("same names in order:", [n for _,n in a]==[n for _,n in b])
print("steam slots:", [hex(s) for s,_ in b])
