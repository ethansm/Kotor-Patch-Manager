#!/usr/bin/env python3
"""Tests for scripts/selfheal_log_check.py (fixtures in scripts/testdata/selfheal/).
Run: .venv/bin/python -B -m unittest scripts/test_selfheal_log_check.py   (or python3 -m pytest where pytest is installed)"""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import selfheal_log_check as S  # noqa: E402

FX = HERE / "testdata" / "selfheal"


def lines(name):
    return (FX / name).read_text().splitlines()


def test_pass():
    r = S.check(lines("pass.txt"))
    assert (r["p1"], r["p3"], len(r["blocks"]), r["verdict"]) == ("PASS", "PASS", 1, "PASS")
    assert len(r["table"]) == 2 and r["table"][0]["pick"][1] == 65
    assert S.fmt(r).splitlines()[-1] == "SELFHEAL P1=PASS P3=PASS DUMP=1 blocks VERDICT=PASS"


def test_fail_knone():
    r = S.check(lines("fail_knone.txt"))
    assert r["p3"] == "FAIL" and r["verdict"] == "FAIL" and r["p3_fail"]


def test_fail_nop1():
    r = S.check(lines("fail_nop1.txt"))
    assert r["p1"] == "MISSING" and r["verdict"] == "FAIL"
    assert S.fmt(r).splitlines()[-1] == "SELFHEAL P1=MISSING P3=PASS DUMP=1 blocks VERDICT=FAIL"


def test_incomplete():
    r = S.check([l for l in lines("pass.txt") if "DUMP" not in l and not l.startswith(("9001", "9002"))
                 and " member " not in l])
    assert r["verdict"] == "INCOMPLETE" and r["p1"] == "PASS" and not r["blocks"]


def test_from_skips_earlier_lines():
    r = S.check(lines("pass.txt"), lo=6)  # drops connect + P1 (+P3 slots)
    assert r["p1"] == "MISSING"
    r = S.check(lines("pass.txt"), lo=1, hi=5)
    assert r["p1"] == "PASS" and r["verdict"] == "INCOMPLETE"


def test_dump_failures():
    L = lines("pass.txt")
    L = [l.replace("hp=12/40", "hp=50/40").replace("row=84 heal=17", "row=99 heal=17").replace("row=65 heal=27", "row=65 heal=0").replace("idOk=1", "idOk=0") for l in L]
    r = S.check(L)
    msgs = " ".join(m for _, m in r["dump_fail"])
    assert "cur 50 > max 40" in msgs and "row=99" in msgs and "heal row 65 but heal=0" in msgs and "idOk=0" in msgs
    assert "Frag Grenade" not in msgs  # non-heal item (row=-1 heal=-1) is skipped
    assert r["verdict"] == "FAIL" and all(isinstance(n, int) for n, _ in r["dump_fail"])


def test_malformed_numbers():
    L = [l.replace("ctlCount=212", "ctlCount=xyz").replace("hp=30/30", "hp=ab/30") for l in lines("pass.txt")]
    r = S.check(L)
    assert r["p1"] == "FAIL" and r["verdict"] == "FAIL"
    assert any("malformed hp" in m for _, m in r["dump_fail"])
    assert S.check(["P1 hud built layoutOpen=1 ctlCount=5000"])["p1"] == "FAIL"
    L = [l.replace("row=65 heal=27", "row=?? heal=27") for l in lines("pass.txt")]
    assert any("malformed row" in m for _, m in S.check(L)["dump_fail"])


def test_bare_lines_without_prefix():
    r = S.check([l.split(" ", 2)[2] if l[0].isdigit() else l for l in lines("pass.txt")])
    assert r["verdict"] == "PASS"


def test_missing_file():
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / "nope.txt"
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            assert S.main([str(p)]) == 2
        assert out.getvalue().strip() == "no log: %s" % p


def test_main_exit_codes():
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        assert S.main(["--log", str(FX / "pass.txt")]) == 0
        assert S.main([str(FX / "fail_knone.txt")]) == 1
        assert S.main(["--log", str(FX / "pass.txt"), "--from", "99"]) == 0
    assert "VERDICT=INCOMPLETE" in out.getvalue()


def v30(extra):
    return lines("pass.txt") + extra


def test_v030_pass():
    r = S.check(lines("v030.txt"))
    assert r["verdict"] == "PASS" and not r["v"]["fail"]
    assert len(r["v"]["use"]) == 3 and len(r["v"]["done"]) == 3 and r["v"]["refuse"] == {"keep reserve": 2, "in combat lock": 1}
    out = S.fmt(r)
    assert out.splitlines()[-1] == "SELFHEAL P1=PASS P3=PASS DUMP=1 blocks USE=3 DONE=3 TIMEOUT=1 VERDICT=PASS"
    assert "m1" in out and "hp+40 stack-1 timeout=0" in out and "keep reserve x2" in out


