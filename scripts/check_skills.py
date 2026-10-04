#!/usr/bin/env python3
"""Lint .claude/skills/*/SKILL.md. Python 3, stdlib only. Read-only: the only commands ever executed are
`python3 <script> [sub] --help` (10 s timeout, cwd = repo root, bytecode writing disabled) and `bash -n <file.sh>`.
Files whose name contains "credential" or "secret" are never opened (existence checks only).

Usage:
    check_skills.py [--skills-dir DIR] [--user-dir DIR] [--repo-root DIR] [--json]

  --skills-dir  skills to lint (default <repo-root>/.claude/skills).   --user-dir  user copies (default ~/.claude/skills).
  --repo-root   root that repo-relative paths resolve against (default: parent of scripts/).   --json  emit one JSON object.

Checks per <dir>/SKILL.md:
  1. frontmatter parses; name == dir name; description non-empty and <= 1024 chars.
  2. repo-relative paths (prefixes scripts/ patch_manager_mods/ research/ tests/ .claude/) written in backticks or fenced
     code blocks must exist. Paths containing < * $ { are ignored; trailing : , . ) and :NNN are stripped.
  3. `python3 scripts/X.py [sub] [--flags]` commands (inline or fenced): script exists, `--help` exits 0 (missing
     third-party module -> WARN), sub appears in the argparse {a,b,c} list, each --flag appears in that help.
     A script whose source has no help parser (argparse/optparse/click/typer/docopt/--help) is not executed (WARN).
  4. referenced .sh files pass `bash -n`.
  5. same-name skill in --user-dir with different SKILL.md -> WARN DIVERGED.

Output: `FAIL|WARN <skill>:<line> <message>` sorted, final line `VERDICT: PASS|FAIL skills=N fails=F warns=W`.
Exit 1 on any FAIL.
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

PREFIXES = ("scripts/", "patch_manager_mods/", "research/", "tests/", ".claude/")
MAX_DESC = 1024
HELP_TIMEOUT = 10
RE_PATH = re.compile(r"(?<![\w/.$~-])((?:scripts|patch_manager_mods|research|tests|\.claude)/[^\s`'\"()|;,]*)")
RE_PYCMD = re.compile(r"(?<![\w/.-])python3\s+(?:\./)?(scripts/[^\s`'\"|;&()]+?\.py)(?![\w])")
RE_KEY = re.compile(r"^([A-Za-z_][\w-]*):\s*(.*)$")
RE_FLAG = re.compile(r"^--[A-Za-z][\w-]*$")
RE_SUBCAND = re.compile(r"^\w[\w.-]*$")
RE_HELP_MARK = re.compile(r"argparse|optparse|\bclick\b|\btyper\b|docopt|['\"]--help['\"]|['\"]-h['\"]")
IGNORE_CHARS = "<*${"


def unsafe_name(path):
    n = os.path.basename(str(path)).lower()
    return "credential" in n or "secret" in n


# ---------------------------------------------------------------- frontmatter
def parse_frontmatter(lines):
    """Return (dict, body_start_index, error). lines = SKILL.md lines without newlines."""
    if not lines or lines[0].strip() != "---":
        return {}, 0, "frontmatter missing (file must start with ---)"
    end = None
    for i in range(1, len(lines)):
        if lines[i].strip() == "---":
            end = i
            break
    if end is None:
        return {}, 0, "frontmatter not terminated (no closing ---)"
    fm = {}
    i = 1
    while i < end:
        m = RE_KEY.match(lines[i])
        if not m:
            i += 1
            continue
        key, val = m.group(1), m.group(2).strip()
        j = i + 1
        if re.match(r"^[>|][+-]?\d*$", val):
            literal = val.startswith("|")
            block = []
            while j < end and (lines[j].startswith((" ", "\t")) or not lines[j].strip()):
                block.append(lines[j].strip())
                j += 1
            while block and not block[-1]:
                block.pop()
            val = ("\n" if literal else " ").join(block)
        else:
            cont = []
            while j < end and lines[j].startswith((" ", "\t")) and lines[j].strip():
                cont.append(lines[j].strip())
                j += 1
            if cont:
                val = " ".join([val] + cont)
            if len(val) >= 2 and val[0] == val[-1] and val[0] in "'\"":
                q = val[0]
                val = val[1:-1]
                val = val.replace("''", "'") if q == "'" else val.replace('\\"', '"').replace("\\\\", "\\")
        fm[key] = (val, i + 1)
        i = j
    return fm, end + 1, None


# ---------------------------------------------------------------- body scanning
def code_units(lines, start):
    """Yield (lineno, text) for every backtick span (non-fence lines) and every logical fenced line."""
    units = []
    in_fence = False
    pending = None
    for idx in range(start, len(lines)):
        ln = idx + 1
        line = lines[idx]
        st = line.lstrip()
        if st.startswith("```") or st.startswith("~~~"):
            in_fence = not in_fence
            if pending:
                units.append(pending)
                pending = None
            continue
        if in_fence:
            if pending:
                pending = (pending[0], pending[1] + " " + line.strip())
            else:
                pending = (ln, line)
            if pending[1].rstrip().endswith("\\"):
                pending = (pending[0], pending[1].rstrip()[:-1])
                continue
            units.append(pending)
            pending = None
        else:
            for m in re.finditer(r"`([^`\n]+)`", line):
                units.append((ln, m.group(1)))
    if pending:
        units.append(pending)
    return units


def clean_path(p):
    p = p.rstrip(":,.)]'\";")
    p = re.sub(r":\d+(?::\d+)?$", "", p)
    return p.rstrip(":,.)]'\";")


def extract_paths(text):
    out = []
    for m in RE_PATH.finditer(text):
        raw = m.group(1)
        if any(c in raw for c in IGNORE_CHARS):
            continue
        p = clean_path(raw)
        if p and p.startswith(PREFIXES) and not any(c in p for c in IGNORE_CHARS):
            out.append(p)
    return out


def tokenize_segment(s):
    """Whitespace tokens of s up to the first unquoted shell operator (| || && ; & ) # > <<)."""
    toks, buf = [], []
    quote = None
    depth = 0
    i, n = 0, len(s)
    have = False  # token started (allows empty quoted)

    def flush():
        nonlocal buf, have
        if have:
            toks.append("".join(buf))
        buf, have = [], False

    while i < n:
        c = s[i]
        if quote:
            if c == quote:
                quote = None
            elif c == "\\" and quote == '"' and i + 1 < n:
                i += 1
                buf.append(s[i])
            else:
                buf.append(c)
            i += 1
            continue
        if c in "'\"":
            quote = c
            have = True
            i += 1
            continue
        if c == "\\" and i + 1 < n:
            buf.append(s[i + 1])
            have = True
            i += 2
            continue
        if c.isspace():
            flush()
            i += 1
            continue
        if c in "[{":
            depth += 1
        elif c in "]}":
            depth = max(0, depth - 1)
        if depth == 0:
            if c in "|;":
                break
            if c == "&" and not (buf and buf[-1] == ">"):
                break
            if c == ")":
                break
        if not have and c == "#":
            break
        if not have and (c == ">" or (c.isdigit() and s[i:i + 2].lstrip("0123456789").startswith(">"))):
            break
        if not have and s.startswith("<<", i):
            break
        if not have and c == "<" and (i + 1 >= n or s[i + 1].isspace()):
            break
        buf.append(c)
        have = True
        i += 1
    flush()
    return toks


