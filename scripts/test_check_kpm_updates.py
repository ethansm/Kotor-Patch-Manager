#!/usr/bin/env python3
"""test_check_kpm_updates.py - Unit test suite for check_kpm_updates.py"""

import io
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import check_kpm_updates as cku  # noqa: E402


class TestCheckKpmUpdates(unittest.TestCase):
    def test_state_load_default(self):
        state = cku.load_state("/non/existent/path/state.json")
        self.assertIsNone(state["last_check"])
        self.assertEqual(state["seen_comment_ids"], [])
        self.assertEqual(state["tracked_items"], {})

    def test_state_save_and_load(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, "kpm_tracker", "state.json")
            orig = {
                "last_check": "2026-10-02T12:00:00Z",
                "seen_comment_ids": [101, 102],
                "tracked_items": {"24": {"title": "Test Plan", "type": "issue", "comments": 2}},
                "last_discovery": "2026-10-02T12:00:00Z",
            }
            cku.save_state(path, orig)
            loaded = cku.load_state(path)
            self.assertEqual(loaded, orig)

    def test_quiet_zero_output_when_no_updates(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            state_path = os.path.join(tmpdir, "state.json")
            cku.save_state(state_path, {
                "last_check": "2026-10-02T12:00:00Z",
                "seen_comment_ids": [101],
                "tracked_items": {"24": {"title": "Test Issue", "type": "issue", "comments": 1}},
                "last_discovery": "2026-10-02T12:00:00Z",
            })

            # Mock gh JSON calls to return no comments
            with patch.object(cku, "run_gh_json", return_value=[]), \
                 patch.object(cku, "get_current_user", return_value="ethansm"), \
                 patch("sys.argv", ["check_kpm_updates.py", "--quiet", "--state-file", state_path]), \
                 patch("sys.stdout", new_callable=io.StringIO) as mock_stdout:
                
                ret = cku.main()
                self.assertEqual(ret, 0)
                # Output must be completely empty for 0 token overhead
                self.assertEqual(mock_stdout.getvalue(), "")

    def test_ignores_own_comments(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            state_path = os.path.join(tmpdir, "state.json")
            cku.save_state(state_path, {
                "last_check": "2026-10-02T12:00:00Z",
                "seen_comment_ids": [],
                "tracked_items": {"24": {"title": "Test Issue", "type": "issue", "comments": 1}},
                "last_discovery": "2026-10-02T12:00:00Z",
            })

            own_comment = [{
                "id": 999,
                "user": {"login": "ethansm"},
                "issue_url": "https://api.github.com/repos/LaneDibello/Kotor-Patch-Manager/issues/24",
                "body": "My own update",
                "created_at": "2026-10-03T10:00:00Z",
            }]

            with patch.object(cku, "run_gh_json", side_effect=[own_comment, []]), \
                 patch.object(cku, "get_current_user", return_value="ethansm"), \
                 patch("sys.argv", ["check_kpm_updates.py", "--quiet", "--state-file", state_path]), \
                 patch("sys.stdout", new_callable=io.StringIO) as mock_stdout:
                
                ret = cku.main()
                self.assertEqual(ret, 0)
                self.assertEqual(mock_stdout.getvalue(), "")

                state = cku.load_state(state_path)
                self.assertIn(999, state["seen_comment_ids"])

    def test_alerts_on_upstream_reply(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            state_path = os.path.join(tmpdir, "state.json")
            cku.save_state(state_path, {
                "last_check": "2026-10-02T12:00:00Z",
                "seen_comment_ids": [],
                "tracked_items": {"24": {"title": "Detour Hook PR", "type": "pr", "comments": 1}},
                "last_discovery": "2026-10-02T12:00:00Z",
            })

            upstream_comment = [{
                "id": 1001,
                "user": {"login": "LaneDibello"},
                "issue_url": "https://api.github.com/repos/LaneDibello/Kotor-Patch-Manager/issues/24",
                "body": "Looks great, approved!",
                "created_at": "2026-10-03T11:00:00Z",
                "html_url": "https://github.com/LaneDibello/Kotor-Patch-Manager/issues/24#issuecomment-1001",
            }]

            with patch.object(cku, "run_gh_json", side_effect=[upstream_comment, []]), \
                 patch.object(cku, "get_current_user", return_value="ethansm"), \
                 patch.object(cku, "send_desktop_notification") as mock_notify, \
                 patch("sys.argv", ["check_kpm_updates.py", "--state-file", state_path]), \
                 patch("sys.stdout", new_callable=io.StringIO) as mock_stdout:
                
                ret = cku.main()
                self.assertEqual(ret, 0)
                out = mock_stdout.getvalue()
                self.assertIn("#24", out)
                self.assertIn("LaneDibello", out)
                self.assertIn("Looks great, approved!", out)
                mock_notify.assert_called_once()


if __name__ == "__main__":
    unittest.main()
