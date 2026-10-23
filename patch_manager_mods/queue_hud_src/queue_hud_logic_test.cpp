// Tests for QueueHudLogic.h (queue-hud pure logic). Every rule in the header comment (D1..D5) and every boundary is pinned.
#include "QueueHudLogic.h"
#include "check.h"
#include "SelfHealUi.h"   // real neighbour geometry: shui::buttonRect / ringRect
#include "../hud_host_src/HhStrip.h"   // the real hud-host strip math (CS10 equivalence test)
#include <initializer_list>
#include <stdio.h>
#include <string.h>

using namespace qh;

namespace {

// The left-side footprint the in-game caller passes when hud-host is absent: shui::stripReserve (the fallback). The legacy 5-argument
// form below keeps the existing geometry tests unchanged; they now exercise the fallback path of the real caller.
int stripRects(Rect card, Rect vit, int n, const Config& c, Rect* out, int screenW = 0) {
    return qh::stripRects(card, vit, n, c, out, screenW, shui::stripReserve(shui::Rect{ vit.x, vit.y, vit.w, vit.h }));
}

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
    // show_in_combat (T1b): combat hides only when showInCombat is off; the other gates still apply with it on
    KT_CHECK(!shouldShow(true, true, false, 100, 1, false));   // default: combat hides
    KT_CHECK(shouldShow(true, true, false, 100, 1, true));     // showInCombat: combat no longer hides
    KT_CHECK(shouldShow(true, true, false, 249, 4, true));     // boundary: age < 250 still required
    KT_CHECK(!shouldShow(true, true, false, 250, 1, true));
    KT_CHECK(!shouldShow(true, true, false, 100, 0, true));    // slot gate still applies
    KT_CHECK(!shouldShow(false, true, false, 100, 1, true));   // enabled gate still applies
    KT_CHECK(!shouldShow(true, true, true, 100, 1, true));     // menu gate still applies
    KT_CHECK(shouldShow(true, false, false, 100, 1, false));   // out of combat: unchanged
    KT_CHECK(!shouldShow(true, false, true, 100, 1, true));    // menu gates out of combat too
}

void testConfigDefaultsAndStyle() {
    Config c;
    KT_CHECK(c.enabled && c.waitingDim && c.log && c.traceQueue);
    KT_CHECK(c.bg[0] == 0.05f && c.bg[1] == 0.25f && c.bg[2] == 0.28f && c.bgAlpha == 0.45f);
    KT_CHECK(c.border[0] == 0.10f && c.border[1] == 0.58f && c.border[2] == 0.62f && c.borderAlpha == 0.9f);
    KT_CHECK(c.waitingBg[0] == 0.03f && c.waitingBg[1] == 0.15f && c.waitingBg[2] == 0.18f);
    KT_CHECK_EQ(c.borderPx, 2); KT_CHECK(c.waitingAlpha == 0.6f);

    Style s = slotStyle(c, 0, false);
    KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f && s.borderA == 0.9f && s.borderPx == 2);
    KT_CHECK(s.bg[0] == 0.05f && s.bg[1] == 0.25f && s.bg[2] == 0.28f && s.border[0] == 0.10f && s.border[1] == 0.58f && s.border[2] == 0.62f);
    s = slotStyle(c, 0, true);   // dimmed
    KT_CHECK(s.iconA == 0.6f && s.bgA == 0.45f * 0.6f && s.borderA == 0.9f * 0.6f);
    KT_CHECK(s.bg[0] == 0.03f && s.bg[1] == 0.15f && s.bg[2] == 0.18f);   // bg changed to waitingBg
    for (int i = 1; i < 4; ++i) { s = slotStyle(c, i, true); KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f && s.borderA == 0.9f); }
    s = slotStyle(c, -1, true); KT_CHECK(s.iconA == 1.0f);
    c.waitingDim = false;
    s = slotStyle(c, 0, true); KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f && s.borderA == 0.9f);
    c.waitingDim = true; c.waitingAlpha = 0.5f; c.bgAlpha = 0.8f; c.borderAlpha = 0.4f; c.borderPx = 5; c.bg[0] = 0.25f; c.border[2] = 0.75f;
    s = slotStyle(c, 0, true);
    KT_CHECK(s.iconA == 0.5f && s.bgA == 0.4f && s.borderA == 0.2f && s.borderPx == 5 && s.border[2] == 0.75f);
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
    // waiting_dim_rgb sets waitingBg only (the slot bg keeps its value)
    {
        Config w; const float bg0 = w.bg[0], bg1 = w.bg[1], bg2 = w.bg[2];
        KT_CHECK(parseIniLine(w, "waiting_dim_rgb = 0.25,0.5,0.75"));
        KT_CHECK(w.waitingBg[0] == 0.25f && w.waitingBg[1] == 0.5f && w.waitingBg[2] == 0.75f);
        KT_CHECK(w.bg[0] == bg0 && w.bg[1] == bg1 && w.bg[2] == bg2);
        KT_CHECK(parseIniLine(w, "waiting_dim_rgb=2,-1,0.5")); KT_CHECK(w.waitingBg[0] == 1.0f && w.waitingBg[1] == 0.0f && w.waitingBg[2] == 0.5f);   // clamped per component
    }
    // border_px
    for (int px = 1; px <= 6; ++px) { char l[32]; snprintf(l, sizeof l, "border_px=%d", px); KT_CHECK(parseIniLine(c, l)); KT_CHECK_EQ(c.borderPx, px); }
    // independence: setting one key never touches another
    Config d; parseIniLine(d, "border_px=4");
    Config e0;
    KT_CHECK(d.enabled == e0.enabled && d.bgAlpha == e0.bgAlpha && d.borderAlpha == e0.borderAlpha && d.waitingAlpha == e0.waitingAlpha && d.bg[0] == e0.bg[0] && d.border[0] == e0.border[0] && d.waitingDim == e0.waitingDim && d.log == e0.log);
}

// Field-wise Config compare over every member (memcmp would read indeterminate padding bytes).
// A new Config member needs an edit here and in testConfigEqualSeesEveryField below.
bool configEqual(const Config& a, const Config& b) {
    auto s3 = [](const float* x, const float* y) { return x[0] == y[0] && x[1] == y[1] && x[2] == y[2]; };
    return a.enabled == b.enabled && a.bgAlpha == b.bgAlpha && a.borderAlpha == b.borderAlpha && a.borderPx == b.borderPx && a.waitingDim == b.waitingDim && a.waitingAlpha == b.waitingAlpha && a.log == b.log && a.traceQueue == b.traceQueue && a.companions == b.companions && a.stripIconPx == b.stripIconPx && a.stripSide == b.stripSide && a.stripMax == b.stripMax
        && a.kit == b.kit && a.frameAlpha == b.frameAlpha && a.cancel == b.cancel && a.pauseReinject == b.pauseReinject && a.dispatchWaitFlags == b.dispatchWaitFlags && a.traceClicks == b.traceClicks && a.pauseEatenWait == b.pauseEatenWait && a.handoffOnCombat == b.handoffOnCombat && a.showInCombat == b.showInCombat && a.handoffClearFirst == b.handoffClearFirst
        && a.handoffKeepNew == b.handoffKeepNew && a.handoffKeepUnpaused == b.handoffKeepUnpaused && a.handoffDetachHeld == b.handoffDetachHeld && a.handoffResendHeld == b.handoffResendHeld && a.spellReplayPosSeen == b.spellReplayPosSeen && a.hideBannerSoft == b.hideBannerSoft && a.hideRedbarSoft == b.hideRedbarSoft && a.traceMode == b.traceMode && a.modeCap == b.modeCap && a.waitingIconDim == b.waitingIconDim && a.stripHeadPct == b.stripHeadPct && s3(a.bg, b.bg) && s3(a.border, b.border) && s3(a.waitingBg, b.waitingBg);
}

// Positive control: configEqual must see a change in every member, else the checks that replaced memcmp prove nothing.
void testConfigEqualSeesEveryField() {
    Config ref;
    KT_CHECK(configEqual(ref, Config()));
#define CFG_FLIP(F) do { Config m; m.F = !m.F; KT_CHECK(!configEqual(m, ref)); } while (0)
    CFG_FLIP(enabled); CFG_FLIP(bgAlpha); CFG_FLIP(borderAlpha); CFG_FLIP(borderPx); CFG_FLIP(waitingDim); CFG_FLIP(waitingAlpha); CFG_FLIP(log); CFG_FLIP(traceQueue); CFG_FLIP(companions); CFG_FLIP(stripIconPx);
    CFG_FLIP(stripSide); CFG_FLIP(stripMax); CFG_FLIP(kit); CFG_FLIP(frameAlpha); CFG_FLIP(cancel); CFG_FLIP(pauseReinject); CFG_FLIP(dispatchWaitFlags); CFG_FLIP(traceClicks); CFG_FLIP(pauseEatenWait); CFG_FLIP(handoffOnCombat);
    CFG_FLIP(showInCombat); CFG_FLIP(handoffClearFirst); CFG_FLIP(handoffKeepNew); CFG_FLIP(handoffKeepUnpaused); CFG_FLIP(handoffDetachHeld); CFG_FLIP(handoffResendHeld); CFG_FLIP(spellReplayPosSeen); CFG_FLIP(hideBannerSoft); CFG_FLIP(hideRedbarSoft);
    CFG_FLIP(traceMode); CFG_FLIP(modeCap); CFG_FLIP(waitingIconDim); CFG_FLIP(stripHeadPct);
#undef CFG_FLIP
    for (int i = 0; i < 3; ++i) {
        Config m; m.bg[i] += 1.0f;        KT_CHECK(!configEqual(m, ref));
        Config n; n.border[i] += 1.0f;    KT_CHECK(!configEqual(n, ref));
        Config o; o.waitingBg[i] += 1.0f; KT_CHECK(!configEqual(o, ref));
    }
}

void testIniBad() {
    // each bad line returns false and leaves the whole Config unchanged
    const char* bad[] = {
        "enabled=maybe", "enabled=", "enabled=2", "enabled=truee", "enabled=tru", "enabled=1 1",
        "bg_alpha=abc", "bg_alpha=", "bg_alpha=0.5x", "bg_alpha=1e1", "bg_alpha=inf", "bg_alpha=nan", "bg_alpha=0x1", "bg_alpha=--1", "bg_alpha=.", "bg_alpha=+", "bg_alpha=1.2.3", "bg_alpha=0,5",
        "border_alpha=x", "waiting_alpha=", "waiting_alpha=high",
        "bg_rgb=", "bg_rgb=1,2", "bg_rgb=1,2,3,4", "bg_rgb=1;2;3", "bg_rgb=a,b,c", "bg_rgb=1,,3", "bg_rgb=0.1,0.2,", "bg_rgb=,0.2,0.3", "bg_rgb=1 2 3",
        "border_rgb=1,2", "border_rgb=x",
        "waiting_dim_rgb=", "waiting_dim_rgb=0.25,0.5", "waiting_dim_rgb=0.25,oops,0.75", "waiting_dim_rgb=1,2,3,4",
        "border_px=0", "border_px=7", "border_px=-1", "border_px=", "border_px=2.5", "border_px=abc", "border_px=99999999999", "border_px=2px", "border_px=06", "border_px=10", "border_px=1,2", "border_px=/;", "border_px=:", "border_px=/",
        "waiting_dim=2", "log=x", "trace_queue=x", "pause_reinject=x", "handoff_on_combat=", "show_in_combat=2",
        "handoff_clear_first=2", "handoff_clear_first=",
        "trace_mode=2", "trace_mode=", "mode_cap=0", "mode_cap=abc", "mode_cap=100001", "mode_cap=-1", "mode_cap=1000000", "mode_cap=",
        "unknown=1", "enabledx=1", "enable=1", "bg=1", "novalue", "=1", "enabled 1",
    };
    for (const char* l : bad) {
        Config c; Config ref;
        KT_CHECK(!parseIniLine(c, l));
        KT_CHECK(configEqual(c, ref));
        if (!configEqual(c, ref)) printf("   changed by: %s\n", l);
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
    Config f; parseIni(f, ""); parseIni(f, nullptr); Config ref; KT_CHECK(configEqual(f, ref));
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
    // left side (doc 33): auto size = vit.h * 0.60f clamped [48, 72]
    { Config l; l.stripSide = kSideLeft;
      KT_CHECK_EQ(stripIconSize(Rect{ 3671, 1285, 47, 96 }, l), 58);   // live small card: 96 * 0.6 = 57.6 -> 58
      KT_CHECK_EQ(stripIconSize(Rect{ 0, 0, 47, 96 }, l), 58);         // x does not matter
      KT_CHECK_EQ(stripIconSize(Rect{ 100, 0, 47, 36 }, l), 48);       // floor 48 (36 * 0.6 = 21 -> 48)
      KT_CHECK_EQ(stripIconSize(Rect{ 100, 0, 47, 80 }, l), 48);       // 80 * 0.6 = 48
      KT_CHECK_EQ(stripIconSize(Rect{ 100, 0, 47, 400 }, l), 72);      // cap 72 (400 * 0.6 = 240 -> 72)
      KT_CHECK_EQ(stripIconSize(Rect{ 100, 0, 103, 208 }, l), 72);     // cap 72 (208 * 0.6 = 124 -> 72)
      l.stripIconPx = 30; KT_CHECK_EQ(stripIconSize(Rect{ 3671, 1285, 47, 96 }, l), 30);   // strip_icon_px still overrides
      KT_CHECK_EQ(stripIconSize(Rect{ 3671, 1285, 47, 96 }, c), 58); }                      // above / below keep 60% of h
    // above: bottom edge 2px above the card top, left-aligned, 2px gap
    int n = stripRects(card, vit, 3, c, out);
    KT_CHECK_EQ(n, 3);
    for (int i = 0; i < 3; ++i) { KT_CHECK_EQ(out[i].w, 58); KT_CHECK_EQ(out[i].h, 58); KT_CHECK_EQ(out[i].y + out[i].h, card.y - 2); KT_CHECK_EQ(out[i].x, 3711 + (2 - i) * 60); }   // head rightmost
    // below: top edge 2px under the card bottom
    c.stripSide = kSideBelow;
    n = stripRects(card, vit, 2, c, out);
    KT_CHECK_EQ(n, 2);
    KT_CHECK_EQ(out[0].y, 1285 + 96 + 2); KT_CHECK_EQ(out[1].y, out[0].y); KT_CHECK_EQ(out[0].x, out[1].x + 60);
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
    KT_CHECK_EQ(n, 4); KT_CHECK_EQ(out[0].x, 3711 + 3 * 18); KT_CHECK_EQ(out[0].y, 1285 - 2 - 16);
    // screen clamp: a row that would run off the right edge slides left; screenW 0 = no clamp
    c.stripIconPx = 0;
    n = stripRects(card, vit, 4, c, out, 3840);
    KT_CHECK_EQ(n, 4); KT_CHECK_EQ(out[0].x + out[0].w, 3840); KT_CHECK_EQ(out[0].x - out[1].x, 60);
    n = stripRects(card, vit, 2, c, out, 3840);                     // 3711 + 2*58 + 2 = 3829 <= 3840: unchanged
    KT_CHECK_EQ(out[1].x, 3711);
    n = stripRects(card, vit, 4, c, out, 3840 + 0);
    KT_CHECK(out[0].x + out[0].w <= 3840);
    n = stripRects(Rect{ 10, 100, 85, 96 }, vit, 4, c, out, 50);   // a screen narrower than the row: pinned to x = 0, never negative
    KT_CHECK_EQ(out[3].x, 0);
    n = stripRects(card, vit, 4, c, out, 0);
    KT_CHECK_EQ(out[3].x, 3711);
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
        // buttonRect() refuses vit.w >= 70 (leader); the real small-card vit is 47 wide and its x/y depend only on vit.x/y/h, so use w = 47.
        const shui::Rect btn = shui::buttonRect(shui::Rect{ cs.vit.x, cs.vit.y, 47, cs.vit.h });
        KT_CHECK(btn.w > 0);                                   // the case must exercise a real button
        const shui::Rect ring = shui::ringRect(btn);
        for (int i = 0; i < n; ++i) {
            KT_CHECK(!overlaps(out[i], btn));
            KT_CHECK(!overlaps(out[i], ring));
            if (i) KT_CHECK(out[i - 1].x >= out[i].x + out[i].w + 2);   // icons never overlap each other (index i - 1 is to the right of i)
            if (side == kSideLeft) {                                         // left of the button / ring, never on the bar or card
                KT_CHECK(out[i].x + out[i].w <= (btn.x < ring.x ? btn.x : ring.x) - 2);
                KT_CHECK(!overlaps(out[i], cs.vit));
                KT_CHECK(!overlaps(out[i], shui::Rect{ cs.card.x, cs.card.y, cs.card.w, cs.card.h }));
            }
        }
    }
}

// left (the default) on the live 3840x1600 layout (doc 08 probe 1): auto 58 px icons centred on the 96 px bar, ending 38 px left of
// vit.x; the two stacked companions (104 px apart) and the leader's bars never touch a strip.
void testStripLeftLive() {
    Config c;
    Rect out[4];
    const Rect vit2{ 3671, 1285, 47, 96 }, card2{ 3711, 1285, 85, 85 }, vit3{ 3671, 1181, 47, 96 }, card3{ 3711, 1181, 85, 85 };
    const shui::Rect leaderVit{ 3511, 1386, 103, 208 }, leaderCard{ 3591, 1394, 191, 192 }, leaderForce{ 3762, 1386, 103, 208 };
    KT_CHECK_EQ(stripIconSize(vit2, c), 58);                    // auto: vit.h * 0.60 = 58 (was 38)
    KT_CHECK_EQ(stripIconSize(vit2, c), autoIconPx(vit2.h));
    KT_CHECK_EQ(shui::stripReserve(shui::Rect{ vit2.x, vit2.y, vit2.w, vit2.h }), 66);                    // gap 6 + disc 58 (= queue icon) + 2 (H3)
    KT_CHECK_EQ(stripRects(card2, vit2, 1, c, out), 1);
    KT_CHECK_EQ(out[0].x, 3671 - 66 - 58); KT_CHECK_EQ(out[0].y, 1285 + 19); KT_CHECK_EQ(out[0].w, 58);   // y centres 58 px on the 96 px bar (1285 + (96-58)/2 = 1285 + 19)
    KT_CHECK_EQ(stripRects(card2, vit2, 4, c, out), 4);
    KT_CHECK_EQ(out[0].x + out[0].w, 3671 - 66); KT_CHECK_EQ(out[3].x, 3605 - 4 * 58 - 3 * 2);   // head rightmost, next to the portrait
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
        const shui::Rect btn = shui::buttonRect(shui::Rect{ 1000, 100, 47, h });
        if (btn.w <= 0) continue;
        const shui::Rect ring = shui::ringRect(btn);
        const int need = 1000 - (btn.x < ring.x ? btn.x : ring.x);
        const int r = shui::stripReserve(shui::Rect{ 1000, 100, 47, h });
        KT_CHECK(r - 2 >= need);
        KT_CHECK(r - 2 <= need + 3);
    }
    KT_CHECK_EQ(stripRects(Rect{ 40, 0, 85, 85 }, Rect{ 10, 0, 47, 96 }, 2, c, out), 2);   // too close to the left edge: pinned to x = 0
    KT_CHECK_EQ(out[1].x, 0);
}


// ---- CS10: hud-host strip slots. With selfheal registered at order 0 (width = shui::stripReserve) and queue-hud at order 1 (width =
// the row), the row lands exactly where the old hard-coded formula put it; without selfheal the row moves flush to the vit bar.
int oldSelfHealReserve(Rect vit) {   // the fallback reserve formula (H3: disc-based, was button + ring overhang): gap + disc/ring side + kStripGap
    int gap = (vit.h + 17) / 18;
    if (gap < 2) gap = 2;
    return gap + shui::ringSideFor(shui::btnSizeForDisc(shui::discDiameter(vit.h))) + kStripGap;
}

// What QueueHud.cpp does on the left side: register, then reserve = vit.x - slot.x - rowW.
int slotReserve(hh::Strip& st, Rect vit, int n, const Config& c) {
    hh::Box r{};
    const int rowW = stripRowWidth(vit, n, c);
    if (!st.slot(1, hh::Box{ vit.x, vit.y, vit.w, vit.h }, rowW, stripIconSize(vit, c), &r)) return -1;
    return vit.x - r.x - rowW;
}

void testStripHudHostSlots() {
    Config c;
    Rect oldOut[4], newOut[4];
    const Rect vit{ 3671, 1285, 47, 96 }, card{ 3711, 1285, 85, 85 };
    hh::Strip st;
    hh::Box sh{};
    KT_CHECK_EQ(st.slot(0, hh::Box{ vit.x, vit.y, vit.w, vit.h }, shui::stripReserve(shui::Rect{ vit.x, vit.y, vit.w, vit.h }), 24, &sh), 1);   // selfheal, order 0
    KT_CHECK_EQ(oldSelfHealReserve(vit), 66);
    KT_CHECK_EQ(slotReserve(st, vit, 3, c), 66);
    for (int n = 1; n <= 4; ++n) {
        KT_CHECK_EQ(qh::stripRects(card, vit, n, c, oldOut, 0, oldSelfHealReserve(vit)), n);
        KT_CHECK_EQ(qh::stripRects(card, vit, n, c, newOut, 0, slotReserve(st, vit, n, c)), n);
        for (int i = 0; i < n; ++i) { KT_CHECK_EQ(newOut[i].x, oldOut[i].x); KT_CHECK_EQ(newOut[i].y, oldOut[i].y); KT_CHECK_EQ(newOut[i].w, oldOut[i].w); }
        KT_CHECK_EQ(newOut[n - 1].x, vit.x - 66 - (n * 58 + (n - 1) * 2));   // the literal live value: 3605 - rowW
    }
    // every bar height: the slot path is the old formula
    for (int h = 8; h <= 400; ++h) {
        const Rect v{ 1500, 300, 47, h }, cd{ 1540, 300, 85, 85 };
        hh::Strip s2;
        s2.slot(0, hh::Box{ v.x, v.y, v.w, v.h }, shui::stripReserve(shui::Rect{ v.x, v.y, v.w, v.h }), 24, &sh);
        Rect a[4], b[4];
        const int na = qh::stripRects(cd, v, 3, c, a, 3840, oldSelfHealReserve(v)), nb = qh::stripRects(cd, v, 3, c, b, 3840, slotReserve(s2, v, 3, c));
        KT_CHECK_EQ(na, nb);
        for (int i = 0; i < na; ++i) KT_CHECK_EQ(a[i].x, b[i].x);
    }
    // selfheal not registered (mod absent / not yet seen): the row sits flush against the vit bar - the reason the caller keeps the fallback
    hh::Strip alone;
    KT_CHECK_EQ(slotReserve(alone, vit, 2, c), 0);
    // stripRowWidth mirrors stripRects: clamped to strip_max / kMaxSlots, 0 for n <= 0
    KT_CHECK_EQ(stripRowWidth(vit, 0, c), 0); KT_CHECK_EQ(stripRowWidth(vit, 9, c), 4 * 58 + 3 * 2);
    c.stripMax = 2; KT_CHECK_EQ(stripRowWidth(vit, 4, c), 2 * 58 + 2);
}

// ---- 0.3.0: kit frame + Cancel ----
bool rectEq(Rect r, int x, int y, int w, int h) { return r.x == x && r.y == y && r.w == w && r.h == h; }
bool ov(Rect a, Rect b) { return a.w > 0 && b.w > 0 && a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h; }

void testKitConfig() {
    Config c;
    KT_CHECK(c.kit && c.cancel && c.frameAlpha == 0.9f);
    KT_CHECK(slotStyle(c, 0, false).frameA == 0.9f);
    KT_CHECK(slotStyle(c, 0, true).frameA == 0.9f * 0.6f);          // the dimmed head slot dims its frame too
    KT_CHECK(slotStyle(c, 1, true).frameA == 0.9f);
    c.waitingDim = false; KT_CHECK(slotStyle(c, 0, true).frameA == 0.9f);
    Config d;
    KT_CHECK(parseIniLine(d, "kit = 1") && d.kit);
    KT_CHECK(parseIniLine(d, "cancel=off") && !d.cancel);
    KT_CHECK(parseIniLine(d, "frame_alpha = 0.5") && d.frameAlpha == 0.5f);
    KT_CHECK(parseIniLine(d, "frame_alpha = 7") && d.frameAlpha == 1.0f);   // clamped
    KT_CHECK(!parseIniLine(d, "frame_alpha = x") && d.frameAlpha == 1.0f);
    KT_CHECK(!parseIniLine(d, "kit = maybe") && d.kit);
    Config e; parseIni(e, "kit=0\r\nframe_alpha=0.25\r\ncancel=0\r\nborder_px=3\r\n");
    KT_CHECK(!e.kit && e.frameAlpha == 0.25f && !e.cancel && e.borderPx == 3);
}

// ---- T1b: pause_reinject / handoff_on_combat / show_in_combat (all default on; parsed like the other bool keys) ----
void testPauseHandoffShowConfig() {
    Config c;
    KT_CHECK(c.pauseReinject && c.handoffOnCombat && c.showInCombat && c.handoffClearFirst);     // defaults
    KT_CHECK(parseIniLine(c, "pause_reinject=0") && !c.pauseReinject);
    KT_CHECK(parseIniLine(c, "pause_reinject=1") && c.pauseReinject);
    KT_CHECK(parseIniLine(c, "Handoff_On_Combat = off") && !c.handoffOnCombat);
    KT_CHECK(parseIniLine(c, "handoff_on_combat=yes") && c.handoffOnCombat);
    KT_CHECK(parseIniLine(c, "show_in_combat = 0") && !c.showInCombat);
    KT_CHECK(parseIniLine(c, "show_in_combat=true") && c.showInCombat);
    // bad values are rejected and keep the previous value
    KT_CHECK(!parseIniLine(c, "pause_reinject=x") && c.pauseReinject);
    KT_CHECK(!parseIniLine(c, "handoff_on_combat=") && c.handoffOnCombat);
    KT_CHECK(!parseIniLine(c, "show_in_combat=2") && c.showInCombat);
    // each key sets only its own field
    Config d;
    KT_CHECK(parseIniLine(d, "show_in_combat=0") && !d.showInCombat && d.pauseReinject && d.handoffOnCombat && d.handoffClearFirst);
    Config e;
    KT_CHECK(parseIniLine(e, "pause_reinject=0") && !e.pauseReinject && e.handoffOnCombat && e.showInCombat && e.handoffClearFirst);
    // absent keys keep the defaults (true) in a whole-file parse; present keys apply
    Config f; parseIni(f, "enabled=1\r\nborder_px=3\r\n");
    KT_CHECK(f.pauseReinject && f.handoffOnCombat && f.showInCombat && f.handoffClearFirst);
    Config g; parseIni(g, "show_in_combat=0\r\npause_reinject=0\r\nhandoff_on_combat=0\r\nhandoff_clear_first=0\r\n");
    KT_CHECK(!g.showInCombat && !g.pauseReinject && !g.handoffOnCombat && !g.handoffClearFirst);
}

