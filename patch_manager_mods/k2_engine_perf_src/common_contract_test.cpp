// common_contract_test.cpp -- Common CONTRACT test for K2 Engine Perf
// Pins the semantics this mod relies on from _shared_src (SafeRead, IniKit, FileWatch, CodePatch).
#include "CodePatch.h"
#include "FileWatch.h"
#include "IniKit.h"
#include "K2EnginePerfLogic.h"
#include "SafeRead.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

#define LCHECK(why, expr) ::kt::Check("k2_engine_perf relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))

int main() {
    g_stubVirtualQueryAll = true;

    // 1. SafeRead contract (init-time probes of the exe globals and the IAT slot)
    uint32_t word = 0x00986128u;
    uint32_t out = 0;
    LCHECK("probeRead accepts a committed 4-byte global", saferead::probeRead(1, &word, sizeof(word)));
    LCHECK("readAt reads a uint32", saferead::readAt<uint32_t>(2, reinterpret_cast<uintptr_t>(&word), 0, &out) && out == word);

    // 2. IniKit contract
    char sample[] = "[EnginePerf]\ntex_scan_cache = 1\nmem_status_ms = 500\n";
    k2ep::Config cfg{};
    inikit::LoadStats st = k2ep::parseIniText(sample, cfg);
    LCHECK("forEachLine parses 2 key-value lines and skips [EnginePerf] as noEquals", st.ok == 2 && st.bad == 0 && st.noEquals == 1);
    LCHECK("parsed mem_status_ms", cfg.memStatusMs == 500);

    // 3. FileWatch contract
    filewatch::ReloadPoll poll{};
    LCHECK("first ReloadPoll::changed returns true", filewatch::changed(poll, "/nonexistent/k2_engine_perf.ini", 1000));
    LCHECK("throttled ReloadPoll::changed within 1000ms returns false", !filewatch::changed(poll, "/nonexistent/k2_engine_perf.ini", 1500));

    // 4. CodePatch contract (5-byte E8 rel32 and 6-byte FF 15 rewrites + FreeLibrary restore)
    uint8_t site[6] = {0xFF, 0x15, 0x28, 0x61, 0x98, 0x00};
    const uint8_t orig[6] = {0xFF, 0x15, 0x28, 0x61, 0x98, 0x00};
    const uint8_t repl[6] = {0xFF, 0x15, 0x10, 0x20, 0x30, 0x40};
    uint8_t seen[6] = {};
    LCHECK("writeChecked writes when the site matches", codepatch::writeChecked(reinterpret_cast<uintptr_t>(site), orig, repl, 6, seen) == 1 && memcmp(site, repl, 6) == 0);
    LCHECK("writeChecked refuses a mismatch (-1), fills seen, writes nothing",
           codepatch::writeChecked(reinterpret_cast<uintptr_t>(site), orig, repl, 6, seen) == -1 && memcmp(seen, repl, 6) == 0);
    LCHECK("restore with expect = repl puts the original back", codepatch::writeChecked(reinterpret_cast<uintptr_t>(site), repl, orig, 6, seen) == 1 && memcmp(site, orig, 6) == 0);
    LCHECK("writeChecked refuses n <= 0 with -4", codepatch::writeChecked(reinterpret_cast<uintptr_t>(site), orig, repl, 0, seen) == -4);

    return kt::Report("k2_engine_perf_contract_test");
}
