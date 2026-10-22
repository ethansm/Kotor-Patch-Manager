// T1 host tests for HseSpeed.h (the Speed slider of the Holstered Weapons page, holster-editor 0.7.1-dev, HW44): the press / drag mapping, the thumb
// place, the value labels, the holster.ini read (flat key slow_holster_x, last line wins, commented or absent = 1, a key past 16 KB found, a real 21 KB
// file read from disk) and the write round trip through iniw::setKey.
// Build/run: python3 scripts/run_tests.py hse_speed
#include "HseSpeed.h"
#include "check.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace hse;

static int parse(const std::string& s) { return speedFromIniBuf(s.data(), s.size()); }

static void testMapping() {
    KT_CHECK_EQ(kSpeedMin, 1);
    KT_CHECK_EQ(kSpeedMax, 8);
    KT_CHECK_EQ(kSpeedDefault, 1);
    KT_CHECK_EQ(speedClamp(0), 1);
    KT_CHECK_EQ(speedClamp(9), 8);
    KT_CHECK_EQ(speedClamp(4), 4);
    // a track at x = 100, 200 px wide, a 10 px thumb: the thumb centre travels from 105 to 295
    KT_CHECK_EQ(speedFromX(0, 100, 200, 10), kSpeedMin);
    KT_CHECK_EQ(speedFromX(100, 100, 200, 10), kSpeedMin);
    KT_CHECK_EQ(speedFromX(105, 100, 200, 10), kSpeedMin);     // the thumb centre at the left end
    KT_CHECK_EQ(speedFromX(295, 100, 200, 10), kSpeedMax);     // and at the right end
    KT_CHECK_EQ(speedFromX(300, 100, 200, 10), kSpeedMax);
    KT_CHECK_EQ(speedFromX(100000, 100, 200, 10), kSpeedMax);
    KT_CHECK_EQ(speedFromX(200, 100, 200, 10), 5);             // half way (position 0.5 of 7 steps = 4.5, rounded half up)

    // every value is reachable, and the value only rises with the cursor (a smooth, monotonic drag)
    int prev = kSpeedMin, seen[kSpeedMax + 1] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int px = 0; px <= 320; ++px) {
        const int v = speedFromX(px, 100, 200, 10);
        KT_CHECK(v >= prev && v >= kSpeedMin && v <= kSpeedMax);
        seen[v] = 1;
        prev = v;
    }
    for (int v = kSpeedMin; v <= kSpeedMax; ++v) KT_CHECK(seen[v]);

    // no travel (the track is no wider than the thumb): always 1, and the thumb stays at the track start; a zero-width track never moves the value
    KT_CHECK_EQ(speedFromX(500, 10, 10, 10), kSpeedMin);
    KT_CHECK_EQ(speedFromX(100, 100, 0, 10), kSpeedMin);
    KT_CHECK_EQ(speedThumbX(8, 30, 10, 10), 30);
}

static void testThumbPlace() {
    // the thumb's left edge: the track start at 1, the track's right end at 8; a press at the thumb centre gives the value back (no jump on a grab)
    const int widths[] = {24, 60, 200, 333, 1000};
    for (int W : widths) {
        for (int v = kSpeedMin; v <= kSpeedMax; ++v) {
            const int X = speedThumbX(v, 50, W, 10);
            KT_CHECK(X >= 50 && X + 10 <= 50 + W);
            KT_CHECK_EQ(speedFromX(X + 5, 50, W, 10), v);
        }
        KT_CHECK_EQ(speedThumbX(kSpeedMin, 50, W, 10), 50);
        KT_CHECK_EQ(speedThumbX(kSpeedMax, 50, W, 10), 50 + W - 10);
    }
    KT_CHECK_EQ(speedThumbX(0, 50, 200, 10), 50);               // values outside 1..8 are clamped
    KT_CHECK_EQ(speedThumbX(99, 50, 200, 10), 50 + 200 - 10);
}

