// IniKit.h -- tiny INI helpers shared by KOTOR2 patch DLLs (header-only, no hooks, no engine access, no logging).
// Survey: patch_manager_mods/19b_cross_patch_sharing_survey.md section D (the trim/split/parse trio and configEnabled() were copied per mod).
//
// What is shared:
//   - trim / parseFloats / splitIniLine: the line-level primitives (moved verbatim from Equip3dLogic.h; behaviour unchanged).
//   - forEachLine(): the loadIni loop (split text into lines, trim, split, skip blank/comment, count) with a per-key callback.
//     The caller keeps ownership of key meaning, value parsing, clamping and logging.
//   - enabledFlag / enabledFile: the one-key "kill switch" rule (configEnabled() in texmem, equip-icon-fix, activation-fix).
// Dialect: "key = value" lines; '#'/';' at column 0 (after trim) is a comment line, and '#'/';' inside a line cuts the value;
// no [sections] (a section line is reported as noEquals), no quoting, keys are case-insensitive (lower-cased), values keep case.
// Plain C/C++ runtime only, so it builds unchanged with g++ on the host and in the MSVC / clang-cl mod build.
#pragma once
#include <cmath>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace inikit {

// Strip leading and trailing whitespace in place (an fgets line keeps its "\r\n"; this removes it).
inline void trim(char* s) {
    char* p = s; while (*p && isspace((unsigned char)*p)) ++p;
    memmove(s, p, strlen(p) + 1);
    for (int n = (int)strlen(s); n > 0 && isspace((unsigned char)s[n - 1]); --n) s[n - 1] = 0;
}

// Exactly n comma-separated floats, whitespace allowed around each; anything else (trailing junk, too few/many, nan/inf, empty) is false.
// strtof based, so "0x10" (hex float) and "1e2" are accepted; a space is NOT a separator ("1 2" is false). n == 0 accepts only a blank value.
inline bool parseFloats(const char* v, float* out, int n) {
    const char* p = v;
    for (int i = 0; i < n; ++i) {
        char* e; out[i] = strtof(p, &e);
        if (e == p || !std::isfinite(out[i])) return false;
        p = e; while (*p && isspace((unsigned char)*p)) ++p;
        if (i < n - 1) { if (*p != ',') return false; ++p; }
    }
    while (*p && isspace((unsigned char)*p)) ++p;
    return *p == 0;
}

// One INI line (already trim()ed by the caller) -> key/value. Blank lines and '#'/';' comment lines are skipped; a line
// without '=' is rejected (and left untouched so the caller can log it). Otherwise `line` is cut in place: *key is
// trimmed and lower-cased, *val is trimmed with everything from the first ';' or '#' dropped (so a value can never
// contain those characters).
enum IniLine { IniLineSkip = 0, IniLineNoEquals, IniLineKeyValue };
inline IniLine splitIniLine(char* line, char** key, char** val) {
    if (!line[0] || line[0] == '#' || line[0] == ';') return IniLineSkip;
    char* eq = strchr(line, '=');
    if (!eq) return IniLineNoEquals;
    *eq = 0; char* k = line; char* v = eq + 1;
    for (char* cm = v; *cm; ++cm) if (*cm == ';' || *cm == '#') { *cm = 0; break; }
    trim(k); trim(v);
    for (char* p = k; *p; ++p) *p = (char)tolower((unsigned char)*p);
    *key = k; *val = v;
    return IniLineKeyValue;
}

// What the per-key callback of forEachLine() decided.
enum IniVerdict { IniVerdictOk = 0, IniVerdictBad, IniVerdictUnknown };

// Line counts of one forEachLine() pass. noEquals includes "[section]" lines. Blank and comment lines are not counted.
struct LoadStats { int ok = 0, bad = 0, unknown = 0, noEquals = 0; };

// Walk `text` line by line (LF or CRLF; the last line needs no newline). `text` is modified IN PLACE (newlines and '=' become NULs):
// the callback's key/value pointers are only valid during the call. onKV(int lineNo, const char* key, const char* val) -> IniVerdict;
// lineNo is 1-based over ALL lines (blank and comment lines included), matching what an fgets loop counts.
template <class F>
inline LoadStats forEachLine(char* text, F onKV) {
    LoadStats st;
    if (!text) return st;
    int lineNo = 0;
    char* p = text;
    while (*p) {
        char* nl = strchr(p, '\n');
        char* next = nl ? nl + 1 : p + strlen(p);
        if (nl) *nl = 0;
        ++lineNo;
        trim(p);
        char* k = nullptr; char* v = nullptr;
        IniLine sp = splitIniLine(p, &k, &v);
        if (sp == IniLineNoEquals) ++st.noEquals;
        else if (sp == IniLineKeyValue) {
            IniVerdict r = onKV(lineNo, (const char*)k, (const char*)v);
            if (r == IniVerdictOk) ++st.ok;
            else if (r == IniVerdictBad) ++st.bad;
            else ++st.unknown;
        }
        p = next;
    }
    return st;
}

// The shared kill-switch rule: scan every line; after optional leading spaces/tabs, a line that STARTS WITH `key` is a match; skip any
// ' ' and '=' characters; the flag is ON iff the next character is not '0'. The LAST matching line wins. No text, no key, or no matching
// line = true (on). Quirks kept from the three original configEnabled() copies:
//   - prefix match only: "enabledX=0" and "enabled_foo=0" also match key "enabled", but the skip loop stops at 'X' / '_', so the
//     value char is that letter (!= '0') and the match turns the flag ON, overriding an earlier "enabled=0" line;
//   - "enabled=" with nothing after it counts as on ('\0' != '0'); "enabled = 00" is off, "enabled=10" is on, "enabled=0.5" is off;
//   - the first character decides, so "enabled=off" is ON; a ';' comment before the key ("; enabled=0") never matches.
inline bool enabledFlag(const char* text, const char* key = "enabled") {
    if (!text || !key || !*key) return true;
    size_t kn = strlen(key);
    bool on = true;
    const char* p = text;
    while (*p) {
        const char* q = p;
        while (*q == ' ' || *q == '\t') ++q;
        if (!strncmp(q, key, kn)) {
            q += kn;
            while (*q == ' ' || *q == '=') ++q;
            on = *q != '0';
        }
        const char* nl = strchr(p, '\n');
        if (!nl) break;
        p = nl + 1;
    }
    return on;
}

// enabledFlag() over a small file (read whole, capped at 64 KB). A missing or unreadable file = true (on).
inline bool enabledFile(const char* path, const char* key = "enabled") {
    if (!path) return true;
    FILE* f = fopen(path, "rb");
    if (!f) return true;
    std::string s; char b[512]; size_t n;
    while (s.size() < 65536 && (n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n);
    fclose(f);
    return enabledFlag(s.c_str(), key);
}

}  // namespace inikit
