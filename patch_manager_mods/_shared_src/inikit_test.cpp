// Host test of IniKit.h: trim / parseFloats / splitIniLine (ported from equip3d_logic_test.cpp), forEachLine stats and
// line numbers, and the enabledFlag / enabledFile kill-switch rule (incl. its prefix-match quirk).
// Build/run: python3 scripts/run_tests.py -k inikit
#include "IniKit.h"
#include "check.h"
#include <cmath>
#include <string>
#include <vector>
#include <unistd.h>

using namespace inikit;

static bool approx(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }

static void testTrim() {
    auto t = [](const char* in) { char b[64]; snprintf(b, sizeof b, "%s", in); trim(b); return std::string(b); };
    KT_CHECK_EQ(t("").c_str(), "");
    KT_CHECK_EQ(t("   ").c_str(), "");
    KT_CHECK_EQ(t("\t\r\n ").c_str(), "");
    KT_CHECK_EQ(t("a").c_str(), "a");
    KT_CHECK_EQ(t("  a").c_str(), "a");
    KT_CHECK_EQ(t("a  ").c_str(), "a");
    KT_CHECK_EQ(t("  a b  ").c_str(), "a b");
    KT_CHECK_EQ(t("\ta\t\r\n").c_str(), "a");
    KT_CHECK_EQ(t("\xff x \xff").c_str(), "\xff x \xff");   // high bytes are not whitespace (unsigned char cast)
    char b[8] = "  ab  ";
    trim(b); KT_CHECK(b[0] == 'a' && b[1] == 'b' && b[2] == 0);
}

static void testParseFloats() {
    float o[4] = {};
    KT_CHECK(parseFloats("1.5", o, 1)); KT_CHECK(approx(o[0], 1.5f));
    KT_CHECK(parseFloats("  -2.25  ", o, 1)); KT_CHECK(approx(o[0], -2.25f));
    KT_CHECK(parseFloats("+3", o, 1)); KT_CHECK(approx(o[0], 3.f));
    KT_CHECK(parseFloats(".5", o, 1)); KT_CHECK(approx(o[0], 0.5f));
    KT_CHECK(parseFloats("5.", o, 1)); KT_CHECK(approx(o[0], 5.f));
    KT_CHECK(parseFloats("1e2", o, 1)); KT_CHECK(approx(o[0], 100.f));
    KT_CHECK(parseFloats("0x10", o, 1)); KT_CHECK(approx(o[0], 16.f));   // strtof accepts C99 hex floats
    KT_CHECK(!parseFloats("", o, 1));
    KT_CHECK(!parseFloats("   ", o, 1));
    KT_CHECK(!parseFloats("abc", o, 1));
    KT_CHECK(!parseFloats("1.5x", o, 1));
    KT_CHECK(!parseFloats("1 2", o, 1));
    KT_CHECK(!parseFloats("1,", o, 1));
    KT_CHECK(!parseFloats("1,2", o, 1));
    KT_CHECK(!parseFloats("nan", o, 1));
    KT_CHECK(!parseFloats("inf", o, 1));
    KT_CHECK(!parseFloats("-inf", o, 1));
    KT_CHECK(!parseFloats("infinity", o, 1));
    KT_CHECK(!parseFloats("1e999", o, 1));
    KT_CHECK(!parseFloats("1,nan", o, 2));
    KT_CHECK(!parseFloats("nan,1", o, 2));
    KT_CHECK(!parseFloats("1,inf", o, 2));
    KT_CHECK(parseFloats("1,2", o, 2)); KT_CHECK(approx(o[0], 1.f) && approx(o[1], 2.f));
    KT_CHECK(parseFloats(" 1 , 2 ", o, 2)); KT_CHECK(approx(o[0], 1.f) && approx(o[1], 2.f));
    KT_CHECK(parseFloats("-1.5,2.5e1", o, 2)); KT_CHECK(approx(o[0], -1.5f) && approx(o[1], 25.f));
    KT_CHECK(!parseFloats("1", o, 2));
    KT_CHECK(!parseFloats("1 2", o, 2));                // space is not a separator
    KT_CHECK(!parseFloats("1;2", o, 2));
    KT_CHECK(!parseFloats("1,", o, 2));
    KT_CHECK(!parseFloats("1,2,3", o, 2));
    KT_CHECK(!parseFloats(",2", o, 2));
    KT_CHECK(!parseFloats("1,2x", o, 2));
    KT_CHECK(parseFloats("1,2,3", o, 3)); KT_CHECK(approx(o[2], 3.f));
    KT_CHECK(parseFloats("1,2,3,4", o, 4)); KT_CHECK(approx(o[3], 4.f));
    KT_CHECK(!parseFloats("1,2", o, 3));
    KT_CHECK(parseFloats("", o, 0)); KT_CHECK(parseFloats("  ", o, 0)); KT_CHECK(!parseFloats("0", o, 0));   // n=0: blank only
    // a failed parse of a later element must not be reported as success even though earlier ones parsed
    KT_CHECK(!parseFloats("1,2,x", o, 3));
}

