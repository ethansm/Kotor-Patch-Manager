// Holster.cpp -- holster 0.16.0-dev (Steam build; 0.13.0-dev added hook H4b HolsterAnimStyle = REINSTALL, so this stays a REINSTALL): P0 probe for the holstered-weapons mod (patch_manager_mods/42_holstered_weapons.md).
// Plan: .claude/plans/holstered_weapons.md. Handlers are plain extern "C" void __cdecl functions; the detour wrappers in
// kotor2-steam-aspyr.hooks.toml call them at function entry with the game arguments BEFORE the stolen bytes run.
// Hard rules for P0 (exceptions: the bone_test code patch, and HolsterPlace's 28-byte pose write with probe=1 bone_test=1 place=1, both default off; and the 0.13.0-dev unarmed_anim
// style byte, ONE byte in the resolver's stack frame written by HolsterAnimStyle, default on but inert until a holster set exists): no other writes to game memory or registers, no engine calls, reads only through saferead, and every handler returns at
// once while its channel is off (holster.ini: probe=1 AND ch_<name>=1; the one deliberate state-only exception is the hand record, which runs with probe=1 alone). All channels share one line budget (max_lines).
//
// Gates this probe answers (doc 42):
//   G2  ATTACH lines: obj/tag/creature plus creature+0x68 (body object) and the hand handles +0x278/+0x27C before and after
//   G3  COMB lines: does 0x7682B0 fire for NPCs and party members alike, old/new state per client creature
//   G4  ANIM lines: animation ids resolved/sent around combat entry (0x2712/0x2713 = suspected wield-transition ids)
//   G5  SWAP lines: the swap executor entry and the first dwords of its object
//   G1  GOB lines: per-update samples of the Gob that re-copies the attach-node transform (+0xA4 vec3 / +0xB0 quat)
// Pure logic (classify, mounts, state machine) lives in HolsterLogic.h and is not used by the probe yet.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "SafeRead.h"
#include "KpLog.h"
#include "K2Addr.h"
#include "HolsterLogic.h"
#include "HolsterBonePatch.h"
#include "HolsterQuat.h"
#include "HolsterHand.h"
#include "HolsterTune.h"
#include "HolsterLogRate.h"
#include "HolsterDrawGate.h"
#include "HolsterSlide.h"
#include "HolsterReach.h"
#include "HolsterAnimStyle.h"
#include "HolsterToggle.h"
#include "HolsterDelay.h"
#include "HolsterMove.h"
#include "Party.h"
#include "HolsterPreview.h"
#include "HolsterApi.h"
#include "FileWatch.h"
#include <stddef.h>
#include <string>

KP_LOG_PATCH("holster");

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chComb, "comb");
KP_CHANNEL(chAttach, "attach");
KP_CHANNEL(chSwap, "swap");
KP_CHANNEL(chAnim, "anim");
KP_CHANNEL(chGob, "gob");
KP_CHANNEL(chPlace, "place");
KP_CHANNEL(chToggle, "toggle");   // 0.3.0-dev: TOGGLE / DELAY / HOSTILE lines (toggle key + shorter combat exit), probe=1 AND ch_toggle=1
KP_CHANNEL(chEdit, "edit");   // k2.holster editor API (holster-editor patch): own line cap kEditLines, not max_lines

// Fallback sink for an old core or a failed KpLog_Init. Stores the path and level only.
const bool g_fallbackSet = (KpLog_SetFallbackFile("holster_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// Config. Written once in KPatchInit (before the handlers do anything); defaults are silent.
static bool g_probe = false;
static bool g_onComb = false, g_onAttach = false, g_onSwap = false, g_onAnim = false, g_onGob = false, g_onPlace = false;   // probe=1 AND ch_<name>=1 (g_onPlace gates PLACE logging only)
// bone_test=1 (needs probe=1); bone_test_left defaults to 1 (the lhand attach node is repointed like the rhand, so a left weapon is holstered
// and mirrored); bone_test_left=0 is the escape hatch that leaves the lhand unpatched. g_boneLeftCfg is the ini value alone (CONFIG log).
static bool g_boneTest = false, g_boneLeft = false, g_boneLeftCfg = true;
// place_mode (0.2.0-dev, HW-UX3 C1): drawgate (default) = no rhand/lhand repoint, HolsterPlace writes the hip pose only while the owner is not drawn
// (HolsterDrawGate.h); repoint = the old global attach-node repoint (escape hatch, old behaviour unchanged). Needs probe=1 and bone_test=1 either way.
static hs::PlaceMode g_placeMode = hs::PlaceDrawgate;
static inline bool drawgateMode() { return g_placeMode == hs::PlaceDrawgate; }
// flourish_draw (0.5.0-dev, HW13): 1 = a party member's weapon stays in the hand while its flourish window is open (flourishOpenFor, per owner).
static bool g_flourishDraw = true;
// draw_slide_ms / stow_slide_ms (0.5.1-dev, HW14): a weapon eases between the hip and the hand over this many ms (HolsterSlide.h Slide). 0 = instant for that
// direction; 0 and 0 = the 0.5.0 snap exactly. Clamped by clampDrawSlideMs (0..570, the vanilla draw window) / clampStowSlideMs (0..1000).
// 0.15.0-dev (HW28): each key also has a per-category form (draw_slide_ms_<cat>, stow_slide_ms_<cat>, reach_grab_pct_<cat>); g_slideTune holds the parsed keys, and
// hs::slideResolve(g_slideTune, cat) gives the weapon's own draw ms / stow ms / grab percent (category key > global key > built-in per-category default).
static hs::SlideTune g_slideTune;
// move_hold_ms (0.6.0-dev): no DELAY cut while the creature is moving (a walk / run animation resolved for it, HolsterMove.h MoveTable) and for this many ms after
// it stops; 0 = off (the 0.5.x behaviour). party_combat_hold (0.6.0-dev): 1 = while ANY party member's client combat flag is set, every party member's weapons stay drawn.
static uint32_t g_moveHoldMs = 1500;
static bool g_partyCombatHold = true;
// unarmed_anim (0.13.0-dev, HW16, hook H4b): 1 = a holstered party member (weapon on the hip, not in combat) resolves its walk / run clips (rifle / heavy / dual-wield rows)
// as unarmed, so it no longer runs around holding an invisible weapon. 0 = off (the 0.11 behaviour). Read once at boot; needs probe=1, bone_test=1 and place_mode=drawgate
// (the holster set is filled by the drawgate placement), otherwise the handler never finds an owner and changes nothing.
static bool g_unarmedAnim = true;
// reach_test (0.7.0-dev, HW19): 1 = bend the owner's RIGHT arm toward the weapon while it slides between the hip and the hand (probe; needs probe=1, bone_test=1,
// place_mode=drawgate and a non-zero slide). Default 0 = no arm writes.
static bool g_reachTest = false;
// reach_carry (0.9.0-dev): 1 = the weapon rides the real hand bone while it slides (the arm swings between the mount and the animation's hand and the blade goes with it,
// spinning with the wrist); 0 = the weapon slides on its own line and the arm chases it (0.7.x). Only with reach_test=1.
static bool g_reachCarry = true;
// reach_hinge (0.11.0-dev, HW23): 1 = a back mount is reached by the hinged path (L-shaped ready pose, the whole arm pitches up with the elbow locked at 90 deg, the upper arm
// parks forward while the forearm closes over the shoulder onto the mount; the carry is the exact reverse); 0 = the bowed line of 0.10.0-dev. Hip mounts keep the bowed line.
static bool g_reachHinge = true;
// reach_grab_pct (0.9.1-dev): share of a carried slide spent reaching the mount before the weapon leaves it (draw) / releasing it after (stow). 0..60, default 30
// (0.15.0-dev: per category, see g_slideTune; the runtime value is the fraction grabFor(cat)).
static inline float grabFor(hs::Category c) { return static_cast<float>(hs::grabPctFor(g_slideTune, c)) / 100.f; }
// reach_wrist_pct / reach_wrist_max_deg (0.10.0-dev): the hand bone is turned from the weapon's mount orientation (at the grab) to its resting orientation, so the weapon
// swings with the wrist. pct 0..100 scales the turn (0 = no wrist), max_deg 0..180 limits it (a real wrist twists far less than the weapon may need).
static float g_reachWristPct = 1.0f, g_reachWristMax = 120.f;   // 0.15.0-dev (HW26): max 90 -> 120
// reach_resid_ms (0.15.0-dev, HW26): a carried draw starts with the weapon at its mount orientation and position in the HAND frame; that offset unwinds over this many ms
// (and a holster lands on the mount the same way) instead of the 15% world blend of the carry weight. 0 = the old world blend. Default hs::kReachResidMs.
static uint32_t g_reachResidMs = hs::kReachResidMs;
// reach_hinge_out_<cat>=outT,outPole (0.15.0-dev, HW27): the hinged back reach's outward hand offset (forearm lengths) and outward share of the parked pole, per category.
static float g_hingeOutT[hs::CatCount], g_hingeOutPole[hs::CatCount];   // defaults set with g_reachStyle (ReachStyleInit)
static hs::ReachStyle g_reachStyle[hs::CatCount];   // elbow bend per category (right arm, body frame); defaults from hs::reachDefaultStyle, reach_pole_<hip|over>_<cat> in holster.ini
// Toggle-key draw hold (0.3.0-dev, HolsterDrawGate.h holdActive): the owner (client creature) whose weapons stay in the hand until g_holdUntil (GetTickCount ms,
// wrap-safe). Written by the tick (g_holdUntil first, then g_holdOwner), read by HolsterPlace. 0 owner = no hold.
static volatile uintptr_t g_holdOwner = 0;
static volatile uint32_t g_holdUntil = 0;
// Toggle key + shorter combat exit config (0.3.0-dev). Written once in readConfig; inert unless bone_test=1 AND place_mode=drawgate.
static bool g_onToggle = false;                 // probe=1 AND ch_toggle=1: TOGGLE / DELAY / HOSTILE log lines
static int g_toggleKey = 0x77;                  // toggle_key VK code (0x77 = F8); 0 = off
static uint32_t g_drawHoldMs = 8000;            // draw_hold_ms; 0 = hold until the next press
static hs::DelayCfg g_delayCfg;                 // combat_exit_ms / flourish_exit_ms (0 = off)
static float g_hostileRange = 30.0f;            // hostile_range (hull-gap metres)
static uint32_t g_hostileBackoffMs = 1000;      // hostile_backoff_ms
static bool g_delayParty = true;                // delay_party: 1 = every party member, 0 = controlled creature only
// Config warnings found before KpLog_Init (readConfig runs first): flushed once by KPatchInit.
static char g_cfgWarn[8][120];
static int g_cfgWarnN = 0;
static void cfgWarn(const char* key, const char* val, const char* why) {
    if (g_cfgWarnN < 8) snprintf(g_cfgWarn[g_cfgWarnN++], sizeof g_cfgWarn[0], "ini %.40s=%.40s ignored: %s", key, val, why);
}
static bool g_editor = true;            // editor=0: RegisterPreview refuses and the preview fast path never runs (no probe needed)
static char g_boneName[32] = "pelvis_g";              // attach node name; its ADDRESS becomes the push immediate, so it must stay static
static LONG g_maxLines = 3000;
static bool g_logVerbose = false;       // log_verbose=1: the old per-sample COMB/ATTACH/ANIMRES/PLACE/PLACEREFUSE volume; 0 = rate-limited (HolsterLogRate.h)
static hs::KeyCounts<256> g_combKeys, g_attachKeys, g_gateKeys, g_gateWarnKeys;   // quiet mode: per-creature hit counts (COMB, ATTACH)
static hs::PlaceTable g_placeTab;
static hs::RefuseTable g_refuseTab;
static int g_gobEvery = 600;            // GOB lines: first 24 updates, then every Nth (the handler runs per Gob per frame)
static uint32_t g_placeGobvt = 0;       // place_gobvt: required Gob vtable (0 = no check); boot-time only
static volatile LONG g_lines = 0;
static volatile LONG g_gobCalls = 0;
static volatile LONG g_placeCalls = 0;   // PLACE lines: first 24 placements, then every gob_every-th
static volatile LONG g_animPerId[0x100] = {};   // ANIMRES lines per id 0x27xx: ids resolve every frame (run 2: 0x2711 x3976, 0x2718 x1900), cap each at 20

// saferead site tags (failure ring only).
static const unsigned kSiteComb = 0xE70010u, kSiteAttach = 0xE70020u, kSiteSwap = 0xE70030u, kSiteAnim = 0xE70040u, kSiteGob = 0xE70050u, kSitePlace = 0xE70060u, kSiteTick = 0xE70070u;

static inline unsigned U(const void* p) { return static_cast<unsigned>(reinterpret_cast<uintptr_t>(p)); }

static bool takeLine() {
    LONG n = InterlockedIncrement(&g_lines);
    if (n <= g_maxLines) return true;
    if (n == g_maxLines + 1) KP_LOG(chMain, "cap reached max_lines=%ld: further lines dropped", static_cast<long>(g_maxLines));
    return false;
}

// EDIT lines have their own budget, so a long probe session cannot starve the editor log (and the reverse).
static const LONG kEditLines = 200;
static volatile LONG g_editLines = 0;
static bool takeEditLine() {
    if (!KP_ENABLED(chEdit, KPLOG_INFO)) return false;
    const LONG n = InterlockedIncrement(&g_editLines);
    if (n <= kEditLines) return true;
    if (n == kEditLines + 1) KP_LOG(chEdit, "cap reached edit lines=%ld: further EDIT lines dropped", static_cast<long>(kEditLines));
    return false;
}

static char* trimIni(char* s) {   // inikit::trim works in place (it also strips \v and \f), so the pointer stays valid
    inikit::trim(s);
    return s;
}

// Strict on: only 1 / on / true / yes switch a key on. Anything else (including a blank value) is off.
static bool flagValue(const char* v) {
    return !_stricmp(v, "1") || !_stricmp(v, "on") || !_stricmp(v, "true") || !_stricmp(v, "yes");
}

static_assert(sizeof(hs::Pose) == 28, "Gob+0xA4..+0xBF is {pos[3], q.w,q.x,q.y,q.z}");

// Live-tunable placement (place, place_rot, place_off). g_tuneW is the writer's master copy; g_tunePub is the reader copy published
// under a sequence counter (odd = write in progress). Readers retry up to 3 times and otherwise keep their previous local copy.
static hs::TuneCfg g_tuneW, g_tunePub;
static volatile LONG g_tuneSeq = 0;
static filewatch::ReloadPoll g_poll;
static volatile LONG g_reloadBusy = 0;

static void publishTune(const hs::TuneCfg& t) {
    g_tuneW = t;
    InterlockedIncrement(&g_tuneSeq);   // odd
    MemoryBarrier();
    memcpy(static_cast<void*>(&g_tunePub), &t, sizeof t);
    MemoryBarrier();
    InterlockedIncrement(&g_tuneSeq);   // even
}

// True with dst = the published config (and *seqOut = its sequence); false after 3 torn reads, dst unchanged.
static bool loadTune(hs::TuneCfg& dst, LONG* seqOut = nullptr) {
    for (int i = 0; i < 3; ++i) {
        const LONG s1 = g_tuneSeq;
        MemoryBarrier();
        if (s1 & 1) continue;
        hs::TuneCfg tmp;
        memcpy(&tmp, static_cast<const void*>(&g_tunePub), sizeof tmp);
        MemoryBarrier();
        if (g_tuneSeq == s1) {
            dst = tmp;
            if (seqOut) *seqOut = s1;
            return true;
        }
    }
    return false;
}

// Per-frame reader: copies the 4 KiB config only when the sequence moved. lastSeq advances only after a successful copy, so a torn
// read is retried next frame. lastSeq starts at 0; readConfig's publishTune leaves the sequence at 2 before any hook runs.
static bool loadTuneIfChanged(hs::TuneCfg& dst, LONG& lastSeq) {
    if (g_tuneSeq == lastSeq) return false;
    LONG got = 0;
    if (!loadTune(dst, &got)) return false;
    lastSeq = got;
    return true;
}

// The tune keys of an ini text applied onto t (the caller seeds t): every override is cleared first, so a deleted key falls back
// to place_rot / place_off. maybeReload and the editor Save both use it, so a saved file parses exactly like a reloaded one.
static void parseTuneText(const char* text, hs::TuneCfg& t) {
    hs::clearCatTune(t);
    const char* s = text;
    while (s && *s) {
        const char* e = s;
        while (*e && *e != '\n') ++e;
        char line[256];
        size_t n = static_cast<size_t>(e - s);
        if (n >= sizeof line) n = sizeof line - 1;   // longer lines are cut, like fgets with a 256-byte buffer would split them
        memcpy(line, s, n);
        line[n] = 0;
        s = *e ? e + 1 : e;
        char* p = trimIni(line);
        if (*p == 0 || *p == ';' || *p == '#' || *p == '[') continue;
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = 0;
        char* key = trimIni(p);
        char* val = trimIni(eq + 1);
        if (hs::isTuneKey(key) && !hs::applyTuneKey(t, key, val) && hs::isLeftExtraKey(key))
            KP_LOGV(chMain, KPLOG_WARN, "ini %.40s=%.40s ignored: need one number within +-%.0f (known suffixes: lightsaber melee qstaff pistol rifle heavy)", key, val,
                    static_cast<double>(hs::kLeftExtraParseMax));
    }
}

// WARN for every left_extra_rot value that the clamp changes (the raw value stays in the ini; resolveFull uses +-kLeftExtraMax).
static void warnLeftExtraClamp(const hs::TuneCfg& t, const char* where) {
    if (hs::leftExtraClamped(t.leftExtra))
        KP_LOGV(chMain, KPLOG_WARN, "%s left_extra_rot=%.1f outside +-%.0f: clamped to %.1f", where, static_cast<double>(t.leftExtra),
                static_cast<double>(hs::kLeftExtraMax), static_cast<double>(hs::leftExtraFor(t, hs::CatUnknown)));
    for (int i = hs::CatUnknown + 1; i < hs::CatCount; ++i)
        if (t.hasLeftExtraCat[i] && hs::leftExtraClamped(t.leftExtraCat[i]))
            KP_LOGV(chMain, KPLOG_WARN, "%s left_extra_rot_%s=%.1f outside +-%.0f: clamped to %.1f", where, hs::categoryName(static_cast<hs::Category>(i)),
                    static_cast<double>(t.leftExtraCat[i]), static_cast<double>(hs::kLeftExtraMax), static_cast<double>(hs::leftExtraFor(t, static_cast<hs::Category>(i))));
}

// Whole file into out. False when it cannot be opened (out cleared).
static bool readFileText(const char* path, std::string& out) {
    out.clear();
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) out.append(buf, n);
    fclose(f);
    return true;
}

// HW28: draw_slide_ms, stow_slide_ms and reach_grab_pct, each with an optional _<category> suffix (lightsaber melee qstaff pistol rifle heavy). A key without a suffix is the
// global; hs::slideResolve picks category > global > built-in default. Every value is clamped at set time and a clamp that bites is a cfgWarn (the old silent 900 -> 570).
// True when `key` is one of these keys (value applied or warned); false leaves it to the other branches.
static bool parseSlideKey(const char* key, const char* val) {
    static const char* const kName[3] = {"draw_slide_ms", "stow_slide_ms", "reach_grab_pct"};
    for (int k = 0; k < 3; ++k) {
        const size_t n = strlen(kName[k]);
        if (_strnicmp(key, kName[k], n) != 0 || (key[n] != 0 && key[n] != '_')) continue;
        hs::Category cat = hs::CatUnknown;
        if (key[n] == '_') {
            int ci = -1;
            for (int i = hs::CatUnknown + 1; i < hs::CatCount; ++i)
                if (!_stricmp(key + n + 1, hs::categoryName(static_cast<hs::Category>(i)))) ci = i;
            if (ci < 0) { cfgWarn(key, val, "unknown category suffix (lightsaber melee qstaff pistol rifle heavy)"); return true; }
            cat = static_cast<hs::Category>(ci);
        }
        char* end = nullptr;
        const long v = strtol(val, &end, 10);
        if (end == val || *end != 0) { cfgWarn(key, val, "not a number, keeping the default"); return true; }
        bool clamped;
        if (k == 0) clamped = hs::slideSetDraw(g_slideTune, cat, v);
        else if (k == 1) clamped = hs::slideSetStow(g_slideTune, cat, v);
        else clamped = hs::slideSetGrab(g_slideTune, cat, v);
        if (clamped) cfgWarn(key, val, k == 0 ? "out of range: used clamped to 0..570" : (k == 1 ? "out of range: used clamped to 0..1000" : "out of range: used clamped to 0..60"));
        return true;
    }
    return false;
}

static void readConfig() {
    hs::TuneCfg tune;
    bool probe = false, boneTest = false, boneLeft = true, comb = false, attach = false, swap = false, anim = false, gob = false, place = false, toggle = false;
    FILE* f = fopen("holster.ini", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char* p = trimIni(line);
            if (*p == 0 || *p == ';' || *p == '#' || *p == '[') continue;
            char* eq = strchr(p, '=');
            if (!eq) continue;
            *eq = 0;
            char* key = trimIni(p);
            char* val = trimIni(eq + 1);
            if (!_stricmp(key, "probe")) probe = flagValue(val);
            else if (!_stricmp(key, "bone_test")) boneTest = flagValue(val);
            else if (!_stricmp(key, "place_mode")) {
                hs::PlaceMode pm = g_placeMode;
                if (hs::placeModeParse(val, pm)) g_placeMode = pm;
                else cfgWarn(key, val, "not drawgate or repoint, keeping the default drawgate");
            }
            else if (!_stricmp(key, "bone_test_left")) {
                bool b = true;
                if (hs::tuneParseBool(val, b)) boneLeft = b;
                else cfgWarn(key, val, "not a boolean word, keeping the default 1");
            }
            else if (!_stricmp(key, "bone_name")) {
                // node names are plain identifiers; anything else (or empty) keeps the default
                size_t n = strlen(val);
                bool ok = n > 0 && n < sizeof(g_boneName);
                for (size_t i = 0; ok && i < n; ++i) ok = isalnum(static_cast<unsigned char>(val[i])) || val[i] == '_';
                if (ok) memcpy(g_boneName, val, n + 1);
            }
            else if (!_stricmp(key, "ch_comb")) comb = flagValue(val);
            else if (!_stricmp(key, "ch_attach")) attach = flagValue(val);
            else if (!_stricmp(key, "ch_swap")) swap = flagValue(val);
            else if (!_stricmp(key, "ch_anim")) anim = flagValue(val);
            else if (!_stricmp(key, "ch_gob")) gob = flagValue(val);
            else if (!_stricmp(key, "ch_place")) place = flagValue(val);
            else if (!_stricmp(key, "ch_toggle")) toggle = flagValue(val);
            else if (!_stricmp(key, "toggle_key")) {
                int k = 0;
                if (hs::parseKey(val, &k)) g_toggleKey = k;
                else cfgWarn(key, val, "not a key code 0..0xFE (decimal or 0x hex), keeping the default 0x77");
            }
            else if (!_stricmp(key, "draw_hold_ms")) {
                char* end = nullptr;
                const long v = strtol(val, &end, 10);
                if (end != val && *end == 0) g_drawHoldMs = hs::clampDrawHold(v);
                else cfgWarn(key, val, "not a number, keeping the default 8000");
            }
            else if (!_stricmp(key, "combat_exit_ms") || !_stricmp(key, "flourish_exit_ms")) {
                char* end = nullptr;
                const long v = strtol(val, &end, 10);
                if (end == val || *end != 0) { cfgWarn(key, val, "not a number, keeping the default 1500"); continue; }
                if (!_stricmp(key, "combat_exit_ms")) g_delayCfg.combatExitMs = hs::clampExitMs(v);
                else g_delayCfg.flourishExitMs = hs::clampExitMs(v);
            }
            else if (!_stricmp(key, "hostile_range")) {
                char* end = nullptr;
                const float v = strtof(val, &end);
                if (end != val && *end == 0 && v == v) g_hostileRange = v < 5.0f ? 5.0f : (v > 60.0f ? 60.0f : v);
                else cfgWarn(key, val, "not a number, keeping the default 30.0");
            }
            else if (!_stricmp(key, "hostile_backoff_ms")) {
                char* end = nullptr;
                const long v = strtol(val, &end, 10);
                if (end != val && *end == 0) g_hostileBackoffMs = static_cast<uint32_t>(v < 250 ? 250 : (v > 5000 ? 5000 : v));
                else cfgWarn(key, val, "not a number, keeping the default 1000");
            }
            else if (!_stricmp(key, "flourish_draw")) {
                bool b = true;
                if (hs::tuneParseBool(val, b)) g_flourishDraw = b;
                else cfgWarn(key, val, "not a boolean word, keeping the default 1");
            }
            else if (parseSlideKey(key, val)) {}   // draw_slide_ms[_<cat>], stow_slide_ms[_<cat>], reach_grab_pct[_<cat>] (HW28)
            else if (!_stricmp(key, "move_hold_ms")) {
                char* end = nullptr;
                const long v = strtol(val, &end, 10);
                if (end != val && *end == 0) g_moveHoldMs = static_cast<uint32_t>(v < 0 ? 0 : (v > 10000 ? 10000 : v));
                else cfgWarn(key, val, "not a number, keeping the default 1500");
            }
            else if (!_stricmp(key, "unarmed_anim")) {
                bool b = true;
                if (hs::tuneParseBool(val, b)) g_unarmedAnim = b;
                else cfgWarn(key, val, "not a boolean word, keeping the default 1");
            }
            else if (!_stricmp(key, "party_combat_hold")) {
                bool b = true;
                if (hs::tuneParseBool(val, b)) g_partyCombatHold = b;
                else cfgWarn(key, val, "not a boolean word, keeping the default 1");
            }
            else if (!_stricmp(key, "reach_wrist_pct")) {
                char* end = nullptr;
                const long v = strtol(val, &end, 10);
                if (end != val && *end == 0) g_reachWristPct = static_cast<float>(v < 0 ? 0 : (v > 100 ? 100 : v)) / 100.f;
                else cfgWarn(key, val, "not a number, keeping the default 100");
            }
            else if (!_stricmp(key, "reach_wrist_max_deg")) {
                char* end = nullptr;
                const long v = strtol(val, &end, 10);
                if (end != val && *end == 0) g_reachWristMax = static_cast<float>(v < 0 ? 0 : (v > 180 ? 180 : v));
                else cfgWarn(key, val, "not a number, keeping the default 120");
            }
            else if (!_strnicmp(key, "reach_bulge_", 12)) {   // reach_bulge_<category>=up,forward,outward : how far the hand path bows for a high mount (arm lengths)
                float* dst = nullptr;
                for (int i = 0; i < hs::CatCount; ++i)
                    if (!_stricmp(key + 12, hs::categoryName(static_cast<hs::Category>(i)))) dst = g_reachStyle[i].bulge;
                float v[3];
                char* end = const_cast<char*>(val);
                bool ok = dst != nullptr;
                for (int i = 0; ok && i < 3; ++i) {
                    char* next = nullptr;
                    v[i] = strtof(end, &next);
                    ok = next != end && v[i] == v[i] && v[i] >= -2.f && v[i] <= 2.f && (i == 2 ? *next == 0 : *next == ',');
                    end = next + 1;
                }
                if (ok) { dst[0] = v[0]; dst[1] = v[1]; dst[2] = v[2]; }
                else cfgWarn(key, val, "need <lightsaber|melee|qstaff|pistol|rifle|heavy>=up,forward,outward (each -2..2), keeping the default");
            }
            else if (!_strnicmp(key, "reach_pole_", 11)) {   // reach_pole_<hip|over>_<category>=x,y,z : elbow bend direction, right arm, body frame (x right, y forward, z up)
                const char* k = key + 11;
                const bool hipK = !_strnicmp(k, "hip_", 4), overK = !_strnicmp(k, "over_", 5);
                float* dst = nullptr;
                if (hipK || overK)
                    for (int i = 0; i < hs::CatCount; ++i)
                        if (!_stricmp(k + (hipK ? 4 : 5), hs::categoryName(static_cast<hs::Category>(i)))) dst = hipK ? g_reachStyle[i].hip : g_reachStyle[i].over;
                float v[3];
                char* end = const_cast<char*>(val);
                bool ok = dst != nullptr;
                for (int i = 0; ok && i < 3; ++i) {
                    char* next = nullptr;
                    v[i] = strtof(end, &next);
                    ok = next != end && v[i] == v[i] && (i == 2 ? *next == 0 : *next == ',');
                    end = next + 1;
                }
                if (ok && hs::r3len(v) > 1e-3f) { dst[0] = v[0]; dst[1] = v[1]; dst[2] = v[2]; }
                else cfgWarn(key, val, "need <hip|over>_<lightsaber|melee|qstaff|pistol|rifle|heavy>=x,y,z (non-zero), keeping the default");
            }
            else if (!_stricmp(key, "reach_resid_ms")) {
                char* end = nullptr;
                const long v = strtol(val, &end, 10);
                if (end != val && *end == 0) g_reachResidMs = static_cast<uint32_t>(v < 0 ? 0 : (v > 600 ? 600 : v));
                else cfgWarn(key, val, "not a number, keeping the default 240");
            }
            else if (!_strnicmp(key, "reach_hinge_out_", 16)) {   // reach_hinge_out_<category>=outT,outPole : hinged back reach, hand offset out (forearm lengths) and outward pole share
                int ci = -1;
                for (int i = hs::CatUnknown + 1; i < hs::CatCount; ++i)
                    if (!_stricmp(key + 16, hs::categoryName(static_cast<hs::Category>(i)))) ci = i;
                float v[2];
                char* end = const_cast<char*>(val);
                bool ok = ci >= 0;
                for (int i = 0; ok && i < 2; ++i) {
                    char* next = nullptr;
                    v[i] = strtof(end, &next);
                    ok = next != end && v[i] == v[i] && v[i] >= 0.f && v[i] <= (i == 0 ? 1.f : 2.f) && (i == 1 ? *next == 0 : *next == ',');
                    end = next + 1;
                }
                if (ok) { g_hingeOutT[ci] = v[0]; g_hingeOutPole[ci] = v[1]; }
                else cfgWarn(key, val, "need <lightsaber|melee|qstaff|pistol|rifle|heavy>=outT,outPole (0..1, 0..2), keeping the default");
            }
            else if (!_stricmp(key, "reach_carry")) {
                bool b = true;
                if (hs::tuneParseBool(val, b)) g_reachCarry = b;
                else cfgWarn(key, val, "not a boolean word, keeping the default 1");
            }
            else if (!_stricmp(key, "reach_hinge")) {
                bool b = true;
                if (hs::tuneParseBool(val, b)) g_reachHinge = b;
                else cfgWarn(key, val, "not a boolean word, keeping the default 1");
            }
            else if (!_stricmp(key, "reach_test")) {
                bool b = false;
                if (hs::tuneParseBool(val, b)) g_reachTest = b;
                else cfgWarn(key, val, "not a boolean word, keeping the default 0");
            }
            else if (!_stricmp(key, "delay_party")) {
                bool b = true;
                if (hs::tuneParseBool(val, b)) g_delayParty = b;
                else cfgWarn(key, val, "not a boolean word, keeping the default 1");
            }
            else if (!_stricmp(key, "editor")) g_editor = flagValue(val);
            else if (!_stricmp(key, "log_verbose")) g_logVerbose = flagValue(val);
            else if (!_stricmp(key, "place_gobvt")) {
                char* end = nullptr;
                const unsigned long v = strtoul(val, &end, 16);   // accepts a 0x prefix; blank or junk = no check
                g_placeGobvt = (end == val) ? 0u : static_cast<uint32_t>(v);
            }
            else if (hs::isTuneKey(key)) {
                if (!hs::applyTuneKey(tune, key, val) && hs::isLeftExtraKey(key)) cfgWarn(key, val, "need one number within +-720 and a known category suffix");
            }
            else if (!_stricmp(key, "max_lines") || !_stricmp(key, "gob_every")) {
                char* end = nullptr;
                unsigned long v = strtoul(val, &end, 10);
                if (end == val) continue;
                if (!_stricmp(key, "max_lines") && v >= 1 && v <= 1000000ul) g_maxLines = static_cast<LONG>(v);
                if (!_stricmp(key, "gob_every") && v >= 1 && v <= 1000000ul) g_gobEvery = static_cast<int>(v);
            }
        }
        fclose(f);
    }
    g_probe = probe;
    g_boneTest = probe && boneTest;
    g_boneLeftCfg = boneLeft;
    g_boneLeft = g_boneTest && boneLeft;
    g_onComb = probe && comb;
    g_onAttach = probe && attach;
    g_onSwap = probe && swap;
    g_onAnim = probe && anim;
    g_onGob = probe && gob;
    g_onPlace = probe && place;
    g_onToggle = probe && toggle;
    tune.bootBoneIdx = hs::boneIndexOf(g_boneName);   // 0 = custom bone_name (not one of the five editor bones)
    publishTune(tune);
}

