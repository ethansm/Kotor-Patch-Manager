#!/usr/bin/env python3
"""Mutation gate for the level-up mod (Phase 6b A3).

Each mutation is (id, file, regex, replacement, description); the regex must match EXACTLY ONCE in today's source of
patch_manager_mods/level_up_override_src/<file> (else the tool errors: a stale mutation is a bug in this list, not a result).
A scratch tree is built under $TMPDIR/kotor-mutate/<id>/ (the sources, research/companion_levelup runtime/tools/twins, tests/luo_scenarios and
_shared_src, same relative paths because the test .cpp files include the sources by relative path), ONE mutation is applied there, and the gates run
in the scratch tree: options_parity.py, wizard_parity.py, parity.py (levelNow/dry-run/apply/gates/playthrough/ui incl. -m32), scenarios.py.
Any gate non-zero (or a gate that no longer compiles... reported as BROKEN, which also fails the run) = KILLED; all green = SURVIVED.
The real sources are never touched.

Usage (from anywhere): python3 tools/parity/mutate.py [--jobs N] [--only ID] [--list] [--keep] [--report PATH]
Exit 1 if any mutation survives or is broken/stale.
"""
import concurrent.futures as cf, os, re, shutil, subprocess, sys, tempfile, time

HERE = os.path.dirname(os.path.abspath(__file__))
CL = os.path.dirname(os.path.dirname(HERE))   # research/companion_levelup
REPO = os.path.dirname(os.path.dirname(CL))
SRC = os.path.join(REPO, "patch_manager_mods", "level_up_override_src")
DEFAULT_REPORT = os.path.join(CL, "phase6_evidence", "A3_mutation_report.md")

CPP, UI, OPT, PT, WIZ = "LevelUpOverride.cpp", "LevelUpUi.h", "OptionsLogic.h", "Playthrough.h", "WizardLogic.h"
# regex helpers: M = mutation list entry
M = []
def m(mid, f, rx, rep, desc): M.append((mid, f, rx, rep, desc))

