// WorkbenchUpgradeLogic.h -- pure helpers and state transitions for Workbench Upgrade (doc 55). No engine access; host-testable.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../Common/ItemCompare.h"

namespace wbu {

// State machine for workbench screen navigation (doc 55 P2/P3/P4).
enum class ScreenState : uint8_t {
    CLOSED = 0,
    HUB = 1,
    CREATE = 2,
    UPGRADE_LIST = 3,
    UPGRADE_SLOTS = 4,
    UPGRADE_SELECT = 3,
    UPGRADE_SLOT = 4
};

enum class SwitchAction : uint8_t {
    NONE = 0,
    OPEN_UPGRADE = 1,
    OPEN_CREATE = 2
};

// P1b / P3 / P4 static anchors and offsets on CSWSItem and property structs.
constexpr int kMaxUpgradeSlots = 6;
constexpr int kItemUpgradesMaskOff = 0x2d4;  // uint32_t GFF "Upgrades" bitmask on CSWSItem
constexpr int kItemUpgradeSlot0Off = 0x2f4;  // 6 dwords slot 0..5 template indices on CSWSItem, 0xffffffff when empty
constexpr int kPropUpgradeSlotOff = 0x12;   // uint16_t slot + 1 in 28-byte property struct at item+0x294, 0 = intrinsic

inline bool isUpgradeCompatible(uint32_t weaponUpgradesMask, uint8_t upgradeSlotBit) {
    return weaponUpgradesMask != 0 && upgradeSlotBit < 32 && ((weaponUpgradesMask & (1u << upgradeSlotBit)) != 0);
}

inline bool isUpgradeSlotOccupied(uint32_t slotTemplateIdx) {
    return slotTemplateIdx != 0xffffffffu;
}

inline int propOriginSlot(uint16_t propSlotField) {
    return propSlotField == 0 ? -1 : static_cast<int>(propSlotField) - 1;
}

// snprintf returns the would-be length; clamp so out+len / n-len stay inside the buffer (n >= 1).
inline int clampLen(int len, size_t n) {
    if (len < 0) return 0;
    if (static_cast<size_t>(len) >= n) return static_cast<int>(n) - 1;
    return len;
}

inline void fmtUpgradeSlots(char* out, size_t n, const uint32_t slots[kMaxUpgradeSlots], const bool ok[kMaxUpgradeSlots]) {
    if (n == 0 || out == nullptr) return;
    if (slots == nullptr) {
        snprintf(out, n, "slots=unreadable");
        return;
    }
    int len = clampLen(snprintf(out, n, "slots=["), n);
    for (int i = 0; i < kMaxUpgradeSlots && len < static_cast<int>(n) - 8; ++i) {
        const bool isOk = (ok != nullptr) ? ok[i] : true;
        if (!isOk) {
            len = clampLen(len + snprintf(out + len, n - len, "%s??", i ? " " : ""), n);
        } else if (slots[i] == 0xffffffffu) {
            len = clampLen(len + snprintf(out + len, n - len, "%s-", i ? " " : ""), n);
        } else {
            len = clampLen(len + snprintf(out + len, n - len, "%s%x", i ? " " : "", static_cast<unsigned>(slots[i])), n);
        }
    }
    snprintf(out + len, n - len, "]");
}

// "usable on equipped" (doc 55 P4)
struct EquippedItem {
    uint32_t objectId;
    uint32_t baseType;
    uint32_t upgradesMask;
    uint32_t installedSlots[kMaxUpgradeSlots];
    bool isEquipped;
    char name[64];
};

inline int filterCompatibleEquipped(const EquippedItem* items, int count, uint8_t upgradeSlotBit, int* outIndices, int maxOut) {
    if (items == nullptr || outIndices == nullptr || count <= 0 || maxOut <= 0) {
        return 0;
    }
    int matched = 0;
    for (int i = 0; i < count && matched < maxOut; ++i) {
        if (items[i].isEquipped && isUpgradeCompatible(items[i].upgradesMask, upgradeSlotBit)) {
            outIndices[matched++] = i;
        }
    }
    return matched;
}

// "already in assembly" (doc 55 P4)
struct AssemblySlotStatus {
    bool occupied;
    uint32_t templateIdx;
    int propCount;
};

inline bool isSlotEmptyInAssembly(const uint32_t slots[kMaxUpgradeSlots], int slotIdx) {
    if (slots == nullptr || slotIdx < 0 || slotIdx >= kMaxUpgradeSlots) {
        return true;
    }
    return !isUpgradeSlotOccupied(slots[slotIdx]);
}

inline void inspectAssemblySlots(const uint32_t slots[kMaxUpgradeSlots], const uint16_t* propOrigins, int nProps, AssemblySlotStatus out[kMaxUpgradeSlots]) {
    if (out == nullptr) return;
    for (int i = 0; i < kMaxUpgradeSlots; ++i) {
        out[i].occupied = false;
        out[i].templateIdx = 0xffffffffu;
        out[i].propCount = 0;
    }
    if (slots != nullptr) {
        for (int i = 0; i < kMaxUpgradeSlots; ++i) {
            out[i].templateIdx = slots[i];
            out[i].occupied = isUpgradeSlotOccupied(slots[i]);
        }
    }
    if (propOrigins != nullptr && nProps > 0) {
        for (int p = 0; p < nProps; ++p) {
            int slot = propOriginSlot(propOrigins[p]);
            if (slot >= 0 && slot < kMaxUpgradeSlots) {
                out[slot].propCount++;
            }
        }
    }
}

// "skill met" (doc 55 P4)
struct CraftingRequirement {
    int skillId;
    int requiredRank;
    int componentCost;
};

inline bool isSkillMet(int playerSkillRank, int requiredRank) {
    return requiredRank <= 0 || playerSkillRank >= requiredRank;
}

inline bool canCraftComponent(int playerSkillRank, int playerComponents, const CraftingRequirement& req) {
    return isSkillMet(playerSkillRank, req.requiredRank) && playerComponents >= req.componentCost && req.componentCost >= 0;
}

// Crafting compare preview & toggle (doc 55 P4)
inline bool isCraftingCompareActive(int configVal, ScreenState state) {
    return configVal != 0 && (state == ScreenState::CREATE || state == ScreenState::UPGRADE_SELECT || state == ScreenState::UPGRADE_SLOT);
}

// Compare row kinds matching inventory_sort_filter_src and vanilla UI palette
enum class RowKind : uint8_t {
    PLAIN = 0,
    HEADER = 1,
    UP = 2,
    DOWN = 3,
    DIM = 4
};

constexpr RowKind RK_Plain = RowKind::PLAIN;
constexpr RowKind RK_Header = RowKind::HEADER;
constexpr RowKind RK_Up = RowKind::UP;
constexpr RowKind RK_Down = RowKind::DOWN;
constexpr RowKind RK_Dim = RowKind::DIM;

struct RgbColor {
    float r;
    float g;
    float b;
};

// Vanilla upgrade/workbench palette (upgrade_p.gui, upgradesel_p.gui TEXT/COLOR): cyan-blue.
constexpr RgbColor kVanillaCyan = {0.0f, 0.6588f, 0.9804f};

// Colors:
// - RK_Header: cream {0.80f, 0.80f, 0.698f} (vanilla title)
// - RK_Up: soft green {0.40f, 0.90f, 0.40f} (semantic, kept)
// - RK_Down: soft coral {0.98f, 0.38f, 0.31f} (semantic, kept)
// - RK_Dim: vanilla cyan {0.0f, 0.6588f, 0.9804f}
// - Name column: vanilla cyan
// - Plain value: white {1.0f, 1.0f, 1.0f} (engine default)
inline RgbColor rowColor(RowKind kind, bool valueCol = true) {
    switch (kind) {
        case RowKind::UP:     return {0.40f, 0.90f, 0.40f};
        case RowKind::DOWN:   return {0.98f, 0.38f, 0.31f};
        case RowKind::DIM:    return kVanillaCyan;
        case RowKind::HEADER: return {0.80f, 0.80f, 0.698f};
        case RowKind::PLAIN:
        default:
            return valueCol ? RgbColor{1.0f, 1.0f, 1.0f} : RgbColor{0.80f, 0.80f, 0.698f};
    }
}

constexpr RgbColor kNameCyan = kVanillaCyan;

constexpr int kMaxCompareRows = 6;

struct CompareRow {
    char leftCol[64];   // Name / Stat (40% width in UI) or full label if not split
    char rightCol[64];  // Delta / Value (60% width in UI)
    char fullLine[128]; // Full line for single-label display / logging
    RowKind kind;
    bool hasSplit;
    bool unmet;         // requirement row where the player's skill rank is below the required rank
};

struct CraftingCompareOut {
    char header[64];
    char deltaLine[64];
    bool hasDelta;
    bool skillMet;
    int nRows;
    CompareRow rows[kMaxCompareRows];
};

// Player effective skill ranks in skills.2da row order (0..7). readable=false when the player's stats could not be read.
struct SkillView {
    bool readable;
    int ranks[8];
};

// Clamp a raw skill rank byte read from engine CSWSCreatureStats::GetSkillRank (0..127 per engine procedure).
inline int sanitizeSkillRank(int rawRank) {
    if (rawRank < 0) return 0;
    if (rawRank > 127) return 127;
    return rawRank;
}

// Populate a SkillView from a rank array with bounds clamping and null-pointer safety.
inline SkillView populateSkillView(const int* ranks, int count = 8, bool readable = true) {
    SkillView v = {};
    v.readable = readable && (ranks != nullptr);
    if (v.readable) {
        for (int i = 0; i < 8; ++i) {
            v.ranks[i] = (i < count) ? sanitizeSkillRank(ranks[i]) : 0;
        }
    }
    return v;
}

// skills.2da row for a skill name as the item text spells it (order matches the skill labels in ChareffDecode / LevelUpOverride).
inline int skillIndexForName(const char* name) {
    static const char* const kNames[8] = {"Computer Use", "Demolitions", "Stealth", "Awareness",
                                          "Persuade", "Repair", "Security", "Treat Injury"};
    if (name == nullptr) return -1;
    for (int i = 0; i < 8; ++i) {
        if (strcmp(name, kNames[i]) == 0) return i;
    }
    return -1;
}

// Requirement row ("<skill>" | "requires N"): append the player's rank inline as "(you R)" and mark the row unmet when R < N.
// Unknown skill or unreadable stats: the row keeps "requires N" and counts as met.
inline void annotateRequirement(CompareRow& row, const SkillView* skills) {
    static const char kReq[] = "requires ";
    const size_t reqLen = sizeof(kReq) - 1;
    if (row.kind != RowKind::UP || !row.hasSplit) return;
    if (strncmp(row.rightCol, kReq, reqLen) != 0) return;
    int need = 0;
    if (sscanf(row.rightCol + reqLen, "%d", &need) != 1) return;
    if (skills == nullptr || !skills->readable) return;
    const int idx = skillIndexForName(row.leftCol);
    if (idx < 0) return;
    const int have = skills->ranks[idx];
    snprintf(row.rightCol, sizeof(row.rightCol), "requires %d (you %d)", need, have);
    snprintf(row.fullLine, sizeof(row.fullLine), "%s requires %d (you %d)", row.leftCol, need, have);
    row.unmet = !isSkillMet(have, need);
}

// Unmet requirement rows draw red; every other row keeps its kind colour.
inline RgbColor rowDrawColor(const CompareRow& row) {
    if (row.unmet) return {1.00f, 0.25f, 0.25f};
    return rowColor(row.kind, row.hasSplit);
}

// Cost table polarity (WB-F5):
// 0  = categorical (no numeric order: 0 = none/unspecified, 25 = iprp_onhitdc, 26 = iprp_attribcost)
// -1 = penalty table (higher row index = worse penalty: 20 = iprp_neg5cost, 21 = iprp_neg10cost, 22 = iprp_damvulcost)
// +1 = standard numeric bonus table (higher row index = better)
inline int costTablePolarity(uint8_t costTable) {
    if (costTable == 0 || costTable == 25 || costTable == 26) {
        return 0;
    }
    if (costTable == 20 || costTable == 21 || costTable == 22) {
        return -1;
    }
    return 1;
}

inline bool isAlnumChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

inline bool hasWholeWord(const char* text, const char* word) {
    if (!text || !word || !*word) return false;
    const size_t wordLen = strlen(word);
    const char* p = strstr(text, word);
    while (p != nullptr) {
        bool leftBoundary = (p == text || !isAlnumChar(p[-1]));
        bool rightBoundary = (p[wordLen] == '\0' || !isAlnumChar(p[wordLen]));
        if (leftBoundary && rightBoundary) {
            return true;
        }
        p = strstr(p + 1, word);
    }
    return false;
}

// Workbench-local crafting polarity helper (WB-F6)
inline int craftingTextPolarity(const char* name) {
    if (name == nullptr || name[0] == '\0') return 0;

    // Penalty substrings
    static const char* const kPenaltyWords[] = {
        "Penalty", "Decrease", "Decreased", "Vulnerability", "Reduced"
    };
    for (const char* pw : kPenaltyWords) {
        if (strstr(name, pw) != nullptr) {
            return -1;
        }
    }

    // Positive whole-word matches
    static const char* const kPositiveWords[] = {
        "Attack", "Enhancement", "Damage", "Armor", "AC", "Defense",
        "Save", "Saving", "Fortitude", "Reflex", "Will", "Regeneration",
        "Blaster", "Massive", "Keen", "Strength", "Dexterity", "Constitution",
        "Intelligence", "Wisdom", "Charisma", "Computer", "Demolitions",
        "Stealth", "Awareness", "Persuade", "Repair", "Security", "Treat"
    };
    for (const char* pw : kPositiveWords) {
        if (hasWholeWord(name, pw)) {
            return 1;
        }
    }

    return 0;
}

inline bool parseLeadingSignedInt(const char* s, int* outNum, const char** outSuffix) {
    if (s == nullptr || outNum == nullptr || outSuffix == nullptr) return false;
    while (*s == ' ') ++s;
    int sign = 1;
    if (*s == '+') {
        sign = 1;
        ++s;
    } else if (*s == '-') {
        sign = -1;
        ++s;
    }
    if (*s < '0' || *s > '9') return false;
    int val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        ++s;
    }
    // Reject dice ranges like 1-6 or 1d6 where - or d follows digits
    const char* peek = s;
    while (*peek == ' ') ++peek;
    if (*peek == 'd' || *peek == 'D') return false;
    if (*peek == '-' && (peek[1] >= '0' && peek[1] <= '9')) return false;

