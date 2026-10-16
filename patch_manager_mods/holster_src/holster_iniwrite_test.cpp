// T1 host tests for HolsterIniWrite.h (pure holster.ini text rewrite for the holster editor; doc 42).
// Build/run: python3 scripts/run_tests.py holster_iniwrite
#include "HolsterIniWrite.h"
#include "check.h"
#include <string.h>
#include <string>
#include <vector>

using namespace hs;

static const std::string kM = "; --- holster editor (saved in game) ---";

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

static int countOf(const std::string& s, const char* needle) {
    int c = 0;
    size_t m = strlen(needle);
    for (size_t p = s.find(needle); p != std::string::npos; p = s.find(needle, p + m)) ++c;
    return c;
}

// True when some LF is not preceded by CR.
static bool bareLf(const std::string& s) {
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] == '\n' && (i == 0 || s[i - 1] != '\r')) return true;
    return false;
}

static bool setTo(std::string& r, const std::string& in, const KV* kv, int count, size_t cap = 4096) {
    std::vector<char> buf(cap, 'X');
    if (!setKeys(in.data(), in.size(), kv, count, buf.data(), cap)) return false;
    KT_CHECK(memchr(buf.data(), 0, cap) != nullptr);
    r = buf.data();
    return true;
}

static bool set1(std::string& r, const std::string& in, const char* key, const char* val) {
    KV kv[] = {{key, val}};
    return setTo(r, in, kv, 1);
}

static bool rmTo(std::string& r, const std::string& in, const char* const* keys, int count, size_t cap = 4096) {
    std::vector<char> buf(cap, 'X');
    if (!removeKeys(in.data(), in.size(), keys, count, buf.data(), cap)) return false;
    KT_CHECK(memchr(buf.data(), 0, cap) != nullptr);
    r = buf.data();
    return true;
}

static void testReplaceInPlace() {
    std::string r;
    KT_CHECK(set1(r, "; top\nprobe=0\nplace=0\n", "place", "1"));
    expectStr("replace in place", r, "; top\nprobe=0\nplace=1\n");
    KT_CHECK(countOf(r, "holster editor") == 0);   // replacing needs no marker
    KT_CHECK(set1(r, "probe=0\n\n; comment\n\nplace_rot=0,0,0\n", "place_rot", "1,2,3"));
    expectStr("replace keeps blank lines and comments", r, "probe=0\n\n; comment\n\nplace_rot=1,2,3\n");
    KT_CHECK(set1(r, "probe=0\nplace=0", "place", "1"));
    expectStr("replace last line without final newline", r, "probe=0\nplace=1");
}

static void testAppendCreatesMarker() {
    std::string r;
    KT_CHECK(set1(r, "probe=0\n", "place_rot", "1,2,3"));
    expectStr("append creates marker after blank line", r, "probe=0\n\n" + kM + "\nplace_rot=1,2,3\n");
}

static void testSecondAppendReusesMarker() {
    std::string r1, r2, r3;
    KT_CHECK(set1(r1, "probe=0\n", "place_rot", "1,2,3"));
    KT_CHECK(set1(r2, r1, "place_off", "0.1,0,0"));
    expectStr("second append reuses marker", r2, "probe=0\n\n" + kM + "\nplace_rot=1,2,3\nplace_off=0.1,0,0\n");
    KT_CHECK_EQ(countOf(r2, "--- holster editor"), 1);
    KT_CHECK(set1(r3, r2, "place_rot", "9,9,9"));   // editor key replaced in place, still one marker
    expectStr("editor key replaced in place", r3, "probe=0\n\n" + kM + "\nplace_rot=9,9,9\nplace_off=0.1,0,0\n");
    KT_CHECK_EQ(countOf(r3, "--- holster editor"), 1);
}

static void testMultiKeyAppendAndReplace() {
    std::string r;
    KV kv[] = {{"place", "1"}, {"place_off", "0.1,0,0"}};
    KT_CHECK(setTo(r, "probe=0\nplace=0\n", kv, 2));
    expectStr("multi-key: replace one, append one", r, "probe=0\nplace=1\n\n" + kM + "\nplace_off=0.1,0,0\n");
}