static void testText() {
    char t[32];
    speedText(t, sizeof t, 4);
    KT_CHECK(strcmp(t, "Speed 4x") == 0);
    speedText(t, sizeof t, 0);
    KT_CHECK(strcmp(t, "Speed 1x") == 0);
    speedText(t, sizeof t, 12);
    KT_CHECK(strcmp(t, "Speed 8x") == 0);   // clamped
    speedTextShort(t, sizeof t, 5);
    KT_CHECK(strcmp(t, "5x") == 0);
    speedTextShort(t, sizeof t, 99);
    KT_CHECK(strcmp(t, "8x") == 0);
    speedText(nullptr, 0, 3);               // no crash
    speedTextShort(nullptr, 0, 3);
    KT_CHECK(strlen(speedTip()) < 190);     // the description pane text (ASCII, under the mod-config item text cap for hover help)
}

static void testValueText() {
    KT_CHECK_EQ(speedFromValueText("1", 1), 1);
    KT_CHECK_EQ(speedFromValueText("4", 1), 4);
    KT_CHECK_EQ(speedFromValueText(" 5 ", 3), 5);
    KT_CHECK_EQ(speedFromValueText("2.0", 3), 2);
    KT_CHECK_EQ(speedFromValueText("2.6", 3), 3);               // a float rounds to the nearest step
    KT_CHECK_EQ(speedFromValueText("2.5", 3), 3);               // half up
    KT_CHECK_EQ(speedFromValueText("0", 1), kSpeedMin);
    KT_CHECK_EQ(speedFromValueText("-3", 2), kSpeedMin);
    KT_CHECK_EQ(speedFromValueText("99", 2), kSpeedMax);
    KT_CHECK_EQ(speedFromValueText("1e9", 3), kSpeedMax);
    KT_CHECK_EQ(speedFromValueText("inf", 3), kSpeedMax);       // strtod reads "inf": clamped, never a cast of inf
    KT_CHECK_EQ(speedFromValueText("abc", 3), kSpeedDefault);   // unreadable
    KT_CHECK_EQ(speedFromValueText("nan", 3), kSpeedDefault);
    KT_CHECK_EQ(speedFromValueText("", 0), kSpeedDefault);
    KT_CHECK_EQ(speedFromValueText(nullptr, 3), kSpeedDefault);
}

static void testIniBuf() {
    KT_CHECK_EQ(parse(""), 1);
    KT_CHECK_EQ(speedFromIniBuf(nullptr, 0), 1);
    KT_CHECK_EQ(parse("probe=1\nplace_mode=1\n"), 1);                   // absent
    KT_CHECK_EQ(parse("slow_holster_x=3\n"), 3);
    KT_CHECK_EQ(parse("slow_holster_x=3"), 3);                          // no trailing newline
    KT_CHECK_EQ(parse("probe=0\r\nslow_holster_x=4\r\n"), 4);           // CRLF
    KT_CHECK_EQ(parse("slow_holster_x = 5\r\n"), 5);                    // spaces around '='
    KT_CHECK_EQ(parse("   slow_holster_x\t=\t2  ; one x slower\n"), 2); // indent, tabs, inline comment
    KT_CHECK_EQ(parse("slow_holster_x=6 ; note\n"), 6);
    KT_CHECK_EQ(parse("SLOW_HOLSTER_X = 2\n"), 2);                      // the key is case-insensitive (as iniw::updateFile matches it)
    KT_CHECK_EQ(parse("slow_holster_x=2.0\n"), 2);
    KT_CHECK_EQ(parse("slow_holster_x=2.6\n"), 3);                      // rounded to the nearest whole step
    KT_CHECK_EQ(parse("slow_holster_x=0\n"), 1);                        // clamped
    KT_CHECK_EQ(parse("slow_holster_x=12\n"), 8);
    KT_CHECK_EQ(parse("slow_holster_x=-4\n"), 1);
    KT_CHECK_EQ(parse("slow_holster_x=abc\n"), 1);                      // no number: default
    KT_CHECK_EQ(parse("slow_holster_x=\n"), 1);
    KT_CHECK_EQ(parse("; slow_holster_x=6\n"), 1);                      // a comment line is not the key
    KT_CHECK_EQ(parse("# slow_holster_x=6\n"), 1);
    KT_CHECK_EQ(parse(";slow_holster_x=6\n"), 1);
    KT_CHECK_EQ(parse("slow_holster_xyz=6\n"), 1);                      // a longer key is not the key
    KT_CHECK_EQ(parse("slow_holster_x_extra=6\n"), 1);
    KT_CHECK_EQ(parse("slow_holster_x=2\nslow_holster_x=5\n"), 5);      // the last line that sets it wins
    KT_CHECK_EQ(parse("probe=1\n  slow_holster_x=7\nbone_test=1\n"), 7);

    // the key past 16 KB (the shipped holster.ini template is 21 KB; the stock declarative reader would cut at 16 KB) is still found
    static char big[24 * 1024];
    size_t n = 0;
    int k = 0;
    while (n < 20000) n += static_cast<size_t>(snprintf(big + n, sizeof big - n, "probe_line_%05d=0\n", k++));
    n += static_cast<size_t>(snprintf(big + n, sizeof big - n, "slow_holster_x=7\n"));
    KT_CHECK(n > 16 * 1024);
    KT_CHECK_EQ(speedFromIniBuf(big, n), 7);
}

