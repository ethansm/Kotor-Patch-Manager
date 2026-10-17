// Host test of the voice-probe trigger logic (TriggerCore.h): config parsing and validation, edge + modifier + foreground gating,
// cooldown, and fire() against a fake Env (skip on no PC / OBJECT_INVALID, one log line per fire). No engine, no Win32.
#include "TriggerCore.h"
#include "check.h"
#include <string>
#include <vector>

using namespace vpt;

#define CHK(why, expr) ::kt::Check("voice-probe trigger: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define EQ(why, a, b) ::kt::CheckEq("voice-probe trigger: " why " (line " KT_STR(__LINE__) ")", (a), (b))

struct FakeEnv : Env {
    uint32_t oid = 0x00000123u;
    const char* failWhy = "party table null";
    bool vmReachable = true;
    int ret = 1;
    std::vector<std::string> lines, ran;
    std::vector<uint32_t> oids;
    uint32_t pcObjectId(const char** why) override { if (oid == 0 && why) *why = failWhy; return oid; }
    int runScript(const char* script, uint32_t o, bool* called) override {
        *called = vmReachable;
        if (!vmReachable) return 0;
        ran.push_back(script); oids.push_back(o);
        return ret;
    }
    void log(const char* line) override { lines.push_back(line); }
};

static Config on() { return makeConfig(0x77, 1, 0, 1000, "a_attspch1"); }

