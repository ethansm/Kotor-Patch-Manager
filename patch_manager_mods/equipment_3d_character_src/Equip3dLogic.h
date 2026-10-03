// Equipment 3D Character -- pure logic extracted from EquipCharPreview.cpp so it can be unit-tested on the host
// (equip3d_logic_test.cpp, doc 20 T1). No Win32 types: coordinates are plain int/long/float, and the DLL converts at the
// call sites. Every function is a behaviour-identical lift of the expression that used to sit inline in the .cpp
// (constants, rounding, < vs <=, float vs int arithmetic all copied verbatim); the .cpp keeps all logging and engine access.
//
// Needs only the C library: <string.h> <stdio.h> <math.h> and, for the INI parser that already used them, <stdlib.h>
// (strtof) and <ctype.h> (isspace, tolower). EquipWeaponTable.inc is generated and has no Win32 types.
#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

namespace eq3d {

#include "EquipWeaponTable.inc"   // kWeaponRows, struct WeaponRow, kWeapon[]: baseitems row -> itemclass + weaponwield

// ---------------------------------------------------------------- configuration ($GAME/equip3d.ini)
struct Cfg {
    int enabled = 1, stage = 5;
    char rig[64] = "charrec_light";
    float fov = 22.726f;
    int bevels = 1;
    float dragDegPx = 0.5f; int wheelZoom = 1; float wheelStep = 0.9f, zoomMin = 0.6f, zoomMax = 2.5f;
    int hitFlipY = 0; float screenW = 3840, screenH = 1600; bool screenExplicit = false;
    int flourish = 1; float flMin = 12, flMax = 25, flFirst = 6, flQuiet = 3; int flOnEquip = 1;
    char flStance[512] = "";   // optional flourish list used while a weapon stance is on (empty = same list)
    char flList[512] = "shrug,nodyes,salute,bow,greeting,touchheart,point,rolleyes,scanning,victory,pausesh";
    char equipAnim[64] = "itemequip";
    int pollMs = 100, rebuildMinMs = 250, keepObj10 = 0, helmet = 1, buttons = 1; float spinToggleDegS = 20;
    float spinDegS = 0;
};

inline void trim(char* s) {
    char* p = s; while (*p && isspace((unsigned char)*p)) ++p;
    memmove(s, p, strlen(p) + 1);
    for (int n = (int)strlen(s); n > 0 && isspace((unsigned char)s[n - 1]); --n) s[n - 1] = 0;
}
inline bool parseFloats(const char* v, float* out, int n) {
    const char* p = v;
    for (int i = 0; i < n; ++i) {
        char* e; out[i] = strtof(p, &e);
        if (e == p || !isfinite(out[i])) return false;
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

// Apply one key (already lower-cased) = value to `c`.
//   IniOk           accepted
//   IniBad          known key, value rejected (c unchanged for that key)
//   IniUnknown      no such key
//   IniOkKeepObj10  accepted, and the DEPRECATED keep_obj10 was set non-zero (the .cpp logs the warning before the "INI k = v" line)
enum IniResult { IniOk = 0, IniBad, IniUnknown, IniOkKeepObj10 };
inline IniResult parseIniLine(Cfg& c, const char* k, const char* v) {
    float a[2]; bool good = true;
    if (!strcmp(k, "enabled")) { good = parseFloats(v, a, 1); if (good) c.enabled = (int)a[0]; }
    else if (!strcmp(k, "stage")) { good = parseFloats(v, a, 1); if (good) c.stage = (int)a[0]; }
    else if (!strcmp(k, "rig")) { if (v[0] && strlen(v) < sizeof(c.rig)) strcpy(c.rig, v); else good = false; }
    else if (!strcmp(k, "fov_deg")) { good = parseFloats(v, a, 1); if (good) c.fov = a[0]; }
    else if (!strcmp(k, "bevels")) { good = parseFloats(v, a, 1); if (good) c.bevels = (int)a[0]; }
    else if (!strcmp(k, "drag_deg_per_px")) { good = parseFloats(v, a, 1); if (good) c.dragDegPx = a[0]; }
    else if (!strcmp(k, "wheel_zoom")) { good = parseFloats(v, a, 1); if (good) c.wheelZoom = (int)a[0]; }
    else if (!strcmp(k, "wheel_step")) { good = parseFloats(v, a, 1); if (good) c.wheelStep = a[0]; }
    else if (!strcmp(k, "zoom_min")) { good = parseFloats(v, a, 1); if (good) c.zoomMin = a[0]; }
    else if (!strcmp(k, "zoom_max")) { good = parseFloats(v, a, 1); if (good) c.zoomMax = a[0]; }
    else if (!strcmp(k, "hit_flip_y")) { good = parseFloats(v, a, 1); if (good) c.hitFlipY = (int)a[0]; }
    else if (!strcmp(k, "screen_w")) { good = parseFloats(v, a, 1); if (good) { c.screenW = a[0]; c.screenExplicit = true; } }
    else if (!strcmp(k, "screen_h")) { good = parseFloats(v, a, 1); if (good) { c.screenH = a[0]; c.screenExplicit = true; } }
    else if (!strcmp(k, "flourish")) { good = parseFloats(v, a, 1); if (good) c.flourish = (int)a[0]; }
    else if (!strcmp(k, "flourish_min_s")) { good = parseFloats(v, a, 1); if (good) c.flMin = a[0]; }
    else if (!strcmp(k, "flourish_max_s")) { good = parseFloats(v, a, 1); if (good) c.flMax = a[0]; }
    else if (!strcmp(k, "flourish_first_s")) { good = parseFloats(v, a, 1); if (good) c.flFirst = a[0]; }
    else if (!strcmp(k, "flourish_quiet_s")) { good = parseFloats(v, a, 1); if (good) c.flQuiet = a[0]; }
    else if (!strcmp(k, "flourish_on_equip")) { good = parseFloats(v, a, 1); if (good) c.flOnEquip = (int)a[0]; }
    else if (!strcmp(k, "flourish_stance_list")) { if (strlen(v) < sizeof(c.flStance)) strcpy(c.flStance, v); else good = false; }
    else if (!strcmp(k, "flourish_list")) { if (strlen(v) < sizeof(c.flList)) strcpy(c.flList, v); else good = false; }
    else if (!strcmp(k, "equip_anim")) { if (v[0] && strlen(v) < sizeof(c.equipAnim)) strcpy(c.equipAnim, v); else good = false; }
    else if (!strcmp(k, "poll_ms")) { good = parseFloats(v, a, 1); if (good) c.pollMs = (int)a[0]; }
    else if (!strcmp(k, "buttons")) { good = parseFloats(v, a, 1); if (good) c.buttons = (int)a[0]; }
    else if (!strcmp(k, "spin_toggle_deg_s")) { good = parseFloats(v, a, 1); if (good) c.spinToggleDegS = a[0]; }
    else if (!strcmp(k, "keep_obj10")) { good = parseFloats(v, a, 1); if (good) c.keepObj10 = (int)a[0]; if (good && c.keepObj10) return IniOkKeepObj10; }
    else if (!strcmp(k, "helmet")) { good = parseFloats(v, a, 1); if (good) c.helmet = (int)a[0]; }
    else if (!strcmp(k, "rebuild_min_ms")) { good = parseFloats(v, a, 1); if (good) c.rebuildMinMs = (int)a[0]; }
    else if (!strcmp(k, "spin_deg_s")) { good = parseFloats(v, a, 1); if (good) c.spinDegS = a[0]; }
    else return IniUnknown;
    return good ? IniOk : IniBad;
}

// Range clamps applied once after the whole file is read. Returns a bit mask of the two clamps that RESET to a default and
// are logged by the caller: ClampFov ("INI fov_deg out of range, reset 22.726"), ClampWheelStep ("... wheel_step ... reset 0.9").
enum { ClampFov = 1, ClampWheelStep = 2 };
inline int clampCfg(Cfg& c) {
    int logged = 0;
    if (c.stage < 0) c.stage = 0;
    if (c.stage > 5) c.stage = 5;
    if (c.fov < 5 || c.fov > 90) { logged |= ClampFov; c.fov = 22.726f; }
    if (c.wheelStep < 0.5f || c.wheelStep > 0.99f) { logged |= ClampWheelStep; c.wheelStep = 0.9f; }
    if (c.zoomMin < 0.2f) c.zoomMin = 0.2f;
    if (c.zoomMax < c.zoomMin) c.zoomMax = c.zoomMin;
    if (c.flMin < 2) c.flMin = 2;
    if (c.flMax < c.flMin) c.flMax = c.flMin;
    if (c.pollMs < 16) c.pollMs = 16;
    if (c.screenW < 100 || c.screenH < 100) { c.screenW = 3840; c.screenH = 1600; }
    return logged;
}

// ---------------------------------------------------------------- weapons
// Engine wield-style rule (R7f 0076d590 / R7h 00863650). Only the weapons we actually attached count.
inline constexpr const char* kStyleAnim[10] = {"", "g1r1", "g2r1", "g3r1", "g4r1", "g5r1", "g6r1", "g7r1", "g8r1", "g7r1"};
inline int styleFor(int rWield, int lWield, bool rHas, bool lHas) {
    if (rHas && lHas) return lWield == 2 ? 4 : (lWield == 4 ? 6 : 0);
    if (rHas) { static const int map[7] = {0, 1, 2, 3, 5, 7, 9}; return (rWield >= 1 && rWield <= 6) ? map[rWield] : 0; }
    return 0;   // left only, or nothing (unarmed style 8 deliberately not applied: keep the normal idle)
}
// baseitems row + variation -> weapon model ResRef "<itemclass>_<var:03d>" and the row's weaponwield. false (and nothing
// written) when the row is out of range or not a wieldable weapon (weaponwield 0).
inline bool weaponModelName(int base, int var, char* out, size_t n, int* wield) {
    if (base < 0 || base >= kWeaponRows || !kWeapon[base].wield) return false;
    snprintf(out, n, "%s_%03d", kWeapon[base].cls, var);
    *wield = kWeapon[base].wield;
    return true;
}

// ---------------------------------------------------------------- idle / droids
inline bool isDroidRow(int r) { return r == 2 || r == 57 || r == 231 || r == 232 || r == 453 || r == 3 || r == 538 || r == 539; }
// Idle name as 0077ec20 picks it (evil <40, neutral <60, else good; droids: pause1) - verified by the anim length probe.
inline const char* idleFor(int appearanceRow, short align) {
    return isDroidRow(appearanceRow) ? "pause1" : (align < 40 ? "evil" : (align < 60 ? "neutral" : "good"));
}

// ---------------------------------------------------------------- camera / input
// Engine framebuffer size sanity (Aspyr globals 0x009F42A4/0x009F42A8).
inline bool screenValid(int w, int h) { return w >= 320 && w <= 16384 && h >= 200 && h <= 16384; }

// Client-area pixel -> engine UI space. Precondition cw >= 1 && ch >= 1 (callers check). Same float arithmetic as the
// inline `pt.x * screenW / r.right` it replaces.
inline void clientToEngine(long px, long py, long cw, long ch, float screenW, float screenH, float* x, float* y) {
    *x = px * screenW / cw; *y = py * screenH / ch;
}
// Half-open box test: left/top inclusive, right/bottom exclusive. (All five inline copies in the .cpp were this.)
inline bool rectContains(float x, float y, float rx, float ry, float rw, float rh) {
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}
// Top edge of the 3D view's hit box: the extent's Y as-is, or flipped against the engine height (hit_flip_y=1).
inline float hitTop(int flipY, float screenH, int extY, int extH) {
    return flipY ? screenH - extY - extH : (float)extY;
}
inline float yawDrag(float yaw, float cx, float lastX, float dragDegPx) { return yaw + (cx - lastX) * dragDegPx; }
// Wheel (+120 per notch up = zoom in) -> new zoom, clamped to [zoomMin, zoomMax].
inline float zoomStep(float zoom, long wheel, float wheelStep, float zoomMin, float zoomMax) {
    zoom *= powf(wheelStep, -wheel / 120.0f);
    return fminf(fmaxf(zoom, zoomMin), zoomMax);
}
inline float fovForZoom(float fov, float zoom) { return fov / zoom; }
inline void yawVec(float yawDeg, float v[3]) {
    float rad = yawDeg * 0.0174532925f;
    v[0] = cosf(rad); v[1] = sinf(rad); v[2] = 0.f;
}

// ---------------------------------------------------------------- toolbar / pop-up layout
struct Box { int x, y, w, h; };

// Spin row: a single Spin button, or (after the first click) play/pause 30 % + Reset on the same row.
// Returns whether Reset is shown; *spin / *reset are the engine rects (reset untouched when hidden).
inline bool layoutSpin(Box row, bool spinUi, Box* spin, Box* reset) {
    int x = row.x, y = row.y, W = row.w, H = row.h;
    if (!spinUi) { *spin = Box{x, y, W, H}; return false; }
    int w1 = (int)(W * 0.30f), gp = (int)(W * 0.04f);
    *spin = Box{x, y, w1, H}; *reset = Box{x + w1 + gp, y, W - w1 - gp, H};
    return true;
}

constexpr int MenuRows = 8;   // items per pop-up column
constexpr const char* kPageToggle = "@page";
// Vertical pitch of the pop-up rows: half the distance between toolbar buttons 0 and 2 when both rects exist and 2 is below 0,
// else 1.2 x the Emotes button height.
inline float menuPitch(bool haveBoth, float y0, float y2, float h) {
    return (haveBoth && y2 - y0 > 0) ? (y2 - y0) / 2 : h * 1.2f;
}
struct MenuGeom { float left, top, gap; };
struct MenuPos { int col, row, x, y; };
// Emotes pop-up grid, anchored at the view's LEFT edge below the top arc; margins keep clear of the curved cut-outs.
// Items fill column 0 rows 0..MenuRows-2 then overflow into columns 1.. (MenuRows per column); the "@page" toggle item is
// always column 0, last row. out[i] for i < min(count, maxOut). vh <= 0 skips the "fit to view" shift.
template <int N>
inline MenuGeom layoutMenu(float w, float pitch, int vx, int vy, int vh, int count, const char (*anim)[N], int maxOut, MenuPos* out) {
    MenuGeom g;
    g.gap = w * 0.08f;
    g.left = vx + w * 0.10f;
    g.top = vy + vh * 0.12f;
    float bottom = vy + vh - vh * 0.13f;
    if (vh > 0 && g.top + MenuRows * pitch > bottom) g.top = fmaxf((float)vy, bottom - MenuRows * pitch);
    if (count > maxOut) count = maxOut;
    int j = 0;   // running index over the non-toggle items
    for (int i = 0; i < count; ++i) {
        int col, row;
        if (!strcmp(anim[i], kPageToggle)) { col = 0; row = MenuRows - 1; }
        else { if (j < MenuRows - 1) { col = 0; row = j; } else { col = 1 + (j - (MenuRows - 1)) / MenuRows; row = (j - (MenuRows - 1)) % MenuRows; } ++j; }
        out[i].col = col; out[i].row = row;
        out[i].x = (int)(g.left + col * (w + g.gap)); out[i].y = (int)(g.top + row * pitch);
    }
    return g;
}

}  // namespace eq3d
