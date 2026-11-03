// RichTextClassifier.h -- Pure C++17 inline span classifier & multi-section description splitter (Doc 53 RMT0).
// Zero Win32/GL dependencies; host-testable. Used by Hook Site 1 (CAurGUIStringInternal::Draw_2)
// and Hook Site 2 (RichDescBox.h at CSWGuiListBox::AddControls_2).
#pragma once

#include "ItemProps.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace rmt {

// Float RGB vectors for CSWGuiText::TextColor
constexpr float kCream[3]      = { 0.8000f, 0.8000f, 0.6980f }; // Titles, primary values, section headers
constexpr float kTealDim[3]    = { 0.0663f, 0.4537f, 0.3569f }; // Property labels ("Damage:"), subtitles, footers
constexpr float kTealText[3]   = { 0.1020f, 0.6980f, 0.5490f }; // Flavor / lore prose body
constexpr float kTealFrame[3]  = { 0.0510f, 0.3490f, 0.2710f }; // 2px divider bars ("uibit_fill_16g")
constexpr float kBonusAmber[3] = { 0.9500f, 0.6600f, 0.3000f }; // Positive modifiers (+N), matches SetLabelBonusPenaltyColour
constexpr float kPenaltyRed[3] = { 0.7400f, 0.1100f, 0.0000f }; // Negative modifiers (-N), matches SetLabelBonusPenaltyColour

// Packed 0xAABBGGRR bytes for glColor4ubv in Draw_2 (caller alpha byte preserved in bits 24..31)
constexpr uint32_t packRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
    return static_cast<uint32_t>(r) | (static_cast<uint32_t>(g) << 8) |
           (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(a) << 24);
}
constexpr uint32_t kRgbCream      = 0x00B2CCCCu; // R=204, G=204, B=178
constexpr uint32_t kRgbTealDim    = 0x005B7411u; // R=17,  G=116, B=91
constexpr uint32_t kRgbTealText   = 0x008CB21Au; // R=26,  G=178, B=140
constexpr uint32_t kRgbBonusAmber = 0x004CA8F2u; // R=242, G=168, B=76
constexpr uint32_t kRgbPenaltyRed = 0x00001CBDu; // R=189, G=28,  B=0

// Font ResRefs (NEVER pass trailing 'b'; CSWGuiManager::ResolveFontName appends 'b' automatically)
constexpr const char* kFontTitle = "dialogfont16x16"; // Resolves to dialogfont16x16b -> d2xfont16x16b
constexpr const char* kFontSub   = "dialogfont10x10"; // Resolves to dialogfont10x10b -> d2xfont16x16b
constexpr const char* kFontDesc  = "fnt_d16x16";      // Resolves to fnt_d16x16b      -> d2xfnt_d16x16b

// CAurGUIStringInternal layout offsets
constexpr size_t kStrOff_PszStr = 0x14;
constexpr size_t kStrOff_ColorR = 0x30;
constexpr size_t kStrOff_ColorG = 0x34;
constexpr size_t kStrOff_ColorB = 0x38;

enum class LineKind : uint8_t {
    Prose    = 0, // Default teal prose body (kRgbTealText)
    Header   = 1, // "Header:" line -> entire line kRgbCream
    KeyValue = 2, // "Key: Value" line -> key (0..colonPos) kRgbTealDim, value kRgbCream + bonus/penalty
};

enum class SpanKind : uint8_t {
    Prose   = 0, // kRgbTealText
    Header  = 1, // kRgbCream
    Key     = 2, // kRgbTealDim
    Value   = 3, // kRgbCream
    Bonus   = 4, // kRgbBonusAmber (+N)
    Penalty = 5, // kRgbPenaltyRed (-N)
};

struct TokenSpan {
    uint16_t start = 0; // Inclusive byte offset from paraStart
    uint16_t end   = 0; // Exclusive byte offset from paraStart
    SpanKind kind  = SpanKind::Prose;
    uint32_t rgb   = kRgbTealText; // 0x00BBGGRR (no alpha)
};

struct LineSpan {
    const char* paraStart = nullptr;
    const char* paraEnd   = nullptr;
    LineKind    kind      = LineKind::Prose;
    uint16_t    colonPos  = 0;
    uint16_t    nSpans    = 0;
    TokenSpan   spans[24] = {};
};

