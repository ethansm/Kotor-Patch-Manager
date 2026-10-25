// common_contract_test.cpp -- Common CONTRACT test for K2 Grass Draw Distance (doc 48)
// Pins the semantics this mod relies on from _shared_src (SafeRead, IniKit, FileWatch, ModConfigDecl, ProviderClient).
#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "FileWatch.h"
#include "IniKit.h"
#include "K2GrassDrawDistanceLogic.h"
#include "ModConfigDecl.h"
#include "ProviderClient.h"
#include "SafeRead.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

#define LCHECK(why, expr) ::kt::Check("k2_grass_draw_distance relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))

int main() {
    g_stubVirtualQueryAll = true;

    // 1. SafeRead contract
    float val = 225.0f;
    float outVal = 0.0f;
    LCHECK("readAt reads valid float", saferead::readAt<float>(1, reinterpret_cast<uintptr_t>(&val), 0, &outVal) && outVal == 225.0f);
    LCHECK("probeRead detects valid float buffer", saferead::probeRead(1, &val, sizeof(val)));

    // 2. IniKit contract
    char sample[] = "[Grass]\nenabled = 1\nfade_start_dist = 75\nfade_end_dist = 120\nwind_dist = 25\nalpha_cutoff = 15\n";
    k2grass::Config cfg{};
    inikit::LoadStats st = k2grass::parseIniText(sample, cfg);
    LCHECK("forEachLine parses 5 key-value lines and skips [Grass] as noEquals", st.ok == 5 && st.bad == 0 && st.noEquals == 1);
    LCHECK("parsed fade_start_dist and fade_end_dist", cfg.fadeStartDist == 75.0f && cfg.fadeEndDist == 120.0f);
    LCHECK("parsed scaled alpha_cutoff 15 -> 0.15f", std::fabs(cfg.alphaCutoff - 0.15f) < 1e-5f);

    // 3. ModConfigDecl schema contract
    mcdecl::PageDecl pd{};
    char err[96] = {};
    LCHECK("kPageSchema parses cleanly with ModConfigDecl",
           mcdecl::parseSchema(k2grass::kPageSchema, strlen(k2grass::kPageSchema), pd, err, sizeof(err)));
    LCHECK("kPageSchema has 5 rows", pd.nRows == 5);
    LCHECK("kPageSchema title has K2 in name", strcmp(pd.name, "K2 Grass Draw Distance") == 0);

    // 4. FileWatch contract
    filewatch::ReloadPoll poll{};
    LCHECK("first ReloadPoll::changed returns true", filewatch::changed(poll, "/nonexistent/k2_grass_draw_distance.ini", 1000));
    LCHECK("throttled ReloadPoll::changed within 1000ms returns false", !filewatch::changed(poll, "/nonexistent/k2_grass_draw_distance.ini", 1500));

    return kt::Report("k2_grass_draw_distance_contract_test");
}