int main() {
    // ---- config ----
    CHK("default config is off", !Config().enabled());
    CHK("key 0 = off", !makeConfig(0, 1, 0, 1000, "a_attspch1").enabled());
    CHK("empty script = off", !makeConfig(0x77, 1, 0, 1000, "").enabled());
    CHK("null script = off", !makeConfig(0x77, 1, 0, 1000, nullptr).enabled());
    CHK("key 255 rejected", !makeConfig(255, 1, 0, 1000, "x").enabled());
    CHK("negative key rejected", !makeConfig(-3, 1, 0, 1000, "x").enabled());
    CHK("17-char resref rejected", !makeConfig(0x77, 1, 0, 1000, "abcdefghijklmnopq").enabled());
    CHK("16-char resref accepted", makeConfig(0x77, 1, 0, 1000, "abcdefghijklmnop").enabled());
    CHK("path separator rejected", !makeConfig(0x77, 1, 0, 1000, "..\\x").enabled());
    CHK("extension rejected (resref only)", !makeConfig(0x77, 1, 0, 1000, "kpv_e1.ncs").enabled());
    EQ("bad resref names its reason", makeConfig(0x77, 1, 0, 1000, "a b").why, "bad script resref");
    {
        Config c = on();
        CHK("valid config is on", c.enabled());
        EQ("script copied", c.script, "a_attspch1");
        EQ("cooldown kept", (unsigned long)c.cooldownMs, 1000ul);
        EQ("cooldown clamped low", (unsigned long)makeConfig(0x77, 1, 0, 0, "x").cooldownMs, (unsigned long)MinCooldownMs);
        EQ("cooldown clamped high", (unsigned long)makeConfig(0x77, 1, 0, 999999, "x").cooldownMs, (unsigned long)MaxCooldownMs);
        char line[MaxLine];
        configLine(c, line, sizeof line);
        EQ("cfg line", line, "TRIGGER cfg state=on key=77 ctrl=1 shift=0 cooldown_ms=1000 script=a_attspch1");
        configLine(Config(), line, sizeof line);
        EQ("cfg line off", line, "TRIGGER cfg state=off key=00 ctrl=1 shift=0 cooldown_ms=1000 script=-");
    }

    // ---- gating ----
    {
        Config c = on();
        Trigger t;
        auto st = [&](Trigger& tr, const Config& cf, bool fg, bool key, bool ctrl, bool shift, uint32_t now, bool ready = true) {
            Input in; in.areaReady = ready; in.foreground = fg; in.key = key; in.ctrl = ctrl; in.shift = shift;
            return tr.step(cf, in, now);
        };
        auto ran = [&](Trigger& tr, const Config& cf, uint32_t now) { Step s = st(tr, cf, true, true, true, false, now); if (s == Step::Fire) tr.markRan(now); st(tr, cf, true, false, true, false, now); return s; };
        CHK("no key = idle", st(t, c, true, false, true, false, 0) == Step::Idle);
        CHK("key without ctrl = idle", st(t, c, true, true, false, false, 10) == Step::Idle);
        CHK("release", st(t, c, true, false, false, false, 20) == Step::Idle);
        CHK("key in background = idle", st(t, c, false, true, true, false, 30) == Step::Idle);
        CHK("release 2", st(t, c, false, false, false, false, 40) == Step::Idle);
        CHK("ctrl+key in foreground fires", st(t, c, true, true, true, false, 50) == Step::Fire);
        t.markRan(50);
        CHK("held key does not repeat", st(t, c, true, true, true, false, 2000) == Step::Idle);
        CHK("release 3", st(t, c, true, false, true, false, 2010) == Step::Idle);
        CHK("second press after cooldown fires", st(t, c, true, true, true, false, 2020) == Step::Fire);
        t.markRan(2020);
        st(t, c, true, false, true, false, 2030);
        CHK("press inside cooldown = Cooldown", st(t, c, true, true, true, false, 2500) == Step::Cooldown);
        EQ("fires counted (cooldown presses excluded)", t.fires, 2L);
        Trigger off;
        CHK("disabled config never fires", st(off, Config(), true, true, true, true, 0) == Step::Idle);
        Config noCtrl = makeConfig(0x77, 0, 1, 1000, "x");
        Trigger s;
        CHK("shift required when set", st(s, noCtrl, true, true, false, false, 0) == Step::Idle);
        st(s, noCtrl, true, false, false, false, 1);
        CHK("shift+key fires without ctrl", st(s, noCtrl, true, true, false, true, 2) == Step::Fire);
        Trigger w;
        CHK("first fire at t=0 is not blocked by cooldown", st(w, c, true, true, true, false, 0) == Step::Fire);

        Trigger nr;
        CHK("press before AREA_STABLE = NotReady", st(nr, c, true, true, true, false, 0, false) == Step::NotReady);
        st(nr, c, true, false, true, false, 10, false);
        CHK("press after AREA_STABLE fires", st(nr, c, true, true, true, false, 20, true) == Step::Fire);

        Trigger skip;
        CHK("first press fires", ran(skip, c, 100) == Step::Fire);
        Trigger noPc;
        CHK("press 1 fires (skipped: no markRan)", st(noPc, c, true, true, true, false, 100) == Step::Fire);
        st(noPc, c, true, false, true, false, 110);
        CHK("a skipped press does not start the cooldown", st(noPc, c, true, true, true, false, 120) == Step::Fire);

        Trigger wrap; wrap.markRan(0xFFFFFFF0u);
        CHK("cooldown holds across the GetTickCount wrap", st(wrap, c, true, true, true, false, 0x10u) == Step::Cooldown);
        st(wrap, c, true, false, true, false, 0x20u);
        CHK("cooldown ends across the wrap", st(wrap, c, true, true, true, false, 0xFFFFFFF0u + 1000u) == Step::Fire);

        Trigger edge; edge.markRan(0);
        CHK("press at exactly cooldown_ms fires", st(edge, c, true, true, true, false, 1000) == Step::Fire);

        Trigger fg;
        CHK("held key fires once", st(fg, c, true, true, true, false, 0) == Step::Fire);
        fg.markRan(0);
        CHK("held key, focus lost = idle", st(fg, c, false, true, true, false, 5000) == Step::Idle);
        CHK("held key, focus back = a new edge (fires)", st(fg, c, true, true, true, false, 6000) == Step::Fire);
    }

    // ---- fire ----
    {
        Config c = on();
        FakeEnv e;
        CHK("fire calls RunScript", fire(e, c, 1, 50));
        EQ("script passed", e.ran.size() == 1 ? e.ran[0].c_str() : "", "a_attspch1");
        EQ("PC id passed", e.oids.size() == 1 ? (unsigned long)e.oids[0] : 0ul, 0x123ul);
        EQ("one line", e.lines.size(), (size_t)1);
        EQ("fire line", e.lines[0].c_str(), "TRIGGER #1 t=50 script=a_attspch1 oid=00000123 called=1 ret=1");

        FakeEnv none; none.oid = 0;
        CHK("no PC: no call", !fire(none, c, 2, 60) && none.ran.empty());
        EQ("no PC line names the reason", none.lines[0].c_str(), "TRIGGER #2 t=60 script=a_attspch1 skip=party table null");

        FakeEnv inv; inv.oid = ObjectInvalid;
        CHK("OBJECT_INVALID: no call", !fire(inv, c, 3, 70) && inv.ran.empty());

        FakeEnv novm; novm.vmReachable = false;
        CHK("no VM: reports not called", !fire(novm, c, 4, 80));
        EQ("no VM line", novm.lines[0].c_str(), "TRIGGER #4 t=80 script=a_attspch1 oid=00000123 called=0 ret=0");

        FakeEnv bad; bad.ret = 0;
        fire(bad, c, 5, 90);
        CHK("engine failure ret=0 is logged", bad.lines[0].find("called=1 ret=0") != std::string::npos);

        Config longest = makeConfig(0x77, 1, 0, 1000, "abcdefghijklmnop");
        FakeEnv big; big.oid = 0xFFFFFFFEu; big.ret = -2147483647 - 1;
        fire(big, longest, 2147483647L, 0xFFFFFFFFu);
        CHK("longest fire line < 232 B", big.lines[0].size() < 232 && big.lines[0].find("ret=-2147483648") != std::string::npos);
    }
    return kt::Report("trigger_core_test (voice-probe)");
}
