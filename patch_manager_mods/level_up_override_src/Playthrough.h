// Playthrough.h -- pure model of the per-playthrough level-up config (levelup_playthroughs.ini) and the playthrough GUID bit packing.
// No windows.h, no STL, fixed caps (host-tested by research/companion_levelup/tools/parity/playthrough_test.cpp).
//
// File format: sections `[g<8 lowercase hex>]` (one per playthrough GUID), lines `<tag>=<preset id>|instant=0|1|hold=0|1|pause=N|pcfilter=<role>/<style>/<align>|ffilter=<mode>|pfilter=<mode>` (missing fields:
// instant 0, hold 1, pause 0, pcfilter/ffilter/pfilter empty; `|pause=N` is written only when N > 0 and the three R10 filter fields only when set (ffilter/pfilter
// `all` = default = not written), so older files round-trip byte-identical; a garbage/negative pause parses as 0 like the other numeric fields. The R10 PC level-up filter
// fields are stored as RAW lowercase tokens ([a-z0-9_/] only, shorter than the field, else ignored = empty) and validated by the caller with R10Filter.h
// (r10f::parseProfile / parseMode): this header stays independent of R10Filter.h.) Everything else (comments, blank lines, unparseable lines, other sections, lines before the first section) is kept
// verbatim and in order, so a rewrite never destroys hand-written content: section header lines are kept byte-identical (text after `]` included), a tag
// line keeps its inline `; comment` (normalised value, comment appended unchanged), and the file's line endings are kept (CRLF when most lines of the parsed
// file end in CRLF, else LF; a mixed file is normalised to the majority). Tags/presets are lowercased on parse. If a cap is exceeded the
// document is flagged `lossy` and the DLL refuses to rewrite the file (removeTagRaw below is the one safe edit of a lossy file).
//
// GUID <-> 32 Boolean globals LUO_G00..LUO_G31 (bit i = LUO_G<ii>).
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace pt {

constexpr int MaxSections = 64, MaxTags = 16, MaxExtra = 16, MaxLines = MaxTags + MaxExtra, MaxLine = 256, TagLen = 32, PresetLen = 48, NameLen = 64;
constexpr int FilterLen = 24, ModeLen = 12;   // raw token caps incl. NUL (melee/single/light and build_hide fit)
constexpr size_t MaxText = static_cast<size_t>(MaxSections + 1) * (static_cast<size_t>(MaxLines) * (MaxLine + 2 + 96) + NameLen + 8);

// ---- GUID bit packing ---------------------------------------------------------------------------------------
inline void bitName(int i, char* out /* >= 8 */) { snprintf(out, 8, "LUO_G%02d", i); }
inline void unpackBits(uint32_t guid, uint8_t* bits /* 32 */) { for (int i = 0; i < 32; ++i) bits[i] = static_cast<uint8_t>((guid >> i) & 1u); }
inline uint32_t packBits(const uint8_t* bits /* 32, nonzero = set */) {
    uint32_t g = 0;
    for (int i = 0; i < 32; ++i) if (bits[i]) g |= 1u << i;
    return g;
}
inline void sectionName(uint32_t guid, char* out /* >= 10 */) { snprintf(out, 10, "g%08x", static_cast<unsigned>(guid)); }

// ---- document -----------------------------------------------------------------------------------------------
struct Line {
    bool isTag = false;
    char text[MaxLine] = "";        // verbatim line (isTag == false); for a tag line the inline comment tail (" ; text", "" when none)
    char tag[TagLen] = "";          // isTag: lowercase tag
    char preset[PresetLen] = "";    // isTag: lowercase preset id
    bool instant = false, hold = true;
    int pause = 0;                  // 0 = preset default; N > 0 = last character level taken before banking
    char pcfilter[FilterLen] = "";  // R10 PC build profile "role/style/align" (raw token, "" = unset)
    char ffilter[ModeLen] = "";     // R10 feat panel filter mode token ("" = all)
    char pfilter[ModeLen] = "";     // R10 power panel filter mode token ("" = all)
};
struct Section {
    char name[NameLen] = "";        // as written between the brackets; sec[0] (the preamble) has no header
    char header[MaxLine] = "";      // the header line exactly as read (incl. text after `]`); "" = write `[name]`
    bool isGuid = false;
    uint32_t guid = 0;
    int n = 0;                      // lines used
    Line lines[MaxLines];
};
struct Doc {
    Section sec[MaxSections + 1];   // sec[0] = preamble (lines before the first header)
    int n = 1;                      // sections used incl. the preamble
    bool lossy = false;             // a cap was exceeded while parsing: some input did not fit
    bool crlf = false;              // the parsed file used CRLF line endings (written back the same way)
    void clear() { crlf = false; for (int i = 0; i < n; ++i) { sec[i].n = 0; sec[i].name[0] = 0; sec[i].header[0] = 0; sec[i].isGuid = false; sec[i].guid = 0; } n = 1; lossy = false; }
};

inline bool isHex(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
// "g" + exactly 8 hex digits -> guid.
inline bool parseSectionName(const char* s, uint32_t* guid) {
    if ((s[0] != 'g' && s[0] != 'G') || strlen(s) != 9) return false;
    uint32_t v = 0;
    for (int i = 1; i < 9; ++i) {
        if (!isHex(s[i])) return false;
        const char c = s[i];
        v = v << 4 | static_cast<uint32_t>(c <= '9' ? c - '0' : (c | 32) - 'a' + 10);
    }
    *guid = v;
    return true;
}
inline void lowerStr(char* s) { for (; *s; ++s) if (*s >= 'A' && *s <= 'Z') *s += 32; }
inline char* trimStr(char* s) {
    while (*s == ' ' || *s == '\t') ++s;
    char* e = s + strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n')) *--e = 0;
    return s;
}
inline int eqi(const char* a, const char* b) {
    for (;; ++a, ++b) {
        char x = *a, y = *b;
        if (x >= 'A' && x <= 'Z') x += 32;
        if (y >= 'A' && y <= 'Z') y += 32;
        if (x != y) return 0;
        if (!x) return 1;
    }
}

// Stores a raw filter token: lowercased, only [a-z0-9_/], shorter than cap, else empty. `all` is the default and stored as empty when allDefault.
inline void setToken(char* dst, int cap, const char* v, bool allDefault) {
    dst[0] = 0;
    if (!v) return;
    const size_t n = strlen(v);
    if (n == 0 || n >= static_cast<size_t>(cap)) return;
    for (size_t i = 0; i < n; ++i) {
        char c = v[i];
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '/')) { dst[0] = 0; return; }
        dst[i] = c;
    }
    dst[n] = 0;
    if (allDefault && !strcmp(dst, "all")) dst[0] = 0;
}

