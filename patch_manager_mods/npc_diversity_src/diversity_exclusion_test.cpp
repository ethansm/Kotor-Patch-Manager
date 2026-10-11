// diversity_exclusion_test.cpp -- unit tests for DiversityExclusion.h
#include "DiversityExclusion.h"
#include "check.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

using NpcDiversity::ExcludeKind;

ExcludeKind Kind(const char* tag) {
    return NpcDiversity::ClassifyExclusion(tag);
}

void testOriginalCompanionTags() {
    // Bare companion names, case-insensitive. Kind must be Companion.
    KT_CHECK(Kind("kreia") == ExcludeKind::Companion);
    KT_CHECK(Kind("KREIA") == ExcludeKind::Companion);
    KT_CHECK(Kind("Kreia") == ExcludeKind::Companion);
    KT_CHECK(Kind("atton") == ExcludeKind::Companion);
    KT_CHECK(Kind("ATTON") == ExcludeKind::Companion);
    KT_CHECK(Kind("bao_dur") == ExcludeKind::Companion);
    KT_CHECK(Kind("BAO_DUR") == ExcludeKind::Companion);
    KT_CHECK(Kind("mira") == ExcludeKind::Companion);
    KT_CHECK(Kind("MIRA") == ExcludeKind::Companion);
    KT_CHECK(Kind("hanharr") == ExcludeKind::Companion);
    KT_CHECK(Kind("HANHARR") == ExcludeKind::Companion);
    KT_CHECK(Kind("mandalore") == ExcludeKind::Companion);
    KT_CHECK(Kind("MANDALORE") == ExcludeKind::Companion);
    KT_CHECK(Kind("visas") == ExcludeKind::Companion);
    KT_CHECK(Kind("VISAS") == ExcludeKind::Companion);
    KT_CHECK(Kind("disciple") == ExcludeKind::Companion);
    KT_CHECK(Kind("DISCIPLE") == ExcludeKind::Companion);
    KT_CHECK(Kind("handmaiden") == ExcludeKind::Companion);
    KT_CHECK(Kind("HANDMAIDEN") == ExcludeKind::Companion);

    // Aliases and droids.
    KT_CHECK(Kind("baodur") == ExcludeKind::Companion);
    KT_CHECK(Kind("t3m4") == ExcludeKind::Companion);
    KT_CHECK(Kind("t3_m4") == ExcludeKind::Companion);
    KT_CHECK(Kind("hk47") == ExcludeKind::Companion);
    KT_CHECK(Kind("hk_47") == ExcludeKind::Companion);
    KT_CHECK(Kind("g0t0") == ExcludeKind::Companion);
    KT_CHECK(Kind("g0_t0") == ExcludeKind::Companion);
    KT_CHECK(Kind("player") == ExcludeKind::Companion);
}

void testOriginalStoryTags() {
    KT_CHECK(Kind("chodo") == ExcludeKind::Story);
    KT_CHECK(Kind("CHODO") == ExcludeKind::Story);
    KT_CHECK(Kind("luxa") == ExcludeKind::Story);
    KT_CHECK(Kind("LUXA") == ExcludeKind::Story);
    KT_CHECK(Kind("grenn") == ExcludeKind::Story);
    KT_CHECK(Kind("GRENN") == ExcludeKind::Story);
    KT_CHECK(Kind("opo_chano") == ExcludeKind::Story);
    KT_CHECK(Kind("OPO_CHANO") == ExcludeKind::Story);
    KT_CHECK(Kind("opochano") == ExcludeKind::Story);
    KT_CHECK(Kind("lorso") == ExcludeKind::Story);
    KT_CHECK(Kind("LORSO") == ExcludeKind::Story);
    KT_CHECK(Kind("b4d4") == ExcludeKind::Story);
    KT_CHECK(Kind("B4D4") == ExcludeKind::Story);
    KT_CHECK(Kind("ralon") == ExcludeKind::Story);
    KT_CHECK(Kind("RALON") == ExcludeKind::Story);
    KT_CHECK(Kind("moza") == ExcludeKind::Story);
    KT_CHECK(Kind("MOZA") == ExcludeKind::Story);
    KT_CHECK(Kind("vrook") == ExcludeKind::Story);
    KT_CHECK(Kind("VROOK") == ExcludeKind::Story);
    KT_CHECK(Kind("zreztik") == ExcludeKind::Story);
    KT_CHECK(Kind("ZREZTIK") == ExcludeKind::Story);
    KT_CHECK(Kind("geeda") == ExcludeKind::Story);
    KT_CHECK(Kind("GEEDA") == ExcludeKind::Story);
}