def flag_names(tok):
    """--flag names found in one token (handles [--a|--b], --x=val, brackets)."""
    t = tok.strip("[]()")
    names = []
    for alt in t.split("|"):
        alt = alt.strip("[](){},.…")
        if not alt.startswith("--"):
            continue
        name = alt.split("=")[0]
        if RE_FLAG.match(name) and name != "--help":
            names.append(name)
    return names


# ---------------------------------------------------------------- help runner
class HelpRunner:
    def __init__(self, repo_root):
        self.root = str(repo_root)
        self.cache = {}

    def run(self, script, sub):
        """Return (status, text). status: ok | fail | dep | skip | timeout."""
        key = (script, sub)
        if key in self.cache:
            return self.cache[key]
        res = self._run(script, sub)
        self.cache[key] = res
        return res

    def _run(self, script, sub):
        full = os.path.join(self.root, script)
        if unsafe_name(full):
            return ("skip", "name is credential-like; not executed")
        try:
            with open(full, "rb") as f:
                src = f.read().decode("utf-8", "replace")
        except OSError as e:
            return ("fail", "cannot read: %s" % e)
        if not RE_HELP_MARK.search(src):
            return ("skip", "no help parser in %s; not executed" % script)
        cmd = ["python3", script] + ([sub] if sub else []) + ["--help"]
        env = dict(os.environ, PYTHONDONTWRITEBYTECODE="1")
        try:
            p = subprocess.run(cmd, cwd=self.root, capture_output=True, text=True, timeout=HELP_TIMEOUT,
                               env=env, stdin=subprocess.DEVNULL)
        except subprocess.TimeoutExpired:
            return ("timeout", "")
        except OSError as e:
            return ("fail", str(e))
        if p.returncode != 0:
            m = re.search(r"No module named '?([\w.]+)'?", p.stderr)
            if m:
                return ("dep", m.group(1))
            if "ImportError" in p.stderr:
                m = re.search(r"ImportError: (.*)", p.stderr)
                return ("dep", (m.group(1) if m else "?").strip()[:80])
            return ("fail", "exit %d: %s" % (p.returncode, (p.stderr.strip().splitlines() or [""])[-1][:120]))
        return ("ok", p.stdout + "\n" + p.stderr)