static void testSplitIniLine() {
    auto sp = [](const char* in, std::string* k, std::string* v) {
        char line[600]; snprintf(line, sizeof line, "%s", in);
        char *kk = nullptr, *vv = nullptr;
        IniLine r = splitIniLine(line, &kk, &vv);
        if (r == IniLineKeyValue) { *k = kk; *v = vv; }
        return r;
    };
    std::string k, v;
    KT_CHECK_EQ(sp("", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("# comment", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("; comment", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("#a=b", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp(";a=b", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("novalue", &k, &v), IniLineNoEquals);
    KT_CHECK_EQ(sp("[Section]", &k, &v), IniLineNoEquals);
    KT_CHECK_EQ(sp("stage=3", &k, &v), IniLineKeyValue);
    KT_CHECK_EQ(k.c_str(), "stage"); KT_CHECK_EQ(v.c_str(), "3");
    sp("  Stage  =  3  ", &k, &v);
    KT_CHECK_EQ(k.c_str(), "stage"); KT_CHECK_EQ(v.c_str(), "3");
    sp("STAGE=3", &k, &v); KT_CHECK_EQ(k.c_str(), "stage");
    sp("MiXeD=3", &k, &v); KT_CHECK_EQ(k.c_str(), "mixed");
    sp("rig=Charrec_Light", &k, &v); KT_CHECK_EQ(v.c_str(), "Charrec_Light");   // value case preserved
    sp("stage=3 ; five", &k, &v); KT_CHECK_EQ(v.c_str(), "3");
    sp("stage=3 # five", &k, &v); KT_CHECK_EQ(v.c_str(), "3");
    sp("rig=a#b", &k, &v); KT_CHECK_EQ(v.c_str(), "a");
    sp("rig=a;b#c", &k, &v); KT_CHECK_EQ(v.c_str(), "a");
    sp("rig=a#b;c", &k, &v); KT_CHECK_EQ(v.c_str(), "a");
    sp("stage=;3", &k, &v); KT_CHECK_EQ(v.c_str(), "");
    sp("a=b=c", &k, &v); KT_CHECK_EQ(k.c_str(), "a"); KT_CHECK_EQ(v.c_str(), "b=c");
    sp("=v", &k, &v); KT_CHECK_EQ(k.c_str(), ""); KT_CHECK_EQ(v.c_str(), "v");
    sp("=", &k, &v); KT_CHECK_EQ(k.c_str(), ""); KT_CHECK_EQ(v.c_str(), "");
    char line[] = "no equals here"; char *kk = nullptr, *vv = nullptr;
    splitIniLine(line, &kk, &vv); KT_CHECK_EQ(line, "no equals here");   // rejected line untouched
    KT_CHECK(kk == nullptr && vv == nullptr);
    sp("a;b=c", &k, &v); KT_CHECK_EQ(k.c_str(), "a;b"); KT_CHECK_EQ(v.c_str(), "c");   // ';' only comments at column 0
    sp("a#b=c", &k, &v); KT_CHECK_EQ(k.c_str(), "a#b");
}

struct Seen { int line; std::string k, v; };

static void testForEachLine() {
    std::string text =
        "; header comment\r\n"          // 1 comment
        "\r\n"                          // 2 blank
        "[Section]\r\n"                 // 3 noEquals
        "Stage = 3\r\n"                 // 4 ok
        "bogus_key=1 ; why\r\n"         // 5 unknown
        "fov=abc\r\n"                   // 6 bad
        "no equals here\r\n"            // 7 noEquals
        "# another\r\n"                 // 8 comment
        "  rig =  Foo#Bar \r\n"         // 9 ok, value cut at '#'
        "rig2=\r\n"                     // 10 ok, empty value
        "last=7";                       // 11 no trailing newline
    std::vector<char> buf(text.begin(), text.end()); buf.push_back(0);
    std::vector<Seen> seen;
    LoadStats st = forEachLine(buf.data(), [&](int ln, const char* k, const char* v) {
        seen.push_back({ln, k, v});
        if (!strcmp(k, "bogus_key")) return IniVerdictUnknown;
        if (!strcmp(k, "fov")) return IniVerdictBad;
        return IniVerdictOk;
    });
    KT_CHECK_EQ(st.ok, 4);          // stage, rig, rig2, last
    KT_CHECK_EQ(st.bad, 1);
    KT_CHECK_EQ(st.unknown, 1);
    KT_CHECK_EQ(st.noEquals, 2);    // "[Section]" counts, as in the old loadIni loops
    KT_CHECK_EQ((int)seen.size(), 6);
    if (seen.size() == 6) {
        KT_CHECK_EQ(seen[0].line, 4);  KT_CHECK_EQ(seen[0].k.c_str(), "stage");     KT_CHECK_EQ(seen[0].v.c_str(), "3");
        KT_CHECK_EQ(seen[1].line, 5);  KT_CHECK_EQ(seen[1].k.c_str(), "bogus_key"); KT_CHECK_EQ(seen[1].v.c_str(), "1");
        KT_CHECK_EQ(seen[2].line, 6);  KT_CHECK_EQ(seen[2].k.c_str(), "fov");       KT_CHECK_EQ(seen[2].v.c_str(), "abc");
        KT_CHECK_EQ(seen[3].line, 9);  KT_CHECK_EQ(seen[3].k.c_str(), "rig");       KT_CHECK_EQ(seen[3].v.c_str(), "Foo");
        KT_CHECK_EQ(seen[4].line, 10); KT_CHECK_EQ(seen[4].k.c_str(), "rig2");      KT_CHECK_EQ(seen[4].v.c_str(), "");
        KT_CHECK_EQ(seen[5].line, 11); KT_CHECK_EQ(seen[5].k.c_str(), "last");      KT_CHECK_EQ(seen[5].v.c_str(), "7");
    }

    // LF-only endings, trailing newline, and the line number counts blank lines
    char lf[] = "\n\na=1\nb=2\n";
    std::vector<int> lns;
    st = forEachLine(lf, [&](int ln, const char*, const char*) { lns.push_back(ln); return IniVerdictOk; });
    KT_CHECK_EQ(st.ok, 2); KT_CHECK_EQ((int)lns.size(), 2);
    if (lns.size() == 2) { KT_CHECK_EQ(lns[0], 3); KT_CHECK_EQ(lns[1], 4); }

    // empty / null / blank-only text: no callbacks, all zero
    int calls = 0;
    auto count = [&](int, const char*, const char*) { ++calls; return IniVerdictOk; };
    char e[] = "";
    st = forEachLine(e, count);
    KT_CHECK_EQ(st.ok + st.bad + st.unknown + st.noEquals, 0);
    st = forEachLine(nullptr, count);
    KT_CHECK_EQ(st.ok + st.bad + st.unknown + st.noEquals, 0);
    char blanks[] = "\n\r\n  \n# c\n; d\n";
    st = forEachLine(blanks, count);
    KT_CHECK_EQ(st.ok + st.bad + st.unknown + st.noEquals, 0);
    KT_CHECK_EQ(calls, 0);

    // a lone final line without '=' and without newline
    char lone[] = "oops";
    st = forEachLine(lone, count);
    KT_CHECK_EQ(st.noEquals, 1); KT_CHECK_EQ(calls, 0);

    // every verdict is counted into exactly its own field
    char verd[] = "a=1\nb=2\nb=2\nc=3\nc=3\nc=3\n";
    st = forEachLine(verd, [](int, const char* k, const char*) {
        return k[0] == 'a' ? IniVerdictOk : k[0] == 'b' ? IniVerdictBad : IniVerdictUnknown; });
    KT_CHECK_EQ(st.ok, 1); KT_CHECK_EQ(st.bad, 2); KT_CHECK_EQ(st.unknown, 3); KT_CHECK_EQ(st.noEquals, 0);

    // whole-buffer behaviour: a line longer than the old fgets buffers (600) is handled as ONE line
    std::string big = "key=" + std::string(2000, 'x') + "\nnext=1\n";
    std::vector<char> bb(big.begin(), big.end()); bb.push_back(0);
    std::vector<Seen> bs;
    forEachLine(bb.data(), [&](int ln, const char* k, const char* v) { bs.push_back({ln, k, v}); return IniVerdictOk; });
    KT_CHECK_EQ((int)bs.size(), 2);
    if (bs.size() == 2) { KT_CHECK_EQ((int)bs[0].v.size(), 2000); KT_CHECK_EQ(bs[1].line, 2); }
}

static void testEnabledFlag() {
    KT_CHECK(enabledFlag(nullptr));
    KT_CHECK(enabledFlag(""));
    KT_CHECK(enabledFlag("other=0\n"));                  // key absent = on
    KT_CHECK(enabledFlag("enabled=1\n"));
    KT_CHECK(enabledFlag("enabled=1"));                  // no trailing newline
    KT_CHECK(!enabledFlag("enabled=0\n"));
    KT_CHECK(!enabledFlag("enabled=0"));
    KT_CHECK(!enabledFlag("enabled = 0\n"));
    KT_CHECK(!enabledFlag("enabled = 00\n"));
    KT_CHECK(!enabledFlag("enabled=0.5\n"));             // first character only
    KT_CHECK(!enabledFlag("enabled==0\n"));              // any run of ' ' and '='
    KT_CHECK(enabledFlag("enabled \t=0\n"));   // a TAB after the key is not skipped: value char is '\t' != '0' -> on
    KT_CHECK(enabledFlag("enabled=10\n"));
    KT_CHECK(enabledFlag("enabled=off\n"));              // CHARACTERIZATION: only a leading '0' turns it off
    KT_CHECK(enabledFlag("enabled=\n"));                 // empty value = on
    KT_CHECK(enabledFlag("enabled\n"));
    KT_CHECK(enabledFlag("enabled"));
    KT_CHECK(!enabledFlag("enabled=0\r\n"));             // CRLF
    KT_CHECK(enabledFlag("enabled=1\r\n"));
    // leading space / tab before the key
    KT_CHECK(!enabledFlag("  enabled=0\n"));
    KT_CHECK(!enabledFlag("\tenabled=0\n"));
    KT_CHECK(!enabledFlag(" \t \tenabled=0\n"));
    KT_CHECK(enabledFlag("x\n\tenabled=1\n"));
    KT_CHECK(!enabledFlag("x=1\n\tenabled=0\n"));        // match on a later line
    // comments are not understood: "; enabled=0" does not start with the key, so it never matches
    KT_CHECK(enabledFlag("; enabled=0\n"));
    KT_CHECK(enabledFlag("# enabled=0\n"));
    // QUIRK (inherited from the three original configEnabled copies): prefix match only, so "enabledX=0" and
    // "enabled_foo=0" DO match key "enabled". The skip loop only eats ' ' and '=', so the value char is the 'X' / '_' itself
    // (!= '0'): such a line turns the flag ON, and being a later match it overrides an earlier "enabled=0".
    KT_CHECK(enabledFlag("enabledX=0\n"));
    KT_CHECK(enabledFlag("enabled_x = 0\n"));
    KT_CHECK(enabledFlag("enabled=0\nenabledX=0\n"));    // the quirk is observable: the prefix line won (last wins)
    KT_CHECK(!enabledFlag("enabledX=0\nenabled=0\n"));
    KT_CHECK(!enabledFlag("enabledX=0\n", "enabledX"));
        KT_CHECK(!enabledFlag("enabled=0 ; why\n"));
    KT_CHECK(enabledFlag("enabledXYZ=0\n"));
    KT_CHECK(enabledFlag("fooBar=0\n", "foo"));   // prefix applies to any key; 'B' is the value char -> on
    KT_CHECK(enabledFlag("foo=0\nfoo_=0\n", "foo"));
    KT_CHECK(enabledFlag("enable=0\n"));                 // shorter than the key: no match
    KT_CHECK(enabledFlag("Enabled=0\n"));                // case-sensitive
    // last match wins, in both directions
    KT_CHECK(!enabledFlag("enabled=1\nenabled=0\n"));
    KT_CHECK(enabledFlag("enabled=0\nenabled=1\n"));
    KT_CHECK(enabledFlag("enabled=0\nenabled=1\nother=0\n"));
    KT_CHECK(!enabledFlag("enabled=1\nenabled=0\nenabled=1\nenabled=0\n"));
    // custom key
    KT_CHECK(!enabledFlag("enabled=1\nfoo=0\n", "foo"));
    KT_CHECK(enabledFlag("enabled=0\nfoo=1\n", "foo"));
    KT_CHECK(enabledFlag("enabled=0\n", "foo"));
    KT_CHECK(enabledFlag("enabled=0\n", nullptr));       // no key: nothing to look for
    KT_CHECK(enabledFlag("enabled=0\n", ""));
    // a match in the middle of a line (not at line start after blanks) is not a match
    KT_CHECK(enabledFlag("x enabled=0\n"));
    // a match on the first line and no trailing data after the last newline
    KT_CHECK(!enabledFlag("enabled=0\n\n\n"));
    KT_CHECK(!enabledFlag("a=1\n\n\nenabled=0\n\n"));
}

static void testEnabledFile() {
    const char* t = getenv("TMPDIR");
    std::string path = std::string(t && *t ? t : "/tmp") + "/inikit_test_" + std::to_string((long)getpid()) + ".txt";
    remove(path.c_str());
    KT_CHECK(enabledFile(path.c_str()));                 // missing file = on
    KT_CHECK(enabledFile(path.c_str(), "foo"));
    KT_CHECK(enabledFile(nullptr));
    auto put = [&](const std::string& s) { FILE* f = fopen(path.c_str(), "wb"); if (f) { fwrite(s.data(), 1, s.size(), f); fclose(f); } };
    put("");
    KT_CHECK(enabledFile(path.c_str()));                 // empty file = on
    put("enabled=0\r\n");
    KT_CHECK(!enabledFile(path.c_str()));
    put("# hi\r\n  enabled = 1\r\nfoo=0\r\n");
    KT_CHECK(enabledFile(path.c_str()));
    KT_CHECK(!enabledFile(path.c_str(), "foo"));
    put("enabled=0\nenabled=1\n");
    KT_CHECK(enabledFile(path.c_str()));                 // last wins across the whole file
    put("enabled=1\nenabled=0");
    KT_CHECK(!enabledFile(path.c_str()));
    // lines longer than the old 128-byte fgets buffer no longer split into phantom lines
    put(std::string("x=") + std::string(300, 'a') + "\nenabled=0\n");
    KT_CHECK(!enabledFile(path.c_str()));
    put("pad=" + std::string(200, 'z') + "\nenabled=1\n");
    KT_CHECK(enabledFile(path.c_str()));
    // read cap: a key past 64 KB is not seen (file is "small config" by contract)
    put(std::string(70000, '\n') + "enabled=0\n");
    KT_CHECK(enabledFile(path.c_str()));
    put(std::string(65600, '\n') + "enabled=0\n");     // just past the cap (65536), inside the next 512-byte chunk
    KT_CHECK(enabledFile(path.c_str()));
    put(std::string(65000, '\n') + "enabled=0\n");
    KT_CHECK(!enabledFile(path.c_str()));
    // the final short chunk (here a single byte, the '0') is read too
    put(std::string(503, 'p') + "\nenabled=0");   // 513 bytes
    KT_CHECK(!enabledFile(path.c_str()));
    put("p=" + std::string(500, 'p') + "\nenabled=0");   // 513 bytes
    KT_CHECK(!enabledFile(path.c_str()));
    remove(path.c_str());
    KT_CHECK(enabledFile(path.c_str()));                 // gone again
}

int main() {
    testTrim();
    testParseFloats();
    testSplitIniLine();
    testForEachLine();
    testEnabledFlag();
    testEnabledFile();
    return kt::Report("inikit_test");
}