// ---- handoff_clear_first: default on, parsed like the other bool keys, invalid values rejected, other fields untouched ----
void testHandoffClearFirstIni() {
    Config c;
    KT_CHECK(c.handoffClearFirst);                                                  // default on
    KT_CHECK(parseIniLine(c, "handoff_clear_first=0") && !c.handoffClearFirst);     // 0 = off
    KT_CHECK(parseIniLine(c, "handoff_clear_first=1") && c.handoffClearFirst);      // 1 = on
    KT_CHECK(parseIniLine(c, "Handoff_Clear_First = off") && !c.handoffClearFirst); // case-insensitive key, spaces trimmed
    KT_CHECK(parseIniLine(c, "handoff_clear_first=yes") && c.handoffClearFirst);
    KT_CHECK(parseIniLine(c, "handoff_clear_first=no") && !c.handoffClearFirst);
    KT_CHECK(parseIniLine(c, "handoff_clear_first=true") && c.handoffClearFirst);
    KT_CHECK(parseIniLine(c, "handoff_clear_first=false") && !c.handoffClearFirst);
    // invalid values are rejected and keep the previous value (on and off)
    KT_CHECK(parseIniLine(c, "handoff_clear_first=on") && c.handoffClearFirst);
    KT_CHECK(!parseIniLine(c, "handoff_clear_first=2") && c.handoffClearFirst);
    KT_CHECK(!parseIniLine(c, "handoff_clear_first=") && c.handoffClearFirst);
    KT_CHECK(parseIniLine(c, "handoff_clear_first=0") && !c.handoffClearFirst);
    KT_CHECK(!parseIniLine(c, "handoff_clear_first=2") && !c.handoffClearFirst);
    KT_CHECK(!parseIniLine(c, "handoff_clear_first=") && !c.handoffClearFirst);
    // a rejected line changes no field at all
    Config ref;
    Config b; b.handoffClearFirst = false;
    KT_CHECK(!parseIniLine(b, "handoff_clear_first=x") && !b.handoffClearFirst);
    KT_CHECK(b.enabled == ref.enabled && b.showInCombat == ref.showInCombat && b.handoffOnCombat == ref.handoffOnCombat && b.pauseReinject == ref.pauseReinject);
    // the key sets only its own field (whole-file parse)
    Config d; parseIni(d, "handoff_clear_first=0\r\n");
    KT_CHECK(!d.handoffClearFirst);
    KT_CHECK(d.enabled == ref.enabled && d.showInCombat == ref.showInCombat && d.handoffOnCombat == ref.handoffOnCombat && d.pauseReinject == ref.pauseReinject);
    // absent key keeps the default (true) in a whole-file parse; other keys still apply
    Config f; parseIni(f, "enabled=1\r\nshow_in_combat=0\r\n");
    KT_CHECK(f.handoffClearFirst && !f.showInCombat);
}

// ---- T1 panel hold / active slot / clear-before-handoff pure rules ----
void testPanelHoldAndSlotRules() {
    KT_CHECK(kPanelHoldMs == 400u);
    // panelHold: held while 0 <= elapsed < 400 (399 held, 400 released)
    KT_CHECK(panelHold(1000, 1000, false, false));       // 0 elapsed
    KT_CHECK(panelHold(1000, 1399, false, false));       // 399 elapsed
    KT_CHECK(!panelHold(1000, 1400, false, false));      // 400 elapsed: released
    KT_CHECK(!panelHold(1000, 1401, false, false));      // 401 elapsed
    KT_CHECK(!panelHold(1000, 999, false, false));       // now before lastGood (wrap): not held
    // lastGoodTick == 0 means no good read yet: never held
    KT_CHECK(!panelHold(0, 0, false, false));
    KT_CHECK(!panelHold(0, 100, false, false));
    // a menu or a combat hide cancels the hold even inside the window
    KT_CHECK(!panelHold(1000, 1399, true, false));
    KT_CHECK(!panelHold(1000, 1399, false, true));
    KT_CHECK(!panelHold(1000, 1399, true, true));
    // activeNeedsSlot: all four combos
    KT_CHECK(activeNeedsSlot(true, false));
    KT_CHECK(!activeNeedsSlot(true, true));
    KT_CHECK(!activeNeedsSlot(false, false));
    KT_CHECK(!activeNeedsSlot(false, true));
    // clearBeforeHandoff: only when the config is on and nothing was sent yet
    KT_CHECK(clearBeforeHandoff(true, 0));
    KT_CHECK(!clearBeforeHandoff(true, 1));
    KT_CHECK(!clearBeforeHandoff(true, 2));
    KT_CHECK(!clearBeforeHandoff(false, 0));
    KT_CHECK(!clearBeforeHandoff(false, 1));
    // handoff_keep_new (QL10): keepNew suppresses the clear and is logged on the first send only
    KT_CHECK(!clearBeforeHandoff(true, 0, true));
    KT_CHECK(clearBeforeHandoff(true, 0, false));
    KT_CHECK(keepNewAtHandoff(true, 0));
    KT_CHECK(!keepNewAtHandoff(true, 1));
    KT_CHECK(!keepNewAtHandoff(false, 0));
    KT_CHECK(clearBeforeHandoff(true, 0, false) && !clearBeforeHandoff(true, 0, true));
    Config k;
    KT_CHECK(k.handoffKeepNew);                                                     // default on
    KT_CHECK(parseIniLine(k, "handoff_keep_new=0") && !k.handoffKeepNew);
    KT_CHECK(parseIniLine(k, "handoff_keep_new=1") && k.handoffKeepNew);
    KT_CHECK(!parseIniLine(k, "handoff_keep_new=maybe") && k.handoffKeepNew);       // invalid: rejected, kept
    char kl[120];
    fmtHandoffKeep(kl, sizeof kl, 0, 7, 5, 2);
    KT_CHECK(strcmp(kl, "HANDOFF_KEEP m=0 cmd=7 ne=5 round=2") == 0);
    // roundPartition: the player's new click (foreign) ends up last, everything else keeps its order
    {
        int o[4];
        const uint32_t foreign[1] = {0xA1};
        const uint32_t p1[2] = {0xA1, 0xB1};                        // [attack, ours]
        KT_CHECK(roundPartition(p1, 2, foreign, 1, o) == 2 && o[0] == 1 && o[1] == 0);
        const uint32_t p2[3] = {0xA1, 0xB1, 0xB2};                  // [attack, ours, ours]
        KT_CHECK(roundPartition(p2, 3, foreign, 1, o) == 3 && o[0] == 1 && o[1] == 2 && o[2] == 0);
        const uint32_t p3[3] = {0xB1, 0xA1, 0xB2};                  // [ours, attack, ours]
        KT_CHECK(roundPartition(p3, 3, foreign, 1, o) == 2 && o[0] == 0 && o[1] == 2 && o[2] == 1);
        const uint32_t p4[3] = {0xB1, 0xB2, 0xA1};                  // already ordered
        KT_CHECK(roundPartition(p4, 3, foreign, 1, o) == 0 && o[0] == 0 && o[1] == 1 && o[2] == 2);
        const uint32_t p5[2] = {0xB1, 0xB2};                        // no foreign present
        KT_CHECK(roundPartition(p5, 2, foreign, 1, o) == 0);
        KT_CHECK(roundPartition(p5, 0, foreign, 1, o) == 0);
        const uint32_t f2[2] = {0xA1, 0xA2};                        // two foreign nodes keep their relative order
        const uint32_t p6[4] = {0xA1, 0xA2, 0xB1, 0xB2};
        KT_CHECK(roundPartition(p6, 4, f2, 2, o) == 4 && o[0] == 2 && o[1] == 3 && o[2] == 0 && o[3] == 1);
        // reattachReady: only when nothing of ours is pending anywhere and the last send is old enough
        KT_CHECK(reattachReady(false, false, 0, kReattachQuietMs, 0, 1));
        KT_CHECK(!reattachReady(true, false, 0, 1000, 0, 1));                    // hand-off / buffer busy
        KT_CHECK(!reattachReady(false, true, 0, 1000, 0, 1));                    // item/spell in the engine queue
        KT_CHECK(!reattachReady(false, false, 1, 1000, 0, 1));                   // our round node still pending
        KT_CHECK(!reattachReady(false, false, 0, kReattachQuietMs - 1, 0, 1));   // just sent: the server has no node yet
        KT_CHECK(!reattachReady(false, false, 0, 1000, 4, 1));                   // list full
        KT_CHECK(reattachReady(false, false, 0, 1000, 3, 1));
        KT_CHECK(!reattachReady(false, false, 0, 1000, 0, 0));                   // nothing detached
        KT_CHECK(keepPathAllowed(true, true, false));              // paused: always
        KT_CHECK(keepPathAllowed(true, false, true));              // unpaused: only with handoff_keep_unpaused
        KT_CHECK(!keepPathAllowed(true, false, false));
        KT_CHECK(!keepPathAllowed(false, true, true));             // handoff_keep_new off wins
        Config ku;
        KT_CHECK(ku.handoffKeepUnpaused);
        KT_CHECK(parseIniLine(ku, "handoff_keep_unpaused=0") && !ku.handoffKeepUnpaused);
        KT_CHECK(parseIniLine(ku, "handoff_keep_unpaused=1") && ku.handoffKeepUnpaused);
        Config dc;
        KT_CHECK(dc.handoffDetachHeld);
        KT_CHECK(parseIniLine(dc, "handoff_detach_held=0") && !dc.handoffDetachHeld);
        KT_CHECK(parseIniLine(dc, "handoff_detach_held=1") && dc.handoffDetachHeld);
        char dl[120];
        fmtRoundDetach(dl, sizeof dl, 0, 1, "detach", true, "held_head");
        KT_CHECK(strcmp(dl, "ROUND_DETACH m=0 n=1 act=detach ok=1 why=held_head") == 0);
        char rr[80];
        fmtRoundRot(rr, sizeof rr, 0, 3, 2, true);
        KT_CHECK(strcmp(rr, "ROUND_ROT m=0 n=3 moved=2 ok=1") == 0);
    }
}

// ---- T1 HANDOFF_CLEAR / ROUND formatter goldens, clamp and tiny-buffer behaviour ----
void testHandoffClearAndRoundFormat() {
    char out[kLogLineMax];
    // HANDOFF_CLEAR m=<k> cmd=<n> ne=<n> round=<n>
    fmtHandoffClear(out, sizeof out, 0, 4, 3, 2);
    KT_CHECK_EQ(out, "HANDOFF_CLEAR m=0 cmd=4 ne=3 round=2");
    fmtHandoffClear(out, sizeof out, -1, 4294967295u, 0, 0);
    KT_CHECK_EQ(out, "HANDOFF_CLEAR m=? cmd=4294967295 ne=0 round=0");
    fmtHandoffClear(out, sizeof out, 2, 0, -1, -2);
    KT_CHECK_EQ(out, "HANDOFF_CLEAR m=2 cmd=0 ne=-1 round=-2");
    // ROUND m=<k> n=<count> subs=<s,...|-> cur=<n|-> ne=<n> types=<hex,...|->
    const int subs2[2] = { 1, 10 };
    const int types3[3] = { 0x3f, 1, 0x0d };
    KT_CHECK_EQ(fmtRound(out, sizeof out, 0, subs2, 2, -1, 3, types3, 3), 49);
    KT_CHECK_EQ(out, "ROUND m=0 n=2 subs=1,10 cur=- ne=3 types=3f,01,0d");
    fmtRound(out, sizeof out, 0, nullptr, 0, -1, 0, nullptr, 0);
    KT_CHECK_EQ(out, "ROUND m=0 n=0 subs=- cur=- ne=0 types=-");
    // cur >= 0 prints decimal (0 included); hex boundary: 0x0f pads, 0x10 does not; 0 pads to 00
    const int subs1[1] = { 5 };
    const int types2[2] = { 0x0f, 0x10 };
    fmtRound(out, sizeof out, 1, subs1, 1, 7, 2, types2, 2);
    KT_CHECK_EQ(out, "ROUND m=1 n=1 subs=5 cur=7 ne=2 types=0f,10");
    const int zero[1] = { 0 };
    fmtRound(out, sizeof out, -1, nullptr, 0, 0, 1, zero, 1);
    KT_CHECK_EQ(out, "ROUND m=? n=0 subs=- cur=0 ne=1 types=00");
    // a negative type prints as unsigned 32-bit hex, the same as fmtQj
    const int neg[1] = { -1 };
    fmtRound(out, sizeof out, 0, subs1, 1, -1, 1, neg, 1);
    KT_CHECK_EQ(out, "ROUND m=0 n=1 subs=5 cur=- ne=1 types=ffffffff");
    // a null subs list with nSubs > 0 prints no subs (never dereferenced)
    fmtRound(out, sizeof out, 0, nullptr, 2, -1, 0, nullptr, 0);
    KT_CHECK_EQ(out, "ROUND m=0 n=0 subs=- cur=- ne=0 types=-");
    // a null types list with nTypes > 0 prints no types (never dereferenced)
    fmtRound(out, sizeof out, 0, nullptr, 0, -1, 2, nullptr, 2);
    KT_CHECK_EQ(out, "ROUND m=0 n=0 subs=- cur=- ne=2 types=-");
    // clamp: 60 types overflow the 200-char line; output clamped to 199 chars, return value is the full length
    int many[60];
    for (int i = 0; i < 60; ++i) many[i] = 0x3f;
    const int nr = fmtRound(out, sizeof out, 0, nullptr, 0, -1, 60, many, 60);
    KT_CHECK_EQ(nr, 218);
    KT_CHECK_EQ(strlen(out), (size_t)(kLogLineMax - 1));
    KT_CHECK(strncmp(out, "ROUND m=0 n=0 subs=- cur=- ne=60 types=3f,3f,", 45) == 0);
    // tiny buffers: truncated and NUL-terminated, no overflow, return value is the full length
    char tinyC[10];
    memset(tinyC, 'Z', sizeof tinyC);
    KT_CHECK_EQ(fmtHandoffClear(tinyC, sizeof tinyC, 0, 4, 3, 2), 36);
    KT_CHECK_EQ(strlen(tinyC), (size_t)9);
    KT_CHECK(strncmp(tinyC, "HANDOFF_C", 9) == 0 && tinyC[9] == '\0');
    char tinyR[12];
    memset(tinyR, 'Z', sizeof tinyR);
    KT_CHECK_EQ(fmtRound(tinyR, sizeof tinyR, 0, subs2, 2, -1, 3, types3, 3), 49);
    KT_CHECK_EQ(strlen(tinyR), (size_t)11);
    KT_CHECK(strncmp(tinyR, "ROUND m=0 n", 11) == 0 && tinyR[11] == '\0');
    // null buffers: nothing written, length still reported
    KT_CHECK_EQ(fmtHandoffClear(nullptr, 0, 0, 4, 3, 2), 36);
    KT_CHECK_EQ(fmtRound(nullptr, 0, 0, subs2, 2, -1, 3, types3, 3), 49);
}

void testInsetRect() {
    KT_CHECK(rectEq(insetRect(Rect{ 432, 521, 40, 40 }), 438, 527, 28, 28));   // 15% of 40 = 6
    KT_CHECK(rectEq(insetRect(Rect{ 378, 531, 20, 20 }), 381, 534, 14, 14));   // 3 px floor
    KT_CHECK(rectEq(insetRect(Rect{ 10, 20, 6, 6 }), 10, 20, 6, 6));          // 15% of 6 floors to 0: unchanged
    KT_CHECK(rectEq(insetRect(Rect{ 0, 0, 100, 60 }), 15, 9, 70, 42));        // each axis insets by its own size
}

void testCancelRect() {
    // vanilla 800x600 design extents (R7 FINDINGS s4): QUEUE0 [432,521,40,40], BTN_CLEARALL [317,570,166,24]
    const Rect q0{ 432, 521, 40, 40 }, q1{ 378, 531, 20, 20 };
    Rect r = cancelRect(q0, q1);
    KT_CHECK(rectEq(r, 317, 570, 166, 24));                  // wide rounded bar beneath Slot 0 matching native combat BTN_CLEARALL
    KT_CHECK_EQ(r.y, q0.y + q0.h + 9);
    KT_CHECK(!ov(r, q0));
    // the same layout scaled x2 (live extents scale with the HUD): [634, 1140, 332, 48]
    r = cancelRect(Rect{ 864, 1042, 80, 80 }, Rect{ 756, 1062, 40, 40 });
    KT_CHECK(rectEq(r, 634, 1140, 332, 48));
    KT_CHECK_EQ(r.y, 1042 + 80 + 18);
    // unusable extents -> none
    KT_CHECK(rectEq(cancelRect(Rect{ 432, 521, 0, 40 }, q1), 0, 0, 0, 0));
    KT_CHECK(rectEq(cancelRect(Rect{ 432, 521, 40, 0 }, q1), 0, 0, 0, 0));
}

void testChevronRect() {
    const Rect q0{ 432, 521, 40, 40 };
    Rect c = chevronRect(q0);
    KT_CHECK(rectEq(c, 410, 528, 18, 26)); // points right into Slot 0, right edge = 428 (gap 4 from q0.x 432)
    KT_CHECK_EQ(c.x + c.w, q0.x - 4);
    KT_CHECK_EQ(c.y + c.h / 2, q0.y + q0.h / 2);
    // scaled x2
    Rect c2 = chevronRect(Rect{ 864, 1042, 80, 80 });
    KT_CHECK(rectEq(c2, 820, 1056, 36, 52));
    // unusable
    KT_CHECK(rectEq(chevronRect(Rect{ 0, 0, 0, 0 }), 0, 0, 0, 0));

    // combatBgRect parity with native LBL_COMBATBG3 [310, 498, 180, 90]
    Rect bg = combatBgRect(q0);
    KT_CHECK(rectEq(bg, 310, 498, 180, 90));
    Rect bg2 = combatBgRect(Rect{ 864, 1042, 80, 80 });
    KT_CHECK(rectEq(bg2, 864 - 244, 1042 - 46, 360, 180));
    // Live 3840x1600 coordinates from play logs (q0 = 106x106 at x=2005, y=1389)
    Rect bgLive = combatBgRect(Rect{ 2005, 1389, 106, 106 });
    KT_CHECK(rectEq(bgLive, 1682, 1328, 477, 239));
    KT_CHECK(rectEq(combatBgRect(Rect{ 0, 0, 0, 0 }), 0, 0, 0, 0));
}

void testItemBuffer() {
    ItemBuffer b;
    KT_CHECK(b.empty() && !b.full() && b.count == 0);
    QueuedItemAction a1{ 0x1000, 101, 0, 1, {1,2,3}, 1000 };
    QueuedItemAction a2{ 0x1000, 102, 0, 1, {1,2,3}, 1050 };
    QueuedItemAction a3{ 0x1000, 103, 0, 1, {1,2,3}, 1100 };
    QueuedItemAction a4{ 0x1000, 104, 0, 1, {1,2,3}, 1150 };

    KT_CHECK(b.push(a1));
    KT_CHECK(b.push(a2));
    KT_CHECK(b.push(a3));
    KT_CHECK(!b.push(a4)); // full cap 3
    KT_CHECK(b.full() && b.count == 3);

    // FIFO order
    QueuedItemAction out;
    KT_CHECK(b.pop(&out) && out.clientItem == 101);
    KT_CHECK(b.count == 2);
    KT_CHECK(b.pop(&out) && out.clientItem == 102);
    KT_CHECK(b.count == 1);
    KT_CHECK(b.pop(&out) && out.clientItem == 103);
    KT_CHECK(b.empty() && b.count == 0);
    KT_CHECK(!b.pop(&out));

    // Decision
    KT_CHECK_EQ((int)decideItemAction(b, true, 0), (int)DECIDE_DISPATCH_NOW);
    KT_CHECK_EQ((int)decideItemAction(b, false, 0), (int)DECIDE_DISPATCH_NOW);
    KT_CHECK_EQ((int)decideItemAction(b, false, 1), (int)DECIDE_BUFFER);
    b.active = true;
    KT_CHECK_EQ((int)decideItemAction(b, false, 0), (int)DECIDE_BUFFER);
    b.push(a1); b.push(a2); b.push(a3);
    KT_CHECK_EQ((int)decideItemAction(b, false, 0), (int)DECIDE_DROP_FULL);

    // Re-inject decision
    b.clear();
    b.active = true;
    b.activeStartTick = 1000;
    // Still in queue -> no re-inject
    KT_CHECK(!shouldReInject(b, true, false, false, 1100));
    // Follower AI 0x3D wiped queue -> re-inject!
    KT_CHECK(shouldReInject(b, false, true, false, 1100));
    // Paused wipe without followLeader -> no re-inject (loop stripped)
    KT_CHECK(!shouldReInject(b, false, false, true, 1500));
    // User cancellation -> no re-inject even with followLeader
    b.cancel();
    KT_CHECK(!shouldReInject(b, false, true, true, 1500));
    KT_CHECK(!shouldReInject(b, false, true, false, 1500, true));

    // shouldDispatchNext decision
    b.clear();
    KT_CHECK(!shouldDispatchNext(b, 0)); // empty -> no
    b.push(a1);
    KT_CHECK(shouldDispatchNext(b, 0));  // has items, idle engine -> yes
    KT_CHECK(!shouldDispatchNext(b, 1)); // has items, busy engine -> no
    b.active = true;
    KT_CHECK(!shouldDispatchNext(b, 0)); // has items, dispatch in-flight -> no
}

void testPausedItemQueueing() {
    // 1. Paused single/multi-item queueing
    // Dispatches immediate or buffers up to cap 3, no infinite pause re-inject loop
    ItemBuffer buf;
    KT_CHECK(buf.empty());

    QueuedItemAction a1{ 0x1000, 101, 0, 1, {1,2,3}, 1000 };
    QueuedItemAction a2{ 0x1000, 102, 0, 1, {1,2,3}, 1050 };
    QueuedItemAction a3{ 0x1000, 103, 0, 1, {1,2,3}, 1100 };
    QueuedItemAction a4{ 0x1000, 104, 0, 1, {1,2,3}, 1150 };
    QueuedItemAction a5{ 0x1000, 105, 0, 1, {1,2,3}, 1200 };

    // When paused and idle (engine queue len = 0, no active action), first item dispatches immediately
    KT_CHECK_EQ((int)decideItemAction(buf, /*inCombat=*/false, /*engineQueueLen=*/0), (int)DECIDE_DISPATCH_NOW);
    buf.active = true;
    buf.activeAction = a1;
    buf.activeStartTick = 1000;

    // Simulate 60 Hz paused ticker across 3000ms:
    // When paused, if hasFollowLeader is false, shouldReInject MUST remain false on every frame.
    for (uint32_t t = 1000; t <= 4000; t += 16) {
        KT_CHECK(!shouldReInject(buf, /*itemInQueue=*/false, /*hasFollowLeader=*/false, /*paused=*/true, t));
        KT_CHECK(!shouldReInject(buf, /*itemInQueue=*/true, /*hasFollowLeader=*/false, /*paused=*/true, t));
    }

    // Now while paused and active item is in flight, queue further items up to cap 3
    KT_CHECK_EQ((int)decideItemAction(buf, false, 0), (int)DECIDE_BUFFER);
    KT_CHECK(buf.push(a2));
    KT_CHECK_EQ(buf.count, 1);

    KT_CHECK_EQ((int)decideItemAction(buf, false, 0), (int)DECIDE_BUFFER);
    KT_CHECK(buf.push(a3));
    KT_CHECK_EQ(buf.count, 2);

    KT_CHECK_EQ((int)decideItemAction(buf, false, 0), (int)DECIDE_BUFFER);
    KT_CHECK(buf.push(a4));
    KT_CHECK_EQ(buf.count, 3);
    KT_CHECK(buf.full());

    // 5th item exceeds cap 3: rejected
    KT_CHECK_EQ((int)decideItemAction(buf, false, 0), (int)DECIDE_DROP_FULL);
    KT_CHECK(!buf.push(a5));
    KT_CHECK_EQ(buf.count, 3);

    // Engine is still busy with active item or paused
    KT_CHECK(!shouldDispatchNext(buf, /*engineQueueLen=*/0));
}

