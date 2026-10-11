#!/usr/bin/env python3
"""Per-run Claude Code settings + bubblewrap argv for the Fix pipeline (no LLM). Contract: ../INTERFACES.md.

Usage:
  fix_settings.py fixer <tid> --attempt N        -> $FIX_WORK/settings/fixer_<tid>.a<N>.json
  fix_settings.py reviewer <tid> --attempt N     -> $FIX_WORK/settings/reviewer_<tid>.a<N>.json
  fix_settings.py integrator <batch>             -> $FIX_WORK/settings/integrator_<batch>.json
  fix_settings.py bwrap-argv claude|gate <rw_dir>... [--print]
        NUL-separated bwrap argv PREFIX on stdout (without "bwrap" and without the "--" + command);
        --print shows it one arg per line instead.
  fix_settings.py --self-test

Permission model: defaultMode dontAsk (anything not allowed is denied). Deny always wins over allow in Claude
Code, so the deny list holds only secrets / the real repo / config dirs; the allow list is the task copy only:
  fixer       Edit(//abs) + Write(//abs) per allowed file, Read(//<copy>/**)
  reviewer    Read(//<review copy>/**)
  integrator  Edit/Write/Read(//<integ>/**), ONE Bash allow: Bash(<FIX_HOME>/bin/fixgate_<batch>.sh:*)
"""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402

SECRET_PATTERNS = [".env", "*.pem", ".credentials.json", "endpoint.env", "*credential*", "*secret*",
                   "*credential*/**", "*secret*/**", "sunshine/credentials/**", "tests/static/raw_addresses.toml"]
FILE_TOOLS = ["Read", "Edit", "Write"]
TOOL_DENY = ["WebFetch", "WebSearch", "Agent", "Task", "Skill", "NotebookEdit"]
DEFAULT_KPM = "~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"


def abs_rule(p) -> str:
    """Claude Code absolute-path form: // + path-from-filesystem-root."""
    s = str(p)
    return "/" + s if s.startswith("/") else s


def deny_list(integrator: bool = False) -> list[str]:
    deny: list[str] = []
    for tool in FILE_TOOLS:
        for p in SECRET_PATTERNS:
            for form in (f"{tool}({p})", f"{tool}(**/{p})", f"{tool}(//**/{p})"):
                if form not in deny:
                    deny.append(form)
    for tool in FILE_TOOLS:
        deny += [f"{tool}({abs_rule(fc.REPO)}/**)", f"{tool}({abs_rule(fc.REPO.parent)}/**)",
                 f"{tool}(~/.claude/**)", f"{tool}(~/.config/**)", f"{tool}(~/.ssh/**)", f"{tool}(//**/.git/**)"]
    if not integrator:
        deny.append("Bash")
    deny += TOOL_DENY
    return deny


def check_no_overlap(allow: list[str]) -> None:
    pref = [abs_rule(fc.REPO) + "/", abs_rule(fc.REPO.parent) + "/"]
    for r in allow:
        inner = r[r.find("(") + 1:-1] if "(" in r else ""
        if r.startswith("Bash("):
            continue
        if any((inner + "/").startswith(p) or inner == p.rstrip("/") for p in pref):
            sys.exit(f"fix_settings: allow rule {r} sits inside a denied real-repo path")


def write_settings(name: str, allow: list[str], integrator: bool = False) -> Path:
    check_no_overlap(allow)
    out = fc.WORK / "settings" / f"{name}.json"
    fc.write_json(out, {"permissions": {"allow": allow, "deny": deny_list(integrator), "defaultMode": "dontAsk"}})
    return out


def settings_fixer(tid: str, n: int) -> Path:
    t = fc.task_by_id(tid)
    copy = fc.TASKS / f"{tid}.a{n}"
    allow = [f"Read({abs_rule(copy)}/**)"]
    for f in t["allowed_files"]:
        rel = fc.safe_rel(f)
        if rel is None or fc.never_edit(rel):
            sys.exit(f"fix_settings: allowed file {f} is unsafe")
        allow += [f"Edit({abs_rule(copy / rel)})", f"Write({abs_rule(copy / rel)})"]
    return write_settings(f"fixer_{tid}.a{n}", allow)


def settings_reviewer(tid: str, n: int) -> Path:
    fc.task_by_id(tid)
    rc = fc.TASKS / f"{tid}.a{n}.review"
    out = write_settings(f"reviewer_{tid}.a{n}", [f"Read({abs_rule(rc)}/**)"])
    return out