# ---- levelNow ---------------------------------------------------------------------------------------------------------------------------
m("LN01", CPP, r"\(id >= 3 && id <= 5\) \|\| \(id >= 11 && id <= 13\)", "(id >= 3 && id <= 5) || (id >= 11 && id <= 12)", "isJediClass: drop Watchman (13)")
m("LN02", CPP, r"\(id >= 3 && id <= 5\)", "(id >= 3 && id <= 4)", "isJediClass: drop Sentinel (5)")
m("LN03", CPP, r"key > pauseAt\) \{", "key >= pauseAt) {", "pause branch: key > pauseAt -> >=")
m("LN04", CPP, r"if \(hold && pauseAt > 0 && p->holdClass && key > pauseAt\)", "if (pauseAt > 0 && p->holdClass && key > pauseAt)", "pause branch ignores hold=0")
m("LN05", CPP, r"clsId\[i\] == p->holdClass \|\| isJediClass\(clsId\[i\]\)\) lacks", "clsId[i] == p->holdClass) lacks", "pause branch: other Jedi class no longer counts as having the hold class (F2 rule)")
m("LN06", CPP, r"if \(lacks\) return Decision::Hold;", "if (!lacks) return Decision::Hold;", "pause branch: hold when NOT lacking")
m("LN07", CPP, r"top >= 0 && lvl >= top\)", "top >= 0 && lvl > top)", "hold rule: records used up needs lvl > top")
m("LN08", CPP, r"if \(!has\) return Decision::Hold;", "if (has) return Decision::Hold;", "hold rule: hold when the creature HAS the hold class")
m("LN09", CPP, r"return cl == key \? Decision::Path : Decision::Rebase;", "return cl == key ? Decision::Rebase : Decision::Path;", "PATH/REBASE swapped")
m("LN10", CPP, r"r\.cls == cls && r\.clvl == lvl \+ 1\)", "r.cls == cls && r.clvl == lvl)", "record match uses the current class level, not +1")
m("LN11", CPP, r"if \(!r\.present \|\| r\.clvl == 0\) continue;", "if (!r.present) continue;", "unknown class level (clvl 0) may now match")
m("LN12", CPP, r"const int cls = clsId\[nClass - 1\]", "const int cls = clsId[0]", "newest class = first class")
# ---- decideAct --------------------------------------------------------------------------------------------------------------------------
m("DA01", CPP, r"if \(!applyBuild \|\| !cfgApply\) return LuAct::WouldRewrite;", "if (!applyBuild && !cfgApply) return LuAct::WouldRewrite;", "decideAct: apply gate || -> &&")
m("DA02", CPP, r'if \(!strcmp\(status, "already"\)\) return LuAct::Already;', 'if (!strcmp(status, "already")) return LuAct::SkipStatus;', "decideAct: already -> SkipStatus")
m("DA03", CPP, r'if \(strcmp\(status, "rewrite"\)\) return LuAct::SkipStatus;', 'if (strcmp(status, "rewrite") && strcmp(status, "no-record")) return LuAct::SkipStatus;', "decideAct: no-record treated as rewritable")
m("DA04", CPP, r"if \(!viaAuto\) return LuAct::SkipManual;", "if (!viaAuto && strcmp(status, \"rewrite\")) return LuAct::SkipManual;", "decideAct: manual level-up may rewrite")
# ---- hold / instant gates ---------------------------------------------------------------------------------------------------------------
m("GT01", CPP, r"return held && real \? 0 : r;", "return held ? 0 : r;", "holdGate vetoes also in mode=log")
m("GT02", CPP, r"if \(held && real\) maybeBanked\(", "if (held) maybeBanked(", "BANKED message also in mode=log")
m("GT03", CPP, r"e\.d == Decision::Hold\) return p;(?=\n    const uintptr_t pa)", "false) return p;", "instantGate ignores HOLD")
m("GT04", CPP, r"e\.d == Decision::Hold\) return p;(?=\n    // F4)", "false) return p;", "leaderInstant ignores HOLD")
m("GT05", CPP, r"\|\| reinterpret_cast<CharChange_t>\(g_eng\.charChange\)\(ecx\)\) return p;", ") return p;", "F4: leader instant ignores GetCharacterChangeInProgress")
m("GT06", CPP, r"!readAt\(S_Opt, pa, 8, &flags\) \|\| \(flags & 1\) \|\|", "!readAt(S_Opt, pa, 8, &flags) ||", "instantGate re-sets an already-set auto flag")
m("GT07", CPP, r'(site\);\n        return )1;', r"\g<1>0;", "guiGuard no longer blocks a held leader")
m("GT08", CPP, r'(GUARD autook[^\n]*\n        return )nullptr;', r"\g<1>c;", "autoOkBackstop no longer blocks")
m("GT09", CPP, r"if \(!e\.ok \|\| !e\.cfg->instant \|\| e\.d == Decision::Hold\) return p;(?=\n    const uintptr_t pa)", "if (!e.ok || e.d == Decision::Hold) return p;", "instantGate ignores the instant=0 setting")
# ---- log cap (F1) / HOLD log rate limit (F14) -------------------------------------------------------------------------------------------
m("LG01", CPP, r"    logOpen\(\);   // F1", "    if (!logOpen()) return;   // F1", "F1 regression: capped log disables the rewrite")
m("LG02", CPP, r"if \(!g_f \|\| g_logLines >= MaxLogLines\) return;", "if (!g_f) return;", "log cap never reached")
m("LG03", CPP, r"if \(g_logLines >= MaxLogLines\) return false;", "if (g_logLines > MaxLogLines + 100000) return false;", "logOpen ignores the cap")
m("LG04", CPP, r"\+\+s->suppressed; return; \}", "return; }", "HOLD log: suppressed transitions not counted")
m("LG05", CPP, r"s->everLogged = true; s->suppressed = 0;", "s->everLogged = true;", "HOLD log: suppressed counter never reset")
m("LG06", CPP, r"if \(s->everLogged && now - s->lastLog < HoldLogGapMs\)", "if (false)", "HOLD log: rate limit off")
m("LG07", CPP, r"held \? \(!s->logHeld \|\| s->logKey != key\) : s->logHeld;", "held ? !s->logHeld : s->logHeld;", "HOLD log: a new key while held is not logged")
# ---- BANKED -----------------------------------------------------------------------------------------------------------------------------
m("BK01", CPP, r"if \(n <= be->lastN\) return;", "if (n < be->lastN) return;", "BANKED: re-announce same count")
m("BK02", CPP, r"if \(n < be->lastN\) be->lastN = n;", "", "BANKED: lastN never lowered after levels are taken")
m("BK03", CPP, r"now - g_bankedLast < BankedMinGapMs", "now - g_bankedLast > BankedMinGapMs", "BANKED: min gap inverted")
m("BK04", CPP, r"tbl\[level\] <= xp\)", "tbl[level] < xp)", "bankedFromTable: XP boundary exclusive")
m("BK05", CPP, r"if \(!tbl \|\| level < 1\) return 0;", "if (!tbl) return 0;", "bankedFromTable: level 0 allowed")
m("BK06", CPP, r"if \(n > 1\) snprintf\(out, cap, \"%s: %d levels banked", 'if (n > 2) snprintf(out, cap, "%s: %d levels banked', "banked text: plural threshold")
m("BK07", CPP, r"if \(g_guid != g_bankedGuid\) \{ g_nBankedTags = 0; g_bankedGuid = g_guid; \}", "", "BANKED: per-playthrough reset removed")
# ---- Bank cycler / pause range (LevelUpUi.h) --------------------------------------------------------------------------------------------
m("UI01", UI, r"return cl - 1 > 1 \? cl - 1 : 1;", "return cl > 1 ? cl : 1;", "uiMinPause off by one")
m("UI02", UI, r"pause > p\.convertAt - 2\) return 0;", "pause > p.convertAt - 1) return 0;", "uiClampPause upper bound")
m("UI03", UI, r"pause < uiMinPause\(p\) \|\|", "pause <= uiMinPause(p) ||", "uiClampPause lower bound exclusive")
m("UI04", UI, r"if \(next < uiMinPause\(p\)\)", "if (next <= uiMinPause(p))", "uiBankCycle stops one early")
m("UI05", UI, r"\(\*pause > 0 \? \*pause : p\.convertAt - 1\) - 1;", "(*pause > 0 ? *pause : p.convertAt) - 1;", "uiBankCycle first step starts at convertAt")
m("UI06", UI, r"if \(!\*bank\) \{ \*bank = true; \*pause = 0; return; \}", "if (!*bank) { *bank = true; *pause = 1; return; }", "uiBankCycle Off -> default sets pause 1")
m("UI07", UI, r"p\.holdClass != 0 && p\.convertAt > 0;", "p.holdClass != 0 || p.convertAt > 0;", "uiCanPause && -> ||")
m("UI08", UI, r"\(P\.holdClass && r->bank\) \? uiClampPause", "(P.holdClass) ? uiClampPause", "Apply stores a pause although Bank is off")
m("UI09", UI, r"if \(!uiCanPause\(p\)\) \{ \*bank = !\*bank; \*pause = 0; return; \}", "if (!uiCanPause(p)) { *bank = !*bank; return; }", "uiBankCycle toggle keeps a stale pause")
# ---- status text builder / Configure button ---------------------------------------------------------------------------------------------
m("ST01", UI, r"banked == 1 \? \"\" : \"s\"", 'banked == 2 ? "" : "s"', "status: singular/plural")
m("ST02", UI, r"else if \(held\) snprintf", "else if (false) snprintf", "status: 'Holding the next level' text removed")
m("ST03", UI, r"instant \? \" \(instant\)\" : \"\"", 'instant ? "" : " (instant)"', "status: (instant) suffix inverted")
m("ST04", UI, r"\*highlight = !configured && canLevelUp;", "*highlight = !configured || canLevelUp;", "Configure highlight && -> ||")
m("ST05", UI, r"if \(held && banked > 0\)", "if (held && banked > 1)", "status: banked>0 threshold")
# ---- OptionsLogic -----------------------------------------------------------------------------------------------------------------------
m("OP01", OPT, r"for \(int i = 1; i < s\.n; \+\+i\) if \(!ieq\(val\(s, i, row\), val\(s, 0, row\)\)\) return true;", "for (int i = 2; i < s.n; ++i) if (!ieq(val(s, i, row), val(s, 0, row))) return true;", "rowShown skips item 1")
m("OP02", OPT, r"for \(int r = 0; r < upTo; \+\+r\)", "for (int r = 0; r <= upTo; ++r)", "matchBefore includes the row itself")
m("OP03", OPT, r"(s\.cur = first\[\(\(at \+ \()dir < 0 \? -1 : 1(\)\) % nv)", r"\g<1>1\g<2>", "Variant cycle ignores direction")
m("OP04", OPT, r"(const char\* want = detail::val\(s, first\[\(\(at \+ \()dir < 0 \? -1 : 1(\)\) % nv)", r"\g<1>1\g<2>", "row cycle ignores direction")
m("OP05", OPT, r"better = sc\[r\]; break;", "better = !sc[r]; break;", "cycle tie-break inverted")
m("OP06", OPT, r"return detail::variantGroup\(s, g\) > 1;", "return detail::variantGroup(s, g) > 0;", "Variant always shown")
m("OP07", OPT, r"sc\[r\] = r > row && ieq", "sc[r] = r > row + 1 && ieq", "cycle tie-break skips the next row")
# ---- Playthrough ------------------------------------------------------------------------------------------------------------------------
m("PT01", PT, r"l->pause = n > 0 \? n : 0; \}", "l->pause = n; }", "parse: negative pause kept")
m("PT02", PT, r"l->instant = false; l->hold = true; l->pause = 0;", "l->instant = false; l->hold = false; l->pause = 0;", "parse: hold defaults to 0")
m("PT03", PT, r"if \(l\.pause > 0\) snprintf", "if (l.pause >= 0) snprintf", "serialise: always writes |pause=")
m("PT04", PT, r"l->pause = pause > 0 \? pause : 0;\n", "l->pause = pause;\n", "setTag: negative pause stored")
m("PT05", PT, r"d\.crlf = nCrlf > 0 && nCrlf \* 2 > nLf;", "d.crlf = nCrlf > 0 && nCrlf * 2 >= nLf;", "CRLF detection: tie counts as CRLF")
m("PT06", PT, r" \|\| b\[-1\] == '\\t'\)\) --b;", ")) --b;", "inline comment: tab before ';' not kept")
m("PT07", PT, r"memcpy\(l->text, cm, strlen\(cm\) \+ 1\)", 'memcpy(l->text, "", 1)', "inline comment dropped on rewrite")
m("PT08", PT, r"const size_t next = e < \*len \? e \+ 1 : \*len;", "const size_t next = e + 1;", "removeTagRaw: last line without newline overruns")
m("PT09", PT, r"if \(inSec\) return false;   // the first matching", "if (false) return false;   // the first matching", "removeTagRaw: continues into later sections")
m("PT10", PT, r"memmove\(text \+ pos, text \+ next, \*len - next\);", "memmove(text + pos, text + e, *len - e);", "removeTagRaw leaves the line break")
m("PT11", PT, r"inSec = parseSectionName\(trimStr\(t \+ 1\), &g\) && g == guid;", "inSec = parseSectionName(trimStr(t + 1), &g);", "removeTagRaw: any GUID's section counts")
m("PT12", PT, r"if \(eqi\(trimStr\(t\), tag\)\) \{", "if (true) {", "removeTagRaw removes the first line, not the tag's")
# ---- GUID bit packing -------------------------------------------------------------------------------------------------------------------
m("GU01", PT, r"\(guid >> i\) & 1u", "(guid >> (i ^ 1)) & 1u", "unpackBits: neighbouring bits swapped")
m("GU02", PT, r"g \|= 1u << i;", "g |= 1u << (31 - i);", "packBits: reversed")
m("GU03", PT, r'"LUO_G%02d", i\)', '"LUO_G%02d", i + 1)', "bitName off by one")
m("GU04", PT, r'"g%08x", static_cast<unsigned>\(guid\)', '"g%07x", static_cast<unsigned>(guid)', "sectionName width")
m("GU05", PT, r"\(const uint8_t\* bits /\* 32, nonzero = set \*/\) \{\n    uint32_t g = 0;\n    for \(int i = 0; i < 32; \+\+i\)", "(const uint8_t* bits /* 32, nonzero = set */) {\n    uint32_t g = 0;\n    for (int i = 0; i < 31; ++i)", "packBits drops bit 31")
# ---- WizardLogic (pure twin-tested logic) -----------------------------------------------------------------------------------------------
m("WZ01", WIZ, r"if \(nc == 0\) return v;", "", "wizard: no candidates falls through to a Pick page")
m("WZ02", WIZ, r"if \(nv >= 2\) \{\n            v\.page = p;", "if (nv >= 1) {\n            v.page = p;", "wizard: single-valued pages are asked")
m("WZ03", WIZ, r"if \(a\.page == Pick\) \{ if \(i != a\.ref\) return false; \}", "if (a.page == Pick) { }", "wizard: a Pick answer does not pin the item")
m("WZ04", WIZ, r"\(ans\[i\]\[0\] == '-' && !ans\[i\]\[1\]\)\) return false;", "false) return false;", "wizard: '-' answers selectable")
m("WZ05", WIZ, r"if \(s\.depth <= 0\) return false;\n    --s\.depth;", "if (s.depth < 0) return false;\n    --s.depth;", "wizard: back() at depth 0")


