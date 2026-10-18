// HolsterEditor.cpp -- holster-editor 0.3.2-dev (Steam build): a HOOKLESS consumer of the mod-config provider (k2.modconfig@1, k2.menuui@1).
// (doc 42 HW-EDIT, plan .claude/plans/holster_editor_options.md.)
//
// mod-config owns everything about the Options screen: the hooks, the root "Mod Configuration" button, the list page, control cloning,
// hover text, page navigation and freeing its controls. This patch registers ONE entry ("Holstered Weapons") and keeps only what is
// holster-specific:
//   build    (Options ctor window, vanilla layout open) opens the embedded HSE_VIEW layout, lets preview-3d bind the 3D view to it, restores
//            the panel's layout slot FIRST, then makes the page: 3D view (AddCustom, MUI_NOFREE), status label, member picker, animation controls, and Reset/Flip.
//   enter    places the view and the status label on the provider's VIEW and STATUS panes, lays out the left strip buttons, installs mouse wheel, and resets view to config.
//   tick     polls the preview-3d and holster clients, handles picker, anim and edit buttons (unsaved edits need a second click to discard), the mouse, and drives preview-3d and holster placement.
// Mouse (U2): nav mode is the equip-screen drag (LMB orbit/pan, MMB click resets, wheel zooms). Weapon edit mode runs hse::Chord (HseView.h):
//   LMB alone = orbit the character, RMB held + LMB drag = act on the weapon per the mode bar (Move / Depth / Tilt / Roll+Turn, latched at the
//   press), RMB + wheel = depth, MMB drag = spin, Shift = fine (x0.2), Ctrl = snap (1 cm / 5 deg, carried). RMB alone does nothing.
// Inspector (U3, weapon mode): six rows Slide/Depth/Height (offset x/y/z, cm) and Tilt/Roll/Turn (rotation x/y/z, deg). Each row is a scrub button whose
//   text is the live value ("Depth  -11.0 cm", "* " = differs from the value when the edit opened) plus an "R" button that puts that one value back.
//   LMB press on a row button + horizontal drag: 1 px = 1 mm or 0.5 deg, Shift x0.2, Ctrl snaps 1 cm / 5 deg (carried), SetValues with absolute values,
//   window +-0.30 m / +-180 deg around the value at the press, hard limits 3 m / 720 deg. The press is a DEAD press for hse::Chord (no orbit, no weapon
//   chord). A scrub is cancelled by foreground loss, Back/Esc/leave, a scope change or the edit closing. The open values are captured at edit open, on a
//   scope change and after Save / Revert / Reset Scope. The page holds 38 of mod-config's 40 items; the 15 strip rows share the strip height.
//   destroy  destroys the preview viewer (and, behind free_view, the view block once mod-config has unlinked it).
// A failed connect to mod-config means no menu entry, never a crash. KPatchInit does no file I/O: holster_editor.ini is read in build().
// Probe lines (log only): REGISTER, CONFIG, BUILD, EMBED, ENTER, DTOR, FREEVIEW, LAYOUT, REBUILD, NAME, REG, CAM, CAMHOOK, PROJ, CAMPTR, CAMRAW, CAMENG, CAMATT, CAMPROJ, CAMCHK, ITEMNAME, PICK, BTN, EDIT, STANCE, EMOTE, MMB, DRAG, WHEEL, PLAY_PAUSE, UNREG, LEAVE, DISCARD, CALIB,
// CHORD (state transitions with the reason, dropped presses, resets), MODE (mode bar), GESTURE (weapon gesture begin/end summary), SNAP (carry
// resets), NUDGE (WARN: a provider nudge that failed or had no effect), SCOPE (scope button text), HANDS (weapon button captions),
// INSP (U3 inspector: BUILD items=N/40, open values and the left-mirror source, RESET row from to, row-text refresh counts per second, WARN lines),
// SCRUB (U3: begin row/open value, end row/final value/frames, cancel with the reason, CLAMP window or hard limit, WARN SetValues failures).
// CALIB (U0b, ini probe_calib): a calibration click probe. While the page is LIVE in nav mode the status label asks for one body target at a time
// ("Calib 3/6 (round 1): click the right hand"); each LMB press inside the view logs the click (engine px, same space as ext) with the camera
// pose and the world positions of the body nodes, to fit the real camera offline. Lines: CALIB cfg/prompt/round=..click/skip/done/leave and CALIB WARN.
// CAMENG (U4, ini probe_cam): reads the engine camera object of the preview view (view+0x78: pose, fovy, viewport, attach behaviour) and calls its
// MapToScreen (camera vtable slot 0x28, pure math, only when the viewport is non-empty) for both hand nodes and the weapon mount, then compares the
// engine answer with the maths we assume (hse::eyeOf / rawPredict) and with hse::worldToScreen. Lines: CAMPTR, CAMRAW (once), CAMENG, CAMATT,
// CAMPROJ (R, L, M), CAMCHK per capture (at most kMaxCamEngCaptures, only when the view / edit state changed); CAMENG WARN for every failure path.
// Every probe failure path logs a "<PROBE> WARN <reason>" line (WARN level, at most kMaxWarnLogs per kind per Editor).
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "SafeRead.h"
#include "KpLog.h"
#include "K2Addr.h"
#include "SceneView.h"   // sceneview::camera(view): the preview view's engine camera object (CAMENG probe)
#include "GameAddr.h"
#include "GuiKit.h"
#include "EmbedGui.h"
#include "HolsterApi.h"
#include "Preview3dApi.h"
#include "ModConfigApi.h"
#include "ViewDrag.h"
#include "HseView.h"
#include "TextFit.h"
#include "HseParty.h"
#include "HseViewGui.inc"   // generated by scripts/gen_hse_layout.py: kHseViewGui[] (GFF "GUI " V3.2, HSE_VIEW with ID -1)

KP_LOG_PATCH("holster-editor");

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chEmbed, "embed");
KP_CHANNEL(chView, "view");

