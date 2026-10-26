// common_contract_test.cpp -- Common CONTRACT test for K2 Frame Pacing
// Pins the semantics this mod relies on from _shared_src (SafeRead, IniKit, FileWatch, ModConfigDecl, ProviderClient).
#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "FileWatch.h"
#include "IniKit.h"
#include "K2FramePacingLogic.h"
#include "ModConfigDecl.h"
#include "ProviderClient.h"
#include "SafeRead.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

#define LCHECK(why, expr) ::kt::Check("k2_frame_pacing relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))

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
    char sample[] = "[FramePacing]\nenabled = 1\nbackground_fps = 20\nperf_log = 0\n";
    k2fp::Config cfg{};
    inikit::LoadStats st = k2fp::parseIniText(sample, cfg);
    LCHECK("forEachLine parses 3 key-value lines and skips [FramePacing] as noEquals", st.ok == 3 && st.bad == 0 && st.noEquals == 1);
    LCHECK("parsed background_fps and perf_log", cfg.backgroundFps == 20 && !cfg.perfLog);

    // 3. ModConfigDecl schema contract
    mcdecl::PageDecl pd{};
    char err[96] = {};
    LCHECK("kPageSchema parses cleanly with ModConfigDecl",
           mcdecl::parseSchema(k2fp::kPageSchema, strlen(k2fp::kPageSchema), pd, err, sizeof(err)));
    LCHECK("kPageSchema has 3 rows", pd.nRows == 3);
    LCHECK("kPageSchema title has K2 in name", strcmp(pd.name, "K2 Frame Pacing") == 0);

    // 4. FileWatch contract
    filewatch::ReloadPoll poll{};
    LCHECK("first ReloadPoll::changed returns true", filewatch::changed(poll, "/nonexistent/k2_frame_pacing.ini", 1000));
    LCHECK("throttled ReloadPoll::changed within 1000ms returns false", !filewatch::changed(poll, "/nonexistent/k2_frame_pacing.ini", 1500));

    return kt::Report("k2_frame_pacing_contract_test");
}
