// T1 host tests for IniWrite.h (generic section-aware INI text rewriter for mod settings pages).
// Build/run: python3 scripts/run_tests.py iniwrite
#define INIW_FILE_API
#include "IniWrite.h"
#include "check.h"
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <unistd.h>

static std::string show(const std::string& s) {
    std::string o;
    for (char c : s) {
        if (c == '\n') o += "\\n";
        else if (c == '\r') o += "\\r";
        else o += c;
    }
    return o;
}

static void expectStr(const char* what, const std::string& got, const std::string& want) {
    std::string d = "got \"" + show(got) + "\" want \"" + show(want) + "\"";
    kt::Check(what, got == want, d.c_str());
}

static bool setTo(std::string& r, const std::string& in, const char* sec, const char* key, const char* val, size_t cap = 4096) {
    std::vector<char> buf(cap, 'X');
    size_t len = 99999;
    bool ok = iniw::setKey(in.data(), in.size(), sec, key, val, buf.data(), cap, &len);
    if (!ok) { r.clear(); return false; }
    KT_CHECK(len < cap && buf[len] == '\0' && strlen(buf.data()) == len);
    r.assign(buf.data(), len);
    return true;
}

static bool rmTo(std::string& r, const std::string& in, const char* sec, const char* key, size_t cap = 4096) {
    std::vector<char> buf(cap, 'X');
    size_t len = 0;
    if (!iniw::removeKey(in.data(), in.size(), sec, key, buf.data(), cap, &len)) return false;
    r.assign(buf.data(), len);
    return true;
}

static void testReplace() {
    std::string r;
    // rule 1: replace value only, keep indentation / spacing / key spelling / EOL
    KT_CHECK(setTo(r, "[A]\nx=1\ny=2\n", "A", "y", "9"));
    expectStr("replace plain", r, "[A]\nx=1\ny=9\n");
    KT_CHECK(setTo(r, "[A]\n  Key = 5\n", "a", "KEY", "7"));
    expectStr("replace keeps indent, spacing, key spelling; names case-insensitive", r, "[A]\n  Key = 7\n");
    KT_CHECK(setTo(r, "[A]\nk = 5   ; the note\nz=1\n", "A", "k", "6"));
    expectStr("replace keeps trailing comment", r, "[A]\nk = 6   ; the note\nz=1\n");
    KT_CHECK(setTo(r, "[A]\nk=a;b\n", "A", "k", "c"));
    expectStr("glued ; is part of the value", r, "[A]\nk=c\n");
    KT_CHECK(setTo(r, "[A]\nk = ; none\n", "A", "k", "7"));
    expectStr("empty value before comment", r, "[A]\nk = 7 ; none\n");
    KT_CHECK(setTo(r, "[A]\nk = 5 ; c\n", "A", "k", ""));
    expectStr("empty new value keeps comment", r, "[A]\nk = ; c\n");
    KT_CHECK(setTo(r, "[A]\nk=5\r\nm=1\r\n", "A", "k", "6"));
    expectStr("replace keeps CRLF", r, "[A]\nk=6\r\nm=1\r\n");
    KT_CHECK(setTo(r, "[A]\nk=5", "A", "k", "6"));
    expectStr("replace last line without final newline", r, "[A]\nk=6");
    KT_CHECK(setTo(r, "[A]\nk=5\n", "A", "k", nullptr));
    expectStr("null val is empty", r, "[A]\nk=\n");
    KT_CHECK(setTo(r, "[ A ]\nk=5\n", "A", "k", "1"));
    expectStr("header name is trimmed", r, "[ A ]\nk=1\n");
    KT_CHECK(setTo(r, "[A]\n# c\nk=5 # not a comment\n", "A", "k", "1"));
    expectStr("only ';' is an inline comment", r, "[A]\n# c\nk=1\n");
}

