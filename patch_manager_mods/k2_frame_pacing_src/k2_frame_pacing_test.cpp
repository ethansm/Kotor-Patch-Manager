// k2_frame_pacing_test.cpp -- Unit tests for K2FramePacingLogic.h.
#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "K2FramePacingLogic.h"
#include "check.h"
#include <string.h>

using k2fp::Config;
using k2fp::Mode;

static void test_defaults() {
    Config d{};
    KT_CHECK(d.enabled);
    KT_CHECK(d.minimizedSleep);
    KT_CHECK_EQ(d.minimizedSleepMs, 50);
    KT_CHECK_EQ(d.backgroundFps, 30);
    KT_CHECK_EQ(d.fpsCap, 0);
    KT_CHECK_EQ(d.spinUs, 1500);
    KT_CHECK(d.perfLog);
    KT_CHECK(!d.verbose);
    KT_CHECK(d == Config{});
    Config e = d;
    e.fpsCap = 60;
    KT_CHECK(e != d);
    KT_CHECK(strcmp(k2fp::modeName(Mode::Active), "active") == 0);
    KT_CHECK(strcmp(k2fp::modeName(Mode::Background), "background") == 0);
    KT_CHECK(strcmp(k2fp::modeName(Mode::Minimized), "minimized") == 0);
}

static void test_sanitize() {
    Config c{};
    c.backgroundFps = 1;
    c.fpsCap = 1;
    c.sanitize();
    KT_CHECK_EQ(c.backgroundFps, 5);
    KT_CHECK_EQ(c.fpsCap, 20);
    c.backgroundFps = 4;
    c.fpsCap = 19;
    c.sanitize();
    KT_CHECK_EQ(c.backgroundFps, 5);
    KT_CHECK_EQ(c.fpsCap, 20);
    c.backgroundFps = 5;
    c.fpsCap = 20;
    c.sanitize();
    KT_CHECK_EQ(c.backgroundFps, 5);
    KT_CHECK_EQ(c.fpsCap, 20);
    c.backgroundFps = 0;
    c.fpsCap = 0;
    c.sanitize();
    KT_CHECK_EQ(c.backgroundFps, 0);
    KT_CHECK_EQ(c.fpsCap, 0);
    c.backgroundFps = -3;
    c.fpsCap = -3;
    c.sanitize();
    KT_CHECK_EQ(c.backgroundFps, 0);
    KT_CHECK_EQ(c.fpsCap, 0);
    c.backgroundFps = 9999;
    c.fpsCap = 9999;
    c.minimizedSleepMs = 9999;
    c.spinUs = 99999;
    c.sanitize();
    KT_CHECK_EQ(c.backgroundFps, 120);
    KT_CHECK_EQ(c.fpsCap, 360);
    KT_CHECK_EQ(c.minimizedSleepMs, 200);
    KT_CHECK_EQ(c.spinUs, 5000);
    c.minimizedSleepMs = 0;
    c.spinUs = -5;
    c.sanitize();
    KT_CHECK_EQ(c.minimizedSleepMs, 5);
    KT_CHECK_EQ(c.spinUs, 0);
}

static void test_target_and_sleep() {
    Config c{};
    KT_CHECK_EQ(k2fp::targetFrameUs(c, Mode::Active), 0);   // fps_cap 0 = off
    KT_CHECK_EQ(k2fp::targetFrameUs(c, Mode::Background), 33333);
    KT_CHECK_EQ(k2fp::targetFrameUs(c, Mode::Minimized), 0);
    c.fpsCap = 60;
    KT_CHECK_EQ(k2fp::targetFrameUs(c, Mode::Active), 16666);
    c.fpsCap = 100;
    KT_CHECK_EQ(k2fp::targetFrameUs(c, Mode::Active), 10000);
    c.backgroundFps = 0;
    KT_CHECK_EQ(k2fp::targetFrameUs(c, Mode::Background), 0);
    c.enabled = false;
    KT_CHECK_EQ(k2fp::targetFrameUs(c, Mode::Active), 0);
    c.backgroundFps = 30;
    KT_CHECK_EQ(k2fp::targetFrameUs(c, Mode::Background), 0);

    Config f{};
    KT_CHECK_EQ(k2fp::fixedSleepMs(f, Mode::Minimized), 50u);
    KT_CHECK_EQ(k2fp::fixedSleepMs(f, Mode::Active), 0u);
    KT_CHECK_EQ(k2fp::fixedSleepMs(f, Mode::Background), 0u);
    f.minimizedSleepMs = 80;
    KT_CHECK_EQ(k2fp::fixedSleepMs(f, Mode::Minimized), 80u);
    f.minimizedSleep = false;
    KT_CHECK_EQ(k2fp::fixedSleepMs(f, Mode::Minimized), 0u);
    f.minimizedSleep = true;
    f.enabled = false;
    KT_CHECK_EQ(k2fp::fixedSleepMs(f, Mode::Minimized), 0u);
}

