#!/usr/bin/env python3
"""Stdlib unittest for scripts/quick_loot_mockup.py: formatter port vs quick_loot_logic_test.cpp vectors, the list layer
(row text, scroll view, block build, placement, Took duration; QL2V 0.5.0-dev: row / icon geometry, RowH variants, icon resref, scrollbar layout,
hit test and click steps) vs the same C++ vectors, width measurement on a synthetic glyph
table, and (skipped without PIL/pykotor/the install) a real render smoke test.

run: python3 -m unittest scripts/test_quick_loot_mockup.py
"""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import quick_loot_mockup as q  # noqa: E402


class FormatterVectors(unittest.TestCase):
    N3 = ["Medpac", "Blaster Pistol", "Vibroblade"]

    def test_took_stack_suffix(self):
        # cpp: "count in brackets, stack suffix for 2 and 999, none for 1"
        self.assertEqual(q.fmt_took(self.N3, [2, 1, 999], 3), "Acquired (3): Medpac x2, Blaster Pistol, Vibroblade x999")
        self.assertEqual(q.fmt_took(["Medpac"], None, 1), "Acquired: Medpac")

    def test_took_garbage_names(self):
        ng = ["Med\x01pac\xff", "", None, "   ", "Name\0Hidden"]
        self.assertEqual(q.fmt_took(ng, None, 5), "Acquired (5): Med?pac?, item, item, item, Name")

    def test_review_examples(self):
        # cpp: review examples 1 and 2, and the Took twin
        n2 = ["Medpac", "Blaster Pistol"]
        self.assertEqual(q.fmt_took(n2, [2, 1], 2, 2), "Acquired (4): Medpac x2, Blaster Pistol, +2 more")
        self.assertEqual(q.fmt_peek("F", 4, n2, [2, 1], 2), "[F] Get Items (4): Medpac x2, Blaster Pistol, +2 more")
        real = ["Medpac", "Blaster Pistol", "Adhesive Grenade", "Lightsaber Crystal: Opila"] + ["Z%d" % i for i in range(12)]
        st = [2] + [1] * 15
        self.assertEqual(q.fmt_peek("F", 25, real, st, 16), "[F] Get Items (25): Medpac x2, Blaster Pistol, +23 more")

    def test_took_56_boundary(self):
        a, b13, b14 = "A" * 20, "B" * 13, "B" * 14
        self.assertEqual(len(q.fmt_took([a, b13], [99, 9], 2)), 56)          # exactly 56 fits whole
        self.assertEqual(q.fmt_took([a, b14], [99, 9], 2), "Acquired (2): " + a + " x99, +1 more")   # 57 overflows

    def test_peek_label(self):
        self.assertEqual(q.fmt_peek("F", 3, self.N3, [2, 1, 1], 3), "[F] Get Items (3): Medpac x2, Blaster Pistol, Vibroblade")
        self.assertEqual(q.fmt_peek("F", 1, ["Medpac"], None, 1), "[F] Get Item: Medpac")
        self.assertEqual(q.fmt_peek("F", 6, self.N3, [2, 1, 1], 3), "[F] Get Items (6): Medpac x2, Blaster Pistol, +4 more")
        self.assertEqual(q.fmt_peek("F", 0, self.N3, [2, 1, 1], 0), "")
        self.assertEqual(q.fmt_peek("F", 4, self.N3, [2, 1, 1], 0), "")

    def test_peek_56_boundary(self):
        a, b, c = "A" * 20, "B" * 15, "B" * 16
        self.assertEqual(len(q.fmt_peek("F", 2, [a, b], None, 2)), 56)
        self.assertIn(", +1 more", q.fmt_peek("F", 2, [a, c], None, 2))

    def test_long_list_caps(self):
        nm = ["N" * 20] * 16
        t = q.fmt_peek("F", 25, nm, None, 16)
        self.assertLessEqual(len(t), q.PEEK_MAX_CHARS)
        k = int(t.rsplit("+", 1)[1].split()[0])
        self.assertEqual(t.count("N" * 20) + k, 25)

    def test_entry_cut(self):
        # cpp: entryName vectors
        self.assertEqual(q.entry_name("Q" * 20), "Q" * 20)
        self.assertEqual(q.entry_name("Q" * 21), "Q" * 17 + "...")
        self.assertEqual(q.entry_name("Lightsaber Crystal: Opila"), "Lightsaber Crysta...")
        self.assertEqual(q.entry_name("Abcdefghijklmn: --xyzw"), "Abcdefghijklmn...")
        self.assertEqual(len(q.fmt_took(["Z" * 100], None, 1)), 30)
        self.assertEqual(q.fmt_took(["Lightsaber Crystal: Opila"], [3], 1), "Acquired: Lightsaber Crysta... x3")
        self.assertEqual(len(q.sanitize_name("Z" * 100)), 40)

    def test_took_duration(self):
        self.assertEqual(q.toast_duration_ms(3, 0), 3000)
        self.assertEqual(q.toast_duration_ms(4, 0), 3500)
        self.assertEqual(q.toast_duration_ms(2, 1), 3500)
        self.assertLessEqual(q.toast_duration_ms(16, 4000), q.TOAST_CAP_MS)