void testPausedCancelClick() {
    // 2. Paused cancel click
    // buf.cancel() sets userCancelled = true, disallows re-injection even if follower AI sets hasFollowLeader
    ItemBuffer buf;
    QueuedItemAction a1{ 0x1000, 101, 0, 1, {1,2,3}, 1000 };
    QueuedItemAction a2{ 0x1000, 102, 0, 1, {1,2,3}, 1050 };
    QueuedItemAction a3{ 0x1000, 103, 0, 1, {1,2,3}, 1100 };

    buf.active = true;
    buf.activeAction = a1;
    buf.activeStartTick = 1000;
    buf.push(a2);
    buf.push(a3);
    KT_CHECK_EQ(buf.count, 2);
    KT_CHECK(!buf.userCancelled);

    // User clicks Cancel while paused:
    buf.cancel();
    KT_CHECK_EQ(buf.count, 0);
    KT_CHECK(buf.empty());
    KT_CHECK(!buf.active);
    KT_CHECK(buf.userCancelled);
    KT_CHECK(!buf.reInjectPending);

    // Follower AI tries to wipe / run followLeader:
    // Even if follower AI sets hasFollowLeader=true, re-injection is strictly blocked!
    KT_CHECK(!shouldReInject(buf, /*itemInQueue=*/false, /*hasFollowLeader=*/true, /*paused=*/true, 1050));
    KT_CHECK(!shouldReInject(buf, /*itemInQueue=*/false, /*hasFollowLeader=*/true, /*paused=*/false, 1050));
    KT_CHECK(!shouldReInject(buf, /*itemInQueue=*/false, /*hasFollowLeader=*/true, /*paused=*/false, 1050, /*userCancelled=*/true));

    // Even if caller artificially set active=true, userCancelled still blocks re-inject:
    buf.active = true;
    KT_CHECK(!shouldReInject(buf, false, true, true, 1050));
    buf.active = false;

    // Buffer does not dispatch next because it is empty
    KT_CHECK(!shouldDispatchNext(buf, 0));

    // When a fresh action is later queued, userCancelled resets
    QueuedItemAction aFresh{ 0x1000, 104, 0, 1, {1,2,3}, 2000 };
    KT_CHECK(buf.push(aFresh));
    KT_CHECK(!buf.userCancelled);
}

void testFollowerWipeProtection() {
    // 3. Follower wipes
    // Legitimate AI wipe with hasFollowLeader triggers re-inject if not user-cancelled
    ItemBuffer buf;
    QueuedItemAction a1{ 0x1000, 201, 0, 1, {1,2,3}, 1000 };
    QueuedItemAction a2{ 0x1000, 202, 0, 1, {1,2,3}, 1050 };

    buf.active = true;
    buf.activeAction = a1;
    buf.activeStartTick = 1000;
    buf.push(a2);

    // A. Normal execution: item is present in queue, no re-inject
    KT_CHECK(!shouldReInject(buf, /*itemInQueue=*/true, /*hasFollowLeader=*/false, false, 1050));

    // B. Follower AI (0x3D) wipes queue: itemInQueue=false, hasFollowLeader=true
    KT_CHECK(shouldReInject(buf, /*itemInQueue=*/false, /*hasFollowLeader=*/true, false, 1050));

    // Simulation of re-injection dispatch:
    buf.reInjectPending = true;
    // While reInjectPending is set, no secondary re-inject can fire
    KT_CHECK(!shouldReInject(buf, false, true, false, 1051));

    // Re-inject dispatch finishes
    buf.reInjectPending = false;
    buf.activeStartTick = 1060;

    // Item now executes in server queue
    KT_CHECK(!shouldReInject(buf, /*itemInQueue=*/true, /*hasFollowLeader=*/false, false, 1100));

    // C. Natural item completion:
    // The engine showed it executing (1100), then it leaves the queue (first absent frame 1200): itemInQueue=false, hasFollowLeader=false
    buf.noteView(true, 1100); buf.notePresent(true, true, 1100);
    buf.noteView(false, 1200);
    // Gone 200ms (past the debounce) but only 340ms after dispatch: the min-active rule keeps the verdict at present, so the buffer stays active
    const uint32_t nowEarly = 1400;
    KT_CHECK(!shouldReInject(buf, false, false, false, nowEarly));
    const EngineView evGone{};   // action absent from the engine queue
    KT_CHECK_EQ((int)activeVerdict(buf, evGone, nowEarly), (int)AV_PRESENT);
    KT_CHECK(buf.active);

    // At 550ms after activeStartTick (1610): the engine verdict decides completion (executed), not a local flag
    const uint32_t nowDone = 1610;
    const ActiveVerdict doneV = activeVerdict(buf, evGone, nowDone);
    KT_CHECK_EQ((int)doneV, (int)AV_EXECUTED);
    buf.active = (doneV == AV_PRESENT);
    KT_CHECK(!buf.active);

    // D. Now next buffered action (a2) is ready for dispatch
    KT_CHECK(shouldDispatchNext(buf, /*engineQueueLen=*/0));
    QueuedItemAction nextA;
    KT_CHECK(buf.pop(&nextA));
    KT_CHECK_EQ(nextA.clientItem, 202u);
    buf.active = true;
    buf.activeAction = nextA;
    buf.activeStartTick = nowDone;

    KT_CHECK_EQ(buf.count, 0);
    KT_CHECK(buf.empty());

    // E. Contrast with user cancel: if user cancelled, wipe does NOT re-inject
    buf.cancel();
    KT_CHECK(!shouldReInject(buf, false, true, false, 1700));
}

void testMixedForcePowersAndItems() {
    // 4. Mixed force powers and items
    // Item buffer waits for engine spell queue to reach 0 before dispatch
    ItemBuffer buf;
    QueuedItemAction medpac1{ 0x1000, 301, 0, 1, {0,0,0}, 1000 };
    QueuedItemAction medpac2{ 0x1000, 302, 0, 1, {0,0,0}, 1050 };

    // Situation: Player queued 2 Force Powers in engine queue (e.g. Force Heal + Force Speed)
    int engineQueueLen = 2;

    // Out-of-combat: Player activates Medpac 1
    // decideItemAction sees engineQueueLen > 0 -> DECIDE_BUFFER
    KT_CHECK_EQ((int)decideItemAction(buf, /*inCombat=*/false, engineQueueLen), (int)DECIDE_BUFFER);
    KT_CHECK(buf.push(medpac1));
    KT_CHECK_EQ(buf.count, 1);

    // While engine queue has force powers, item buffer cannot dispatch
    KT_CHECK(!shouldDispatchNext(buf, engineQueueLen));

    // Player activates Medpac 2:
    KT_CHECK_EQ((int)decideItemAction(buf, false, engineQueueLen), (int)DECIDE_BUFFER);
    KT_CHECK(buf.push(medpac2));
    KT_CHECK_EQ(buf.count, 2);

    // Engine finishes 1st Force Power: 1 remains
    engineQueueLen = 1;
    KT_CHECK(!shouldDispatchNext(buf, engineQueueLen));

    // Engine finishes 2nd Force Power: queue is now empty
    engineQueueLen = 0;
    KT_CHECK(shouldDispatchNext(buf, engineQueueLen));

    // Sequential dispatch of Medpac 1:
    QueuedItemAction dispatched1;
    KT_CHECK(buf.pop(&dispatched1));
    KT_CHECK_EQ(dispatched1.clientItem, 301u);
    buf.active = true;
    buf.activeAction = dispatched1;
    buf.activeStartTick = 2000;

    // Engine is now executing Medpac 1 (engineQueueLen = 1)
    engineQueueLen = 1;
    // Medpac 2 must wait in buffer:
    KT_CHECK(!shouldDispatchNext(buf, engineQueueLen));

    // Medpac 1 completes: engineQueueLen = 0, elapsed > 500ms
    buf.active = false;
    engineQueueLen = 0;

    // Now Medpac 2 can dispatch:
    KT_CHECK(shouldDispatchNext(buf, engineQueueLen));
    QueuedItemAction dispatched2;
    KT_CHECK(buf.pop(&dispatched2));
    KT_CHECK_EQ(dispatched2.clientItem, 302u);
    buf.active = true;
    buf.activeAction = dispatched2;

    // Buffer is now empty; once Medpac 2 completes, nothing more to dispatch
    KT_CHECK(buf.empty());
    buf.active = false;
    KT_CHECK(!shouldDispatchNext(buf, 0));

    // Combat override verification:
    // If in combat, engine queue length does not cause buffering; it dispatches immediately
    KT_CHECK_EQ((int)decideItemAction(buf, /*inCombat=*/true, /*engineQueueLen=*/3), (int)DECIDE_DISPATCH_NOW);
}

void testStripHead() {
    // B3 (QL17): the head (slot 0) is opaque when waiting_icon_dim=0 and bigger on companion strips (strip_head_pct).
    { Config c; Style s = slotStyle(c, 0, true);   // default waitingIconDim=true: unchanged
      KT_CHECK(c.waitingIconDim && c.stripHeadPct == 100 && s.iconA == 0.6f);
      c.waitingIconDim = false;
      s = slotStyle(c, 0, true);
      KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f * 0.6f && s.borderA == 0.9f * 0.6f && s.frameA == 0.9f * 0.6f);   // bg / border / frame still dim
      KT_CHECK(s.bg[0] == 0.03f && s.bg[1] == 0.15f && s.bg[2] == 0.18f);
      s = slotStyle(c, 0, false); KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f && s.frameA == 0.9f);
      s = slotStyle(c, 1, true); KT_CHECK(s.iconA == 1.0f && s.bgA == 0.45f); }
    const Rect card{ 3711, 1285, 85, 96 }, vit{ 3671, 1285, 47, 96 };
    { Config c; c.stripSide = kSideLeft;   // stripHeadSize
      KT_CHECK_EQ(stripHeadSize(vit, c), 58);   // pct 100
      c.stripHeadPct = 150; KT_CHECK_EQ(stripHeadSize(vit, c), 87);
      c.stripHeadPct = 200; KT_CHECK_EQ(stripHeadSize(vit, c), 96);   // 116 capped at vit.h
      c.stripHeadPct = 999; KT_CHECK_EQ(stripHeadSize(vit, c), 96);   // pct itself clamped to 200
      c.stripHeadPct = 50; KT_CHECK_EQ(stripHeadSize(vit, c), 58);    // never below the strip icon size
      c.stripHeadPct = 150; c.stripIconPx = 60; KT_CHECK_EQ(stripHeadSize(vit, c), 90);   // strip_icon_px override honoured
      c.stripIconPx = 70; KT_CHECK_EQ(stripHeadSize(vit, c), 96);
      c.stripIconPx = 58; KT_CHECK_EQ(stripHeadSize(Rect{ 0, 0, 47, 20 }, c), 58); }   // a short vit never shrinks the icons
    { Config c; c.stripSide = kSideLeft; c.stripHeadPct = 150; Rect out[4];   // n=3 left layout
      const int reserve = 66;
      KT_CHECK_EQ(stripRects(card, vit, 3, c, out, 3840, reserve), 3);
      // Head (index 0) is the RIGHTMOST icon, next to the portrait; later actions extend left.
      KT_CHECK(rectEq(out[0], 3518, 1289, 87, 87));
      KT_CHECK(rectEq(out[1], 3458, 1304, 58, 58));
      KT_CHECK(rectEq(out[2], 3398, 1304, 58, 58));
      KT_CHECK_EQ(out[0].x, out[1].x + out[1].w + kStripGap);                // contiguous, 2 px gap
      KT_CHECK_EQ(out[1].x, out[2].x + out[2].w + kStripGap);
      KT_CHECK_EQ(out[0].x + out[0].w, vit.x - reserve);                      // row right edge unchanged
      KT_CHECK_EQ(stripRowWidth(vit, 3, c), 87 + 2 * (58 + kStripGap));
      KT_CHECK_EQ(stripRowWidth(vit, 3, c), out[0].x + out[0].w - out[2].x);  // == sum of the placed rects
      KT_CHECK_EQ(stripRowWidth(vit, 1, c), 87); KT_CHECK_EQ(stripRowWidth(vit, 0, c), 0);
      KT_CHECK_EQ(stripRowWidth(vit, 9, c), 87 + 3 * (58 + kStripGap));       // n clamped to 4
      // Cancel: keeps the 58 px square, centred on the (bigger) head, same gap rule
      KT_CHECK(rectEq(stripCancelRect(out, 3, kSideLeft, 3840, 58), 3398 - 14 - 58, 1289 + (87 - 58) / 2, 58, 58));
      KT_CHECK(rectEq(stripCancelRect(out, 3, kSideLeft, 3840, 0), 3398 - 21 - 87, 1289, 87, 87));   // sq=0: follows the head
      c.stripSide = kSideAbove;   // above: bottom edges aligned 2 px over the card
      KT_CHECK_EQ(stripRects(card, vit, 2, c, out, 0, 0), 2);
      KT_CHECK(rectEq(out[0], 3771, 1196, 87, 87)); KT_CHECK(rectEq(out[1], 3711, 1225, 58, 58));
      KT_CHECK_EQ(out[0].y + out[0].h, out[1].y + out[1].h);
      c.stripSide = kSideBelow;   // below: top edges aligned 2 px under the card
      KT_CHECK_EQ(stripRects(card, vit, 2, c, out, 0, 0), 2);
      KT_CHECK(rectEq(out[0], 3771, 1383, 87, 87)); KT_CHECK(rectEq(out[1], 3711, 1383, 58, 58)); }
    { // pct=100 regression pin: byte-identical to the pre-B3 rects on every side
      const int sides[] = { kSideLeft, kSideAbove, kSideBelow };
      for (int sd : sides) for (int n = 1; n <= 4; ++n) {
          Config c; c.stripSide = sd; Rect out[4];
          KT_CHECK_EQ(stripRects(card, vit, n, c, out, 3840, 66), n);
          const int s = 58, rowW = n * s + (n - 1) * kStripGap;
          int x0 = sd == kSideLeft ? vit.x - 66 - rowW : card.x;
          int y = sd == kSideLeft ? vit.y + (vit.h - s) / 2 : (sd == kSideBelow ? card.y + card.h + kStripGap : card.y - kStripGap - s);
          const int over = x0 + rowW - 3840; if (over > 0) x0 -= over;
          for (int i = 0; i < n; ++i) KT_CHECK(rectEq(out[i], x0 + (n - 1 - i) * (s + kStripGap), y, s, s));   // head rightmost
          KT_CHECK_EQ(stripRowWidth(vit, n, c), rowW);
      } }
    { Config c; Rect icons[1] = { Rect{ 3700, 1285, 58, 58 } };   // default sq: identical to the old behaviour
      KT_CHECK(rectEq(stripCancelRect(icons, 1, kSideLeft, 3840), 3700 - 14 - 58, 1285, 58, 58)); (void)c; }
    { Config c; // ini: defaults, valid, clamped, bad
      KT_CHECK(c.waitingIconDim && c.stripHeadPct == 100);
      KT_CHECK(parseIniLine(c, "waiting_icon_dim=0")); KT_CHECK(!c.waitingIconDim);
      KT_CHECK(parseIniLine(c, "waiting_icon_dim = on")); KT_CHECK(c.waitingIconDim);
      KT_CHECK(!parseIniLine(c, "waiting_icon_dim=2")); KT_CHECK(c.waitingIconDim);
      KT_CHECK(parseIniLine(c, "strip_head_pct=150")); KT_CHECK_EQ(c.stripHeadPct, 150);
      KT_CHECK(parseIniLine(c, "strip_head_pct = 200")); KT_CHECK_EQ(c.stripHeadPct, 200);
      KT_CHECK(parseIniLine(c, "strip_head_pct=100")); KT_CHECK_EQ(c.stripHeadPct, 100);
      KT_CHECK(parseIniLine(c, "strip_head_pct=300")); KT_CHECK_EQ(c.stripHeadPct, 200);   // clamped
      KT_CHECK(parseIniLine(c, "strip_head_pct=50")); KT_CHECK_EQ(c.stripHeadPct, 100);    // clamped
      c.stripHeadPct = 150;
      KT_CHECK(!parseIniLine(c, "strip_head_pct=")); KT_CHECK(!parseIniLine(c, "strip_head_pct=abc")); KT_CHECK(!parseIniLine(c, "strip_head_pct=-5")); KT_CHECK(!parseIniLine(c, "strip_head_pct=12345"));
      KT_CHECK_EQ(c.stripHeadPct, 150); }
}

void testStripCancel() {
    // live 3840x1600 companion 2 / 3 (doc 08 probe 1, as testStripLeftLive): 58 px icons centred on the 96 px bar
    Config c;
    const Rect vit2{ 3671, 1285, 47, 96 }, card2{ 3711, 1285, 85, 85 }, vit3{ 3671, 1181, 47, 96 }, card3{ 3711, 1181, 85, 85 };
    const shui::Rect leaderVit{ 3511, 1386, 103, 208 }, leaderCard{ 3591, 1394, 191, 192 }, leaderForce{ 3762, 1386, 103, 208 };
    const Rect* vits[] = { &vit2, &vit3 }; const Rect* cards[] = { &card2, &card3 };
    for (int k = 0; k < 2; ++k) for (int n = 1; n <= 4; ++n) {
        Rect icons[4];
        KT_CHECK_EQ(stripRects(*cards[k], *vits[k], n, c, icons, 3840), n);
        const Rect cr = stripCancelRect(icons, n, kSideLeft, 3840);
        KT_CHECK_EQ(cr.w, 58); KT_CHECK_EQ(cr.h, 58); KT_CHECK_EQ(cr.y, icons[0].y);
        KT_CHECK_EQ(icons[n - 1].x - (cr.x + cr.w), 14);   // left of the leftmost icon                  // gap = 58 / 4 = 14
        for (int i = 0; i < n; ++i) KT_CHECK(!ov(cr, icons[i]));
        // clear of the real selfheal button + ring, the bars and cards
        const shui::Rect btn = shui::buttonRect(shui::Rect{ vits[k]->x, vits[k]->y, 47, vits[k]->h });
        const shui::Rect ring = shui::ringRect(btn);
        KT_CHECK(!ov(cr, Rect{ btn.x, btn.y, btn.w, btn.h })); KT_CHECK(!ov(cr, Rect{ ring.x, ring.y, ring.w, ring.h }));
        KT_CHECK(!ov(cr, *vits[k])); KT_CHECK(!ov(cr, *cards[k]));
        for (const Rect* o : { &vit2, &vit3, &card2, &card3 }) KT_CHECK(!ov(cr, *o));
        for (const shui::Rect& o : { leaderVit, leaderCard, leaderForce }) KT_CHECK(!ov(cr, Rect{ o.x, o.y, o.w, o.h }));
    }
    { Rect icons[4]; stripRects(card2, vit2, 1, c, icons, 3840);
      KT_CHECK(rectEq(stripCancelRect(icons, 1, kSideLeft, 3840), 3671 - 66 - 58 - 14 - 58, 1285 + 19, 58, 58)); }
    // left: no room left of the row (x < 0) -> no cancel, never moved into the selfheal reserve
    { const Rect icons[1] = { Rect{ 20, 100, 38, 38 } };
      KT_CHECK(rectEq(stripCancelRect(icons, 1, kSideLeft, 3840), 0, 0, 0, 0)); }
    // above / below: x < 0 falls back to just RIGHT of the head icon (icons[0] is the rightmost)
    { Rect icons[2] = { Rect{ 27, 100, 20, 20 }, Rect{ 5, 100, 20, 20 } };
      KT_CHECK(rectEq(stripCancelRect(icons, 2, kSideAbove, 3840), 47 + 5, 100, 20, 20));
      KT_CHECK(rectEq(stripCancelRect(icons, 2, kSideBelow, 3840), 47 + 5, 100, 20, 20)); }
    // above / below with room: just left of the first icon
    { const Rect icons[1] = { Rect{ 3711, 1370, 16, 16 } };
      KT_CHECK(rectEq(stripCancelRect(icons, 1, kSideAbove, 3840), 3711 - 4 - 16, 1370, 16, 16)); }
    // the min gap (kStripGap) for tiny icons
    { const Rect icons[1] = { Rect{ 100, 50, 8, 8 } };
      KT_CHECK(rectEq(stripCancelRect(icons, 1, kSideLeft, 0), 100 - 2 - 8, 50, 8, 8)); }
    // a square past the right edge is refused (only reachable through the right-hand fallback)
    { Rect icons[1] = { Rect{ 5, 100, 20, 20 } };
      KT_CHECK(rectEq(stripCancelRect(icons, 1, kSideAbove, 40), 0, 0, 0, 0));
      KT_CHECK(rectEq(stripCancelRect(icons, 1, kSideAbove, 55), 30, 100, 20, 20)); }
    // null / empty / unusable
    KT_CHECK(rectEq(stripCancelRect(nullptr, 1, kSideLeft), 0, 0, 0, 0));
    { const Rect icons[1] = { Rect{ 100, 50, 20, 20 } };
      KT_CHECK(rectEq(stripCancelRect(icons, 0, kSideLeft), 0, 0, 0, 0)); KT_CHECK(rectEq(stripCancelRect(icons, -1, kSideLeft), 0, 0, 0, 0)); }
    { const Rect icons[1] = { Rect{ 100, 50, 0, 20 } };
      KT_CHECK(rectEq(stripCancelRect(icons, 1, kSideLeft), 0, 0, 0, 0)); }
}

void testCancelVisible() {
    KT_CHECK(cancelVisible(true, true, false, false, 0, 1));
    KT_CHECK(cancelVisible(true, true, false, false, 249, 4));
    KT_CHECK(!cancelVisible(true, true, false, false, 250, 1));   // stale capture
    KT_CHECK(!cancelVisible(true, true, false, false, 0, 0));     // no queue shown -> no cancel
    KT_CHECK(!cancelVisible(true, true, true, false, 0, 1));      // vanilla red combat bar owns combat
    KT_CHECK(!cancelVisible(true, true, false, true, 0, 1));      // menu
    KT_CHECK(!cancelVisible(false, true, false, false, 0, 1));
    KT_CHECK(!cancelVisible(true, false, false, false, 0, 1));    // cancel = 0
}

// ---- 0.4.1: a Down (HP <= 0) companion hides its strip; unknown vitals never hide it ----
void testStripHiddenForDown() {
    KT_CHECK(stripHiddenForDown(true, true));
    KT_CHECK(!stripHiddenForDown(true, false));
    KT_CHECK(!stripHiddenForDown(false, true));    // unknown vitals: the strip stays
    KT_CHECK(!stripHiddenForDown(false, false));
}

// ---- Unified action buffer (doc 33 QH4/H10): item + spell actions share one ActionBuffer ----
void fillBuf(ActionBuffer& b, int n) {
    for (int i = 0; i < n; ++i) KT_CHECK(b.push(makeItemAction(0x1000, 100 + i, 0, 1, nullptr, 1)));
}

void testReplayArgs() {
    const float pos[3] = { 1.0f, 2.0f, 3.0f };
    const QueuedAction a = makeSpellAction(0x2000, 0x55, 3, 0, 0, 0, pos, 0, 1);
    const ReplayArgs ra = buildReplayArgs(a);
    KT_CHECK(ra.pos == a.pos);
    KT_CHECK(ra.pos != nullptr);
    KT_CHECK(ra.pos[0] == 1.0f && ra.pos[1] == 2.0f && ra.pos[2] == 3.0f);
    KT_CHECK_EQ(ra.flag, 0);
    const QueuedAction n = makeSpellAction(0x2000, 0x55, 3, 0, 0, 0, nullptr, 5, 1);
    const ReplayArgs rn = buildReplayArgs(n);
    KT_CHECK(rn.pos != nullptr && rn.pos == n.pos);
    KT_CHECK(rn.pos[0] == 0.0f && rn.pos[1] == 0.0f && rn.pos[2] == 0.0f);
    KT_CHECK_EQ(rn.flag, 5);
}

void testUnifiedActionMake() {
    const float pos[3] = { 1.5f, 2.5f, 3.5f };
    QueuedAction it = makeItemAction(0x1000, 301, 4, 7, pos, 999);
    KT_CHECK_EQ(it.creature, (uintptr_t)0x1000);
    KT_CHECK_EQ((int)it.kind, (int)ActionKind::Item);
    KT_CHECK_EQ(it.clientItem, 301u);
    KT_CHECK_EQ(it.packedByte, 4);
    KT_CHECK_EQ(it.targetId, 7u);
    KT_CHECK(it.pos[0] == 1.5f && it.pos[1] == 2.5f && it.pos[2] == 3.5f);
    KT_CHECK_EQ(it.queueTime, 999u);
    KT_CHECK_EQ(it.spellId, 0u); KT_CHECK_EQ(it.arg2, 0); KT_CHECK_EQ(it.multispell, 0);
    KT_CHECK_EQ(it.metamagic, 0); KT_CHECK_EQ(it.domain, 0); KT_CHECK_EQ(it.reInjectCount, 0);

    QueuedAction sp = makeSpellAction(0x2000, 0x55, 3, 2, 4, 5, pos, 7, 77);
    KT_CHECK_EQ(sp.creature, (uintptr_t)0x2000);
    KT_CHECK_EQ((int)sp.kind, (int)ActionKind::Spell);
    KT_CHECK_EQ(sp.clientItem, 0x55u);          // spellId mirrored into clientItem
    KT_CHECK_EQ(sp.packedByte, 3);              // arg2 mirrored into packedByte
    KT_CHECK_EQ(sp.targetId, 3u);               // mirror of arg2
    KT_CHECK_EQ(sp.flag, 7);
    KT_CHECK_EQ(sp.spellId, 0x55u);
    KT_CHECK_EQ(sp.arg2, 3);
    KT_CHECK_EQ(sp.multispell, 2);
    KT_CHECK_EQ(sp.metamagic, 4);
    KT_CHECK_EQ(sp.domain, 5);
    KT_CHECK(sp.pos[0] == 1.5f && sp.pos[1] == 2.5f && sp.pos[2] == 3.5f);
    KT_CHECK_EQ(sp.queueTime, 77u);
    KT_CHECK_EQ(sp.reInjectCount, 0);

    // null pos leaves zeros, for both kinds
    QueuedAction itNull = makeItemAction(0x1000, 1, 0, 0, nullptr, 0);
    KT_CHECK(itNull.pos[0] == 0.0f && itNull.pos[1] == 0.0f && itNull.pos[2] == 0.0f);
    QueuedAction spNull = makeSpellAction(0x2000, 1, 0, 0, 0, 0, nullptr, 0, 0);
    KT_CHECK(spNull.pos[0] == 0.0f && spNull.pos[1] == 0.0f && spNull.pos[2] == 0.0f);
    KT_CHECK_EQ((int)spNull.kind, (int)ActionKind::Spell);
    testReplayArgs();

    // default-constructed action is an Item with no re-inject history
    QueuedAction def;
    KT_CHECK_EQ((int)def.kind, (int)ActionKind::Item);
    KT_CHECK_EQ(def.reInjectCount, 0);
}