    *outNum = sign * val;
    *outSuffix = s;
    return true;
}

inline void splitDeltaRow(const char* fromText, const char* toText,
                          char* leftCol, size_t leftSz,
                          char* rightCol, size_t rightSz,
                          char* fullLine, size_t fullSz,
                          bool* hasSplit,
                          RowKind* outKind = nullptr) {
    if (!fromText || !toText || !leftCol || !rightCol || !fullLine || !hasSplit || leftSz == 0 || rightSz == 0 || fullSz == 0) return;
    snprintf(fullLine, fullSz, "%s -> %s", fromText, toText);
    leftCol[0] = '\0';
    rightCol[0] = '\0';
    *hasSplit = false;

    // Split at the first ": " separator in both strings (WB-F6)
    const char* sepA = strstr(fromText, ": ");
    const char* sepB = strstr(toText, ": ");

    if (sepA != nullptr && sepB != nullptr) {
        size_t lenA = static_cast<size_t>(sepA - fromText);
        size_t lenB = static_cast<size_t>(sepB - toText);
        if (lenA == lenB && memcmp(fromText, toText, lenA) == 0) {
            size_t nameLen = (lenA < leftSz - 1) ? lenA : (leftSz - 1);
            memcpy(leftCol, fromText, nameLen);
            leftCol[nameLen] = '\0';

            const char* valA = sepA + 2;
            const char* valB = sepB + 2;

            int pol = craftingTextPolarity(leftCol);

            int numA = 0, numB = 0;
            const char* sufA = nullptr;
            const char* sufB = nullptr;
            bool hasNumA = parseLeadingSignedInt(valA, &numA, &sufA);
            bool hasNumB = parseLeadingSignedInt(valB, &numB, &sufB);

            if ((!hasNumA || !hasNumB) && valA != nullptr && valB != nullptr) {
                size_t cfx = 0;
                while (valA[cfx] && valA[cfx] == valB[cfx]) ++cfx;
                while (cfx > 0 && valA[cfx - 1] != ' ') --cfx;
                if (cfx > 0) {
                    hasNumA = parseLeadingSignedInt(valA + cfx, &numA, &sufA);
                    hasNumB = parseLeadingSignedInt(valB + cfx, &numB, &sufB);
                }
            }

            bool suffixesMatch = (hasNumA && hasNumB && sufA != nullptr && sufB != nullptr && strcmp(sufA, sufB) == 0);

            if (suffixesMatch) {
                int d = numB - numA;
                if (pol == 0) {
                    snprintf(rightCol, rightSz, "%s -> %s (changed)", valA, valB);
                    if (outKind) *outKind = RowKind::PLAIN;
                } else {
                    snprintf(rightCol, rightSz, "%s -> %s", valA, valB);
                    if (outKind) {
                        bool isSignedNeg = (numA < 0 || numB < 0 || valA[0] == '-' || valB[0] == '-');
                        if (pol < 0 && !isSignedNeg) {
                            d = -d;
                        }
                        if (d > 0) {
                            *outKind = RowKind::UP;
                        } else if (d < 0) {
                            *outKind = RowKind::DOWN;
                        } else {
                            *outKind = RowKind::PLAIN;
                        }
                    }
                }
            } else {
                snprintf(rightCol, rightSz, "%s -> %s (changed)", valA, valB);
                if (outKind) *outKind = RowKind::PLAIN;
            }

            *hasSplit = true;
            return;
        }
    }

    // Fallback: Check for common prefix followed by a space (e.g. "Attack Bonus +1" and "Attack Bonus +3")
    size_t common = 0;
    while (fromText[common] && fromText[common] == toText[common]) {
        ++common;
    }
    while (common > 0 && fromText[common - 1] != ' ') {
        --common;
    }

    if (common > 1 && fromText[common] != '\0' && toText[common] != '\0') {
        size_t nameLen = common - 1;
        while (nameLen > 0 && (fromText[nameLen - 1] == ':' || fromText[nameLen - 1] == ' ')) {
            --nameLen;
        }
        if (nameLen >= leftSz) nameLen = leftSz - 1;
        memcpy(leftCol, fromText, nameLen);
        leftCol[nameLen] = '\0';

        const char* remA = fromText + common;
        const char* remB = toText + common;

        int pol = craftingTextPolarity(leftCol);
        int numA = 0, numB = 0;
        const char* sufA = nullptr;
        const char* sufB = nullptr;
        bool hasNumA = parseLeadingSignedInt(remA, &numA, &sufA);
        bool hasNumB = parseLeadingSignedInt(remB, &numB, &sufB);

        bool suffixesMatch = (hasNumA && hasNumB && sufA != nullptr && sufB != nullptr && strcmp(sufA, sufB) == 0);

        if (suffixesMatch && pol != 0) {
            snprintf(rightCol, rightSz, "%s -> %s", remA, remB);
            if (outKind) {
                int d = numB - numA;
                bool isSignedNeg = (numA < 0 || numB < 0 || remA[0] == '-' || remB[0] == '-');
                if (pol < 0 && !isSignedNeg) d = -d;
                if (d > 0) *outKind = RowKind::UP;
                else if (d < 0) *outKind = RowKind::DOWN;
                else *outKind = RowKind::PLAIN;
            }
        } else if (suffixesMatch && pol == 0) {
            snprintf(rightCol, rightSz, "%s -> %s (changed)", remA, remB);
            if (outKind) *outKind = RowKind::PLAIN;
        } else if (!suffixesMatch && (hasNumA || hasNumB)) {
            snprintf(rightCol, rightSz, "%s -> %s (changed)", remA, remB);
            if (outKind) *outKind = RowKind::PLAIN;
        } else {
            snprintf(rightCol, rightSz, "%s -> %s", remA, remB);
        }
        *hasSplit = true;
        return;
    }

    snprintf(leftCol, leftSz, "%s", fullLine);
    rightCol[0] = '\0';
    *hasSplit = false;
}

