#!/usr/bin/env python3
"""Stdlib unittest for scripts/isf_inventory_mockup.py: the SortLogic.h ports (captionWithCount, categoryOf, beforeBy / sortEntries, state cycle)
against vectors taken from patch_manager_mods/inventory_sort_filter_src/sort_logic_test.cpp, the sample backpack contract (22 items, 4 New, 3 open
upgrade slots, tinted rows) and, when the game install + PIL + pykotor exist, a render smoke test. Logic tests need no install."""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import isf_inventory_mockup as m  # noqa: E402

NU, US, UG = 1 << m.S_NEW, 1 << m.S_USABLE, 1 << m.S_UPGRADE
ALL3 = NU | US | UG


def ent(item, orig, is_new, usable, base, cat, name):
    return m.Entry(item=item, id=item, orig=orig, is_new=is_new, usable=usable, base_item=base, tint=False, name=name, category=cat)


class CaptionVectors(unittest.TestCase):
    def test_caption_of(self):
        self.assertEqual(m.caption_of(m.S_ALL), "Filter")
        self.assertEqual(m.caption_of(m.S_UPGRADE), "Filter: Upgradeable")

    def test_with_count_basic(self):  # cpp :136-139
        self.assertEqual(m.caption_with_count(m.S_ALL, ALL3, 5), "Filter")
        self.assertEqual(m.caption_with_count(m.S_USABLE, ALL3, 0), "Filter: Usable 2/3 (0)")
        self.assertEqual(m.caption_with_count(m.S_UPGRADE, ALL3, 123), "Filter: Upgradeable 3/3 (123)")
        self.assertEqual(m.caption_with_count(m.S_NEW, ALL3, 7), "Filter: New 1/3 (7)")

    def test_truncation(self):  # cpp :141-143, :219-226
        self.assertEqual(m.caption_with_count(m.S_UPGRADE, ALL3, 123, 8), "Filter:")
        self.assertEqual(m.caption_with_count(m.S_USABLE, ALL3, 0, 1), "")
        self.assertEqual(m.caption_with_count(m.S_UPGRADE, ALL3, 1024, 6), "Filte")
        self.assertIsNone(m.caption_with_count(m.S_UPGRADE, ALL3, 1024, 0))

    def test_cycle_position_table(self):  # cpp :190-205
        table = [
            (m.S_NEW, ALL3, 5, "Filter: New 1/3 (5)"), (m.S_USABLE, ALL3, 14, "Filter: Usable 2/3 (14)"),
            (m.S_UPGRADE, ALL3, 1024, "Filter: Upgradeable 3/3 (1024)"),
            (m.S_NEW, NU | US, 5, "Filter: New 1/2 (5)"), (m.S_USABLE, NU | US, 14, "Filter: Usable 2/2 (14)"),
            (m.S_USABLE, US | UG, 14, "Filter: Usable 1/2 (14)"), (m.S_UPGRADE, US | UG, 1024, "Filter: Upgradeable 2/2 (1024)"),
            (m.S_USABLE, NU | UG, 14, "Filter: Usable (14)"), (m.S_UPGRADE, NU | UG, 2, "Filter: Upgradeable 2/2 (2)"),
            (m.S_USABLE, US, 14, "Filter: Usable (14)"), (m.S_NEW, UG, 2, "Filter: New (2)"),
            (m.S_NEW, 0, 14, "Filter"), (m.S_UPGRADE, 0, 1024, "Filter"), (m.S_ALL, ALL3, 14, "Filter"),
        ]
        for s, mask, rows, want in table:
            self.assertEqual(m.caption_with_count(s, mask, rows), want)
        self.assertEqual(m.caption_with_count(m.S_USABLE, ALL3, -1), "Filter: Usable 2/3")

    def test_longest_is_30_chars(self):  # cpp :210-215
        s = m.caption_with_count(m.S_UPGRADE, ALL3, 1024)
        self.assertEqual(len(s), 30)
        self.assertLess(len(s), m.CAPTION_BYTES)

    def test_state_cycle(self):
        order, s = [], m.S_ALL
        for _ in range(4):
            s = m.next_state(s, ALL3)
            order.append(s)
        self.assertEqual(order, [m.S_NEW, m.S_USABLE, m.S_UPGRADE, m.S_ALL])
        self.assertEqual(m.next_state(m.S_ALL, UG), m.S_UPGRADE)
        self.assertEqual(m.next_state(m.S_ALL, 0), m.S_ALL)


