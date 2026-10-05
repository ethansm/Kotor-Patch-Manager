// Tests for QueueHudLogic.h (queue-hud pure logic). Every rule in the header comment (D1..D5) and every boundary is pinned.
#include "QueueHudLogic.h"
#include "check.h"
#include "SelfHealUi.h"   // real neighbour geometry: shui::buttonRect / ringRect
#include <initializer_list>
#include <stdio.h>
#include <string.h>

using namespace qh;

namespace {

RNode N(uint8_t sub, uint32_t ptr, uint32_t spell = 0, uint32_t item = 0, uint32_t feat = 0) { RNode n; n.subType = sub; n.spellId = spell; n.itemId = item; n.featId = feat; n.ptr = ptr; return n; }
QEntry E(int type, uint32_t p0 = 0) { return QEntry{ type, p0 }; }

void testConstants() {
    KT_CHECK_EQ(kMaxSlots, 4); KT_CHECK_EQ(kMaxEntries, 16); KT_CHECK_EQ(kMaxRound, 4);
    KT_CHECK_EQ((int)NODE_NONE, -1); KT_CHECK_EQ((int)NODE_CUR, -2);
    KT_CHECK(isQualifying(0x0C) && isQualifying(0x0F) && isQualifying(0x2E) && isQualifying(0x3F));
    for (int t : { 0, 1, 0x0B, 0x0D, 0x0E, 0x10, 0x2D, 0x2F, 0x3E, 0x40, -1 }) KT_CHECK(!isQualifying(t));
}

void testKeys() {
    KT_CHECK_EQ(mixKey(0x0F, 5), 0xffa5885fu);                   // pinned FNV-1a values (a changed key scheme invalidates the DLL icon cache)
    KT_CHECK_EQ(mixKey(0x2E, 0x7F000001u), 0x6b29bab3u);
    KT_CHECK_EQ(mixKey(0x109, 300), 0x65329360u);
    KT_CHECK_EQ(mixKey(0x101, 0), 0x72d56f3fu);
    KT_CHECK_EQ(mixKey(0, 0), 0x9be17165u);
    KT_CHECK(mixKey(0x0F, 5) != mixKey(0x0F, 6)); KT_CHECK(mixKey(0x0F, 5) != mixKey(0x2E, 5));
    KT_CHECK_EQ(entryKey(E(0x0F, 5)), 0xffa5885fu); KT_CHECK_EQ(entryKey(E(0x2E, 0x7F000001u)), 0x6b29bab3u);
    // node keys: the id that matters per sub-type, nothing else
    KT_CHECK_EQ(nodeKey(N(9, 1, 300, 77, 88)), 0x65329360u);                 // spell id only
    KT_CHECK_EQ(nodeKey(N(9, 99, 300, 1, 2)), nodeKey(N(9, 1, 300, 77, 88)));  // ptr/item/feat are not part of a spell key
    KT_CHECK(nodeKey(N(9, 1, 301)) != nodeKey(N(9, 1, 300)));
    KT_CHECK_EQ(nodeKey(N(10, 1, 5, 0x7F000002u, 9)), 0x08b2d917u);           // item id only
    KT_CHECK(nodeKey(N(10, 1, 0, 0x7F000003u)) != nodeKey(N(10, 1, 0, 0x7F000002u)));
    KT_CHECK_EQ(nodeKey(N(11, 1, 5, 6, 0x12)), 0xb637b39bu);                  // feat id only
    KT_CHECK_EQ(nodeKey(N(11, 1, 0, 0, 0xAB120012u)), 0xb637b39bu);           // feat is a WORD: high 16 bits ignored
    KT_CHECK(nodeKey(N(11, 1, 0, 0, 0x13)) != nodeKey(N(11, 1, 0, 0, 0x12)));
    KT_CHECK_EQ(nodeKey(N(1, 1, 300, 5, 6)), 0x72d56f3fu);                    // attack: ids ignored
    KT_CHECK_EQ(nodeKey(N(6, 1, 300, 5, 6)), nodeKey(N(6, 2, 0, 0, 0)));
    KT_CHECK(nodeKey(N(6, 1)) != nodeKey(N(7, 1)));
    KT_CHECK(nodeKey(N(13, 1)) != nodeKey(N(14, 1)));
    KT_CHECK(mixKey(0, 0) != 0);
}

void testNodeIconValid() {
    const uint8_t both[] = { 1, 6, 7, 9, 10, 11 };
    for (uint8_t s : both) { KT_CHECK(nodeIconValid(0x0C, s)); KT_CHECK(nodeIconValid(0x3F, s)); }
    KT_CHECK(!nodeIconValid(0x0C, 13) && !nodeIconValid(0x0C, 14));   // 0x0C table has no stance / switch-weapon
    KT_CHECK(nodeIconValid(0x3F, 13) && nodeIconValid(0x3F, 14));
    const uint8_t blank[] = { 0, 2, 3, 4, 5, 8, 12, 15, 16, 255 };
    for (uint8_t s : blank) { KT_CHECK(!nodeIconValid(0x0C, s)); KT_CHECK(!nodeIconValid(0x3F, s)); }
}

void testEmptyAndNulls() {
    Slot out[4];
    QEntry e[1] = { E(0x0F, 1) };
    KT_CHECK_EQ(buildSlots(e, 0, nullptr, 0, nullptr, out), 0);
    KT_CHECK_EQ(buildSlots(e, -3, nullptr, 0, nullptr, out), 0);
    KT_CHECK_EQ(buildSlots(nullptr, 1, nullptr, 0, nullptr, out), 0);
    KT_CHECK_EQ(buildSlots(e, 1, nullptr, 0, nullptr, nullptr), 0);
    QEntry none[3] = { E(0), E(0x10), E(0x40) };   // no qualifying type
    RNode cur = N(1, 5);
    KT_CHECK_EQ(buildSlots(none, 3, nullptr, 0, &cur, out), 0);
}

void testFirstEntry() {
    Slot out[4];
    // 0x0F first, no cur: slot(entry, NONE), entry-keyed
    QEntry e1[1] = { E(0x0F, 5) };
    KT_CHECK_EQ(buildSlots(e1, 1, nullptr, 0, nullptr, out), 1);
    KT_CHECK_EQ(out[0].entry, 0); KT_CHECK_EQ(out[0].node, (int)NODE_NONE); KT_CHECK_EQ(out[0].key, 0xffa5885fu);
    // 0x2E first, no cur
    QEntry e2[1] = { E(0x2E, 0x7F000001u) };
    KT_CHECK_EQ(buildSlots(e2, 1, nullptr, 0, nullptr, out), 1);
    KT_CHECK_EQ(out[0].node, (int)NODE_NONE); KT_CHECK_EQ(out[0].key, 0x6b29bab3u);
    // 0x0F first with cur: NODE_CUR but still keyed by the entry (icon ignores the node)
    RNode cur = N(1, 0x1000);
    KT_CHECK_EQ(buildSlots(e1, 1, nullptr, 0, &cur, out), 1);
    KT_CHECK_EQ(out[0].node, (int)NODE_CUR); KT_CHECK_EQ(out[0].key, entryKey(e1[0]));
    // 0x0F first with a cur whose sub-type is blank: still a slot (entry icon does not care)
    RNode curBlank = N(0, 0x1000);
    KT_CHECK_EQ(buildSlots(e1, 1, nullptr, 0, &curBlank, out), 1);
    // 0x0C first: needs the node
    QEntry c0[1] = { E(0x0C) };
    KT_CHECK_EQ(buildSlots(c0, 1, nullptr, 0, nullptr, out), 0);
    RNode curSpell = N(9, 0x1000, 300);
    KT_CHECK_EQ(buildSlots(c0, 1, nullptr, 0, &curSpell, out), 1);
    KT_CHECK_EQ(out[0].entry, 0); KT_CHECK_EQ(out[0].node, (int)NODE_CUR); KT_CHECK_EQ(out[0].key, 0x65329360u);
    // 0x0C with a blank-icon cur (D5): no slot; 0x0C does not own stance/switch icons
    KT_CHECK_EQ(buildSlots(c0, 1, nullptr, 0, &curBlank, out), 0);
    RNode curStance = N(13, 0x1000), curSwitch = N(14, 0x1000);
    KT_CHECK_EQ(buildSlots(c0, 1, nullptr, 0, &curStance, out), 0);
    KT_CHECK_EQ(buildSlots(c0, 1, nullptr, 0, &curSwitch, out), 0);
    // 0x3F first with cur: node-keyed NODE_CUR (D1: no combat flag needed)
    QEntry t0[1] = { E(0x3F) };
    KT_CHECK_EQ(buildSlots(t0, 1, nullptr, 0, &curSpell, out), 1);
    KT_CHECK_EQ(out[0].node, (int)NODE_CUR); KT_CHECK_EQ(out[0].key, 0x65329360u);
    KT_CHECK_EQ(buildSlots(t0, 1, nullptr, 0, &curStance, out), 1);   // 0x3F does own the stance icon
    KT_CHECK_EQ(out[0].key, nodeKey(curStance));
    KT_CHECK_EQ(buildSlots(t0, 1, nullptr, 0, &curBlank, out), 0);    // D5
    // D4: lone 0x3F with nothing -> nothing
    KT_CHECK_EQ(buildSlots(t0, 1, nullptr, 0, nullptr, out), 0);
}

void testLaterEntries() {
    Slot out[4];
    RNode cur = N(1, 0x1000);
    QEntry e[4] = { E(0x0F, 1), E(0x2E, 2), E(0x0F, 3), E(0x2E, 4) };
    KT_CHECK_EQ(buildSlots(e, 4, nullptr, 0, &cur, out), 4);
    KT_CHECK_EQ(out[0].node, (int)NODE_CUR);
    for (int i = 1; i < 4; ++i) { KT_CHECK_EQ(out[i].entry, i); KT_CHECK_EQ(out[i].node, (int)NODE_NONE); KT_CHECK_EQ(out[i].key, entryKey(e[i])); }
    KT_CHECK_EQ(out[0].key, entryKey(e[0]));
    // later 0x0C never gets a slot (no node); non-qualifying entries are skipped without consuming positions
    QEntry m[5] = { E(0x0F, 1), E(0x0C), E(0x05), E(0x2E, 9), E(0x0C) };
    KT_CHECK_EQ(buildSlots(m, 5, nullptr, 0, nullptr, out), 2);
    KT_CHECK_EQ(out[0].entry, 0); KT_CHECK_EQ(out[1].entry, 3); KT_CHECK_EQ(out[1].node, (int)NODE_NONE);
    // a leading 0x0C is the "first" entry (it takes cur and blanks), so entries after it are "later" (NODE_NONE, not NODE_CUR)
    QEntry l[2] = { E(0x0C), E(0x0F, 7) };
    KT_CHECK_EQ(buildSlots(l, 2, nullptr, 0, nullptr, out), 1);
    KT_CHECK_EQ(out[0].entry, 1); KT_CHECK_EQ(out[0].node, (int)NODE_NONE);
    RNode badCur = N(0, 0x1000);
    KT_CHECK_EQ(buildSlots(l, 2, nullptr, 0, &badCur, out), 1);   // cur consumed by the 0x0C even though blank: the 0x0F is NOT NODE_CUR
    KT_CHECK_EQ(out[0].entry, 1); KT_CHECK_EQ(out[0].node, (int)NODE_NONE);
    // a 0x3F without cur does not consume "first": the next 0x0F is first (no cur -> NODE_NONE either way, so this is observable only through cur)
    QEntry t[2] = { E(0x3F), E(0x0F, 7) };
    KT_CHECK_EQ(buildSlots(t, 2, nullptr, 0, nullptr, out), 1);
    KT_CHECK_EQ(out[0].entry, 1); KT_CHECK_EQ(out[0].node, (int)NODE_NONE);
    // with cur, a leading 0x3F takes it (NODE_CUR), the later 0x0F is NODE_NONE
    KT_CHECK_EQ(buildSlots(t, 2, nullptr, 0, &cur, out), 2);
    KT_CHECK_EQ(out[0].entry, 0); KT_CHECK_EQ(out[0].node, (int)NODE_CUR);
    KT_CHECK_EQ(out[1].entry, 1); KT_CHECK_EQ(out[1].node, (int)NODE_NONE);
    // a leading blank-cur 0x3F still consumes cur (vanilla sets the flag): later 0x0F is NODE_NONE
    KT_CHECK_EQ(buildSlots(t, 2, nullptr, 0, &badCur, out), 1);
    KT_CHECK_EQ(out[0].entry, 1); KT_CHECK_EQ(out[0].node, (int)NODE_NONE);
}

void testExpansion() {
    Slot out[4];
    RNode r[4] = { N(1, 0x2000), N(9, 0x2010, 300), N(10, 0x2020, 0, 0x7F000002u), N(11, 0x2030, 0, 0, 0x12) };
    QEntry t[1] = { E(0x3F) };
    // no cur: one slot per round node in order, node = index k, all keyed by node
    KT_CHECK_EQ(buildSlots(t, 1, r, 4, nullptr, out), 4);
    for (int k = 0; k < 4; ++k) { KT_CHECK_EQ(out[k].entry, 0); KT_CHECK_EQ(out[k].node, k); KT_CHECK_EQ(out[k].key, nodeKey(r[k])); }
    // nrn cap = 4: a bigger count does not read past the array (5 nodes passed, 4 used)
    RNode r5[5] = { r[0], r[1], r[2], r[3], N(1, 0x2040) };
    KT_CHECK_EQ(buildSlots(t, 1, r5, 5, nullptr, out), 4);
    // 2 nodes
    KT_CHECK_EQ(buildSlots(t, 1, r, 2, nullptr, out), 2);
    KT_CHECK_EQ(out[1].node, 1);
    // negative nrn / null rn behave as empty
    KT_CHECK_EQ(buildSlots(t, 1, r, -1, nullptr, out), 0);
    KT_CHECK_EQ(buildSlots(t, 1, nullptr, 4, nullptr, out), 0);
    // cur + round: cur takes slot 0, round nodes follow (distinct ptrs, nothing deduped)
    RNode cur = N(6, 0x1000);
    KT_CHECK_EQ(buildSlots(t, 1, r, 3, &cur, out), 4);
    KT_CHECK_EQ(out[0].node, (int)NODE_CUR); KT_CHECK_EQ(out[0].key, nodeKey(cur));
    for (int k = 0; k < 3; ++k) { KT_CHECK_EQ(out[k + 1].node, k); KT_CHECK_EQ(out[k + 1].key, nodeKey(r[k])); }
    // slot cap 4 inside the expansion: cur + 4 nodes -> 4 slots, last node dropped
    KT_CHECK_EQ(buildSlots(t, 1, r, 4, &cur, out), 4);
    KT_CHECK_EQ(out[3].node, 2);
    // D3 dedupe: a round node with cur's ptr is skipped when cur produced a slot (the first, middle, and last position)
    for (int dup = 0; dup < 3; ++dup) {
        RNode rd[3] = { N(1, 0x2000), N(9, 0x2010, 300), N(10, 0x2020, 0, 5) };
        RNode c = rd[dup];
        KT_CHECK_EQ(buildSlots(t, 1, rd, 3, &c, out), 3);
        KT_CHECK_EQ(out[0].node, (int)NODE_CUR);
        int seen = 0;
        for (int i = 1; i < 3; ++i) { KT_CHECK(out[i].node != dup); seen += out[i].node >= 0; }
        KT_CHECK_EQ(seen, 2);
    }
    // dedupe compares ptr, not content: same sub-type/ids but a different ptr is kept
    RNode same = N(1, 0x3000);
    RNode rs[1] = { N(1, 0x3001) };
    KT_CHECK_EQ(buildSlots(t, 1, rs, 1, &same, out), 2);
    // no dedupe when cur produced no slot (blank cur): the node with that ptr is shown
    RNode curBlank = N(0, 0x4000);
    RNode rb[2] = { N(1, 0x4000), N(6, 0x4010) };
    KT_CHECK_EQ(buildSlots(t, 1, rb, 2, &curBlank, out), 2);
    KT_CHECK_EQ(out[0].node, 0); KT_CHECK_EQ(out[1].node, 1);
    // dedupe also applies when cur was consumed by a 0x0F (cur produced a slot) and a later 0x3F expands
    QEntry mix[2] = { E(0x0F, 5), E(0x3F) };
    RNode c2 = N(1, 0x5000);
    RNode r2[2] = { N(1, 0x5000), N(6, 0x5010) };
    KT_CHECK_EQ(buildSlots(mix, 2, r2, 2, &c2, out), 2);
    KT_CHECK_EQ(out[0].entry, 0); KT_CHECK_EQ(out[0].node, (int)NODE_CUR);
    KT_CHECK_EQ(out[1].entry, 1); KT_CHECK_EQ(out[1].node, 1);
    // D5: blank round nodes are skipped without consuming a slot or an index
    RNode rbl[4] = { N(2, 0x6000), N(1, 0x6010), N(0, 0x6020), N(14, 0x6030) };
    KT_CHECK_EQ(buildSlots(t, 1, rbl, 4, nullptr, out), 2);
    KT_CHECK_EQ(out[0].node, 1); KT_CHECK_EQ(out[1].node, 3);
    const uint8_t blank[] = { 0, 2, 3, 4, 5, 8, 12, 15, 99 };
    for (uint8_t s : blank) { RNode one[1] = { N(s, 0x7000) }; KT_CHECK_EQ(buildSlots(t, 1, one, 1, nullptr, out), 0); }
    const uint8_t valid[] = { 1, 6, 7, 9, 10, 11, 13, 14 };
    for (uint8_t s : valid) { RNode one[1] = { N(s, 0x7000) }; KT_CHECK_EQ(buildSlots(t, 1, one, 1, nullptr, out), 1); }
    // nrn cap 4 is applied before reading: a valid 5th node must not be seen when the first four are blank
    RNode r5b[5] = { N(2, 0x8000), N(3, 0x8010), N(4, 0x8020), N(5, 0x8030), N(1, 0x8040) };
    KT_CHECK_EQ(buildSlots(t, 1, r5b, 5, nullptr, out), 0);
    // D2: only the FIRST 0x3F expands; a second 0x3F adds nothing
    QEntry two[2] = { E(0x3F), E(0x3F) };
    KT_CHECK_EQ(buildSlots(two, 2, r, 2, nullptr, out), 2);
    KT_CHECK_EQ(out[0].entry, 0); KT_CHECK_EQ(out[1].entry, 0);
    // the expansion is attributed to the first 0x3F entry's index, in queue order with the 0x0F/0x2E slots around it
    QEntry ord[4] = { E(0x0F, 1), E(0x05), E(0x3F), E(0x2E, 2) };
    KT_CHECK_EQ(buildSlots(ord, 4, r, 2, nullptr, out), 4);
    KT_CHECK_EQ(out[0].entry, 0); KT_CHECK_EQ(out[0].node, (int)NODE_NONE);
    KT_CHECK_EQ(out[1].entry, 2); KT_CHECK_EQ(out[1].node, 0);
    KT_CHECK_EQ(out[2].entry, 2); KT_CHECK_EQ(out[2].node, 1);
    KT_CHECK_EQ(out[3].entry, 3); KT_CHECK_EQ(out[3].node, (int)NODE_NONE);
    // the 0x3F does not expand a second time even with a first-qualifying 0x3F that had no cur (D4 + D2)
    QEntry td[3] = { E(0x3F), E(0x3F), E(0x0F, 9) };
    KT_CHECK_EQ(buildSlots(td, 3, r, 1, nullptr, out), 2);
    KT_CHECK_EQ(out[0].entry, 0); KT_CHECK_EQ(out[0].node, 0); KT_CHECK_EQ(out[1].entry, 2);
    // an expansion before a 0x0F that follows: later 0x0F fills the remaining slot, not more
    QEntry full[3] = { E(0x3F), E(0x0F, 1), E(0x0F, 2) };
    KT_CHECK_EQ(buildSlots(full, 3, r, 4, nullptr, out), 4);
    KT_CHECK_EQ(out[3].node, 3);   // the 4 round nodes filled all slots; the 0x0F entries never get in
}

void testCaps() {
    Slot out[4];
    // slot cap 4 over plain entries
    QEntry many[16];
    for (int i = 0; i < 16; ++i) many[i] = E(0x0F, 100 + i);
    KT_CHECK_EQ(buildSlots(many, 16, nullptr, 0, nullptr, out), 4);
    for (int i = 0; i < 4; ++i) { KT_CHECK_EQ(out[i].entry, i); KT_CHECK_EQ(out[i].key, entryKey(many[i])); }
    // ne cap 16: a qualifying entry at index 16 is invisible, one at index 15 is read. Slot buffer guards catch writes past 4.
    QEntry far[20];
    for (int i = 0; i < 20; ++i) far[i] = E(0x05);
    far[15] = E(0x0F, 1);
    KT_CHECK_EQ(buildSlots(far, 20, nullptr, 0, nullptr, out), 1);
    KT_CHECK_EQ(out[0].entry, 15);
    far[15] = E(0x05); far[16] = E(0x0F, 1);
    KT_CHECK_EQ(buildSlots(far, 20, nullptr, 0, nullptr, out), 0);
    far[16] = E(0x05); far[19] = E(0x3F);
    RNode r[1] = { N(1, 0x2000) };
    KT_CHECK_EQ(buildSlots(far, 20, r, 1, nullptr, out), 0);
    far[14] = E(0x3F);
    KT_CHECK_EQ(buildSlots(far, 20, r, 1, nullptr, out), 1);
    KT_CHECK_EQ(out[0].entry, 14);
    // no write past slot 4: guarded buffer
    struct { Slot s[4]; Slot guard[2]; } g;
    memset(&g, 0xAB, sizeof g);
    KT_CHECK_EQ(buildSlots(many, 16, nullptr, 0, nullptr, g.s), 4);
    Slot want; memset(&want, 0xAB, sizeof want);
    KT_CHECK(memcmp(&g.guard[0], &want, sizeof want) == 0 && memcmp(&g.guard[1], &want, sizeof want) == 0);
    // the expansion also never writes past slot 4 (guarded buffer; cur + 4 round nodes would be 5)
    {
        RNode r4[4] = { N(1, 0x10), N(6, 0x20), N(7, 0x30), N(9, 0x40, 1) };
        RNode cur = N(1, 0x99);
        QEntry t[1] = { E(0x3F) };
        struct { Slot s[4]; Slot guard[2]; } g2;
        memset(&g2, 0xAB, sizeof g2);
        KT_CHECK_EQ(buildSlots(t, 1, r4, 4, &cur, g2.s), 4);
        KT_CHECK(memcmp(&g2.guard[0], &want, sizeof want) == 0 && memcmp(&g2.guard[1], &want, sizeof want) == 0);
        memset(&g2, 0xAB, sizeof g2);
        QEntry mixed[5] = { E(0x3F), E(0x0F, 1), E(0x0F, 2), E(0x0F, 3), E(0x0F, 4) };
        KT_CHECK_EQ(buildSlots(mixed, 5, r4, 4, nullptr, g2.s), 4);
        KT_CHECK(memcmp(&g2.guard[0], &want, sizeof want) == 0 && memcmp(&g2.guard[1], &want, sizeof want) == 0);
    }
    // exactly 3 entries -> 3 slots; 4 -> 4 (the cap is not off by one on the low side)
    KT_CHECK_EQ(buildSlots(many, 3, nullptr, 0, nullptr, out), 3);
    KT_CHECK_EQ(buildSlots(many, 4, nullptr, 0, nullptr, out), 4);
    KT_CHECK_EQ(buildSlots(many, 5, nullptr, 0, nullptr, out), 4);
    KT_CHECK_EQ(buildSlots(many, 1, nullptr, 0, nullptr, out), 1);
}

void testWaitingAndShow() {
    KT_CHECK(!isWaiting(false, 0, 0));
    KT_CHECK(isWaiting(true, 0, 0));
    KT_CHECK(isWaiting(false, 1, 0));    // boundary: ab8 >= 1
    KT_CHECK(isWaiting(false, 5, 0));
    KT_CHECK(!isWaiting(false, -1, 0));  // negative is not waiting
    KT_CHECK(isWaiting(false, 0, 1));
    KT_CHECK(isWaiting(false, 0, -1));   // aa8 != 0, either sign
    KT_CHECK(isWaiting(true, 3, 4));

    KT_CHECK(shouldShow(true, false, false, 0, 1));
    KT_CHECK(shouldShow(true, false, false, 249, 1));    // boundary: < 250
    KT_CHECK(!shouldShow(true, false, false, 250, 1));
    KT_CHECK(!shouldShow(true, false, false, 251, 1));
    KT_CHECK(!shouldShow(true, false, false, 0xFFFFFFFFu, 1));
    KT_CHECK(shouldShow(true, false, false, 100, 4));
    KT_CHECK(!shouldShow(true, false, false, 100, 0));   // boundary: nSlots > 0
    KT_CHECK(!shouldShow(true, false, false, 100, -1));
    KT_CHECK(!shouldShow(false, false, false, 100, 1));  // each gate alone
    KT_CHECK(!shouldShow(true, true, false, 100, 1));
    KT_CHECK(!shouldShow(true, false, true, 100, 1));
}

void testConfigDefaultsAndStyle() {
    Config c;
    KT_CHECK(c.enabled && c.waitingDim && c.log);
    KT_CHECK(c.bg[0] == 0.10f && c.bg[1] == 0.55f && c.bg[2] == 0.20f && c.bgAlpha == 0.45f);
    KT_CHECK(c.border[0] == 0.30f && c.border[1] == 0.95f && c.border[2] == 0.40f && c.borderAlpha == 0.9f);
    KT_CHECK_EQ(c.borderPx, 2); KT_CHECK(c.waitingAlpha == 0.6f);

    Style s = slotStyle(c, 0, false);
    KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f && s.borderA == 0.9f && s.borderPx == 2);
    KT_CHECK(s.bg[0] == 0.10f && s.bg[1] == 0.55f && s.bg[2] == 0.20f && s.border[0] == 0.30f && s.border[1] == 0.95f && s.border[2] == 0.40f);
    s = slotStyle(c, 0, true);   // dimmed
    KT_CHECK(s.iconA == 0.6f && s.bgA == 0.45f * 0.6f && s.borderA == 0.9f * 0.6f);
    KT_CHECK(s.bg[1] == 0.55f && s.border[1] == 0.95f);   // colours are not dimmed
    for (int i = 1; i < 4; ++i) { s = slotStyle(c, i, true); KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f && s.borderA == 0.9f); }
    s = slotStyle(c, -1, true); KT_CHECK(s.iconA == 1.0f);
    c.waitingDim = false;
    s = slotStyle(c, 0, true); KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f && s.borderA == 0.9f);
    c.waitingDim = true; c.waitingAlpha = 0.5f; c.bgAlpha = 0.8f; c.borderAlpha = 0.4f; c.borderPx = 5; c.bg[0] = 0.25f; c.border[2] = 0.75f;
    s = slotStyle(c, 0, true);
    KT_CHECK(s.iconA == 0.5f && s.bgA == 0.4f && s.borderA == 0.2f && s.borderPx == 5 && s.bg[0] == 0.25f && s.border[2] == 0.75f);
}

