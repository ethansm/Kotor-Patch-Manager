// DiversityTables.h -- appearance-row tables of generic (diversifiable) NPCs
//
// Each table lists only vanilla KOTOR II civilian rows of override/appearance.2da.
// The row lists are verified against override/appearance.2da by
// scripts/check_diversity_assets.py: run it after editing any case label here.
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace NpcDiversity {

// Generic civilian appearance checks (matching generic civilian appearance rows)
inline bool IsGenericMaleCommoner(uint16_t app) {
    switch (app) {
        // Vanilla appearance.2da generic male commoners
        case 17:
        case 190: case 191: case 192: case 193: case 194:
        case 345: case 346: case 347: case 348: case 349:
        case 423: case 424: case 428: case 430:
        case 591: case 592: case 593: case 594: case 595: case 596:
        case 615: case 616: case 617:
            return true;
        default:
            return false;
    }
}

inline bool IsGenericFemaleCommoner(uint16_t app) {
    switch (app) {
        // Vanilla appearance.2da generic female commoners
        case 14:
        case 185: case 186: case 187: case 188: case 189:
        case 339: case 340: case 341: case 342: case 343: case 344:
        case 429: case 431:
        case 585: case 586: case 587: case 588: case 589: case 590:
        case 609: case 610: case 611: case 612: case 613: case 614:
            return true;
        default:
            return false;
    }
}

inline bool IsGenericTwilekMale(uint16_t app) {
    switch (app) {
        // Vanilla appearance.2da generic male Twi'leks
        case 416: case 417: case 445: case 449: case 450:
        case 606: case 618: case 619:
            return true;
        default:
            return false;
    }
}

inline bool IsGenericTwilekFemale(uint16_t app) {
    switch (app) {
        // Vanilla appearance.2da generic female Twi'leks
        case 280: case 446: case 447: case 448:
        case 620: case 621: case 622:
            return true;
        default:
            return false;
    }
}

} // namespace NpcDiversity