void testCreaturePrefixStripping() {
    KT_CHECK(Kind("n_chodo") == ExcludeKind::Story);
    KT_CHECK(Kind("n_luxa") == ExcludeKind::Story);
    KT_CHECK(Kind("c_grenn") == ExcludeKind::Story);
    KT_CHECK(Kind("npc_atton") == ExcludeKind::Companion);
    KT_CHECK(Kind("n_kreia") == ExcludeKind::Companion);
    KT_CHECK(Kind("N_Kreia") == ExcludeKind::Companion);
    KT_CHECK(Kind("npc_Atton") == ExcludeKind::Companion);
    KT_CHECK(Kind("C_Bao_Dur") == ExcludeKind::Companion);
    KT_CHECK(Kind("HK47") == ExcludeKind::Companion);
}

void testAreaPrefixedStoryTags() {
    // Area-prefixed tags were not excluded before. They must be excluded now.
    KT_CHECK(Kind("203_Lorso") == ExcludeKind::Story);
    KT_CHECK(Kind("203_Chano") == ExcludeKind::Story);
    KT_CHECK(Kind("202_Falt") == ExcludeKind::Story);
    KT_CHECK(Kind("207_Falt") == ExcludeKind::Story);
    KT_CHECK(Kind("201_b4d4") == ExcludeKind::Story);
    KT_CHECK(Kind("203_b4d4") == ExcludeKind::Story);
    KT_CHECK(Kind("201_chodo") == ExcludeKind::Story);
    KT_CHECK(Kind("203_batono") == ExcludeKind::Story);
    KT_CHECK(Kind("203_LORSO") == ExcludeKind::Story);
}

void testTokenBoundary() {
    // Digits and '_' end a name token.
    KT_CHECK(Kind("mira_2") == ExcludeKind::Companion);
    KT_CHECK(Kind("mira01") == ExcludeKind::Companion);
    KT_CHECK(Kind("atton2") == ExcludeKind::Companion);
    KT_CHECK(Kind("n_mira_2") == ExcludeKind::Companion);

    // Letters after a name do not match. This was the over-match bug.
    KT_CHECK(Kind("mirage_guard") == ExcludeKind::None);
    KT_CHECK(Kind("miranda") == ExcludeKind::None);
    KT_CHECK(Kind("kreiax") == ExcludeKind::None);
    KT_CHECK(Kind("chodos") == ExcludeKind::None);
    KT_CHECK(Kind("lorsoa") == ExcludeKind::None);
    KT_CHECK(Kind("opo") == ExcludeKind::None);
}

void testLegacyExactPrefixNames() {
    // Kept on purpose. These were excluded by the old prefix rule.
    KT_CHECK(Kind("miramess") == ExcludeKind::Companion);
    KT_CHECK(Kind("miramess1") == ExcludeKind::Companion);
    KT_CHECK(Kind("miraqstpris") == ExcludeKind::Companion);
    KT_CHECK(Kind("miraqstpris1") == ExcludeKind::Companion);
    KT_CHECK(Kind("miraqstpris2") == ExcludeKind::Companion);
    KT_CHECK(Kind("g0t0droid") == ExcludeKind::Companion);
    KT_CHECK(Kind("g0t0droid2") == ExcludeKind::Companion);
    KT_CHECK(Kind("n_miramess1") == ExcludeKind::Companion);
}

void testNotExcluded() {
    // Generic commoners and unrelated NPCs.
    KT_CHECK(Kind("citizen_m") == ExcludeKind::None);
    KT_CHECK(Kind("citizen_f") == ExcludeKind::None);
    KT_CHECK(Kind("citizen") == ExcludeKind::None);
    KT_CHECK(Kind("203_citizen") == ExcludeKind::None);
    KT_CHECK(Kind("commoner") == ExcludeKind::None);
    KT_CHECK(Kind("n_commoner") == ExcludeKind::None);
    KT_CHECK(Kind("c_bystander") == ExcludeKind::None);
    KT_CHECK(Kind("bystander") == ExcludeKind::None);
    KT_CHECK(Kind("civ") == ExcludeKind::None);
    KT_CHECK(Kind("merchant01") == ExcludeKind::None);
    KT_CHECK(Kind("random_npc") == ExcludeKind::None);
    KT_CHECK(Kind("test") == ExcludeKind::None);

    // Names that share a listed prefix but not a token boundary.
    KT_CHECK(Kind("mirage_guard") == ExcludeKind::None);
    KT_CHECK(Kind("miranda") == ExcludeKind::None);
    KT_CHECK(Kind("n_mirage") == ExcludeKind::None);
    KT_CHECK(Kind("203_miranda") == ExcludeKind::None);
}

