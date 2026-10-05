/* SaveEvents.h -- the "k2.saveevents" v1 registry interface (provider: the save-events patch) + a C++ consumer helper.
 *
 * Why: the engine's save / load / new-game paths are interesting to several mods (the action hotbar's per-save INI snapshot, level-up
 * GUID binding, any future per-save config), but the KPM installer rejects two patches hooking one address. save-events owns the five
 * detours (doc 22 section 4.1), parses the save folder, and calls registered consumers with one K2SaveEvent per occurrence.
 * Design + install rules: patch_manager_mods/22_hotbar_shared_services.md sections 4.1 / 4.1a.
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 *
 * Kinds (a bit mask when registering, exactly one bit in an event):
 *   K2SE_SAVED          a save was written (menu save, quicksave, timer autosave)       folder = that save
 *   K2SE_AUTOSAVED      the module-entry autosave was written                           folder = "000001 - AUTOSAVE" under the save root
 *   K2SE_LOAD_REQUEST   a save is loading (load menu, main menu, quickload, autosave slot)  folder = the save being loaded
 *   K2SE_NEW_GAME       the chargen "Play" commit; folder = NULL
 *
 * K2SaveEvent.folder: the save folder qualified with its save root, "saves/000012 - Game11" or "cloudsaves/<steam id>/000012 - Game11"
 * (cloud = 1). The path is relative to the game directory (the process working directory), exactly what a consumer needs to open files
 * in it. Valid only during the callback: copy it. An event whose folder the provider cannot parse is NOT dispatched (it is logged).
 *
 * Threading / cost: callbacks run on the engine thread inside the save / load / new-game code. A callback MUST be cheap and MUST NOT
 * block, throw, or start a modal wait. The provider times every callback and logs one WARN when one takes longer than 5 ms; it keeps
 * dispatching. Registering / unregistering from inside a callback is allowed: a consumer registered during a dispatch is first called
 * for the NEXT event, a consumer unregistered during a dispatch is not called afterwards (even later in the same dispatch).
 *
 * CurrentSaveFolder(buf, cap): the folder of the last SAVED / AUTOSAVED / LOAD_REQUEST, in the same form as K2SaveEvent.folder; it is
 * updated BEFORE the callbacks run and cleared by NEW_GAME. Returns 0 = none (buf[0] = 0), else the full length of the folder string
 * (snprintf-style: a return >= cap means it was truncated). With cap > 0 the buffer is always NUL-terminated; buf may be NULL or cap 0
 * to only ask for the length.
 */
#ifndef K2_SAVEEVENTS_H
#define K2_SAVEEVENTS_H

#include "kpatch_api.h"
#include <stddef.h>
#include <stdint.h>

#define K2SE_IFACE   "k2.saveevents"
#define K2SE_VERSION 1

#define K2SE_SAVED        1
#define K2SE_AUTOSAVED    2
#define K2SE_LOAD_REQUEST 4
#define K2SE_NEW_GAME     8
#define K2SE_ALL          15

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2SaveEvent {
    uint32_t struct_size;
    int32_t kind;            /* exactly one K2SE_* bit */
    const char* folder;      /* root-qualified save folder (see above), NULL for K2SE_NEW_GAME */
    int32_t cloud;           /* 1 = the folder is under cloudsaves/<id>/, 0 = under saves/ */
} K2SaveEvent;

typedef void (KPATCH_CALL *K2SaveCallback)(const K2SaveEvent* ev, void* user);

typedef struct K2SaveEventsApi {
    uint32_t struct_size;
    /* Call fn for every event whose kind is in kindMask (K2SE_*). owner = your manifest id (logged). Returns a token > 0,
       or <= 0 (no valid kind bit, null fn, table full). */
    int32_t (KPATCH_CALL *Register)(int32_t kindMask, K2SaveCallback fn, void* user, const char* owner);
    void    (KPATCH_CALL *Unregister)(int32_t token);
    int32_t (KPATCH_CALL *CurrentSaveFolder)(char* buf, uint32_t cap);
} K2SaveEventsApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */

