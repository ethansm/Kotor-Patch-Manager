// k2_engine_perf_test.cpp -- Unit tests for K2EnginePerfLogic.h.
#include "K2EnginePerfLogic.h"
#include "check.h"
#include <cmath>
#include <string.h>

static void test_defaults() {
    k2ep::Config c{};
    KT_CHECK(c.texScanCache);
    KT_CHECK(c.texVerifyEvery == 240);
    KT_CHECK(c.memStatusCache);
    KT_CHECK(c.memStatusMs == 250);
    KT_CHECK(c.perfLog);
    KT_CHECK(!c.verbose);
}

static void test_parse() {
    char text[] = "[EnginePerf]\ntex_scan_cache = 0\ntex_verify_every = 10\nmem_status_cache = off\nmem_status_ms = 1000\n"
                  "perf_log = 0\nverbose = yes\nunknown_key = 3\n";
    k2ep::Config c{};
    inikit::LoadStats st = k2ep::parseIniText(text, c);
    KT_CHECK(st.ok == 6);
    KT_CHECK(st.bad == 0);
    KT_CHECK(!c.texScanCache && c.texVerifyEvery == 10 && !c.memStatusCache && c.memStatusMs == 1000);
    KT_CHECK(!c.perfLog && c.verbose);

    // Bad values are rejected and the key keeps its default.
    char bad[] = "tex_scan_cache = maybe\nmem_status_ms = 12x\n";
    st = k2ep::parseIniText(bad, c);
    KT_CHECK(st.bad == 2);
    KT_CHECK(c.texScanCache && c.memStatusMs == 250);

    // Clamp.
    char wide[] = "tex_verify_every = -5\nmem_status_ms = 99999\n";
    k2ep::parseIniText(wide, c);
    KT_CHECK(c.texVerifyEvery == 0 && c.memStatusMs == 5000);
}

static void test_default_ini_roundtrip() {
    char buf[2048];
    const int n = k2ep::formatDefaultIni(buf, sizeof(buf));
    KT_CHECK(n > 0 && static_cast<size_t>(n) < sizeof(buf));
    KT_CHECK(strstr(buf, "[EnginePerf]") != nullptr);
    k2ep::Config c{};
    inikit::LoadStats st = k2ep::parseIniText(buf, c);
    KT_CHECK(st.ok == 6 && st.bad == 0);
    KT_CHECK(c == k2ep::Config{});
}

static void test_perf_window() {
    k2ep::PerfWindow w{};
    KT_CHECK(!w.due(100000));
    w.reset(1000);
    KT_CHECK(!w.due(5999));
    KT_CHECK(w.due(6000));
    // GetTickCount wrap.
    w.reset(0xFFFFF000u);
    KT_CHECK(!w.due(0x00000100u));
    KT_CHECK(w.due(0xFFFFF000u + 5000u));
}

static void test_estimate() {
    // 10 real calls of 1,000 ticks each at 1 MHz = 1,000 us average; 50 hits over 5 s = 10,000 us/s saved.
    k2ep::SavedEstimate e = k2ep::estimateSaved(10000, 10, 50, 1000000, 5.0f);
    KT_CHECK(std::fabs(e.avgUs - 1000.0) < 1e-6);
    KT_CHECK(std::fabs(e.savedUsPerSec - 10000.0) < 1e-6);
    e = k2ep::estimateSaved(10000, 0, 50, 1000000, 5.0f);
    KT_CHECK(e.avgUs == 0.0 && e.savedUsPerSec == 0.0);
    e = k2ep::estimateSaved(10000, 10, 50, 0, 5.0f);
    KT_CHECK(e.avgUs == 0.0);
}

int main() {
    test_defaults();
    test_parse();
    test_default_ini_roundtrip();
    test_perf_window();
    test_estimate();
    return kt::Report("k2_engine_perf_test");
}
