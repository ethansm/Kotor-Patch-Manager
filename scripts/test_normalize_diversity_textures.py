#!/usr/bin/env python3
"""Guard: normalize_diversity_textures must not regenerate the rejected pool slots."""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import normalize_diversity_textures as N  # noqa: E402


class RejectedSkipTests(unittest.TestCase):
    def test_skip_set_size_and_exemptions(self):
        skip = N.rejected_skip_set()
        self.assertEqual(len(skip), 32)
        for stem in N.ROW_REFERENCED:
            self.assertNotIn(stem, skip)

    def test_allowed_slots_never_skipped(self):
        skip = N.rejected_skip_set()
        for i in list(range(1, 17)) + [34]:
            self.assertNotIn("n_commm%02d" % i, skip)
        for i in list(range(1, 18)) + [35]:
            self.assertNotIn("n_commf%02d" % i, skip)


if __name__ == "__main__":
    unittest.main()
