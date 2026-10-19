#!/usr/bin/env python3
"""Tests for k1_port_dialog.py (K1 conversation -> K2 dialog converter). Plain python3, stdlib only: synthetic plain
graphs, no pykotor, no game install. FakeBackend is the JSON stand-in for PykotorBackend that test_k1_port_import.py's
fake interpreter uses for the hidden `_dialog` helper. One test re-runs the real PykotorBackend writer/reader under the k1port
venv python (SKIP when the venv is missing).

Run: python3 -u test_k1_port_dialog.py   (exit 0 = all PASS)
"""
from __future__ import annotations

import contextlib
import hashlib
import io
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "kvoice"))
import k1_port_dialog as D  # noqa: E402
import kvoice_lip  # noqa: E402

VENV_PY = Path(os.environ.get("K1PORT_VENV") or "~/Games/KOTOR2_Mod_Workspace/venvs/k1port").expanduser() / "bin" / "python"
GOOD_LIP = kvoice_lip.write_lip([(0.0, 0), (0.5, 3)], 1.0)
BAD_LIP = b"BAD " + GOOD_LIP[4:]


# ------------------------------------------------------------------------------------------- graph builders

def link(i, active="", active2="", is_child=False):
    return {"index": i, "active": active, "active2": active2, "is_child": is_child}


def node(text="", vo="", links=(), strref=-1, **kw):
    n = {"strref": strref, "text": text, "vo": vo, "speaker": "", "listener": "", "script": "", "script2": "", "sound": "",
         "quest": "", "delay": -1, "wait_flags": 0, "camera_angle": 0, "camera_id": None, "camera_anim": None,
         "camera_effect": None, "camera_fov": None, "camera_height": None, "target_height": None, "fade_type": 0,
         "fade_delay": None, "fade_length": None, "animations": [], "links": list(links)}
    n.update(kw)
    return n


def graph(entries, replies=(), starters=(0,)):
    return {"entries": list(entries), "replies": list(replies),
            "starters": [s if isinstance(s, dict) else link(s) for s in starters],
            "root": {"skippable": True, "old_hit_check": False, "unequip_items": True, "unequip_hands": False,
                     "conversation_type": 0, "computer_type": 0}}


def run(g, *, tlk=None, vo=(), lips=None, **kw):
    """convert_dialog with every VO name in `vo` present as a wav (bytes = the name) and lips from `lips` ({name: bytes})."""
    tlk = tlk or {}
    index = {n: f"WAV:{n}".encode() for n in vo}
    lips = {n: GOOD_LIP for n in vo} if lips is None else lips
    asked = []

    def lip_lookup(names):
        asked.append(list(names))
        return {n: lips.get(n) for n in names}

    res = D.convert_dialog(g, tlk.get, index, lip_lookup, **kw)
    res["lip_asked"] = asked
    return res


def chain(n, prefix="v"):
    """entries 0..n-1 each with a VO, e_i -> r_i -> e_(i+1); returns the graph."""
    ents = [node(f"line {i}", f"{prefix}{i}", [link(i)]) for i in range(n)]
    reps = [node(f"reply {i}", "", [link(i + 1)] if i + 1 < n else []) for i in range(n)]
    return graph(ents, reps, [0])