static void testReadFile() {
    // a missing file is the default (the page shows 1x and writes nothing)
    int bytes = 0;
    KT_CHECK_EQ(speedReadIni("/nonexistent/holster.ini", &bytes), 1);
    KT_CHECK_EQ(bytes, -1);
    KT_CHECK_EQ(speedReadIni(nullptr), 1);

    // a real 21 KB file on disk whose key sits past the 16 KB cut: read back through the file path (the one the editor uses), not just the buffer parser
    const char* tmp = getenv("TMPDIR");
    const std::string pathStr = std::string(tmp && *tmp ? tmp : "/tmp") + "/hse_speed_test_holster.ini";
    const char* path = pathStr.c_str();
    FILE* f = fopen(path, "wb");
    KT_CHECK(f != nullptr);
    if (!f) return;
    size_t written = 0;
    int k = 0;
    char line[64];
    while (written < 21 * 1024) {
        const int m = snprintf(line, sizeof line, "probe_line_%05d=0\n", k++);
        fwrite(line, 1, static_cast<size_t>(m), f);
        written += static_cast<size_t>(m);
    }
    fputs("slow_holster_x=6\n", f);
    fclose(f);
    KT_CHECK_EQ(speedReadIni(path, &bytes), 6);
    KT_CHECK(bytes > 21 * 1024);
    // a later line for the key wins over the earlier one
    f = fopen(path, "ab");
    if (f) { fputs("slow_holster_x=3\n", f); fclose(f); }
    KT_CHECK_EQ(speedReadIni(path), 3);   // the last line wins
    remove(path);
}

static void testWriteRoundTrip() {
    // the value the page writes (iniw::updateFile -> setKey) reads back as the same speed; a second write replaces the line
    const char* in = "probe=0\n";
    char out[256] = {0};
    size_t len = 0;
    KT_CHECK(iniw::setKey(in, strlen(in), nullptr, kSpeedIniKey, "5", out, sizeof out, &len));
    KT_CHECK_EQ(speedFromIniBuf(out, len), 5);
    KT_CHECK(strstr(out, "slow_holster_x=5") != nullptr);
    char out2[256] = {0};
    size_t len2 = 0;
    KT_CHECK(iniw::setKey(out, len, nullptr, kSpeedIniKey, "8", out2, sizeof out2, &len2));
    KT_CHECK_EQ(speedFromIniBuf(out2, len2), 8);
    KT_CHECK(strstr(out2, "slow_holster_x=5") == nullptr);
}

int main() {
    testMapping();
    testThumbPlace();
    testText();
    testValueText();
    testIniBuf();
    testReadFile();
    testWriteRoundTrip();
    return kt::Report("hse_speed");
}