static void testCommentedDefault() {
    std::string r;
    // rule 2: ";key = v" is not present -> append at end of section
    KT_CHECK(setTo(r, "[A]\n;k = 1\nm=2\n\n[B]\nz=3\n", "A", "k", "9"));
    expectStr("commented default: append at end of section, before next header", r, "[A]\n;k = 1\nm=2\nk=9\n\n[B]\nz=3\n");
    KT_CHECK(setTo(r, "[A]\n#k = 1\n", "A", "k", "9"));
    expectStr("'#' commented default also not present", r, "[A]\n#k = 1\nk=9\n");
    KT_CHECK(setTo(r, "[A]\nm=2\n[B]\nk=1\n", "A", "k", "9"));
    expectStr("append never lands inside another section", r, "[A]\nm=2\nk=9\n[B]\nk=1\n");
    KT_CHECK(setTo(r, "[A]\n", "A", "k", "9"));
    expectStr("append into header-only section", r, "[A]\nk=9\n");
    KT_CHECK(setTo(r, "[A]\nm=2", "A", "k", "9"));
    expectStr("append at EOF without final newline keeps it that way", r, "[A]\nm=2\nk=9");
    KT_CHECK(setTo(r, "[B]\nz=1\n[A]\nm=2\n", "A", "k", "9"));
    expectStr("append to last section at EOF", r, "[B]\nz=1\n[A]\nm=2\nk=9\n");
    // flat file
    KT_CHECK(setTo(r, "a=1\n;k=2\n[S]\nx=1\n", nullptr, "k", "9"));
    expectStr("flat append stays before the first header", r, "a=1\n;k=2\nk=9\n[S]\nx=1\n");
    KT_CHECK(setTo(r, "a=1\n", "", "a", "2"));
    expectStr("flat replace via empty section", r, "a=2\n");
    KT_CHECK(setTo(r, "k=1\n[S]\nk=5\n", nullptr, "k", "2"));
    expectStr("flat never touches a sectioned key", r, "k=2\n[S]\nk=5\n");
}

static void testSectionMissing() {
    std::string r;
    // rule 3
    KT_CHECK(setTo(r, "[A]\nx=1\n", "New", "k", "v"));
    expectStr("missing section appended with blank separator", r, "[A]\nx=1\n\n[New]\nk=v\n");
    KT_CHECK(setTo(r, "[A]\nx=1\n\n", "New", "k", "v"));
    expectStr("no second blank line when one is there", r, "[A]\nx=1\n\n[New]\nk=v\n");
    KT_CHECK(setTo(r, "[A]\nx=1", "New", "k", "v"));
    expectStr("missing section after unterminated last line", r, "[A]\nx=1\n\n[New]\nk=v\n");
    KT_CHECK(setTo(r, "", "New", "k", "v"));
    expectStr("empty input, named section", r, "[New]\nk=v\n");
    KT_CHECK(setTo(r, "", nullptr, "k", "v"));
    expectStr("empty input, flat", r, "k=v\n");
    KT_CHECK(setTo(r, "[A]\r\nx=1\r\n", "New", "k", "v"));
    expectStr("CRLF input -> CRLF new lines", r, "[A]\r\nx=1\r\n\r\n[New]\r\nk=v\r\n");
}

static void testDuplicates() {
    std::string r;
    // rule 4: last one replaced, others untouched (last-wins parsers)
    KT_CHECK(setTo(r, "[A]\nk=1\nm=0\nk=2\n", "A", "k", "9"));
    expectStr("duplicate key: last replaced", r, "[A]\nk=1\nm=0\nk=9\n");
    KT_CHECK(setTo(r, "[A]\nk=1\n[B]\nk=2\n[A]\nk=3\n", "A", "k", "9"));
    expectStr("split section: last occurrence replaced", r, "[A]\nk=1\n[B]\nk=2\n[A]\nk=9\n");
    // rule 7: other section untouched
    KT_CHECK(setTo(r, "[A]\nk=1\n[B]\nk=2\n", "B", "k", "9"));
    expectStr("same key in another section untouched", r, "[A]\nk=1\n[B]\nk=9\n");
    KT_CHECK(setTo(r, "[AB]\nk=1\n", "A", "k", "9"));
    expectStr("section prefix is not a match", r, "[AB]\nk=1\n\n[A]\nk=9\n");
    KT_CHECK(setTo(r, "[A]\nkk=1\nk2=1\n", "A", "k", "9"));
    expectStr("key prefix is not a match", r, "[A]\nkk=1\nk2=1\nk=9\n");
}

