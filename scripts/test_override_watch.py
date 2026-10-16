import io
import os
import sys
import tempfile
import threading
import time
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import override_watch as W  # noqa: E402


class WatchTest(unittest.TestCase):
    def test_open_logged_and_filtered(self):
        with tempfile.TemporaryDirectory() as td:
            d = Path(td) / "override"
            d.mkdir()
            (d / "kvjv0001.wav").write_bytes(b"x")
            (d / "other.wav").write_bytes(b"x")

            def touch():
                time.sleep(0.3)
                (d / "kvjv0001.wav").read_bytes()
                (d / "kvjv0001.wav").read_bytes()
                (d / "other.wav").read_bytes()
                (d / "KVJNEW.lip").write_bytes(b"y")

            t = threading.Thread(target=touch)
            t.start()
            out = io.StringIO()
            counts = W.watch([d], "kvj", seconds=1.2, out=out)
            t.join()
            self.assertEqual(counts["override/kvjv0001.wav"], 2)
            self.assertNotIn("override/other.wav", counts)
            self.assertIn("CREATE override/KVJNEW.lip", out.getvalue())

    def test_parse_events(self):
        raw = W.EVENT.pack(1, W.IN_OPEN, 0, 8) + b"abc.wav\0"
        self.assertEqual(list(W.parse_events(raw)), [(1, W.IN_OPEN, "abc.wav")])


if __name__ == "__main__":
    unittest.main()
