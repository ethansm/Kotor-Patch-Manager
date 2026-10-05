#!/usr/bin/env python3
"""Tests for scripts/selfheal_log_check.py (fixtures in scripts/testdata/selfheal/). Run: python3 -m pytest scripts/test_selfheal_log_check.py -q"""
import sys
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


def test_missing_file(tmp_path, capsys):
    p = tmp_path / "nope.txt"
    assert S.main([str(p)]) == 2
    assert capsys.readouterr().out.strip() == "no log: %s" % p


def test_main_exit_codes(capsys):
    assert S.main(["--log", str(FX / "pass.txt")]) == 0
    assert S.main([str(FX / "fail_knone.txt")]) == 1
    assert S.main(["--log", str(FX / "pass.txt"), "--from", "99"]) == 0
    assert "VERDICT=INCOMPLETE" in capsys.readouterr().out


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
