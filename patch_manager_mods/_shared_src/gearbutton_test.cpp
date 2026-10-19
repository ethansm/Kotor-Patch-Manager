// Host test of GearButton.h (MCF8): geometry pinned to level_up_override's LevelUpUi.h numbers (side 28 units, inset 0.18), art names, and
// registration through EmbedTex with a fake engine. Expected values are independent literals.
#ifndef EMBTEX_HOST
#define EMBTEX_HOST 1
#endif
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "GearButton.h"
#include "check.h"
#include <stdlib.h>
#include <string.h>

static int nNew = 0, nCtor = 0, nAdd = 0; static char g_ctorName[8][32];
static void* g_alloc[32]; static int g_nAlloc = 0;
static void* track(void* p) { if (g_nAlloc < 32) g_alloc[g_nAlloc++] = p; return p; }
static void* fNew(unsigned n) { ++nNew; return track(calloc(1, n)); }
static void* fNewArr(unsigned n) { return track(malloc(n)); }
static void* fCtor(void* t, const char* a, const char*) { if (nCtor < 8) strncpy(g_ctorName[nCtor], a, 31); ++nCtor; return t; }
static void fAdd(void*, void*) { ++nAdd; }
static void fReset(void*) {}

int main() {
    // ---- geometry: LevelUpUi.h kIconSide 28 / kGearInset 0.18 ----
    KT_CHECK_EQ(gearbtn::sidePx(28, 1.0f), 28); KT_CHECK_EQ(gearbtn::sidePx(28, 1.5f), 42); KT_CHECK_EQ(gearbtn::sidePx(28, 0.0f), 0);
    gearbtn::Rect ic = gearbtn::iconRect(520, 172, 28);
    KT_CHECK_EQ(ic.x, 492); KT_CHECK_EQ(ic.y, 172); KT_CHECK_EQ(ic.w, 28); KT_CHECK_EQ(ic.h, 28);
    gearbtn::Rect g = gearbtn::gearRect(ic);   // round(28 * 0.18) = round(5.04) = 5
    KT_CHECK_EQ(g.x, 497); KT_CHECK_EQ(g.y, 177); KT_CHECK_EQ(g.w, 18); KT_CHECK_EQ(g.h, 18);
    gearbtn::Rect ic2 = gearbtn::iconRect(1000, 10, 42);   // 42 * 0.18 = 7.56 -> 8
    gearbtn::Rect g2 = gearbtn::gearRect(ic2);
    KT_CHECK_EQ(ic2.x, 958); KT_CHECK_EQ(g2.x, 966); KT_CHECK_EQ(g2.y, 18); KT_CHECK_EQ(g2.w, 26); KT_CHECK_EQ(g2.h, 26);
    KT_CHECK_EQ(gearbtn::gearRect(gearbtn::iconRect(10, 0, 0)).w, 0);   // degenerate side stays 0
    KT_CHECK(gearbtn::kHiTint[0] > 0.79f && gearbtn::kHiTint[0] < 0.81f && gearbtn::kHiTint[2] > 0.69f && gearbtn::kHiTint[2] < 0.70f);
    KT_CHECK(gearbtn::hoverFill(true) && !gearbtn::hoverFill(false));

    // ---- art names + table ----
    KT_CHECK(!strcmp(gearbtn::artName(false), "e3d_gear")); KT_CHECK(!strcmp(gearbtn::artName(true), "e3d_gear_hi"));
    KT_CHECK(!strcmp(gearbtn::fillName(), "e3d_fill"));
    KT_CHECK_EQ(gearbtn::textureCount(), 5);
    for (int i = 0; i < 3; ++i) KT_CHECK(strlen(gearbtn::textures()[i].resref) <= 16);
    KT_CHECK_EQ(gearbtn::textures()[0].size, 16428u); KT_CHECK_EQ(gearbtn::textures()[1].size, 16428u); KT_CHECK_EQ(gearbtn::textures()[2].size, 300u);
    embtex::TgaInfo ti;
    KT_CHECK_EQ((int)embtex::tgaInspect(gearbtn::textures()[0].data, gearbtn::textures()[0].size, &ti), (int)embtex::TGA_OK);
    KT_CHECK_EQ(ti.w, 64); KT_CHECK_EQ(ti.h, 64);
    KT_CHECK(memcmp(gearbtn::textures()[0].data, gearbtn::textures()[1].data, 16428) != 0);   // hover glyph differs

    // ---- registration with a fake engine ----
    static unsigned char pending[16] = {0};
    embtex::g_et.newFn = fNew; embtex::g_et.newArrFn = fNewArr; embtex::g_et.ctorFn = fCtor; embtex::g_et.listAddFn = fAdd; embtex::g_et.resetFn = fReset;
    embtex::g_et.pendingList = reinterpret_cast<uintptr_t>(pending);
    KT_CHECK(gearbtn::registerArt());
    KT_CHECK_EQ(nNew, 5); KT_CHECK_EQ(nCtor, 5); KT_CHECK_EQ(nAdd, 5);
    KT_CHECK(!strcmp(g_ctorName[0], "e3d_gear")); KT_CHECK(!strcmp(g_ctorName[1], "e3d_gear_hi")); KT_CHECK(!strcmp(g_ctorName[2], "e3d_fill"));
    KT_CHECK(gearbtn::registerArt()); KT_CHECK_EQ(nNew, 5);   // idempotent

    for (int i = 0; i < g_nAlloc; ++i) free(g_alloc[i]);
    return kt::Report("gearbutton_test");
}