class ConvertTests(unittest.TestCase):
    def test_unconditional_kept_scripted_links_and_starters_dropped(self):
        ents = [node("cond start", "", [link(0)]), node("free start", "", [link(0), link(1, active="k_con_x")]),
                node("hidden", "", [])]
        reps = [node("keep me"), node("drop me")]
        g = graph(ents, reps, [link(0, active="k_swg_a"), link(1)])
        rep = run(g)["report"]
        self.assertEqual((rep["nodes_total"], rep["nodes_kept"]), (5, 2))
        self.assertEqual((rep["starters_total"], rep["starters_unconditional"], rep["starters_kept"]), (2, 1, 1))
        self.assertEqual(rep["links_dropped_by_condition"], 2)  # the scripted starter + the scripted reply link
        self.assertEqual(rep["condition_scripts_dropped"], ["k_con_x", "k_swg_a"])
        self.assertEqual(rep["nodes_dropped_by_condition"], 2)  # entry 0 + reply 1 are reachable only through a condition
        out = run(g)["graph"]
        self.assertEqual([n["text"] for n in out["entries"]], ["free start"])
        self.assertEqual([n["text"] for n in out["replies"]], ["keep me"])
        self.assertEqual(out["starters"], [{"index": 0}])

    def test_active2_alone_is_a_condition(self):
        g = graph([node("a", "", [link(0, active2="k_x")]), node("b")], [node("r")], [1])
        self.assertEqual(run(g)["report"]["links_dropped_by_condition"], 0)
        g = graph([node("a", "", [link(0, active2="k_x")])], [node("r")], [0])
        self.assertEqual(run(g)["report"]["links_dropped_by_condition"], 1)

    def test_starters_after_the_first_unconditional_one_are_shadowed(self):
        g = graph([node("first"), node("second"), node("third")], [], [link(0, active="c"), link(1), link(2)])
        res = run(g)
        self.assertEqual([n["text"] for n in res["graph"]["entries"]], ["second"])
        self.assertEqual((res["report"]["starters_unconditional"], res["report"]["starters_kept"]), (2, 1))
        self.assertEqual(res["report"]["links_dropped_shadowed"], 1)
        self.assertEqual(res["report"]["nodes_dropped_shadowed"], 1)
        both = run(g, shadow=False)
        self.assertEqual([n["text"] for n in both["graph"]["entries"]], ["second", "third"])

    def test_reply_to_entry_links_are_shadowed_but_entry_to_reply_links_are_all_kept(self):
        # the player sees every passing reply; the NPC answers with the first passing entry only
        g = graph([node("npc", "", [link(0), link(1)]), node("a1"), node("a2")],
                  [node("r0", "", [link(1), link(2)]), node("r1")], [0])
        out = run(g)["graph"]
        self.assertEqual([n["text"] for n in out["replies"]], ["r0", "r1"])
        self.assertEqual([n["text"] for n in out["entries"]], ["npc", "a1"])  # a2 is shadowed behind a1

    def test_cycles_terminate_and_keep_each_node_once(self):
        ents = [node("e0", "", [link(0)]), node("e1", "", [link(1)])]
        reps = [node("r0", "", [link(1), link(0)]), node("r1", "", [link(0)])]
        res = run(ents and graph(ents, reps, [0]), shadow=False)
        rep = res["report"]
        self.assertEqual((rep["entries_kept"], rep["replies_kept"]), (2, 2))
        self.assertEqual(rep["links_kept"], 1 + 1 + 1 + 2 + 1)  # starter + e0 + e1 + r0 + r1
        e0 = res["graph"]["entries"][0]
        self.assertEqual([l["index"] for l in e0["links"]], [0])
        self.assertEqual([l["index"] for l in res["graph"]["replies"][0]["links"]], [1, 0])

    def test_bad_link_indices_are_ignored_and_counted(self):
        g = graph([node("e", "", [link(0), link(7), link(-1)])], [node("r")], [link(0), link(5)])
        res = run(g)
        self.assertEqual(res["report"]["links_bad"], 3)
        self.assertEqual(len(res["graph"]["entries"][0]["links"]), 1)

    def test_no_unconditional_starter_is_an_error(self):
        g = graph([node("only")], [], [link(0, active="k_x")])
        with self.assertRaises(D.DialogError) as cm:
            run(g)
        self.assertIn("starter", str(cm.exception))

    def test_literal_text_embedded_from_the_tlk_and_the_dlg(self):
        g = graph([node("", "", [link(0)], strref=100), node("embedded wins nothing", "", strref=-1)],
                  [node("", "", [], strref=101)], [0])
        g["entries"][1]["text"] = "inline"
        res = run(g, tlk={100: "K1 line one", 101: "Player reply"})
        out = res["graph"]
        self.assertEqual(out["entries"][0]["text"], "K1 line one")
        self.assertEqual(out["replies"][0]["text"], "Player reply")
        self.assertEqual((res["report"]["text_from_tlk"], res["report"]["text_embedded"], res["report"]["text_empty"]), (2, 0, 0))
        g2 = graph([node("inline text", "", strref=-1), node("", "", strref=555)], [], [0, 1])
        res = run(g2, tlk={}, shadow=False)
        self.assertEqual([n["text"] for n in res["graph"]["entries"]], ["inline text", ""])
        self.assertEqual((res["report"]["text_embedded"], res["report"]["text_empty"], res["report"]["tlk_missing"]), (1, 1, [555]))
        self.assertTrue(all("strref" not in n for n in res["graph"]["entries"]))  # stringref -1 is written by the DLG writer

    def test_scripts_and_quest_fields_are_stripped(self):
        n = node("hi", "", script="k_swg_do", script2="k_other", sound="snd_x", quest="q_carth", camera_effect=3,
                 camera_id=2, camera_anim=4, camera_angle=4)
        res = run(graph([n], [], [0]))
        out = res["graph"]["entries"][0]
        for key in ("script", "script2", "sound", "quest", "camera_effect", "camera_id", "camera_anim", "strref"):
            self.assertNotIn(key, out)
        self.assertEqual(out["camera_angle"], 0)  # a static camera angle needs the K1 area camera
        self.assertEqual((res["report"]["nodes_with_script_stripped"], res["report"]["nodes_with_other_fields_stripped"],
                          res["report"]["camera_angle_reset"]), (1, 1, 1))
        self.assertNotIn("active", json.dumps(res["graph"]))
        self.assertEqual(set(res["graph"]["root"]), {"skippable", "old_hit_check", "unequip_items", "unequip_hands",
                                                     "conversation_type", "computer_type"})

    def test_speaker_animations_and_timing_are_kept(self):
        n = node("hi", "", speaker="Bastila", listener="Player", delay=1500, wait_flags=2, camera_angle=2, camera_fov=40.0,
                 animations=[{"animation": 28, "participant": "OWNER"}, {"animation": 3, "participant": "Bastila"}])
        res = run(graph([n], [], [0]))
        out = res["graph"]["entries"][0]
        self.assertEqual((out["speaker"], out["listener"], out["delay"], out["wait_flags"], out["camera_angle"], out["camera_fov"]),
                         ("Bastila", "Player", 1500, 2, 2, 40.0))
        self.assertEqual(out["animations"], n["animations"])
        self.assertEqual((res["report"]["speakers"], res["report"]["animation_participants"]), (["Bastila"], ["Bastila"]))

    def test_dlg_asset_uses_the_writer_and_counts_even_without_it(self):
        g = chain(2)
        seen = []
        res = run(g, vo=("v0", "v1"), write_dlg=lambda og: seen.append(og) or b"DLGBYTES", k2_dlg="k1p_xdlg")
        self.assertEqual(seen[0], res["graph"])
        self.assertEqual([(a[0], a[2]) for a in res["assets"]],
                         [("k1p_xdlg.dlg", "dlg"), ("v0.wav", "wav"), ("v1.wav", "wav"), ("v0.lip", "lip"), ("v1.lip", "lip")])
        self.assertEqual(res["report"]["asset_count"], 5)
        res = run(g, vo=("v0", "v1"))
        self.assertEqual(res["report"]["asset_count"], 5)  # the dry run counts the dlg it does not build
        self.assertEqual(len(res["assets"]), 4)


