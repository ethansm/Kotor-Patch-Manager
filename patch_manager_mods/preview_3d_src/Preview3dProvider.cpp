// Preview3dProvider.cpp -- Provider of the "k2.preview3d" v1 registry interface (Preview3dApi.h).
// A per-token 3D character viewer for GUI panels. Port of the equip3d preview recipe (EquipCharPreview.cpp: allocCreature, readApp,
// buildPreview, attachWeapons, detachWeapons, refreshWeapons, destroyCreature, applyView, vtOf, getItemObj, detachHelmet,
// attachHelmet, partGob, playPart, chooseIdle, and the BindEquipView scene/rig/camera setup) into one Viewer per token.
// equip3d is a read-only reference: nothing here includes or calls it. Pure helpers (token table, camera maths) are in
// Preview3dLogic.h (host-tested). No hooks: the consumer owns the panel hooks and calls CreateView from its panel ctor and
// Destroy from its panel dtor.
//
// Differences from equip3d (deliberate):
//   * the weapon model is the item ResRef at item+0x128 ONLY ("" or "i_null" = no weapon, logged); no baseitems weapon table, so
//     no wield-style stance animation (the consumer picks one through PlayAnim).
//   * the BeforeFree callback fires before every weapon Gob free and before Destroy frees anything.
//   * hand identity is the item id at creature +0x238 (right) / +0x23C (left); the K2Addr Creature_HandleR/L pair is only logged.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>

#include "kpatch_api.h"
#include "KpLog.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "SceneView.h"
#include "Preview3dApi.h"
#include "Preview3dLogic.h"

KP_LOG_PATCH("preview-3d");

