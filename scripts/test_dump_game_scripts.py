#!/usr/bin/env python3
"""scripts/test_dump_game_scripts.py - tests for the helpers in scripts/dump_game_scripts.py.

No game install needed: fake resources and fake decompilers only. Output goes to a tmp dir.
"""
from __future__ import annotations

import sys
import tempfile
import time
import unittest
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

import dump_game_scripts as D  # noqa: E402


class FakeRes:
    def __init__(self, data):
        self.data = data


def fake_lookup(table):
    def lookup(name):
        if name == 'boom':
            raise RuntimeError('bad lookup')
        return table.get(name)
    return lookup


class ResolveNamesTests(unittest.TestCase):
    def test_default_list(self):
        names = D.resolve_names(None, False)
        self.assertEqual(names, list(D.DEFAULT_NAMES))
        self.assertEqual(len(names), 13)
        self.assertIn('k_def_spawn01', names)

    def test_names_normalised_and_deduped(self):
        got = D.resolve_names(' K_Def_Spawn01 , k_inc_generic,k_def_spawn01,', False)
        self.assertEqual(got, ['k_def_spawn01', 'k_inc_generic'])

    def test_unsafe_or_empty_names_rejected(self):
        for bad in ('../evil', 'a/b', 'x y'):
            with self.assertRaises(ValueError):
                D.resolve_names(bad, False)
        with self.assertRaises(ValueError):
            D.resolve_names(' , ', False)

    def test_all_uses_sorted_chitin_names_and_skips_unsafe(self):
        got = D.resolve_names(None, True, ['K_B', 'k_a', 'k_a', 'bad/name', 'k_c'])
        self.assertEqual(got, ['k_a', 'k_b', 'k_c'])

    def test_names_and_all_are_exclusive(self):
        with self.assertRaises(ValueError):
            D.resolve_names('k_a', True, [])


class FormatTests(unittest.TestCase):
    def test_clean_field_strips_tabs_and_newlines(self):
        self.assertEqual(D.clean_field('a\tb\r\nc  d'), 'a b c d')

    def test_index_rows_header_and_shape(self):
        rows = [
            D.DumpRow('k_a', 12, 'ok', '', 'void main() {}'),
            D.DumpRow('k_b', 0, 'fail', 'no NCS\tresource\nhere'),
        ]
        lines = D.format_index_rows(rows)
        self.assertEqual(lines[0], 'name\tbytes\tstatus\terror')
        self.assertEqual(lines[1], 'k_a\t12\tok\t')
        self.assertEqual(lines[2], 'k_b\t0\tfail\tno NCS resource here')
        for line in lines:
            self.assertEqual(line.count('\t'), 3)


class DumpOneTests(unittest.TestCase):
    def test_ok(self):
        row = D.dump_one('k_a', fake_lookup({'k_a': FakeRes(b'\x42\x00\x01')}), lambda d: 'void main() {}')
        self.assertEqual((row.status, row.nbytes, row.error, row.source), ('ok', 3, '', 'void main() {}'))

    def test_missing_resource(self):
        def never(_d):
            raise AssertionError('must not decompile a missing resource')
        row = D.dump_one('k_a', fake_lookup({}), never)
        self.assertEqual((row.status, row.nbytes, row.error), ('fail', 0, 'no NSS/NCS resource in game dir'))

    def test_lookup_exception_is_a_fail_row(self):
        row = D.dump_one('boom', fake_lookup({}), lambda d: '')
        self.assertEqual((row.status, row.error), ('fail', 'lookup RuntimeError: bad lookup'))

    def test_decompile_exception_is_one_line(self):
        def bad(_d):
            raise ValueError('line one\nline two')
        row = D.dump_one('k_a', fake_lookup({'k_a': FakeRes(b'abc')}), bad)
        self.assertEqual((row.status, row.nbytes, row.error), ('fail', 3, 'ValueError: line one line two'))

    def test_timeout_not_swallowed_by_inner_except(self):
        def slow(_d):
            try:
                time.sleep(3)
            except Exception:
                return 'swallowed'
            return 'finished'
        t0 = time.monotonic()
        row = D.dump_one('k_a', fake_lookup({'k_a': FakeRes(b'abc')}), slow, timeout_s=0.2)
        self.assertEqual(row.status, 'fail')
        self.assertIn('timeout', row.error)
        self.assertLess(time.monotonic() - t0, 2.5)


class WriteOutputsTests(unittest.TestCase):
    def test_writes_ok_nss_and_index(self):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / 'nss'
            rows = [
                D.DumpRow('k_a', 3, 'ok', '', 'void main() {\n}'),
                D.DumpRow('k_b', 0, 'fail', 'no NCS resource in game dir'),
            ]
            ok, fail = D.write_outputs(out, rows)
            self.assertEqual((ok, fail), (1, 1))
            self.assertEqual((out / 'k_a.nss').read_text(encoding='utf-8'), 'void main() {\n}\n')
            self.assertFalse((out / 'k_b.nss').exists())
            idx = (out / '_index.tsv').read_text(encoding='utf-8').splitlines()
            self.assertEqual(idx, [
                'name\tbytes\tstatus\terror',
                'k_a\t3\tok\t',
                'k_b\t0\tfail\tno NCS resource in game dir',
            ])
            self.assertEqual(sum(1 for line in idx[1:] if line.split('\t')[2] == 'ok'), 1)


class PathAndMainTests(unittest.TestCase):
    def test_is_inside(self):
        with tempfile.TemporaryDirectory() as td:
            game = Path(td) / 'game'
            game.mkdir()
            self.assertTrue(D.is_inside(game / 'x', game))
            self.assertTrue(D.is_inside(game, game))
            self.assertFalse(D.is_inside(Path(td) / 'nss', game))

    def test_missing_game_dir_returns_2_without_writing(self):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / 'nss'
            self.assertEqual(D.main(['--out', str(out), '--game', str(Path(td) / 'nope')]), 2)
            self.assertFalse(out.exists())


if __name__ == '__main__':
    unittest.main()
