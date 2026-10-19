// Host test of SelfHealRoster.h: order, dedupe (case / separators), names, slotN keys, cap, null safety.
#include "SelfHealRoster.h"
#include "check.h"
#include <stdio.h>

using namespace shroster;

static const Known kKnown[] = {{"atton", "Atton Rand"}, {"kreia", "Kreia"}, {"bao_dur", "Bao-Dur"}, {"hk47", "HK-47"}, {"t3m4", "T3-M4"}};
static const int kNK = 5;

static bool isTag(const RosterEntry& e, const char* t) { return strcmp(e.tag, t) == 0; }
static bool isName(const RosterEntry& e, const char* n) { return strcmp(e.name, n) == 0; }

static void test_order_and_names() {
    RosterEntry r[kMaxRoster];
    const char* party[] = {"player", "kreia", "atton"};
    const char* names[] = {"Exile", nullptr, "Atton"};
    const char* ini[] = {"t3m4", "kreia", "slot2", "slot0", "mystery"};
    const int n = buildRoster(party, names, 3, ini, 5, kKnown, kNK, r, kMaxRoster);
    KT_CHECK(isTag(r[0], "") && isName(r[0], "Everyone (defaults)") && !r[0].inParty);
    KT_CHECK(isTag(r[1], "player") && isName(r[1], "Exile") && r[1].inParty);       // the party's own display name wins
    KT_CHECK(isTag(r[2], "kreia") && isName(r[2], "Kreia") && r[2].inParty);         // no party name: the built-in table
    KT_CHECK(isTag(r[3], "atton") && isName(r[3], "Atton"));                          // party name beats "Atton Rand"
    KT_CHECK(isTag(r[4], "t3m4") && isName(r[4], "T3-M4") && !r[4].inParty);          // INI order next (kreia already there)
    KT_CHECK(isTag(r[5], "slot2") && isName(r[5], "Slot 2"));                         // legacy key, no tag known
    KT_CHECK(isTag(r[6], "slot0") && isName(r[6], "Slot 0"));
    KT_CHECK(isTag(r[7], "mystery") && isName(r[7], "mystery"));                      // unknown tag shows as itself
    KT_CHECK(isTag(r[8], "bao_dur") && isName(r[8], "Bao-Dur"));                      // built-ins last, not yet seen ones only
    KT_CHECK(isTag(r[9], "hk47"));
    KT_CHECK_EQ(n, 10);
}

static void test_dedupe_aliases() {
    RosterEntry r[kMaxRoster];
    const char* party[] = {"Bao-Dur", "ATTON", "atton", "hk_47"};
    const char* ini[] = {"baodur", "BAO_DUR", "Hk47"};
    const int n = buildRoster(party, nullptr, 4, ini, 3, kKnown, kNK, r, kMaxRoster);
    KT_CHECK(isTag(r[1], "Bao-Dur") && isName(r[1], "Bao-Dur"));                      // first spelling wins and is the edit key
    KT_CHECK(isTag(r[2], "ATTON") && isName(r[2], "Atton Rand"));
    KT_CHECK(isTag(r[3], "hk_47") && isName(r[3], "HK-47"));
    int count = 0;
    for (int i = 1; i < n; ++i) if (strncmp(r[i].tag, "bao", 3) == 0 || strncmp(r[i].tag, "Bao", 3) == 0) ++count;
    KT_CHECK_EQ(count, 1);                                                             // one Bao-Dur whatever the spelling
    KT_CHECK_EQ(n, 6);                                                                 // + kreia, t3m4 from the built-ins
    KT_CHECK_EQ(indexOfTag(r, n, "baodur"), 1);
    KT_CHECK_EQ(indexOfTag(r, n, "HK47"), 3);
    KT_CHECK_EQ(indexOfTag(r, n, "nobody"), 0);
    KT_CHECK_EQ(indexOfTag(r, n, ""), 0);
    KT_CHECK_EQ(indexOfTag(r, n, nullptr), 0);
}