def settings_integrator(batch: str) -> Path:
    if not fc.ID_RE.match(batch):
        sys.exit(f"fix_settings: bad batch id {batch}")
    root = fc.INTEG_HOME / batch
    allow = [f"Read({abs_rule(root)}/**)", f"Edit({abs_rule(root)}/**)", f"Write({abs_rule(root)}/**)",
             f"Bash({fc.FIX_HOME}/bin/fixgate_{batch}.sh:*)"]
    return write_settings(f"integrator_{batch}", allow, integrator=True)


def bwrap_argv(profile: str, rw_dirs: list[str]) -> list[str]:
    if profile not in ("claude", "gate"):
        sys.exit("fix_settings: profile must be claude|gate")
    home = Path(os.environ.get("HOME") or Path.home())
    a = ["--ro-bind", "/", "/", "--dev", "/dev", "--proc", "/proc", "--tmpfs", "/tmp"]
    hides = [home / "orca", home / ".config", home / ".ssh", home / "Games", home / ".local/share/Steam", fc.RHOME]
    for h in hides:
        if h.exists():
            a += ["--tmpfs", str(h)]
    if profile == "gate":
        if (home / ".claude").exists():
            a += ["--tmpfs", str(home / ".claude")]
        kpm = Path(os.path.expanduser(os.environ.get("KPM_DIR") or DEFAULT_KPM))
        if kpm.exists():
            a += ["--ro-bind", str(kpm), str(kpm)]
    else:
        for p in (home / ".claude", home / ".claude.json"):
            if p.exists():
                a += ["--bind", str(p), str(p)]
        # --settings files live in WORK/settings, under a hidden tree (~/orca or RHOME): re-expose them read-only
        sdir = fc.WORK / "settings"
        sdir.mkdir(parents=True, exist_ok=True)
        a += ["--ro-bind", str(sdir), str(sdir)]
    for d in rw_dirs:
        p = Path(d)
        if not p.exists():
            sys.exit(f"fix_settings: rw dir {d} does not exist")
        a += ["--bind", str(p), str(p)]
    if profile == "gate":
        a += ["--unshare-net", "--unshare-pid", "--new-session"]
    a += ["--die-with-parent"]
    return a