inline void splitSinglePropRow(char prefixChar, const char* propText, char* leftCol, size_t leftSz, char* rightCol, size_t rightSz, char* fullLine, size_t fullSz, bool* hasSplit) {
    if (!propText || !leftCol || !rightCol || !fullLine || !hasSplit || leftSz == 0 || rightSz == 0 || fullSz == 0) return;
    snprintf(fullLine, fullSz, "%c %s", prefixChar, propText);
    leftCol[0] = '\0';
    rightCol[0] = '\0';
    *hasSplit = false;

    // Requirement text "Awareness 17 needed": split before the number so the skill is the name and "requires 17" the value (a bare "17 needed" read like a gap to close, not a rank to have).
    {
        const size_t len = strlen(propText);
        static const char kNeeded[] = " needed";
        const size_t nl = sizeof(kNeeded) - 1;
        if (len > nl && strcmp(propText + len - nl, kNeeded) == 0) {
            const char* numEnd = propText + len - nl;   // one past the number
            const char* sp = numEnd;
            while (sp > propText && sp[-1] != ' ') --sp;   // sp = start of number token
            if (sp > propText + 1 && sp < numEnd && sp[0] >= '0' && sp[0] <= '9') {
                size_t nameLen = static_cast<size_t>(sp - 1 - propText);
                if (nameLen >= leftSz) nameLen = leftSz - 1;
                memcpy(leftCol, propText, nameLen);
                leftCol[nameLen] = '\0';
                snprintf(rightCol, rightSz, "requires %.*s", static_cast<int>(numEnd - sp), sp);
                *hasSplit = true;
                return;
            }
        }
    }

    // Find last space before number or bonus: e.g. "Attack Bonus +3" or "Defense Bonus 2"
    const char* lastSpace = strrchr(propText, ' ');
    if (lastSpace != nullptr && lastSpace > propText &&
        (lastSpace[1] == '+' || lastSpace[1] == '-' || (lastSpace[1] >= '0' && lastSpace[1] <= '9'))) {
        size_t nameLen = static_cast<size_t>(lastSpace - propText);
        if (nameLen >= leftSz) nameLen = leftSz - 1;
        memcpy(leftCol, propText, nameLen);
        leftCol[nameLen] = '\0';

        if (prefixChar == '+') {
            if (lastSpace[1] == '+') {
                snprintf(rightCol, rightSz, "%s", lastSpace + 1);
            } else {
                snprintf(rightCol, rightSz, "+%s", lastSpace + 1);
            }
        } else {
            if (lastSpace[1] == '+') {
                snprintf(rightCol, rightSz, "-%s", lastSpace + 2);
            } else if (lastSpace[1] == '-') {
                snprintf(rightCol, rightSz, "%s", lastSpace + 1);
            } else {
                snprintf(rightCol, rightSz, "-%s", lastSpace + 1);
            }
        }
        *hasSplit = true;
        return;
    }

    snprintf(leftCol, leftSz, "%s", fullLine);
    rightCol[0] = '\0';
    *hasSplit = false;
}