namespace {

KP_CHANNEL(chCore, "core");
KP_CHANNEL(chBuild, "build");

// Fallback sink before / without the core logger (path and level only: safe from a static initialiser).
const bool g_fallbackSet = (KpLog_SetFallbackFile("preview-3d_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// ---- engine addresses (Steam Aspyr). Named gameaddr:: values where they exist; the rest are the equip3d constants (same values). ----
constexpr int FnNew = static_cast<int>(gameaddr::New);                  // cdecl(size)
constexpr int ItemMgrGlobal = static_cast<int>(gameaddr::AppGlobal);    // *(app)+4 -> client item manager (equip3d ItemMgrGlobal)
constexpr int ExpectedGobVtable = static_cast<int>(gameaddr::GobVtable);
constexpr int CreatureVftable = static_cast<int>(gameaddr::ClientCreatureVtable);
constexpr int FnGetItemObj = static_cast<int>(gameaddr::ClientGetItemById);            // thiscall(mgr=[[a1b4a4]+4], id) -> client item*
constexpr int FnCreateModel = static_cast<int>(gameaddr::CreateModelGob);              // cdecl(name, inst, 0, 0) -> Gob*
constexpr int ModelInstName = static_cast<int>(gameaddr::PreviewModelInstName);        // instance-name string passed to FnCreateModel
constexpr int FnCreatureCtor = static_cast<int>(gameaddr::ClientCreatureCtor);         // CSWCCreature ctor, fastcall ECX = block (0x470 bytes)
constexpr int FnBuildApp = static_cast<int>(gameaddr::ClientCreatureSetAppearance);    // thiscall(preview, copy60, flags, 1, 0) RET 0x10
constexpr int FnSetAlign = static_cast<int>(gameaddr::PreviewSetAlign);                // thiscall(preview[0x89], int align) RET 4
constexpr int FnSetupScene = static_cast<int>(gameaddr::ClientCreatureSetup3DScene);   // thiscall(preview, view, real, 0, 1) RET 0x10

// ---- engine offsets (equip3d values) ----
constexpr int CreatureSize = 0x470;
constexpr int CreatureAppOff = 0x224;       // creature[0x89] = appearance object pointer (0x6c bytes, first 0x3c copied)
constexpr int CreatureStatsOff = 0x310;     // creature[0xc4] = stats; +0x80 short = alignment
constexpr int CreatureHolderOff = 0x68;     // creature[0x1a] = part holder: holder->vt[8](0xff) body Gob, (0xfe) head Gob
constexpr int CrRightItemOff = 0x238, CrLeftItemOff = 0x23c, NoItemId = 0x7f000000;
constexpr int ItemModelResOff = 0x128;      // client item +0x128: model ResRef char[16] the item ctor chose
constexpr int ItemBaseIdxOff = 0xc, ItemVariationOff = 0x11c;   // diagnostics only
constexpr int GobAttachSlot = 0x50, GobDetachSlot = 0x4c;
constexpr int CrSetScene = 0x94, CrSetPos = 0x8c, CrSetFacing = 0x88;
constexpr int PartPlaySlot = 0x18;
// Gob animation entries (static dumps steam_6a522e71 decomp/4e/004e3270.c = Gob::PlayAnimation, decomp/4e/004e1d40.c = per-frame advance;
// REA ledger 20261018_nb2-play-semantics.tsv): list data ptr Gob+0xDC, count Gob+0xE0, entry[0] = descriptor (name char[] at desc+8, clip
// length float at desc+0x50), entry+0x04 speed FLOAT, +0x08 flags, +0x10 playback position FLOAT. Read-only here (diagnostics).
constexpr int GobAnimListOff = 0xDC, GobAnimCountOff = 0xE0;
constexpr int AnimEntrySpeedOff = 0x4, AnimEntryFlagsOff = 0x8, AnimEntryPosOff = 0x10;
constexpr int AnimDescNameOff = 0x8, AnimDescLenOff = 0x50;
constexpr int MaxAnimEntries = 64;
constexpr int AppCopyBytes = 0x3c;
constexpr int PartBodyId = 0xff, PartHeadId = 0xfe;
constexpr short kNeutralAlign = 50;         // stats unreadable -> neutral idle, not evil (Equip3dLogic.h:180)
constexpr float kDefaultFov = 22.726f;      // equip3d Cfg default (Equip3dLogic.h:24)
const char kRigModel[] = "charrec_light";   // equip3d Cfg default rig (Equip3dLogic.h:23)
const char kSceneRoom[] = "gui3D_room";
constexpr unsigned SiteP3d = 0x7B00;        // saferead site id for this DLL's own probes
constexpr int MaxLogLines = 20000;

using Fast1 = void(__fastcall*)(void*);
using NewFn = void*(__cdecl*)(unsigned);
using BuildAppFn = int(__thiscall*)(void*, void*, int, int, int);
using SetAlignFn = void(__thiscall*)(void*, int);
using SetupSceneFn = void(__thiscall*)(void*, void*, void*, int, int);
using CrPtrFn = void(__thiscall*)(void*, void*);
using CrDtorFn = void(__thiscall*)(void*, int);
using PartGetFn = void*(__thiscall*)(void*, int);
using PartPlayFn = void(__thiscall*)(void*, const char*, float, unsigned, float);
using AttachFn = void(__thiscall*)(void*, void*, const char*, int);
using ItemObjFn = int(__thiscall*)(int, int);
using CreateModelFn = void*(__cdecl*)(const char*, const char*, int, int);

// ---- logging ----
int g_logLines = 0;
void vlog(int level, const char* fmt, va_list ap) {
    char buf[2048];
    vsnprintf(buf, sizeof buf, fmt, ap);
    KP_LOGV(chBuild, level, "%s", buf);
}
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    ++g_logLines;
    va_list ap; va_start(ap, fmt); vlog(KPLOG_INFO, fmt, ap); va_end(ap);
}
void logw(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); vlog(KPLOG_WARN, fmt, ap); va_end(ap);
}
// Log the first `maxN` calls of one API entry (counter per entry), so a per-tick call cannot flood the file.
bool firstN(int* counter, int maxN) {
    if (*counter >= maxN) return false;
    ++*counter;
    return true;
}

template <typename T>
bool readAt(int base, int off, T* out) {
    return saferead::readAt<T>(SiteP3d, static_cast<uintptr_t>(static_cast<unsigned>(base)), off, out);
}
void dumpMem(const char* tag, int addr, int words) {
    if (addr == 0) { logf("  dump %s @0: null", tag); return; }
    for (int i = 0; i < words; i += 8) {
        char line[200]; int p = 0;
        for (int j = i; j < i + 8 && j < words; ++j) {
            if (!saferead::probeRead(SiteP3d, reinterpret_cast<const void*>(addr + j * 4), 4)) p += snprintf(line + p, sizeof(line) - p, " ????????");
            else p += snprintf(line + p, sizeof(line) - p, " %08x", *reinterpret_cast<const int*>(addr + j * 4));
        }
        logf("  dump %s +%03x:%s", tag, i * 4, line);
    }
}

// ---- BeforeFree callbacks: up to kMaxBeforeFree (cb, user) pairs, so a second consumer never replaces the first (review F2) ----
constexpr int kMaxBeforeFree = 4;
struct BeforeFreeSlot { K2Preview3dBeforeFree cb; void* user; };
BeforeFreeSlot g_beforeFree[kMaxBeforeFree] = {};

// ---- the viewer ----
struct Viewer {
    uint32_t token = 0;
    int panel = 0;
    char tag[32] = "";
    unsigned seq = 0;
    unsigned char* block = nullptr;   // the CSWGui3DSceneView block (= view)
    void* rig = nullptr;
    // bound source + built preview creature
    int bound = 0;                    // Bind(): the real client creature
    unsigned char* creature = nullptr;
    bool creatureBad = false;         // construction failed: never retry on this view
    int real = 0;                     // the real creature the built preview came from
    unsigned char app[AppCopyBytes] = {};
    short align = -1;
    bool built = false;
    int builds = 0, baseModels = -1;
    // view state
    float yaw = 0, zoom = 1, panZ = 0, appliedPanZ = 0;
    // weapons (item ResRef at +0x128 only), helmet (own model instance, never the real item's Gob)
    bool weaponsOn = false;
    void* wpn[2] = {};
    int wpnIds[2] = {0, 0};
    char wpnNames[2][20] = {};
    void* helm = nullptr;
    char helmName[20] = "";
    char idleName[16] = "";
    // animation freeze (SetAnimPaused): the clip PlayAnim last started on the body and head, and its wall-clock bookkeeping
    char animName[40] = "";
    p3d::AnimClock animClock;
};

p3d::Viewers g_tokens;            // token table (Preview3dLogic.h); index = position in g_view
Viewer g_view[p3d::kMaxViewers];
unsigned g_seq = 0;
bool g_inited = false;

void ensureInit() {
    if (g_inited) return;
    g_inited = true;
    p3d::viewersInit(&g_tokens);
}

Viewer* find(uint32_t token) {
    int i = p3d::viewersIndex(&g_tokens, token);
    return i < 0 ? nullptr : &g_view[i];
}

void notifyBeforeFree(const Viewer* v, const char* why) {
    for (const BeforeFreeSlot& s : g_beforeFree) {
        if (!s.cb) continue;
        logf("before-free: token=%u (%s) cb=%p", v->token, why, reinterpret_cast<void*>(s.cb));
        s.cb(v->token, s.user);
    }
}

// ---- engine helpers (ports) ----
void** vtOf(void* obj) { void** vt = nullptr; readAt<void**>(reinterpret_cast<int>(obj), 0, &vt); return vt; }

int getItemObj(int id) {
    int mgr = 0, tbl = 0;
    if (!readAt<int>(ItemMgrGlobal, 0, &mgr) || !mgr || !readAt<int>(mgr, 4, &tbl) || !tbl) return 0;
    return reinterpret_cast<ItemObjFn>(FnGetItemObj)(tbl, id);
}

void* partGob(Viewer* v, int id) {
    int holder = 0;
    if (!v->creature || !readAt<int>(reinterpret_cast<int>(v->creature), CreatureHolderOff, &holder) || !holder) return nullptr;
    void** hv = vtOf(reinterpret_cast<void*>(holder));
    if (!hv || !saferead::probeRead(SiteP3d, hv + 2, 4)) return nullptr;
    return reinterpret_cast<PartGetFn>(hv[2])(reinterpret_cast<void*>(holder), id);
}

void playPart(void* part, const char* name, float speed, unsigned flags) {
    void** pv = vtOf(part);
    if (!pv || !saferead::probeRead(SiteP3d, pv + PartPlaySlot / 4, 4)) return;
    reinterpret_cast<PartPlayFn>(pv[PartPlaySlot / 4])(part, name, speed, flags, 0.0f);
}

// Read-only look at the Gob's animation entry named `name` (the engine's own lookup: newest entry first, case-insensitive). Every pointer
// is probed; a Gob that does not match the layout simply yields found=false. Never writes.
struct AnimProbe { bool found = false; float speed = 0, pos = 0, len = 0; unsigned flags = 0; };
AnimProbe probeAnim(void* gob, const char* name) {
    AnimProbe r;
    int base = reinterpret_cast<int>(gob), list = 0, count = 0;
    if (!base || !name || !readAt<int>(base, GobAnimListOff, &list) || !list || !readAt<int>(base, GobAnimCountOff, &count)) return r;
    if (count <= 0 || count > MaxAnimEntries) return r;
    for (int i = count - 1; i >= 0; --i) {
        int entry = 0, desc = 0;
        if (!readAt<int>(list, i * 4, &entry) || !entry || !readAt<int>(entry, 0, &desc) || !desc) continue;
        char nm[40]; int n = 0;
        for (; n < 39; ++n) { char c = 0; if (!readAt<char>(desc, AnimDescNameOff + n, &c)) { n = -1; break; } nm[n] = c; if (!c) break; }
        if (n < 0) continue;
        nm[39] = 0;
        if (_stricmp(nm, name) != 0) continue;
        if (!readAt<float>(entry, AnimEntrySpeedOff, &r.speed) || !readAt<unsigned>(entry, AnimEntryFlagsOff, &r.flags) ||
            !readAt<float>(entry, AnimEntryPosOff, &r.pos) || !readAt<float>(desc, AnimDescLenOff, &r.len)) return AnimProbe();
        r.found = true;
        return r;
    }
    return r;
}

int appRow(const Viewer* v) { return static_cast<int>(*reinterpret_cast<const unsigned short*>(v->app + 0x18)); }
void chooseIdle(Viewer* v) {
    void* body = partGob(v, PartBodyId);
    const char* want = p3d::idleFor(appRow(v), v->align);
    snprintf(v->idleName, sizeof v->idleName, "%s", want);
    logf("anim: idle for align=%d row=%d -> '%s' body=%p", v->align, appRow(v), v->idleName, body);
}

// Frees one attached Gob the way equip3d does: attach(0,0,0), vt[0x4c](0), delete. Vtable-guarded: a mismatch leaks, never crashes.
bool freeGob(void* g, const char* what, const char* why) {
    void** vt = vtOf(g); int vt0 = 0; readAt<int>(reinterpret_cast<int>(g), 0, &vt0);
    if (!vt || vt0 != ExpectedGobVtable || !saferead::probeRead(SiteP3d, vt + GobAttachSlot / 4, 4)) {
        logf("%s: detach %p (%s): vtable %08x unexpected, NOT freeing (leak instead of crash)", what, g, why, vt0);
        return false;
    }
    reinterpret_cast<AttachFn>(vt[GobAttachSlot / 4])(g, nullptr, nullptr, 0);
    reinterpret_cast<void(__thiscall*)(void*, int)>(vt[GobDetachSlot / 4])(g, 0);
    reinterpret_cast<void(__thiscall*)(void*, int)>(vt[0])(g, 1);
    saferead::beginScope();
    return true;
}

// ---- weapons ----
// notify = fire BeforeFree before each Gob free (false only when the caller already fired it for this operation).
void detachWeapons(Viewer* v, const char* why, bool notify) {
    for (int i = 0; i < 2; ++i) {
        void* g = v->wpn[i]; if (!g) continue;
        if (notify) notifyBeforeFree(v, "weapon free");   // before clearing: the callback may still ask WeaponGob(token) which Gob goes
        v->wpn[i] = nullptr;
        if (freeGob(g, "weapon", why)) logf("weapon[%d]: detached+freed %p '%s' (%s)", i, g, v->wpnNames[i], why);
    }
    v->wpnNames[0][0] = v->wpnNames[1][0] = 0;
}

void detachHelmet(Viewer* v, const char* why) {
    void* g = v->helm; if (!g) return;
    v->helm = nullptr;
    if (freeGob(g, "helmet", why)) logf("helmet: detached+freed %p '%s' (%s)", g, v->helmName, why);
    v->helmName[0] = 0;
}

// Reads the item's model ResRef (+0x128, char[16]). False = unreadable or not a plain resource name. *none = empty or "i_null".
bool itemModelName(int item, char* name /*17*/, bool* none) {
    name[0] = 0; *none = false;
    if (!saferead::probeRead(SiteP3d, reinterpret_cast<void*>(item + ItemModelResOff), 16)) return false;
    memcpy(name, reinterpret_cast<void*>(item + ItemModelResOff), 16); name[16] = 0;
    bool ok = name[0] != 0;
    for (const char* p = name; *p; ++p) if (!isalnum(static_cast<unsigned char>(*p)) && *p != '_') ok = false;
    if (!ok) { *none = name[0] == 0; return *none; }   // empty = readable "no model"; any other bad name is refused
    if (!_stricmp(name, "i_null")) *none = true;
    return true;
}

void attachWeapons(Viewer* v, int real, const char* why) {
    void* body = partGob(v, PartBodyId);
    if (!body) { logf("weapons: no body part, cannot attach (%s)", why); return; }
    // Hand identity: the item id at creature +0x238 / +0x23C. The K2Addr Creature_HandleR/L pair (+0x278/+0x27C) is logged only.
    int hR = 0, hL = 0, oR = k2addr::off(&K2AddrTable::Creature_HandleR), oL = k2addr::off(&K2AddrTable::Creature_HandleL);
    if (oR) readAt<int>(real, oR, &hR);
    if (oL) readAt<int>(real, oL, &hL);
    for (int i = 0; i < 2; ++i) {
        int id = NoItemId; readAt<int>(real, i ? CrLeftItemOff : CrRightItemOff, &id); v->wpnIds[i] = id;
        const char* hook = i ? "lhand" : "rhand";
        logf("weapons: %s hand id=%08x (creature+0x%x); K2Addr handle pair R=%08x L=%08x (log only)", hook, id, i ? CrLeftItemOff : CrRightItemOff, hR, hL);
        if (id == NoItemId || id == 0) { logf("weapons: %s empty (id=%08x)", hook, id); continue; }
        int item = getItemObj(id);
        if (!item) { logf("weapons: %s id=%08x -> no client item object", hook, id); continue; }
        int base = -1; readAt<int>(item, ItemBaseIdxOff, &base);
        unsigned char var = 0; readAt<unsigned char>(item, ItemVariationOff, &var);
        char name[17]; bool none = false;
        if (!itemModelName(item, name, &none)) { logw("weapons: %s id=%08x item=%08x model ResRef unreadable or not a resource name, skipped", hook, id, item); continue; }
        if (none) { logf("weapons: %s id=%08x item=%08x base=%d var=%d model='%s': no weapon model, skipped", hook, id, item, base, var, name[0] ? name : "<empty>"); continue; }
        void* g = reinterpret_cast<CreateModelFn>(FnCreateModel)(name, reinterpret_cast<const char*>(ModelInstName), 0, 0);
        saferead::beginScope();
        int gvt = 0; if (g) readAt<int>(reinterpret_cast<int>(g), 0, &gvt);
        logf("weapons: %s id=%08x item=%08x base=%d var=%d model='%s' -> gob=%p vtable=%08x (expect %08x)", hook, id, item, base, var, name, g, gvt, ExpectedGobVtable);
        if (!g) continue;
        if (gvt != ExpectedGobVtable) { logw("weapons: model gob vtable mismatch, not attaching (leak instead of crash)"); continue; }
        void** vt = vtOf(g);
        reinterpret_cast<AttachFn>(vt[GobAttachSlot / 4])(g, body, hook, 0);
        saferead::beginScope();
        v->wpn[i] = g; snprintf(v->wpnNames[i], sizeof v->wpnNames[i], "%s", name);
        logf("weapons: attached %p to body %p node %s", g, body, hook);
    }
    logf("weapons: (%s) L/R ids %08x/%08x", why, v->wpnIds[1], v->wpnIds[0]);
}

void refreshWeapons(Viewer* v, int real, const char* why) {
    detachWeapons(v, why, true);
    if (v->weaponsOn) attachWeapons(v, real, why);
}

// ---- helmet: own model instance on the preview head (the real item's Gob is never reparented: equip3d DB lesson 149) ----
void attachHelmet(Viewer* v, int id, const char* why) {
    if (id == NoItemId || id == 0) { logf("helmet: none (id=%08x, %s)", id, why); return; }
    int item = getItemObj(id);
    if (!item) { logf("helmet: id=%08x -> no client item object (%s), skipped", id, why); return; }
    char name[17]; bool none = false;
    if (!itemModelName(item, name, &none)) { logw("helmet: item %08x model ResRef unreadable, skipped", item); return; }
    int base = -1; readAt<int>(item, ItemBaseIdxOff, &base);
    unsigned char var = 0; readAt<unsigned char>(item, ItemVariationOff, &var);
    if (none) { logf("helmet: id=%08x item=%08x base=%d var=%d model='%s': no visible model, skipped", id, item, base, var, name[0] ? name : "<empty>"); return; }
    void* head = partGob(v, PartHeadId);
    if (!head) { logw("helmet: no head part on the preview (%s), skipped", why); return; }
    void* g = reinterpret_cast<CreateModelFn>(FnCreateModel)(name, reinterpret_cast<const char*>(ModelInstName), 0, 0);
    saferead::beginScope();
    int gvt = 0; if (g) readAt<int>(reinterpret_cast<int>(g), 0, &gvt);
    if (!g) { logw("helmet: model '%s' failed to load (id=%08x), skipped", name, id); return; }
    if (gvt != ExpectedGobVtable) { logw("helmet: model '%s' gob=%p vtable %08x (expect %08x), not attaching (leak instead of crash)", name, g, gvt, ExpectedGobVtable); return; }
    void** vt = vtOf(g);
    reinterpret_cast<AttachFn>(vt[GobAttachSlot / 4])(g, head, "GoggleHook", 0);   // missing node = silently unattached
    reinterpret_cast<PartPlayFn>(vt[PartPlaySlot / 4])(g, "default", 1.0f, 0, 0.0f);   // animated visors
    saferead::beginScope();
    v->helm = g; snprintf(v->helmName, sizeof v->helmName, "%s", name);
    logf("helmet: item id=%08x obj=%08x base=%d var=%d model='%s' gob=%p head=%p attach ok (%s)", id, item, base, var, name, g, head, why);
}

// ---- creature ----
// Order: BeforeFree first (everything still attached), then weapons, helmet, creature. Does NOT reset app / align / panZ / bound.
void destroyCreature(Viewer* v, const char* why) {
    if (v->creature || v->wpn[0] || v->wpn[1] || v->helm) notifyBeforeFree(v, why);
    detachWeapons(v, why, false);
    detachHelmet(v, why);
    if (!v->creature) return;
    void** vt = vtOf(v->creature);
    int vt0 = 0; readAt<int>(reinterpret_cast<int>(v->creature), 0, &vt0);
    logf("creature: destroy %p (%s) vtable=%08x", v->creature, why, vt0);
    if (vt0 == CreatureVftable && vt && saferead::probeRead(SiteP3d, vt, 4)) reinterpret_cast<CrDtorFn>(vt[0])(v->creature, 1);
    else logw("creature: vtable mismatch, NOT calling dtor (leak instead of crash)");
    saferead::beginScope();
    v->creature = nullptr; v->built = false; v->real = 0; v->baseModels = -1;
    p3d::clockReset(&v->animClock); v->animName[0] = 0;   // the clip died with the creature's Gobs
}

bool allocCreature(Viewer* v) {
    if (v->creature) return true;
    if (v->creatureBad) return false;
    unsigned char* c = static_cast<unsigned char*>(reinterpret_cast<NewFn>(FnNew)(CreatureSize));
    if (!c) { logw("creature: allocation failed"); v->creatureBad = true; return false; }
    memset(c, 0, CreatureSize);
    reinterpret_cast<Fast1>(FnCreatureCtor)(c);
    saferead::beginScope();
    int vt = 0; readAt<int>(reinterpret_cast<int>(c), 0, &vt);
    logf("creature: allocated %p vtable=%08x (expect %08x)", c, vt, CreatureVftable);
    if (vt != CreatureVftable) {
        logw("creature: ctor left wrong vtable, abandoning this view");
        dumpMem("creature", reinterpret_cast<int>(c), 24);
        v->creatureBad = true;
        return false;
    }
    int app = 0, holder = 0, stats = 0;
    readAt<int>(reinterpret_cast<int>(c), CreatureAppOff, &app);
    readAt<int>(reinterpret_cast<int>(c), CreatureHolderOff, &holder);
    readAt<int>(reinterpret_cast<int>(c), CreatureStatsOff, &stats);
    logf("creature: app=%08x holder=%08x stats=%08x", app, holder, stats);
    v->creature = c;
    return true;
}

bool realValid(int real) {
    int vt = 0;
    return readAt<int>(real, 0, &vt) && vt == CreatureVftable;
}

bool readApp(int real, unsigned char* out, short* align) {
    int app = 0, stats = 0;
    if (!readAt<int>(real, CreatureAppOff, &app) || !app || !saferead::probeRead(SiteP3d, reinterpret_cast<void*>(app), AppCopyBytes)) return false;
    memcpy(out, reinterpret_cast<void*>(app), AppCopyBytes);
    *align = kNeutralAlign;   // unreadable stats -> neutral idle, not evil
    if (readAt<int>(real, CreatureStatsOff, &stats) && stats) readAt<short>(stats, 0x80, align);
    return true;
}

// Camera fov from zoom, creature facing from yaw; the creature position is re-sent only when panZ changed.
void applyView(Viewer* v) {
    bool fovOk = sceneview::cameraFov(v->block, p3d::fovForZoom(kDefaultFov, v->zoom));
    if (!v->creature) { logf("view: token=%u fov applied=%d (no creature) yaw=%.2f zoom=%.3f", v->token, fovOk, v->yaw, v->zoom); return; }
    float f[3]; p3d::yawVec(v->yaw, f);
    void** cv = vtOf(v->creature);
    if (cv) {
        reinterpret_cast<CrPtrFn>(cv[CrSetFacing / 4])(v->creature, f);
        if (v->panZ != v->appliedPanZ) {
            float pos[3] = {0, 0, v->panZ};
            reinterpret_cast<CrPtrFn>(cv[CrSetPos / 4])(v->creature, pos);
            v->appliedPanZ = v->panZ;
            logf("view: token=%u panZ=%.3f sent", v->token, v->panZ);
        }
    }
}

// Builds / rebuilds the preview from `real` exactly like SetStats (0084f2c6..0084f482).
bool buildPreview(Viewer* v, int real, const char* why) {
    detachWeapons(v, "before rebuild", true);   // our weapon models hang off the old body Gob: free them first
    detachHelmet(v, "before rebuild");          // likewise the helmet on the old head Gob
    p3d::clockReset(&v->animClock); v->animName[0] = 0;   // a rebuild re-creates the body and head Gobs: no clip is playing (paused flag cleared)
    if (v->creature && v->real != real) destroyCreature(v, "character change");
    if (!allocCreature(v)) return false;
    unsigned char copy[AppCopyBytes]; short al = 0;
    if (!readApp(real, copy, &al)) { logf("build: real %08x appearance unreadable", real); return false; }
    unsigned char raw[AppCopyBytes]; memcpy(raw, copy, AppCopyBytes);   // snapshot BEFORE the +0x10 override
    logf("build: raw +0x10=%08x", *reinterpret_cast<int*>(copy + 0x10));
    // Blank the head item like SetStats: with a real id 0085a9e0 reparents the REAL item's Gob onto our head and our dtor then takes it down.
    int headId = *reinterpret_cast<int*>(copy + 0x10);
    *reinterpret_cast<int*>(copy + 0x10) = NoItemId;
    int gate = 0; sceneview::gateOpen(&gate);
    if (!gate) { logf("build: gate is zero, skipping"); return false; }
    void* pv = v->creature; void** vt = vtOf(pv);
    int scene = 0; readAt<int>(reinterpret_cast<int>(v->block), k2addr::off(&K2AddrTable::View_SceneObj), &scene);
    if (!vt || !scene) { logf("build: bad vtable %p / scene %08x", vt, scene); return false; }
    logf("build #%d (%s): real=%08x align=%d creature=%p scene=%08x panZ=%.3f", v->builds + 1, why, real, al, pv, scene, v->panZ);
    dumpMem("appcopy", reinterpret_cast<int>(copy), 15);
    int ok = reinterpret_cast<BuildAppFn>(FnBuildApp)(pv, copy, 3, 1, 0);
    logf("build: 0076d200 returned %d", ok);
    int pApp = 0; readAt<int>(reinterpret_cast<int>(pv), CreatureAppOff, &pApp);
    if (pApp) reinterpret_cast<SetAlignFn>(FnSetAlign)(reinterpret_cast<void*>(pApp), static_cast<int>(static_cast<unsigned short>(al)));
    reinterpret_cast<CrPtrFn>(vt[CrSetScene / 4])(pv, reinterpret_cast<void*>(scene));
    float pos[3] = {0, 0, 0}, face[3] = {1, 0, 0};
    reinterpret_cast<CrPtrFn>(vt[CrSetPos / 4])(pv, pos);
    v->appliedPanZ = 0;   // creature is back at the origin; applyView below restores panZ
    reinterpret_cast<CrPtrFn>(vt[CrSetFacing / 4])(pv, face);
    reinterpret_cast<SetupSceneFn>(FnSetupScene)(pv, v->block, reinterpret_cast<void*>(real), 0, 1);
    saferead::beginScope();
    {
        int mc = sceneview::modelCount(v->block);
        if (v->baseModels < 0) v->baseModels = mc;
        logf("build: scene models=%d (baseline %d)%s body=%p head=%p", mc, v->baseModels,
             mc > v->baseModels ? " WARNING: scene models GREW across rebuilds" : "", partGob(v, PartBodyId), partGob(v, PartHeadId));
    }
    memcpy(v->app, raw, AppCopyBytes); v->align = al; v->real = real; v->built = true; ++v->builds;
    chooseIdle(v);
    applyView(v);
    if (v->weaponsOn) refreshWeapons(v, real, "after build");
    attachHelmet(v, headId, "after build");
    return true;
}

// ---- API entries ----
int g_nCreate, g_nBind, g_nRebuild, g_nWeapons, g_nWeaponGob, g_nSetView, g_nPlay, g_nRigGob, g_nDestroy, g_nSetCb, g_nPause;

void freeViewer(Viewer* v, const char* why) {
    destroyCreature(v, why);
    p3d::viewersRemove(&g_tokens, v->token);
    *v = Viewer();
}

// Review F4: a view block whose CreateView fails after InitControl. Freed through its own vtable slot 0 (scalar deleting dtor, flag 1)
// ONLY when it is provably not in the panel list (ID -1: InitControl lists a control only for ID >= 0) and its vtable is the scene
// view's. A listed block (the consumer's layout gave the control an ID) or an unknown vtable is left as a logged leak: a leak beats a
// dangling list entry or a wild call.
void freeUnlistedBlock(unsigned char* block, const char* why) {
    if (!block) return;
    int id = 0, vt0 = 0;
    readAt<int>(reinterpret_cast<int>(block), k2addr::off(&K2AddrTable::Ctl_Id), &id);
    readAt<int>(reinterpret_cast<int>(block), 0, &vt0);
    const uint32_t want = static_cast<uint32_t>(k2addr::addr(&K2AddrTable::SceneViewVtable));
    void** vt = vtOf(block);
    if (id != -1 || !want || static_cast<uint32_t>(vt0) != want || !vt || !saferead::probeRead(SiteP3d, vt, 4) || !vt[0]) {
        logw("CreateView (%s): view block %p left allocated (id=%d vtable=%08x expect %08x)", why, block, id, vt0, static_cast<unsigned>(want));
        return;
    }
    reinterpret_cast<void(__thiscall*)(void*, int)>(vt[0])(block, 1);
    saferead::beginScope();
    logf("CreateView (%s): unlisted view block %p freed", why, block);
}

uint32_t KPATCH_CALL apiCreateView(void* panel, const char* tag) {
    ensureInit();
    saferead::beginScope();
    if (firstN(&g_nCreate, 8)) logf("API CreateView panel=%p tag=%s", panel, tag ? tag : "(null)");
    if (!panel || !tag || !tag[0]) { logw("CreateView: null panel/tag"); return 0; }
    int p = static_cast<int>(reinterpret_cast<intptr_t>(panel));
    int gate = 0; sceneview::gateOpen(&gate);
    int ctlCount = -1; readAt<int>(p, 0x28, &ctlCount);
    logf("CreateView panel=%08x tag=%s controls=%d gate=%d", p, tag, ctlCount, gate);
    if (!gate) { logw("CreateView: GUI gate global is zero, refusing"); return 0; }

    unsigned char* block = sceneview::createView(static_cast<uintptr_t>(static_cast<unsigned>(p)), tag);
    saferead::beginScope();
    if (!block) { logw("CreateView: view block not created (engine address unknown or allocation failed)"); return 0; }
    int id = -999, newCount = -1, ext[4] = {0};
    readAt<int>(reinterpret_cast<int>(block), 0x54, &id);
    for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(block), 4 + i * 4, &ext[i]);
    readAt<int>(p, 0x28, &newCount);
    logf("CreateView: block=%p control id=%d extent=[%d,%d,%d,%d] aspect=%.3f controls now %d (was %d)", block, id, ext[0], ext[1], ext[2], ext[3],
         ext[3] ? static_cast<float>(ext[2]) / ext[3] : 0.f, newCount, ctlCount);
    dumpMem("view", reinterpret_cast<int>(block), 38);

