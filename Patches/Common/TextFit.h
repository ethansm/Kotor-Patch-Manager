// TextFit.h -- pixel-width text layer shared by the mods: textPx (glyph-width sum x scale), fitText ("..." cut), marqueeText
// (scrolling cut) and wrapAppend/wrapAppendW (greedy word wrap). Header-only, engine-free. Moved verbatim out of
// active_effects_panel_src/ChareffDecode.h (cfx::), which keeps the old names via using-declarations; textfit_test.cpp pins it.
// The font width tables (kFontTexW / kFont10TexW / kFont10LineH) are GENERATED (scripts/gen_effect_name_table.py) into
// FontWidths.inc. Include as "TextFit.h" (Patches/Common is on every patch's include path).
#pragma once
#include <stdio.h>
#include <string.h>

namespace textfit {

#include "FontWidths.inc"   // scripts/gen_effect_name_table.py: kFontTexW, kFont10TexW, kFont10LineH

// rendered dialogfont16x16 width = texels * sy * TextK (screenshot 19:09: 1.98 px/texel at sy 2.6667, all three names)
constexpr float TextK = 0.75f;

// text width in engine px (kFontTexW / kFont10TexW from gen_effect_name_table.py; TextK measured; text_scale = INI fine-tune)
enum Font { F16, F10 };
struct TextCtx { float sy, textK10, textScale; };   // Slot::sy, g_cfg.textK10, g_cfg.textScale
inline float textPx(const TextCtx& c, const char* t, int len, Font f = F16) {
    const unsigned char* tw = f == F10 ? kFont10TexW : kFontTexW;
    float w = 0; for (int i = 0; i < len; ++i) w += tw[(unsigned char)t[i]];
    return w * c.sy * (f == F10 ? c.textK10 : TextK) * c.textScale;
}
// fitted text: cut with "..." to the label width; returns true when it had to cut
inline bool fitText(const TextCtx& c, const char* name, float maxPx, char* out, int n, Font f = F16) {
    int len = (int)strlen(name);
    if (textPx(c, name, len, f) <= maxPx) { snprintf(out, n, "%s", name); return false; }
    float dots = textPx(c, "...", 3, f);
    while (len > 0 && textPx(c, name, len, f) + dots > maxPx) --len;
    while (len > 0 && name[len - 1] == ' ') --len;
    snprintf(out, n, "%.*s...", len, name);
    return true;
}
// marquee: the cut text with `off` characters dropped from the front (no leading "..."; a trailing one while more
// remains); *maxOff = the offset at which the rest fits (0 when the whole text fits)
inline void marqueeText(const TextCtx& c, const char* full, float maxPx, int off, char* out, int n, int* maxOff, Font f = F16) {
    int len = (int)strlen(full), m = 0;
    while (m < len && textPx(c, full + m, len - m, f) > maxPx) ++m;
    *maxOff = m;
    if (off > m) off = m;
    if (off < 0) off = 0;
    while (off > 0 && off < len && full[off] == ' ') ++off;   // never start on a space
    fitText(c, full + off, maxPx, out, n, f);
}
// greedy word wrap of src into out (appends, '\n' between lines); returns the number of lines added. `wide(text, len)` is
// the width measure (the DLL passes textPx F10; the host test also passes a plain character count to pin the algorithm).
template <class Width>
inline int wrapAppendW(Width wide, const char* src, float maxPx, char* out, int* len, int cap) {
    int lines = 0; char cur[256] = ""; int cl = 0;
    auto emit = [&]() {
        int w = snprintf(out + *len, cap - *len, "%s%s", *len ? "\n" : "", cur);
        if (w > 0 && w < cap - *len) { *len += w; ++lines; } else out[*len] = 0;   // no clipped tail on overflow
        cur[0] = 0; cl = 0;
    };
    const char* p = src;
    while (*p) {
        while (*p == ' ') ++p;
        if (!*p) break;
        const char* q = p; while (*q && *q != ' ') ++q;
        int wl = (int)(q - p); if (wl > 120) wl = 120;
        char cand[256]; snprintf(cand, sizeof cand, "%s%s%.*s", cur, cl ? " " : "", wl, p);
        if (cl && wide(cand, (int)strlen(cand)) > maxPx) { emit(); snprintf(cur, sizeof cur, "%.*s", wl, p); }
        else snprintf(cur, sizeof cur, "%s", cand);
        cl = (int)strlen(cur); p = q;
    }
    if (cl) emit();
    return lines;
}
inline int wrapAppend(const TextCtx& c, const char* src, float maxPx, char* out, int* len, int cap) {
    return wrapAppendW([&](const char* t, int n) { return textPx(c, t, n, F10); }, src, maxPx, out, len, cap);
}

} // namespace textfit