void testDecideBranches() {
    ActionBuffer idle;                               // inactive, empty
    KT_CHECK(!isPartyCaster(-1)); KT_CHECK(isPartyCaster(0)); KT_CHECK(isPartyCaster(2)); KT_CHECK(!isPartyCaster(3));
    KT_CHECK_EQ((int)decideAction(idle, false, 0), (int)DECIDE_DISPATCH_NOW);   // idle + nothing queued in engine
    KT_CHECK_EQ((int)decideAction(idle, false, 1), (int)DECIDE_BUFFER);         // engine queue busy
    KT_CHECK_EQ((int)decideAction(idle, false, 3), (int)DECIDE_BUFFER);

    ActionBuffer active;                             // active, empty
    active.active = true;
    KT_CHECK_EQ((int)decideAction(active, false, 0), (int)DECIDE_BUFFER);       // active with empty buffer still buffers
    KT_CHECK_EQ((int)decideAction(active, false, 2), (int)DECIDE_BUFFER);

    ActionBuffer two;                                // count 2 is not full
    fillBuf(two, 2);
    KT_CHECK(!two.full());
    KT_CHECK_EQ((int)decideAction(two, false, 0), (int)DECIDE_BUFFER);
    KT_CHECK_EQ((int)decideAction(two, false, 1), (int)DECIDE_BUFFER);

    ActionBuffer three;                              // count 3 is full
    fillBuf(three, 3);
    KT_CHECK(three.full());
    KT_CHECK_EQ((int)decideAction(three, false, 0), (int)DECIDE_DROP_FULL);
    KT_CHECK_EQ((int)decideAction(three, false, 5), (int)DECIDE_DROP_FULL);
    KT_CHECK_EQ((int)decideAction(three, true, 0), (int)DECIDE_DISPATCH_NOW);   // combat beats full
    KT_CHECK_EQ((int)decideAction(three, true, 4), (int)DECIDE_DISPATCH_NOW);
    three.active = true;
    KT_CHECK_EQ((int)decideAction(three, false, 0), (int)DECIDE_DROP_FULL);

    KT_CHECK_EQ((int)decideAction(two, true, 3), (int)DECIDE_DISPATCH_NOW);     // combat beats engine queue
    KT_CHECK_EQ((int)decideItemAction(three, false, 0), (int)DECIDE_DROP_FULL); // wrapper matches
    KT_CHECK_EQ((int)decideItemAction(idle, false, 0), (int)DECIDE_DISPATCH_NOW);
    // haveCur: round node still running (QH4 in-flight wipe); buffer even with an empty engine queue
    KT_CHECK_EQ((int)decideAction(idle, false, 0, true), (int)DECIDE_BUFFER);
    KT_CHECK_EQ((int)decideItemAction(idle, false, 0, true), (int)DECIDE_BUFFER);   // wrapper parity
}

void testFifoMixedKinds() {
    const float pos[3] = { 0.f, 0.f, 0.f };
    ActionBuffer b;
    KT_CHECK(b.push(makeItemAction(0x1000, 301, 0, 1, pos, 1)));
    KT_CHECK(b.push(makeSpellAction(0x1000, 0x55, 2, 0, 0, 0, pos, 0, 2)));
    KT_CHECK(b.push(makeItemAction(0x1000, 302, 0, 1, pos, 3)));
    KT_CHECK_EQ(b.count, 3);
    KT_CHECK(b.full());
    KT_CHECK(!b.push(makeSpellAction(0x1000, 0x66, 0, 0, 0, 0, pos, 0, 4)));   // full: rejected
    KT_CHECK_EQ(b.count, 3);
    KT_CHECK_EQ(b.queue[2].clientItem, 302u);                                  // rejected push did not overwrite

    QueuedAction out;
    KT_CHECK(b.pop(&out));
    KT_CHECK_EQ((int)out.kind, (int)ActionKind::Item);
    KT_CHECK_EQ(out.clientItem, 301u);
    KT_CHECK(b.pop(&out));
    KT_CHECK_EQ((int)out.kind, (int)ActionKind::Spell);
    KT_CHECK_EQ(out.spellId, 0x55u);
    KT_CHECK_EQ(out.arg2, 2);
    KT_CHECK(b.pop(&out));
    KT_CHECK_EQ((int)out.kind, (int)ActionKind::Item);
    KT_CHECK_EQ(out.clientItem, 302u);
    KT_CHECK_EQ(b.count, 0);
    KT_CHECK(b.empty());
    KT_CHECK(!b.pop(&out));                                                    // pop on empty fails
    KT_CHECK(b.push(makeSpellAction(0x1000, 0x66, 0, 0, 0, 0, pos, 0, 5)));    // room again after draining
    KT_CHECK_EQ(b.count, 1);
}

void testReInjectEachCondition() {
    const float pos[3] = { 0.f, 0.f, 0.f };
    ActionBuffer base;
    base.active = true;
    base.activeAction = makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1);
    KT_CHECK(shouldReInject(base, false, true));                              // true case
    KT_CHECK(shouldReInject(base, false, true, /*paused=*/true, 999, false));  // (no gate) paused/now are ignored
    KT_CHECK(!shouldReInject(base, false, false));                            // no follow leader

    ActionBuffer inactive = base; inactive.active = false;
    KT_CHECK(!shouldReInject(inactive, false, true));                         // not active

    ActionBuffer pending = base; pending.reInjectPending = true;
    KT_CHECK(!shouldReInject(pending, false, true));                          // re-inject already pending

    ActionBuffer cancelled = base; cancelled.userCancelled = true;
    KT_CHECK(!shouldReInject(cancelled, false, true));                        // buffer-level user cancel

    KT_CHECK(!shouldReInject(base, false, true, false, 0, /*userCancelled=*/true));   // call-level user cancel
    KT_CHECK(!shouldReInject(base, /*actionInQueue=*/true, true));            // action still in engine queue

    ActionBuffer once = base; once.activeAction.reInjectCount = 1;
    KT_CHECK(shouldReInject(once, false, true));                              // 1 < kMaxReInjectCount
    ActionBuffer twice = base; twice.activeAction.reInjectCount = kMaxReInjectCount;
    KT_CHECK(!shouldReInject(twice, false, true));                            // 2 == cap -> no more re-injects
    KT_CHECK_EQ(kMaxReInjectCount, 2);
}

void testReInjectGate() {
    const float pos[3] = { 0.f, 0.f, 0.f };
    ActionBuffer base;
    base.active = true; base.activeStartTick = 1000;
    base.activeAction = makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1);
    base.activeAction.reInjectCount = 0;
    RiGate g; g.on = true; g.pauseEatenWait = true; g.pauseReinject = true;
    KT_CHECK_EQ((int)reInjectWhy(base, false, true, true, 2000, false, g), (int)RI_PAUSED);
    RiGate g2 = g; g2.pauseEatenWait = false;
    KT_CHECK_EQ((int)reInjectWhy(base, false, true, true, 2000, false, g2), (int)RI_OK);
    KT_CHECK_EQ((int)reInjectWhy(base, false, true, false, 1500, false, g), (int)RI_SETTLE);
    KT_CHECK_EQ((int)reInjectWhy(base, false, true, false, 1501, false, g), (int)RI_OK);
    ActionBuffer sp = base; sp.lastReInjectTick = 1800;
    KT_CHECK_EQ((int)reInjectWhy(sp, false, true, false, 2049, false, g), (int)RI_SPACING);
    KT_CHECK_EQ((int)reInjectWhy(sp, false, true, false, 2050, false, g), (int)RI_OK);
    RiGate g3 = g; g3.pauseReinject = false;
    KT_CHECK_EQ((int)reInjectWhy(base, false, true, false, 2000, false, g3), (int)RI_OFF);
    ActionBuffer df = base; df.dispatchDeferred = true;
    KT_CHECK_EQ((int)reInjectWhy(df, false, true, false, 2000, false, g), (int)RI_DEFERRED);
    KT_CHECK(!shouldReInject(df, false, true));
    KT_CHECK_EQ((int)reInjectWhy(base, false, false, false, 2000, false, g), (int)RI_NO_LEADER);
    KT_CHECK_EQ((int)reInjectWhy(base, true, true, true, 1001, false, g), (int)RI_IN_QUEUE);   // the gate never masks it
}

void testDeferStep() {
    ActionBuffer b; b.dispatchDeferred = true; b.deferTick = 1000; b.deferLastTry = 1000;
    KT_CHECK_EQ((int)deferStep(b, false, 1100), (int)DEFER_WAIT);
    KT_CHECK_EQ(b.deferUnpausedMs, 100u);
    KT_CHECK_EQ((int)deferStep(b, false, 1250), (int)DEFER_RETRY);
    KT_CHECK_EQ((int)deferStep(b, true, 1300), (int)DEFER_WAIT);
    KT_CHECK_EQ(b.deferUnpausedMs, 250u);                  // 100 + 150, the paused step added nothing
    ActionBuffer g; g.deferUnpausedMs = 4900; g.deferTick = 1000; g.deferLastTry = 1000;
    KT_CHECK_EQ((int)deferStep(g, false, 1100), (int)DEFER_GIVE_UP);
    ActionBuffer h; h.deferUnpausedMs = 4899; h.deferTick = 1000; h.deferLastTry = 1000;
    KT_CHECK(deferStep(h, false, 1100) != DEFER_GIVE_UP);
    ActionBuffer c; c.deferTick = 1000; c.deferLastTry = 1000;
    deferStep(c, false, 9000);
    KT_CHECK_EQ(c.deferUnpausedMs, 1000u);                 // dt cap
    ActionBuffer w; w.deferTick = 0xFFFFFFF0u; w.deferLastTry = 0xFFFFFFF0u;
    deferStep(w, false, 0x10u);
    KT_CHECK_EQ(w.deferUnpausedMs, 32u);                   // wrap-safe
}

void testEdgeHeadAction() {
    KT_CHECK_EQ((int)edgeHeadAction(false, false, false, false, 900, true), (int)EDGE_NONE);
    KT_CHECK_EQ((int)edgeHeadAction(true, false, false, false, 900, true), (int)EDGE_RESEND);
    KT_CHECK_EQ((int)edgeHeadAction(true, false, false, false, 80, true), (int)EDGE_WAIT);
    KT_CHECK_EQ((int)edgeHeadAction(true, false, false, false, 500, true), (int)EDGE_WAIT);
    KT_CHECK_EQ((int)edgeHeadAction(true, false, false, false, 501, true), (int)EDGE_RESEND);
    KT_CHECK_EQ((int)edgeHeadAction(true, false, true, false, 80, true), (int)EDGE_DROP_EXEC);
    KT_CHECK_EQ((int)edgeHeadAction(true, false, false, true, 900, true), (int)EDGE_DROP_EXEC);
    KT_CHECK_EQ((int)edgeHeadAction(true, true, true, true, 900, true), (int)EDGE_WAIT);     // held path decides
    KT_CHECK_EQ((int)edgeHeadAction(true, false, true, true, 900, false), (int)EDGE_WAIT);   // gather failed
    KT_CHECK_EQ((int)edgeHeadAction(true, false, false, false, 80, true, true), (int)EDGE_RESEND);   // deferred
}

void testDetachableMask() {
    QueuedAction sp = makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, nullptr, 0, 1);
    { const RNode rn[] = { N(9, 1, 0x55) }; KT_CHECK_EQ(detachableMask(rn, 1, sp), 0u); }
    { const RNode rn[] = { N(9, 1, 0x55), N(9, 2, 0x66) }; KT_CHECK_EQ(detachableMask(rn, 2, sp), 2u); }
    { const RNode rn[] = { N(9, 1, 0x66), N(9, 2, 0x55) }; KT_CHECK_EQ(detachableMask(rn, 2, sp), 1u); }
    { const RNode rn[] = { N(9, 1, 0x55), N(9, 2, 0x55) }; KT_CHECK_EQ(detachableMask(rn, 2, sp), 2u); }   // only the first is kept
    QueuedAction it = makeItemAction(0x1000, 0x80000860u, 0, 0, nullptr, 1);
    { const RNode rn[] = { N(10, 1, 0, 0x00000860) }; KT_CHECK_EQ(detachableMask(rn, 1, it), 0u); }
    { const RNode rn[] = { N(10, 1, 0, 0x860), N(1, 2) }; KT_CHECK_EQ(detachableMask(rn, 2, it), 2u); }
    { const RNode rn[] = { N(1, 1), N(10, 2, 0, 0x860) }; KT_CHECK_EQ(detachableMask(rn, 2, it), 1u); }
    { const RNode rn[] = { N(10, 1, 0, 0x861) }; KT_CHECK_EQ(detachableMask(rn, 1, it), 1u); }
    { const RNode rn[] = { N(9, 1, 0x55) }; KT_CHECK_EQ(detachableMask(rn, 0, sp), 0u); }
    KT_CHECK_EQ(detachableMask(nullptr, 2, sp), 0u);
    { const RNode rn[] = { N(1, 1), N(1, 2), N(1, 3), N(1, 4) }; KT_CHECK_EQ(detachableMask(rn, 4, sp), 15u); }
}

// BG23: any spell/item in the engine suppresses re-inject (intended, doc 30 QH4); the suppression is transient.
void testReInjectForeignCastSuppressionTransient() {
    const float pos[3] = { 0.f, 0.f, 0.f };
    ActionBuffer a;
    a.active = true;
    a.activeAction = makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1);
    // Our action A was eaten (follow-leader wiped it) while an unrelated spell B holds the engine.
    KT_CHECK(!shouldReInject(a, /*actionInQueue=*/true, /*hasFollowLeader=*/true));
    KT_CHECK_EQ((int)reInjectWhy(a, true, true), (int)RI_IN_QUEUE);
    // B ended without ever being seen executing: suppression lifts, A is re-injected (no lost action).
    KT_CHECK(!a.sawExecuting);
    KT_CHECK(shouldReInject(a, /*actionInQueue=*/false, /*hasFollowLeader=*/true));
    KT_CHECK_EQ((int)reInjectWhy(a, false, true), (int)RI_OK);
}

void testReInjectNotAfterExecuting() {
    const float pos[3] = { 0.f, 0.f, 0.f };
    ActionBuffer b;
    b.active = true;
    b.activeAction = makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1);
    KT_CHECK(shouldReInject(b, false, true));
    b.sawExecuting = true;                                                    // the engine showed it executing: finished, not eaten
    KT_CHECK(!shouldReInject(b, false, true));
    KT_CHECK(reInjectWhy(b, false, true) == RI_EXECUTED);
    b.markDispatched(10);                                                     // a fresh dispatch resets the flag
    KT_CHECK(shouldReInject(b, false, true));
    b.sawExecuting = true; b.cancel();
    KT_CHECK(!b.sawExecuting);
    b.sawExecuting = true; b.clear();
    KT_CHECK(!b.sawExecuting);

    // reInjectWhy == RI_OK <=> shouldReInject across every sawExecuting/inQueue/leader combination
    for (int m = 0; m < 8; ++m) {
        ActionBuffer c;
        c.active = true;
        c.activeAction = makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1);
        c.sawExecuting = (m & 1) != 0;
        const bool inQ = (m & 2) != 0, lead = (m & 4) != 0;
        KT_CHECK((reInjectWhy(c, inQ, lead) == RI_OK) == shouldReInject(c, inQ, lead));
        if (m == 4) KT_CHECK(shouldReInject(c, inQ, lead));                  // only leader-queued, not executing, not in queue
    }
}

void testDispatchNextEachCondition() {
    const float pos[3] = { 0.f, 0.f, 0.f };
    ActionBuffer b;
    KT_CHECK(b.push(makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1)));
    KT_CHECK(shouldDispatchNext(b, 0));                                        // all clear
    KT_CHECK(shouldDispatchNext(b, 0, false, 0));

    ActionBuffer busy = b; busy.active = true;
    KT_CHECK(!shouldDispatchNext(busy, 0));                                    // active alone blocks

    ActionBuffer drained;
    KT_CHECK(!shouldDispatchNext(drained, 0));                                 // empty alone blocks

    KT_CHECK(!shouldDispatchNext(b, 1));                                       // engine queue non-empty alone blocks

    KT_CHECK(!shouldDispatchNext(b, 0, /*haveCur=*/true, 0));                  // current action alone blocks

    KT_CHECK(!shouldDispatchNext(b, 0, false, /*nrn=*/1));                     // nrn alone blocks
}

// ---- T1: why-codes, queue signature, lost-command ids, LOG GRAMMAR formatters ----
void testWhyParityMatrix() {
    const float pos[3] = { 0.f, 0.f, 0.f };
    int reHits[8] = { 0 };
    for (int active = 0; active < 2; ++active)
    for (int pend = 0; pend < 2; ++pend)
    for (int bcan = 0; bcan < 2; ++bcan)
    for (int rc = 1; rc <= 3; ++rc)
    for (int inq = 0; inq < 2; ++inq)
    for (int lead = 0; lead < 2; ++lead)
    for (int pcan = 0; pcan < 2; ++pcan)
    for (int paused = 0; paused < 2; ++paused)
    for (int nowSet = 0; nowSet < 2; ++nowSet)
    for (int ex = 0; ex < 2; ++ex) {
        ActionBuffer b;
        b.active = active != 0;
        b.sawExecuting = ex != 0;   // drives RI_EXECUTED
        b.reInjectPending = pend != 0;
        b.userCancelled = bcan != 0;
        b.activeAction = makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1);
        b.activeAction.reInjectCount = rc;
        const uint32_t now = nowSet ? 999u : 0u;
        const bool want = shouldReInject(b, inq != 0, lead != 0, paused != 0, now, pcan != 0);
        const ReInjectWhy why = reInjectWhy(b, inq != 0, lead != 0, paused != 0, now, pcan != 0);
        KT_CHECK_EQ(why == RI_OK, want);
        reHits[why]++;
    }
    for (int r = 0; r < 8; ++r) KT_CHECK(reHits[r] > 0);   // every ReInjectWhy is reachable in the matrix

    int dsHits[7] = { 0 };
    for (int active = 0; active < 2; ++active)
    for (int isEmpty = 0; isEmpty < 2; ++isEmpty)
    for (int eng = 0; eng < 3; ++eng)
    for (int cur = 0; cur < 2; ++cur)
    for (int nrn = -1; nrn <= 1; ++nrn)
    for (int fh = 0; fh < 2; ++fh) {
        ActionBuffer b;
        if (!isEmpty) KT_CHECK(b.push(makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1)));
        b.active = active != 0;
        // shouldDispatchNext has no flagHold input (the flag gate is FlagGate); DS_FLAG_BUSY is the one extra reason after it passes
        const bool want = shouldDispatchNext(b, eng, cur != 0, nrn) && fh == 0;
        const DispatchWhy why = dispatchWhy(b, eng, cur != 0, nrn, fh != 0);   // fh drives DS_FLAG_BUSY
        KT_CHECK_EQ(why == DS_OK, want);
        dsHits[why]++;
    }
    for (int r = 0; r < 7; ++r) KT_CHECK(dsHits[r] > 0);   // every DispatchWhy is reachable in the matrix
}

void testWhyNames() {
    KT_CHECK_EQ(whyName(RI_OK), "ok");
    KT_CHECK_EQ(whyName(RI_INACTIVE), "inactive");
    KT_CHECK_EQ(whyName(RI_PENDING), "pending");
    KT_CHECK_EQ(whyName(RI_CANCEL), "cancel");
    KT_CHECK_EQ(whyName(RI_MAX), "max_reinject");
    KT_CHECK_EQ(whyName(RI_IN_QUEUE), "in_queue");
    KT_CHECK_EQ(whyName(RI_NO_LEADER), "no_leader");
    KT_CHECK_EQ(whyName(RI_EXECUTED), "executed");
    KT_CHECK_EQ(whyName(RI_DEFERRED), "deferred");
    KT_CHECK_EQ(whyName(RI_OFF), "off");
    KT_CHECK_EQ(whyName(RI_PAUSED), "paused");
    KT_CHECK_EQ(whyName(RI_SETTLE), "settle");
    KT_CHECK_EQ(whyName(RI_SPACING), "spacing");
    KT_CHECK_EQ(whyName(DR_REFUSED), "refused");
    KT_CHECK_EQ(whyName(DR_EXECUTED), "executed");
    KT_CHECK_EQ(whyName(DR_PRESENT_TIMEOUT), "present_timeout");
    KT_CHECK_EQ(whyName(DS_OK), "ok");
    KT_CHECK_EQ(whyName(DS_ACTIVE), "active");
    KT_CHECK_EQ(whyName(DS_EMPTY), "empty");
    KT_CHECK_EQ(whyName(DS_ENGINE_BUSY), "engine_busy");
    KT_CHECK_EQ(whyName(DS_CUR_BUSY), "cur_busy");
    KT_CHECK_EQ(whyName(DS_NRN_BUSY), "nrn_busy");
    KT_CHECK_EQ(whyName(DS_FLAG_BUSY), "flags");
    // DROP words are the grammar set verbatim
    KT_CHECK_EQ(whyName(DR_COMBAT_CLEAR), "combat_clear");
    KT_CHECK_EQ(whyName(DR_NULL_CREATURE), "null_creature");
    KT_CHECK_EQ(whyName(DR_CANCEL), "cancel");
    KT_CHECK_EQ(whyName(DR_BUFFER_FULL), "buffer_full");
    KT_CHECK_EQ(whyName(DR_UNREADABLE_CREATURE), "unreadable_creature");
    KT_CHECK_EQ(whyName(DR_REINJECT_EXHAUSTED), "reinject_exhausted");
    KT_CHECK_EQ(whyName(DR_EATEN_TIMEOUT), "eaten_timeout");
    KT_CHECK_EQ(whyName(DR_IDENTITY_LOST), "identity_lost");
}

void testQueueSig() {
    QEntry q[10];
    for (int i = 0; i < 10; ++i) { q[i].type = 0x0C + i; q[i].param0 = (uint32_t)i; }
    const QueueSig a = makeQueueSig(q, 3, 1, 0x0C, true, false);
    KT_CHECK_EQ(a.ne, 3); KT_CHECK_EQ(a.nTypes, 3); KT_CHECK_EQ(a.types[2], 0x0E);
    KT_CHECK(sigEqual(a, makeQueueSig(q, 3, 1, 0x0C, true, false)));
    KT_CHECK(!sigEqual(a, makeQueueSig(q, 4, 1, 0x0C, true, false)));    // ne
    KT_CHECK(!sigEqual(a, makeQueueSig(q, 3, 2, 0x0C, true, false)));    // round
    KT_CHECK(!sigEqual(a, makeQueueSig(q, 3, 1, 0x0D, true, false)));    // cur
    KT_CHECK(!sigEqual(a, makeQueueSig(q, 3, 1, 0x0C, false, false)));   // aa8
    KT_CHECK(!sigEqual(a, makeQueueSig(q, 3, 1, 0x0C, true, true)));     // ab8
    QEntry r[3] = { q[0], q[1], q[2] };
    r[1].type = 0x3F;
    KT_CHECK(!sigEqual(a, makeQueueSig(r, 3, 1, 0x0C, true, false)));    // a stored type changed
    // 8-type cap: 10 entries keep 8 types, ne still counts 10
    const QueueSig big = makeQueueSig(q, 10, 1, -1, false, false);
    KT_CHECK_EQ(big.ne, 10); KT_CHECK_EQ(big.nTypes, 8);
    KT_CHECK(!sigEqual(big, makeQueueSig(q, 9, 1, -1, false, false)));  // ne differs
    QEntry t[10];
    for (int i = 0; i < 10; ++i) t[i] = q[i];
    t[9].type = 0x99;                                                    // past the cap: not compared
    KT_CHECK(sigEqual(big, makeQueueSig(t, 10, 1, -1, false, false)));
    t[7].type = 0x99;                                                    // inside the cap: compared
    KT_CHECK(!sigEqual(big, makeQueueSig(t, 10, 1, -1, false, false)));
    // null queue keeps ne but no types; default signature
    const QueueSig nq = makeQueueSig(nullptr, 2, 0, -1, false, false);
    KT_CHECK_EQ(nq.ne, 2); KT_CHECK_EQ(nq.nTypes, 0);
    const QueueSig d{};
    KT_CHECK_EQ(d.ne, 0); KT_CHECK_EQ(d.cur, -1); KT_CHECK(!d.aa8 && !d.ab8 && sigEqual(d, QueueSig{}));
}

void testBufferedCmdIdsAndSeen() {
    const float pos[3] = { 0.f, 0.f, 0.f };
    ActionBuffer b;
    KT_CHECK(b.push(makeItemAction(0x1000, 301, 0, 1, pos, 1, 11)));
    KT_CHECK(b.push(makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1, 12)));
    b.active = true;
    b.activeAction = makeItemAction(0x1000, 9, 0, 1, pos, 1, 13);
    uint32_t ids[4] = { 0, 0, 0, 0 };
    KT_CHECK_EQ(bufferedCmdIds(b, ids, 4), 3);                 // two buffered + the active one
    KT_CHECK(ids[0] == 11u && ids[1] == 12u && ids[2] == 13u);
    b.active = false;
    KT_CHECK_EQ(bufferedCmdIds(b, ids, 4), 2);                 // inactive: active id not listed
    b.active = true;
    KT_CHECK_EQ(bufferedCmdIds(b, ids, 2), 2);                 // cap: stops after 11, 12
    KT_CHECK(ids[0] == 11u && ids[1] == 12u);
    ActionBuffer e;
    KT_CHECK_EQ(bufferedCmdIds(e, ids, 4), 0);
    // cmdId: trailing parameter, default 0 keeps the legacy positional calls unchanged
    KT_CHECK_EQ(makeItemAction(0x1000, 301, 0, 1, pos, 1).cmdId, 0u);
    KT_CHECK_EQ(makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1).cmdId, 0u);
    KT_CHECK_EQ(makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pos, 0, 1, 77).cmdId, 77u);
    KT_CHECK_EQ(makeItemAction(0x1000, 301, 0, 1, pos, 1, 42).cmdId, 42u);
    // seenInQueue is reset by cancel() and clear() only
    ActionBuffer s;
    KT_CHECK(!s.seenInQueue);
    s.seenInQueue = true; s.cancel(); KT_CHECK(!s.seenInQueue);
    s.seenInQueue = true; s.clear(); KT_CHECK(!s.seenInQueue);
}

