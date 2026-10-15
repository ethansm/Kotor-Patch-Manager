// Host test of nw::readCreatureTag (NwCore.h): the creature tag read at AddToArea. Regression for the 10-15 live run, where the
// guessed path [c+0x24]+0x18 was empty on every engine add and a save-reload spawned the roster a second time (doc 40 13.6).
#include "NwCore.h"
#include "check.h"
#include <map>
#include <string.h>
#include <string>

struct FakeMem : nw::MemReader {
    std::map<uintptr_t, unsigned> dw;
    std::map<uintptr_t, std::string> str;
    int u32Calls = 0;
    bool u32(uintptr_t a, unsigned* v) override { ++u32Calls; auto it = dw.find(a); if (it == dw.end()) return false; *v = it->second; return true; }
    bool cstr(uintptr_t p, char* out, int cap, int* bad) override {
        out[0] = 0; *bad = 0;
        auto it = str.find(p);
        if (it == str.end()) return false;
        int n = 0;
        for (char ch : it->second) {
            if (n >= cap - 1) break;
            if (static_cast<unsigned char>(ch) < 0x20 || static_cast<unsigned char>(ch) >= 0x7f) ++*bad;
            out[n++] = ch;
        }
        out[n] = 0;
        return true;
    }
};

static std::string tagOf(FakeMem& m, uintptr_t c, nw::TagVia* via = nullptr) {
    char t[nw::TagChars + 1];
    nw::TagVia v = nw::readCreatureTag(m, c, t);
    if (via) *via = v;
    return t;
}

int main() {
    const uintptr_t C = 0x0c913da0;
    nw::TagVia via;
    // 1. primary path c+0x18 -> char*
    { FakeMem m; m.dw[C + 0x18] = 0x0ce68ba8; m.str[0x0ce68ba8] = "MouseDroid";
      KT_CHECK(tagOf(m, C, &via) == "MouseDroid"); KT_CHECK_EQ(int(via), int(nw::TagVia18)); }
    // 2. the old path is the fallback when c+0x18 holds nothing usable
    { FakeMem m; m.dw[C + 0x24] = 0x1000; m.dw[0x1000 + 0x18] = 0x2000; m.str[0x2000] = "k1p_native_npc";
      KT_CHECK(tagOf(m, C, &via) == "k1p_native_npc"); KT_CHECK_EQ(int(via), int(nw::TagVia24)); }
    // 3. primary wins when both exist
    { FakeMem m; m.dw[C + 0x18] = 0x10; m.str[0x10] = "A"; m.dw[C + 0x24] = 0x1000; m.dw[0x1018] = 0x2000; m.str[0x2000] = "B";
      KT_CHECK(tagOf(m, C, &via) == "A"); KT_CHECK_EQ(int(via), int(nw::TagVia18)); }
    // 4. a primary pointer to garbage (non-printable bytes) is rejected, fallback is used
    { FakeMem m; m.dw[C + 0x18] = 0x10; m.str[0x10] = std::string("\x01\x02\xff", 3); m.dw[C + 0x24] = 0x1000; m.dw[0x1018] = 0x2000; m.str[0x2000] = "B";
      KT_CHECK(tagOf(m, C, &via) == "B"); KT_CHECK_EQ(int(via), int(nw::TagVia24)); }
    // 5. empty string, null pointer, unreadable pointer, nothing at all -> "" and TagNone
    { FakeMem m; m.dw[C + 0x18] = 0x10; m.str[0x10] = ""; KT_CHECK(tagOf(m, C, &via) == ""); KT_CHECK_EQ(int(via), int(nw::TagNone)); }
    { FakeMem m; m.dw[C + 0x18] = 0; KT_CHECK(tagOf(m, C, &via) == ""); KT_CHECK_EQ(int(via), int(nw::TagNone)); }
    { FakeMem m; m.dw[C + 0x18] = 0x99; KT_CHECK(tagOf(m, C, &via) == ""); KT_CHECK_EQ(int(via), int(nw::TagNone)); }
    { FakeMem m; KT_CHECK(tagOf(m, C, &via) == ""); KT_CHECK_EQ(int(via), int(nw::TagNone)); KT_CHECK(m.u32Calls <= 2); }
    // 6. a tag longer than 32 chars is cut at 32
    { FakeMem m; m.dw[C + 0x18] = 0x10; m.str[0x10] = std::string(40, 'x'); KT_CHECK(tagOf(m, C) == std::string(32, 'x')); }
    // 7. end to end: the restored engine creature's tag is remembered, so the roster is not spawned again after a save-reload
    {
        nw::World w;
        w.eng.spawn = [](void*, void*, const char*, float, float, float, float) -> uint32_t { return 0x3b0; };
        char area[4];
        FakeMem m; m.dw[C + 0x18] = 0x10; m.str[0x10] = "MouseDroid";
        char tag[nw::TagChars + 1];
        w.onLoad(1);
        nw::readCreatureTag(m, C, tag);
        nw::CreatureInfo ci; ci.oid = 0x348; ci.tag = tag; ci.areaPtr = area; ci.areaRes = "204tel";
        w.onCreatureAdded(ci);
        for (int i = 0; i < 31; ++i) w.onTick();
        KT_CHECK(w.stable());
        KT_CHECK(w.tagPresent("MouseDroid"));
        KT_CHECK(w.tagPresent("mousedroid"));
        KT_CHECK_EQ(w.seenTags(), 1);
        KT_CHECK_EQ(int(w.spawn("c_drdmouse01", "MouseDroid", 0, 0, 0, 0, nullptr)), int(nw::SpawnResult::TagPresent));
    }
    return kt::Report("nw_tag_test");
}