# ---- carry-over (computeCarry / rule parser / known-power read) ------------------------------------------------------------------------------
m("CO01", CPP, r"if \(r\.minChar > key\) return CSt::Level;", "if (r.minChar >= key) return CSt::Level;", "carry: feat min character level off by one")
m("CO02", CPP, r"if \(mc > key\) return CSt::Level;", "if (mc >= key) return CSt::Level;", "carry: power min character level off by one")
m("CO03", CPP, r"if \(r\.nOr && !any\) for", "if (false) for", "carry: the any-of (OR) prerequisite list is ignored")
m("CO04", CPP, r"if \(key == cl && !nOwedF && !nOwedP\) \{ ci->kind = \"ontime\"", "if (false && !nOwedF && !nOwedP) { ci->kind = \"ontime\"", "carry: the on-time gate is skipped")
m("CO05", CPP, r"if \(st == CSt::Prereq && depth < 4 && nm\) \{", "if (false && st == CSt::Prereq && depth < 4 && nm) {", "carry: no prerequisite pull")
m("CO06", CPP, r"if \(st == CSt::Prereq && depth < 4 && nm\) \{", "if (st == CSt::Prereq && depth < 5 && nm) {", "carry: prerequisite chain depth cap 4 -> 5")
m("CO07", CPP, r"int f = carryPick\(true, ownF, nOwnF,", "int f = carryPick(true, owedF, nOwedF,", "carry: owed picks first, the current record's picks never planned (Jedi-first tie-break lost)")
m("CO08", CPP, r"for \(int i = 0; i < in\.nFeats && nFbF < noFillF; \+\+i\) \{", "for (int i = 0; i < in.nFeats && nFbF < 0; ++i) {", "carry: no engine-pick fallback for an unfilled feat slot")
m("CO09", CPP, r"for \(int i = 0; i < in\.nSpells && nFbP < noFillP; \+\+i\) \{", "for (int i = 0; i < in.nSpells && nFbP < 0; ++i) {", "carry: no engine-pick fallback for an unfilled power slot")
m("CO10", CPP, r"constexpr int St_ClassSpells = 0x90;", "constexpr int St_ClassSpells = 0x8C;", "known powers read from the wrong struct offset")
m("CO11", CPP, r"return isEngineGrantPower\(v\) \|\| has\(p, np, v\);", "return has(p, np, v);", "carry: engine-granted powers no longer count as known")
m("CO12", CPP, r"if \(cls < 0 \|\| cls > MaxRuleClass \|\| !\(\(r\.clsMask >> cls\) & 1\)\) return CSt::Class;", "if (cls < 0 || cls > MaxRuleClass) return CSt::Class;", "carry: the class may-pick mask is ignored")
m("CO13", CPP, r"pr\.minLvl\[a\[0\]\] = b\[0\];", "pr.minLvl[a[0]] = 0;", "parser: [powerrules] minimum levels dropped")
m("CO14", CPP, r"res\.status = samePend\(o, in\) \? \"already\" : \"rewrite\";", "res.status = \"rewrite\";", "carry: a replayed carry result is not 'already'")
m("CO15", CPP, r"if \(!strcmp\(res\.status, \"negative-unspent\"\)\) \{ ci->kind = \"negative-unspent\"; return res; \}", "", "carry: negative-unspent no longer returns the untouched pending")
m("CO16", CPP, r"if \(!g_rules\) logLine\(\"CARRY disabled", "if (g_rules) logLine(\"CARRY disabled", "old table: the 'carry-over off' line inverted")
m("CO17", CPP, r"if \(!ks \|\| !p\) \{ ci->kind = \"skipped\"; return res; \}", "if (!p) { ci->kind = \"skipped\"; return res; }", "carry: unreadable feat lists no longer skip carry-over")
m("CO18", CPP, r"\|\| !readAt\(S_Class, stats, off \+ 4, &pc\) \|\| pc > static_cast<uint32_t>\(MaxPowersPerClass\)\) \{ ok = false; break; \}", "|| !readAt(S_Class, stats, off + 4, &pc)) { ok = false; break; }", "known powers: the per-class cap check dropped")
m("CO19", CPP, r"if \(!g_rules\) \{ ci->kind = \"norules\"; return res; \}", "", "carry: runs without rule sections (old table)")
m("CO20", CPP, r"for \(int i = 0; i < nFbF; \+\+i\) o\.feats\[o\.nFeats\+\+\] = fbF\[i\];\n    res\.nKeptFeats = o\.nFeats;", "res.nKeptFeats = o.nFeats;\n    for (int i = 0; i < nFbF; ++i) o.feats[o.nFeats++] = fbF[i];", "carry: engine-pick fallback feats not part of the kept prefix")

