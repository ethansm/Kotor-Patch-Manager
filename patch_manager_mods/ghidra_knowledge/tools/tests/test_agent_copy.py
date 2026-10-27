"""Pins the rea-recon agent (D7).  Run from tools/: python3 -m unittest tests.test_agent_copy"""
import re
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
TRACKED = HERE.parent.parent / "agents" / "rea-recon.md"
LIVE = Path.home() / ".claude" / "agents" / "rea-recon.md"
WRITE_TOOL = re.compile(r"annotate|set_|goto_address|rename|create_|delete_|patch", re.I)
ENV_KEYS = ("GHIDRA_INSTALL_DIR", "JAVA_HOME", "REA_GHIDRA_STARTUP_TIMEOUT_MS")
BUDGET = "at most 3 REA calls per address plus 3 overall"


def split(text):
    m = re.match(r"---\n(.*?)\n---\n(.*)", text, re.S)
    assert m, "no frontmatter"
    return m.group(1), m.group(2)


class AgentCopyTests(unittest.TestCase):
    def setUp(self):
        self.text = TRACKED.read_text(encoding="utf-8")
        self.front, self.body = split(self.text)

    def test_live_copy_equal(self):
        if not LIVE.exists():
            self.skipTest(f"{LIVE} absent")
        self.assertEqual(LIVE.read_text(encoding="utf-8"), self.text,
                         "live agent differs: cp the tracked copy to ~/.claude/agents/")

    def test_tools_read_only(self):
        m = re.search(r"^tools:\s*(.+)$", self.front, re.M)
        self.assertIsNotNone(m)
        tools = [t.strip() for t in m.group(1).split(",")]
        self.assertIn("mcp__rea__open_binary", tools)
        for t in tools:
            if t.startswith("mcp__rea__"):
                self.assertIsNone(WRITE_TOOL.search(t[len("mcp__rea__"):]), t)

    def test_env_keys(self):
        for key in ENV_KEYS:
            self.assertRegex(self.front, rf"(?m)^\s+{key}:", key)

    def test_body_contract(self):
        self.assertIn("LEDGER:", self.body)
        self.assertIn(BUDGET, self.body)
        self.assertIn("static-facts file", self.body)


if __name__ == "__main__":
    unittest.main()