static void testCrlfStaysCrlf() {
    std::string r;
    KT_CHECK(set1(r, "probe=0\r\nplace=0\r\n", "place", "1"));
    expectStr("CRLF replace keeps CRLF", r, "probe=0\r\nplace=1\r\n");
    KT_CHECK(!bareLf(r));
    KT_CHECK(set1(r, "probe=0\r\n", "place_off", "0.1,0,0"));
    expectStr("CRLF append uses CRLF", r, "probe=0\r\n\r\n" + kM + "\r\nplace_off=0.1,0,0\r\n");
    KT_CHECK(!bareLf(r));
    KT_CHECK(set1(r, "place=1\r\nplace=2\r\n", "place", "9"));
    expectStr("CRLF duplicate keeps CRLF", r, "place=9\r\n; dup place=2\r\n");
    KT_CHECK(!bareLf(r));
}

static void testDuplicatesCommentedOut() {
    std::string r;
    KT_CHECK(set1(r, "place=1\nprobe=0\nPLACE = 2\n  place=3  \n", "place", "9"));
    expectStr("duplicates get '; dup ' prefix", r, "place=9\nprobe=0\n; dup PLACE = 2\n; dup   place=3  \n");
    KV kv[] = {{"place", "1"}, {"place", "2"}};
    KT_CHECK(setTo(r, "probe=0\n", kv, 2));
    expectStr("repeated KV entry: first wins", r, "probe=0\n\n" + kM + "\nplace=1\n");
    KT_CHECK(countOf(r, "place=2") == 0);
}

static void testCommentsAreNotKeys() {
    std::string r;
    KT_CHECK(set1(r, "; place=1\n# place=2\n  ;place=3\nprobe=0\n", "place", "5"));
    expectStr("commented-out keys are not keys", r,
              "; place=1\n# place=2\n  ;place=3\nprobe=0\n\n" + kM + "\nplace=5\n");
    KT_CHECK(countOf(r, "; dup") == 0);
}

static void testCaseAndWhitespaceInsensitiveMatch() {
    std::string r;
    KT_CHECK(set1(r, "   Place_Rot   =   1,2,3   \nprobe=0\n", "place_rot", "0,0,0"));
    expectStr("case/whitespace-insensitive match", r, "place_rot=0,0,0\nprobe=0\n");
    KT_CHECK(set1(r, "place_rot_pistol=1\n", "place_rot", "0"));
    expectStr("key match is exact, not prefix", r, "place_rot_pistol=1\n\n" + kM + "\nplace_rot=0\n");
}

static void testEmptyAndNullInput() {
    std::string r;
    KT_CHECK(set1(r, "", "place", "1"));
    expectStr("empty input gets marker and key", r, kM + "\nplace=1\n");
    std::vector<char> buf(64, 'X');
    KV kv[] = {{"place", "1"}};
    KT_CHECK(setKeys(nullptr, 0, kv, 1, buf.data(), buf.size()));
    KT_CHECK(strcmp(buf.data(), (kM + "\nplace=1\n").c_str()) == 0);
    KV nokey[] = {{nullptr, "x"}};
    KT_CHECK(set1(r, "probe=0\n", "", "x"));   // empty key is ignored (set1 with "" key)
    expectStr("empty key ignored", r, "probe=0\n");
    KT_CHECK(setTo(r, "probe=0\n", nokey, 1));
    expectStr("null key ignored", r, "probe=0\n");
}

static void testMissingFinalNewline() {
    std::string r;
    KT_CHECK(set1(r, "probe=0", "place", "1"));
    expectStr("append to file without final newline", r, "probe=0\n\n" + kM + "\nplace=1\n");
    KT_CHECK(set1(r, "probe=0\r\nplace=0", "place_off", "0.1,0,0"));
    expectStr("missing final newline + CRLF file uses CRLF", r,
              "probe=0\r\nplace=0\r\n\r\n" + kM + "\r\nplace_off=0.1,0,0\r\n");
    KT_CHECK(!bareLf(r));
}