def test_v030_absent_unchanged():
    assert not S.check(lines("pass.txt"))["v"]["seen"]


def test_v030_build_why():
    assert S.check(v30(["BUILD n=10/10 why="]))["verdict"] == "PASS"
    assert S.check(v30(["BUILD n=4/10 why="]))["verdict"] == "FAIL"
    assert S.check(v30(["BUILD n=4/10"]))["verdict"] == "FAIL"
    assert S.check(v30(["BUILD n=4/10 why=no slot"]))["verdict"] == "PASS"


def test_v030_route_a_never_lands():
    L = v30(['USE member=1 item=0c000100 "Medical Supplies" heal=27 hp=12/40 src=click', "DONE member=1 hp+0 stack+0 timeout=1"])
    r = S.check(L)
    assert r["verdict"] == "FAIL" and any("never lands" in m for _, m in r["v"]["fail"])
    assert S.fmt(r).splitlines()[-1].endswith("USE=1 DONE=1 TIMEOUT=1 VERDICT=FAIL")


def test_v030_cfg_save_fail():
    r = S.check(v30(["CFG save tag=hud auto=1 hp=35 keep=2 ok=0"]))
    assert r["verdict"] == "FAIL" and any("ok=0" in m for _, m in r["v"]["fail"])


def test_v040_pass_with_warns():
    r = S.check(lines("v040.txt"))
    assert r["verdict"] == "PASS" and not r["v"]["fail"]
    assert [b[1] for b in r["v"]["build"]] == [12, 7] and r["v"]["tex"] == {"queued": 2, "refilled": 1, "giving up": 1}
    out = S.fmt(r).splitlines()
    assert any(l.startswith("WARN") and "ICON fallback member=2" in l for l in out) and any(l.startswith("WARN") and "giving up" in l for l in out)
    assert out[-1].endswith("VERDICT=PASS")


def test_v040_build_denominator():
    assert S.check(v30(["BUILD n=12/12 why="]))["verdict"] == "PASS"
    assert S.check(v30(["BUILD n=10/12 why="]))["verdict"] == "FAIL"
    assert S.check(v30(["BUILD n=10/12"]))["verdict"] == "FAIL"
    assert S.check(v30(["BUILD n=10/12 why=ring failed"]))["verdict"] == "PASS"
    assert "n=10/12 without why" in S.check(v30(["BUILD n=10/12 why="]))["v"]["fail"][0][1]


def test_v050_pass_new_refuse_format():
    r = S.check(lines("v050.txt"))
    assert r["verdict"] == "PASS" and not r["v"]["fail"]
    assert [b[1] for b in r["v"]["build"]] == [14]
    assert any(k.startswith("canuse=0 member=1") for k in r["v"]["refuse"])
    assert S.fmt(r).splitlines()[-1].endswith("VERDICT=PASS")


V06 = [
    'REFUSE already queued member=1 (click)',
    'USE member=1 item=0c000100 "Medical Supplies" heal=27 hp=12/40 src=click',
    'KICK skipped member=1 (running=1)',
    'KICK skipped member=1 (running=1)',
    'KICK skipped member=1 (ab8=2)',
    'KICK member=1 before=[3d,3f] after=[3f,3d] k=1 route=rotate',
    'DONE member=1 hp+27 stack-1 queued=0',
]


def test_v060_kick_pass():
    r = S.check(v30(V06))
    v = r["v"]
    assert r["verdict"] == "PASS" and not v["fail"] and not v["warn"]
    assert len(v["kick"]) == 1 and v["kick"][0]["k"] == 1 and v["kick_skip"] == {"running=1": 2, "ab8=2": 1} and v["refuse_queued"] == 1
    assert v["done"][0]["queued"] == 0 and v["done"][0]["timeout"] == 0
    out = S.fmt(r)
    assert "CS12: KICK=1 skipped=3 starved=0 REFUSE-queued=1 TIMEOUT-unqueued=0" in out and "queued=0" in out
    assert out.splitlines()[-1].endswith("USE=1 DONE=1 TIMEOUT=0 VERDICT=PASS")