static void test_slot_names_and_limits() {
    RosterEntry r[kMaxRoster];
    const char* ini[] = {"slot12", "slot", "slotx", "slot7a", "Slot3"};
    const int n = buildRoster(nullptr, nullptr, 0, ini, 5, nullptr, 0, r, kMaxRoster);
    KT_CHECK(isName(r[1], "Slot 12"));
    KT_CHECK(isName(r[2], "slot"));          // no digits: shown as the tag
    KT_CHECK(isName(r[3], "slotx"));
    KT_CHECK(isName(r[4], "slot7a"));
    KT_CHECK(isName(r[5], "Slot 3"));        // case-insensitive prefix
    KT_CHECK_EQ(n, 6);
    // a slot key with a known tag name stays as the tag (slot rows never alias to a tag)
    const char* longTag[] = {"abcdefghijklmnopqrstuvwxyz"};
    KT_CHECK_EQ(buildRoster(longTag, nullptr, 1, nullptr, 0, nullptr, 0, r, kMaxRoster), 1);   // too long for a RosterEntry: skipped
    const char* edge[] = {"abcdefghijklmnopqrstuvw"};                                          // 23 chars: fits
    KT_CHECK_EQ(buildRoster(edge, nullptr, 1, nullptr, 0, nullptr, 0, r, kMaxRoster), 2);
    KT_CHECK(strlen(r[1].tag) == 23);
    const char* longName[] = {"This display name is far too long to fit"};
    const char* tags[] = {"x"};
    buildRoster(tags, longName, 1, nullptr, 0, nullptr, 0, r, kMaxRoster);
    KT_CHECK(strlen(r[1].name) == (size_t)kNameLen - 1);                                         // truncated, terminated
    const char* blank[] = {"", nullptr, "ok"};
    KT_CHECK_EQ(buildRoster(blank, nullptr, 3, nullptr, 0, nullptr, 0, r, kMaxRoster), 2);       // empty / null tags skipped
}

static void test_cap_and_nulls() {
    RosterEntry r[kMaxRoster];
    KT_CHECK_EQ(buildRoster(nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 5), 0);
    KT_CHECK_EQ(buildRoster(nullptr, nullptr, 0, nullptr, 0, nullptr, 0, r, 0), 0);
    KT_CHECK_EQ(buildRoster(nullptr, nullptr, 0, nullptr, 0, nullptr, 0, r, 1), 1);               // defaults only
    KT_CHECK(isName(r[0], "Everyone (defaults)"));
    const char* party[] = {"a", "b", "c"};
    KT_CHECK_EQ(buildRoster(party, nullptr, 3, nullptr, 0, kKnown, kNK, r, 3), 3);                // party has priority over the built-ins
    KT_CHECK(isTag(r[1], "a") && isTag(r[2], "b"));
    // 60 distinct built-ins: the list stops at the cap
    static Known many[60];
    static char tagBuf[60][16];
    for (int i = 0; i < 60; ++i) { snprintf(tagBuf[i], sizeof tagBuf[i], "c%d", i); many[i].tag = tagBuf[i]; many[i].name = tagBuf[i]; }
    KT_CHECK_EQ(buildRoster(nullptr, nullptr, 0, nullptr, 0, many, 60, r, kMaxRoster), kMaxRoster);
    // sameTag edge cases
    KT_CHECK(detail::sameTag("", ""));
    KT_CHECK(detail::sameTag("a_b", "AB"));
    KT_CHECK(!detail::sameTag("ab", "abc"));
    KT_CHECK(!detail::sameTag("a", ""));
    KT_CHECK(detail::sameTag("__", ""));
}

int main() {
    test_order_and_names();
    test_dedupe_aliases();
    test_slot_names_and_limits();
    test_cap_and_nulls();
    return kt::Report("selfheal_roster_test");
}
