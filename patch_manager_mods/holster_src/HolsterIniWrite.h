// HolsterIniWrite.h -- pure text rewrite of holster.ini for the holster editor: header-only, no file I/O, no allocation, no STL.
// Host-tested by holster_iniwrite_test.cpp.
//
//   setKeys     "key=val" for each KV. The first existing key line is replaced in place (its own line ending is kept).
//               Later duplicate lines of that key get "; dup " in front. Keys not present are appended at the end of the file,
//               after the editor marker line; the marker (preceded by a blank line) is added first if it is missing.
//   removeKeys  deletes every key line whose key is in keys[].
//
// A key line is a line whose first non-whitespace character is not ';' or '#' and that contains '='. Its key is the text before
// the first '=', trimmed. Key compare is ASCII case-insensitive. Everything else (comments, blank lines, sections, other keys)
// is copied byte for byte. LF and CRLF are both kept. New lines use CRLF when the first line ends in CRLF, else LF.
// A file with no final newline stays that way, unless a key is appended (a newline is added first).
// Limits: at most 64 KV entries per setKeys call (more returns false). A KV whose key repeats an earlier entry is ignored.
// Output is NUL-terminated. false = the result plus its NUL did not fit in cap bytes; out is then not valid.
#pragma once
#include <ctype.h>
#include <stddef.h>
#include <string.h>

namespace hs {

struct KV { const char* key; const char* val; };

namespace iniwrite {

static const char kMarker[] = "; --- holster editor (saved in game) ---";
static const int kMaxKV = 64;

inline bool usable(const KV& e) { return e.key && *e.key; }

struct Line {
    const char* p;     // first byte of the line
    size_t body;       // bytes before the line ending
    size_t total;      // bytes including the line ending ("\n", "\r\n", or none at end of input)
};

// Next line at pos (advances pos). False at end of input.
inline bool nextLine(const char* in, size_t n, size_t& pos, Line& ln) {
    if (pos >= n) return false;
    const char* s = in + pos;
    size_t rem = n - pos;
    const char* q = (const char*)memchr(s, '\n', rem);
    if (q) {
        size_t idx = (size_t)(q - s);
        ln.total = idx + 1;
        ln.body = (idx > 0 && s[idx - 1] == '\r') ? idx - 1 : idx;
    } else {
        ln.total = rem;
        ln.body = rem;
    }
    ln.p = s;
    pos += ln.total;
    return true;
}

// Key of a key line (body only). False for blank lines, comments, lines without '=', or an empty key.
inline bool keyOf(const char* b, size_t len, const char*& key, size_t& klen) {
    size_t i = 0;
    while (i < len && isspace((unsigned char)b[i])) ++i;
    if (i == len || b[i] == ';' || b[i] == '#') return false;
    size_t eq = i;
    while (eq < len && b[eq] != '=') ++eq;
    if (eq == len) return false;
    size_t e = eq;
    while (e > i && isspace((unsigned char)b[e - 1])) --e;
    if (e == i) return false;
    key = b + i;
    klen = e - i;
    return true;
}

// ASCII case-insensitive: klen bytes of key against a NUL-terminated name.
inline bool keyIs(const char* key, size_t klen, const char* name) {
    if (!name || strlen(name) != klen) return false;
    for (size_t i = 0; i < klen; ++i)
        if (tolower((unsigned char)key[i]) != tolower((unsigned char)name[i])) return false;
    return true;
}

// ASCII case-insensitive: two NUL-terminated names.
inline bool nameEq(const char* a, const char* b) {
    for (; *a && *b; ++a, ++b)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    return *a == *b;
}

// True when an earlier usable entry has the same key (the earlier one wins, so this entry is never written).
inline bool shadowed(const KV* kv, int i) {
    for (int j = 0; j < i; ++j)
        if (usable(kv[j]) && nameEq(kv[j].key, kv[i].key)) return true;
    return false;
}

inline bool isMarker(const char* b, size_t len) {
    size_t i = 0, e = len;
    while (i < e && isspace((unsigned char)b[i])) ++i;
    while (e > i && isspace((unsigned char)b[e - 1])) --e;
    const size_t m = sizeof kMarker - 1;
    return (e - i) == m && memcmp(b + i, kMarker, m) == 0;
}

inline bool hasMarkerLine(const char* in, size_t n) {
    size_t pos = 0;
    Line ln;
    while (nextLine(in, n, pos, ln))
        if (isMarker(ln.p, ln.body)) return true;
    return false;
}

// CRLF when the first line ends in CRLF, else LF.
inline const char* dominantEol(const char* in, size_t n) {
    const char* q = n ? (const char*)memchr(in, '\n', n) : nullptr;
    return (q && q > in && q[-1] == '\r') ? "\r\n" : "\n";
}

// Bounded writer. ok turns false on the first write that would not leave room for the final NUL.
struct Out {
    char* buf;
    size_t cap;
    size_t len;
    bool ok;