static void addName(char* buf, size_t cap, bool on, const char* name) {
    if (!on) return;
    size_t n = strlen(buf);
    snprintf(buf + n, cap - n, "%s%s", n ? "," : "", name);
}

// Reads a u32 at base+off, 0 when unreadable (the probe prints rd-ok as a separate bit only where it matters).
static uint32_t rdU32(unsigned site, uintptr_t base, int off) {
    uint32_t v = 0;
    (void)saferead::readAt(site, base, off, &v);
    return v;
}
// Like rdU32, but reports whether the read succeeded (*out is 0 when it did not). Only the COMB old-state read uses it.
static bool rdU32Ok(unsigned site, uintptr_t base, int off, uint32_t* out) {
    uint32_t v = 0;
    const bool ok = saferead::readAt(site, base, off, &v);
    *out = v;
    return ok;
}
static float rdF(unsigned site, uintptr_t base, int off) {
    float v = 0.f;
    (void)saferead::readAt(site, base, off, &v);
    return v;
}

// Reads a NUL-terminated printable ASCII tag (at most cap-1 chars). "?" when unreadable, "" when null.
static void readTag(unsigned site, const char* p, char* out, int cap) {
    out[0] = 0;
    if (!p) return;
    int n = 0;
    for (; n < cap - 1; ++n) {
        char c = 0;
        if (!saferead::readAt(site, reinterpret_cast<uintptr_t>(p), n, &c)) { snprintf(out, cap, "?"); return; }
        if (c == 0) break;
        out[n] = (c >= 0x20 && c <= 0x7E) ? c : '.';
    }
    out[n] = 0;
}

// ---- handlers: called from the detour wrappers, before the stolen bytes run. Read-only. ----

// 0x007682B0 client combat flag setter. ECX = client creature, [esp+4] = new state. Logs EVERY call (G3: party only, or NPCs too).
extern "C" void __cdecl HolsterCombat(void* creature, int newState) {
    if (!g_onComb) return;
    saferead::beginScope();
    if (!KP_ENABLED(chComb, KPLOG_INFO)) return;
    const uintptr_t c = reinterpret_cast<uintptr_t>(creature);
    const int off = k2addr::off(&K2AddrTable::Creature_ClientCombat);
    uint32_t raw = 0;
    const bool readOk = off != 0 && rdU32Ok(kSiteComb, c, off, &raw);
    // Quiet mode: every real edge (old != new, or old unreadable) is logged; only the repeats (edge=0) are rate-limited per creature.
    const bool edge = !readOk || static_cast<int>(raw) != newState;
    if (!g_logVerbose) {
        const uint32_t cnt = edge ? 0u : hs::keyBump(g_combKeys, hs::creatureKey(c));
        if (!hs::combLogWanted(edge, cnt, false)) return;
    }
    if (!takeLine()) return;
    const hs::CombFields f = hs::combFields(readOk, raw, newState);
    char oldS[12] = "?", edgeS[4] = "?";
    if (f.oldKnown) { snprintf(oldS, sizeof oldS, "%d", f.old); snprintf(edgeS, sizeof edgeS, "%d", f.edge); }
    KP_LOG(chComb, "COMB creature=%x old=%s new=%d edge=%s tick=%lu", U(creature), oldS, newState, edgeS,
           static_cast<unsigned long>(GetTickCount()));
}

// 0x007637E0 in-world weapon attach. [esp+4] = obj (u32), [esp+8] = tag (char*), [esp+0xC] = creature (int*).
// Logs the creature's body object and the hand handles the call is about to replace.
extern "C" void __cdecl HolsterAttach(uint32_t obj, const char* tag, int* creature) {
    if (!g_onAttach) return;
    saferead::beginScope();
    if (!KP_ENABLED(chAttach, KPLOG_INFO)) return;
    // Quiet mode: first 4 attaches per creature, then every 20th.
    if (!g_logVerbose && !hs::keyedWant(hs::keyBump(g_attachKeys, hs::creatureKey(reinterpret_cast<uintptr_t>(creature))), 4, 20)) return;
    if (!takeLine()) return;
    char t[48];
    readTag(kSiteAttach, tag, t, sizeof(t));
    const uintptr_t c = reinterpret_cast<uintptr_t>(creature);
    const int oBody = k2addr::off(&K2AddrTable::Creature_BodyObj), oR = k2addr::off(&K2AddrTable::Creature_HandleR),
              oL = k2addr::off(&K2AddrTable::Creature_HandleL);
    KP_LOG(chAttach, "ATTACH obj=%x tag='%s' creature=%x body=%x handleR=%x handleL=%x tick=%lu", static_cast<unsigned>(obj), t, U(creature),
           oBody ? static_cast<unsigned>(rdU32(kSiteAttach, c, oBody)) : 0u, oR ? static_cast<unsigned>(rdU32(kSiteAttach, c, oR)) : 0u,
           oL ? static_cast<unsigned>(rdU32(kSiteAttach, c, oL)) : 0u, static_cast<unsigned long>(GetTickCount()));
}

// 0x005DF920 swap executor. ECX = executor object. Dumps its first 8 dwords (G5: which fields carry creature / slots).
extern "C" void __cdecl HolsterSwap(void* exec) {
    if (!g_onSwap) return;
    saferead::beginScope();
    if (!KP_ENABLED(chSwap, KPLOG_INFO) || !takeLine()) return;
    const uintptr_t e = reinterpret_cast<uintptr_t>(exec);
    KP_LOG(chSwap, "SWAP exec=%x d0=%x d1=%x d2=%x d3=%x d4=%x d5=%x d6=%x d7=%x tick=%lu", U(exec), static_cast<unsigned>(rdU32(kSiteSwap, e, 0)),
           static_cast<unsigned>(rdU32(kSiteSwap, e, 4)), static_cast<unsigned>(rdU32(kSiteSwap, e, 8)), static_cast<unsigned>(rdU32(kSiteSwap, e, 12)),
           static_cast<unsigned>(rdU32(kSiteSwap, e, 16)), static_cast<unsigned>(rdU32(kSiteSwap, e, 20)), static_cast<unsigned>(rdU32(kSiteSwap, e, 24)),
           static_cast<unsigned>(rdU32(kSiteSwap, e, 28)), static_cast<unsigned long>(GetTickCount()));
}

// 0x00863650 anim-id resolver (ECX = object, [esp+4] = u16 anim id). (H5 0x005644B0 was removed: A3 showed it is an action enqueue, not an anim sender.)
// Only ids in the hold/ready/wield-transition family 0x2710..0x27FF are logged; the resolver runs for every animation in the game.
// 0.5.2-dev (HW15 probe): also the extra ids 0x283C 0x283D 0x283E 0x2806 0x28BD (outside 0x27xx, so they get their own counters, g_animPerIdX).
static const uint32_t kAnimExtra[5] = {0x283Cu, 0x283Du, 0x283Eu, 0x2806u, 0x28BDu};
static volatile LONG g_animPerIdX[8] = {};   // one slot per kAnimExtra id; the same cap as g_animPerId
static int animExtraSlot(uint32_t id) {
    for (int i = 0; i < 5; ++i) if ((id & 0xFFFFu) == kAnimExtra[i]) return i;
    return -1;
}
static bool animInteresting(uint32_t id) { return ((id & 0xFFFFu) >= 0x2710u && (id & 0xFFFFu) <= 0x27FFu) || animExtraSlot(id) >= 0; }

// Filled by HolsterPlace each placement while the owner's weapon is on the hip; read by the resolver probe (ANIMRES hol=).
// Keyed by the CLIENT creature (owner) with its body object as base. The resolver hook gets the BODY object as `self` (0.16.0-dev; .claude/waves/hr7/r3.md), so
// resolver-side code asks g_holSet.ownerOfBase(self) for the creature and reads the combat flag from that creature, never from the body.
static hs::HolsterSet g_holSet;

static void maybeReload();

// 0.6.0-dev: last locomotion sighting per creature (the resolver runs every frame for every animated creature). Single writer (the game thread), read by the tick.
// 0.16.0-dev: keyed by the creature's BODY object (what the resolver hook receives), not the client creature; the tick looks it up with [creature + Creature_BodyObj].
static hs::MoveTable g_move;

// One ANIMRES line (caller has checked the channel and the budget): style / gate / cc / d4 / e8 bytes of the anim object (`self` = the creature's BODY object)
// plus hol= (the body belongs to a creature whose weapon is on the hip), comb= (that creature's client combat flag, -1 when no owner / unreadable) and own= (the owner).
static void logAnimRes(void* self, uint32_t animId) {
    const uintptr_t s = reinterpret_cast<uintptr_t>(self);
    uint8_t b[5] = {0, 0, 0, 0, 0};
    static const int offs[5] = {0xC5, 0xC6, 0xCC, 0xD4, 0xE8};
    for (int i = 0; i < 5; ++i) (void)saferead::readAt(kSiteAnim, s, offs[i], &b[i]);
    // HW15 probe: hol = the creature that owns this body has its weapon on the hip (HolsterSet, filled by HolsterPlace), comb = that creature's client combat flag
    // (-1: no owner in the set / unreadable / no offset), own = the owner (client creature), 0 when none.
    const unsigned long nowTick = GetTickCount();
    const uintptr_t owner = g_holSet.ownerOfBase(s, static_cast<uint32_t>(nowTick));
    const int hol = owner != 0 ? 1 : 0;
    const int combOff = k2addr::off(&K2AddrTable::Creature_ClientCombat);
    uint32_t combRaw = 0;
    const int comb = (owner != 0 && combOff != 0 && rdU32Ok(kSiteAnim, owner, combOff, &combRaw)) ? static_cast<int>(combRaw) : -1;
    KP_LOG(chAnim, "ANIMRES self=%x id=%x style=%u gate=%u cc=%u d4=%u e8=%u hol=%d comb=%d tick=%lu own=%x", U(self), static_cast<unsigned>(animId & 0xFFFFu), b[0], b[1],
           b[2], b[3], b[4], hol, comb, nowTick, static_cast<unsigned>(owner));
}

// 0.13.0-dev (HW16 verification): the first sighting of (creature, locomotion id 0x2711..0x2714) while its weapon is on the hip is logged as an ANIMRES line even
// after the per-id and first-sighting caps are used up (separate budget kLocoLines), so a play session shows which loco ids a holstered creature still resolves and
// with which style byte (before the H4b override, which acts one step later; the line shows the engine's style, not the overridden one). Cheap path: a counter test,
// then a compare scan of a small table; the holster-set read happens at most every 250 ms per (creature, id) until the hol=1 line is out.
static const LONG kLocoLines = 60;
static const int kLocoSeenCap = 256;
struct LocoSeen { uintptr_t self; uint32_t id; uint32_t next; bool done; };
static LocoSeen s_locoSeen[kLocoSeenCap];
static volatile LONG s_nLocoSeen = 0;
static volatile LONG g_locoLines = 0;
static bool isLocoId(uint32_t id) { const uint32_t i = id & 0xFFFFu; return i >= 0x2711u && i <= 0x2714u; }
static bool locoHolProbe(void* self, uint32_t animId) {
    if (g_locoLines >= kLocoLines) return false;
    const uintptr_t sp = reinterpret_cast<uintptr_t>(self);
    const uint32_t id = animId & 0xFFFFu;
    const uint32_t now = GetTickCount();
    LocoSeen* e = nullptr;
    const LONG n = s_nLocoSeen < kLocoSeenCap ? s_nLocoSeen : kLocoSeenCap;
    for (LONG i = 0; i < n; ++i)
        if (s_locoSeen[i].self == sp && s_locoSeen[i].id == id) { e = &s_locoSeen[i]; break; }
    if (e) {
        if (e->done || static_cast<int32_t>(now - e->next) < 0) return false;
    } else {
        const LONG slot = InterlockedIncrement(&s_nLocoSeen) - 1;
        if (slot >= kLocoSeenCap) return false;
        e = &s_locoSeen[slot];
        e->self = sp; e->id = id; e->done = false;
    }
    e->next = now + 250;
    saferead::beginScope();
    if (g_holSet.ownerOfBase(sp, now) == 0) return false;   // `self` is the BODY object: map it back to the owning creature
    e->done = true;
    if (InterlockedIncrement(&g_locoLines) > kLocoLines || !KP_ENABLED(chAnim, KPLOG_INFO) || !takeLine()) return false;
    logAnimRes(self, animId);
    return true;
}