static void testExistingMarkerNotDuplicated() {
    std::string r;
    KT_CHECK(set1(r, "probe=0\n\n" + kM + "\nplace=1\n\n", "place_off", "0,0,0"));
    expectStr("existing marker: new key goes at end, no second marker, no extra blank",
              r, "probe=0\n\n" + kM + "\nplace=1\n\nplace_off=0,0,0\n");
    KT_CHECK_EQ(countOf(r, "--- holster editor"), 1);
}

static void testRemoveKeys() {
    std::string r;
    const char* keys[] = {"place", "place_rot"};
    KT_CHECK(rmTo(r, "probe=0\nplace=1\n; place=1\nPLACE_ROT=1,2,3\nplace=2\nplace_off=0,0,0\n", keys, 2));
    expectStr("removeKeys removes all matches, keeps others", r, "probe=0\n; place=1\nplace_off=0,0,0\n");
    KT_CHECK(rmTo(r, "probe=0\nplace_off=1\n", keys, 2));
    expectStr("removeKeys with no match is a copy", r, "probe=0\nplace_off=1\n");
    KT_CHECK(rmTo(r, "", keys, 2));
    expectStr("removeKeys empty input", r, "");
    KT_CHECK(rmTo(r, "probe=0\nplace=1", keys, 2));
    expectStr("removeKeys last line without newline", r, "probe=0\n");
    KT_CHECK(rmTo(r, "probe=0\r\nplace=1\r\nprobe2=1\r\n", keys, 1));
    expectStr("removeKeys keeps CRLF", r, "probe=0\r\nprobe2=1\r\n");
}

static void testOverflow() {
    std::string exp = "probe=0\n\n" + kM + "\nplace=1\n";
    std::vector<char> buf(exp.size() + 2, 'X');
    KV kv[] = {{"place", "1"}};
    KT_CHECK(!setKeys("probe=0\n", 8, kv, 1, buf.data(), exp.size()));   // no room for the NUL
    KT_CHECK(setKeys("probe=0\n", 8, kv, 1, buf.data(), exp.size() + 1));  // exact fit
    expectStr("exact-fit cap", std::string(buf.data()), exp);
    KT_CHECK(!setKeys("probe=0\n", 8, kv, 1, buf.data(), 0));
    KT_CHECK(!setKeys("probe=0\n", 8, kv, 1, nullptr, 16));

    // Replace-in-place: "place=1\n" is 8 bytes.
    std::string r;
    KT_CHECK(set1(r, "place=0\n", "place", "1"));   // default 4096 cap succeeds
    expectStr("replace in place, default cap", r, "place=1\n");
    std::vector<char> small(8, 'X');
    KT_CHECK(!setKeys("place=0\n", 8, kv, 1, small.data(), 8));
    std::vector<char> fit(9, 'X');
    KT_CHECK(setKeys("place=0\n", 8, kv, 1, fit.data(), 9));
    expectStr("replace exact fit", std::string(fit.data()), "place=1\n");

    // removeKeys: "a=1\n" is 4 bytes.
    const char* keys[] = {"b"};
    std::vector<char> rm4(4, 'X');
    KT_CHECK(!removeKeys("a=1\nb=2\n", 8, keys, 1, rm4.data(), 4));
    std::vector<char> rm5(5, 'X');
    KT_CHECK(removeKeys("a=1\nb=2\n", 8, keys, 1, rm5.data(), 5));
    expectStr("removeKeys exact fit", std::string(rm5.data()), "a=1\n");
}

static void testTooManyEntries() {
    KV many[65];
    for (int i = 0; i < 65; ++i) many[i] = {"place", "1"};
    std::string r;
    KT_CHECK(!setTo(r, "probe=0\n", many, 65));
}

int main() {
    testReplaceInPlace();
    testAppendCreatesMarker();
    testSecondAppendReusesMarker();
    testMultiKeyAppendAndReplace();
    testCrlfStaysCrlf();
    testDuplicatesCommentedOut();
    testCommentsAreNotKeys();
    testCaseAndWhitespaceInsensitiveMatch();
    testEmptyAndNullInput();
    testMissingFinalNewline();
    testExistingMarkerNotDuplicated();
    testRemoveKeys();
    testOverflow();
    testTooManyEntries();
    return kt::Report("holster_iniwrite");
}
