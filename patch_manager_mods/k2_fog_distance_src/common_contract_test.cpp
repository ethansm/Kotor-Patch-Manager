// common_contract_test.cpp -- Common CONTRACT test for K2 Fog Distance
// Pins the semantics this mod relies on from _shared_src (SafeRead, IniKit, FileWatch, ModConfigDecl, ProviderClient).
#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "FileWatch.h"
#include "IniKit.h"
#include "K2FogDistanceLogic.h"
#include "ModConfigDecl.h"
#include "ProviderClient.h"
#include "SafeRead.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

#define LCHECK(why, expr) ::kt::Check("k2_fog_distance relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))

int main() {
    g_stubVirtualQueryAll = true;

    // 1. SafeRead contract: scene field reads at an offset
    struct { int32_t pad[24]; int32_t on; float start; float end; } scene{};
    scene.on = 1;
    scene.start = 20.0f;
    scene.end = 60.0f;
    const uintptr_t base = reinterpret_cast<uintptr_t>(&scene);
    int32_t on = 0;
    float end = 0.0f;
    LCHECK("readAt reads int at +0x60", saferead::readAt<int32_t>(1, base, 0x60, &on) && on == 1);
    LCHECK("readAt reads float at +0x68", saferead::readAt<float>(3, base, 0x68, &end) && end == 60.0f);

    // 2. IniKit contract
    char sample[] = "[Fog]\nenabled = 1\nfog_end_pct = 300\nfog_start_pct = 125\n";
    k2fog::Config cfg{};
    inikit::LoadStats st = k2fog::parseIniText(sample, cfg);
    LCHECK("forEachLine parses 3 key-value lines and skips [Fog] as noEquals", st.ok == 3 && st.bad == 0 && st.noEquals == 1);
    LCHECK("parsed percentages", cfg.fogEndPct == 300 && cfg.fogStartPct == 125);

    // 3. ModConfigDecl schema contract
    mcdecl::PageDecl pd{};
    char err[96] = {};
    LCHECK("kPageSchema parses cleanly with ModConfigDecl",
           mcdecl::parseSchema(k2fog::kPageSchema, strlen(k2fog::kPageSchema), pd, err, sizeof(err)));
    LCHECK("kPageSchema has 3 rows", pd.nRows == 3);
    LCHECK("kPageSchema title has K2 in name", strcmp(pd.name, "K2 Fog Distance") == 0);

    // 4. FileWatch contract
    filewatch::ReloadPoll poll{};
    LCHECK("first ReloadPoll::changed returns true", filewatch::changed(poll, "/nonexistent/k2_fog_distance.ini", 1000));
    LCHECK("throttled ReloadPoll::changed within 1000ms returns false", !filewatch::changed(poll, "/nonexistent/k2_fog_distance.ini", 1500));

    return kt::Report("k2_fog_distance_contract_test");
}
