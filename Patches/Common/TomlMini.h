// TomlMini.h -- a tiny TOML subset parser for mod-declared settings pages (header-only, namespace tomlmini). Host test: tomlmini_test.cpp.
// No heap, no STL, no I/O, no Windows headers; fixed arrays only.
//
// Accepted (anything else is an error with a short "line N: ..." message):
//   # comment                      whole-line comments and trailing comments (a '#' inside a "string" is text)
//   [name]                         a table header, name = [A-Za-z0-9_]+ (the callback decides which names exist)
//   [[name]]                       an array-of-tables header (one new table per occurrence)
//   key = 12 | -3                  integer (decimal, optional sign, fits int32)
//   key = true | false             boolean
//   key = "text"                   string, escapes \" and \\ only, at most kMaxStr-1 = 95 bytes, single line
//   key = ["a", "b", ...]          single-line array of strings (at most kMaxArr entries, each at most kMaxStr-1 bytes)
//   key names are [A-Za-z0-9_]+ ; blank lines are ignored ; CRLF and LF both work ; keys before any header go to table "" .
// Not supported on purpose: floats, dates, inline tables, dotted keys, multi-line strings/arrays, literal ('...') strings.
//
// Usage: tomlmini::parse(text, n, cb, err, errCap). The callbacks run in file order and return nullptr to continue or a static
// message to abort (parse then fails with "line N: <message>"). parse() returns true when the whole text was accepted.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace tomlmini {

enum { kMaxStr = 96, kMaxArr = 16 };
enum VType { V_INT = 0, V_BOOL = 1, V_STR = 2, V_ARR = 3 };

struct Value {
    int type = V_INT;
    int32_t i = 0;                   // V_INT value, V_BOOL 0/1
    char s[kMaxStr] = {};            // V_STR
    int n = 0;                       // V_ARR entries
    char arr[kMaxArr][kMaxStr] = {}; // V_ARR strings
};

struct Callbacks {
    void* user = nullptr;
    // A table header: name, isArray ([[name]]). nullptr = ok.
    const char* (*table)(void* user, const char* name, bool isArray) = nullptr;
    // key = value inside the current table (or "" before any header). nullptr = ok.
    const char* (*kv)(void* user, const char* key, const Value& v) = nullptr;
};

namespace detail {

inline bool isBlank(char c) { return c == ' ' || c == '\t'; }
inline bool isNameCh(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; }

inline void setErr(char* err, int cap, int line, const char* msg) {
    if (!err || cap <= 0) return;
    char tmp[12];
    int tl = 0;
    unsigned u = line < 0 ? 0u : (unsigned)line;
    do { tmp[tl++] = (char)('0' + u % 10u); u /= 10u; } while (u);
    int o = 0;
    const char* pre = "line ";
    while (*pre && o + 1 < cap) err[o++] = *pre++;
    while (tl && o + 1 < cap) err[o++] = tmp[--tl];
    const char* mid = ": ";
    while (*mid && o + 1 < cap) err[o++] = *mid++;
    while (msg && *msg && o + 1 < cap) err[o++] = *msg++;
    err[o] = 0;
}

// Parses a "string" starting at b[i] == '"'. Advances i past the closing quote. nullptr = ok, else a message.
inline const char* parseStr(const char* b, size_t len, size_t& i, char* out) {
    ++i;   // opening quote
    size_t o = 0;
    while (i < len) {
        const char c = b[i];
        if (c == '"') { ++i; out[o] = 0; return nullptr; }
        if (c == '\\') {
            if (i + 1 >= len) return "unterminated string";
            const char e = b[i + 1];
            if (e != '"' && e != '\\') return "unsupported escape (only \\\" and \\\\)";
            if (o + 1 >= (size_t)kMaxStr) return "string too long (max 95)";
            out[o++] = e;
            i += 2;
            continue;
        }
        if (o + 1 >= (size_t)kMaxStr) return "string too long (max 95)";
        out[o++] = c;
        ++i;
    }
    return "unterminated string";
}

}  // namespace detail