bool near(float a, float b) { return a - b < 1e-5f && b - a < 1e-5f; }

void testIniLine() {
    Config c;
    // ignored lines
    for (const char* l : { "", "   ", "\t", "; comment", "# comment", "  ; indented", "[queue_hud]", "  [General]", "\r" }) { Config d; KT_CHECK(parseIniLine(d, l)); }
    // booleans
    KT_CHECK(parseIniLine(c, "enabled=0")); KT_CHECK(!c.enabled);
    KT_CHECK(parseIniLine(c, "enabled = 1")); KT_CHECK(c.enabled);
    KT_CHECK(parseIniLine(c, "  Enabled\t=\tFALSE  ")); KT_CHECK(!c.enabled);
    KT_CHECK(parseIniLine(c, "ENABLED=0")); KT_CHECK(!c.enabled);
    KT_CHECK(parseIniLine(c, "enabled=ON")); KT_CHECK(c.enabled);
    KT_CHECK(parseIniLine(c, "enabled=No")); KT_CHECK(!c.enabled);
    KT_CHECK(parseIniLine(c, "enabled=true")); KT_CHECK(c.enabled);
    KT_CHECK(parseIniLine(c, "enabled=off")); KT_CHECK(!c.enabled);
    KT_CHECK(parseIniLine(c, "enabled=yes")); KT_CHECK(c.enabled);
    KT_CHECK(parseIniLine(c, "waiting_dim=0")); KT_CHECK(!c.waitingDim);
    KT_CHECK(parseIniLine(c, "waiting_dim=1")); KT_CHECK(c.waitingDim);
    KT_CHECK(parseIniLine(c, "log=0")); KT_CHECK(!c.log);
    KT_CHECK(parseIniLine(c, "log=1")); KT_CHECK(c.log);
    // floats (clamped)
    KT_CHECK(parseIniLine(c, "BG_ALPHA=0.5")); KT_CHECK(near(c.bgAlpha, 0.5f));
    KT_CHECK(parseIniLine(c, "bg_alpha=0.25")); KT_CHECK(near(c.bgAlpha, 0.25f));
    KT_CHECK(parseIniLine(c, "bg_alpha = 1")); KT_CHECK(c.bgAlpha == 1.0f);
    KT_CHECK(parseIniLine(c, "bg_alpha=0")); KT_CHECK(c.bgAlpha == 0.0f);
    KT_CHECK(parseIniLine(c, "bg_alpha=7.5")); KT_CHECK(c.bgAlpha == 1.0f);
    KT_CHECK(parseIniLine(c, "bg_alpha=-0.5")); KT_CHECK(c.bgAlpha == 0.0f);
    KT_CHECK(parseIniLine(c, "bg_alpha=+0.5")); KT_CHECK(near(c.bgAlpha, 0.5f));
    KT_CHECK(parseIniLine(c, "bg_alpha=.75")); KT_CHECK(near(c.bgAlpha, 0.75f));
    KT_CHECK(parseIniLine(c, "bg_alpha=1.")); KT_CHECK(c.bgAlpha == 1.0f);
    KT_CHECK(parseIniLine(c, "border_alpha=0.3")); KT_CHECK(near(c.borderAlpha, 0.3f));
    KT_CHECK(parseIniLine(c, "border_alpha=2")); KT_CHECK(c.borderAlpha == 1.0f);
    KT_CHECK(parseIniLine(c, "border_alpha=-1")); KT_CHECK(c.borderAlpha == 0.0f);
    KT_CHECK(parseIniLine(c, "waiting_alpha=0.35")); KT_CHECK(near(c.waitingAlpha, 0.35f));
    KT_CHECK(parseIniLine(c, "waiting_alpha=9")); KT_CHECK(c.waitingAlpha == 1.0f);
    KT_CHECK(parseIniLine(c, "waiting_alpha=-9")); KT_CHECK(c.waitingAlpha == 0.0f);
    // rgb
    KT_CHECK(parseIniLine(c, "bg_rgb=0.1,0.2,0.3")); KT_CHECK(near(c.bg[0], 0.1f) && near(c.bg[1], 0.2f) && near(c.bg[2], 0.3f));
    KT_CHECK(parseIniLine(c, "bg_rgb=0,1,1")); KT_CHECK(c.bg[0] == 0.0f && c.bg[1] == 1.0f && c.bg[2] == 1.0f);
    KT_CHECK(parseIniLine(c, "bg_rgb=1,0,1")); KT_CHECK(c.bg[0] == 1.0f && c.bg[1] == 0.0f && c.bg[2] == 1.0f);
    KT_CHECK(parseIniLine(c, "bg_rgb=1,1,0")); KT_CHECK(c.bg[0] == 1.0f && c.bg[1] == 1.0f && c.bg[2] == 0.0f);
    KT_CHECK(parseIniLine(c, "bg_rgb = 1, 0 ,0.5 ")); KT_CHECK(c.bg[0] == 1.0f && c.bg[1] == 0.0f && near(c.bg[2], 0.5f));
    KT_CHECK(parseIniLine(c, "bg_rgb=2,-1,0.5")); KT_CHECK(c.bg[0] == 1.0f && c.bg[1] == 0.0f && near(c.bg[2], 0.5f));   // clamped per component
    KT_CHECK(parseIniLine(c, "border_rgb=0.9,0.8,0.7")); KT_CHECK(near(c.border[0], 0.9f) && near(c.border[1], 0.8f) && near(c.border[2], 0.7f));
    // border_px
    for (int px = 1; px <= 6; ++px) { char l[32]; snprintf(l, sizeof l, "border_px=%d", px); KT_CHECK(parseIniLine(c, l)); KT_CHECK_EQ(c.borderPx, px); }
    // independence: setting one key never touches another
    Config d; parseIniLine(d, "border_px=4");
    Config e0;
    KT_CHECK(d.enabled == e0.enabled && d.bgAlpha == e0.bgAlpha && d.borderAlpha == e0.borderAlpha && d.waitingAlpha == e0.waitingAlpha && d.bg[0] == e0.bg[0] && d.border[0] == e0.border[0] && d.waitingDim == e0.waitingDim && d.log == e0.log);
}