def test_v060_starved_and_timeout_are_warn_not_fail():
    L = v30(['USE member=2 item=0c000100 "Medical Supplies" heal=27 hp=12/40 src=auto', "KICK starved member=2",
             "DONE member=2 hp+0 stack+0 queued=0", "TIMEOUT unqueued member=2"])
    r = S.check(L)
    v = r["v"]
    assert r["verdict"] == "PASS" and not v["fail"]       # not FAIL, also not the legacy "never lands" check
    assert v["kick_starved"] == 1 and v["timeout_unq"] == 1 and v["done"][0]["timeout"] == 1
    out = S.fmt(r)
    assert any(l.startswith("WARN") and "KICK starved member=2" in l for l in out.splitlines())
    assert any(l.startswith("WARN") and "TIMEOUT unqueued member=2" in l for l in out.splitlines())
    assert out.splitlines()[-1].endswith("USE=1 DONE=1 TIMEOUT=1 VERDICT=PASS")


def test_v060_kick_that_did_not_reach_head_warns():
    r = S.check(v30(["KICK member=1 before=[3d,3f] after=[3d,3f] k=0 route=rotate"]))
    assert r["verdict"] == "PASS" and any("did not bring" in m for _, m in r["v"]["warn"])


def test_v060_prefixed_lines_and_legacy_done_still_parse():
    r = S.check(v30(["12000 companion-selfheal/call KICK member=1 before=[3d,3d,3f] after=[3f,3d,3d] k=2 route=rotate",
                     "12001 companion-selfheal/call WARN KICK starved member=1", "DONE member=1 hp+0 stack+0 timeout=1"]))
    assert r["v"]["kick"][0]["k"] == 2 and r["v"]["kick_starved"] == 1 and r["v"]["done"][0]["timeout"] == 1


def test_kplog_v1_prefix_and_diag_pop():
    lines = [
        "T=100.000 f=- companion-selfheal/hud P1 hud built panel=08790760 +0x30=08302b58 +0x48=ffdbe447 layoutOpen=1 ctlList=0883cdf8 ctlCount=213",
        "T=100.001 f=- companion-selfheal/hud P3 slot[1]=332b05e8 creature=33701258 hud=332ad8d0 k=1",
        "T=100.002 f=- companion-selfheal/hud BUILD n=16/16 why=ok",
        'T=100.003 f=- companion-selfheal/ui DIAG pop item=12 rgb=0.85,0.95,0.85 alpha=1.00 txt="Use below 35%" (rgbOk=1 alphaOk=1)',
    ]
    r = S.check(lines)
    assert r["p1"] == "PASS"
    assert r["p3"] == "PASS"
    assert len(r["v"]["build"]) == 1
    assert len(r["v"]["diag_pop"]) == 1
    assert r["v"]["diag_pop"][0][2] == 1  # rgbOk
    assert r["v"]["diag_pop"][0][3] == 1  # alphaOk
    assert r["verdict"] == "PASS"


# ---- 0.7.0-dev: popalign10 + disc2 (P4 popover rows, P5 discs/TEXSET, SETFAIL, new tex lines, BUILD proj/cleared, STATE lookA)

HPM = 'POPROW m=1 item=hpm ext=[1712,1140,96,24] extOk=1 vis=1 txt="HP -" tobj=1 rend=0b2d1000 rgb=0.85,0.95,0.85 rgbOk=1 rectSet=1'
BG = 'POPROW m=1 item=bg ext=[1700,1100,220,300] extOk=1 vis=1 txt="" tobj=0 rend=00000000 rgb=-1.00,-1.00,-1.00 rgbOk=0 rectSet=1'


def swap(L, old, new):
    """Replace the message part of each fixture line that ends with `old` (lines carry a T= prefix)."""
    assert any(l.endswith(old) for l in L), "fixture row not found: %s" % old[:60]
    return [l[:-len(old)] + new if l.endswith(old) else l for l in L]


def test_v070_pass():
    r = S.check(lines("v070.txt"))
    v = r["v"]
    assert (r["p1"], r["p3"], r["p4"], r["p5"], r["verdict"]) == ("PASS", "PASS", "PASS", "PASS", "PASS")
    assert not v["fail"] and not v["warn"] and not r["p4_fail"] and not r["p5_fail"]
    assert v["version"] == ("0.7.0-dev", "popalign10+disc2")
    assert r["p4_info"] == "rows h=24 screen=3840x1600" and r["p5_info"] == "discs slots=11/16 DISC=3 hover=1"
    assert len(v["pop_rows"]) == 8 and len(v["disc"]) == 3 and len(v["texset"]) == 1
    assert v["build"][0][1:3] == (16, "") and v["build"][0][3:] == (1, 0)     # proj=1 cleared=0 parsed, not leaked into why
    assert v["state"][0]["lookA"] == "0.50" and v["tex"] == {"queued": 2, "refilled": 1}
    assert S.fmt(r).splitlines()[-1] == "SELFHEAL P1=PASS P3=PASS DUMP=0 blocks USE=0 DONE=0 TIMEOUT=0 P4=PASS P5=PASS VERDICT=PASS"