static void test_cap_sleep() {
    KT_CHECK_EQ(k2fp::capSleepMs(10000, 1500), 8u);
    KT_CHECK_EQ(k2fp::capSleepMs(1500, 1500), 0u);
    KT_CHECK_EQ(k2fp::capSleepMs(1000, 1500), 0u);
    KT_CHECK_EQ(k2fp::capSleepMs(0, 1500), 0u);
    KT_CHECK_EQ(k2fp::capSleepMs(-500, 1500), 0u);
    KT_CHECK_EQ(k2fp::capSleepMs(2499, 1500), 0u);
    KT_CHECK_EQ(k2fp::capSleepMs(2500, 1500), 1u);
    KT_CHECK_EQ(k2fp::capSleepMs(5000, 0), 5u);
    KT_CHECK_EQ(k2fp::capSleepMs(-1, 0), 0u);
}

static void test_perf_window() {
    k2fp::PerfWindow w{};
    KT_CHECK(!w.due(100000));   // not started
    w.reset(1000);
    KT_CHECK(!w.due(5999));
    KT_CHECK(w.due(6000));

    // GetTickCount wrap: start just before 2^32.
    k2fp::PerfWindow g{};
    g.reset(0xFFFFFC00u);
    KT_CHECK(!g.due(0xFFFFFFFFu));
    KT_CHECK(!g.due(0x00000F00u));   // 4864 ms elapsed across the wrap
    KT_CHECK(g.due(0x00001000u));
}

static void test_perf_format() {
    k2fp::PerfWindow w{};
    w.reset(0);
    w.addFrame(Mode::Active, 4000, 10000, 1000);
    w.addFrame(Mode::Active, 8000, 8000, 2000);
    w.addFrame(Mode::Background, 6000, 20000, 1500);
    w.addFrame(Mode::Minimized, 2000, 50000, 0);
    KT_CHECK_EQ(w.frames[0], 2u);
    KT_CHECK_EQ(w.frames[1], 1u);
    KT_CHECK_EQ(w.frames[2], 1u);
    KT_CHECK_EQ(w.workMaxUs, 8000u);
    char buf[256] = {};
    const int n = w.format(buf, sizeof(buf), 2000);
    KT_CHECK(n > 0 && static_cast<size_t>(n) < sizeof(buf));
    KT_CHECK(strstr(buf, "PERF fps=2.0 ") == buf);   // 4 frames / 2 s
    KT_CHECK(strstr(buf, "active=2") != nullptr);
    KT_CHECK(strstr(buf, "bg=1") != nullptr);
    KT_CHECK(strstr(buf, "min=1") != nullptr);
    KT_CHECK(strstr(buf, "work_ms_avg=5.00") != nullptr);
    KT_CHECK(strstr(buf, "work_ms_max=8.00") != nullptr);
    KT_CHECK(strstr(buf, "slept_ms=88") != nullptr);
    KT_CHECK(strstr(buf, "spun_ms=4") != nullptr);
    w.reset(2000);
    KT_CHECK_EQ(w.totalFrames(), 0u);
    w.format(buf, sizeof(buf), 2000);   // zero elapsed must not divide by zero
    KT_CHECK(strstr(buf, "fps=0.0") != nullptr);
}

