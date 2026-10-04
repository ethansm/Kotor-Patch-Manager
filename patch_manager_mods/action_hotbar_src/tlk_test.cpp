// tlk_test: hb::Tlk against testdata/fixture.tlk (made by testdata/make_tlk_fixture.py; cwd = this dir) plus
// malformed / edge-case files generated into a mkdtemp dir under $TMPDIR (nothing extra is committed).
// Behaviour pinned here:
//  - Open accepts count 1..9999999; Get returns "" for any invalid ref / out-of-range data (never throws).
//  - A failed read (file missing / truncated at Get time) is NOT cached, so a later Get can succeed once the
//    file is back. Successful reads, empty entries and size>=65536 (structurally invalid) ARE cached.
//  - strOff+offset is computed in 64 bits and must lie inside the file (no uint32 wrap-around).
#include "Tlk.h"
#include "check.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>

static std::string g_dir;
static int g_n = 0;

static void put32(std::string& s, uint32_t v) { for (int i = 0; i < 4; ++i) s += (char)((v >> (8 * i)) & 0xFF); }

struct Opt {
    uint32_t count = 0xFFFFFFFFu;     // header count override (default = entries.size())
    uint32_t strOff = 0xFFFFFFFFu;    // header strings offset override
    uint32_t addOff = 0;              // added to each entry's real offset
    uint32_t lang = 0;                // header language id (bytes 8..11)
    int32_t sizeDelta = 0;            // added to each entry's real size field
    const char* magic = "TLK V3.0";
    size_t truncateTo = (size_t)-1;   // cut the finished file to this many bytes
};

static std::string Build(const std::vector<std::string>& ents, const Opt& o = Opt()) {
    uint32_t n = (uint32_t)ents.size();
    uint32_t realStr = 20 + n * 40;
    std::string s(o.magic, 8);
    put32(s, o.lang);
    put32(s, o.count == 0xFFFFFFFFu ? n : o.count);
    put32(s, o.strOff == 0xFFFFFFFFu ? realStr : o.strOff);
    std::string blob;
    for (const std::string& e : ents) {
        put32(s, e.empty() ? 0 : 1); s.append(16, '\0'); put32(s, 0); put32(s, 0);
        put32(s, (uint32_t)blob.size() + o.addOff); put32(s, (uint32_t)((int64_t)e.size() + o.sizeDelta)); put32(s, 0);
        blob += e;
    }
    s += blob;
    if (o.truncateTo < s.size()) s.resize(o.truncateTo);
    return s;
}

static std::string Write(const std::string& bytes, const char* name = nullptr) {
    char nm[32]; snprintf(nm, sizeof nm, "t%d.tlk", ++g_n);
    std::string p = g_dir + "/" + (name ? name : nm);
    FILE* f = fopen(p.c_str(), "wb");
    if (f) { fwrite(bytes.data(), 1, bytes.size(), f); fclose(f); }
    return p;
}

// One-entry file -> Get(0).
static std::string One(const std::string& raw) {
    hb::Tlk t; std::string p = Write(Build({raw}));
    if (!t.Open(p.c_str())) return "<open failed>";
    return t.Get(0);
}