inline bool isDefaultTeal(float r, float g, float b) noexcept {
    return r < 0.20f && g > 0.55f && b > 0.40f;
}

inline bool isDefaultTealPacked(uint32_t packedRgba) noexcept {
    const uint8_t r = static_cast<uint8_t>(packedRgba & 0xFFu);
    const uint8_t g = static_cast<uint8_t>((packedRgba >> 8) & 0xFFu);
    const uint8_t b = static_cast<uint8_t>((packedRgba >> 16) & 0xFFu);
    return r < 51u && g > 140u && b > 102u;
}

inline bool isSentencePunct(char c) noexcept {
    return c == '.' || c == '!' || c == '?';
}

inline bool isSpaceChar(char c) noexcept {
    return c == ' ' || c == '\t' || c == '\r';
}

inline bool hasSentencePunctInRange(const char* s, const char* e) noexcept {
    if (!s || !e || s >= e) return false;
    for (const char* p = s; p < e; ++p) {
        if (*p == '!' || *p == '?') return true;
        if (*p == '.') {
            if (p >= s + 2 && (p[-2] == 'v' || p[-2] == 'V') && (p[-1] == 's' || p[-1] == 'S')) {
                continue;
            }
            return true;
        }
    }
    return false;
}

inline bool eqNoCaseN(const char* a, const char* b, size_t n) noexcept {
    if (!a || !b) return false;
    for (size_t i = 0; i < n; ++i) {
        if (!a[i] || !b[i] || itemprops::lower(a[i]) != itemprops::lower(b[i])) return false;
    }
    return b[n] == '\0';
}

inline bool isRuleHeaderPrefix(const char* s, size_t len) noexcept {
    if (!s || len == 0) return false;
    while (len > 0 && isSpaceChar(*s)) { ++s; --len; }
    while (len > 0 && (s[len - 1] == ':' || isSpaceChar(s[len - 1]))) { --len; }
    if (len == 0) return false;

    auto matchPrefix = [](const char* str, size_t l, const char* target, size_t tlen) {
        if (l < tlen) return false;
        if (!eqNoCaseN(str, target, tlen)) return false;
        return l == tlen || isSpaceChar(str[tlen]) || str[tlen] == ':';
    };

    if (matchPrefix(s, len, "FEATS REQUIRED", 14) || matchPrefix(s, len, "FEAT REQUIRED", 13)) return true;
    if (matchPrefix(s, len, "REQUIREMENTS", 12) || matchPrefix(s, len, "REQUIREMENT", 11)) return true;
    if (matchPrefix(s, len, "ATTRIBUTES REQUIRED", 19) || matchPrefix(s, len, "ATTRIBUTE REQUIRED", 18)) return true;
    if (matchPrefix(s, len, "CANNOT EQUIP", 12)) return true;
    if (matchPrefix(s, len, "RESTRICTED", 10)) return true;
    if (matchPrefix(s, len, "USER LIMITATION", 15)) return true;
    return false;
}

inline bool isUpgradeTagPrefix(const char* s, size_t len) noexcept {
    if (!s || len == 0) return false;
    while (len > 0 && isSpaceChar(*s)) { ++s; --len; }
    while (len > 0 && isSpaceChar(s[len - 1])) { --len; }
    if (len == 0) return false;
    if (eqNoCaseN(s, "FULLY UPGRADEABLE", 17)) return true;
    if (eqNoCaseN(s, "UPGRADEABLE", 11)) return true;
    if (eqNoCaseN(s, "NOT UPGRADEABLE", 15)) return true;
    return false;
}

// Appends or coalesces a span into LineSpan::spans.
inline void pushSpan(LineSpan* out, uint16_t s, uint16_t e, SpanKind kind, uint32_t rgb) noexcept {
    if (!out || s >= e) return;
    if (out->nSpans > 0 && out->spans[out->nSpans - 1].kind == kind &&
        out->spans[out->nSpans - 1].end == s) {
        out->spans[out->nSpans - 1].end = e;
        return;
    }
    if (out->nSpans < sizeof(out->spans) / sizeof(out->spans[0])) {
        out->spans[out->nSpans++] = { s, e, kind, rgb };
    } else {
        out->spans[out->nSpans - 1].end = e;
    }
}

