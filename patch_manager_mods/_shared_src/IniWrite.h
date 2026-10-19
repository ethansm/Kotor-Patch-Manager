// IniWrite.h -- generic, section-aware INI text rewriter for mod settings pages (header-only).
// Host-tested by iniwrite_test.cpp. Generalises holster_src/HolsterIniWrite.h (flat file, many keys) to one key per call, with sections.
//
// Pure core (namespace iniw): buffer to buffer, no heap, no STL, no I/O, no Windows headers.
//
//   setKey(in, n, section, key, val, out, cap, &outLen)     set one key's value in one section
//   removeKey(in, n, section, key, out, cap, &outLen)       delete every line of that key in that section
//
// section == nullptr or "" means a FLAT file: the keys before the first [Section] header.
//
// Parsing (same shape as the engine / mod parsers):
//   - A header line is  [Name]  (first non-blank char '[', a ']' later on; text after ']' is ignored). Section and key names are
//     compared ASCII case-insensitively, with the header name trimmed.
//   - A comment line starts (after blanks) with ';' or '#'. A commented default such as ";key = 1" is a comment, NOT a key.
//   - A key line is  key = value  (optional blanks around '='; key is the trimmed text before the first '='). A value may end
//     with an inline comment: ';' preceded by a blank ("key = 5  ; note"). A ';' glued to the value ("a;b") is part of the value.
//
// setKey rules:
//   1. Key present in the target section: only that line's value is replaced; indentation, key spelling, spacing before the value
//      and a trailing "; comment" are kept. The line ending is kept.
//   2. Key only present as a commented default (or not at all): a new "key=val" line is appended at the END of the target section
//      (after its last non-blank line, so blank separators before the next header stay put), never inside another section.
//      For a flat file the "section" is the text before the first header.
//   3. Section missing: "[Section]" + "key=val" are appended at EOF, separated from earlier text by one blank line.
//   4. Duplicate keys in the target section: the LAST one is replaced, the others are untouched (the engine and mod parsers are
//      last-wins, so the last line is the one that currently counts).
//   5. Line ending: new lines use the dominant EOL of the input (CRLF only when CRLF lines outnumber bare-LF lines; LF for empty
//      input or a tie). Existing lines keep their own ending. If the input has no final newline and the new line lands at the very
//      end, the output has no final newline either (a newline is put in front of it).
//   6. Everything else (comments, blank lines, unknown lines, other sections, ordering) is copied byte for byte.
//   7. The same key name in a different section is never touched.
//   8. Output buffer too small: returns false, *outLen = 0, out[0] = '\0' (when cap > 0). Nothing is written past cap.
//   9. Rejected (false, nothing written): null/empty key, a key with '=', CR, LF, a leading blank or a leading ';' '#' '[';
//      a section with ']', CR or LF; a value containing CR or LF. A null val is the empty string. in may be null only when n == 0.
//
// Output is NUL-terminated (needs cap >= outLen + 1); outLen excludes the NUL. removeKey follows the same section/key matching,
// removes ALL matching lines (removing only the last would let an earlier duplicate start winning), and succeeds with an
// unchanged copy when the key is absent.
//
// Optional file wrapper: define INIW_FILE_API before including to get iniw::updateFile (reads the file, calls setKey, writes with
// filewatch::atomicWrite). It adds no Win32 calls of its own; it only uses stdio plus what FileWatch.h already uses
// (GetFileAttributesExA, MoveFileExA, _commit). Without the define this header pulls in no <windows.h> and no STL.
#pragma once
#include <stddef.h>
#include <string.h>

namespace iniw {
namespace detail {

inline bool isBlank(char c) { return c == ' ' || c == '\t'; }
inline char lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c; }

struct Line {
    const char* p;   // first byte
    size_t pos;      // offset of p in the input
    size_t body;     // bytes before the line ending
    size_t total;    // bytes including the line ending (none at an unterminated last line)
};

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
    ln.pos = pos;
    pos += ln.total;
    return true;
}

