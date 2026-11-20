// HseSpeed.h -- the Speed slider of the Holstered Weapons page (holster-editor 0.7.1-dev, HW44). Pure: no Win32 (HolsterEditor.cpp writes holster.ini
// through iniw::updateFile; hse_speed_test.cpp pins the mapping, the label and the ini read on the host).
//
// The control is four page items in the bottom bar row, beside the Holster toggle: a track (a press or a held drag anywhere on it sets the speed), its
// wedge fill, a thumb and the value label "Speed 4x". The value is the flat integer key slow_holster_x in the game-dir holster.ini: 1 = normal speed,
// 8 = eight times slower. holster.dll polls that file every 500 ms (holster 0.21.0-dev), so the preview and the in-game draw / stow slides follow at
// once. A change is written the moment the whole-step value changes; opening the page only reads.
#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Common/IniWrite.h"   // iniw::detail::scan: the reader finds the very line iniw::updateFile would replace (flat file, last line wins, ';' / '#' lines are comments)

namespace hse {

constexpr int kSpeedMin = 1;                              // normal speed
constexpr int kSpeedMax = 8;                              // eight times slower
constexpr int kSpeedDefault = 1;                          // shown when the key is absent (nothing is written for it)
constexpr int kSpeedItems = 3;                            // page boxes: track, thumb, value label (the track's wedge is one more item beside them)
constexpr const char* kSpeedIniFile = "holster.ini";      // game-dir file, relative (as holster.dll opens it)
constexpr const char* kSpeedIniKey = "slow_holster_x";    // flat key: holster.ini has no [section] headers
constexpr size_t kSpeedIniCap = 64 * 1024;                // the reader takes this much of the file (the stock declarative reader keeps 16 KB; the template is 21 KB)

inline int speedClamp(int v) { return v < kSpeedMin ? kSpeedMin : (v > kSpeedMax ? kSpeedMax : v); }

// Speed at the cursor px on a track that starts at trackX and is trackW wide, with a thumb thumbW wide. The thumb centre travels from
// trackX + thumbW / 2 to trackX + trackW - thumbW / 2; the value is the nearest of the eight steps to that position (rounded half up), so the thumb
// sits under the cursor at every step and a press on the thumb never jumps. A track with no travel (trackW <= thumbW) always gives kSpeedMin.
inline int speedFromX(int px, int trackX, int trackW, int thumbW) {
    const int travel = trackW - thumbW;
    if (travel <= 0) return kSpeedMin;
    int frac = px - (trackX + thumbW / 2);
    if (frac < 0) frac = 0;
    if (frac > travel) frac = travel;
    const int steps = kSpeedMax - kSpeedMin;
    return kSpeedMin + (2 * steps * frac + travel) / (2 * travel);
}

// Left edge of the thumb for a speed value (the inverse of speedFromX, up to the integer rounding of the thumb position). Integer math only.
inline int speedThumbX(int value, int trackX, int trackW, int thumbW) {
    const int travel = trackW - thumbW;
    if (travel <= 0) return trackX;
    return trackX + (speedClamp(value) - kSpeedMin) * travel / (kSpeedMax - kSpeedMin);
}

// The value label, e.g. "Speed 4x" (the value is clamped first).
inline void speedText(char* out, size_t cap, int value) {
    if (!out || cap == 0) return;
    snprintf(out, cap, "Speed %dx", speedClamp(value));
}

// The short form of the label, e.g. "4x": what a narrow label box shows instead of a "Spe..." cut that would lose the number.
inline void speedTextShort(char* out, size_t cap, int value) {
    if (!out || cap == 0) return;
    snprintf(out, cap, "%dx", speedClamp(value));
}

// The hover help of the whole slider (track, thumb and label), shown in the right description pane.
inline const char* speedTip() {
    return "Holster and draw speed, in the preview and in game: 1x is normal, 8x is eight times slower. Click or drag; saved to holster.ini at once. Takes effect at the next draw or stow.";
}

// One holster.ini value (the text after the '=', already cut and trimmed): an integer or a float, rounded to the nearest step and clamped to 1..8.
// Anything unreadable gives kSpeedDefault.
inline int speedFromValueText(const char* text, size_t len) {
    char tmp[32];
    if (!text || len == 0) return kSpeedDefault;
    if (len >= sizeof tmp) len = sizeof tmp - 1;
    memcpy(tmp, text, len);
    tmp[len] = '\0';
    char* end = nullptr;
    const double v = strtod(tmp, &end);
    if (end == tmp || !(v == v)) return kSpeedDefault;   // not a number (also NaN)
    if (v < static_cast<double>(kSpeedMin)) return kSpeedMin;
    if (v > static_cast<double>(kSpeedMax)) return kSpeedMax;
    return speedClamp(static_cast<int>(v + 0.5));
}

// slow_holster_x of a whole holster.ini buffer (flat key, case-insensitive, the last line wins, a commented line is absent). An inline comment
// (';' after a blank) ends the value. Absent or unreadable gives kSpeedDefault.
inline int speedFromIniBuf(const char* in, size_t n) {
    if (!in || n == 0) return kSpeedDefault;
    const iniw::detail::Scan s = iniw::detail::scan(in, n, nullptr, kSpeedIniKey);
    if (!s.matched) return kSpeedDefault;
    const char* line = in + s.lineStart;
    size_t from = s.eqOff + 1, to = s.lineBody;
    for (size_t k = from; k < to; ++k) {
        if (line[k] == ';' && k > from && (line[k - 1] == ' ' || line[k - 1] == '\t')) {
            to = k;
            break;
        }
    }
    while (to > from && (line[to - 1] == ' ' || line[to - 1] == '\t')) --to;
    while (from < to && (line[from] == ' ' || line[from] == '\t')) ++from;
    return speedFromValueText(line + from, to - from);
}

// Reads the file (up to kSpeedIniCap bytes) and returns its Speed value. *bytes = the bytes read, -1 when the file cannot be opened (then kSpeedDefault).
// Reads only; opening the page never writes.
inline int speedReadIni(const char* path, int* bytes = nullptr) {
    static char buf[kSpeedIniCap];   // static: not on the stack
    FILE* f = path ? fopen(path, "rb") : nullptr;
    if (!f) {
        if (bytes) *bytes = -1;
        return kSpeedDefault;
    }
    const size_t n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    if (bytes) *bytes = static_cast<int>(n);
    return speedFromIniBuf(buf, n);
}

}  // namespace hse
