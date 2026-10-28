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