static void testEolAndBytes() {
    std::string r;
    // rule 5: dominant EOL
    KT_CHECK(setTo(r, "[A]\r\na=1\r\nb=2\nc=3\r\n", "A", "n", "1"));
    expectStr("mixed, CRLF dominant", r, "[A]\r\na=1\r\nb=2\nc=3\r\nn=1\r\n");
    KT_CHECK(setTo(r, "[A]\na=1\nb=2\r\n", "A", "n", "1"));
    expectStr("mixed, LF dominant", r, "[A]\na=1\nb=2\r\nn=1\n");
    KT_CHECK(setTo(r, "[A]\r\na=1\r\nb=2", "A", "n", "1"));
    expectStr("CRLF, no final newline, append", r, "[A]\r\na=1\r\nb=2\r\nn=1");
    // rule 6: byte for byte
    const std::string odd = "\xEF\xBB\xBF; head\n\n  # note \t\njunk line\n[A]\n\tweird = 1\t\n\n\n[B]\n =novalue-key\nq=1\n";
    KT_CHECK(setTo(r, odd, "B", "q", "2"));
    expectStr("everything else byte for byte", r,
              "\xEF\xBB\xBF; head\n\n  # note \t\njunk line\n[A]\n\tweird = 1\t\n\n\n[B]\n =novalue-key\nq=2\n");
    KT_CHECK(setTo(r, odd, "A", "new", "1"));
    expectStr("append keeps trailing blank lines after the section", r,
              "\xEF\xBB\xBF; head\n\n  # note \t\njunk line\n[A]\n\tweird = 1\t\nnew=1\n\n\n[B]\n =novalue-key\nq=1\n");
    KT_CHECK(setTo(r, "[A\nk=1\n", nullptr, "k", "2"));
    expectStr("unclosed '[' is not a header", r, "[A\nk=2\n");
}

static void testBounds() {
    std::string r;
    // rule 8
    const std::string in = "[A]\nk=1\n";
    char buf[64];
    size_t len = 7;
    memset(buf, 'X', sizeof buf);
    KT_CHECK(!iniw::setKey(in.data(), in.size(), "A", "k", "22", buf, 8, &len));   // result is 9 bytes + NUL
    KT_CHECK(len == 0);
    KT_CHECK(buf[0] == '\0');
    KT_CHECK(buf[8] == 'X' && buf[9] == 'X');                                      // no overrun
    memset(buf, 'X', sizeof buf);
    KT_CHECK(!iniw::setKey(in.data(), in.size(), "A", "k", "2", buf, 8, &len));    // 8 bytes, no room for NUL
    KT_CHECK(buf[8] == 'X');
    memset(buf, 'X', sizeof buf);
    KT_CHECK(iniw::setKey(in.data(), in.size(), "A", "k", "2", buf, 9, &len));     // exact fit with NUL
    KT_CHECK(len == 8 && strcmp(buf, "[A]\nk=2\n") == 0);
    KT_CHECK(!iniw::setKey(in.data(), in.size(), "A", "k", "2", nullptr, 100, &len));
    KT_CHECK(!iniw::setKey(in.data(), in.size(), "A", "k", "2", buf, 0, &len));
    KT_CHECK(!iniw::setKey(in.data(), in.size(), "Nope", "k", "2", buf, 12, &len));  // append path too small
    KT_CHECK(len == 0 && buf[0] == '\0');
    KT_CHECK(!iniw::removeKey(in.data(), in.size(), "A", "k", buf, 3, &len));
    KT_CHECK(!iniw::removeKey(in.data(), in.size(), "A", "zz", buf, 8, &len));       // unchanged copy needs 9 with NUL
    KT_CHECK(iniw::removeKey(in.data(), in.size(), "A", "zz", buf, 9, &len) && len == 8);
    // null outLen tolerated
    KT_CHECK(iniw::setKey(in.data(), in.size(), "A", "k", "2", buf, 64, nullptr));
    KT_CHECK(iniw::setKey(nullptr, 0, "A", "k", "2", buf, 64, &len) && strcmp(buf, "[A]\nk=2\n") == 0);
}

static void testRejects() {
    std::string r;
    // rule 9
    KT_CHECK(!setTo(r, "[A]\n", "A", nullptr, "1"));
    KT_CHECK(!setTo(r, "[A]\n", "A", "", "1"));
    KT_CHECK(!setTo(r, "[A]\n", "A", "a=b", "1"));
    KT_CHECK(!setTo(r, "[A]\n", "A", "a\nb", "1"));
    KT_CHECK(!setTo(r, "[A]\n", "A", ";k", "1"));
    KT_CHECK(!setTo(r, "[A]\n", "A", " k", "1"));
    KT_CHECK(!setTo(r, "[A]\n", "A", "[k", "1"));
    KT_CHECK(!setTo(r, "[A]\n", "A", "k", "1\n2"));
    KT_CHECK(!setTo(r, "[A]\n", "A", "k", "1\r"));
    KT_CHECK(!setTo(r, "[A]\n", "A]", "k", "1"));
    KT_CHECK(!setTo(r, "[A]\n", "A\n", "k", "1"));
    std::string d;
    KT_CHECK(!rmTo(d, "[A]\n", "A", ""));
    KT_CHECK(!rmTo(d, "[A]\n", "A", nullptr));
}