    uintptr_t scene = 0;
    int sf = sceneview::initScene(block, kSceneRoom, &scene);
    if (sf != sceneview::SF_None) { logw("CreateView: initScene failed (code %d, scene %08x)", sf, static_cast<unsigned>(scene)); freeUnlistedBlock(block, "initScene"); return 0; }
    saferead::beginScope();
    logf("CreateView: scene %08x initialised (%s)", static_cast<unsigned>(scene), kSceneRoom);
    void* rig = sceneview::addModel(block, kRigModel, -1);
    saferead::beginScope();
    int n = sceneview::modelCount(block);
    logf("CreateView: rig '%s' model=%p, scene model count=%d", kRigModel, rig, n);
    if (!rig || n < 1) { logw("CreateView: rig missing, view unusable"); freeUnlistedBlock(block, "rig"); return 0; }
    uint32_t cam = 0, camv = 0; bool camOk = sceneview::camera(block, &cam, &camv);
    logf("CreateView: camera=%08x vtable=%08x ok=%d", static_cast<unsigned>(cam), static_cast<unsigned>(camv), camOk);
    if (camOk) {
        bool a = sceneview::cameraAttach(block, rig, "camerahook", 1);
        bool f = sceneview::cameraFov(block, kDefaultFov);
        logf("CreateView: camera attach 'camerahook'=%d fov=%.3f set=%d", a, kDefaultFov, f);
    } else logw("CreateView: camera vtable mismatch, camera left as is");