void testTraceQueueIni() {
    Config c;
    KT_CHECK(c.traceQueue);                                            // default on
    KT_CHECK(parseIniLine(c, "trace_queue=0")); KT_CHECK(!c.traceQueue);
    KT_CHECK(parseIniLine(c, "trace_queue=1")); KT_CHECK(c.traceQueue);
    KT_CHECK(parseIniLine(c, "Trace_Queue = off")); KT_CHECK(!c.traceQueue);
    KT_CHECK(parseIniLine(c, "trace_queue=yes")); KT_CHECK(c.traceQueue);
    KT_CHECK(!parseIniLine(c, "trace_queue=x")); KT_CHECK(c.traceQueue);   // bad value keeps the previous value
    Config d;
    KT_CHECK(!parseIniLine(d, "trace_queue=")); KT_CHECK(d.traceQueue);
    Config e;
    KT_CHECK(parseIniLine(e, "log=0")); KT_CHECK(e.traceQueue);            // independent of log=
    KT_CHECK(parseIniLine(e, "trace_queue=0")); KT_CHECK(!e.traceQueue && !e.log);   // each key sets only its own field
}

void testLogFormatGolden() {
    char out[kLogLineMax];
    CmdLine c;
    c.cmdId = 12; c.m = 0; c.cr = 0x1a2b3c4u; c.kind = (int)ActionKind::Item;
    c.hasId = true; c.id = 0x12d; c.hasTgt = false; c.dec = "buffer";
    c.fb = false; c.engq = 0; c.buf = 2; c.paused = false; c.combat = false;
    fmtCmd(out, sizeof out, c);
    KT_CHECK_EQ(out, "CMD#12 m=0 cr=1a2b3c4 kind=item id=12d tgt=- dec=buffer fb=0 engq=0 buf=2 paused=0 combat=0");
    CmdLine d;
    d.cmdId = 7; d.m = -1; d.cr = 0; d.kind = (int)ActionKind::Spell;
    d.hasId = true; d.id = 0x55; d.hasTgt = true; d.tgt = 0x7f000001u;
    d.dec = "pass_unreadable"; d.fb = true; d.engq = -1; d.buf = 0; d.paused = true; d.combat = true;
    fmtCmd(out, sizeof out, d);
    KT_CHECK_EQ(out, "CMD#7 m=? cr=0 kind=spell id=55 tgt=7f000001 dec=pass_unreadable fb=1 engq=unreadable buf=0 paused=1 combat=1");
    CmdLine e;
    e.cmdId = 4294967295u; e.m = 2; e.cr = 0xffffffffu; e.kind = 5; e.dec = "drop_full"; e.engq = 3; e.buf = 3;
    fmtCmd(out, sizeof out, e);
    KT_CHECK_EQ(out, "CMD#4294967295 m=2 cr=ffffffff kind=? id=- tgt=- dec=drop_full fb=0 engq=3 buf=3 paused=0 combat=0");

    CmdLine f;
    f.cmdId = 9; f.m = -1; f.cr = 0x1a2b; f.kind = (int)ActionKind::Spell;
    f.hasId = true; f.id = 0x55; f.hasTgt = true; f.tgt = 7;
    f.dec = "pass_nonparty"; f.fb = true; f.engq = 0; f.buf = 0; f.paused = false; f.combat = false;
    fmtCmd(out, sizeof out, f);
    KT_CHECK_EQ(out, "CMD#9 m=? cr=1a2b kind=spell id=55 tgt=7 dec=pass_nonparty fb=1 engq=0 buf=0 paused=0 combat=0");

    fmtWait(out, sizeof out, 5, 0, DS_ENGINE_BUSY, 2, 1, 120);
    KT_CHECK_EQ(out, "WAIT cmd=5 m=0 why=engine_busy buf=2 engNe=1 age=120");
    fmtWait(out, sizeof out, 6, 2, DS_NRN_BUSY, 0, 0, 0);
    KT_CHECK_EQ(out, "WAIT cmd=6 m=2 why=nrn_busy buf=0 engNe=0 age=0");

    fmtResolve(out, sizeof out, 5, 0, true, 1, 3400);
    KT_CHECK_EQ(out, "RESOLVE cmd=5 m=0 seen=1 reinj=1 age=3400");
    fmtResolve(out, sizeof out, 6, -1, false, 0, 0);
    KT_CHECK_EQ(out, "RESOLVE cmd=6 m=? seen=0 reinj=0 age=0");

    fmtDrop(out, sizeof out, 1, 1, DR_COMBAT_CLEAR, 2);
    KT_CHECK_EQ(out, "DROP cmd=1 m=1 why=combat_clear n=2");
    fmtDrop(out, sizeof out, 2, 0, DR_NULL_CREATURE, 1);
    KT_CHECK_EQ(out, "DROP cmd=2 m=0 why=null_creature n=1");
    fmtDrop(out, sizeof out, 3, 0, DR_CANCEL, 1);
    KT_CHECK_EQ(out, "DROP cmd=3 m=0 why=cancel n=1");
    fmtDrop(out, sizeof out, 5, 2, DR_BUFFER_FULL, 1);
    KT_CHECK_EQ(out, "DROP cmd=5 m=2 why=buffer_full n=1");
    fmtDrop(out, sizeof out, 9, 0, DR_UNREADABLE_CREATURE, 3);
    KT_CHECK_EQ(out, "DROP cmd=9 m=0 why=unreadable_creature n=3");
    fmtDrop(out, sizeof out, 11, 1, DR_REINJECT_EXHAUSTED, 1);
    KT_CHECK_EQ(out, "DROP cmd=11 m=1 why=reinject_exhausted n=1");
    fmtDrop(out, sizeof out, 12, 2, DR_EATEN_TIMEOUT, 2);
    KT_CHECK_EQ(out, "DROP cmd=12 m=2 why=eaten_timeout n=2");
    fmtDrop(out, sizeof out, 13, 0, DR_IDENTITY_LOST, 1);
    KT_CHECK_EQ(out, "DROP cmd=13 m=0 why=identity_lost n=1");

    // T1b vanish / handoff / stale-cur / identity-swap grammar
    KT_CHECK_EQ(vanishKindName(VK_EXECUTED), "executed");
    KT_CHECK_EQ(vanishKindName(VK_UNSEEN), "unseen");
    KT_CHECK_EQ(vanishKindName(VK_EATEN_PAUSED), "eaten_paused");
    KT_CHECK_EQ(vanishKindName(VK_EATEN_WAIT), "eaten_wait");
    fmtVanish(out, sizeof out, 3, 0, VK_EATEN_PAUSED, true, false, 0, 0, 0, false, false, 1592);
    KT_CHECK_EQ(out, "VANISH cmd=3 m=0 kind=eaten_paused paused=1 exec=0 ne=0 round=0 cur=0 aa8=0 ab8=0 age=1592");
    fmtVanish(out, sizeof out, 4, -1, VK_EXECUTED, false, true, 2, 7, 0x3F, true, false, 0);
    KT_CHECK_EQ(out, "VANISH cmd=4 m=? kind=executed paused=0 exec=1 ne=2 round=7 cur=3f aa8=1 ab8=0 age=0");
    fmtVanish(out, sizeof out, 5, 1, VK_UNSEEN, false, false, 0, 0, -1, false, false, 300);
    KT_CHECK_EQ(out, "VANISH cmd=5 m=1 kind=unseen paused=0 exec=0 ne=0 round=0 cur=- aa8=0 ab8=0 age=300");
    const uint32_t hIds[2] = { 6, 7 };
    fmtHandoff(out, sizeof out, 0, true, 5, hIds, 2);
    KT_CHECK_EQ(out, "HANDOFF m=0 active=5 buffered=2 cmds=6,7");
    fmtHandoff(out, sizeof out, 1, false, 0, hIds, 2);
    KT_CHECK_EQ(out, "HANDOFF m=1 active=- buffered=2 cmds=6,7");
    fmtHandoff(out, sizeof out, 0, false, 0, nullptr, 0);
    KT_CHECK_EQ(out, "HANDOFF m=0 active=- buffered=0 cmds=-");
    fmtHandoffOne(out, sizeof out, 6, 0, 1, 2);
    KT_CHECK_EQ(out, "HANDOFF_ONE cmd=6 m=0 seq=1/2");
    fmtStaleCur(out, sizeof out, 0, 1, 0, false, false, true);
    KT_CHECK_EQ(out, "STALE_CUR m=0 sub=1 since=0 aa8=0 ab8=0 paused=1");
    fmtIdentSwap(out, sizeof out, 0, 2);
    KT_CHECK_EQ(out, "IDENT_SWAP from=0 to=2");

    const int edgeBuf[3] = { 3, 0, 1 };
    fmtEdge(out, sizeof out, "combat", false, true, "hud", edgeBuf, 3);
    KT_CHECK_EQ(out, "EDGE what=combat old=0 new=1 src=hud buf=3,0,1");
    fmtEdge(out, sizeof out, "paused", true, false, "unreadable", nullptr, 0);
    KT_CHECK_EQ(out, "EDGE what=paused old=1 new=0 src=unreadable buf=");

    QEntry q2[2] = { { 0x0C, 0 }, { 0x3F, 0 } };
    const QueueSig s2 = makeQueueSig(q2, 2, 1, -1, false, true);
    fmtQj(out, sizeof out, 0, s2, false, false, "cmd5", 120);
    KT_CHECK_EQ(out, "QJ m=0 ne=2 types=[c,3f] round=1 cur=- aa8=0 ab8=1 paused=0 combat=0 ours=cmd5@120ms");
    const QueueSig s3 = makeQueueSig(q2, 2, 3, 0x3F, true, false);
    fmtQj(out, sizeof out, 1, s3, true, true, nullptr, 0);
    KT_CHECK_EQ(out, "QJ m=1 ne=2 types=[c,3f] round=3 cur=3f aa8=1 ab8=0 paused=1 combat=1 ours=external");
    QEntry q10[10];
    for (int i = 0; i < 10; ++i) { q10[i].type = 0x0C + i; q10[i].param0 = 0; }
    const QueueSig s10 = makeQueueSig(q10, 10, 1, -1, false, false);
    fmtQj(out, sizeof out, 2, s10, false, false, "x", 5);
    KT_CHECK_EQ(out, "QJ m=2 ne=10 types=[c,d,e,f,10,11,12,13,+2] round=1 cur=- aa8=0 ab8=0 paused=0 combat=0 ours=x@5ms");
    const QueueSig s0;
    fmtQj(out, sizeof out, 0, s0, false, false, nullptr, 0);
    KT_CHECK_EQ(out, "QJ m=0 ne=0 types=[] round=0 cur=- aa8=0 ab8=0 paused=0 combat=0 ours=external");

    // MODE: (a) out of combat, soft/hostile/party unknown
    ModeState ma;
    ma.combat = 0; ma.menu = 0; ma.paused = 0; ma.strip = "shown"; ma.cancel = "shown"; ma.banner = "vanilla";
    fmtMode(out, sizeof out, 0, ma, "ui");
    KT_CHECK_EQ(out, "MODE m=0 combat=0 soft=- menu=0 paused=0 hostile=? party=? strip=shown cancel=shown banner=vanilla why=ui");
    // (b) combat edge, soft and hostile known
    ModeState mb;
    mb.combat = 1; mb.soft = 1; mb.menu = 0; mb.paused = 0; mb.hostile = 0; mb.party = 1; mb.strip = "shown"; mb.cancel = "shown"; mb.banner = "hidden";
    const char* goldB = "MODE m=2 combat=1 soft=1 menu=0 paused=0 hostile=0 party=1 strip=shown cancel=shown banner=hidden why=edge";
    fmtMode(out, sizeof out, 2, mb, "edge");
    KT_CHECK_EQ(out, goldB);
    // (c) soft target, hidden strip/cancel, banner restored
    ModeState mc;
    mc.combat = 1; mc.soft = 0; mc.menu = 0; mc.paused = 0; mc.hostile = 1; mc.party = 0; mc.strip = "hidden:combat"; mc.cancel = "hidden:combat"; mc.banner = "restored";
    fmtMode(out, sizeof out, 1, mc, "target");
    KT_CHECK_EQ(out, "MODE m=1 combat=1 soft=0 menu=0 paused=0 hostile=1 party=0 strip=hidden:combat cancel=hidden:combat banner=restored why=target");
    // (d) combat unknown (-1) prints '?'; unknown m prints '?'
    ModeState md;
    md.strip = "hidden:stale"; md.cancel = "hidden:off"; md.banner = "hidden";
    fmtMode(out, sizeof out, -1, md, "ui");
    KT_CHECK_EQ(out, "MODE m=? combat=? soft=- menu=0 paused=0 hostile=? party=? strip=hidden:stale cancel=hidden:off banner=hidden why=ui");
    // (e) null reason strings print '?'
    ModeState me;
    me.combat = 1; me.strip = nullptr; me.cancel = nullptr; me.banner = nullptr;
    fmtMode(out, sizeof out, 3, me, nullptr);
    KT_CHECK_EQ(out, "MODE m=3 combat=1 soft=- menu=0 paused=0 hostile=? party=? strip=? cancel=? banner=? why=?");
    // truncation: a small cap keeps a NUL-terminated prefix and returns the full length; null buffer writes nothing
    char tinyM[16];
    KT_CHECK_EQ(fmtMode(tinyM, sizeof tinyM, 2, mb, "edge"), (int)strlen(goldB));
    KT_CHECK_EQ(strlen(tinyM), (size_t)15);
    KT_CHECK(strncmp(tinyM, "MODE m=2 combat", 15) == 0);
    KT_CHECK_EQ(fmtMode(nullptr, 0, 2, mb, "edge"), (int)strlen(goldB));
}

// modeChanged truth table, modeWhy priority order.
void testModeChangedWhy() {
    ModeState base;
    base.combat = 0; base.soft = 0; base.menu = 0; base.paused = 0; base.hostile = 0; base.party = 0;
    base.strip = "shown"; base.cancel = "shown"; base.banner = "vanilla";
    KT_CHECK(!modeChanged(base, base));
    const ModeState same = base;
    KT_CHECK(!modeChanged(base, same));
    // each of the 9 fields flipped alone reads as a change
    ModeState f;
    f = base; f.combat = 1;             KT_CHECK(modeChanged(base, f));
    f = base; f.soft = 1;               KT_CHECK(modeChanged(base, f));
    f = base; f.menu = 1;               KT_CHECK(modeChanged(base, f));
    f = base; f.paused = 1;             KT_CHECK(modeChanged(base, f));
    f = base; f.hostile = 1;            KT_CHECK(modeChanged(base, f));
    f = base; f.party = 1;              KT_CHECK(modeChanged(base, f));
    f = base; f.strip = "hidden:combat"; KT_CHECK(modeChanged(base, f));
    f = base; f.cancel = "hidden:menu";  KT_CHECK(modeChanged(base, f));
    f = base; f.banner = "restored";     KT_CHECK(modeChanged(base, f));
    // reason strings compare by content: two distinct buffers with equal text are unchanged; nullptr reads as ""
    char a1[] = "shown", b1[] = "shown";
    ModeState pa = base, pb = base;
    pa.strip = a1; pb.strip = b1; pa.cancel = a1; pb.cancel = b1; pa.banner = a1; pb.banner = b1;
    KT_CHECK(!modeChanged(pa, pb));
    ModeState nn = base; nn.strip = nullptr;
    ModeState ee = base; ee.strip = "";
    KT_CHECK(!modeChanged(nn, ee));
    KT_CHECK(modeChanged(nn, base));

    // modeWhy: first changed field in priority order
    ModeState c1 = base; c1.combat = 1;
    KT_CHECK_EQ(modeWhy(base, c1), "edge");
    KT_CHECK_EQ(modeWhy(c1, base), "ended");
    ModeState cm = base; cm.combat = 1; cm.menu = 1;
    KT_CHECK_EQ(modeWhy(base, cm), "edge");
    ModeState m1 = base; m1.menu = 1;
    KT_CHECK_EQ(modeWhy(base, m1), "menu");
    ModeState p1 = base; p1.paused = 1;
    KT_CHECK_EQ(modeWhy(base, p1), "pause");
    ModeState s1 = base; s1.soft = 1;
    KT_CHECK_EQ(modeWhy(base, s1), "target");
    ModeState h1 = base; h1.hostile = 1;
    KT_CHECK_EQ(modeWhy(base, h1), "hostile");
    ModeState y1 = base; y1.party = 1;
    KT_CHECK_EQ(modeWhy(base, y1), "hostile");
    ModeState t1 = base; t1.strip = "hidden:combat";
    KT_CHECK_EQ(modeWhy(base, t1), "ui");
    KT_CHECK_EQ(modeWhy(base, base), "ui");
}

// trace_mode / mode_cap: defaults, bool and integer parsing; a bad value keeps the previous value.
void testTraceModeIni() {
    Config d;
    KT_CHECK(d.traceMode);
    KT_CHECK_EQ(d.modeCap, 2000);
    Config c;
    KT_CHECK(parseIniLine(c, "trace_mode=0") && !c.traceMode);
    KT_CHECK(parseIniLine(c, "trace_mode=1") && c.traceMode);
    KT_CHECK(!parseIniLine(c, "trace_mode=2") && c.traceMode);
    KT_CHECK(parseIniLine(c, "mode_cap=50") && c.modeCap == 50);
    KT_CHECK(!parseIniLine(c, "mode_cap=0") && c.modeCap == 50);
    KT_CHECK(!parseIniLine(c, "mode_cap=abc") && c.modeCap == 50);
    KT_CHECK(parseIniLine(c, "mode_cap=1") && c.modeCap == 1);
    KT_CHECK(parseIniLine(c, "mode_cap=100000") && c.modeCap == 100000);
    KT_CHECK(!parseIniLine(c, "mode_cap=100001") && c.modeCap == 100000);
    Config f;
    parseIni(f, "trace_mode=0\r\nmode_cap=50\r\n");
    KT_CHECK(!f.traceMode && f.modeCap == 50);
}

void testLogFormatBounds() {
    char out[kLogLineMax];
    // worst-case QJ: 8 full-width types, ne 16, every flag set, 16-char tag, max age
    QEntry w[8];
    for (int i = 0; i < 8; ++i) w[i].type = -1;
    const QueueSig ws = makeQueueSig(w, 16, 99, 0x7fffffff, true, true);
    const int n = fmtQj(out, sizeof out, 2, ws, true, true, "abcdefghijklmnop", 4294967295u);
    KT_CHECK_EQ(out, "QJ m=2 ne=16 types=[ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,+8] round=99 cur=7fffffff aa8=1 ab8=1 paused=1 combat=1 ours=abcdefghijklmnop@4294967295ms");
    KT_CHECK(n < kLogLineMax && strlen(out) < (size_t)kLogLineMax);
    // over-long tag: output clamped to 199 chars + NUL, return value still reports the full length
    char longTag[301];
    memset(longTag, 'x', 300); longTag[300] = '\0';
    const int nl = fmtQj(out, sizeof out, 0, ws, false, false, longTag, 1);
    KT_CHECK(nl > kLogLineMax);
    KT_CHECK_EQ(strlen(out), (size_t)(kLogLineMax - 1));
    // tiny buffer: truncated, NUL-terminated, returns the full length
    char tiny[16];
    KT_CHECK_EQ(fmtResolve(tiny, sizeof tiny, 5, 0, true, 1, 3400), 41);
    KT_CHECK_EQ(strlen(tiny), (size_t)15);
    KT_CHECK(strncmp(tiny, "RESOLVE cmd=5 m", 15) == 0);
    // null buffer: nothing written, length still reported
    KT_CHECK_EQ(fmtResolve(nullptr, 0, 5, 0, true, 1, 3400), 41);
    // HANDOFF list over the limit: clamped to 199 chars + NUL, return value still reports the full length
    uint32_t many[60];
    for (int i = 0; i < 60; ++i) many[i] = 4000000000u + (uint32_t)i;
    const int nh = fmtHandoff(out, sizeof out, 0, true, 4294967295u, many, 60);
    KT_CHECK(nh > kLogLineMax);
    KT_CHECK_EQ(strlen(out), (size_t)(kLogLineMax - 1));
    const char* hPrefix = "HANDOFF m=0 active=4294967295 buffered=60 cmds=4000000000,4000000001,";
    KT_CHECK(strncmp(out, hPrefix, strlen(hPrefix)) == 0);
    // tiny buffer: truncated to 11 chars + NUL (no overflow), return value is the full 40-char line
    char tinyH[12];
    memset(tinyH, 'Z', sizeof tinyH);
    const uint32_t hIds2[2] = { 6, 7 };
    KT_CHECK_EQ(fmtHandoff(tinyH, sizeof tinyH, 0, true, 5, hIds2, 2), 40);
    KT_CHECK_EQ(strlen(tinyH), (size_t)11);
    KT_CHECK(strncmp(tinyH, "HANDOFF m=0", 11) == 0 && tinyH[11] == '\0');
    // null buffer: nothing written, length still reported
    KT_CHECK_EQ(fmtHandoff(nullptr, 0, 0, true, 5, hIds2, 2), 40);
    // worst-case CMD line stays under the limit
    CmdLine worst;
    worst.cmdId = 4294967295u; worst.m = 2; worst.cr = 0xffffffffu; worst.kind = (int)ActionKind::Spell;
    worst.hasId = true; worst.id = 0xffffffffu; worst.hasTgt = true; worst.tgt = 0xffffffffu;
    worst.dec = "pass_unreadable"; worst.fb = true; worst.engq = 16; worst.buf = 3; worst.paused = true; worst.combat = true;
    KT_CHECK(fmtCmd(out, sizeof out, worst) < kLogLineMax);
    KT_CHECK(strlen(out) < (size_t)kLogLineMax);
}

// ---- eaten-action verdict (activeVerdict), its ActionBuffer bookkeeping, curReal and makeEngineView ----
EngineView Ev(bool present, bool paused = false, bool foreign = false, bool idle = true) {
    EngineView e;
    e.actionPresent = present; e.paused = paused; e.foreign = foreign; e.idle = idle;
    return e;
}

int V(const ActionBuffer& b, const EngineView& e, uint32_t now) { return (int)activeVerdict(b, e, now); }

bool verdictFieldsClear(const ActionBuffer& b) {
    return !b.sawExecuting && !b.eaten && !b.staleCurLogged && !b.pausedEaten && b.eatenReInjects == 0 && b.goneSinceTick == 0 &&
           b.lastReInjectTick == 0 && b.staleCurSince == 0 && b.eatenUnpausedMs == 0 && b.lastVerdictTick == 0 &&
           !b.sawExpanded && b.flagBase == -1 && b.activeUnpausedMs == 0 && b.totalReInjects == 0;
}

void dirtyVerdictFields(ActionBuffer& b) {
    b.sawExecuting = true; b.eaten = true; b.staleCurLogged = true; b.pausedEaten = true; b.eatenReInjects = 4;
    b.goneSinceTick = 9; b.lastReInjectTick = 9; b.staleCurSince = 9; b.eatenUnpausedMs = 9; b.lastVerdictTick = 9;
    b.sawExpanded = true; b.flagBase = 3; b.activeUnpausedMs = 9; b.totalReInjects = 9;
}