class VoiceTests(unittest.TestCase):
    def test_wav_bytes_are_unchanged(self):
        res = run(chain(1), vo=("v0",))
        self.assertEqual([a for a in res["assets"] if a[2] == "wav"], [("v0.wav", b"WAV:v0", "wav")])

    def test_voice_budget_is_breadth_first_and_overflow_is_silent(self):
        # e0 has two replies; BFS order e0, r0, r1, e1, e2... a depth-first walk would voice the long branch first
        ents = [node("e0", "v_root", [link(0), link(1)]), node("deep a", "v_deep_a", [link(2)]), node("deep b", "v_deep_b"),
                node("side", "v_side")]
        reps = [node("r0", "", [link(1)]), node("r1", "", [link(3)]), node("r2", "", [link(2)])]
        g = graph(ents, reps, [0])
        res = run(g, vo=("v_root", "v_deep_a", "v_deep_b", "v_side"), max_voiced=2)
        by_text = {n["text"]: n["vo"] for n in res["graph"]["entries"]}
        self.assertEqual(by_text["e0"], "v_root")
        self.assertEqual(by_text["deep a"], "v_deep_a")  # e0 -> r0 -> deep a is discovered before side (via r1)
        self.assertEqual((by_text["side"], by_text["deep b"]), ("", ""))
        rep = res["report"]
        self.assertEqual((rep["voiced_lines"], rep["silent_budget"], rep["silent_no_wav"], rep["asset_count"]), (2, 2, 0, 1 + 2 + 2))
        self.assertEqual(sorted(res["lip_asked"][0]), ["v_deep_a", "v_root"])  # lips are only fetched for voiced lines

    def test_text_is_kept_on_silent_lines(self):
        res = run(chain(3), vo=("v0", "v1", "v2"), max_voiced=1)
        self.assertEqual([n["text"] for n in res["graph"]["entries"]], ["line 0", "line 1", "line 2"])
        self.assertEqual([n["vo"] for n in res["graph"]["entries"]], ["v0", "", ""])

    def test_zero_budget_is_all_silent_and_a_bad_budget_is_refused(self):
        res = run(chain(2), vo=("v0", "v1"), max_voiced=0)
        self.assertEqual((res["report"]["voiced_lines"], res["report"]["asset_count"], res["lip_asked"]), (0, 1, []))
        for bad in (-1, 81, True, "30"):
            with self.assertRaises(D.DialogError):
                run(chain(1), vo=("v0",), max_voiced=bad)
        run(chain(1), vo=("v0",), max_voiced=80)

    def test_nodes_sharing_a_vo_name_cost_one_line(self):
        ents = [node("a", "same", [link(0)]), node("b", "same", [link(1)]), node("c", "other")]
        reps = [node("r", "", [link(1)]), node("r2", "", [link(2)])]
        g = graph(ents, reps, [0])
        res = run(g, vo=("same", "other"), max_voiced=1, shadow=False)
        self.assertEqual([n["vo"] for n in res["graph"]["entries"]], ["same", "same", ""])
        self.assertEqual((res["report"]["voiced_lines"], res["report"]["voiced_nodes"], res["report"]["silent_budget"]), (1, 2, 1))

    def test_a_vo_name_without_a_wav_stays_silent_and_does_not_spend_budget(self):
        g = chain(2)
        res = run(g, vo=("v1",), max_voiced=1)
        self.assertEqual([n["vo"] for n in res["graph"]["entries"]], ["", "v1"])
        self.assertEqual((res["report"]["silent_no_wav"], res["report"]["silent_no_wav_names"]), (1, ["v0"]))

    def test_vo_names_are_lowercased_and_the_trailing_underscore_kept(self):
        g = graph([node("a", "NM13AACART02016_")], [], [0])
        res = run(g, vo=("nm13aacart02016_",))
        self.assertEqual(res["graph"]["entries"][0]["vo"], "nm13aacart02016_")
        self.assertEqual([a[0] for a in res["assets"] if a[2] != "dlg"], ["nm13aacart02016_.wav", "nm13aacart02016_.lip"])
        self.assertEqual(res["report"]["vo_renamed"], [])

    def test_invalid_vo_names_are_renamed_consistently(self):
        long_name = "nm13aacart020161234"      # 19 chars
        odd = "nm13 aa-cart"                   # characters outside [a-z0-9_]
        ents = [node("a", long_name, [link(0)]), node("b", odd)]
        g = graph(ents, [node("r", "", [link(1)])], [0])
        res = run(g, vo=(long_name, odd), max_voiced=5, shadow=False)
        want_long = "kv" + hashlib.sha1(long_name.encode()).hexdigest()[:14]
        want_odd = "kv" + hashlib.sha1(odd.encode()).hexdigest()[:14]
        self.assertEqual([n["vo"] for n in res["graph"]["entries"]], [want_long, want_odd])
        names = sorted(a[0] for a in res["assets"])
        self.assertEqual(names, sorted([f"{want_long}.wav", f"{want_long}.lip", f"{want_odd}.wav", f"{want_odd}.lip"]))
        self.assertTrue(all(D.RESREF_RE.fullmatch(n.rsplit(".", 1)[0]) for n in names))
        self.assertEqual(res["report"]["vo_renamed"], sorted([long_name, odd]))
        self.assertEqual(res["lip_asked"][0], [long_name, odd])  # the lip is looked up by the K1 name

    def test_rename_avoids_taken_names(self):
        a = D.rename_vo("a" * 20, set())
        b = D.rename_vo("a" * 20, {a})
        self.assertNotEqual(a, b)
        self.assertTrue(D.RESREF_RE.fullmatch(b))
        self.assertEqual(D.rename_vo("ok_name", {"ok_name"})[:2], "kv")  # a taken valid name is renamed too