inline Section* findGuid(Doc& d, uint32_t guid) {
    for (int i = 1; i < d.n; ++i) if (d.sec[i].isGuid && d.sec[i].guid == guid) return &d.sec[i];
    return nullptr;
}
inline const Section* findGuid(const Doc& d, uint32_t guid) { return findGuid(const_cast<Doc&>(d), guid); }
inline int tagCount(const Section& s) { int c = 0; for (int i = 0; i < s.n; ++i) c += s.lines[i].isTag; return c; }
inline const Line* findTag(const Section& s, const char* tag) {
    for (int i = 0; i < s.n; ++i) if (s.lines[i].isTag && eqi(s.lines[i].tag, tag)) return &s.lines[i];
    return nullptr;
}

// Parses one value `<preset>|instant=0|1|hold=0|1|pause=N` into l (tag already set). False when the preset is empty / too long.
inline bool parseValue(char* v, Line* l) {
    l->instant = false; l->hold = true; l->pause = 0; l->pcfilter[0] = l->ffilter[0] = l->pfilter[0] = 0;
    char* bar = strchr(v, '|');
    if (bar) *bar = 0;
    char* p = trimStr(v);
    if (!*p || strlen(p) >= PresetLen) return false;
    snprintf(l->preset, sizeof l->preset, "%s", p);
    lowerStr(l->preset);
    while (bar) {
        char* f = bar + 1;
        bar = strchr(f, '|');
        if (bar) *bar = 0;
        char* eq = strchr(f, '=');
        if (!eq) continue;
        *eq = 0;
        char* k = trimStr(f); char* val = trimStr(eq + 1);
        lowerStr(k);
        if (!strcmp(k, "instant")) l->instant = atoi(val) != 0;
        else if (!strcmp(k, "hold")) l->hold = atoi(val) != 0;
        else if (!strcmp(k, "pause")) { const int n = atoi(val); l->pause = n > 0 ? n : 0; }
        else if (!strcmp(k, "pcfilter")) setToken(l->pcfilter, FilterLen, val, false);
        else if (!strcmp(k, "ffilter")) setToken(l->ffilter, ModeLen, val, true);
        else if (!strcmp(k, "pfilter")) setToken(l->pfilter, ModeLen, val, true);
    }
    return true;
}

inline Line* addLine(Doc& d, Section& s) {
    if (s.n >= MaxLines) { d.lossy = true; return nullptr; }
    Line* l = &s.lines[s.n++];
    *l = Line();
    return l;
}