// [Name] header: name = trimmed text between '[' and the first ']'.
inline bool headerOf(const char* b, size_t len, const char*& name, size_t& nlen) {
    size_t i = 0;
    while (i < len && isBlank(b[i])) ++i;
    if (i == len || b[i] != '[') return false;
    size_t close = i + 1;
    while (close < len && b[close] != ']') ++close;
    if (close == len) return false;
    size_t s = i + 1, e = close;
    while (s < e && isBlank(b[s])) ++s;
    while (e > s && isBlank(b[e - 1])) --e;
    name = b + s;
    nlen = e - s;
    return true;
}

inline bool isComment(const char* b, size_t len) {
    size_t i = 0;
    while (i < len && isBlank(b[i])) ++i;
    return i < len && (b[i] == ';' || b[i] == '#');
}

inline bool isBlankLine(const char* b, size_t len) {
    for (size_t i = 0; i < len; ++i) if (!isBlank(b[i])) return false;
    return true;
}

// key = value line. eq = offset of '='. False for comments, headers, lines without '=' and empty keys.
inline bool keyOf(const char* b, size_t len, const char*& key, size_t& klen, size_t& eq) {
    size_t i = 0;
    while (i < len && isBlank(b[i])) ++i;
    if (i == len || b[i] == ';' || b[i] == '#' || b[i] == '[') return false;
    size_t q = i;
    while (q < len && b[q] != '=') ++q;
    if (q == len) return false;
    size_t e = q;
    while (e > i && isBlank(b[e - 1])) --e;
    if (e == i) return false;
    key = b + i;
    klen = e - i;
    eq = q;
    return true;
}

// ASCII case-insensitive: len bytes of a against a NUL-terminated name.
inline bool nameIs(const char* a, size_t len, const char* name) {
    if (strlen(name) != len) return false;
    for (size_t i = 0; i < len; ++i) if (lower(a[i]) != lower(name[i])) return false;
    return true;
}

// Bounded writer: ok turns false on the first write that would not leave room for the final NUL.
struct Out {
    char* buf;
    size_t cap;
    size_t len;
    bool ok;
    Out(char* b, size_t c) : buf(b), cap(c), len(0), ok(b != nullptr && c > 0) {}
    void put(const char* s, size_t k) {
        if (!ok) return;
        if (k > cap - 1 - len) { ok = false; return; }
        if (k) memcpy(buf + len, s, k);
        len += k;
    }
    void puts(const char* s) { put(s, strlen(s)); }
    bool finish(size_t* outLen) {
        if (outLen) *outLen = 0;
        if (!ok) { if (buf && cap) buf[0] = '\0'; return false; }
        buf[len] = '\0';
        if (outLen) *outLen = len;
        return true;
    }
};

// Dominant EOL: CRLF only when CRLF lines outnumber bare-LF lines.
inline const char* dominantEol(const char* in, size_t n) {
    size_t crlf = 0, lf = 0;
    for (size_t i = 0; i < n; ++i)
        if (in[i] == '\n') { if (i > 0 && in[i - 1] == '\r') ++crlf; else ++lf; }
    return crlf > lf ? "\r\n" : "\n";
}

inline bool badName(const char* s, bool isKey) {
    for (const char* p = s; *p; ++p) {
        if (*p == '\r' || *p == '\n') return true;
        if (isKey && *p == '=') return true;
        if (!isKey && *p == ']') return true;
    }
    if (isKey && (isBlank(*s) || *s == ';' || *s == '#' || *s == '[')) return true;
    return false;
}

inline bool badValue(const char* v) {
    for (; *v; ++v) if (*v == '\r' || *v == '\n') return true;
    return false;
}

