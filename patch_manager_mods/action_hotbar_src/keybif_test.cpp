// keybif_test: hb::ReadKeyBifResource against testdata/keybif_fixture (cwd = this dir). Real-data step only when
// env KOTOR2_GAME_DIR is set: reads <dir>/chitin.key and expects spells.2da with > 100 rows; otherwise SKIP (pass).
#include "KeyBif.h"
#include "TwoDA.h"
#include "check.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

static const char* const FX = "testdata/keybif_fixture";

static std::string ReadFile(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// Expects failure with `err` containing `needle`.
static void ExpectFail(const char* what, const std::string& key, const std::string& dir, const char* resref, const char* needle) {
    std::string out = "stale", err;
    bool ok = hb::ReadKeyBifResource(key.c_str(), dir.c_str(), resref, hb::ResType2DA, out, err);
    printf("%s -> ok=%d err='%s'\n", what, (int)ok, err.c_str());
    kt::Check(what, !ok && out.empty() && err.find(needle) != std::string::npos, err.c_str());
}

int main() {
    const std::string fx = FX;
    std::string out, err;

    // Happy path: exact bytes of the source 2DA, then parse it.
    bool ok = hb::ReadKeyBifResource((fx + "/chitin.key").c_str(), FX, "spells", hb::ResType2DA, out, err);
    KT_CHECK(ok);
    if (ok) {
        KT_CHECK(out == ReadFile("testdata/spells_fixture.2da"));
        hb::TwoDA t;
        KT_CHECK(t.Parse(out));
        KT_CHECK_EQ(t.rows.size(), 43);
        KT_CHECK_EQ(t.Get(8, "label").c_str(), "FORCE_POWER_SPEED_BURST");
        KT_CHECK_EQ(t.Get(10, "iconresref").c_str(), "ip_cure");
    } else printf("err=%s\n", err.c_str());

    // ResRef is case-insensitive; other type with the same name and the decoy resource are distinguished.
    KT_CHECK(hb::ReadKeyBifResource((fx + "/chitin.key").c_str(), FX, "SPELLS", hb::ResType2DA, out, err) && out.size() == 1387);
    KT_CHECK(hb::ReadKeyBifResource((fx + "/chitin.key").c_str(), FX, "spells", 2002, out, err) && out == "wrong-type-decoy");
    KT_CHECK(hb::ReadKeyBifResource((fx + "/chitin.key").c_str(), FX, "decoy", hb::ResType2DA, out, err) && out == "decoy-2da-bytes");
    // Mixed-case BIF name in the KEY ("Data\2DA.BIF"): lowercase fallback opens data/2da.bif.
    KT_CHECK(hb::ReadKeyBifResource((fx + "/case.key").c_str(), FX, "spells", hb::ResType2DA, out, err) && out.size() == 1387);
    // Trailing slash on gameDir is fine.
    KT_CHECK(hb::ReadKeyBifResource((fx + "/chitin.key").c_str(), (fx + "/").c_str(), "spells", hb::ResType2DA, out, err));

    // Errors: never crash, short message, `out` left empty.
    ExpectFail("missing key file", fx + "/nope.key", FX, "spells", "cannot open key");
    ExpectFail("wrong key magic", fx + "/bad/bad_magic.key", FX, "spells", "bad key magic");
    ExpectFail("truncated key tables", fx + "/bad/trunc.key", FX, "spells", "out of range");
    ExpectFail("bif index out of range", fx + "/bad/bad_bif_index.key", FX, "spells", "bif index 3 out of range");
    ExpectFail("resref not found", fx + "/chitin.key", FX, "nosuchres", "resref not found");
    ExpectFail("empty resref", fx + "/chitin.key", FX, "", "bad arguments");
    const std::string bad = fx + "/bad";
    ExpectFail("oversize resource", bad + "/bad_res.key", bad, "big", "resource too large");
    ExpectFail("resource past EOF", bad + "/bad_res.key", bad, "past", "resource out of range");
    ExpectFail("bad bif magic", bad + "/bad_res.key", bad, "badmagic", "bad bif magic");
    ExpectFail("missing bif", bad + "/bad_res.key", bad, "ghost", "cannot open bif");

    // Real-data mode.
    const char* dir = getenv("KOTOR2_GAME_DIR");
    if (!dir || !*dir) {
        printf("SKIP real-data: KOTOR2_GAME_DIR not set\n");
    } else {
        const std::string d = dir;
        ok = hb::ReadKeyBifResource((d + "/chitin.key").c_str(), d.c_str(), "spells", hb::ResType2DA, out, err);
        KT_CHECK(ok);
        if (ok) {
            hb::TwoDA t;
            KT_CHECK(t.Parse(out));
            printf("real spells.2da: bytes=%d rows=%d cols=%d\n", (int)out.size(), (int)t.rows.size(), (int)t.cols.size());
            KT_CHECK(t.rows.size() > 100);
            KT_CHECK(t.Col("iconresref") > 0);
        } else printf("real-data err=%s\n", err.c_str());
    }
    return kt::Report("keybif_test");
}