// Parses `len` bytes of text (NOT modified, copied per line). Never fails; sets d.lossy when something did not fit.
inline void parse(Doc& d, const char* text, size_t len) {
    d.clear();
    Section* cur = &d.sec[0];
    size_t pos = 0;
    int nLf = 0, nCrlf = 0;   // newline-terminated lines / those ending in CRLF (line-ending detection)
    while (pos < len) {
        size_t e = pos;
        while (e < len && text[e] != '\n') ++e;
        size_t n = e - pos;
        if (e < len) { ++nLf; if (n && text[pos + n - 1] == '\r') ++nCrlf; }
        if (n && text[pos + n - 1] == '\r') --n;
        char buf[MaxLine];
        if (n >= sizeof buf) { d.lossy = true; n = sizeof buf - 1; }
        memcpy(buf, text + pos, n);
        buf[n] = 0;
        pos = e + 1;
        if (e >= len && n == 0) break;   // no trailing empty line for a final "\n"
        char tmp[MaxLine];
        memcpy(tmp, buf, n + 1);
        char* t = trimStr(tmp);
        if (*t == '[' && strchr(t, ']')) {
            *strchr(t, ']') = 0;
            char* nm = trimStr(t + 1);
            if (d.n >= MaxSections + 1 || strlen(nm) >= NameLen) { d.lossy = true; cur = nullptr; continue; }
            cur = &d.sec[d.n++];
            snprintf(cur->name, sizeof cur->name, "%s", nm);
            cur->n = 0;
            snprintf(cur->header, sizeof cur->header, "%s", buf);
            cur->isGuid = parseSectionName(nm, &cur->guid);
            continue;
        }
        if (!cur) { d.lossy = true; continue; }   // inside a section that did not fit
        Line* l = nullptr;
        char* eq = (cur->isGuid && *t && *t != ';' && *t != '#') ? strchr(t, '=') : nullptr;
        if (eq && tagCount(*cur) < MaxTags) {
            char cm[MaxLine]; cm[0] = 0;   // inline comment tail: from the blanks before the first ';' after the '=' (kept verbatim on write)
            if (char* sc = strchr(eq + 1, ';')) {
                char* b = sc;
                while (b > eq + 1 && (b[-1] == ' ' || b[-1] == '\t')) --b;
                snprintf(cm, sizeof cm, "%s", b);
                *b = 0;
            }
            *eq = 0;
            char* k = trimStr(t);
            char val[MaxLine];
            snprintf(val, sizeof val, "%s", eq + 1);
            Line probe;
            if (*k && strlen(k) < TagLen && parseValue(val, &probe)) {
                memcpy(probe.tag, k, strlen(k) + 1);   // strlen(k) < TagLen checked above
                lowerStr(probe.tag);
                if (!findTag(*cur, probe.tag)) {   // a duplicate tag line falls through and is kept verbatim (never matched)
                    l = addLine(d, *cur);
                    if (l) { probe.isTag = true; *l = probe; memcpy(l->text, cm, strlen(cm) + 1); }
                    continue;
                }
            }
        }
        l = addLine(d, *cur);
        if (l) snprintf(l->text, sizeof l->text, "%s", buf);
    }
    d.crlf = nCrlf > 0 && nCrlf * 2 > nLf;
}

// Serialises into out (cap bytes, NUL-terminated); returns length, or 0 when it does not fit.
inline size_t serialise(const Doc& d, char* out, size_t cap) {
    size_t n = 0;
    auto put = [&](const char* s) -> bool {
        size_t l = strlen(s);
        if (n + l + 1 >= cap) return false;
        memcpy(out + n, s, l); n += l;
        return true;
    };
    const char* const nl = d.crlf ? "\r\n" : "\n";
    for (int i = 0; i < d.n; ++i) {
        const Section& s = d.sec[i];
        if (i > 0) {
            if (s.header[0]) { if (!put(s.header) || !put(nl)) return 0; }
            else if (!put("[") || !put(s.name) || !put("]") || !put(nl)) return 0;
        }
        for (int j = 0; j < s.n; ++j) {
            const Line& l = s.lines[j];
            if (l.isTag) {
                char b[2 * MaxLine + 160], x[96];
                x[0] = 0;   // the optional fields, written only when set (older files round-trip byte-identical)
                if (l.pause > 0) snprintf(x + strlen(x), sizeof x - strlen(x), "|pause=%d", l.pause);
                if (l.pcfilter[0]) snprintf(x + strlen(x), sizeof x - strlen(x), "|pcfilter=%s", l.pcfilter);
                if (l.ffilter[0]) snprintf(x + strlen(x), sizeof x - strlen(x), "|ffilter=%s", l.ffilter);
                if (l.pfilter[0]) snprintf(x + strlen(x), sizeof x - strlen(x), "|pfilter=%s", l.pfilter);
                snprintf(b, sizeof b, "%s=%s|instant=%d|hold=%d%s%s%s", l.tag, l.preset, l.instant, l.hold, x, l.text, nl);
                if (!put(b)) return 0;
            } else if (!put(l.text) || !put(nl)) {
                return 0;
            }
        }
    }
    out[n] = 0;
    return n;
}

