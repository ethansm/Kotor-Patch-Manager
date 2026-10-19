// Host test of TomlMini.h: the accepted subset, every rejected construct, error line numbers and limits.
#include "TomlMini.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

using namespace tomlmini;

struct Rec {
    char log[2048];
    int tables = 0, kvs = 0;
    int abortAt = -1;   // abort the Nth callback (0-based over table+kv) with a message
    int calls = 0;
};

static const char* onTable(void* u, const char* name, bool arr) {
    Rec* r = (Rec*)u;
    char b[64];
    snprintf(b, sizeof b, "T:%s%s;", arr ? "[[" : "[", name);
    strncat(r->log, b, sizeof r->log - strlen(r->log) - 1);
    ++r->tables;
    if (r->calls++ == r->abortAt) return "stop here";
    return nullptr;
}

static const char* onKv(void* u, const char* key, const Value& v) {
    Rec* r = (Rec*)u;
    char b[160];
    switch (v.type) {
        case V_INT: snprintf(b, sizeof b, "%s=i%d;", key, v.i); break;
        case V_BOOL: snprintf(b, sizeof b, "%s=b%d;", key, v.i); break;
        case V_STR: snprintf(b, sizeof b, "%s=s'%s';", key, v.s); break;
        default: {
            snprintf(b, sizeof b, "%s=a%d", key, v.n);
            for (int i = 0; i < v.n; ++i) { strncat(b, ":", sizeof b - strlen(b) - 1); strncat(b, v.arr[i], sizeof b - strlen(b) - 1); }
            strncat(b, ";", sizeof b - strlen(b) - 1);
        }
    }
    strncat(r->log, b, sizeof r->log - strlen(r->log) - 1);
    ++r->kvs;
    if (r->calls++ == r->abortAt) return "stop here";
    return nullptr;
}

static bool run(const char* text, Rec& r, char* err, int cap) {
    r = Rec();
    r.log[0] = 0;
    Callbacks cb;
    cb.user = &r;
    cb.table = onTable;
    cb.kv = onKv;
    return parse(text, strlen(text), cb, err, cap);
}

static void test_basic() {
    Rec r;
    char err[96];
    const char* t =
        "# schema\n"
        "[page]\n"
        "name = \"Companion Self-Heal\"   # trailing comment\n"
        "  title=\"A # not a comment\"\n"
        "\n"
        "[[row]]\n"
        "id = \"auto\"\n"
        "default = 0\n"
        "min = -5\n"
        "wrap = true\n"
        "restart = false\n"
        "choices = [\"Off\", \"Slow\",\"Fast\"]\n"
        "[[row]]\n"
        "id = \"q\\\"x\\\\y\"\n";
    KT_CHECK(run(t, r, err, sizeof err));
    KT_CHECK(strcmp(r.log, "T:[page;name=s'Companion Self-Heal';title=s'A # not a comment';T:[[row;id=s'auto';default=i0;min=i-5;wrap=b1;restart=b0;"
                           "choices=a3:Off:Slow:Fast;T:[[row;id=s'q\"x\\y';") == 0);
    KT_CHECK_EQ(r.tables, 3);
    KT_CHECK_EQ(r.kvs, 9);
    KT_CHECK(run("[page]\r\nname = \"x\"\r\n", r, err, sizeof err));   // CRLF
    KT_CHECK(strcmp(r.log, "T:[page;name=s'x';") == 0);
    KT_CHECK(run("k = 1\n[page]\n", r, err, sizeof err) && strcmp(r.log, "k=i1;T:[page;") == 0);   // keys before any header
    KT_CHECK(run("", r, err, sizeof err) && r.tables == 0);
    KT_CHECK(run("# only comments\n\n   \n", r, err, sizeof err) && r.kvs == 0);
    KT_CHECK(run("a = 1", r, err, sizeof err) && strcmp(r.log, "a=i1;") == 0);   // no final newline
    KT_CHECK(run("a=+7\nb=2147483647\nc=-2147483648\nd = []\n", r, err, sizeof err));
    KT_CHECK(strcmp(r.log, "a=i7;b=i2147483647;c=i-2147483648;d=a0;") == 0);
    KT_CHECK(run("[ok]  # c\n", r, err, sizeof err));
}

static void expectFail(const char* text, const char* wantPrefix) {
    Rec r;
    char err[96];
    const bool ok = run(text, r, err, sizeof err);
    if (ok || strncmp(err, wantPrefix, strlen(wantPrefix)) != 0) {
        printf("expectFail: '%s' -> ok=%d err='%s' want '%s'\n", text, ok ? 1 : 0, err, wantPrefix);
        KT_CHECK(false);
    }
}