    // A second view on the same panel + tag replaces the first (the old block belongs to a dead layout).
    for (int i = 0; i < p3d::kMaxViewers; ++i)
        if (g_tokens.ids[i] && g_view[i].panel == p && !strcmp(g_view[i].tag, tag)) {
            logf("CreateView: replacing token=%u (same panel+tag)", g_view[i].token);
            freeViewer(&g_view[i], "re-create");
        }
    if (p3d::viewersCount(&g_tokens) >= p3d::kMaxViewers) {   // table full: evict the oldest view
        int old = -1;
        for (int i = 0; i < p3d::kMaxViewers; ++i)
            if (g_tokens.ids[i] && (old < 0 || g_view[i].seq < g_view[old].seq)) old = i;
        if (old >= 0) { logw("CreateView: table full, evicting oldest token=%u panel=%08x", g_view[old].token, g_view[old].panel); freeViewer(&g_view[old], "evict"); }
    }
    uint32_t token = p3d::viewersAdd(&g_tokens);
    if (!token) { logw("CreateView: no token available"); freeUnlistedBlock(block, "token"); return 0; }
    Viewer* v = find(token);
    *v = Viewer();
    v->token = token; v->panel = p; v->seq = ++g_seq; v->block = block; v->rig = rig;
    snprintf(v->tag, sizeof v->tag, "%s", tag);
    logf("CreateView: token=%u ready (panel=%08x tag=%s views=%d)", token, p, tag, p3d::viewersCount(&g_tokens));
    return token;
}