int main() {
    const char* tmp = getenv("TMPDIR");
    std::string tpl = std::string(tmp && *tmp ? tmp : "/tmp") + "/tlk_test.XXXXXX";
    std::vector<char> tb(tpl.begin(), tpl.end()); tb.push_back('\0');
    if (!mkdtemp(tb.data())) { puts("FAIL mkdtemp"); return 1; }
    g_dir = tb.data();

    // ---- committed fixture
    {
        hb::Tlk t;
        KT_CHECK_EQ(t.count, 0); KT_CHECK_EQ(t.strOff, 0); KT_CHECK(!t.ok);   // fresh state: nothing opened
        KT_CHECK(!t.Open("testdata/no_such.tlk")); KT_CHECK_EQ(t.count, 0); KT_CHECK_EQ(t.strOff, 0); KT_CHECK(!t.ok);   // a failed first Open keeps it
        KT_CHECK_EQ(t.Get(0).c_str(), "");                    // Get before Open
        KT_CHECK(t.Open("testdata/fixture.tlk"));
        KT_CHECK_EQ(t.count, 6);
        KT_CHECK_EQ(t.strOff, 20 + 6 * 40);
        KT_CHECK_EQ(t.Get(0).c_str(), "Hello, world.");
        KT_CHECK_EQ(t.Get(1).c_str(), "'quoted' \"double\" a-b-c");
        KT_CHECK_EQ(t.Get(2).c_str(), "line1 line2 col end");
        KT_CHECK_EQ(t.Get(3).c_str(), "");
        KT_CHECK_EQ(t.Get(4).c_str(), "padded text");
        KT_CHECK_EQ(t.Get(5).c_str(), "Force Speed");
        KT_CHECK_EQ(t.Get(0).c_str(), "Hello, world.");       // cached path returns the same
        KT_CHECK(t.cache.size() == 6);
        KT_CHECK_EQ(t.Get(6).c_str(), "");                    // ref == count
        KT_CHECK_EQ(t.Get(0xFFFFFFFFu).c_str(), "");
        KT_CHECK(t.cache.size() == 6);                        // out-of-range refs are not cached
        // Re-Open resets the cache and state.
        KT_CHECK(!t.Open("testdata/no_such.tlk"));
        KT_CHECK_EQ(t.error.c_str(), "cannot open");
        KT_CHECK(!t.ok);                                      // a failed re-Open drops the previous ok
        KT_CHECK_EQ(t.Get(0).c_str(), "");                    // !ok -> ""
        KT_CHECK(t.Open("testdata/fixture.tlk"));
        KT_CHECK_EQ(t.Get(4).c_str(), "padded text");
    }

    // ---- header validation
    {
        hb::Tlk t;
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.magic = "TLK V2.0"; return o; }())).c_str()));
        KT_CHECK_EQ(t.error.c_str(), "not TLK V3.0");
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.magic = "tlk v3.0"; return o; }())).c_str()));
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.magic = "TLK V3.1"; return o; }())).c_str()));   // all 8 magic bytes are compared
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.magic = "TLK V3.0"; o.count = 0x01000000u; return o; }())).c_str()));   // high count byte counts (16777216 >= limit)
        KT_CHECK(t.Open(Write(Build({"x"}, [] { Opt o; o.lang = 0x01020304u; return o; }())).c_str()));   // the language id (header bytes 8..11) is not part of the magic
        KT_CHECK_EQ(t.Get(0).c_str(), "x");
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.truncateTo = 19; return o; }())).c_str()));   // short header
        KT_CHECK_EQ(t.error.c_str(), "not TLK V3.0");
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.truncateTo = 0; return o; }())).c_str()));    // empty file
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.truncateTo = 8; return o; }())).c_str()));
        KT_CHECK(t.Open(Write(Build({"x"}, [] { Opt o; o.truncateTo = 20 + 40 + 1; return o; }())).c_str()));
        KT_CHECK(!t.Open(Write(Build({}, [] { Opt o; o.count = 0; return o; }())).c_str()));            // count 0
        KT_CHECK_EQ(t.error.c_str(), "implausible count");
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.count = 0; return o; }())).c_str()));
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.count = 10000000; return o; }())).c_str()));  // limit
        KT_CHECK(!t.Open(Write(Build({"x"}, [] { Opt o; o.count = 0xFFFFFFFFu - 1; return o; }())).c_str()));
        KT_CHECK(t.Open(Write(Build({"x"}, [] { Opt o; o.count = 9999999; return o; }())).c_str()));    // just under
        KT_CHECK_EQ(t.Get(0).c_str(), "x");                   // the one real entry still reads; the header count is just a bound
        KT_CHECK_EQ(t.Get(9999998).c_str(), "");              // far past EOF, no crash/overflow of (long)
    }

    // ---- ref >= count, data past EOF, string size limits
    {
        // Header claims 3 entries but only the first entry + blob exist.
        std::string full = Build({"abc", "def", "ghi"});
        hb::Tlk t; KT_CHECK(t.Open(Write(full).c_str()));
        KT_CHECK_EQ(t.Get(2).c_str(), "ghi");
        KT_CHECK_EQ(t.Get(3).c_str(), "");
        hb::Tlk t2; KT_CHECK(t2.Open(Write(Build({"abc"}, [] { Opt o; o.count = 3; return o; }())).c_str()));
        KT_CHECK_EQ(t2.Get(0).c_str(), "abc");                // header count lies (3 vs 1 entry); entry 0 still valid
        KT_CHECK_EQ(t2.Get(2).c_str(), "");                   // entry table past EOF
        // Entry data past EOF: size field larger than what the file holds.
        KT_CHECK_EQ(Build({"abc"}).size(), 20 + 40 + 3);
        hb::Tlk t3; KT_CHECK(t3.Open(Write(Build({"abc"}, [] { Opt o; o.sizeDelta = 1; return o; }())).c_str()));
        KT_CHECK_EQ(t3.Get(0).c_str(), "");
        hb::Tlk t4; KT_CHECK(t4.Open(Write(Build({"abcdef"}, [] { Opt o; o.truncateTo = 20 + 40 + 3; return o; }())).c_str()));
        KT_CHECK_EQ(t4.Get(0).c_str(), "");                   // blob cut mid-string
        hb::Tlk t5; KT_CHECK(t5.Open(Write(Build({"abc"}, [] { Opt o; o.addOff = 100; return o; }())).c_str()));
        KT_CHECK_EQ(t5.Get(0).c_str(), "");                   // offset beyond EOF
        hb::Tlk t6; KT_CHECK(t6.Open(Write(Build({"abc"}, [] { Opt o; o.strOff = 0x80000000u; return o; }())).c_str()));
        KT_CHECK_EQ(t6.Get(0).c_str(), "");                   // strOff beyond EOF (>= 2^31)
        hb::Tlk t7; KT_CHECK(t7.Open(Write(Build({"abc"}, [] { Opt o; o.strOff = 0xFFFFFFFFu - 1; return o; }())).c_str()));
        KT_CHECK_EQ(t7.Get(0).c_str(), "");
        // Size limit: 65535 ok, 65536 rejected (and does not allocate/crash), 0xFFFFFFFF rejected.
        KT_CHECK_EQ(One(std::string(65535, 'a')).size(), 65535);
        KT_CHECK_EQ(One(std::string(65536, 'a')).c_str(), "");
        hb::Tlk t8; KT_CHECK(t8.Open(Write(Build({"abc"}, [] { Opt o; o.sizeDelta = -4; return o; }())).c_str()));   // size = 0xFFFFFFFF
        KT_CHECK_EQ(t8.Get(0).c_str(), "");
    }

    // ---- data position exactly at the 2^31-1 fseek limit: an empty entry there is still a cached success
    {
        Opt o; o.strOff = 0x7FFFFFFFu;
        hb::Tlk t; KT_CHECK(t.Open(Write(Build({""}, o)).c_str()));
        KT_CHECK_EQ(t.Get(0).c_str(), "");
        KT_CHECK(t.cache.count(0) == 1);                      // size 0 at dataPos == 0x7FFFFFFF is within the limit (<=)
    }

    // ---- offset+size wrap: strOff + off wraps past 2^32 onto a valid-looking blob position
    {
        const uint32_t dataPos = 20 + 40;
        Opt o; o.strOff = 0xFFFFFFF0u; o.addOff = dataPos + 0x10;   // 0xFFFFFFF0 + dataPos + 0x10 == 2^32 + dataPos
        hb::Tlk t; KT_CHECK(t.Open(Write(Build({"WRAP"}, o)).c_str()));
        KT_CHECK_EQ(t.Get(0).c_str(), "");                    // must not read the wrapped position
        // Sanity: the same layout without the wrap reads fine.
        KT_CHECK_EQ(One("WRAP").c_str(), "WRAP");
    }

    // ---- failed reads are not cached: file gone at Get time, then restored
    {
        std::string bytes = Build({"first", "second"});
        std::string p = Write(bytes, "retry.tlk");
        hb::Tlk t; KT_CHECK(t.Open(p.c_str()));
        KT_CHECK_EQ(t.Get(0).c_str(), "first");
        std::string moved = p + ".away";
        KT_CHECK(rename(p.c_str(), moved.c_str()) == 0);
        KT_CHECK_EQ(t.Get(1).c_str(), "");                    // file missing: transient failure
        KT_CHECK_EQ(t.Get(0).c_str(), "first");               // already-cached success survives
        KT_CHECK(rename(moved.c_str(), p.c_str()) == 0);
        KT_CHECK_EQ(t.Get(1).c_str(), "second");              // retried, not poisoned
        // Truncated at Get time (partial copy), then completed.
        std::string p2 = Write(Build({"alpha", "omega"}, [] { Opt o; o.truncateTo = 20 + 80 + 5; return o; }()), "partial.tlk");
        hb::Tlk t2; KT_CHECK(t2.Open(p2.c_str()));
        KT_CHECK_EQ(t2.Get(1).c_str(), "");
        Write(Build({"alpha", "omega"}), "partial.tlk");
        KT_CHECK_EQ(t2.Get(1).c_str(), "omega");
        // Open itself is retryable after a missing file.
        hb::Tlk t3; std::string p3 = g_dir + "/late.tlk";
        KT_CHECK(!t3.Open(p3.c_str()));
        Write(Build({"late"}), "late.tlk");
        KT_CHECK(t3.Open(p3.c_str()));
        KT_CHECK_EQ(t3.Get(0).c_str(), "late");
        // Structurally invalid entries (size >= 65536) and empty entries ARE cached (no re-read forever).
        hb::Tlk t4; KT_CHECK(t4.Open(Write(Build({std::string(65536, 'a'), ""})).c_str()));
        KT_CHECK_EQ(t4.Get(0).c_str(), ""); KT_CHECK_EQ(t4.Get(1).c_str(), "");
        KT_CHECK(t4.cache.size() == 2);
    }

    // ---- Clean (through Get)
    {
        KT_CHECK_EQ(One("\x91it\x92s \x93ok\x94").c_str(), "'it's \"ok\"");
        KT_CHECK_EQ(One("a\x96" "b\x97" "c").c_str(), "a-b-c");
        KT_CHECK_EQ(One("a\r\nb\tc\nd").c_str(), "a b c d");
        KT_CHECK_EQ(One("a \r\n\t b").c_str(), "a b");                 // mixed whitespace run -> one space
        KT_CHECK_EQ(One("   lead").c_str(), "lead");
        KT_CHECK_EQ(One("trail   ").c_str(), "trail");
        KT_CHECK_EQ(One("\r\n\t  ").c_str(), "");                       // whitespace only
        KT_CHECK_EQ(One("a\nb\n").c_str(), "a b");
        KT_CHECK_EQ(One(std::string("ab\0cd", 5)).c_str(), "ab");       // embedded NUL truncates
        KT_CHECK_EQ(One(std::string("\0cd", 3)).c_str(), "");
        KT_CHECK_EQ(One("caf\xC3\xA9").c_str(), "caf??");               // UTF-8 bytes each become '?'
        KT_CHECK_EQ(One("\xE2\x80\x99").c_str(), "???");                // UTF-8 right quote is NOT mapped
        KT_CHECK_EQ(One("a\x01" "b\x7F" "c\x80\xFF").c_str(), "a?b?c??");
        KT_CHECK_EQ(One("a\x85 b").c_str(), "a? b");
        KT_CHECK_EQ(One("x \x92").c_str(), "x '");
        KT_CHECK_EQ(One("quote\x94  ").c_str(), "quote\"");
        KT_CHECK_EQ(One("\x01").c_str(), "?");
        KT_CHECK_EQ(One("a\x1F" "b\x20" "c").c_str(), "a?b c");        // 0x1F is the last control char; 0x20 is a space
        KT_CHECK_EQ(One("tilde~ and {brace}").c_str(), "tilde~ and {brace}");   // 0x7E/0x7B..0x7D pass through
    }

    // cleanup
    {
        std::string cmd = "rm -rf '" + g_dir + "'";
        if (system(cmd.c_str()) != 0) puts("warning: cleanup failed");
    }
    return kt::Report("tlk_test");
}
