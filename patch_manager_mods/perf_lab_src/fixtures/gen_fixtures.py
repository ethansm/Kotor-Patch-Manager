#!/usr/bin/env python3
"""Regenerates fixtures/game/ (a miniature fake KOTOR II game dir) for test_analyze_runs.py.

4 launches (GetTickCount based, see analyze_runs.py docstring for the matching rule):
  1  pid 100  perf-lab 0.3.0  healthy, first load 5000 ms, 2 module-unload loads, 145 s idle span (S1, S2), texmem DISABLED
  2  pid 101  perf-lab 0.3.0  HANG: LOAD_UP without LOAD_DOWN; robustness FIRED line; activationfix IGNORED x2
  3  pid 102  perf-lab 0.4.0  hwmip=1, texmem patched (tpc_freed=120 kept=3), other_tpc_retained=7 (gate FAIL), hwmip GL fields
  4  pid 103  perf-lab 0.3.0  healthy, first load 6000 ms, 10 quicksave-like pairs + 1 quickload (S3)
Run:  python3 gen_fixtures.py
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "game")
os.makedirs(OUT, exist_ok=True)


def wall(tick):
    s = (36000 + tick // 1000) % 86400
    return "%02d:%02d:%02d.%03d" % (s // 3600, (s // 60) % 60, s % 60, tick % 1000)


def block(n, ev, tick, frames, live, other_ret=0, ctrl_ret=0, kt=(0, 0), tpc=0, level=0, extra_hdr="", frame_hist=None,
          net_max=0, ab=(0, 0), textures=300, mip=(0, 0.0), hw=None):
    """level 0 = header+GL, 1 = + RT/LIVE, 2 = every line type the analyzer reads (+ skipped NAME/BRRING/... lines)."""
    mipmaps, mip_ms = mip
    L = ["#METRICS %d %s tick=%d frames(hook)=%d live_bytes=%d allocs=%d tx=0 g1skip=0%s" % (n, ev, tick, frames, live, frames * 3, extra_hdr)]
    glline = "GL glimage=%d glfinish=%d glfinish_ms=%.1f glfinish_max_ms=%.2f mipmaps=%d mip_ms=%.1f mip_mpix=1.00" % (
        mipmaps, mipmaps, mipmaps * 0.1, 2.5, mipmaps, mip_ms)
    if hw:
        glline += " " + " ".join("%s=%s" % kv for kv in hw)
    if level >= 2:
        L += ["G a1b710=0(1) a7eb9c=0(1) resman=058142a8 budget4=1 budget8=1(1)",
              "# RT restype allocN allocB evictN evictB freeN freeB relN relZero demN redem60 resident",
              "RT 3 10 100 0 0 10 100 10 10 10 0 0"]
    if level >= 1:
        L.append("RT %d 100 %d 0 0 0 0 0 0 0 0 %d" % (3007, tpc * 1048576, tpc * 1048576))
    if level >= 2:
        L += ["GETIMG calls=127 tpc=78 tpc_retained=78 upload=127 ctrl=1 ctrl_tpc_retained=%d other=2 other_tpc_retained=%d" % (ctrl_ret, other_ret),
              "BUILD calls=103 requeue_tpc_retained=%d tpc_with_ctrl=0" % (other_ret + 5)]
    L.append(glline)
    if level >= 2:
        fh = frame_hist or [0, 0, 1, 2, 10, 80, 5, 1, 1, 0]
        L += ["FRAME frames=%d max_ms=120.5 bucket_calls=10 max_bucket_per_frame=7 actionbar_calls=%d max_actionbar_per_frame=%d hist_ms %s allocs_per_frame_hist 0 1 0 0" % (
            sum(fh), ab[0], ab[1], " ".join(map(str, fh))),
              "HOOKCOST_BY ResMalloc calls=405 cycles=6450 samples=6",
              "NAME tga_load 3 screen ra=00000000", "BRRING %d gui_setter1 x ra=0041496c" % tick,
              "KT async_start=0 async_bytes=0 destroy_table=12 still_in_demand=%d uaf_write=%d release_ok=268 txi_via_tpc=57 unload=0" % kt,
              "VM run_script=0 exec_cmd=0 exec_bad_id=0 instr_limit=0 delay_cmd=0",
              "VMCMD 1:5 2:3",
              "NET send=0 bytes=0 max_len=%d ovf_test=0 ovf_hit=0 | SCENE focus_dummy=1 scene_dtor=1" % net_max,
              "VQTOP 3006d000 1024KB state=10000 type=20000 prot=1"]
    if level >= 1:
        L.append("LIVE textures=%d gl_bytes_est=1 models=0 emitters=0" % textures)
    L.append("#END %d" % n)
    return "\n".join(L) + "\n"


files = {k: [] for k in ("perflab_metrics.txt", "perflab_log.txt", "perflab_alloc_snaps.txt", "robustnessfixes_log.txt", "texmem_log.txt",
                         "activationfix_log.txt", "equipiconfix_log.txt")}


def attach(t, ver, extra=""):
    files["perflab_log.txt"].append("[%d t0] perf-lab %s attach exe=00400000-00ab9000 game=1 alloc=1 heapfilter=1 scan=48 metrics=1 vq=1 tx=OFF g1skip=OFF%s\n" % (t, ver, extra))
    files["perflab_log.txt"].append("[%d t1] experiments at first tick: tx=OFF\n" % (t + 2500))
    files["perflab_log.txt"].append("[%d t2027] live=6953325B blocks=10112 sites=1854 dropped=0\n" % (t + 30000))


def snap(n, ev, tick, live):
    files["perflab_alloc_snaps.txt"].append("#SNAP %d %s tick=%d frames(hook)=1 allocs=5 frees=4 reallocs=0 live_bytes=%d live_blocks=9000 total_bytes=10\nS 0000a1b2 00000000 100 2\n" % (n, ev, tick, live))


def textlogs(t, pid, rob, tex, act, ei=True, fired=None, ignored=0, rob_extra=""):
    tl = lambda name, tick, msg: files[name].append("[%s %d] %s\n" % (wall(tick), tick, msg))
    tl("robustnessfixes_log.txt", t + 143, "k2-robustness-fixes 1.1.0 loaded: 6/6 groups patched: kt=ON vmid=ON sndri=ON netovf=ON guiid=ON abbound=ON cave=054e0000 pid=%d" % pid)
    tl("robustnessfixes_log.txt", t + 5143, "COUNTERS kt_destroy_orphan=0 kt_update_orphan=0 ab_shrink_clamp=0 sndri_restart=%d vm_reject=0 vm_neg=%d vm_last_id=%d gui_reject=0 gui_last_id=0 net_drop=0 net_last_len=0" % (
        1 if fired else 0, 1 if fired else 0, 77 if fired else 0))
    if fired:
        tl("robustnessfixes_log.txt", t + 5144, "FIRED vmid neg=1 last_id=77")
        tl("robustnessfixes_log.txt", t + 5145, "FIRED sndri restart=1")
    msg = {"DISABLED": "k2-texture-memory-release 1.0.0 loaded: DISABLED by texmem.txt (site 0x004261cf untouched) pid=%d",
           "patched": "k2-texture-memory-release 1.0.0 loaded: site 0x004261cf patched (cave 054e0000) pid=%d"}[tex]
    tl("texmem_log.txt", t + 150, msg % pid)
    if tex == "patched":
        tl("texmem_log.txt", t + 30150, "COUNTERS tpc_freed=60 tpc_kept_controller=1")
        tl("texmem_log.txt", t + 60150, "COUNTERS tpc_freed=120 tpc_kept_controller=3")
    tl("activationfix_log.txt", t + 60, "k2-activation-fix 1.0.0 loaded: site 0x0040d69e patched (CALL [PostMessageA] -> ActFixMoviePost 795f15a0) pid=%d" % pid)
    for i in range(ignored):
        tl("activationfix_log.txt", t + 8000 + i * 10, "IGNORED synthetic WM_ACTIVATEAPP(0,0) post from movie thread tid=424 hwnd=000100de fg=000100de fgPid=%d (ours) a1b4e0=0 76c=1 n=%d" % (pid, i + 1))
    tl("equipiconfix_log.txt", t + 70, "k2-equip-icon-fix 1.0.0 loaded: 8 sites patched (pass 0x8ABA8C) pid=%d" % pid)
    tl("equipiconfix_log.txt", t + 10070, "STATS passes=5 deferred=9 cancelled=0 replaced=1 applied=4 sets=12 outside_pass=0")


M = files["perflab_metrics.txt"]
MB = 1048576
# ---- launch 1: healthy; 2 module-unload loads; 145 s idle span after the first load
t0 = 1_000_000
attach(t0, "0.3.0")
seq = [("FLUSH", 1_030_000, 7.0, 0, 80), ("LOAD_UP", 1_040_000, 7.0, 0, 80), ("LOAD_DOWN", 1_045_000, 257.0, 1, 190), ("LOAD_UP", 1_190_000, 258, 0, 190),
       ("UNLOAD", 1_190_500, 120, 2, 190), ("LOAD_DOWN", 1_195_000, 250.0, 1, 190), ("LOAD_UP", 1_210_000, 250, 0, 190), ("UNLOAD", 1_210_400, 120, 2, 190),
       ("LOAD_DOWN", 1_214_000, 251.0, 1, 190), ("FLUSH", 1_225_000, 251, 2, 190)]
mips = {1_040_000: (0, 0.0), 1_045_000: (60, 500.0), 1_190_000: (60, 500.0), 1_195_000: (62, 520.0), 1_210_000: (62, 520.0), 1_214_000: (63, 524.0)}
for n, (ev, t, liveMB, lvl, tpc) in enumerate(seq, 1):
    M.append(block(n, ev, t, t // 100, int(liveMB * MB), tpc=tpc, level=lvl, kt=(0, 2) if t > 1_190_000 else (0, 0), mip=mips.get(t, (63, 524.0)), ab=(100, 3), net_max=14035))
    if ev in ("LOAD_UP", "LOAD_DOWN", "UNLOAD"):
        snap(n, ev, t - 2, int(liveMB * MB))
textlogs(t0, 100, True, "DISABLED", True, ignored=0)

# ---- launch 2: hang (UP, no DOWN)
t0 = 2_000_000
attach(t0, "0.3.0")
for n, (ev, t, lvl) in enumerate([("FLUSH", 2_030_000, 0), ("LOAD_UP", 2_040_000, 0), ("FLUSH", 2_090_000, 2)], 1):
    M.append(block(n, ev, t, t // 100, 7 * MB, tpc=1, level=lvl))
textlogs(t0, 101, True, "DISABLED", True, fired=True, ignored=2)

# ---- launch 3: hwmip=1, texmem patched, other_tpc_retained > 0
t0 = 3_000_000
attach(t0, "0.4.0", " hwmip=ON")
hw0 = [("hwmip_calls", 0), ("hwmip_hw", 0), ("hwmip_fallback_npot", 0), ("hwmip_glerror", 0), ("hwmip_cube", 0)]
hw1 = [("hwmip_calls", 40), ("hwmip_hw", 38), ("hwmip_fallback_npot", 2), ("hwmip_glerror", 0), ("hwmip_cube", 1)]
x = " hwmip=1"
M.append(block(1, "LOAD_UP", 3_040_000, 200, 7 * MB, tpc=2, extra_hdr=x, hw=hw0, level=1))
M.append(block(2, "LOAD_DOWN", 3_043_000, 400, 120 * MB, tpc=60, extra_hdr=x, mip=(60, 100.0), hw=hw1, level=1))
M.append(block(3, "FLUSH", 3_073_000, 500, 121 * MB, tpc=61, other_ret=7, ctrl_ret=1, extra_hdr=x, mip=(60, 100.0), hw=hw1, level=2, kt=(1, 0)))
textlogs(3_000_000, 102, True, "patched", True)
snap(1, "LOAD_UP", 3_039_999, 7 * MB)

# ---- launch 4: healthy, first load 6000 ms, S3 burst (10 short pairs + 1 pair with UNLOAD = quickload)
t0 = 4_000_000
attach(t0, "0.3.0")
M.append(block(1, "FLUSH", 4_030_000, 100, 7 * MB))
M.append(block(2, "LOAD_UP", 4_040_000, 200, 7 * MB))
M.append(block(3, "LOAD_DOWN", 4_046_000, 400, 250 * MB, tpc=190, mip=(60, 600.0), level=1))
n = 4
for k in range(10):
    t = 4_060_000 + 1500 * k
    M.append(block(n, "LOAD_UP", t, 500 + k * 10, 250 * MB, mip=(60, 600.0))); n += 1
    M.append(block(n, "LOAD_DOWN", t + 500, 505 + k * 10, 250 * MB, mip=(60, 600.0))); n += 1
t = 4_075_000
M.append(block(n, "LOAD_UP", t, 700, 250 * MB, mip=(60, 600.0))); n += 1
M.append(block(n, "UNLOAD", t + 200, 701, 120 * MB, mip=(60, 600.0))); n += 1
M.append(block(n, "LOAD_DOWN", t + 500, 702, 250 * MB, mip=(60, 600.0))); n += 1
M.append(block(n, "FLUSH", 4_100_000, 900, 250 * MB, tpc=190, mip=(60, 600.0), level=2)); n += 1
textlogs(t0, 103, True, "DISABLED", True)

for name, parts in files.items():
    with open(os.path.join(OUT, name), "w", newline="") as f:
        f.write("".join(parts))
with open(os.path.join(OUT, "perflab_debug.txt"), "w") as f:
    f.write("alloc=1\nmetrics=1\nvq=1\nflush_ms=30000\n")
with open(os.path.join(OUT, "texmem.txt"), "w") as f:
    f.write("enabled=0\n")
