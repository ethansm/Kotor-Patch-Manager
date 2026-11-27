// HolsterEditor.cpp -- the Holstered Weapons page of the holster patch (holster 0.22.0; was the separate holster-editor 0.7.3-dev DLL): a hookless consumer of the
// mod-config provider (k2.modconfig@1, k2.menuui@1), linked into Holster.dll. Holster.cpp KPatchInit calls holsterEditorInit() last, after it provided k2.holster@1.
// (doc 42 HW-EDIT, plan .claude/plans/holster_editor_options.md; page rebuild: .claude/plans/holster_editor_layout_v3.md, holster_hw_ux3_build.md Wave B.)
//
// mod-config owns everything about the Options screen: the hooks, the root "Mod Configuration" button, the list page, control cloning,
// hover text, page navigation and freeing its controls. This patch registers ONE entry ("Holstered Weapons") and keeps only what is
// holster-specific:
//   build    (Options ctor window, vanilla layout open) opens the embedded HSE_VIEW layout, lets preview-3d bind the 3D view to it, restores
//            the panel's layout slot FIRST, then makes the page: the 73 widgets of hse::pageLayoutV3 (HseLayoutV3.h, created in its pageBoxesV3 order
//            into Editor::item[]) plus one wedge label per slider row, and asks for the description split (SetDescSplit 850). A provider without
//            SetDescSplit (mod-config older than 0.1.3, which also caps a page at 40 items) or a refused AddX gives a page that only says
//            "needs a newer mod-config": a log line and a message, never a crash.
//   enter    reads the column / view / preview panes, places every item through hse::pageLayoutV3 (the view grows left over the old column band and up
//            to the inspector), installs the mouse wheel on the enlarged view and resets the view to the configured fit zoom.
//   tick     polls the preview-3d and holster clients, handles the buttons (unsaved edits need a second click to discard), the mouse, drives
//            preview-3d and holster placement, and re-asserts every item rect each tick (mod-config drops a re-layout fight after 8 re-applies; a
//            SetRect resets that count, and the thumbs move with the values anyway).
// Editing is always on: the page opens editing the right weapon (else the left); a hand button switches the edited weapon. There is no nav mode and
// no Done button; Back leaves. Selectors are direct buttons (no cycling): hand, five bones (a click loops CycleBone, at most 5 steps, until
// GetEdit reports the bone), three scopes, five stances; the active one is lit (text colour, or >marks< when the colour call is refused).
// Mouse: hse::Chord (HseView.h): LMB alone = orbit the character, RMB held + LMB drag = act on the weapon per the mode bar (Move / Depth / Tilt /
//   Roll+Turn, latched at the press), RMB + wheel = depth, MMB drag = spin, Shift = fine (x0.2), Ctrl = snap (1 cm / 5 deg, carried). RMB alone does
//   nothing. A press that starts on any of the page's buttons is a dead press (never a view drag).
// Inspector (weapon mode, right pane): six slider rows Slide/Depth/Height (offset x/y/z, cm) and Tilt/Roll/Turn (rotation x/y/z, deg): name, a track
//   (hse_sld_frame over a hse_sld_wedge cut to the value), a thumb (hse_sld_mark) and the value text OUTSIDE the track (amber when it differs from
//   the value when the edit opened), a reset icon button per row and per group (ResetIcon.h). The track spans +-30 cm / +-180 deg around zero. A
//   press on the track outside the thumb jumps the value there; a press on the thumb or the track then scrubs: 1 px = 1 mm or 0.5 deg, Shift x0.2,
//   Ctrl snaps 1 cm / 5 deg (carried), window +-0.30 m / +-180 deg around the value at the press, hard limits 3 m / 720 deg. The press is a DEAD
//   press for hse::Chord. A scrub is cancelled by foreground loss, Back/Esc/leave, a scope change or the edit closing.
//   destroy  destroys the preview viewer (and, behind free_view, the view block once mod-config has unlinked it).
// A failed connect to mod-config means no menu entry, never a crash. KpatchInit does no file I/O: holster_editor.ini is read in build().
// Logging: per-edit lines (EDIT, BTN, SCRUB, INSP open/RESET, GESTURE, ...), WARN/ERR, REG, ENTER, LEAVE, BUILD, DISCARD and the CAM2 probe lines are
// always on. CHORD state transitions, the INSP refresh counts and repeated PROJ/CAMHOOK re-projections are behind ini log_verbose (default 0).
// Probe lines (log only): REGISTER, CONFIG, BUILD, EMBED, ENTER, DTOR, FREEVIEW, LAYOUT, ART, NAME, REG, CAM, CAMHOOK, PROJ, CAMPTR, CAMENG, CAMATT, CAMPROJ,
// CAMCHK, CAM2, ITEMNAME, PICK, BTN, EDIT, STANCE, MMB, DRAG, WHEEL, UNREG, LEAVE, DISCARD, CHORD (dropped presses always; transitions and resets
// with log_verbose), MODE, GESTURE, SNAP, NUDGE (WARN), SCOPE, HANDS, INSP, SCRUB.
// CAMENG (U4, ini probe_cam): reads the engine camera object of the preview view (view+0x78: pose, fovy, viewport, attach behaviour) and calls its
// MapToScreen (camera vtable slot 0x28, pure math, only when the viewport is non-empty) for both hand nodes and the weapon mount, then compares the
// engine answer with the maths we assume (hse::eyeOf / rawPredict) and with hse::worldToScreen. Lines: CAMPTR (once), CAMENG, CAMATT, CAMPROJ (R, L, M),
// CAMCHK per capture (at most kMaxCamEngCaptures, only when the view / edit state changed); CAMENG WARN for every failure path.
// CAM2 (ux_v3 A1, ini probe_cam): the draw viewport / scene rect / clip stack probe behind the "1.0 m instead of 1.7 m" camera question (logCam2).
// Every probe failure path logs a "<PROBE> WARN <reason>" line (WARN level, at most kMaxWarnLogs per kind per Editor).
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "../Common/SafeRead.h"
#include "../Common/KpLog.h"
#include "../Common/K2Addr.h"
#include "../Common/SceneView.h"   // sceneview::camera(view): the preview view's engine camera object (CAMENG probe)
#include "../Common/GameAddr.h"
#include "../Common/GuiKit.h"
#include "../Common/GuiButton.h"   // guibutton::ButtonVtable / Button_Text: the button caption fix gates on them
#include "../Common/EmbedGui.h"
#include "../Common/HolsterApi.h"
#include "../Common/Preview3dApi.h"
#define INIW_FILE_API 1   // the Speed slider writes holster.ini through iniw::updateFile (IniWrite.h); must come before the first include of it (HseSpeed.h, below)
#include "../Common/ModConfigApi.h"
#include "HseSpeed.h"         // the Speed slider's pure helpers: press -> 1..8, thumb place, the label, the holster.ini read (host-tested by hse_speed_test.cpp)
#include "../Common/ProviderClient.h"   // k2pc::ClientSet / RegisterOnce: the lazy-connect plumbing for the four clients below
#include "../Common/ViewDrag.h"
#include "HseView.h"
#include "../Common/TextFit.h"
#include "../Common/HseParty.h"
#include "HseLayoutV3.h"   // hse::pageLayoutV3: every box of the page
#include "../Common/ResetIcon.h"       // reset arrow icon (Steam only; the "R" text stays when it cannot be applied)
#include "../Common/SliderArt.h"       // slider frame / wedge / marker art
#include "HseViewGui.inc"   // generated by scripts/gen_hse_layout.py: kHseViewGui[] (GFF "GUI " V3.2, HSE_VIEW with ID -1)

// KP_LOG_PATCH is declared once, in Holster.cpp: the page logs to holster_log.txt under the holster patch (channels main, embed, view).

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chEmbed, "embed");
KP_CHANNEL(chView, "view");


namespace {

constexpr uint32_t kOptionsVtable = static_cast<uint32_t>(gameaddr::OptionsInGameVtable);
constexpr uint32_t kSceneViewVtable = static_cast<uint32_t>(gameaddr::SceneViewVtable);

struct StanceDef {
    const char* label;
    const char* anim;
};

// The five stance buttons of the page (hse::kV3Stances); the rifle walk / run and the emote cycler are gone from the editor.
const StanceDef kStanceList[] = {
    {"Idle", "pause1"},
    {"Walk", "walk"},
    {"Run", "run"},
    {"Alert", "pause2"},
    {"Combat", "pause3"},
};
constexpr int kStanceCount = sizeof(kStanceList) / sizeof(kStanceList[0]);
static_assert(kStanceCount == hse::kV3Stances, "one stance button per stance");
// Wave B item budget: the 70 boxes of hse::pageLayoutV3 plus one wedge label per slider row must fit mod-config's per-page cap.
static_assert(hse::kPageItemsV3Total <= hse::kPageItemCap, "the v3 page must fit the consumer item cap");

bool g_enabled = true;
bool g_freeView = false;   // also free the 3D view block after preview-3d destroys the creature (probe)
int g_logVerbose = 0;      // ini log_verbose: 1 = also log CHORD transitions, INSP refresh counts and repeated PROJ/CAMHOOK re-projections (default 0 = quiet)
int g_probeCam = 1;        // ini probe_cam: 1 = the CAMENG engine camera probe (U4, log only), 0 = off
int g_probeCaption = 1;    // ini probe_caption: 1 = log the CAPTION lines (button caption rect before / after the fix), 0 = quiet
int g_captionFix = 1;      // ini caption_fix: 1 = set each button's caption text rect to {engine x/w, button y/h} (hse::captionRect), 0 = leave the engine's inset rect
int g_probeNames = 1;      // ini probe_names: 0 = off (weapon buttons show the model resref), 1 = item GetName with identified=0 (the button name; ITEMNAME log), 2 = also identified=1 (log only)
float g_textK = 2.65f;      // ini text_k: vertical scale (sy) used when measuring label text; the engine draws the page labels about 2.64x wider than sy 1 predicts (measured at 3840x1600)
float g_statusFit = 1.0f;  // ini status_fit: multiplier on the status pane width when fitting the status text (the label font is not measured)
float g_cfgYaw = 180.0f;
float g_cfgZoom = hse::kZoomFit;
int g_descCoverPx = hse::kV3CoverDefault;   // ini desc_cover_px: the right description box is inflated this far on all four sides to cover LB_DESC's own frame (0 = off)
int g_marginPx = 120;   // ini margin_px: the left column pane grows this far left and the preview (inspector) pane this far right
float g_cfgPanZ = 0.0f;
hse::View g_cfgView = {180.0f, hse::kZoomFit, 0.0f};

k2p3::Client g_p3;
k2holster::Client g_hol;
k2mc::ConfigClient g_cfgClient;
k2mc::MenuClient g_menu;
k2pc::ClientSet<k2p3::Client, k2holster::Client, k2mc::ConfigClient, k2mc::MenuClient> g_clients(g_p3, g_hol, g_cfgClient, g_menu);   // pointers: the clients above outlive it
k2pc::RegisterOnce g_reg;

enum State { IDLE, NEEDBUILD, LIVE, FAILED };
// MODE_WEAPON = a weapon edit is open (the normal state of the page); MODE_NAV = none is (the member has no weapon, or the edit was refused or lost).
enum EditMode { MODE_NAV = 0, MODE_WEAPON = 1 };
// Actions guarded by the unsaved-edits two-click confirm (each direct scope button is its own action, so arming one never confirms another). The bone
// buttons are not guarded: a bone change keeps the unsaved values.
enum GuardAction { ACT_NONE = 0, ACT_HAND_R, ACT_HAND_L, ACT_PREV, ACT_NEXT, ACT_SCOPE0 = 10, ACT_RESET_SCOPE = 20 };
constexpr uint32_t kDiscardWindowMs = 2000;
constexpr float kPreviewFov = 22.726f;       // preview-3d kDefaultFov: the scene fov is p3d::fovForZoom(kPreviewFov, zoom) (applyView)
constexpr float kProjUpMetres = 0.3f;        // PROJ probe: second point is the mount + this far along world +Z
constexpr int kMaxWarnLogs = 20;             // cap on WARN lines per probe kind per Editor
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
constexpr int kModeCount = hse::WPM_COUNT;
static_assert(kModeCount == hse::kV3Modes, "one mode button per weapon mode");
constexpr const char* kNeedsModConfig = "Needs a newer mod-config (0.1.3 or later)";   // the refusal message (old provider: no SetDescSplit, 40-item cap)
constexpr float kColLit[3] = {1.0f, 0.86f, 0.35f};     // lit selector button text
constexpr float kColAmber[3] = {1.0f, 0.70f, 0.15f};   // value text that differs from the value when the edit opened
constexpr int kCrRightItemOff = 0x238, kCrLeftItemOff = 0x23C;   // client creature: equipped item ids (preview-3d CrRightItemOff/CrLeftItemOff)
constexpr uint32_t kNoItemId = 0x7F000000u;
constexpr int kItemModelResOff = 0x128;      // client item: model ResRef char[16]
constexpr int kItemBaseIdxOff = 0xC;         // client item: base item index (GetName's lookup branch dereferences its row)

enum WarnKind { W_ITEMNAME = 0, W_PROJ, W_CAMHOOK, W_CALIB, W_NUDGE, W_SCRUB, W_INSP, W_CAMENG, W_PAUSE, W_KINDS };
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
    bool refused = false;      // the page only shows kNeedsModConfig (old provider, or an AddX failed): nothing else runs
    // The page items, indexed like hse::pageBoxesV3 (hse::PI_*): item[i] is the mod-config item id (-1 = missing). The wedge labels are the sixth item of each slider row.
    int32_t item[hse::kPageItemsV3];
    int32_t wedge[hse::kV3Rows];
    int32_t viewItem = -1, statusItem = -1;   // item[PI_VIEW], item[PI_STATUS]
    int itemsBuilt = 0;                // items the page holds after build
    uintptr_t viewCtl = 0;     // the CSWGui3DSceneView control preview-3d bound to HSE_VIEW
    uint32_t token = 0;        // preview-3d viewer token
    int member = 0;
    hse::View view = {180.0f, hse::kZoomFit, 0.0f};
    hse::View cfgView = {180.0f, hse::kZoomFit, 0.0f};
    State state = IDLE;

    EditMode mode = MODE_NAV;
    int editHand = -1;  // 0=right, 1=left
    int editScope = K2HOLSTER_SCOPE_WEAPON;
    K2HolsterEditInfo editInfo = {};
    char wpnName[2][32] = {{0}, {0}};

    int weaponMode = hse::WPM_MOVE;                     // selected mode; a running gesture keeps the mode it began with

    // Panes (UI units, re-read every tick) and the layout built from them (hse::pageLayoutV3; the enlarged view is lay.view = viewRect)
    hse::HseRect paneCol = {0, 0, 0, 0}, paneView = {0, 0, 0, 0}, panePreview = {0, 0, 0, 0};
    editkit::Box coverShown = {0, 0, 0, 0}; // the inflated description pane the last HINTR cover line was logged for
    hse::HseRect paneDesc = {0, 0, 0, 0};   // MUI_PANE_DESC: mod-config's shrunk LB_DESC (clipped under the preview pane); all zero = unavailable
    int hintRState = -1;                    // the right description box: -1 not decided, 0 absent (no pane / too small), 1 placed (logged on change)
    bool hintLHave = false, hintRHave = false;   // the left / right description box had room at the last hint refresh (a change rewrites the texts)
    char hintRBuf[256] = {0};               // the text the right pane (SetDesc) last got
    char longTxt[2][256] = {{0}, {0}};      // the full text of the left (0) / right (1) description label (SetText is cut to 95 characters by mod-config)
    bool panesOk = false;
    hse::PageLayoutV3 lay = {};
    bool layOk = false;
    hse::HseRect wheelRect = {0, 0, 0, 0};   // the rect the wheel hook counts in (the enlarged view)
    hse::HseRect viewRect = {0, 0, 0, 0};    // the enlarged view rect in UI units (the camera extent for the PROJ probe, the CAM2 pane, the scrub hit area)