class LipTests(unittest.TestCase):
    def test_a_failing_lip_is_not_shipped_but_the_voice_is(self):
        res = run(chain(2), vo=("v0", "v1"), lips={"v0": GOOD_LIP, "v1": BAD_LIP})
        rep = res["report"]
        self.assertEqual((rep["lips_ok"], rep["lips_missing"], [f["name"] for f in rep["lips_failed"]]), (1, [], ["v1"]))
        self.assertTrue(rep["lips_failed"][0]["problems"])
        self.assertEqual(sorted(a[0] for a in res["assets"]), ["v0.lip", "v0.wav", "v1.wav"])
        self.assertEqual([n["vo"] for n in res["graph"]["entries"]], ["v0", "v1"])
        self.assertEqual({v["name"]: v["lip"] for v in res["voiced"]}, {"v0": "ok", "v1": "failed"})
        self.assertEqual(rep["asset_count"], 1 + 2 + 1)

    def test_a_missing_lip_ships_the_wav_alone(self):
        res = run(chain(2), vo=("v0", "v1"), lips={"v0": GOOD_LIP})
        self.assertEqual(res["report"]["lips_missing"], ["v1"])
        self.assertEqual(sorted(a[0] for a in res["assets"]), ["v0.lip", "v0.wav", "v1.wav"])

    def test_every_lip_check_rule_is_applied(self):
        short = GOOD_LIP[:-1]
        empty = b""
        res = run(chain(3), vo=("v0", "v1", "v2"), lips={"v0": short, "v1": empty, "v2": GOOD_LIP})
        self.assertEqual(res["report"]["lips_ok"], 1)
        self.assertEqual(res["report"]["lips_missing"], ["v1"])  # an empty lip counts as missing
        self.assertEqual([f["name"] for f in res["report"]["lips_failed"]], ["v0"])