int32_t KPATCH_CALL apiBind(uint32_t token, void* creature) {
    ensureInit();
    Viewer* v = find(token);
    if (firstN(&g_nBind, 8)) logf("API Bind token=%u creature=%p view=%s", token, creature, v ? "ok" : "unknown");
    if (!v || !creature) return 0;
    saferead::beginScope();
    v->bound = static_cast<int>(reinterpret_cast<intptr_t>(creature));
    logf("Bind: token=%u real=%08x valid=%d", token, v->bound, realValid(v->bound));
    return 1;
}

int32_t KPATCH_CALL apiRebuild(uint32_t token) {
    ensureInit();
    Viewer* v = find(token);
    saferead::beginScope();
    if (firstN(&g_nRebuild, 8)) logf("API Rebuild token=%u view=%s", token, v ? "ok" : "unknown");
    if (!v) return 0;
    if (!v->bound) {
        detachWeapons(v, "rebuild without bound creature", true);   // API: Rebuild removes the weapon and helmet Gobs before any step that can fail
        detachHelmet(v, "rebuild without bound creature");
        logw("Rebuild: token=%u has no bound creature", token);
        return 0;
    }
    if (!realValid(v->bound)) {
        detachWeapons(v, "rebuild bad creature", true);
        detachHelmet(v, "rebuild bad creature");
        logw("Rebuild: token=%u bound creature %08x is not a client creature (vtable mismatch)", token, v->bound);
        return 0;
    }
    bool ok = buildPreview(v, v->bound, "Rebuild");
    saferead::beginScope();
    logf("Rebuild: token=%u -> %d", token, ok);
    return ok ? 1 : 0;
}