    // Inspector (weapon mode): thumb fraction per row, value texts, the value the edit opened with (the reset target)
    float rowFrac[hse::kV3Rows] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    // Speed slider (global, holster.ini slow_holster_x; HW44): the value (read at page open, written at each change), the drag state, the track's wedge item
    int speed = hse::kSpeedDefault;
    bool speedDrag = false;           // LMB is held from a press on the Speed track (the value follows the cursor)
    int32_t speedWedge = -1;
    int speedValW = -1;                // the value label's box width the text was fitted for (the label is refitted when the box changes)
    float openRot[3] = {0, 0, 0}, openOff[3] = {0, 0, 0};   // the values when the edit opened (or after the last Save / scope change): the reset target
    bool openValid = false;
    hse::Scrub scrub;                  // the running row scrub (pure state)
    hse::TextCache inspText;           // what each value label was last given: SetText only on a change
    bool inspForce = true;             // the next refresh rewrites every value text
    int inspValW = -1;                 // width of the value boxes the cached texts were fitted to
    float inspSeenRot[3] = {0, 0, 0}, inspSeenOff[3] = {0, 0, 0};
    int inspSeenScope = -1;
    DWORD inspWinStart = 0;            // INSP refresh log window (log_verbose only)
    int inspWinSets = 0, inspWinTicks = 0;
    int scrubLogs = 0, inspLogs = 0;
    int hintRow = -2;                  // the slider row whose help is in the right description pane (-1 none, -2 not written yet)
    // Remove menu (the "..." toggle opens it; PI_REMOVE is shown only while it is open): closed again when the scope, hand or member it was opened for changes
    hse::RemoveMenu removeMenu;
    int removeScope = -1, removeHand = -1, removeMember = -1;   // the edit the menu was opened for
    // SCOPE log state: the last (scope, body, type) logged for the open edit (scope == -1: nothing logged for this edit yet)
    int scopeLogScope = -1;
    char scopeLogBody[20] = {0}, scopeLogType[16] = {0};

    // Style caches (item index -> what was last applied): text colour state (-1 unknown, 0 default, 1 lit/amber), the control's own default colour, selector caption
    int8_t colState[hse::kPageItemsV3];
    bool defColOk[hse::kPageItemsV3];
    float defCol[hse::kPageItemsV3][3];
    float brdNormal[hse::kPageItemsV3][3], brdHover[hse::kPageItemsV3][3];   // a selector button's own normal / hover frame tint (read once)
    bool brdOk[hse::kPageItemsV3];
    char selText[hse::kPageItemsV3][64];   // the text each item last got (setItemText compares against it)
    bool colorBad = false;             // the selected-frame tint could not be applied on a button: lit selectors show >marks< instead
    bool valueColorBad = false;        // ... on a value label: changed values are not amber
    bool selKeyValid = false;
    int selKey[18] = {};
    char noteShown[48] = {0};
    char statusShown[100] = {0};
    bool artDone = false;              // slider / reset art applied (or given up)
    int artTicks = 0;
    // Every page control at art time (items + wedges + speed wedge): cbDestroy detaches this DLL's pooled images from them before mod-config frees
    // them, because mod-config's detachPooledFills only knows mod-config's own pool (crash 10-26 00:51: label border dtor called a freed slider image).
    uintptr_t artCtl[hse::kPageItemsV3 + hse::kV3Rows + 1] = {};
    int artCtlCount = 0;
    bool paneWarned = false;

    int stanceIdx = 0;
    bool drawn = false;                // the Draw / Holster toggle (holster 0.8.0-dev SetPreviewDraw): the preview weapons are heading to the hand; reset with the page
    bool paused = false;               // the preview animation is frozen (SetAnimPaused); every PlayAnim / rebuild resumes it in the provider, so those clear this too

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
    int camEngKeyEdit = 0;            // 0 = no edit, 1 = right-hand edit, 2 = left-hand edit
    bool camEngCfgLogged = false;     // CAMPTR was logged once
    bool capBoxLogged = false;        // CAPBOX (the three section caption heights) was logged once
    bool camEngCapNoted = false;
    int cam2Logs = 0;                 // CAM2 line sets logged (capped at kMaxCam2Logs)

    // Button caption fix state (reset with the Editor, i.e. per Options open): per item the button rect the caption was last set for
    hse::HseRect capBtn[hse::kPageItemsV3] = {};
    bool capSeen[hse::kPageItemsV3] = {};      // capBtn[i] is valid
    unsigned char capReapply[hse::kPageItemsV3] = {};   // re-applies since the button rect last changed (capped at kCaptionReapplyMax)
    bool capCapped[hse::kPageItemsV3] = {};    // the "reapply capped" line was logged for the item
    int capLogs = 0;                  // CAPTION probe lines logged (capped at kMaxCaptionLogs)
    int capWarns = 0;                 // CAPTION WARN vtable lines logged (one per Options open)

    // Per-kind WARN counters (reset with the Editor) and the CAM2 node name cache
    int warnCount[W_KINDS] = {};
    int calibNodeIdx[kCalibNodes] = {-1, -1, -1, -1, -1, -1};   // index into kCalibCand[i] that resolved, -1 = none yet

    // Weapon gesture summary (logged at the end) and chord logging caps
    bool gActive = false;
    int gMode = 0, gHand = 0, gFrames = 0;
    long gDx = 0, gDy = 0;
    float gA = 0.0f, gB = 0.0f;
    int chordLogs = 0, dropLogs = 0, snapLogs = 0, gestureLogs = 0;

    // Item display names (cache: CSWCItem::GetName is called once per member build, never per frame). Empty = fall back to the model resref.
    char itemName[2][64] = {{0}, {0}};

    int shownDirty = -1;       // the dirty flag the status label was last written for

    Editor() {
        for (int i = 0; i < hse::kPageItemsV3; ++i) { item[i] = -1; colState[i] = -1; defColOk[i] = false; selText[i][0] = '\0'; defCol[i][0] = defCol[i][1] = defCol[i][2] = 0.0f;
            brdOk[i] = false; for (int k = 0; k < 3; ++k) brdNormal[i][k] = brdHover[i][k] = 0.0f; }
        for (int r = 0; r < hse::kV3Rows; ++r) wedge[r] = -1;
    }
};
Editor g_ed;

void scrubCancel(Editor& e, const char* why);   // defined with the inspector helpers below
void exitEdit(Editor& e);                       // defined after enterEdit
void speedReadNow(Editor& e);                   // defined with the Speed slider helpers (before the slider row helpers)
void speedShowText(Editor& e);                  // ditto
bool speedHover(const Editor& e, int x, int y); // ditto
bool speedBuilt(const Editor& e);               // ditto

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
    g_logVerbose = 0;
    g_probeCam = 1;
    g_probeCaption = 1;
    g_captionFix = 1;
    g_statusFit = 1.0f;
    g_cfgYaw = 180.0f;
    g_cfgZoom = hse::kZoomFit;
    g_cfgPanZ = 0.0f;
    g_marginPx = 120;
    g_descCoverPx = hse::kV3CoverDefault;
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
        else if (!_stricmp(k, "log_verbose")) g_logVerbose = (v[0] >= '0' && v[0] <= '9') ? (atoi(v) != 0 ? 1 : 0) : (on ? 1 : 0);
        else if (!_stricmp(k, "probe_cam")) g_probeCam = (v[0] >= '0' && v[0] <= '9') ? (atoi(v) != 0 ? 1 : 0) : (on ? 1 : 0);
        else if (!_stricmp(k, "probe_caption")) g_probeCaption = (v[0] >= '0' && v[0] <= '9') ? (atoi(v) != 0 ? 1 : 0) : (on ? 1 : 0);
        else if (!_stricmp(k, "caption_fix")) g_captionFix = (v[0] >= '0' && v[0] <= '9') ? (atoi(v) != 0 ? 1 : 0) : (on ? 1 : 0);
        else if (!_stricmp(k, "text_k")) {
            const float f = static_cast<float>(atof(v));
            g_textK = (f >= 0.5f && f <= 6.0f) ? f : 2.65f;
        }
        else if (!_stricmp(k, "status_fit")) {
            const float f = static_cast<float>(atof(v));
            g_statusFit = (f >= 0.5f && f <= 8.0f) ? f : 1.0f;
        }
        else if (!_stricmp(k, "view_yaw")) g_cfgYaw = static_cast<float>(atof(v));
        else if (!_stricmp(k, "view_zoom")) g_cfgZoom = static_cast<float>(atof(v));
        else if (!_stricmp(k, "view_panz")) g_cfgPanZ = static_cast<float>(atof(v));
        else if (!_stricmp(k, "margin_px")) { const int m = atoi(v); g_marginPx = m < 0 ? 0 : (m > 400 ? 400 : m); }
        else if (!_stricmp(k, "desc_cover_px")) { const int m = atoi(v); g_descCoverPx = m < 0 ? 0 : (m > hse::kV3CoverMax ? hse::kV3CoverMax : m); }
    }
    fclose(f);
    g_cfgView = hse::makeView(g_cfgYaw, g_cfgZoom, g_cfgPanZ);
}