class CategoryVectors(unittest.TestCase):
    def test_category_of(self):  # cpp :301-307
        c = m.category_of
        self.assertEqual((c(1, 0x30, False, 2), c(2, 0x10, False, 12), c(3, 0x2, False, 38)), (0, 1, 2))
        self.assertEqual((c(4, 0x8, False, 45), c(0, 0x400, False, 47), c(0, 0x200, False, 48)), (3, 3, 3))
        self.assertEqual((c(0, 0, False, m.BASE_UPGRADE_ITEMS), c(0, 0, False, m.BASE_LIGHTSABER_CRYSTALS)), (4, 4))
        self.assertEqual(c(0, 0, True, m.BASE_UPGRADE_ITEMS), 4)
        self.assertEqual((c(0, 0, True, 53), c(0, 0, False, 52), c(0, 0, False, 0)), (5, 6, 6))
        self.assertEqual(c(1, 0x30, True, 2), 0)
        self.assertEqual((c(0, 0, False, 100), c(0, 0, False, 102)), (6, 6))

    def test_tint_and_usable(self):
        self.assertTrue(m.tint_row(True, False))
        self.assertFalse(m.tint_row(True, True) or m.tint_row(False, False) or m.tint_row(False, True))
        self.assertTrue(m.usable_for_cycle(True, False) and m.usable_for_cycle(False, True) and not m.usable_for_cycle(False, False))


class SortVectors(unittest.TestCase):
    def o7(self):  # cpp :318-323: item/orig, new, usable, base, category, name
        return [ent(130, 0, False, True, 5, 6, "delta"), ent(131, 1, False, True, 9, 0, "alpha"), ent(132, 2, False, True, 3, 1, "charlie"),
                ent(133, 3, False, True, 1, 0, "bravo"), ent(134, 4, True, True, 7, 5, "zulu"), ent(135, 5, False, False, 2, 0, "echo"),
                ent(136, 6, False, True, 9, 0, "aardvark")]

    def test_four_orders(self):  # cpp :324-330
        want = {m.SO_CATEGORY: [134, 133, 136, 131, 132, 130, 135], m.SO_BASE: [134, 133, 132, 130, 136, 131, 135],
                m.SO_NAME: [134, 136, 131, 133, 132, 130, 135], m.SO_VANILLA: [130, 131, 132, 133, 134, 135, 136]}
        for order, ids in want.items():
            self.assertEqual([e.item for e in m.sort_entries(self.o7(), order)], ids, m.SORT_NAMES[order])

    def test_vanilla_is_noop(self):  # cpp :331
        r = list(reversed(self.o7()))
        self.assertEqual([e.item for e in m.sort_entries(r, m.SO_VANILLA)], [136 - i for i in range(7)])

    def test_default_is_base(self):  # cpp :333-335 (2-argument sortEntries == SO_Base)
        self.assertEqual([e.item for e in m.sort_entries(self.o7())], [e.item for e in m.sort_entries(self.o7(), m.SO_BASE)])

    def test_name_fold_and_empty_first(self):
        a = [ent(1, 0, False, True, 1, 0, "Bravo"), ent(2, 1, False, True, 1, 0, "alpha"), ent(3, 2, False, True, 1, 0, "")]
        self.assertEqual([e.item for e in m.sort_entries(a, m.SO_NAME)], [3, 2, 1])
        self.assertEqual(m.fold_cmp("Abc", "aBC"), 0)


class SampleBackpack(unittest.TestCase):
    def test_contract(self):
        es = m.build_entries()
        self.assertEqual(len(es), 22)
        self.assertEqual(sum(e.is_new for e in es), 4)
        self.assertEqual(sum(it.open_slot for it in m.SAMPLE), 3)
        self.assertEqual(sum(e.tint for e in es), 2)
        cats = {e.category for e in es}
        self.assertEqual(cats, {0, 1, 2, 3, 4, 5, 6})

    def test_filters(self):
        es = m.build_entries()
        self.assertEqual(len(m.visible_entries(m.S_NEW, es)), 4)
        self.assertTrue(all(e.usable for e in m.visible_entries(m.S_USABLE, es)))
        self.assertEqual({m.SAMPLE[e.item].name for e in m.visible_entries(m.S_UPGRADE, es)}, {"Blaster Pistol", "Padded Gloves", "Lightsaber"})
        self.assertFalse(any(e.tint for e in m.visible_entries(m.S_USABLE, es)))

    def test_category_order_groups(self):
        rows = m.arrange(m.S_ALL, m.SO_CATEGORY)
        # New block first, then usable by category, then the unusable block (tinted armour / implant, crystals, upgrades, misc)
        self.assertTrue(all(e.is_new for e in rows[:4]))
        usable = [e for e in rows[4:] if e.usable]
        self.assertEqual([e.category for e in usable], sorted(e.category for e in usable))
        first_unusable = 4 + next(i for i, e in enumerate(rows[4:]) if not e.usable)   # the New block (usable or not) comes first
        self.assertTrue(all(not e.usable for e in rows[first_unusable:]))