const bool g_fallbackSet = (KpLog_SetFallbackFile("holster-editor_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

namespace {

constexpr uint32_t kOptionsVtable = static_cast<uint32_t>(gameaddr::OptionsInGameVtable);
constexpr uint32_t kSceneViewVtable = static_cast<uint32_t>(gameaddr::SceneViewVtable);

struct StanceDef {
    const char* label;
    const char* anim;
};

const StanceDef kStanceList[] = {
    {"Idle", "pause1"},
    {"Walk", "walk"},
    {"Run", "run"},
    {"Walk (Rifle)", "walkrf"},
    {"Run (Rifle)", "runrf"},
    {"Alert", "pause2"},
    {"Combat", "pause3"},
};
constexpr int kStanceCount = sizeof(kStanceList) / sizeof(kStanceList[0]);

struct EmoteDef {
    const char* label;
    const char* anim;
    float duration;
};

const EmoteDef kEmoteList[] = {
    {"Salute", "salute", 1.6f},
    {"Bow", "bow", 2.0f},
    {"Greeting", "greeting", 1.5f},
    {"Victory", "victory", 1.3f},
    {"Taunt", "taunt", 1.4f},
    {"Shrug", "shrug", 1.6f},
    {"Nod", "nodyes", 1.5f},
    {"Point", "point", 2.0f},
    {"Shift Idle", "pausesh", 2.0f},
};
constexpr int kEmoteCount = sizeof(kEmoteList) / sizeof(kEmoteList[0]);

bool g_enabled = true;
bool g_freeView = false;   // also free the 3D view block after preview-3d destroys the creature (probe)
int g_probeCalib = 1;      // ini probe_calib: 1 = the CALIB click probe (U0b, log only; borrows the status label in nav mode), 0 = off
int g_probeCam = 1;        // ini probe_cam: 1 = the CAMENG engine camera probe (U4, log only), 0 = off
int g_probeNames = 1;      // ini probe_names: 0 = off (weapon buttons show the model resref), 1 = item GetName with identified=0 (the button name; ITEMNAME log), 2 = also identified=1 (log only)
float g_statusFit = 1.0f;  // ini status_fit: multiplier on the status pane width when fitting the status text (the label font is not measured)
float g_cfgYaw = 180.0f;
float g_cfgZoom = 2.2f;
float g_cfgPanZ = 0.0f;
hse::View g_cfgView = {180.0f, 2.2f, 0.0f};

k2p3::Client g_p3;
k2holster::Client g_hol;
k2mc::ConfigClient g_cfgClient;
k2mc::MenuClient g_menu;

enum State { IDLE, NEEDBUILD, LIVE, FAILED };
enum EditMode { MODE_NAV = 0, MODE_WEAPON = 1 };
// Actions guarded by the unsaved-edits two-click confirm.
enum GuardAction { ACT_NONE = 0, ACT_HAND_R, ACT_HAND_L, ACT_SCOPE, ACT_RESET_SCOPE, ACT_DONE, ACT_PREV, ACT_NEXT };
constexpr uint32_t kDiscardWindowMs = 2000;
constexpr float kPreviewFov = 22.726f;       // preview-3d kDefaultFov: the scene fov is p3d::fovForZoom(kPreviewFov, zoom) (applyView)
constexpr float kProjUpMetres = 0.3f;        // PROJ probe: second point is the mount + this far along world +Z
constexpr int kMaxWarnLogs = 20;             // cap on WARN lines per probe kind per Editor
constexpr int kMaxCalibSkips = 20;           // cap on CALIB skip lines per Editor
constexpr int kMaxCamEngCaptures = 10;       // cap on CAMENG captures per Editor
constexpr int kMaxProjLogs = 24;             // cap on CAMHOOK/PROJ re-logs per Editor
constexpr int kProjRetryTicks = 180;         // ticks to wait for GetMountWorld after an edit opens before giving up
constexpr int kMaxChordLogs = 300;           // cap on CHORD transition lines per Editor
constexpr int kMaxDropLogs = 30;             // cap on CHORD drop lines per Editor
constexpr int kMaxSnapLogs = 30;             // cap on SNAP carry-reset lines per Editor
constexpr int kMaxGestureLogs = 300;         // cap on GESTURE lines per Editor
constexpr int kMaxScrubLogs = 300;           // cap on SCRUB begin/end/cancel/clamp lines per Editor
constexpr int kMaxInspLogs = 120;            // cap on INSP refresh/reset/open lines per Editor
constexpr DWORD kInspLogWindowMs = 1000;     // INSP refresh counts are logged at most once per this window
constexpr DWORD kStatusHoldMs = 2500;        // weapon mode: the EDIT numbers stay on the status line this long after a change, then the help line returns
constexpr int kModeCount = hse::WPM_COUNT;
constexpr int kCrRightItemOff = 0x238, kCrLeftItemOff = 0x23C;   // client creature: equipped item ids (preview-3d CrRightItemOff/CrLeftItemOff)
constexpr uint32_t kNoItemId = 0x7F000000u;
constexpr int kItemModelResOff = 0x128;      // client item: model ResRef char[16]
constexpr int kItemBaseIdxOff = 0xC;         // client item: base item index (GetName's lookup branch dereferences its row)

enum WarnKind { W_ITEMNAME = 0, W_PROJ, W_CAMHOOK, W_CALIB, W_NUDGE, W_SCRUB, W_INSP, W_CAMENG, W_KINDS };
constexpr int kCalibNodes = 6;
// Candidate engine node names per calibration target (tried in order; the first that GetNodeWorld resolves is kept). Short names rhand/lhand are the
// ones the PROJ probe resolves; K2 skeletons also carry the *_g forms.
const char* const kCalibLabel[kCalibNodes] = {"head", "belt", "rhand", "lhand", "rfoot", "lfoot"};
const char* const kCalibCand[kCalibNodes][4] = {
    {"head_g", "head", "headhook", nullptr},
    {"pelvis_g", "torso_g", "torso", "pelvis"},
    {"rhand", "rhand_g", nullptr, nullptr},
    {"lhand", "lhand_g", nullptr, nullptr},
    {"rfoot_g", "rfoot", nullptr, nullptr},
    {"lfoot_g", "lfoot", nullptr, nullptr},
};

struct Editor {
    void* panel = nullptr;
    uint32_t page = 0;
    int32_t viewItem = -1, statusItem = -1;
    uintptr_t viewCtl = 0;     // the CSWGui3DSceneView control preview-3d bound to HSE_VIEW
    uint32_t token = 0;        // preview-3d viewer token
    int member = 0;
    hse::View view = {180.0f, 2.2f, 0.0f};
    hse::View cfgView = {180.0f, 2.2f, 0.0f};
    State state = IDLE;

    EditMode mode = MODE_NAV;
    int editHand = -1;  // 0=right, 1=left
    int editScope = K2HOLSTER_SCOPE_WEAPON;
    K2HolsterEditInfo editInfo = {};
    char wpnName[2][32] = {{0}, {0}};

    int32_t nameItem = -1;
    int32_t prevItem = -1, nextItem = -1;
    int32_t wpnRightItem = -1, wpnLeftItem = -1;
    int32_t stanceLabelItem = -1;
    int32_t stancePrevItem = -1, stanceNextItem = -1;
    int32_t emoteLabelItem = -1;
    int32_t emotePrevItem = -1, emoteNextItem = -1;
    int32_t playPauseItem = -1;
    int32_t boneItem = -1, scopeItem = -1;
    int32_t saveItem = -1, revertItem = -1;
    int32_t resetScopeItem = -1, doneItem = -1;
    int32_t resetItem = -1, flipItem = -1;
    int32_t modeItem[kModeCount] = {-1, -1, -1, -1};   // mode bar buttons (weapon mode only; parked off screen in nav)
    bool modeBarOk = false;                            // all four mode buttons exist; false = no bar and the mode stays Move
    int weaponMode = hse::WPM_MOVE;                     // selected mode; a running gesture keeps the mode it began with

    // U3 inspector: a scrub button (live value) and a reset button per row; weapon mode only, parked off screen in nav mode.
    int32_t inspItem[hse::IR_COUNT] = {-1, -1, -1, -1, -1, -1};
    int32_t inspResetItem[hse::IR_COUNT] = {-1, -1, -1, -1, -1, -1};
    bool inspOk = false;               // all 12 items exist; false = no inspector and the strip keeps the nine-row layout
    int itemsBuilt = 0;                // items the page holds after build (highest item id + 1)
    float openRot[3] = {0, 0, 0}, openOff[3] = {0, 0, 0};   // the values when the edit opened (or after the last Save / scope change): the reset target
    bool openValid = false;
    hse::Scrub scrub;                  // the running row scrub (pure state)
    hse::TextCache inspText;           // what each row button was last given: SetText only on a change
    bool inspForce = true;             // the next refresh rewrites every row text
    float inspSeenRot[3] = {0, 0, 0}, inspSeenOff[3] = {0, 0, 0};
    int inspSeenScope = -1;
    DWORD inspWinStart = 0;            // INSP refresh log window
    int inspWinSets = 0, inspWinTicks = 0;
    int scrubLogs = 0, inspLogs = 0;

    int stanceIdx = 0;
    int emoteIdx = 0;
    bool playingEmote = false;
    DWORD emoteEndMs = 0;

    p3d::Drag drag;
    p3d::DragCfg dragCfg;
    bool pressed = false;      // LMB was down on the previous tick
    bool pressedMmb = false;   // MMB was down on the previous tick
    bool pressedRmb = false;   // RMB was down on the previous tick
    hse::Chord chord;          // weapon-mode mouse state machine (HseView.h)
    hse::SnapAcc snap;         // Ctrl-snap remainders of the current weapon gesture (and the wheel); zeroed on begin, end and reset
    void* regGob[2] = {nullptr, nullptr};

    // Unsaved-edits two-click confirm
    bool discardArmed = false;
    int discardAction = ACT_NONE;
    DWORD discardArmedAt = 0;

    // CAM log state (per Editor; reset with the Editor in cbBuild)
    float camSeenYaw = -999.0f, camSeenZoom = -999.0f, camSeenPanZ = -999.0f;
    float camLogYaw = -999.0f, camLogZoom = -999.0f, camLogPanZ = -999.0f;
    DWORD camSeenMs = 0;
    int camLogs = 0;
    bool camHookPending = false;
    int projLogs = 0;           // CAMHOOK/PROJ lines logged (capped at kMaxProjLogs)
    bool projEditPending = false;   // an edit just opened: log the mount PROJ as soon as GetMountWorld answers
    int projEditTries = 0;

    // U4 CAMENG probe state (reset with the Editor): a capture runs only when this key changed
    int camEngCaps = 0;
    bool camEngKeyValid = false;
    float camEngKeyYaw = 0.0f, camEngKeyZoom = 0.0f, camEngKeyPanZ = 0.0f;
    uint32_t camEngKeyCam = 0;
    int camEngKeyEdit = 0;            // 0 = nav, 1 = right-hand edit, 2 = left-hand edit
    bool camEngCfgLogged = false;     // CAMPTR was logged once
    bool camEngDumped = false;        // CAMRAW was logged once
    bool camEngCapNoted = false;
    int cam2Logs = 0;                 // CAM2 line sets logged (capped at kMaxCam2Logs)

    // U0b CALIB probe state and per-kind WARN counters (reset with the Editor)
    int warnCount[W_KINDS] = {};
    hse::CalibScript calib;
    int calibNodeIdx[kCalibNodes] = {-1, -1, -1, -1, -1, -1};   // index into kCalibCand[i] that resolved, -1 = none yet
    unsigned calibMissWarned = 0;                               // bit i: the one-time "node not found" WARN for target i was logged
    int calibRoundClicks = 0;                                   // clicks recorded in the current round
    int calibSkips = 0;
    bool calibCfgLogged = false;
    hse::HseRect statusRect = {0, 0, 0, 0};                     // MUI_PANE_STATUS in UI units (the fit width of the calibration prompt)

    hse::HseRect strip = {0, 0, 0, 0};
    hse::HseRect viewRect = {0, 0, 0, 0};   // MUI_PANE_VIEW in UI units (the camera extent for the PROJ probe)

    // Weapon gesture summary (logged at the end) and chord logging caps
    bool gActive = false;
    int gMode = 0, gHand = 0, gFrames = 0;
    long gDx = 0, gDy = 0;
    float gA = 0.0f, gB = 0.0f;
    int chordLogs = 0, dropLogs = 0, snapLogs = 0, gestureLogs = 0;

    // Item display names (cache: CSWCItem::GetName is called once per member build, never per frame). Empty = fall back to the model resref.
    char itemName[2][64] = {{0}, {0}};

    // Status line: the EDIT numbers show while something changes, then the help line (weapon mode)
    DWORD statusHoldUntil = 0;
    bool statusHolding = false;   // the EDIT numbers are on the status line until statusHoldUntil
    int shownDirty = -1;
    float shownRot[3] = {0, 0, 0}, shownOff[3] = {0, 0, 0};
};
Editor g_ed;

void scrubCancel(Editor& e, const char* why);   // defined with the inspector helpers below
void exitEdit(Editor& e);                       // defined after enterEdit

void embedLog(const char* fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    KP_LOG(chEmbed, "EMBED %s", buf);
}

template <typename T>
bool rd(uintptr_t base, int off, T* out) { return saferead::readAt<T>(0xE70100u, base, off, out); }

int ctlFlagsOff() { return k2addr::off(&K2AddrTable::Ctl_Flags); }

int ctlId(uintptr_t ctl) {
    int id = -999;
    const int oId = k2addr::off(&K2AddrTable::Ctl_Id);
    if (ctl && oId) (void)rd<int>(ctl, oId, &id);
    return id;
}

bool iniBool(const char* v) { return !_stricmp(v, "1") || !_stricmp(v, "on") || !_stricmp(v, "true") || !_stricmp(v, "yes"); }

void readConfig() {
    g_enabled = true;   // defaults first, so a later Options open never keeps stale values from an earlier read
    g_freeView = false;
    g_probeNames = 1;
    g_probeCalib = 1;
    g_probeCam = 1;
    g_statusFit = 1.0f;
    g_cfgYaw = 180.0f;
    g_cfgZoom = 2.2f;
    g_cfgPanZ = 0.0f;
    FILE* f = fopen("holster_editor.ini", "r");
    if (!f) {
        g_cfgView = hse::makeView(g_cfgYaw, g_cfgZoom, g_cfgPanZ);
        return;
    }
    char line[256];
    while (fgets(line, sizeof line, f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == ';' || *p == '#' || *p == '[' || !*p) continue;
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = 0;
        char* v = eq + 1;
        size_t n = strlen(v);
        while (n && (v[n - 1] == '\r' || v[n - 1] == '\n' || v[n - 1] == ' ' || v[n - 1] == '\t')) v[--n] = 0;
        char* k = p;
        n = strlen(k);
        while (n && (k[n - 1] == ' ' || k[n - 1] == '\t')) k[--n] = 0;
        const bool on = iniBool(v);
        if (!_stricmp(k, "enabled")) g_enabled = on;
        else if (!_stricmp(k, "free_view")) g_freeView = on;
        else if (!_stricmp(k, "probe_names")) g_probeNames = (v[0] >= '0' && v[0] <= '9') ? atoi(v) : (on ? 1 : 0);
        else if (!_stricmp(k, "probe_calib")) g_probeCalib = on ? 1 : 0;
        else if (!_stricmp(k, "probe_cam")) g_probeCam = (v[0] >= '0' && v[0] <= '9') ? (atoi(v) != 0 ? 1 : 0) : (on ? 1 : 0);
        else if (!_stricmp(k, "status_fit")) {
            const float f = static_cast<float>(atof(v));
            g_statusFit = (f >= 0.5f && f <= 8.0f) ? f : 1.0f;
        }
        else if (!_stricmp(k, "view_yaw")) g_cfgYaw = static_cast<float>(atof(v));
        else if (!_stricmp(k, "view_zoom")) g_cfgZoom = static_cast<float>(atof(v));
        else if (!_stricmp(k, "view_panz")) g_cfgPanZ = static_cast<float>(atof(v));
    }
    fclose(f);
    g_cfgView = hse::makeView(g_cfgYaw, g_cfgZoom, g_cfgPanZ);
}

const char* statusText() {
    return g_p3.ok() && g_hol.ok() ? "0.3.2 dev: preview + holster connected" : "0.3.2 dev: a provider is missing";
}

bool editDirty();   // defined with the status helpers below

void KPATCH_CALL cbBeforeFree(uint32_t token, void* user) {
    saferead::beginScope();
    Editor* e = reinterpret_cast<Editor*>(user);
    if (!e || !g_p3.ok() || token != e->token) return;
    if (e->mode == MODE_WEAPON) {
        if (editDirty()) KP_LOG(chView, "DISCARD why=before_free");
        scrubCancel(*e, "before_free");
        if (g_hol.ok()) g_hol.h->EndEdit();
        e->mode = MODE_NAV;
        e->editHand = -1;
    }
    for (int hand = 0; hand < 2; ++hand) {
        void* g = g_p3.p->WeaponGob(token, hand);
        if (g && g_hol.ok()) {
            g_hol.h->UnregisterPreview(g);
        }
        e->regGob[hand] = nullptr;
        KP_LOG(chView, "UNREG hand=%d gob=%p", hand, g);
    }
}

// ---- k2.modconfig callbacks (cdecl, main thread) ----

// Options ctor window: the vanilla layout is open. Opens the embedded layout, binds the 3D view, restores the layout slot, builds the page.
void KPATCH_CALL cbBuild(void* panel, void*) {
    readConfig();
    KP_LOG(chMain, "CONFIG enabled=%d free_view=%d probe_names=%d probe_calib=%d probe_cam=%d status_fit=%.2f view=(%.2f,%.2f,%.2f)",
           g_enabled ? 1 : 0, g_freeView ? 1 : 0, g_probeNames, g_probeCalib, g_probeCam, g_statusFit,
           g_cfgView.yaw, g_cfgView.zoom, g_cfgView.panZ);
    if (!g_enabled || !panel || !g_menu.ok()) {
        KP_LOG(chMain, "BUILD skipped enabled=%d panel=%p menuui=%d", g_enabled ? 1 : 0, panel, g_menu.ok() ? 1 : 0);
        return;
    }
    saferead::beginScope();
    const uintptr_t P = reinterpret_cast<uintptr_t>(panel);
    uint32_t vt = 0, slot = 0;
    int flags48 = 0, count = 0;
    (void)rd<uint32_t>(P, 0, &vt);
    (void)rd<uint32_t>(P, embedgui::PanelLayoutSlot, &slot);
    (void)rd<int>(P, ctlFlagsOff(), &flags48);
    (void)rd<int>(P, k2addr::off(&K2AddrTable::Panel_CtlCount), &count);
    KP_LOG(chMain, "BUILD panel=%x vt=%x (expect %x) flags48=%x layout=%x controls=%d", static_cast<unsigned>(P), static_cast<unsigned>(vt),
           static_cast<unsigned>(kOptionsVtable), static_cast<unsigned>(flags48), static_cast<unsigned>(slot), count);
    if (g_ed.panel && g_ed.panel != panel) KP_LOG(chMain, "BUILD previous panel %p was not destroyed: state reset", g_ed.panel);
    g_ed = Editor();
    g_ed.cfgView = g_cfgView;
    g_ed.view = g_cfgView;
    g_ed.dragCfg.dragDegPx = 0.5f;
    g_ed.dragCfg.panPerPx = 0.003f;
    g_ed.dragCfg.lockPx = 4.0f;
    g_ed.dragCfg.panMin = -1.2f;
    g_ed.dragCfg.panMax = 1.2f;

    if (vt != kOptionsVtable || !(flags48 & 2) || !slot) { KP_LOG(chMain, "BUILD refused: not the open Options layout"); return; }
    g_ed.panel = panel;
    const bool p3 = g_p3.connect(), hol = g_hol.connect();
    KP_LOG(chMain, "BUILD providers preview3d=%d holster=%d", p3 ? 1 : 0, hol ? 1 : 0);

    // Embedded layout window: P+0x30 = our handle only for the preview's InitControl, then restored before anything else.
    embedgui::Handle h = embedgui::Open(reinterpret_cast<unsigned char*>(P), kHseViewGui, static_cast<unsigned>(sizeof kHseViewGui));
    KP_LOG(chEmbed, "EMBED open obj=%p saved=%p bytes=%u", h.obj, h.saved, static_cast<unsigned>(sizeof kHseViewGui));
    if (h.obj && p3) {
        g_ed.token = g_p3.p->CreateView(reinterpret_cast<void*>(P), "HSE_VIEW");
        g_ed.viewCtl = g_ed.token ? reinterpret_cast<uintptr_t>(g_p3.p->ViewControl(g_ed.token)) : 0;
        int after = 0;
        (void)rd<int>(P, k2addr::off(&K2AddrTable::Panel_CtlCount), &after);
        KP_LOG(chEmbed, "EMBED view token=%u ctl=%x id=%d (expect -1: not listed) controls=%d (was %d)", g_ed.token, static_cast<unsigned>(g_ed.viewCtl),
               ctlId(g_ed.viewCtl), after, count);
    }
    embedgui::Close(reinterpret_cast<unsigned char*>(P), h);
    uint32_t back = 0;
    (void)rd<uint32_t>(P, embedgui::PanelLayoutSlot, &back);
    KP_LOG(chEmbed, "EMBED restore layout=%x (expect %x) %s", static_cast<unsigned>(back), static_cast<unsigned>(slot), back == slot ? "ok" : "MISMATCH");
    if (back != slot) {   // never touch the layout again if the restore did not hold; no page is made
        if (g_ed.token && g_p3.ok()) g_p3.p->Destroy(g_ed.token);
        g_ed = Editor();
        return;
    }

    // The page: the 3D view (listed, shown and hidden with the page, never freed by mod-config) and the status label.
    const K2MenuUiApi* ui = g_menu.p;
    g_ed.page = ui->NewPage(panel, "Holstered Weapons");
    if (!g_ed.page) {
        KP_LOG(chMain, "BUILD NewPage failed");
        if (g_ed.token && g_p3.ok()) g_p3.p->Destroy(g_ed.token);
        g_ed = Editor();
        return;
    }
    if (g_ed.viewCtl) g_ed.viewItem = ui->AddCustom(g_ed.page, reinterpret_cast<void*>(g_ed.viewCtl), MUI_NOFREE);
    g_ed.statusItem = ui->AddLabel(g_ed.page, statusText());

    if (g_ed.token && g_p3.ok()) {
        g_p3.p->SetBeforeFree(cbBeforeFree, &g_ed);
        g_p3.p->SetWeapons(g_ed.token, 1);
    }

    g_ed.nameItem = ui->AddLabel(g_ed.page, "");
    g_ed.prevItem = ui->AddButton(g_ed.page, "<", "Show the previous party member.");
    g_ed.nextItem = ui->AddButton(g_ed.page, ">", "Show the next party member.");

    g_ed.wpnRightItem = ui->AddButton(g_ed.page, "Right: None", "Edit holster placement for right-hand weapon.");
    g_ed.wpnLeftItem = ui->AddButton(g_ed.page, "Left: None", "Edit holster placement for left-hand weapon.");

    char stanceBuf[64];
    snprintf(stanceBuf, sizeof(stanceBuf), "Stance: %s", kStanceList[g_ed.stanceIdx].label);
    g_ed.stanceLabelItem = ui->AddLabel(g_ed.page, stanceBuf);
    g_ed.stancePrevItem = ui->AddButton(g_ed.page, "<", "Previous stance (idle/walk/run).");
    g_ed.stanceNextItem = ui->AddButton(g_ed.page, ">", "Next stance (idle/walk/run).");

    char emoteBuf[64];
    snprintf(emoteBuf, sizeof(emoteBuf), "Emote: %s", kEmoteList[g_ed.emoteIdx].label);
    g_ed.emoteLabelItem = ui->AddLabel(g_ed.page, emoteBuf);
    g_ed.emotePrevItem = ui->AddButton(g_ed.page, "<", "Previous emote.");
    g_ed.emoteNextItem = ui->AddButton(g_ed.page, ">", "Next emote.");

    g_ed.playPauseItem = ui->AddButton(g_ed.page, "Play", "Play selected emote, or pause movement.");

    g_ed.boneItem = ui->AddButton(g_ed.page, "Bone: Back", "Cycle mounting bone (back/hip/thigh).");
    g_ed.scopeItem = ui->AddButton(g_ed.page, "For: this weapon", hse::scopeTip());
    g_ed.saveItem = ui->AddButton(g_ed.page, "Save", "Save placement to holster.ini under [editor].");
    g_ed.revertItem = ui->AddButton(g_ed.page, "Revert", "Revert unsaved edits back to saved values.");
    g_ed.resetScopeItem = ui->AddButton(g_ed.page, "Reset Scope", "Delete custom overrides for this scope.");
    g_ed.doneItem = ui->AddButton(g_ed.page, "Done", "Exit weapon edit mode.");

    g_ed.resetItem = ui->AddButton(g_ed.page, "Reset view", "Back to the saved framing.");
    g_ed.flipItem = ui->AddButton(g_ed.page, "Flip view", "Turn the character around.");

    // Mode bar (weapon mode): what RMB + LMB drag does to the weapon. Four more items; when the page is full the bar is off and the mode stays Move.
    static const char* const kModeDesc[kModeCount] = {
        "RMB + LMB drag moves the weapon across the screen. Shift is fine, Ctrl snaps to 1 cm.",
        "RMB + LMB drag pushes the weapon along its forward axis (drag up = forward). RMB + wheel does this in any mode.",
        "RMB + LMB drag tilts the weapon about its side axis. Shift is fine, Ctrl snaps to 5 degrees.",
        "RMB + LMB drag: across turns the weapon about its up axis, up or down rolls it about its forward axis.",
    };
    g_ed.modeBarOk = true;
    for (int i = 0; i < kModeCount; ++i) {
        g_ed.modeItem[i] = ui->AddButton(g_ed.page, hse::weaponModeName(i), kModeDesc[i]);
        if (g_ed.modeItem[i] < 0) g_ed.modeBarOk = false;
    }
    if (!g_ed.modeBarOk) KP_LOGV(chMain, KPLOG_WARN, "MODE WARN mode bar unavailable: AddButton failed (items %d,%d,%d,%d); the weapon drag stays Move",
                                 g_ed.modeItem[0], g_ed.modeItem[1], g_ed.modeItem[2], g_ed.modeItem[3]);

    // Inspector rows (U3): 12 more items. Item ids run 0.. in creation order, so the highest id + 1 is the count so far. The rows need the page to have
    // room for all 12; otherwise (or when any AddButton fails) the inspector is off and the strip keeps its nine rows.
    {
        int32_t maxId = -1;
        const int32_t base[] = {g_ed.viewItem, g_ed.statusItem, g_ed.nameItem, g_ed.prevItem, g_ed.nextItem, g_ed.wpnRightItem, g_ed.wpnLeftItem,
                                g_ed.stanceLabelItem, g_ed.stancePrevItem, g_ed.stanceNextItem, g_ed.emoteLabelItem, g_ed.emotePrevItem, g_ed.emoteNextItem,
                                g_ed.playPauseItem, g_ed.boneItem, g_ed.scopeItem, g_ed.saveItem, g_ed.revertItem, g_ed.resetScopeItem, g_ed.doneItem,
                                g_ed.resetItem, g_ed.flipItem, g_ed.modeItem[0], g_ed.modeItem[1], g_ed.modeItem[2], g_ed.modeItem[3]};
        for (int32_t it : base) if (it > maxId) maxId = it;
        const int before = static_cast<int>(maxId) + 1;
        if (before != hse::kPageItemsBase)
            KP_LOGV(chMain, KPLOG_WARN, "INSP WARN BUILD base item count %d, expected %d (an AddX failed above, or the base list changed)", before, hse::kPageItemsBase);
        if (before + hse::kInspItems > hse::kPageItemCap) {
            KP_LOGV(chMain, KPLOG_WARN, "INSP WARN BUILD no room for the inspector: %d items + %d > %d", before, hse::kInspItems, hse::kPageItemCap);
        } else {
            bool all = true;
            for (int r = 0; r < hse::IR_COUNT; ++r) {
                char first[48];
                hse::inspRowText(first, sizeof first, r, 0.0f, false, true);
                g_ed.inspItem[r] = ui->AddButton(g_ed.page, first, hse::inspTip(r));
                g_ed.inspResetItem[r] = ui->AddButton(g_ed.page, "R", hse::inspResetTip());
                if (g_ed.inspItem[r] < 0 || g_ed.inspResetItem[r] < 0) {
                    all = false;
                    KP_LOGV(chMain, KPLOG_WARN, "INSP WARN BUILD AddButton failed for row %s (scrub item %d, reset item %d)", hse::inspRowName(r), g_ed.inspItem[r],
                            g_ed.inspResetItem[r]);
                }
            }
            g_ed.inspOk = all;
        }
        int32_t last = maxId;
        for (int r = 0; r < hse::IR_COUNT; ++r) {
            if (g_ed.inspItem[r] > last) last = g_ed.inspItem[r];
            if (g_ed.inspResetItem[r] > last) last = g_ed.inspResetItem[r];
        }
        g_ed.itemsBuilt = static_cast<int>(last) + 1;
        KP_LOG(chMain, "BUILD items=%d/%d inspector=%s", g_ed.itemsBuilt, hse::kPageItemCap, g_ed.inspOk ? "on" : "OFF");
    }

    if (g_ed.nameItem < 0 || g_ed.prevItem < 0 || g_ed.nextItem < 0 ||
        g_ed.wpnRightItem < 0 || g_ed.wpnLeftItem < 0 ||
        g_ed.stanceLabelItem < 0 || g_ed.stancePrevItem < 0 || g_ed.stanceNextItem < 0 ||
        g_ed.emoteLabelItem < 0 || g_ed.emotePrevItem < 0 || g_ed.emoteNextItem < 0 ||
        g_ed.playPauseItem < 0 ||
        g_ed.boneItem < 0 || g_ed.scopeItem < 0 ||
        g_ed.saveItem < 0 || g_ed.revertItem < 0 ||
        g_ed.resetScopeItem < 0 || g_ed.doneItem < 0 ||
        g_ed.resetItem < 0 || g_ed.flipItem < 0) {
        KP_LOG(chMain, "BUILD item failed");
    }

    KP_LOG(chMain, "BUILD page=%u view item=%d status item=%d token=%u", g_ed.page, g_ed.viewItem, g_ed.statusItem, g_ed.token);
}

void placeItem(int32_t item, int32_t pane, const char* what) {
    if (item < 0) return;
    K2MuiRect r = {0, 0, 0, 0};
    const bool ok = g_menu.p->PaneRect(g_ed.panel, pane, &r) != 0;
    KP_LOG(chMain, "ENTER %s item=%d pane ok=%d (%d,%d,%d,%d)", what, item, ok ? 1 : 0, r.x, r.y, r.w, r.h);
    if (ok) g_menu.p->SetRect(g_ed.page, item, &r);
}

void placeRect(int32_t item, const hse::HseRect& hr) {
    if (item < 0 || !g_menu.ok() || !g_ed.page) return;
    K2MuiRect r = {hr.x, hr.y, hr.w, hr.h};
    g_menu.p->SetRect(g_ed.page, item, &r);
}

void readWeaponModel(void* gob, char* out, size_t cap) {
    if (!out || cap == 0) return;
    out[0] = '\0';
    if (!gob) return;
    uintptr_t model = 0;
    if (rd<uintptr_t>(reinterpret_cast<uintptr_t>(gob), 0x84, &model) && model) {
        size_t n = 0;
        for (; n < cap - 1 && n < 16; ++n) {
            char c = 0;
            if (!saferead::readAt(0xE70100u, model + 0x08, static_cast<int>(n), &c) || c == 0) break;
            out[n] = (c >= 0x20 && c <= 0x7E) ? c : '.';
        }
        out[n] = '\0';
    }
}

const char* handName(int hand) { return hand == 1 ? "L" : "R"; }

// Zeroes the Ctrl-snap remainders; a non-zero carry that is thrown away is logged (capped).
void resetSnap(Editor& e, const char* why) {
    if (!e.snap.nonzero()) return;
    if (e.snapLogs < kMaxSnapLogs) {
        ++e.snapLogs;
        KP_LOG(chView, "SNAP carry reset why=%s acc=(%.5f,%.5f,%.5f)", why, e.snap.a[0], e.snap.a[1], e.snap.a[2]);
    }
    e.snap.reset();
}

// GESTURE end line for the running weapon gesture (no-op when none runs).
void logGestureEnd(Editor& e, const char* why) {
    if (!e.gActive) return;
    e.gActive = false;
    if (e.gestureLogs < kMaxGestureLogs) {
        ++e.gestureLogs;
        KP_LOG(chView, "GESTURE end mode=%s hand=%s frames=%d total d=(%ld,%ld) applied=(%.4f,%.4f) why=%s", hse::weaponModeName(e.gMode), handName(e.gHand),
               e.gFrames, e.gDx, e.gDy, e.gA, e.gB, why);
    }
}

// Drops every drag in flight and re-reads the CURRENT button state into the "was down" flags, so a button that is still held (the click that
// entered or left edit mode) cannot start a drag until it is released and pressed again. cbLeave clears the flags outright instead.
void resetInput(Editor& e, const char* why = "reset") {
    const bool fg = guikit::g_gk.gameForeground ? guikit::g_gk.gameForeground() : true;
    if (e.chord.busy() && e.chordLogs < kMaxChordLogs) {
        ++e.chordLogs;
        KP_LOG(chView, "CHORD reset from=%s why=%s", hse::chordStateName(e.chord.state), why);
    }
    logGestureEnd(e, why);
    scrubCancel(e, why);   // a row scrub is dropped with every other drag (enter/exit edit, scope change, leave, provider lost, discard guard)
    p3d::dragEnd(&e.drag);
    e.chord.reset();
    resetSnap(e, why);
    e.pressed = fg && guikit::g_gk.lbuttonDown && guikit::g_gk.lbuttonDown();
    e.pressedRmb = fg && guikit::g_gk.rbuttonDown && guikit::g_gk.rbuttonDown();
    e.pressedMmb = fg && guikit::g_gk.mbuttonDown && guikit::g_gk.mbuttonDown();
}

// ---- U0 probes (log only): camera projection and equipped item names ----

struct CamInfo {
    float pos[3];
    float q[4];            // w,x,y,z (GetNodeWorld order)
    float fovy;            // vertical, degrees
    hse::HseRect ext;      // extent used for the projection
    hse::HseRect pane;     // MUI_PANE_VIEW
    hse::HseRect ctlExt;   // the view control's own extent (block +4..+0x10), or all zero when unreadable
    bool ok;
    const char* why;       // failure reason when !ok (static string), else ""
};

// True while this probe kind is under its WARN cap (and counts the line). Callers log with KP_LOGV(chView, KPLOG_WARN, ...).
bool warnOk(Editor& e, WarnKind k) {
    if (k < 0 || k >= W_KINDS) return false;
    if (e.warnCount[k] >= kMaxWarnLogs) return false;
    ++e.warnCount[k];
    return true;
}

bool readCam(Editor& e, CamInfo& c) {
    c.pos[0] = c.pos[1] = c.pos[2] = 0.0f;
    c.q[0] = 1.0f; c.q[1] = c.q[2] = c.q[3] = 0.0f;   // w-first identity
    c.fovy = p3d::fovForZoom(kPreviewFov, e.view.zoom > 0.0f ? e.view.zoom : 1.0f);   // same as preview-3d applyView
    c.pane = e.viewRect;
    c.ctlExt = {0, 0, 0, 0};
    c.ext = c.pane;
    c.ok = false;
    c.why = "";
    if (e.viewCtl) {
        int v[4] = {0, 0, 0, 0};
        bool all = true;
        for (int i = 0; i < 4; ++i) all = rd<int>(e.viewCtl, 4 + i * 4, &v[i]) && all;
        if (all) {
            c.ctlExt = {v[0], v[1], v[2], v[3]};
            if (v[2] > 0 && v[3] > 0) c.ext = c.ctlExt;
        }
    }
    if (!g_p3.ok() || !g_hol.ok() || !e.token) {
        c.why = !g_p3.ok() ? "preview3d provider missing" : (!g_hol.ok() ? "holster provider missing" : "no viewer token");
        return false;
    }
    void* rig = g_p3.p->RigGob(e.token, 0);
    if (!rig) { c.why = "RigGob null"; return false; }
    c.ok = g_hol.h->GetNodeWorld(rig, "camerahook", c.pos, c.q) != 0;
    if (!c.ok) c.why = "GetNodeWorld(camerahook) false";
    return c.ok;
}

void logProj(const CamInfo& c, const char* hand, const char* src, const float m[3]) {
    const float up[3] = {m[0], m[1], m[2] + kProjUpMetres};
    const hse::WorldScreen a = hse::worldToScreen(c.pos, c.q, c.fovy, c.ext, m);
    const hse::WorldScreen b = hse::worldToScreen(c.pos, c.q, c.fovy, c.ext, up);
    KP_LOG(chView, "PROJ hand=%s mount=(%.3f,%.3f,%.3f) s=(%.1f,%.1f) ok=%d up=(%.1f,%.1f) ok=%d depth=%.3f src=%s",
           hand, m[0], m[1], m[2], a.sx, a.sy, a.ok ? 1 : 0, b.sx, b.sy, b.ok ? 1 : 0, a.depth, src);
}

// CAMHOOK (camera pose, fov, extents) plus a PROJ line per hand node (rhand/lhand on the body rig: needs no open edit).
void logCamHook(Editor& e, const char* why) {
    CamInfo c;
    const bool ok = readCam(e, c);
    if (!ok && warnOk(e, W_CAMHOOK)) KP_LOGV(chView, KPLOG_WARN, "CAMHOOK WARN why=%s reason=%s", why, c.why);
    KP_LOG(chView, "CAMHOOK why=%s pos=(%.3f,%.3f,%.3f) q(w,x,y,z)=(%.4f,%.4f,%.4f,%.4f) fovy=%.3f zoom=%.3f yaw=%.2f ext=(%d,%d,%d,%d) pane=(%d,%d,%d,%d) ctl=(%d,%d,%d,%d) ok=%d",
           why, c.pos[0], c.pos[1], c.pos[2], c.q[0], c.q[1], c.q[2], c.q[3], c.fovy, e.view.zoom, e.view.yaw,
           c.ext.x, c.ext.y, c.ext.w, c.ext.h, c.pane.x, c.pane.y, c.pane.w, c.pane.h, c.ctlExt.x, c.ctlExt.y, c.ctlExt.w, c.ctlExt.h, ok ? 1 : 0);
    if (!ok) return;
    void* rig = g_p3.p->RigGob(e.token, 0);
    if (!rig && warnOk(e, W_PROJ)) KP_LOGV(chView, KPLOG_WARN, "PROJ WARN why=%s reason=RigGob null on second read", why);
    static const char* const kNode[2] = {"rhand", "lhand"};
    static const char* const kHand[2] = {"R", "L"};
    for (int h = 0; h < 2; ++h) {
        float p[3] = {0, 0, 0}, q[4] = {1, 0, 0, 0};
        if (rig && g_hol.h->GetNodeWorld(rig, kNode[h], p, q)) logProj(c, kHand[h], kNode[h], p);
        else if (warnOk(e, W_PROJ)) KP_LOGV(chView, KPLOG_WARN, "PROJ WARN hand=%s node=%s unavailable rig=%p", kHand[h], kNode[h], rig);
    }
}

// PROJ for the mount bone of the hand being edited (GetMountWorld answers only while an edit is open). True when logged.
bool logEditProj(Editor& e) {
    if (!g_hol.ok() || e.mode != MODE_WEAPON) {
        if (warnOk(e, W_PROJ)) KP_LOGV(chView, KPLOG_WARN, "PROJ WARN mount skipped: %s", !g_hol.ok() ? "holster provider missing" : "not in weapon edit");
        return false;
    }
    float mp[3] = {0, 0, 0}, mq[4] = {1, 0, 0, 0};
    if (!g_hol.h->GetMountWorld(mp, mq)) {
        // retried every tick for kProjRetryTicks: warn on the first try and every 60th only (the final "mount=none" line summarises)
        if (e.projEditTries % 60 == 0 && warnOk(e, W_PROJ)) KP_LOGV(chView, KPLOG_WARN, "PROJ WARN GetMountWorld false (try %d)", e.projEditTries);
        return false;
    }
    CamInfo c;
    if (!readCam(e, c)) {
        if (warnOk(e, W_PROJ)) KP_LOGV(chView, KPLOG_WARN, "PROJ WARN mount camera unreadable: %s", c.why);
        return false;
    }
    logProj(c, e.editHand == 1 ? "L" : "R", "mount", mp);
    return true;
}

// ---- U4 CAMENG probe (log only; research/holstered_weapons/ux_v2/u4/L_camera_probe_design.md) ----
// Reads the REAL engine camera object of the preview view (view+0x78: pose, fovy, viewport, attach behaviour), then calls its MapToScreen (camera vtable
// slot 0x28, pure math: no GL as long as the viewport is non-empty) for the hand nodes and the weapon mount, and compares the engine's answer with
// hse::eyeOf / hse::rawPredict (the maths we think the engine uses) and with hse::worldToScreen ("ours"). Nothing is written to the engine.
// Gated by ini probe_cam. Every failure path logs a rate-limited "CAMENG WARN <reason>" (W_CAMENG).
using MapToScreenFn = float*(__thiscall*)(void* cam, float* out, float x, float y, float z, char normalized);
constexpr int kBehOffOff = 0x3C;     // attach behaviour object: float[3] camera offset at +0x3C..+0x44 (D_camera_attach.md)
constexpr int kBehFlagOff = 0x48;    // attach behaviour object: flag (int)

template <typename T>
bool rdc(uintptr_t base, int off, T* out) { return saferead::readAt<T>(0xE70400u, base, off, out); }

bool vec3Finite(const float v[3]) { return hse::finiteF(v[0]) && hse::finiteF(v[1]) && hse::finiteF(v[2]); }

void camEngWarn(Editor& e, const char* fmt, ...) {
    if (!warnOk(e, W_CAMENG)) return;
    char buf[200];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    KP_LOGV(chView, KPLOG_WARN, "CAMENG WARN %s", buf);
}

// One tracked point of a capture: the hand nodes (R, L) and the weapon mount (M).
struct CamPt {
    char hand;
    float p[3];
};

// CAMPROJ for one world point: engine raw (normalized=0) and normalized (=1) answers against the predicted and ours. Returns the per-point deltas
// through dPx (max |eng - pred| in px) and dFrac (max |eng - ours| in viewport fractions); both stay -1 when the point is not comparable.
void camEngProject(Editor& e, const char* why, uintptr_t cam, uint32_t fn, const CamPt& pt, const float pos[3], const float q[4], float fovy, const int vp[4],
                   float* dPx, float* dFrac) {
    *dPx = -1.0f;
    *dFrac = -1.0f;
    float eye[3] = {0, 0, 0};
    const bool eyeOk = hse::eyeOf(pos, q, pt.p, eye);
    const bool front = eyeOk && hse::eyeInFront(eye);
    float pred[2] = {0, 0};
    const bool predOk = eyeOk && hse::rawPredict(eye, vp, fovy, pred);
    float outRaw[3] = {0, 0, 0}, outN[3] = {0, 0, 0};
    bool called = false;
    if (predOk) {   // ez != 0 and the viewport / fovy are valid: the engine call is pure math (behind-camera points mirror, like the render does not)
        MapToScreenFn f = reinterpret_cast<MapToScreenFn>(static_cast<uintptr_t>(fn));
        f(reinterpret_cast<void*>(cam), outRaw, pt.p[0], pt.p[1], pt.p[2], 0);
        f(reinterpret_cast<void*>(cam), outN, pt.p[0], pt.p[1], pt.p[2], 1);
        called = true;
    } else {
        camEngWarn(e, "hand=%c projection skipped: %s", pt.hand, eyeOk ? "prediction refused (ez == 0 or viewport / fovy invalid)" : "eye-space point not finite");
    }
    const hse::HseRect ext = {vp[0], vp[1], vp[2], vp[3]};
    const hse::WorldScreen ours = hse::worldToScreen(pos, q, fovy, ext, pt.p);
    const bool inside = called && front && outRaw[0] >= static_cast<float>(vp[0]) && outRaw[0] <= static_cast<float>(vp[0] + vp[2]) &&
                        outRaw[1] >= static_cast<float>(vp[1]) && outRaw[1] <= static_cast<float>(vp[1] + vp[3]);
    KP_LOG(chView, "CAMPROJ why=%s hand=%c p=(%.3f,%.3f,%.3f) eye=(%.4f,%.4f,%.4f) valid=%d eng_raw=(%.2f,%.2f) eng_n=(%.5f,%.5f) pred_raw=(%.2f,%.2f) ours=(%.1f,%.1f) inside=%d",
           why, pt.hand, pt.p[0], pt.p[1], pt.p[2], eye[0], eye[1], eye[2], front ? 1 : 0, outRaw[0], outRaw[1], outN[0], outN[1], pred[0], pred[1],
           ours.ok ? ours.sx : 0.0f, ours.ok ? ours.sy : 0.0f, inside ? 1 : 0);
    if (!called || !front) return;   // valid=0 is a state, not a failure; it stays out of CAMCHK
    if (outRaw[0] == 0.0f && outRaw[1] == 0.0f) camEngWarn(e, "hand=%c MapToScreen returned zero", pt.hand);
    *dPx = fmaxf(fabsf(outRaw[0] - pred[0]), fabsf(outRaw[1] - pred[1]));
    if (ours.ok) {
        float fe[2];
        hse::rawFrac(outRaw, vp, fe);
        const float fx = (ours.sx - static_cast<float>(vp[0])) / static_cast<float>(vp[2]);
        const float fy = (ours.sy - static_cast<float>(vp[1])) / static_cast<float>(vp[3]);
        *dFrac = fmaxf(fabsf(fe[0] - fx), fabsf(fe[1] - fy));
    }
}

// One CAMENG capture (CAMPTR / CAMRAW once, then CAMENG + CAMATT + CAMPROJ x(2..3) + CAMCHK). Runs only when the key (view, camera pointer, edit state)
// changed since the last capture, at most kMaxCamEngCaptures per Editor. Main thread only.
void logCamEng(Editor& e, const char* why) {
    if (!g_probeCam) return;
    saferead::beginScope();
    const uintptr_t view = e.viewCtl;
    uint32_t cam = 0, vt = 0;
    const bool vtOk = view && sceneview::camera(reinterpret_cast<void*>(view), &cam, &vt);
    const int editState = e.mode == MODE_WEAPON ? 1 + (e.editHand == 1 ? 1 : 0) : 0;
    if (e.camEngKeyValid && e.camEngKeyYaw == e.view.yaw && e.camEngKeyZoom == e.view.zoom && e.camEngKeyPanZ == e.view.panZ && e.camEngKeyCam == cam &&
        e.camEngKeyEdit == editState)
        return;
    if (e.camEngCaps >= kMaxCamEngCaptures) {
        if (!e.camEngCapNoted) {
            e.camEngCapNoted = true;
            KP_LOG(chView, "CAMENG cap reached (%d captures)", kMaxCamEngCaptures);
        }
        return;
    }
    e.camEngKeyValid = true;
    e.camEngKeyYaw = e.view.yaw;
    e.camEngKeyZoom = e.view.zoom;
    e.camEngKeyPanZ = e.view.panZ;
    e.camEngKeyCam = cam;
    e.camEngKeyEdit = editState;
    ++e.camEngCaps;

    if (!view) { camEngWarn(e, "view control null"); return; }
    using T = K2AddrTable;
    if (!k2addr::get() || !k2addr::addr(&T::SceneCameraVtable) || !k2addr::off(&T::View_Camera) || !k2addr::off(&T::SceneCamVt_MapToScreen) ||
        !k2addr::off(&T::Cam_Pos) || !k2addr::off(&T::Cam_Quat) || !k2addr::off(&T::Cam_Fovy) || !k2addr::off(&T::Cam_Near) || !k2addr::off(&T::Cam_Far) ||
        !k2addr::off(&T::Cam_Vp) || !k2addr::off(&T::Cam_Ortho) || !k2addr::off(&T::Cam_Behaviour)) {
        camEngWarn(e, "offsets unknown on this build; skipped");
        return;
    }
    const uint32_t wantVt = static_cast<uint32_t>(k2addr::addr(&T::SceneCameraVtable));

    // CAMPTR: the pointer chain (once, and on every vtable mismatch)
    if (!e.camEngCfgLogged || !vtOk) {
        e.camEngCfgLogged = true;
        uint32_t s08 = 0, s28 = 0, s44 = 0, s74 = 0, sceneCam = 0;
        if (vt) {
            (void)rdc<uint32_t>(vt, 0x08, &s08);
            (void)rdc<uint32_t>(vt, k2addr::off(&T::SceneCamVt_MapToScreen), &s28);
            (void)rdc<uint32_t>(vt, k2addr::off(&T::SceneCamVt_Fov), &s44);
            (void)rdc<uint32_t>(vt, k2addr::off(&T::SceneCamVt_Attach), &s74);
        }
        if (k2addr::off(&T::View_Scene)) (void)rdc<uint32_t>(view + static_cast<uintptr_t>(k2addr::off(&T::View_Scene)), 0x18, &sceneCam);
        KP_LOG(chView, "CAMPTR why=%s view=%08x cam=%08x vt=%08x vt_ok=%d s08=%08x s28=%08x s44=%08x s74=%08x scene_cam=%08x", why, static_cast<unsigned>(view),
               static_cast<unsigned>(cam), static_cast<unsigned>(vt), vtOk ? 1 : 0, static_cast<unsigned>(s08), static_cast<unsigned>(s28),
               static_cast<unsigned>(s44), static_cast<unsigned>(s74), static_cast<unsigned>(sceneCam));
    }
    if (!vtOk) {
        camEngWarn(e, "camera vtable %08x != %08x (cam=%08x)", static_cast<unsigned>(vt), static_cast<unsigned>(wantVt), static_cast<unsigned>(cam));
        for (int off = 0x74; off <= 0x7C; off += 4) {   // candidate walk: the three pointers around View_Camera
            uint32_t cand = 0, cvt = 0;
            const bool got = rdc<uint32_t>(view, off, &cand);
            if (got && cand) (void)rdc<uint32_t>(cand, 0, &cvt);
            camEngWarn(e, "candidate view+%03x=%08x vt=%08x%s", off, static_cast<unsigned>(cand), static_cast<unsigned>(cvt), cvt == wantVt ? " (matches)" : "");
        }
        return;
    }

    // CAMRAW (once): raw dwords around the fields the probe reads, for offline checking of the offsets
    if (!e.camEngDumped) {
        e.camEngDumped = true;
        static const int kBases[5] = {0x000, 0x0A0, 0x1B0, 0x200, 0x220};
        for (int b = 0; b < 5; ++b) {
            char line[120];
            int n = 0;
            for (int i = 0; i < 8 && n < static_cast<int>(sizeof line) - 12; ++i) {
                uint32_t d = 0;
                const bool got = rdc<uint32_t>(cam, kBases[b] + i * 4, &d);
                n += snprintf(line + n, sizeof line - n, got ? " %08x" : " --------", static_cast<unsigned>(d));
            }
            KP_LOG(chView, "CAMRAW base=%03x d=%s", kBases[b], line);
        }
    }

    // CAMENG: the pose and projection parameters the engine renders with
    float pos[3] = {0, 0, 0}, q[4] = {1, 0, 0, 0}, fovy = 0, nearP = 0, farP = 0, ortho = 0;
    int vp[4] = {0, 0, 0, 0};
    uint32_t slot = 0, beh = 0;
    bool readOk = true;
    auto need = [&](bool got, int off) {
        if (!got) { readOk = false; camEngWarn(e, "read failed off=%03x", off); }
    };
    for (int i = 0; i < 3; ++i) { const int o = k2addr::off(&T::Cam_Pos) + i * 4; need(rdc<float>(cam, o, &pos[i]), o); }
    for (int i = 0; i < 4; ++i) { const int o = k2addr::off(&T::Cam_Quat) + i * 4; need(rdc<float>(cam, o, &q[i]), o); }
    need(rdc<float>(cam, k2addr::off(&T::Cam_Fovy), &fovy), k2addr::off(&T::Cam_Fovy));
    need(rdc<float>(cam, k2addr::off(&T::Cam_Near), &nearP), k2addr::off(&T::Cam_Near));
    need(rdc<float>(cam, k2addr::off(&T::Cam_Far), &farP), k2addr::off(&T::Cam_Far));
    need(rdc<float>(cam, k2addr::off(&T::Cam_Ortho), &ortho), k2addr::off(&T::Cam_Ortho));
    for (int i = 0; i < 4; ++i) { const int o = k2addr::off(&T::Cam_Vp) + i * 4; need(rdc<int>(cam, o, &vp[i]), o); }
    need(rdc<uint32_t>(cam, k2addr::off(&T::Cam_Behaviour), &beh), k2addr::off(&T::Cam_Behaviour));
    need(rdc<uint32_t>(vt, k2addr::off(&T::SceneCamVt_MapToScreen), &slot), k2addr::off(&T::SceneCamVt_MapToScreen));
    const float qlen = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    int flag = -1;
    if (gameaddr::SceneCamRotFlag && !rdc<int>(gameaddr::SceneCamRotFlag, 0, &flag)) flag = -1;
    const int rotFast = (flag > 0 && (q[0] == 0.0f || q[0] == 1.0f)) ? 1 : 0;

    CamInfo hook;   // the camerahook node (what preview-3d aims the camera at) for the hook_dpos / hook_dot comparison
    const bool hookOk = readCam(e, hook);
    float dpos[3] = {0, 0, 0}, dot = -1.0f;
    if (hookOk) {
        for (int i = 0; i < 3; ++i) dpos[i] = hook.pos[i] - pos[i];
        const float hl = sqrtf(hook.q[0] * hook.q[0] + hook.q[1] * hook.q[1] + hook.q[2] * hook.q[2] + hook.q[3] * hook.q[3]);
        if (qlen > 1e-6f && hl > 1e-6f)
            dot = fabsf(q[0] * hook.q[0] + q[1] * hook.q[1] + q[2] * hook.q[2] + q[3] * hook.q[3]) / (qlen * hl);
    }
    KP_LOG(chView, "CAMENG why=%s pos=(%.4f,%.4f,%.4f) q=(%.5f,%.5f,%.5f,%.5f) qlen=%.5f fovy=%.3f near=%.3f far=%.3f vp=(%d,%d,%d,%d) ortho=%.4f hook_dpos=(%.4f,%.4f,%.4f) hook_dot=%.6f ctl=(%d,%d,%d,%d) flag=%d rot_fast=%d yaw=%.2f zoom=%.3f panZ=%.3f",
           why, pos[0], pos[1], pos[2], q[0], q[1], q[2], q[3], qlen, fovy, nearP, farP, vp[0], vp[1], vp[2], vp[3], ortho, dpos[0], dpos[1], dpos[2], dot,
           hook.ctlExt.x, hook.ctlExt.y, hook.ctlExt.w, hook.ctlExt.h, flag, rotFast, e.view.yaw, e.view.zoom, e.view.panZ);
    if (!hookOk) camEngWarn(e, "hook pose unavailable: %s", hook.why);
    if (rotFast && q[0] != 1.0f) camEngWarn(e, "fast path active w=%.9g flag=%d", q[0], flag);

    // CAMATT: the attach behaviour (camera offset from the attached node)
    {
        float off3[3] = {0, 0, 0};
        int flag48 = -1;
        if (!beh) {
            camEngWarn(e, "attach behaviour null (offset unknown)");
        } else {
            bool got = true;
            for (int i = 0; i < 3; ++i) got = rdc<float>(beh, kBehOffOff + i * 4, &off3[i]) && got;
            if (!rdc<int>(beh, kBehFlagOff, &flag48)) { flag48 = -1; got = false; }
            if (!got) camEngWarn(e, "read failed off=%03x (attach behaviour %08x)", kBehOffOff, static_cast<unsigned>(beh));
        }
        KP_LOG(chView, "CAMATT why=%s beh=%08x off=(%.4f,%.4f,%.4f) flag48=%d", why, static_cast<unsigned>(beh), off3[0], off3[1], off3[2], flag48);
    }
    if (!readOk) return;

    // The engine call is guarded exactly like the K contract: vtable ok, slot readable and non-zero, viewport non-empty (an empty one would make the
    // engine call glGetIntegerv), fovy in (0, 179), pose finite, quaternion not degenerate.
    uint8_t probeByte = 0;
    const char* refuse = nullptr;
    if (!slot) refuse = "MapToScreen slot is zero; not called";
    else if (!rdc<uint8_t>(slot, 0, &probeByte)) refuse = "MapToScreen slot target unreadable; not called";
    else if (vp[2] <= 0 || vp[3] <= 0) refuse = "viewport unset; engine would call glGetIntegerv; not called";
    else if (!hse::finiteF(fovy) || !(fovy > 0.0f) || !(fovy < 179.0f)) refuse = "fovy out of range (0,179); not called";
    else if (!vec3Finite(pos) || !hse::finiteF(q[0]) || !hse::finiteF(q[1]) || !hse::finiteF(q[2]) || !hse::finiteF(q[3])) refuse = "pose not finite; not called";
    else if (!(qlen > 1e-6f)) refuse = "quaternion length <= 1e-6; not called";
    if (refuse) {
        camEngWarn(e, "%s (slot=%08x vp=%d,%d,%d,%d fovy=%.3f qlen=%.5f)", refuse, static_cast<unsigned>(slot), vp[0], vp[1], vp[2], vp[3], fovy, qlen);
        return;
    }

    // the points: both hand nodes on the body rig, plus the mount bone while a weapon edit is open
    CamPt pts[3];
    int np = 0;
    if (!g_p3.ok() || !g_hol.ok() || !e.token) {
        camEngWarn(e, "no projection points: %s", !g_p3.ok() ? "preview3d provider missing" : (!g_hol.ok() ? "holster provider missing" : "no viewer token"));
    } else {
        void* rig = g_p3.p->RigGob(e.token, 0);
        static const char* const kNode[2] = {"rhand", "lhand"};
        static const char kHandCh[2] = {'R', 'L'};
        for (int h = 0; h < 2; ++h) {
            float p[3] = {0, 0, 0}, pq[4] = {1, 0, 0, 0};
            if (rig && g_hol.h->GetNodeWorld(rig, kNode[h], p, pq)) {
                pts[np].hand = kHandCh[h];
                memcpy(pts[np].p, p, sizeof p);
                ++np;
            } else {
                camEngWarn(e, "hand=%c unavailable (node %s, rig=%p)", kHandCh[h], kNode[h], rig);
            }
        }
        if (e.mode == MODE_WEAPON) {
            float mp[3] = {0, 0, 0}, mq[4] = {1, 0, 0, 0};
            if (g_hol.h->GetMountWorld(mp, mq)) {
                pts[np].hand = 'M';
                memcpy(pts[np].p, mp, sizeof mp);
                ++np;
            } else {
                camEngWarn(e, "hand=M unavailable (GetMountWorld false)");
            }
        }
    }
    int nChk = 0;
    float maxPx = 0.0f, maxFrac = 0.0f;
    for (int i = 0; i < np; ++i) {
        float dPx = -1.0f, dFrac = -1.0f;
        camEngProject(e, why, cam, slot, pts[i], pos, q, fovy, vp, &dPx, &dFrac);
        if (dPx >= 0.0f) {
            ++nChk;
            if (dPx > maxPx) maxPx = dPx;
            if (dFrac > maxFrac) maxFrac = dFrac;
        }
    }
    KP_LOG(chView, "CAMCHK why=%s n=%d max_eng_pred_px=%.2f max_eng_ours_frac=%.5f", why, nChk, maxPx, maxFrac);
}

// CSWCItem::GetName(out, identified) into name (at most 63 printable chars). identified=0 only copies the item's own name (may be empty);
// identified=1 can take the engine's strref lookup branch (unchecked base-item row), so it runs only at probe_names >= 2. Frees the CExoString.
bool callItemName(uintptr_t item, int identified, char* name, int cap, const char** why) {
    name[0] = 0;
    *why = "";
    if (!gameaddr::ClientItemGetName || !gameaddr::ExoDtor) { *why = "GetName/ExoDtor address unknown"; return false; }
    int exo[2] = {0, 0};   // CExoString {char* text, int len}
    reinterpret_cast<uintptr_t(__thiscall*)(void*, void*, int)>(gameaddr::ClientItemGetName)(reinterpret_cast<void*>(item), exo, identified);
    bool got = false;
    if (exo[0]) {
        char tmp[256];
        if (saferead::readExo(0xE70117u, reinterpret_cast<uintptr_t>(exo), tmp, sizeof(tmp), 255, false)) {
            int k = 0;
            for (; tmp[k] && k < cap - 1 && k < 63; ++k) {
                unsigned char ch = static_cast<unsigned char>(tmp[k]);
                name[k] = (ch == '"') ? '\'' : ((ch >= 0x20 && ch <= 0x7E) ? static_cast<char>(ch) : '.');
            }
            name[k] = 0;
            got = name[0] != 0;
            if (!got) *why = "name empty";
        } else {
            *why = "readExo failed";
        }
        reinterpret_cast<void(__thiscall*)(void*)>(gameaddr::ExoDtor)(exo);
    } else {
        *why = "GetName returned no text";
    }
    return got;
}

// ITEMNAME probe (U0, log only; not shown in the UI): the equipped item ids of the client creature -> client item -> model resref and
// the engine's CSWCItem::GetName. Gated by ini probe_names (0 off, 1 identified=0, 2 adds identified=1). The item's base item index
// (item+0xC) must be readable before any call. Every read is guarded.
void probeItemNames(Editor& e, uintptr_t clientMember) {
    e.itemName[0][0] = e.itemName[1][0] = '\0';   // the cache is rebuilt on every member build; an empty name means "show the resref"
    if (g_probeNames <= 0) return;   // probe switched off: not a failure
    if (!clientMember) {
        if (warnOk(e, W_ITEMNAME)) KP_LOGV(chView, KPLOG_WARN, "ITEMNAME WARN no client creature (null member)");
        return;
    }
    saferead::beginScope();
    static const char* const kHand[2] = {"R", "L"};
    uintptr_t gApp = 0, tbl = 0;
    const bool haveTbl = gameaddr::AppGlobal && rd<uintptr_t>(gameaddr::AppGlobal, 0, &gApp) && gApp && rd<uintptr_t>(gApp, 4, &tbl) && tbl;
    if (!haveTbl && warnOk(e, W_ITEMNAME))
        KP_LOGV(chView, KPLOG_WARN, "ITEMNAME WARN client item table unavailable (AppGlobal=%x app=%x tbl=%x)", static_cast<unsigned>(gameaddr::AppGlobal),
                static_cast<unsigned>(gApp), static_cast<unsigned>(tbl));
    for (int h = 0; h < 2; ++h) {
        uint32_t id = kNoItemId;
        const bool idOk = rd<uint32_t>(clientMember, h ? kCrLeftItemOff : kCrRightItemOff, &id);
        uintptr_t item = 0;
        char resref[17] = {0};
        char name0[64] = {0}, name1[64] = {0}, clean[64] = {0};
        const char* note1 = "skipped";
        const char* why = "";
        if (!idOk) {
            if (warnOk(e, W_ITEMNAME)) KP_LOGV(chView, KPLOG_WARN, "ITEMNAME WARN hand=%s equipped item id unreadable", kHand[h]);
        } else if (id != kNoItemId && id != 0 && haveTbl) {
            if (!gameaddr::ClientGetItemById) {
                if (warnOk(e, W_ITEMNAME)) KP_LOGV(chView, KPLOG_WARN, "ITEMNAME WARN hand=%s ClientGetItemById address unknown", kHand[h]);
            } else {
                item = reinterpret_cast<uintptr_t(__thiscall*)(void*, uint32_t)>(gameaddr::ClientGetItemById)(reinterpret_cast<void*>(tbl), id);
                uint32_t vt = 0, baseIdx = 0;
                if (item && !(rd<uint32_t>(item, 0, &vt) && vt && rd<uint32_t>(item, kItemBaseIdxOff, &baseIdx))) {
                    if (warnOk(e, W_ITEMNAME)) KP_LOGV(chView, KPLOG_WARN, "ITEMNAME WARN hand=%s item=0x%x not readable (vt=%x)", kHand[h], static_cast<unsigned>(item), vt);
                    item = 0;   // not a readable item
                } else if (!item && warnOk(e, W_ITEMNAME)) {
                    KP_LOGV(chView, KPLOG_WARN, "ITEMNAME WARN hand=%s id=0x%08x not found in the client item table", kHand[h], static_cast<unsigned>(id));
                }
            }
        }
        if (item) {
            int n = 0;
            for (; n < 16; ++n) {
                char ch = 0;
                if (!rd<char>(item, kItemModelResOff + n, &ch) || ch == 0) break;
                resref[n] = (ch >= 0x20 && ch <= 0x7E) ? ch : '.';
            }
            resref[n] = 0;
            if (!callItemName(item, 0, name0, sizeof(name0), &why) && warnOk(e, W_ITEMNAME))
                KP_LOGV(chView, KPLOG_WARN, "ITEMNAME WARN hand=%s item=0x%x GetName(identified=0) gave no name: %s", kHand[h], static_cast<unsigned>(item), why);
            if (g_probeNames >= 2) {
                if (!callItemName(item, 1, name1, sizeof(name1), &why) && warnOk(e, W_ITEMNAME))
                    KP_LOGV(chView, KPLOG_WARN, "ITEMNAME WARN hand=%s item=0x%x GetName(identified=1) gave no name: %s", kHand[h], static_cast<unsigned>(item), why);
                note1 = "";
            }
        }
        hse::stripNameTag(name0, clean, sizeof clean);   // name0 keeps the engine's "{NN}" tag; clean is the same name without it
        snprintf(e.itemName[h], sizeof e.itemName[h], "%s", clean);   // cached for the weapon buttons (fitted in setHandNames)
        if (note1[0]) KP_LOG(chView, "ITEMNAME hand=%s id=0x%08x item=0x%x resref=%s name0=\"%s\" clean=\"%s\" name1=(skipped)", kHand[h], static_cast<unsigned>(id),
                             static_cast<unsigned>(item), resref, name0, clean);
        else KP_LOG(chView, "ITEMNAME hand=%s id=0x%08x item=0x%x resref=%s name0=\"%s\" clean=\"%s\" name1=\"%s\"", kHand[h], static_cast<unsigned>(id),
                    static_cast<unsigned>(item), resref, name0, clean, name1);
    }
}

// ---- U0b CALIB probe: click targets on the character, log each click with the camera pose and the body node world positions ----

void updateStatusText(Editor& e);   // defined with the status helpers below

// The prompt is shown (and clicks are recorded) only while the page is LIVE in nav mode, never while an edit is open.
bool calibActive(const Editor& e) { return g_probeCalib && e.state == LIVE && e.mode == MODE_NAV; }

// Width the status text is fitted to: the status pane width (the label font is not measured; ini status_fit scales it).
float statusFitPx(const Editor& e) {
    return (e.statusRect.w > 0 ? static_cast<float>(e.statusRect.w) : 400.0f) * g_statusFit;
}

// raw cut with "..." to the status width.
void fitStatus(const Editor& e, const char* raw, char* out, int cap) {
    const textfit::TextCtx tc = {1.0f, 1.0f, 1.0f};   // UI units: texel width * 0.75 (TextK), no scale
    textfit::fitText(tc, raw, statusFitPx(e), out, cap);
}

// "Calib 3/6 (round 1): click the right hand (wheel = zoom out)" fitted to the status label (about 95 characters at most). The wheel hint is
// appended only while the whole line fits; otherwise the prompt alone is shown (cut with "..." when even that does not fit).
void calibStatusText(const Editor& e, char* out, int cap) {
    char raw[96];
    hse::calibPrompt(raw, sizeof raw, &e.calib);
    char withHint[160];
    snprintf(withHint, sizeof withHint, "%s%s", raw, hse::kCalibHint);
    const textfit::TextCtx tc = {1.0f, 1.0f, 1.0f};
    if (textfit::textPx(tc, withHint, static_cast<int>(strlen(withHint))) <= statusFitPx(e)) snprintf(out, cap, "%s", withHint);
    else fitStatus(e, raw, out, cap);
    if (cap > 95) out[95] = '\0';
}

struct CalibNode {
    bool ok;
    const char* name;   // the candidate that resolved (static string), "" on a miss
    float p[3];
};

// Resolves target i on the body rig: the cached candidate first, then every candidate in order. Logs nothing (the caller owns the WARNs).
bool calibNode(Editor& e, void* rig, int i, CalibNode& n) {
    n.ok = false;
    n.name = "";
    n.p[0] = n.p[1] = n.p[2] = 0.0f;
    if (!rig || !g_hol.ok() || i < 0 || i >= kCalibNodes) return false;
    float q[4] = {1, 0, 0, 0};
    const int cached = e.calibNodeIdx[i];
    if (cached >= 0 && cached < 4 && kCalibCand[i][cached] && g_hol.h->GetNodeWorld(rig, kCalibCand[i][cached], n.p, q)) {
        n.ok = true;
        n.name = kCalibCand[i][cached];
        return true;
    }
    for (int c = 0; c < 4 && kCalibCand[i][c]; ++c) {
        if (g_hol.h->GetNodeWorld(rig, kCalibCand[i][c], n.p, q)) {
            e.calibNodeIdx[i] = c;
            n.ok = true;
            n.name = kCalibCand[i][c];
            return true;
        }
    }
    e.calibNodeIdx[i] = -1;
    return false;
}

// One-time "node not found" WARN per target (bit mask in the Editor), counted against the CALIB cap.
void calibWarnMiss(Editor& e, int i) {
    if (i < 0 || i >= kCalibNodes || (e.calibMissWarned & (1u << i))) return;
    e.calibMissWarned |= 1u << i;
    if (!warnOk(e, W_CALIB)) return;
    char tried[64] = {0};
    int len = 0;
    for (int c = 0; c < 4 && kCalibCand[i][c]; ++c) len += snprintf(tried + len, sizeof tried - len, "%s%s", c ? "," : "", kCalibCand[i][c]);
    KP_LOGV(chView, KPLOG_WARN, "CALIB WARN node '%s' not found (tried %s)", kCalibLabel[i], tried);
}

void calibLogPrompt(const Editor& e, const char* why) {
    KP_LOG(chView, "CALIB prompt step=%d/%d round=%d target=%s why=%s", e.calib.step + 1, hse::CalibScript::kCount, e.calib.round, e.calib.current(), why);
}

// LIVE start (every build of the preview creature): resolve the node names once, log the config, show the first prompt.
void calibStart(Editor& e) {
    for (int i = 0; i < kCalibNodes; ++i) e.calibNodeIdx[i] = -1;
    e.calibMissWarned = 0;
    if (!g_probeCalib) {
        if (!e.calibCfgLogged) KP_LOG(chView, "CALIB cfg probe_calib=0 (off: normal status text)");
        e.calibCfgLogged = true;
        return;
    }
    void* rig = (g_p3.ok() && e.token) ? g_p3.p->RigGob(e.token, 0) : nullptr;
    if (!rig && warnOk(e, W_CALIB)) KP_LOGV(chView, KPLOG_WARN, "CALIB WARN start: body rig unavailable (p3=%d token=%u)", g_p3.ok() ? 1 : 0, e.token);
    if (!g_hol.ok() && warnOk(e, W_CALIB)) KP_LOGV(chView, KPLOG_WARN, "CALIB WARN start: holster provider unavailable");
    char names[200] = {0};
    int len = 0;
    for (int i = 0; i < kCalibNodes; ++i) {
        CalibNode n;
        const bool ok = calibNode(e, rig, i, n);
        if (!ok && rig) calibWarnMiss(e, i);
        len += snprintf(names + len, sizeof names - len, "%s%s=%s", i ? " " : "", kCalibLabel[i], ok ? n.name : "MISS");
        if (len >= static_cast<int>(sizeof names)) { len = sizeof names - 1; break; }
    }
    KP_LOG(chView, "CALIB cfg probe_calib=%d targets=%d node-names resolved: %s", g_probeCalib, hse::CalibScript::kCount, names);
    e.calibCfgLogged = true;
    calibLogPrompt(e, "live");
    updateStatusText(e);
}

// An LMB press edge in nav mode (the click is never swallowed: the caller still starts the orbit drag). Logs one line per click inside the view.
void calibClick(Editor& e, int cx, int cy, bool inside) {
    if (!g_probeCalib || e.state != LIVE || e.mode != MODE_NAV) return;
    if (!inside) {
        if (e.calibSkips < kMaxCalibSkips) {
            ++e.calibSkips;
            KP_LOG(chView, "CALIB skip reason=outside click=(%d,%d) view=(%d,%d,%d,%d) n=%d", cx, cy, e.viewRect.x, e.viewRect.y, e.viewRect.w, e.viewRect.h, e.calibSkips);
        }
        return;
    }
    CamInfo c;
    const bool camOk = readCam(e, c);
    if (!camOk && warnOk(e, W_CALIB)) KP_LOGV(chView, KPLOG_WARN, "CALIB WARN readCam false: %s (click step=%d)", c.why, e.calib.step + 1);
    void* rig = (g_p3.ok() && e.token) ? g_p3.p->RigGob(e.token, 0) : nullptr;
    if (!g_p3.ok() && warnOk(e, W_CALIB)) KP_LOGV(chView, KPLOG_WARN, "CALIB WARN preview3d provider null");
    else if (g_p3.ok() && !rig && warnOk(e, W_CALIB)) KP_LOGV(chView, KPLOG_WARN, "CALIB WARN body rig null (token=%u)", e.token);
    if (!g_hol.ok() && warnOk(e, W_CALIB)) KP_LOGV(chView, KPLOG_WARN, "CALIB WARN holster provider null");

    char nodes[300] = {0};
    int len = 0;
    unsigned mask = 0;
    int missing = 0;
    for (int i = 0; i < kCalibNodes; ++i) {
        CalibNode n;
        char one[64];
        if (calibNode(e, rig, i, n)) {
            mask |= 1u << i;
            snprintf(one, sizeof one, "%s=%s(%.3f,%.3f,%.3f)", kCalibLabel[i], n.name, n.p[0], n.p[1], n.p[2]);
        } else {
            ++missing;
            snprintf(one, sizeof one, "%s=MISS", kCalibLabel[i]);
            if (rig) calibWarnMiss(e, i);
        }
        len += snprintf(nodes + len, sizeof nodes - len, "%s%s", i ? " " : "", one);
        if (len >= static_cast<int>(sizeof nodes)) { len = sizeof nodes - 1; break; }
    }
    if (missing && warnOk(e, W_CALIB)) KP_LOGV(chView, KPLOG_WARN, "CALIB WARN click round=%d step=%d: %d of %d nodes missing", e.calib.round, e.calib.step + 1, missing, kCalibNodes);
    KP_LOG(chView, "CALIB round=%d step=%d target=%s click=(%d,%d) yaw=%.2f zoom=%.2f panZ=%.2f fovy=%.3f ext=(%d,%d,%d,%d) hook=(%.3f,%.3f,%.3f) q=(%.4f,%.4f,%.4f,%.4f) nodes: %s ok=0x%02x cam=%d member=%d",
           e.calib.round, e.calib.step + 1, e.calib.current(), cx, cy, e.view.yaw, e.view.zoom, e.view.panZ, c.fovy, c.ext.x, c.ext.y, c.ext.w, c.ext.h,
           c.pos[0], c.pos[1], c.pos[2], c.q[0], c.q[1], c.q[2], c.q[3], nodes, mask, camOk ? 1 : 0, e.member + 1);
    ++e.calibRoundClicks;
    const int roundNo = e.calib.round;
    if (e.calib.advance()) {
        KP_LOG(chView, "CALIB round=%d done clicks=%d", roundNo, e.calibRoundClicks);
        e.calibRoundClicks = 0;
    }
    calibLogPrompt(e, "click");
    updateStatusText(e);
}

// ---- CAM2 probe (log only, ux_v3 A1): why the viewport shows ~1.0 m where the camera maths predicts ~1.7 m ----
// One line set per call (CAM2 gl / rect / stack / flag / node x6), at most kMaxCam2Logs per Editor, from the tick (idle GUI stack, not the draw-time push).
// Engine globals are read with safe reads only; the one GL call (glGetIntegerv) runs only when this thread has a current GL context.
constexpr int kMaxCam2Logs = 3;
constexpr uintptr_t kCam2StackDepth = 0x00A309C0;   // clip stack depth (low 16 bits signed: 0 HUD, 0xFFFF = -1 menu frames); names: A1_cam2_addresses.txt
constexpr uintptr_t kCam2StackEntry0 = 0x00A309C8;  // entry[i] = int16 x,y,w,h + uint16 flag at +i*10; entry[0].h (0xA309CE) = root height
constexpr uintptr_t kCam2RenderFlag = 0x00A32A54;   // dword, 0 = RenderScene takes the modelview from cam+0xA8/+0xB4
constexpr uintptr_t kCam2PushReject = 0x00A32A30;   // dword, nonzero = 0x479460 refuses the push

void logCam2(Editor& e, const char* why) {
    if (!g_probeCam || e.cam2Logs >= kMaxCam2Logs) return;
    ++e.cam2Logs;
    saferead::beginScope();
    const int n = e.cam2Logs;

    // 1. GL viewport of this thread (only with a current GL context)
    {
        typedef void*(APIENTRY * CurCtxFn)();
        typedef void(APIENTRY * GetIntFn)(unsigned, int*);
        static CurCtxFn curCtx = nullptr;
        static GetIntFn getInt = nullptr;
        static bool resolved = false;
        if (!resolved) {
            resolved = true;
            HMODULE m = GetModuleHandleA("opengl32.dll");
            if (m) {
                curCtx = reinterpret_cast<CurCtxFn>(reinterpret_cast<void*>(GetProcAddress(m, "wglGetCurrentContext")));
                getInt = reinterpret_cast<GetIntFn>(reinterpret_cast<void*>(GetProcAddress(m, "glGetIntegerv")));
            }
        }
        void* ctx = curCtx ? curCtx() : nullptr;
        if (!getInt || !ctx) {
            KP_LOG(chView, "CAM2 viewport skipped (no GL thread) n=%d why=%s fn=%d ctx=%p", n, why, getInt ? 1 : 0, ctx);
        } else {
            int gv[4] = {0, 0, 0, 0};
            getInt(0x0BA2, gv);   // GL_VIEWPORT
            KP_LOG(chView, "CAM2 viewport n=%d why=%s phase=tick gl=(%d,%d,%d,%d) ctx=%p", n, why, gv[0], gv[1], gv[2], gv[3], ctx);
        }
    }

    // 2. scene sub-object rect (S+4..S+0x10), the view control's own rect (V+4..V+0x10) and the control rect used for the view
    {
        using T = K2AddrTable;
        const uintptr_t view = e.viewCtl;
        int s[4] = {0, 0, 0, 0}, v[4] = {0, 0, 0, 0};
        bool sOk = false, vOk = false;
        if (view && k2addr::get() && k2addr::off(&T::View_Scene)) {
            const uintptr_t sc = view + static_cast<uintptr_t>(k2addr::off(&T::View_Scene));
            sOk = true;
            for (int i = 0; i < 4; ++i) sOk = rdc<int>(sc, 4 + i * 4, &s[i]) && sOk;
        }
        if (view) {
            vOk = true;
            for (int i = 0; i < 4; ++i) vOk = rdc<int>(view, 4 + i * 4, &v[i]) && vOk;
        }
        CamInfo c;
        const bool camOk = readCam(e, c);
        KP_LOG(chView, "CAM2 rect n=%d why=%s S=(%d,%d,%d,%d) s_ok=%d V=(%d,%d,%d,%d) v_ok=%d ctl=(%d,%d,%d,%d) ext=(%d,%d,%d,%d) pane=(%d,%d,%d,%d) viewRect=(%d,%d,%d,%d) cam=%d",
               n, why, s[0], s[1], s[2], s[3], sOk ? 1 : 0, v[0], v[1], v[2], v[3], vOk ? 1 : 0, c.ctlExt.x, c.ctlExt.y, c.ctlExt.w, c.ctlExt.h, c.ext.x, c.ext.y,
               c.ext.w, c.ext.h, c.pane.x, c.pane.y, c.pane.w, c.pane.h, e.viewRect.x, e.viewRect.y, e.viewRect.w, e.viewRect.h, camOk ? 1 : 0);
    }

    // 3. parent stack used by 0x479460: raw depth, then entry[-1], entry[0] (root) and every entry up to the depth (all raw int16, flag as uint16)
    {
        uint32_t depthRaw = 0;
        const bool dOk = rdc<uint32_t>(kCam2StackDepth, 0, &depthRaw);
        const int idx = static_cast<int16_t>(depthRaw & 0xFFFFu);
        int16_t rootH = 0;
        const bool hOk = rdc<int16_t>(0x00A309CE, 0, &rootH);
        KP_LOG(chView, "CAM2 stack n=%d why=%s depth_raw=%08x depth_ok=%d idx16=%d rootH(a309ce)=%d h_ok=%d", n, why, static_cast<unsigned>(depthRaw), dOk ? 1 : 0, idx,
               static_cast<int>(rootH), hOk ? 1 : 0);
        const int last = (idx >= 0 && idx <= 7) ? idx : 0;
        for (int i = -1; i <= last; ++i) {
            int16_t en[5] = {0, 0, 0, 0, 0};
            bool got = true;
            for (int k = 0; k < 5; ++k) got = rdc<int16_t>(kCam2StackEntry0, i * 10 + k * 2, &en[k]) && got;
            KP_LOG(chView, "CAM2 stack n=%d entry[%d]=(%d,%d,%d,%d) flag=%u ok=%d%s", n, i, en[0], en[1], en[2], en[3], static_cast<unsigned>(static_cast<uint16_t>(en[4])), got ? 1 : 0,
                   i == idx ? " <- current parent" : "");
        }
        if (idx < -1 || idx > 7) {
            int16_t en[5] = {0, 0, 0, 0, 0};
            bool got = true;
            for (int k = 0; k < 5; ++k) got = rdc<int16_t>(kCam2StackEntry0, idx * 10 + k * 2, &en[k]) && got;
            KP_LOG(chView, "CAM2 stack n=%d entry[%d](current, out of the dumped range)=(%d,%d,%d,%d) flag=%u ok=%d", n, idx, en[0], en[1], en[2], en[3],
                   static_cast<unsigned>(static_cast<uint16_t>(en[4])), got ? 1 : 0);
        }
    }

    // 4. render flag and push-reject flag, raw
    {
        uint32_t f54 = 0, f30 = 0;
        const bool a = rdc<uint32_t>(kCam2RenderFlag, 0, &f54);
        const bool b = rdc<uint32_t>(kCam2PushReject, 0, &f30);
        KP_LOG(chView, "CAM2 flags n=%d why=%s a32a54=%08x ok=%d a32a30=%08x ok=%d", n, why, static_cast<unsigned>(f54), a ? 1 : 0, static_cast<unsigned>(f30), b ? 1 : 0);
    }

    // 5. CALIB node world positions, one line per node (never cut: the CALIB click line caps its node list at 300 characters)
    {
        void* rig = (g_p3.ok() && e.token) ? g_p3.p->RigGob(e.token, 0) : nullptr;
        if (!rig) KP_LOG(chView, "CAM2 nodes n=%d why=%s rig unavailable (p3=%d token=%u)", n, why, g_p3.ok() ? 1 : 0, e.token);
        for (int i = 0; rig && i < kCalibNodes; ++i) {
            CalibNode cn;
            if (calibNode(e, rig, i, cn)) KP_LOG(chView, "CAM2 node n=%d %s=%s(%.4f,%.4f,%.4f)", n, kCalibLabel[i], cn.name, cn.p[0], cn.p[1], cn.p[2]);
            else KP_LOG(chView, "CAM2 node n=%d %s=MISS", n, kCalibLabel[i]);
        }
    }
}

// Fresh GetEdit into e.editInfo; true while an edit is open.
bool refreshEdit(Editor& e) {
    memset(&e.editInfo, 0, sizeof(e.editInfo));
    e.editInfo.struct_size = sizeof(e.editInfo);
    return g_hol.ok() && g_hol.h->GetEdit(&e.editInfo) != 0;
}

// True when the open edit has unsaved changes (reads the provider, not the cached e.editInfo).
bool editDirty() {
    if (!g_hol.ok()) return false;
    K2HolsterEditInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = sizeof(info);
    return g_hol.h->GetEdit(&info) != 0 && info.dirty != 0;
}

void setBoneText(Editor& e) {
    if (e.boneItem < 0 || !g_menu.ok()) return;
    char bBuf[64];
    snprintf(bBuf, sizeof(bBuf), "Bone: %s", e.editInfo.bone[0] ? e.editInfo.bone : "None");
    g_menu.p->SetText(e.page, e.boneItem, bBuf);
}

// Button text cut with "..." to maxPx (maxPx <= 0: the strip is not measured yet, no cut).
void fitButton(float maxPx, const char* raw, char* out, int cap) {
    const textfit::TextCtx tc = {1.0f, 1.0f, 1.0f};   // UI units: texel width * 0.75 (TextK), no scale
    if (maxPx <= 0.0f) snprintf(out, cap, "%s", raw);
    else textfit::fitText(tc, raw, maxPx, out, cap);
}

// Weapon button caption in nav mode: the item display name (cached at build), else the model resref. "Right: Vibrosword" when that fits the
// half strip, else "R: Vibrosword", cut with "..." when even that is too wide.
void handCaption(const Editor& e, int hand, char* out, int cap) {
    const float maxPx = e.strip.w > 0 ? static_cast<float>(hse::half(hse::stripSlot(e.strip, 2), 0, 6).w) : 0.0f;
    const textfit::TextCtx tc = {1.0f, 1.0f, 1.0f};
    char longT[96], shortT[96];
    hse::handButtonText(longT, sizeof longT, hand, false, e.itemName[hand], e.wpnName[hand], e.regGob[hand] != nullptr);
    if (maxPx <= 0.0f || textfit::textPx(tc, longT, static_cast<int>(strlen(longT))) <= maxPx) {
        snprintf(out, cap, "%s", longT);
        return;
    }
    hse::handButtonText(shortT, sizeof shortT, hand, true, e.itemName[hand], e.wpnName[hand], e.regGob[hand] != nullptr);
    fitButton(maxPx, shortT, out, cap);
}

// Right/Left button captions in nav mode.
void setHandNames(Editor& e) {
    if (!g_menu.ok()) return;
    char rBuf[96], lBuf[96];
    handCaption(e, 0, rBuf, sizeof rBuf);
    handCaption(e, 1, lBuf, sizeof lBuf);
    if (e.wpnRightItem >= 0) g_menu.p->SetText(e.page, e.wpnRightItem, rBuf);
    if (e.wpnLeftItem >= 0) g_menu.p->SetText(e.page, e.wpnLeftItem, lBuf);
}

// Scope button text from the open edit's type ("For: this weapon" / "For: all rifles" / "For: this body"), fitted to the full strip slot.
void setScopeText(Editor& e) {
    if (e.scopeItem < 0 || !g_menu.ok()) return;
    char raw[64], fit[64];
    hse::scopeButtonText(raw, sizeof raw, e.editScope, e.editInfo.type);
    fitButton(e.strip.w > 0 ? static_cast<float>(hse::stripSlot(e.strip, 3).w) : 0.0f, raw, fit, sizeof fit);
    g_menu.p->SetText(e.page, e.scopeItem, fit);
    KP_LOG(chView, "SCOPE text=\"%s\" scope=%d type=%s", fit, e.editScope, e.editInfo.type[0] ? e.editInfo.type : "-");
}

// Mode bar captions: the active mode is marked ">Move<". Fitted to the half strip.
void setModeBarText(Editor& e) {
    if (!e.modeBarOk || !g_menu.ok()) return;
    const float maxPx = e.strip.w > 0 ? static_cast<float>(hse::half(hse::stripSlot(e.strip, 7), 0, 6).w) : 0.0f;
    for (int i = 0; i < kModeCount; ++i) {
        char raw[32], fit[32];
        hse::modeLabel(raw, sizeof raw, i, i == e.weaponMode);
        fitButton(maxPx, raw, fit, sizeof fit);
        g_menu.p->SetText(e.page, e.modeItem[i], fit);
    }
}

// Selects the weapon mode. A running gesture keeps the mode it began with; the new one applies from the next press.
void setWeaponMode(Editor& e, int mode, const char* why) {
    if (mode < 0 || mode >= kModeCount || mode == e.weaponMode) return;
    KP_LOG(chView, "MODE %s -> %s why=%s%s", hse::weaponModeName(e.weaponMode), hse::weaponModeName(mode), why, e.gActive ? " (applies from the next press)" : "");
    e.weaponMode = mode;
    setModeBarText(e);
}

// Stance back to Idle: index, emote state, stance label and Play button caption.
void resetStanceUi(Editor& e) {
    e.stanceIdx = 0;
    e.playingEmote = false;
    e.emoteEndMs = 0;
    if (!g_menu.ok() || !e.page) return;
    char sBuf[64];
    snprintf(sBuf, sizeof(sBuf), "Stance: %s", kStanceList[0].label);
    if (e.stanceLabelItem >= 0) g_menu.p->SetText(e.page, e.stanceLabelItem, sBuf);
    if (e.playPauseItem >= 0) g_menu.p->SetText(e.page, e.playPauseItem, "Play");
}

void updateStatusText(Editor& e) {
    if (!g_menu.ok() || !e.page || e.statusItem < 0) return;
    char buf[160] = {0};
    if (e.mode == MODE_WEAPON && e.discardArmed && hse::guardArmLive(true, e.discardArmedAt, GetTickCount(), kDiscardWindowMs)) {
        snprintf(buf, sizeof(buf), "Unsaved: click again to discard");
    } else if (e.mode == MODE_WEAPON) {
        refreshEdit(e);
        // The numbers show while a gesture runs or for kStatusHoldMs after any value (or the unsaved flag) changed; at rest the help line shows.
        const DWORD now = GetTickCount();
        const bool changed = e.shownDirty != e.editInfo.dirty || memcmp(e.shownRot, e.editInfo.rot, sizeof e.shownRot) != 0 ||
                             memcmp(e.shownOff, e.editInfo.off, sizeof e.shownOff) != 0;
        if (changed) {
            e.shownDirty = e.editInfo.dirty;
            memcpy(e.shownRot, e.editInfo.rot, sizeof e.shownRot);
            memcpy(e.shownOff, e.editInfo.off, sizeof e.shownOff);
            e.statusHoldUntil = now + kStatusHoldMs;
            e.statusHolding = true;
        }
        if (e.statusHolding && static_cast<int32_t>(e.statusHoldUntil - now) <= 0) e.statusHolding = false;
        if (e.gActive || e.statusHolding) {
            const char* shown = e.editHand >= 0 && e.itemName[e.editHand][0] ? e.itemName[e.editHand] : (e.editInfo.model[0] ? e.editInfo.model : (e.editInfo.type[0] ? e.editInfo.type : "wpn"));
            snprintf(buf, sizeof(buf), "%sEDIT [%s]: %s on %s | rot=(%.1f,%.1f,%.1f) off=(%.2f,%.2f,%.2f)",
                     e.editInfo.dirty ? "* " : "",
                     e.editHand == 0 ? "R" : "L",
                     shown,
                     e.editInfo.bone[0] ? e.editInfo.bone : "-",
                     e.editInfo.rot[0], e.editInfo.rot[1], e.editInfo.rot[2],
                     e.editInfo.off[0], e.editInfo.off[1], e.editInfo.off[2]);
            buf[95] = '\0';   // the status label holds about 95 characters
        } else {
            char raw[160];
            hse::idleStatusText(raw, sizeof raw, e.editInfo.dirty != 0, e.editHand == 1 && hse::leftMirrored(e.editInfo.left != 0, e.editInfo.src));
            fitStatus(e, raw, buf, static_cast<int>(sizeof buf));
            buf[95] = '\0';
        }
    } else if (calibActive(e)) {
        calibStatusText(e, buf, static_cast<int>(sizeof buf));
    } else {
        snprintf(buf, sizeof(buf), "Member %d/%d | Weapons: %s%s%s | Stance: %s",
                 e.member + 1, hse::memberCount(),
                 e.regGob[0] ? "Right" : "-",
                 (e.regGob[0] && e.regGob[1]) ? "+" : "",
                 e.regGob[1] ? "Left" : (e.regGob[0] ? "" : "None"),
                 kStanceList[e.stanceIdx].label);
    }
    g_menu.p->SetText(e.page, e.statusItem, buf);
}

void applyLayout(Editor& e) {
    if (!g_menu.ok() || !e.page || e.strip.w <= 0) return;
    const hse::HseRect offscreen = {-9999, -9999, 0, 0};

    if (e.mode == MODE_NAV) {
        placeRect(e.nameItem, hse::stripSlot(e.strip, 0));
        placeRect(e.prevItem, hse::half(hse::stripSlot(e.strip, 1), 0, 6));
        placeRect(e.nextItem, hse::half(hse::stripSlot(e.strip, 1), 1, 6));

        placeRect(e.wpnRightItem, hse::half(hse::stripSlot(e.strip, 2), 0, 6));
        placeRect(e.wpnLeftItem, hse::half(hse::stripSlot(e.strip, 2), 1, 6));

        placeRect(e.stanceLabelItem, hse::stripSlot(e.strip, 3));
        placeRect(e.stancePrevItem, hse::half(hse::stripSlot(e.strip, 4), 0, 6));
        placeRect(e.stanceNextItem, hse::half(hse::stripSlot(e.strip, 4), 1, 6));

        placeRect(e.emoteLabelItem, hse::stripSlot(e.strip, 5));
        placeRect(e.emotePrevItem, hse::half(hse::stripSlot(e.strip, 6), 0, 6));
        placeRect(e.emoteNextItem, hse::half(hse::stripSlot(e.strip, 6), 1, 6));

        placeRect(e.playPauseItem, hse::stripSlot(e.strip, 7));

        placeRect(e.resetItem, hse::half(hse::stripSlot(e.strip, 8), 0, 6));
        placeRect(e.flipItem, hse::half(hse::stripSlot(e.strip, 8), 1, 6));

        // Park Mode B edit controls
        for (int i = 0; i < hse::IR_COUNT; ++i) {
            placeRect(e.inspItem[i], offscreen);
            placeRect(e.inspResetItem[i], offscreen);
        }
        for (int i = 0; i < kModeCount; ++i) placeRect(e.modeItem[i], offscreen);
        placeRect(e.boneItem, offscreen);
        placeRect(e.scopeItem, offscreen);
        placeRect(e.saveItem, offscreen);
        placeRect(e.revertItem, offscreen);
        placeRect(e.resetScopeItem, offscreen);
        placeRect(e.doneItem, offscreen);
    } else {
        // Weapon mode: nine strip rows, plus six inspector rows when the inspector exists. The rows share the strip's vertical room (the view pane
        // height): with the inspector they are squeezed to a smaller pitch, without it the layout is the vanilla stripSlot one.
        const int rows = e.inspOk ? hse::kStripRowsInsp : hse::kStripRowsBase;
        const int availH = e.viewRect.h > e.strip.h ? e.viewRect.h : e.strip.h;
        auto slot = [&](int row) { return hse::compactSlot(e.strip, availH, row, rows); };
        placeRect(e.nameItem, slot(0));
        placeRect(e.wpnRightItem, hse::half(slot(1), 0, 6));
        placeRect(e.wpnLeftItem, hse::half(slot(1), 1, 6));

        placeRect(e.boneItem, slot(2));
        placeRect(e.scopeItem, slot(3));

        placeRect(e.saveItem, hse::half(slot(4), 0, 6));
        placeRect(e.revertItem, hse::half(slot(4), 1, 6));

        placeRect(e.resetScopeItem, hse::half(slot(5), 0, 6));
        placeRect(e.doneItem, hse::half(slot(5), 1, 6));

        placeRect(e.resetItem, hse::half(slot(6), 0, 6));
        placeRect(e.flipItem, hse::half(slot(6), 1, 6));

        // Mode bar: two rows of two under the view buttons (parked when the page had no room for it)
        for (int i = 0; i < kModeCount; ++i) {
            if (e.modeBarOk) placeRect(e.modeItem[i], hse::half(slot(7 + i / 2), i % 2, 6));
            else placeRect(e.modeItem[i], offscreen);
        }

        // Inspector rows below the mode bar: the scrub button and the small reset button of each row
        for (int i = 0; i < hse::IR_COUNT; ++i) {
            if (e.inspOk) {
                hse::HseRect sc, rs;
                hse::inspRects(slot(hse::kStripRowsBase + i), &sc, &rs);
                placeRect(e.inspItem[i], sc);
                placeRect(e.inspResetItem[i], rs);
            } else {
                placeRect(e.inspItem[i], offscreen);
                placeRect(e.inspResetItem[i], offscreen);
            }
        }

        // Park Mode A nav controls
        placeRect(e.prevItem, offscreen);
        placeRect(e.nextItem, offscreen);
        placeRect(e.stanceLabelItem, offscreen);
        placeRect(e.stancePrevItem, offscreen);
        placeRect(e.stanceNextItem, offscreen);
        placeRect(e.emoteLabelItem, offscreen);
        placeRect(e.emotePrevItem, offscreen);
        placeRect(e.emoteNextItem, offscreen);
        placeRect(e.playPauseItem, offscreen);
    }
}

// ---- U3 inspector rows: open values, live texts, scrub, reset ----

// True while `counter` is under `cap` (and counts the line); logs one "capped" line when the cap is reached.
bool logRoom(int& counter, int cap, const char* what) {
    if (counter < cap) {
        ++counter;
        return true;
    }
    if (counter == cap) {
        ++counter;
        KP_LOG(chView, "%s log capped at %d lines", what, cap);
    }
    return false;
}

const char* rowUnit(int row) { return hse::inspIsRot(row) ? "deg" : "m"; }

// Remembers the open edit's values as the reset target (edit open, scope change, after Save / Revert / Reset Scope) and forces the row texts to rewrite.
void captureOpen(Editor& e, const char* why) {
    if (!refreshEdit(e)) {
        e.openValid = false;
        return;
    }
    memcpy(e.openRot, e.editInfo.rot, sizeof e.openRot);
    memcpy(e.openOff, e.editInfo.off, sizeof e.openOff);
    e.openValid = true;
    e.inspForce = true;
    if (logRoom(e.inspLogs, kMaxInspLogs, "INSP"))
        KP_LOG(chView, "INSP open why=%s hand=%s scope=%d rot=(%.3f,%.3f,%.3f) off=(%.4f,%.4f,%.4f) src=(%u,%u,%u) left=%d mirrored=%d", why, handName(e.editHand),
               e.editInfo.scope, e.editInfo.rot[0], e.editInfo.rot[1], e.editInfo.rot[2], e.editInfo.off[0], e.editInfo.off[1], e.editInfo.off[2],
               static_cast<unsigned>(e.editInfo.src[0]), static_cast<unsigned>(e.editInfo.src[1]), static_cast<unsigned>(e.editInfo.src[2]), e.editInfo.left,
               hse::leftMirrored(e.editInfo.left != 0, e.editInfo.src) ? 1 : 0);
}

// Width of a scrub button's text in UI units (0 = the strip is not measured yet: no cut).
float inspTextPx(const Editor& e) {
    if (e.strip.w <= 0) return 0.0f;
    const int rows = e.inspOk ? hse::kStripRowsInsp : hse::kStripRowsBase;
    const int availH = e.viewRect.h > e.strip.h ? e.viewRect.h : e.strip.h;
    hse::HseRect sc, rs;
    hse::inspRects(hse::compactSlot(e.strip, availH, hse::kStripRowsBase, rows), &sc, &rs);
    return static_cast<float>(sc.w);
}

// Rewrites the six row texts from a FRESH GetEdit, but only when a value, the scope or an open value changed since the last time, and only the rows whose
// string changed (SetText is the expensive call). Logs the SetText count at most once per kInspLogWindowMs (capped).
void refreshInspector(Editor& e, bool force) {
    if (!e.inspOk || e.mode != MODE_WEAPON || !g_menu.ok() || !e.page) return;
    const bool ok = refreshEdit(e);
    const DWORD now = GetTickCount();
    if (now - e.inspWinStart >= kInspLogWindowMs) {
        if (e.inspWinSets > 0 && logRoom(e.inspLogs, kMaxInspLogs, "INSP"))
            KP_LOG(chView, "INSP refresh settext=%d ticks=%d window=%ums", e.inspWinSets, e.inspWinTicks, static_cast<unsigned>(now - e.inspWinStart));
        e.inspWinStart = now;
        e.inspWinSets = 0;
        e.inspWinTicks = 0;
    }
    if (!ok) return;   // the edit was lost: the callers that noticed it leave edit mode
    const bool changed = force || e.inspForce || e.inspSeenScope != e.editInfo.scope || memcmp(e.inspSeenRot, e.editInfo.rot, sizeof e.inspSeenRot) != 0 ||
                         memcmp(e.inspSeenOff, e.editInfo.off, sizeof e.inspSeenOff) != 0;
    if (!changed) return;
    memcpy(e.inspSeenRot, e.editInfo.rot, sizeof e.inspSeenRot);
    memcpy(e.inspSeenOff, e.editInfo.off, sizeof e.inspSeenOff);
    e.inspSeenScope = e.editInfo.scope;
    if (force || e.inspForce) e.inspText.clear();
    e.inspForce = false;
    ++e.inspWinTicks;
    const float maxPx = inspTextPx(e);
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        const bool enabled = hse::inspRowEnabled(r, e.editInfo.scope);
        const float v = hse::inspGet(r, e.editInfo.rot, e.editInfo.off);
        const float open = hse::inspGet(r, e.openRot, e.openOff);
        char raw[48], fit[48];
        hse::inspRowText(raw, sizeof raw, r, v, e.openValid && hse::inspDiffers(r, v, open), enabled);
        fitButton(maxPx, raw, fit, sizeof fit);
        if (e.inspText.update(r, fit) && e.inspItem[r] >= 0) {
            g_menu.p->SetText(e.page, e.inspItem[r], fit);
            ++e.inspWinSets;
        }
    }
}

// The scrub button (row) under the cursor, else -1; any inspector button under the cursor (scrub or reset) for the chord's "press on a button" rule.
bool overItem(const Editor& e, int32_t item, int x, int y) {
    if (item < 0 || !g_menu.ok() || !e.page) return false;
    const uintptr_t c = reinterpret_cast<uintptr_t>(g_menu.p->Control(e.page, item));
    return c && guikit::hit(c, x, y);
}

int inspRowAt(const Editor& e, int x, int y) {
    if (!e.inspOk || e.mode != MODE_WEAPON) return -1;
    for (int r = 0; r < hse::IR_COUNT; ++r)
        if (overItem(e, e.inspItem[r], x, y)) return r;
    return -1;
}

bool overInspector(const Editor& e, int x, int y) {
    if (!e.inspOk || e.mode != MODE_WEAPON) return false;
    for (int r = 0; r < hse::IR_COUNT; ++r)
        if (overItem(e, e.inspItem[r], x, y) || overItem(e, e.inspResetItem[r], x, y)) return true;
    return false;
}

// Ends the running scrub (release) or drops it (cancelled: foreground loss, leave, scope change, edit closing, ...). Applied values stay as they are.
void scrubFinish(Editor& e, const char* why, bool cancelled) {
    if (!e.scrub.active) return;
    const hse::Scrub s = e.scrub;
    e.scrub.cancel();
    if (logRoom(e.scrubLogs, kMaxScrubLogs, "SCRUB"))
        KP_LOG(chView, "SCRUB %s row=%s open=%.4f%s final=%.4f%s frames=%d why=%s%s", cancelled ? "cancel" : "end", hse::inspRowName(s.row), s.open, rowUnit(s.row),
               s.value, rowUnit(s.row), s.frames, why, s.value == s.open ? " (no change)" : "");
}

void scrubCancel(Editor& e, const char* why) { scrubFinish(e, why, true); }

// SetValues with ABSOLUTE values: the fresh edit values with ONE row set to `value`. SetValues returns void, so a failure shows as a missing provider, an edit that
// is no longer open, or a read-back that is not the value sent; each is a WARN (capped per kind). False when the value did not take.
bool applyRowValue(Editor& e, int row, float value, WarnKind kind, const char* what) {
    if (!g_hol.ok()) {
        if (warnOk(e, kind)) KP_LOGV(chView, KPLOG_WARN, "%s WARN %s row=%s: holster provider missing, SetValues dropped", kind == W_SCRUB ? "SCRUB" : "INSP", what, hse::inspRowName(row));
        return false;
    }
    if (!refreshEdit(e)) {
        if (warnOk(e, kind)) KP_LOGV(chView, KPLOG_WARN, "%s WARN %s row=%s: the edit is not open", kind == W_SCRUB ? "SCRUB" : "INSP", what, hse::inspRowName(row));
        exitEdit(e);
        return false;
    }
    float rot[3], off[3];
    memcpy(rot, e.editInfo.rot, sizeof rot);
    memcpy(off, e.editInfo.off, sizeof off);
    hse::inspSet(row, rot, off, value);
    g_hol.h->SetValues(rot, off);
    if (!refreshEdit(e)) {
        if (warnOk(e, kind)) KP_LOGV(chView, KPLOG_WARN, "%s WARN %s row=%s: the edit closed after SetValues", kind == W_SCRUB ? "SCRUB" : "INSP", what, hse::inspRowName(row));
        exitEdit(e);
        return false;
    }
    const float got = hse::inspGet(row, e.editInfo.rot, e.editInfo.off);
    if (!hse::inspMatches(row, got, value)) {
        if (warnOk(e, kind))
            KP_LOGV(chView, KPLOG_WARN, "%s WARN %s row=%s: SetValues did not take, sent %.5f read back %.5f (scope=%d%s)", kind == W_SCRUB ? "SCRUB" : "INSP", what,
                    hse::inspRowName(row), value, got, e.editInfo.scope, e.editInfo.scope == K2HOLSTER_SCOPE_BODY && hse::inspIsRot(row) ? " body: rotation is ignored" : "");
        return false;
    }
    return true;
}

// LMB press on a row button: start the scrub at the row's current value.
void scrubBegin(Editor& e, int row, int x, int y, bool shift, bool ctrl) {
    if (!refreshEdit(e)) {
        if (warnOk(e, W_SCRUB)) KP_LOGV(chView, KPLOG_WARN, "SCRUB WARN begin row=%s: the edit is not open", hse::inspRowName(row));
        return;
    }
    if (!hse::inspRowEnabled(row, e.editInfo.scope)) {
        if (logRoom(e.scrubLogs, kMaxScrubLogs, "SCRUB"))
            KP_LOG(chView, "SCRUB ignore row=%s: no rotation in the body scope at=(%d,%d)", hse::inspRowName(row), x, y);
        return;
    }
    const float v = hse::inspGet(row, e.editInfo.rot, e.editInfo.off);
    if (!e.scrub.begin(row, x, v)) {
        if (warnOk(e, W_SCRUB)) KP_LOGV(chView, KPLOG_WARN, "SCRUB WARN begin refused row=%s open=%.5f (not a finite value, or a scrub is running)", hse::inspRowName(row), v);
        return;
    }
    if (logRoom(e.scrubLogs, kMaxScrubLogs, "SCRUB"))
        KP_LOG(chView, "SCRUB begin row=%s open=%.4f%s at=(%d,%d) window=[%.4f,%.4f] scope=%d hand=%s shift=%d ctrl=%d", hse::inspRowName(row), v, rowUnit(row), x, y,
               e.scrub.lo, e.scrub.hi, e.editInfo.scope, handName(e.editHand), shift ? 1 : 0, ctrl ? 1 : 0);
}

void scrubStepTick(Editor& e, int x, bool shift, bool ctrl) {
    const int row = e.scrub.row;
    const hse::ScrubStep st = e.scrub.step(x, shift, ctrl);
    if (st.clampedNow && logRoom(e.scrubLogs, kMaxScrubLogs, "SCRUB"))
        KP_LOGV(chView, KPLOG_WARN, "SCRUB CLAMP row=%s %s end at %.4f%s (%s) open=%.4f window=[%.4f,%.4f]", hse::inspRowName(row), st.clampSide < 0 ? "low" : "high",
                e.scrub.value, rowUnit(row), st.hardLimit ? "holster.ini limit" : "scrub window", e.scrub.open, e.scrub.lo, e.scrub.hi);
    if (!st.changed) return;
    if (applyRowValue(e, row, st.value, W_SCRUB, "scrub")) updateStatusText(e);
}

// One tick of the row scrub, after the chord has seen the mouse. `co` is the chord's answer for this tick, `lEdge` the LMB press edge.
// A scrub begins only when the chord turned an IDLE press into a DEAD "press_outside" (so a press on a button, never a foreground-return or stale
// press) with the cursor on a row button and no other button down. It runs while LMB is held, ends on release, and is cancelled by foreground loss.
void scrubTick(Editor& e, const hse::ChordOut& co, bool lEdge, int x, int y, bool fg, bool lmb, bool rmb, bool mmb, bool shift, bool ctrl) {
    if (e.scrub.active) {
        if (!fg) {
            scrubFinish(e, "fg_lost", true);
        } else if (!lmb) {
            scrubFinish(e, "release", false);
            updateStatusText(e);
        } else {
            scrubStepTick(e, x, shift, ctrl);
        }
        return;
    }
    if (!e.inspOk || !fg || !lEdge || rmb || mmb) return;
    if (co.drop != hse::CW_PRESS_OUTSIDE || co.why != hse::CW_PRESS_OUTSIDE) return;
    const int row = inspRowAt(e, x, y);
    if (row < 0) return;
    scrubBegin(e, row, x, y, shift, ctrl);
}

// The R button: that one value goes back to what it was when the edit opened. The other five are sent unchanged.
void inspReset(Editor& e, int row) {
    if (row < 0 || row >= hse::IR_COUNT) return;
    if (!e.openValid) {
        if (warnOk(e, W_INSP)) KP_LOGV(chView, KPLOG_WARN, "INSP WARN RESET row=%s: no open values stored", hse::inspRowName(row));
        return;
    }
    scrubCancel(e, "reset");
    if (!refreshEdit(e)) {
        if (warnOk(e, W_INSP)) KP_LOGV(chView, KPLOG_WARN, "INSP WARN RESET row=%s: the edit is not open", hse::inspRowName(row));
        exitEdit(e);
        return;
    }
    if (!hse::inspRowEnabled(row, e.editInfo.scope)) {
        if (logRoom(e.inspLogs, kMaxInspLogs, "INSP")) KP_LOG(chView, "INSP RESET row=%s ignored: no rotation in the body scope", hse::inspRowName(row));
        return;
    }
    float rot[3], off[3];
    const float from = hse::inspGet(row, e.editInfo.rot, e.editInfo.off);
    const float to = hse::inspGet(row, e.openRot, e.openOff);
    const bool changes = hse::inspRevert(row, e.editInfo.rot, e.editInfo.off, e.openRot, e.openOff, rot, off);
    if (logRoom(e.inspLogs, kMaxInspLogs, "INSP"))
        KP_LOG(chView, "INSP RESET row=%s from=%.4f%s to=%.4f%s %s", hse::inspRowName(row), from, rowUnit(row), to, rowUnit(row), changes ? "applied" : "(already at the open value)");
    if (!changes) return;
    if (applyRowValue(e, row, to, W_INSP, "reset")) updateStatusText(e);
}

void enterEdit(Editor& e, int hand) {
    if (hand < 0 || hand > 1 || !e.regGob[hand] || !g_hol.ok()) return;
    const bool wasWeapon = e.mode == MODE_WEAPON;
    if (wasWeapon) {
        g_hol.h->EndEdit();
    }
    if (!g_hol.h->BeginEdit(e.regGob[hand], e.editScope)) {
        KP_LOG(chView, "EDIT refused hand=%d gob=%p scope=%d", hand, e.regGob[hand], e.editScope);
        e.mode = MODE_NAV;   // the previous edit (if any) was just ended
        e.editHand = -1;
        e.projEditPending = false;
        resetInput(e, "edit_refused");
        if (wasWeapon) setHandNames(e);
        applyLayout(e);
        if (g_menu.ok() && e.statusItem >= 0) g_menu.p->SetText(e.page, e.statusItem, "Edit unavailable");
        return;
    }
    e.mode = MODE_WEAPON;
    e.editHand = hand;
    resetInput(e, "enter_edit");   // drags from the previous mode are dropped; a button still held (the click that entered) must be released first
    e.projEditPending = true;
    e.projEditTries = 0;
    e.shownDirty = -1;   // the status line shows the EDIT numbers first, then the help line
    refreshEdit(e);
    KP_LOG(chView, "EDIT enter hand=%d gob=%p model=%s bone=%s", hand, e.regGob[hand], e.editInfo.model, e.editInfo.bone);
    e.inspText.clear();   // the first refresh rewrites every row text
    captureOpen(e, "enter_edit");

    const K2MenuUiApi* ui = g_menu.p;
    char bBuf[64];
    snprintf(bBuf, sizeof(bBuf), "Bone: %s", e.editInfo.bone[0] ? e.editInfo.bone : "None");
    if (e.boneItem >= 0) ui->SetText(e.page, e.boneItem, bBuf);
    setScopeText(e);
    setModeBarText(e);

    char rBuf[64], lBuf[64];
    snprintf(rBuf, sizeof(rBuf), "%sRight%s", hand == 0 ? "[Edit: " : "", hand == 0 ? "]" : (e.regGob[0] ? "" : ": None"));
    snprintf(lBuf, sizeof(lBuf), "%sLeft%s", hand == 1 ? "[Edit: " : "", hand == 1 ? "]" : (e.regGob[1] ? "" : ": None"));
    if (e.wpnRightItem >= 0) ui->SetText(e.page, e.wpnRightItem, rBuf);
    if (e.wpnLeftItem >= 0) ui->SetText(e.page, e.wpnLeftItem, lBuf);

    applyLayout(e);
    updateStatusText(e);
    refreshInspector(e, true);
}

void exitEdit(Editor& e) {
    if (e.mode != MODE_WEAPON) return;
    if (g_hol.ok()) g_hol.h->EndEdit();
    e.mode = MODE_NAV;
    e.editHand = -1;
    e.projEditPending = false;
    resetInput(e, "exit_edit");
    KP_LOG(chView, "EDIT exit");

    setHandNames(e);
    applyLayout(e);
    updateStatusText(e);
}

void KPATCH_CALL cbEnter(uint32_t page, void*) {
    if (!g_ed.panel || page != g_ed.page || !g_menu.ok()) return;
    saferead::beginScope();
    placeItem(g_ed.viewItem, MUI_PANE_VIEW, "view");
    placeItem(g_ed.statusItem, MUI_PANE_STATUS, "status");
    if (g_ed.statusItem >= 0) g_menu.p->SetText(g_ed.page, g_ed.statusItem, statusText());

    K2MuiRect col = {0, 0, 0, 0}, view = {0, 0, 0, 0};
    g_menu.p->PaneRect(g_ed.panel, MUI_PANE_COLUMN, &col);
    g_menu.p->PaneRect(g_ed.panel, MUI_PANE_VIEW, &view);
    hse::HseRect hcol = {col.x, col.y, col.w, col.h};
    hse::HseRect hview = {view.x, view.y, view.w, view.h};
    g_ed.viewRect = hview;
    K2MuiRect st = {0, 0, 0, 0};
    if (g_menu.p->PaneRect(g_ed.panel, MUI_PANE_STATUS, &st)) g_ed.statusRect = {st.x, st.y, st.w, st.h};
    g_ed.strip = hse::leftStrip(hcol, hview, 6);
    KP_LOG(chView, "LAYOUT strip=(%d,%d,%d,%d)", g_ed.strip.x, g_ed.strip.y, g_ed.strip.w, g_ed.strip.h);

    applyLayout(g_ed);

    guikit::wheelInstall();
    guikit::g_wheel.setRect(0, view.x, view.y, view.w, view.h);
    (void)guikit::g_wheel.take();   // drop any notches counted before this page was shown
    guikit::g_wheel.touch(GetTickCount());

    g_ed.view = g_ed.cfgView;
    g_ed.state = NEEDBUILD;
}

void KPATCH_CALL cbLeave(uint32_t, void*) {
    saferead::beginScope();
    guikit::g_wheel.clearRect(0);   // stop the wheel hook counting this page's area
    g_ed.discardArmed = false;
    if (g_ed.mode == MODE_WEAPON) {
        if (editDirty()) KP_LOG(chView, "DISCARD why=leave");
        exitEdit(g_ed);
    }
    g_ed.state = IDLE;
    if (g_probeCalib) {
        KP_LOG(chView, "CALIB leave rounds=%d clicks=%d", g_ed.calib.round - 1, g_ed.calib.clicks);
        if (g_menu.ok() && g_ed.page && g_ed.statusItem >= 0) g_menu.p->SetText(g_ed.page, g_ed.statusItem, statusText());   // restore the normal text
    }
    resetInput(g_ed, "leave");
    g_ed.pressed = false;   // the page is gone: a button still held must not look like "was down" next time
    g_ed.pressedMmb = false;
    g_ed.pressedRmb = false;
    g_ed.playingEmote = false;
    g_ed.emoteEndMs = 0;
    if (g_ed.token && g_p3.ok()) {
        g_p3.p->Destroy(g_ed.token);
    }
    KP_LOG(chView, "LEAVE destroy");
}

// Two-click confirm for transitions that would drop unsaved edits. Returns true when the caller may proceed (nothing unsaved, or the
// same action was armed within kDiscardWindowMs). The first click on a dirty edit arms the window and shows the warning instead.
bool guardTransition(Editor& e, int action, const char* why) {
    const bool dirty = editDirty();   // fresh read, not e.editInfo
    const DWORD now = GetTickCount();
    const bool armedSame = e.discardArmed && e.discardAction == action;
    if (hse::guardProceed(dirty, armedSame, e.discardArmedAt, now, kDiscardWindowMs)) {
        if (dirty) KP_LOG(chView, "DISCARD why=%s", why);
        e.discardArmed = false;
        return true;
    }
    e.discardArmed = true;
    e.discardAction = action;
    e.discardArmedAt = now;
    KP_LOG(chView, "DISCARD armed why=%s", why);
    updateStatusText(e);
    return false;
}

// True when the cursor is over one of the mode bar buttons (a press there is a button click, never a view gesture).
bool overModeBar(const Editor& e, int x, int y) {
    if (!e.modeBarOk || !g_menu.ok()) return false;
    for (int i = 0; i < kModeCount; ++i) {
        const uintptr_t c = e.modeItem[i] >= 0 ? reinterpret_cast<uintptr_t>(g_menu.p->Control(e.page, e.modeItem[i])) : 0;
        if (c && guikit::hit(c, x, y)) return true;
    }
    return false;
}

// Sends the provider calls of one tick and checks they took. NudgeWorld/NudgeLocal return void, so a failure shows as: the provider is gone, the
// edit is no longer open, or the open values did not change at all. Each case is a NUDGE WARN line (capped). False when the edit was lost.
bool applyCalls(Editor& e, const hse::WeaponCall* calls, int n, const char* what) {
    if (n <= 0) return true;
    if (!g_hol.ok()) {
        if (warnOk(e, W_NUDGE)) KP_LOGV(chView, KPLOG_WARN, "NUDGE WARN %s: holster provider missing (%d call%s dropped)", what, n, n == 1 ? "" : "s");
        return false;
    }
    float rot[3], off[3];
    memcpy(rot, e.editInfo.rot, sizeof rot);
    memcpy(off, e.editInfo.off, sizeof off);
    for (int i = 0; i < n; ++i) {
        if (calls[i].kind == hse::WK_WORLD) g_hol.h->NudgeWorld(nullptr, calls[i].dp);
        else if (calls[i].kind == hse::WK_LOCAL) g_hol.h->NudgeLocal(calls[i].axis, calls[i].deg, calls[i].metres);
    }
    if (!refreshEdit(e)) {
        if (warnOk(e, W_NUDGE)) KP_LOGV(chView, KPLOG_WARN, "NUDGE WARN %s: the edit is no longer open after %d call%s", what, n, n == 1 ? "" : "s");
        exitEdit(e);
        return false;
    }
    if (!memcmp(rot, e.editInfo.rot, sizeof rot) && !memcmp(off, e.editInfo.off, sizeof off) && warnOk(e, W_NUDGE)) {
        const hse::WeaponCall& c = calls[0];
        KP_LOGV(chView, KPLOG_WARN, "NUDGE WARN %s: no change (call0 kind=%d axis=%d deg=%.4f m=%.4f dp=(%.4f,%.4f,%.4f) scope=%d%s)", what, c.kind, c.axis, c.deg, c.metres,
                c.dp[0], c.dp[1], c.dp[2], e.editScope, e.editScope == K2HOLSTER_SCOPE_BODY ? " body: rotation is ignored" : "");
    }
    return true;
}

// Weapon-mode mouse for one tick: feeds hse::Chord and acts on what it answers. All logging of the chord lives here.
hse::ChordOut weaponMouse(Editor& e, int curX, int curY, bool insideView, bool fg, bool shift, bool ctrl, bool lmb, bool rmb, bool mmb, LONG wheel) {
    hse::ChordIn ci = {};
    ci.l = lmb;
    ci.r = rmb;
    ci.m = mmb;
    ci.lEdge = lmb && !e.pressed;
    ci.rEdge = rmb && !e.pressedRmb;
    ci.mEdge = mmb && !e.pressedMmb;
    e.pressed = lmb;
    e.pressedRmb = rmb;
    e.pressedMmb = mmb;
    ci.inView = insideView && !overModeBar(e, curX, curY) && !overInspector(e, curX, curY);   // a press on any button is a dead press (U3: the row scrub starts from it)
    ci.fg = fg;
    ci.shift = shift;
    ci.ctrl = ctrl;
    ci.x = curX;
    ci.y = curY;
    ci.wheel = static_cast<int>(wheel);
    if (!rmb && e.snap.nonzero() && !e.gActive) resetSnap(e, "rmb_up");   // a wheel remainder does not outlive the held RMB

    const hse::ChordOut o = e.chord.step(ci);

    // CHORD log: every state change or gesture begin/end with its reason. A press that is dropped (outside the view, over a button, ...) is a
    // drop line, capped separately; when the drop is also the transition reason (-> DEAD) the transition line is the drop line.
    const bool transition = o.state != o.prevState || o.why != hse::CW_NONE;
    const bool dropIsTransition = transition && o.drop != hse::CW_NONE && o.drop == o.why;
    if (transition) {
        int& counter = dropIsTransition ? e.dropLogs : e.chordLogs;
        const int cap = dropIsTransition ? kMaxDropLogs : kMaxChordLogs;
        if (counter < cap) {
            ++counter;
            KP_LOG(chView, "CHORD %s->%s why=%s began=%s ended=%s at=(%d,%d) btn=%d%d%d view=%d fg=%d", hse::chordStateName(o.prevState), hse::chordStateName(o.state),
                   hse::chordWhyName(o.why), hse::chordActName(o.began), hse::chordActName(o.ended), curX, curY, lmb ? 1 : 0, rmb ? 1 : 0, mmb ? 1 : 0,
                   ci.inView ? 1 : 0, fg ? 1 : 0);
        } else if (counter == cap) {
            ++counter;
            KP_LOG(chView, "CHORD %s log capped at %d lines", dropIsTransition ? "drop" : "transition", cap);
        }
    }
    if (o.drop != hse::CW_NONE && !dropIsTransition) {
        if (e.dropLogs < kMaxDropLogs) {
            ++e.dropLogs;
            KP_LOG(chView, "CHORD drop reason=%s state=%s at=(%d,%d) btn=%d%d%d view=%d", hse::chordWhyName(o.drop), hse::chordStateName(o.state), curX, curY, lmb ? 1 : 0,
                   rmb ? 1 : 0, mmb ? 1 : 0, ci.inView ? 1 : 0);
        } else if (e.dropLogs == kMaxDropLogs) {
            ++e.dropLogs;
            KP_LOG(chView, "CHORD drop log capped at %d lines", kMaxDropLogs);
        }
    }

    // Gestures that ended
    if (o.ended == hse::CA_ORBIT) {
        p3d::dragEnd(&e.drag);
        KP_LOG(chView, "DRAG end yaw=%.2f panZ=%.2f", e.view.yaw, e.view.panZ);
    } else if (o.ended == hse::CA_WEAPON) {
        logGestureEnd(e, hse::chordWhyName(o.why));
        resetSnap(e, "gesture_end");
        updateStatusText(e);
    } else if (o.ended == hse::CA_SPIN) {
        KP_LOG(chView, "MMB spin end yaw=%.2f why=%s", e.view.yaw, hse::chordWhyName(o.why));
    }

    // Gestures that began
    if (o.began == hse::CA_ORBIT) {
        if (!e.drag.active) {
            p3d::dragBegin(&e.drag, static_cast<float>(curX), static_cast<float>(curY));
            KP_LOG(chView, "DRAG begin pos=(%d,%d)", curX, curY);
        }
    } else if (o.began == hse::CA_WEAPON) {
        refreshEdit(e);
        e.gActive = true;
        e.gMode = e.weaponMode;   // the mode bar can change meanwhile; this gesture keeps the mode it began with
        e.gHand = e.editHand == 1 ? 1 : 0;
        e.gFrames = 0;
        e.gDx = e.gDy = 0;
        e.gA = e.gB = 0.0f;
        resetSnap(e, "gesture_begin");
        if (e.gestureLogs < kMaxGestureLogs) {
            ++e.gestureLogs;
            KP_LOG(chView, "GESTURE begin mode=%s hand=%s scope=%d zoom=%.2f shift=%d ctrl=%d how=%s", hse::weaponModeName(e.gMode), handName(e.gHand), e.editScope,
                   e.view.zoom, shift ? 1 : 0, ctrl ? 1 : 0, hse::chordWhyName(o.why));
        }
        updateStatusText(e);
    } else if (o.began == hse::CA_SPIN) {
        KP_LOG(chView, "MMB spin begin yaw=%.2f", e.view.yaw);
    }

    // The running gesture
    if (o.act == hse::CA_ORBIT && o.began != hse::CA_ORBIT && e.drag.active) {
        p3d::ViewState vs = {e.view.yaw, e.view.zoom, e.view.panZ};
        if (p3d::dragMove(&e.drag, static_cast<float>(curX), static_cast<float>(curY), e.dragCfg, &vs)) {
            e.view.yaw = hse::wrapYaw(vs.yaw);
            e.view.panZ = hse::panStep(vs.panZ, 0);   // clamp to [-1, 1]
        }
    } else if (o.act == hse::CA_SPIN && (o.dx != 0)) {
        e.view.yaw = hse::spin(e.view.yaw, static_cast<float>(o.dx));   // dx > 0 turns the yaw up, same sign as NAV and the equip screen
    } else if (o.act == hse::CA_WEAPON && e.gActive && (o.dx != 0 || o.dy != 0)) {
        const hse::WeaponStep st = hse::weaponGesture(e.gMode, o.dx, o.dy, e.view.zoom, o.fine, o.snap, e.snap);
        ++e.gFrames;
        e.gDx += o.dx;
        e.gDy += o.dy;
        if (st.n > 0) {
            if (applyCalls(e, st.c, st.n, hse::weaponModeName(e.gMode))) {
                e.gA += st.a;
                e.gB += st.b;
                updateStatusText(e);
            }
        }
    }

    // Wheel: RMB + wheel is depth, otherwise zoom
    if (o.wheelAct == hse::CA_DEPTH && e.mode == MODE_WEAPON) {
        const hse::WeaponCall c = hse::wheelDepth(o.wheel, o.fine, o.snap, e.snap);
        KP_LOG(chView, "WHEEL depth delta=%d metres=%.4f fine=%d snap=%d carry=%.4f", o.wheel, c.kind == hse::WK_LOCAL ? c.metres : 0.0f, o.fine ? 1 : 0, o.snap ? 1 : 0,
               e.snap.a[2]);
        if (c.kind != hse::WK_NONE && applyCalls(e, &c, 1, "WheelDepth")) updateStatusText(e);
    } else if (o.wheelAct == hse::CA_ZOOM && e.token && g_p3.ok()) {
        e.view.zoom = p3d::zoomStep(e.view.zoom, o.wheel, 0.9f, hse::kZoomMin, hse::kZoomMax);
        KP_LOG(chView, "WHEEL delta=%d zoom=%.2f", o.wheel, e.view.zoom);
    }
    return o;
}

void KPATCH_CALL cbTick(uint32_t, void*) {
    g_p3.poll();
    g_hol.poll();
    if (!g_ed.panel || !g_menu.ok() || !g_ed.page) return;

    if (!g_p3.ok() || !g_hol.ok() || !g_ed.token) {
        if (g_ed.chord.busy() || g_ed.gActive || g_ed.scrub.active) resetInput(g_ed, "provider_lost");
        if (g_ed.statusItem >= 0) g_menu.p->SetText(g_ed.page, g_ed.statusItem, statusText());
        return;
    }
    saferead::beginScope();
    Editor& e = g_ed;
    const uint32_t page = e.page;
    const K2MenuUiApi* ui = g_menu.p;
    const int n = hse::memberCount();

    // 1. Button clicks in LIVE state
    if ((e.state == LIVE || e.state == FAILED) && n > 0) {
        // Navigation buttons (work in both modes; auto-exit weapon edit mode on member switch)
        if (e.prevItem >= 0 && ui->Clicked(page, e.prevItem)) {
            if (e.mode == MODE_WEAPON) {
                if (!guardTransition(e, ACT_PREV, "member")) {
                    resetInput(e, "discard_guard");
                    return;
                }
                exitEdit(e);
            }
            e.member = hse::wrapIndex(e.member, -1, n);
            KP_LOG(chView, "PICK member=%d/%d", e.member + 1, n);
            if (e.token && g_p3.ok()) g_p3.p->Destroy(e.token);
            e.state = NEEDBUILD;   // the stance UI is reset in NEEDBUILD, before the rebuild
            return;
        } else if (e.nextItem >= 0 && ui->Clicked(page, e.nextItem)) {
            if (e.mode == MODE_WEAPON) {
                if (!guardTransition(e, ACT_NEXT, "member")) {
                    resetInput(e, "discard_guard");
                    return;
                }
                exitEdit(e);
            }
            e.member = hse::wrapIndex(e.member, 1, n);
            KP_LOG(chView, "PICK member=%d/%d", e.member + 1, n);
            if (e.token && g_p3.ok()) g_p3.p->Destroy(e.token);
            e.state = NEEDBUILD;
            return;
        } else if (e.resetItem >= 0 && ui->Clicked(page, e.resetItem)) {
            e.view = e.cfgView;
            KP_LOG(chView, "BTN Reset view=(%.2f,%.2f,%.2f)", e.view.yaw, e.view.zoom, e.view.panZ);
        } else if (e.flipItem >= 0 && ui->Clicked(page, e.flipItem)) {
            e.view.yaw = hse::flipYaw(e.view.yaw);
            KP_LOG(chView, "BTN Flip view=(%.2f,%.2f,%.2f)", e.view.yaw, e.view.zoom, e.view.panZ);
        }

        if (e.state != LIVE) {
            // FAILED: only the picker and the view buttons above work until a rebuild succeeds
        } else if (e.mode == MODE_NAV) {
            // Mode NAV buttons
            if (e.wpnRightItem >= 0 && ui->Clicked(page, e.wpnRightItem) && e.regGob[0]) {
                enterEdit(e, 0);
            } else if (e.wpnLeftItem >= 0 && ui->Clicked(page, e.wpnLeftItem) && e.regGob[1]) {
                enterEdit(e, 1);
            } else if (e.stancePrevItem >= 0 && ui->Clicked(page, e.stancePrevItem)) {
                e.stanceIdx = hse::wrapIndex(e.stanceIdx, -1, kStanceCount);
                e.playingEmote = false;
                e.emoteEndMs = 0;
                char sBuf[64];
                snprintf(sBuf, sizeof(sBuf), "Stance: %s", kStanceList[e.stanceIdx].label);
                if (e.stanceLabelItem >= 0) ui->SetText(page, e.stanceLabelItem, sBuf);
                if (e.token && g_p3.ok()) g_p3.p->PlayAnim(e.token, kStanceList[e.stanceIdx].anim);
                const bool isMoving = (e.stanceIdx != 0);
                if (e.playPauseItem >= 0) ui->SetText(page, e.playPauseItem, isMoving ? "Pause" : "Play");
                KP_LOG(chView, "STANCE sel=%s anim=%s", kStanceList[e.stanceIdx].label, kStanceList[e.stanceIdx].anim);
            } else if (e.stanceNextItem >= 0 && ui->Clicked(page, e.stanceNextItem)) {
                e.stanceIdx = hse::wrapIndex(e.stanceIdx, 1, kStanceCount);
                e.playingEmote = false;
                e.emoteEndMs = 0;
                char sBuf[64];
                snprintf(sBuf, sizeof(sBuf), "Stance: %s", kStanceList[e.stanceIdx].label);
                if (e.stanceLabelItem >= 0) ui->SetText(page, e.stanceLabelItem, sBuf);
                if (e.token && g_p3.ok()) g_p3.p->PlayAnim(e.token, kStanceList[e.stanceIdx].anim);
                const bool isMoving = (e.stanceIdx != 0);
                if (e.playPauseItem >= 0) ui->SetText(page, e.playPauseItem, isMoving ? "Pause" : "Play");
                KP_LOG(chView, "STANCE sel=%s anim=%s", kStanceList[e.stanceIdx].label, kStanceList[e.stanceIdx].anim);
            } else if (e.emotePrevItem >= 0 && ui->Clicked(page, e.emotePrevItem)) {
                e.emoteIdx = hse::wrapIndex(e.emoteIdx, -1, kEmoteCount);
                char eBuf[64];
                snprintf(eBuf, sizeof(eBuf), "Emote: %s", kEmoteList[e.emoteIdx].label);
                if (e.emoteLabelItem >= 0) ui->SetText(page, e.emoteLabelItem, eBuf);
                KP_LOG(chView, "EMOTE sel=%s anim=%s", kEmoteList[e.emoteIdx].label, kEmoteList[e.emoteIdx].anim);
            } else if (e.emoteNextItem >= 0 && ui->Clicked(page, e.emoteNextItem)) {
                e.emoteIdx = hse::wrapIndex(e.emoteIdx, 1, kEmoteCount);
                char eBuf[64];
                snprintf(eBuf, sizeof(eBuf), "Emote: %s", kEmoteList[e.emoteIdx].label);
                if (e.emoteLabelItem >= 0) ui->SetText(page, e.emoteLabelItem, eBuf);
                KP_LOG(chView, "EMOTE sel=%s anim=%s", kEmoteList[e.emoteIdx].label, kEmoteList[e.emoteIdx].anim);
            } else if (e.playPauseItem >= 0 && ui->Clicked(page, e.playPauseItem)) {
                if (e.playingEmote || e.stanceIdx != 0) {
                    resetStanceUi(e);
                    if (e.token && g_p3.ok()) g_p3.p->PlayAnim(e.token, "pause1");
                    KP_LOG(chView, "PLAY_PAUSE -> Pause (returned to idle)");
                } else {
                    const EmoteDef& em = kEmoteList[e.emoteIdx];
                    if (e.token && g_p3.ok()) g_p3.p->PlayAnim(e.token, em.anim);
                    e.playingEmote = true;
                    e.emoteEndMs = em.duration > 0.0f ? GetTickCount() + static_cast<DWORD>(em.duration * 1000.0f) : 0;
                    ui->SetText(page, e.playPauseItem, "Pause");
                    KP_LOG(chView, "PLAY_PAUSE -> Play emote=%s dur=%.1f", em.anim, em.duration);
                }
            }
        } else {
            // Mode WEAPON buttons
            if (e.wpnRightItem >= 0 && ui->Clicked(page, e.wpnRightItem)) {
                if ((e.editHand != 0 && e.regGob[0]) || e.editHand == 0) {
                    if (guardTransition(e, ACT_HAND_R, "hand")) {
                        if (e.editHand != 0) enterEdit(e, 0);
                        else exitEdit(e);
                    }
                }
            } else if (e.wpnLeftItem >= 0 && ui->Clicked(page, e.wpnLeftItem)) {
                if ((e.editHand != 1 && e.regGob[1]) || e.editHand == 1) {
                    if (guardTransition(e, ACT_HAND_L, "hand")) {
                        if (e.editHand != 1) enterEdit(e, 1);
                        else exitEdit(e);
                    }
                }
            } else if (e.boneItem >= 0 && ui->Clicked(page, e.boneItem) && g_hol.ok()) {
                int newBone = g_hol.h->CycleBone(1);   // returns a bone index, never a success flag
                if (!refreshEdit(e)) {
                    KP_LOG(chView, "EDIT lost after CycleBone idx=%d", newBone);
                    exitEdit(e);
                } else {
                    setBoneText(e);
                    KP_LOG(chView, "BTN CycleBone -> %d (%s)", newBone, e.editInfo.bone);
                    updateStatusText(e);
                }
            } else if (e.scopeItem >= 0 && ui->Clicked(page, e.scopeItem) && g_hol.ok()) {
                if (guardTransition(e, ACT_SCOPE, "scope")) {
                    e.editScope = (e.editScope + 1) % 3;
                    const int sc = g_hol.h->SetScope(e.editScope, e.editHand == 1 ? 1 : 0);
                    if (!sc || !refreshEdit(e)) {
                        KP_LOG(chView, "EDIT lost after SetScope scope=%d rc=%d", e.editScope, sc);
                        exitEdit(e);
                    } else {
                        resetInput(e, "scope");
                        captureOpen(e, "scope");   // the per-row reset after a scope switch goes back to the NEW scope's open value
                        setScopeText(e);
                        setBoneText(e);
                        KP_LOG(chView, "BTN SetScope -> %d (%s)", e.editScope, hse::scopeLabel(e.editScope));
                        updateStatusText(e);
                    }
                }
            } else if (e.saveItem >= 0 && ui->Clicked(page, e.saveItem) && g_hol.ok()) {
                int rc = g_hol.h->Save();
                KP_LOG(chView, "BTN Save rc=%d", rc);
                if (rc) captureOpen(e, "save");   // what was written is the new reset target
                refreshEdit(e);
                updateStatusText(e);
                if (!rc && e.statusItem >= 0) ui->SetText(page, e.statusItem, "Save failed, not written (see log)");
            } else if (e.revertItem >= 0 && ui->Clicked(page, e.revertItem) && g_hol.ok()) {
                g_hol.h->Revert();
                KP_LOG(chView, "BTN Revert");
                captureOpen(e, "revert");
                refreshEdit(e);
                setBoneText(e);
                updateStatusText(e);
            } else if (e.resetScopeItem >= 0 && ui->Clicked(page, e.resetScopeItem) && g_hol.ok()) {
                if (guardTransition(e, ACT_RESET_SCOPE, "reset_scope")) {
                    g_hol.h->ResetScope();
                    KP_LOG(chView, "BTN ResetScope scope=%d", e.editScope);
                    captureOpen(e, "reset_scope");
                    refreshEdit(e);
                    setBoneText(e);
                    updateStatusText(e);
                }
            } else if (e.doneItem >= 0 && ui->Clicked(page, e.doneItem)) {
                if (guardTransition(e, ACT_DONE, "done")) exitEdit(e);
            } else if (e.modeBarOk) {
                for (int i = 0; i < kModeCount; ++i) {
                    if (e.modeItem[i] >= 0 && ui->Clicked(page, e.modeItem[i])) {
                        setWeaponMode(e, i, "click");
                        break;
                    }
                }
            }
            if (e.mode == MODE_WEAPON && e.inspOk) {
                // The scrub buttons act on the press, not on the click: drain their release-edge latch. The R buttons put one value back.
                for (int i = 0; i < hse::IR_COUNT; ++i) (void)ui->Clicked(page, e.inspItem[i]);
                for (int i = 0; i < hse::IR_COUNT; ++i) {
                    if (e.inspResetItem[i] >= 0 && ui->Clicked(page, e.inspResetItem[i])) {
                        inspReset(e, i);
                        break;
                    }
                }
            }
        }
    }

    // 2. Emote timer: auto-return to active stance / idle
    if (e.discardArmed && !hse::guardArmLive(true, e.discardArmedAt, GetTickCount(), kDiscardWindowMs)) {
        e.discardArmed = false;   // the confirm window ran out: back to the plain status line
        updateStatusText(e);
    }
    if (e.mode == MODE_WEAPON && e.statusHolding && !e.gActive && !e.discardArmed && static_cast<int32_t>(e.statusHoldUntil - GetTickCount()) <= 0)
        updateStatusText(e);   // the numbers have been up long enough: back to the help line
    if (e.playingEmote && e.emoteEndMs && GetTickCount() >= e.emoteEndMs) {
        e.playingEmote = false;
        e.emoteEndMs = 0;
        const char* resumeAnim = kStanceList[e.stanceIdx].anim;
        if (e.token && g_p3.ok()) g_p3.p->PlayAnim(e.token, resumeAnim);
        const bool isMoving = (e.stanceIdx != 0);
        if (e.playPauseItem >= 0) ui->SetText(page, e.playPauseItem, isMoving ? "Pause" : "Play");
        KP_LOG(chView, "EMOTE done -> resume anim=%s", resumeAnim);
    }

    // 3. Mouse input. Nav mode: equip-screen style drag (LMB orbit/pan), MMB click reset, wheel zoom. Weapon mode: hse::Chord (weaponMouse).
    int curX = 0, curY = 0;
    guikit::cursorEngine(&curX, &curY);
    uintptr_t ctl = e.viewItem >= 0 ? reinterpret_cast<uintptr_t>(ui->Control(page, e.viewItem)) : 0;
    const bool insideView = ctl && guikit::hit(ctl, curX, curY);
    const bool fg = guikit::g_gk.gameForeground ? guikit::g_gk.gameForeground() : true;
    const bool shift = fg && guikit::g_gk.shiftDown && guikit::g_gk.shiftDown();
    const bool ctrl = fg && guikit::g_gk.ctrlDown && guikit::g_gk.ctrlDown();

    guikit::g_wheel.touch(GetTickCount());
    const LONG w = guikit::g_wheel.take();
    const bool lmb = fg && guikit::g_gk.lbuttonDown && guikit::g_gk.lbuttonDown();
    const bool rmb = fg && guikit::g_gk.rbuttonDown && guikit::g_gk.rbuttonDown();
    const bool mmb = fg && guikit::g_gk.mbuttonDown && guikit::g_gk.mbuttonDown();

    if (e.mode == MODE_WEAPON) {
        const bool lEdge = lmb && !e.pressed;   // weaponMouse updates the "was down" flags
        const hse::ChordOut co = weaponMouse(e, curX, curY, insideView, fg, shift, ctrl, lmb, rmb, mmb, w);
        if (e.mode == MODE_WEAPON) scrubTick(e, co, lEdge, curX, curY, fg, lmb, rmb, mmb, shift, ctrl);
        if (e.mode == MODE_WEAPON) refreshInspector(e, false);
    } else {
        // Mode NAV
        if (w != 0 && e.token && g_p3.ok()) {
            e.view.zoom = p3d::zoomStep(e.view.zoom, w, 0.9f, hse::kZoomMin, hse::kZoomMax);
            KP_LOG(chView, "WHEEL delta=%ld zoom=%.2f", w, e.view.zoom);
        }
        // MMB click: reset view
        const bool mmbClick = mmb && !e.pressedMmb;
        e.pressedMmb = mmb;
        e.pressedRmb = rmb;
        if (mmbClick && insideView) {
            e.view = e.cfgView;
            KP_LOG(chView, "MMB reset view=(%.2f,%.2f,%.2f)", e.view.yaw, e.view.zoom, e.view.panZ);
        }

        // LMB: equip-screen style drag with axis lock
        if (lmb) {
            if (!e.pressed) {
                e.pressed = true;
                if (!e.drag.active && insideView) {
                    p3d::dragBegin(&e.drag, static_cast<float>(curX), static_cast<float>(curY));
                    KP_LOG(chView, "DRAG begin pos=(%d,%d)", curX, curY);
                }
                calibClick(e, curX, curY, insideView);   // U0b: records the press edge; the orbit drag above is unaffected
            } else if (e.drag.active) {
                p3d::ViewState vs = {e.view.yaw, e.view.zoom, e.view.panZ};
                if (p3d::dragMove(&e.drag, static_cast<float>(curX), static_cast<float>(curY), e.dragCfg, &vs)) {
                    e.view.yaw = hse::wrapYaw(vs.yaw);
                    e.view.panZ = hse::panStep(vs.panZ, 0); // clamp to [-1, 1]
                }
            }
        } else {
            e.pressed = false;
            if (e.drag.active) {
                p3d::dragEnd(&e.drag);
                KP_LOG(chView, "DRAG end yaw=%.2f panZ=%.2f", e.view.yaw, e.view.panZ);
            }
        }
    }

    // 4. State machine
    if (e.state == NEEDBUILD) {
        if (n <= 0) {
            if (e.statusItem >= 0) ui->SetText(page, e.statusItem, "no party");
            return;
        }
        e.member = hse::wrapIndex(e.member, 0, n);
        uintptr_t C = hse::memberAt(e.member);
        resetStanceUi(e);
        g_p3.p->Bind(e.token, reinterpret_cast<void*>(C));
        int built = g_p3.p->Rebuild(e.token);
        if (!built) {
            if (e.statusItem >= 0) ui->SetText(page, e.statusItem, "preview rebuild failed");
            KP_LOG(chView, "REBUILD fail member=%d", e.member + 1);
            e.state = FAILED;
            return;
        }
        if (e.token && g_p3.ok()) g_p3.p->PlayAnim(e.token, kStanceList[e.stanceIdx].anim);
        for (int hand = 0; hand < 2; ++hand) {
            void* g = g_p3.p->WeaponGob(e.token, hand);
            int rc = 0;
            if (g && g_hol.ok()) {
                rc = g_hol.h->RegisterPreview(g, nullptr, hand == 1);
            }
            e.regGob[hand] = (rc != 0) ? g : nullptr;
            KP_LOG(chView, "REG hand=%d gob=%p rc=%d", hand, g, rc);
        }
        KP_LOG(chView, "REBUILD ok member=%d/%d creature=%p", e.member + 1, n, reinterpret_cast<void*>(C));

        char rawName[64] = {0};
        bool gotName = hse::memberName(C, rawName, sizeof(rawName));
        char label[64] = {0};
        hse::memberLabel(label, sizeof(label), e.member, rawName);
        if (e.nameItem >= 0) ui->SetText(page, e.nameItem, label);
        KP_LOG(chView, "NAME member=%d text=%s read=%d", e.member + 1, label, gotName ? 1 : 0);

        readWeaponModel(e.regGob[0], e.wpnName[0], sizeof(e.wpnName[0]));
        readWeaponModel(e.regGob[1], e.wpnName[1], sizeof(e.wpnName[1]));
        probeItemNames(e, C);   // fills e.itemName (cached for this build; GetName is never called per frame)

        char rBuf[96], lBuf[96];
        handCaption(e, 0, rBuf, sizeof rBuf);
        handCaption(e, 1, lBuf, sizeof lBuf);
        if (e.wpnRightItem >= 0) ui->SetText(page, e.wpnRightItem, rBuf);
        if (e.wpnLeftItem >= 0) ui->SetText(page, e.wpnLeftItem, lBuf);
        KP_LOG(chView, "HANDS R=\"%s\" (name=\"%s\" resref=%s) L=\"%s\" (name=\"%s\" resref=%s)", rBuf, e.itemName[0], e.wpnName[0][0] ? e.wpnName[0] : "-", lBuf,
               e.itemName[1], e.wpnName[1][0] ? e.wpnName[1] : "-");

        e.mode = MODE_NAV;
        e.editHand = -1;
        applyLayout(e);
        updateStatusText(e);

        g_p3.p->SetView(e.token, e.view.yaw, e.view.zoom, e.view.panZ);
        e.camHookPending = true;   // CAMHOOK is logged on the first LIVE tick, not while the rig is still building
        e.state = LIVE;
        return;
    }

    if (e.state == LIVE) {
        g_p3.p->SetView(e.token, e.view.yaw, e.view.zoom, e.view.panZ);
        if (e.camHookPending) {
            e.camHookPending = false;
            logCamHook(e, "live");
            ++e.projLogs;
            logCamEng(e, "live");
            logCam2(e, "live");
            calibStart(e);
        }
        if (e.projEditPending) {
            if (e.mode != MODE_WEAPON) {
                e.projEditPending = false;
            } else if (e.projLogs < kMaxProjLogs && logEditProj(e)) {
                e.projEditPending = false;
                ++e.projLogs;
                logCamEng(e, "edit");   // the mount point is only available now (a view change is not needed for this capture)
            } else if (++e.projEditTries >= kProjRetryTicks) {
                e.projEditPending = false;
                KP_LOG(chView, "PROJ hand=%s mount=none (GetMountWorld gave nothing in %d ticks)", e.editHand == 1 ? "L" : "R", e.projEditTries);
            }
        }
        // CAM: log only a view that has been steady for >= 500 ms and differs from the last logged one (per Editor, capped).
        const DWORD nowMs = GetTickCount();
        if (e.view.yaw != e.camSeenYaw || e.view.zoom != e.camSeenZoom || e.view.panZ != e.camSeenPanZ) {
            e.camSeenYaw = e.view.yaw;
            e.camSeenZoom = e.view.zoom;
            e.camSeenPanZ = e.view.panZ;
            e.camSeenMs = nowMs;
        } else if (e.camLogs < 200 && nowMs - e.camSeenMs >= 500 &&
                   (e.view.yaw != e.camLogYaw || e.view.zoom != e.camLogZoom || e.view.panZ != e.camLogPanZ)) {
            KP_LOG(chView, "CAM yaw=%.2f zoom=%.2f panZ=%.2f", e.view.yaw, e.view.zoom, e.view.panZ);
            e.camLogYaw = e.view.yaw;
            e.camLogZoom = e.view.zoom;
            e.camLogPanZ = e.view.panZ;
            ++e.camLogs;
            if (e.projLogs < kMaxProjLogs) {   // the creature turned or the fov changed: re-project (the camera itself is not yawed)
                logCamHook(e, "settled");
                if (e.mode == MODE_WEAPON) logEditProj(e);
                ++e.projLogs;
                logCamEng(e, "settled");
                logCam2(e, "settled");
            }
        }
    }
}

// Options dtor entry, before mod-config frees its controls.
void KPATCH_CALL cbDestroy(void* panel, void*) {
    if (!panel || panel != g_ed.panel) return;
    saferead::beginScope();
    Editor& e = g_ed;
    KP_LOG(chMain, "DTOR panel=%p page=%u token=%u free_view=%d", panel, e.page, e.token, g_freeView ? 1 : 0);
    if (e.mode == MODE_WEAPON) {
        if (editDirty()) KP_LOG(chView, "DISCARD why=destroy");
        exitEdit(e);
    }
    const uintptr_t view = e.viewCtl;
    if (e.token && g_p3.ok()) {
        g_p3.p->Destroy(e.token);
    }
    if (g_p3.ok()) {
        g_p3.p->SetBeforeFree(nullptr, &g_ed);
    }
    if (g_freeView && view) {
        const bool unlinked = g_menu.ok() && e.page && e.viewItem >= 0 && g_menu.p->Control(e.page, e.viewItem) == nullptr;
        if (unlinked) {
            uint32_t vt = 0, scene = 0, svt = 0, resman = 0, r34 = 0, fn = 0;
            (void)rd<uint32_t>(view, 0, &vt);
            (void)rd<uint32_t>(view, 0x74, &scene);
            if (scene) (void)rd<uint32_t>(scene, 0, &svt);
            if (rd<uint32_t>(gameaddr::ResManGlobal, 0, &resman) && resman) (void)rd<uint32_t>(resman, 0x34, &r34);
            KP_LOG(chMain, "FREEVIEW view=%x vt=%x (expect %x) scene=%x scene_vt=%x resman34=%x", static_cast<unsigned>(view), static_cast<unsigned>(vt),
                   static_cast<unsigned>(kSceneViewVtable), static_cast<unsigned>(scene), static_cast<unsigned>(svt), static_cast<unsigned>(r34));
            if (vt == kSceneViewVtable && rd<uint32_t>(vt, 0, &fn) && fn && guikit::g_gkCallLabelDtor) {
                guikit::g_gkCallLabelDtor(fn, view, 1u);
                KP_LOG(chMain, "FREEVIEW freed");
            } else {
                KP_LOG(chMain, "FREEVIEW skipped (vtable mismatch)");
            }
        } else {
            KP_LOG(chMain, "FREEVIEW skip linked");
        }
    }
    g_ed = Editor();
}

K2ModConfigEntry g_entry;   // Register copies it; the strings are static

}  // namespace

// Written out literally (create-patch's export scan reads the source text) and listed in exports.def. No file I/O here.
extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_p3.init(api);
    g_hol.init(api);
    g_cfgClient.init(api);
    g_menu.init(api);
    embedgui::g_log = embedLog;
    KP_LOG(chMain, "holster-editor 0.3.2-dev loaded (hookless mod-config consumer): table=%s layout=%u bytes", k2addr::get() ? "ok" : "none",
           static_cast<unsigned>(sizeof kHseViewGui));
    const bool cfgOk = g_cfgClient.connect(), menuOk = g_menu.connect();
    if (!cfgOk || !menuOk) {
        KP_LOG(chMain, "REGISTER fail why=noapi modconfig=%d menuui=%d", cfgOk ? 1 : 0, menuOk ? 1 : 0);
        return;
    }
    g_entry = K2ModConfigEntry();
    g_entry.struct_size = sizeof(K2ModConfigEntry);
    g_entry.id = "holster-editor";
    g_entry.name = "Holstered Weapons";
    g_entry.desc = "Adjust where holstered weapons sit on each party member. Changes preview here and save to holster.ini.";
    g_entry.user = nullptr;
    g_entry.build = cbBuild;
    g_entry.enter = cbEnter;
    g_entry.leave = cbLeave;
    g_entry.tick = cbTick;
    g_entry.destroy = cbDestroy;
    const int32_t token = g_cfgClient.p->Register(&g_entry);
    if (token > 0) KP_LOG(chMain, "REGISTER ok token=%d", static_cast<int>(token));
    else KP_LOG(chMain, "REGISTER fail why=refused");
}

// No I/O here (loader lock). The config is read in build().
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