void testIniBad() {
    // each bad line returns false and leaves the whole Config unchanged
    const char* bad[] = {
        "enabled=maybe", "enabled=", "enabled=2", "enabled=truee", "enabled=tru", "enabled=1 1",
        "bg_alpha=abc", "bg_alpha=", "bg_alpha=0.5x", "bg_alpha=1e1", "bg_alpha=inf", "bg_alpha=nan", "bg_alpha=0x1", "bg_alpha=--1", "bg_alpha=.", "bg_alpha=+", "bg_alpha=1.2.3", "bg_alpha=0,5",
        "border_alpha=x", "waiting_alpha=", "waiting_alpha=high",
        "bg_rgb=", "bg_rgb=1,2", "bg_rgb=1,2,3,4", "bg_rgb=1;2;3", "bg_rgb=a,b,c", "bg_rgb=1,,3", "bg_rgb=0.1,0.2,", "bg_rgb=,0.2,0.3", "bg_rgb=1 2 3",
        "border_rgb=1,2", "border_rgb=x",
        "border_px=0", "border_px=7", "border_px=-1", "border_px=", "border_px=2.5", "border_px=abc", "border_px=99999999999", "border_px=2px", "border_px=06", "border_px=10", "border_px=1,2", "border_px=/;", "border_px=:", "border_px=/",
        "waiting_dim=2", "log=x",
        "unknown=1", "enabledx=1", "enable=1", "bg=1", "novalue", "=1", "enabled 1",
    };
    for (const char* l : bad) {
        Config c; Config ref;
        KT_CHECK(!parseIniLine(c, l));
        KT_CHECK(memcmp(&c, &ref, sizeof c) == 0);
        if (memcmp(&c, &ref, sizeof c) != 0) printf("   changed by: %s\n", l);
    }
    // changed config + bad value keeps the changed value (not the default)
    Config c; parseIniLine(c, "bg_alpha=0.2"); parseIniLine(c, "bg_alpha=zzz"); KT_CHECK(near(c.bgAlpha, 0.2f));
    parseIniLine(c, "bg_rgb=0.5,0.5,0.5"); KT_CHECK(!parseIniLine(c, "bg_rgb=0.9,0.9,x")); KT_CHECK(near(c.bg[0], 0.5f) && near(c.bg[1], 0.5f) && near(c.bg[2], 0.5f));   // no partial write
    parseIniLine(c, "border_px=5"); parseIniLine(c, "border_px=9"); KT_CHECK_EQ(c.borderPx, 5);
    KT_CHECK(!parseIniLine(c, nullptr));
}