inline void formatCraftingCompareRows(const itemcmp::ItemFacts* installed, const itemcmp::ItemFacts* viewed, CraftingCompareOut* out) {
    if (out == nullptr) return;
    memset(out, 0, sizeof(*out));

    // Row 0: Header (vs <installed name> or vs empty slot)
    if (installed != nullptr && installed->ok && installed->name[0] != '\0') {
        char hdr[96];
        snprintf(hdr, sizeof(hdr), "vs %s", installed->name);
        itemcmp::cmpd::fit(out->header, sizeof(out->header), hdr, 63);
    } else {
        itemcmp::cmpd::fit(out->header, sizeof(out->header), "vs empty slot", 63);
    }

    out->rows[0].kind = RowKind::HEADER;
    snprintf(out->rows[0].leftCol, sizeof(out->rows[0].leftCol), "%s", out->header);
    snprintf(out->rows[0].fullLine, sizeof(out->rows[0].fullLine), "%s", out->header);
    out->rows[0].hasSplit = false;
    out->nRows = 1;

    if (viewed == nullptr || !viewed->ok) {
        out->hasDelta = false;
        return;
    }

    // Empty slot or no installed properties baseline
    if (installed == nullptr || !installed->ok || installed->nProps == 0) {
        if (viewed->nProps > 0 && viewed->props[0].text[0] != '\0') {
            out->hasDelta = true;
            char d[128];
            snprintf(d, sizeof(d), "+ %s", viewed->props[0].text);
            itemcmp::cmpd::fit(out->deltaLine, sizeof(out->deltaLine), d, 63);

            for (unsigned i = 0; i < viewed->nProps && out->nRows < kMaxCompareRows; ++i) {
                if (viewed->props[i].text[0] == '\0') continue;
                int r = out->nRows++;
                out->rows[r].kind = RowKind::UP;
                splitSinglePropRow('+', viewed->props[i].text,
                                   out->rows[r].leftCol, sizeof(out->rows[r].leftCol),
                                   out->rows[r].rightCol, sizeof(out->rows[r].rightCol),
                                   out->rows[r].fullLine, sizeof(out->rows[r].fullLine),
                                   &out->rows[r].hasSplit);
            }
        } else {
            out->hasDelta = false;
            itemcmp::cmpd::fit(out->deltaLine, sizeof(out->deltaLine), "No properties", 63);
            if (out->nRows < kMaxCompareRows) {
                int r = out->nRows++;
                out->rows[r].kind = RowKind::DIM;
                snprintf(out->rows[r].leftCol, sizeof(out->rows[r].leftCol), "No properties");
                snprintf(out->rows[r].fullLine, sizeof(out->rows[r].fullLine), "No properties");
                out->rows[r].hasSplit = false;
            }
        }
        return;
    }

    // Multi-property deltas via itemcmp::propDiff
    itemcmp::PropDiff pd;
    itemcmp::propDiff(installed->props, installed->nProps, viewed->props, viewed->nProps, installed->dropped, viewed->dropped, &pd);

    bool firstDeltaSet = false;

    // 1. Changed props
    for (unsigned i = 0; i < pd.nChanged && out->nRows < kMaxCompareRows; ++i) {
        out->hasDelta = true;
        uint8_t a = pd.changed[i].a;
        uint8_t b = pd.changed[i].b;
        const itemcmp::PropRow& pa = installed->props[a];
        const itemcmp::PropRow& pb = viewed->props[b];

        int r = out->nRows++;
        int pol = costTablePolarity(pa.costTable);
        bool costValid = (pa.costTable == pb.costTable && pol != 0);
        if (costValid) {
            if (pol < 0) {
                if (pb.costValue > pa.costValue) {
                    out->rows[r].kind = RowKind::DOWN;
                } else if (pb.costValue < pa.costValue) {
                    out->rows[r].kind = RowKind::UP;
                } else {
                    out->rows[r].kind = RowKind::PLAIN;
                }
            } else {
                if (pb.costValue > pa.costValue) {
                    out->rows[r].kind = RowKind::UP;
                } else if (pb.costValue < pa.costValue) {
                    out->rows[r].kind = RowKind::DOWN;
                } else {
                    out->rows[r].kind = RowKind::PLAIN;
                }
            }
        } else {
            out->rows[r].kind = RowKind::PLAIN;
        }

        splitDeltaRow(pa.text, pb.text,
                      out->rows[r].leftCol, sizeof(out->rows[r].leftCol),
                      out->rows[r].rightCol, sizeof(out->rows[r].rightCol),
                      out->rows[r].fullLine, sizeof(out->rows[r].fullLine),
                      &out->rows[r].hasSplit,
                      nullptr);   // cost-table kind (set above) is the single source of truth; text polarity must not overwrite it

        if (!costValid) {
            out->rows[r].kind = RowKind::PLAIN;
            if (out->rows[r].hasSplit && out->rows[r].rightCol[0] != '\0' &&
                strstr(out->rows[r].rightCol, "(changed)") == nullptr) {
                size_t rLen = strlen(out->rows[r].rightCol);
                if (rLen + 11 < sizeof(out->rows[r].rightCol)) {
                    strcat(out->rows[r].rightCol, " (changed)");
                }
            }
        }

        if (!firstDeltaSet) {
            itemcmp::cmpd::fit(out->deltaLine, sizeof(out->deltaLine), out->rows[r].fullLine, 63);
            firstDeltaSet = true;
        }
    }

    // 2. Gained props
    for (unsigned i = 0; i < pd.nGained && out->nRows < kMaxCompareRows; ++i) {
        out->hasDelta = true;
        uint8_t b = pd.gained[i].b;
        const itemcmp::PropRow& pb = viewed->props[b];

        int r = out->nRows++;
        out->rows[r].kind = RowKind::UP;
        splitSinglePropRow('+', pb.text,
                           out->rows[r].leftCol, sizeof(out->rows[r].leftCol),
                           out->rows[r].rightCol, sizeof(out->rows[r].rightCol),
                           out->rows[r].fullLine, sizeof(out->rows[r].fullLine),
                           &out->rows[r].hasSplit);

        if (!firstDeltaSet) {
            itemcmp::cmpd::fit(out->deltaLine, sizeof(out->deltaLine), out->rows[r].fullLine, 63);
            firstDeltaSet = true;
        }
    }

    // 3. Lost props
    for (unsigned i = 0; i < pd.nLost && out->nRows < kMaxCompareRows; ++i) {
        out->hasDelta = true;
        uint8_t a = pd.lost[i].a;
        const itemcmp::PropRow& pa = installed->props[a];

        int r = out->nRows++;
        out->rows[r].kind = RowKind::DOWN;
        splitSinglePropRow('-', pa.text,
                           out->rows[r].leftCol, sizeof(out->rows[r].leftCol),
                           out->rows[r].rightCol, sizeof(out->rows[r].rightCol),
                           out->rows[r].fullLine, sizeof(out->rows[r].fullLine),
                           &out->rows[r].hasSplit);

        if (!firstDeltaSet) {
            itemcmp::cmpd::fit(out->deltaLine, sizeof(out->deltaLine), out->rows[r].fullLine, 63);
            firstDeltaSet = true;
        }
    }

    if (!firstDeltaSet) {
        out->hasDelta = false;
        itemcmp::cmpd::fit(out->deltaLine, sizeof(out->deltaLine), "No stat difference", 63);
        if (out->nRows < kMaxCompareRows) {
            int r = out->nRows++;
            out->rows[r].kind = RowKind::DIM;
            snprintf(out->rows[r].leftCol, sizeof(out->rows[r].leftCol), "No stat difference");
            snprintf(out->rows[r].fullLine, sizeof(out->rows[r].fullLine), "No stat difference");
            out->rows[r].hasSplit = false;
        }
    }
}

