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
R10F = "R10Filter.h"
R10P = "R10Page.h"
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
m("AN01", CPP, r"if \(!e\.cfg->instant\) return false;   // a non-instant", "if (false) return false;   // a non-instant", "apply-now levels a non-instant config")
m("AN02", CPP, r"\|\| pcr\) \{", ") {", "apply-now ignores PlayerCreated")
m("AN03", CPP, r"if \(g_cfg\.mode != Mode::Rewrite\) \{ logLine\(\"INSTANT apply-now WOULD", "if (false) { logLine(\"INSTANT apply-now WOULD", "apply-now acts in log mode")
m("AN04", CPP, r"g_applyNow\.pending = false;   // one-shot, also", "g_applyNow.pending = true;   // one-shot, also", "apply-now request never cleared (not one-shot)")
m("AN05", CPP, r"if \(!held && g_applyNow\.pending && isMenuDrawSite", "if (g_applyNow.pending && isMenuDrawSite", "apply-now also levels a HELD creature")
# ---- log cap (F1) / HOLD log rate limit (F14) -------------------------------------------------------------------------------------------
m("LG01", CPP, r"    logOpen\(\);   // F1", "    if (!logOpen()) return;   // F1", "F1 regression: capped log disables the rewrite")
m("LG02", CPP, r"if \(!g_f \|\| g_logLines >= MaxLogLines\) return;", "if (!g_f) return;", "log cap never reached")
m("LG03", CPP, r"if \(g_logLines >= MaxLogLines\) return false;", "if (g_logLines > MaxLogLines + 100000) return false;", "logOpen ignores the cap")
m("LG04", CPP, r"if \(differs\) \+\+s->suppressed; return; \}", "return; }", "HOLD log: suppressed transitions not counted")
m("LG05", CPP, r"s->everLogged = true; s->suppressed = 0;", "s->everLogged = true;", "HOLD log: suppressed counter never reset")
m("LG06", CPP, r"if \(s->everLogged && now - s->lastLog < HoldLogGapMs\)", "if (false)", "HOLD log: rate limit off")
m("LG08", CPP, r"if \(!differs && !s->suppressed\) return;", "if (!differs) return;", "HOLD log: a withheld flap that ended on the logged state is never flushed")
m("LG09", CPP, r"if \(differs\) \+\+s->suppressed; return; \}", "++s->suppressed; return; }", "HOLD log: unchanged-state calls inside the gap are counted as suppressed transitions")
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
m("UI02", UI, r"pause >= uiPauseHi\(p\)\) return 0;", "pause > uiPauseHi(p)) return 0;", "uiClampPause upper bound")
m("UI03", UI, r"pause < uiPauseLo\(p, cur\) \|\|", "pause <= uiPauseLo(p, cur) ||", "uiClampPause lower bound exclusive")
m("UI04", UI, r"if \(e > lo\) \*pause = e - 1 >= hi", "if (e > lo + 1) *pause = e - 1 >= hi", "uiPauseStep '-' stops one early")
m("UI05", UI, r"else if \(e < hi\) \*pause = e \+ 1 >= hi \? 0 : e \+ 1;", "else if (e < hi) *pause = e + 1;", "uiPauseStep '+' reaching hi does not store 0")
m("UI06", UI, r"\*bank = true; \*pause = uiStartPause\(p, cur\);", "*bank = true; *pause = 0;", "uiBankToggle Off -> On ignores the current level")
m("UI07", UI, r"p\.holdClass != 0 && p\.convertAt > 0;", "p.holdClass != 0 || p.convertAt > 0;", "uiCanPause && -> ||")
m("UI08", UI, r"\(P\.holdClass && r->bank\) \? uiClampPause", "(P.holdClass) ? uiClampPause", "Apply stores a pause although Bank is off")
m("UI09", UI, r"if \(!uiCanPause\(p\)\) \{ \*bank = !\*bank; \*pause = 0; return; \}\n    if \(!\*bank\)", "if (!uiCanPause(p)) { *bank = !*bank; return; }\n    if (!*bank)", "uiBankToggle toggle keeps a stale pause")
m("UI10", UI, r"int uiPauseLo\(const Preset& p, int cur\) \{ const int mn = uiMinPause\(p\); return cur > mn \? cur : mn; \}", "int uiPauseLo(const Preset& p, int cur) { const int mn = uiMinPause(p); return mn; }", "pause lo bound ignores the current level")
m("UI11", UI, r"int uiPauseHi\(const Preset& p\) \{ return p\.convertAt - 1; \}", "int uiPauseHi(const Preset& p) { return p.convertAt; }", "pause hi bound off by one")
m("UI12", UI, r"r->pause = configured \? cfg->pause : uiStartPause\(sp, curLevel\);", "r->pause = configured ? cfg->pause : 0;", "unconfigured page default ignores the current level")
m("UI13", UI, r"return pause < lo \? lo : uiClampPause\(p, pause, cur\);", "return uiClampPause(p, pause, cur);", "configured pause below the current level is zeroed, not raised")
m("UI14", UI, r"uiOpenPage\(r, tag, r->curLevel\)", "uiOpenPage(r, tag, -1)", "cfg click ignores the current level")
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
m("CO18", CPP, r"pc > static_cast<uint32_t>\(MaxPowersPerClass\) \|\| ", "", "known powers: the per-class cap check dropped")
m("CO19", CPP, r"if \(!g_rules\) \{ ci->kind = \"norules\"; return res; \}", "", "carry: runs without rule sections (old table)")
m("CO20", CPP, r"for \(int i = 0; i < nFbF && o\.nFeats < MaxPend; \+\+i\) o\.feats\[o\.nFeats\+\+\] = fbF\[i\];\n    res\.nKeptFeats = o\.nFeats;", "res.nKeptFeats = o.nFeats;\n    for (int i = 0; i < nFbF && o.nFeats < MaxPend; ++i) o.feats[o.nFeats++] = fbF[i];", "carry: engine-pick fallback feats not part of the kept prefix")