// Tokenizes a value range [valStart, len) inside [paraStart, paraEnd) into Value (kRgbCream),
// Bonus (kRgbBonusAmber), and Penalty (kRgbPenaltyRed) spans.
inline void classifyValueTokens(const char* paraStart, uint16_t valStart, uint16_t len, LineSpan* out) noexcept {
    uint16_t cur = valStart;
    uint16_t i   = valStart;
    while (i < len) {
        const char c = paraStart[i];
        const char prev = (i > 0) ? paraStart[i - 1] : ' ';
        // Bonus: '+' followed by a digit when preceded by whitespace, '(', ':', ',', '/', or start of value
        if (c == '+' && i + 1 < len && itemprops::isDigit(paraStart[i + 1]) &&
            (i == valStart || isSpaceChar(prev) || prev == '(' || prev == ':' || prev == ',' || prev == '/')) {
            if (cur < i) pushSpan(out, cur, i, SpanKind::Value, kRgbCream);
            uint16_t j = i + 1;
            while (j < len && itemprops::isDigit(paraStart[j])) ++j;
            if (j + 1 < len && (paraStart[j] == '-' || paraStart[j] == 'd' || paraStart[j] == 'D') &&
                itemprops::isDigit(paraStart[j + 1])) {
                ++j;
                while (j < len && itemprops::isDigit(paraStart[j])) ++j;
            }
            if (j < len && paraStart[j] == '%') ++j;
            pushSpan(out, i, j, SpanKind::Bonus, kRgbBonusAmber);
            cur = i = j;
            continue;
        }
        // Penalty: '-' followed by a digit ONLY when preceded by whitespace, '(', ':', ',', or '/' (never inside "4-16" or "Light-Scan")
        if (c == '-' && i + 1 < len && itemprops::isDigit(paraStart[i + 1]) &&
            (i == valStart || isSpaceChar(prev) || prev == '(' || prev == ':' || prev == ',' || prev == '/')) {
            if (cur < i) pushSpan(out, cur, i, SpanKind::Value, kRgbCream);
            uint16_t j = i + 1;
            while (j < len && itemprops::isDigit(paraStart[j])) ++j;
            if (j < len && paraStart[j] == '%') ++j;
            pushSpan(out, i, j, SpanKind::Penalty, kRgbPenaltyRed);
            cur = i = j;
            continue;
        }
        ++i;
    }
    if (cur < len) pushSpan(out, cur, len, SpanKind::Value, kRgbCream);
}

