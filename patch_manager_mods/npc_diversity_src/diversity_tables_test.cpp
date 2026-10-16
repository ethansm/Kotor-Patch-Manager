// diversity_tables_test.cpp -- unit tests for DiversityTables.h
#include "DiversityTables.h"
#include "DiversityRows.h"
#include "check.h"

#include <cstddef>

namespace {

// Table index: 0 = male commoner, 1 = female commoner, 2 = Twi'lek male, 3 = Twi'lek female.
bool inTable(int table, uint16_t app) {
    switch (table) {
        case 0: return NpcDiversity::IsGenericMaleCommoner(app);
        case 1: return NpcDiversity::IsGenericFemaleCommoner(app);
        case 2: return NpcDiversity::IsGenericTwilekMale(app);
        default: return NpcDiversity::IsGenericTwilekFemale(app);
    }
}

// Every row in `rows` must match `table` and no other table.
template <std::size_t N>
void expectOnlyIn(int table, const uint16_t (&rows)[N]) {
    for (uint16_t app : rows) {
        for (int t = 0; t < 4; ++t) {
            KT_CHECK(inTable(t, app) == (t == table));
        }
    }
}

void testGenericAppearanceChecks() {
    // Rows that are not generic civilians (Sith, Jedi, Mandalorians, droids, rancors, Krayt, story NPCs)
    // must match no table.
    const uint16_t nonGeneric[] = {300, 310, 350, 360, 500, 510, 550, 560, 554, 556, 558, 507, 508, 351, 309, 353};
    for (uint16_t app : nonGeneric) {
        KT_CHECK(!NpcDiversity::IsGenericMaleCommoner(app));
        KT_CHECK(!NpcDiversity::IsGenericFemaleCommoner(app));
        KT_CHECK(!NpcDiversity::IsGenericTwilekMale(app));
        KT_CHECK(!NpcDiversity::IsGenericTwilekFemale(app));
    }

    // Custom rows 802..APP_COUNT-1 (APP_* constants) are not diversification sources and must match no table.
    for (uint16_t app = NpcDiversity::APP_FIRST_APPENDED; app < NpcDiversity::APP_COUNT; ++app) {
        KT_CHECK(!NpcDiversity::IsGenericMaleCommoner(app));
        KT_CHECK(!NpcDiversity::IsGenericFemaleCommoner(app));
        KT_CHECK(!NpcDiversity::IsGenericTwilekMale(app));
        KT_CHECK(!NpcDiversity::IsGenericTwilekFemale(app));
    }

    // Boundaries.
    KT_CHECK(!NpcDiversity::IsGenericMaleCommoner(0));
    KT_CHECK(!NpcDiversity::IsGenericFemaleCommoner(0));
    KT_CHECK(!NpcDiversity::IsGenericTwilekMale(0));
    KT_CHECK(!NpcDiversity::IsGenericTwilekFemale(0));
    KT_CHECK(!NpcDiversity::IsGenericMaleCommoner(65535));
    KT_CHECK(!NpcDiversity::IsGenericFemaleCommoner(65535));
    KT_CHECK(!NpcDiversity::IsGenericTwilekMale(65535));
    KT_CHECK(!NpcDiversity::IsGenericTwilekFemale(65535));

    // Vanilla rows are true in their own table only.
    const uint16_t maleRows[] = {17, 190, 591, 615};        // Commoner_Mal_*
    const uint16_t femaleRows[] = {14, 185, 585, 609};      // Commoner_Fem_*
    const uint16_t twiMaleRows[] = {416, 606, 618};         // Twilek_Commoner_M_*
    const uint16_t twiFemaleRows[] = {280, 446, 620};       // Twilek_Commoner_F_*
    expectOnlyIn(0, maleRows);
    expectOnlyIn(1, femaleRows);
    expectOnlyIn(2, twiMaleRows);
    expectOnlyIn(3, twiFemaleRows);

    // Widened to every row: no row matches two generic tables, and each table matches some row.
    long perTable[4] = {};
    for (uint16_t app = 0; app < NpcDiversity::APP_COUNT; ++app) {
        int hits = 0;
        for (int t = 0; t < 4; ++t) {
            if (inTable(t, app)) {
                ++hits;
                perTable[t]++;
            }
        }
        KT_CHECK(hits <= 1);
    }
    for (int t = 0; t < 4; ++t) {
        KT_CHECK(perTable[t] > 0);
    }
}

} // namespace

int main() {
    testGenericAppearanceChecks();
    return kt::Report("diversity_tables_test");
}