def test_v070_popalign_fail():
    r = S.check(lines("fail_popalign.txt"))
    assert r["p4"] == "FAIL" and r["verdict"] == "FAIL" and r["p5"] == "PASS"
    assert len(r["p4_fail"]) == 1 and "item=hpm: tobj=0, rend=00000000" in r["p4_fail"][0][1]   # the bg row (tobj=0) is exempt
    assert S.fmt(r).splitlines()[-1].endswith("P4=FAIL P5=PASS VERDICT=FAIL")


def test_v070_popalign_row_rules():
    for name, new, want in [
        ("null text row", "POPROW m=1 item=hpm null", "null (text item has no row)"),
        ("vis=0", HPM.replace("vis=1", "vis=0"), "vis=0"),
        ("extOk=0", HPM.replace("extOk=1", "extOk=0"), "extOk=0"),
        ("rectSet=0", HPM.replace("rectSet=1", "rectSet=0"), "rectSet=0"),
        ("ext h=0", HPM.replace("96,24]", "96,0]"), "ext=[1712,1140,96,0]"),
    ]:
        r = S.check(swap(lines("v070.txt"), HPM, new))
        assert r["p4"] == "FAIL" and r["verdict"] == "FAIL", name
        assert any(want in m for _, m in r["p4_fail"]), name
    assert S.check(swap(lines("v070.txt"), BG, "POPROW m=1 item=bg null"))["p4"] == "PASS"   # bg is never a text check
    r = S.check([l.replace("ext=[1712,1226,196,24]", "ext=[1712,1226,196,14]") for l in lines("v070.txt")])
    assert r["p4"] == "PASS" and r["p4_info"] == "rows h=14 screen=3840x1600"       # min row height is reported, no line-height check


def test_v070_disc_null_fail():
    r = S.check(lines("fail_disc_null.txt"))
    msgs = " ".join(m for _, m in r["p5_fail"])
    assert r["p5"] == "FAIL" and r["verdict"] == "FAIL" and r["p4"] == "PASS"
    assert "TEXSET disc=00000000" in msgs and "DISC m=2 img=00000000" in msgs and "DISC m=2 setfill=0" in msgs
    assert S.fmt(r).splitlines()[-1].endswith("P4=PASS P5=FAIL VERDICT=FAIL")


def test_v083_tog_texset_fails_disc_null():
    # CompanionSelfHeal.cpp:517 writes tog=a,b,c,d between dischi and ring0; TEXSET_RE used to miss it, so P5 never saw disc=00000000
    r = S.check(lines("v083_tog.txt"))
    assert r["p5"] == "FAIL", r["p5"]
    assert "TEXSET disc=00000000" in " ".join(m for _, m in r["p5_fail"])
    assert len(r["v"]["texset"]) == 1 and r["v"]["texset"][0]["dischi"] == "0a0b0c02" and r["v"]["texset"][0]["ring0"] == "0a0b0c07"


def test_v083_tog_texset_good_disc_passes():
    # negative control: the same tog line with a real disc handle must not FAIL P5
    r = S.check([l.replace("disc=00000000", "disc=0a0b0c00") for l in lines("v083_tog.txt")])
    assert r["p5"] == "PASS" and len(r["v"]["texset"]) == 1


def test_v083_texset_legacy_no_tog_still_parses():
    # the tog= group is optional: a pre-tog TEXSET line keeps the same nine groups
    legacy = "TEXSET ensure=1 supported=1 ready=1 cross=0a0b0c01 disc=0a0b0c02 dischi=0a0b0c03 ring0=0a0b0c04 slots=11/16"
    m = S.TEXSET_RE.match(legacy)
    assert m is not None and len(m.groups()) == 9 and m.group(5) == "0a0b0c02" and m.group(7) == "0a0b0c04" and m.group(8) == "11"
    assert len(S.check(lines("v070.txt"))["v"]["texset"]) == 1


def test_v070_ensure_zero_is_warn_only():
    r = S.check([l.replace("ensure=1", "ensure=0") if "TEXSET" in l else l for l in lines("v070.txt")])
    assert r["p5"] == "PASS" and r["verdict"] == "PASS"
    assert any(l.startswith("WARN") and "ensure=0" in l for l in S.fmt(r).splitlines())