void testIniText() {
    Config c;
    parseIni(c, "[queue_hud]\r\n; green panel\r\nenabled = 0\r\nbg_rgb=0.0,1.0,0.0\r\nbg_alpha=0.3\r\nborder_px=3\r\nwaiting_alpha=0.5\r\nlog=0\r\n");
    KT_CHECK(!c.enabled && !c.log && c.bg[0] == 0.0f && c.bg[1] == 1.0f && near(c.bgAlpha, 0.3f) && c.borderPx == 3 && near(c.waitingAlpha, 0.5f));
    // \n only, no trailing newline, bad lines skipped without stopping the parse
    Config d;
    parseIni(d, "garbage line\nunknown=5\nborder_alpha=0.1\nborder_px=99\nborder_px=6\nwaiting_dim=0");
    KT_CHECK(near(d.borderAlpha, 0.1f) && d.borderPx == 6 && !d.waitingDim);
    // blank lines, CR-only blank, leading spaces, last line without newline
    Config e;
    parseIni(e, "\n\n\r\n   bg_alpha=0.9\n\n   log=0");
    KT_CHECK(near(e.bgAlpha, 0.9f) && !e.log);
    // empty / null text leave the config alone
    Config f; parseIni(f, ""); parseIni(f, nullptr); Config ref; KT_CHECK(memcmp(&f, &ref, sizeof f) == 0);
    // a line of 255 bytes is parsed, 256+ ignored
    char buf[400];
    memset(buf, ' ', sizeof buf);
    memcpy(buf, "border_px=4", 11);
    buf[255] = '\n'; buf[256] = 0;   // 255-byte line (padded with trailing spaces)
    Config g; parseIni(g, buf);
    KT_CHECK_EQ(g.borderPx, 4);
    memset(buf, ' ', sizeof buf);
    memcpy(buf, "border_px=5", 11);
    buf[256] = '\n'; buf[257] = 0;   // 256-byte line: ignored
    Config h; parseIni(h, buf);
    KT_CHECK_EQ(h.borderPx, 2);
    // line after the long one is still parsed
    memset(buf, ' ', sizeof buf);
    memcpy(buf, "border_px=5", 11);
    buf[300] = '\n'; memcpy(buf + 301, "border_px=3", 12);
    Config k; parseIni(k, buf);
    KT_CHECK_EQ(k.borderPx, 3);
}