void KPATCH_CALL apiSetWeapons(uint32_t token, int32_t on) {
    ensureInit();
    Viewer* v = find(token);
    if (firstN(&g_nWeapons, 12)) logf("API SetWeapons token=%u on=%d view=%s built=%d", token, on, v ? "ok" : "unknown", v ? v->built : 0);
    if (!v) return;
    saferead::beginScope();
    v->weaponsOn = on != 0;
    if (!v->built || !v->bound) { logf("SetWeapons: token=%u weapons=%d recorded (not built)", token, on); return; }
    refreshWeapons(v, v->real, on ? "SetWeapons on" : "SetWeapons off");
    saferead::beginScope();
    logf("SetWeapons: token=%u on=%d R=%p L=%p", token, on, v->wpn[0], v->wpn[1]);
}

void* KPATCH_CALL apiWeaponGob(uint32_t token, int32_t hand) {
    ensureInit();
    Viewer* v = find(token);
    if (firstN(&g_nWeaponGob, 8)) logf("API WeaponGob token=%u hand=%d view=%s", token, hand, v ? "ok" : "unknown");
    if (!v || (hand != K2PREVIEW3D_HAND_RIGHT && hand != K2PREVIEW3D_HAND_LEFT)) return nullptr;
    return v->wpn[hand];
}