// Full compare: builds the rows, then annotates each requirement row with the player's rank. skillMet = no row is unmet.
inline void formatCraftingCompare(const itemcmp::ItemFacts* installed, const itemcmp::ItemFacts* viewed, const SkillView* skills, CraftingCompareOut* out) {
    formatCraftingCompareRows(installed, viewed, out);
    if (out == nullptr) return;
    out->skillMet = true;
    for (int i = 1; i < out->nRows && i < kMaxCompareRows; ++i) {
        annotateRequirement(out->rows[i], skills);
        if (out->rows[i].unmet) out->skillMet = false;
    }
}

// Pure box/row geometry helper (WB-F7)
struct BoxGeoIn {
    int lbX;
    int lbY;
    int lbW;
    int lbH;
    int sh;
    int nRows;
};

struct BoxGeoOut {
    int pad;
    int gap;
    int rowH;
    int boxX;
    int boxY;
    int boxW;
    int boxH;
    int newLbH;
    bool valid;
};

inline BoxGeoOut computeBoxGeo(const BoxGeoIn& in) {
    if (in.lbW <= 0 || in.lbH <= 0 || in.sh <= 0 || in.nRows <= 0) {
        return {0, 0, 0, 0, 0, 0, 0, 0, false};
    }
    int pad = 6 * in.sh / 600;
    if (pad < 2) pad = 2;

    int rowP = 18 * in.sh / 600;
    if (rowP < 10) rowP = 10;

    int gap = in.lbH * 2 / 100;
    if (gap < 2) gap = 2;

    int maxBh = in.lbH * 35 / 100;
    int rawBh = 2 * pad + in.nRows * rowP;
    int boxH = (rawBh < maxBh) ? rawBh : maxBh;
    int rowH = (boxH - 2 * pad) / in.nRows;

    int boxX = in.lbX;
    int boxW = in.lbW;
    int boxY = in.lbY + in.lbH - boxH;
    int newLbH = in.lbH - boxH - gap;

    bool valid = (newLbH > 0 && boxH > 0 && rowH > 0);

    BoxGeoOut out;
    out.pad = pad;
    out.gap = gap;
    out.rowH = rowH;
    out.boxX = boxX;
    out.boxY = boxY;
    out.boxW = boxW;
    out.boxH = boxH;
    out.newLbH = newLbH;
    out.valid = valid;
    return out;
}