m("CO21", CPP, r"for \(int i = 0; i < nPickF && o\.nFeats < MaxPend; \+\+i\)", "for (int i = 0; i < nPickF; ++i)", "carry: planned feat picks appended without the MaxPend bound")
m("CO22", CPP, r"for \(int i = 0; i < nPickP && o\.nSpells < MaxPend; \+\+i\)", "for (int i = 0; i < nPickP; ++i)", "carry: planned power picks appended without the MaxPend bound")
m("CO23", CPP, r"for \(int i = 0; i < nKeptF && o\.nFeats < MaxPend; \+\+i\)", "for (int i = 0; i < nKeptF && o.nFeats < MaxPend - 1; ++i)", "carry: kept feat bound off by one (valid prefix lost)")
m("CO24", CPP, r"if \(g_ruleBadLine\) \{ memset\(g_fr, 0, sizeof g_fr\);", "if (false) { memset(g_fr, 0, sizeof g_fr);", "parser: a bad rule row leaves the partial rule tables active (carry not disabled)")
m("CO25", CPP, r"\? parseFeatRule\(k, v\) : parsePowerRule\(k, v\)\)\) g_ruleBadLine", "? parseFeatRule(k, v) : true)) g_ruleBadLine", "parser: [powerrules] rows are never validated")
m("CO26", CPP, r"if \(g_ruleBadLine\) logLine\(", "if (false) logLine(", "parser: the 'CARRY disabled: malformed rule line N' log line dropped")
m("CO27", CPP, r"if \(!eq\) \{\n            if \(sec != FeatRules && sec != PowerRules\) goto bad;", "if (!eq) {\n            goto bad;", "parser: a rule row without '=' disables the whole table again")
m("CO28", CPP, r"pcap >= 4096\) \{ ok = false; break; \}", "pcap >= 40960) { ok = false; break; }", "known powers: cap < 4096 sanity loosened")
m("CO29", CPP, r"pc > pcap \|\|", "", "known powers: count <= cap sanity dropped")
m("CO30", CPP, r"lv >= 2 && !ks->np\) ok = false;", "lv >= 3 && !ks->np) ok = false;", "known powers: Jedi class level 2 with no powers accepted")
m("CO31", CPP, r"isJediClass\(id\) && lv >= 2", "isJediClass(id) && lv >= 2 && id != 13", "known powers: Watchman (13) exempt from the empty-array sanity")