def test_v070_tex_new_format_warns():
    extra = [
        "T=9.700 f=- companion-selfheal/ui tex csh_ring3: giving up after 20 failed refills (built=1 missing=1 pixels=00000000); art missing, labels still work",
        "T=9.710 f=- companion-selfheal/ui tex csh_ring4: slot table full (16)",
        "T=9.720 f=- companion-selfheal/ui tex csh_ring5: allocation failed",
        "T=9.730 f=- companion-selfheal/ui tex csh_ring6: engine malloc(4096) failed",
        "T=9.740 f=- companion-selfheal/ui tex bad_type: embedded TGA not uncompressed 32-bit, skipped",
    ]
    r = S.check(lines("v070.txt") + extra)
    out = S.fmt(r).splitlines()
    assert r["verdict"] == "PASS" and not r["v"]["fail"]
    assert r["v"]["tex"] == {"queued": 2, "refilled": 1, "giving up": 1, "slot full": 1, "alloc failed": 2, "other": 1}
    assert any(l.startswith("WARN") and "TEX csh_ring3 giving up" in l for l in out)
    assert any(l.startswith("WARN") and "TEX csh_ring4 slot table full" in l for l in out)
    assert any(l.startswith("WARN") and "TEX csh_ring5 allocation failed" in l for l in out)
    assert any(l.startswith("WARN") and "TEX csh_ring6 allocation failed" in l for l in out)
    assert not any("bad_type" in l for l in out)     # informational, never WARN


def test_v070_setfail_counted_as_warn():
    L = lines("v070.txt") + ["T=11.000 f=- companion-selfheal/ui WARN SETFAIL op=rect item=3",
                             "T=11.001 f=- companion-selfheal/ui SETFAIL op=rect item=5",
                             "T=11.002 f=- companion-selfheal/hud SETFAIL op=build tmpl=csh_cross font=0 align=1 text=1"]
    r = S.check(L)
    out = S.fmt(r)
    first = len(lines("v070.txt")) + 1
    assert r["verdict"] == "PASS" and len(r["v"]["setfail"]) == 3
    assert "WARN line %d: SETFAIL x3: op=build x1, op=rect x2" % first in out


def test_v070_build_proj_cleared_and_why():
    assert S.check(v30(["BUILD n=10/10 why= proj=1 cleared=0"]))["verdict"] == "PASS"
    r = S.check(v30(["BUILD n=4/10 why= proj=0 cleared=1"]))       # empty why with proj/cleared still needs a reason
    assert r["verdict"] == "FAIL" and any("n=4/10 without why" in m for _, m in r["v"]["fail"])
    r = S.check(v30(["BUILD n=4/10 why=no slot proj=1 cleared=0"]))
    assert r["verdict"] == "PASS" and r["v"]["build"][-1][2] == "no slot" and r["v"]["build"][-1][3:] == (1, 0)
    r = S.check(v30(["BUILD n=10/12 why=ring failed"]))            # legacy line: proj/cleared absent
    assert r["v"]["build"][-1][2:] == ("ring failed", None, None)


def test_v070_state_look_optional():
    r = S.check(["T=1.000 f=- companion-selfheal/ui STATE member=2 Normal->Down hp=5/30 lookA=0.25",
                 "9000 companion-selfheal/ui STATE member=2 Down->Normal hp=30/30"])
    assert [(s["member"], s["old"], s["new"], s["hp"], s["lookA"]) for s in r["v"]["state"]] == [
        ("2", "Normal", "Down", "5/30", "0.25"), ("2", "Down", "Normal", "30/30", None)]
    assert not r["v"]["seen"] and r["verdict"] == "INCOMPLETE"


def test_v070_version_both_forms():
    assert S.VERSION_RE.match("0.5.0-dev: probe call=0 CallKey=0x4A cfg rows=3").groups() == ("0.5.0-dev", None)
    assert S.VERSION_RE.match("0.7.0-dev ui=popalign10+disc2: probe call=3 CallKey=0x4A kick=1 cfg rows=3").groups() == (
        "0.7.0-dev", "popalign10+disc2")
    assert S.check(lines("v050.txt"))["v"]["version"] == ("0.5.0-dev", None)