class IndexTests(unittest.TestCase):
    def test_nested_case_insensitive_index_skips_symlinks(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td) / "StreamWaves"
            (root / "M13AA" / "CART02").mkdir(parents=True)
            (root / "M13AA" / "CART02" / "NM13AACART02016_.wav").write_bytes(b"a")
            (root / "TOP.WAV").write_bytes(b"t")
            (root / "notes.txt").write_bytes(b"n")
            (root / "other").mkdir()
            (root / "other" / "nm13aacart02016_.wav").write_bytes(b"dup")
            outside = Path(td) / "outside"
            outside.mkdir()
            (outside / "evil.wav").write_bytes(b"e")
            (root / "linkdir").symlink_to(outside)
            (root / "link.wav").symlink_to(outside / "evil.wav")
            index, info = D.build_vo_index(root)
            self.assertEqual(sorted(index), ["nm13aacart02016_", "top"])
            self.assertEqual(index["nm13aacart02016_"].parent.name, "CART02")  # first in sorted path order wins
            self.assertEqual(info["duplicates"], ["nm13aacart02016_"])
            self.assertEqual(info["skipped_symlinks"], 2)
            self.assertEqual(D.build_vo_index(Path(td) / "nope")[0], {})

    def test_index_paths_are_read_as_wav_bytes(self):
        with tempfile.TemporaryDirectory() as td:
            f = Path(td) / "a.wav"
            f.write_bytes(b"\xff\xfbMP3")
            res = D.convert_dialog(graph([node("x", "a")], [], [0]), lambda r: None, {"a": f}, lambda n: {"a": GOOD_LIP}, max_voiced=3)
            self.assertEqual([a[1] for a in res["assets"] if a[2] == "wav"], [b"\xff\xfbMP3"])


class ReportTests(unittest.TestCase):
    def test_report_lines_carry_the_counts(self):
        g = graph([node("a", "v0", [link(0)]), node("b")], [node("r", "", [link(1, active="k")])], [0])
        rep = run(g, vo=("v0",))["report"]
        text = "\n".join(D.report_lines(rep))
        self.assertIn("nodes 3 total (2 entries, 1 replies): 2 kept", text)
        self.assertIn("starters 1 total, 1 without a condition, 1 kept", text)
        self.assertIn("1/30 voiced line(s)", text)
        self.assertIn("lips 1 ok, 0 missing, 0 failed", text)


