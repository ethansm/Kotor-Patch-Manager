// k2-texture-memory-release 1.1.0 (Steam Aspyr build 6A522E71...). Candidate TX (doc 10, Phase 6C).
//
// Problem: BuildAndStoreTexture 0x4260E0 frees the CPU-side image after the GL upload only for non-TPC textures
// (`if (!IsTPCLoaded()) FreeImage()`, 0x4261CA-0x4261E4). TPC textures use the CResTPC buffer zero-copy (tex+0x44) and
// keep their resman Demand reference, so every uploaded TPC stays in RAM (~195 MB measured in run A).
// Fix (the perf-lab `tx=1` experiment, ported unchanged): 0x4261CF `0F B6 C8 85 C9 75 10` -> `E9 rel32 90 90` into a
// cave that also frees TPC images, except textures with a TextureController ([tex+0x34] != 0): the controller family
// 0x48FF30 / 0x491C40 / 0x492310 / 0x492460 / 0x4907F0 / 0x490710 reads the image after upload (T1, W1: SAFE WITH GUARD).
//   cave: movzx ecx,al / test ecx,ecx / jz nontpc     ; not TPC: free as the engine always did
//         mov edx,[ebp-4] / cmp dword [edx+0x34],0 / jnz skip
//         lock inc [freed] / jmp 0x4261D6           ; TPC without controller: FreeImage (vtable +0xB4)
//   skip: lock inc [kept]  / jmp 0x4261E6           ; TPC with controller: keep the image (engine behaviour)
//   nontpc: jmp 0x4261D6
// EAX = IsTPCLoaded() result, [EBP-4] = texture; ECX/EDX are reloaded at 0x4261D6; flags/EAX/ECX/EDX dead at the
// epilogue 0x4261E6. Re-use after the free is safe: Reset/quality change/GL context re-creation (0x409750 -> 0x476BE0 ->
// 0x426C30 -> 0x427DE0) re-Demand from resman; a requeue with +0x44 == 0 skips glImage (JZ 0x426141).
// Conflict: perf-lab's own `tx=1` experiment writes the same site; keep it 0 (the second writer refuses on bytes).
//
// Config: $G/texmem.txt `enabled=0` disables (site untouched). Log (1.1.0): shared KpLog.h (channel main). The load line (+ tick=) is
// formatted in DllMain, emitted by KPatchInit; a helper thread appends COUNTERS every 30 s. Old core: fallback texmem_log.txt at INFO.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "CodePatch.h"
#include "IniKit.h"
#include "KpLog.h"

KP_LOG_PATCH("k2-texture-memory-release");

namespace {

constexpr uintptr_t Site = 0x004261CF, TxFree = 0x004261D6, TxSkip = 0x004261E6;
const uint8_t SiteOrig[7] = { 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x75, 0x10 };

volatile LONG g_freed = 0, g_kept = 0;
int g_state = 0;            // 1 patched, 0 disabled, -1 refused (bytes), -2 refused (protect), -3 refused (unmapped), -4 no cave
uint8_t g_seen[7]; uint8_t* g_cave = nullptr;

KP_CHANNEL(chMain, "main");

// Only stores the path and the level (no I/O): safe from a static initialiser. Old core: same file as 1.0.0, at INFO.
const bool g_fallbackSet = (KpLog_SetFallbackFile("texmem_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// The load line is formatted in DllMain (setLoad) but emitted later, once the logger is known: by KPatchInit (whether or not
// KpLog_Init succeeded), else by the first hook call / helper-thread tick (old core without KPatchInit).
char g_loadLine[400]; int g_loadLevel = KPLOG_INFO;
volatile LONG g_loadEmitted = 0;
void setLoad(int level, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); vsnprintf(g_loadLine, sizeof(g_loadLine), fmt, ap); va_end(ap);
    size_t n = strlen(g_loadLine);
    snprintf(g_loadLine + n, sizeof(g_loadLine) - n, " tick=%lu", GetTickCount());
    g_loadLevel = level;
}
void emitLoadLine() {
    if (g_loadEmitted) return;                         // cheap fast path (hot after the first call)
    if (InterlockedExchange(&g_loadEmitted, 1)) return;
    KP_LOGV(chMain, g_loadLevel, "%s", g_loadLine);
}

bool configEnabled() { return inikit::enabledFile("texmem.txt"); }

void putAbs(uint8_t* at, volatile LONG* p) { uint32_t a = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p)); memcpy(at, &a, 4); }

