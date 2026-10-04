// Host test of ItemRead.h on a fake memory layout in a MAP_32BIT arena: HUD group lists, "Name (N)" parsing, the app -> objTable -> item chain, property 0.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "ItemRead.h"
#include "check.h"
#include <sys/mman.h>

using namespace itemread;

static_assert(ItemKeyFlag == 0x40000000, "item key flag");
static_assert(EntryStride == 0x3C && Hud_GroupLists == 0x78, "entry/group layout");

static uint8_t* g_a;
static uintptr_t A(size_t off) { return reinterpret_cast<uintptr_t>(g_a + off); }
template <typename T> static void put(size_t off, T v) { memcpy(g_a + off, &v, sizeof v); }
static void putExo(size_t entry, size_t strAt, const char* s) { memcpy(g_a + strAt, s, strlen(s) + 1); put<uint32_t>(entry, (uint32_t)A(strAt)); put<int32_t>(entry + 4, (int32_t)strlen(s) + 1); }

static uint32_t fakeC2S(uintptr_t t, uint32_t id) { return t == A(0x100) ? (id == 0x7F000005u ? 0x8001u : 0u) : 0u; }
static uint32_t fakeGet(uintptr_t t, uint32_t id) { return t == A(0x100) && id == 0x8001u ? (uint32_t)A(0x2000) : 0u; }

int main() {
    void* m = mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_a = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;

    // ---- stackCount ----
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (3)"), 3);
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac"), 0);                 // no suffix
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (Type) (12)"), 12);
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (Type)"), 0);          // non-numeric suffix
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (3"), 0);              // malformed: no closing paren
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac ()"), 0);
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (-3)"), 0);
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (3 4)"), 0);
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (0)"), 0);
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (1234567)"), 0);       // > 6 digits
    KT_CHECK_EQ(stackCount(0x40000001, "Medpac (999999)"), 999999);
    KT_CHECK_EQ(stackCount(0x00000001, "Medpac (3)"), 0);             // not an item key
    KT_CHECK_EQ(stackCount(0x40000001, "(3)"), 0);                    // too short for " ("
    KT_CHECK_EQ(stackCount(0x40000001, ""), 0);

    // ---- readGroup: hud at 0x4000; group 1 = 2 entries at 0x5000; group 2 = implausible count; group 3 = empty ----
    const size_t hud = 0x4000, ents = 0x5000;
    put<int32_t>(hud + Hud_GroupLists + 1 * 12, (int32_t)A(ents)); put<int32_t>(hud + Hud_GroupLists + 1 * 12 + 4, 2);
    put<int32_t>(hud + Hud_GroupLists + 2 * 12, (int32_t)A(ents)); put<int32_t>(hud + Hud_GroupLists + 2 * 12 + 4, 100000);
    put<int32_t>(hud + Hud_GroupLists + 3 * 12, 0);                 put<int32_t>(hud + Hud_GroupLists + 3 * 12 + 4, 0);
    putExo(ents, 0x6000, "Medpac (3)");   put<int32_t>(ents + Entry_Key, 0x7F000005 | ItemKeyFlag); put<int32_t>(ents + Entry_Flags, 7);
    memcpy(g_a + ents + Entry_Icon, "i_medpac\x01", 9);
    putExo(ents + EntryStride, 0x6100, "Stim"); put<int32_t>(ents + EntryStride + Entry_Key, 0x00000009);
    std::vector<Entry> v;
    KT_CHECK_EQ(readGroup(A(hud), 1, &v), 2);
    if (v.size() == 2) {
        KT_CHECK_EQ(v[0].name.c_str(), "Medpac (3)"); KT_CHECK_EQ(v[0].key, 0x7F000005 | ItemKeyFlag); KT_CHECK_EQ(v[0].flags, 7);
        KT_CHECK_EQ(v[0].icon.c_str(), "i_medpac?"); KT_CHECK_EQ(v[0].group, 1); KT_CHECK_EQ(v[0].index, 0);
        KT_CHECK_EQ(v[1].name.c_str(), "Stim"); KT_CHECK_EQ(v[1].index, 1); KT_CHECK_EQ(v[1].icon.c_str(), "");
    }
    KT_CHECK_EQ(readGroup(A(hud), 2, &v), 0); KT_CHECK(v.empty());
    KT_CHECK_EQ(readGroup(A(hud), 3, &v), 0);
    KT_CHECK_EQ(readGroup(A(hud), 7, &v), 0); KT_CHECK_EQ(readGroup(A(hud), -1, &v), 0);

    // ---- serverItemFor: app cell at 0x80 -> app at 0x90, objTable = *(app+8) = A(0x100) ----
    put<uint32_t>(0x80, (uint32_t)A(0x90)); put<uint32_t>(0x90 + 8, (uint32_t)A(0x100));
    g_appGlobal = A(0x80); g_clientToServer = fakeC2S; g_getItem = fakeGet;
    uint32_t srv = 0;
    KT_CHECK_EQ(serverItemFor(0x7F000005u, &srv), A(0x2000)); KT_CHECK_EQ(srv, 0x8001u);
    srv = 0; KT_CHECK_EQ(serverItemFor(0x7F000006u, &srv), (uintptr_t)0); KT_CHECK_EQ(srv, 0u);   // no server id
    put<uint32_t>(0x90 + 8, 0); KT_CHECK_EQ(serverItemFor(0x7F000005u), (uintptr_t)0);             // no object table
    put<uint32_t>(0x90 + 8, (uint32_t)A(0x100)); put<uint32_t>(0x80, 0); KT_CHECK_EQ(serverItemFor(0x7F000005u), (uintptr_t)0);   // no app
    put<uint32_t>(0x80, (uint32_t)A(0x90)); g_getItem = nullptr; KT_CHECK_EQ(serverItemFor(0x7F000005u), (uintptr_t)0);

    // ---- readItemProp: item at 0x2000, count +0x288, array +0x290 -> props at 0x3000 stride 0x1C ----
    put<int32_t>(0x2000 + Item_PropCount, 2); put<uint32_t>(0x2000 + Item_PropArray, (uint32_t)A(0x3000));
    put<int16_t>(0x3000 + Prop_Type, 10); put<int16_t>(0x3000 + Prop_Row, 65); put<uint8_t>(0x3000 + Prop_Usable, 1);
    put<int16_t>(0x3000 + PropStride, 99);   // property 1 must not leak into the result
    ItemProp p = readItemProp(A(0x2000));
    KT_CHECK_EQ(p.count, 2); KT_CHECK_EQ(p.type, 10); KT_CHECK_EQ(p.row, 65); KT_CHECK_EQ(p.usable, 1);
    put<uint8_t>(0x3000 + Prop_Usable, 0); KT_CHECK_EQ(readItemProp(A(0x2000)).usable, 0);
    put<int16_t>(0x3000 + Prop_Row, -3);   KT_CHECK_EQ(readItemProp(A(0x2000)).row, -3);
    put<int32_t>(0x2000 + Item_PropCount, 0); p = readItemProp(A(0x2000)); KT_CHECK_EQ(p.count, 0); KT_CHECK_EQ(p.type, -1);
    put<int32_t>(0x2000 + Item_PropCount, 1); put<uint32_t>(0x2000 + Item_PropArray, 0); KT_CHECK_EQ(readItemProp(A(0x2000)).count, 0);
    return kt::Report("item_read_test");
}