// Sets (creating the section / line when missing) tag = preset|instant|hold|pause (pause < 0 stored as 0) in the section of `guid`. False on a bad/too-long argument or a full cap.
inline bool setTag(Doc& d, uint32_t guid, const char* tag, const char* preset, bool instant, bool hold, int pause) {
    if (!tag || !*tag || strlen(tag) >= TagLen || !preset || !*preset || strlen(preset) >= PresetLen || strchr(tag, '=') || strchr(preset, '|') || strchr(preset, '\n')) return false;
    Section* s = findGuid(d, guid);
    if (!s) {
        if (d.n >= MaxSections + 1) return false;
        s = &d.sec[d.n++];
        s->n = 0;
        s->header[0] = 0;
        sectionName(guid, s->name);
        s->isGuid = true; s->guid = guid;
    }
    Line* l = nullptr;
    for (int i = 0; i < s->n; ++i) if (s->lines[i].isTag && eqi(s->lines[i].tag, tag)) l = &s->lines[i];
    if (!l) {
        if (tagCount(*s) >= MaxTags || s->n >= MaxLines) return false;
        l = &s->lines[s->n++];
        *l = Line();
        l->isTag = true;
        snprintf(l->tag, sizeof l->tag, "%s", tag);
        lowerStr(l->tag);
    }
    snprintf(l->preset, sizeof l->preset, "%s", preset);
    lowerStr(l->preset);
    l->instant = instant; l->hold = hold; l->pause = pause > 0 ? pause : 0;
    return true;
}

// Sets the R10 filter fields of an EXISTING tag line (setTag leaves them untouched, and a new line starts with all three unset). Tokens are sanitised
// like parsed ones (see setToken; null / "" / invalid = unset; "all" for ffilter/pfilter = unset). False when the section or tag line is absent.
inline bool setFilters(Doc& d, uint32_t guid, const char* tag, const char* pcfilter, const char* ffilter, const char* pfilter) {
    Section* s = findGuid(d, guid);
    if (!s || !tag) return false;
    for (int i = 0; i < s->n; ++i) {
        Line& l = s->lines[i];
        if (!l.isTag || !eqi(l.tag, tag)) continue;
        setToken(l.pcfilter, FilterLen, pcfilter, false);
        setToken(l.ffilter, ModeLen, ffilter, true);
        setToken(l.pfilter, ModeLen, pfilter, true);
        return true;
    }
    return false;
}

// Removes the tag line; the section stays (an empty section still means "playthrough-bound: dev keys do not apply"). False when absent.
inline bool removeTag(Doc& d, uint32_t guid, const char* tag) {
    Section* s = findGuid(d, guid);
    if (!s || !tag) return false;
    for (int i = 0; i < s->n; ++i) {
        if (!s->lines[i].isTag || !eqi(s->lines[i].tag, tag)) continue;
        for (int j = i; j + 1 < s->n; ++j) s->lines[j] = s->lines[j + 1];
        --s->n;
        return true;
    }
    return false;
}

// The one safe edit of a LOSSY file (a parse that dropped data, so serialise would lose it): deletes, in the raw `text[0..*len)`, the line of `tag` in the first
// section of `guid` (the line the parser applies) and nothing else - every other byte, comments, CRLF and over-long lines included, stays. False (text unchanged)
// when the section or tag line is absent. Same line view as parse(): lines are read up to MaxLine-1 chars, a header is `[name]` + anything.
inline bool removeTagRaw(char* text, size_t* len, uint32_t guid, const char* tag) {
    if (!text || !len || !tag || !*tag) return false;
    bool inSec = false;
    size_t pos = 0;
    while (pos < *len) {
        size_t e = pos;
        while (e < *len && text[e] != '\n') ++e;
        size_t n = e - pos;
        if (n && text[pos + n - 1] == '\r') --n;
        char tmp[MaxLine];
        const size_t take = n >= sizeof tmp ? sizeof tmp - 1 : n;
        memcpy(tmp, text + pos, take);
        tmp[take] = 0;
        const size_t next = e < *len ? e + 1 : *len;
        char* t = trimStr(tmp);
        if (*t == '[' && strchr(t, ']')) {
            *strchr(t, ']') = 0;
            uint32_t g = 0;
            if (inSec) return false;   // the first matching section ended without the tag
            inSec = parseSectionName(trimStr(t + 1), &g) && g == guid;
        } else if (inSec && *t && *t != ';' && *t != '#') {
            if (char* eq = strchr(t, '=')) {
                *eq = 0;
                if (eqi(trimStr(t), tag)) {
                    memmove(text + pos, text + next, *len - next);
                    *len -= next - pos;
                    return true;
                }
            }
        }
        pos = next;
    }
    return false;
}

} // namespace pt