void testNullAndEmptyAndBarePrefixes() {
    KT_CHECK(Kind(nullptr) == ExcludeKind::None);
    KT_CHECK(Kind("") == ExcludeKind::None);
    KT_CHECK(Kind("203_") == ExcludeKind::None);
    KT_CHECK(Kind("203") == ExcludeKind::None);
    KT_CHECK(Kind("n_") == ExcludeKind::None);
    KT_CHECK(Kind("c_") == ExcludeKind::None);
    KT_CHECK(Kind("npc_") == ExcludeKind::None);
}

void testLongTags() {
    // 31 chars: "mira_" then 26 'x'. Companion.
    const std::string thirtyOne = "mira_" + std::string(26, 'x');
    KT_CHECK(thirtyOne.size() == 31);
    KT_CHECK(Kind(thirtyOne.c_str()) == ExcludeKind::Companion);

    // 31 garbage chars.
    const std::string thirtyOneGarbage(31, 'z');
    KT_CHECK(Kind(thirtyOneGarbage.c_str()) == ExcludeKind::None);

    // 40 garbage chars.
    const std::string fortyGarbage(40, 'z');
    KT_CHECK(fortyGarbage.size() == 40);
    KT_CHECK(Kind(fortyGarbage.c_str()) == ExcludeKind::None);

    // 40 chars: the match is in the first 32, so the cap does not change it.
    const std::string fortyMira = "mira_" + std::string(35, 'x');
    KT_CHECK(fortyMira.size() == 40);
    KT_CHECK(Kind(fortyMira.c_str()) == ExcludeKind::Companion);

    const std::string fortyLorso = "203_Lorso_" + std::string(30, 'x');
    KT_CHECK(fortyLorso.size() == 40);
    KT_CHECK(Kind(fortyLorso.c_str()) == ExcludeKind::Story);
}

void testUnterminatedBuffers() {
    // Exactly 16 bytes, no NUL. Heap-allocated so ASan reports any read at [16].
    char* exact = new char[16];
    std::memcpy(exact, "MiraMiraMiraMira", 16);
    KT_CHECK(NpcDiversity::ClassifyExclusionN(exact, 16) == ExcludeKind::None);

    std::memcpy(exact, "203_Lorso_Zzzzzz", 16);
    KT_CHECK(NpcDiversity::ClassifyExclusionN(exact, 16) == ExcludeKind::Story);
    delete[] exact;

    // Sentinel check: 'x' at index maxLen must never be read.
    // With maxLen 4 the buffer is "mira" (Companion). With 5 it is "mirax" (None).
    char sentinel[] = "mirax";
    KT_CHECK(NpcDiversity::ClassifyExclusionN(sentinel, 4) == ExcludeKind::Companion);
    KT_CHECK(NpcDiversity::ClassifyExclusionN(sentinel, 5) == ExcludeKind::None);

    // 40-byte buffer with no NUL. Only 31 chars may be copied.
    char big[40];
    std::memcpy(big, "mira_", 5);
    std::memset(big + 5, 'x', 35);
    KT_CHECK(NpcDiversity::ClassifyExclusionN(big, 40) == ExcludeKind::Companion);
    std::memset(big, 'z', 40);
    KT_CHECK(NpcDiversity::ClassifyExclusionN(big, 40) == ExcludeKind::None);

    // Degenerate arguments.
    KT_CHECK(NpcDiversity::ClassifyExclusionN(nullptr, 16) == ExcludeKind::None);
    KT_CHECK(NpcDiversity::ClassifyExclusionN("kreia", 0) == ExcludeKind::None);
    KT_CHECK(NpcDiversity::ClassifyExclusionN("kreia", 4) == ExcludeKind::None);
    KT_CHECK(NpcDiversity::ClassifyExclusionN("kreia", 100) == ExcludeKind::Companion);
}

