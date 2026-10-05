// Host test of TextFit.h textfit:: (the pixel-width text layer moved verbatim out of ChareffDecode.h, C4c): pins the font width
// tables, textPx, fitText, marqueeText and wrapAppend/wrapAppendW. Expected values are copied from the Python-oracle cases in
// active_effects_panel_src/testdata/decode_cases.txt (ctx sy 2.66669989, textK10 1.02999997, textScale 1), so this stays
// green on its own if chareff_decode_test is ever retired; chareff_decode_test also checks cfx:: re-exports the same functions.
#include "TextFit.h"
#include "check.h"
#include <string.h>
#include <string>

using namespace textfit;

int main() {
    // 1. Font width tables (texels): spot values + the line pitch constant.
    KT_CHECK_EQ((int)kFontTexW['A'], 14);
    KT_CHECK_EQ((int)kFontTexW[' '], 12);
    KT_CHECK_EQ((int)kFontTexW['i'], 3);
    KT_CHECK_EQ((int)kFont10TexW['W'], 9);
    KT_CHECK_EQ((int)kFont10TexW['i'], 3);
    KT_CHECK_EQ((int)kFont10TexW[0], 7);
    KT_CHECK(kFont10LineH == 12.50f);
    KT_CHECK(TextK == 0.75f);

    const TextCtx ctx = {2.66669989f, 1.02999997f, 1.0f};

    // 2. textPx (exact float equality, same as the oracle cases).
    KT_CHECK(textPx(ctx, "Burst of Speed", 14, F16) == 292.003662f);
    KT_CHECK(textPx(ctx, "Droid Deflector Mark I", 22, F16) == 442.005524f);
    KT_CHECK(textPx(ctx, "Speed +199%", 11, F10) == 241.709671f);
    KT_CHECK(textPx(ctx, "", 0, F10) == 0.0f);

    char out[128];
    // 3. fitText: fits exactly / one px short (cut) / F10 cut.
    KT_CHECK(!fitText(ctx, "Droid Deflector Mark I", 442.005524f, out, sizeof out, F16));
    KT_CHECK_EQ((const char*)out, "Droid Deflector Mark I");
    KT_CHECK(fitText(ctx, "Droid Deflector Mark I", 441.005524f, out, sizeof out, F16));
    KT_CHECK_EQ((const char*)out, "Droid Deflector Mark...");
    KT_CHECK(fitText(ctx, "Droid Deflector Mark I", 265.203308f, out, sizeof out, F16));
    KT_CHECK_EQ((const char*)out, "Droid Deflec...");
    KT_CHECK(fitText(ctx, "Droid Deflector Mark I", 223.856125f, out, sizeof out, F10));
    KT_CHECK_EQ((const char*)out, "Droid Def...");

    // 4. marqueeText: whole text fits (maxOff 0), offsets 0 / 3 / past the end, F10 offset landing on a space.
    int mo = -1;
    marqueeText(ctx, "Droid Deflector Mark I", 443.005524f, 0, out, sizeof out, &mo, F16);
    KT_CHECK_EQ((const char*)out, "Droid Deflector Mark I"); KT_CHECK_EQ(mo, 0);
    marqueeText(ctx, "Droid Deflector Mark I", 265.203308f, 0, out, sizeof out, &mo, F16);
    KT_CHECK_EQ((const char*)out, "Droid Deflec..."); KT_CHECK_EQ(mo, 9);
    marqueeText(ctx, "Droid Deflector Mark I", 265.203308f, 3, out, sizeof out, &mo, F16);
    KT_CHECK_EQ((const char*)out, "id Deflector..."); KT_CHECK_EQ(mo, 9);
    marqueeText(ctx, "Droid Deflector Mark I", 265.203308f, 100, out, sizeof out, &mo, F16);
    KT_CHECK_EQ((const char*)out, "lector Mark I"); KT_CHECK_EQ(mo, 9);
    marqueeText(ctx, "Droid Deflector Mark I", 223.856125f, 7, out, sizeof out, &mo, F10);
    KT_CHECK_EQ((const char*)out, "eflector..."); KT_CHECK_EQ(mo, 12);

    // 5. wrapAppend (px10) and wrapAppendW (plain character count measure).
    {
        char buf[1025]; int len = 0;
        int n = wrapAppend(ctx, "WWW WWW iiiiiiii iiiiiiii", 164.802048f, buf, &len, 1024);
        KT_CHECK_EQ(n, 3);
        KT_CHECK_EQ((const char*)buf, "WWW\nWWW iiiiiiii\niiiiiiii");
        KT_CHECK_EQ(len, 25);
    }
    {
        char buf[1025]; int len = 0;
        auto chars = [](const char*, int n) { return (float)n; };
        KT_CHECK_EQ(wrapAppendW(chars, "aaa bbb ccc ddd", 7, buf, &len, 1024), 2);
        KT_CHECK_EQ((const char*)buf, "aaa bbb\nccc ddd");
        KT_CHECK_EQ(len, 15);
        // second call appends after a newline into the same buffer
        KT_CHECK_EQ(wrapAppendW(chars, "ee", 7, buf, &len, 1024), 1);
        KT_CHECK_EQ((const char*)buf, "aaa bbb\nccc ddd\nee");
        // cap overflow: the line is dropped, no clipped tail
        char tiny[9]; int tl = 0;
        KT_CHECK_EQ(wrapAppendW(chars, "aaaa bbbb", 4, tiny, &tl, 8), 1);
        KT_CHECK_EQ((const char*)tiny, "aaaa");
        KT_CHECK_EQ(tl, 4);
    }

    return kt::Report("textfit_test");
}