# ------------------------------------------------------------------ self-test
def self_test_inner(T: Path) -> int:
    import fix_workspace as fw  # type: ignore
    bad = []
    n_chk = [0]

    def chk(c, m):
        n_chk[0] += 1
        if not c:
            bad.append(m)

    out = settings_fixer("T0000000001", 1)
    s = fc.rj(out)
    allow, deny = s["permissions"]["allow"], s["permissions"]["deny"]
    copy = fc.TASKS / "T0000000001.a1"
    chk(s["permissions"]["defaultMode"] == "dontAsk", "defaultMode")
    for tool in FILE_TOOLS:
        for p in SECRET_PATTERNS:
            for form in (f"{tool}({p})", f"{tool}(**/{p})", f"{tool}(//**/{p})"):
                chk(form in deny, f"deny form missing: {form}")
    for need in ("Bash", "WebFetch", "WebSearch", "Agent", "Task", "Skill", "NotebookEdit", "Read(~/.ssh/**)",
                 "Edit(~/.claude/**)", "Write(//**/.git/**)", f"Read({abs_rule(fc.REPO)}/**)",
                 f"Read({abs_rule(fc.REPO.parent)}/**)"):
        chk(need in deny, f"deny missing: {need}")
    chk(f"Edit({abs_rule(copy)}/scripts/a.py)" in allow and f"Write({abs_rule(copy)}/scripts/a.py)" in allow, "fixer allow")
    chk(all(str(copy) in r for r in allow), f"allow outside the task copy: {allow}")
    chk(sum(1 for r in allow if r.startswith(("Edit", "Write"))) == 2, "fixer allows more than its allowed files")
    chk(not any(r.startswith("Bash") for r in allow), "fixer has Bash")
    chk(all(r.startswith("Edit(//") or r.startswith("Write(//") or r.startswith("Read(//") for r in allow), "non-// allow")
    r = fc.rj(settings_reviewer("T0000000001", 1))
    chk(r["permissions"]["allow"] == [f"Read({abs_rule(fc.TASKS / 'T0000000001.a1.review')}/**)"], "reviewer allow")
    chk("Bash" in r["permissions"]["deny"], "reviewer bash")
    i = fc.rj(settings_integrator("batch_01"))
    ia = i["permissions"]["allow"]
    bash = [x for x in ia if x.startswith("Bash")]
    chk(bash == [f"Bash({fc.FIX_HOME}/bin/fixgate_batch_01.sh:*)"], f"integrator bash allow: {bash}")
    chk("Bash" not in i["permissions"]["deny"], "integrator denies Bash entirely")
    chk(all(x.startswith("Bash") or str(fc.INTEG_HOME / "batch_01") in x for x in ia), "integrator allow outside integ")
    # a fixer whose allow would sit inside the denied repo must be refused
    try:
        check_no_overlap([f"Edit({abs_rule(fc.REPO)}/x)"])
        bad.append("overlap not refused")
    except SystemExit:
        pass
    # bwrap argv
    rw = T / "rw"
    rw.mkdir()
    g = bwrap_argv("gate", [str(rw)])
    home = os.environ["HOME"]
    for h in ("orca", ".config", ".ssh", "Games"):
        if (Path(home) / h).exists():
            chk(["--tmpfs", f"{home}/{h}"] in pairs(g), f"hide {h}")
    chk("--unshare-net" in g and "--die-with-parent" in g and "--ro-bind" in g[:1], "gate flags")
    chk(["--tmpfs", str(fc.RHOME)] in pairs(g), "RHOME hidden")
    chk(g.index("--bind") > g.index("--tmpfs"), "rw bind before tmpfs hides")
    c = bwrap_argv("claude", [str(rw)])
    chk("--unshare-net" not in c, "claude profile has no net unshare")
    sd = str(fc.WORK / "settings")
    chk(["--ro-bind", sd] in pairs(c) and c.index(sd) > c.index("--tmpfs"), "claude sees settings dir (ro, after hides)")
    if (Path(home) / ".claude").exists():
        chk(str(Path(home) / ".claude") in c and ["--tmpfs", f"{home}/.claude"] not in pairs(c), "claude rw ~/.claude")
    cli = subprocess.run([sys.executable, "-I", str(Path(__file__).resolve()), "bwrap-argv", "gate", str(rw)],
                         capture_output=True, env=os.environ)
    chk(cli.returncode == 0 and cli.stdout.split(b"\0")[:-1] == [x.encode() for x in g], "CLI NUL output")
    if shutil_which("bwrap"):
        p = subprocess.run(["bwrap", *g, "--", "true"], capture_output=True, text=True)
        chk(p.returncode == 0, f"bwrap true failed: {p.stderr[-200:]}")
        probe = ("test -z \"$(ls -A \"$HOME/orca\" 2>/dev/null)\" && test -z \"$(ls -A \"$HOME/.ssh\" 2>/dev/null)\" "
                 f"&& test ! -e \"{fc.FIX_HOME}\" && touch \"{rw}/w\"")
        p = subprocess.run(["bwrap", *g, "--", "sh", "-c", probe], capture_output=True, text=True)
        chk(p.returncode == 0 and (rw / "w").exists(), f"gate sandbox probe failed rc={p.returncode} {p.stderr[-200:]}")
        p = subprocess.run(["bwrap", *g, "--", "sh", "-c", "python3 -c 'import socket; socket.create_connection((\"1.1.1.1\", 53), 2)'"],
                           capture_output=True, text=True)
        chk(p.returncode != 0, "network reachable inside gate sandbox")
    else:
        print("fix_settings self-test: bwrap not installed, sandbox probes skipped")
    print("fix_settings self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad), f"({n_chk[0]} checks)")
    return 1 if bad else 0


def pairs(a):
    return [a[k:k + 2] for k in range(len(a) - 1)]


def shutil_which(x):
    import shutil
    return shutil.which(x)


def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        import fix_workspace as fw  # type: ignore
        r = fw.reexec_selftest(str(Path(__file__).resolve()))
        return r
    if argv[:1] == ["--self-test-inner"]:
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        fw_path = Path(__file__).resolve().parent
        if str(fw_path) not in sys.path:
            sys.path.insert(0, str(fw_path))
        return self_test_inner(Path(argv[1]))
    if len(argv) >= 2 and argv[0] in ("fixer", "reviewer"):
        if "--attempt" not in argv or len(argv) < 4:
            sys.exit("usage: fix_settings.py fixer|reviewer <tid> --attempt N")
        tid, n = argv[1], int(argv[argv.index("--attempt") + 1])
        if not fc.TASK_ID_RE.match(tid):
            sys.exit(f"bad task id {tid}")
        print((settings_fixer if argv[0] == "fixer" else settings_reviewer)(tid, n))
        return 0
    if len(argv) == 2 and argv[0] == "integrator":
        print(settings_integrator(argv[1]))
        return 0
    if len(argv) >= 2 and argv[0] == "bwrap-argv":
        pr = "--print" in argv
        rest = [x for x in argv[2:] if x != "--print"]
        res = bwrap_argv(argv[1], rest)
        if pr:
            print("\n".join(res))
        else:
            sys.stdout.buffer.write(b"".join(x.encode() + b"\0" for x in res))
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