// Classifies a single paragraph [paraStart, paraEnd) (bounded by '\n' or string bounds).
// Also inspects strStart when an indented hard-wrapped continuation line ("\n  Critical Hits") follows a Key: Value line.
inline void classifyParagraph(const char* strStart, const char* paraStart, const char* paraEnd, LineSpan* out) noexcept {
    if (!out) return;
    out->paraStart = paraStart;
    out->paraEnd   = paraEnd;
    out->kind      = LineKind::Prose;
    out->colonPos  = 0;
    out->nSpans    = 0;
    if (!paraStart || !paraEnd || paraEnd <= paraStart) return;

    const size_t rawLen = static_cast<size_t>(paraEnd - paraStart);
    const uint16_t len  = rawLen > 0xFFF0u ? 0xFFF0u : static_cast<uint16_t>(rawLen);

    uint16_t trimStart = 0;
    while (trimStart < len && isSpaceChar(paraStart[trimStart])) ++trimStart;
    uint16_t trimEnd = len;
    while (trimEnd > trimStart && isSpaceChar(paraStart[trimEnd - 1])) --trimEnd;

    if (trimStart >= trimEnd) {
        pushSpan(out, 0, len, SpanKind::Prose, kRgbTealText);
        return;
    }

    // Scan for first ':' and check sentence punctuation before it
    int colonIdx = -1;
    bool punctBeforeColon = false;
    for (uint16_t i = trimStart; i < trimEnd; ++i) {
        if (paraStart[i] == ':') {
            colonIdx = static_cast<int>(i);
            break;
        }
        if (isSentencePunct(paraStart[i])) punctBeforeColon = true;
    }

    // Rule 1 (Header: Line): trimmed content ends with ':' and has no sentence punctuation before ':'
    if (paraStart[trimEnd - 1] == ':' && colonIdx == static_cast<int>(trimEnd - 1) && !punctBeforeColon &&
        (trimEnd - trimStart) > 1 && (trimEnd - trimStart) <= 48) {
        out->kind     = LineKind::Header;
        out->colonPos = static_cast<uint16_t>(colonIdx);
        pushSpan(out, 0, len, SpanKind::Header, kRgbCream);
        return;
    }

    // Rule 1b (Requirement / Rule Header Line without colon): e.g. "FEATS REQUIRED", "FEAT REQUIRED"
    if (colonIdx < 0 && !punctBeforeColon && (trimEnd - trimStart) <= 48 &&
        isRuleHeaderPrefix(paraStart + trimStart, static_cast<size_t>(trimEnd - trimStart))) {
        out->kind     = LineKind::Header;
        out->colonPos = trimEnd;
        pushSpan(out, 0, len, SpanKind::Header, kRgbCream);
        return;
    }

    // Rule 2 (Key: Value Line): ": " within first 32 chars, no sentence punctuation before ':'
    if (colonIdx >= static_cast<int>(trimStart) && colonIdx < 32 && !punctBeforeColon &&
        static_cast<uint16_t>(colonIdx + 1) < trimEnd && isSpaceChar(paraStart[colonIdx + 1])) {
        out->kind     = LineKind::KeyValue;
        out->colonPos = static_cast<uint16_t>(colonIdx);
        const uint16_t keyEnd = static_cast<uint16_t>(colonIdx + 1);
        pushSpan(out, 0, keyEnd, SpanKind::Key, kRgbTealDim);
        classifyValueTokens(paraStart, keyEnd, len, out);
        return;
    }

    // Indented continuation line check: if line starts with >= 2 spaces, has no ':', and the preceding paragraph
    // was a Key: Value or Header line, treat this line as a continuation Value span!
    if (colonIdx < 0 && trimStart >= 2 && strStart && paraStart > strStart && paraStart[-1] == '\n') {
        const char* prevEnd = paraStart - 1;
        const char* prevStart = prevEnd;
        while (prevStart > strStart && prevStart[-1] != '\n') --prevStart;
        LineSpan prevSpan;
        classifyParagraph(strStart, prevStart, prevEnd, &prevSpan);
        if (prevSpan.kind == LineKind::KeyValue || prevSpan.kind == LineKind::Header) {
            out->kind = LineKind::KeyValue;
            classifyValueTokens(paraStart, 0, len, out);
            return;
        }
    }

    // Rule 3 (Prose Body): retain default teal
    out->kind = LineKind::Prose;
    pushSpan(out, 0, len, SpanKind::Prose, kRgbTealText);
}

// Convenience helper to classify a NUL-terminated single line or first paragraph of `line`.
inline LineSpan classifyLine(const char* line) noexcept {
    LineSpan span;
    if (!line) return span;
    const char* end = line;
    while (*end && *end != '\n') ++end;
    classifyParagraph(line, line, end, &span);
    return span;
}

inline uint32_t hashParagraph(const char* s, const char* e) noexcept {
    uint32_t h = 2166136261u;
    for (const char* p = s; p < e; ++p) {
        h ^= static_cast<uint8_t>(*p);
        h *= 16777619u;
    }
    return h;
}