// What the scan found. flat (section empty): region = text before the first header.
struct Scan {
    bool sectionFound = false;
    bool matched = false;       // a key line was found in the target section
    size_t lineStart = 0;       // last matching key line: offset, body length, total length
    size_t lineBody = 0;
    size_t lineTotal = 0;
    size_t eqOff = 0;           // offset of '=' within that line
    size_t insertAt = 0;        // end of the target section: just after its last non-blank line (or its header)
};

// Walks the lines tracking the current section. want == "" -> flat.
// Last matching section occurrence wins for insertAt; every matching key line updates the "last match".
// all == non-null counts nothing; removal uses a second pass (see removeKey).
inline Scan scan(const char* in, size_t n, const char* section, const char* key) {
    Scan s;
    const bool flat = (section == nullptr || *section == '\0');
    bool inTarget = flat;               // flat region is open from offset 0
    if (flat) { s.sectionFound = true; s.insertAt = 0; }
    size_t pos = 0;
    Line ln;
    while (nextLine(in, n, pos, ln)) {
        const char* name = nullptr;
        size_t nlen = 0;
        if (headerOf(ln.p, ln.body, name, nlen)) {
            inTarget = !flat && nameIs(name, nlen, section ? section : "");
            if (inTarget) { s.sectionFound = true; s.insertAt = ln.pos + ln.total; }
            continue;
        }
        if (!inTarget) continue;
        if (!isBlankLine(ln.p, ln.body)) s.insertAt = ln.pos + ln.total;
        const char* k = nullptr;
        size_t klen = 0, eq = 0;
        if (keyOf(ln.p, ln.body, k, klen, eq) && nameIs(k, klen, key)) {
            s.matched = true;
            s.lineStart = ln.pos;
            s.lineBody = ln.body;
            s.lineTotal = ln.total;
            s.eqOff = eq;
        }
    }
    return s;
}

}  // namespace detail

// Set key=val in section. See the header comment for the rules. true = written to out (NUL-terminated, *outLen set).
inline bool setKey(const char* in, size_t n, const char* section, const char* key, const char* val,
                   char* out, size_t cap, size_t* outLen) {
    using namespace detail;
    if (outLen) *outLen = 0;
    if (!in) n = 0;
    if (!key || !*key || badName(key, true)) return false;
    if (section && badName(section, false)) return false;
    if (!val) val = "";
    if (badValue(val)) return false;
    Out w(out, cap);
    if (!w.ok) return false;
    const char* eol = dominantEol(in, n);
    const bool endsNl = n > 0 && in[n - 1] == '\n';
    Scan sc = scan(in, n, section, key);

    if (sc.matched) {
        // Rules 1 + 4: rewrite the value of the last matching line only.
        const char* b = in + sc.lineStart;
        const size_t len = sc.lineBody;
        const size_t eq = sc.eqOff;
        w.put(in, sc.lineStart);
        w.put(b, eq + 1);                                   // indentation, key spelling, blanks, '='
        // Inline comment: first ';' after '=' that follows a blank.
        size_t cmt = len;                                   // len == none
        for (size_t j = eq + 2; j < len; ++j)
            if (b[j] == ';' && isBlank(b[j - 1])) { cmt = j; break; }
        size_t tail = len;                                  // start of the kept tail ("  ; comment")
        if (cmt < len) { tail = cmt; while (tail > eq + 1 && isBlank(b[tail - 1])) --tail; }
        if (*val) {
            if (tail == eq + 1 && tail < len) w.puts(" ");  // empty value before a comment: keep one blank
            size_t lead = eq + 1;
            while (lead < len && isBlank(b[lead]) && lead < tail) ++lead;
            if (lead > eq + 1) w.put(b + eq + 1, lead - (eq + 1));   // original blanks before the value
            w.puts(val);
        }
        if (tail < len) w.put(b + tail, len - tail);
        w.put(b + len, sc.lineTotal - len);                 // original line ending
        w.put(in + sc.lineStart + sc.lineTotal, n - (sc.lineStart + sc.lineTotal));
        return w.finish(outLen);
    }

    if (sc.sectionFound) {
        // Rule 2: new line at the end of the target section.
        const size_t at = sc.insertAt;
        w.put(in, at);
        const bool atEofNoNl = (at == n) && n > 0 && !endsNl;
        if (at > 0 && in[at - 1] != '\n') w.puts(eol);      // only possible when at == n with no final newline
        w.puts(key);
        w.puts("=");
        w.puts(val);
        if (!atEofNoNl) w.puts(eol);
        w.put(in + at, n - at);
        return w.finish(outLen);
    }

    // Rule 3: section missing -> header + key at EOF, one blank line in between.
    w.put(in, n);
    if (n > 0) {
        if (!endsNl) w.puts(eol);
        // last line blank? find its start
        size_t e = endsNl ? n - 1 : n;                      // end of last line content (before '\n')
        if (e > 0 && in[e - 1] == '\r') --e;
        size_t s = e;
        while (s > 0 && in[s - 1] != '\n') --s;
        if (!isBlankLine(in + s, e - s)) w.puts(eol);
    }
    w.puts("[");
    w.puts(section);
    w.puts("]");
    w.puts(eol);
    w.puts(key);
    w.puts("=");
    w.puts(val);
    w.puts(eol);
    return w.finish(outLen);
}