def test_v070_not_seen_for_legacy_logs():
    for name in ("pass.txt", "v030.txt", "v040.txt", "v050.txt"):
        r = S.check(lines(name))
        assert (r["p4"], r["p5"], r["p4_fail"], r["p5_fail"]) == ("NOT-SEEN", "NOT-SEEN", [], []), name
        assert " P4=" not in S.fmt(r) and r["verdict"] == "PASS", name
    assert S.check(["P1 hud built layoutOpen=1 ctlCount=212"])["p4"] == "NOT-SEEN"


def test_v070_main_exit_codes():
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        assert S.main([str(FX / "v070.txt")]) == 0
        assert S.main([str(FX / "fail_popalign.txt")]) == 1
        assert S.main([str(FX / "fail_disc_null.txt")]) == 1
    assert "P4=FAIL P5=PASS VERDICT=FAIL" in out.getvalue() and "P4=PASS P5=FAIL VERDICT=FAIL" in out.getvalue()


# ---- 0.8.0-dev: popsize + togglepill + leaderclick (P6 popover fit, P7 popover/leader-click interaction; P7 never FAIL)

V08_VERSION = ("0.8.0-dev", "popsize+togglepill+leaderclick")
V08_POP = "POPSIZE m=1 unit=24 panelW=200 widest=140 line=14 screen=3840x1600"
USE0 = 'USE member=0 item=0c000100 "Medical Supplies" heal=27 hp=12/40 src=click'
DONE0 = "DONE member=0 hp+27 stack-1 queued=0"


def test_v080_pass():
    r = S.check(lines("v080.txt"))
    v = r["v"]
    assert (r["p1"], r["p3"], r["p4"], r["p5"], r["p6"], r["p7"], r["verdict"]) == ("PASS",) * 5 + ("PASS", "PASS")
    assert not v["fail"] and not v["warn"] and not r["p6_fail"]
    assert v["version"] == V08_VERSION
    assert r["p6_info"] == "popsize n=1 min_slack=12"                      # 200 - (140 + 2*24)
    assert r["p7_info"] == "interact POPHOVER=2 POPCLICK=1 CLICK0 ok=1 noneed=0 noitem=0"
    assert len(v["popsize"]) == 1 and len(v["click0"]) == 1 and len(v["use"]) == 1 and len(v["done"]) == 1
    assert S.fmt(r).splitlines()[-1] == (
        "SELFHEAL P1=PASS P3=PASS DUMP=0 blocks USE=1 DONE=1 TIMEOUT=0 P4=PASS P5=PASS P6=PASS P7=PASS VERDICT=PASS")


def test_v080_popsize_fail_names_the_rule():
    r = S.check(lines("fail_popsize.txt"))
    assert r["p6"] == "FAIL" and r["verdict"] == "FAIL" and r["p7"] == "PASS"
    assert any("panelW=180 < widest+2*unit (140+2*24=188)" in m for _, m in r["p6_fail"])
    out = S.fmt(r).splitlines()
    assert any(l.startswith("FAIL P6 line 16") for l in out)
    assert out[-1].endswith("P6=FAIL P7=PASS VERDICT=FAIL")


def test_v080_popsize_boundaries_and_line_height():
    L = lines("v080.txt")

    def with_popsize(new):
        return [l.replace(V08_POP, new) if "POPSIZE" in l else l for l in L]

    assert S.check(with_popsize(V08_POP.replace("panelW=200", "panelW=188")))["p6"] == "PASS"   # equality passes
    assert S.check(with_popsize(V08_POP.replace("panelW=200", "panelW=187")))["p6"] == "FAIL"
    assert S.check(with_popsize(V08_POP.replace("line=14", "line=36")))["p6"] == "PASS"         # line == 1.5*unit passes
    r = S.check(with_popsize(V08_POP.replace("line=14", "line=37")))
    assert r["p6"] == "FAIL" and any("line=37 > 1.5*unit (1.5*24=36)" in m for _, m in r["p6_fail"])
    assert S.check(["POPSIZE m=1 unit=x panelW=200 widest=140 line=14 screen=3840x1600"])["p6"] == "FAIL"  # malformed


def test_v080_popsize_absent_is_not_seen():
    r = S.check([l for l in lines("v080.txt") if "POPSIZE" not in l])
    assert (r["p6"], r["p6_fail"], r["p6_info"]) == ("NOT-SEEN", [], None) and r["p7"] == "PASS"
    assert "P6=NOT-SEEN P7=PASS VERDICT=PASS" in S.fmt(r)