const char* statusText() {
    return g_p3.ok() && g_hol.ok() ? "0.4.5 dev: preview + holster connected" : "0.4.5 dev: a provider is missing";
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

// ---- page items (indexed like hse::pageBoxesV3, hse::PI_*) ----

// Buttons of the page; every other item is a label (PI_VIEW is the 3D view control).
bool itemIsButton(int i) {
    using namespace hse;
    if (i == PI_MEMBER_PREV || i == PI_MEMBER_NEXT || i == PI_SAVE || i == PI_REVERT || i == PI_RESET_SCOPE || i == PI_REMOVE || i == PI_VIEW_RESET || i == PI_VIEW_FLIP || i == PI_PAUSE || i == PI_DRAW) return true;
    if (i >= PI_WEAPON && i < PI_WEAPON + kV3Weapons) return true;
    if (i >= PI_BONE && i < PI_BONE + kV3Bones) return true;
    if (i >= PI_SCOPE && i < PI_SCOPE + kV3Scopes) return true;
    if (i >= PI_MODE && i < PI_MODE + kV3Modes) return true;
    if (i >= PI_STANCE && i < PI_STANCE + kV3Stances) return true;
    for (int g = 0; g < kV3Groups; ++g)
        if (i == PI_GROUP + g * 2 + 1) return true;
    for (int r = 0; r < kV3Rows; ++r)
        if (i == PI_ROW + r * kRowStride + 4) return true;
    return false;
}

// Items a mod-config with a smaller page cap may leave out (the page works without them): the Draw / Holster toggle and the Speed slider (the last items).
bool itemOptional(int i) { return i == hse::PI_DRAW || (i >= hse::PI_SPEED_TRACK && i < hse::PI_SPEED_TRACK + hse::kSpeedItems); }

// The reset icon buttons: the view reset, the two group resets and the six row resets.
bool itemIsResetIcon(int i) {
    using namespace hse;
    if (i == PI_VIEW_RESET) return true;
    for (int g = 0; g < kV3Groups; ++g)
        if (i == PI_GROUP + g * 2 + 1) return true;
    for (int r = 0; r < kV3Rows; ++r)
        if (i == PI_ROW + r * kRowStride + 4) return true;
    return false;
}

// (The party member prev / next buttons carry the words "Prev" / "Next": the arrow art was invisible, the button border insets any fill by 16 px a side.)
bool itemIsHintBox(int i) { return i == hse::PI_HINT || i == hse::PI_HINTR; }             // the left / right description box (fill art)
bool itemIsHintText(int i) { return i == hse::PI_HINT + 1 || i == hse::PI_HINTR + 1; }   // its text label
// Buttons whose hover help goes to the LEFT description box (the editor writes it) instead of mod-config's right pane (which then keeps the slider help).
bool itemIsLeftTip(int i) {
    using namespace hse;
    if (i == PI_MEMBER_PREV || i == PI_MEMBER_NEXT || i == PI_SAVE || i == PI_REVERT || i == PI_RESET_SCOPE || i == PI_REMOVE) return true;
    return (i >= PI_WEAPON && i < PI_WEAPON + kV3Weapons) || (i >= PI_BONE && i < PI_BONE + kV3Bones) || (i >= PI_SCOPE && i < PI_SCOPE + kV3Scopes);
}

// First text of item i (buf is the scratch for composed ones).
const char* itemInitText(int i, char* buf, int cap) {
    using namespace hse;
    static const char* const kGroup[kV3Groups] = {"POSITION", "ROTATION"};
    if (i == PI_MEMBER_PREV) return "Prev";
    if (i == PI_MEMBER_NEXT) return "Next";
    if (i == PI_MEMBER_NAME) return "";
    if (i == PI_CAP_WEAPON) return "WEAPON";
    if (i == PI_CAP_BONE) return "BONE";
    if (i == PI_CAP_SCOPE) return "APPLIES TO";
    if (i == PI_WEAPON) return "Right: None";
    if (i == PI_WEAPON + 1) return "Left: None";
    if (i >= PI_BONE && i < PI_BONE + kV3Bones) return boneLabel(i - PI_BONE);
    if (i >= PI_SCOPE && i < PI_SCOPE + kV3Scopes) { scopeSegText(buf, cap, i - PI_SCOPE, ""); return buf; }
    if (i == PI_SAVE) return "Save";
    if (i == PI_REVERT) return "Revert";
    if (i == PI_RESET_SCOPE) return removeToggleLabel();
    if (i == PI_REMOVE) return removeLabel();
    if (i >= PI_MODE && i < PI_MODE + kV3Modes) return weaponModeName(i - PI_MODE);
    if (i == PI_VIEW_RESET) return "R";
    if (i == PI_VIEW_FLIP) return "<>";
    if (i >= PI_STANCE && i < PI_STANCE + kV3Stances) return kStanceList[i - PI_STANCE].label;
    if (i == PI_PAUSE) return "Pause";
    if (i == PI_DRAW) return "Draw";
    if (i == PI_SPEED_VALUE) { hse::speedText(buf, static_cast<size_t>(cap), g_ed.speed); return buf; }
    if (i == PI_LEGEND) return helpLine();
    for (int g = 0; g < kV3Groups; ++g) {
        if (i == PI_GROUP + g * 2) return kGroup[g];
        if (i == PI_GROUP + g * 2 + 1) return "R";
    }
    for (int r = 0; r < kV3Rows; ++r) {
        const int b = PI_ROW + r * kRowStride;
        if (i == b) return inspRowName(r);
        if (i == b + 3) return "-";
        if (i == b + 4) return "R";
    }
    return "";   // tracks, thumbs, the note and the hint lines start empty
}

// Hover text of button i (the mod-config description pane; ASCII, under 190 characters).
const char* itemTip(int i) {
    using namespace hse;
    static const char* const kModeDesc[kV3Modes] = {
        "RMB + LMB drag moves the weapon across the screen. Shift is fine, Ctrl snaps to 1 cm.",
        "RMB + LMB drag pushes the weapon along its forward axis (drag up = forward). RMB + wheel does this in any mode.",
        "RMB + LMB drag tilts the weapon about its side axis. Shift is fine, Ctrl snaps to 5 degrees.",
        "RMB + LMB drag: across turns the weapon about its up axis, up or down rolls it about its forward axis.",
    };
    if (i == PI_MEMBER_PREV) return "Show the previous party member.";
    if (i == PI_MEMBER_NEXT) return "Show the next party member.";
    if (i == PI_WEAPON) return "Edit the holster placement of the right-hand weapon.";
    if (i == PI_WEAPON + 1) return "Edit the holster placement of the left-hand weapon.";
    if (i >= PI_BONE && i < PI_BONE + kV3Bones) return "Mount the weapon on this bone while holstered.";
    if (i >= PI_SCOPE && i < PI_SCOPE + kV3Scopes) return scopeTip();   // the editor writes the per-button sentence into the left box (leftTipText)
    if (i == PI_SAVE) return "Save placement to holster.ini under [editor].";
    if (i == PI_REVERT) return revertTip();
    if (i == PI_RESET_SCOPE) return removeToggleTip();
    if (i == PI_REMOVE) return removeLabel();   // leftTipText writes removeSentence
    if (i >= PI_MODE && i < PI_MODE + kV3Modes) return kModeDesc[i - PI_MODE];
    if (i == PI_VIEW_RESET) return "Reset view: back to the fit framing.";
    if (i == PI_VIEW_FLIP) return "Flip view: turn the character around.";
    if (i >= PI_STANCE && i < PI_STANCE + kV3Stances) return "Preview this stance.";
    if (i == PI_PAUSE) return "Freeze the preview animation to check clipping. Click again to play.";
    if (i == PI_DRAW) return "Draw: the preview weapons slide to the hand, with the arm reaching (reach_test) as in the game. Click again to holster them.";
    for (int g = 0; g < kV3Groups; ++g)
        if (i == PI_GROUP + g * 2 + 1) return "Put these three values back to what they were when the edit opened (or after the last Save).";
    for (int r = 0; r < kV3Rows; ++r)
        if (i == PI_ROW + r * kRowStride + 4) return inspResetTip();
    return "";
}

// ---- k2.modconfig callbacks (cdecl, main thread) ----

// Options ctor window: the vanilla layout is open. Opens the embedded layout, binds the 3D view, restores the layout slot, builds the page.
void KPATCH_CALL cbBuild(void* panel, void*) {
    readConfig();
    KP_LOG(chMain, "CONFIG enabled=%d free_view=%d probe_names=%d probe_cam=%d probe_caption=%d caption_fix=%d log_verbose=%d text_k=%.2f status_fit=%.2f margin_px=%d desc_cover_px=%d view=(%.2f,%.2f,%.2f)",
           g_enabled ? 1 : 0, g_freeView ? 1 : 0, g_probeNames, g_probeCam, g_probeCaption, g_captionFix, g_logVerbose, g_textK, g_statusFit, g_marginPx, g_descCoverPx,
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
    if (hol && g_hol.canDraw()) (void)g_hol.h->SetPreviewDraw(0);   // a fresh page starts holstered (the toggle state lives in holster until its previews are gone)

    // An older mod-config has neither SetDescSplit nor the 80-item page cap (no cap query in the ABI; SetDescSplit shipped with the cap, 0.1.3): the page
    // only says so. A refused AddX further down ends the same way.
    if (!g_menu.hasDescSplit()) {
        g_ed.refused = true;
        g_ed.page = g_menu.p->NewPage(panel, "Holstered Weapons");
        if (g_ed.page) g_ed.item[hse::PI_STATUS] = g_ed.statusItem = g_menu.p->AddLabel(g_ed.page, kNeedsModConfig);
        KP_LOGV(chMain, KPLOG_WARN, "BUILD WARN refused: this mod-config has no SetDescSplit (older than 0.1.3, page cap 40); the page needs %d items. Install mod-config 0.1.3 or later",
                hse::kPageItemsV3Total);
        return;
    }

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

    // The page: the status label first (it carries the refusal message), the 3D view (listed, shown and hidden with the page, never freed by mod-config),
    // then every other widget in hse::pageBoxesV3 order, with the six slider wedges just before the inspector (later items draw over earlier ones:
    // wedge, then the frame track, then the thumb).
    const K2MenuUiApi* ui = g_menu.p;
    g_ed.page = ui->NewPage(panel, "Holstered Weapons");
    if (!g_ed.page) {
        KP_LOG(chMain, "BUILD NewPage failed");
        if (g_ed.token && g_p3.ok()) g_p3.p->Destroy(g_ed.token);
        g_ed = Editor();
        return;
    }
    const int split = ui->SetDescSplit(g_ed.page, hse::kDescSplitPermille);
    KP_LOG(chMain, "BUILD desc split %d permille rc=%d", hse::kDescSplitPermille, split);
    embtex::g_log = embedLog;
    const bool sliderOk = hse::sliderArtEnsure(), resetOk = hse::resetIconEnsure();   // both always tried (Steam only); plain controls stay when one cannot be made
    KP_LOG(chMain, "BUILD art textures ensure slider=%d reset=%d", sliderOk ? 1 : 0, resetOk ? 1 : 0);

    g_ed.item[hse::PI_STATUS] = g_ed.statusItem = ui->AddLabel(g_ed.page, statusText());
    if (g_ed.viewCtl) g_ed.item[hse::PI_VIEW] = g_ed.viewItem = ui->AddCustom(g_ed.page, reinterpret_cast<void*>(g_ed.viewCtl), MUI_NOFREE);
    if (g_ed.token && g_p3.ok()) {
        g_p3.p->SetBeforeFree(cbBeforeFree, &g_ed);
        g_p3.p->SetWeapons(g_ed.token, 1);
    }
    speedReadNow(g_ed);   // the Speed label's first text comes from the saved value (the items below read it)
    int failedAt = (g_ed.statusItem < 0) ? hse::PI_STATUS : -1;
    for (int i = 0; i < hse::kPageItemsV3 && failedAt < 0; ++i) {
        if (i == hse::PI_STATUS || i == hse::PI_VIEW) continue;
        if (i == hse::PI_GROUP) {
            for (int r = 0; r < hse::kV3Rows && failedAt < 0; ++r) {
                g_ed.wedge[r] = ui->AddLabel(g_ed.page, "");
                if (g_ed.wedge[r] < 0) failedAt = 1000 + r;
            }
            if (failedAt >= 0) break;
        }
        if (i == hse::PI_SPEED_TRACK) g_ed.speedWedge = ui->AddLabel(g_ed.page, "");   // the Speed track's wedge: just before the track and thumb, so they draw over it
        char buf[64];
        const char* text = itemInitText(i, buf, sizeof buf);
        g_ed.item[i] = itemIsButton(i) ? ui->AddButton(g_ed.page, text, itemIsLeftTip(i) ? "" : itemTip(i)) : ui->AddLabel(g_ed.page, text);
        if (g_ed.item[i] < 0 && !itemOptional(i)) failedAt = i;   // the Draw toggle and the Speed slider come last: a mod-config with a smaller cap just leaves them out
    }
    if (g_ed.item[hse::PI_DRAW] < 0) KP_LOGV(chMain, KPLOG_WARN, "BUILD WARN no Draw / Holster toggle: mod-config page cap below %d (needs the mod-config build after 0.3.7-dev)", hse::kPageItemsV3Total);
    if (!speedBuilt(g_ed)) KP_LOGV(chMain, KPLOG_WARN, "BUILD WARN no Speed slider: mod-config page cap below %d", hse::kPageItemsV3Total);
    int built = 0;
    for (int i = 0; i < hse::kPageItemsV3; ++i) if (g_ed.item[i] >= 0) ++built;
    for (int r = 0; r < hse::kV3Rows; ++r) if (g_ed.wedge[r] >= 0) ++built;
    if (g_ed.speedWedge >= 0) ++built;
    g_ed.itemsBuilt = built;
    KP_LOG(chMain, "BUILD items=%d/%d (page cap %d) page=%u view item=%d status item=%d token=%u", built, hse::kPageItemsV3Total, hse::kPageItemCap, g_ed.page,
           g_ed.viewItem, g_ed.statusItem, g_ed.token);
    if (failedAt >= 0) {
        g_ed.refused = true;
        KP_LOGV(chMain, KPLOG_WARN, "BUILD WARN refused: AddX failed at %s %d after %d items (mod-config page cap below %d?). Install mod-config 0.1.3 or later",
                failedAt >= 1000 ? "wedge" : "item", failedAt >= 1000 ? failedAt - 1000 : failedAt, built, hse::kPageItemsV3Total);
    }
}

// The refusal page: the message label goes on the provider's status pane.
void placeItem(int32_t item, int32_t pane, const char* what) {
    if (item < 0) return;
    K2MuiRect r = {0, 0, 0, 0};
    const bool ok = g_menu.p->PaneRect(g_ed.panel, pane, &r) != 0;
    KP_LOG(chMain, "ENTER %s item=%d pane ok=%d (%d,%d,%d,%d)", what, item, ok ? 1 : 0, r.x, r.y, r.w, r.h);
    if (ok) g_menu.p->SetRect(g_ed.page, item, &r);
}

void placeBox(int32_t item, const editkit::Box& b) {
    if (item < 0 || !g_menu.ok() || !g_ed.page) return;
    const K2MuiRect r = hse::rectOf<K2MuiRect>(b);
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
    if (g_logVerbose && e.chord.busy() && e.chordLogs < kMaxChordLogs) {
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

// One CAMENG capture (CAMPTR once, then CAMENG + CAMATT + CAMPROJ x(2..3) + CAMCHK). Runs only when the key (view, camera pointer, edit state)
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
        snprintf(e.itemName[h], sizeof e.itemName[h], "%s", clean);   // cached for the weapon buttons (fitted in handCaption)
        if (note1[0]) KP_LOG(chView, "ITEMNAME hand=%s id=0x%08x item=0x%x resref=%s name0=\"%s\" clean=\"%s\" name1=(skipped)", kHand[h], static_cast<unsigned>(id),
                             static_cast<unsigned>(item), resref, name0, clean);
        else KP_LOG(chView, "ITEMNAME hand=%s id=0x%08x item=0x%x resref=%s name0=\"%s\" clean=\"%s\" name1=\"%s\"", kHand[h], static_cast<unsigned>(id),
                    static_cast<unsigned>(item), resref, name0, clean, name1);
    }
}

// ---- status text fitting and the body node lookup (the CAM2 probe) ----

void updateStatusText(Editor& e);   // defined with the status helpers below

// Text measuring context for every label fit: sy = ini text_k (default 2.65), UI units, texel width * 0.75 (TextK), no scale.
textfit::TextCtx textCtx() { return textfit::TextCtx{g_textK, 1.0f, 1.0f}; }

// raw cut with "..." to maxPx (the label font is not measured; ini status_fit scales the width).
void fitStatus(float maxPx, const char* raw, char* out, int cap) {
    const textfit::TextCtx tc = textCtx();
    textfit::fitText(tc, raw, maxPx * g_statusFit, out, cap);
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

// ---- text, colour and selector helpers ----

uintptr_t ctlOf(const Editor& e, int32_t item) {
    return (item >= 0 && g_menu.ok() && e.page) ? reinterpret_cast<uintptr_t>(g_menu.p->Control(e.page, item)) : 0;
}

// SetText on item idx only when the text differs from what the page last got (SetText is the expensive call). Cached text is cut to 63 characters.
void setItemText(Editor& e, int idx, const char* text) {
    if (idx < 0 || idx >= hse::kPageItemsV3 || e.item[idx] < 0 || !g_menu.ok() || !e.page || !text) return;
    char cut[sizeof e.selText[0]];
    snprintf(cut, sizeof cut, "%s", text);
    if (!strcmp(e.selText[idx], cut)) return;
    memcpy(e.selText[idx], cut, sizeof cut);
    g_menu.p->SetText(e.page, e.item[idx], cut);
}

// Empties the caption the item was built with (the cache starts at "", so a plain setItemText(e, idx, "") would be skipped as unchanged).
void clearItemText(Editor& e, int idx) {
    if (idx < 0 || idx >= hse::kPageItemsV3) return;
    e.selText[idx][0] = '\x01';
    e.selText[idx][1] = '\0';
    setItemText(e, idx, "");
}

// Selected look of a selector BUTTON: the vanilla hover frame. The hover border (button +0xE8) is drawn only while the mouse is over the button, so a lit
// selector gets the hover border's tint (border +0x28) on its normal border (+0x70); unlit puts the button's own tint back. Both tints are read once and logged.
// False (nothing written) when the control is not a linked Steam button, a tint read/write is refused, or the two tints are the same (nothing would show).
// Reads (once per item) the normal and hover frame tint of a selector / locked button into e.brdNormal / e.brdHover. False when a read is refused.
bool readBrd(Editor& e, int idx, uintptr_t n, uintptr_t h) {
    const int oTint = guikit::detail::off(&K2AddrTable::Border_Tint);
    if (!oTint) return false;
    if (!e.brdOk[idx]) {
        bool ok = true;
        for (int k = 0; k < 3 && ok; ++k)
            ok = guikit::detail::rd<float>(n, oTint + 4 * k, &e.brdNormal[idx][k]) && guikit::detail::rd<float>(h, oTint + 4 * k, &e.brdHover[idx][k]);
        e.brdOk[idx] = ok;
        KP_LOGV(chView, KPLOG_INFO, "STYLE frame tint item %d read=%d normal=(%.2f,%.2f,%.2f) hover=(%.2f,%.2f,%.2f)", idx, ok ? 1 : 0, e.brdNormal[idx][0], e.brdNormal[idx][1],
                e.brdNormal[idx][2], e.brdHover[idx][0], e.brdHover[idx][1], e.brdHover[idx][2]);
        if (!ok) return false;
    }
    return true;
}

bool setSelFrame(Editor& e, int idx, bool on) {
    const uintptr_t c = ctlOf(e, e.item[idx]);
    const int oTint = guikit::detail::off(&K2AddrTable::Border_Tint);
    if (!c || !oTint) return false;
    const uintptr_t n = guikit::detail::buttonBorder(c, false), h = guikit::detail::buttonBorder(c, true);
    if (!n || !h) return false;
    if (!readBrd(e, idx, n, h)) return false;
    float diff = 0.0f;
    for (int k = 0; k < 3; ++k) diff += fabsf(e.brdHover[idx][k] - e.brdNormal[idx][k]);
    if (diff < 0.05f) return false;
    const float* t = on ? e.brdHover[idx] : e.brdNormal[idx];
    for (int k = 0; k < 3; ++k)
        if (!guikit::detail::wr<float>(n, oTint + 4 * k, t[k])) return false;
    return true;
}

// Text colour of item idx: litCol (on) or the control's own default (off, and only after it was lit). The default is read on the first call. A refused
// read or write sets colorBad once: lit selectors then show >marks< instead (setSel). A control that is not linked yet is retried on the next call.
// Selector BUTTONS take the hover frame instead (setSelFrame; their text object is not the label one the text colour helpers write).
void setItemColor(Editor& e, int idx, bool on, const float* litCol) {
    if (idx < 0 || idx >= hse::kPageItemsV3 || e.item[idx] < 0 || e.colState[idx] == (on ? 1 : 0)) return;
    const uintptr_t c = ctlOf(e, e.item[idx]);
    if (!c) return;
    if (itemIsButton(idx)) {
        e.colState[idx] = on ? 1 : 0;
        if (setSelFrame(e, idx, on) || e.colorBad) return;
        e.colorBad = true;
        KP_LOGV(chView, KPLOG_WARN, "STYLE WARN hover-frame tint refused on button item %d (read=%d): lit selectors show >marks< instead", idx, e.brdOk[idx] ? 1 : 0);
        return;
    }
    if (!e.defColOk[idx]) e.defColOk[idx] = guikit::getTextColor(c, e.defCol[idx]);
    bool ok = e.defColOk[idx];
    if (on) {
        ok = ok && guikit::setTextColor(c, litCol[0], litCol[1], litCol[2]);
    } else if (e.colState[idx] == 1) {
        ok = ok && guikit::setTextColor(c, e.defCol[idx][0], e.defCol[idx][1], e.defCol[idx][2]);
    }
    e.colState[idx] = on ? 1 : 0;
    if (ok) return;
    if (!e.valueColorBad) {
        e.valueColorBad = true;
        KP_LOGV(chView, KPLOG_WARN, "STYLE WARN text colour refused on label item %d (default read=%d): changed values are not amber", idx, e.defColOk[idx] ? 1 : 0);
    }
}

// Selector button: lit when `on` (text colour, or >raw< when the colour call is refused), caption raw.
void setSel(Editor& e, int idx, bool on, const char* raw) {
    if (idx < 0 || idx >= hse::kPageItemsV3 || e.item[idx] < 0 || !g_menu.ok() || !raw) return;
    setItemColor(e, idx, on, kColLit);
    char text[64];
    if (on && e.colorBad) snprintf(text, sizeof text, ">%s<", raw);
    else snprintf(text, sizeof text, "%s", raw);
    setItemText(e, idx, text);
}

// Button text cut with "..." to maxPx (maxPx <= 0: the page is not laid out yet, no cut).
void fitButton(float maxPx, const char* raw, char* out, int cap) {
    const textfit::TextCtx tc = textCtx();
    if (maxPx <= 0.0f) snprintf(out, cap, "%s", raw);
    else textfit::fitText(tc, raw, maxPx, out, cap);
}

// Text room of a BUTTON caption: the box width less the engine's 7 px inset and 3 px slack per side (HseView.h buttonTextPx; 0 = not laid out yet).
float buttonPx(const Editor& e, const editkit::Box& b) { return e.layOk ? static_cast<float>(hse::buttonTextPx(b.w)) : 0.0f; }
// Text room of a selector caption: buttonPx, less the width of the ">" "<" marks that a lit selector gets when the colour call was refused (setSel),
// so the marked text still fits the button (never below 1 px: 0 would mean "not laid out yet").
float selPx(const Editor& e, const editkit::Box& b, bool lit) {
    float px = buttonPx(e, b);
    if (px <= 0.0f || !lit || !e.colorBad) return px;
    px -= textfit::textPx(textCtx(), "><", 2);
    return px < 1.0f ? 1.0f : px;
}
// Text room of a LABEL (status, note, hints, legend, inspector values): the box width less 4 px per side (0 = not laid out yet).
float labelPx(const Editor& e, const editkit::Box& b) { return e.layOk ? static_cast<float>(hse::labelTextPx(b.w)) : 0.0f; }

// Weapon button caption: the item display name (cached at build), else the model resref. "Right: Vibrosword" when that fits the button, else
// "R: Vibrosword", cut with "..." when even that is too wide.
void handCaption(const Editor& e, int hand, float maxPx, char* out, int cap) {
    const textfit::TextCtx tc = textCtx();
    char longT[96], shortT[96];
    hse::handButtonText(longT, sizeof longT, hand, false, e.itemName[hand], e.wpnName[hand], e.regGob[hand] != nullptr);
    if (maxPx <= 0.0f || textfit::textPx(tc, longT, static_cast<int>(strlen(longT))) <= maxPx) {
        snprintf(out, cap, "%s", longT);
        return;
    }
    hse::handButtonText(shortT, sizeof shortT, hand, true, e.itemName[hand], e.wpnName[hand], e.regGob[hand] != nullptr);
    fitButton(maxPx, shortT, out, cap);
}

// The lit state and caption of every selector (weapons, bones, scopes, stances, modes). Runs when the selection key changed (or `force`).
void refreshSelectors(Editor& e, bool force) {
    if (!g_menu.ok() || !e.page || e.refused || !e.layOk) return;
    const bool editing = e.mode == MODE_WEAPON;
    const int boneIdx = editing ? hse::boneIndexOf(e.editInfo.bone) : -1;
    int typeSum = 0;
    for (const char* c = e.editInfo.type; *c; ++c) typeSum = typeSum * 31 + static_cast<unsigned char>(*c);
    const int key[18] = {editing ? e.editHand : -2, boneIdx, e.editScope, e.stanceIdx, e.weaponMode, e.regGob[0] ? 1 : 0, e.regGob[1] ? 1 : 0, e.lay.weapon[0].w,
                         e.lay.bone[0].w, e.lay.scope[0].w, e.lay.stance[0].w, e.lay.modeBtn[0].w, e.colorBad ? 1 : 0, typeSum, editing ? 1 : 0, e.member, e.paused ? 1 : 0, e.drawn ? 1 : 0};
    if (!force && e.selKeyValid && !memcmp(key, e.selKey, sizeof key)) return;
    memcpy(e.selKey, key, sizeof key);
    e.selKeyValid = true;
    char raw[64], fit[64];
    for (int h = 0; h < hse::kV3Weapons; ++h) {
        const bool lit = editing && e.editHand == h;
        handCaption(e, h, selPx(e, e.lay.weapon[h], lit), raw, sizeof raw);
        setSel(e, hse::PI_WEAPON + h, lit, raw);
    }
    for (int i = 0; i < hse::kV3Bones; ++i) {
        const bool lit = editing && boneIdx == i;
        fitButton(selPx(e, e.lay.bone[i], lit), hse::boneLabel(i), fit, sizeof fit);
        setSel(e, hse::PI_BONE + i, lit, fit);
    }
    for (int i = 0; i < hse::kV3Scopes; ++i) {
        hse::scopeSegText(raw, sizeof raw, i, e.editInfo.type);
        const bool lit = editing && e.editScope == i;
        fitButton(selPx(e, e.lay.scope[i], lit), raw, fit, sizeof fit);
        setSel(e, hse::PI_SCOPE + i, lit, fit);
    }
    for (int i = 0; i < hse::kV3Stances; ++i) {
        const bool lit = e.stanceIdx == i;
        fitButton(selPx(e, e.lay.stance[i], lit), kStanceList[i].label, fit, sizeof fit);
        setSel(e, hse::PI_STANCE + i, lit, fit);
    }
    {   // Pause / Play toggle: caption "Play" and lit while the animation is frozen
        const bool lit = e.paused;
        fitButton(selPx(e, e.lay.pause, lit), lit ? "Play" : "Pause", fit, sizeof fit);
        setSel(e, hse::PI_PAUSE, lit, fit);
    }
    {   // Draw / Holster toggle: caption names the next action, lit while the weapons are drawn
        const bool lit = e.drawn;
        fitButton(selPx(e, e.lay.draw, lit), lit ? "Holster" : "Draw", fit, sizeof fit);
        setSel(e, hse::PI_DRAW, lit, fit);
    }
    for (int i = 0; i < kModeCount; ++i) {
        const bool lit = e.weaponMode == i;
        fitButton(selPx(e, e.lay.modeBtn[i], lit), hse::weaponModeName(i), fit, sizeof fit);
        setSel(e, hse::PI_MODE + i, lit, fit);
    }
}

// Selects the weapon mode. A running gesture keeps the mode it began with; the new one applies from the next press.
void setWeaponMode(Editor& e, int mode, const char* why) {
    if (mode < 0 || mode >= kModeCount || mode == e.weaponMode) return;
    KP_LOG(chView, "MODE %s -> %s why=%s%s", hse::weaponModeName(e.weaponMode), hse::weaponModeName(mode), why, e.gActive ? " (applies from the next press)" : "");
    e.weaponMode = mode;
    refreshSelectors(e, true);
}

// Stance back to Idle (a member rebuild starts from the idle pose).
void resetStanceUi(Editor& e) {
    e.stanceIdx = 0;
    e.paused = false;   // a rebuild (Destroy / Rebuild) clears the pause in the provider
    e.selKeyValid = false;
}

void updateStatusText(Editor& e) {
    if (!g_menu.ok() || !e.page || e.statusItem < 0) return;
    char raw[100] = {0}, buf[100] = {0};
    if (e.mode == MODE_WEAPON && e.discardArmed && e.discardAction == ACT_RESET_SCOPE && hse::guardArmLive(true, e.discardArmedAt, GetTickCount(), kDiscardWindowMs)) {
        snprintf(raw, sizeof raw, "Remove? Click again");
    } else if (e.mode == MODE_WEAPON && e.discardArmed && hse::guardArmLive(true, e.discardArmedAt, GetTickCount(), kDiscardWindowMs)) {
        snprintf(raw, sizeof raw, "Unsaved: click again to discard");
    } else if (e.mode == MODE_WEAPON) {
        refreshEdit(e);
        e.shownDirty = e.editInfo.dirty;
        snprintf(raw, sizeof raw, "%s", e.editInfo.dirty ? "Unsaved changes *" : "No unsaved changes");
    } else {
        snprintf(raw, sizeof raw, "Member %d/%d | Weapons: %s%s%s", e.member + 1, hse::memberCount(), e.regGob[0] ? "Right" : "-", (e.regGob[0] && e.regGob[1]) ? "+" : "",
                 e.regGob[1] ? "Left" : (e.regGob[0] ? "" : "None"));
    }
    fitStatus(labelPx(e, e.lay.status), raw, buf, static_cast<int>(sizeof buf));
    setItemText(e, hse::PI_STATUS, buf);
}

// The "Left: mirrored from right" note under the inspector rows (empty otherwise).
void refreshNote(Editor& e) {
    const bool mirrored = e.mode == MODE_WEAPON && e.editHand == 1 && hse::leftMirrored(e.editInfo.left != 0, e.editInfo.src);
    char fit[64];
    fitButton(labelPx(e, e.lay.note), mirrored ? "Left: mirrored from right" : "", fit, sizeof fit);
    setItemText(e, hse::PI_NOTE, fit);
}

// Reads the three panes. Logs LAYOUT once per change (and one WARN while a pane is unavailable).
bool refreshPanes(Editor& e) {
    if (!g_menu.ok() || !e.panel) return false;
    K2MuiRect c = {0, 0, 0, 0}, v = {0, 0, 0, 0}, p = {0, 0, 0, 0};
    const int okC = g_menu.p->PaneRect(e.panel, MUI_PANE_COLUMN, &c), okV = g_menu.p->PaneRect(e.panel, MUI_PANE_VIEW, &v), okP = g_menu.p->PaneRect(e.panel, MUI_PANE_PREVIEW, &p);
    if (!okC || !okV || !okP) {
        if (!e.paneWarned) {
            e.paneWarned = true;
            KP_LOGV(chView, KPLOG_WARN, "LAYOUT WARN pane rect unavailable (column=%d view=%d preview=%d): the page keeps its last layout", okC, okV, okP);
        }
        return e.panesOk;
    }
    const hse::HseRect nc = {c.x, c.y, c.w, c.h}, nv = {v.x, v.y, v.w, v.h}, np = {p.x, p.y, p.w, p.h};
    // MUI_PANE_DESC answers the LIVE LB_DESC rect: the shrunk one while the split is applied (ModConfig.cpp paneRect), the full original rect for the first ticks
    // before syncDescSplit ran. Both end on the same bottom, so the top is clipped under the preview pane (the preview bottom is where the shrunk rect starts, less
    // mod-config's small gap). All zero = unavailable: the right box stays absent.
    K2MuiRect d = {0, 0, 0, 0};
    hse::HseRect nd = {0, 0, 0, 0};
    if (g_menu.p->PaneRect(e.panel, MUI_PANE_DESC, &d) && d.w > 0 && d.h > 0) {
        const int pb = np.y + np.h;
        nd = hse::HseRect{d.x, d.y, d.w, d.h};
        if (nd.y < pb) { nd.h -= pb - nd.y; nd.y = pb; }
        if (nd.h <= 0 || nd.w <= 0) nd = hse::HseRect{0, 0, 0, 0};
    }
    if (!e.panesOk || memcmp(&nc, &e.paneCol, sizeof nc) != 0 || memcmp(&nv, &e.paneView, sizeof nv) != 0 || memcmp(&np, &e.panePreview, sizeof np) != 0 ||
        memcmp(&nd, &e.paneDesc, sizeof nd) != 0) {
        e.paneCol = nc;
        e.paneView = nv;
        e.panePreview = np;
        e.paneDesc = nd;
        e.panesOk = true;
        KP_LOG(chView, "LAYOUT panes column=(%d,%d,%d,%d) view=(%d,%d,%d,%d) preview=(%d,%d,%d,%d) desc=(%d,%d,%d,%d) raw=(%d,%d,%d,%d)", nc.x, nc.y, nc.w, nc.h, nv.x, nv.y, nv.w, nv.h,
               np.x, np.y, np.w, np.h, nd.x, nd.y, nd.w, nd.h, d.x, d.y, d.w, d.h);
    }
    return true;
}

// Lays the page out (hse::pageLayoutV3 from the panes and the thumb fractions) and sets EVERY item rect. SetRect runs every tick, not only on a move:
// mod-config gives up re-applying a rect the engine keeps rewriting after 8 tries, and each SetRect zeroes that count (mod-config only writes the
// engine when the extent really differs, so an unchanged rect costs a compare). The enlarged view, the wheel hit rect and the cached viewRect follow.
void applyCaptions(Editor& e);   // defined after logRoom below
void applyLayout(Editor& e) {
    if (!g_menu.ok() || !e.page || e.refused || !e.panesOk) return;
    const editkit::Box none = {0, 0, 0, 0};   // no hint rect: the description pane under the split is mod-config's hover text (HseLayoutV3.h)
    editkit::Box col = hse::boxOf(e.paneCol), prev = hse::boxOf(e.panePreview);
    col.x -= g_marginPx; col.w += g_marginPx;   // into the empty screen margins (ini margin_px 0 = stay inside the panes)
    prev.w += g_marginPx;
    // The right description box covers LB_DESC's own frame: the pane is inflated by desc_cover_px (clamped under the preview pane and inside the screen) before the
    // layout, so both description boxes (bottom aligned on that pane) keep equal bottoms.
    editkit::Box screen = {0, 0, 0, 0};
    {
        int sw = 0, sh = 0;
        if (guikit::g_gk.screenWAddr) (void)guikit::detail::rdInt(guikit::g_gk.screenWAddr, 0, &sw);
        if (guikit::g_gk.screenHAddr) (void)guikit::detail::rdInt(guikit::g_gk.screenHAddr, 0, &sh);
        if (sw >= 320 && sw <= 16384 && sh >= 200 && sh <= 16384) screen = editkit::Box{0, 0, sw, sh};
    }
    const editkit::Box paneBox = hse::boxOf(e.paneDesc);
    const editkit::Box coverBox = hse::coverDescPane(paneBox, prev, screen, g_descCoverPx);
    const hse::PageLayoutV3 L = hse::pageLayoutV3(col, hse::boxOf(e.paneView), prev, none, e.rowFrac, coverBox, e.speed);
    const bool first = !e.layOk;
    const int rState = L.hintR[0].w > 0 ? 1 : 0;
    if (rState != e.hintRState) {   // the right description box (a framed label over mod-config's LB_DESC, hides its scrollbar) was created / lost
        e.hintRState = rState;
        if (rState)
            KP_LOG(chView, "HINTR box created over LB_DESC rect=(%d,%d,%d,%d) text=(%d,%d,%d,%d) left=(%d,%d,%d,%d)", L.hintR[0].x, L.hintR[0].y, L.hintR[0].w, L.hintR[0].h,
                   L.hintR[1].x, L.hintR[1].y, L.hintR[1].w, L.hintR[1].h, L.hint[0].x, L.hint[0].y, L.hint[0].w, L.hint[0].h);
        else
            KP_LOG(chView, "HINTR box absent (desc pane=(%d,%d,%d,%d) unavailable or too small): the right help goes through SetDesc only", e.paneDesc.x, e.paneDesc.y, e.paneDesc.w,
                   e.paneDesc.h);
    }
    if (L.hintR[0].w > 0 && (first || memcmp(&coverBox, &e.coverShown, sizeof coverBox) != 0)) {   // once per page build (and when the panes move)
        e.coverShown = coverBox;
        KP_LOG(chView, "HINTR cover=%d pane=(%d,%d,%d,%d) box=(%d,%d,%d,%d)", g_descCoverPx, paneBox.x, paneBox.y, paneBox.w, paneBox.h, L.hintR[0].x, L.hintR[0].y,
               L.hintR[0].w, L.hintR[0].h);
    }
    e.lay = L;
    e.layOk = true;
    editkit::Box all[hse::kPageItemsV3];
    hse::pageBoxesV3(L, all);
    for (int i = 0; i < hse::kPageItemsV3; ++i) {
        const bool emptyHint = (itemIsHintBox(i) || itemIsHintText(i)) && (all[i].w <= 0 || all[i].h <= 0);   // no room for the left / right description box
        const bool hiddenRemove = i == hse::PI_REMOVE && !(e.removeMenu.open && e.mode == MODE_WEAPON);   // the Remove button exists only while the "..." menu is open
        const bool hiddenDraw = i == hse::PI_DRAW && !g_hol.canDraw();   // an older holster has no SetPreviewDraw: no toggle
        placeBox(e.item[i], (emptyHint || hiddenRemove || hiddenDraw) ? editkit::Box{-9999, -9999, 0, 0} : all[i]);
    }
    for (int r = 0; r < hse::kV3Rows; ++r) {   // the wedge is the track cut at the thumb centre (nothing at 0: parked off screen like the hidden items)
        const editkit::Box t = L.rows[r].track, th = L.rows[r].thumb;
        // The wedge's tall end is the right edge of its texture: cut the rect at the centre of the thumb box (the layout clamps the thumb inside the track),
        // so the wedge always ends exactly where the marker sits.
        const int w = th.w > 0 ? th.x + th.w / 2 - t.x : 0;
        placeBox(e.wedge[r], w > 0 && t.h > 0 ? editkit::Box{t.x, t.y, w, t.h} : editkit::Box{-9999, -9999, 0, 0});
    }
    {   // the Speed track's wedge: the same cut as the rows' wedges
        const editkit::Box t = L.speedTrack, th = L.speedThumb;
        const int w = th.w > 0 ? th.x + th.w / 2 - t.x : 0;
        placeBox(e.speedWedge, w > 0 && t.h > 0 ? editkit::Box{t.x, t.y, w, t.h} : editkit::Box{-9999, -9999, 0, 0});
    }
    if (e.speedValW != L.speedValue.w) {   // the value label box changed width (a resize): refit its text
        e.speedValW = L.speedValue.w;
        speedShowText(e);
    }
    applyCaptions(e);   // after every button rect is set: the engine's SetExtent just re-insetted each caption text rect
    if (!e.capBoxLogged) {   // once per Editor: the section titles (WEAPON, BONE, APPLIES TO) are full row boxes; a play check reads their heights here
        e.capBoxLogged = true;
        const int capIdx[3] = {hse::PI_CAP_WEAPON, hse::PI_CAP_BONE, hse::PI_CAP_SCOPE};
        for (int k = 0; k < 3; ++k)
            KP_LOG(chView, "CAPBOX i=%d h=%d rect=(%d,%d,%d,%d) rowH=%d", capIdx[k], all[capIdx[k]].h, all[capIdx[k]].x, all[capIdx[k]].y, all[capIdx[k]].w, all[capIdx[k]].h, L.leftRowH);
    }
    const hse::HseRect vr = hse::rectOf<hse::HseRect>(L.view);
    if (first || memcmp(&vr, &e.viewRect, sizeof vr) != 0) {
        KP_LOG(chView, "LAYOUT v3 view=(%d,%d,%d,%d) rows left=%d insp=%d bar=%d", vr.x, vr.y, vr.w, vr.h, L.leftRowH, L.inspRowH, L.barH);
        e.viewRect = vr;   // the cached view rect (the PROJ probe extent, the scrub hit area) follows the resized view
    }
    if (first || memcmp(&vr, &e.wheelRect, sizeof vr) != 0) {
        guikit::g_wheel.setRect(0, vr.x, vr.y, vr.w, vr.h);   // the wheel counts over the enlarged view
        e.wheelRect = vr;
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

// ---- button caption fix (HW-UX3 captions; research/holstered_weapons/ux_v3/F3_R1_inset.txt) ----
// CSWGuiButton::SetExtent (0x419ED0) gives the caption text sub-object (btn+0x160, vtable TextVtable) an inset rect (border dimension per side). At
// 34 px rows the inner height collapses, so the centred 32 px line is dropped or drawn on the box seam. After the button rect is set we set the text
// rect ourselves (hse::captionRect: engine x/w, button y/h) through the text object's own vtable slot 1 (guikit::setRect), no new engine address.
constexpr int kTextRectOff = 4;        // CSWGuiText +0x04..+0x10 = rect x,y,w,h (F3_R1_inset.txt "T+0x04..+0x10 rect"; decomp 41/00417780.c)
constexpr int kTextRendererOff = 0x14; // CSWGuiText +0x14 = renderer R (F3_R1_inset.txt "T+0x14 R pointer"; guikit::Text_Renderer)
constexpr int kRendWOff = 0x0C;        // renderer +0x0C = width (decomp 41/00417780.c, setSize w; probe only, read with the safe reader)
constexpr int kRendHOff = 0x10;        // renderer +0x10 = height (decomp 41/00417780.c, setSize h; probe only)
constexpr int kRendLinesOff = 0x4C;    // renderer +0x4C = line count (decomp 41/00417780.c; probe only)
constexpr int kRendScaleOff = 0x54;    // renderer +0x54 = float text scale (decomp 41/00417780.c; probe only)
constexpr int kRendAlignOff = 0x5A;    // renderer +0x5A = uint8 alignment flags (decomp 41/00417780.c; probe only)
constexpr int kParkedX = -9000;        // a button at x <= this is parked off screen (placeBox -9999) or hidden
constexpr int kCaptionReapplyMax = 8;  // re-applies per item when the engine resets the caption rect without a button rect change
constexpr int kMaxCaptionLogs = 40;    // cap on CAPTION probe lines per Editor

bool rdRect(uintptr_t obj, int base, hse::HseRect* r) {
    return rd<int>(obj, base, &r->x) && rd<int>(obj, base + 4, &r->y) && rd<int>(obj, base + 8, &r->w) && rd<int>(obj, base + 12, &r->h);
}

void logCaption(Editor& e, int i, const char* what, const hse::HseRect& b, const hse::HseRect& t, uintptr_t tx) {
    uint32_t rend = 0;
    int rbw = -1, rbh = -1, lines = -1;
    float scale = -1.0f;
    unsigned align = 0xFFFFFFFFu;   // printed as 0xff when unreadable
    if (rd<uint32_t>(tx, kTextRendererOff, &rend) && rend) {
        int v = 0;
        float f = 0.0f;
        uint8_t a = 0;
        if (rd<int>(rend, kRendWOff, &v)) rbw = v;
        if (rd<int>(rend, kRendHOff, &v)) rbh = v;
        if (rd<float>(rend, kRendScaleOff, &f)) scale = f;
        if (rd<uint8_t>(rend, kRendAlignOff, &a)) align = a;
        if (rd<int>(rend, kRendLinesOff, &v)) lines = v;
    }
    KP_LOG(chView, "CAPTION i=%d %s btn=(%d,%d,%d,%d) text=(%d,%d,%d,%d) rend=%p rb=(%d,%d) scale=%.2f align=0x%02x lines=%d fix=%d", i, what, b.x, b.y, b.w, b.h,
           t.x, t.y, t.w, t.h, reinterpret_cast<void*>(rend), rbw, rbh, scale, align & 0xFFu, lines, g_captionFix);
}

// Runs every layout tick, after every button rect is set. Per button: gate on the vtables, skip parked / hidden buttons, then set the caption rect when
// the button moved (new rect) or the engine reset it behind our back (capped re-applies).
void applyCaptions(Editor& e) {
    if (!g_captionFix && !g_probeCaption) return;
    for (int i = 0; i < hse::kPageItemsV3; ++i) {
        if (!itemIsButton(i) || e.item[i] < 0) continue;
        if (i >= hse::PI_ROW && (i - hse::PI_ROW) % hse::kRowStride == 4 && (i - hse::PI_ROW) / hse::kRowStride < hse::kV3Rows) continue;   // row reset icons show art, no caption
        const uintptr_t btn = ctlOf(e, e.item[i]);
        if (!btn) continue;
        const uintptr_t tx = btn + guibutton::Button_Text;
        uint32_t vb = 0, vt = 0;
        if (!rd<uint32_t>(btn, 0, &vb) || vb != guibutton::ButtonVtable || !rd<uint32_t>(tx, 0, &vt) || vt != static_cast<uint32_t>(guikit::TextVtable)) {
            if (e.capWarns < 1) {
                ++e.capWarns;
                KP_LOG(chView, "CAPTION WARN vtable i=%d btn=%p vb=0x%08x vt=0x%08x", i, reinterpret_cast<void*>(btn), vb, vt);
            }
            continue;
        }
        hse::HseRect b = {}, t = {};
        if (!rdRect(btn, kTextRectOff, &b) || !rdRect(tx, kTextRectOff, &t)) continue;
        if (b.x <= kParkedX || b.w <= 0 || b.h <= 0) continue;   // parked / hidden: leave it, the cache stays so the next real placement counts as moved
        const bool moved = !e.capSeen[i] || memcmp(&b, &e.capBtn[i], sizeof b) != 0;
        const hse::HseRect want = hse::captionRect(b, t);
        const bool differs = memcmp(&t, &want, sizeof t) != 0;
        if (g_probeCaption && moved && logRoom(e.capLogs, kMaxCaptionLogs, "CAPTION")) logCaption(e, i, "before", b, t, tx);
        if (g_captionFix && differs) {
            bool apply = false;
            if (moved) {
                e.capReapply[i] = 0;
                e.capCapped[i] = false;
                apply = true;
            } else if (e.capReapply[i] < kCaptionReapplyMax) {
                ++e.capReapply[i];
                apply = true;
            } else if (!e.capCapped[i]) {
                e.capCapped[i] = true;
                KP_LOG(chView, "CAPTION reapply capped i=%d after %d tries", i, kCaptionReapplyMax);
            }
            if (apply) {
                (void)guikit::setRect(tx, want.x, want.y, want.w, want.h);
                if (g_probeCaption && moved && e.capLogs <= kMaxCaptionLogs) {
                    hse::HseRect a = {};
                    if (rdRect(tx, kTextRectOff, &a)) KP_LOG(chView, "CAPTION i=%d after text=(%d,%d,%d,%d)", i, a.x, a.y, a.w, a.h);
                    else KP_LOG(chView, "CAPTION i=%d after text=(-1,-1,-1,-1)", i);
                }
            }
        }
        e.capBtn[i] = b;
        e.capSeen[i] = true;
    }
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

// Rewrites the six value texts, thumb fractions and amber marks from a FRESH GetEdit, but only when a value, the scope or an open value changed since
// the last time, and only the texts that changed (SetText is the expensive call). The thumb / wedge rects follow through rowFrac (applyLayout runs every
// tick). With log_verbose it logs the SetText count at most once per kInspLogWindowMs (capped).
void refreshInspector(Editor& e, bool force) {
    if (e.mode != MODE_WEAPON || !g_menu.ok() || !e.page) return;
    const bool ok = refreshEdit(e);
    const DWORD now = GetTickCount();
    if (now - e.inspWinStart >= kInspLogWindowMs) {
        if (g_logVerbose && e.inspWinSets > 0 && logRoom(e.inspLogs, kMaxInspLogs, "INSP"))
            KP_LOG(chView, "INSP refresh settext=%d ticks=%d window=%ums", e.inspWinSets, e.inspWinTicks, static_cast<unsigned>(now - e.inspWinStart));
        e.inspWinStart = now;
        e.inspWinSets = 0;
        e.inspWinTicks = 0;
    }
    if (!ok) return;   // the edit was lost: the callers that noticed it leave edit mode
    if (e.shownDirty != e.editInfo.dirty) updateStatusText(e);   // an unsaved edit starts / ends: the status label follows
    if (e.inspValW != e.lay.rows[0].value.w) {   // the value boxes were resized: refit every value text
        e.inspValW = e.lay.rows[0].value.w;
        e.inspForce = true;
    }
    const bool changed = force || e.inspForce || e.inspSeenScope != e.editInfo.scope || memcmp(e.inspSeenRot, e.editInfo.rot, sizeof e.inspSeenRot) != 0 ||
                         memcmp(e.inspSeenOff, e.editInfo.off, sizeof e.inspSeenOff) != 0;
    if (!changed) return;
    memcpy(e.inspSeenRot, e.editInfo.rot, sizeof e.inspSeenRot);
    memcpy(e.inspSeenOff, e.editInfo.off, sizeof e.inspSeenOff);
    e.inspSeenScope = e.editInfo.scope;
    if (force || e.inspForce) e.inspText.clear();
    e.inspForce = false;
    ++e.inspWinTicks;
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        const float v = hse::inspGet(r, e.editInfo.rot, e.editInfo.off);
        const float open = hse::inspGet(r, e.openRot, e.openOff);
        e.rowFrac[r] = hse::inspThumbFrac(r, v);
        char raw[32], fit[32];
        hse::inspValueText(raw, sizeof raw, r, v, true);
        fitButton(labelPx(e, e.lay.rows[r].value), raw, fit, sizeof fit);   // an over-wide value would word-wrap and lose the number
        const int idx = hse::PI_ROW + r * hse::kRowStride + 3;
        if (e.inspText.update(r, fit) && e.item[idx] >= 0) {
            setItemText(e, idx, fit);
            ++e.inspWinSets;
        }
        setItemColor(e, idx, e.openValid && hse::inspDiffers(r, v, open), kColAmber);
    }
}

// No edit open: the value labels read "-" and the thumbs sit in the middle.
void showNoEdit(Editor& e) {
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        e.rowFrac[r] = 0.5f;
        const int idx = hse::PI_ROW + r * hse::kRowStride + 3;
        setItemText(e, idx, "-");
        setItemColor(e, idx, false, kColAmber);
    }
    e.inspText.clear();
    e.inspForce = true;
    e.openValid = false;
}

// True when the cursor is over the control of an item.
bool overItem(const Editor& e, int32_t item, int x, int y) {
    const uintptr_t c = ctlOf(e, item);
    return c && guikit::hit(c, x, y);
}

// True when the cursor is over any button of the page (a press there is a button click, never a view gesture).
bool overAnyButton(const Editor& e, int x, int y) {
    if (!g_menu.ok() || !e.page) return false;
    for (int i = 0; i < hse::kPageItemsV3; ++i)
        if (itemIsButton(i) && overItem(e, e.item[i], x, y)) return true;
    return false;
}

// ---- Speed slider (holster-editor 0.7.1-dev, HW44): a global 1..8 slider under the 3D view, beside the Draw / Holster toggle ----
// Global: it does not depend on the weapon or the scope, and it never goes through the placement save or Revert, so it works in every mode. The value is
// the flat key slow_holster_x in the game-dir holster.ini (HseSpeed.h); holster 0.21.0-dev polls that file, so the preview and the game follow at once.
// A press (LMB edge) on the track or the thumb starts a drag: the value follows the cursor in whole steps (the thumb snaps to the step) and every step
// change is written (a drag writes at most seven times). Nothing is written until the value changes; the page only reads the key when it is built and
// whenever it is entered. The label and the wedge follow the value; the hover help covers the whole slider.

// All three boxes of the slider exist (a mod-config with a smaller page cap leaves them out; the slider is then absent).
bool speedBuilt(const Editor& e) {
    return e.item[hse::PI_SPEED_TRACK] >= 0 && e.item[hse::PI_SPEED_THUMB] >= 0 && e.item[hse::PI_SPEED_VALUE] >= 0;
}

// The cursor is over the track, the thumb or the value label (the hover help covers all three).
bool speedHover(const Editor& e, int x, int y) {
    return speedBuilt(e) && (overItem(e, e.item[hse::PI_SPEED_TRACK], x, y) || overItem(e, e.item[hse::PI_SPEED_THUMB], x, y) || overItem(e, e.item[hse::PI_SPEED_VALUE], x, y));
}

// The value label, fitted to its box: "Speed 4x", else the short "4x" when the box is narrow (a "Spe..." cut would lose the number), else cut.
void speedShowText(Editor& e) {
    if (e.item[hse::PI_SPEED_VALUE] < 0) return;   // not built yet: the item text starts from g_ed.speed when it is added
    char txt[32], fit[32];
    const float px = labelPx(e, e.lay.speedValue);
    hse::speedText(txt, sizeof txt, e.speed);
    if (px > 0.0f && textfit::textPx(textCtx(), txt, static_cast<int>(strlen(txt))) > px) hse::speedTextShort(txt, sizeof txt, e.speed);
    fitButton(px, txt, fit, sizeof fit);
    setItemText(e, hse::PI_SPEED_VALUE, fit);
}

// Reads the key the page shows (build, or the page is entered). Writes nothing.
void speedReadNow(Editor& e) {
    e.speedDrag = false;
    int bytes = 0;
    e.speed = hse::speedReadIni(hse::kSpeedIniFile, &bytes);
    speedShowText(e);
    KP_LOG(chView, "SPEED read %s %s=%d bytes=%d", hse::kSpeedIniFile, hse::kSpeedIniKey, e.speed, bytes);
}

// Sets the value (clamped). A change is shown and written at once (one log line per write); an unchanged value does nothing.
void speedSet(Editor& e, int v, const char* how) {
    v = hse::speedClamp(v);
    if (v == e.speed) return;
    e.speed = v;
    speedShowText(e);
    char val[8];
    snprintf(val, sizeof val, "%d", v);
    const bool ok = iniw::updateFile(hse::kSpeedIniFile, nullptr, hse::kSpeedIniKey, val, false);
    KP_LOG(chView, "SPEED write %s %s=%s ok=%d how=%s", hse::kSpeedIniFile, hse::kSpeedIniKey, val, ok ? 1 : 0, how);
}

// The Speed drag, once per tick after the mouse has been read. lEdge is the LMB press edge of this tick.
void speedTick(Editor& e, int x, int y, bool fg, bool lmb, bool lEdge) {
    if (!e.layOk || !e.page || !g_menu.ok() || !speedBuilt(e)) return;
    const editkit::Box t = e.lay.speedTrack;
    if (t.w <= 0) return;
    if (e.speedDrag) {
        if (!fg || !lmb) { e.speedDrag = false; return; }   // released, or the game lost the foreground (the value stays as it was written)
        speedSet(e, hse::speedFromX(x, t.x, t.w, hse::kV3ThumbW), "drag");
        return;
    }
    if (!fg || !lEdge) return;
    if (!overItem(e, e.item[hse::PI_SPEED_TRACK], x, y) && !overItem(e, e.item[hse::PI_SPEED_THUMB], x, y)) return;
    e.speedDrag = true;
    speedSet(e, hse::speedFromX(x, t.x, t.w, hse::kV3ThumbW), "press");
}

// The slider row under the cursor (its track or thumb: where a press scrubs), else -1. *onThumb is true when the cursor is over the thumb itself.
int sliderRowAt(const Editor& e, int x, int y, bool* onThumb) {
    if (onThumb) *onThumb = false;
    if (e.mode != MODE_WEAPON) return -1;
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        const int b = hse::PI_ROW + r * hse::kRowStride;
        const bool thumb = overItem(e, e.item[b + 2], x, y);
        if (thumb || overItem(e, e.item[b + 1], x, y)) {
            if (onThumb) *onThumb = thumb;
            return r;
        }
    }
    return -1;
}

// The row whose name, track, thumb, value or reset button is under the cursor (the hint line 2 follows it), else -1.
int hoverRowAt(const Editor& e, int x, int y) {
    if (e.mode != MODE_WEAPON) return -1;
    const hse::HseRect& pv = e.panePreview;   // the rows live in the preview pane: nothing to test elsewhere
    if (x < pv.x || y < pv.y || x >= pv.x + pv.w || y >= pv.y + pv.h) return -1;
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        const int b = hse::PI_ROW + r * hse::kRowStride;
        for (int k = 0; k < hse::kRowStride; ++k)
            if (overItem(e, e.item[b + k], x, y)) return r;
    }
    return -1;
}

constexpr const char* kSliderHelp = "Drag or click a slider. Shift fine, Ctrl snap.";
constexpr const char* kLeftHelp = "Pick the weapon, the bone and where Save applies, then adjust the sliders. Hover a button for help.";

// Sets the text of a description label (PI_HINT + 1 = left, PI_HINTR + 1 = right). mod-config keeps item text in 96 bytes, so the provider is told the cut text
// (keeps its state sane) and the label gets the FULL text straight through guikit. Written only when it changed.
void setHintText(Editor& e, int idx, const char* text) {
    if (idx < 0 || idx >= hse::kPageItemsV3 || e.item[idx] < 0 || !g_menu.ok() || !e.page || !text) return;
    char (&cache)[256] = e.longTxt[idx == hse::PI_HINTR + 1 ? 1 : 0];
    if (!strcmp(cache, text)) return;
    snprintf(cache, sizeof cache, "%s", text);
    g_menu.p->SetText(e.page, e.item[idx], cache);
    if (const uintptr_t c = ctlOf(e, e.item[idx])) (void)guikit::setText(c, cache);
}

// ---- Remove menu, SCOPE sentences ----

// The Remove two-click confirm is armed and still live (the left box then shows the "Click again to remove." sentence).
bool removeArmed(const Editor& e) {
    return e.discardArmed && e.discardAction == ACT_RESET_SCOPE && hse::guardArmLive(true, e.discardArmedAt, GetTickCount(), kDiscardWindowMs);
}

// Closes the "..." menu (no-op when closed) and drops a Remove arm that belonged to it. why: scope / hand / member / edit_end / leave / done / toggle.
void closeRemoveMenu(Editor& e, const char* why) {
    if (!e.removeMenu.open) return;
    e.removeMenu.close();
    if (e.discardArmed && e.discardAction == ACT_RESET_SCOPE) e.discardArmed = false;
    KP_LOG(chView, "REMOVE menu close why=%s", why);
}

// Opens or closes the menu from the "..." button; remembers which edit it belongs to.
void toggleRemoveMenu(Editor& e) {
    if (e.removeMenu.open) {
        closeRemoveMenu(e, "toggle");
        return;
    }
    e.removeMenu.toggle();
    e.removeScope = e.editScope;
    e.removeHand = e.editHand;
    e.removeMember = e.member;
    KP_LOG(chView, "REMOVE menu open scope=%d", e.editScope);
}

// Every tick: the menu belongs to one edit (scope, hand, member); any change of those, or the edit ending, closes it.
void syncRemoveMenu(Editor& e) {
    if (!e.removeMenu.open) return;
    if (e.mode != MODE_WEAPON) closeRemoveMenu(e, "edit_end");
    else if (e.editScope != e.removeScope) closeRemoveMenu(e, "scope");
    else if (e.editHand != e.removeHand || e.member != e.removeMember) closeRemoveMenu(e, "weapon");
}

// Logs the SCOPE line once per change of (scope, body, type) of the open edit, and again when an edit opens.
void logScopeIfChanged(Editor& e) {
    if (e.mode != MODE_WEAPON) {
        e.scopeLogScope = -1;
        return;
    }
    if (e.scopeLogScope == e.editScope && !strncmp(e.scopeLogBody, e.editInfo.body, 16) && !strncmp(e.scopeLogType, e.editInfo.type, 11)) return;
    e.scopeLogScope = e.editScope;
    snprintf(e.scopeLogBody, sizeof e.scopeLogBody, "%.16s", e.editInfo.body);
    snprintf(e.scopeLogType, sizeof e.scopeLogType, "%.11s", e.editInfo.type);
    char seg[48];
    hse::scopeSegText(seg, sizeof seg, e.editScope, e.scopeLogType);
    KP_LOG(chView, "SCOPE label=\"%s\" body=%s armor=\"%s\"", seg, e.scopeLogBody, hse::armorClassLabel(e.scopeLogBody));
}

// The Remove button while the menu is open: the selected (hover) frame tint marks it as the dangerous one (setItemColor, the same path the selectors use); the
// caption asks for the second click while the confirm is armed.
void refreshRemoveButton(Editor& e) {
    if (!e.removeMenu.open) return;
    setItemColor(e, hse::PI_REMOVE, true, kColAmber);
    char fit[64];
    fitButton(buttonPx(e, e.lay.remove), removeArmed(e) ? "Click again to remove" : hse::removeLabel(), fit, sizeof fit);
    setItemText(e, hse::PI_REMOVE, fit);
}

// The English name of the weapon being edited (the one on its button), or "" when none is cached / no hand is open: the sentences then show the model resref.
const char* editedItemName(const Editor& e) {
    return (e.mode == MODE_WEAPON && e.editHand >= 0 && e.editHand < 2) ? e.itemName[e.editHand] : "";
}

// Left description text of a hovered left-column button: the scope buttons and the Remove button answer with their sentence, the rest with itemTip.
void leftTipText(const Editor& e, int tip, char* buf, int cap) {
    const bool editing = e.mode == MODE_WEAPON;
    const char* model = editing ? e.editInfo.model : "";
    const char* type = editing ? e.editInfo.type : "";
    const char* body = editing ? e.editInfo.body : "";
    if (tip >= hse::PI_SCOPE && tip < hse::PI_SCOPE + hse::kV3Scopes) hse::scopeSentence(buf, cap, tip - hse::PI_SCOPE, model, type, body, editedItemName(e));
    else if (tip == hse::PI_REMOVE) hse::removeSentence(buf, cap, e.editScope, model, type, body, removeArmed(e), editedItemName(e));
    else snprintf(buf, cap, "%s", itemTip(tip));
}

// Estimated line count of `t` word-wrapped to maxPx (greedy on spaces, the fit measure of textCtx; at least 1).
int wrapLinesEst(const char* t, float maxPx) {
    const textfit::TextCtx tc = textCtx();
    const float sp = textfit::textPx(tc, " ", 1);
    int lines = 1;
    float cur = 0.0f;
    const char* p = t;
    while (*p) {
        while (*p == ' ') ++p;
        const char* w = p;
        while (*p && *p != ' ') ++p;
        if (p == w) break;
        const float ww = textfit::textPx(tc, w, static_cast<int>(p - w));
        if (cur > 0.0f && cur + sp + ww > maxPx) { ++lines; cur = ww; }
        else cur = cur > 0.0f ? cur + sp + ww : ww;
    }
    return lines;
}

// The left description box with nothing hovered: the armed / open Remove sentence, else "Save changes go to ..." for the current scope followed by the short help
// when the pair still fits the text area (estimated lines of kLeftLinePx each; the box is small at low resolutions and the text would run out of it); without an
// open edit the short help alone.
constexpr int kLeftLinePx = 32;
void leftDefaultText(const Editor& e, char* buf, int cap) {
    if (e.mode != MODE_WEAPON) {
        snprintf(buf, cap, "%s", kLeftHelp);
        return;
    }
    if (e.removeMenu.open) {
        hse::removeSentence(buf, cap, e.editScope, e.editInfo.model, e.editInfo.type, e.editInfo.body, removeArmed(e), editedItemName(e));
        return;
    }
    hse::scopeSentence(buf, cap, e.editScope, e.editInfo.model, e.editInfo.type, e.editInfo.body, editedItemName(e));
    const size_t n = strlen(buf), h = strlen(kLeftHelp);
    if (n + 1 + h >= static_cast<size_t>(cap) || e.lay.hint[1].w <= 0 || e.lay.hint[1].h <= 0) return;
    char both[256];
    snprintf(both, sizeof both, "%s %s", buf, kLeftHelp);
    if (wrapLinesEst(both, static_cast<float>(e.lay.hint[1].w)) * kLeftLinePx <= e.lay.hint[1].h) snprintf(buf, cap, "%s", both);
}

// Right description pane (mod-config's green LB_DESC, written through SetDesc, and our framed label drawn over it): the help of the hovered slider row plus the
// drag keys, else the help of the hovered RIGHT-side button (mode, view, stance, reset icons), else the drag keys. Written when the shown text changes only.
// Left description box (hint items): the help of the hovered left-column button, else the live "Save changes ..." sentence of the current scope. The text is
// rebuilt every tick and compared with what the box last got (setHintText), so a change of scope, model, type, body or the Remove menu state refreshes it.
// Without the left box (small panes) the left help goes to the right pane.
void refreshHints(Editor& e, int x, int y) {
    if (!g_menu.ok() || !e.page) return;
    const bool haveL = e.lay.hint[0].w > 0, haveR = e.lay.hintR[0].w > 0;
    if (haveR != e.hintRHave) { e.hintRHave = haveR; e.hintRow = -2; }   // the box appeared / went: write its text again
    if (haveL != e.hintLHave) { e.hintLHave = haveL; }
    int tip = -1;
    for (int i = 0; i < hse::kPageItemsV3 && tip < 0; ++i)
        if (itemIsLeftTip(i) && overItem(e, e.item[i], x, y)) tip = i;
    const int row = hoverRowAt(e, x, y);
    int rtip = -1;   // a RIGHT-side button with help (the mode bar, the view icons, the stances, the group resets)
    for (int i = 0; row < 0 && i < hse::kPageItemsV3 && rtip < 0; ++i)
        if (itemIsButton(i) && !itemIsLeftTip(i) && itemTip(i)[0] && overItem(e, e.item[i], x, y)) rtip = i;
    const bool speedHov = row < 0 && rtip < 0 && speedHover(e, x, y);   // the whole Speed slider (track, thumb, label), not only a part of it
    // key of what the right pane shows: a slider row, 100 + a right button, 200 + a left button (no left box), 300 the Speed slider, -1 the default
    const int key = row >= 0 ? row : (rtip >= 0 ? 100 + rtip : (speedHov ? 300 : ((!haveL && tip >= 0) ? 200 + tip : -1)));
    char buf[256];
    if (row >= 0) snprintf(buf, sizeof buf, "%s %s", hse::inspHint(row), kSliderHelp);
    else if (rtip >= 0) snprintf(buf, sizeof buf, "%s", itemTip(rtip));
    else if (speedHov) snprintf(buf, sizeof buf, "%s", hse::speedTip());
    else if (key >= 200) leftTipText(e, tip, buf, sizeof buf);
    else snprintf(buf, sizeof buf, "%s", kSliderHelp);
    if (key != e.hintRow || strcmp(buf, e.hintRBuf)) {
        e.hintRow = key;
        snprintf(e.hintRBuf, sizeof e.hintRBuf, "%s", buf);
        g_menu.p->SetDesc(e.page, buf);
        if (haveR) setHintText(e, hse::PI_HINTR + 1, buf);
    }
    if (!haveL) return;
    char lbuf[256];
    if (tip >= 0) leftTipText(e, tip, lbuf, sizeof lbuf);
    else leftDefaultText(e, lbuf, sizeof lbuf);
    setHintText(e, hse::PI_HINT + 1, lbuf);   // compares with the text the box last got: a no-op while nothing changed
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
            KP_LOGV(chView, KPLOG_WARN, "%s WARN %s row=%s: SetValues did not take, sent %.5f read back %.5f (scope=%d)", kind == W_SCRUB ? "SCRUB" : "INSP", what,
                    hse::inspRowName(row), value, got, e.editInfo.scope);
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
    const float v = hse::inspGet(row, e.editInfo.rot, e.editInfo.off);
    if (!e.scrub.begin(row, x, v, e.lay.rows[row].track.w)) {
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
// A scrub begins only when the chord turned an IDLE press into a DEAD "press_outside" (so a press on a button or outside the view, never a
// foreground-return or stale press) with the cursor on a slider's track or thumb and no other button down. A press on the track outside the thumb first
// jumps the value to the pressed position (the track spans +-30 cm / +-180 deg around zero). The drag is scaled to the track width, so the thumb follows the cursor (Shift/Ctrl detach it on purpose). It runs while LMB is held, ends on release, and is cancelled
// by foreground loss.
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
    if (!fg || !lEdge || rmb || mmb) return;
    if (co.drop != hse::CW_PRESS_OUTSIDE || co.why != hse::CW_PRESS_OUTSIDE) return;
    bool onThumb = false;
    const int row = sliderRowAt(e, x, y, &onThumb);
    if (row < 0) return;
    if (!onThumb && refreshEdit(e)) {
        const editkit::Box t = e.lay.rows[row].track;
        const float to = hse::inspTrackValue(row, x, t.x, t.w);
        if (hse::finiteF(to)) {
            if (logRoom(e.scrubLogs, kMaxScrubLogs, "SCRUB"))
                KP_LOG(chView, "SCRUB jump row=%s from=%.4f%s to=%.4f%s at=(%d,%d)", hse::inspRowName(row), hse::inspGet(row, e.editInfo.rot, e.editInfo.off), rowUnit(row), to,
                       rowUnit(row), x, y);
            if (!applyRowValue(e, row, to, W_SCRUB, "jump")) return;   // the scrub below would start from a value that did not take
        }
    }
    scrubBegin(e, row, x, y, shift, ctrl);
}

// The row's reset button: that one value goes back to what it was when the edit opened. The other five are sent unchanged.
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
    float rot[3], off[3];
    const float from = hse::inspGet(row, e.editInfo.rot, e.editInfo.off);
    const float to = hse::inspGet(row, e.openRot, e.openOff);
    const bool changes = hse::inspRevert(row, e.editInfo.rot, e.editInfo.off, e.openRot, e.openOff, rot, off);
    if (logRoom(e.inspLogs, kMaxInspLogs, "INSP"))
        KP_LOG(chView, "INSP RESET row=%s from=%.4f%s to=%.4f%s %s", hse::inspRowName(row), from, rowUnit(row), to, rowUnit(row), changes ? "applied" : "(already at the open value)");
    if (!changes) return;
    if (applyRowValue(e, row, to, W_INSP, "reset")) updateStatusText(e);
}

// The group reset (Position = rows 0..2, Rotation = rows 3..5): its rows go back to the open values in ONE SetValues. Body scope skips the rotation rows.
void inspResetGroup(Editor& e, int group) {
    if (group < 0 || group >= hse::kV3Groups) return;
    if (!e.openValid) {
        if (warnOk(e, W_INSP)) KP_LOGV(chView, KPLOG_WARN, "INSP WARN RESET group=%d: no open values stored", group);
        return;
    }
    scrubCancel(e, "reset_group");
    if (!refreshEdit(e)) {
        if (warnOk(e, W_INSP)) KP_LOGV(chView, KPLOG_WARN, "INSP WARN RESET group=%d: the edit is not open", group);
        exitEdit(e);
        return;
    }
    float rot[3], off[3];
    memcpy(rot, e.editInfo.rot, sizeof rot);
    memcpy(off, e.editInfo.off, sizeof off);
    int moved = 0;
    for (int r = group * 3; r < group * 3 + 3; ++r) {
        const float to = hse::inspGet(r, e.openRot, e.openOff);
        if (hse::inspGet(r, rot, off) != to) ++moved;
        hse::inspSet(r, rot, off, to);
    }
    if (logRoom(e.inspLogs, kMaxInspLogs, "INSP")) KP_LOG(chView, "INSP RESET group=%s rows moved=%d %s", group == 0 ? "Position" : "Rotation", moved, moved ? "applied" : "(already at the open values)");
    if (!moved) return;
    g_hol.h->SetValues(rot, off);
    if (!refreshEdit(e)) {
        if (warnOk(e, W_INSP)) KP_LOGV(chView, KPLOG_WARN, "INSP WARN RESET group=%d: the edit closed after SetValues", group);
        exitEdit(e);
        return;
    }
    updateStatusText(e);
}

void enterEdit(Editor& e, int hand) {
    if (hand < 0 || hand > 1 || !e.regGob[hand] || !g_hol.ok()) return;
    closeRemoveMenu(e, "weapon");
    if (e.mode == MODE_WEAPON) g_hol.h->EndEdit();
    if (!g_hol.h->BeginEdit(e.regGob[hand], e.editScope)) {
        KP_LOG(chView, "EDIT refused hand=%d gob=%p scope=%d", hand, e.regGob[hand], e.editScope);
        e.mode = MODE_NAV;   // the previous edit (if any) was just ended
        e.editHand = -1;
        e.projEditPending = false;
        resetInput(e, "edit_refused");
        showNoEdit(e);
        refreshSelectors(e, true);
        refreshNote(e);
        setItemText(e, hse::PI_STATUS, "Edit unavailable");
        return;
    }
    e.mode = MODE_WEAPON;
    e.editHand = hand;
    resetInput(e, "enter_edit");   // drags from the previous edit are dropped; a button still held (the click that switched hands) must be released first
    e.projEditPending = true;
    e.projEditTries = 0;
    e.shownDirty = -1;
    refreshEdit(e);
    KP_LOG(chView, "EDIT enter hand=%d gob=%p model=%s bone=%s", hand, e.regGob[hand], e.editInfo.model, e.editInfo.bone);
    e.inspText.clear();   // the first refresh rewrites every value text
    captureOpen(e, "enter_edit");
    refreshSelectors(e, true);
    refreshNote(e);
    updateStatusText(e);
    refreshInspector(e, true);
}

void exitEdit(Editor& e) {
    if (e.mode != MODE_WEAPON) return;
    closeRemoveMenu(e, "edit_end");
    if (g_hol.ok()) g_hol.h->EndEdit();
    e.mode = MODE_NAV;
    e.editHand = -1;
    e.projEditPending = false;
    resetInput(e, "exit_edit");
    KP_LOG(chView, "EDIT exit");
    showNoEdit(e);
    refreshSelectors(e, true);
    refreshNote(e);
    updateStatusText(e);
}

void KPATCH_CALL cbEnter(uint32_t page, void*) {
    if (!g_ed.panel || page != g_ed.page || !g_menu.ok()) return;
    saferead::beginScope();
    if (g_ed.refused) {   // the page only carries the message: everything else sits off screen
        for (int i = 0; i < hse::kPageItemsV3; ++i) {
            if (i == hse::PI_STATUS) continue;
            placeBox(g_ed.item[i], editkit::Box{-9999, -9999, 0, 0});
        }
        for (int r = 0; r < hse::kV3Rows; ++r) placeBox(g_ed.wedge[r], editkit::Box{-9999, -9999, 0, 0});
        placeBox(g_ed.speedWedge, editkit::Box{-9999, -9999, 0, 0});
        placeItem(g_ed.statusItem, MUI_PANE_STATUS, "refusal message");
        if (g_ed.statusItem >= 0) g_menu.p->SetText(g_ed.page, g_ed.statusItem, kNeedsModConfig);
        KP_LOG(chView, "ENTER refused page: %s", kNeedsModConfig);
        return;
    }
    if (g_menu.hasDescSplit()) {   // build asked for it already; asking again is harmless and covers a provider that dropped it with the previous page
        const int rc = g_menu.p->SetDescSplit(g_ed.page, hse::kDescSplitPermille);
        KP_LOG(chView, "ENTER desc split %d permille rc=%d", hse::kDescSplitPermille, rc);
    }
    g_ed.panesOk = false;
    g_ed.layOk = false;
    g_ed.paneWarned = false;
    speedReadNow(g_ed);   // the Speed value is read on every open (holster.ini may have changed since the last one)
    g_ed.selKeyValid = false;
    g_ed.hintRow = -2;
    refreshPanes(g_ed);
    guikit::wheelInstall();
    applyLayout(g_ed);
    (void)guikit::g_wheel.take();   // drop any notches counted before this page was shown
    guikit::g_wheel.touch(GetTickCount());
    setItemText(g_ed, hse::PI_STATUS, statusText());
    char fit[64];
    g_ed.longTxt[0][0] = g_ed.longTxt[1][0] = '\x01';   // rewrite both description texts on this visit
    g_ed.hintRState = -1;
    {   // the legend line under the view: cut to the view width
        char leg[64];
        fitButton(labelPx(g_ed, g_ed.lay.legend), hse::helpLine(), leg, sizeof leg);
        setItemText(g_ed, hse::PI_LEGEND, leg);
    }

    g_ed.view = g_ed.cfgView;
    g_ed.state = NEEDBUILD;
}

void KPATCH_CALL cbLeave(uint32_t, void*) {
    saferead::beginScope();
    guikit::g_wheel.clearRect(0);   // stop the wheel hook counting this page's area
    g_ed.wheelRect = {0, 0, 0, 0};
    g_ed.discardArmed = false;
    closeRemoveMenu(g_ed, "leave");
    if (g_ed.mode == MODE_WEAPON) {
        if (editDirty()) KP_LOG(chView, "DISCARD why=leave");
        exitEdit(g_ed);
    }
    g_ed.state = IDLE;
    resetInput(g_ed, "leave");
    g_ed.pressed = false;   // the page is gone: a button still held must not look like "was down" next time
    g_ed.pressedMmb = false;
    g_ed.pressedRmb = false;
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

// Two-click confirm that is NOT about unsaved edits (Reset Scope deletes saved keys): always needs the second click on the same action within
// kDiscardWindowMs. True when the caller may run the action; otherwise arms the window and shows the warning in the status line.
bool guardConfirm(Editor& e, int action, const char* why) {
    const DWORD now = GetTickCount();
    const bool armedSame = e.discardArmed && e.discardAction == action;
    if (hse::guardArmLive(armedSame, e.discardArmedAt, now, kDiscardWindowMs)) {
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
        KP_LOGV(chView, KPLOG_WARN, "NUDGE WARN %s: no change (call0 kind=%d axis=%d deg=%.4f m=%.4f dp=(%.4f,%.4f,%.4f) scope=%d)", what, c.kind, c.axis, c.deg, c.metres,
                c.dp[0], c.dp[1], c.dp[2], e.editScope);
    }
    return true;
}

// Weapon-mode mouse for one tick: feeds hse::Chord and acts on what it answers. All logging of the chord lives here.
hse::ChordOut weaponMouse(Editor& e, int curX, int curY, bool insideView, bool fg, bool shift, bool ctrl, bool lmb, bool rmb, bool mmb, LONG wheel) {
    // Without an open edit (no weapon on this member) RMB means nothing: the chord can only orbit, spin and zoom.
    const bool weaponOk = e.mode == MODE_WEAPON;
    const bool rmbIn = rmb && weaponOk;
    hse::ChordIn ci = {};
    ci.l = lmb;
    ci.r = rmbIn;
    ci.m = mmb;
    ci.lEdge = lmb && !e.pressed;
    ci.rEdge = rmbIn && !e.pressedRmb;
    ci.mEdge = mmb && !e.pressedMmb;
    e.pressed = lmb;
    e.pressedRmb = rmbIn;
    e.pressedMmb = mmb;
    ci.inView = insideView && !overAnyButton(e, curX, curY);   // a press on any button is a dead press (the slider scrub starts from a dead press outside the view)
    ci.fg = fg;
    ci.shift = shift;
    ci.ctrl = ctrl;
    ci.x = curX;
    ci.y = curY;
    ci.wheel = static_cast<int>(wheel);
    if (!rmb && e.snap.nonzero() && !e.gActive) resetSnap(e, "rmb_up");   // a wheel remainder does not outlive the held RMB

    const hse::ChordOut o = e.chord.step(ci);

    // CHORD log: a press that is dropped (outside the view, over a button, ...) is a drop line, capped separately; when the drop is also the transition
    // reason (-> DEAD) the transition line is the drop line. Every other state change or gesture begin/end is logged only with ini log_verbose.
    const bool transition = o.state != o.prevState || o.why != hse::CW_NONE;
    const bool dropIsTransition = transition && o.drop != hse::CW_NONE && o.drop == o.why;
    if (transition && (dropIsTransition || g_logVerbose)) {
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
        p3d::ViewState vs = e.view;   // hse::View is p3d::ViewState
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

// ---- slider and reset art ----

uintptr_t g_artCtl = 0;   // the control the reset-icon callbacks work on (resetIconApply takes plain function pointers)
bool artBtnFill() {
    if (!guikit::setButtonFill(g_artCtl, hse::resetIconImage(false), hse::resetIconImage(true))) return false;   // false on a label clone: the label path follows
    (void)guikit::setButtonFillStyle(g_artCtl, 2);   // stretch the 32x32 arrow over the small square
    return true;
}
bool artLabelFill() {
    if (!guikit::setFill(g_artCtl, hse::resetIconImage(false))) return false;
    (void)guikit::setFillStyle(g_artCtl, 2);
    return true;
}

// Puts the slider art (frame on each track, wedge, marker on each thumb) and the reset arrows on their controls, once, as soon as the textures exist and
// every control is linked. A control that refuses its art keeps its plain look (the reset buttons keep their "R"). Gives up after kArtMaxTicks.
void applyArt(Editor& e) {
    constexpr int kArtMaxTicks = 900;
    if (e.artDone || e.refused || !g_menu.ok() || !e.page) return;
    // The slider art and the reset arrows are separate texture sets: a set that is still missing at the deadline is skipped, the other one is applied.
    const bool timedOut = ++e.artTicks > kArtMaxTicks;
    bool haveIcon = hse::resetIconImage(false) && hse::resetIconImage(true) && hse::flipIconImage(false) && hse::flipIconImage(true);
    bool haveSlider = true;
    for (int w = 0; w < hse::SA_Count; ++w)
        if (!hse::sliderArtImage(w)) haveSlider = false;
    for (int r = 0; r < hse::kV3Rows && haveSlider; ++r) {
        const int b = hse::PI_ROW + r * hse::kRowStride;
        if (!ctlOf(e, e.item[b + 1]) || !ctlOf(e, e.wedge[r]) || !ctlOf(e, e.item[b + 2])) haveSlider = false;
    }
    for (int i = 0; i < hse::kPageItemsV3 && haveSlider; ++i)
        if ((itemIsHintBox(i) || itemIsHintText(i)) && !ctlOf(e, e.item[i])) haveSlider = false;
    for (int i = 0; i < hse::kPageItemsV3 && haveIcon; ++i)
        if ((itemIsResetIcon(i) || i == hse::PI_VIEW_FLIP) && !ctlOf(e, e.item[i])) haveIcon = false;
    if (!timedOut && (!haveIcon || !haveSlider)) return;
    if (timedOut)
        KP_LOGV(chView, KPLOG_WARN, "ART WARN not ready after %d ticks (slider=%d reset_icon=%d): the missing part keeps its plain controls", kArtMaxTicks,
                haveSlider ? 1 : 0, haveIcon ? 1 : 0);
    int frame = 0, wedge = 0, mark = 0, icon = 0, icons = 0, flip = 0, hintBox = 0, descStyle = 0, nameBox = 0;
    for (int r = 0; r < hse::kV3Rows && haveSlider; ++r) {
        const int b = hse::PI_ROW + r * hse::kRowStride;
        const uintptr_t tc = ctlOf(e, e.item[b + 1]), wc = ctlOf(e, e.wedge[r]), mc = ctlOf(e, e.item[b + 2]);
        if (hse::sliderArtFill(tc, hse::SA_Frame)) { ++frame; (void)guikit::setFillStyle(tc, 2); }
        if (hse::sliderArtFill(wc, hse::SA_Wedge)) { ++wedge; (void)guikit::setFillStyle(wc, 2); }
        if (hse::sliderArtFill(mc, hse::SA_Mark)) { ++mark; (void)guikit::setFillStyle(mc, 2); }
    }
    if (haveSlider) {   // the Speed slider (optional item, absent with a small page cap): the same frame, wedge and marker art as the inspector rows
        const uintptr_t tc = ctlOf(e, e.item[hse::PI_SPEED_TRACK]), wc = ctlOf(e, e.speedWedge), mc = ctlOf(e, e.item[hse::PI_SPEED_THUMB]);
        int speedArt = 0;
        if (hse::sliderArtFill(tc, hse::SA_Frame)) { ++speedArt; (void)guikit::setFillStyle(tc, 2); }
        if (hse::sliderArtFill(wc, hse::SA_Wedge)) { ++speedArt; (void)guikit::setFillStyle(wc, 2); }
        if (hse::sliderArtFill(mc, hse::SA_Mark)) { ++speedArt; (void)guikit::setFillStyle(mc, 2); }
        KP_LOG(chView, "ART speed slider=%d/3", speedArt);
    }
    for (int i = 0; i < hse::kPageItemsV3 && haveSlider; ++i) {   // the left description box: the vanilla description colours as a stretched fill, its text in the description font
        if (itemIsHintBox(i)) {
            const uintptr_t hc = ctlOf(e, e.item[i]);
            if (hse::sliderArtFill(hc, hse::SA_Box)) { ++hintBox; (void)guikit::setFillStyle(hc, 2); }
        } else if (itemIsHintText(i)) {
            const uintptr_t tc = ctlOf(e, e.item[i]);
            if (tc) {   // the description teal (37,200,164) + top-left, as the Options description list draws. No SetFont: SetFont(fnt_d16x16b) on this label
                        // crashed live (10-18 21:51, 0x47AFA8 null font data in the wrap pass, backtrace SetFont 0x416DE0 <- holster-editor) so the label keeps its font
                descStyle |= (guikit::setTextColor(tc, 37.0f / 255.0f, 200.0f / 255.0f, 164.0f / 255.0f) ? 2 : 0) |
                             (guikit::setAlign(tc, 9) ? 4 : 0);
            }
        }
    }
    if (haveSlider) {   // the party member name: the hint-box frame fill, centred, lit colour, so it reads as a name plate and not as a section title
        const uintptr_t nc = ctlOf(e, e.item[hse::PI_MEMBER_NAME]);
        if (nc && hse::sliderArtFill(nc, hse::SA_Box)) {
            (void)guikit::setFillStyle(nc, 2);
            (void)guikit::setAlign(nc, 18);
            (void)guikit::setTextColor(nc, kColLit[0], kColLit[1], kColLit[2]);   // a refused colour only keeps the plain text colour
            nameBox = 1;
        }
    }
    for (int i = 0; i < hse::kPageItemsV3 && haveIcon; ++i) {
        if (!itemIsResetIcon(i)) continue;
        ++icons;
        g_artCtl = ctlOf(e, e.item[i]);
        if (hse::resetIconApply(artBtnFill, artLabelFill)) {
            ++icon;
            clearItemText(e, i);   // the arrow replaces the "R"
        }
    }
    g_artCtl = 0;
    if (haveIcon) {
        const uintptr_t fc = ctlOf(e, e.item[hse::PI_VIEW_FLIP]);
        if (fc && guikit::setButtonFill(fc, hse::flipIconImage(false), hse::flipIconImage(true))) {
            (void)guikit::setButtonFillStyle(fc, 2);
            clearItemText(e, hse::PI_VIEW_FLIP);
            flip = 1;
        }
    }
    e.artDone = true;
    e.artCtlCount = 0;
    auto noteCtl = [&e](int32_t item) {
        const uintptr_t c = ctlOf(e, item);
        if (c && e.artCtlCount < static_cast<int>(sizeof e.artCtl / sizeof e.artCtl[0])) e.artCtl[e.artCtlCount++] = c;
    };
    for (int i = 0; i < hse::kPageItemsV3; ++i) noteCtl(e.item[i]);
    for (int r = 0; r < hse::kV3Rows; ++r) noteCtl(e.wedge[r]);
    noteCtl(e.speedWedge);
    KP_LOG(chView, "ART applied frame=%d/%d wedge=%d/%d mark=%d/%d reset_icon=%d/%d flip=%d/1 hint_box=%d/2 desc_style=%d/6 after %d ticks", frame, hse::kV3Rows, wedge, hse::kV3Rows, mark,
           hse::kV3Rows, icon, icons, flip, hintBox, descStyle, e.artTicks);
    KP_LOG(chView, "NAMEBOX art=%d", nameBox);
}

// Mounting bone `want` (0..4): HolsterApi has no absolute bone setter, so this cycles CycleBone until GetEdit reports the bone, at most
// hse::kEditBones steps. True when the edit now reports it; false (logged) when it never does or the edit was lost.
bool selectBone(Editor& e, int want) {
    if (!g_hol.ok() || want < 0 || want >= hse::kEditBones) return false;
    int steps = 0;
    bool ok = false;
    for (;; ++steps) {
        if (!refreshEdit(e)) {
            KP_LOG(chView, "EDIT lost during SelectBone want=%s after %d steps", hse::boneId(want), steps);
            exitEdit(e);
            return false;
        }
        if (hse::boneIndexOf(e.editInfo.bone) == want) { ok = true; break; }
        if (steps >= hse::kEditBones) break;
        g_hol.h->CycleBone(1);   // returns a bone index, never a success flag
    }
    KP_LOG(chView, "BTN SelectBone %s steps=%d now=%s%s", hse::boneId(want), steps, e.editInfo.bone, ok ? "" : " (never reported: loop ended)");
    return ok;
}

// ---- the tick ----

// The page's buttons, mouse, state machine and probes for one tick (the page's panes, layout and art follow in cbTick).
void tickBody(Editor& e) {
    const uint32_t page = e.page;
    const K2MenuUiApi* ui = g_menu.p;
    const int n = hse::memberCount();

    // 1. Button clicks. Every button's release-edge latch is read each tick (a click on a button that does nothing right now must not fire later).
    bool clk[hse::kPageItemsV3] = {};
    for (int i = 0; i < hse::kPageItemsV3; ++i)
        if (itemIsButton(i) && e.item[i] >= 0 && ui->Clicked(page, e.item[i])) clk[i] = true;
    if ((e.state == LIVE || e.state == FAILED) && n > 0) {
        // Member picker and view buttons (work in the FAILED state too; a member switch ends the weapon edit first)
        if (clk[hse::PI_MEMBER_PREV] || clk[hse::PI_MEMBER_NEXT]) {
            const bool next = clk[hse::PI_MEMBER_NEXT];
            if (e.mode == MODE_WEAPON) {
                if (!guardTransition(e, next ? ACT_NEXT : ACT_PREV, "member")) {
                    resetInput(e, "discard_guard");
                    return;
                }
                exitEdit(e);
            }
            e.member = hse::wrapIndex(e.member, next ? 1 : -1, n);
            KP_LOG(chView, "PICK member=%d/%d", e.member + 1, n);
            if (e.token && g_p3.ok()) g_p3.p->Destroy(e.token);
            e.state = NEEDBUILD;   // the stance UI is reset in NEEDBUILD, before the rebuild
            return;
        }
        if (clk[hse::PI_VIEW_RESET]) {
            e.view = e.cfgView;
            KP_LOG(chView, "BTN Reset view=(%.2f,%.2f,%.2f)", e.view.yaw, e.view.zoom, e.view.panZ);
        } else if (clk[hse::PI_VIEW_FLIP]) {
            e.view.yaw = hse::flipYaw(e.view.yaw);
            KP_LOG(chView, "BTN Flip view=(%.2f,%.2f,%.2f)", e.view.yaw, e.view.zoom, e.view.panZ);
        }

        if (e.state == LIVE) {
            // Stances (a preview animation; independent of the edit)
            for (int i = 0; i < hse::kV3Stances; ++i) {
                if (!clk[hse::PI_STANCE + i] || i == e.stanceIdx) continue;
                e.stanceIdx = i;
                e.paused = false;   // PlayAnim always starts running and clears the provider pause
                if (e.token && g_p3.p) g_p3.p->PlayAnim(e.token, kStanceList[i].anim);
                KP_LOG(chView, "STANCE sel=%s anim=%s", kStanceList[i].label, kStanceList[i].anim);
                refreshSelectors(e, true);
                updateStatusText(e);
            }
            // Pause / Play: freeze the animation at the current pose (needs the provider's optional SetAnimPaused slot)
            if (clk[hse::PI_PAUSE]) {
                if (e.token && g_p3.canPause()) {
                    e.paused = !e.paused;
                    g_p3.p->SetAnimPaused(e.token, e.paused ? 1 : 0);
                    KP_LOG(chView, "PAUSE %s ok=%d", e.paused ? "on" : "off", 1);
                    refreshSelectors(e, true);
                } else if (warnOk(e, W_PAUSE)) {
                    KP_LOGV(chView, KPLOG_WARN, "PAUSE unavailable (provider too old)");
                }
            }
            // Draw / Holster: the preview weapons slide to the hand (and the arm reaches) or back to the mount (holster 0.8.0-dev SetPreviewDraw)
            if (clk[hse::PI_DRAW] && g_hol.canDraw()) {
                e.drawn = !e.drawn;
                (void)g_hol.h->SetPreviewDraw(e.drawn ? 1 : 0);
                KP_LOG(chView, "DRAW %s", e.drawn ? "draw" : "holster");
                refreshSelectors(e, true);
            }
            // Weapon buttons: a click on the other hand switches the edit (after the unsaved-edits guard)
            for (int h = 0; h < hse::kV3Weapons; ++h) {
                if (!clk[hse::PI_WEAPON + h] || !e.regGob[h] || (e.mode == MODE_WEAPON && e.editHand == h)) continue;
                if (guardTransition(e, ACT_HAND_R + h, "hand")) enterEdit(e, h);
            }
            if (e.mode == MODE_WEAPON && g_hol.ok()) {
                for (int i = 0; i < hse::kV3Bones && e.mode == MODE_WEAPON; ++i) {
                    if (!clk[hse::PI_BONE + i]) continue;
                    if (refreshEdit(e) && hse::boneIndexOf(e.editInfo.bone) == i) continue;   // already the mounting bone
                    // No unsaved-edits guard: CycleBone only changes the bone of the open edit (HolsterPreview.h overlayCycleBone), the unsaved offsets and
                    // rotation stay, so nothing is discarded (0.3.2 never guarded the bone either). A guard here showed "click again to discard" and
                    // logged DISCARD for a change that discards nothing.
                    if (selectBone(e, i)) {
                        refreshSelectors(e, true);
                        updateStatusText(e);
                    }
                }
                for (int i = 0; i < hse::kV3Scopes && e.mode == MODE_WEAPON; ++i) {
                    if (!clk[hse::PI_SCOPE + i] || i == e.editScope) continue;
                    if (!guardTransition(e, ACT_SCOPE0 + i, "scope")) continue;
                    closeRemoveMenu(e, "scope");
                    e.editScope = i;
                    const int sc = g_hol.h->SetScope(e.editScope, e.editHand == 1 ? 1 : 0);
                    if (!sc || !refreshEdit(e)) {
                        KP_LOG(chView, "EDIT lost after SetScope scope=%d rc=%d", e.editScope, sc);
                        exitEdit(e);
                    } else {
                        resetInput(e, "scope");
                        captureOpen(e, "scope");   // the per-row reset after a scope switch goes back to the NEW scope's open value
                        KP_LOG(chView, "BTN SetScope -> %d (%s)", e.editScope, hse::scopeLabel(e.editScope));
                        refreshSelectors(e, true);
                        refreshNote(e);
                        updateStatusText(e);
                    }
                }
            }
            if (e.mode == MODE_WEAPON && g_hol.ok()) {
                if (clk[hse::PI_SAVE]) {
                    int rc = g_hol.h->Save();
                    KP_LOG(chView, "BTN Save rc=%d", rc);
                    if (rc) captureOpen(e, "save");   // what was written is the new reset target
                    refreshEdit(e);
                    updateStatusText(e);
                    if (!rc) setItemText(e, hse::PI_STATUS, "Save failed, not written (see log)");
                } else if (clk[hse::PI_REVERT]) {
                    g_hol.h->Revert();
                    KP_LOG(chView, "BTN Revert");
                    captureOpen(e, "revert");
                    refreshEdit(e);
                    refreshSelectors(e, true);
                    updateStatusText(e);
                } else if (clk[hse::PI_RESET_SCOPE]) {
                    toggleRemoveMenu(e);   // the "..." button only opens / closes the menu; nothing is deleted here
                } else if (clk[hse::PI_REMOVE] && e.removeMenu.open) {
                    const int scope = e.editScope;
                    const bool wasArmed = removeArmed(e);
                    if (guardConfirm(e, ACT_RESET_SCOPE, "remove_scope")) {
                        g_hol.h->ResetScope();   // deletes this scope's saved keys and saves; the open edit stays open
                        KP_LOG(chView, "REMOVE done scope=%d", scope);
                        KP_LOG(chView, "BTN ResetScope");
                        captureOpen(e, "remove_scope");
                        refreshEdit(e);
                        refreshSelectors(e, true);
                        updateStatusText(e);
                        closeRemoveMenu(e, "done");
                    } else if (!wasArmed) {
                        KP_LOG(chView, "REMOVE armed scope=%d", scope);
                    }
                }
                bool didReset = false;
                for (int g = 0; g < hse::kV3Groups && !didReset; ++g)
                    if (clk[hse::PI_GROUP + g * 2 + 1]) { inspResetGroup(e, g); didReset = true; }
                for (int r = 0; r < hse::IR_COUNT && !didReset && e.mode == MODE_WEAPON; ++r)
                    if (clk[hse::PI_ROW + r * hse::kRowStride + 4]) { inspReset(e, r); didReset = true; }
            }
            for (int i = 0; i < kModeCount; ++i) {   // the mode bar picks what RMB + LMB does; it works with or without an open edit
                if (!clk[hse::PI_MODE + i]) continue;
                setWeaponMode(e, i, "click");
            }
        }
    }

    // 2. The unsaved-edits confirm window ran out: back to the plain status line
    if (e.discardArmed && !hse::guardArmLive(true, e.discardArmedAt, GetTickCount(), kDiscardWindowMs)) {
        e.discardArmed = false;
        updateStatusText(e);
    }

    // 3. Mouse input: hse::Chord (weaponMouse). The scrub starts from a dead press on a slider; with no open edit the chord only orbits, spins and zooms.
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

    {
        const bool lEdge = lmb && !e.pressed;   // weaponMouse updates the "was down" flags
        const hse::ChordOut co = weaponMouse(e, curX, curY, insideView, fg, shift, ctrl, lmb, rmb, mmb, w);
        speedTick(e, curX, curY, fg, lmb, lEdge);   // every mode: the slider sits outside the 3D view, so its press is never a view gesture
        if (e.mode == MODE_WEAPON) scrubTick(e, co, lEdge, curX, curY, fg, lmb, rmb, mmb, shift, ctrl);
        if (e.mode == MODE_WEAPON) refreshInspector(e, false);
        syncRemoveMenu(e);
        logScopeIfChanged(e);
        refreshRemoveButton(e);
        refreshSelectors(e, false);
        refreshHints(e, curX, curY);
    }

    // 4. State machine
    if (e.state == NEEDBUILD) {
        if (n <= 0) {
            setItemText(e, hse::PI_STATUS, "no party");
            return;
        }
        e.member = hse::wrapIndex(e.member, 0, n);
        uintptr_t C = hse::memberAt(e.member);
        resetStanceUi(e);
        g_p3.p->Bind(e.token, reinterpret_cast<void*>(C));
        int built = g_p3.p->Rebuild(e.token);
        if (!built) {
            setItemText(e, hse::PI_STATUS, "preview rebuild failed");
            KP_LOG(chView, "REBUILD fail member=%d", e.member + 1);
            e.state = FAILED;
            return;
        }
        e.paused = false;   // the rebuild above cleared the provider pause
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
        char labelFit[64];
        fitButton(labelPx(e, e.lay.memberName), label, labelFit, sizeof labelFit);
        setItemText(e, hse::PI_MEMBER_NAME, labelFit);
        KP_LOG(chView, "NAME member=%d text=%s read=%d", e.member + 1, label, gotName ? 1 : 0);

        readWeaponModel(e.regGob[0], e.wpnName[0], sizeof(e.wpnName[0]));
        readWeaponModel(e.regGob[1], e.wpnName[1], sizeof(e.wpnName[1]));
        probeItemNames(e, C);   // fills e.itemName (cached for this build; GetName is never called per frame)

        char rBuf[96], lBuf[96];
        handCaption(e, 0, buttonPx(e, e.lay.weapon[0]), rBuf, sizeof rBuf);
        handCaption(e, 1, buttonPx(e, e.lay.weapon[1]), lBuf, sizeof lBuf);
        KP_LOG(chView, "HANDS R=\"%s\" (name=\"%s\" resref=%s) L=\"%s\" (name=\"%s\" resref=%s)", rBuf, e.itemName[0], e.wpnName[0][0] ? e.wpnName[0] : "-", lBuf,
               e.itemName[1], e.wpnName[1][0] ? e.wpnName[1] : "-");

        // Editing is always on: the right weapon opens first (else the left); a member with no weapon shows the empty inspector.
        e.mode = MODE_NAV;
        e.editHand = -1;
        showNoEdit(e);
        const int first = e.regGob[0] ? 0 : (e.regGob[1] ? 1 : -1);
        if (first >= 0) {
            enterEdit(e, first);
        } else {
            refreshSelectors(e, true);
            refreshNote(e);
            updateStatusText(e);
        }

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
                if (g_logVerbose) {   // the repeated CAMHOOK / PROJ lines are verbose only; the CAMENG and CAM2 captures below keep their own caps
                    logCamHook(e, "settled");
                    if (e.mode == MODE_WEAPON) logEditProj(e);
                }
                ++e.projLogs;
                logCamEng(e, "settled");
                logCam2(e, "settled");
            }
        }
    }
}

void KPATCH_CALL cbTick(uint32_t, void*) {
    g_clients.poll();   // p3 + holster retry here (every 256th call until connected); config / menu are already connected, so theirs are no-ops
    if (!g_ed.panel || !g_menu.ok() || !g_ed.page || g_ed.refused) return;
    saferead::beginScope();
    embtex::heal();   // the slider / reset art textures survive a GL reset

    if (!g_p3.ok() || !g_hol.ok() || !g_ed.token) {
        if (g_ed.chord.busy() || g_ed.gActive || g_ed.scrub.active) resetInput(g_ed, "provider_lost");
        setItemText(g_ed, hse::PI_STATUS, statusText());
        return;
    }
    tickBody(g_ed);
    // Every tick, whatever the state: the panes, the layout with SetRect on every item (the thumbs follow the values), the art once the textures exist.
    refreshPanes(g_ed);
    applyLayout(g_ed);
    applyArt(g_ed);
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
    // mod-config frees these labels after this callback; their border dtor deletes whatever fill they hold, so take our pooled images back first.
    int detached = 0;
    for (int i = 0; i < e.artCtlCount; ++i) detached += guikit::detachPooledFills(e.artCtl[i]);
    KP_LOG(chMain, "DTOR art detached=%d of %d ctls", detached, e.artCtlCount);
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

void regLog(int level, const char* msg) { KP_LOGV(chMain, level, "%s", msg); }

}  // namespace

// Called from Holster.cpp KPatchInit, after KpLog_Init and after k2.holster@1 is provided (g_hol connects to this same DLL). No file I/O here.
void holsterEditorInit(const KPatchApi* api) {
    g_clients.init(api);
    embedgui::g_log = embedLog;
    embtex::g_log = embedLog;
    KP_LOG(chMain, "editor page loaded (holster 0.22.0, hookless mod-config consumer): table=%s layout=%u bytes", k2addr::get() ? "ok" : "none",
           static_cast<unsigned>(sizeof kHseViewGui));
    g_clients.poll();   // the first poll() of every client tries to connect
    // This DLL is hookless: there is no frame hook to retry mod-config / menuui from later, so a missing provider now means no page this run.
    if (!g_cfgClient.ok() || !g_menu.ok()) {
        KP_LOG(chMain, "REGISTER fail why=noapi modconfig=%d menuui=%d", g_cfgClient.ok() ? 1 : 0, g_menu.ok() ? 1 : 0);
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
    k2pc::registerOnce(g_reg, g_cfgClient, g_entry, regLog);   // logs "page REGISTER ok token=N" or "page REGISTER refused ..."
}