class HelperTests(unittest.TestCase):
    def run_helper(self, *argv, backend=None):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = D.helper_main(list(argv), backend_factory=backend or FakeBackend)
        return rc, out.getvalue(), err.getvalue()

    def make_k1(self, td):
        k1 = Path(td) / "k1"
        (k1 / "_bif").mkdir(parents=True)
        (k1 / "lips").mkdir()
        (k1 / "dialog.tlk").write_text(json.dumps({"7": "Hello there"}))
        (k1 / "_bif" / "k_x_dialog.dlg.json").write_text(json.dumps(graph([node("", "v0", strref=7)], [], [0])))
        (k1 / "lips" / "b.mod").write_text(json.dumps({"v0": GOOD_LIP.hex()}))
        return k1

    def test_read_lips_write_ops_print_one_json_line_last(self):
        with tempfile.TemporaryDirectory() as td:
            k1 = self.make_k1(td)
            rc, out, err = self.run_helper("read", str(k1), str(k1 / "dialog.tlk"), "k_x_dialog")
            self.assertEqual(rc, 0, err)
            got = json.loads(out.strip().splitlines()[-1])
            self.assertTrue(got["found"])
            self.assertEqual((got["strings"], got["source"]), ({"7": "Hello there"}, "k_x_dialog.dlg.json"))
            self.assertEqual(got["sha256"], hashlib.sha256((k1 / "_bif" / "k_x_dialog.dlg.json").read_bytes()).hexdigest())
            rc, out, _ = self.run_helper("read", str(k1), str(k1 / "dialog.tlk"), "missing")
            self.assertEqual(json.loads(out)["found"], False)
            rc, out, _ = self.run_helper("lips", str(k1), "v0", "nolip")
            got = json.loads(out.strip().splitlines()[-1])["lips"]
            self.assertEqual(got["nolip"], None)
            self.assertEqual((got["v0"]["mod"], bytes.fromhex(kvoice_hex(got["v0"]["b64"]))), ("b.mod", GOOD_LIP))
            gf = Path(td) / "g.json"
            gf.write_text(json.dumps({"entries": []}))
            rc, out, _ = self.run_helper("write", str(gf))
            self.assertEqual(rc, 0)
            import base64
            self.assertTrue(base64.b64decode(json.loads(out)["data_b64"]).startswith(b"FAKEDLG:"))

    def test_unknown_op_and_errors(self):
        rc, _, err = self.run_helper("bogus")
        self.assertEqual(rc, 2)
        self.assertIn("unknown _dialog op", err)
        rc, _, err = self.run_helper("read", "/nonexistent", "/nonexistent/x.tlk", "r", backend=lambda: 1 / 0)
        self.assertEqual(rc, 1)
        self.assertIn("error: _dialog read", err)


def kvoice_hex(b64: str) -> str:
    import base64
    return base64.b64decode(b64).hex()


# ------------------------------------------------------------------------------------------- the JSON stand-in backend

class FakeBackend:
    """PykotorBackend's contract over a fake K1 dir: <k1>/{Override,_bif,modules}/<resref>.dlg.json holds a plain graph, the
    TLK is a JSON {strref: text}, lips live in <k1>/lips/*.mod as JSON {name: hex}; write() returns FAKEDLG: + the graph."""

    DIRS = ("Override", "_bif", "modules")

    def read(self, k1, resref, tlk_path):
        for d in self.DIRS:
            f = Path(k1) / d / f"{resref}.dlg.json"
            if f.is_file():
                raw = f.read_bytes()
                g = json.loads(raw)
                tlk = json.loads(Path(tlk_path).read_text())
                strings = {n["strref"]: tlk[str(n["strref"])] for n in g["entries"] + g["replies"]
                           if n["strref"] >= 0 and str(n["strref"]) in tlk}
                return g, strings, f.name, hashlib.sha256(raw).hexdigest()
        return None

    def lips(self, k1, names):
        found = {}
        ldir = Path(k1) / "lips"
        for mod in sorted(ldir.glob("*.mod")) if ldir.is_dir() else []:
            for name, hx in json.loads(mod.read_text()).items():
                found.setdefault(name.lower(), (bytes.fromhex(hx), mod.name))
        return {n: found.get(n.lower()) for n in names}

    def write(self, graph_):
        return b"FAKEDLG:" + json.dumps(graph_, sort_keys=True).encode()


