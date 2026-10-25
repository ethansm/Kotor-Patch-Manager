#!/usr/bin/env python3
"""Tests for _load_limit: slots cap concurrency across threads, exclusive() serialises, memory wait times out."""
import os
import sys
import tempfile
import threading
import time
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _load_limit as ll  # noqa: E402


class LoadLimitTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        p = mock.patch.object(ll, "LOCKS", Path(self.tmp.name) / "locks")
        p.start()
        self.addCleanup(p.stop)
        self.addCleanup(self.tmp.cleanup)

    def test_slots_cap_parallelism(self):
        peak, cur, lock = [0], [0], threading.Lock()

        def work():
            with ll.heavy_slot():
                with lock:
                    cur[0] += 1
                    peak[0] = max(peak[0], cur[0])
                time.sleep(0.05)
                with lock:
                    cur[0] -= 1
        with mock.patch.dict(os.environ, {"KOTOR_HEAVY_SLOTS": "2", "KOTOR_MEM_RESERVE_MB": "0"}):
            ts = [threading.Thread(target=work) for _ in range(6)]
            for t in ts:
                t.start()
            for t in ts:
                t.join()
        self.assertEqual(peak[0], 2)

    def test_slot_released_on_exception(self):
        with mock.patch.dict(os.environ, {"KOTOR_HEAVY_SLOTS": "1", "KOTOR_MEM_RESERVE_MB": "0"}):
            with self.assertRaises(ValueError):
                with ll.heavy_slot():
                    raise ValueError
            with ll.heavy_slot():   # would spin forever if the slot leaked
                pass

    def test_nested_slot_in_child_is_noop(self):
        with mock.patch.dict(os.environ, {"KOTOR_HEAVY_SLOTS": "1", "KOTOR_MEM_RESERVE_MB": "0"}):
            with ll.heavy_slot():
                with mock.patch.dict(os.environ, ll.SLOT_HELD_ENV):
                    with ll.heavy_slot():   # would spin forever without the held-slot env
                        pass

    def test_exclusive_serialises(self):
        order = []

        def work(tag):
            with ll.exclusive("x"):
                order.append(tag + "<")
                time.sleep(0.03)
                order.append(tag + ">")
        ts = [threading.Thread(target=work, args=(str(i),)) for i in range(3)]
        for t in ts:
            t.start()
        for t in ts:
            t.join()
        for i in range(0, 6, 2):
            self.assertEqual(order[i][0], order[i + 1][0])

    def test_memory_wait_gives_up(self):
        with mock.patch.object(ll, "mem_available_mb", return_value=10), \
             mock.patch.dict(os.environ, {"KOTOR_MEM_RESERVE_MB": "100", "KOTOR_MEM_WAIT_S": "0"}):
            t0 = time.monotonic()
            ll._wait_for_memory()
            self.assertLess(time.monotonic() - t0, 2.0)

    def test_slot_count_env(self):
        with mock.patch.dict(os.environ, {"KOTOR_HEAVY_SLOTS": "3"}):
            self.assertEqual(ll.slot_count(), 3)
        with mock.patch.dict(os.environ, {"KOTOR_HEAVY_SLOTS": ""}):
            self.assertGreaterEqual(ll.slot_count(), 2)


if __name__ == "__main__":
    unittest.main()