extern "C" void __cdecl HolsterAnimResolve(void* self, uint32_t animId) {
    if (g_moveHoldMs > 0 && hs::isLocoAnim(animId)) g_move.note(reinterpret_cast<uintptr_t>(self), GetTickCount());   // before every early return; no reads, no logging
    if (g_probe) maybeReload();   // the resolver runs every frame; the 0x5047B0 handler only on attach events (live test 10-15: GOB lines=12)
    if (!g_onAnim || !animInteresting(animId)) return;
    if (isLocoId(animId) && locoHolProbe(self, animId)) return;   // 0.13.0-dev: hol=1 first sighting of a locomotion id (own cap, logged even past the caps below)
    // Quiet mode: 4 lines per id and 32 first-sightings of (creature, id, style); log_verbose=1: 20 and 64.
    const int xSlot = animExtraSlot(animId);
    if (InterlockedIncrement(xSlot >= 0 ? &g_animPerIdX[xSlot] : &g_animPerId[(animId & 0xFFFFu) - 0x2700u]) > (g_logVerbose ? 20 : 4)) {
        // C6: past the per-id cap, still log the first sighting of each (creature, id, style) so every party member's wield style shows.
        struct Seen { uintptr_t self; uint32_t id; uint8_t style; };
        static Seen s_seen[64];
        static volatile LONG s_nSeen = 0;
        uint8_t st = 0;
        saferead::beginScope();
        (void)saferead::readAt(kSiteAnim, reinterpret_cast<uintptr_t>(self), 0xC5, &st);
        const LONG seenCap = g_logVerbose ? 64 : 32;
        const LONG n = s_nSeen < seenCap ? s_nSeen : seenCap;
        for (LONG i = 0; i < n; ++i)
            if (s_seen[i].self == reinterpret_cast<uintptr_t>(self) && s_seen[i].id == (animId & 0xFFFFu) && s_seen[i].style == st) return;
        const LONG slot = InterlockedIncrement(&s_nSeen) - 1;
        if (slot >= seenCap) return;
        s_seen[slot] = {reinterpret_cast<uintptr_t>(self), animId & 0xFFFFu, st};
    }
    saferead::beginScope();
    if (!KP_ENABLED(chAnim, KPLOG_INFO) || !takeLine()) return;
    logAnimRes(self, animId);
}

// One byte into the game's memory, only when VirtualQuery says the page is committed and writable (probeWrite). The 0.13.0-dev style override is the only user.
static bool writeU8Safe(unsigned site, uint8_t* p, uint8_t v) {
    if (p == nullptr || !saferead::probeWrite(site, p, sizeof(uint8_t))) return false;
    *reinterpret_cast<volatile uint8_t*>(p) = v;
    return true;
}

// ---- 0.13.0-dev (HW16) hook H4b: mid-function in the resolver, after the weapon style byte was spilled to [ebp-1] and before any style switch reads it.
// Params (hooks.toml): ebp = the resolver's frame, self = [ebp-0x34] (the creature's BODY object, ECX at entry; NOT the client creature, 0.16.0-dev), animIdRaw = [ebp+8] (u16 id, upper half is garbage).
// Frame facts (static dump of the resolver): gate byte [ebp-0xD] (anim object +0xC6; 1 selects the wield table), style byte [ebp-1] (+0xC5; 8 = unarmed).
// Writes ONE byte ([ebp-1] = 8) and only when HolsterAnimStyle.h overrideStyle says so: gate 1, id in the locomotion filter, style with its own wield row, the creature's
// weapon is on the hip (g_holSet.ownerOfBase(self): a live entry whose body object is `self`, <= 500 ms old) and the OWNER creature's client combat flag reads 0. Attacks /
// ranged / flourish ids are not in the filter, and the later second style read in the resolver is untouched. Any unreadable value refuses; no engine call, no register change.
// Known limitation (0.16.0-dev): the damage reaction clip (id 0x283E) is not in the filter and has no hook here: its style read feeds the clip choice directly and cannot be
// overridden by a byte written beforehand (.claude/waves/hr7/r5.md).
static const LONG kStyleLogLines = 40;
static volatile LONG g_styleOverrides = 0;
extern "C" void __cdecl HolsterAnimStyle(void* ebpv, void* selfv, uint32_t animIdRaw) {
    if (!g_unarmedAnim) return;
    const uintptr_t ebp = reinterpret_cast<uintptr_t>(ebpv);
    const uintptr_t self = reinterpret_cast<uintptr_t>(selfv);
    if (!hs::frameOk(ebp) || self == 0) return;
    if (g_probe) maybeReload();
    if (!hs::idInFilter(hs::animId16(animIdRaw))) return;   // the cheap exit: most animations never reach a read
    saferead::beginScope();
    uint8_t gate = 0, style = 0;
    if (!saferead::readAt(kSiteAnim, ebp, -0xD, &gate) || !saferead::readAt(kSiteAnim, ebp, -1, &style)) return;
    if (gate != 1) return;
    const uint32_t now = GetTickCount();
    const uintptr_t owner = g_holSet.ownerOfBase(self, now);   // self is the BODY object; owner = the client creature (0 = not holstered: no override)
    const bool inSet = owner != 0;
    const int combOff = k2addr::off(&K2AddrTable::Creature_ClientCombat);
    uint32_t combat = 1;
    const bool combZero = inSet && combOff != 0 && rdU32Ok(kSiteAnim, owner, combOff, &combat) && combat == 0;
    uint8_t newStyle = 0;
    if (!hs::overrideStyle(animIdRaw, gate, style, inSet, combZero, &newStyle)) return;
    if (!writeU8Safe(kSiteAnim, reinterpret_cast<uint8_t*>(ebp) - 1, newStyle)) return;
    const LONG n = InterlockedIncrement(&g_styleOverrides);
    if (n <= kStyleLogLines && g_onAnim && KP_ENABLED(chAnim, KPLOG_INFO) && takeLine())
        KP_LOG(chAnim, "ANIMSTYLE owner=%x id=%x style=%u->%u", static_cast<unsigned>(owner), static_cast<unsigned>(hs::animId16(animIdRaw)), static_cast<unsigned>(style), static_cast<unsigned>(newStyle));
}

// Live reload of the tune keys (place, place_rot, place_off) when holster.ini changes. Throttled to 500 ms by ReloadPoll; a failed open keeps
// the last good config. Runs on the GobUpdate caller's thread; the busy flag keeps two threads from publishing at once.
static void maybeReload() {
    if (!filewatch::changed(g_poll, "holster.ini", GetTickCount())) return;
    if (InterlockedCompareExchange(&g_reloadBusy, 1, 0) != 0) return;
    std::string text;
    if (readFileText("holster.ini", text)) {
        hs::TuneCfg t = g_tuneW;
        parseTuneText(text.c_str(), t);
        publishTune(t);
        KP_LOG(chMain, "RELOAD place=%d rot=(%.1f,%.1f,%.1f) off=(%.3f,%.3f,%.3f) left_extra_rot=%.1f", t.place ? 1 : 0, t.rot[0], t.rot[1], t.rot[2], t.off[0], t.off[1], t.off[2],
               static_cast<double>(t.leftExtra));
        warnLeftExtraClamp(t, "RELOAD");
        for (int i = hs::CatUnknown + 1; i < hs::CatCount; ++i) {
            const hs::CatTune& ct = t.cat[i];
            if (ct.hasRot || ct.hasOff)
                KP_LOG(chMain, "RELOAD cat=%s rot=(%.1f,%.1f,%.1f)%s off=(%.3f,%.3f,%.3f)%s", hs::categoryName(static_cast<hs::Category>(i)), ct.rot[0], ct.rot[1],
                       ct.rot[2], ct.hasRot ? "" : "[global]", ct.off[0], ct.off[1], ct.off[2], ct.hasOff ? "" : "[global]");
        }
    }
    InterlockedExchange(&g_reloadBusy, 0);
}

// ---- 0.3.0-dev: toggle key (draw / stow) + shorter post-combat exit (doc 42; research/holstered_weapons/keybind/S2_design.txt). ----
// Tick-side only (called from HolsterGobUpdate, no AIUpdate detour). Inert unless bone_test=1 AND place_mode=drawgate. Every read goes through saferead;
// the only writes are the draw-hold statics and server +0x524 (aligned dword, probeWrite first). Unreadable / unknown state never cuts and never writes.
struct PartyCache {
    uintptr_t cli[3];   // client creatures, party::member(table, i); slot 0 = controlled
    uintptr_t srv[3];   // server creatures, party::serverCreature(cli); 0 = unusable slot
    uint32_t id[3];     // server +0x04 object id (rejects a recycled address)
    int n;
};
static PartyCache g_party = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 0};
static volatile LONG g_partyCombat = 0;   // 0.6.0-dev: any party member's client combat flag is set (holsterTick, 250 ms); read by HolsterPlace
static hs::Throttle g_delayThrottle[3];
static hs::KeyCounts<8> g_tickKeys;

static bool tickDue(DWORD& last, DWORD now, int32_t ms) {
    if (static_cast<int32_t>(now - last) < ms) return false;
    last = now;
    return true;
}

static bool tickLogOk() { return g_onToggle && KP_ENABLED(chToggle, KPLOG_INFO) && takeLine(); }

// Foreground test like companion_selfheal_src/CompanionSelfHeal.cpp foreground(): the game process owns the foreground window. The three user32 calls are
// resolved at run time from the already-loaded user32.dll so the DLL gains no new import (a new import makes kpm_hotswap demand a full reinstall).
struct User32Fns {
    HWND(WINAPI* getForeground)();
    DWORD(WINAPI* getWindowThread)(HWND, LPDWORD);
    SHORT(WINAPI* getAsyncKey)(int);
    bool ok;
};
static User32Fns& user32Fns() {
    static User32Fns f = {nullptr, nullptr, nullptr, false};
    static bool tried = false;
    if (!tried) {
        tried = true;
        const HMODULE m = GetModuleHandleA("user32.dll");
        if (m) {
            f.getForeground = reinterpret_cast<HWND(WINAPI*)()>(GetProcAddress(m, "GetForegroundWindow"));
            f.getWindowThread = reinterpret_cast<DWORD(WINAPI*)(HWND, LPDWORD)>(GetProcAddress(m, "GetWindowThreadProcessId"));
            f.getAsyncKey = reinterpret_cast<SHORT(WINAPI*)(int)>(GetProcAddress(m, "GetAsyncKeyState"));
            f.ok = f.getForeground && f.getWindowThread && f.getAsyncKey;
        }
        if (!f.ok) KP_LOGV(chMain, KPLOG_WARN, "toggle key: user32 entry points not found, key disabled (logged once)");
    }
    return f;
}
static bool winForeground() {
    const User32Fns& u = user32Fns();
    if (!u.ok) return false;
    HWND w = u.getForeground();
    DWORD pid = 0;
    if (!w) return false;
    u.getWindowThread(w, &pid);
    return pid == GetCurrentProcessId();
}

// Aligned dword write into the game's memory, only when VirtualQuery says the page is committed and writable.
static bool writeU32Safe(unsigned site, uintptr_t base, int off, uint32_t v) {
    void* const p = reinterpret_cast<void*>(base + static_cast<uintptr_t>(off));
    if (base == 0 || !saferead::probeWrite(site, p, sizeof(uint32_t))) return false;
    *reinterpret_cast<volatile uint32_t*>(p) = v;
    return true;
}

static void refreshParty() {
    saferead::beginScope();
    PartyCache p = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 0};
    const uintptr_t t = party::table();
    if (t) {
        int n = party::partyCount(t);
        if (n > 3) n = 3;
        for (int i = 0; i < n; ++i) {
            const uintptr_t cli = party::member(t, i);
            if (!cli) break;
            p.cli[i] = cli;
            p.n = i + 1;
            const uintptr_t srv = party::serverCreature(cli);
            uint32_t id = 0;
            if (srv && rdU32Ok(kSiteTick, srv, 0x04, &id)) { p.srv[i] = srv; p.id[i] = id; }
        }
    }
    g_party = p;
}

// 0.6.0-dev: the owner is one of the cached party members (client creature).
static bool ownerIsParty(uintptr_t owner) {
    if (owner == 0) return false;
    for (int i = 0; i < g_party.n && i < 3; ++i)
        if (g_party.cli[i] == owner) return true;
    return false;
}

// 0.6.0-dev party combat hold: recompute g_partyCombat from the cached members' client combat flags (unreadable counts as 0); log the edges.
static void refreshPartyCombat() {
    const int combatOff = k2addr::off(&K2AddrTable::Creature_ClientCombat);
    uint32_t flags[3] = {0, 0, 0};
    const int n = g_party.n < 3 ? g_party.n : 3;
    if (combatOff != 0) {
        saferead::beginScope();
        for (int i = 0; i < n; ++i) {
            uint32_t f = 0;
            flags[i] = (g_party.cli[i] != 0 && rdU32Ok(kSiteTick, g_party.cli[i], combatOff, &f)) ? f : 0u;
        }
    }
    const LONG on = hs::anyCombat(flags, n) ? 1 : 0;
    if (on == g_partyCombat) return;
    InterlockedExchange(&g_partyCombat, on);
    static volatile LONG s_lines = 0;
    if (InterlockedIncrement(&s_lines) <= 40 && tickLogOk()) KP_LOG(chToggle, "PARTYHOLD %s n=%d", on ? "on" : "off", n);
}

// HW13 flourish window for one client owner creature (called from HolsterPlace). Party members only; the server creature is re-validated against the cached
// object id (a recycled address never reads as open). Edge logs FLOURISH open/close per slot on the toggle channel.
static bool flourishOpenFor(uintptr_t owner) {
    if (owner == 0) return false;
    int slot = -1;
    for (int i = 0; i < g_party.n && i < 3; ++i)
        if (g_party.cli[i] == owner) { slot = i; break; }
    if (slot < 0 || g_party.srv[slot] == 0) return false;
    const int combatOff = k2addr::off(&K2AddrTable::Creature_ClientCombat);
    if (combatOff == 0) return false;
    const uintptr_t S = g_party.srv[slot];
    uint32_t id = 0, p520 = 0, p524 = 0, p528 = 0, p11E8 = 0, combat = 0;
    const bool ok = rdU32Ok(kSitePlace, S, 0x04, &id) && id == g_party.id[slot] && rdU32Ok(kSitePlace, S, 0x520, &p520) && rdU32Ok(kSitePlace, S, 0x524, &p524) &&
                    rdU32Ok(kSitePlace, S, 0x528, &p528) && rdU32Ok(kSitePlace, S, 0x11E8, &p11E8) && rdU32Ok(kSitePlace, owner, combatOff, &combat);
    hs::DelayIn in;
    in.plus520 = p520;
    in.plus524 = static_cast<int32_t>(p524);
    in.plus528 = p528;
    in.plus11E8 = p11E8;
    in.clientCombat = combat;
    in.clientKnown = ok;
    in.serverReadable = ok;
    const bool open = hs::flourishWindow(in);
    static bool s_open[3] = {false, false, false};
    static volatile LONG s_lines = 0;
    if (open != s_open[slot]) {
        s_open[slot] = open;
        if (g_probe && g_onToggle && KP_ENABLED(chToggle, KPLOG_INFO) && InterlockedIncrement(&s_lines) <= 40 && takeLine()) {
            if (open)
                KP_LOG(chToggle, "FLOURISH open slot=%d cli=%x srv=%x p524=%d p528=%u b11E8=%u", slot, U(reinterpret_cast<void*>(owner)), U(reinterpret_cast<void*>(S)),
                       static_cast<int>(p524), static_cast<unsigned>(p528), static_cast<unsigned>(hs::stanceByte(p11E8)));
            else
                KP_LOG(chToggle, "FLOURISH close slot=%d p520=%u p524=%d", slot, static_cast<unsigned>(p520), static_cast<int>(p524));
        }
    }
    return open;
}

// Engine nearest-enemy scan: CSWSCreature::GetNearestEnemy 0x0057C990 (Ghidra name), thiscall(self = server creature; float range, int exclude, int ref) RET 0xC.
// Guards from S2 sec 2 (a null deref inside crashes): [S+0x380] != 0 and [[S+0x380]+8] readable. Returns false when the scan could not run (guards,
// GOG table, null creature); then *hostile is untouched and the caller fails closed. The first 20 call durations are logged (QueryPerformanceCounter).
static bool hostileScan(uintptr_t S, bool* hostile) {
    using NearestFn = int(__thiscall*)(uintptr_t self, float range, int exclude, int ref);
    if (!S || k2addr::off(&K2AddrTable::Creature_ClientCombat) == 0) return false;   // Steam table only (GOG offset is 0)
    uint32_t pi = 0, f = 0;
    if (!rdU32Ok(kSiteTick, S, 0x380, &pi) || pi == 0 || !rdU32Ok(kSiteTick, pi, 8, &f)) return false;
    static volatile LONG s_calls = 0;
    const bool timed = g_onToggle && s_calls < 20;
    LARGE_INTEGER t0 = {}, t1 = {}, fq = {};
    if (timed) { QueryPerformanceCounter(&t0); QueryPerformanceFrequency(&fq); }
    const int r = reinterpret_cast<NearestFn>(static_cast<uintptr_t>(0x0057C990))(S, g_hostileRange, 0x7F000000, 0x7F000000);
    *hostile = r != 0x7F000000 && r != 0;
    if (timed) {
        QueryPerformanceCounter(&t1);
        const LONG c = InterlockedIncrement(&s_calls);
        if (c <= 20 && fq.QuadPart > 0 && tickLogOk())
            KP_LOG(chToggle, "HOSTILE call=%ld srv=%x range=%.1f result=%x dur_us=%lld", static_cast<long>(c), U(reinterpret_cast<void*>(S)), static_cast<double>(g_hostileRange),
                   static_cast<unsigned>(r), static_cast<long long>((t1.QuadPart - t0.QuadPart) * 1000000LL / fq.QuadPart));
    }
    saferead::beginScope();   // engine code ran: cached region answers may be stale
    return true;
}

// Feature B: per cached party slot, cut the post-combat countdown (server +0x524 = 0) once the gate says enough time has elapsed and no hostile is near.
static void runDelay(DWORD now) {
    const int combatOff = k2addr::off(&K2AddrTable::Creature_ClientCombat);
    if (combatOff == 0) return;
    saferead::beginScope();
    static bool s_warned = false;
    const int slots = g_delayParty ? g_party.n : (g_party.n > 0 ? 1 : 0);
    for (int i = 0; i < slots; ++i) {
        const uintptr_t S = g_party.srv[i], C = g_party.cli[i];
        if (!S || !C) continue;
        uint32_t id = 0, p520 = 0, p524 = 0, p528 = 0, p11E8 = 0, combat = 0;
        const bool srvOk = rdU32Ok(kSiteTick, S, 0x04, &id) && id == g_party.id[i] && rdU32Ok(kSiteTick, S, 0x520, &p520) && rdU32Ok(kSiteTick, S, 0x524, &p524) &&
                           rdU32Ok(kSiteTick, S, 0x528, &p528) && rdU32Ok(kSiteTick, S, 0x11E8, &p11E8);
        const bool cliOk = rdU32Ok(kSiteTick, C, combatOff, &combat);
        if (!srvOk || !cliOk) {
            if (!s_warned) {
                s_warned = true;
                KP_LOGV(chMain, KPLOG_WARN, "DELAY slot=%d fields unreadable (srv=%d cli=%d): no cut (this WARN is logged once)", i, srvOk ? 1 : 0, cliOk ? 1 : 0);
            }
            continue;
        }
        hs::DelayIn in;
        in.plus520 = p520;
        in.plus524 = static_cast<int32_t>(p524);
        in.plus528 = p528;
        in.plus11E8 = p11E8;
        in.clientCombat = combat;
        in.clientKnown = true;
        in.serverReadable = true;
        const hs::DelayAct a = hs::delayGate(in, g_delayCfg);
        // 0.6.0-dev move hold: a creature that is walking / running (or stopped less than move_hold_ms ago) keeps its stance; the cut waits.
        static bool s_moveLogged[3] = {false, false, false};
        static volatile LONG s_moveLines = 0;
        // 0.16.0-dev: g_move is keyed by the BODY object (the resolver hook's `self`), so look up with [C + Creature_BodyObj]; unreadable / 0 = not moving.
        const int bodyOff = k2addr::off(&K2AddrTable::Creature_BodyObj);
        uint32_t bodyPtr = 0;
        const bool bodyOk = bodyOff != 0 && rdU32Ok(kSiteTick, C, bodyOff, &bodyPtr) && bodyPtr != 0;
        const bool moving = hs::moveHoldBlocksCut(g_moveHoldMs > 0, bodyOk && g_move.movedWithin(bodyPtr, static_cast<uint32_t>(now), g_moveHoldMs));
        if (!moving) s_moveLogged[i] = false;
        if (a == hs::DelayNeedHostileCheck && moving) {
            if (!s_moveLogged[i]) {
                s_moveLogged[i] = true;
                if (InterlockedIncrement(&s_moveLines) <= 40 && tickLogOk()) KP_LOG(chToggle, "MOVEHOLD slot=%d skip cut (moving)", i);
            }
            continue;
        }
        if (a != hs::DelayNeedHostileCheck || !g_delayThrottle[i].due(now)) continue;
        bool hostile = false;
        if (!hostileScan(S, &hostile)) {   // scan could not run: fail closed, retry after the back-off
            g_delayThrottle[i].backoff(now, g_hostileBackoffMs);
            continue;
        }
        if (!hs::shouldCut(a, hostile)) { g_delayThrottle[i].backoff(now, g_hostileBackoffMs); continue; }
        const int32_t before = in.plus524;
        const int elapsed = hs::elapsedMs(before);
        const bool wrote = writeU32Safe(kSiteTick, S, 0x524, 0u);
        if (!wrote) g_delayThrottle[i].backoff(now, g_hostileBackoffMs);
        if (hs::keyedWant(hs::keyBump(g_tickKeys, S), 20, 10) && tickLogOk())
            KP_LOG(chToggle, "DELAY slot=%d cut before=%d elapsed=%d kind=%s wrote=%d", i, before, elapsed, combat != 0 ? "combat" : "flourish", wrote ? 1 : 0);
    }
}

// Feature A: a rising edge of the toggle key. Draw = timed per-owner hold (g_holdOwner/g_holdUntil, read by HolsterPlace); Stow = clear the hold and, in
// combat, end the countdown. The explicit press ignores +0x528 (S2 sec 2, C2).
static void runToggle(DWORD now) {
    refreshParty();
    const bool ready = g_party.n > 0 && g_party.cli[0] != 0;
    const uintptr_t cli0 = ready ? g_party.cli[0] : 0, srv0 = ready ? g_party.srv[0] : 0;
    const int combatOff = k2addr::off(&K2AddrTable::Creature_ClientCombat);
    saferead::beginScope();
    uint32_t combat = 0;
    const bool known = ready && combatOff != 0 && rdU32Ok(kSiteTick, cli0, combatOff, &combat);
    const bool holdOn = ready && hs::holdActive(cli0, g_holdOwner, g_holdUntil, now);
    bool hostile = false;
    if (ready && known && (holdOn || combat != 0)) {
        bool h = false;
        hostile = hostileScan(srv0, &h) ? h : true;   // scan could not run: treat as hostile, the stow is refused
    }
    hs::ToggleIn in;
    in.ready = ready;
    in.holdOn = holdOn;
    in.clientCombat = combat;
    in.combatKnown = known;
    in.hostileNear = hostile;
    const hs::ToggleAct act = hs::toggleDecide(in);
    const bool log = hs::keyedWant(hs::keyBump(g_tickKeys, 1), 200, 10);
    if (log && tickLogOk())
        KP_LOG(chToggle, "TOGGLE edge act=%d key=0x%x holdOn=%d combat=%u hostile=%d cli0=%x srv0=%x ready=%d known=%d", static_cast<int>(act), static_cast<unsigned>(g_toggleKey),
               holdOn ? 1 : 0, static_cast<unsigned>(combat), hostile ? 1 : 0, U(reinterpret_cast<void*>(cli0)), U(reinterpret_cast<void*>(srv0)), ready ? 1 : 0, known ? 1 : 0);
    if (act == hs::ToggleDraw) {
        g_holdUntil = now + (g_drawHoldMs ? g_drawHoldMs : 0x7FFFFFFFu);   // deadline first, then the owner
        g_holdOwner = cli0;
        if (log && tickLogOk()) KP_LOG(chToggle, "TOGGLE draw owner=%x hold_ms=%u", U(reinterpret_cast<void*>(cli0)), static_cast<unsigned>(g_drawHoldMs));
    } else if (act == hs::ToggleStow) {
        g_holdUntil = now;
        bool wrote = false;
        uint32_t before = 0;
        if (combat != 0 && srv0 && rdU32Ok(kSiteTick, srv0, 0x524, &before)) wrote = writeU32Safe(kSiteTick, srv0, 0x524, 0u);
        if (log && tickLogOk()) KP_LOG(chToggle, "TOGGLE stow owner=%x combat=%u cut=%d before=%d", U(reinterpret_cast<void*>(cli0)), static_cast<unsigned>(combat), wrote ? 1 : 0, static_cast<int>(before));
    } else if (log && tickLogOk()) {
        KP_LOG(chToggle, "TOGGLE %s", act == hs::ToggleRefused ? "refused" : "none");
    }
}