SORT_LOGIC_H = Path(__file__).resolve().parent.parent / "patch_manager_mods" / "inventory_sort_filter_src" / "SortLogic.h"


class VariantLogic(unittest.TestCase):
    """Redesign-variant tables against SortLogic.h (kCycleOrder, SortOrder enum); no install needed."""

    def src(self):
        return SORT_LOGIC_H.read_text()

    def test_filter_segments_match_kCycleOrder(self):
        import re
        mm = re.search(r"kCycleOrder\[\]\s*=\s*\{([^}]*)\}", self.src())
        names = [t.strip() for t in mm.group(1).split(",")]
        enum = {"S_New": m.S_NEW, "S_Usable": m.S_USABLE, "S_Upgrade": m.S_UPGRADE}
        self.assertEqual([enum[n] for n in names], list(m.CYCLE_ORDER))
        self.assertEqual(m.FILTER_SEGMENTS, (m.S_ALL,) + tuple(enum[n] for n in names))   # All | New | Can Equip | Upgradeable
        self.assertEqual([m.VNAMES[s] for s in m.FILTER_SEGMENTS], ["All", "New", "Can Equip", "Upgradeable"])

    def test_sort_cycle_matches_SortOrder_enum(self):
        import re
        mm = re.search(r"enum SortOrder[^{]*\{([^}]*)\}", self.src())
        pairs = [(a.split("=")[0].strip(), int(a.split("=")[1])) for a in mm.group(1).split(",") if "=" in a]
        self.assertEqual([n for n, _ in sorted(pairs, key=lambda t: t[1])], ["SO_Category", "SO_Base", "SO_Name", "SO_Vanilla"])
        self.assertEqual([v for _, v in sorted(pairs, key=lambda t: t[1])], list(m.SORT_CYCLE))
        self.assertEqual([m.SORT_LABELS[o] for o in m.SORT_CYCLE], ["Category", "Base item", "Name", "Vanilla"])
        order, o = [], m.SO_CATEGORY
        for _ in range(4):
            o = m.next_sort(o)
            order.append(o)
        self.assertEqual(order, [m.SO_BASE, m.SO_NAME, m.SO_VANILLA, m.SO_CATEGORY])

    def test_usable_renamed_in_variants_only(self):
        self.assertEqual(m.VCAPTION[m.S_USABLE], "Can Equip")
        self.assertEqual(m.VNAMES[m.S_USABLE], "Can Equip")
        self.assertEqual(m.caption_of(m.S_USABLE), "Filter: Usable")           # the shipped (current) UX is untouched
        self.assertEqual(m.VCAPTION[m.S_ALL], "Filter")

    def test_filter_counts(self):
        self.assertEqual(m.filter_counts(), {m.S_ALL: 22, m.S_NEW: 4, m.S_USABLE: 13, m.S_UPGRADE: 3})

    def test_dividers_only_in_v3_category(self):
        rows = m.arrange(m.S_ALL, m.SO_CATEGORY)
        for v in ("current", "v1", "v2"):
            self.assertTrue(all(k[0] == "row" for k in m.build_elements(rows, m.SO_CATEGORY, v)))
        self.assertTrue(all(k[0] == "row" for k in m.build_elements(m.arrange(m.S_NEW, m.SO_NAME), m.SO_NAME, "v3")))   # no first-letter dividers in Name sort
        els = m.build_elements(rows, m.SO_CATEGORY, "v3")
        labels = [k[1] for k in els if k[0] == "div"]
        self.assertEqual(labels[0], "New")
        self.assertEqual(labels[1:5], ["Weapons", "Armour", "Gear", "Consumables"])
        self.assertIn("Armour", [k[2][2] for k in els if k[0] == "div" and k[2][1] is False])
        self.assertEqual([k[1].item for k in els if k[0] == "row"], [e.item for e in rows])    # dividers add no rows, order untouched
        self.assertLessEqual(set(labels), set(m.DIV_LABEL.values()) | {m.NEW_DIV})

    def test_scroll_helpers(self):
        self.assertEqual(m.fit_count([10, 10, 5, 10], 0, 26), 3)
        self.assertEqual(m.max_scroll([10, 10, 5, 10], 26), 1)
        self.assertEqual(m.max_scroll([10], 26), 0)