// Resolves the packed 0xAABBGGRR color for character `curChar` inside `fullStr`.
// Preserves the caller's alpha byte (`baseCol & 0xFF000000u`).
inline uint32_t resolveGlyphColorInString(const char* fullStr, const char* curChar, uint32_t baseCol) noexcept {
    if (!curChar || !*curChar) return baseCol;
    const char* strStart = (fullStr && reinterpret_cast<uintptr_t>(fullStr) <= reinterpret_cast<uintptr_t>(curChar))
                               ? fullStr
                               : curChar;

    // Scan backward from curChar to nearest '\n' or strStart
    const char* paraStart = curChar;
    while (paraStart > strStart && paraStart[-1] != '\n') --paraStart;

    // Scan forward from curChar to nearest '\n' or '\0'
    const char* paraEnd = curChar;
    while (*paraEnd && *paraEnd != '\n') ++paraEnd;

    // Per-paragraph 1-slot cache (invalidated on pointer or content hash change)
    struct CacheSlot {
        const char* strStart  = nullptr;
        const char* paraStart = nullptr;
        const char* paraEnd   = nullptr;
        uint32_t    hash      = 0;
        LineSpan    span      = {};
    };
    static thread_local CacheSlot s_cache;
    const uint32_t h = hashParagraph(paraStart, paraEnd);
    if (s_cache.strStart != strStart || s_cache.paraStart != paraStart ||
        s_cache.paraEnd != paraEnd || s_cache.hash != h) {
        s_cache.strStart  = strStart;
        s_cache.paraStart = paraStart;
        s_cache.paraEnd   = paraEnd;
        s_cache.hash      = h;
        classifyParagraph(strStart, paraStart, paraEnd, &s_cache.span);
    }

    const LineSpan& ls = s_cache.span;
    if (ls.kind == LineKind::Prose) {
        return (baseCol & 0xFF000000u) | kRgbTealText;
    }
    const uint16_t off = static_cast<uint16_t>(curChar - paraStart);
    for (uint16_t i = 0; i < ls.nSpans; ++i) {
        if (off >= ls.spans[i].start && off < ls.spans[i].end) {
            return (baseCol & 0xFF000000u) | ls.spans[i].rgb;
        }
    }
    return baseCol;
}

// Entry point called by Hook Site 1 (`CAurGUIStringInternal::Draw_2` per-glyph override).
// `self` points to `CAurGUIStringInternal` (`+0x14` = `m_pszStr`, `+0x30..0x38` = float R,G,B).
inline uint32_t resolveGlyphColor(const uint8_t* self, const char* curChar, int lineIdx, uint32_t baseCol) noexcept {
    (void)lineIdx;
    if (!curChar) return baseCol;

    const char* fullStr = nullptr;
    if (self) {
        float r = 0.0f, g = 0.0f, b = 0.0f;
        memcpy(&r, self + kStrOff_ColorR, sizeof(float));
        memcpy(&g, self + kStrOff_ColorG, sizeof(float));
        memcpy(&b, self + kStrOff_ColorB, sizeof(float));
        if (r != 0.0f || g != 0.0f || b != 0.0f) {
            if (!isDefaultTeal(r, g, b)) return baseCol;
        } else if (!isDefaultTealPacked(baseCol)) {
            return baseCol;
        }
        memcpy(&fullStr, self + kStrOff_PszStr, sizeof(const char*));
    } else if (!isDefaultTealPacked(baseCol)) {
        return baseCol;
    }

    return resolveGlyphColorInString(fullStr, curChar, baseCol);
}

// Structured multi-section output for RichDescBox.h (Hook Site 2: CSWGuiListBox::AddControls_2)
struct DescSections {
    char     title[128]    = {}; // Optional Header / Planet / Title row (dialogfont16x16, kCream)
    char     subtitle[128] = {}; // Optional Category / Upgrade tag row (dialogfont10x10, kTealDim)
    char     rules[2048]   = {}; // Key: Value / Header: rules block (dialogfont10x10, inline-colored)
    char     body[4096]    = {}; // Lore / flavor prose body (fnt_d16x16 or dialogfont10x10, kTealText)
    bool     hasTopDivider    = false;
    bool     hasBottomDivider = false;
    unsigned nDividers        = 0;
};

