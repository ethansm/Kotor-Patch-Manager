// hotbar_log_test.cpp -- the hotbar's multi-field log formatters (HotbarLogic.h fmt*), host-only, no game.
// Pins (1) the exact text each formatter emits (byte-identical to the pre-extraction KP_LOG calls), (2) the core's 232-byte line limit
// with worst-case arguments (map 07b section 3), and (3) that each line survives KpLog.h's Write() intact (no '\n' split, no 511 cut).
#include "HotbarLogic.h"
#include "KpLog.h"
#include "check.h"
#include <climits>
#include <cstring>
#include <string>
#include <vector>
using namespace hb;

KP_LOG_PATCH("action-hotbar");

namespace {
struct Got { std::string chan; int lvl; std::string text; };
std::vector<Got> g_got;
std::vector<std::string> g_chanNames;
int32_t KPATCH_CALL fReg(const char*) { return 7; }
int32_t KPATCH_CALL fChan(int32_t, const char* n) { g_chanNames.push_back(n); return (int32_t)g_chanNames.size() - 1; }
int32_t KPATCH_CALL fEnabled(int32_t, int32_t) { return 1; }
void KPATCH_CALL fSubmit(int32_t c, int32_t l, const char* t, uint32_t n) { g_got.push_back({g_chanNames[c], l, std::string(t, n)}); }
void KPATCH_CALL fMark(const char*) {}
void KPATCH_CALL fTick(int32_t) {}
uint32_t KPATCH_CALL fFrame() { return 0; }
uint64_t KPATCH_CALL fNow() { return 0; }
KPatchLogApi g_log;
const void* KPATCH_CALL fRequire(const char*, uint32_t) { return &g_log; }

const std::string P260(260, 'p'), N80(80, 'n'), R100(100, 'r'), K64(64, 'k');
const char* const WHY24 = "restoreFromSave-24chars!";   // 24 bytes, the longest `why` the call sites pass

// One tagged line: starts with `tag`, <= 232 bytes (core cut), no '\n'.
void LineOk(const char* what, const std::string& s, const char* tag) {
  std::string w = std::string(what);
  KT_CHECK(!w.empty());
  kt::Check((w + ": starts with tag").c_str(), s.compare(0, strlen(tag), tag) == 0);
  kt::Check((w + ": <= 232 bytes").c_str(), s.size() <= LogMax, std::to_string(s.size()).c_str());
  kt::Check((w + ": no newline").c_str(), s.find('\n') == std::string::npos);
}
}  // namespace