static void test_ini() {
    char all[] =
        "enabled = 0\nminimized_sleep = off\nminimized_sleep_ms = 80\nbackground_fps = 15\n"
        "fps_cap = 144\nspin_us = 2000\nperf_log = no\nverbose = yes\n";
    Config c{};
    inikit::LoadStats st = k2fp::parseIniText(all, c);
    KT_CHECK_EQ(st.ok, 8);
    KT_CHECK_EQ(st.bad, 0);
    KT_CHECK_EQ(st.unknown, 0);
    KT_CHECK(!c.enabled);
    KT_CHECK(!c.minimizedSleep);
    KT_CHECK_EQ(c.minimizedSleepMs, 80);
    KT_CHECK_EQ(c.backgroundFps, 15);
    KT_CHECK_EQ(c.fpsCap, 144);
    KT_CHECK_EQ(c.spinUs, 2000);
    KT_CHECK(!c.perfLog);
    KT_CHECK(c.verbose);

    char bad[] = "enabled = maybe\nfps_cap = fast\nbackground_fps = 12x\nspin_us = 100\nmystery = 1\nminimized_sleep_ms =\n";
    Config b{};
    st = k2fp::parseIniText(bad, b);
    KT_CHECK_EQ(st.bad, 4);
    KT_CHECK_EQ(st.unknown, 1);
    KT_CHECK(b.enabled);
    KT_CHECK_EQ(b.fpsCap, 0);
    KT_CHECK_EQ(b.backgroundFps, 30);
    KT_CHECK_EQ(b.minimizedSleepMs, 50);
    KT_CHECK_EQ(b.spinUs, 100);

    char lo[] = "background_fps = 2\nfps_cap = 7\nminimized_sleep_ms = 1\n";
    k2fp::parseIniText(lo, c);
    KT_CHECK_EQ(c.backgroundFps, 5);
    KT_CHECK_EQ(c.fpsCap, 20);
    KT_CHECK_EQ(c.minimizedSleepMs, 5);
}

static void test_ini_roundtrip() {
    char buf[2048] = {};
    Config orig{};
    orig.enabled = false;
    orig.minimizedSleep = false;
    orig.minimizedSleepMs = 120;
    orig.backgroundFps = 60;
    orig.fpsCap = 90;
    orig.spinUs = 800;
    orig.perfLog = false;
    orig.verbose = true;
    int n = k2fp::formatDefaultIni(buf, sizeof(buf), orig);
    KT_CHECK(n > 0 && static_cast<size_t>(n) < sizeof(buf));
    Config parsed{};
    inikit::LoadStats st = k2fp::parseIniText(buf, parsed);
    KT_CHECK_EQ(st.bad, 0);
    KT_CHECK_EQ(st.unknown, 0);
    KT_CHECK_EQ(st.ok, 8);
    KT_CHECK(parsed == orig);

    n = k2fp::formatDefaultIni(buf, sizeof(buf));
    KT_CHECK(n > 0 && static_cast<size_t>(n) < sizeof(buf));
    parsed = orig;
    st = k2fp::parseIniText(buf, parsed);
    KT_CHECK_EQ(st.bad, 0);
    KT_CHECK(parsed == Config{});
}

static void test_modconfig_schema() {
    mcdecl::PageDecl pd{};
    char err[96] = {};
    KT_CHECK(mcdecl::parseSchema(k2fp::kPageSchema, strlen(k2fp::kPageSchema), pd, err, sizeof(err)));
    KT_CHECK_EQ(pd.nRows, 3);
    KT_CHECK(strcmp(pd.name, "K2 Frame Pacing") == 0);
    for (int i = 0; i < pd.nRows; ++i) {
        const mcdecl::RowDecl& r = pd.rows[i];
        KT_CHECK(strcmp(r.ini, k2fp::kIniFile) == 0);
        KT_CHECK(strcmp(r.section, k2fp::kIniSection) == 0);
        KT_CHECK(strlen(r.label) > 0 && strlen(r.label) <= 40);
        KT_CHECK(strlen(r.desc) > 0 && strlen(r.desc) < 95);
    }
    KT_CHECK(strcmp(pd.rows[0].key, "minimized_sleep") == 0);
    KT_CHECK(strcmp(pd.rows[1].key, "background_fps") == 0);
    KT_CHECK(strcmp(pd.rows[2].key, "fps_cap") == 0);
    KT_CHECK(strcmp(pd.rows[2].label, "FPS Cap") == 0);
}

int main() {
    test_defaults();
    test_sanitize();
    test_target_and_sleep();
    test_cap_sleep();
    test_perf_window();
    test_perf_format();
    test_ini();
    test_ini_roundtrip();
    test_modconfig_schema();
    return kt::Report("k2_frame_pacing_test");
}