struct SwitchContext {
    ScreenState state;
    bool disableCreate;
    bool disableUpgrade;
    bool subDialogOpen;
    bool inTransition;
    uintptr_t hub;
    uintptr_t createPanel;
    uintptr_t itemListPanel;
    uint32_t elapsedMsSinceLastSwitch;
    uint32_t partyMemberId;
};

// Check if BTN_SWITCH should be rendered / enabled for the current state and workbench flags.
// Both overloads use identical parameter order: (state, disableUpgrade, disableItemCreation, ...)
inline bool canShowSwitchButton(ScreenState state, uint32_t disableUpgrade, uint32_t disableItemCreation, int upgradableItemCount) {
    if (state == ScreenState::CREATE) {
        if (disableUpgrade != 0 || upgradableItemCount <= 0) return false;
        return true;
    }
    if (state == ScreenState::UPGRADE_LIST) {
        if (disableItemCreation != 0) return false;
        return true;
    }
    return false;
}

inline bool canShowSwitchButton(ScreenState state, uint32_t disableUpgrade, uint32_t disableItemCreation) {
    return canShowSwitchButton(state, disableUpgrade, disableItemCreation, 1);
}

// Evaluate a click on BTN_SWITCH, returning the action to dispatch and optional nextState.
inline SwitchAction evaluateSwitchClick(const SwitchContext& ctx, ScreenState* nextState, uint32_t* outPartyMemberId = nullptr) {
    if (nextState != nullptr) {
        *nextState = ctx.state;
    }
    if (outPartyMemberId != nullptr) {
        *outPartyMemberId = ctx.partyMemberId;
    }

    // Re-entry / double-click guard: ignore while in transition or before 250ms debounce
    if (ctx.inTransition || ctx.elapsedMsSinceLastSwitch < 250) {
        return SwitchAction::NONE;
    }

    // Sub-dialog open (e.g. breakdown/upgrade confirmation dialog)
    if (ctx.subDialogOpen) {
        return SwitchAction::NONE;
    }

    // Null pointer safety: hub must be valid
    if (ctx.hub == 0) {
        return SwitchAction::NONE;
    }

    switch (ctx.state) {
        case ScreenState::CREATE: {
            // Flag suppression: cannot switch to upgrade if upgrade is disabled
            if (ctx.disableUpgrade) {
                return SwitchAction::NONE;
            }
            // Null pointer safety for destination panel
            if (ctx.itemListPanel == 0) {
                return SwitchAction::NONE;
            }
            if (nextState != nullptr) {
                *nextState = ScreenState::UPGRADE_LIST;
            }
            return SwitchAction::OPEN_UPGRADE;
        }
        case ScreenState::UPGRADE_LIST: {
            // Flag suppression: cannot switch to create if item creation is disabled
            if (ctx.disableCreate) {
                return SwitchAction::NONE;
            }
            // Null pointer safety for destination panel
            if (ctx.createPanel == 0) {
                return SwitchAction::NONE;
            }
            if (nextState != nullptr) {
                *nextState = ScreenState::CREATE;
            }
            return SwitchAction::OPEN_CREATE;
        }
        default:
            return SwitchAction::NONE;
    }
}