static void test_rejects() {
    expectFail("[page\n", "line 1: expected ']'");
    expectFail("[[row]\n", "line 1: expected ']]'");
    expectFail("[]\n", "line 1: empty table name");
    expectFail("[a.b]\n", "line 1: expected ']'");              // dotted names are not supported
    expectFail("[page] junk\n", "line 1: text after table header");
    expectFail("\n\nkey\n", "line 3: expected '=' after key");
    expectFail("= 1\n", "line 1: expected a key");
    expectFail("a.b = 1\n", "line 1: expected '=' after key");   // dotted keys
    expectFail("a = \n", "line 1: missing value");
    expectFail("a = 1.5\n", "line 1: only plain integers");
    expectFail("a = 0x10\n", "line 1: only plain integers");
    expectFail("a = 1e3\n", "line 1: only plain integers");
    expectFail("a = 1_000\n", "line 1: only plain integers");
    expectFail("a = 99999999999\n", "line 1: integer out of range");
    expectFail("a = -2147483649\n", "line 1: integer out of range");
    expectFail("a = -\n", "line 1: expected digits");
    expectFail("a = 12 34\n", "line 1: text after value");
    expectFail("a = yes\n", "line 1: unsupported value");
    expectFail("a = trueish\n", "line 1: unsupported value");
    expectFail("a = 'lit'\n", "line 1: unsupported value");
    expectFail("a = {x=1}\n", "line 1: unsupported value");
    expectFail("a = \"open\n", "line 1: unterminated string");
    expectFail("a = \"bad\\n\"\n", "line 1: unsupported escape");
    expectFail("a = \"x\" y\n", "line 1: text after value");
    expectFail("a = [1, 2]\n", "line 1: arrays hold strings only");
    expectFail("a = [\"x\"\n", "line 1: expected ',' or ']' in array");
    expectFail("a = [\"x\",\n", "line 1: unterminated array");
    expectFail("a = [\"x\" \"y\"]\n", "line 1: expected ',' or ']'");
    expectFail("[p]\nok = 1\nbad line\n", "line 3:");
}

static void test_limits() {
    Rec r;
    char err[96];
    char big[512];
    // 95-byte string is the longest accepted
    char s95[96], s96[97];
    memset(s95, 'x', 95); s95[95] = 0;
    memset(s96, 'x', 96); s96[96] = 0;
    snprintf(big, sizeof big, "a = \"%s\"\n", s95);
    KT_CHECK(run(big, r, err, sizeof err));
    snprintf(big, sizeof big, "a = \"%s\"\n", s96);
    KT_CHECK(!run(big, r, err, sizeof err) && strstr(err, "string too long") != nullptr);
    // array: 16 entries accepted, 17 rejected
    char arr[512] = "a = [";
    for (int i = 0; i < 16; ++i) strcat(arr, i ? ",\"v\"" : "\"v\"");
    strcat(arr, "]\n");
    KT_CHECK(run(arr, r, err, sizeof err));
    char arr17[512] = "a = [";
    for (int i = 0; i < 17; ++i) strcat(arr17, i ? ",\"v\"" : "\"v\"");
    strcat(arr17, "]\n");
    KT_CHECK(!run(arr17, r, err, sizeof err) && strstr(err, "array too long") != nullptr);
    // long key / table name
    char lk[80];
    memset(lk, 'k', 60); lk[60] = 0;
    snprintf(big, sizeof big, "%s = 1\n", lk);
    KT_CHECK(!run(big, r, err, sizeof err) && strstr(err, "key too long") != nullptr);
    snprintf(big, sizeof big, "[%s]\n", lk);
    KT_CHECK(!run(big, r, err, sizeof err) && strstr(err, "table name too long") != nullptr);
}

static void test_callback_abort_and_err_buffer() {
    Rec r;
    char err[96];
    r = Rec();
    Callbacks cb;
    cb.user = &r;
    cb.table = onTable;
    cb.kv = onKv;
    r.abortAt = 1;
    const char* t = "[p]\na = 1\nb = 2\n";
    KT_CHECK(!parse(t, strlen(t), cb, err, sizeof err));
    KT_CHECK(strcmp(err, "line 2: stop here") == 0);
    KT_CHECK_EQ(r.kvs, 1);                                       // parsing stopped at the failing callback
    char tiny[8];
    KT_CHECK(!parse(t, strlen(t), Callbacks{&r, onTable, nullptr}, tiny, 0) || true);   // cap 0: no write, no crash
    r = Rec();
    r.abortAt = 0;
    KT_CHECK(!parse(t, strlen(t), cb, tiny, sizeof tiny));
    KT_CHECK(strlen(tiny) < sizeof tiny && strncmp(tiny, "line 1:", 7) == 0);   // truncated but terminated
    KT_CHECK(!parse(nullptr, 5, cb, err, sizeof err));
    KT_CHECK(parse(nullptr, 0, cb, err, sizeof err));
    Callbacks none;                                              // no callbacks: syntax check only
    KT_CHECK(parse(t, strlen(t), none, err, sizeof err));
}

int main() {
    test_basic();
    test_rejects();
    test_limits();
    test_callback_abort_and_err_buffer();
    return kt::Report("tomlmini_test");
}