// ---- 0.2.0 companion strips ----
bool overlaps(const Rect& a, const shui::Rect& b) {
    if (b.w <= 0 || b.h <= 0) return false;
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

void testStripConfig() {
    Config c;
    KT_CHECK(c.companions); KT_CHECK_EQ(c.stripIconPx, 0); KT_CHECK_EQ(c.stripSide, kSideLeft); KT_CHECK_EQ(c.stripMax, 4);
    KT_CHECK(parseIniLine(c, "companions = 0")); KT_CHECK(!c.companions);
    KT_CHECK(parseIniLine(c, "Companions=yes")); KT_CHECK(c.companions);
    KT_CHECK(!parseIniLine(c, "companions=2")); KT_CHECK(c.companions);
    KT_CHECK(parseIniLine(c, "strip_icon_px=24")); KT_CHECK_EQ(c.stripIconPx, 24);
    KT_CHECK(parseIniLine(c, "strip_icon_px=8")); KT_CHECK_EQ(c.stripIconPx, 8);
    KT_CHECK(parseIniLine(c, "strip_icon_px=96")); KT_CHECK_EQ(c.stripIconPx, 96);
    KT_CHECK(!parseIniLine(c, "strip_icon_px=7")); KT_CHECK_EQ(c.stripIconPx, 96);
    KT_CHECK(!parseIniLine(c, "strip_icon_px=97")); KT_CHECK_EQ(c.stripIconPx, 96);
    KT_CHECK(!parseIniLine(c, "strip_icon_px=100")); KT_CHECK(!parseIniLine(c, "strip_icon_px=-5")); KT_CHECK(!parseIniLine(c, "strip_icon_px=")); KT_CHECK(!parseIniLine(c, "strip_icon_px=1x"));
    KT_CHECK_EQ(c.stripIconPx, 96);
    KT_CHECK(parseIniLine(c, "strip_icon_px=0")); KT_CHECK_EQ(c.stripIconPx, 0);   // auto
    KT_CHECK(parseIniLine(c, "strip_side=below")); KT_CHECK_EQ(c.stripSide, kSideBelow);
    KT_CHECK(parseIniLine(c, "strip_side = ABOVE")); KT_CHECK_EQ(c.stripSide, kSideAbove);
    KT_CHECK(!parseIniLine(c, "strip_side=right")); KT_CHECK_EQ(c.stripSide, kSideAbove);
    KT_CHECK(parseIniLine(c, "strip_side=Left")); KT_CHECK_EQ(c.stripSide, kSideLeft);
    KT_CHECK(parseIniLine(c, "strip_max=1")); KT_CHECK_EQ(c.stripMax, 1);
    KT_CHECK(parseIniLine(c, "strip_max=4")); KT_CHECK_EQ(c.stripMax, 4);
    KT_CHECK(!parseIniLine(c, "strip_max=0")); KT_CHECK(!parseIniLine(c, "strip_max=5")); KT_CHECK(!parseIniLine(c, "strip_max=12")); KT_CHECK_EQ(c.stripMax, 4);
}

void testStripGeometry() {
    Config c;
    c.stripSide = kSideAbove;
    Rect out[4];
    const Rect card{ 3711, 1285, 85, 96 }, vit{ 3671, 1285, 47, 96 };
    KT_CHECK_EQ(stripIconSize(vit, c), 58);                    // 96 * 0.6 = 57.6 -> 58
    KT_CHECK_EQ(stripIconSize(Rect{ 0, 0, 47, 20 }, c), 12);   // 12 exactly
    KT_CHECK_EQ(stripIconSize(Rect{ 0, 0, 47, 19 }, c), 12);   // floor at 12 (11.4 -> 12)
    KT_CHECK_EQ(stripIconSize(Rect{ 0, 0, 47, 40 }, c), 24);
    KT_CHECK_EQ(stripIconSize(Rect{ 0, 0, 47, 41 }, c), 25);   // 24.6 -> 25
    KT_CHECK_EQ(stripIconSize(Rect{ 0, 0, 47, 24 }, c), 14);   // 14.4 rounds down (a round-half-up bias would give 15)
    c.stripIconPx = 20; KT_CHECK_EQ(stripIconSize(vit, c), 20); c.stripIconPx = 1; KT_CHECK_EQ(stripIconSize(vit, c), 1); c.stripIconPx = 0;
    // above: bottom edge 2px above the card top, left-aligned, 2px gap
    int n = stripRects(card, vit, 3, c, out);
    KT_CHECK_EQ(n, 3);
    for (int i = 0; i < 3; ++i) { KT_CHECK_EQ(out[i].w, 58); KT_CHECK_EQ(out[i].h, 58); KT_CHECK_EQ(out[i].y + out[i].h, card.y - 2); KT_CHECK_EQ(out[i].x, 3711 + i * 60); }
    // below: top edge 2px under the card bottom
    c.stripSide = kSideBelow;
    n = stripRects(card, vit, 2, c, out);
    KT_CHECK_EQ(n, 2);
    KT_CHECK_EQ(out[0].y, 1285 + 96 + 2); KT_CHECK_EQ(out[1].y, out[0].y); KT_CHECK_EQ(out[1].x, out[0].x + 60);
    c.stripSide = kSideAbove;
    // clamps: n to strip_max and to 4; n <= 0 and bad inputs give 0
    c.stripMax = 1; KT_CHECK_EQ(stripRects(card, vit, 3, c, out), 1);
    c.stripMax = 2; KT_CHECK_EQ(stripRects(card, vit, 4, c, out), 2);
    c.stripMax = 4; KT_CHECK_EQ(stripRects(card, vit, 9, c, out), 4);
    c.stripMax = 0; KT_CHECK_EQ(stripRects(card, vit, 3, c, out), 1);   // defensive floor
    c.stripMax = 9; KT_CHECK_EQ(stripRects(card, vit, 9, c, out), 4);   // defensive ceiling
    c.stripMax = 4;
    KT_CHECK_EQ(stripRects(card, vit, 0, c, out), 0); KT_CHECK_EQ(stripRects(card, vit, -1, c, out), 0);
    KT_CHECK_EQ(stripRects(card, vit, 1, c, nullptr), 0);
    KT_CHECK_EQ(stripRects(Rect{ 0, 0, 0, 96 }, vit, 1, c, out), 0); KT_CHECK_EQ(stripRects(Rect{ 0, 0, 85, 0 }, vit, 1, c, out), 0);
    KT_CHECK_EQ(stripRects(card, Rect{ 0, 0, 47, 0 }, 1, c, out), 0);
    KT_CHECK_EQ(stripRects(Rect{ 0, 0, 1, 96 }, vit, 1, c, out), 1); KT_CHECK_EQ(stripRects(Rect{ 0, 0, 85, 1 }, vit, 1, c, out), 1);   // 1px card is usable
    KT_CHECK_EQ(stripRects(card, Rect{ 0, 0, 47, 1 }, 1, c, out), 1);
    KT_CHECK_EQ(stripRects(card, vit, 1, c, out), 1);
    // an explicit icon size is used as is
    c.stripIconPx = 16;
    n = stripRects(card, vit, 4, c, out);
    KT_CHECK_EQ(n, 4); KT_CHECK_EQ(out[3].x, 3711 + 3 * 18); KT_CHECK_EQ(out[0].y, 1285 - 2 - 16);
    // screen clamp: a row that would run off the right edge slides left; screenW 0 = no clamp
    c.stripIconPx = 0;
    n = stripRects(card, vit, 4, c, out, 3840);
    KT_CHECK_EQ(n, 4); KT_CHECK_EQ(out[3].x + out[3].w, 3840); KT_CHECK_EQ(out[1].x - out[0].x, 60);
    n = stripRects(card, vit, 2, c, out, 3840);                     // 3711 + 2*58 + 2 = 3829 <= 3840: unchanged
    KT_CHECK_EQ(out[0].x, 3711);
    n = stripRects(card, vit, 4, c, out, 3840 + 0);
    KT_CHECK(out[3].x + out[3].w <= 3840);
    n = stripRects(Rect{ 10, 100, 85, 96 }, vit, 4, c, out, 50);   // a screen narrower than the row: pinned to x = 0, never negative
    KT_CHECK_EQ(out[0].x, 0);
    n = stripRects(card, vit, 4, c, out, 0);
    KT_CHECK_EQ(out[0].x, 3711);
    n = stripRects(card, vit, 1, c, out, 3768);                     // overflow of exactly 1px still shifts
    KT_CHECK_EQ(out[0].x, 3710);
    n = stripRects(card, vit, 1, c, out, 3769);                     // fits exactly: unchanged
    KT_CHECK_EQ(out[0].x, 3711);
    n = stripRects(card, vit, 1, c, out, 1);                        // screenW 1 is still a clamp
    KT_CHECK_EQ(out[0].x, 0);
    (void)n;
}

// Real small-card rects (doc 08 s5 / 27 R2: 3840x1600 vit (3671,1285|1181,47,96), card x 3711 w 85) plus other aspect/scales.
void testStripAvoidsSelfHeal() {
    struct Case { shui::Rect vit; Rect card; };
    const Case cases[] = {
        { { 3671, 1285, 47, 96 }, { 3711, 1285, 85, 96 } },    // 3840x1600 companion 2 (real)
        { { 3671, 1181, 47, 96 }, { 3711, 1181, 85, 96 } },    // companion 3 (real)
        { { 1727, 700, 73, 65 }, { 1767, 700, 57, 65 } },      // 1080p-ish (vit.w >= 70 so a button exists)
        { { 700, 482, 72, 36 }, { 740, 482, 36, 36 } },        // 800x600 units, no scaling
        { { 1000, 300, 100, 150 }, { 1040, 300, 120, 150 } },  // large scale
    };
    for (int side = kSideLeft; side <= kSideBelow; ++side) for (int nIcons = 1; nIcons <= 4; ++nIcons) for (int px : { 0, 12, 24 }) for (const Case& cs : cases) {
        Config c; c.stripSide = side; c.stripIconPx = px;
        Rect out[4];
        const int n = stripRects(cs.card, Rect{ cs.vit.x, cs.vit.y, cs.vit.w, cs.vit.h }, nIcons, c, out);
        KT_CHECK_EQ(n, nIcons);
        // buttonRect() refuses vit.w < 70 (the real small-card vit is 47 wide); its x/y depend only on vit.x/y/h, so widen w to get the geometry.
        const shui::Rect btn = shui::buttonRect(shui::Rect{ cs.vit.x, cs.vit.y, cs.vit.w < 70 ? 70 : cs.vit.w, cs.vit.h });
        KT_CHECK(btn.w > 0);                                   // the case must exercise a real button
        const shui::Rect ring = shui::ringRect(btn);
        for (int i = 0; i < n; ++i) {
            KT_CHECK(!overlaps(out[i], btn));
            KT_CHECK(!overlaps(out[i], ring));
            if (i) KT_CHECK(out[i].x >= out[i - 1].x + out[i - 1].w + 2);   // icons never overlap each other
            if (side == kSideLeft) {                                         // left of the button / ring, never on the bar or card
                KT_CHECK(out[i].x + out[i].w <= (btn.x < ring.x ? btn.x : ring.x) - 2);
                KT_CHECK(!overlaps(out[i], cs.vit));
                KT_CHECK(!overlaps(out[i], shui::Rect{ cs.card.x, cs.card.y, cs.card.w, cs.card.h }));
            }
        }
    }
}

// left (the default) on the live 3840x1600 layout (doc 08 probe 1): auto 38 px icons centred on the 96 px bar, ending 38 px left of
// vit.x; the two stacked companions (104 px apart) and the leader's bars never touch a strip.
void testStripLeftLive() {
    Config c;
    Rect out[4];
    const Rect vit2{ 3671, 1285, 47, 96 }, card2{ 3711, 1285, 85, 85 }, vit3{ 3671, 1181, 47, 96 }, card3{ 3711, 1181, 85, 85 };
    const shui::Rect leaderVit{ 3511, 1386, 103, 208 }, leaderCard{ 3591, 1394, 191, 192 }, leaderForce{ 3762, 1386, 103, 208 };
    KT_CHECK_EQ(stripIconSize(vit2, c), 38);                    // 96 * 0.4 = 38.4 -> 38
    KT_CHECK_EQ(selfHealReserve(vit2), 38);                    // gap 6 + button 24 + ring overhang 6 + 2
    KT_CHECK_EQ(stripRects(card2, vit2, 1, c, out), 1);
    KT_CHECK_EQ(out[0].x, 3671 - 38 - 38); KT_CHECK_EQ(out[0].y, 1285 + 29); KT_CHECK_EQ(out[0].w, 38);
    KT_CHECK_EQ(stripRects(card2, vit2, 4, c, out), 4);
    KT_CHECK_EQ(out[3].x + out[3].w, 3671 - 38); KT_CHECK_EQ(out[0].x, 3633 - 4 * 38 - 3 * 2);   // queue order left to right
    const Rect* vits[] = { &vit2, &vit3 }; const Rect* cards[] = { &card2, &card3 };
    for (int k = 0; k < 2; ++k) {
        KT_CHECK_EQ(stripRects(*cards[k], *vits[k], 4, c, out), 4);
        for (int i = 0; i < 4; ++i) {
            for (const Rect* r : { &vit2, &vit3, &card2, &card3 }) KT_CHECK(!overlaps(out[i], shui::Rect{ r->x, r->y, r->w, r->h }));
            KT_CHECK(!overlaps(out[i], leaderVit)); KT_CHECK(!overlaps(out[i], leaderCard)); KT_CHECK(!overlaps(out[i], leaderForce));
        }
    }
    // the reserve covers the real selfheal button + ring at every bar height, and wastes at most 3 px beyond kStripGap
    for (int h = 8; h <= 400; ++h) {
        const shui::Rect btn = shui::buttonRect(shui::Rect{ 1000, 100, 100, h });
        if (btn.w <= 0) continue;
        const shui::Rect ring = shui::ringRect(btn);
        const int need = 1000 - (btn.x < ring.x ? btn.x : ring.x);
        const int r = selfHealReserve(Rect{ 1000, 100, 47, h });
        KT_CHECK(r - 2 >= need);
        KT_CHECK(r - 2 <= need + 3);
    }
    KT_CHECK_EQ(stripRects(Rect{ 40, 0, 85, 85 }, Rect{ 10, 0, 47, 96 }, 2, c, out), 2);   // too close to the left edge: pinned to x = 0
    KT_CHECK_EQ(out[0].x, 0);
}

}  // namespace

int main() {
    testConstants();
    testKeys();
    testNodeIconValid();
    testEmptyAndNulls();
    testFirstEntry();
    testLaterEntries();
    testExpansion();
    testCaps();
    testWaitingAndShow();
    testConfigDefaultsAndStyle();
    testIniLine();
    testIniBad();
    testIniText();
    testStripConfig();
    testStripGeometry();
    testStripAvoidsSelfHeal();
    testStripLeftLive();
    return kt::Report("queue_hud_logic_test");
}