def help_subcommands(text):
    """Subcommand names from an argparse top-level help ({a,b,c} positional group); [] if none."""
    m = re.search(r"^usage:(.*?)(?:\n\s*\n|\Z)", text, re.S | re.M | re.I)
    if m:
        u = m.group(1)
        prev = None
        while prev != u:  # drop [optional ...] groups (incl. nested) so --mode {a,b} choices don't count
            prev = u
            u = re.sub(r"\[[^\[\]]*\]", " ", u)
        g = re.search(r"\{([^{}\s]+)\}", u)
        if g:
            return g.group(1).split(",")
    g = re.search(r"^\s+\{([^{}\s]+)\}", text, re.M)
    return g.group(1).split(",") if g else []


def docstring_subcommands(text, script):
    """Fallback for hand-rolled parsers: words in `<script name> <word>` usage lines (e.g. `kotor2_logs.py check NAME`)."""
    name = re.escape(os.path.basename(script))
    subs = []
    for m in re.finditer(r"^\s*(?:usage:\s*)?(?:python3\s+)?\S*" + name + r"\s+([a-z][\w-]*)", text, re.M | re.I):
        if m.group(1) not in subs:
            subs.append(m.group(1))
    return subs


def flag_in_help(flag, text):
    return re.search(r"(?<![\w-])" + re.escape(flag) + r"(?![\w-])", text) is not None


def flag_takes_value(flag, text):
    return re.search(re.escape(flag) + r"[ =](?=[A-Z{<\[])", text) is not None


# ---------------------------------------------------------------- per-skill checks
class Linter:
    def __init__(self, repo_root, user_dir):
        self.root = Path(repo_root)
        self.user_dir = Path(user_dir) if user_dir else None
        self.help = HelpRunner(self.root)
        self.sh_cache = {}
        self.problems = []  # (skill, line, level, message)

    def add(self, skill, line, level, msg):
        self.problems.append((skill, line, level, msg))

    def lint_skill(self, sdir):
        name = sdir.name
        skill_md = sdir / "SKILL.md"
        try:
            text = skill_md.read_text(encoding="utf-8", errors="replace")
        except OSError as e:
            self.add(name, 0, "FAIL", "cannot read SKILL.md: %s" % e)
            return
        lines = text.splitlines()
        fm, body, err = parse_frontmatter(lines)
        if err:
            self.add(name, 1, "FAIL", err)
        else:
            if "name" not in fm or not fm["name"][0]:
                self.add(name, 1, "FAIL", "frontmatter has no name")
            elif fm["name"][0] != name:
                self.add(name, fm["name"][1], "FAIL", "BAD NAME name=%r != dir %r" % (fm["name"][0], name))
            if "description" not in fm or not fm["description"][0].strip():
                self.add(name, fm["description"][1] if "description" in fm else 1, "FAIL", "DESCRIPTION empty or missing")
            elif len(fm["description"][0]) > MAX_DESC:
                self.add(name, fm["description"][1], "FAIL",
                         "DESCRIPTION too long (%d > %d chars)" % (len(fm["description"][0]), MAX_DESC))
        units = code_units(lines, body)
        cmd_paths = set()
        for ln, t in units:
            for m in RE_PYCMD.finditer(t):
                script = m.group(1)
                if any(c in script for c in IGNORE_CHARS):
                    continue
                cmd_paths.add((ln, script))
                self.check_command(name, ln, script, tokenize_segment(t[m.end():]))
        seen = set()
        for ln, t in units:
            for p in extract_paths(t):
                if (ln, p) in seen or (ln, p) in cmd_paths:
                    continue
                seen.add((ln, p))
                full = self.root / p
                if not os.path.exists(full):
                    self.add(name, ln, "FAIL", "MISSING PATH %s" % p)
                elif p.endswith(".sh") and full.is_file():
                    self.check_sh(name, ln, p)
        self.check_diverged(name, skill_md)

    def check_command(self, skill, ln, script, toks):
        if not os.path.exists(self.root / script):
            self.add(skill, ln, "FAIL", "SCRIPT MISSING %s" % script)
            return
        st, top = self.help.run(script, "")
        if st != "ok":
            self.help_problem(skill, ln, script, "", st, top)
            return
        subs = help_subcommands(top) or docstring_subcommands(top, script)
        sub = ""
        i = 0
        while i < len(toks):
            t = toks[i]
            if t.startswith("-"):
                names = flag_names(t)
                if len(names) == 1 and "=" not in t and flag_takes_value(names[0], top):
                    i += 1
            else:
                if subs and RE_SUBCAND.match(t) and not t.startswith("["):
                    sub = t
                break
            i += 1
        text = top
        if sub:
            if sub not in subs:
                self.add(skill, ln, "FAIL", "UNKNOWN SUBCOMMAND %s for %s (have: %s)" % (sub, script, ",".join(subs)))
                return
            st2, subtxt = self.help.run(script, sub)
            if st2 != "ok":
                self.help_problem(skill, ln, script, sub, st2, subtxt)
                return
            text = top + "\n" + subtxt
        reported = set()
        for t in toks:
            for f in flag_names(t):
                if f in reported:
                    continue
                if not flag_in_help(f, text):
                    reported.add(f)
                    self.add(skill, ln, "FAIL", "UNKNOWN FLAG %s for %s%s" % (f, script, " " + sub if sub else ""))

    def help_problem(self, skill, ln, script, sub, st, info):
        label = script + (" " + sub if sub else "")
        if st == "dep":
            self.add(skill, ln, "WARN", "skipped: missing dep %s (%s)" % (info, label))
        elif st == "skip":
            self.add(skill, ln, "WARN", "skipped: %s" % info)
        elif st == "timeout":
            self.add(skill, ln, "FAIL", "HELP TIMEOUT %s --help (> %ds)" % (label, HELP_TIMEOUT))
        else:
            self.add(skill, ln, "FAIL", "HELP FAILED %s --help (%s)" % (label, info))

    def check_sh(self, skill, ln, rel):
        if unsafe_name(rel):
            return
        if rel not in self.sh_cache:
            try:
                p = subprocess.run(["bash", "-n", str(self.root / rel)], cwd=str(self.root), capture_output=True,
                                   text=True, timeout=HELP_TIMEOUT, stdin=subprocess.DEVNULL)
                err = None if p.returncode == 0 else (p.stderr.strip().splitlines() or ["exit %d" % p.returncode])[0]
            except (OSError, subprocess.TimeoutExpired) as e:
                err = str(e)
            self.sh_cache[rel] = err
        err = self.sh_cache[rel]
        if err:
            self.add(skill, ln, "FAIL", "BASH SYNTAX %s: %s" % (rel, err[:160]))

    def check_diverged(self, name, skill_md):
        if not self.user_dir:
            return
        other = self.user_dir / name / "SKILL.md"
        if not other.is_file():
            return
        try:
            if os.path.samefile(other, skill_md):
                return
            if other.read_bytes() != skill_md.read_bytes():
                self.add(name, 0, "WARN", "DIVERGED %s (user copy differs)" % name)
        except OSError:
            pass