# ---- hover tooltips (GuiTip.h in _shared_src + the LevelUpUi.h adoption; killed by ui_test section 5) ---------------------------------------------------
G = "../_shared_src/GuiTip.h"
m("TP01", G, r"delay \+ FallbackGraceMs;", "delay;", "tip fallback: grace after the engine delay removed")
m("TP02", G, r">= delay \+ FallbackGraceMs", "> delay + FallbackGraceMs", "tip fallback: box one ms late (>= -> >)")
m("TP03", G, r"if \(nativeOwnsHovered\) s\.nativeSeen = true;", "if (false) s.nativeSeen = true;", "tip fallback: nativeSeen never set")
m("TP04", G, r"if \(nativeOwnsHovered \|\| s\.nativeSeen\) \{", "if (nativeOwnsHovered) {", "tip fallback: nativeSeen not sticky")
m("TP05", G, r"m\.appear \* 1000\.f", "m.appear * 100.f", "tip delay: appear time seconds -> ms factor")
m("TP06", G, r"return DefaultDelayMs;", "return 0;", "tip delay: unreadable manager gives no default delay")
m("TP07", G, r"if \(r\.x \+ r\.w > reg\.x1\) r\.x = reg\.x1 - r\.w;", "", "tip layout: right clamp removed")
m("TP08", G, r"r\.y = ctl\.y - r\.h - gap;", "r.y = ctl.y + ctl.h + gap;", "tip layout: no flip above at the bottom")
m("TP10", UI, r"guitip::setNative\(r->c\[i\], uiTipText\(i\)\)", "false", "UI: native tip strings never set")
m("TP11", UI, r"if \(clicked \|\| r->open != r->tipOpen\)", "if (r->open != r->tipOpen)", "UI: a click no longer hides the tip box")
m("TP12", UI, r"uiPlace\(UiPanel\* r, const guikit::Scale& s\) \{\n    for \(int i = 0; i < C_TIPBG; \+\+i\)", "uiPlace(UiPanel* r, const guikit::Scale& s) {\n    for (int i = 0; i < C_N; ++i)", "UI: uiPlace also moves the tip controls back to their spec rect each tick")
# ---- R10 PC level-up filter (R10Filter.h + the Playthrough.h filter fields; killed by r10_filter_test.cpp (run from parity.py) and playthrough_test.cpp) ---------
m("RF01", R10F, r"if \(kept == 0 && picksLeft > 0 && m != All\) return -1;", "if (kept == 0 && picksLeft >= 0 && m != All) return -1;", "filterChoosable: fallback also with no picks left")
m("RF02", R10F, r"if \(kept == 0 && picksLeft > 0 && m != All\) return -1;", "if (kept == 0 && picksLeft > 1 && m != All) return -1;", "filterChoosable: fallback only with 2+ picks left (soft-lock with one pick)")
m("RF03", R10F, r"m == BuildHide \? Combat :", "m == BuildHide ? Force :", "nextMode: BuildHide skips Combat")
m("RF04", R10F, r"m == Combat \? Force : All;", "m == Combat ? All : All;", "nextMode: Force unreachable")
m("RF05", R10F, r"    if \(\(tags & T_Dark\) && \(p\.align & T_Light\)\) return false;\n", "", "fits: dark row vs light build no longer conflicts")
m("RF06", R10F, r"\(tags & T_Light\) && \(p\.align & T_Dark\)", "(tags & T_Light) && (p.align & T_Neutral)", "fits: light row conflicts with a neutral build instead of dark")
m("RF07", R10F, r"    if \(tags & T_General\) return true;\n    if \(!\(\(tags & p\.role\)", "    if (!((tags & p.role)", "fitsMask: the general shortcut dropped")
m("RF08", R10F, r"\(tags & p\.role\) \|\| \(tags & p\.style\)", "(tags & p.role) && (tags & p.style)", "fitsMask: role AND style required")
m("RF09", R10F, r"    if \(tags & T_General\) return true;\n    return \(tags & \(m == Combat", "    return (tags & (m == Combat", "fits: general no longer passes the Combat/Force modes")
m("RF10", R10F, r"if \(!tags\) return true;   // untagged / unknown: never filter", "if (!tags) return false;", "fits: untagged ids are filtered")
m("RF11", R10F, r"m == Combat \? T_Combat : T_Force", "m == Combat ? T_Force : T_Combat", "fits: Combat and Force tags swapped")
m("RF12", R10F, r"return m == BuildHide;", "return m == BuildHide || m == BuildDim;", "hides(): BuildDim hides")
m("RF13", R10F, r"return m == BuildDim \|\| m == Combat \|\| m == Force;", "return m == BuildDim || m == Combat;", "dims(): Force never dims")
m("RF14", R10F, r"if \(\*slot & Present\) \{ setErr\(err, errCap, lineNo, \"duplicate id\"\);", "if (false) { setErr(err, errCap, lineNo, \"duplicate id\");", "tag ini: duplicate ids accepted")
m("RF15", R10F, r"else \{ setErr\(err, errCap, lineNo, \"unknown section\"\); t->clear\(\); return false; \}", "else sec = Feat;", "tag ini: unknown sections accepted as feats")
m("RF16", R10F, r"if \(id >= MaxId\) \{", "if (id > MaxId) {", "tag ini: id == MaxId accepted")
m("RF17", R10F, r"if \(!\(role & RoleMask\)\) return false;", "if (!role) return false;", "parseProfile: a non-role word accepted as the role")
m("RF18", R10F, r"strcmp\(parts\[2\], \"none\"\) && !\(align & AlignMask\)", "strcmp(parts[2], \"none\") && !align", "parseProfile: a non-alignment word accepted as the alignment")
m("RF19", R10F, r"\*p = Profile\(\);\n    if \(!text\) return false;", "if (!text) return false;", "parseProfile: failure leaves a stale profile")
m("RF20", R10F, r"\"unknown tag word\"\); t->clear\(\); return false; \}", "\"unknown tag word\"); return false; }", "tag ini: an unknown tag word leaves partial data")
# ---- R10 dim (batch 2a: r10f::dimInPlace + LuoR10Dim_* in LevelUpOverride.cpp; killed by r10_filter_test.cpp and gate_test.cpp) ---------------------
m("RF21", R10F, r"\*newCount = w;", "*newCount = w + 1;", "dimInPlace: compacted count off by one")
m("RF22", R10F, r"    if \(kept == 0 && picksLeft > 0\) return DimFallback;\n", "", "dimInPlace: empties the vector while picks are left (no fallback)")
m("RF23", R10F, r"    if \(m == BuildDim && !p\.valid\(\)\) return DimNoProfile;\n", "", "dimInPlace: an invalid profile dims almost everything")
m("RF24", R10F, r"return m == BuildHide \? BuildDim : m;", "return m;", "dimMode: build_hide no longer dims in batch 2a")
m("RF25", R10F, r"    if \(kept == n\) return DimNoChange;\n", "", "dimInPlace: rewrites an unchanged vector")
m("RD01", CPP, r"if \(pc != 1\) \{ logLine", "if (pc == 1) { logLine", "r10Dim: PC gate inverted (companions dimmed, PC vanilla)")
m("RD02", CPP, r"if \(!eff\.on \|\| g_cfg\.mode == Mode::Off\) return;", "if (g_cfg.mode == Mode::Off) return;", "r10Dim: r10=0 / pc line off ignored")
m("RD03", CPP, r"    \*reinterpret_cast<int32_t\*>\(static_cast<uintptr_t>\(vec\) \+ 4\) = n;\n", "", "r10DimVector: count not written back")
m("RD04", CPP, r"if \(!eff\.on \|\| g_cfg\.mode == Mode::Off\) return;", "if (!eff.on) return;", "r10Dim: mode=off no longer disables the dim")
# ---- R10 batch 3a (Playthrough.h pc line, R10Page.h, LevelUpUi.h icon/PC page, r10Dim precedence; killed by playthrough/r10_page/ui/gate tests) -----
m("RB01", PT, r"if \(l\.r10\) snprintf", "if (true) snprintf", "serialise: |r10=1 written when off (old files no longer byte-identical)")
m("RB02", PT, r"s->lines\[i\]\.r10 = on;", "s->lines[i].r10 = true;", "setPc: filter off saved as on")
m("RB03", PT, r"    if \(d\.lossy\) return false;\n    if \(!findPc", "    if (!findPc", "setPc: rewrites a lossy doc")
m("RB04", R10P, r"\*f = \(\(\*f \+ \(dir < 0 \? -1 : 1\)\) % n \+ n\) % n;", "*f = ((*f + (dir < 0 ? -1 : 1)) % (n - 1) + n - 1) % (n - 1);", "R10Page cycle: wraps one value early (last value unreachable)")
m("RB05", R10P, r"st\.feats = modeIndex\(r10f::parseMode\(ffilter && \*ffilter \? ffilter : \"all\"\)\);", "st.feats = 1;", "R10Page fromTokens: a saved feats mode (incl. all) reloads as build_dim")
m("RB06", UI, r"    uiShow\(r, C_CFG, lvl\);   // batch 3a", "    uiShow(r, C_CFG, !lvl);   // batch 3a", "companion: Configure / icon rule inverted")
m("RB07", UI, r"    uiShowIcon\(r, !lvl\);\n    char st", "    uiShowIcon(r, lvl);\n    char st", "PC: icon shown next to the level-up buttons instead of without them")
m("RB08", UI, r"if \(!haveTag \|\| \(!pc && !uiCountSelectable\(tag\)\)\)", "if (!haveTag || pc || !uiCountSelectable(tag))", "the PC treated as hidden again (no filter page)")
m("RB09", UI, r"if \(r->pc\) uiOpenR10\(r\); else uiOpenPage", "if (false) uiOpenR10(r); else uiOpenPage", "the PC's icon opens the companion options page")
m("RB10", CPP, r"if \(const pt::Line\* l = ptPcLine\(\)\) \{\n        e\.playthrough = true;", "if (const pt::Line* l = g_cfg.r10 ? nullptr : ptPcLine()) {\n        e.playthrough = true;", "r10Effective: ini r10=1 wins over the playthrough pc line")
m("RB11", CPP, r"if \(!l\.isTag \|\| pt::eqi\(l\.tag, pt::kPcKey\)\) continue;", "if (!l.isTag) continue;", "rebuildPtCfg: the pc line becomes a companion tag")
m("RD05", CPP, r"kind == r10::Powers \? r10DimVector<uint32_t>", "kind == r10::Powers ? r10DimVector<uint16_t>", "r10Dim: powers vector read as ushort ids")
m("RD06", CPP, r"return \(!str \|\| !len\) \? 1 : 0;", "return (!str || !len) ? 1 : 1;", "r10IsPc: every readable creature counts as the PC")
m("PF01", PT, r"if \(l\.pcfilter\[0\]\) snprintf", "if (true) snprintf", "serialise: |pcfilter= always written")
m("PF02", PT, r"if \(l\.ffilter\[0\]\) snprintf", "if (true) snprintf", "serialise: |ffilter= always written")
m("PF03", PT, r"if \(l\.pfilter\[0\]\) snprintf", "if (true) snprintf", "serialise: |pfilter= always written")
m("PF04", PT, r"if \(allDefault && !strcmp\(dst, \"all\"\)\) dst\[0\] = 0;", "", "setToken: `all` mode kept (default would be written)")
m("PF06", PT, r"c == '_' \|\| c == '/'\)\) \{ dst\[0\] = 0; return; \}", "c == '_' || c == '/' || c == ' ')) { dst[0] = 0; return; }", "setToken: blanks accepted in a token")
m("PF07", PT, r"if \(n == 0 \|\| n >= static_cast<size_t>\(cap\)\) return;", "if (n == 0) return;", "setToken: over-long token not rejected")