static void testRemove() {
    std::string r;
    KT_CHECK(rmTo(r, "[A]\nk=1\nm=2\nk=3\n[B]\nk=4\n", "A", "k"));
    expectStr("remove deletes every match in the section only", r, "[A]\nm=2\n[B]\nk=4\n");
    KT_CHECK(rmTo(r, "[A]\n;k=1\nm=2\n", "A", "k"));
    expectStr("remove leaves a commented default alone", r, "[A]\n;k=1\nm=2\n");
    KT_CHECK(rmTo(r, "k=1\n[A]\nk=2\n", nullptr, "k"));
    expectStr("remove flat", r, "[A]\nk=2\n");
    KT_CHECK(rmTo(r, "[A]\r\nk=1\r\nm=2\r\n", "a", "K"));
    expectStr("remove CRLF case-insensitive", r, "[A]\r\nm=2\r\n");
    KT_CHECK(rmTo(r, "[A]\nm=2", "A", "k"));
    expectStr("remove absent key copies", r, "[A]\nm=2");
}

// ---- independent reference model for the fuzz loop ----
struct Entry { std::string sec, key, val; };

static std::string lowerS(std::string s) { for (auto& c : s) if (c >= 'A' && c <= 'Z') c = (char)(c + 32); return s; }
static std::string trimS(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t')) --b;
    return s.substr(a, b - a);
}

static std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> v;
    size_t p = 0;
    while (p < s.size()) {
        size_t q = s.find('\n', p);
        std::string l = q == std::string::npos ? s.substr(p) : s.substr(p, q - p);
        if (!l.empty() && l.back() == '\r') l.pop_back();
        v.push_back(l);
        if (q == std::string::npos) break;
        p = q + 1;
    }
    return v;
}

// Effective (last-wins) value per (section,key); inline comments stripped like the rewriter does.
static std::vector<Entry> model(const std::string& text) {
    std::vector<Entry> out;
    std::string sec;
    for (const std::string& l : splitLines(text)) {
        std::string t = trimS(l);
        if (t.empty() || t[0] == ';' || t[0] == '#') continue;
        if (t[0] == '[') {
            size_t c = t.find(']');
            if (c != std::string::npos) { sec = lowerS(trimS(t.substr(1, c - 1))); continue; }
        }
        size_t eq = l.find('=');
        if (eq == std::string::npos) continue;
        std::string k = lowerS(trimS(l.substr(0, eq)));
        if (k.empty()) continue;
        std::string v = l.substr(eq + 1);
        for (size_t j = 1; j < v.size(); ++j)
            if (v[j] == ';' && (v[j - 1] == ' ' || v[j - 1] == '\t')) { v = v.substr(0, j); break; }
        v = trimS(v);
        bool found = false;
        for (Entry& e : out) if (e.sec == sec && e.key == k) { e.val = v; found = true; }
        if (!found) out.push_back({sec, k, v});
    }
    return out;
}

static const Entry* findE(const std::vector<Entry>& m, const std::string& sec, const std::string& key) {
    for (const Entry& e : m) if (e.sec == sec && e.key == key) return &e;
    return nullptr;
}

static unsigned g_seed = 12345;
static unsigned rnd(unsigned n) { g_seed = g_seed * 1664525u + 1013904223u; return (g_seed >> 8) % n; }