static void holsterTick() {
    if (!g_boneTest || !drawgateMode()) return;
    const bool keyOn = g_toggleKey != 0;
    const bool delayOn = g_delayCfg.combatExitMs > 0 || g_delayCfg.flourishExitMs > 0;
    if (!keyOn && !delayOn && !g_flourishDraw && !g_partyCombatHold) return;   // flourish_draw / party_combat_hold alone still need the 250 ms party cache
    static volatile LONG s_busy = 0;
    if (InterlockedCompareExchange(&s_busy, 1, 0) != 0) return;   // never re-enter (e.g. from inside an engine call)
    static DWORD s_partyAt = 0, s_delayAt = 0, s_keyAt = 0;
    static hs::KeyEdge s_edge;
    const DWORD now = GetTickCount();
    if (tickDue(s_partyAt, now, 250)) { refreshParty(); if (g_partyCombatHold) refreshPartyCombat(); }
    if (delayOn && tickDue(s_delayAt, now, 100)) runDelay(now);
    if (keyOn && tickDue(s_keyAt, now, 16)) {
        const bool down = winForeground() && (user32Fns().getAsyncKey(g_toggleKey) & 0x8000) != 0;
        if (s_edge.update(down)) runToggle(now);
    }
    InterlockedExchange(&s_busy, 0);
}

// 0x005047B0 behavior update handler (UpdateDependentPosition). C1 recon (.claude/research/holster_c1_postcopy.md): ECX is the CAurBehaviorAttach
// object, NOT the Gob. Child Gob = [this+0x10], attach node N = [this+0x18], name-lookup parent = [this+0x14]. The handler calls
// N->vtbl[+0xC](gob+0xA4, gob+0xB0), which re-copies the node transform into the child. Runs per delivered event: count every call, log the
// first 24 and then every gob_every-th. Logs the child WORLD transform (C3: 0x457510 composes the parent chain and the owner Gob) (+0xA4 vec3, +0xB0 quat w-first) of the real Gob.
extern "C" void __cdecl HolsterGobUpdate(void* beh) {
    if (g_probe) {
        static volatile LONG s_tid = 0;
        if (InterlockedCompareExchange(&s_tid, 1, 0) == 0) KP_LOG(chMain, "TID gobupdate=%lu", static_cast<unsigned long>(GetCurrentThreadId()));
        maybeReload();
        holsterTick();   // 0.3.0-dev: toggle key + shorter combat exit (inert unless bone_test=1 and place_mode=drawgate)
    }
    if (!g_onGob) return;
    const LONG n = InterlockedIncrement(&g_gobCalls);
    if (n > 24 && (n % g_gobEvery) != 0) return;
    saferead::beginScope();
    if (!KP_ENABLED(chGob, KPLOG_INFO) || !takeLine()) return;
    const uintptr_t b = reinterpret_cast<uintptr_t>(beh);
    const uintptr_t g = static_cast<uintptr_t>(rdU32(kSiteGob, b, 0x10));
    KP_LOG(chGob, "GOB n=%ld gob=%x parent18=%x pos=(%.3f,%.3f,%.3f) quat=(%.3f,%.3f,%.3f,%.3f) beh=%x par14=%x node=%x", static_cast<long>(n),
           static_cast<unsigned>(g), static_cast<unsigned>(g ? rdU32(kSiteGob, g, 0x18) : 0u), g ? rdF(kSiteGob, g, 0xA4) : 0.f,
           g ? rdF(kSiteGob, g, 0xA8) : 0.f, g ? rdF(kSiteGob, g, 0xAC) : 0.f, g ? rdF(kSiteGob, g, 0xB0) : 0.f, g ? rdF(kSiteGob, g, 0xB4) : 0.f,
           g ? rdF(kSiteGob, g, 0xB8) : 0.f, g ? rdF(kSiteGob, g, 0xBC) : 0.f, U(beh), static_cast<unsigned>(rdU32(kSiteGob, b, 0x14)),
           static_cast<unsigned>(rdU32(kSiteGob, b, 0x18)));
}

// Node name of an attach node, no engine call: H = [N+4]; index = [H+4]; owner = [H+8]; t = [owner+0x4C] & 0x7F; t==2 -> table = [owner+0xB8];
// t==5 -> table = [[owner+0x84]+0xB8]; name = (char*)[table + index*4]. step = the first failing step (0 = name read), for the PLACENODE log.
struct NodeInfo { uint32_t t; int step; char name[32]; };
static bool nodeName(uintptr_t node, NodeInfo& ni) {
    ni.t = 0xFFu; ni.step = 1; ni.name[0] = 0;
    const uintptr_t h = rdU32(kSitePlace, node, 0x04);
    if (!h) return false;
    ni.step = 2;
    const uint32_t index = rdU32(kSitePlace, h, 0x04);
    const uintptr_t owner = rdU32(kSitePlace, h, 0x08);
    if (!owner || index > 0xFFFFu) return false;
    ni.step = 3;
    ni.t = rdU32(kSitePlace, owner, 0x4C) & 0x7Fu;
    uintptr_t table = 0;
    if (ni.t == 2) table = rdU32(kSitePlace, owner, 0xB8);
    else if (ni.t == 5) { const uintptr_t inner = rdU32(kSitePlace, owner, 0x84); table = inner ? rdU32(kSitePlace, inner, 0xB8) : 0; }
    if (!table) return false;
    ni.step = 4;
    const uintptr_t namePtr = rdU32(kSitePlace, table, static_cast<int>(index * 4u));
    if (!namePtr) return false;
    ni.step = 0;
    readTag(kSitePlace, reinterpret_cast<const char*>(namePtr), ni.name, sizeof ni.name);
    return true;
}
// repoint: the node named bone_name; drawgate: the engine's native rhand / lhand attach nodes (lhand only with bone_test_left=1).
static bool nodeIsTarget(uintptr_t node, NodeInfo& ni) { return nodeName(node, ni) && hs::nodeAccepted(drawgateMode(), ni.name, g_boneName, g_boneLeft); }

// Inline model name of a Gob: [gob+0x84] = model, model+0x08 = name ("" when unreadable).
static void gobModelName(uintptr_t gob, char* out, int cap) {
    out[0] = 0;
    const uintptr_t model = gob ? rdU32(kSitePlace, gob, 0x84) : 0;
    if (model) readTag(kSitePlace, reinterpret_cast<const char*>(model + 0x08), out, cap);
}

// ---- hand record (0.1.1-dev, r6 recon): FUN_007637E0 attaches the weapon Gob to the creature's rhand (0x763B49) or lhand (0x763E7B) node. ----
// Both hand detours run BEFORE the stolen `MOV EAX,[EDX+0x7C] / CALL EAX` (vtbl+0x7C is a no-op on the verified classes), so they only record:
// gob -> hand into a 128-entry ring (HolsterHand.h) and log HANDATTACH. HolsterPlace then reports HANDMAP once per placed gob found in the ring.
// The record feeds HolsterPlace's left-hand resolve (kWorldKnowsHand is true). Both run on the main thread; the ring is not locked. Own caps: 64 lines each.
static hs::HandRing g_handRing;
static hs::SeenSet g_handSeen;
static volatile LONG g_handAttachLines = 0;
static const LONG kHandLines = 64;

// ecx = body anim object, gob = [EBP-0x2C], item = [EBP-0x40] (R) / [EBP-0x30] (L); item+0x14 = W.
static void handAttach(int hand, void* gob, void* item, void* body) {
    if (!g_probe) return;
    const uintptr_t g = reinterpret_cast<uintptr_t>(gob);
    (void)hs::handRecord(g_handRing, g, hand);   // state-only: runs with probe=1 even when ch_attach=0 (placement needs the record)
    if (!g_onAttach) return;
    saferead::beginScope();
    if (!KP_ENABLED(chAttach, KPLOG_INFO)) return;
    if (InterlockedIncrement(&g_handAttachLines) > kHandLines || !takeLine()) return;
    char name[17];
    gobModelName(g, name, sizeof name);
    const uintptr_t it = reinterpret_cast<uintptr_t>(item), b = reinterpret_cast<uintptr_t>(body);
    const uint32_t w = it ? rdU32(kSiteAttach, it, 0x14) : 0u;
    const uint32_t bvt = b ? rdU32(kSiteAttach, b, 0) : 0u;
    KP_LOG(chAttach, "HANDATTACH hand=%c gob=%x item=%x W=%x name=%s bodyvt=%x", hs::handChar(hand), U(gob), U(item), static_cast<unsigned>(w),
           name[0] ? name : "-", static_cast<unsigned>(bvt));
}

// 0x763B49 right-hand attach site. Args (hooks.toml order): weapon Gob [ebp-0x2c], item [ebp-0x40], body anim object ecx.
extern "C" void __cdecl OnHandAttachR(void* gob, void* item, void* body) { handAttach(hs::HandR, gob, item, body); }
// 0x763E7B left-hand attach site. Args: weapon Gob [ebp-0x2c], item [ebp-0x30], body anim object ecx.
extern "C" void __cdecl OnHandAttachL(void* gob, void* item, void* body) { handAttach(hs::HandL, gob, item, body); }

// Called first thing in HolsterPlace: logs HANDMAP once per placed gob that the ring knows (dedupe and cap 64 in hs::SeenSet).
static void handMapLog(uintptr_t g) {
    if (!g_onAttach || !g) return;
    const int hand = hs::handLookup(g_handRing, g);
    if (hand == hs::HandNone || !hs::seenFirst(g_handSeen, g)) return;
    saferead::beginScope();
    if (!KP_ENABLED(chAttach, KPLOG_INFO) || !takeLine()) return;
    char name[17];
    gobModelName(g, name, sizeof name);
    KP_LOG(chAttach, "HANDMAP g=%x hand=%c name=%s", static_cast<unsigned>(g), hs::handChar(hand), name[0] ? name : "-");
}

// HE0 probe (doc 42 HW-EDIT, plan .claude/plans: holster editor). Answers: does the per-frame attach refresh (this hook) run for the
// equip-screen preview weapons, and which body / thread does it see? The preview attaches weapons to the engine's own "rhand"/"lhand"
// literal, so its weapon nodes keep those names (in-world weapons under bone_test point at bone_name instead). PREVIEWNODE: one line per
// new (parent, node name, model) triple whose model starts with w_ and whose node is rhand/lhand; own cap of 40, independent of PLACENODE,
// placed before the bone_test/place gates. CAMNODE: once per distinct parent (first 8), its model name, its own parent ([parent+0x18])
// and whether FindPart("camerahook") resolves on either (FindPart slot checked against the weapon Gob's own +0x10C slot, as mountPose does).
static uintptr_t camFind(uintptr_t gob, uintptr_t on) {
    if (!on) return 0;
    const uintptr_t ovt = rdU32(kSitePlace, on, 0), gvt = rdU32(kSitePlace, gob, 0);
    const uintptr_t find = ovt ? rdU32(kSitePlace, ovt, 0x10C) : 0, findRef = gvt ? rdU32(kSitePlace, gvt, 0x10C) : 0;
    if (!find || find != findRef) return 1;   // 1 = slot mismatch, not called
    return reinterpret_cast<uintptr_t>(reinterpret_cast<void*(__thiscall*)(void*, const char*)>(find)(reinterpret_cast<void*>(on), "camerahook"));
}
static void camNodeOnce(uintptr_t gob, uintptr_t parent) {
    static uintptr_t s_par[8];
    static volatile LONG s_n = 0;
    if (!parent || s_n >= 8) return;
    for (LONG i = 0; i < s_n; ++i)
        if (s_par[i] == parent) return;
    const LONG slot = InterlockedIncrement(&s_n) - 1;
    if (slot >= 8) return;
    s_par[slot] = parent;
    if (!KP_ENABLED(chPlace, KPLOG_INFO) || !takeLine()) return;
    const uintptr_t pp = rdU32(kSitePlace, parent, 0x18);
    char pname[32], ppname[32];
    gobModelName(parent, pname, sizeof pname);
    gobModelName(pp, ppname, sizeof ppname);
    const uintptr_t camP = camFind(gob, parent), camPP = camFind(gob, pp);
    KP_LOG(chPlace, "CAMNODE parent=%x pmodel=%s pvt=%x pparent=%x ppmodel=%s ppvt=%x camhook_p=%x camhook_pp=%x", static_cast<unsigned>(parent),
           pname[0] ? pname : "-", static_cast<unsigned>(rdU32(kSitePlace, parent, 0)), static_cast<unsigned>(pp), ppname[0] ? ppname : "-",
           static_cast<unsigned>(pp ? rdU32(kSitePlace, pp, 0) : 0u), static_cast<unsigned>(camP), static_cast<unsigned>(camPP));
}
static void previewProbe(uintptr_t g, uintptr_t beh) {
    static volatile LONG s_lines = 0;
    if (!g_onPlace || s_lines >= 40 || !g || !beh) return;
    saferead::beginScope();
    const uintptr_t n = rdU32(kSitePlace, beh, 0x18);
    if (!n) return;
    struct Seen { uintptr_t g, n; };   // (gob, node) pairs already looked at, so each attach is classified once
    static Seen s_seen[256];
    static unsigned s_next = 0;
    for (const Seen& s : s_seen)
        if (s.g == g && s.n == n) return;
    s_seen[s_next] = {g, n};
    s_next = (s_next + 1) % 256u;
    char mname[32];
    gobModelName(g, mname, sizeof mname);
    if ((mname[0] != 'w' && mname[0] != 'W') || mname[1] != '_') return;
    NodeInfo ni;
    if (!nodeName(n, ni) || (_stricmp(ni.name, "rhand") != 0 && _stricmp(ni.name, "lhand") != 0)) return;
    const uintptr_t parent = rdU32(kSitePlace, beh, 0x14);
    struct Trip { uintptr_t parent; char node[8]; char mname[32]; };
    static Trip s_trip[40];
    static volatile LONG s_nTrip = 0;
    for (LONG i = 0; i < s_nTrip && i < 40; ++i)
        if (s_trip[i].parent == parent && !_stricmp(s_trip[i].node, ni.name) && !_stricmp(s_trip[i].mname, mname)) return;
    const LONG slot = InterlockedIncrement(&s_lines) - 1;
    if (slot >= 40) return;
    s_trip[slot].parent = parent;
    snprintf(s_trip[slot].node, sizeof s_trip[slot].node, "%s", ni.name);
    snprintf(s_trip[slot].mname, sizeof s_trip[slot].mname, "%s", mname);
    InterlockedIncrement(&s_nTrip);
    if (KP_ENABLED(chPlace, KPLOG_INFO) && takeLine()) {
        char body[32];
        gobModelName(parent, body, sizeof body);
        KP_LOG(chPlace, "PREVIEWNODE gob=%x node=%x name=%s parent=%x owner=%x body=%s mname=%s cat=%s gobvt=%x tid=%lu tick=%lu", static_cast<unsigned>(g),
               static_cast<unsigned>(n), ni.name, static_cast<unsigned>(parent), static_cast<unsigned>(parent ? rdU32(kSitePlace, parent, 0x1D8) : 0u),
               body[0] ? body : "-", mname, hs::categoryName(hs::classify(mname, 0)), static_cast<unsigned>(rdU32(kSitePlace, g, 0)),
               static_cast<unsigned long>(GetCurrentThreadId()), static_cast<unsigned long>(GetTickCount()));
    }
    camNodeOnce(g, parent);
}