int apply() {
    if (!codepatch::committed(Site, 7)) return -3;
    memcpy(g_seen, reinterpret_cast<const void*>(Site), 7);
    if (memcmp(g_seen, SiteOrig, 7) != 0) return -1;
    g_cave = codepatch::allocCave(64);
    if (!g_cave) return -4;
    uint8_t* c = g_cave;
    // +0  0F B6 C8        movzx ecx,al
    // +3  85 C9           test ecx,ecx
    // +5  74 21           jz nontpc (+40)
    // +7  8B 55 FC        mov edx,[ebp-4]
    // +10 83 7A 34 00     cmp dword [edx+0x34],0
    // +14 75 0C           jnz skip (+28)
    // +16 F0 FF 05 abs32  lock inc [g_freed]
    // +23 E9 rel32        jmp 0x4261D6
    // +28 F0 FF 05 abs32  skip: lock inc [g_kept]
    // +35 E9 rel32        jmp 0x4261E6
    // +40 E9 rel32        nontpc: jmp 0x4261D6
    static const uint8_t head[16] = { 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x74, 0x21, 0x8B, 0x55, 0xFC, 0x83, 0x7A, 0x34, 0x00, 0x75, 0x0C };
    memcpy(c, head, 16);
    c[16] = 0xF0; c[17] = 0xFF; c[18] = 0x05; putAbs(c + 19, &g_freed);
    c[23] = 0xE9; codepatch::putRel32(c + 24, reinterpret_cast<uintptr_t>(c + 24), TxFree);
    c[28] = 0xF0; c[29] = 0xFF; c[30] = 0x05; putAbs(c + 31, &g_kept);
    c[35] = 0xE9; codepatch::putRel32(c + 36, reinterpret_cast<uintptr_t>(c + 36), TxSkip);
    c[40] = 0xE9; codepatch::putRel32(c + 41, reinterpret_cast<uintptr_t>(c + 41), TxFree);
    FlushInstructionCache(GetCurrentProcess(), c, 45);
    uint8_t jmp[7]; codepatch::jmpRel(Site, reinterpret_cast<uintptr_t>(c), jmp, 7);
    if (codepatch::write(Site, jmp, 7) != 1) return -2;
    return 1;
}

DWORD WINAPI helperThread(LPVOID) {
    LONG lf = -1, lk = -1;
    for (;;) {
        Sleep(30000);
        emitLoadLine();
        LONG f = g_freed, k = g_kept;
        if (f != lf || k != lk) { KP_LOG(chMain, "COUNTERS tpc_freed=%ld tpc_kept_controller=%ld", f, k); lf = f; lk = k; }
    }
}

}  // namespace

// Harness-only exports (never called by the game).
extern "C" int __cdecl TexMemTestState() { return g_state; }
extern "C" void __cdecl TexMemTestCounters(long* out2) { out2[0] = g_freed; out2[1] = g_kept; }

// Written out literally (create-patch's export scan reads the source text) and listed in exports.def.
extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); emitLoadLine(); }

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    if (!configEnabled()) {
        setLoad(KPLOG_INFO, "k2-texture-memory-release 0.1.1 loaded: DISABLED by texmem.txt (site 0x004261cf untouched) pid=%lu", GetCurrentProcessId());
        emitLoadLine();   // unpatched: nothing later emits it
        return TRUE;
    }
    g_state = apply();
    if (g_state == 1)
        setLoad(KPLOG_INFO, "k2-texture-memory-release 0.1.1 loaded: site 0x004261cf patched (cave %p) pid=%lu", (void*)g_cave, GetCurrentProcessId());
    else if (g_state == -1)
        setLoad(KPLOG_WARN, "k2-texture-memory-release 0.1.1 loaded: REFUSED(bytes %02x %02x %02x %02x %02x %02x %02x at 0x004261cf; perf-lab tx=1 active?) site untouched pid=%lu",
             g_seen[0], g_seen[1], g_seen[2], g_seen[3], g_seen[4], g_seen[5], g_seen[6], GetCurrentProcessId());
    else
        setLoad(KPLOG_WARN, "k2-texture-memory-release 0.1.1 loaded: REFUSED(%s) site untouched pid=%lu", g_state == -2 ? "protect" : g_state == -3 ? "unmapped" : "cave alloc", GetCurrentProcessId());
    // Unpatched: no thread or hook runs later to emit the line, so write it now (fallback sink, as 1.0.0 did).
    if (g_state == 1) CloseHandle(CreateThread(nullptr, 0, helperThread, nullptr, 0, nullptr));
    else emitLoadLine();
    return TRUE;
}