BY_ID = {x[0]: x for x in M}
assert len(BY_ID) == len(M), "duplicate mutation id"


def apply(mut, text):
    mid, f, rx, rep, desc = mut
    found = list(re.finditer(rx, text))
    if len(found) != 1:
        raise SystemExit("STALE mutation %s: regex matches %d times in %s (need exactly 1): %s" % (mid, len(found), f, rx))
    new = text[:found[0].start()] + found[0].expand(rep) + text[found[0].end():]
    if new == text:
        raise SystemExit("STALE mutation %s: replacement is a no-op" % mid)
    return new


def build_base(base):
    ig = shutil.ignore_patterns("*_bin", "*_bin32", "__pycache__", "*.pyc", "logs")
    def cp(src, dst): shutil.copytree(os.path.join(REPO, src), os.path.join(base, dst or src), ignore=ig)
    cp("patch_manager_mods/level_up_override_src", None)
    cp("patch_manager_mods/_shared_src", None)
    cp("tests/luo_scenarios", None)
    os.makedirs(os.path.join(base, "research/companion_levelup"))
    for sub in ("runtime", "tools/parity"):
        cp("research/companion_levelup/" + sub, None)
    for f in ("simulate_rewrite.py", "levelup_rules.py", "options_twin.py", "wizard_twin.py"):
        shutil.copy(os.path.join(CL, f), os.path.join(base, "research/companion_levelup", f))