// Delete every line of key in section (same matching as setKey). Absent key: unchanged copy, still true.
inline bool removeKey(const char* in, size_t n, const char* section, const char* key,
                      char* out, size_t cap, size_t* outLen) {
    using namespace detail;
    if (outLen) *outLen = 0;
    if (!in) n = 0;
    if (!key || !*key || badName(key, true)) return false;
    if (section && badName(section, false)) return false;
    Out w(out, cap);
    if (!w.ok) return false;
    const bool flat = (section == nullptr || *section == '\0');
    bool inTarget = flat;
    size_t pos = 0;
    Line ln;
    while (nextLine(in, n, pos, ln)) {
        const char* name = nullptr;
        size_t nlen = 0;
        bool drop = false;
        if (headerOf(ln.p, ln.body, name, nlen)) {
            inTarget = !flat && nameIs(name, nlen, section);
        } else if (inTarget) {
            const char* k = nullptr;
            size_t klen = 0, eq = 0;
            drop = keyOf(ln.p, ln.body, k, klen, eq) && nameIs(k, klen, key);
        }
        if (!drop) w.put(ln.p, ln.total);
    }
    return w.finish(outLen);
}

}  // namespace iniw

#ifdef INIW_FILE_API
#include "FileWatch.h"
#include <string>
#include <stdio.h>

namespace iniw {

// Read path (missing file = empty), setKey, atomicWrite back. False on any failure; the file is then unchanged.
// stdio for the read; FileWatch.h supplies the write (Win32: GetFileAttributesExA, MoveFileExA, _commit; no additions).
inline bool updateFile(const char* path, const char* section, const char* key, const char* val, bool makeBakOnce = false) {
    if (!path || !*path) return false;
    std::string text;
    if (FILE* f = fopen(path, "rb")) {
        char buf[4096];
        size_t k;
        while ((k = fread(buf, 1, sizeof buf, f)) > 0) text.append(buf, k);
        bool bad = ferror(f) != 0;
        fclose(f);
        if (bad) return false;
    } else if (filewatch::stamp(path).exists) {
        return false;                                       // exists but unreadable: do not clobber it
    }
    // New text is at most: old + eol + blank eol + "[sec]" eol + key=val eol (all bounded by these lengths).
    size_t extra = 16 + (section ? strlen(section) : 0) + (key ? strlen(key) : 0) + (val ? strlen(val) : 0);
    std::string out(text.size() + extra + 1, '\0');
    size_t len = 0;
    if (!setKey(text.data(), text.size(), section, key, val, &out[0], out.size(), &len)) return false;
    out.resize(len);
    return filewatch::atomicWrite(path, out, makeBakOnce);
}

}  // namespace iniw
#endif
