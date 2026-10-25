// common_contract_test.cpp -- Common CONTRACT test for K2 Shadow Distance
// Pins the semantics this mod relies on from _shared_src (SafeRead, IniKit, FileWatch, ModConfigDecl, ProviderClient,
// CodePatch).
#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "CodePatch.h"
#include "FileWatch.h"
#include "IniKit.h"
#include "K2ShadowDistanceLogic.h"
#include "ModConfigDecl.h"
#include "ProviderClient.h"
#include "SafeRead.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

#define LCHECK(why, expr) ::kt::Check("k2_shadow_distance relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))

int main() {
    g_stubVirtualQueryAll = true;

    // 1. SafeRead contract
    float val = 35.0f;
    float outVal = 0.0f;
    int32_t flag = 1;
    int32_t outFlag = 0;
    LCHECK("readAt reads valid float", saferead::readAt<float>(1, reinterpret_cast<uintptr_t>(&val), 0, &outVal) && outVal == 35.0f);
    LCHECK("readAt reads valid int", saferead::readAt<int32_t>(2, reinterpret_cast<uintptr_t>(&flag), 0, &outFlag) && outFlag == 1);
    LCHECK("probeRead detects valid float buffer", saferead::probeRead(1, &val, sizeof(val)));

    // 2. IniKit contract
    char sample[] = "[Shadows]\nenabled = 1\nshadow_dist = 70\nperf_log = 0\n";
    k2shadow::Config cfg{};
    inikit::LoadStats st = k2shadow::parseIniText(sample, cfg);
    LCHECK("forEachLine parses 3 key-value lines and skips [Shadows] as noEquals", st.ok == 3 && st.bad == 0 && st.noEquals == 1);
    LCHECK("parsed shadow_dist and perf_log", cfg.shadowDist == 70.0f && !cfg.perfLog);

    // 3. ModConfigDecl schema contract
    mcdecl::PageDecl pd{};
    char err[96] = {};
    LCHECK("kPageSchema parses cleanly with ModConfigDecl",
           mcdecl::parseSchema(k2shadow::kPageSchema, strlen(k2shadow::kPageSchema), pd, err, sizeof(err)));
    LCHECK("kPageSchema has 2 rows", pd.nRows == 2);
    LCHECK("kPageSchema title has K2 in name", strcmp(pd.name, "K2 Shadow Distance") == 0);

    // 4. FileWatch contract
    filewatch::ReloadPoll poll{};
    LCHECK("first ReloadPoll::changed returns true", filewatch::changed(poll, "/nonexistent/k2_shadow_distance.ini", 1000));
    LCHECK("throttled ReloadPoll::changed within 1000ms returns false", !filewatch::changed(poll, "/nonexistent/k2_shadow_distance.ini", 1500));

    // 5. CodePatch contract (view-reference FLD operand rewrite + FreeLibrary restore)
    uint8_t site[6] = {0xD9, 0x05, 0x98, 0xBA, 0x98, 0x00};
    const uint8_t orig[6] = {0xD9, 0x05, 0x98, 0xBA, 0x98, 0x00};
    const uint8_t repl[6] = {0xD9, 0x05, 0x10, 0x20, 0x30, 0x40};
    uint8_t seen[6] = {};
    LCHECK("writeChecked writes when the site matches", codepatch::writeChecked(reinterpret_cast<uintptr_t>(site), orig, repl, 6, seen) == 1 && memcmp(site, repl, 6) == 0);
    LCHECK("writeChecked refuses a mismatch (-1), fills seen, writes nothing",
           codepatch::writeChecked(reinterpret_cast<uintptr_t>(site), orig, repl, 6, seen) == -1 && memcmp(seen, repl, 6) == 0 && memcmp(site, repl, 6) == 0);
    LCHECK("restore with expect = repl puts the original back", codepatch::writeChecked(reinterpret_cast<uintptr_t>(site), repl, orig, 6, seen) == 1 && memcmp(site, orig, 6) == 0);

    return kt::Report("k2_shadow_distance_contract_test");
}