# ---- R10 batch 3c (EmbedTex.h heal/give-up, LevelUpUi.h square icon / framed window; killed by ui_test section 7 + embtex via the stub twin) -----
E = "../_shared_src/EmbedTex.h"
m("IC01", UI, r"lroundf\(kIconSide \* s\.sy\)", "lroundf(kIconSide * s.sx)", "icon side from sx instead of sy (stretched / wrong size on a wide screen)")
m("IC02", UI, r"if \(x\) \*x = right - side;", "if (x) *x = guikit::unitX(s, static_cast<float>(kIconRight - kIconSide));", "icon right-edge pin dropped (left edge from the spec rect)")
m("IC03", UI, r"if \(i == C_BG0 && \(c = guikit::createLabelFromTemplate\(panel, kTplWindow\)\) != 0\) r->win = true;", "if (i == C_BG0 && (c = guikit::createLabelFromTemplate(panel, kTplWindow)) == 0) r->win = true;", "window template fallback inverted")
m("IC04", UI, r"    embtex::heal\(\);   // refill", "    // embtex::heal();   // refill", "embtex::heal() never called")
m("IC05", E, r"if \(s\.heals >= MaxHeals\) continue;", "if (s.heals > MaxHeals) continue;", "EmbedTex: refill cap off by one (21 refills)")
# ---- R10 batch 3d (LevelUpUi.h hover tint, gear closes the page, one opaque back, gear inset; killed by ui_test section 8) -----
m("HL01", UI, r"if \(r->tinted >= 0 && r->c\[r->tinted\]\) guikit::setBorderTint\(r->c\[r->tinted\], r->baseTint\[r->tinted\]\);", "", "hover tint never restored")
m("HL02", UI, r"if \(i == C_ICON\) return true;", "if (i == C_ICON) return !r->open;", "icon not clickable while the page is open")
m("HL03", UI, r"if \(ctl == C_ICON && r->open\) \{", "if (ctl == C_ICON && r->open && false) {", "icon click on an open page re-opens instead of closing")
m("HL04", UI, r"r->oneBack = r->win \|\| r->fillOk;", "r->oneBack = r->win;", "one-back flag ignores the fill result")
m("HL05", UI, r"lroundf\(side \* kGearInset\)", "lroundf(side * 0.f)", "gear inset 0 (C_GEAR == C_ICON rect)")

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
    cp("tests/include", None)   # check.h for the colocated r10_filter_test.cpp built by parity.py
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
    mutated = apply(mut, text)
    with open(tgt, "w", newline="") as fh: fh.write(mutated)
    twin = os.path.join(d, "research/companion_levelup/tools/parity/stub/_shared", os.path.basename(mut[1]))   # build_base copied the stub's symlinked headers as plain files
    if mut[1].startswith("../_shared_src/") and os.path.isfile(twin) and not os.path.islink(twin):
        with open(twin, "w", newline="") as fh: fh.write(mutated)
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