def test_v080_leader_click_without_use_warns_not_fails():
    L = [l for l in lines("v080.txt") if "USE member=0" not in l and "DONE member=0" not in l]
    r = S.check(L)
    out = S.fmt(r).splitlines()
    assert r["p7"] == "WARN" and r["verdict"] == "PASS" and not r["v"]["fail"] and r["p6"] == "PASS"
    assert any(l.startswith("WARN line") and "CLICK member=0 ok x1 with no later USE member=0 or DONE member=0" in l for l in out)
    assert out[-1].endswith("P6=PASS P7=WARN VERDICT=PASS")
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / "warn.txt"
        p.write_text("\n".join(L) + "\n")
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            assert S.main([str(p)]) == 0                                       # WARN never changes the exit code
    assert "VERDICT=PASS" in buf.getvalue()


def test_v080_leader_click_landed_rules():
    assert S.check(["CLICK member=0 ok", USE0])["p7"] == "PASS"
    assert S.check(["CLICK member=0 ok", DONE0])["p7"] == "PASS"                    # a later DONE member=0 also counts
    assert S.check([USE0, "CLICK member=0 ok"])["p7"] == "WARN"                    # a USE before the click does not
    assert S.check(["CLICK member=0 ok", USE0.replace("member=0", "member=2")])["p7"] == "WARN"   # other members do not


def test_v080_p7_counts_and_legacy_noitem():
    r = S.check(v30(["CLICK member=0 ok", "CLICK member=0 noneed", "CLICK member=0 noitem", "POPHOVER m=1 row=1", "POPCLICK m=1 row=5"]))
    assert r["p7_info"] == "interact POPHOVER=1 POPCLICK=1 CLICK0 ok=1 noneed=1 noitem=1"
    assert r["v"]["click_noitem"] == 1 and r["p7"] == "WARN" and r["verdict"] == "PASS"   # ok with no later USE/DONE
    r = S.check(v30(["CLICK member=2 noitem"]))                                       # legacy 0.3.0 form, other member
    assert r["v"]["click_noitem"] == 1 and r["v"]["click0"] == [] and r["p7"] == "NOT-SEEN" and r["p7_info"] is None


def test_v080_version_line_and_fixture():
    assert S.VERSION_RE.match("0.8.0-dev ui=popsize+togglepill+leaderclick: probe call=1 CallKey=0x4A kick=0 cfg rows=3").groups() == V08_VERSION
    assert S.check(lines("v080.txt"))["v"]["version"] == V08_VERSION


def test_v080_not_seen_for_legacy_logs():
    for name in ("pass.txt", "v030.txt", "v040.txt", "v050.txt", "v070.txt"):
        r = S.check(lines(name))
        assert (r["p6"], r["p7"], r["p6_fail"], r["p7_info"]) == ("NOT-SEEN", "NOT-SEEN", [], None), name
        assert " P6=" not in S.fmt(r) and r["verdict"] == "PASS", name


def test_v080_main_exit_codes():
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        assert S.main([str(FX / "v080.txt")]) == 0
        assert S.main([str(FX / "fail_popsize.txt")]) == 1
    assert "P6=FAIL P7=PASS VERDICT=FAIL" in out.getvalue()


# ---- 0.8.2-dev: vanillaarrows + popocclude (ARROW handles/fills INFO + WARN, POPOCCLUDE count on P7; never FAIL)

V82_VERSION = ("0.8.2-dev", "popsize+togglepill+leaderclick+vanillaarrows+pillnarrow+popocclude")
V82_P7 = "interact POPHOVER=2 POPCLICK=1 CLICK0 ok=1 noneed=0 noitem=0"


def with_arrow(extra):
    """v082 with `extra` lines inserted right after the first ARROW neg= line (the arrow handle set-up)."""
    L = lines("v082.txt")
    i = next(k for k, l in enumerate(L) if "ARROW neg=" in l)
    return L[:i + 1] + extra + L[i + 1:]


def test_v082_pass():
    r = S.check(lines("v082.txt"))
    v = r["v"]
    assert (r["p1"], r["p3"], r["p4"], r["p5"], r["p6"], r["p7"], r["arrow"], r["verdict"]) == ("PASS",) * 8
    assert not v["fail"] and not v["warn"] and not r["p6_fail"]
    assert v["version"] == V82_VERSION
    assert r["arrow_info"] == "arrows neg=0b2c8000 pos=0b2c8800 fills ok=4/4 retries=0"
    assert r["p7_info"] == V82_P7 + " POPOCCLUDE=3" and len(v["popocclude"]) == 3
    assert len(v["arrow_hdr"]) == 1 and len(v["arrow_fill"]) == 4 and not v["arrow_retry"]
    assert S.fmt(r).splitlines()[-1] == (
        "SELFHEAL P1=PASS P3=PASS DUMP=0 blocks USE=1 DONE=1 TIMEOUT=0 P4=PASS P5=PASS P6=PASS P7=PASS ARROW=PASS VERDICT=PASS")