int main() {
  // ---- exact text (what the old KP_LOG format strings produced)
  KT_CHECK_EQ(fmtSlotResolve("Bottom1", 2, 3, 0x20000008u, 0x1u, true, "spell:8").c_str(),
              "slot Bottom1: group=2 index=3 key=20000008 flags=00000001 usable=1 INVOKING ref=spell:8");
  KT_CHECK_EQ(fmtSlotResolve("Bottom2", 0, 1, 0x40000001u, 0x2u, false, "x").c_str(),
              "slot Bottom2: group=0 index=1 key=40000001 flags=00000002 usable=0 (Invoke=0) ref=x");
  KT_CHECK_EQ(fmtSlotMiss("Bottom3", "item:Medpac", "obj0000abcd").c_str(),
              "slot Bottom3: entry not available for 'obj0000abcd' ref=item:Medpac");
  {
    HeartbeatInfo h; h.frame = 1800; h.gate = 1; h.charKey = "obj00001234"; h.ring = 3; h.invoke = 1; h.cfgMtime = 1234567890123LL;
    h.panel = 0xDEADBEEFu; h.built = 10; for (int i = 0; i < 10; ++i) h.vis[i] = i % 3 - 1;
    h.widgetFaults = 1; h.probeFaults = 2; h.exoFaults = 3; h.iconFaults = 4; h.gateFlaps = 5;
    KT_CHECK_EQ(fmtHeartbeat(h).c_str(), "heartbeat: frame=1800 gate=1 char='obj00001234' ring=3 invoke=1 cfgMtime=1234567890123");
    KT_CHECK_EQ(fmtHeartbeatBar(h).c_str(), "heartbeat: bar panel=deadbeef built=10 vis=?01?01?01? wfaults=1 pfaults=2 efaults=3 ifaults=4 gateflaps=5");
  }
  { int v[4] = {-1, 0, 1, 7}; KT_CHECK_EQ(fmtVisMask(v, 4).c_str(), "?011"); KT_CHECK_EQ(fmtVisMask(v, 0).c_str(), ""); KT_CHECK_EQ(fmtVisMask(nullptr, 3).c_str(), ""); }
  {
    H3Summary s; s.created = 20; s.total = 21; s.first = 100; s.last = 119; s.buttons = 10; s.buttonCount = 10; s.editorBound = 11; s.editorCount = 11; s.editorEnabled = true; s.frame = 5;
    s.rect[0] = 1; s.rect[1] = 2; s.rect[2] = 3; s.rect[3] = 4;
    KT_CHECK_EQ(fmtH3Summary(s).c_str(), "H3: 20/21 controls created (indices 100..119) buttons=10/10 editor=11/11 -> editor ENABLED frame=5");
    s.editorEnabled = false;
    KT_CHECK_EQ(fmtH3Summary(s).c_str(), "H3: 20/21 controls created (indices 100..119) buttons=10/10 editor=11/11 -> editor DISABLED frame=5");
    KT_CHECK_EQ(fmtH3Rect(s).c_str(), "H3: picker background rect [1,2,3,4]");
  }
  KT_CHECK_EQ(fmtPersaveSnapshot("save", true, 2).c_str(), "persave: save: save ok profiles=2");
  KT_CHECK_EQ(fmtPersaveSnapshot("save", false, 0).c_str(), "persave: save: save FAILED profiles=0");
  KT_CHECK_EQ(fmtPersaveDst("save", "a/b.ini").c_str(), "persave: save: dst=a/b.ini");
  KT_CHECK_EQ(fmtPersaveReplace("load", true, 3, 1, true).c_str(), "persave: load: live hotbar replaced profiles=3 snapshot_warnings=1 backup=ok");
  KT_CHECK_EQ(fmtPersaveReplace("load", false, 0, 0, false).c_str(), "persave: load: live hotbar NOT replaced (save failed) profiles=0 snapshot_warnings=0 backup=none");
  KT_CHECK_EQ(fmtPersaveSource("load", "found", "x/y.ini").c_str(), "persave: load: found: source=x/y.ini");
  { std::string l = fmtPersaveSource(WHY24, "UNREADABLE (live hotbar kept)", P260);   // status survives the path clip
    LineOk("source 300", l, "persave: "); KT_CHECK(l.find("UNREADABLE (live hotbar kept)") != std::string::npos);
    std::string l3 = fmtPersaveSource(WHY24, "found", std::string(300, 'q')); LineOk("source 300q", l3, "persave: "); KT_CHECK(l3.find(": found: source=") != std::string::npos); }
  KT_CHECK_EQ(fmtAssign(4, "Bottom5", true, 0, 99LL, "obj1").c_str(), "edit: assign slot=4 key=Bottom5 save=ok froze=0 mtime=99 char='obj1'");
  KT_CHECK_EQ(fmtAssign(4, "Bottom5", false, 1, -2LL, "obj1").c_str(), "edit: assign slot=4 key=Bottom5 save=FAILED froze=1 mtime=-2 char='obj1'");
  KT_CHECK_EQ(fmtAssignValue(true, "spell:8").c_str(), "edit: assign value=spell:8");
  KT_CHECK_EQ(fmtAssignValue(false, "ignored").c_str(), "edit: assign value=none");
  {
    std::vector<std::string> t = fmtTipLines("Force Heal\nFP: 10\n\nlast");
    KT_CHECK_EQ(t.size(), 4);
    if (t.size() == 4) {
      KT_CHECK_EQ(t[0].c_str(), "tip: text[0] \"Force Heal\"");
      KT_CHECK_EQ(t[1].c_str(), "tip: text[1] \"FP: 10\"");
      KT_CHECK_EQ(t[2].c_str(), "tip: text[2] \"\"");
      KT_CHECK_EQ(t[3].c_str(), "tip: text[3] \"last\"");
    }
    KT_CHECK_EQ(fmtTipLines("one").size(), 1);
    std::vector<std::string> e = fmtTipLines("");   // the old loop emitted one empty line for an empty text
    KT_CHECK_EQ(e.size(), 1);
    if (e.size() == 1) KT_CHECK_EQ(e[0].c_str(), "tip: text[0] \"\"");
    std::vector<std::string> tr = fmtTipLines("a\n");   // trailing '\n' -> a final empty piece, as before
    KT_CHECK_EQ(tr.size(), 2);
    std::vector<std::string> cl = fmtTipLines(std::string(250, 'x'));   // %.200s clip
    KT_CHECK_EQ(cl.size(), 1);
    if (cl.size() == 1) KT_CHECK_EQ(cl[0].size(), strlen("tip: text[0] \"\"") + 200);
  }
  {
    std::vector<std::string> c = fmtChunks("cwd", std::string(450, 'a'));
    KT_CHECK_EQ(c.size(), 3);
    if (c.size() == 3) { KT_CHECK_EQ(c[0].size(), 7 + 200); KT_CHECK_EQ(c[2].size(), 7 + 50); KT_CHECK(c[1].compare(0, 7, "cwd[1]=") == 0); }
    std::vector<std::string> e = fmtChunks("cwd", "");
    KT_CHECK_EQ(e.size(), 1);
    if (e.size() == 1) KT_CHECK_EQ(e[0].c_str(), "cwd[0]=");
    KT_CHECK_EQ(fmtChunks("t", "abcd", 2).size(), 2);          // generalised chunk size
    KT_CHECK_EQ(fmtChunks("t", "abcd", 2)[1].c_str(), "t[1]=cd");
    KT_CHECK_EQ(fmtChunks("t", "abcd", 0).size(), 1);          // chunk 0 must not loop forever
    KT_CHECK_EQ(fmtChunks("t", std::string(10, 'z'), 4)[2].c_str(), "t[2]=zz");   // remainder chunk
    KT_CHECK_EQ(fmtChunks("t", "x", 5)[0].c_str(), "t[0]=x");
  }

  // ---- worst-case lengths: 260-char path, 80-char names, 100-char Format refs, 64-char keys, extreme ints
  {
    std::vector<std::pair<std::string, const char*>> all;   // (line, tag)
    all.push_back({fmtSlotResolve("Bottom10", INT_MIN, INT_MIN, 0xFFFFFFFFu, 0xFFFFFFFFu, true, R100), "slot "});
    all.push_back({fmtSlotResolve("Bottom10", INT_MAX, INT_MAX, 0xFFFFFFFFu, 0xFFFFFFFFu, false, R100), "slot "});
    all.push_back({fmtSlotMiss("Bottom10", R100, K64), "slot "});
    HeartbeatInfo h; h.frame = LONG_MIN; h.gate = INT_MIN; h.charKey = K64; h.ring = INT_MIN; h.invoke = INT_MIN; h.cfgMtime = LLONG_MIN;
    h.panel = 0xFFFFFFFFu; h.built = INT_MIN; for (int i = 0; i < 10; ++i) h.vis[i] = INT_MIN;
    h.widgetFaults = h.probeFaults = h.exoFaults = h.iconFaults = h.gateFlaps = 0xFFFFFFFFu;
    all.push_back({fmtHeartbeat(h), "heartbeat: "});
    all.push_back({fmtHeartbeatBar(h), "heartbeat: "});
    h.frame = LONG_MAX; h.gate = INT_MAX; h.cfgMtime = LLONG_MAX; h.built = INT_MAX;
    all.push_back({fmtHeartbeat(h), "heartbeat: "});
    all.push_back({fmtHeartbeatBar(h), "heartbeat: "});
    H3Summary s; s.created = s.total = s.first = INT_MIN; s.last = INT_MIN; s.buttons = s.buttonCount = s.editorBound = s.editorCount = INT_MIN; s.frame = LONG_MIN;
    for (int& r : s.rect) r = INT_MIN;
    s.editorEnabled = false;
    all.push_back({fmtH3Summary(s), "H3: "}); all.push_back({fmtH3Rect(s), "H3: "});
    s.created = s.total = s.first = s.last = s.buttons = s.buttonCount = s.editorBound = s.editorCount = INT_MAX; s.frame = LONG_MAX;
    for (int& r : s.rect) r = INT_MAX;
    s.editorEnabled = true;
    all.push_back({fmtH3Summary(s), "H3: "}); all.push_back({fmtH3Rect(s), "H3: "});
    all.push_back({fmtPersaveSnapshot(WHY24, false, INT_MIN), "persave: "});
    all.push_back({fmtPersaveSnapshot(WHY24, true, INT_MAX), "persave: "});
    all.push_back({fmtPersaveDst(WHY24, P260), "persave: "});
    all.push_back({fmtPersaveReplace(WHY24, false, INT_MIN, INT_MIN, false), "persave: "});
    all.push_back({fmtPersaveReplace(WHY24, true, INT_MAX, INT_MAX, true), "persave: "});
    all.push_back({fmtPersaveSource(WHY24, snapStatus(SS_Legacy, true), P260), "persave: "});
    all.push_back({fmtAssign(INT_MIN, "Bottom10", false, INT_MIN, LLONG_MIN, K64), "edit: assign "});
    all.push_back({fmtAssign(INT_MAX, "Bottom10", true, INT_MAX, LLONG_MAX, K64), "edit: assign "});
    all.push_back({fmtAssignValue(true, R100), "edit: assign "});
    for (const auto& l : all) LineOk(("formatter " + l.first.substr(0, 40)).c_str(), l.first, l.second);

    std::string tipText; for (int i = 0; i < 10; ++i) tipText += (i ? "\n" : "") + std::string(80, 'T');
    for (const std::string& l : fmtTipLines(tipText)) LineOk("tip line", l, "tip: ");
    for (const std::string& l : fmtTipLines(std::string(4000, 'T') + "\n" + std::string(4000, 'U'))) LineOk("tip long", l, "tip: ");
    for (const std::string& l : fmtChunks("cwd", P260)) LineOk("chunk cwd", l, "cwd[");
    for (const std::string& l : fmtChunks("H3: missing controls", std::string(2000, 'm'))) LineOk("chunk h3", l, "H3: missing controls[");
    // 10 control names of up to 24 bytes, space separated (the shape the hotbar builds)
    std::string names; for (int i = 0; i < 10; ++i) names += (i ? " " : "") + std::string(24, 'c');
    for (const std::string& l : fmtChunks("H3: missing controls", names)) LineOk("chunk names", l, "H3: missing controls[");
    // a 64-char char key inside a 10-member list in a chunk
    std::string members; for (int i = 0; i < 10; ++i) members += K64 + ",";
    for (const std::string& l : fmtChunks("members", members)) LineOk("chunk members", l, "members[");
  }

  // ---- end to end through KpLog.h: every formatter line reaches the core as exactly one intact line
  {
    std::memset(&g_log, 0, sizeof g_log);
    g_log.struct_size = sizeof g_log; g_log.Register = fReg; g_log.Channel = fChan; g_log.Enabled = fEnabled; g_log.Submit = fSubmit;
    g_log.Mark = fMark; g_log.FrameTick = fTick; g_log.Frame = fFrame; g_log.NowUs = fNow;
    KPatchApi api; std::memset(&api, 0, sizeof api); api.struct_size = sizeof api; api.abi_version = KPATCH_ABI_VERSION; api.require = fRequire;
    KT_CHECK_EQ(KpLog_Init(&api), 1);
    KP_CHANNEL(ch, "main");
    std::vector<std::string> lines;
    lines.push_back(fmtSlotMiss("Bottom10", R100, K64));
    lines.push_back(fmtPersaveSource(WHY24, snapStatus(SS_Absent, false), P260));
    lines.push_back(fmtAssignValue(true, R100));
    for (auto& l : fmtTipLines("a\nb\n\nc")) lines.push_back(l);
    for (auto& l : fmtChunks("cwd", P260)) lines.push_back(l);
    g_got.clear();
    for (const std::string& l : lines) KP_LOGV(ch, KPLOG_WARN, "%s", l.c_str());
    KT_CHECK_EQ(g_got.size(), lines.size());
    for (size_t i = 0; i < g_got.size() && i < lines.size(); ++i) {
      KT_CHECK(g_got[i].text == lines[i]);
      KT_CHECK(g_got[i].lvl == KPLOG_WARN && g_got[i].chan == "main");
    }
  }
  return kt::Report("hotbar_log_test");
}