void testActiveVerdictRules() {
    // rule 1: a present frame is always PRESENT, whatever the other fields say
    ActionBuffer b;
    KT_CHECK_EQ(V(b, Ev(true), 0), (int)AV_PRESENT);
    KT_CHECK_EQ(V(b, Ev(true, true, true, false), 99999), (int)AV_PRESENT);
    b.eaten = true; b.sawExecuting = true; b.eatenReInjects = 6; b.eatenUnpausedMs = 20000;
    KT_CHECK_EQ(V(b, Ev(true), 99999), (int)AV_PRESENT);
    // rule 1: never noted gone (goneSinceTick == 0) -> PRESENT
    ActionBuffer g0;
    KT_CHECK_EQ(V(g0, Ev(false), 10000), (int)AV_PRESENT);
    // rule 1: vanish debounce boundary, 149 ms still PRESENT, 150 ms not
    ActionBuffer d; d.goneSinceTick = 1000;
    KT_CHECK_EQ(V(d, Ev(false), 1149), (int)AV_PRESENT);
    KT_CHECK_EQ(V(d, Ev(false), 1150), (int)AV_UNSEEN);
    // rule 1: min-active window boundary, 500 ms PRESENT, 501 ms not (debounce already passed)
    ActionBuffer m; m.activeStartTick = 1000; m.goneSinceTick = 1;
    KT_CHECK_EQ(V(m, Ev(false), 1500), (int)AV_PRESENT);
    KT_CHECK_EQ(V(m, Ev(false), 1501), (int)AV_UNSEEN);
    // rule 2: executed then vanished, paused or not, eaten or not
    ActionBuffer x; x.goneSinceTick = 1; x.sawExecuting = true;
    KT_CHECK_EQ(V(x, Ev(false, false), 10000), (int)AV_EXECUTED);
    KT_CHECK_EQ(V(x, Ev(false, true), 10000), (int)AV_EXECUTED);
    x.eaten = true;
    KT_CHECK_EQ(V(x, Ev(false, true), 10000), (int)AV_EXECUTED);
    KT_CHECK_EQ(V(x, Ev(false, false), 10000), (int)AV_EXECUTED);
    // rule 3: vanished, never executed, not eaten, not paused -> UNSEEN
    ActionBuffer u; u.goneSinceTick = 1;
    KT_CHECK_EQ(V(u, Ev(false), 10000), (int)AV_UNSEEN);
    // rule 3 skipped while paused: a wipe during pause counts as eaten -> re-inject, never UNSEEN
    KT_CHECK_EQ(V(u, Ev(false, true), 10000), (int)AV_EATEN_REINJECT);
    // eaten survives unpause: wiped while paused, unpaused later, still NOT UNSEEN
    ActionBuffer p; p.eaten = true; p.goneSinceTick = 1;
    KT_CHECK_EQ(V(p, Ev(false, true), 10000), (int)AV_EATEN_REINJECT);
    KT_CHECK_EQ(V(p, Ev(false, false), 10000), (int)AV_EATEN_REINJECT);
    KT_CHECK(V(p, Ev(false, false), 10000) != (int)AV_UNSEEN);
    // rule 4: eaten unpaused time 19999 ms still waits, 20000 ms gives up
    ActionBuffer e4; e4.eaten = true; e4.goneSinceTick = 1; e4.eatenUnpausedMs = 19999;
    KT_CHECK_EQ(V(e4, Ev(false, false, true, false), 10000), (int)AV_EATEN_WAIT);
    e4.eatenUnpausedMs = 20000;
    KT_CHECK_EQ(V(e4, Ev(false, false, true, false), 10000), (int)AV_GIVE_UP);
    // rule 4 beats rule 5 (foreign order) and applies while paused too
    KT_CHECK_EQ(V(e4, Ev(false, true, true, false), 10000), (int)AV_GIVE_UP);
    // rule 5: a foreign order or a non-idle character -> EATEN_WAIT
    ActionBuffer e5; e5.eaten = true; e5.goneSinceTick = 1;
    KT_CHECK_EQ(V(e5, Ev(false, false, true, true), 10000), (int)AV_EATEN_WAIT);
    KT_CHECK_EQ(V(e5, Ev(false, false, false, false), 10000), (int)AV_EATEN_WAIT);
    // rule 6: re-inject budget 5 re-injects, 6 gives up
    e5.eatenReInjects = 5;
    KT_CHECK_EQ(V(e5, Ev(false), 10000), (int)AV_EATEN_REINJECT);
    e5.eatenReInjects = 6;
    KT_CHECK_EQ(V(e5, Ev(false), 10000), (int)AV_GIVE_UP);
    // rule 6 before rule 7: budget spent beats spacing
    e5.lastReInjectTick = 10000;
    KT_CHECK_EQ(V(e5, Ev(false), 10000), (int)AV_GIVE_UP);
    // rule 7: spacing 249 ms waits, 250 ms re-injects
    ActionBuffer s; s.eaten = true; s.goneSinceTick = 1; s.lastReInjectTick = 1000;
    KT_CHECK_EQ(V(s, Ev(false), 1249), (int)AV_EATEN_WAIT);
    KT_CHECK_EQ(V(s, Ev(false), 1250), (int)AV_EATEN_REINJECT);
    // rule 8: otherwise re-inject
    ActionBuffer r8; r8.eaten = true; r8.goneSinceTick = 1;
    KT_CHECK_EQ(V(r8, Ev(false), 10000), (int)AV_EATEN_REINJECT);
    // verdictName, every value plus the fallback
    KT_CHECK(strcmp(verdictName(AV_PRESENT), "present") == 0);
    KT_CHECK(strcmp(verdictName(AV_EXECUTED), "executed") == 0);
    KT_CHECK(strcmp(verdictName(AV_UNSEEN), "unseen") == 0);
    KT_CHECK(strcmp(verdictName(AV_EATEN_WAIT), "eaten_wait") == 0);
    KT_CHECK(strcmp(verdictName(AV_EATEN_REINJECT), "eaten_reinject") == 0);
    KT_CHECK(strcmp(verdictName(AV_GIVE_UP), "give_up") == 0);
    KT_CHECK(strcmp(verdictName(AV_PRESENT_TIMEOUT), "present_timeout") == 0);
    KT_CHECK(strcmp(verdictName(static_cast<ActiveVerdict>(99)), "?") == 0);
    // R6: after an expansion only ONE re-send is allowed; without it the cap stays kMaxEatenReInject
    {
        ActionBuffer c; c.eaten = true; c.goneSinceTick = 1; c.activeStartTick = 0;
        c.sawExpanded = true; c.eatenReInjects = 1;
        KT_CHECK_EQ(V(c, Ev(false), 10000), (int)AV_GIVE_UP);
        c.eatenReInjects = 0;
        KT_CHECK_EQ(V(c, Ev(false), 10000), (int)AV_EATEN_REINJECT);
        c.sawExpanded = false; c.eatenReInjects = 1;
        KT_CHECK_EQ(V(c, Ev(false), 10000), (int)AV_EATEN_REINJECT);
        c.eatenReInjects = 6;
        KT_CHECK_EQ(V(c, Ev(false), 10000), (int)AV_GIVE_UP);
    }
}

// QL6a: while paused an eaten action waits (no re-inject, no budget); the unpause edge gives one re-inject with a fresh budget.
void testPauseEatenWait() {
    // rule 4b only with the flag; the default keeps the old behaviour
    ActionBuffer b; b.eaten = true; b.goneSinceTick = 1;
    KT_CHECK_EQ((int)activeVerdict(b, Ev(false, true), 10000), (int)AV_EATEN_REINJECT);
    KT_CHECK_EQ((int)activeVerdict(b, Ev(false, true), 10000, true), (int)AV_EATEN_WAIT);
    KT_CHECK_EQ((int)activeVerdict(b, Ev(false, false), 10000, true), (int)AV_EATEN_REINJECT);   // unpaused: normal rules
    // not yet flagged eaten and paused: waits too (rule 3 is skipped while paused)
    ActionBuffer u; u.goneSinceTick = 1;
    KT_CHECK_EQ((int)activeVerdict(u, Ev(false, true), 10000, true), (int)AV_EATEN_WAIT);
    // rule 4 (unpaused wait cap) still beats 4b; PRESENT and EXECUTED still win
    ActionBuffer t; t.eaten = true; t.goneSinceTick = 1; t.eatenUnpausedMs = 20000;
    KT_CHECK_EQ((int)activeVerdict(t, Ev(false, true), 10000, true), (int)AV_GIVE_UP);
    ActionBuffer x; x.goneSinceTick = 1; x.sawExecuting = true;
    KT_CHECK_EQ((int)activeVerdict(x, Ev(false, true), 10000, true), (int)AV_EXECUTED);
    KT_CHECK_EQ((int)activeVerdict(b, Ev(true, true), 10000, true), (int)AV_PRESENT);
    // 10 eaten ticks while paused: never a give-up, budget stays 0 (the caller only counts on AV_EATEN_REINJECT)
    ActionBuffer p; p.eaten = true; p.goneSinceTick = 1;
    for (uint32_t t2 = 10000; t2 < 15000; t2 += 500) {
        p.accrueEaten(true, t2);
        KT_CHECK_EQ((int)activeVerdict(p, Ev(false, true), t2, true), (int)AV_EATEN_WAIT);
    }
    KT_CHECK_EQ(p.eatenReInjects, 0);
    KT_CHECK(p.pausedEaten);
    KT_CHECK_EQ(p.eatenUnpausedMs, 0u);
    // unpause edge: a spent budget (4) and a recent re-inject stamp are cleared, then exactly one re-inject
    p.eatenReInjects = 4; p.lastReInjectTick = 14900;
    p.accrueEaten(false, 15000);
    KT_CHECK(!p.pausedEaten);
    KT_CHECK_EQ(p.eatenReInjects, 0); KT_CHECK_EQ(p.lastReInjectTick, 0u);
    KT_CHECK_EQ((int)activeVerdict(p, Ev(false, false), 15000, true), (int)AV_EATEN_REINJECT);
    // unpaused eats still reach the cap of 6
    for (int i = 0; i < kMaxEatenReInject; ++i) {
        KT_CHECK_EQ((int)activeVerdict(p, Ev(false, false), 20000, true), (int)AV_EATEN_REINJECT);
        p.eatenReInjects++;
    }
    KT_CHECK_EQ((int)activeVerdict(p, Ev(false, false), 20000, true), (int)AV_GIVE_UP);
    // an unpaused tick without a prior paused-eaten latch does not touch the budget
    ActionBuffer n; n.eaten = true; n.eatenReInjects = 3; n.lastReInjectTick = 77;
    n.accrueEaten(false, 1000);
    KT_CHECK_EQ(n.eatenReInjects, 3); KT_CHECK_EQ(n.lastReInjectTick, 77u);
    // markDispatched drops the latch
    ActionBuffer m; m.eaten = true; m.accrueEaten(true, 10); KT_CHECK(m.pausedEaten);
    m.markDispatched(20); KT_CHECK(!m.pausedEaten);
    // ini key
    Config c;
    KT_CHECK(c.pauseEatenWait);
    KT_CHECK(parseIniLine(c, "pause_eaten_wait=0") && !c.pauseEatenWait);
    KT_CHECK(parseIniLine(c, "pause_eaten_wait=1") && c.pauseEatenWait);
    KT_CHECK(!parseIniLine(c, "pause_eaten_wait=x") && c.pauseEatenWait);
}

// ---- RV2: orphan clock, present ceiling, combat tracking, exec tie, dispatch reset, total cap, paused flag gate, kick give-up ----
void testOrphanRoundAge() {
    UnpausedClock c;
    KT_CHECK_EQ(orphanRoundAge(false, false, true, false, c, 1000), 0u);
    KT_CHECK_EQ(orphanRoundAge(false, false, true, false, c, 1100), 100u);
    KT_CHECK_EQ(orphanRoundAge(false, false, true, false, c, 1200), 200u);
    UnpausedClock c2;   // 100 ms ticks from 1000
    uint32_t a = 0;
    for (uint32_t t = 1000; t <= 2400; t += 100) a = orphanRoundAge(false, false, true, false, c2, t);
    KT_CHECK_EQ(a, 1400u); KT_CHECK(a < kOrphanMs);
    KT_CHECK_EQ(orphanRoundAge(false, false, true, false, c2, 2499), 1499u);
    KT_CHECK_EQ(orphanRoundAge(false, false, true, false, c2, 2500), 1500u);
    KT_CHECK(orphanRoundAge(false, false, true, false, c2, 2500) >= kOrphanMs);
    KT_CHECK_EQ(orphanRoundAge(true, false, true, false, c2, 2600), 0u);   // a consumer entry exists
    KT_CHECK_EQ(c2.ms, 0u);
    KT_CHECK_EQ(orphanRoundAge(false, true, true, false, c2, 2700), 0u);   // real cur
    KT_CHECK_EQ(orphanRoundAge(false, false, false, false, c2, 2800), 0u); // no 9/10 node
    // pause freezes; the first tick after the pause adds nothing
    UnpausedClock p; uint32_t v = 0;
    for (uint32_t t = 1000; t <= 1500; t += 100) v = orphanRoundAge(false, false, true, false, p, t);
    KT_CHECK_EQ(v, 500u);
    KT_CHECK_EQ(orphanRoundAge(false, false, true, true, p, 1600), 500u);
    KT_CHECK_EQ(orphanRoundAge(false, false, true, true, p, 5000), 500u);
    KT_CHECK_EQ(orphanRoundAge(false, false, true, false, p, 5100), 500u);
    KT_CHECK_EQ(orphanRoundAge(false, false, true, false, p, 5200), 600u);
    // a gap > kClockGapMs between ticks adds nothing
    UnpausedClock g;
    g.tick(false, 1000); g.tick(false, 1100);
    KT_CHECK_EQ(g.tick(false, 3000), 100u);
    KT_CHECK(entryConsumesRound(0x3F) && entryConsumesRound(0x0F) && entryConsumesRound(0x2E));
    KT_CHECK(!entryConsumesRound(0x3D) && !entryConsumesRound(0x01) && !entryConsumesRound(-1));
}

void testPresentCeiling() {
    ActionBuffer b;
    b.activeUnpausedMs = 29999;
    KT_CHECK_EQ(V(b, Ev(true), 100), (int)AV_PRESENT);
    b.activeUnpausedMs = 30000;
    KT_CHECK_EQ(V(b, Ev(true), 100), (int)AV_PRESENT_TIMEOUT);
    KT_CHECK_EQ(V(b, Ev(true, true), 100), (int)AV_PRESENT);
    b.goneSinceTick = 1;
    KT_CHECK_EQ(V(b, Ev(false), 10000), (int)AV_UNSEEN);
    ActionBuffer a;
    a.accrueEaten(false, 1000);   // first call: d = 1000 (capped)
    a.accrueEaten(false, 1500);
    a.accrueEaten(false, 9000);   // d capped at 1000
    KT_CHECK_EQ(a.activeUnpausedMs, 1000u + 500u + 1000u);
    a.accrueEaten(true, 9500);
    KT_CHECK_EQ(a.activeUnpausedMs, 2500u);
    char out[120];
    fmtDrop(out, sizeof out, 14, 0, DR_PRESENT_TIMEOUT, 1);
    KT_CHECK_EQ(out, "DROP cmd=14 m=0 why=present_timeout n=1");
}

void testShouldTrackAfterCombat() {
    KT_CHECK(shouldTrackAfterCombat(5000, 5000));
    KT_CHECK(!shouldTrackAfterCombat(4999, 5000));
    KT_CHECK(shouldTrackAfterCombat(5001, 5000));
    KT_CHECK(shouldTrackAfterCombat(123, 0));
    KT_CHECK(shouldTrackAfterCombat(0x10u, 0xFFFFFFF0u));
    KT_CHECK(!shouldTrackAfterCombat(0xFFFFFFF0u, 0x10u));
    char out[160];
    fmtResolve(out, sizeof out, 5, 0, true, 1, 3400, "combat_end");
    KT_CHECK_EQ(out, "RESOLVE cmd=5 m=0 seen=1 reinj=1 age=3400 why=combat_end");
    fmtResolve(out, sizeof out, 5, 0, true, 1, 3400);
    KT_CHECK_EQ(out, "RESOLVE cmd=5 m=0 seen=1 reinj=1 age=3400");
}

void testExecTie() {
    ExecTie st; st.spell = true; st.id = 0x28; st.curSpell = 0; st.flagBase = 2;
    const int t0f[] = {0x0F}, t2e[] = {0x2E};
    const uint32_t p28[] = {0x28}, p99[] = {0x99};
    EngineView e = makeEngineView(t0f, 1, false, false, 0, 0, 0, 0, false, 0, 1000, p28, &st);
    KT_CHECK(e.expanded && !e.executing);
    e = makeEngineView(t0f, 1, false, false, 0, 0, 0, 0, false, 0, 1000, p99, &st);
    KT_CHECK(!e.expanded);
    e = makeEngineView(t2e, 1, false, false, 0, 0, 0, 0, false, 0, 1000, p28, &st);
    KT_CHECK(!e.expanded);
    // flags: set at dispatch (flagBase=2) is not evidence; a rose bit is
    e = makeEngineView(nullptr, 0, false, false, 0, 0, 0, 1, false, 0, 1000, nullptr, &st);
    KT_CHECK(!e.executing);
    st.flagBase = 0;
    e = makeEngineView(nullptr, 0, false, false, 0, 0, 0, 1, false, 0, 1000, nullptr, &st);
    KT_CHECK(e.executing);
    st.flagBase = 1;
    e = makeEngineView(nullptr, 0, false, false, 0, 0, 0, 1, false, 0, 1000, nullptr, &st);
    KT_CHECK(e.executing);
    st.flagBase = -1;
    e = makeEngineView(nullptr, 0, false, false, 0, 0, 0, 1, false, 0, 1000, nullptr, &st);
    KT_CHECK(e.executing);
    st.flagBase = 0;
    e = makeEngineView(nullptr, 0, false, false, 0, 0, 0, 1, true, 0, 1000, nullptr, &st);
    KT_CHECK(!e.executing);
    // cur node
    st.flagBase = 3;
    e = makeEngineView(nullptr, 0, false, true, 9, 0, 1, 0, false, 0, 1000, nullptr, &st);
    st.curSpell = 0x28;
    e = makeEngineView(nullptr, 0, false, true, 9, 0, 1, 0, false, 0, 1000, nullptr, &st);
    KT_CHECK(e.executing);
    st.curSpell = 0x99; st.flagBase = 0;
    e = makeEngineView(nullptr, 0, false, true, 9, 0, 0, 0, false, 0, 1000, nullptr, &st);   // aa8/ab8 clear: stale-able cur, curReal true until stale
    KT_CHECK(!e.executing);
    st.curSpell = 0;
    e = makeEngineView(nullptr, 0, false, true, 9, 0, 0, 0, false, 0, 1000, nullptr, &st);
    KT_CHECK(e.executing);
    // item tie is lenient
    ExecTie it; it.spell = false; it.flagBase = 3;
    e = makeEngineView(t2e, 1, false, false, 0, 0, 0, 0, false, 0, 1000, p99, &it);
    KT_CHECK(e.expanded);
    e = makeEngineView(nullptr, 0, false, true, 10, 0, 0, 0, false, 0, 1000, nullptr, &it);
    KT_CHECK(e.executing);
    // no tie: legacy reading unchanged
    e = makeEngineView(t2e, 1, false, false, 0, 0, 0, 0, false, 0, 1000);
    KT_CHECK(e.executing && e.expanded);
    // noteFlags only shrinks the baseline
    ActionBuffer b;
    b.noteFlags(3); KT_CHECK_EQ(b.flagBase, 3);
    b.noteFlags(1); KT_CHECK_EQ(b.flagBase, 1);
    b.noteFlags(3); KT_CHECK_EQ(b.flagBase, 1);
    b.noteFlags(0); KT_CHECK_EQ(b.flagBase, 0);
    KT_CHECK(flagsRose(0, 0, 1));
    KT_CHECK(!flagsRose(2, 0, 1) && flagsRose(2, 1, 0));   // ab8 was set at dispatch; aa8 (bit 0) is new
    KT_CHECK_EQ(flagBits(1, 0), 1); KT_CHECK_EQ(flagBits(0, 5), 2); KT_CHECK_EQ(flagBits(1, 1), 3);
    // verdict: expanded then wiped
    ActionBuffer v; v.goneSinceTick = 1; v.sawExpanded = true;
    KT_CHECK_EQ(V(v, Ev(false), 10000), (int)AV_EATEN_REINJECT);
    v.sawExpanded = false;
    KT_CHECK_EQ(V(v, Ev(false), 10000), (int)AV_UNSEEN);
    ActionBuffer n;
    n.notePresent(false, true, 1000, true);
    KT_CHECK(n.sawExpanded);
}

void testMarkDispatchedResets() {
    ActionBuffer b;
    b.eatenReInjects = 5; b.eatenUnpausedMs = 15000; b.lastReInjectTick = 900; b.sawExpanded = true; b.totalReInjects = 7; b.activeUnpausedMs = 9;
    b.active = false;
    b.markDispatched(2000, 3);
    KT_CHECK(b.eatenReInjects == 0 && b.eatenUnpausedMs == 0 && b.lastReInjectTick == 0 && !b.sawExpanded && b.totalReInjects == 0 && b.activeUnpausedMs == 0);
    KT_CHECK_EQ(b.flagBase, 3);
    // first eaten episode gets the full budget: 6 REINJECT verdicts before GIVE_UP
    b.eaten = true; b.goneSinceTick = 1; b.activeStartTick = 0;
    for (int i = 0; i < kMaxEatenReInject; ++i) {
        KT_CHECK_EQ(V(b, Ev(false), 10000 + 1000u * i), (int)AV_EATEN_REINJECT);
        b.eatenReInjects++; b.lastReInjectTick = 10000 + 1000u * i;
    }
    KT_CHECK_EQ(V(b, Ev(false), 20000), (int)AV_GIVE_UP);
    ActionBuffer r;
    r.eatenReInjects = 5; r.totalReInjects = 3; r.sawExecuting = true; r.eaten = true; r.goneSinceTick = 4; r.activeUnpausedMs = 77; r.sawExpanded = true;
    r.markReInjected(3000, 1);
    KT_CHECK_EQ(r.eatenReInjects, 5); KT_CHECK_EQ(r.totalReInjects, 3);
    KT_CHECK(!r.sawExecuting && !r.eaten && r.goneSinceTick == 0 && r.activeUnpausedMs == 0 && !r.sawExpanded);
    KT_CHECK_EQ(r.flagBase, 1); KT_CHECK_EQ(r.activeStartTick, 3000u);
}

void testTotalReInjectCap() {
    ActionBuffer b; b.activeStartTick = 0;
    for (int i = 0; i < 13; ++i) {
        const uint32_t now = 10000u + 3000u * i;
        b.eaten = true; b.goneSinceTick = 1;
        if (i < 12) {
            KT_CHECK_EQ(V(b, Ev(false), now), (int)AV_EATEN_REINJECT);
            b.eatenReInjects++; b.totalReInjects++; b.lastReInjectTick = now;
            b.markReInjected(now);
            b.notePresent(false, true, now + 2500);
        } else {
            KT_CHECK_EQ(V(b, Ev(false), now), (int)AV_GIVE_UP);
        }
    }
}

void testFlagGatePaused() {
    FlagGate g; bool cap = false;
    KT_CHECK(!g.step(true, 1000));
    KT_CHECK(!g.step(true, 2500));
    KT_CHECK(!g.step(false, 2600, &cap, true));
    KT_CHECK(!g.step(false, 5600, &cap, true));
    KT_CHECK(!g.step(true, 5700, &cap));
    KT_CHECK(!g.step(true, 6099, &cap));
    KT_CHECK(g.step(true, 6100, &cap)); KT_CHECK(cap);
    FlagGate f;   // paused first tick, never busy: dispatch
    KT_CHECK(f.step(false, 100, &cap, true));
    FlagGate c;   // pause mid clear-run restarts the run
    KT_CHECK(!c.step(true, 1000));
    KT_CHECK(!c.step(false, 1100));        // clearing
    KT_CHECK(!c.step(false, 1200, &cap, true));
    KT_CHECK(!c.step(false, 1300));        // new clear run starts
    KT_CHECK(!c.step(false, 1449));
    KT_CHECK(c.step(false, 1450));
}

void testKickGiveUp() {
    const int a[] = {0x3D, 0x3D, 0x3D, 0x3D, 0x3F};
    KT_CHECK_EQ(castStarvedAt(a, 5, true), 4);
    int big[16]; for (int i = 0; i < 15; ++i) big[i] = 0x3D; big[15] = 0x3F;
    KT_CHECK_EQ(castStarvedAt(big, 16, true), 15);
    const int f[] = {0x3D, 0x01, 0x3F};
    KT_CHECK_EQ(castStarvedAt(f, 3, true), 0);
    const int d[] = {0x3D, 0x3F, 0x3F};
    KT_CHECK_EQ(castStarvedAt(d, 3, true), 1);
    UnpausedClock k; int first = -1; uint32_t t = 1000;
    for (int i = 0; i < 60; ++i, t += 100) { if (k.tick(false, t) >= kKickGiveUpMs && first < 0) first = i; }
    KT_CHECK_EQ(first, 50);   // the 51st tick
    UnpausedClock pk; pk.tick(false, 1000); pk.tick(true, 2000); pk.tick(true, 9000);
    KT_CHECK_EQ(pk.ms, 0u);
    char out[160];
    fmtOrphan(out, sizeof out, 0, 7, 1, 1500);
    KT_CHECK_EQ(out, "ORPHAN m=0 cmd=7 n=1 age=1500 act=removed");
    fmtOrphan(out, sizeof out, 1, 0, -1, 1500);
    KT_CHECK_EQ(out, "ORPHAN m=1 cmd=0 n=0 age=1500 act=failed");
    fmtKickGiveup(out, sizeof out, 1, 17, 5000, 1);
    KT_CHECK_EQ(out, "SRV_KICK_GIVEUP m=1 cmd=17 ms=5000 n=1 act=removed");
    char tiny[8];
    const int n1 = fmtOrphan(tiny, sizeof tiny, 0, 7, 1, 1500);
    KT_CHECK_EQ(n1, 41); KT_CHECK_EQ(strlen(tiny), (size_t)7);
    KT_CHECK_EQ(fmtOrphan(nullptr, 0, 0, 7, 1, 1500), 41);
    const int n2 = fmtKickGiveup(tiny, sizeof tiny, 1, 17, 5000, 1);
    KT_CHECK_EQ(n2, 50); KT_CHECK_EQ(strlen(tiny), (size_t)7);
    KT_CHECK_EQ(fmtKickGiveup(nullptr, 0, 1, 17, 5000, 1), 50);
    // R3 masks
    QueuedAction head = makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, nullptr, 0, 1);
    { const RNode rn[] = { N(9, 1, 0x66), N(9, 2, 0x55) }; KT_CHECK_EQ(headNodeMask(rn, 2, head), 2u); }
    { const RNode rn[] = { N(9, 1, 0x55), N(9, 2, 0x55) }; KT_CHECK_EQ(headNodeMask(rn, 2, head), 1u); }
    { const RNode rn[] = { N(9, 1, 0x66) }; KT_CHECK_EQ(headNodeMask(rn, 1, head), 0u); }
    KT_CHECK_EQ(headNodeMask(nullptr, 0, head), 0u);
    { const RNode rn[] = { N(1, 1), N(9, 2), N(10, 3) }; KT_CHECK_EQ(itemSpellMask(rn, 3), 6u); }
    { const RNode rn[] = { N(1, 1) }; KT_CHECK_EQ(itemSpellMask(rn, 1), 0u); }
    KT_CHECK_EQ(itemSpellMask(nullptr, 0), 0u);
}