@unittest.skipUnless((Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II").exists(), "game install not present")
class VariantLayout(unittest.TestCase):
    """Fit + overlap checks of every element a redesign adds (needs the install for inventory_p.gui and the font metrics)."""

    @classmethod
    def setUpClass(cls):
        try:
            import PIL  # noqa: F401
            import pykotor  # noqa: F401
        except ImportError:
            raise unittest.SkipTest("PIL / pykotor not importable (use the repo .venv)")
        cls.cpm = m._cpm()
        _, cls.ctls = m.load_gui(cls.cpm)

    def test_short_captions_fit_345px(self):
        label = int(72 * self.cpm.SX)
        self.assertEqual(label, 345)
        for st, cap in m.VCAPTION.items():
            self.assertLessEqual(self.cpm.text_size(m.FILTER_FONT, cap)[0], label, cap)
        self.assertLessEqual(max(self.cpm.text_size(m.FILTER_FONT, c)[0] for c in m.VCAPTION.values()), label - 100)   # keeps a margin

    def test_no_added_rect_overlaps_a_gui_control(self):
        for v in ("v1", "v2", "v3"):
            for tag, st, so in m.VSTATES:
                L = m.variant_layout(self.cpm, v, st, so, self.ctls)
                self.assertEqual(L["overlaps"], [], "%s %s" % (v, tag))
                self.assertGreaterEqual(len(L["added"]), 5, v)

    def test_all_fit_checks_pass(self):
        for v in ("v1", "v2", "v3"):
            for tag, st, so in m.VSTATES:
                L = m.variant_layout(self.cpm, v, st, so, self.ctls)
                bad = [f for f in L["fits"] if not f[4]]
                self.assertEqual(bad, [], "%s %s" % (v, tag))

    def test_overlap_detector_flags_a_clash(self):
        px = m.free_report(self.cpm, self.ctls)["px"]
        self.assertTrue(m.rects_overlap(px["BTN_USEITEM"], (px["BTN_USEITEM"][0] + 5, px["BTN_USEITEM"][1] + 5, 50, 20)))
        self.assertFalse(m.rects_overlap((0, 0, 10, 10), (10, 0, 10, 10)))

    def test_v2_controls_use_both_bands_and_v1_only_band_b(self):
        l1 = m.variant_layout(self.cpm, "v1", m.S_USABLE, m.SO_CATEGORY, self.ctls)
        l2 = m.variant_layout(self.cpm, "v2", m.S_USABLE, m.SO_CATEGORY, self.ctls)
        self.assertFalse(any(a["name"] == "Sort button" for a in l1["added"]))
        names = [a["name"] for a in l2["added"]]
        self.assertIn("Sort button", names)
        self.assertEqual([n for n in names if n.startswith("Filter segment")], ["Filter segment " + m.VNAMES[s] for s in m.FILTER_SEGMENTS])

    def test_variant_render_smoke(self):
        im, info = m.render(m.S_USABLE, m.SO_CATEGORY, variant="v3")
        self.assertEqual(im.size, (3840, 1600))
        self.assertEqual(info["caption"], "Can Equip")
        self.assertFalse(info["overflow"])
        self.assertEqual(info["layout"]["overlaps"], [])


@unittest.skipUnless((Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II").exists(), "game install not present")
class V4Layout(unittest.TestCase):
    """v4a (bottom bar) / v4b (strip above the list): vanilla top strip, own ISF bar, no overlaps, exact list split."""

    @classmethod
    def setUpClass(cls):
        try:
            import PIL  # noqa: F401
            import pykotor  # noqa: F401
        except ImportError:
            raise unittest.SkipTest("PIL / pykotor not importable (use the repo .venv)")
        cls.cpm = m._cpm()
        _, cls.ctls = m.load_gui(cls.cpm)

    def test_v4_registered(self):
        self.assertEqual(m.V4, ("v4a", "v4b"))
        self.assertTrue(set(m.V4) <= set(m.VARIANTS))

    def test_added_rects_overlap_nothing(self):
        for v in m.V4:
            for tag, st, so in m.V4STATES:
                L = m.variant_layout(self.cpm, v, st, so, self.ctls)
                self.assertEqual(L["overlaps"], [], "%s %s" % (v, tag))
                top = [a for a in L["added"] if a["kind"] in ("band", "strip")]
                for i, a in enumerate(top):
                    for b in top[i + 1:]:
                        self.assertFalse(m.rects_overlap(a["rect"], b["rect"]), (v, a["name"], b["name"]))
                    for t, cr in L["px"].items():
                        if t == "LBL_BAR2" and a["kind"] == "strip":
                            continue         # decorative bar rect already overlaps the original LB_ITEMS (drawn only 16 px tall, above it)
                        self.assertFalse(m.rects_overlap(a["rect"], cr), (v, a["name"], t))

    def test_all_fit_and_no_font_below_10x10(self):
        small_h = self.cpm.text_size(m.SMALL, "A")[1]
        for v in m.V4:
            for tag, st, so in m.V4STATES:
                L = m.variant_layout(self.cpm, v, st, so, self.ctls)
                self.assertEqual([f for f in L["fits"] if not f[4]], [], "%s %s" % (v, tag))
                self.assertEqual(L["fonts"], {m.FILTER_FONT, m.SMALL})
                for f in L["fonts"]:
                    self.assertGreaterEqual(self.cpm.text_size(f, "A")[1], small_h)

    def test_v4b_split_fills_original_list_rect(self):
        lb = next(m.unit_px(self.cpm, c["RECT"]) for c in self.ctls if c["TAG"] == "LB_ITEMS")
        g = m.list_geometry(self.cpm, lb, "v4b")
        row_h = int(m.PROTO[3] * self.cpm.SY)
        self.assertEqual(g["strip"][3], row_h)
        self.assertEqual(g["strip"], (lb[0], lb[1], lb[2], row_h))
        self.assertEqual(g["items"], (lb[0], lb[1] + row_h, lb[2], lb[3] - row_h))
        self.assertEqual(g["strip"][1] + g["strip"][3], g["items"][1])
        self.assertEqual(g["items"][1] + g["items"][3], lb[1] + lb[3])
        self.assertEqual(g["strip"][2], g["items"][2])
        self.assertEqual(m.list_geometry(self.cpm, lb, "v4a")["items"], tuple(lb))
        self.assertIsNone(m.list_geometry(self.cpm, lb, "v4a")["strip"])
        self.assertEqual(g["avail"] // row_h, 5)
        self.assertEqual(m.list_geometry(self.cpm, lb, "v3")["avail"] // row_h, 6)
        L = m.variant_layout(self.cpm, "v4b", m.S_USABLE, m.SO_CATEGORY, self.ctls)
        self.assertEqual(L["px"]["LB_ITEMS"], g["items"])
        self.assertEqual(next(a["rect"] for a in L["added"] if a["kind"] == "strip"), g["strip"])

    def test_top_strip_is_vanilla_in_every_v4_state(self):
        self.assertEqual(m.VANILLA_CAPTION, "Filter")
        for v in m.V4:
            for tag, st, so in m.V4STATES:
                im, info = m.render(st, so, variant=v)
                self.assertEqual(info["caption"], "Filter", "%s %s" % (v, tag))
                self.assertFalse(info["overflow"])
                L = info["layout"]
                self.assertEqual([f for f in L["fits"] if f[0] == "LBL_FILTER caption"][0][1], "Filter")
                self.assertEqual(L["overlaps"], [])

    def test_v4_dividers_and_scroll(self):
        rows = m.arrange(m.S_ALL, m.SO_CATEGORY)
        for v in m.V4:
            self.assertTrue(any(k[0] == "div" for k in m.build_elements(rows, m.SO_CATEGORY, v)))
            self.assertTrue(all(k[0] == "row" for k in m.build_elements(m.arrange(m.S_NEW, m.SO_NAME), m.SO_NAME, v)))
            self.assertGreaterEqual(m.v3_scroll(self.cpm, v), 1)


@unittest.skipUnless((Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II").exists(), "game install not present")
class RenderSmoke(unittest.TestCase):
    def test_render_and_fit(self):
        try:
            import PIL  # noqa: F401
            import pykotor  # noqa: F401
        except ImportError:
            self.skipTest("PIL / pykotor not importable (use the repo .venv)")
        with tempfile.TemporaryDirectory() as td:
            im, info = m.render(m.S_UPGRADE, m.SO_CATEGORY, rows_for_caption=1024)
            self.assertEqual(im.size, (3840, 1600))
            self.assertEqual(info["caption"], "Filter: Upgradeable 3/3 (1024)")
            self.assertTrue(info["overflow"])
            self.assertEqual(info["rows_visible"], 3)
            self.assertGreater(len(set(im.convert("RGB").resize((64, 26)).getdata())), 50)   # not blank
            im2, info2 = m.render(m.S_ALL, m.SO_CATEGORY)
            self.assertFalse(info2["overflow"])
            self.assertEqual(info2["rows_window"], 6)
            self.assertIsNotNone(td)


if __name__ == "__main__":
    unittest.main()