static void testFuzz() {
    const char* secs[] = {"", "A", "B", "Cc"};
    const char* keys[] = {"k0", "k1", "k2", "K3"};
    int bad = 0;
    std::string firstBad;
    for (int iter = 0; iter < 400; ++iter) {
        const bool crlf = rnd(2);
        const char* eol = crlf ? "\r\n" : "\n";
        std::string in;
        int lines = (int)rnd(14);
        for (int i = 0; i < lines; ++i) {
            switch (rnd(8)) {
            case 0: in += std::string("[") + secs[1 + rnd(3)] + "]"; break;
            case 1: in += ""; break;
            case 2: in += std::string("; ") + keys[rnd(4)] + " = " + std::to_string(rnd(100)); break;
            case 3: in += "# free text"; break;
            case 4: in += std::string(rnd(2) ? "  " : "") + keys[rnd(4)] + (rnd(2) ? " = " : "=") + std::to_string(rnd(100)); break;
            case 5: in += std::string(keys[rnd(4)]) + "=" + std::to_string(rnd(100)) + "  ; note"; break;
            case 6: in += "not a key line"; break;
            default: in += std::string(keys[rnd(4)]) + "=" + std::to_string(rnd(100)); break;
            }
            if (i + 1 < lines || rnd(4)) in += eol;
        }
        const char* sec = secs[rnd(4)];
        const char* key = keys[rnd(4)];
        std::string val = "v" + std::to_string(rnd(1000));
        std::string out;
        if (!setTo(out, in, sec, key, val.c_str(), 8192)) { ++bad; firstBad = "setKey failed on:" + show(in); continue; }
        std::vector<Entry> before = model(in), after = model(out);
        std::string ls = lowerS(sec), lk = lowerS(key);
        const Entry* t = findE(after, ls, lk);
        bool ok = t && t->val == val;
        // every other (section,key) is unchanged, and no new one appeared
        for (const Entry& e : before)
            if (!(e.sec == ls && e.key == lk)) { const Entry* a = findE(after, e.sec, e.key); ok = ok && a && a->val == e.val; }
        for (const Entry& e : after)
            if (!(e.sec == ls && e.key == lk)) ok = ok && findE(before, e.sec, e.key);
        // all input lines survive in order, except at most the one replaced line; at most 3 lines added
        std::vector<std::string> li = splitLines(in), lo = splitLines(out);
        std::vector<std::vector<size_t>> dp(li.size() + 1, std::vector<size_t>(lo.size() + 1, 0));
        for (size_t i = 1; i <= li.size(); ++i)
            for (size_t k = 1; k <= lo.size(); ++k)
                dp[i][k] = li[i - 1] == lo[k - 1] ? dp[i - 1][k - 1] + 1 : std::max(dp[i - 1][k], dp[i][k - 1]);
        size_t skipped = li.size() - dp[li.size()][lo.size()];
        ok = ok && skipped <= 1 && lo.size() >= li.size() && lo.size() <= li.size() + 3;
        // EOL style: new text never mixes in the other EOL when the input was uniform
        {
            bool bare = false, anyCrlf = false;
            for (size_t i = 0; i < out.size(); ++i)
                if (out[i] == '\n') { if (i && out[i - 1] == '\r') anyCrlf = true; else bare = true; }
            ok = ok && !(bare && anyCrlf);
        }
        // idempotent
        std::string again;
        ok = ok && setTo(again, out, sec, key, val.c_str(), 8192) && again == out;
        // removal: key gone, others intact
        std::string rem;
        ok = ok && rmTo(rem, out, sec, key, 8192);
        if (ok) {
            std::vector<Entry> r2 = model(rem);
            ok = !findE(r2, ls, lk);
            for (const Entry& e : after)
                if (!(e.sec == ls && e.key == lk)) { const Entry* a = findE(r2, e.sec, e.key); ok = ok && a && a->val == e.val; }
        }
        if (!ok) {
            ++bad;
            if (firstBad.empty())
                firstBad = "t=" + std::string(t ? t->val : "null") + " skipped=" + std::to_string(skipped) + " in=" + show(in) +
                           " sec=" + sec + " key=" + key + " out=" + show(out);
        }
    }
    kt::Check("fuzz: 400 generated inputs set/remove/idempotent/preserve", bad == 0, firstBad.c_str());
}

static void testUpdateFile() {
    char tmpl[] = "/tmp/iniwrite_test.XXXXXX";
    char* d = mkdtemp(tmpl);
    KT_CHECK(d != nullptr);
    if (!d) return;
    std::string dir = d, path = dir + "/m.ini";
    KT_CHECK(iniw::updateFile(path.c_str(), "Sec", "k", "1"));              // missing file is created
    auto slurp = [](const std::string& p) {
        std::string s; if (FILE* f = fopen(p.c_str(), "rb")) { char b[512]; size_t k; while ((k = fread(b, 1, sizeof b, f)) > 0) s.append(b, k); fclose(f); } return s; };
    expectStr("updateFile creates", slurp(path), "[Sec]\nk=1\n");
    KT_CHECK(iniw::updateFile(path.c_str(), "Sec", "k", "22"));
    KT_CHECK(iniw::updateFile(path.c_str(), "Sec", "m", "3"));
    expectStr("updateFile replaces and appends", slurp(path), "[Sec]\nk=22\nm=3\n");
    KT_CHECK(!iniw::updateFile(path.c_str(), "Sec", "", "3"));
    expectStr("updateFile reject leaves file", slurp(path), "[Sec]\nk=22\nm=3\n");
    KT_CHECK(access((path + ".tmp").c_str(), F_OK) != 0);
    remove(path.c_str());
    rmdir(dir.c_str());
}

int main() {
    testReplace();
    testCommentedDefault();
    testSectionMissing();
    testDuplicates();
    testEolAndBytes();
    testBounds();
    testRejects();
    testRemove();
    testFuzz();
    testUpdateFile();
    return kt::Report("iniwrite_test");
}
