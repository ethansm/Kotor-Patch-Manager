// Inventory 3D Viewport -- pure logic extracted from Inventory3DViewport.cpp so it can be unit-tested on the host
// (inv3d_logic_test.cpp, doc 20 T1). No Win32 types: coordinates are plain int/long/float, and the DLL converts at the
// call sites. Every function is a behaviour-identical lift of the code that used to sit inline in the .cpp (constants,
// rounding, < vs <=, float vs int arithmetic all copied verbatim); the .cpp keeps all logging, file I/O and engine access.
//
// Needs only the C library: <string.h> <stdio.h> <stdlib.h> (strtof, strtol) <ctype.h> <math.h>, plus Inv3dQuat.h (Q, qrotate,
// vcross) and the two generated tables ItemFitTable.inc / RigCamera.inc (no Win32 types).
#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#ifndef _WIN32
#include <strings.h>
#endif
#include "Inv3dQuat.h"

namespace inv3d {

#include "ItemFitTable.inc"   // struct FitEntry, kFit[], kFitCount (sorted by (row,var))
#include "RigCamera.inc"      // kRigCamPos/Fwd/Up/Right: the inv3d_light rig's camera rest pose

// ---------------------------------------------------------------- configuration ($GAME/inv3d.ini)
struct Ov { bool set; float v[5]; };  // rx,ry,rz,pitch[,zoom]
struct Cfg {
    int enabled = 1; char rig[64] = "inv3d_light"; int onlyWeaponsArmor = 0;
    float spin = 70, dragDegPx = 0.5f, pitchClamp = 80, idleResume = 1.5f;
    int fitScale = 0; float margin = 1.15f, distMin = 0.25f, distMax = 4.0f;
    float fov = 22.726f; char fovAxis = 'v';
    float camPos[3], camFwd[3], camUp[3];
    float armorBox[6] = {0, 0, 0.95f, 0.6f, 0.4f, 1.9f}; int armorSpin = 1; int hitFlipY = 0;
    float screenW = 3840, screenH = 1600; bool screenExplicit = false;
    int bevels = 1; int wheelZoom = 1; float wheelStep = 0.9f, zoomDrag = 0.004f, rollDegPx = 0.5f, zoomMin = 0.3f, zoomMax = 3.0f;
    float fillRef = 0.6f, fillExp = 0.35f, fillMin = 0.4f;  // small items fill less of the view: fill=clamp((diag/ref)^exp, min, 1)
    Ov itype[256], row[256];
};

inline void cfgDefaults(Cfg* c) {
    *c = Cfg();
    for (int i = 0; i < 3; ++i) { c->camPos[i] = kRigCamPos[i]; c->camFwd[i] = kRigCamFwd[i]; c->camUp[i] = kRigCamUp[i]; }
    memset(c->itype, 0, sizeof(c->itype)); memset(c->row, 0, sizeof(c->row));
}
// F31: call right after fgets(line, n, f). If the physical line did not fit (buffer full, no '\n', more data follows),
// consume the rest of it and return true so the caller rejects the line ONCE instead of parsing the tail as a new line.
// A line of exactly n-1 chars (with its '\n' next, or at EOF) fits and returns false.
inline bool drainLongLine(char* line, size_t n, FILE* f) {
    if (n < 2 || strlen(line) != n - 1 || line[n - 2] == '\n') return false;
    int c = getc(f);
    if (c == '\r') { int d = getc(f); if (d == '\n' || d == EOF) return false; c = d; }   // CRLF after exactly n-1 chars fits
    if (c == EOF || c == '\n') return false;
    while (c != '\n' && c != EOF) c = getc(f);
    return true;
}
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
// Case-insensitive ASCII compare, 0 = equal (_stricmp on Windows, strcasecmp elsewhere).
inline int stricmp_(const char* a, const char* b) {
#ifdef _WIN32
    return _stricmp(a, b);
#else
    return strcasecmp(a, b);
#endif
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
    for (char* cm = v; *cm; ++cm) if (*cm == ';' || *cm == '#') { *cm = 0; break; }  // inline comment
    trim(k); trim(v);
    for (char* p = k; *p; ++p) *p = (char)tolower((unsigned char)*p);
    *key = k; *val = v;
    return IniLineKeyValue;
}

// Apply one key (already lower-cased) = value to `c`.
//   IniOk       accepted
//   IniBad      known key, value rejected (c unchanged for that key)
//   IniUnknown  no such key
enum IniResult { IniOk = 0, IniBad, IniUnknown };
inline IniResult parseIniLine(Cfg& c, const char* k, const char* v) {
    float a[6]; bool good = true;
    if (!strcmp(k, "enabled")) { good = parseFloats(v, a, 1); if (good) c.enabled = (int)a[0]; }
    else if (!strcmp(k, "rig")) { if (v[0] && strlen(v) < sizeof(c.rig)) strcpy(c.rig, v); else good = false; }
    else if (!strcmp(k, "only_weapons_armor")) { good = parseFloats(v, a, 1); if (good) c.onlyWeaponsArmor = (int)a[0]; }
    else if (!strcmp(k, "spin_deg_s")) { good = parseFloats(v, a, 1); if (good) c.spin = a[0]; }
    else if (!strcmp(k, "drag_deg_per_px")) { good = parseFloats(v, a, 1); if (good) c.dragDegPx = a[0]; }
    else if (!strcmp(k, "pitch_clamp")) { good = parseFloats(v, a, 1); if (good) c.pitchClamp = a[0]; }
    else if (!strcmp(k, "idle_resume_s")) { good = parseFloats(v, a, 1); if (good) c.idleResume = a[0]; }
    else if (!strcmp(k, "fit_mode")) { if (!stricmp_(v, "distance")) c.fitScale = 0; else if (!stricmp_(v, "scale")) c.fitScale = 1; else good = false; }
    else if (!strcmp(k, "fit_margin")) { good = parseFloats(v, a, 1); if (good) c.margin = a[0]; }
    else if (!strcmp(k, "dist_min")) { good = parseFloats(v, a, 1); if (good) c.distMin = a[0]; }
    else if (!strcmp(k, "dist_max")) { good = parseFloats(v, a, 1); if (good) c.distMax = a[0]; }
    else if (!strcmp(k, "fov_deg")) { good = parseFloats(v, a, 1); if (good) c.fov = a[0]; }
    else if (!strcmp(k, "fov_axis")) { if (!stricmp_(v, "v")) c.fovAxis = 'v'; else if (!stricmp_(v, "h")) c.fovAxis = 'h'; else good = false; }
    else if (!strcmp(k, "cam_pos")) { good = parseFloats(v, a, 3); if (good) memcpy(c.camPos, a, 12); }
    else if (!strcmp(k, "cam_fwd")) { good = parseFloats(v, a, 3); if (good) memcpy(c.camFwd, a, 12); }
    else if (!strcmp(k, "cam_up")) { good = parseFloats(v, a, 3); if (good) memcpy(c.camUp, a, 12); }
    else if (!strcmp(k, "armor_box")) { good = parseFloats(v, a, 6); if (good) memcpy(c.armorBox, a, 24); }
    else if (!strcmp(k, "armor_spin")) { good = parseFloats(v, a, 1); if (good) c.armorSpin = (int)a[0]; }
    else if (!strcmp(k, "hit_flip_y")) { good = parseFloats(v, a, 1); if (good) c.hitFlipY = (int)a[0]; }
    else if (!strcmp(k, "bevels")) { good = parseFloats(v, a, 1); if (good) c.bevels = (int)a[0]; }
    else if (!strcmp(k, "wheel_zoom")) { good = parseFloats(v, a, 1); if (good) c.wheelZoom = (int)a[0]; }
    else if (!strcmp(k, "wheel_step")) { good = parseFloats(v, a, 1); if (good) c.wheelStep = a[0]; }
    else if (!strcmp(k, "zoom_drag")) { good = parseFloats(v, a, 1); if (good) c.zoomDrag = a[0]; }
    else if (!strcmp(k, "roll_deg_per_px")) { good = parseFloats(v, a, 1); if (good) c.rollDegPx = a[0]; }
    else if (!strcmp(k, "zoom_min")) { good = parseFloats(v, a, 1); if (good) c.zoomMin = a[0]; }
    else if (!strcmp(k, "zoom_max")) { good = parseFloats(v, a, 1); if (good) c.zoomMax = a[0]; }
    else if (!strcmp(k, "fill_ref")) { good = parseFloats(v, a, 1); if (good) c.fillRef = a[0]; }
    else if (!strcmp(k, "fill_exp")) { good = parseFloats(v, a, 1); if (good) c.fillExp = a[0]; }
    else if (!strcmp(k, "fill_min")) { good = parseFloats(v, a, 1); if (good) c.fillMin = a[0]; }
    else if (!strcmp(k, "screen_w")) { good = parseFloats(v, a, 1); if (good) { c.screenW = a[0]; c.screenExplicit = true; } }
    else if (!strcmp(k, "screen_h")) { good = parseFloats(v, a, 1); if (good) { c.screenH = a[0]; c.screenExplicit = true; } }
    else if (!strncmp(k, "itype.", 6) || !strncmp(k, "row.", 4)) {
        bool isT = k[0] == 'i'; const char* num = k + (isT ? 6 : 4); char* e; long n = strtol(num, &e, 10);
        good = *num && !*e && n >= 0 && n < 256 && (parseFloats(v, a, 5) || (a[4] = 1.0f, parseFloats(v, a, 4)));
        if (good) { Ov& o = isT ? c.itype[n] : c.row[n]; o.set = true; memcpy(o.v, a, 20); }
    } else return IniUnknown;
    return good ? IniOk : IniBad;
}

// Range clamps applied once after the whole file is read. Returns a bit mask of the clamps that the caller logs (in this
// order): ClampMargin ("INI fit_margin out of range, reset 1.15"), ClampWheelStep ("... wheel_step ... reset 0.9"),
// ClampScreen ("INI screen_w/h invalid, reset 3840x1600"), ClampFov ("INI fov_deg out of range, reset 22.726"). The other
// clamps are silent.
enum { ClampMargin = 1, ClampWheelStep = 2, ClampScreen = 4, ClampFov = 8 };
inline int clampCfg(Cfg& c) {
    int logged = 0;
    if (c.margin < 1.0f || c.margin > 3) { logged |= ClampMargin; c.margin = 1.15f; }
    if (c.zoomMin < 0.05f) c.zoomMin = 0.05f;
    if (c.zoomMax < c.zoomMin) c.zoomMax = c.zoomMin;
    if (c.wheelStep < 0.5f || c.wheelStep > 0.99f) { logged |= ClampWheelStep; c.wheelStep = 0.9f; }
    if (c.fillRef < 0.05f) c.fillRef = 0.05f;
    if (c.fillMin < 0.1f || c.fillMin > 1) c.fillMin = 0.4f;
    if (c.pitchClamp < 0) c.pitchClamp = 0;
    if (c.idleResume < 0) c.idleResume = 0;
    if (c.screenW < 100 || c.screenH < 100) { logged |= ClampScreen; c.screenW = 3840; c.screenH = 1600; }
    if (c.distMin < 0.05f) c.distMin = 0.05f;
    if (c.distMax < c.distMin) c.distMax = c.distMin;
    if (c.fov < 5 || c.fov > 120) { logged |= ClampFov; c.fov = 22.726f; }
    return logged;
}

// ---------------------------------------------------------------- fit table lookup (sorted by row, var)
inline const FitEntry* fitLookup(int row, int var) {
    if (row < 0 || row > 255 || var < 0 || var > 255) return nullptr;   // F32: key packs row<<8|var; out-of-byte var would alias another row
    int lo = 0, hi = kFitCount - 1; unsigned key = ((unsigned)row << 8) | (unsigned)var;
    while (lo <= hi) {
        int m = (lo + hi) / 2; unsigned k = ((unsigned)kFit[m].row << 8) | kFit[m].var;
        if (k == key) return &kFit[m];
        if (k < key) lo = m + 1; else hi = m - 1;
    }
    return nullptr;
}

// ---------------------------------------------------------------- camera / fit
// Camera right vector = fwd x up.
inline void camBasis(const Cfg& cfg, float* right) { vcross(cfg.camFwd, cfg.camUp, right); }

// Everything computeFit works out; the caller stores D/s/fill into its Pres and logs the "  fit: ..." line from the rest
// (aspect is the value AFTER the aspect<0.1 -> 1 fallback, as the log line printed it).
struct FitResult { float ex, ey, ez, hs, vs, aspect, tanV, fill, need, D, s, d0; };
// base = item base orientation, e[3] = model-space bbox extent, defPitch / cat as in Pres.
inline void computeFit(const Cfg& cfg, const Q& base, const float* e, float defPitch, int cat, float aspect, FitResult* out) {
    // bbox after qBase: AABB of the 8 rotated corners
    float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
    for (int i = 0; i < 8; ++i) {
        float v[3] = {(i & 1 ? .5f : -.5f) * e[0], (i & 2 ? .5f : -.5f) * e[1], (i & 4 ? .5f : -.5f) * e[2]}, r[3];
        qrotate(base, v, r);
        for (int k = 0; k < 3; ++k) { if (r[k] < lo[k]) lo[k] = r[k]; if (r[k] > hi[k]) hi[k] = r[k]; }
    }
    float ex = hi[0] - lo[0], ey = hi[1] - lo[1], ez = hi[2] - lo[2];
    float hs, vs;
    if (defPitch == 0) { hs = sqrtf(ex * ex + ey * ey); vs = ez; }
    else { hs = vs = sqrtf(ex * ex + ey * ey + ez * ez); }
    if (aspect < 0.1f) aspect = 1;
    float t = tanf(cfg.fov * 3.14159265f / 360.0f);
    float tanV = cfg.fovAxis == 'v' ? t : t / aspect;
    float need = cfg.margin * fmaxf(vs / (2 * tanV), hs / (2 * tanV * aspect));
    float diag = sqrtf(ex * ex + ey * ey + ez * ez);
    float fill = 1;
    if (cat != 4) fill = fminf(1.0f, fmaxf(cfg.fillMin, powf(diag / cfg.fillRef, cfg.fillExp)));
    need /= fill;
    float d0 = -(cfg.camPos[0] * cfg.camFwd[0] + cfg.camPos[1] * cfg.camFwd[1] + cfg.camPos[2] * cfg.camFwd[2]);
    float s = 1, D;
    if (cfg.fitScale) {
        D = d0;
        if (need > 1e-4f) s = d0 / need;
        s = fminf(fmaxf(s, 0.01f), 100.0f);
    } else {
        D = need;
        if (D < cfg.distMin) D = cfg.distMin;
        if (D > cfg.distMax) { s = cfg.distMax / need; D = cfg.distMax; }
    }
    out->ex = ex; out->ey = ey; out->ez = ez; out->hs = hs; out->vs = vs; out->aspect = aspect; out->tanV = tanV;
    out->fill = fill; out->need = need; out->D = D; out->s = s; out->d0 = d0;
}

// ---------------------------------------------------------------- cursor / hit rect
// Client-area pixel -> engine UI space. Precondition cw >= 1 && ch >= 1 (callers check). Same float arithmetic as the
// inline `pt.x * screenW / r.right` it replaces.
inline void clientToEngine(long px, long py, long cw, long ch, float screenW, float screenH, float* x, float* y) {
    *x = px * screenW / cw; *y = py * screenH / ch;
}
// Top edge of the 3D view's hit box: the extent's Y as-is, or flipped against the engine height (hit_flip_y=1).
// Same left-to-right float arithmetic as the inline `screenH - ext[1] - ext[3]`.
inline float flipTop(int flipY, float screenH, int extY, int extH) {
    return flipY ? screenH - extY - extH : (float)extY;
}
// Half-open box test with an all-integer rect (the wheel hook's g_viewRect): left/top inclusive, right/bottom exclusive;
// the right/bottom edges are summed in integer arithmetic BEFORE the float compare, as the inline code did.
inline bool rectContains(float x, float y, long rx, long ry, long rw, long rh) {
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}
// Same, for the drag tick: integer left/width/height but a float top (flipTop); the bottom edge is a float sum.
inline bool rectContainsTop(float x, float y, int rx, float top, int rw, int rh) {
    return x >= rx && x < rx + rw && y >= top && y < top + rh;
}

}  // namespace inv3d