def test_v082_warn_arrow_null():
    r = S.check(lines("warn_arrow_null.txt"))
    out = S.fmt(r).splitlines()
    assert r["arrow"] == "WARN" and r["verdict"] == "PASS" and not r["v"]["fail"]
    assert r["arrow_info"] == "arrows neg=00000000 pos=00000000 fills ok=0/4 retries=0"
    assert any(l.startswith("WARN") and "ARROW neg=00000000 null handle" in l for l in out)
    assert any(l.startswith("WARN") and "ARROW pos=00000000 null handle" in l for l in out)
    assert any(l.startswith("WARN") and "ARROW fill ok=0 x4" in l for l in out)
    assert out[-1].endswith("P7=PASS ARROW=WARN VERDICT=PASS")
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        assert S.main([str(FX / "warn_arrow_null.txt")]) == 0                  # WARN never changes the exit code
    assert "ARROW=WARN VERDICT=PASS" in buf.getvalue()


def test_v082_final_handles_come_from_last_retry():
    r = S.check(with_arrow(["ARROW retry=1 neg=00000000 pos=0b2c8800", "ARROW retry=2 neg=0b2c9000 pos=0b2c9800"]))
    assert r["arrow"] == "PASS" and r["verdict"] == "PASS"                    # a null first try is superseded by the last retry
    assert r["arrow_info"] == "arrows neg=0b2c9000 pos=0b2c9800 fills ok=4/4 retries=2"
    r = S.check(with_arrow(["ARROW retry=1 neg=0b2c9000 pos=0b2c9800", "ARROW retry=2 neg=00000000 pos=0b2c9800"]))
    assert r["arrow"] == "WARN" and r["verdict"] == "PASS" and any("ARROW neg=00000000 null" in m for _, m in r["v"]["warn"])


def test_v082_fill_ok0_warns_and_counts():
    L = [l.replace("ARROW fill m=1 item=2 ok=1", "ARROW fill m=1 item=2 ok=0") for l in lines("v082.txt")]
    assert L != lines("v082.txt")                                              # the fixture row was found
    r = S.check(L)
    assert r["arrow"] == "WARN" and r["verdict"] == "PASS"
    assert r["arrow_info"] == "arrows neg=0b2c8000 pos=0b2c8800 fills ok=3/4 retries=0"
    assert any("ARROW fill ok=0 x1" in m for _, m in r["v"]["warn"])


def test_v082_arrow_absent_is_not_seen():
    for name in ("pass.txt", "v030.txt", "v040.txt", "v050.txt", "v070.txt", "v080.txt"):
        r = S.check(lines(name))
        assert (r["arrow"], r["arrow_info"], r["verdict"]) == ("NOT-SEEN", None, "PASS"), name
        assert not any(s in S.fmt(r) for s in (" ARROW=", "arrows neg=", "POPOCCLUDE")), name
    r = S.check([l for l in lines("v082.txt") if "ARROW" not in l])            # no ARROW line at all
    assert r["arrow"] == "NOT-SEEN" and r["arrow_info"] is None and r["verdict"] == "PASS"
    assert " ARROW=" not in S.fmt(r)


def test_v082_popocclude_is_count_only():
    assert S.check(lines("v080.txt"))["p7_info"] == V82_P7                    # no POPOCCLUDE lines: no count on the line
    r = S.check([l for l in lines("v082.txt") if "POPOCCLUDE" not in l])
    assert r["p7"] == "PASS" and r["p7_info"] == V82_P7 and r["verdict"] == "PASS"
    assert S.check(["POPOCCLUDE m=1 btn=2"])["p7"] == "NOT-SEEN"              # alone it never turns P7 on


def test_v082_version_line():
    assert S.VERSION_RE.match("0.8.2-dev ui=popsize+togglepill+leaderclick+vanillaarrows+pillnarrow+popocclude: probe call=1 "
                              "CallKey=0x4A kick=0 cfg rows=3").groups() == V82_VERSION
    assert S.check(lines("v082.txt"))["v"]["version"] == V82_VERSION


def load_tests(loader, tests, pattern):
    """`python -m unittest` collects only TestCase classes; wrap the plain test_ functions so it runs them too."""
    suite = unittest.TestSuite(tests)
    for name, fn in sorted(globals().items()):
        if name.startswith("test_") and callable(fn):
            suite.addTest(unittest.FunctionTestCase(fn))
    return suite