GATES = (["tools/parity/options_parity.py"], ["tools/parity/wizard_parity.py"], ["tools/parity/parity.py"], ["tools/parity/scenarios.py"])


def run_one(mut, root, keep):
    mid = mut[0]
    d = os.path.join(root, mid)
    shutil.copytree(os.path.join(root, "base"), d, symlinks=True)
    tgt = os.path.join(d, "patch_manager_mods/level_up_override_src", mut[1])
    with open(tgt, newline="") as fh: text = fh.read()
    with open(tgt, "w", newline="") as fh: fh.write(apply(mut, text))
    cwd = os.path.join(d, "research/companion_levelup")
    t0 = time.time()
    status, by = "SURVIVED", ""
    for g in GATES:
        try:
            r = subprocess.run([sys.executable] + g, cwd=cwd, capture_output=True, text=True, timeout=900)
        except subprocess.TimeoutExpired:
            status, by = "KILLED", os.path.basename(g[0]) + " (timeout)"
            break
        out = r.stdout + r.stderr
        if r.returncode:
            comp = "error:" in out or "COMPILE FAILED" in out
            status, by = ("BROKEN" if comp else "KILLED"), os.path.basename(g[0])
            if comp:
                by += " (does not compile)"
            break
    if not keep:
        shutil.rmtree(d, ignore_errors=True)
    return mid, status, by, time.time() - t0