void KPATCH_CALL apiSetView(uint32_t token, float yaw, float zoom, float panZ) {
    ensureInit();
    Viewer* v = find(token);
    if (firstN(&g_nSetView, 6)) logf("API SetView token=%u yaw=%.2f zoom=%.3f panZ=%.3f view=%s", token, yaw, zoom, panZ, v ? "ok" : "unknown");
    if (!v) return;
    saferead::beginScope();
    v->yaw = yaw; v->zoom = zoom > 0.f ? zoom : 1.f; v->panZ = panZ;   // zoom must stay > 0 (Preview3dLogic.h fovForZoom does not guard it)
    applyView(v);
}

void KPATCH_CALL apiPlayAnim(uint32_t token, const char* anim) {
    ensureInit();
    Viewer* v = find(token);
    if (firstN(&g_nPlay, 8)) logf("API PlayAnim token=%u anim=%s view=%s built=%d", token, anim ? anim : "(null)", v ? "ok" : "unknown", v ? v->built : 0);
    if (!v || !anim || !anim[0] || !v->built) return;
    const char* toPlay = _stricmp(anim, "idle") == 0 ? v->idleName : anim;
    saferead::beginScope();
    void* body = partGob(v, PartBodyId); void* head = partGob(v, PartHeadId);
    if (body) playPart(body, toPlay, 1.0f, 0);   // flags 0 = cross-fade in
    if (head) playPart(head, toPlay, 1.0f, 0);
    saferead::beginScope();
    snprintf(v->animName, sizeof v->animName, "%s", toPlay);
    p3d::clockStart(&v->animClock, GetTickCount());   // a new clip runs; this also clears a pause
    logf("PlayAnim: token=%u '%s' body=%p head=%p", token, toPlay, body, head);
}