// Format an item's property list (count, properties, keen flag, dropped count) into out buffer.
// Matches prompt L4 format: prop count and propId:subType:costValue list in a single line.
inline void fmtProps(char* out, size_t n, bool ok, unsigned nProps, const itemcmp::PropRow* props, bool keen, unsigned dropped) {
    if (n == 0 || out == nullptr) return;
    if (props == nullptr) nProps = 0;
    if (!ok) {
        snprintf(out, n, "props=unreadable");
        return;
    }
    int len = clampLen(snprintf(out, n, "n=%u%s%s props=[", nProps, keen ? " keen" : "", dropped ? " dropped" : ""), n);
    for (unsigned i = 0; i < nProps && len < static_cast<int>(n) - 30; ++i) {
        len = clampLen(len + snprintf(out + len, n - len, "%s%u:%u:%u", i ? " " : "",
                        static_cast<unsigned>(props[i].type),
                        static_cast<unsigned>(props[i].subtype),
                        static_cast<unsigned>(props[i].costValue)), n);
    }
    snprintf(out + len, n - len, "%s", dropped ? " ...]" : "]");
}

// Panel flags dword lives at panel+0x48 (FUN_00410530: param_2[0x12]). Bits seen in the dump: bit2 (visible-ish, UNVERIFIED),
// bit3 (set from flags&2), bit6 = 0x40 focus, bits 8-9 = a 2-bit state (FUN_00410530 else-branch).
inline void fmtPanelFlags(char* out, size_t n, uint32_t f) {
    snprintf(out, n, "%08x[b2=%u b3=%u b6=%u s=%u]", static_cast<unsigned>(f), (f >> 2) & 1u, (f >> 3) & 1u, (f >> 6) & 1u, (f >> 8) & 3u);
}

// "xxxxxxxx" when ok, else "??".
inline void fmtVal(char* out, size_t n, bool ok, uint32_t v) {
    if (ok) snprintf(out, n, "%x", static_cast<unsigned>(v));
    else snprintf(out, n, "??");
}

// Clamp a list count read from the engine: values outside 0..cap are reported as -1 (untrusted) and never iterated.
inline int sane(uint32_t count, int cap) { return count <= static_cast<uint32_t>(cap) ? static_cast<int>(count) : -1; }

// Identical consecutive log bodies are suppressed; the next different body reports how many were dropped.
struct Dedupe {
    char last[640];
    unsigned reps;
    Dedupe() : reps(0) { last[0] = 0; }
    // Returns true when `body` should be written. *dropped = repeats suppressed since the previous written line.
    bool admit(const char* body, unsigned* dropped) {
        *dropped = 0;
        if (strncmp(last, body, sizeof last - 1) == 0 && last[0] != 0) { ++reps; return false; }
        *dropped = reps;
        reps = 0;
        strncpy(last, body, sizeof last - 1);
        last[sizeof last - 1] = 0;
        return true;
    }
};

// =============================================================================
// P7: Live Lightsaber Crystal Picker (doc 55 P7)
// Pure state machine, geometry constants, and slot predicates for live blade preview.
// =============================================================================

// CSWGuiUpgrade P7 layout & member offsets (008cd2c0.c, 008ceb00.c, 008cb2f0.c)
constexpr int kWbSlotSubmodeOff = 0x3d28;  // uint32_t selection mode (0 = slots, 1 = item list / crystal list)
constexpr int kWbModeOff = 0x3d2c;         // uint8_t mode (1 = lightsaber 3D view, 2 = ranged, 3 = melee, 4 = armor)
constexpr int kWbWorkingItemOff = 0x3d34;  // uintptr_t working/staged item pointer
constexpr int kWbLbItemsOff = 0x2380;      // CSWGuiUpgrade LB_ITEMS list control
constexpr int kWb3dModelLsOff = 0x3ac4;    // CSWGuiUpgrade 3D_MODEL_LS viewport control (rect [168, 115, 168, 228])
constexpr int kWbColorCrystalSlotIdx = 1;  // Slot 1 (BTN_UPGRADE32_LS) = Color Crystal (UpgradeType = -1)
constexpr int kItemModelVarOff = 0x2a4;    // uint8_t ModelVariation on CSWSItem (blade color row, e.g. 4=Yellow, 10=Violet)

// 800x600 GUI coordinates for crystal picker inline repositioning
// Inline list covers LBL_SLOTNAME [80, 345, 344, 38] + LBL_PROPERTY [80, 388, 344, 100]
constexpr int kCrystalInlineListRect[4] = {80, 345, 344, 143};
// Pristine vanilla LB_ITEMS extent in upgrade_p.gui
constexpr int kCrystalPristineListRect[4] = {80, 118, 344, 370};

// State tracker for live crystal color preview on 3D_MODEL_LS.
struct CrystalPreviewState {
    bool active;
    uint8_t origModelVar;
    uint8_t previewModelVar;
    uintptr_t workingItem;