// ---- C++ consumer helper ------------------------------------------------------------------------------------------------------
// In KPatchInit:   g_se.init(api, "my-patch-id"); g_se.add(K2SE_LOAD_REQUEST | K2SE_NEW_GAME, onSave); g_se.connect();
// In your hooks:   g_se.poll();   (retries connect() if the provider was not there at init: config order is not a contract)
//                  char cur[260]; g_se.currentSaveFolder(cur, sizeof cur);
// Without a provider (not installed, or a patcher that predates KPatchInit) every call is a safe no-op: no callback ever fires and
// currentSaveFolder returns 0. Check ev->struct_size with eventHas() before reading a field newer than the oldest provider you support.
namespace k2se {

constexpr int MaxRegs = 8;

inline bool eventHas(const K2SaveEvent* ev, size_t fieldEnd) { return ev && ev->struct_size >= fieldEnd; }
#define K2SE_EVENT_HAS(ev, field) ::k2se::eventHas((ev), offsetof(K2SaveEvent, field) + sizeof((ev)->field))

struct Client {
    struct Reg { int32_t mask; K2SaveCallback fn; void* user; int32_t token; };
    const KPatchApi* api = nullptr;
    const K2SaveEventsApi* se = nullptr;
    const char* owner = "";
    Reg regs[MaxRegs] = {};
    int nRegs = 0;
    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;   // optional: log the outcome of each connect attempt that changes state
    bool reportedMissing = false;

    void init(const KPatchApi* a, const char* id) { api = a; owner = id ? id : ""; }
    bool add(int32_t kindMask, K2SaveCallback fn, void* user = nullptr) {
        if (nRegs >= MaxRegs || !fn || (kindMask & K2SE_ALL) == 0) return false;
        regs[nRegs++] = { kindMask, fn, user, 0 };
        return true;
    }
    static bool usable(const K2SaveEventsApi* p) {
        return p && p->struct_size >= offsetof(K2SaveEventsApi, CurrentSaveFolder) + sizeof(p->CurrentSaveFolder)
                 && p->Register && p->Unregister && p->CurrentSaveFolder;
    }
    // require + register everything once. Idempotent; true when connected.
    bool connect() {
        if (connected) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2SaveEventsApi* p = static_cast<const K2SaveEventsApi*>(api->require(K2SE_IFACE, K2SE_VERSION));
        if (!usable(p)) {
            if (!reportedMissing && onConnect) onConnect(false, p ? "k2.saveevents v1 table too small" : "k2.saveevents v1 not provided (save-events missing or loaded later)");
            reportedMissing = true;
            return false;
        }
        se = p;
        int ok = 0;
        for (int i = 0; i < nRegs; ++i) { regs[i].token = se->Register(regs[i].mask, regs[i].fn, regs[i].user, owner); if (regs[i].token > 0) ++ok; }
        connected = true;
        if (onConnect) onConnect(ok == nRegs, ok == nRegs ? "registered" : "some registrations refused");
        return true;
    }
    // Cheap retry from a hook: one require every 256 calls until connected.
    void poll() { if (!connected && (pollCount++ & 255) == 0) connect(); }
    // Unregisters everything that was accepted and forgets the provider (the next connect() registers again).
    void release() {
        if (!connected || !se) return;
        for (int i = 0; i < nRegs; ++i) { if (regs[i].token > 0) se->Unregister(regs[i].token); regs[i].token = 0; }
        se = nullptr; connected = false;
    }
    // See the header comment: 0 = none / no provider (buf[0] = 0), else the full length (>= cap means truncated).
    int32_t currentSaveFolder(char* buf, uint32_t cap) const {
        if (!se) { if (buf && cap) buf[0] = 0; return 0; }
        return se->CurrentSaveFolder(buf, cap);
    }
};

} // namespace k2se
#endif /* __cplusplus */

#endif /* K2_SAVEEVENTS_H */