void testCaseInsensitivity() {
    KT_CHECK(Kind("KrEiA") == ExcludeKind::Companion);
    KT_CHECK(Kind("NPC_ATTON") == ExcludeKind::Companion);
    KT_CHECK(Kind("Npc_Atton") == ExcludeKind::Companion);
    KT_CHECK(Kind("C_BAO_DUR") == ExcludeKind::Companion);
    KT_CHECK(Kind("201_B4D4") == ExcludeKind::Story);
    KT_CHECK(Kind("MIRAGE_GUARD") == ExcludeKind::None);
}

void testStartsWithCaseInsensitive() {
    KT_CHECK(NpcDiversity::StartsWithCaseInsensitive("NPC_x", "npc_"));
    KT_CHECK(!NpcDiversity::StartsWithCaseInsensitive("np", "npc_"));
    KT_CHECK(!NpcDiversity::StartsWithCaseInsensitive(nullptr, "npc_"));
    KT_CHECK(!NpcDiversity::StartsWithCaseInsensitive("npc_", nullptr));
}

void testCompatibilityWrapper() {
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification(nullptr));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification(""));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("203_Lorso"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("kreia"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("mirage_guard"));

    const char* const tags[] = {
        "kreia", "203_Lorso", "mirage_guard", "citizen_m", "n_chodo", "203_",
    };
    for (const char* t : tags) {
        const bool excluded = NpcDiversity::IsExcludedFromDiversification(t);
        KT_CHECK(excluded == (Kind(t) != ExcludeKind::None));
    }
}

// Every listed name, in its four forms: bare, "n_", "npc_" and "203_".
struct NameCase {
    const char* name;
    ExcludeKind kind;
};

void testEveryNameAllForms() {
    const NameCase names[] = {
        {"kreia", ExcludeKind::Companion},
        {"atton", ExcludeKind::Companion},
        {"bao_dur", ExcludeKind::Companion},
        {"baodur", ExcludeKind::Companion},
        {"mira", ExcludeKind::Companion},
        {"hanharr", ExcludeKind::Companion},
        {"mandalore", ExcludeKind::Companion},
        {"visas", ExcludeKind::Companion},
        {"disciple", ExcludeKind::Companion},
        {"handmaiden", ExcludeKind::Companion},
        {"t3m4", ExcludeKind::Companion},
        {"t3_m4", ExcludeKind::Companion},
        {"hk47", ExcludeKind::Companion},
        {"hk_47", ExcludeKind::Companion},
        {"g0t0", ExcludeKind::Companion},
        {"g0_t0", ExcludeKind::Companion},
        {"player", ExcludeKind::Companion},
        {"chodo", ExcludeKind::Story},
        {"luxa", ExcludeKind::Story},
        {"grenn", ExcludeKind::Story},
        {"opo_chano", ExcludeKind::Story},
        {"opochano", ExcludeKind::Story},
        {"chano", ExcludeKind::Story},
        {"lorso", ExcludeKind::Story},
        {"b4d4", ExcludeKind::Story},
        {"ralon", ExcludeKind::Story},
        {"moza", ExcludeKind::Story},
        {"vrook", ExcludeKind::Story},
        {"zreztik", ExcludeKind::Story},
        {"geeda", ExcludeKind::Story},
        {"falt", ExcludeKind::Story},
        {"batono", ExcludeKind::Story},
    };
    const char* const prefixes[] = {"", "n_", "npc_", "203_"};

    for (const NameCase& nc : names) {
        for (const char* prefix : prefixes) {
            char tag[64];
            std::snprintf(tag, sizeof(tag), "%s%s", prefix, nc.name);
            char msg[96];
            std::snprintf(msg, sizeof(msg), "every-name: %s", tag);
            ::kt::Check(msg, Kind(tag) == nc.kind);
        }
    }
}

} // namespace

int main() {
    testOriginalCompanionTags();
    testOriginalStoryTags();
    testCreaturePrefixStripping();
    testAreaPrefixedStoryTags();
    testTokenBoundary();
    testLegacyExactPrefixNames();
    testNotExcluded();
    testNullAndEmptyAndBarePrefixes();
    testLongTags();
    testUnterminatedBuffers();
    testCaseInsensitivity();
    testStartsWithCaseInsensitive();
    testCompatibilityWrapper();
    testEveryNameAllForms();
    return kt::Report("diversity_exclusion_test");
}