def main():
    a = sys.argv[1:]
    if "--list" in a:
        for mid, f, rx, rep, desc in M:
            print("%-5s %-18s %s" % (mid, f, desc))
        print(len(M), "mutations")
        return 0
    jobs = int(a[a.index("--jobs") + 1]) if "--jobs" in a else 4
    only = a[a.index("--only") + 1] if "--only" in a else None
    report = a[a.index("--report") + 1] if "--report" in a else DEFAULT_REPORT
    keep = "--keep" in a
    muts = [BY_ID[only]] if only else M
    # staleness check on the real sources first (read-only)
    for mut in M:
        with open(os.path.join(SRC, mut[1]), newline="") as fh:
            apply(mut, fh.read())
    root = os.path.join(os.environ.get("TMPDIR") or tempfile.gettempdir(), "kotor-mutate")
    shutil.rmtree(root, ignore_errors=True)
    os.makedirs(root)
    build_base(os.path.join(root, "base"))
    t0 = time.time()
    res = {}
    with cf.ThreadPoolExecutor(jobs) as ex:
        for mid, st, by, dt in ex.map(lambda mu: run_one(mu, root, keep), muts):
            res[mid] = (st, by, dt)
            print("%-5s %-8s %-34s %5.0fs  %s" % (mid, st, by, dt, BY_ID[mid][4]), flush=True)
    elapsed = time.time() - t0
    shutil.rmtree(os.path.join(root, "base"), ignore_errors=True)
    killed = sum(1 for v in res.values() if v[0] == "KILLED")
    bad = [k for k, v in res.items() if v[0] != "KILLED"]
    print("\n%d mutations, %d killed (%.1f%%), %d survived/broken, %.0fs" % (len(res), killed, 100.0 * killed / len(res), len(bad), elapsed))
    if not only:
        with open(report, "w") as fh:
            fh.write("# A3 mutation report\n\nGenerated by `research/companion_levelup/tools/parity/mutate.py` (jobs=%d, %.0fs wall).\n" % (jobs, elapsed))
            fh.write("Gates per mutation (scratch tree): options_parity.py, wizard_parity.py, parity.py (incl. -m32 gate/ui), scenarios.py.\n\n")
            fh.write("**%d mutations, %d killed (%.1f%%), %d survived/broken.**\n\n" % (len(res), killed, 100.0 * killed / len(res), len(bad)))
            fh.write("| id | file | status | killed by | mutation |\n|---|---|---|---|---|\n")
            for mid, f, rx, rep, desc in M:
                st, by, dt = res[mid]
                fh.write("| %s | %s | %s | %s | %s |\n" % (mid, f, st, by, desc))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