def run_lint(skills_dir, user_dir, repo_root):
    lin = Linter(repo_root, user_dir)
    n = 0
    sd = Path(skills_dir)
    if not sd.is_dir():
        lin.add(sd.name or "skills", 0, "FAIL", "skills dir not found: %s" % sd)
    else:
        for d in sorted(p for p in sd.iterdir() if p.is_dir()):
            if (d / "SKILL.md").is_file():
                n += 1
                lin.lint_skill(d)
            else:
                lin.add(d.name, 0, "WARN", "no SKILL.md in directory")
    probs = sorted(set(lin.problems), key=lambda p: (p[0], p[1], p[2] != "FAIL", p[3]))
    return n, probs


def main(argv=None, out=sys.stdout):
    ap = argparse.ArgumentParser(prog="check_skills.py", description="Lint Claude skills (frontmatter, paths, commands).")
    ap.add_argument("--skills-dir", default=None)
    ap.add_argument("--user-dir", default="~/.claude/skills")
    ap.add_argument("--repo-root", default=None)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args(argv)
    root = Path(args.repo_root).expanduser() if args.repo_root else Path(__file__).resolve().parent.parent
    skills_dir = Path(args.skills_dir).expanduser() if args.skills_dir else root / ".claude" / "skills"
    user_dir = Path(args.user_dir).expanduser() if args.user_dir else None
    n, probs = run_lint(skills_dir, user_dir, root)
    fails = sum(1 for p in probs if p[2] == "FAIL")
    warns = len(probs) - fails
    verdict = "VERDICT: %s skills=%d fails=%d warns=%d" % ("FAIL" if fails else "PASS", n, fails, warns)
    if args.json:
        out.write(json.dumps({
            "problems": [{"level": p[2], "skill": p[0], "line": p[1], "message": p[3]} for p in probs],
            "verdict": "FAIL" if fails else "PASS", "skills": n, "fails": fails, "warns": warns,
        }, sort_keys=True) + "\n")
    else:
        for s, ln, lv, msg in probs:
            out.write("%s %s:%d %s\n" % (lv, s, ln, msg))
        out.write(verdict + "\n")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