    CrystalPreviewState()
        : active(false), origModelVar(0), previewModelVar(0), workingItem(0) {}
};

// Returns true if the selected slot is the lightsaber color crystal slot (Slot 1 in Mode 1).
inline bool isColorCrystalSlot(uint8_t mode, int slotIdx) {
    return mode == 1 && slotIdx == kWbColorCrystalSlotIdx;
}

// Begins preview session: captures current working item and original ModelVariation.
inline void beginCrystalPreview(CrystalPreviewState* st, uintptr_t item, uint8_t currentModelVar) {
    if (st == nullptr) return;
    // Re-entry on the same item keeps the ORIGINAL blade colour; overwriting it would make cancel restore a previewed value.
    if (st->active && st->workingItem == item) return;
    st->active = true;
    st->workingItem = item;
    st->origModelVar = currentModelVar;
    st->previewModelVar = currentModelVar;
}

// Updates preview ModelVariation. Returns true if value changed (triggering CSWGuiUpgrade::Update3DView),
// or false if duplicate / no-op or preview not active.
inline bool updateCrystalPreview(CrystalPreviewState* st, uint8_t newModelVar) {
    if (st == nullptr || !st->active) {
        return false;
    }
    if (st->previewModelVar == newModelVar) {
        return false;
    }
    st->previewModelVar = newModelVar;
    return true;
}

// Cancels preview: marks inactive and returns original ModelVariation to restore on working item.
inline uint8_t cancelCrystalPreview(CrystalPreviewState* st) {
    if (st == nullptr) return 0;
    const uint8_t orig = st->origModelVar;
    st->active = false;
    st->previewModelVar = orig;
    st->workingItem = 0;
    return orig;
}

// Commits preview: marks inactive and returns selected preview ModelVariation to keep.
inline uint8_t commitCrystalPreview(CrystalPreviewState* st) {
    if (st == nullptr) return 0;
    const uint8_t committed = st->previewModelVar;
    st->active = false;
    st->origModelVar = committed;
    st->workingItem = 0;
    return committed;
}

// Reset state unconditionally (e.g. on panel close, screen pop, or unhandled exit).
inline void resetCrystalPreview(CrystalPreviewState* st) {
    if (st == nullptr) return;
    st->active = false;
    st->origModelVar = 0;
    st->previewModelVar = 0;
    st->workingItem = 0;
}

// Weapon-change reset safety: if preview was active for a different item pointer, reset it.
inline bool checkWeaponChangeReset(CrystalPreviewState* st, uintptr_t currentItem) {
    if (st != nullptr && st->active && st->workingItem != currentItem) {
        resetCrystalPreview(st);
        return true;
    }
    return false;
}

// Maps upcrystals.2da row index (-1 = currently installed row 0, 0..13 = crystal row) and baseitems.2da index
// (8 = Long Lightsaber, 9 = Short Lightsaber, 10 = Double Lightsaber) to CSWSItem::ModelVariation (+0x2a4).
inline uint8_t crystalRowToModelVar(int upcrystalsRow, uint32_t baseItem, uint8_t origModelVar) {
    if (upcrystalsRow < 0 || upcrystalsRow > 13) return origModelVar;
    // Long Lightsaber (baseItem == 8) uses w_lghtsbr_001..011; Short (9) & Double (10) use w_shortsbr/w_dblsbr_001..010
    static const uint8_t kLongVars[14]  = {1, 4, 3, 5, 2, 7, 8, 11, 7, 10, 8, 9, 2, 7};
    static const uint8_t kShortVars[14] = {1, 4, 3, 5, 2, 6, 7, 10, 6,  9, 7, 8, 2, 6};
    return (baseItem == 8) ? kLongVars[upcrystalsRow] : kShortVars[upcrystalsRow];
}

// Returns true if a dialog ResRef (null-terminated or up to 16-byte CResRef) is a workbench conversation.
inline bool isWorkbenchDialogResref(const char* resref) {
    if (!resref || resref[0] == '\0') return false;
    char buf[17] = {};
    for (int i = 0; i < 16 && resref[i] != '\0'; ++i) {
        char c = resref[i];
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
        buf[i] = c;
    }
    return strcmp(buf, "workbnch") == 0 ||
           strcmp(buf, "000_workbench") == 0 ||
           strcmp(buf, "workbnch_tut") == 0;
}

// Colour-crystal flavour line keyed by the slot-name text ("Crystal, Silver"); the 10 vanilla u_l_colo_NN descriptions (2nd sentence). nullptr for anything else (modded names).
inline const char* colorCrystalSentence(const char* name, size_t len) {
    static const struct { const char* colour; const char* line; } kTbl[] = {
        {"Blue", "It glows faintly with an inner blue light."},
        {"Green", "It glows faintly with an inner green light."},
        {"Yellow", "It glows faintly with an inner yellow light."},
        {"Red", "It glows faintly with an inner red light."},
        {"Violet", "It glows faintly with an inner violet light."},
        {"Cyan", "It glows faintly with an inner cyan light."},
        {"Silver", "It glows faintly with a silvery light."},
        {"Orange", "It glows faintly with an inner orange light."},
        {"Viridian", "It glows faintly with a silvery green light."},
        {"Bronze", "Yellowish-brown in color, the same sort of crystal is sometimes used in high resolution displays."},
    };
    if (!name) return nullptr;
    size_t i = 0;
    while (i + 9 <= len && strncmp(name + i, "Crystal, ", 9) != 0) ++i;
    if (i + 9 > len) return nullptr;
    const char* w = name + i + 9;
    const size_t wl0 = len - (i + 9);
    size_t wl = 0;
    while (wl < wl0 && w[wl] >= 32 && w[wl] != '\0') ++wl;
    while (wl > 0 && w[wl - 1] == ' ') --wl;
    for (const auto& e : kTbl) {
        const size_t n = strlen(e.colour);
        if (n == wl && strncmp(w, e.colour, n) == 0) return e.line;
    }
    return nullptr;
}

}  // namespace wbu