inline bool parse(const char* text, size_t n, const Callbacks& cb, char* err, int errCap) {
    if (err && errCap > 0) err[0] = 0;
    if (!text && n) { detail::setErr(err, errCap, 0, "no text"); return false; }
    size_t pos = 0;
    int line = 0;
    while (pos < n) {
        ++line;
        const char* b = text + pos;
        size_t rem = n - pos;
        const char* q = (const char*)memchr(b, '\n', rem);
        size_t len = q ? (size_t)(q - b) : rem;
        pos += q ? len + 1 : len;
        if (len && b[len - 1] == '\r') --len;
        size_t i = 0;
        while (i < len && detail::isBlank(b[i])) ++i;
        if (i == len || b[i] == '#') continue;

        if (b[i] == '[') {   // table header
            bool arr = false;
            ++i;
            if (i < len && b[i] == '[') { arr = true; ++i; }
            size_t s = i;
            while (i < len && detail::isNameCh(b[i])) ++i;
            if (i == s) { detail::setErr(err, errCap, line, "empty table name"); return false; }
            char name[32];
            if (i - s >= sizeof name) { detail::setErr(err, errCap, line, "table name too long"); return false; }
            memcpy(name, b + s, i - s);
            name[i - s] = 0;
            if (i >= len || b[i] != ']') { detail::setErr(err, errCap, line, "expected ']'"); return false; }
            ++i;
            if (arr) {
                if (i >= len || b[i] != ']') { detail::setErr(err, errCap, line, "expected ']]'"); return false; }
                ++i;
            }
            while (i < len && detail::isBlank(b[i])) ++i;
            if (i < len && b[i] != '#') { detail::setErr(err, errCap, line, "text after table header"); return false; }
            const char* m = cb.table ? cb.table(cb.user, name, arr) : nullptr;
            if (m) { detail::setErr(err, errCap, line, m); return false; }
            continue;
        }

        // key = value
        size_t ks = i;
        while (i < len && detail::isNameCh(b[i])) ++i;
        if (i == ks) { detail::setErr(err, errCap, line, "expected a key"); return false; }
        char key[48];
        if (i - ks >= sizeof key) { detail::setErr(err, errCap, line, "key too long"); return false; }
        memcpy(key, b + ks, i - ks);
        key[i - ks] = 0;
        while (i < len && detail::isBlank(b[i])) ++i;
        if (i >= len || b[i] != '=') { detail::setErr(err, errCap, line, "expected '=' after key"); return false; }
        ++i;
        while (i < len && detail::isBlank(b[i])) ++i;
        if (i >= len) { detail::setErr(err, errCap, line, "missing value"); return false; }

        Value v;   // 1.7 KB on the stack, one per key line (parse runs at page-build time only)
        const char c = b[i];
        if (c == '"') {
            v.type = V_STR;
            const char* m = detail::parseStr(b, len, i, v.s);
            if (m) { detail::setErr(err, errCap, line, m); return false; }
        } else if (c == '[') {
            v.type = V_ARR;
            ++i;
            for (;;) {
                while (i < len && detail::isBlank(b[i])) ++i;
                if (i >= len) { detail::setErr(err, errCap, line, "unterminated array"); return false; }
                if (b[i] == ']') { ++i; break; }
                if (b[i] != '"') { detail::setErr(err, errCap, line, "arrays hold strings only"); return false; }
                if (v.n >= kMaxArr) { detail::setErr(err, errCap, line, "array too long (max 16)"); return false; }
                const char* m = detail::parseStr(b, len, i, v.arr[v.n]);
                if (m) { detail::setErr(err, errCap, line, m); return false; }
                ++v.n;
                while (i < len && detail::isBlank(b[i])) ++i;
                if (i < len && b[i] == ',') { ++i; continue; }
                if (i < len && b[i] == ']') { ++i; break; }
                detail::setErr(err, errCap, line, "expected ',' or ']' in array");
                return false;
            }
        } else if ((c == 't' && len - i >= 4 && memcmp(b + i, "true", 4) == 0 && (len - i == 4 || !detail::isNameCh(b[i + 4]))) ||
                   (c == 'f' && len - i >= 5 && memcmp(b + i, "false", 5) == 0 && (len - i == 5 || !detail::isNameCh(b[i + 5])))) {
            v.type = V_BOOL;
            v.i = (c == 't') ? 1 : 0;
            i += (c == 't') ? 4 : 5;
        } else if (c == '-' || c == '+' || (c >= '0' && c <= '9')) {
            v.type = V_INT;
            bool neg = false;
            if (c == '-' || c == '+') { neg = c == '-'; ++i; }
            size_t ds = i;
            long long acc = 0;
            while (i < len && b[i] >= '0' && b[i] <= '9') {
                acc = acc * 10 + (b[i] - '0');
                if (acc > 2147483648LL) { detail::setErr(err, errCap, line, "integer out of range"); return false; }
                ++i;
            }
            if (i == ds) { detail::setErr(err, errCap, line, "expected digits"); return false; }
            if (i < len && (b[i] == '.' || b[i] == 'e' || b[i] == 'E' || b[i] == '_' || detail::isNameCh(b[i]))) {
                detail::setErr(err, errCap, line, "only plain integers are supported");
                return false;
            }
            if (neg) acc = -acc;
            if (acc > 2147483647LL || acc < -2147483647LL - 1) { detail::setErr(err, errCap, line, "integer out of range"); return false; }
            v.i = (int32_t)acc;
        } else {
            detail::setErr(err, errCap, line, "unsupported value");
            return false;
        }
        while (i < len && detail::isBlank(b[i])) ++i;
        if (i < len && b[i] != '#') { detail::setErr(err, errCap, line, "text after value"); return false; }
        const char* m = cb.kv ? cb.kv(cb.user, key, v) : nullptr;
        if (m) { detail::setErr(err, errCap, line, m); return false; }
    }
    return true;
}

}  // namespace tomlmini