// Splits a full description string (Item, Feat, Force Power, Journal entry, Upgrade, or Option description)
// into structured sections with {tag} stripping and divider bar flags.
inline bool splitDescSections(const char* rawText, DescSections* out) noexcept {
    if (!out) return false;
    *out = DescSections{};
    if (!rawText || !*rawText) return false;

    static thread_local char cleaned[6144];
    itemprops::stripTagsMultiline(cleaned, sizeof(cleaned), rawText);
    {
        // Engine ParseStr leaves "<UNRECOGNIZED TOKEN>" for unset <CUSTOMn> slots; drop the literal.
        static const char kTok[] = "<UNRECOGNIZED TOKEN>";
        const size_t tokLen = sizeof(kTok) - 1;
        char* w = cleaned;
        for (const char* r = cleaned; *r;) {
            if (strncmp(r, kTok, tokLen) == 0) { r += tokLen; continue; }
            *w++ = *r++;
        }
        *w = '\0';
    }

    // Collect non-empty lines and track blank-line boundaries
    struct RawLine {
        const char* b;
        const char* e;
        LineSpan    span;
        bool        blankBefore;
    };
    RawLine lines[128];
    unsigned nLines = 0;
    bool sawBlank = false;

    const char* p = cleaned;
    while (*p && nLines < 128) {
        const char* lineStart = p;
        while (*p && *p != '\n') ++p;
        const char* lineEnd = p;
        if (*p == '\n') ++p;

        const char* ts = lineStart;
        while (ts < lineEnd && isSpaceChar(*ts)) ++ts;
        const char* te = lineEnd;
        while (te > ts && isSpaceChar(te[-1])) --te;

        if (ts >= te) {
            if (nLines > 0) sawBlank = true;
            continue;
        }

        RawLine& rl = lines[nLines++];
        rl.b = lineStart;
        rl.e = lineEnd;
        rl.blankBefore = sawBlank;
        sawBlank = false;
        classifyParagraph(cleaned, lineStart, lineEnd, &rl.span);
    }

    if (nLines == 0) return false;

    unsigned idx = 0;
    // Check if line 0 is a Journal/Planet header ("Nar Shaddaa:") followed directly by prose,
    // OR a short upgrade/category tag without ':' ("Fully Upgradeable", "Not Upgradeable")
    {
        const RawLine& first = lines[0];
        const char* ts = first.b;
        while (ts < first.e && isSpaceChar(*ts)) ++ts;
        const char* te = first.e;
        while (te > ts && isSpaceChar(te[-1])) --te;
        const size_t tlen = static_cast<size_t>(te - ts);

        if (first.span.kind == LineKind::Header && (nLines == 1 || lines[1].span.kind == LineKind::Prose)) {
            const bool isRuleHdr = isRuleHeaderPrefix(ts, tlen);
            if (!isRuleHdr) {
                const size_t copyLen = (tlen > 0 && te[-1] == ':') ? tlen - 1 : tlen;
                const size_t cap = sizeof(out->title) - 1;
                const size_t n = copyLen < cap ? copyLen : cap;
                memcpy(out->title, ts, n);
                out->title[n] = 0;

                bool hasSubCat = false;
                if (nLines > 1) {
                    const char* ts2 = lines[1].b;
                    while (ts2 < lines[1].e && isSpaceChar(*ts2)) ++ts2;
                    const char* te2 = lines[1].e;
                    while (te2 > ts2 && isSpaceChar(te2[-1])) --te2;
                    const size_t tlen2 = static_cast<size_t>(te2 - ts2);
                    const bool sent2 = hasSentencePunctInRange(ts2, te2);
                    if (lines[1].span.kind == LineKind::Prose && tlen2 > 0 && tlen2 <= 48 && !sent2) {
                        const size_t subCap = sizeof(out->subtitle) - 1;
                        const size_t n2 = tlen2 < subCap ? tlen2 : subCap;
                        memcpy(out->subtitle, ts2, n2);
                        out->subtitle[n2] = 0;
                        hasSubCat = true;
                        idx = 2;
                    }
                }

                if (!hasSubCat) {
                    memcpy(out->subtitle, out->title, n + 1);
                    idx = 1;
                }
            }
        } else if (first.span.kind == LineKind::Prose && tlen <= 36 && te[-1] != '.' && te[-1] != '!' && te[-1] != '?' &&
                   !isRuleHeaderPrefix(ts, tlen) &&
                   nLines > 1 && (lines[1].span.kind == LineKind::KeyValue || lines[1].span.kind == LineKind::Header)) {
            const size_t cap = sizeof(out->subtitle) - 1;
            const size_t n = tlen < cap ? tlen : cap;
            memcpy(out->subtitle, ts, n);
            out->subtitle[n] = 0;
            idx = 1;
        }
    }

    // Consume consecutive Rules lines (KeyValue, Header, or rule continuation values)
    bool inHeaderBlock = false;
    while (idx < nLines) {
        const RawLine& rl = lines[idx];

        if (rl.blankBefore) {
            inHeaderBlock = false;
        }

        const char* ts = rl.b;
        while (ts < rl.e && isSpaceChar(*ts)) ++ts;
        const char* te = rl.e;
        while (te > ts && isSpaceChar(te[-1])) --te;
        const size_t tlen = static_cast<size_t>(te - ts);

        const bool hasSentPunct = hasSentencePunctInRange(ts, te);

        bool isRule = false;
        if (rl.span.kind == LineKind::KeyValue) {
            isRule = true;
            // keep inHeaderBlock if already set so following proficiencies stay in rules block
        } else if (rl.span.kind == LineKind::Header) {
            if (isRuleHeaderPrefix(ts, tlen)) {
                isRule = true;
                inHeaderBlock = true;
            } else if (idx + 1 < nLines) {
                const RawLine& next = lines[idx + 1];
                if (next.span.kind == LineKind::KeyValue) {
                    isRule = true;
                    inHeaderBlock = false;
                } else {
                    const char* nts = next.b;
                    while (nts < next.e && isSpaceChar(*nts)) ++nts;
                    const char* nte = next.e;
                    while (nte > nts && isSpaceChar(nte[-1])) --nte;
                    const size_t ntlen = static_cast<size_t>(nte - nts);
                    const bool nextSent = hasSentencePunctInRange(nts, nte);
                    if (ntlen > 0 && ntlen <= 48 && !nextSent) {
                        isRule = true;
                        inHeaderBlock = true;
                    }
                }
            }
        } else if (inHeaderBlock) {
            if (tlen > 0 && tlen <= 48 && !hasSentPunct) {
                isRule = true;
            } else {
                inHeaderBlock = false;
            }
        } else if (idx > 0 && !rl.blankBefore) {
            // Continuation line for a previous KeyValue or rule without an intervening blank line
            const RawLine& prev = lines[idx - 1];
            const char* pe = prev.e;
            while (pe > prev.b && isSpaceChar(pe[-1])) --pe;
            const bool prevComma = (pe > prev.b && (pe[-1] == ',' || pe[-1] == '-' || pe[-1] == '/'));
            const bool startsLower = (ts < te && (*ts >= 'a' && *ts <= 'z'));
            const bool startsOrAnd = ((tlen >= 3 && (ts[0] == 'o' || ts[0] == 'O') && (ts[1] == 'r' || ts[1] == 'R') && isSpaceChar(ts[2])) ||
                                      (tlen >= 4 && (ts[0] == 'a' || ts[0] == 'A') && (ts[1] == 'n' || ts[1] == 'N') && (ts[2] == 'd' || ts[2] == 'D') && isSpaceChar(ts[3])));
            const bool isUpgrade = isUpgradeTagPrefix(ts, tlen);
            const bool prevColon = (pe > prev.b && pe[-1] == ':');
            const bool startsDigit = (ts < te && *ts >= '0' && *ts <= '9');
            if (prevComma || startsLower || startsOrAnd || isUpgrade || (prevColon && startsDigit)) {
                isRule = true;
            }
        }

        if (!isRule) break;

        char lineBuf[itemprops::LineCap];
        const size_t l = static_cast<size_t>(te - ts);
        const size_t n = l < sizeof(lineBuf) - 1 ? l : sizeof(lineBuf) - 1;
        memcpy(lineBuf, ts, n);
        lineBuf[n] = 0;
        if (out->rules[0]) itemprops::appendTrunc(out->rules, sizeof(out->rules), "\n");
        itemprops::appendTrunc(out->rules, sizeof(out->rules), lineBuf);
        ++idx;
    }

    // Remaining lines form the Lore / Flavor Prose Body
    while (idx < nLines) {
        const RawLine& rl = lines[idx];
        const char* ts = rl.b;
        while (ts < rl.e && isSpaceChar(*ts)) ++ts;
        const char* te = rl.e;
        while (te > ts && isSpaceChar(te[-1])) --te;
        char lineBuf[itemprops::LineCap];
        const size_t l = static_cast<size_t>(te - ts);
        const size_t n = l < sizeof(lineBuf) - 1 ? l : sizeof(lineBuf) - 1;
        memcpy(lineBuf, ts, n);
        lineBuf[n] = 0;
        if (out->body[0]) itemprops::appendTrunc(out->body, sizeof(out->body), "\n");
        itemprops::appendTrunc(out->body, sizeof(out->body), lineBuf);
        ++idx;
    }

    out->hasTopDivider    = false;
    out->hasBottomDivider = false;
    out->nDividers        = 0;
    return true;
}

} // namespace rmt