// Refusal reasons of HolsterPlace (live test 10-15: PLACE n=0 with no hint which gate refused). log_verbose=1: each reason logs its first 5 hits, then
// every 500th. Default: once per distinct (reason, gob) plus a summary every 10000th hit.
enum PlaceRefuse : int { PrNull = 0, PrNoHandle, PrNotTarget, PrGobvt, PrReadIn, PrInsaneIn, PrInsaneOut, PrNoWrite, PrNoMount, PrNotWeapon, PrNotHolsterable, PrCount };
static const char* const kRefuseName[PrCount] = {"null", "nohandle", "nottarget", "gobvt", "readin", "insanein", "insaneout", "nowrite", "nomount", "notweapon", "notholsterable"};
static volatile LONG g_refuse[PrCount] = {};
static void refuse(PlaceRefuse r, uintptr_t g, uintptr_t n, uint32_t vt) {
    LONG c = InterlockedIncrement(&g_refuse[r]);
    if (!g_onPlace) return;
    if (g_logVerbose) {
        if (c > 5 && (c % 500) != 0) return;
    } else {
        // Quiet mode: once per distinct (reason, gob) and a summary every 10000th hit of the reason; n= stays the running count per reason.
        uint32_t cq = 0;
        if (!hs::refuseWant(g_refuseTab, static_cast<int>(r), g, &cq)) return;
        c = static_cast<LONG>(cq);
    }
    if (!KP_ENABLED(chPlace, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chPlace, "PLACEREFUSE reason=%s n=%ld gob=%x node=%x gobvt=%x", kRefuseName[r], static_cast<long>(c), static_cast<unsigned>(g),
           static_cast<unsigned>(n), static_cast<unsigned>(vt));
}

// Mount-bone pose for place_bone_<type> (C3/C4 recon): parent Gob = [beh+0x14]; FindPart = parent vtbl+0x10C, thiscall(name) -> node, RET 4;
// node world pose = node vtbl+0xC, thiscall(vec3*, quat*): the same call the engine made for N just before the hook. Before calling, each slot
// must equal the weapon Gob's own +0x10C slot and N's own +0xC slot (same engine function, no address constants). The node is cached per
// (parent, category, bone) and re-checked every frame (owner [node+0x40] equals N's owner, pose slot unchanged).
struct MountEnt { uintptr_t parent, owner, node; int cat; char bone[32]; };
static MountEnt g_mount[64];
static unsigned g_mountNext = 0;
static void mountLog(const char* what, uintptr_t parent, const char* bone, uintptr_t node, uintptr_t a, uintptr_t b) {
    static volatile LONG s_lines = 0;
    if (!g_onPlace || InterlockedIncrement(&s_lines) > 40 || !KP_ENABLED(chPlace, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chPlace, "MOUNT %s parent=%x bone=%s node=%x a=%x b=%x", what, static_cast<unsigned>(parent), bone, static_cast<unsigned>(node),
           static_cast<unsigned>(a), static_cast<unsigned>(b));
}
static bool mountPose(uintptr_t gob, uintptr_t n, uintptr_t beh, hs::Category cat, const char* bone, hs::Pose& out) {
    const uintptr_t parent = rdU32(kSitePlace, beh, 0x14);
    const uintptr_t owner = rdU32(kSitePlace, n, 0x40);
    const uintptr_t nvt = rdU32(kSitePlace, n, 0);
    const uintptr_t poseFn = nvt ? rdU32(kSitePlace, nvt, 0x0C) : 0;
    if (!parent || !owner || !poseFn) return false;
    uintptr_t mn = 0;
    MountEnt* hit = nullptr;
    for (MountEnt& e : g_mount)
        if (e.parent == parent && e.cat == cat && e.owner == owner && _stricmp(e.bone, bone) == 0) { hit = &e; mn = e.node; break; }
    if (mn) {   // stale entry (node freed or reused): drop it and look up again
        const uintptr_t mvt = rdU32(kSitePlace, mn, 0);
        if (rdU32(kSitePlace, mn, 0x40) != owner || !mvt || rdU32(kSitePlace, mvt, 0x0C) != poseFn) { hit->parent = 0; mn = 0; hit = nullptr; }
    }
    if (!mn) {
        const uintptr_t pvt = rdU32(kSitePlace, parent, 0), gvt = rdU32(kSitePlace, gob, 0);
        const uintptr_t find = pvt ? rdU32(kSitePlace, pvt, 0x10C) : 0, findRef = gvt ? rdU32(kSitePlace, gvt, 0x10C) : 0;
        if (!find || find != findRef) { mountLog("refused-findslot", parent, bone, 0, find, findRef); return false; }
        mn = reinterpret_cast<uintptr_t>(reinterpret_cast<void*(__thiscall*)(void*, const char*)>(find)(reinterpret_cast<void*>(parent), bone));
        const uintptr_t mvt = mn ? rdU32(kSitePlace, mn, 0) : 0;
        const uintptr_t mfn = mvt ? rdU32(kSitePlace, mvt, 0x0C) : 0;
        const uintptr_t mowner = mn ? rdU32(kSitePlace, mn, 0x40) : 0;
        if (!mn || mfn != poseFn || mowner != owner) { mountLog("refused-node", parent, bone, mn, mfn, mowner); return false; }
        MountEnt& e = g_mount[g_mountNext];
        g_mountNext = (g_mountNext + 1) % 64u;
        e.parent = parent; e.owner = owner; e.node = mn; e.cat = cat;
        snprintf(e.bone, sizeof e.bone, "%s", bone);
        mountLog("found", parent, bone, mn, owner, poseFn);
    }
    hs::Pose p;
    float* const f = reinterpret_cast<float*>(&p);
    reinterpret_cast<void(__thiscall*)(void*, float*, float*)>(poseFn)(reinterpret_cast<void*>(mn), f, f + 3);
    if (!hs::poseSane(p)) return false;
    out = p;
    return true;
}

// ---- HW19 right-arm reach probe (reach_test, 0.7.0-dev) ----
// During the draw/stow slide the owner's right arm (rbicep_g -> forearm -> rhand grip hook) is bent so the hook chases the shown weapon pose: the engine's
// hand pose at the draw end, the hip at the stow end. Write point = HolsterPlace itself (no new hook): only the local quats of the bicep and forearm
// ([part+0x14], w,x,y,z) are written, through probeWrite, and never SetPartLocalPosition (it freezes the bone). The sampled pose is cached per bone
// (hs::Base): if the engine did not resample since our last write (persist=1) the cached value is restored before the poses are read, so the solve always
// starts from the animation, never from its own output. Parts are found with the same FindPart call as mountPose (slot guard against the weapon Gob's).
struct ReachEnt {
    uintptr_t parent, owner, bic, fore, hand, wrist, poseFn;   // wrist = the hand bone the grip hook hangs on ([hand+0x3C]); 0 = none found
    hs::Base bb, fb, wb;
    bool dirty, refused, left;
    uint32_t refusedAt;
    uint8_t solved, holdN;   // HW25: write bits (bicep 1, forearm 2, wrist 4) of the last successful solve, and the consecutive frames held since (cleared at slide end / entry drop)
    uint8_t wristOn;         // HW26: wristDq holds the last wrist delta of this slide (the reachClampRot sign hint); cleared together with solved / holdN
    hs::Q wristDq;
};
static const uint8_t kReachHoldFrames = 3;   // HW25: a transient refusal re-applies the last solved arm for up to this many frames (about 50 ms), then lets go
static struct ReachStyleInit {
    ReachStyleInit() {
        for (int i = 0; i < hs::CatCount; ++i) {
            g_reachStyle[i] = hs::reachDefaultStyle(i);
            g_hingeOutT[i] = hs::kHingeOutT;
            g_hingeOutPole[i] = i == hs::CatRifle ? hs::kHingeOutPoleRifle : (i == hs::CatHeavy ? hs::kHingeOutPoleHeavy : hs::kHingeOutPole);
        }
    }
} g_reachStyleInit;
static ReachEnt g_reach[64];
static unsigned g_reachNext = 0;
static volatile LONG g_reachLines = 0;

static bool reachNodePose(uintptr_t node, uintptr_t fn, hs::Pose& p) {
    float* const f = reinterpret_cast<float*>(&p);
    reinterpret_cast<void(__thiscall*)(void*, float*, float*)>(fn)(reinterpret_cast<void*>(node), f, f + 3);
    return hs::poseSane(p);
}
static bool reachReadQ(uintptr_t node, hs::Q& q) {
    if (!saferead::readAt(kSitePlace, node, 0x14, &q) || !hs::qfinite(q)) return false;
    const float n2 = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
    return n2 > 0.9f && n2 < 1.1f;
}
static bool reachWriteQ(uintptr_t node, const hs::Q& q) {
    if (!hs::qfinite(q) || !saferead::probeWrite(kSitePlace, reinterpret_cast<void*>(node + 0x14), sizeof(hs::Q))) return false;
    memcpy(reinterpret_cast<void*>(node + 0x14), &q, sizeof(hs::Q));
    return true;
}
static const char* reachTopoName(hs::ReachTopo t) {
    switch (t) {
        case hs::ReachTopo::HingeBack: return "hinge";
        case hs::ReachTopo::OverBack: return "over";
        case hs::ReachTopo::Thigh: return "thigh";
        default: return "hip";
    }
}
static bool reachIsBackBone(const char* b) { return b && (!_stricmp(b, "torsoUpr_g") || !_stricmp(b, "torso_g")); }
static bool reachLogDue() {
    if (!KP_ENABLED(chMain, KPLOG_INFO)) return false;
    const LONG c = InterlockedIncrement(&g_reachLines);
    return (c <= 24 || (c % g_gobEvery) == 0) && takeLine();
}
static void reachRefuse(const char* why, uintptr_t owner) {
    if (KP_ENABLED(chMain, KPLOG_INFO) && takeLine()) KP_LOG(chMain, "REACH owner=%x refuse=%s", static_cast<unsigned>(owner), why);
}
static uintptr_t reachFind(uintptr_t parent, uintptr_t find, const char* const* names, uintptr_t owner, uintptr_t poseFn) {
    for (; *names; ++names) {
        const uintptr_t n = reinterpret_cast<uintptr_t>(reinterpret_cast<void*(__thiscall*)(void*, const char*)>(find)(reinterpret_cast<void*>(parent), *names));
        const uintptr_t nvt = n ? rdU32(kSitePlace, n, 0) : 0;
        if (nvt && rdU32(kSitePlace, nvt, 0x0C) == poseFn && rdU32(kSitePlace, n, 0x40) == owner) return n;
    }
    return 0;
}
static bool reachStale(const ReachEnt& e) {
    const uintptr_t nodes[3] = {e.bic, e.fore, e.hand};
    for (uintptr_t n : nodes) {
        const uintptr_t nvt = rdU32(kSitePlace, n, 0);
        if (rdU32(kSitePlace, n, 0x40) != e.owner || !nvt || rdU32(kSitePlace, nvt, 0x0C) != e.poseFn) return true;
    }
    return false;
}
// w == 0: only undo a leftover write (a body the engine did not resample); w > 0: one reach step toward tgt.pos, or (carry) toward the point on the mount -> hand path at
// slideF. carry: *carried receives the world pose of the attach node (the hand the weapon hangs on) AFTER the arm write; the return value says it is valid.
static bool reachArm(uintptr_t gob, uintptr_t beh, const hs::Pose& tgt, float w, bool left, int cat, float pathF, float restMix, const float* mountPos, const hs::Q* mountQ, bool backMount, uintptr_t attachNode = 0,
                     bool carry = false, hs::Pose* carried = nullptr) {
    const uintptr_t parent = rdU32(kSitePlace, beh, 0x14);
    if (!parent) return false;
    ReachEnt* e = nullptr;
    for (ReachEnt& x : g_reach)
        if (x.parent == parent && x.left == left) { e = &x; break; }
    if (!e) {
        if (!(w > 0.f)) return false;
        const uintptr_t pvt = rdU32(kSitePlace, parent, 0), gvt = rdU32(kSitePlace, gob, 0);
        const uintptr_t find = pvt ? rdU32(kSitePlace, pvt, 0x10C) : 0, findRef = gvt ? rdU32(kSitePlace, gvt, 0x10C) : 0;
        e = &g_reach[g_reachNext];
        g_reachNext = (g_reachNext + 1) % 64u;
        *e = ReachEnt();
        e->parent = parent;
        e->left = left;
        e->refusedAt = GetTickCount();
        e->refused = true;   // until every part resolved
        static const char* const kBic[2][2] = {{"rbicep_g", nullptr}, {"lbicep_g", nullptr}};
        static const char* const kFore[2][3] = {{"Rforearm_g", "rforearm_g", nullptr}, {"Lforearm_g", "lforearm_g", nullptr}};
        static const char* const kHand[2][2] = {{"rhand", nullptr}, {"lhand", nullptr}};
        const int sd = left ? 1 : 0;
        if (!find || find != findRef) { reachRefuse("findslot", parent); return false; }
        // The first part fixes the pose slot and owner every other part must share.
        const uintptr_t bic = reinterpret_cast<uintptr_t>(reinterpret_cast<void*(__thiscall*)(void*, const char*)>(find)(reinterpret_cast<void*>(parent), kBic[sd][0]));
        const uintptr_t bvt = bic ? rdU32(kSitePlace, bic, 0) : 0;
        const uintptr_t fn = bvt ? rdU32(kSitePlace, bvt, 0x0C) : 0, owner = bic ? rdU32(kSitePlace, bic, 0x40) : 0;
        if (!bic || !fn || !owner) { reachRefuse("nobone", parent); return false; }
        const uintptr_t fore = reachFind(parent, find, kFore[sd], owner, fn), hand = reachFind(parent, find, kHand[sd], owner, fn);
        if (!fore || !hand) { reachRefuse("nobone", parent); return false; }
        e->owner = owner; e->bic = bic; e->fore = fore; e->hand = hand; e->poseFn = fn;
        {   // the wrist: the grip hook's parent, when it shares the pose slot and owner (otherwise no wrist rotation)
            const uintptr_t wr = rdU32(kSitePlace, hand, 0x3C), wvt = wr ? rdU32(kSitePlace, wr, 0) : 0;
            e->wrist = (wvt && rdU32(kSitePlace, wvt, 0x0C) == fn && rdU32(kSitePlace, wr, 0x40) == owner) ? wr : 0;
        }
        e->refused = false;
        if (KP_ENABLED(chMain, KPLOG_INFO) && takeLine())
            KP_LOG(chMain, "REACH found side=%c parent=%x owner=%x bic=%x fore=%x hand=%x wrist=%x", left ? 'L' : 'R', static_cast<unsigned>(parent), static_cast<unsigned>(owner), static_cast<unsigned>(bic),
                   static_cast<unsigned>(fore), static_cast<unsigned>(hand), static_cast<unsigned>(e->wrist));
    }
    if (e->refused) {
        if (static_cast<int32_t>(GetTickCount() - e->refusedAt) > 5000) e->parent = 0;   // retry later
        return false;
    }
    if (reachStale(*e)) { e->parent = 0; e->solved = e->holdN = e->wristOn = 0; return false; }

    // HW25: a transient refusal (quat read, pose, solve) while the slide still runs re-applies the last solved local quats instead of dropping the arm to the animation for
    // a frame. Nothing world-space is stored: the carried hand is recomputed from the attach node. Never at slide end (w <= 0) or on a permanent refusal.
    auto holdLast = [&](hs::ReachHoldKind kind, const char* why) -> bool {
        if (!(w > 0.f)) { e->solved = e->holdN = e->wristOn = 0; return false; }   // slide over: nothing to hold, and the next slide starts clean
        if (!hs::reachHoldAllowed(kind, e->solved, e->holdN, w, kReachHoldFrames, carry)) return false;
        bool ok = true;
        if (e->solved & 1) ok = reachWriteQ(e->bic, e->bb.wrote) && ok;
        if (e->solved & 2) ok = reachWriteQ(e->fore, e->fb.wrote) && ok;
        if ((e->solved & 4) && e->wrist) ok = reachWriteQ(e->wrist, e->wb.wrote) && ok;
        if (!ok) return false;
        ++e->holdN;
        static volatile LONG s_holdLines = 0;
        if (KP_ENABLED(chMain, KPLOG_INFO) && InterlockedIncrement(&s_holdLines) <= 40 && takeLine())
            KP_LOG(chMain, "REACH owner=%x hold=%u why=%s solved=%u w=%.2f", static_cast<unsigned>(e->owner), static_cast<unsigned>(e->holdN), why, static_cast<unsigned>(e->solved), static_cast<double>(w));
        if (!(carry && carried && attachNode && (e->solved & 3) == 3)) return false;
        const uintptr_t avt = rdU32(kSitePlace, attachNode, 0);
        return avt && rdU32(kSitePlace, avt, 0x0C) == e->poseFn && reachNodePose(attachNode, e->poseFn, *carried);
    };

    hs::Q curB, curF, curW = {1, 0, 0, 0};
    if (!reachReadQ(e->bic, curB) || !reachReadQ(e->fore, curF)) return holdLast(hs::ReachHoldQuatRead, "read");
    const bool wristOk = e->wrist != 0 && reachReadQ(e->wrist, curW);
    auto same = [](const hs::Q& a, const hs::Q& b) { return std::fabs(a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z) > 0.99999f; };
    const bool persist = (e->bb.have && same(curB, e->bb.wrote)) || (e->fb.have && same(curF, e->fb.wrote)) || (wristOk && e->wb.have && same(curW, e->wb.wrote));
    if (!(w > 0.f)) {   // slide not running: undo what we left behind and go quiet
        if (e->dirty && persist) { (void)reachWriteQ(e->bic, e->bb.q); (void)reachWriteQ(e->fore, e->fb.q); if (wristOk && e->wb.have) (void)reachWriteQ(e->wrist, e->wb.q); }
        e->dirty = false;
        e->solved = e->holdN = e->wristOn = 0;
        e->bb.have = e->fb.have = e->wb.have = false;
        return false;
    }
    const hs::Q baseB = hs::reachBase(e->bb, curB), baseF = hs::reachBase(e->fb, curF), baseW = wristOk ? hs::reachBase(e->wb, curW) : curW;
    if (persist) { (void)reachWriteQ(e->bic, baseB); (void)reachWriteQ(e->fore, baseF); if (wristOk) (void)reachWriteQ(e->wrist, baseW); }   // poses below must come from the animation

    hs::Pose pS, pE, pH;
    const uintptr_t parB = rdU32(kSitePlace, e->bic, 0x3C), parF = rdU32(kSitePlace, e->fore, 0x3C);
    hs::Pose pPB, pPF;
    if (!parB || !parF || !reachNodePose(e->bic, e->poseFn, pS) || !reachNodePose(e->fore, e->poseFn, pE) || !reachNodePose(e->hand, e->poseFn, pH) ||
        !reachNodePose(parB, e->poseFn, pPB)) { reachRefuse("pose", e->owner); return holdLast(hs::ReachHoldPose, "pose"); }
    // Elbow pole: the style of this weapon category blended by how high the MOUNT sits (hip bends back and out, over-the-shoulder bends up and out), eased into the
    // animation's own elbow direction as the weapon reaches the hand. Body frame: x right, y forward, z up (parent Gob +0xA4 pose); not verified live yet (see u= / lt= in the log).
    float poleV[3], u = 0.f, lt[3] = {0, 0, 0};
    float dzA = 0.f, fyA = 0.f, fxA = 0.f;   // HW24: the placed mount point in the body frame, in arm lengths (z up, y forward: <0 = behind, x right)
    float T[3] = {tgt.pos[0], tgt.pos[1], tgt.pos[2]};   // the point the hand is aimed at: the shown weapon (chase) or the carry path point
    const float* pole = nullptr;
    bool hinge = false;
    hs::ReachTopo topo = hs::ReachTopo::Hip;
    hs::Pose body;
    const float es0[3] = {pE.pos[0] - pS.pos[0], pE.pos[1] - pS.pos[1], pE.pos[2] - pS.pos[2]};
    if (saferead::readAt(kSitePlace, parent, 0xA4, &body) && hs::poseSane(body)) {
        const float ms[3] = {mountPos[0] - pS.pos[0], mountPos[1] - pS.pos[1], mountPos[2] - pS.pos[2]};
        float mlt[3];
        const hs::Q inv = hs::reachConj(body.q);
        hs::qrotate(inv, ms, mlt);
        const float he0[3] = {pH.pos[0] - pE.pos[0], pH.pos[1] - pE.pos[1], pH.pos[2] - pE.pos[2]};
        const float arm = hs::r3len(es0) + hs::r3len(he0);
        if (arm > 1e-4f) { dzA = mlt[2] / arm; fyA = mlt[1] / arm; fxA = mlt[0] / arm; }
        // HW27: the mount's height sense, not the bone name, picks the reach: HingeBack / OverBack (shoulder height or above) reach up and over the shoulder, whatever the weapon
        // (u = 1: a rifle slung low still has its grip behind the shoulder as long as it is above kOverDz); Hip and Thigh keep the hip pole weighted by reachOverWeight
        // (0 below the band). backMount (torsoUpr_g / torso_g) no longer forces it: torso_g sits at pelvis height and resolves to Hip. Logged as topo=.
        topo = arm > 1e-4f ? hs::reachTopology(dzA, hs::reachHingeFits(mlt[2], arm)) : hs::ReachTopo::Hip;
        u = (topo == hs::ReachTopo::HingeBack || topo == hs::ReachTopo::OverBack) ? 1.f : hs::reachOverWeight(mlt[2], arm);
        const int ci = (cat >= 0 && cat < hs::CatCount) ? cat : 0;
        const hs::ReachStyle& st = g_reachStyle[ci];
        if (carry) {   // the hand walks mount -> animation hand along an arc bowed up / forward / outward for a high mount (reach: backwards along it, carry: forwards); restMix pulls it to the resting hand
            const float upL[3] = {0.f, 0.f, 1.f}, fwL[3] = {0.f, 1.f, 0.f}, ouL[3] = {left ? -1.f : 1.f, 0.f, 0.f};
            float upW[3], fwW[3], ouW[3], pt[3];
            hs::qrotate(hs::qnormalize(body.q), upL, upW);
            hs::qrotate(hs::qnormalize(body.q), fwL, fwW);
            hs::qrotate(hs::qnormalize(body.q), ouL, ouW);
            hs::reachCarryTarget(mountPos, pH.pos, upW, fwW, ouW, st.bulge, u, arm, pathF, pt);
            for (int i = 0; i < 3; ++i) T[i] = (1.f - restMix) * pt[i] + restMix * pH.pos[i];
            // back mount: the hinged path replaces the bowed line and sets the pole itself (stage by stage, ending in the animation's own elbow plane at the rest hand)
            if (topo == hs::ReachTopo::HingeBack && g_reachHinge && hs::reachHinge(pS.pos, hs::r3len(es0), hs::r3len(he0), upW, fwW, ouW, mountPos, pH.pos, es0, pathF, pt, poleV, g_hingeOutT[ci], g_hingeOutPole[ci])) {
                for (int i = 0; i < 3; ++i) T[i] = pt[i];
                pole = poleV;
                hinge = true;
            }
        }
        const float ts[3] = {T[0] - pS.pos[0], T[1] - pS.pos[1], T[2] - pS.pos[2]};
        hs::qrotate(inv, ts, lt);
        if (!hinge && hs::reachPole(st, u, left, body.q, es0, (restMix > hs::reachAnimMix(pathF) ? restMix : hs::reachAnimMix(pathF)), poleV)) pole = poleV;   // the mount style holds the whole swing, draw and stow alike
    } else if (carry) {
        for (int i = 0; i < 3; ++i) T[i] = (1.f - restMix) * ((1.f - pathF) * mountPos[i] + pathF * pH.pos[i]) + restMix * pH.pos[i];   // no body frame: the straight line
    }
    float enew[3], reach = 1.f;
    if (!hs::solveTwoBone(pS.pos, pE.pos, pH.pos, T, enew, &reach, pole)) { reachRefuse("solve", e->owner); return holdLast(hs::ReachHoldSolve, "solve"); }
    const float es[3] = {pE.pos[0] - pS.pos[0], pE.pos[1] - pS.pos[1], pE.pos[2] - pS.pos[2]};
    const float ns[3] = {enew[0] - pS.pos[0], enew[1] - pS.pos[1], enew[2] - pS.pos[2]};
    const hs::Q outB = hs::blendQ(baseB, hs::reachDeltaToLocal(pPB.q, hs::arcQuat(es, ns), baseB), w);
    int wrote = 0;
    if (reachWriteQ(e->bic, outB)) { hs::reachNoteWrite(e->bb, outB); e->dirty = true; wrote |= 1; }

    // The forearm now hangs from the moved bicep: re-read it and aim the hook at the target from the real elbow.
    hs::Pose pE2, pH2;
    float err = -1.f;
    if (wrote && parF && reachNodePose(e->fore, e->poseFn, pE2) && reachNodePose(e->hand, e->poseFn, pH2) && reachNodePose(parF, e->poseFn, pPF)) {
        const float he[3] = {pH2.pos[0] - pE2.pos[0], pH2.pos[1] - pE2.pos[1], pH2.pos[2] - pE2.pos[2]};
        const float te[3] = {T[0] - pE2.pos[0], T[1] - pE2.pos[1], T[2] - pE2.pos[2]};
        const hs::Q outF = hs::blendQ(baseF, hs::reachDeltaToLocal(pPF.q, hs::arcQuat(he, te), baseF), w);
        if (reachWriteQ(e->fore, outF)) { hs::reachNoteWrite(e->fb, outF); wrote |= 2; }
        hs::Pose pH3;
        if (reachNodePose(e->hand, e->poseFn, pH3)) {
            const float d[3] = {pH3.pos[0] - T[0], pH3.pos[1] - T[1], pH3.pos[2] - T[2]};
            err = hs::r3len(d);
        }
    }
    // Wrist: turn the hand from the weapon's mount orientation (at the grab / the landing) to its resting orientation (at the rest end of the path), so the weapon swings
    // with the wrist instead of keeping the hand's orientation and clipping the head; limited to reach_wrist_max_deg, scaled by reach_wrist_pct.
    float wristDeg = 0.f, gDeg = -1.f;   // gDeg (HW24): angle between the mount orientation and the post-IK hand = what the wrist cap has to cover
    if (carry && wristOk && mountQ && wrote == 3 && g_reachWristPct > 0.f) {
        hs::Pose pHn, pWp;
        const uintptr_t parW = rdU32(kSitePlace, e->wrist, 0x3C);
        if (parW && reachNodePose(e->hand, e->poseFn, pHn) && reachNodePose(parW, e->poseFn, pWp)) {
            gDeg = hs::reachQAngleDeg(hs::qmul(*mountQ, hs::reachConj(pHn.q)));
            // HW26: the previous delta of this slide is the sign hint, so the wrist axis does not flip when the turn passes 180 deg
            const hs::Q dq = hs::reachWristDelta(*mountQ, pH.q, pathF, pHn.q, g_reachWristMax, w * g_reachWristPct, e->wristOn ? &e->wristDq : nullptr);
            const hs::Q outW = hs::reachDeltaToLocal(pWp.q, dq, baseW);
            if (reachWriteQ(e->wrist, outW)) { hs::reachNoteWrite(e->wb, outW); wrote |= 4; wristDeg = hs::reachQAngleDeg(dq); e->wristDq = dq; e->wristOn = 1; }
        }
    }
    // carry: the hand the weapon hangs on, after both writes (the attach node shares the pose slot with the arm parts, same guard as everywhere else)
    bool carriedOk = false;
    if (carry && carried && attachNode && (wrote & 3) == 3) {
        const uintptr_t avt = rdU32(kSitePlace, attachNode, 0);
        carriedOk = avt && rdU32(kSitePlace, avt, 0x0C) == e->poseFn && reachNodePose(attachNode, e->poseFn, *carried);
    }
    const uint8_t heldBefore = e->holdN;   // frames held just before this solve (0 = no hold episode)
    e->solved = static_cast<uint8_t>(wrote);
    e->holdN = 0;
    if (!reachLogDue()) return carriedOk;
    KP_LOG(chMain, "REACH owner=%x w=%.2f reach=%.2f S=(%.3f,%.3f,%.3f) E=(%.3f,%.3f,%.3f) H=(%.3f,%.3f,%.3f) T=(%.3f,%.3f,%.3f) Enew=(%.3f,%.3f,%.3f) side=%c cat=%d u=%.2f lt=(%.3f,%.3f,%.3f) err=%.3f persist=%d wrote=%d wr=%.0f hg=%d f=%.2f dzA=%.2f fy=%.2f fx=%.2f g=%.0f cap=%.0f hold=%u topo=%s bk=%d",
           static_cast<unsigned>(e->owner), static_cast<double>(w), static_cast<double>(reach), static_cast<double>(pS.pos[0]), static_cast<double>(pS.pos[1]),
           static_cast<double>(pS.pos[2]), static_cast<double>(pE.pos[0]), static_cast<double>(pE.pos[1]), static_cast<double>(pE.pos[2]), static_cast<double>(pH.pos[0]),
           static_cast<double>(pH.pos[1]), static_cast<double>(pH.pos[2]), static_cast<double>(T[0]), static_cast<double>(T[1]), static_cast<double>(T[2]),
           static_cast<double>(enew[0]), static_cast<double>(enew[1]), static_cast<double>(enew[2]), left ? 'L' : 'R', cat, static_cast<double>(u), static_cast<double>(lt[0]), static_cast<double>(lt[1]), static_cast<double>(lt[2]),
           static_cast<double>(err), persist ? 1 : 0, wrote, static_cast<double>(wristDeg), hinge ? 1 : 0, static_cast<double>(pathF), static_cast<double>(dzA), static_cast<double>(fyA), static_cast<double>(fxA),
           static_cast<double>(gDeg), static_cast<double>(g_reachWristMax), static_cast<unsigned>(heldBefore), reachTopoName(topo), backMount ? 1 : 0);
    return carriedOk;
}

// ---- HW26 carried-weapon pose: hand-frame residual ----
// The old carry blended the weapon from the mount to the carried hand in the WORLD frame over the first 15% of the carry (carryW), so a big grab angle showed as a spin. Now a
// fresh draw stores the weapon's offset from the carried hand (rotation Rres = conj(hand) * mount, position in the hand frame) at the first carry frame and the weapon rides
// the hand with that offset, which unwinds over reach_resid_ms (hs::reachResidual). A holster lands the same way: over the last reach_resid_ms of the carry the offset from the
// hand to the mount grows back in (measured every frame, so nothing is stored). Both windows are cut to the carry that is left, so the offset is gone at the slide end.
// A reversed slide (partial draw, or a stow that starts below full carry) keeps the old world blend: its carryW is already continuous with the weapon's last pose.
// Reversal continuity: every shown pose is also kept as an offset in the hand frame (lastQ / lastPos); when the slide reverses, the first frames cross-fade from that offset to the
// new direction's own pose (fade*), so the weapon never jumps whatever source (residual, landing window or world blend) was showing the frame before.
struct CarryResid { hs::Q q; float pos[3]; uint32_t startMs, msEff; uint8_t on;
                    hs::Q lastQ, seedQ; float lastPos[3], seedPos[3]; uint32_t lastMs, fadeStart, fadeMs; uint8_t lastDir, lastOn, lastCarry, fade; };   // on: a draw offset is captured; any frame without a fresh-draw carry (reach phase, stow, no residual) clears it
static hs::Pose carryShownBase(const hs::Slide& sl, hs::Slide& lock, CarryResid& cr, const hs::Pose& mount, const hs::Pose& carried, const hs::ReachPlan& pl, float grab, uint32_t now) {
    if (g_reachResidMs == 0 || !pl.active || !(pl.carryW > 0.f) || !hs::qfinite(carried.q)) { cr.on = 0; return hs::blendPoseLocked(lock, mount, carried, pl.carryW); }
    const hs::Q cq = hs::qnormalize(carried.q), inv = hs::reachConj(cq);
    const int32_t d = static_cast<int32_t>(now - sl.t0);
    const float resMs = static_cast<float>(g_reachResidMs);
    float t = 0.f, ms = resMs, off[3];
    hs::Q rres;
    if (sl.dir) {   // draw: fixed offset captured at the first carry frame
        if (sl.from > 0.001f) { cr.on = 0; return hs::blendPoseLocked(lock, mount, carried, pl.carryW); }
        if (!cr.on || static_cast<int32_t>(now - cr.startMs) > static_cast<int32_t>(sl.len) + 100) {   // not captured yet (or a stale capture from an earlier slide)
            const float dm[3] = {mount.pos[0] - carried.pos[0], mount.pos[1] - carried.pos[1], mount.pos[2] - carried.pos[2]};
            cr.q = hs::qmul(inv, mount.q);
            hs::qrotate(inv, dm, cr.pos);
            cr.startMs = now;
            const float left = static_cast<float>(static_cast<int32_t>(sl.len) - d);   // slide time that is left: the offset must be gone by then
            cr.msEff = static_cast<uint32_t>(left < resMs ? (left > 1.f ? left : 1.f) : resMs);
            cr.on = 1;
        }
        t = static_cast<float>(static_cast<int32_t>(now - cr.startMs));
        ms = static_cast<float>(cr.msEff);
        rres = cr.q;
        for (int i = 0; i < 3; ++i) off[i] = cr.pos[i];
    } else {        // holster: the offset to the mount, weighted by the time left until the carry ends
        cr.on = 0;
        if (!(sl.carryFrom >= 0.999f)) return hs::blendPoseLocked(lock, mount, carried, pl.carryW);
        const float gg = grab > 0.6f ? 0.6f : (grab > 0.f ? grab : 0.f);
        const float carryLen = (1.f - gg) * static_cast<float>(sl.len);
        ms = carryLen < resMs ? carryLen : resMs;
        t = carryLen - static_cast<float>(d);
        if (!(ms > 1.f) || t >= ms) return carried;   // outside the landing window the weapon is in the hand
        const float dm[3] = {mount.pos[0] - carried.pos[0], mount.pos[1] - carried.pos[1], mount.pos[2] - carried.pos[2]};
        rres = hs::qmul(inv, mount.q);
        hs::qrotate(inv, dm, off);
    }
    const float k = 1.f - hs::easeSmooth(t / ms);   // share of the offset that is still there
    float o3[3], w3[3];
    for (int i = 0; i < 3; ++i) o3[i] = off[i] * k;
    hs::qrotate(cq, o3, w3);
    hs::Pose o;
    for (int i = 0; i < 3; ++i) o.pos[i] = carried.pos[i] + w3[i];
    o.q = hs::qnormalize(hs::qmul(cq, hs::reachResidual(rres, t, ms)));
    if (!hs::poseSane(o)) { cr.on = 0; return hs::blendPoseLocked(lock, mount, carried, pl.carryW); }
    return o;
}

static hs::Pose carryShown(const hs::Slide& sl, hs::Slide& lock, CarryResid& cr, const hs::Pose& mount, const hs::Pose& carried, const hs::ReachPlan& pl, float grab, uint32_t now) {
    const hs::Pose base = carryShownBase(sl, lock, cr, mount, carried, pl, grab, now);
    if (!hs::qfinite(carried.q) || !hs::poseSane(base)) { cr.lastOn = cr.fade = 0; return base; }
    const hs::Q cq = hs::qnormalize(carried.q), inv = hs::reachConj(cq);
    const int32_t d = static_cast<int32_t>(now - sl.t0);
    const float resMs = static_cast<float>(g_reachResidMs);
    if (cr.lastOn && (cr.lastCarry || pl.carryW > 0.f) && resMs > 0.f && sl.dir != cr.lastDir && static_cast<int32_t>(now - cr.lastMs) <= 100) {   // the slide reversed since the last frame we showed
        cr.seedQ = cr.lastQ;
        for (int i = 0; i < 3; ++i) cr.seedPos[i] = cr.lastPos[i];
        const float gg = grab > 0.6f ? 0.6f : (grab > 0.f ? grab : 0.f);
        const float left = (sl.dir ? static_cast<float>(sl.len) : (1.f - gg) * static_cast<float>(sl.len)) - static_cast<float>(d);   // the fade must be over when the carry ends
        cr.fadeMs = static_cast<uint32_t>(left < resMs ? (left > 1.f ? left : 1.f) : resMs);
        cr.fadeStart = now;
        cr.fade = 1;
    }
    hs::Pose o = base;
    if (cr.fade) {
        const float t = static_cast<float>(static_cast<int32_t>(now - cr.fadeStart)), fm = static_cast<float>(cr.fadeMs);
        if (!(t >= 0.f) || !(t < fm)) cr.fade = 0;
        else {
            const float k = 1.f - hs::easeSmooth(t / fm);   // share of the pre-reversal offset that is still shown
            const float bd[3] = {base.pos[0] - carried.pos[0], base.pos[1] - carried.pos[1], base.pos[2] - carried.pos[2]};
            float np[3], fp[3], wp[3];
            hs::qrotate(inv, bd, np);
            for (int i = 0; i < 3; ++i) fp[i] = (1.f - k) * np[i] + k * cr.seedPos[i];
            hs::qrotate(cq, fp, wp);
            for (int i = 0; i < 3; ++i) o.pos[i] = carried.pos[i] + wp[i];
            o.q = hs::qnormalize(hs::qmul(cq, hs::blendQ(hs::qmul(inv, base.q), cr.seedQ, k)));
            if (!hs::poseSane(o)) o = base;
        }
    }
    const float od[3] = {o.pos[0] - carried.pos[0], o.pos[1] - carried.pos[1], o.pos[2] - carried.pos[2]};
    cr.lastQ = hs::qmul(inv, o.q);
    hs::qrotate(inv, od, cr.lastPos);
    cr.lastDir = sl.dir;
    cr.lastMs = now;
    cr.lastOn = 1;
    cr.lastCarry = pl.carryW > 0.f ? 1 : 0;   // both frames mount-fixed (reach / release phase): no fade, the base already stays on the mount
    return o;
}

// ---- editor preview (k2.holster v1, HE2): registered preview Gobs are placed from the full resolve, or from the open edit overlay ----
// Shared between the editor API (the holster-editor panel tick) and HolsterPlace; g_edLock guards the registry, the overlay and the
// edited Gob's mount pose. Engine calls (FindPart, node pose) run outside the lock.
static SRWLOCK g_edLock = SRWLOCK_INIT;
static hs::PreviewRegistry g_preview;
static volatile LONG g_previewN = 0;     // g_preview.n mirror for the lock-free "no preview" test on every placement
static hs::EditOverlay g_edit;
static void* g_editGob = nullptr;
static hs::Pose g_editMount;              // world pose of the edited Gob's mount bone, refreshed by every placement of that Gob
static bool g_editMountOk = false;
static hs::Slide g_prevSlide;             // HW20: the editor's Draw / Holster toggle: one slide shared by every preview weapon (hand fraction, 0 = mount); guarded by g_edLock
static float g_prevGrab = 0.30f;          // HW28: grab share the shared slide was retargeted with (the category of the edited weapon at the toggle); the readers must pass the same value
// HW26: one hemisphere lock (a Slide used only for lockH / lockOn) and one hand-frame residual per preview Gob; the shared g_prevSlide can not carry a lock for several weapons.
struct PrevLock { uintptr_t gob; hs::Slide lk; CarryResid cres; };
static PrevLock g_prevLock[8];
static unsigned g_prevLockNext = 0;
static PrevLock& prevLockFor(uintptr_t gob) {   // caller holds g_edLock
    for (PrevLock& p : g_prevLock)
        if (p.gob == gob) return p;
    PrevLock& p = g_prevLock[g_prevLockNext];
    g_prevLockNext = (g_prevLockNext + 1) % 8u;
    p = PrevLock();
    p.gob = gob;
    return p;
}
static uint32_t slideNowMs();
static volatile uintptr_t g_refFind = 0, g_refPose = 0;   // FindPart (Gob vtbl+0x10C) / node pose (vtbl+0xC) slots seen on a placed weapon

// In-world hand mapping is active: OnHandAttach records gob -> hand (R/L) into g_handRing,
// and HolsterPlace reads handLookup(g_handRing, g) to resolve left-hand placement and sagittal mirroring.
static const bool kWorldKnowsHand = true;
// The editor preview resolves the hand as it is (S2/H4, 10-18): a left preview weapon is always the left resolve (mirror + left_extra_rot, or the
// explicit l_ keys), whatever bone_test_left says. bone_test_left=0 only keeps the in-world lhand attach node unpatched (that weapon stays in the
// hand and HolsterPlace refuses it as not-target); it no longer changes what the editor shows. The old gate (left && g_boneLeft) resolved a left
// preview as the right hand, so the two weapons overlapped at the same spot and angle.
static bool effLeft(bool left) {
    return left;
}

struct EdLock {
    EdLock() { AcquireSRWLockExclusive(&g_edLock); }
    ~EdLock() { ReleaseSRWLockExclusive(&g_edLock); }
};

// Model name of the Gob (16 chars max, the tune table limit) and body model name of its parent ([parent+0x84]+0x08), "" when unknown.
static void gobNames(uintptr_t gob, uintptr_t parent, char model[17], char body[17]) {
    char m[32], b[32];
    gobModelName(gob, m, sizeof m);
    gobModelName(parent, b, sizeof b);
    hs::tuneCopyName(model, m);
    hs::tuneCopyName(body, b);
}

// Places a registered preview Gob. Returns false when the Gob is not a preview (in-world path continues), true when handled.
static bool previewPlace(uintptr_t g, uintptr_t beh) {
    if (!g_editor || g_previewN == 0 || !g || !beh) return false;
    saferead::beginScope();
    const uintptr_t parent = rdU32(kSitePlace, beh, 0x14), n = rdU32(kSitePlace, beh, 0x18);
    bool left = false, editing = false, moved = false;
    hs::Resolved r;
    {
        EdLock lk;
        const int i = hs::previewFind(g_preview, reinterpret_cast<void*>(g));
        if (i < 0) return false;
        hs::PreviewEntry& e = g_preview.e[i];
        if (e.parent != reinterpret_cast<void*>(parent) || e.node != reinterpret_cast<void*>(n)) {
            moved = e.parent != nullptr;
            e.parent = reinterpret_cast<void*>(parent);
            e.node = reinterpret_cast<void*>(n);
        }
        left = effLeft(e.left);
        editing = g_edit.active && g_editGob == reinterpret_cast<void*>(g);
        if (editing) {
            hs::EditView v;
            hs::overlayGetEdit(g_edit, v);
            r = v.r;
        }
    }
    if (moved && takeEditLine()) KP_LOG(chEdit, "PREVIEW reattach gob=%x parent=%x node=%x", static_cast<unsigned>(g), static_cast<unsigned>(parent), static_cast<unsigned>(n));
    static hs::TuneCfg s_tune;
    static LONG s_seq = 0;
    (void)loadTuneIfChanged(s_tune, s_seq);
    char model[17], body[17];
    gobNames(g, parent, model, body);
    const hs::Category cat = hs::classify(model, 0);
    if (!editing) hs::resolveFull(s_tune, model, cat, body, left, false, r);
    const uint8_t boneIdx = r.boneIdx ? r.boneIdx : hs::defaultBone(s_tune);
    const char* const bone = hs::boneNameOf(boneIdx) ? hs::boneNameOf(boneIdx) : g_boneName;
    const uintptr_t gvt = rdU32(kSitePlace, g, 0), nvt = n ? rdU32(kSitePlace, n, 0) : 0;
    if (gvt) g_refFind = rdU32(kSitePlace, gvt, 0x10C);
    if (nvt) g_refPose = rdU32(kSitePlace, nvt, 0x0C);
    static volatile LONG s_lines = 0;
    const bool logIt = InterlockedIncrement(&s_lines) <= 16 && takeEditLine();
    hs::Pose base;
    if (!n || !mountPose(g, n, beh, cat, bone, base)) {
        if (logIt) KP_LOG(chEdit, "PREVIEW nomount gob=%x model=%s body=%s bone=%s left=%d", static_cast<unsigned>(g), model, body, bone, left ? 1 : 0);
        return true;
    }
    hs::Placement pl;
    for (int k = 0; k < 3; ++k) { pl.rotDeg[k] = r.rot[k]; pl.pos[k] = r.off[k]; }
    const hs::Pose out = hs::applyPlacement(base, pl);
    if (editing) {
        EdLock lk;
        if (g_editGob == reinterpret_cast<void*>(g)) { g_editMount = base; g_editMountOk = true; }
    }
    // HW20: the editor's Draw / Holster toggle. The engine just wrote the HAND pose into the Gob (the preview weapon hangs on rhand / lhand), so it is the draw end of
    // the slide; the mount pose `out` is the stow end. Same blend and the same arm reach as in the world (reach_test), so the preview shows what the game does.
    float slideF = 0.f, reachW = 0.f;
    hs::ReachPlan plan = {false, 0.f, 0.f, 1.f};
    hs::Slide pslide;   // copy of g_prevSlide for the residual timing
    const uint32_t pnow = slideNowMs();
    {
        const uint32_t now = pnow;
        EdLock lk;
        if (g_prevSlide.init) {
            g_prevSlide.touch(now);
            slideF = g_prevSlide.frac(now);
            reachW = hs::reachWeightRamp(g_prevSlide, now, g_reachCarry ? 0.04f : 0.2f);
            plan = g_reachCarry ? hs::reachPlan(g_prevSlide, now, g_prevGrab) : hs::ReachPlan{g_prevSlide.active(now), slideF, 0.f, 1.f};
            pslide = g_prevSlide;
        }
    }
    hs::Pose shown = out;
    bool atHand = false;
    if (slideF > 0.f) {
        hs::Pose in;
        if (saferead::readAt(kSitePlace, g, 0xA4, &in) && hs::poseSane(in)) {
            if (slideF >= 1.f) atHand = true;
            else if (g_reachTest) { EdLock lk; shown = hs::blendPoseLocked(prevLockFor(g).lk, out, in, hs::easeSmooth(slideF)); }   // HW26 (reach_test only: off keeps the plain blend)
            else shown = hs::blendPose(out, in, hs::easeSmooth(slideF));
        }
    }
    hs::Pose carried;
    const bool carryOk = g_reachTest && reachArm(g, beh, atHand ? out : shown, reachW, left, static_cast<int>(cat), plan.pathF, plan.restMix, out.pos, &out.q, reachIsBackBone(bone), n, g_reachCarry && plan.active, &carried);   // w == 0 outside the slide: only undoes a leftover write
    if (atHand) return true;   // fully drawn: the engine's hand pose stays
    if (carryOk) { EdLock lk; PrevLock& pk = prevLockFor(g); shown = carryShown(pslide, pk.lk, pk.cres, out, carried, plan, g_prevGrab, pnow); }   // HW26: hand-frame residual / locked blend; carry: the weapon rides the moved hand only while it carries (plan.carryW); it stays on the mount during the reach / release phases
    if (!hs::poseSane(shown) || !saferead::probeWrite(kSitePlace, reinterpret_cast<void*>(g + 0xA4), sizeof(hs::Pose))) return true;
    memcpy(reinterpret_cast<void*>(g + 0xA4), &shown, sizeof(hs::Pose));
    if (logIt)
        KP_LOG(chEdit, "PREVIEW place gob=%x model=%s cat=%s body=%s left=%d bone=%s edit=%d rot=(%.1f,%.1f,%.1f) off=(%.3f,%.3f,%.3f) src=%u/%u/%u lsrc=%s extra=%.1f",
               static_cast<unsigned>(g), model, hs::categoryName(cat), body[0] ? body : "-", left ? 1 : 0, bone, editing ? 1 : 0, r.rot[0], r.rot[1], r.rot[2],
               r.off[0], r.off[1], r.off[2], r.src[0], r.src[1], r.src[2], hs::leftSrcName(r, left), static_cast<double>(r.extraDeg));
    return true;
}

// DRAWGATE transition line (drawgate mode): a placement entry's owner or hip/drawn state changed. Quiet mode: first 8 per creature, then every 10th
// (log_verbose=1: all). An unreadable owner / combat flag stays a WARN (C1 review #9: rate-limited per gob, the first 3 hits then every 50th, so it is never
// swallowed entirely; log_verbose=1 logs all).
static void gateLog(uintptr_t gob, uintptr_t owner, bool readable, uint32_t combat, bool hip, bool left) {   // flourish= window open for the owner (flourishOpenFor)
    if (!readable) {
        if (!g_logVerbose && !hs::keyedWant(hs::keyBump(g_gateWarnKeys, hs::creatureKey(gob)), 3, 50)) return;
        KP_LOGV(chMain, KPLOG_WARN, "DRAWGATE owner/combat unreadable gob=%x owner=%x: weapon stays in the hand", static_cast<unsigned>(gob), static_cast<unsigned>(owner));
        return;
    }
    if (!g_onPlace) return;
    if (!g_logVerbose && !hs::keyedWant(hs::keyBump(g_gateKeys, hs::creatureKey(owner)), 8, 10)) return;
    if (!KP_ENABLED(chPlace, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chPlace, "DRAWGATE creature=%x gob=%x state=%s combat=%u flourish=%d left=%d", static_cast<unsigned>(owner), static_cast<unsigned>(gob),
           hip ? "hip" : "drawn", static_cast<unsigned>(combat), (g_flourishDraw && readable && combat == 0 && flourishOpenFor(owner)) ? 1 : 0, left ? 1 : 0);
}

// Millisecond tick for the draw/stow slide (HolsterSlide.h; wrap-safe 32-bit compares): QueryPerformanceCounter, frequency cached once.
static uint32_t slideNowMs() {
    static LONGLONG s_fq = 0;
    if (!s_fq) {
        LARGE_INTEGER f;
        f.QuadPart = 0;
        QueryPerformanceFrequency(&f);
        s_fq = f.QuadPart > 0 ? f.QuadPart : 1;
    }
    LARGE_INTEGER c;
    c.QuadPart = 0;
    QueryPerformanceCounter(&c);
    const unsigned long long q = static_cast<unsigned long long>(c.QuadPart), fq = static_cast<unsigned long long>(s_fq);
    return static_cast<uint32_t>((q / fq) * 1000ull + (q % fq) * 1000ull / fq);
}

// H7 (hooks.toml H7, C4 recon): runs on EVERY per-frame refresh of an attach behavior (vtbl+0x08), right after the engine re-copied node N's WORLD
// pose into gob+0xA4 (vec3) / gob+0xB0 (quat w-first). N = [beh+0x18]. For the target node (g_boneName) it composes the live-tuned placement onto
// that pose and writes it back. Needs probe=1 AND bone_test=1 AND place=1; ch_place only gates the PLACE/PLACENODE/PLACEREFUSE log lines.
// place_mode=drawgate (default): the target nodes are the native rhand/lhand attach nodes (no code patch); the hip pose (mountPose of the configured
// bone, always computed here) is written only while the owner is not in combat and no flourish window is open, else the hand pose stays (HolsterDrawGate.h).
extern "C" void __cdecl HolsterPlace(void* gob, void* beh) {
    handMapLog(reinterpret_cast<uintptr_t>(gob));
    if (previewPlace(reinterpret_cast<uintptr_t>(gob), reinterpret_cast<uintptr_t>(beh))) return;
    if (!g_probe) return;
    holsterTick();   // per frame per weapon Gob (H7); HolsterGobUpdate (H6) only fires on attach events, so the key poll and delay tick must also run here (s_busy + tickDue throttle it)
    static volatile LONG s_tid = 0;
    if (InterlockedCompareExchange(&s_tid, 1, 0) == 0) KP_LOG(chMain, "TID place=%lu", static_cast<unsigned long>(GetCurrentThreadId()));
    previewProbe(reinterpret_cast<uintptr_t>(gob), reinterpret_cast<uintptr_t>(beh));
    if (!g_boneTest) return;
    static hs::TuneCfg s_tune;
    static LONG s_seq = 0;
    (void)loadTuneIfChanged(s_tune, s_seq);
    if (!s_tune.place) return;
    saferead::beginScope();
    void* const node = beh ? reinterpret_cast<void*>(static_cast<uintptr_t>(rdU32(kSitePlace, reinterpret_cast<uintptr_t>(beh), 0x18))) : nullptr;
    // Verdict cache keyed on node + gob + the node's name handle [N+4], so a freed and reused address pair is re-resolved. The full
    // resolve (weapon > type > global, plus the body delta) is cached with the tune sequence it was computed for and redone when it moves.
    // Repoint: both hands share the bone_name node; the hand comes from g_handRing (sticky via hs::resolveLeft when evicted). Drawgate: the node name (lhand) decides.
    struct Ent { uintptr_t node, gob, handle, model; bool isTarget, left; hs::Category cat; char mname[17], body[17]; LONG seq; hs::Resolved r; hs::GateState gate; hs::Slide slide; uint32_t slFrames = 0, slMaxDt = 0, slPrev = 0; uint8_t graceN = 0; CarryResid cres = {}; };   // graceN: HW25 hard-reset grace, frames held so far; cres: HW26 hand-frame residual of the running carry
    static Ent s_cache[128];
    static unsigned s_next = 0;
    const uintptr_t g = reinterpret_cast<uintptr_t>(gob), n = reinterpret_cast<uintptr_t>(node);
    if (!g || !n) { refuse(PrNull, g, n, 0); return; }
    const uintptr_t h = rdU32(kSitePlace, n, 0x04);
    if (!h) { refuse(PrNoHandle, g, n, 0); return; }
    bool isTarget = false;
    hs::Category cat = hs::CatUnknown;
    Ent* ent = nullptr;
    Ent* stale = nullptr;   // (node, gob, handle) matched but the weapon model differs: a reused Gob address, entry is rebuilt in place
    const uintptr_t liveModel = rdU32(kSitePlace, g, 0x84);
    for (Ent& e : s_cache)
        if (e.node == n && e.gob == g && e.handle == h) {
            if (hs::sameWeaponModel(e.model, liveModel)) { isTarget = e.isTarget; cat = e.cat; ent = &e; break; }
            stale = &e;
        }
    if (!ent && stale) {
        static volatile LONG s_reuseLines = 0;
        if (g_onPlace && InterlockedIncrement(&s_reuseLines) <= 20 && KP_ENABLED(chPlace, KPLOG_INFO) && takeLine()) {
            char newm[32];
            newm[0] = 0;
            if (liveModel) readTag(kSitePlace, reinterpret_cast<const char*>(liveModel + 0x08), newm, sizeof newm);
            KP_LOG(chPlace, "GOBREUSE gob=%x node=%x oldm=%s newm=%s", static_cast<unsigned>(g), static_cast<unsigned>(n), stale->mname[0] ? stale->mname : "-",
                   newm[0] ? newm : "-");
        }
    }
    if (ent) {
        // Repoint: sticky ring resolve (unchanged). Drawgate: the hand was fixed from the node name at creation and never changes (same node, same entry).
        if (!drawgateMode()) {
            const bool nl = hs::handFromNode(false, nullptr, hs::handLookup(g_handRing, g), ent->left);
            if (nl != ent->left) {
                ent->left = nl;
                ent->seq = -1;
            }
        }
    } else {
        NodeInfo ni;
        isTarget = nodeIsTarget(n, ni);
        // Weapon identity (C5, live-verified 10-16: "w_Lngswrd_001", "w_SonicPstl_002"): [Gob+0x84] = model, model+0x08 = inline model name,
        // classified by the itemclass stem table. Gob+0x64 = inline Gob name (log only). Unknown stem -> CatUnknown -> global place_rot / place_off.
        char mname[32];
        mname[0] = 0;
        if (liveModel) readTag(kSitePlace, reinterpret_cast<const char*>(liveModel + 0x08), mname, sizeof mname);
        cat = hs::classify(mname, 0);
        if (stale) {
            ent = stale;
        } else {
            ent = &s_cache[s_next];
            s_next = (s_next + 1) % 128u;
        }
        *ent = Ent{};
        ent->model = liveModel;
        ent->gate = hs::gateInit();   // drawgate: owner + hip/drawn state, "not evaluated" until the first placement
        ent->node = n; ent->gob = g; ent->handle = h; ent->isTarget = isTarget; ent->cat = cat;
        {
            // Drawgate: node name decides (C1 review B2); the ring only cross-checks. Repoint: ring resolve exactly as before.
            const int ring = hs::handLookup(g_handRing, g);
            ent->left = hs::handFromNode(drawgateMode(), ni.name, ring, false);
            static volatile LONG s_handDisagree = 0;
            if (drawgateMode() && isTarget && ring != hs::HandNone && (ring == hs::HandL) != ent->left && g_onPlace && InterlockedIncrement(&s_handDisagree) <= 5 &&
                KP_ENABLED(chPlace, KPLOG_INFO) && takeLine())
                KP_LOG(chPlace, "HANDCHK node=%s ring=%c gob=%x: node name wins (left=%d)", ni.name[0] ? ni.name : "-", hs::handChar(ring), static_cast<unsigned>(g), ent->left ? 1 : 0);
        }
        ent->seq = -1;   // never a real sequence (those are >= 0), so the first use resolves
        {
            const uintptr_t par = rdU32(kSitePlace, reinterpret_cast<uintptr_t>(beh), 0x14);
            char m17[17], b17[17];
            gobNames(g, par, m17, b17);
            memcpy(ent->mname, m17, sizeof m17);
            memcpy(ent->body, b17, sizeof b17);
        }
        static volatile LONG s_nodeLines = 0;   // one line per newly resolved (node, gob) pair, first 40
        if (g_onPlace && InterlockedIncrement(&s_nodeLines) <= 40 && KP_ENABLED(chPlace, KPLOG_INFO) && takeLine()) {
            char gname[32], body[32];
            readTag(kSitePlace, reinterpret_cast<const char*>(g + 0x64), gname, sizeof gname);
            // C7f probe: parent Gob [beh+0x14] -> +0x1D8 = owner CSWCCreature* (set in LoadModel 0x769F60; compare with COMB creature=),
            // [parent+0x84]+0x08 = body model name (per-body offsets, doc 42 HW-EDIT). Unverified that the parent is the body Gob.
            const uintptr_t parent = rdU32(kSitePlace, reinterpret_cast<uintptr_t>(beh), 0x14);
            const uintptr_t owner = parent ? rdU32(kSitePlace, parent, 0x1D8) : 0;
            const uintptr_t bmodel = parent ? rdU32(kSitePlace, parent, 0x84) : 0;
            body[0] = 0;
            if (bmodel) readTag(kSitePlace, reinterpret_cast<const char*>(bmodel + 0x08), body, sizeof body);
            KP_LOG(chPlace, "PLACENODE node=%x gob=%x t=%u step=%d name=%s target=%d bvt=%x gname=%s mname=%s cat=%s parent=%x owner=%x body=%s left=%d",
                   static_cast<unsigned>(n), static_cast<unsigned>(g), ni.t, ni.step, ni.name[0] ? ni.name : "-", isTarget ? 1 : 0,
                   static_cast<unsigned>(rdU32(kSitePlace, reinterpret_cast<uintptr_t>(beh), 0)), gname[0] ? gname : "-", mname[0] ? mname : "-",
                   hs::categoryName(cat), static_cast<unsigned>(parent), static_cast<unsigned>(owner), body[0] ? body : "-", ent->left ? 1 : 0);
        }
    }
    if (!isTarget) { refuse(PrNotTarget, g, n, 0); return; }
    const uint32_t vt = rdU32(kSitePlace, g, 0);
    if (g_placeGobvt != 0 && vt != g_placeGobvt) { refuse(PrGobvt, g, n, vt); return; }
    // Drawgate target filter (C1 review B1): only weapon Gobs (model "w_...") are placed; menu previews / held props / hand VFX on rhand/lhand stay put.
    // Repoint mode is unaffected (its targets are the patched attach site's weapons only).
    if (drawgateMode() && !hs::isWeaponModel(ent->mname)) { refuse(PrNotWeapon, g, n, vt); return; }
    // HW27 mount sense: creature weapons (w_cr*) and unclassified weapons are never put on a mount (no slide, no reach, no arm write): they stay in the hand as the engine drew them.
    if (drawgateMode() && !hs::holsterable(cat, ent->mname)) { refuse(PrNotHolsterable, g, n, vt); return; }
    float slideF = 0.f;   // hand fraction of the draw/stow slide (0 = hip); only the drawgate slide path sets it
    if (drawgateMode()) {
        // Draw gate (C1): owner creature = [parent+0x1D8]; drawn = client combat flag [owner+0x138] != 0 or a flourish window. Drawn (or anything
        // unreadable) returns with the hand pose the engine just wrote: the failure mode is a weapon in the hand (vanilla), never stuck on the hip.
        const uintptr_t par = rdU32(kSitePlace, reinterpret_cast<uintptr_t>(beh), 0x14);
        const uintptr_t owner = par ? rdU32(kSitePlace, par, 0x1D8) : 0;
        const int combatOff = k2addr::off(&K2AddrTable::Creature_ClientCombat);
        uint32_t combat = 0;
        const bool readable = owner != 0 && combatOff != 0 && rdU32Ok(kSitePlace, owner, combatOff, &combat);
        const bool hip = hs::shouldWriteHip(true, readable, combat,
                                        hs::holdActive(owner, g_holdOwner, g_holdUntil, GetTickCount()) || (g_flourishDraw && readable && combat == 0 && flourishOpenFor(owner)) ||
                                        (g_partyCombatHold && g_partyCombat != 0 && ownerIsParty(owner)));
        if (readable) {   // HW15 probe: track which owners have a weapon on the hip (read by the ANIMRES line)
            if (hip) { const uintptr_t hb = rdU32(kSitePlace, owner, 0x68); if (hb) g_holSet.touch(owner, hb, GetTickCount()); }
            else g_holSet.remove(owner);
        }
        const bool first = ent->gate.hip < 0 || ent->gate.owner != owner;   // not evaluated yet, or a new owner: placed silently, no slide
        const uint32_t now = slideNowMs();
        // HW25 (reach_test only): an owner / combat read that fails for a few frames while a slide runs must not snap the weapon to the hand. The running slide is
        // kept (touched, no retarget, no reset) for up to 3 frames and the gate is not updated, so recovery sees the same owner and no first-placement snap.
        const bool grace = g_reachTest && hs::slideGraceHold(readable, ent->slide.active(now), ent->graceN);
        if (!grace && hs::gateUpdate(ent->gate, owner, hip)) {
            ent->seq = -1;   // state flip or new owner: the resolved placement is redone
            gateLog(g, owner, readable, combat, hip, ent->left);
        }
        const hs::SlideRes sres = hs::slideResolve(g_slideTune, cat);   // HW28: this weapon's own draw / stow ms and grab percent
        const float grab = static_cast<float>(sres.grabPct) / 100.f;
        if (sres.drawMs || sres.stowMs) {
            // HW14: the gate only picks the slide's target; the pose is blended at the end. f == 1 is the hand (identical to the plain `if (!hip) return`).
            if (grace) {
                ent->slide.touch(now);
            } else if (first || !readable) {
                static volatile LONG s_cutLines = 0;   // HW24: a hard reset that lands on a running slide is the visible pop; say why
                if (ent->slide.active(now) && g_onPlace && KP_ENABLED(chPlace, KPLOG_INFO) && InterlockedIncrement(&s_cutLines) <= 40 && takeLine())
                    KP_LOG(chPlace, "SLIDE cut reason=%s owner=%x gob=%x left=%d dir=%s frac=%.2f", readable ? "first" : "unreadable",
                           static_cast<unsigned>(owner), static_cast<unsigned>(g), ent->left ? 1 : 0, ent->slide.dir ? "draw" : "stow", static_cast<double>(ent->slide.frac(now)));
                ent->slide.reset(!hip, now);
                ent->cres = CarryResid();
                ent->slFrames = 0;
            } else {
                const uint8_t dirBefore = ent->slide.dir;
                if (!ent->slide.active(now)) { ent->slide.lockOn = 0; ent->cres = CarryResid(); }   // HW26: a slide that starts from rest picks its hemisphere afresh; only a reversal of a running slide keeps the lock
                ent->slide.touch(now);
                if (g_reachTest && g_reachCarry) hs::reachRetarget(ent->slide, !hip, now, sres.drawMs, sres.stowMs, grab, 0.04f);   // HW25: the reversal keeps path, carry and arm weight continuous
                else ent->slide.target(!hip, now, sres.drawMs, sres.stowMs);
                static volatile LONG s_slideLines = 0;
                const bool logOk = g_onPlace && KP_ENABLED(chPlace, KPLOG_INFO);
                if (ent->slide.dir != dirBefore) {
                    ent->slFrames = 0;
                    if (ent->slide.len > 0) {
                        ent->slFrames = 1; ent->slMaxDt = 0; ent->slPrev = now;
                        if (logOk && InterlockedIncrement(&s_slideLines) <= 60 && takeLine())
                            KP_LOG(chPlace, "SLIDE start dir=%s owner=%x gob=%x left=%d from=%.2f ms=%u", ent->slide.dir ? "draw" : "stow", static_cast<unsigned>(owner),
                                   static_cast<unsigned>(g), ent->left ? 1 : 0, static_cast<double>(ent->slide.from), static_cast<unsigned>(ent->slide.len));
                    }
                } else if (ent->slFrames) {
                    if ((int32_t)(now - ent->slide.t0) < (int32_t)ent->slide.len) {   // still running (not yet at the end tick)
                        const uint32_t dt = now - ent->slPrev;
                        ++ent->slFrames;
                        if (dt > ent->slMaxDt) ent->slMaxDt = dt;
                        ent->slPrev = now;
                    } else {
                        if (logOk && InterlockedIncrement(&s_slideLines) <= 60 && takeLine())
                            KP_LOG(chPlace, "SLIDE end dir=%s owner=%x ms=%u frames=%u maxdt=%u", ent->slide.dir ? "draw" : "stow", static_cast<unsigned>(owner),
                                   static_cast<unsigned>(ent->slide.len), static_cast<unsigned>(ent->slFrames), static_cast<unsigned>(ent->slMaxDt));
                        ent->slFrames = 0;
                    }
                }
            }
            slideF = ent->slide.frac(now);
            if (slideF >= 1.f) {
                if (g_reachTest) { const hs::Pose none = {}; reachArm(g, reinterpret_cast<uintptr_t>(beh), none, 0.f, ent->left, 0, 0.f, 0.f, none.pos, nullptr, false); }   // HW19: undo a leftover arm write
                return;
            }
        } else if (!hip) return;
    }
    hs::Pose in;
    if (!saferead::readAt(kSitePlace, g, 0xA4, &in)) { refuse(PrReadIn, g, n, vt); return; }
    if (!hs::poseSane(in)) { refuse(PrInsaneIn, g, n, vt); return; }
    if (ent->seq != s_seq) {
        hs::resolveFull(s_tune, ent->mname, cat, ent->body, ent->left, false, ent->r);
        ent->seq = s_seq;
    }
    hs::Placement pl;
    for (int k = 0; k < 3; ++k) { pl.rotDeg[k] = ent->r.rot[k]; pl.pos[k] = ent->r.off[k]; }
    hs::Pose base = in;   // place_bone_<...>: compose on that bone's world pose instead of the attach node's
    // No bone at any level -> defaultBone (bone_name's own index, so the weapon stays on the attach node); 0 = custom bone_name, same.
    const uint8_t boneIdx = ent->r.boneIdx ? ent->r.boneIdx : hs::defaultBone(s_tune);
    const char* const mountBone = hs::boneNameOf(boneIdx);
    if (drawgateMode()) {
        // The node is the hand, so the hip pose always comes from mountPose of the configured bone (a custom bone_name has no mount: refused, weapon stays in the hand).
        if (!mountBone || !mountPose(g, n, reinterpret_cast<uintptr_t>(beh), cat, mountBone, base)) {
            refuse(PrNoMount, g, n, vt);
            return;
        }
    } else if (mountBone && _stricmp(mountBone, g_boneName) != 0 && !mountPose(g, n, reinterpret_cast<uintptr_t>(beh), cat, mountBone, base)) {
        refuse(PrNoMount, g, n, vt);
        return;
    }
    const hs::Pose hipPose = hs::applyPlacement(base, pl);
    // Mid-slide: ease between the hip pose and the engine's hand pose `in` (both world poses of this node slot; nothing accumulates, the engine rewrites it each frame).
    // HW26 (reach_test only: off keeps the plain blend): blendPoseLocked picks the hand's hemisphere once per slide instead of per frame
    const hs::Pose out = slideF > 0.f ? (g_reachTest ? hs::blendPoseLocked(ent->slide, hipPose, in, hs::easeSmooth(slideF)) : hs::blendPose(hipPose, in, hs::easeSmooth(slideF))) : hipPose;
    if (!hs::poseSane(out)) { refuse(PrInsaneOut, g, n, vt); return; }
    if (!saferead::probeWrite(kSitePlace, reinterpret_cast<void*>(g + 0xA4), sizeof(hs::Pose))) { refuse(PrNoWrite, g, n, vt); return; }
    memcpy(reinterpret_cast<void*>(g + 0xA4), &out, sizeof(hs::Pose));
    if (g_reachTest && drawgateMode()) {   // HW19 probe
        const uint32_t rnow = slideNowMs();
        const float rgrab = grabFor(cat);
        const hs::ReachPlan pl = g_reachCarry ? hs::reachPlan(ent->slide, rnow, rgrab) : hs::ReachPlan{ent->slide.active(rnow), slideF, 0.f, 1.f};
        const float wa = hs::reachWeightRamp(ent->slide, rnow, g_reachCarry ? 0.04f : 0.2f);
        hs::Pose carried;
        if (reachArm(g, reinterpret_cast<uintptr_t>(beh), out, wa, ent->left, static_cast<int>(cat), pl.pathF, pl.restMix, hipPose.pos, &hipPose.q, reachIsBackBone(mountBone), n, g_reachCarry && pl.active, &carried)) {
            // carry: the weapon rides the moved hand (the engine's hand pose, recomputed after the arm write), pl.carryW: 0 while the hand is still reaching the mount (draw) or already releasing it (stow), 1 while it carries; at the draw end the arm has released, so the carried hand IS the engine hand
            const hs::Pose rid = carryShown(ent->slide, ent->slide, ent->cres, hipPose, carried, pl, rgrab, rnow);   // HW26: hand-frame residual (or the locked world blend)
            if (pl.carryW > 0.f && pl.carryW < 1.f && g_onPlace && KP_ENABLED(chPlace, KPLOG_INFO)) {   // HW24: residual between the carried hand pose and the mount at the carryW step, the flip dot, the style byte
                static volatile LONG s_resLines = 0;
                if (InterlockedIncrement(&s_resLines) <= 40 && takeLine()) {
                    const float dp[3] = {carried.pos[0] - hipPose.pos[0], carried.pos[1] - hipPose.pos[1], carried.pos[2] - hipPose.pos[2]};
                    uint8_t style = 0;
                    const uintptr_t sPar = rdU32(kSitePlace, reinterpret_cast<uintptr_t>(beh), 0x14);
                    if (sPar) (void)saferead::readAt(kSitePlace, sPar, 0xC5, &style);
                    KP_LOG(chPlace, "RESID gob=%x left=%d dir=%s carryW=%.2f f=%.2f pos_cm=%.1f ang=%.0f dq=%.3f style=%u", static_cast<unsigned>(g), ent->left ? 1 : 0,
                           ent->slide.dir ? "draw" : "stow", static_cast<double>(pl.carryW), static_cast<double>(pl.pathF), static_cast<double>(hs::r3len(dp) * 100.f),
                           static_cast<double>(hs::reachQAngleDeg(hs::qmul(carried.q, hs::reachConj(hipPose.q)))), static_cast<double>(hs::qdot(hipPose.q, carried.q)),
                           static_cast<unsigned>(style));
                }
            }
            if (hs::poseSane(rid) && saferead::probeWrite(kSitePlace, reinterpret_cast<void*>(g + 0xA4), sizeof(hs::Pose))) memcpy(reinterpret_cast<void*>(g + 0xA4), &rid, sizeof(hs::Pose));
        }
    }
    if (!g_onPlace) return;
    const LONG c = InterlockedIncrement(&g_placeCalls);
    if (g_logVerbose) {
        if (c > 24 && (c % g_gobEvery) != 0) return;
    } else {
        // Quiet mode: per-gob sampler. The signature covers (cat, left, src); disp/ang are the placement delta (invariant under creature motion).
        const float pin[3] = {in.pos[0], in.pos[1], in.pos[2]}, pout[3] = {out.pos[0], out.pos[1], out.pos[2]};
        const float qin[4] = {in.q.w, in.q.x, in.q.y, in.q.z}, qout[4] = {out.q.w, out.q.x, out.q.y, out.q.z};
        uint32_t sig = hs::fnv1a(hs::leftSrcName(ent->r, ent->left), static_cast<uint32_t>(cat) * 2u + (ent->left ? 1u : 0u));
        sig = hs::fnv1a(hs::categoryName(cat), sig);
        if (!hs::placeWant(g_placeTab, g, sig, hs::vecDist(pin, pout), hs::quatAngleDeg(qin, qout))) return;
    }
    if (!KP_ENABLED(chPlace, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chPlace, "PLACE n=%ld gob=%x node=%x beh=%x gobvt=%x pos0=(%.3f,%.3f,%.3f) quat0=(%.3f,%.3f,%.3f,%.3f) pos1=(%.3f,%.3f,%.3f) quat1=(%.3f,%.3f,%.3f,%.3f) cat=%s left=%d src=%s extra=%.1f",
           static_cast<long>(c), U(gob), U(node), U(beh), static_cast<unsigned>(vt), in.pos[0], in.pos[1], in.pos[2], in.q.w, in.q.x, in.q.y, in.q.z,
           out.pos[0], out.pos[1], out.pos[2], out.q.w, out.q.x, out.q.y, out.q.z, hs::categoryName(cat), ent->left ? 1 : 0, hs::leftSrcName(ent->r, ent->left),
           static_cast<double>(ent->r.extraDeg));
}

// ---- k2.holster v1 (HolsterApi.h): the holster-editor patch drives these from its panel tick (main thread) ----
// The overlay keeps the edited weapon's identity (model, type, body, hand); BeginEdit reads it from the registered Gob.

static void edBegin(const hs::TuneCfg& c, const char* model, hs::Category cat, const char* body, bool left, int scope) {
    hs::overlayBegin(g_edit, c, model, cat, body, left);
    (void)hs::overlaySetScope(g_edit, scope);
    // hidden: the type level is edited while a weapon level of this hand (or, on the left, the right weapon level it mirrors) wins.
    g_edit.hidden = scope == hs::ScopeType && (hs::findModel(c, model, left) || (left && hs::findModel(c, model, false)));
}

// Re-seeds the open overlay from the current published config (after Save / ResetScope / SetScope). Caller holds g_edLock.
static void edReseed(bool left, int scope) {
    hs::TuneCfg c;
    if (!loadTune(c)) return;
    char model[17], body[17];
    memcpy(model, g_edit.model, sizeof model);
    memcpy(body, g_edit.body, sizeof body);
    edBegin(c, model, g_edit.cat, body, left, scope);
}

static int32_t KPATCH_CALL apiRegisterPreview(void* gob, void* beh, int32_t left) {
    if (!g_editor || !gob) return 0;
    void* parent = nullptr;
    void* node = nullptr;
    if (beh) {
        saferead::beginScope();
        parent = reinterpret_cast<void*>(static_cast<uintptr_t>(rdU32(kSitePlace, reinterpret_cast<uintptr_t>(beh), 0x14)));
        node = reinterpret_cast<void*>(static_cast<uintptr_t>(rdU32(kSitePlace, reinterpret_cast<uintptr_t>(beh), 0x18)));
    }
    bool ok;
    LONG count;
    {
        EdLock lk;
        ok = hs::previewRegister(g_preview, gob, parent, node, left != 0);
        count = g_preview.n;
        InterlockedExchange(&g_previewN, count);
    }
    if (takeEditLine()) KP_LOG(chEdit, "API register gob=%x beh=%x left=%d ok=%d n=%ld", U(gob), U(beh), left ? 1 : 0, ok ? 1 : 0, static_cast<long>(count));
    return ok ? 1 : 0;
}

static void KPATCH_CALL apiUnregisterPreview(void* gob) {
    LONG count;
    bool ended = false;
    {
        EdLock lk;
        if (!gob) {
            g_preview = hs::PreviewRegistry();
            g_prevSlide = hs::Slide();   // a cleared preview starts holstered again (the editor resets its toggle with it)
        } else {
            (void)hs::previewUnregister(g_preview, gob);
        }
        if (g_edit.active && (!gob || g_editGob == gob)) {
            g_edit = hs::EditOverlay();
            g_editGob = nullptr;
            g_editMountOk = false;
            ended = true;
        }
        count = g_preview.n;
        if (count == 0) g_prevSlide = hs::Slide();   // the last preview is gone: the next ones start holstered
        InterlockedExchange(&g_previewN, count);
    }
    if (takeEditLine()) KP_LOG(chEdit, "API unregister gob=%x n=%ld endedit=%d", U(gob), static_cast<long>(count), ended ? 1 : 0);
}

static int32_t KPATCH_CALL apiBeginEdit(void* gob, int32_t scope) {
    if (!gob || scope < 0 || scope > 2) return 0;
    hs::TuneCfg c;
    if (!loadTune(c)) return 0;
    void* parent = nullptr;
    bool left = false;
    {
        EdLock lk;
        const int i = hs::previewFind(g_preview, gob);
        if (i < 0) return 0;
        parent = g_preview.e[i].parent;
        left = effLeft(g_preview.e[i].left);
    }
    saferead::beginScope();
    char model[17], body[17];
    gobNames(reinterpret_cast<uintptr_t>(gob), reinterpret_cast<uintptr_t>(parent), model, body);
    const hs::Category cat = hs::classify(model, 0);
    {
        EdLock lk;
        edBegin(c, model, cat, body, left, scope);
        g_editGob = gob;
        g_editMountOk = false;
    }
    if (takeEditLine())
        KP_LOG(chEdit, "API begin gob=%x model=%s cat=%s body=%s left=%d scope=%d", U(gob), model[0] ? model : "-", hs::categoryName(cat), body[0] ? body : "-",
               left ? 1 : 0, static_cast<int>(scope));
    return 1;
}

static void KPATCH_CALL apiNudgeWorld(const float dq[4], const float dp[3]) {
    EdLock lk;
    if (!g_edit.active || !g_editMountOk) return;
    if (dq) (void)hs::overlayNudgeWorldRot(g_edit, g_editMount.q, hs::Q{dq[0], dq[1], dq[2], dq[3]});
    if (dp && (dp[0] != 0.f || dp[1] != 0.f || dp[2] != 0.f)) (void)hs::overlayNudgeWorldMove(g_edit, g_editMount.q, dp);
}

static void KPATCH_CALL apiNudgeLocal(int32_t axis, float deg, float metres) {
    if (axis < 0 || axis > 2) return;
    EdLock lk;
    if (!g_edit.active) return;
    if (deg != 0.f) (void)hs::overlayNudgeLocal(g_edit, axis, deg);
    if (metres != 0.f) {
        float x[3] = {g_edit.cur.off[0], g_edit.cur.off[1], g_edit.cur.off[2]};
        x[axis] += metres;
        hs::overlaySetAbsOff(g_edit, x);
        g_edit.dirty = true;
    }
}

static void KPATCH_CALL apiSetValues(const float rot[3], const float off[3]) {
    if (!rot || !off) return;
    EdLock lk;
    (void)hs::overlaySetValues(g_edit, rot, off);
}

static int32_t KPATCH_CALL apiCycleBone(int32_t dir) {
    EdLock lk;
    (void)hs::overlayCycleBone(g_edit, dir < 0 ? -1 : 1);
    return g_edit.cur.boneIdx;
}

static int32_t KPATCH_CALL apiGetEdit(K2HolsterEditInfo* out) {
    if (!out || out->struct_size < sizeof(K2HolsterEditInfo)) return 0;
    EdLock lk;
    if (!g_edit.active) return 0;
    hs::EditView v;
    hs::overlayGetEdit(g_edit, v);
    hs::tuneCopyName(out->model, g_edit.model);
    snprintf(out->type, sizeof out->type, "%s", hs::categoryName(g_edit.cat));
    hs::tuneCopyName(out->body, g_edit.body);
    const hs::TuneCfg& c = g_tuneW;   // bootBoneIdx only (set once at boot)
    const uint8_t bi = v.r.boneIdx ? v.r.boneIdx : hs::defaultBone(c);
    snprintf(out->bone, sizeof out->bone, "%s", hs::boneNameOf(bi) ? hs::boneNameOf(bi) : "custom");
    out->scope = v.scope;
    out->left = v.left ? 1 : 0;
    out->dirty = v.dirty ? 1 : 0;
    out->hidden = g_edit.hidden ? 1 : 0;
    for (int i = 0; i < 3; ++i) { out->rot[i] = v.r.rot[i]; out->off[i] = v.r.off[i]; out->src[i] = v.r.src[i]; }
    // Reserved bit 8 (hse::kSrcMirrorBit in the editor): the shown left rot / off is the right hand's value mirrored (ABI unchanged, the editor ignores unknown bits).
    if (v.r.leftMirror & 1u) out->src[0] = static_cast<uint8_t>(out->src[0] | 8u);
    if (v.r.leftMirror & 2u) out->src[1] = static_cast<uint8_t>(out->src[1] | 8u);
    return 1;
}

static int32_t KPATCH_CALL apiGetMountWorld(float pos[3], float q[4]) {
    if (!pos || !q) return 0;
    EdLock lk;
    if (!g_edit.active || !g_editMountOk) return 0;
    for (int i = 0; i < 3; ++i) pos[i] = g_editMount.pos[i];
    q[0] = g_editMount.q.w; q[1] = g_editMount.q.x; q[2] = g_editMount.q.y; q[3] = g_editMount.q.z;
    return 1;
}

// World pose of a named node of any Gob (e.g. the preview rig's "camerahook"). Same safety rule as mountPose: the Gob's FindPart slot
// and the node's pose slot must equal the ones seen on a placed weapon, so only the engine's own functions are called.
static int32_t KPATCH_CALL apiGetNodeWorld(void* gob, const char* node, float pos[3], float q[4]) {
    if (!gob || !node || !*node || !pos || !q) return 0;
    saferead::beginScope();
    const uintptr_t gvt = rdU32(kSitePlace, reinterpret_cast<uintptr_t>(gob), 0);
    const uintptr_t find = gvt ? rdU32(kSitePlace, gvt, 0x10C) : 0;
    const uintptr_t refFind = g_refFind, refPose = g_refPose;
    int why = 0;
    hs::Pose p;
    if (!find || !refFind || find != refFind) why = 1;
    else {
        const uintptr_t mn = reinterpret_cast<uintptr_t>(reinterpret_cast<void*(__thiscall*)(void*, const char*)>(find)(gob, node));
        const uintptr_t mvt = mn ? rdU32(kSitePlace, mn, 0) : 0;
        const uintptr_t pf = mvt ? rdU32(kSitePlace, mvt, 0x0C) : 0;
        if (!mn) why = 2;
        else if (!pf || pf != refPose) why = 3;
        else {
            float* const f = reinterpret_cast<float*>(&p);
            reinterpret_cast<void(__thiscall*)(void*, float*, float*)>(pf)(reinterpret_cast<void*>(mn), f, f + 3);
            if (!hs::poseSane(p)) why = 4;
        }
    }
    static volatile LONG s_lines = 0;
    if (why && InterlockedIncrement(&s_lines) <= 20 && takeEditLine())
        KP_LOG(chEdit, "API nodeworld refused gob=%x node=%s why=%d find=%x ref=%x", U(gob), node, why, static_cast<unsigned>(find), static_cast<unsigned>(refFind));
    if (why) return 0;
    for (int i = 0; i < 3; ++i) pos[i] = p.pos[i];
    q[0] = p.q.w; q[1] = p.q.x; q[2] = p.q.y; q[3] = p.q.z;
    return 1;
}

static int32_t KPATCH_CALL apiSetScope(int32_t scope, int32_t left) {
    if (scope < 0 || scope > 2) return 0;
    EdLock lk;
    if (!g_edit.active) return 0;
    edReseed(effLeft(left != 0), scope);   // implicit Revert: the editor confirmed Discard first
    return 1;
}

// Rewrites holster.ini for the open scope (reset: deletes its keys), then publishes the re-parsed text. Takes g_edLock only to build the
// keys and to reseed afterwards; the file read and write run outside it, so a placement on another thread never waits on disk I/O (F6).
static bool edWrite(bool reset) {
    hs::KV set[8];
    const char* unset[8];
    char buf[hs::kScopeTextMax];   // keys and values (set[].val may also point at boneName's static strings)
    hs::ScopeOut so{set, 8, 0, unset, 8, 0, buf, sizeof buf, 0};
    int scope = 0;
    bool left = false;
    hs::Category cat = hs::CatUnknown;
    char model[17] = {0}, body[17] = {0};
    {
        EdLock lk;
        if (!g_edit.active) return false;
        hs::TuneCfg c;
        if (!loadTune(c)) return false;
        bool ok = false;
        left = g_edit.left;
        scope = g_edit.scope;
        cat = g_edit.cat;
        memcpy(model, g_edit.model, sizeof model);
        memcpy(body, g_edit.body, sizeof body);
        switch (g_edit.scope) {
        case hs::ScopeWeapon: {
            if (reset) {
                hs::ModelTune m;
                hs::tuneCopyName(m.model, model);
                m.left = left;
                ok = hs::scopeKeys(m, so);
            } else if (hs::overlayWrite(g_edit, c)) {
                const hs::ModelTune* m = hs::findModel(c, model, left);
                ok = m && hs::scopeKeys(*m, so);
            }
            break;
        }
        case hs::ScopeType:
            if (reset) ok = hs::scopeKeys(cat, left, hs::CatTune(), so);
            else ok = hs::overlayWrite(g_edit, c) && hs::scopeKeys(cat, left, left ? c.catL[cat] : c.cat[cat], so);
            break;
        case hs::ScopeBody:
            if (reset) {
                // Reset deletes both body keys (offset and rotation delta).
                const int n = snprintf(buf, sizeof buf, "place_off_body_%s", body);
                bool fit = body[0] && n > 0 && static_cast<size_t>(n) < sizeof buf;
                const int m = fit ? snprintf(buf + n + 1, sizeof buf - static_cast<size_t>(n) - 1, "place_rot_body_%s", body) : -1;
                fit = fit && m > 0 && static_cast<size_t>(n) + 1 + static_cast<size_t>(m) < sizeof buf;
                ok = fit;
                if (ok) { unset[0] = buf; unset[1] = buf + n + 1; so.nUnset = 2; so.nSet = 0; }
            } else if (hs::overlayWrite(g_edit, c)) {
                const hs::BodyTune* b = hs::findBody(c, body);
                ok = b && hs::scopeKeys(*b, so);
            }
            break;
        }
        if (!ok) {
            if (takeEditLine()) KP_LOG(chEdit, "SAVE refused: no keys for scope=%d model=%s body=%s reset=%d", scope, model, body, reset ? 1 : 0);
            return false;
        }
    }
    std::string text;
    (void)readFileText("holster.ini", text);   // a missing file saves into a new one
    const size_t cap = text.size() + 4096;
    std::string a(cap, '\0'), b(cap, '\0');
    if (!hs::setKeys(text.data(), text.size(), set, so.nSet, &a[0], cap) || !hs::removeKeys(a.c_str(), strlen(a.c_str()), unset, so.nUnset, &b[0], cap)) {
        if (takeEditLine()) KP_LOG(chEdit, "SAVE refused: ini rewrite did not fit (%u bytes)", static_cast<unsigned>(cap));
        return false;
    }
    b.resize(strlen(b.c_str()));
    if (InterlockedCompareExchange(&g_reloadBusy, 1, 0) != 0) {
        if (takeEditLine()) KP_LOG(chEdit, "SAVE refused: reload in progress (retry)");
        return false;
    }
    const bool wrote = filewatch::atomicWrite("holster.ini", b, true);
    if (wrote) {
        filewatch::accept(g_poll, "holster.ini");
        hs::TuneCfg t = g_tuneW;
        parseTuneText(b.c_str(), t);
        publishTune(t);
        warnLeftExtraClamp(t, "SAVE");
    }
    InterlockedExchange(&g_reloadBusy, 0);
    if (takeEditLine())
        KP_LOG(chEdit, "SAVE %s scope=%d model=%s cat=%s body=%s left=%d set=%d unset=%d first=%s=%s ok=%d", reset ? "reset" : "write", scope,
               model[0] ? model : "-", hs::categoryName(cat), body[0] ? body : "-", left ? 1 : 0, so.nSet, so.nUnset,
               so.nSet ? set[0].key : (so.nUnset ? unset[0] : "-"), so.nSet ? set[0].val : "-", wrote ? 1 : 0);
    if (wrote) {
        EdLock lk;
        // Reseed only when the same edit is still open (EndEdit / BeginEdit on another weapon may have run meanwhile).
        if (g_edit.active && g_edit.left == left && g_edit.cat == cat && !strcmp(g_edit.model, model) && !strcmp(g_edit.body, body)) edReseed(left, scope);
    }
    return wrote;
}

static int32_t KPATCH_CALL apiSave(void) {
    return edWrite(false) ? 1 : 0;
}

static void KPATCH_CALL apiRevert(void) {
    EdLock lk;
    hs::overlayRevert(g_edit);
}

static void KPATCH_CALL apiResetScope(void) {
    (void)edWrite(true);
}

static void KPATCH_CALL apiEndEdit(void) {
    {
        EdLock lk;
        g_edit = hs::EditOverlay();
        g_editGob = nullptr;
        g_editMountOk = false;
    }
    if (takeEditLine()) KP_LOG(chEdit, "API endedit");
}

// holster 0.8.0-dev: the editor's Draw / Holster toggle. 1 = every preview weapon slides to the hand over draw_slide_ms (and the arm reaches when reach_test=1), 0 = back to the mount
// over stow_slide_ms. Returns the state now heading. Retargets from the current fraction, so a double click never pops.
static int32_t KPATCH_CALL apiSetPreviewDraw(int32_t drawn) {
    const uint32_t now = slideNowMs();
    const bool want = drawn != 0;
    uint32_t drawMs = 0, stowMs = 0;
    {
        EdLock lk;
        if (!g_prevSlide.init) g_prevSlide.reset(false, now);
        // HW28: the shared preview slide takes the timing of the weapon being edited (the global / unknown timing when none is)
        const hs::SlideRes sres = hs::slideResolve(g_slideTune, g_edit.active ? g_edit.cat : hs::CatUnknown);
        drawMs = sres.drawMs;
        stowMs = sres.stowMs;
        g_prevGrab = static_cast<float>(sres.grabPct) / 100.f;
        if (!g_prevSlide.active(now)) for (PrevLock& p : g_prevLock) { p.lk.lockOn = 0; p.cres = CarryResid(); }   // HW26: a slide from rest picks its hemisphere afresh
        if (g_reachTest && g_reachCarry) hs::reachRetarget(g_prevSlide, want, now, drawMs, stowMs, g_prevGrab, 0.04f);   // HW25: same continuity as the world
        else g_prevSlide.target(want, now, drawMs, stowMs);
    }
    if (takeEditLine()) KP_LOG(chEdit, "API previewdraw %s draw_ms=%u stow_ms=%u reach=%d", want ? "draw" : "stow", static_cast<unsigned>(drawMs), static_cast<unsigned>(stowMs), g_reachTest ? 1 : 0);
    return want ? 1 : 0;
}

static const K2HolsterApi g_holsterApi = {
    sizeof(K2HolsterApi), apiRegisterPreview, apiUnregisterPreview, apiBeginEdit, apiNudgeWorld, apiNudgeLocal, apiSetValues, apiCycleBone,
    apiGetEdit, apiGetMountWorld, apiGetNodeWorld, apiSetScope, apiSave, apiRevert, apiResetScope, apiEndEdit, apiSetPreviewDraw,
};

// bone_test: repoint the rhand (and with bone_test_left the lhand) attach-node literal at g_boneName. Default OFF.
static void applyBoneTest() {
    if (!g_boneTest) return;
    if (drawgateMode()) return;   // drawgate: the engine keeps its native rhand/lhand attach; HolsterPlace gates the hip write instead
    if (!k2addr::get()) { KP_LOG(chMain, "BONETEST refused: unknown exe (no address table)"); return; }
    const uint32_t ptr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(g_boneName));
    struct Site { const char* tag; uintptr_t at; const uint8_t* orig; bool on; };
    const Site sites[2] = {{"rhand", hs::kRhandImm, hs::kRhandOrig, true}, {"lhand", hs::kLhandImm, hs::kLhandOrig, g_boneLeft}};
    for (const Site& s : sites) {
        if (!s.on) continue;
        uint8_t seen[5] = {0, 0, 0, 0, 0};
        const int rc = hs::patchNodeLiteral(s.at, s.orig, ptr, seen);
        if (rc == 1) KP_LOG(chMain, "BONETEST patched %s -> %s", s.tag, g_boneName);
        else KP_LOG(chMain, "BONETEST refused %s: rc=%d seen=%02X %02X %02X %02X %02X", s.tag, rc, seen[0], seen[1], seen[2], seen[3], seen[4]);
    }
}

// Written out literally (create-patch's export scan reads the source text) and listed in exports.def.
extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    readConfig();
    if (api && api->struct_size >= offsetof(KPatchApi, require) + sizeof(api->require) && api->provide) {
        api->provide(K2HOLSTER_IFACE, K2HOLSTER_VERSION, &g_holsterApi);
    }
    KpLog_Init(api);
    g_poll.throttleMs = 500;
    filewatch::accept(g_poll, "holster.ini");
    char names[96] = "";
    addName(names, sizeof(names), g_onComb, "comb");
    addName(names, sizeof(names), g_onAttach, "attach");
    addName(names, sizeof(names), g_onSwap, "swap");
    addName(names, sizeof(names), g_onAnim, "anim");
    addName(names, sizeof(names), g_onGob, "gob");
    addName(names, sizeof(names), g_onPlace, "place");
    addName(names, sizeof(names), g_onToggle, "toggle");
    KP_LOG(chMain, "holster 0.16.0-dev loaded (P0 probe + unarmed_anim=%d): probe=%d channels=%s max_lines=%ld gob_every=%d table=%s", g_unarmedAnim ? 1 : 0, g_probe ? 1 : 0,
           names[0] ? names : "none", static_cast<long>(g_maxLines), g_gobEvery, k2addr::get() ? "ok" : "none");
    {
        hs::TuneCfg t;
        LONG seq = 0;
        (void)loadTuneIfChanged(t, seq);
        char slideCfg[220];   // HW28: the resolved timing per category as <category>:<draw ms>/<stow ms>/<grab %>
        slideCfg[0] = 0;
        for (int i = hs::CatUnknown + 1; i < hs::CatCount; ++i) {
            const hs::SlideRes sr = hs::slideResolve(g_slideTune, static_cast<hs::Category>(i));
            const size_t sn = strlen(slideCfg);
            snprintf(slideCfg + sn, sizeof slideCfg - sn, "%s%s:%u/%u/%u", sn ? "," : "", hs::categoryName(static_cast<hs::Category>(i)), static_cast<unsigned>(sr.drawMs),
                     static_cast<unsigned>(sr.stowMs), static_cast<unsigned>(sr.grabPct));
        }
        KP_LOG(chMain, "CONFIG toggle_key=0x%x draw_hold_ms=%u combat_exit_ms=%d flourish_exit_ms=%d hostile_range=%.1f hostile_backoff_ms=%u delay_party=%d flourish_draw=%d slide=%s resid_ms=%u wrist_max=%.0f move_hold_ms=%u party_combat_hold=%d reach_test=%d reach_carry=%d reach_hinge=%d unarmed_anim=%d (active=%d: needs probe=1, bone_test=1 and place_mode=drawgate)",
               static_cast<unsigned>(g_toggleKey), static_cast<unsigned>(g_drawHoldMs), g_delayCfg.combatExitMs, g_delayCfg.flourishExitMs, static_cast<double>(g_hostileRange),
               static_cast<unsigned>(g_hostileBackoffMs), g_delayParty ? 1 : 0, g_flourishDraw ? 1 : 0, slideCfg, static_cast<unsigned>(g_reachResidMs), static_cast<double>(g_reachWristMax), static_cast<unsigned>(g_moveHoldMs),
               g_partyCombatHold ? 1 : 0, g_reachTest ? 1 : 0, g_reachCarry ? 1 : 0, g_reachHinge ? 1 : 0, g_unarmedAnim ? 1 : 0, (g_boneTest && drawgateMode()) ? 1 : 0);
        KP_LOG(chMain, "PLACEMODE %s (active=%d: needs probe=1 and bone_test=1)", hs::placeModeName(g_placeMode), g_boneTest ? 1 : 0);
        KP_LOG(chMain, "CONFIG left_extra_rot=%.1f bone_test_left=%d (ini=%d, active=%d: needs probe=1 and bone_test=1) log_verbose=%d", static_cast<double>(t.leftExtra),
               g_boneLeftCfg ? 1 : 0, g_boneLeftCfg ? 1 : 0, g_boneLeft ? 1 : 0, g_logVerbose ? 1 : 0);
        for (int i = hs::CatUnknown + 1; i < hs::CatCount; ++i)
            if (t.hasLeftExtraCat[i])
                KP_LOG(chMain, "CONFIG left_extra_rot_%s=%.1f", hs::categoryName(static_cast<hs::Category>(i)), static_cast<double>(t.leftExtraCat[i]));
        for (int i = 0; i < g_cfgWarnN; ++i) KP_LOGV(chMain, KPLOG_WARN, "%s", g_cfgWarn[i]);
        warnLeftExtraClamp(t, "CONFIG");
        KP_LOG(chMain, "TUNE boot place=%d rot=(%.1f,%.1f,%.1f) off=(%.3f,%.3f,%.3f) gobvt=%x bone=%s bootbone=%u models=%d bodies=%d editor=%d", t.place ? 1 : 0,
               t.rot[0], t.rot[1], t.rot[2], t.off[0], t.off[1], t.off[2], static_cast<unsigned>(g_placeGobvt), g_boneName, t.bootBoneIdx, t.nMdl, t.nBody,
               g_editor ? 1 : 0);
    }
    applyBoneTest();
}

// No I/O here (loader lock). The config is read in KPatchInit.
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
