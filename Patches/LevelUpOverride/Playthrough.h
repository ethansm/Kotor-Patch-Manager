// Playthrough.h -- pure model of the per-playthrough level-up config (levelup_playthroughs.ini) and the playthrough GUID bit packing.
// No windows.h, no STL, fixed caps (host-tested by research/companion_levelup/tools/parity/playthrough_test.cpp).
//
// File format: sections `[g<8 lowercase hex>]` (one per playthrough GUID), lines `<tag>=<preset id>|instant=0|1|hold=0|1|pause=N` (missing fields:
// instant 0, hold 1, pause 0; `|pause=N` is written only when N > 0, so older files round-trip byte-identical; a garbage/negative pause parses as 0 like the other numeric fields). Everything else (comments, blank lines, unparseable lines, other sections, lines before the first section) is kept
// verbatim and in order, so a rewrite never destroys hand-written content. Tags/presets are lowercased on parse. If a cap is exceeded the
// document is flagged `lossy` and the DLL refuses to rewrite the file.
//
// GUID <-> 32 Boolean globals LUO_G00..LUO_G31 (bit i = LUO_G<ii>).
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace pt {

constexpr int MaxSections = 64, MaxTags = 16, MaxExtra = 16, MaxLines = MaxTags + MaxExtra, MaxLine = 256, TagLen = 32, PresetLen = 48, NameLen = 64;
constexpr size_t MaxText = static_cast<size_t>(MaxSections + 1) * (static_cast<size_t>(MaxLines) * (MaxLine + 2) + NameLen + 8);

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
    char text[MaxLine] = "";        // verbatim line (isTag == false)
    char tag[TagLen] = "";          // isTag: lowercase tag
    char preset[PresetLen] = "";    // isTag: lowercase preset id
    bool instant = false, hold = true;
    int pause = 0;                  // 0 = preset default; N > 0 = last character level taken before banking
};
struct Section {
    char name[NameLen] = "";        // as written between the brackets; sec[0] (the preamble) has no header
    bool isGuid = false;
    uint32_t guid = 0;
    int n = 0;                      // lines used
    Line lines[MaxLines];
};
struct Doc {
    Section sec[MaxSections + 1];   // sec[0] = preamble (lines before the first header)
    int n = 1;                      // sections used incl. the preamble
    bool lossy = false;             // a cap was exceeded while parsing: some input did not fit
    void clear() { for (int i = 0; i < n; ++i) { sec[i].n = 0; sec[i].name[0] = 0; sec[i].isGuid = false; sec[i].guid = 0; } n = 1; lossy = false; }
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
    l->instant = false; l->hold = true; l->pause = 0;
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
    while (pos < len) {
        size_t e = pos;
        while (e < len && text[e] != '\n') ++e;
        size_t n = e - pos;
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
            cur->isGuid = parseSectionName(nm, &cur->guid);
            continue;
        }
        if (!cur) { d.lossy = true; continue; }   // inside a section that did not fit
        Line* l = nullptr;
        char* eq = (cur->isGuid && *t && *t != ';' && *t != '#') ? strchr(t, '=') : nullptr;
        if (eq && tagCount(*cur) < MaxTags) {
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
                    if (l) { probe.isTag = true; *l = probe; }
                    continue;
                }
            }
        }
        l = addLine(d, *cur);
        if (l) snprintf(l->text, sizeof l->text, "%s", buf);
    }
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
    for (int i = 0; i < d.n; ++i) {
        const Section& s = d.sec[i];
        if (i > 0) { if (!put("[") || !put(s.name) || !put("]\n")) return 0; }
        for (int j = 0; j < s.n; ++j) {
            const Line& l = s.lines[j];
            if (l.isTag) {
                char b[MaxLine + 64];
                if (l.pause > 0) snprintf(b, sizeof b, "%s=%s|instant=%d|hold=%d|pause=%d\n", l.tag, l.preset, l.instant, l.hold, l.pause);
                else snprintf(b, sizeof b, "%s=%s|instant=%d|hold=%d\n", l.tag, l.preset, l.instant, l.hold);
                if (!put(b)) return 0;
            } else if (!put(l.text) || !put("\n")) {
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

} // namespace pt