# ------------------------------------------------------------------------------------------- the real pykotor backend

PYKOTOR_FLAG = "--pykotor-selftest"


def pykotor_selftest() -> int:
    """Runs under the k1port venv python: PykotorBackend.write -> bytes -> pykotor read -> PykotorBackend._graph."""
    import logging
    logging.disable(logging.CRITICAL)
    from pykotor.resource.formats.erf import ERF, ERFType, bytes_erf
    from pykotor.resource.generics.dlg import read_dlg
    from pykotor.resource.type import ResourceType
    be = D.PykotorBackend()
    ents = [node("Yes? What's on your mind?", "nm35aacart04009_", [link(0), link(1)], speaker="Bastila", delay=-1,
                 animations=[{"animation": 28, "participant": "OWNER"}], camera_angle=2),
            node("Fine.", "", [], delay=2000)]
    reps = [node("Tell me more.", "", [link(1, is_child=True)]), node("Never mind.", "")]
    g = graph(ents, reps, [0])
    data = be.write(g)
    assert data[:8] == b"DLG V3.2", data[:8]
    dlg = read_dlg(data)
    back = be._graph(dlg)
    assert [n["text"] for n in back["entries"]] == ["Yes? What's on your mind?", "Fine."], back["entries"]
    assert [n["strref"] for n in back["entries"] + back["replies"]] == [-1, -1, -1, -1]
    assert back["entries"][0]["vo"] == "nm35aacart04009_" and back["entries"][1]["vo"] == ""
    assert back["entries"][0]["speaker"] == "Bastila" and back["entries"][0]["camera_angle"] == 2
    assert back["entries"][0]["animations"] == [{"animation": 28, "participant": "OWNER"}]
    assert back["entries"][1]["delay"] == 2000 and back["entries"][0]["delay"] == -1
    assert [l["index"] for l in back["entries"][0]["links"]] == [0, 1]
    assert back["replies"][0]["links"][0]["is_child"] is True and back["starters"][0]["index"] == 0
    assert all(l["active"] == "" and l["active2"] == "" for n in back["entries"] + back["replies"] for l in n["links"])
    assert all(n["script"] == "" and n["sound"] == "" and n["quest"] == "" for n in back["entries"] + back["replies"])
    assert all(n["camera_id"] is None and n["camera_effect"] is None for n in back["entries"] + back["replies"])
    node0 = dlg.starters[0].node
    assert node0.plot_index == -1 and node0.plot_xp_percentage == 0.0
    assert dlg.skippable and dlg.unequip_items and not dlg.old_hit_check
    # lips: an ERF archive stub, case-insensitive name match, first mod in sorted order wins
    with tempfile.TemporaryDirectory() as td:
        k1 = Path(td) / "swkotor"
        (k1 / "Lips").mkdir(parents=True)
        for modname, name, payload in (("b_loc.mod", "nm35aacart04009_", GOOD_LIP), ("a_loc.mod", "nm35aacart04009_", BAD_LIP),
                                       ("c_loc.mod", "other_", GOOD_LIP)):
            erf = ERF(ERFType.MOD)
            erf.set_data(name, ResourceType.LIP, payload)
            (k1 / "Lips" / modname).write_bytes(bytes_erf(erf))
        got = be.lips(k1, ["NM35AACART04009_", "other_", "absent_"])
        assert got["NM35AACART04009_"] == (BAD_LIP, "a_loc.mod"), got
        assert got["other_"] == (GOOD_LIP, "c_loc.mod") and got["absent_"] is None
    print("pykotor selftest PASS")
    return 0


class PykotorTests(unittest.TestCase):
    @unittest.skipUnless(VENV_PY.exists(), f"{VENV_PY} missing (pykotor venv not set up)")
    def test_pykotor_backend_writes_reads_and_finds_lips(self):
        proc = subprocess.run([str(VENV_PY), "-I", "-B", str(Path(__file__).resolve()), PYKOTOR_FLAG],
                              capture_output=True, text=True, cwd=str(HERE))
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertIn("pykotor selftest PASS", proc.stdout)


if __name__ == "__main__":
    if PYKOTOR_FLAG in sys.argv:
        sys.exit(pykotor_selftest())
    unittest.main(verbosity=1)