// QL6b: the dispatch gate also waits for the engine's aa8/ab8 flags (cmd 4 / cmd 5 of the 10-11 18:xx session).
void testFlagGate() {
    // dispatchWhy: flagHold is the last check; it never masks an earlier reason
    ActionBuffer b;
    float pf[3] = {0, 0, 0};
    KT_CHECK(b.push(makeSpellAction(0x1000, 0x55, 0, 0, 0, 0, pf, 0, 1)));
    KT_CHECK_EQ((int)dispatchWhy(b, 0, false, 0, true), (int)DS_FLAG_BUSY);
    KT_CHECK_EQ((int)dispatchWhy(b, 0, false, 0, false), (int)DS_OK);
    KT_CHECK_EQ((int)dispatchWhy(b, 1, false, 0, true), (int)DS_ENGINE_BUSY);
    KT_CHECK_EQ((int)dispatchWhy(b, 0, true, 0, true), (int)DS_CUR_BUSY);
    KT_CHECK_EQ((int)dispatchWhy(b, 0, false, 1, true), (int)DS_NRN_BUSY);
    // never busy: dispatches on the first tick (no added latency on an idle engine)
    FlagGate g; bool cap = true;
    KT_CHECK(g.step(false, 1000, &cap)); KT_CHECK(!cap);
    // cmd 4 trace: ab8=1 -> wait; then clear must hold on two ticks >= 150 ms apart
    g.reset();
    KT_CHECK(!g.step(true, 2000, &cap));
    KT_CHECK(!g.step(true, 2300, &cap));
    KT_CHECK(!g.step(false, 2400, &cap));        // first clear tick
    KT_CHECK(!g.step(false, 2549, &cap));        // 149 ms
    KT_CHECK(g.step(false, 2550, &cap)); KT_CHECK(!cap);   // 150 ms
    // flicker: busy again resets the clear run
    g.reset();
    KT_CHECK(!g.step(true, 3000));
    KT_CHECK(!g.step(false, 3100));
    KT_CHECK(!g.step(true, 3200));
    KT_CHECK(!g.step(false, 3300));
    KT_CHECK(!g.step(false, 3449));
    KT_CHECK(g.step(false, 3450));
    // cap: 1999 ms of busy still holds, 2000 ms dispatches with capped=true
    g.reset();
    KT_CHECK(!g.step(true, 5000));
    KT_CHECK(!g.step(true, 6999, &cap)); KT_CHECK(!cap);
    KT_CHECK(g.step(true, 7000, &cap)); KT_CHECK(cap);
    // the cap runs from the hold start, not from the first busy tick
    g.reset();
    KT_CHECK(!g.step(true, 1));
    KT_CHECK(!g.step(true, 1999)); KT_CHECK(!g.step(true, 2000));
    KT_CHECK(g.step(true, 2001, &cap)); KT_CHECK(cap);
    // reset clears everything (a new hold starts a new clock)
    g.reset();
    KT_CHECK(!g.holding && !g.clearing && !g.sawBusy);
    KT_CHECK(!g.step(true, 0)); KT_CHECK(g.holding);   // tick 0 is a valid hold start
    // ini key
    Config c;
    KT_CHECK(c.dispatchWaitFlags);
    KT_CHECK(parseIniLine(c, "dispatch_wait_flags=0") && !c.dispatchWaitFlags);
    KT_CHECK(parseIniLine(c, "Dispatch_Wait_Flags=on") && c.dispatchWaitFlags);
    KT_CHECK(!parseIniLine(c, "dispatch_wait_flags=2") && c.dispatchWaitFlags);
}

// T4 trace formatters: CLICK and CMBT_FIELDS, plus the trace_clicks key.
void testClickAndCmbtFields() {
    char out[kLogLineMax];
    fmtClick(out, sizeof out, 123456, true, 640, 360);
    KT_CHECK_EQ(out, "CLICK t=123456 x=640 y=360");
    fmtClick(out, sizeof out, 7, false, 5, 5);
    KT_CHECK_EQ(out, "CLICK t=7 x=- y=-");
    fmtClick(out, sizeof out, 1, true, -3, 0);
    KT_CHECK_EQ(out, "CLICK t=1 x=-3 y=0");
    uint32_t cr[kCrCombatDwords] = {};
    bool ok[kCrCombatDwords];
    for (int i = 0; i < kCrCombatDwords; ++i) ok[i] = true;
    cr[0] = 0x80000448u; cr[11] = 0xabu;
    const int n = fmtCmbtFields(out, sizeof out, 0, cr, ok, 0x7f000001u, true, 0u, true, 0xffffffffu, true);
    KT_CHECK_EQ(out, "CMBT_FIELDS m=0 cr520=80000448,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,000000ab t550=7f000001 t564=00000000 t568=ffffffff");
    KT_CHECK(n < kLogLineMax);
    ok[1] = false;
    fmtCmbtFields(out, sizeof out, 2, cr, ok, 0, false, 5, true, 0, false);
    KT_CHECK_EQ(out, "CMBT_FIELDS m=2 cr520=80000448,?,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,000000ab t550=? t564=00000005 t568=?");
    fmtCmbtFields(out, sizeof out, -1, nullptr, nullptr, 0, false, 0, false, 0, false);
    KT_CHECK_EQ(out, "CMBT_FIELDS m=? cr520=?,?,?,?,?,?,?,?,?,?,?,? t550=? t564=? t568=?");
    Config c;
    KT_CHECK(c.traceClicks);
    KT_CHECK(parseIniLine(c, "trace_clicks=0") && !c.traceClicks);
    KT_CHECK(parseIniLine(c, "trace_clicks=1") && c.traceClicks);
    KT_CHECK(!parseIniLine(c, "trace_clicks=x") && c.traceClicks);
}

void testVerdictBookkeeping() {
    // markDispatched stamps the start and clears the per-action flags
    ActionBuffer b;
    b.sawExecuting = true; b.eaten = true; b.goneSinceTick = 77;
    b.markDispatched(500);
    KT_CHECK_EQ(b.activeStartTick, 500u);
    KT_CHECK(!b.sawExecuting && !b.eaten);
    KT_CHECK_EQ(b.goneSinceTick, 0u);
    // accrueEaten: adds only while eaten and unpaused, each step capped at 1000 ms
    ActionBuffer a;
    a.accrueEaten(false, 1000);                       // not eaten: no add, but stamps lastVerdictTick
    KT_CHECK_EQ(a.eatenUnpausedMs, 0u); KT_CHECK_EQ(a.lastVerdictTick, 1000u);
    a.eaten = true;
    a.accrueEaten(false, 1100);                       // +100
    KT_CHECK_EQ(a.eatenUnpausedMs, 100u);
    a.accrueEaten(true, 1200);                        // paused: no add
    KT_CHECK_EQ(a.eatenUnpausedMs, 100u); KT_CHECK_EQ(a.lastVerdictTick, 1200u);
    a.accrueEaten(false, 3201);                       // delta 2001 capped to 1000
    KT_CHECK_EQ(a.eatenUnpausedMs, 1100u);
    a.accrueEaten(false, 4201);                       // delta exactly 1000 kept
    KT_CHECK_EQ(a.eatenUnpausedMs, 2100u);
    a.accrueEaten(false, 4000);                       // time went backwards: unsigned delta capped to 1000
    KT_CHECK_EQ(a.eatenUnpausedMs, 3100u);
    ActionBuffer f; f.eaten = true;                   // first call, lastVerdictTick == 0: at most 1000 ms
    f.accrueEaten(false, 5000);
    KT_CHECK_EQ(f.eatenUnpausedMs, 1000u);
    ActionBuffer f2; f2.eaten = true;
    f2.accrueEaten(false, 999);                       // 999 below the cap is kept as is
    KT_CHECK_EQ(f2.eatenUnpausedMs, 999u);
    // noteView: first absent frame stamps, later absent frames keep it, a present frame clears it
    ActionBuffer v;
    v.noteView(true, 100);  KT_CHECK_EQ(v.goneSinceTick, 0u);
    v.noteView(false, 200); KT_CHECK_EQ(v.goneSinceTick, 200u);
    v.noteView(false, 300); KT_CHECK_EQ(v.goneSinceTick, 200u);
    v.noteView(true, 400);  KT_CHECK_EQ(v.goneSinceTick, 0u);
    // notePresent: executing sets sawExecuting (even when absent); reset of the re-inject budget needs a present frame at >= 2000 ms
    ActionBuffer q;
    q.notePresent(false, true, 1000);  KT_CHECK(!q.sawExecuting);
    q.notePresent(true, false, 1000);  KT_CHECK(q.sawExecuting);
    ActionBuffer r; r.eatenReInjects = 3; r.eatenUnpausedMs = 7000; r.lastReInjectTick = 100;
    r.notePresent(false, false, 99999); KT_CHECK_EQ(r.eatenReInjects, 3); KT_CHECK_EQ(r.eatenUnpausedMs, 7000u);   // absent: no reset
    r.notePresent(false, true, 2099);   KT_CHECK_EQ(r.eatenReInjects, 3); KT_CHECK_EQ(r.eatenUnpausedMs, 7000u);   // 1999 ms: no reset
    r.notePresent(false, true, 2100);   KT_CHECK_EQ(r.eatenReInjects, 0); KT_CHECK_EQ(r.eatenUnpausedMs, 0u);      // 2000 ms: episode over
    // cancel() and clear() reset every new field; a fresh buffer starts clear
    KT_CHECK(verdictFieldsClear(ActionBuffer{}));
    ActionBuffer c; dirtyVerdictFields(c); c.cancel();
    KT_CHECK(verdictFieldsClear(c));
    ActionBuffer k; dirtyVerdictFields(k); k.clear();
    KT_CHECK(verdictFieldsClear(k));
}

void testCurReal() {
    // no current node -> not real
    KT_CHECK(!curReal(false, 10, 1, 1, false, 0, 0));
    // ghost (subType 1 attack ghost, flags 0): not real immediately, paused or not
    KT_CHECK(!curReal(true, 1, 0, 0, false, 0, 0));
    KT_CHECK(!curReal(true, 1, 0, 0, true, 0, 0));
    // paused item/spell round node with flags 0: real (the stale timer does not run while paused)
    KT_CHECK(curReal(true, 10, 0, 0, true, 0, 0));
    KT_CHECK(curReal(true, 10, 0, 0, true, 1, 999999));
    // unpaused flagless subType 10: stale at exactly kStaleCurMs (1500), real at 1499
    KT_CHECK(curReal(true, 10, 0, 0, false, 1000, 2499));
    KT_CHECK(!curReal(true, 10, 0, 0, false, 1000, 2500));
    // staleSince == 0 -> never stale
    KT_CHECK(curReal(true, 10, 0, 0, false, 0, 999999));
    // subType 9 is the other item/spell node: same stale rule
    KT_CHECK(curReal(true, 9, 0, 0, false, 1000, 2499));
    KT_CHECK(!curReal(true, 9, 0, 0, false, 1000, 2500));
    // unpaused with aa8 or ab8 set -> real, even on a stale timer and for a ghost sub
    KT_CHECK(curReal(true, 10, 1, 0, false, 1000, 999999));
    KT_CHECK(curReal(true, 1, 0, 1, false, 0, 0));
    // paused with flags set on a non-item/spell sub -> not real; on item/spell -> real
    KT_CHECK(!curReal(true, 1, 1, 0, true, 0, 0));
    KT_CHECK(curReal(true, 10, 1, 0, true, 0, 0));
    // other subType with flags 0 -> not real
    KT_CHECK(!curReal(true, 8, 0, 0, false, 0, 0));
}

void testCurActionPresent() {
    // no current node -> never an action
    KT_CHECK(!curActionPresent(curReal(false, 10, 0, 0, false, 0, 0), -1));
    // stale flagless 10 and 9 (unpaused, stale at exactly 1500 ms) -> not present
    KT_CHECK(!curActionPresent(curReal(true, 10, 0, 0, false, 1000, 2500), 10));
    KT_CHECK(!curActionPresent(curReal(true, 9, 0, 0, false, 1000, 2500), 9));
    // same node at 1499 ms is still a real action -> present
    KT_CHECK(curActionPresent(curReal(true, 10, 0, 0, false, 1000, 2499), 10));
    // now wrap: staleSince 0xFFFFFE00 + 1500 ms = 0x3DC (stale); 0x3DB is 1499 ms (real)
    KT_CHECK(!curActionPresent(curReal(true, 10, 0, 0, false, 0xFFFFFE00u, 0x3DCu), 10));
    KT_CHECK(curActionPresent(curReal(true, 10, 0, 0, false, 0xFFFFFE00u, 0x3DBu), 10));
    // paused flagless 9/10 (stale timer frozen) -> present
    KT_CHECK(curActionPresent(curReal(true, 10, 0, 0, true, 0, 0), 10));
    KT_CHECK(curActionPresent(curReal(true, 9, 0, 0, true, 1, 999999), 9));
    // flagged 9/10 unpaused, even on a stale timer -> present
    KT_CHECK(curActionPresent(curReal(true, 10, 1, 0, false, 1000, 999999), 10));
    // never stale (staleSince 0) -> present
    KT_CHECK(curActionPresent(curReal(true, 10, 0, 0, false, 0, 999999), 10));
    // real node of another subType (ghost 1 with ab8 set) -> not an item/spell action
    KT_CHECK(!curActionPresent(curReal(true, 1, 0, 1, false, 0, 0), 1));
}

void testMakeEngineView() {
    const int a3f[] = { 0x3F };
    const int a3d[] = { 0x3D };
    const int a3f01[] = { 0x3F, 0x01 };
    const int a2e[] = { 0x2E };
    const int a0f[] = { 0x0F };
    // sticky ab8 while paused: not executing, idle
    EngineView s = makeEngineView(nullptr, 0, false, false, 0, 0, 0, 1, true, 0, 1000);
    KT_CHECK(!s.executing && s.idle && !s.foreign && s.paused && !s.actionPresent);
    // null entries and ne <= 0 are safe and ignore the entries
    EngineView n = makeEngineView(nullptr, 5, true, false, 0, 0, 0, 0, false, 0, 1000);
    KT_CHECK(!n.foreign && !n.executing && n.idle && n.actionPresent && !n.paused);
    EngineView n0 = makeEngineView(a3f01, 0, false, false, 0, 0, 0, 0, false, 0, 1000);
    KT_CHECK(!n0.foreign && !n0.executing && n0.idle);
    // 0x3F and 0x3D are not foreign; 0x3F with 0x01 is
    EngineView f0 = makeEngineView(a3f, 1, false, false, 0, 0, 0, 0, false, 0, 1000);
    KT_CHECK(!f0.foreign && !f0.executing && f0.idle);
    EngineView fd = makeEngineView(a3d, 1, false, false, 0, 0, 0, 0, false, 0, 1000);
    KT_CHECK(!fd.foreign);
    EngineView f1 = makeEngineView(a3f01, 2, false, false, 0, 0, 0, 0, false, 0, 1000);
    KT_CHECK(f1.foreign && !f1.idle);
    // 0x2E and 0x0F are exec nodes: executing, not idle, not foreign (also while paused)
    EngineView e2e = makeEngineView(a2e, 1, false, false, 0, 0, 0, 0, false, 0, 1000);
    KT_CHECK(e2e.executing && !e2e.idle && !e2e.foreign);
    EngineView e0f = makeEngineView(a0f, 1, false, false, 0, 0, 0, 0, false, 0, 1000);
    KT_CHECK(e0f.executing && !e0f.idle && !e0f.foreign);
    EngineView e2p = makeEngineView(a2e, 1, false, false, 0, 0, 0, 0, true, 0, 1000);
    KT_CHECK(e2p.executing && !e2p.idle);
    // nrn != 0 -> not idle
    EngineView nr = makeEngineView(nullptr, 0, false, false, 0, 1, 0, 0, false, 0, 1000);
    KT_CHECK(!nr.idle && !nr.executing);
    // ghost current node while paused -> idle
    EngineView gh = makeEngineView(nullptr, 0, false, true, 1, 0, 0, 0, true, 0, 1000);
    KT_CHECK(gh.idle && !gh.executing);
    // real current node sub 10 while paused -> neither idle nor executing
    EngineView rp = makeEngineView(nullptr, 0, false, true, 10, 0, 0, 0, true, 0, 1000);
    KT_CHECK(!rp.idle && !rp.executing);
    // real current node sub 10 unpaused -> executing, not idle
    EngineView ru = makeEngineView(nullptr, 0, false, true, 10, 0, 0, 0, false, 0, 1000);
    KT_CHECK(ru.executing && !ru.idle);
    // aa8 is executing while unpaused, ignored while paused
    EngineView au = makeEngineView(nullptr, 0, false, false, 0, 0, 1, 0, false, 0, 1000);
    KT_CHECK(au.executing);
    EngineView ap = makeEngineView(nullptr, 0, false, false, 0, 0, 1, 0, true, 0, 1000);
    KT_CHECK(!ap.executing && ap.idle);
}

// ---- QH-2a-02 / WI-74: a companion's resting FollowLeader (0x3D) must not hold the dispatch gates shut ----
void testGateEngineCount() {
    const int rest[] = { 0x3D };
    const int restBare[] = { 0x3D, 0x3F };
    const int restExec[] = { 0x3D, 0x2E };
    const int restOther[] = { 0x3D, 0x0C };
    // the count itself
    KT_CHECK_EQ(gateEngineCount(rest, 1, 0), 0);
    KT_CHECK_EQ(gateEngineCount(restBare, 2, 0), 0);      // bare 0x3F (no round node) rests too
    KT_CHECK_EQ(gateEngineCount(restBare, 2, 1), 2);      // 0x3F beside a round node still blocks
    KT_CHECK_EQ(gateEngineCount(rest, 1, 1), 1);          // round nodes pending: raw count, as before
    KT_CHECK_EQ(gateEngineCount(restExec, 2, 0), 1);      // an exec node behind the follower still blocks
    KT_CHECK_EQ(gateEngineCount(restOther, 2, 0), 1);     // a foreign entry blocks
    KT_CHECK_EQ(gateEngineCount(rest, 0, 0), 0);          // zero entries
    KT_CHECK_EQ(gateEngineCount(nullptr, 3, 0), 3);       // unreadable types: the raw count
    // decideAction: a resting companion dispatches now; the raw count (BASE) buffered it forever
    ActionBuffer idle;
    KT_CHECK_EQ((int)decideAction(idle, false, gateEngineCount(rest, 1, 0)), (int)DECIDE_DISPATCH_NOW);
    KT_CHECK_EQ((int)decideAction(idle, false, 1), (int)DECIDE_BUFFER);
    KT_CHECK_EQ((int)decideAction(idle, false, gateEngineCount(restExec, 2, 0)), (int)DECIDE_BUFFER);
    KT_CHECK_EQ((int)decideAction(idle, false, gateEngineCount(rest, 1, 1)), (int)DECIDE_BUFFER);
    // dispatchWhy / shouldDispatchNext with one buffered action
    ActionBuffer b;
    fillBuf(b, 1);
    KT_CHECK_EQ((int)dispatchWhy(b, gateEngineCount(rest, 1, 0), false, 0), (int)DS_OK);
    KT_CHECK(shouldDispatchNext(b, gateEngineCount(rest, 1, 0), false, 0));
    KT_CHECK_EQ((int)dispatchWhy(b, gateEngineCount(restBare, 2, 0), false, 0), (int)DS_OK);
    KT_CHECK_EQ((int)dispatchWhy(b, gateEngineCount(restBare, 2, 1), false, 1), (int)DS_ENGINE_BUSY);
    KT_CHECK_EQ((int)dispatchWhy(b, gateEngineCount(rest, 1, 1), false, 1), (int)DS_ENGINE_BUSY);
    KT_CHECK_EQ((int)dispatchWhy(b, gateEngineCount(restExec, 2, 0), false, 0), (int)DS_ENGINE_BUSY);
    KT_CHECK_EQ((int)dispatchWhy(b, gateEngineCount(rest, 1, 0), true, 0), (int)DS_CUR_BUSY);   // other gates unaffected
    KT_CHECK(!shouldDispatchNext(b, gateEngineCount(restExec, 2, 0), false, 0));
}