// Freeze / resume the body and head animation at the CURRENT pose.
// Mechanism (a): re-issue the SAME clip name with speed 0 (pause) or 1 (resume) and flags 0. Evidence, Gob::PlayAnimation
// (static dump decomp/4e/004e3270.c, REA ledger 20261018_nb2-play-semantics.tsv):
//   * the call first searches the Gob's entries by name (stricmp on desc+8; skipped only by flags 0x40); a found entry is unlinked and
//     RE-INSERTED, and only a NEW entry runs the init block (start position = param5 or 0, blend weight 0). So a same-name replay keeps the
//     entry's playback position (+0x10) and blend weight (+0x14): no restart at frame 0, and the 5th argument (start offset, new entries
//     only) is irrelevant and stays 0.0f.
//   * speed is stored at entry+0x04 by FLD float param2; FSTP float [entry+4] (the FSTP in decomp/4e/004e3270.c). It is a FLOAT, not the "(int)" the
//     decompile shows (that cast, and the ledger row "stored as integer", are a decompiler artifact; 0 and 1 are exact either way).
//   * the per-frame advance (decomp/4e/004e1d40.c) does pos += dt*speed (decomp L231) and re-applies the pose at the (unchanged) pos every tick,
//     so speed 0 holds the pose exactly. The entry flags are overwritten with the passed flags (0 = looping, as PlayAnim plays it).
//   * side effects of any replay (same as PlayAnim): the `start<name>` event fires, and entries BEHIND this one (a still-fading cross-fade
//     source) are freed, so pausing inside the first blend-in moves the pose to the new clip at once.
// Fallback (c): if no entry of that name exists any more (the engine dropped it), the replay creates a NEW entry at position 0 with speed 0
// = frozen at the first pose of the clip. The read-only probeAnim before the call tells which of the two applies (a / c in the log).
// The helmet visor ("default") is not frozen. Not proven live: that the pose then stays put over many ticks (the log shows speed/pos).
void KPATCH_CALL apiSetAnimPaused(uint32_t token, int32_t paused) {
    ensureInit();
    Viewer* v = find(token);
    const bool want = paused != 0;
    if (!v || !v->built || !v->animClock.started || !v->animName[0]) {
        if (firstN(&g_nPause, 12)) logf("API SetAnimPaused token=%u paused=%d mech=none elapsed_ms=0 (%s)", token, want ? 1 : 0, !v ? "unknown view" : !v->built ? "not built" : "no clip played");
        return;
    }
    if (v->animClock.paused == want) {
        if (firstN(&g_nPause, 12)) logf("API SetAnimPaused token=%u paused=%d mech=none elapsed_ms=%u (already %s)", token, want ? 1 : 0, p3d::clockElapsed(&v->animClock, GetTickCount()), want ? "paused" : "running");
        return;
    }
    saferead::beginScope();
    const uint32_t now = GetTickCount();
    void* body = partGob(v, PartBodyId); void* head = partGob(v, PartHeadId);
    AnimProbe b0 = body ? probeAnim(body, v->animName) : AnimProbe();
    AnimProbe h0 = head ? probeAnim(head, v->animName) : AnimProbe();
    // 'a' only when every part that exists still holds the entry (the engine then keeps its position); otherwise 'c' (first pose)
    const char mech = (body || head) && (!body || b0.found) && (!head || h0.found) ? 'a' : 'c';
    const float speed = want ? 0.0f : 1.0f;
    if (body) playPart(body, v->animName, speed, 0);
    if (head) playPart(head, v->animName, speed, 0);
    saferead::beginScope();
    uint32_t elapsed;
    if (want) { elapsed = p3d::clockElapsed(&v->animClock, now); p3d::clockPause(&v->animClock, now); }
    else { p3d::clockResume(&v->animClock, now); elapsed = p3d::clockElapsed(&v->animClock, now); }
    if (firstN(&g_nPause, 12)) logf("API SetAnimPaused token=%u paused=%d mech=%c elapsed_ms=%u", token, want ? 1 : 0, mech, elapsed);
    AnimProbe b1 = body ? probeAnim(body, v->animName) : AnimProbe();
    AnimProbe h1 = head ? probeAnim(head, v->animName) : AnimProbe();
    logf("SetAnimPaused: token=%u '%s' %s body=%p found=%d pos %.4f->%.4f speed %.1f->%.1f len=%.4f | head=%p found=%d pos %.4f->%.4f speed %.1f->%.1f",
         token, v->animName, want ? "PAUSE" : "RESUME", body, b0.found, b0.pos, b1.pos, b0.speed, b1.speed, b1.len,
         head, h0.found, h0.pos, h1.pos, h0.speed, h1.speed);
}

void* KPATCH_CALL apiRigGob(uint32_t token, int32_t part) {
    ensureInit();
    Viewer* v = find(token);
    if (firstN(&g_nRigGob, 8)) logf("API RigGob token=%u part=%d view=%s", token, part, v ? "ok" : "unknown");
    if (!v || !v->built) return nullptr;
    if (part == K2PREVIEW3D_PART_BODY) return partGob(v, PartBodyId);
    if (part == K2PREVIEW3D_PART_HEAD) return partGob(v, PartHeadId);
    return nullptr;
}

void KPATCH_CALL apiDestroy(uint32_t token) {
    ensureInit();
    Viewer* v = find(token);
    if (firstN(&g_nDestroy, 8)) logf("API Destroy token=%u view=%s", token, v ? "ok" : "unknown");
    if (!v) return;
    saferead::beginScope();
    destroyCreature(v, "Destroy");   // BeforeFree, weapons, helmet, creature; keeps view state, app, align and Bind
    logf("Destroy: token=%u done", token);
}

void KPATCH_CALL apiSetBeforeFree(K2Preview3dBeforeFree cb, void* user) {
    ensureInit();
    bool ok = false;
    if (!cb) {   // remove every registration of this user
        for (BeforeFreeSlot& s : g_beforeFree)
            if (s.cb && s.user == user) { s = BeforeFreeSlot{}; ok = true; }
    } else {
        for (BeforeFreeSlot& s : g_beforeFree)
            if (s.cb == cb && s.user == user) { ok = true; break; }
        for (BeforeFreeSlot& s : g_beforeFree)
            if (!ok && !s.cb) { s = BeforeFreeSlot{cb, user}; ok = true; }
    }
    if (!ok) logw("SetBeforeFree: cb=%p user=%p not %s (table of %d full or user unknown)", reinterpret_cast<void*>(cb), user, cb ? "added" : "removed", kMaxBeforeFree);
    if (firstN(&g_nSetCb, 8)) logf("API SetBeforeFree cb=%p user=%p ok=%d", reinterpret_cast<void*>(cb), user, ok ? 1 : 0);
}

void* KPATCH_CALL apiViewControl(uint32_t token) {
    ensureInit();
    Viewer* v = find(token);
    return v ? v->block : nullptr;
}

const K2Preview3dApi g_apiTable = {
    sizeof(K2Preview3dApi),
    apiCreateView,
    apiBind,
    apiRebuild,
    apiSetWeapons,
    apiWeaponGob,
    apiSetView,
    apiPlayAnim,
    apiRigGob,
    apiDestroy,
    apiSetBeforeFree,
    apiViewControl,
    apiSetAnimPaused
};

} // anonymous namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    ensureInit();
    KpLog_Init(api);
    if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->provide) {
        KP_LOG(chCore, "no KPatchApi with a registry (patcher predates it?): %s not provided", K2PREVIEW3D_IFACE);
        return;
    }
    int rc = api->provide(K2PREVIEW3D_IFACE, K2PREVIEW3D_VERSION, &g_apiTable);
    KP_LOG(chCore, "provided %s v%d rc=%d (table size %u)", K2PREVIEW3D_IFACE, K2PREVIEW3D_VERSION, rc, static_cast<unsigned>(sizeof(K2Preview3dApi)));
}

} // extern "C"