class Geometry(unittest.TestCase):
    def test_rects(self):
        # cpp: toastRect / peekRectFixed; both labels are 1200 px
        self.assertEqual(q.PEEK_W, 1200)
        self.assertEqual(q.TOAST_W, 1200)
        self.assertEqual(q.toast_rect(100, 50, 200, 24, w=q.PEEK_W), (100 + 100 - 600, 50 + 24 + 20, 1200, 20))
        self.assertEqual(q.fixed_rect(0, 0, 3840, 1600), (1920 - 600, 992, 1200, 20))

    def test_clamp(self):
        self.assertEqual(q.clamp_label_x((-100, 5, 1200, 20), 0, 3840)[0], 24)
        self.assertEqual(q.clamp_label_x((3000, 5, 1200, 20), 0, 3840)[0], 3840 - 24 - 1200)
        self.assertEqual(q.clamp_label_x((1000, 5, 1200, 20), 0, 3840)[0], 1000)
        self.assertEqual(q.clamp_label_x((5, 5, 1200, 20), 0, 0)[0], 5)
        self.assertEqual(q.clamp_label_x((5, 5, 1200, 20), 0, 1000)[0], (1000 - 1200) // 2)
        self.assertEqual(q.toast_rect(100, 522, 300, 32, screen=(0, 3840))[0], 24)

    def test_backing(self):
        lab = (1000, 600, 1200, 20)
        self.assertEqual(q.back_rect(lab, 200), (990, 596, 1220, 28))
        self.assertEqual(q.back_rect(lab, 28)[2], 28 * 24 + 20)
        x, y, w, h = q.back_rect(lab, 28)
        self.assertEqual(x + w // 2, 1600)


class ListLayer(unittest.TestCase):
    """Ports of the QL2L layer in QuickLootLogic.h; vectors mirror quick_loot_logic_test.cpp."""

    def test_text_px10(self):
        self.assertEqual(q.text_px10(""), 0)
        self.assertEqual(q.text_px10("Medpac x2"), 201)   # cpp pin + cpm.text_size 200.5
        self.assertGreater(q.text_px10("ab"), q.text_px10("a"))

    def test_row_name(self):
        self.assertEqual(q.row_name("Medpac"), "Medpac")
        self.assertEqual(q.row_name("Lightsaber Crystal: Opila"), "Lightsaber Crystal: Opila")
        n40 = "0123456789" * 4
        self.assertEqual(q.row_name(n40), n40)
        self.assertEqual(q.row_name(n40 + "X"), "0123456789012345678901234567890123456...")
        self.assertEqual(len(q.row_name(n40 + "X")), 40)
        self.assertTrue(q.row_name("Mandalorian Heavy Armor Plating Mk IV Upgrade Kit").endswith("..."))
        self.assertEqual(q.row_name("Name   "), "Name")
        self.assertEqual(q.row_name(""), "item")
        self.assertEqual(q.row_name(None), "item")
        self.assertEqual(q.row_name("Caf\xc3\xa9"), "Caf??")
        self.assertNotIn(", ...", q.row_name("Aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, , , , bbb"))

    def test_row_text(self):
        self.assertEqual(q.row_text("Medpac", 2), "Medpac x2")
        self.assertEqual(q.row_text("Medpac", 1), "Medpac")
        self.assertEqual(q.row_text("Medpac", 999), "Medpac x999")
        self.assertEqual(q.row_text("Medpac", 1000), "Medpac")
        self.assertEqual(q.row_text("Medpac", -1), "Medpac")
        t = q.row_text("W" * 42, 999)
        self.assertLessEqual(q.text_px10(t), q.ROW_TEXT_MAX_PX)
        self.assertTrue(t.endswith("... x999"))
        t = q.row_text("W" * 20, 1, 200)
        self.assertLessEqual(q.text_px10(t), 200)
        self.assertTrue(t.endswith("..."))

    def test_list_data(self):
        d = q.ListData(["Medpac", "Blaster Pistol", "Adhesive Grenade"], [2, 1, 3])
        self.assertEqual((d.items, d.more, d.rows_total(), d.total_items()), (3, 0, 3, 3))
        self.assertEqual((d.row_at(0), d.row_at(2), d.row_at(3)), ("Medpac x2", "Adhesive Grenade x3", ""))
        d = q.ListData(["Medpac", "Blaster Pistol", "Adhesive Grenade"], [2, 1, 3], total=70)
        self.assertEqual((d.more, d.rows_total(), d.total_items(), d.row_at(3)), (67, 4, 70, "+67 more"))
        self.assertEqual(q.ListData(["a"], None, total=0).more, 0)
        self.assertEqual(q.ListData([], None, total=5).rows_total(), 0)
        big = q.ListData(["X"] * 100, None)
        self.assertEqual((big.items, big.more), (64, 36))

    def test_view_scroll(self):
        v = q.ListView(25, 8)
        self.assertEqual(v.max_off(), 17)
        self.assertFalse(v.scroll(120))
        self.assertTrue(v.scroll(-120) and v.off == 1)
        self.assertTrue(v.scroll(120) and v.off == 0)
        self.assertTrue(v.scroll(-360) and v.off == 3)
        self.assertFalse(v.scroll(-60))
        self.assertTrue(v.scroll(-60) and v.off == 4)
        self.assertTrue(v.scroll(-120 * 99) and v.off == 17)
        self.assertFalse(v.scroll(-120))
        self.assertTrue(v.scroll(120) and v.off == 16)      # the clamp drops the remainder
        self.assertTrue(v.scroll(120 * 99) and v.off == 0)
        self.assertFalse(q.ListView(8, 8).scroll(-120))     # exactly the rows: not scrollable

    def test_build(self):
        names = ["Item %d" % (i + 1) for i in range(25)]
        d = q.ListData(names, None)
        v = q.ListView(d.rows_total(), 8)
        o = q.build_list_draw(d, v, False)
        self.assertEqual((o["header"], o["indicator"], o["vis"], o["off"]), ("[F] Get Items (25)", "1-8 of 25", 8, 0))
        self.assertEqual(o["h"], q.block_height_px(8))
        self.assertTrue(q.BLOCK_MIN_W <= o["w"] <= q.BLOCK_MAX_W)
        v.off = 10
        o2 = q.build_list_draw(d, v, False)
        self.assertEqual((o2["rows"][0], o2["indicator"], o2["w"], o2["h"]), ("Item 11", "11-18 of 25", o["w"], o["h"]))   # scrolling never resizes
        v.off = 99
        self.assertEqual(q.build_list_draw(d, v, False)["off"], 17)
        d4 = q.ListData(names[:4], None)
        o4 = q.build_list_draw(d4, q.ListView(4, 8), True)
        self.assertEqual((o4["header"], o4["vis"], o4["indicator"]), ("Acquired Items (4)", 4, ""))
        self.assertTrue(q.BLOCK_MIN_W <= o4["w"] and not o4["sb"])
        d5 = q.ListData(names[:20], None, total=25)
        v5 = q.ListView(d5.rows_total(), 8)
        v5.off = v5.max_off()
        o5 = q.build_list_draw(d5, v5, False)
        self.assertEqual((o5["rows"][7], o5["indicator"]), ("+5 more", "14-25 of 25"))
        self.assertIsNone(q.build_list_draw(q.ListData([], None), v5, False))
        self.assertEqual(q.build_list_draw(d, q.ListView(25, 40), False)["vis"], 12)
        self.assertEqual(q.clamp_list_rows(0), 1)
        self.assertEqual(q.clamp_list_rows(40), 12)

    def test_took_duration(self):
        self.assertEqual([q.took_duration_ms(n) for n in (1, 3, 4, 8, 9, 65)], [3000, 3000, 3500, 5500, 6000, 6000])

    def test_placement(self):
        panel = (0, 0, 3840, 1600)
        name = (1700, 520, 533, 32)
        a = q.choose_anchor(True, True, name, True, panel)
        self.assertEqual(a, ("A", 1966, 572, 520, True))
        self.assertEqual(q.block_rect(a, 400, 334, panel), (1766, 572, 400, 334))
        b = q.choose_anchor(False, True, name, True, panel)
        self.assertEqual(b, ("B", 1920, 992, 0, False))
        self.assertEqual(q.block_rect(b, 400, 334, panel), (1720, 992, 400, 334))
        self.assertIsNone(q.choose_anchor(False, False, name, False, (0, 0, 0, 0)))
        self.assertEqual(q.block_rect(("A", 50, 100, 0, False), 400, 300, panel)[0], 24)
        self.assertEqual(q.block_rect(("A", 3830, 100, 0, False), 400, 300, panel)[0], 3840 - 24 - 400)
        # bottom clamp: A flips above the name, B moves up, too tall -> top limit
        self.assertEqual(q.block_rect(("A", 1966, 1352, 1300, True), 400, 334, panel)[1], 1300 - 20 - 334)
        self.assertEqual(q.block_rect(("B", 1920, 992, 0, False), 400, 334, (0, 0, 3840, 600))[1], 600 - 24 - 334)
        self.assertEqual(q.block_rect(("B", 1920, 992, 0, False), 400, 900, (0, 0, 3840, 600))[1], 24)
        self.assertEqual(q.header_rect(400), (q.BLOCK_PAD_X, q.BLOCK_PAD_Y, 400 - 2 * q.BLOCK_PAD_X + q.LABEL_SLACK, q.HEADER_H))
        self.assertEqual(q.indicator_rect(500, 120)[0] + 120, 500 - q.BLOCK_PAD_X)
        self.assertEqual(q.row_rect(3, 400)[1], q.row_rect(0, 400)[1] + 3 * (q.ROW_H + q.ROW_GAP))

    def test_samples_fit(self):
        for smp in q.samples():
            for rh in (q.ROW_H, *q.ROW_H_ALTS):
                fr = q.fit_row(smp, rh)
                self.assertTrue(fr["rows_fit"] and fr["head_fit"] and fr["inside"] and fr["sb_inside"], (fr["desc"], rh))
        self.assertIn("rows_fit", q.format_table([q.fit_row(q.samples()[0])]))


class RowsAndScrollbar(unittest.TestCase):
    """QL2V ports; vectors mirror the 0.5.0-dev block of quick_loot_logic_test.cpp."""

    def test_row_geometry(self):
        self.assertEqual((q.ROW_H, q.ROW_H_ALTS), (64, (56, 72)))
        self.assertEqual((q.icon_px(), q.icon_px(56), q.icon_px(72)), (56, 48, 64))
        r0, r1, r0s = q.row_rect(0, 800), q.row_rect(1, 800), q.row_rect(0, 800, True)
        self.assertEqual((r1[1] - r0[1], r0[3], r0[0]), (q.ROW_H + q.ROW_GAP, q.ROW_H, q.BLOCK_PAD_X))
        self.assertEqual((r0[2], r0s[2]), (800 - 2 * q.BLOCK_PAD_X, 800 - 2 * q.BLOCK_PAD_X - q.SB_RESERVE))
        ic, tx = q.row_icon_rect(r0), q.row_text_rect(r0)
        self.assertEqual((ic[2], ic[3]), (ic[2], ic[2]))
        self.assertEqual(ic[1] + ic[3], r0[1] + r0[3] - q.ROW_ICON_PAD)
        self.assertEqual((tx[0], tx[1], tx[3]), (ic[0] + ic[2] + q.ICON_GAP, r0[1] + (q.ROW_H - q.TEXT_GLYPH_H) // 2, q.TEXT_BOX_H))
        self.assertEqual(tx[0] + tx[2] - q.LABEL_SLACK, r0[0] + r0[2] - q.ROW_TEXT_PAD_R)

    def test_row_h_variants_fit(self):
        for rh in (56, 64, 72):
            self.assertLessEqual(q.block_height_px(12, rh) + 2 * q.SCREEN_MARGIN, 1600, rh)
            self.assertEqual(q.block_height_px(8, rh), 2 * q.BLOCK_PAD_Y + q.HEADER_H + q.HEADER_GAP + 8 * rh + 7 * q.ROW_GAP)
            self.assertTrue(q.sb_fits(2, rh) and not q.sb_fits(1, rh), rh)

    def test_resref(self):
        self.assertEqual(q.sanitize_resref("a" * 16), "a" * 16)
        self.assertEqual(q.sanitize_resref("a" * 20), "a" * 16)           # only the 16 engine bytes are read
        self.assertEqual(q.sanitize_resref("iw_blstrpstl_001"), "iw_blstrpstl_001")
        self.assertEqual(q.sanitize_resref("ii_medeqpmnt_1\0xx"), "ii_medeqpmnt_1")
        self.assertEqual(q.sanitize_resref("ia\x01_c\xffass"), "ia?_c?ass")
        self.assertEqual(q.sanitize_resref("abc   "), "abc")
        self.assertEqual((q.sanitize_resref(""), q.sanitize_resref(None)), ("", ""))
        self.assertTrue(q.base_item_idx_ok(0) and q.base_item_idx_ok(1023) and not q.base_item_idx_ok(-1) and not q.base_item_idx_ok(1024))
        self.assertEqual(q.icon_resref("w_BlstrPstl", 1), "iw_blstrpstl_001")   # 0x6D4E60 "i%s_%03d" + the engine's lowercase lookup
        self.assertEqual(q.icon_resref("a_Class4", 12), "ia_class4_012")

    def test_list_icons(self):
        d = q.ListData(["A", "B", "C"], None, total=10, icons=["ok_icon", None, "x" * 30])
        self.assertEqual((d.icon_at(0), d.icon_at(1), d.icon_at(2), d.icon_at(3), d.icon_at(99)), ("ok_icon", "", "x" * 16, "", ""))
        o = q.build_list_draw(d, q.ListView(d.rows_total(), 2), False)
        self.assertEqual(o["icons"], ["ok_icon", ""])
        v = q.ListView(d.rows_total(), 2)
        v.off = 99
        self.assertEqual(q.build_list_draw(d, v, False)["icons"], ["x" * 16, ""])   # the "+7 more" row has none

    def test_scrollbar_layout(self):
        w = 800
        s0, sm, se = q.sb_layout(w, 8, 25, 0), q.sb_layout(w, 8, 25, 8), q.sb_layout(w, 8, 25, 17)
        self.assertTrue(s0["shown"] and sm["shown"] and se["shown"])
        self.assertFalse(q.sb_layout(w, 6, 6, 0)["shown"] or q.sb_layout(w, 8, 8, 0)["shown"] or q.sb_layout(w, 1, 25, 0)["shown"])
        self.assertEqual(s0["col"], (w - q.BLOCK_PAD_X - q.SB_W, q.rows_top_px(), q.SB_W, q.list_height_px(8)))
        c = s0["col"]
        self.assertEqual(s0["up"], (c[0], c[1], q.SB_W, q.SB_W))
        self.assertEqual(s0["down"], (c[0], c[1] + c[3] - q.SB_W, q.SB_W, q.SB_W))
        t = s0["track"]
        self.assertEqual((t[1], t[1] + t[3]), (s0["up"][1] + q.SB_W, s0["down"][1]))
        th = t[3] * 8 // 25
        self.assertEqual(s0["thumb"][3], th)
        self.assertEqual(s0["thumb"][1], t[1])
        self.assertEqual(se["thumb"][1] + se["thumb"][3], t[1] + t[3])
        self.assertEqual(sm["thumb"][1], t[1] + (t[3] - th) * 8 // 17)
        self.assertEqual(q.sb_layout(w, 8, 25, 99)["thumb"], se["thumb"])
        self.assertEqual(q.sb_layout(w, 8, 4000, 0)["thumb"][3], q.SB_THUMB_MIN)

    def test_scrollbar_hit_and_steps(self):
        sm = q.sb_layout(800, 8, 25, 8)
        cx = sm["col"][0] + q.SB_W / 2
        th = sm["thumb"]
        self.assertEqual(q.sb_hit_at(sm, cx, sm["up"][1] + 5), q.SB_UP)
        self.assertEqual(q.sb_hit_at(sm, cx, sm["down"][1] + q.SB_W - 1), q.SB_DOWN)
        self.assertEqual(q.sb_hit_at(sm, cx, th[1] - 2), q.SB_PAGE_UP)
        self.assertEqual(q.sb_hit_at(sm, cx, th[1] + th[3] + 2), q.SB_PAGE_DOWN)
        self.assertEqual(q.sb_hit_at(sm, cx, th[1] + th[3] / 2), q.SB_THUMB)
        self.assertEqual(q.sb_hit_at(sm, sm["col"][0] - 1, sm["track"][1] + 5), q.SB_NONE)
        self.assertEqual(q.sb_hit_at(sm, cx, sm["col"][1] - 1), q.SB_NONE)
        self.assertEqual(q.sb_hit_at({"shown": False}, cx, 100), q.SB_NONE)
        self.assertEqual([q.sb_step_for(p, 8) for p in (q.SB_UP, q.SB_DOWN, q.SB_PAGE_UP, q.SB_PAGE_DOWN, q.SB_THUMB, q.SB_NONE)], [-1, 1, -8, 8, 0, 0])
        v = q.ListView(25, 8)
        self.assertTrue(v.step(1) and v.off == 1)
        self.assertTrue(v.step(8) and v.off == 9)
        self.assertTrue(v.step(8) and v.off == 17)
        self.assertFalse(v.step(8))
        self.assertFalse(v.step(1))
        self.assertTrue(v.step(-8) and v.off == 9)
        self.assertTrue(v.step(-8) and v.step(-8) and v.off == 0)
        self.assertFalse(v.step(-1))
        v.acc = 77
        v.step(1)
        self.assertEqual(v.acc, 0)
        self.assertFalse(q.ListView(5, 8).step(3))

    def test_row_at_point(self):
        blk = (100, 100, 900, 700)
        y0 = 100 + q.rows_top_px()
        self.assertEqual(q.row_at_point(blk, 8, 300, y0 + 10, True), 0)
        self.assertEqual(q.row_at_point(blk, 8, 300, y0 + q.ROW_H + 1, True), -1)             # the gap
        self.assertEqual(q.row_at_point(blk, 8, 300, y0 + q.ROW_H + q.ROW_GAP + 1, True), 1)
        self.assertEqual(q.row_at_point(blk, 8, 100 + 900 - q.BLOCK_PAD_X - 5, y0 + 10, True), -1)   # the scrollbar column
        self.assertEqual(q.row_at_point(blk, 8, 100 + 900 - q.BLOCK_PAD_X - 5, y0 + 10, False), 0)


class WidthMeasure(unittest.TestCase):
    def test_synthetic_glyph_table(self):
        adv = [0] * 256
        adv[ord("A")], adv[ord("B")], adv[ord(" ")], adv[ord("?")] = 8, 6, 3, 5
        self.assertEqual(q.text_width("AB A", adv, 2.0), (8 + 6 + 3 + 8) * 2.0)
        self.assertEqual(q.text_width("", adv, 2.75), 0)
        self.assertEqual(q.text_width("中", adv, 1.0), 5)   # non-Latin1 measures as '?'
        # fit rule: width <= label width
        w = q.text_width("A" * 10, adv, 2.75)
        self.assertEqual(w, 220.0)
        self.assertTrue(w <= q.TOAST_W)
        self.assertFalse(q.text_width("A" * 60, adv, 2.75) <= q.TOAST_W)


class Ql06Ports(unittest.TestCase):
    """0.6.0-dev ports (anchor modes, flash, tooltip) vs the quick_loot_logic_test.cpp vectors."""
    PANEL = (0, 0, 3840, 1600)
    NAME = (1700, 520, 533, 32)

    def test_modes(self):
        r = q.anchor_block(0, True, self.NAME, self.PANEL, None, 400, 300)
        self.assertEqual(r[:3], ((1766, 572, 400, 300), 0, "ok"))
        self.assertIsNone(q.anchor_block(0, False, self.NAME, self.PANEL, None, 400, 300))
        self.assertEqual(q.anchor_block(2, True, self.NAME, self.PANEL, None, 400, 300)[0], (1700 + 533 + q.BESIDE_GAP, 520, 400, 300))
        self.assertEqual(q.anchor_block(2, True, (3500, 520, 300, 32), self.PANEL, None, 400, 300)[0], (3500 - q.BESIDE_GAP - 400, 520, 400, 300))
        self.assertEqual(q.anchor_block(2, True, (1700, 1500, 533, 32), self.PANEL, None, 400, 300)[0][1], 1600 - 24 - 300)
        self.assertEqual(q.anchor_block(1, False, self.NAME, self.PANEL, (1000, 700, 1), 400, 300)[:3], ((800, 700 + q.PROJ_GAP, 400, 300), 1, "ok"))
        self.assertEqual(q.anchor_block(1, False, self.NAME, self.PANEL, (1000, 1590, 1), 400, 300)[0][1] + 300, 1600 - 24)
        fb = q.anchor_block(1, True, self.NAME, self.PANEL, (1000, 700, 2), 400, 300)
        self.assertEqual((fb[0][1], fb[1], fb[2], fb[3]), (572, 0, "fallback-name", "behind"))
        self.assertIsNone(q.anchor_block(1, False, self.NAME, self.PANEL, (1000, 700, 2), 400, 300))
        self.assertEqual(q.anchor_block(1, True, self.NAME, self.PANEL, ("unavail", "vtable"), 400, 300)[3], "unavail")

    def test_proj_reject(self):
        p = self.PANEL
        self.assertIsNone(q.proj_reject(10, 10, 1, p))
        self.assertEqual(q.proj_reject(10, 10, 2, p), "behind")
        self.assertEqual(q.proj_reject(float("nan"), 1, 1, p), "nan")
        self.assertEqual(q.proj_reject(-1, 5, 1, p), "outside")
        self.assertEqual(q.proj_reject(5, 1600, 1, p), "outside")
        self.assertEqual(q.proj_reject(5, 5, 1, (0, 0, 0, 0)), "nopanel")

    def test_flash(self):
        self.assertEqual(q.fmt_flash_header("Medpac", 2000), "Acquired: Medpac")
        t = q.fmt_flash_header("Mandalorian Heavy Armor Plating Mk IV Upgrade", 600)
        self.assertTrue(q.text_px10(t) <= 600 and t.startswith("Acquired: Mandalorian") and "..." in t)
        self.assertEqual(q.fmt_flash_header(None, 2000), "Acquired: item")
        d = q.ListData(["Medpac", "Pistol"], [1, 1])
        plain = q.build_list_draw(d, q.ListView(2, 8), False)
        fl = q.build_list_draw(d, q.ListView(2, 8), False, flash_name="Frag Grenade Launcher Mk II")
        self.assertTrue(fl["header"].startswith("Acquired: Frag Grenade") and fl["w"] >= plain["w"] and fl["h"] == plain["h"] and fl["w"] <= q.BLOCK_MAX_W)
        self.assertTrue(q.build_list_draw(d, q.ListView(2, 8), True, flash_name="x")["header"].startswith("Acquired Items"))

    def test_sanitize_and_wrap(self):
        self.assertEqual(q.sanitize_desc("  A  good\r\n\r\n \n\nblaster.\t \n"), "A good\n\nblaster.")
        self.assertEqual(q.sanitize_desc("caf\xe9 ok"), "caf? ok")
        lines, widest, cut = q.tip_wrap("Duration: 120 sec\n\nA shot of this enhancer provides a temporary boost.", 900)
        self.assertEqual(lines[:3], ["Duration: 120 sec", "", "A shot of this enhancer provides a temporary"])
        self.assertFalse(cut)
        text = " ".join("word%d" % i for i in range(80))
        lines, widest, cut = q.tip_wrap(text, 500)
        self.assertEqual(len(lines), 10)
        self.assertTrue(cut and lines[-1].endswith("...") and widest <= 500)

    def test_tip(self):
        blk, row, panel = (1000, 500, 640, 400), (1028, 600, 560, 64), self.PANEL
        t = q.build_tip("Medpac", 5, "Heals a little.", blk, row, panel)
        self.assertEqual(t["frame"][:2], (1000 + 640 + q.TIP_GAP, 600))
        self.assertTrue(t["big_title"] and t["qty_text"] == "Quantity: 5" and t["lines"] == 1 and not t["flipped"])
        self.assertTrue(q.TIP_MIN_W <= t["frame"][2] <= q.TIP_MAX_W)
        t = q.build_tip("Medpac", 1, "", blk, row, panel)
        self.assertEqual((t["qty"], t["body"], t["frame"][3]), (None, None, q.TIP_PAD_Y * 2 + q.TIP_TITLE_H))
        t = q.build_tip("Medpac", 1, "x", (3000, 500, 640, 400), (3028, 600, 560, 64), panel)
        self.assertTrue(t["flipped"] and t["frame"][0] == 3000 - q.TIP_GAP - t["frame"][2])
        t = q.build_tip("Medpac", 3, " ".join("word%d" % i for i in range(80)), blk, (1028, 1500, 560, 64), panel)
        self.assertEqual((t["frame"][1] + t["frame"][3], t["lines"], t["truncated"], t["body"][3]), (1600 - 24, 10, True, 10 * q.TIP_LINE_H))
        t = q.build_tip("Mandalorian Heavy Armor Plating Mk IV Upgrade", 1, "", blk, row, panel)
        self.assertFalse(t["big_title"])


class RenderSmoke(unittest.TestCase):
    def test_render(self):
        try:
            import PIL  # noqa: F401
            cpm = q._cpm()
            if not (cpm.GAME / "override").is_dir():
                self.skipTest("game install missing")
            cpm.inst()
        except Exception as e:  # missing PIL / pykotor / install
            self.skipTest(f"render prerequisites missing: {e}")
        with tempfile.TemporaryDirectory() as d:
            rows, table, written = q.render_all(d)
            self.assertEqual(len(rows), 13)              # 10 samples + the low-target frame + RowH 56 / 72 variants
            self.assertEqual(len(written), 39)           # 10 samples x (full + zoom) + the low-target frame + rowh A / B + vs vanilla + 15 ql06 files
            from PIL import Image
            self.assertEqual(Image.open(written[0]).size, (3840, 1600))
            self.assertTrue(all(r["rows_fit"] and r["head_fit"] and r["inside"] and r["sb_inside"] for r in rows))   # every row fits its block, every block the panel
            self.assertTrue((Path(d) / "ql2v_fit_table.txt").is_file())
            for n in ("ql2v_rowh_A.png", "ql2v_rowh_B.png", "ql2v_vs_vanilla.png"):
                self.assertTrue((Path(d) / n).is_file(), n)


if __name__ == "__main__":
    unittest.main()