// ---- QL3: an engine-held active command that combat start wipes is re-sent; one the engine really runs is not ----
void testHeldHeadAction() {
    // play test 2026-10-11 cmd 8: held at the edge, gone 70 ms later -> wait while held, re-send once released
    // held: WAIT; stuck (> 3000 ms of UNPAUSED held time) -> DROP; paused never counts as stuck
    KT_CHECK(heldHeadAction(true, false, false, 0) == HELD_WAIT);
    KT_CHECK(heldHeadAction(true, true, false, 0) == HELD_WAIT);
    KT_CHECK(heldHeadAction(true, false, false, kHeldStuckMs) == HELD_WAIT);
    KT_CHECK(heldHeadAction(true, false, false, kHeldStuckMs + 1) == HELD_DROP);
    KT_CHECK(heldHeadAction(true, true, false, kHeldStuckMs + 1) == HELD_DROP);
    KT_CHECK(heldHeadAction(true, false, true, 100000) == HELD_WAIT);
    // gone: it ran -> DROP (never twice), it never ran -> RESEND (wiped / eaten), paused or not
    KT_CHECK(heldHeadAction(false, true, false, 0) == HELD_DROP);
    KT_CHECK(heldHeadAction(false, true, true, 0) == HELD_DROP);
    KT_CHECK(heldHeadAction(false, false, false, 0) == HELD_RESEND);
    KT_CHECK(heldHeadAction(false, false, true, 0) == HELD_RESEND);
    // QL10: a vanished head right after a player click is a cancel, never re-sent; a still-held head ignores the click
    KT_CHECK(heldHeadAction(false, false, false, 0, true) == HELD_DROP);
    KT_CHECK(heldHeadAction(false, false, true, 0, true) == HELD_DROP);
    KT_CHECK(heldHeadAction(true, false, false, 0, true) == HELD_WAIT);
    KT_CHECK(cancelClickRecent(1100, 1000, 1102));            // click after the edge, 2 ms ago
    KT_CHECK(cancelClickRecent(1100, 1000, 1100 + kCancelClickMs));
    KT_CHECK(!cancelClickRecent(1100, 1000, 1100 + kCancelClickMs + 1));   // too old
    KT_CHECK(!cancelClickRecent(990, 1000, 1002));            // the click that started combat is before the edge
    KT_CHECK(!cancelClickRecent(1000, 1000, 1002));           // same tick as the edge: not after it
    KT_CHECK(!cancelClickRecent(0, 1000, 1002));              // no click yet
    // replay of the two logs: 13:07 cmd 8 gone at +70 ms with no exec evidence; 18:xx cmd 6 / cmd 8 lingered with aa8=1 ab8=1
    KT_CHECK(heldHeadAction(false, false, false, 0) == HELD_RESEND);
    KT_CHECK(heldHeadAction(true, true, false, 400) == HELD_WAIT);
    KT_CHECK(heldHeadAction(false, true, false, 400) == HELD_DROP);
    // HANDOFF_OUT m=<k> cmd=<n> exec=<0|1> firstExecMs=<n|-> endMs=<n>
    char o2[kLogLineMax];
    fmtHandoffOut(o2, sizeof o2, 0, 6, true, 120, 8000);
    KT_CHECK_EQ(o2, "HANDOFF_OUT m=0 cmd=6 exec=1 firstExecMs=120 endMs=8000");
    fmtHandoffOut(o2, sizeof o2, 2, 11, false, -1, 8003);
    KT_CHECK_EQ(o2, "HANDOFF_OUT m=2 cmd=11 exec=0 firstExecMs=- endMs=8003");
    fmtHandoffOut(o2, sizeof o2, -1, 0, false, 5, 0);                  // exec=0 never prints a time
    KT_CHECK_EQ(o2, "HANDOFF_OUT m=? cmd=0 exec=0 firstExecMs=- endMs=0");
    // SPELL_OUT: same fields, own keyword
    fmtSpellOut(o2, sizeof o2, 1, 13, true, 340, 8001);
    KT_CHECK_EQ(o2, "SPELL_OUT m=1 cmd=13 exec=1 firstExecMs=340 endMs=8001");
    fmtSpellOut(o2, sizeof o2, 0, 4, false, 77, 8000);
    KT_CHECK_EQ(o2, "SPELL_OUT m=0 cmd=4 exec=0 firstExecMs=- endMs=8000");
    // SPELL_ARGS: hook with the captured float[3], replay of a zero copy with a flag, null pos3
    {
        const float p123[3] = {1.0f, 2.0f, 3.0f}; const float pz[3] = {0.0f, 0.0f, 0.0f};
        fmtSpellArgs(o2, sizeof o2, false, 9, 0, 0x55, 3, 0, 2, 0, p123, 0, 0x12ab30);
        KT_CHECK_EQ(o2, "SPELL_ARGS where=hook cmd=9 m=0 spell=55 tgt=3 multi=0 meta=2 dom=0 pos=3f800000,40000000,40400000 flag=0 posaddr=12ab30");
        fmtSpellArgs(o2, sizeof o2, true, 4, -1, 0x1a3, -1, 0, 0, 0, pz, 5, 0x19f000);
        KT_CHECK_EQ(o2, "SPELL_ARGS where=replay cmd=4 m=? spell=1a3 tgt=ffffffff multi=0 meta=0 dom=0 pos=0,0,0 flag=5 posaddr=19f000");
        fmtSpellArgs(o2, sizeof o2, false, 4, 2, 0x1a3, 0, 0, 0, 0, nullptr, 0, 0);
        KT_CHECK_EQ(o2, "SPELL_ARGS where=hook cmd=4 m=2 spell=1a3 tgt=0 multi=0 meta=0 dom=0 pos=0,0,0 flag=0 posaddr=0");
    }
    // softCandidate: none / party / hostile, sentinel + zero, low-24-bit match, null-safety
    {
        const uint32_t party3[3] = {0x800001afu, 0x800001b3u, 0x800001b5u};
        const uint32_t none3[3] = {0x7f000000u, 0u, 0x7f000000u};
        SoftCand a = softCandidate(none3, 3, party3, 3);
        KT_CHECK(a.allNone && !a.anyHostile && !a.anyParty);
        const uint32_t ally3[3] = {0x1afu, 0x7f000000u, 0x800001b5u};           // low 24 bits match, mixed 0x8000.. prefix
        a = softCandidate(ally3, 3, party3, 3);
        KT_CHECK(!a.allNone && !a.anyHostile && a.anyParty);
        const uint32_t foe3[3] = {0x7f000000u, 0x800002c0u, 0x800001afu};       // one hostile + one party
        a = softCandidate(foe3, 3, party3, 3);
        KT_CHECK(!a.allNone && a.anyHostile && a.anyParty);
        a = softCandidate(foe3, 3, nullptr, 0);                                  // no party ids known: every real id is hostile
        KT_CHECK(a.anyHostile && !a.anyParty);
        const uint32_t zeroParty[1] = {0x80000000u};                             // low 24 bits 0 never matches
        const uint32_t t1[1] = {0x80000000u};
        a = softCandidate(t1, 1, zeroParty, 1);
        KT_CHECK(a.anyHostile && !a.anyParty);
        a = softCandidate(nullptr, 0, party3, 3);
        KT_CHECK(a.allNone && !a.anyHostile && !a.anyParty);
        char o3[kLogLineMax];
        fmtSoftCand(o3, sizeof o3, 1, softCandidate(ally3, 3, party3, 3), 1, true, 0x2au, true);
        KT_CHECK_EQ(o3, "SOFT_CAND m=1 hostile=0 party=1 none=0 cr0=1 cr1=0000002a");
        fmtSoftCand(o3, sizeof o3, -1, softCandidate(none3, 3, party3, 3), 0, false, 0, false);
        KT_CHECK_EQ(o3, "SOFT_CAND m=? hostile=0 party=0 none=1 cr0=? cr1=?");
    }
    // softCombat / bannerAct / fmtBanner: soft = in combat with party-only targets (S3 case), hide/restore table, BANNER goldens
    {
        const uint32_t party3[3] = {0x800001afu, 0x800001b3u, 0x800001b5u};
        const uint32_t pty1[1] = {0x800001afu};
        const uint32_t hst1[1] = {0x800002c0u};
        const uint32_t none1[1] = {0x7f000000u};
        const uint32_t both3[3] = {0x7f000000u, 0x800002c0u, 0x800001afu};     // one member targets a hostile AND a party id
        const SoftCand mPty = softCandidate(pty1, 1, party3, 3);
        const SoftCand mHst = softCandidate(hst1, 1, party3, 3);
        const SoftCand mNone = softCandidate(none1, 1, party3, 3);
        const SoftCand mBoth = softCandidate(both3, 3, party3, 3);
        const SoftCand s3[2] = {mPty, mNone};                                  // S3: member0 targets party only, member1 nothing
        KT_CHECK(softCombat(true, s3, 2));
        const SoftCand oneHostile[2] = {mNone, mHst};                          // one member targets a hostile
        KT_CHECK(!softCombat(true, oneHostile, 2));
        const SoftCand acrossMix[2] = {mPty, mHst};                            // party on member0, hostile on member1
        KT_CHECK(!softCombat(true, acrossMix, 2));
        const SoftCand withinMix[1] = {mBoth};                                 // party + hostile inside one member
        KT_CHECK(!softCombat(true, withinMix, 1));
        const SoftCand allNone[2] = {mNone, mNone};                            // nobody targets anything: counts as HARD
        KT_CHECK(!softCombat(true, allNone, 2));
        KT_CHECK(!softCombat(false, s3, 2));                                   // not in combat
        KT_CHECK(!softCombat(false, nullptr, 0));
        KT_CHECK(!softCombat(true, nullptr, 2));                               // null array
        KT_CHECK(!softCombat(true, s3, 0));                                    // empty array
        KT_CHECK(!softCombat(true, s3, -1));

        // bannerAct(hidden, combat, soft): soft+combat hides (idempotent); hard combat while hidden restores; else none
        KT_CHECK(bannerAct(false, false, false) == BA_NONE);
        KT_CHECK(bannerAct(false, false, true) == BA_NONE);                    // soft but not in combat: nothing to hide
        KT_CHECK(bannerAct(false, true, false) == BA_NONE);                    // hard combat, not hidden: leave banner alone
        KT_CHECK(bannerAct(false, true, true) == BA_HIDE);                     // soft combat: hide
        KT_CHECK(bannerAct(true, false, false) == BA_NONE);                    // combat over while hidden: no restore here
        KT_CHECK(bannerAct(true, false, true) == BA_NONE);
        KT_CHECK(bannerAct(true, true, false) == BA_RESTORE);                  // hard combat began while hidden: restore
        KT_CHECK(bannerAct(true, true, true) == BA_HIDE);                      // still soft while hidden: re-hide

        // BANNER act=<hide|restore|forget|reset> soft=<0|1> ctl=<n> wr=<n>
        char ob[kLogLineMax];
        fmtBanner(ob, sizeof ob, "hide", 1, 8, 8);
        KT_CHECK_EQ(ob, "BANNER act=hide soft=1 ctl=8 wr=8");
        fmtBanner(ob, sizeof ob, "restore", 0, 8, 8);
        KT_CHECK_EQ(ob, "BANNER act=restore soft=0 ctl=8 wr=8");
        fmtBanner(ob, sizeof ob, "forget", 0, 3, 0);
        KT_CHECK_EQ(ob, "BANNER act=forget soft=0 ctl=3 wr=0");
        fmtBanner(ob, sizeof ob, "reset", 0, 8, 8);
        KT_CHECK_EQ(ob, "BANNER act=reset soft=0 ctl=8 wr=8");
        fmtBanner(ob, sizeof ob, "hide", 5, 8, 0);                             // soft prints 0/1 only
        KT_CHECK_EQ(ob, "BANNER act=hide soft=1 ctl=8 wr=0");
    }
    // HANDOFF_HELD m=<k> cmd=<n> act=<resend|drop> ms=<n>
    char out[kLogLineMax];
    fmtHandoffHeld(out, sizeof out, 0, 8, HELD_RESEND, 70);
    KT_CHECK_EQ(out, "HANDOFF_HELD m=0 cmd=8 act=resend ms=70");
    fmtHandoffHeld(out, sizeof out, 2, 12, HELD_DROP, 1500);
    KT_CHECK_EQ(out, "HANDOFF_HELD m=2 cmd=12 act=drop ms=1500");
    fmtHandoffHeld(out, sizeof out, -1, 0, HELD_WAIT, 0);
    KT_CHECK_EQ(out, "HANDOFF_HELD m=? cmd=0 act=wait ms=0");
    // handoff_resend_held: default on, bool parsing, other fields untouched
    Config c;
    KT_CHECK(c.handoffResendHeld);
    KT_CHECK(parseIniLine(c, "handoff_resend_held=0") && !c.handoffResendHeld && c.handoffClearFirst && c.handoffOnCombat);
    KT_CHECK(parseIniLine(c, "Handoff_Resend_Held = yes") && c.handoffResendHeld);
    KT_CHECK(!parseIniLine(c, "handoff_resend_held=2") && c.handoffResendHeld);
    // spell_replay_pos: legacy key, accepted and ignored (only noted)
    KT_CHECK(!c.spellReplayPosSeen);
    KT_CHECK(parseIniLine(c, "spell_replay_pos=0") && c.spellReplayPosSeen && c.handoffResendHeld);
    // hide_banner_soft / hide_redbar_soft: default on, bool parsing, each key independent of the other
    KT_CHECK(c.hideBannerSoft && c.hideRedbarSoft);
    KT_CHECK(parseIniLine(c, "hide_banner_soft=0") && !c.hideBannerSoft && c.hideRedbarSoft);
    KT_CHECK(parseIniLine(c, "hide_redbar_soft=0") && !c.hideRedbarSoft && !c.hideBannerSoft);
    KT_CHECK(parseIniLine(c, "hide_banner_soft=1") && c.hideBannerSoft && !c.hideRedbarSoft);
    KT_CHECK(parseIniLine(c, "hide_redbar_soft=1") && c.hideRedbarSoft && c.hideBannerSoft);
    KT_CHECK(!parseIniLine(c, "hide_banner_soft=x") && c.hideBannerSoft);
    KT_CHECK(!parseIniLine(c, "hide_redbar_soft=x") && c.hideRedbarSoft);
}

// Cancel Queue colours: the leader is dark green-teal (not black), hover is brighter, companion keeps the teal border.
void testCancelStyle() {
    const CancelStyle L = cancelStyle(true);
    KT_CHECK(L.tint[0] + L.tint[1] + L.tint[2] > 0.3f);
    KT_CHECK(L.tint[1] > L.tint[0] && L.tint[2] > L.tint[0]);   // green-teal, not red-dominant
    KT_CHECK(L.tint[1] >= L.tint[2]);                            // green over blue (not a blue tint)
    for (int k = 0; k < 3; ++k) KT_CHECK(L.hoverTint[k] > L.tint[k]);
    KT_CHECK(L.alpha > 0.5f && L.alpha <= 1.0f);
    const CancelStyle C = cancelStyle(false);
    for (int k = 0; k < 3; ++k) { KT_CHECK(C.tint[k] == kTealBorder[k]); KT_CHECK(C.hoverTint[k] > C.tint[k]); }
    KT_CHECK(C.alpha == 1.0f);
    // QL19: the leader BUTTON border is the tray's green-teal (G > B > R), not the cyan companion teal, and hover is brighter.
    const CancelStyle B = cancelButtonStyle();
    KT_CHECK(B.tint[1] > B.tint[2] && B.tint[2] > B.tint[0]);
    KT_CHECK(B.tint[2] < kTealBorder[2]);   // less blue than the 0.4.3 cyan
    for (int k = 0; k < 3; ++k) KT_CHECK(B.hoverTint[k] > B.tint[k]);
}

}  // namespace

// Mutation-gate gaps: values that only a boundary or a default can tell apart.
void testMutationGaps() {
    char out[200];
    // accrueEaten adds at most 1000 ms per step: 1001 -> 1000, exactly 1000 -> 1000, 999 -> 999
    ActionBuffer a; a.eaten = true; a.lastVerdictTick = 100;
    a.accrueEaten(false, 1101);  KT_CHECK_EQ(a.eatenUnpausedMs, 1000u);
    a.accrueEaten(false, 2101);  KT_CHECK_EQ(a.eatenUnpausedMs, 2000u);
    a.accrueEaten(false, 3100);  KT_CHECK_EQ(a.eatenUnpausedMs, 2999u);
    // fmtHandoff: a null id list or n <= 0 prints no ids; one id prints one
    const uint32_t one[1] = { 7 };
    fmtHandoff(out, sizeof out, 0, false, 0, nullptr, 2);  KT_CHECK_EQ(out, "HANDOFF m=0 active=- buffered=0 cmds=-");
    fmtHandoff(out, sizeof out, 0, false, 0, one, 0);      KT_CHECK_EQ(out, "HANDOFF m=0 active=- buffered=0 cmds=-");
    fmtHandoff(out, sizeof out, 0, false, 0, one, -1);     KT_CHECK_EQ(out, "HANDOFF m=0 active=- buffered=0 cmds=-");
    fmtHandoff(out, sizeof out, 0, false, 0, one, 1);      KT_CHECK_EQ(out, "HANDOFF m=0 active=- buffered=1 cmds=7");
    // EngineView defaults: everything false
    const EngineView d;
    KT_CHECK(!d.paused && !d.actionPresent && !d.executing && !d.foreign && !d.idle);
    // verdict rule 1: a present action is PRESENT even when an old gone clock is still set
    ActionBuffer g; g.goneSinceTick = 1000; g.activeStartTick = 1;
    EngineView p; p.actionPresent = true; p.paused = true; p.idle = true;
    KT_CHECK_EQ((int)activeVerdict(g, p, 9000), (int)AV_PRESENT);
    p.actionPresent = false;
    KT_CHECK_EQ((int)activeVerdict(g, p, 9000), (int)AV_EATEN_REINJECT);   // the same buffer, absent: the clock is old enough
    // executing: unpaused, flags 0, a fresh item/spell cur node (9 and 10) executes; other subtypes and stale nodes do not
    EngineView e = makeEngineView(nullptr, 0, false, true, 9, 0, 0, 0, false, 0, 1000);   KT_CHECK(e.executing);
    e = makeEngineView(nullptr, 0, false, true, 10, 0, 0, 0, false, 0, 1000);             KT_CHECK(e.executing);
    e = makeEngineView(nullptr, 0, false, true, 8, 0, 0, 0, false, 0, 1000);              KT_CHECK(!e.executing);
    e = makeEngineView(nullptr, 0, false, true, 11, 0, 0, 0, false, 0, 1000);             KT_CHECK(!e.executing);
    e = makeEngineView(nullptr, 0, false, true, 1, 0, 0, 0, false, 0, 1000);              KT_CHECK(!e.executing);
    e = makeEngineView(nullptr, 0, false, true, 10, 0, 0, 0, false, 1000, 2500);          KT_CHECK(!e.executing);   // stale 1500 ms
    e = makeEngineView(nullptr, 0, false, true, 9, 0, 0, 0, false, 1000, 2500);           KT_CHECK(!e.executing);
    e = makeEngineView(nullptr, 0, false, false, 10, 0, 0, 0, false, 0, 1000);            KT_CHECK(!e.executing);   // no cur at all
}

// WI-82 / QH-2a-07: combat rising edge across unreadable HUD frames. Frames: 'C' readable combat, 'o' readable no combat,
// 'U' unreadable. Counts the edges one slot sees; an unreadable frame must leave the last readable state untouched.
static int combatEdges(const char* frames) {
    bool prev = false;
    int edges = 0;
    for (const char* f = frames; *f; ++f) edges += qh::combatEdgeStep(prev, *f != 'U', *f == 'C') ? 1 : 0;
    return edges;
}
void testCombatEdgeSequences() {
    KT_CHECK_EQ(combatEdges("C"), 1);
    KT_CHECK_EQ(combatEdges("CC"), 1);        // repeat combat: one edge
    KT_CHECK_EQ(combatEdges("CoC"), 2);       // readable off then on: a real second edge
    KT_CHECK_EQ(combatEdges("CUC"), 1);       // the bug: U stored false, so the second C re-fired (2)
    KT_CHECK_EQ(combatEdges("CUUC"), 1);      // two unreadable frames in the gap
    KT_CHECK_EQ(combatEdges("CUoC"), 2);      // readable off inside the gap makes the second C a real edge
    KT_CHECK_EQ(combatEdges("UC"), 1);        // leading gap: prev starts false; a store-true mutant gives 0
    KT_CHECK_EQ(combatEdges("oUC"), 1);       // store-true mutant gives 0
    KT_CHECK_EQ(combatEdges("UUU"), 0);       // nothing readable: no edge
    KT_CHECK_EQ(combatEdges("CUoUCoC"), 3);   // swaps with gaps
    // an unreadable frame leaves prev alone in both directions and never reports an edge
    bool prev = true;   KT_CHECK(!qh::combatEdgeStep(prev, false, true));   KT_CHECK(prev);
    prev = false;       KT_CHECK(!qh::combatEdgeStep(prev, false, true));   KT_CHECK(!prev);
}

// B1 (QL17): a buffer whose owner is not party slot 0 is held, never dispatched on the controlled creature.
void testOwnerHoldB1() {
    KT_CHECK(ownerMayDispatch(0));
    KT_CHECK(!ownerMayDispatch(1));
    KT_CHECK(!ownerMayDispatch(2));
    KT_CHECK(!ownerMayDispatch(-1));   // no slot at all
    KT_CHECK_EQ(whyName(DS_NOT_CTRL), "not_ctrl");
    KT_CHECK_EQ(whyName(RI_NOT_CTRL), "not_ctrl");
    // old values keep their numbering; the new ones are appended
    KT_CHECK_EQ((int)DS_FLAG_BUSY, 6);
    KT_CHECK_EQ((int)DS_NOT_CTRL, 7);
    KT_CHECK_EQ((int)RI_EXECUTED, 7);
    KT_CHECK_EQ((int)RI_NOT_CTRL, 8);
    char out[200];
    fmtHandoffHold(out, sizeof out, 7, 1, 2);
    KT_CHECK_EQ(out, "HANDOFF_HOLD cmd=7 m=1 owner_slot=2 why=not_ctrl");
    fmtHandoffHold(out, sizeof out, 12, 0, -1);
    KT_CHECK_EQ(out, "HANDOFF_HOLD cmd=12 m=0 owner_slot=-1 why=not_ctrl");
    char tiny[16];
    KT_CHECK_EQ(fmtHandoffHold(tiny, sizeof tiny, 7, 1, 2), 48);
    KT_CHECK_EQ(strlen(tiny), (size_t)15);
    KT_CHECK(strncmp(tiny, "HANDOFF_HOLD cm", 15) == 0);
}

// QL20: members 1-2 dispatch only through the server route (Force power on an object target); slot 0 is unchanged.
void testServerRouteQL20() {
    KT_CHECK(ownerMayDispatch(0, false));
    KT_CHECK(ownerMayDispatch(1, true));
    KT_CHECK(ownerMayDispatch(2, true));
    KT_CHECK(!ownerMayDispatch(1, false));
    KT_CHECK(!ownerMayDispatch(-1, true));   // not in the party: never
    KT_CHECK(!ownerMayDispatch(3, true));
    const float* none = nullptr;
    QueuedAction sp = makeSpellAction(0x1000, 0x28, (int)0x8000086Bu, 0, 0, 0, none, 0, 0, 21);
    KT_CHECK(serverRouteEligible(sp));
    QueuedAction loc = makeSpellAction(0x1000, 0x28, (int)kInvalidObjectId, 0, 0, 0, none, 0, 0, 22);
    KT_CHECK(!serverRouteEligible(loc));     // location target: still held
    QueuedAction zero = makeSpellAction(0x1000, 0x28, 0, 0, 0, 0, none, 0, 0, 23);
    KT_CHECK(!serverRouteEligible(zero));
    QueuedAction it = makeItemAction(0x1000, 0x865, 0, 0x80000860u, none, 0, 17);
    KT_CHECK(!serverRouteEligible(it));      // items: still held
    // starvation: 0x3F behind only 0x3D with a pending round node
    const int q1[] = {0x3D, 0x3F};
    KT_CHECK_EQ(castStarvedAt(q1, 2, true), 1);
    KT_CHECK_EQ(castStarvedAt(q1, 2, false), 0);   // no round node: harmless leftover 0x3F
    const int q2[] = {0x3D, 0x3D, 0x3F};
    KT_CHECK_EQ(castStarvedAt(q2, 3, true), 2);
    const int q3[] = {0x3F, 0x3D};
    KT_CHECK_EQ(castStarvedAt(q3, 2, true), 0);    // already leads
    const int q4[] = {0x01, 0x3F};
    KT_CHECK_EQ(castStarvedAt(q4, 2, true), 0);    // a move ahead: mid-expansion, wait
    const int q5[] = {0x3D, 0x3D, 0x3D, 0x3D, 0x3F};
    KT_CHECK_EQ(castStarvedAt(q5, 5, true), 4);    // 4th leading 0x3D now kicked (RL8)
    KT_CHECK_EQ(castStarvedAt(nullptr, 0, true), 0);
    char out[200];
    fmtServerCast(out, sizeof out, 21, 1, 0x28, 0x86b, true, nullptr);
    KT_CHECK_EQ(out, "SRV_CAST cmd=21 m=1 spell=28 tgt=86b ok=1");
    fmtServerCast(out, sizeof out, 9, 2, 0x28, 0, false, "not_commandable");
    KT_CHECK_EQ(out, "SRV_CAST cmd=9 m=2 spell=28 tgt=0 ok=0 why=not_commandable");
    char tiny[16];
    fmtServerCast(tiny, sizeof tiny, 21, 1, 0x28, 0x86b, true, nullptr);
    KT_CHECK_EQ(strlen(tiny), (size_t)15);
    Config c;
    KT_CHECK(c.companionServerCast);
    parseIni(c, "companion_server_cast=0\n");
    KT_CHECK(!c.companionServerCast);
}

// RV3: RQ8 restoreHead, RQ10 detachTarget, RQ11 clickIsCancel, RL6 lateForeignCount + grown foreign set, RL18 cancelHideWhy.
void testRV3() {
    auto act = [](uint32_t cmd) { return makeItemAction(0x1000, 100 + cmd, 0, 1, nullptr, 1, cmd); };
    {   // restoreHead
        ActionBuffer b; QueuedAction lost; lost.cmdId = 0;
        KT_CHECK(restoreHead(true, false, act(7), b, &lost) == RH_FRONT);
        KT_CHECK_EQ(b.count, 1); KT_CHECK_EQ(b.queue[0].cmdId, 7u);
    }
    {
        ActionBuffer b; b.push(act(11)); b.push(act(12));
        QueuedAction lost;
        KT_CHECK(restoreHead(true, false, act(7), b, &lost) == RH_FRONT);
        KT_CHECK_EQ(b.count, 3); KT_CHECK_EQ(b.queue[0].cmdId, 7u); KT_CHECK_EQ(b.queue[1].cmdId, 11u); KT_CHECK_EQ(b.queue[2].cmdId, 12u);
    }
    {
        ActionBuffer b; b.push(act(11)); b.push(act(12)); b.push(act(13));
        QueuedAction lost;
        KT_CHECK(restoreHead(true, false, act(7), b, &lost) == RH_FRONT_DROPPED);
        KT_CHECK_EQ(lost.cmdId, 13u);
        KT_CHECK_EQ(b.count, 3); KT_CHECK_EQ(b.queue[0].cmdId, 7u); KT_CHECK_EQ(b.queue[1].cmdId, 11u); KT_CHECK_EQ(b.queue[2].cmdId, 12u);
    }
    {
        ActionBuffer b; b.push(act(11));
        QueuedAction lost;
        KT_CHECK(restoreHead(false, false, act(7), b, &lost) == RH_NONE);
        KT_CHECK_EQ(b.count, 1); KT_CHECK_EQ(b.queue[0].cmdId, 11u);
        KT_CHECK(restoreHead(true, true, act(7), b, &lost) == RH_NONE);
        KT_CHECK_EQ(b.count, 1); KT_CHECK_EQ(b.queue[0].cmdId, 11u);
    }
    // detachTarget
    KT_CHECK_EQ(detachTarget(1, 1), 1); KT_CHECK_EQ(detachTarget(2, 1), 2); KT_CHECK_EQ(detachTarget(0, 2), 0);
    KT_CHECK_EQ(detachTarget(-1, 1), -1); KT_CHECK_EQ(detachTarget(3, 0), -1);
    // clickIsCancel
    {
        const Rect r[3] = { {100, 100, 20, 20}, {0, 0, 0, 0}, {300, 50, 10, 10} };
        KT_CHECK(clickIsCancel(r, 3, 100, 100));
        KT_CHECK(clickIsCancel(r, 3, 119, 119));
        KT_CHECK(!clickIsCancel(r, 3, 120, 119));
        KT_CHECK(!clickIsCancel(r, 3, 99, 100));
        KT_CHECK(!clickIsCancel(r, 3, 0, 0));
        KT_CHECK(clickIsCancel(r, 3, 305, 55));
        KT_CHECK(!clickIsCancel(r, 0, 100, 100));
        KT_CHECK(!clickIsCancel(nullptr, 3, 100, 100));
    }
    // lateForeignCount + roundPartition with the grown set
    KT_CHECK_EQ(lateForeignCount(2, true, 1), 1); KT_CHECK_EQ(lateForeignCount(1, true, 1), 0); KT_CHECK_EQ(lateForeignCount(3, true, 0), 3);
    KT_CHECK_EQ(lateForeignCount(1, false, 0), 0); KT_CHECK_EQ(lateForeignCount(0, true, 0), 0);
    {
        const uint32_t ptrs[3] = { 10, 20, 30 };
        int order[3];
        const uint32_t f1[1] = { 20 };
        KT_CHECK_EQ(roundPartition(ptrs, 3, f1, 1, order), 2);
        KT_CHECK(order[0] == 0 && order[1] == 2 && order[2] == 1);
        const uint32_t f2[2] = { 20, 10 };
        KT_CHECK_EQ(roundPartition(ptrs, 3, f2, 2, order), 3);
        KT_CHECK(order[0] == 2 && order[1] == 0 && order[2] == 1);
    }
    // cancelHideWhy
    KT_CHECK(!strcmp(cancelHideWhy(false, true, 10), "cancel-off"));
    KT_CHECK(!strcmp(cancelHideWhy(true, false, 0), "no-queue"));
    KT_CHECK(!strcmp(cancelHideWhy(true, true, 0), "no-room"));
    KT_CHECK(cancelHideWhy(true, true, 24) == nullptr);
}

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
    testStripHudHostSlots();
    testKitConfig();
    testPauseHandoffShowConfig();
    testInsetRect();
    testCancelRect();
    testChevronRect();
    testItemBuffer();
    testPausedItemQueueing();
    testPausedCancelClick();
    testFollowerWipeProtection();
    testMixedForcePowersAndItems();
    testStripCancel();
    testStripHead();
    testCancelVisible();
    testStripHiddenForDown();
    testUnifiedActionMake();
    testDecideBranches();
    testFifoMixedKinds();
    testReInjectEachCondition();
    testReInjectGate();
    testDeferStep();
    testEdgeHeadAction();
    testDetachableMask();
    testReInjectForeignCastSuppressionTransient();
    testReInjectNotAfterExecuting();
    testDispatchNextEachCondition();
    testWhyParityMatrix();
    testWhyNames();
    testQueueSig();
    testBufferedCmdIdsAndSeen();
    testTraceQueueIni();
    testLogFormatGolden();
    testLogFormatBounds();
    testModeChangedWhy();
    testTraceModeIni();
    testActiveVerdictRules();
    testVerdictBookkeeping();
    testPauseEatenWait();
    testFlagGate();
    testOrphanRoundAge();
    testPresentCeiling();
    testShouldTrackAfterCombat();
    testExecTie();
    testMarkDispatchedResets();
    testTotalReInjectCap();
    testFlagGatePaused();
    testKickGiveUp();
    testClickAndCmbtFields();
    testCurReal();
    testMakeEngineView();
    testMutationGaps();
    testHandoffClearFirstIni();
    testPanelHoldAndSlotRules();
    testHandoffClearAndRoundFormat();
    testCancelStyle();
    testHeldHeadAction();
    testGateEngineCount();
    testCombatEdgeSequences();
    testCurActionPresent();
    testConfigEqualSeesEveryField();
    testOwnerHoldB1();
    testServerRouteQL20();
    testRV3();
    return kt::Report("queue_hud_logic_test");
}