    Out(char* b, size_t c) : buf(b), cap(c), len(0), ok(b != nullptr && c > 0) {}

    void put(const char* s, size_t k) {
        if (!ok) return;
        if (k > cap - 1 - len) { ok = false; return; }
        memcpy(buf + len, s, k);
        len += k;
    }
    void puts(const char* s) { put(s, strlen(s)); }

    bool finish() {
        if (!ok) {
            if (buf && cap) buf[0] = '\0';
            return false;
        }
        buf[len] = '\0';
        return true;
    }

    bool endsNewline() const { return len > 0 && buf[len - 1] == '\n'; }

    // The last line written is empty or whitespace only. Call only when len == 0 or endsNewline().
    bool lastLineBlank() const {
        if (len == 0) return true;
        size_t i = len - 1;
        while (i > 0 && buf[i - 1] != '\n') --i;
        for (size_t k = i; k + 1 < len; ++k)
            if (!isspace((unsigned char)buf[k])) return false;
        return true;
    }
};

}  // namespace iniwrite

// Replace / append key=val lines (see the header comment). in may be null only when n == 0.
inline bool setKeys(const char* in, size_t n, const KV* kv, int count, char* out, size_t cap) {
    using namespace iniwrite;
    Out w(out, cap);
    if (!in) n = 0;
    if (!kv || count < 0) count = 0;
    if (count > kMaxKV) w.ok = false;
    if (!w.ok) return w.finish();

    bool done[kMaxKV] = {};
    const char* eol = dominantEol(in, n);
    const bool hasMarker = hasMarkerLine(in, n);

    size_t pos = 0;
    Line ln;
    while (nextLine(in, n, pos, ln)) {
        const char* key = nullptr;
        size_t klen = 0;
        int hit = -1;
        if (keyOf(ln.p, ln.body, key, klen))
            for (int i = 0; i < count && hit < 0; ++i)
                if (usable(kv[i]) && keyIs(key, klen, kv[i].key)) hit = i;
        if (hit < 0) {
            w.put(ln.p, ln.total);
        } else if (done[hit]) {
            w.puts("; dup ");
            w.put(ln.p, ln.total);
        } else {
            done[hit] = true;
            w.puts(kv[hit].key);
            w.puts("=");
            w.puts(kv[hit].val ? kv[hit].val : "");
            w.put(ln.p + ln.body, ln.total - ln.body);
        }
    }

    bool append = false;
    for (int i = 0; i < count && !append; ++i)
        append = usable(kv[i]) && !done[i] && !shadowed(kv, i);
    if (append) {
        if (w.len > 0 && !w.endsNewline()) w.puts(eol);
        if (!hasMarker) {
            if (w.len > 0 && !w.lastLineBlank()) w.puts(eol);
            w.puts(kMarker);
            w.puts(eol);
        }
        for (int i = 0; i < count; ++i) {
            if (!usable(kv[i]) || done[i] || shadowed(kv, i)) continue;
            w.puts(kv[i].key);
            w.puts("=");
            w.puts(kv[i].val ? kv[i].val : "");
            w.puts(eol);
        }
    }
    return w.finish();
}

// Delete every key line whose key matches one of keys[] (ASCII case-insensitive). Other bytes are copied as they are.
inline bool removeKeys(const char* in, size_t n, const char* const* keys, int count, char* out, size_t cap) {
    using namespace iniwrite;
    Out w(out, cap);
    if (!in) n = 0;
    if (!keys || count < 0) count = 0;
    size_t pos = 0;
    Line ln;
    while (nextLine(in, n, pos, ln)) {
        const char* key = nullptr;
        size_t klen = 0;
        bool drop = false;
        if (keyOf(ln.p, ln.body, key, klen))
            for (int i = 0; i < count && !drop; ++i)
                drop = keyIs(key, klen, keys[i]);
        if (!drop) w.put(ln.p, ln.total);
    }
    return w.finish();
}

}  // namespace hs
