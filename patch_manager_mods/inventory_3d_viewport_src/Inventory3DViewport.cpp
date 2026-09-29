// Inventory 3D Viewport (Steam Aspyr build 6A522E71...). Shows the item under the cursor as a
// live 3D model in the ultrawide margin of the inventory screen, by reusing the Workbench's
// render function 008cd2c0 with a private CSWGui3DSceneView block ("fake this" = block-0x3a30).
// Research trail: patch_manager_mods/01_inventory_3d_viewport.md (local-setup/KOTOR-II worktree).
//
// Hooks (cdecl detours, original bytes still run afterwards):
//   H1 0x008a69ad BindInventoryView  panel ctor 008a6170, before StopLoadFromLayout
//   H2 0x008a8172 ShowItem           entry event-0 handler 008a8100, after GetItemByObjectId
//   H3 0x008a7350 InventoryTick      inventory Update prologue (logging only in v1)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "ItemModelTable.inc"

// 1 = only weapons/armour get a preview (fast switch-off for untested item classes)
#define INV3D_ONLY_WEAPONS_ARMOR 0

namespace {

constexpr int GateGlobal = 0x009f42b4;
constexpr int FnControlCtor = 0x00418990;   // CSWGuiControl ctor, ECX = ctrl
constexpr int ViewVftable = 0x009a3d34;     // CSWGui3DSceneView
constexpr int FnSceneCtor = 0x00417f40;     // CSWGuiScene ctor, ECX = view+0x60
constexpr int FnInitControl = 0x0040f620;   // thiscall(panel, view, CExoString*, addToList, scale) RET 16
constexpr int FnExoCtor = 0x00733570;
constexpr int FnExoDtor = 0x00733780;
constexpr int FnNew = 0x00919723;           // cdecl(size)
constexpr int FnAddModel = 0x00418360;      // thiscall(view+0x60, CExoString*, int)
constexpr int FnRemoveModel = 0x00418430;   // thiscall(view+0x60, index, deleteIt) RET 8
constexpr int FnRender = 0x008cd2c0;        // fastcall(fakeThis)
constexpr int SceneOffset = 0x60;           // CSWGuiScene embedded in the view
constexpr int SceneObjOffset = 0x74;        // view+0x60+0x14 = Scene*
constexpr int ModelCountOffset = 0x80;      // view+0x80 = scene+0x20
constexpr int FakeCategoryBlockOff = 0x2fc; // fake+0x3d2c
constexpr int FakeItemBlockOff = 0x304;     // fake+0x3d34
constexpr int FakeThisDelta = 0x3a30;       // fake = block - 0x3a30
constexpr int BlockSize = 0x400;
constexpr int ExpectedControlId = 21;

using Fast1 = void(__fastcall*)(void*);
using ThisCall2 = void*(__thiscall*)(void*, void*, int);
using InitCtl = void(__thiscall*)(void*, void*, void*, int, int);
using CExoCtor = void*(__thiscall*)(void*, const char*);
using CExoDtor = void(__thiscall*)(void*);
using Remove = void(__thiscall*)(void*, int, int);
using SceneInit = void(__thiscall*)(void*, const char*, float*, float*);
using NewFn = void*(__cdecl*)(unsigned);
using Render = void(__fastcall*)(void*);

// ---- diagnostics: inv3d_log.txt in the game dir, capped ----
int g_logLines = 0;
constexpr int MaxLogLines = 4000;
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    FILE* f = fopen("inv3d_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

void dumpMem(const char* tag, int addr, int words) {
    if (addr == 0) { logf("  dump %s @0: null", tag); return; }
    for (int i = 0; i < words; i += 8) {
        char line[200]; int p = 0;
        for (int j = i; j < i + 8 && j < words; ++j) {
            if (IsBadReadPtr(reinterpret_cast<const void*>(addr + j * 4), 4)) p += snprintf(line + p, sizeof(line) - p, " ????????");
            else p += snprintf(line + p, sizeof(line) - p, " %08x", *reinterpret_cast<const int*>(addr + j * 4));
        }
        logf("  dump %s +%03x:%s", tag, i * 4, line);
    }
}

template <typename T>
bool readAt(int base, int off, T* out) {
    if (base == 0 || IsBadReadPtr(reinterpret_cast<const void*>(base + off), sizeof(T))) return false;
    *out = *reinterpret_cast<const T*>(base + off);
    return true;
}

struct Slot {
    int panel; unsigned char* block; bool usable;
    int lastItem, lastBase, lastVar; unsigned seq;
};
Slot g_slots[4];
unsigned g_seq = 0;

Slot* findSlot(int panel) {
    Slot* best = nullptr;
    for (auto& s : g_slots)
        if (s.seq && s.panel == panel && (!best || s.seq > best->seq)) best = &s;
    return best;
}

Slot* claimSlot(int panel) {
    Slot* pick = nullptr;
    for (auto& s : g_slots) if (s.seq && s.panel == panel) { pick = &s; break; }
    if (!pick) {
        pick = &g_slots[0];
        for (auto& s : g_slots) if (s.seq < pick->seq) pick = &s;
    }
    return pick;
}

int modelCount(unsigned char* view) {
    int n = -1;
    if (!readAt<int>(reinterpret_cast<int>(view), ModelCountOffset, &n)) return -1;
    return n;
}

} // namespace

extern "C" void __cdecl BindInventoryView(int panel) {
    int ctlCount = -1; readAt<int>(panel, 0x28, &ctlCount);
    int gate = 0; readAt<int>(GateGlobal, 0, &gate);
    int stackLocal = 0;
    logf("H1 BindInventoryView panel=%08x controls=%d gate=%d stack=%p", panel, ctlCount, gate, &stackLocal);
    if (!panel) { logf("H1: null panel, abort"); return; }

    unsigned char* block = static_cast<unsigned char*>(reinterpret_cast<NewFn>(FnNew)(BlockSize));
    if (!block) { logf("H1: operator new failed"); return; }
    memset(block, 0, BlockSize);
    unsigned char* view = block;

    reinterpret_cast<Fast1>(FnControlCtor)(view);
    *reinterpret_cast<void**>(view) = reinterpret_cast<void*>(ViewVftable);
    reinterpret_cast<Fast1>(FnSceneCtor)(view + SceneOffset);

    char tag[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(tag, "3D_MODEL");
    reinterpret_cast<InitCtl>(FnInitControl)(reinterpret_cast<void*>(panel), view, tag, 1, 1);
    reinterpret_cast<CExoDtor>(FnExoDtor)(tag);

    int id = -999, newCount = -1; int ext[4] = {0};
    readAt<int>(reinterpret_cast<int>(view), 0x54, &id);
    for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(view), 4 + i * 4, &ext[i]);
    readAt<int>(panel, 0x28, &newCount);
    logf("H1: block=%p control id=%d (expect %d) extent=[%d,%d,%d,%d] controls now %d (expect %d)",
         block, id, ExpectedControlId, ext[0], ext[1], ext[2], ext[3], newCount, ctlCount + 1);
    dumpMem("view", reinterpret_cast<int>(view), 38);

    Slot* s = claimSlot(panel);
    *s = Slot{panel, block, false, 0, 0, 0, ++g_seq};

    if (id != ExpectedControlId) { logf("H1: GUI control missing (id=%d), slot unusable", id); return; }
    if (!gate) { logf("H1: gate global 009f42b4 is zero, slot unusable"); return; }

    int scene = 0;
    if (!readAt<int>(reinterpret_cast<int>(view), SceneObjOffset, &scene) || !scene || IsBadReadPtr(reinterpret_cast<void*>(scene), 4)) {
        logf("H1: scene pointer bad (%08x), slot unusable", scene); return;
    }
    void** vt = *reinterpret_cast<void***>(scene);
    if (IsBadReadPtr(vt + 0x70 / 4, 4)) { logf("H1: scene vtable bad, slot unusable"); return; }
    float pos[3] = {0, 0, 0}, quat[4] = {0, 0, 0, 1};
    reinterpret_cast<SceneInit>(vt[0x70 / 4])(reinterpret_cast<void*>(scene), "gui3D_room", pos, quat);
    logf("H1: scene %08x initialised (gui3D_room)", scene);

    char tag2[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(tag2, "upgitem_light");
    void* light = reinterpret_cast<ThisCall2>(FnAddModel)(view + SceneOffset, tag2, -1);
    reinterpret_cast<CExoDtor>(FnExoDtor)(tag2);
    int n = modelCount(view);
    logf("H1: light model=%p, scene model count=%d", light, n);
    if (!light || n < 1) { logf("H1: light model missing, slot unusable"); return; }
    s->usable = true;
    logf("H1: slot %d ready (panel=%08x seq=%u)", static_cast<int>(s - g_slots), panel, s->seq);
}

extern "C" void __cdecl ShowItem(int item, int panel) {
    static int repeatLogs = 0, detailLogs = 0;
    Slot* s = findSlot(panel);
    if (!s) { logf("H2: no slot for panel=%08x", panel); return; }
    if (!s->usable) { logf("H2: slot unusable (panel=%08x)", panel); return; }
    if (!item) { logf("H2: null item"); return; }
    unsigned char* view = s->block;

    int id = -1; unsigned char v0 = 0, v1 = 0;
    if (!readAt<int>(item, 0xc, &id) || !readAt<unsigned char>(item, 0x2a4, &v0) || !readAt<unsigned char>(item, 0x2a6, &v1)) {
        logf("H2: item %08x unreadable", item); return;
    }
    int var = v0 | (v1 << 8);
    if (id < 0 || id >= kItemRows) { logf("H2: base item id %d out of range", id); return; }
    int cat = kItemCategory[id];
#if INV3D_ONLY_WEAPONS_ARMOR
    if (!kIsWeaponOrArmor[id]) cat = 0;
#endif
    if (item == s->lastItem && id == s->lastBase && var == s->lastVar) {
        if (repeatLogs < 20) { ++repeatLogs; logf("H2: same item %08x base=%d var=%04x, skip", item, id, var); }
        return;
    }
    int n = modelCount(view);
    if (cat == 0) {
        if (n >= 2) reinterpret_cast<Remove>(FnRemoveModel)(view + SceneOffset, 1, 1);
        s->lastItem = item; s->lastBase = id; s->lastVar = var;
        logf("H2: item %08x base=%d var=%04x cat=0 (no preview), models %d -> %d", item, id, var, n, modelCount(view));
        return;
    }
    if (n < 1) { logf("H2: light model gone (count=%d), slot unusable", n); s->usable = false; return; }

    *(view + FakeCategoryBlockOff) = static_cast<unsigned char>(cat);
    *reinterpret_cast<int*>(view + FakeItemBlockOff) = item;
    reinterpret_cast<Render>(FnRender)(view - FakeThisDelta);
    *reinterpret_cast<int*>(view + FakeItemBlockOff) = 0;
    s->lastItem = item; s->lastBase = id; s->lastVar = var;
    logf("H2: rendered item %08x base=%d var=%04x cat=%d, models %d -> %d", item, id, var, cat, n, modelCount(view));
    if (detailLogs < 10) { ++detailLogs; dumpMem("scene", reinterpret_cast<int>(view + SceneOffset), 16); }
}

extern "C" void __cdecl InventoryTick(int panel) {
    static int calls = 0;
    if (calls >= 20) return;
    ++calls;
    Slot* s = findSlot(panel);
    logf("H3 tick #%d panel=%08x slot=%s", calls, panel, s ? (s->usable ? "usable" : "unusable") : "none");
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) logf("inv3d loaded");
    return TRUE;
}
