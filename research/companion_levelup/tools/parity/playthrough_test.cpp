// Host test of Playthrough.h (GUID bit packing, levelup_playthroughs.ini parse/serialise/set/remove). Pure header, no windows.h. Run via parity.py.
#include "../../../../patch_manager_mods/level_up_override_src/Playthrough.h"
#include <memory>
#include <string>

static int g_fail = 0, g_checks = 0;
#define CHECK(c, ...) do { ++g_checks; if (!(c)) { ++g_fail; fprintf(stderr, "FAIL line %d: ", __LINE__); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

static std::string ser(const pt::Doc& d) {
    static char buf[pt::MaxText];
    size_t n = pt::serialise(d, buf, sizeof buf);
    return std::string(buf, n);
}
static void load(pt::Doc& d, const std::string& s) { pt::parse(d, s.data(), s.size()); }

int main() {
    // 1. GUID bit packing
    const uint32_t vals[] = {0, 1, 0x80000000u, 0xDEADBEEFu, 0xFFFFFFFFu, 0x12345678u};
    for (uint32_t v : vals) {
        uint8_t bits[32];
        pt::unpackBits(v, bits);
        CHECK(pt::packBits(bits) == v, "pack/unpack %08x", v);
        for (int i = 0; i < 32; ++i) CHECK(bits[i] == ((v >> i) & 1), "bit %d of %08x", i, v);
    }
    { char nm[8]; pt::bitName(0, nm); CHECK(!strcmp(nm, "LUO_G00"), "name 0 = %s", nm); pt::bitName(31, nm); CHECK(!strcmp(nm, "LUO_G31"), "name 31 = %s", nm); CHECK(strlen("LUO_G31") <= 21, "name length"); }
    { char sn[10]; pt::sectionName(0xDEADBEEF, sn); CHECK(!strcmp(sn, "gdeadbeef"), "section name %s", sn); sn[0] = 0; pt::sectionName(1, sn); CHECK(!strcmp(sn, "g00000001"), "section name %s", sn); }
    { uint32_t g = 0; CHECK(pt::parseSectionName("gDEADbeef", &g) && g == 0xDEADBEEFu, "parse section name"); CHECK(!pt::parseSectionName("gdeadbee", &g) && !pt::parseSectionName("gdeadbeefa", &g) && !pt::parseSectionName("xdeadbeef", &g) && !pt::parseSectionName("gdeadbeeg", &g), "bad section names"); }

    std::unique_ptr<pt::Doc> dp(new pt::Doc);
    pt::Doc& d = *dp;

    // 2. parse / serialise round trip: comments, unknown lines, other sections, two GUID sections, normalised tag lines
    const std::string text =
        "; leading comment\n"
        "stray=line before any section\n"
        "\n"
        "[g00000001]\n"
        "; first playthrough\n"
        "atton=atton__melee_dual_light_early|instant=1|hold=0\n"
        "weird line without equals\n"
        "kreia=kreia__sage|instant=0|hold=1\n"
        "# hash comment\n"
        "\n"
        "[Other]\n"
        "foo=bar\n"
        "[gdeadbeef]\n"
        "bao-dur=bao-dur__tech|instant=0|hold=1\n";
    load(d, text);
    CHECK(!d.lossy, "lossy");
    CHECK(d.n == 4, "sections %d (want 4 incl. preamble)", d.n);
    CHECK(ser(d) == text, "round trip differs:\n%s\n--\n%s", ser(d).c_str(), text.c_str());
    const pt::Section* s1 = pt::findGuid(d, 1);
    CHECK(s1 && s1->isGuid && pt::tagCount(*s1) == 2, "section g1");
    CHECK(pt::findGuid(d, 0xDEADBEEFu) && pt::tagCount(*pt::findGuid(d, 0xDEADBEEFu)) == 1, "section deadbeef");
    CHECK(!pt::findGuid(d, 2), "no section 2");
    // case-insensitive lookup, field values
    const pt::Line* a = s1 ? pt::findTag(*s1, "ATTON") : nullptr;
    CHECK(a && a->instant && !a->hold && !strcmp(a->preset, "atton__melee_dual_light_early"), "atton entry");
    const pt::Line* k = s1 ? pt::findTag(*s1, "Kreia") : nullptr;
    CHECK(k && !k->instant && k->hold, "kreia entry");
    CHECK(s1 && !pt::findTag(*s1, "bao-dur"), "bao-dur not in g1");
    // [Other] keeps foo=bar verbatim (not parsed as a tag) -> findTag finds nothing there
    { const pt::Section& o = d.sec[2]; CHECK(!o.isGuid && !strcmp(o.name, "Other") && pt::tagCount(o) == 0 && o.n == 1, "Other section verbatim"); }

    // 3. missing fields default (instant 0, hold 1); upper case input is lowercased; CRLF accepted; unknown fields ignored
    load(d, "[G0000000A]\r\nHK47 = HK47__Gunner \r\nT3m4=t3m4__x|hold=0\r\nbastila=bastila__y|instant=1|color=blue\r\n");
    { const pt::Section* s = pt::findGuid(d, 10); CHECK(s && pt::tagCount(*s) == 3, "defaults section");
      const pt::Line* h = s ? pt::findTag(*s, "hk47") : nullptr; CHECK(h && !h->instant && h->hold && !strcmp(h->preset, "hk47__gunner"), "hk47 defaults");
      const pt::Line* t = s ? pt::findTag(*s, "t3m4") : nullptr; CHECK(t && !t->instant && !t->hold, "t3m4 hold=0");
      const pt::Line* b = s ? pt::findTag(*s, "bastila") : nullptr; CHECK(b && b->instant && b->hold, "bastila instant=1, hold default"); }
    CHECK(ser(d) == "[G0000000A]\nhk47=hk47__gunner|instant=0|hold=1\nt3m4=t3m4__x|instant=0|hold=0\nbastila=bastila__y|instant=1|hold=1\n", "normalised output:\n%s", ser(d).c_str());

    // 4. set/remove keep other sections; remove keeps an empty section
    load(d, text);
    CHECK(pt::setTag(d, 1, "HandON", "handon__p", true, false, 0), "setTag new tag");
    CHECK(pt::setTag(d, 1, "atton", "atton__other", false, true, 0), "setTag existing tag");
    CHECK(pt::setTag(d, 0xCAFEBABEu, "atton", "atton__z", false, true, 0), "setTag new section");
    { const pt::Section* s = pt::findGuid(d, 1); CHECK(s && pt::tagCount(*s) == 3, "g1 has 3 tags");
      const pt::Line* a2 = s ? pt::findTag(*s, "atton") : nullptr; CHECK(a2 && !strcmp(a2->preset, "atton__other") && !a2->instant && a2->hold, "atton replaced");
      const pt::Line* hn = s ? pt::findTag(*s, "handon") : nullptr; CHECK(hn && hn->instant && !hn->hold, "handon added"); }
    CHECK(pt::findGuid(d, 0xDEADBEEFu) && pt::findTag(*pt::findGuid(d, 0xDEADBEEFu), "bao-dur"), "other guid section untouched by set");
    { std::string o = ser(d);
      CHECK(o.find("; first playthrough\n") != std::string::npos && o.find("weird line without equals\n") != std::string::npos && o.find("[Other]\nfoo=bar\n") != std::string::npos &&
            o.find("stray=line before any section\n") != std::string::npos && o.find("[gcafebabe]\natton=atton__z|instant=0|hold=1\n") != std::string::npos, "set kept unknown lines/sections:\n%s", o.c_str());
      // re-parse of our own output is stable
      pt::Doc* d2 = new pt::Doc; load(*d2, o); CHECK(ser(*d2) == o, "serialise is a fixed point"); delete d2; }
    CHECK(pt::removeTag(d, 1, "ATTON") && !pt::removeTag(d, 1, "atton"), "remove atton once");
    CHECK(!pt::removeTag(d, 0x55555555u, "atton"), "remove from missing section");
    { const pt::Section* s = pt::findGuid(d, 1); CHECK(s && pt::tagCount(*s) == 2 && !pt::findTag(*s, "atton") && pt::findTag(*s, "kreia"), "g1 after remove");
      CHECK(ser(d).find("; first playthrough\n") != std::string::npos && ser(d).find("[gdeadbeef]") != std::string::npos, "other content kept after remove"); }
    CHECK(pt::removeTag(d, 0xCAFEBABEu, "atton"), "remove last tag");
    { const pt::Section* s = pt::findGuid(d, 0xCAFEBABEu); CHECK(s && pt::tagCount(*s) == 0, "empty section kept in memory");
      CHECK(ser(d).find("[gcafebabe]\n") != std::string::npos, "empty section kept in the file");
      pt::Doc* d2 = new pt::Doc; load(*d2, ser(d)); CHECK(pt::findGuid(*d2, 0xCAFEBABEu) != nullptr, "empty section survives re-parse"); delete d2; }

    // 5. caps / bad input
    CHECK(!pt::setTag(d, 1, "", "p", false, true, 0) && !pt::setTag(d, 1, "t", "", false, true, 0) && !pt::setTag(d, 1, "a=b", "p", false, true, 0) &&
          !pt::setTag(d, 1, "t", "a|b", false, true, 0), "bad setTag args rejected");
    { std::string big = "[g00000001]\n";
      for (int i = 0; i < pt::MaxTags + 1; ++i) { char b[64]; snprintf(b, sizeof b, "t%02d=p%02d\n", i, i); big += b; }
      load(d, big);
      const pt::Section* s = pt::findGuid(d, 1);
      CHECK(s && pt::tagCount(*s) == pt::MaxTags, "tag cap %d", s ? pt::tagCount(*s) : -1);
      CHECK(!pt::setTag(d, 1, "extra", "p", false, true, 0), "setTag at the tag cap fails");
      CHECK(ser(d).find("t16=p16\n") != std::string::npos || d.lossy, "17th tag line kept verbatim or flagged lossy"); }
    { std::string many;
      for (int i = 0; i < pt::MaxSections + 2; ++i) { char b[32]; snprintf(b, sizeof b, "[g%08x]\n", (unsigned)(i + 1)); many += b; }
      load(d, many);
      CHECK(d.lossy && d.n == pt::MaxSections + 1, "section cap lossy=%d n=%d", d.lossy, d.n); }
    { std::string longl = "[g00000001]\n; " + std::string(400, 'x') + "\n";
      load(d, longl); CHECK(d.lossy, "over-long line flagged lossy"); }
    load(d, "");
    CHECK(d.n == 1 && !d.lossy && ser(d).empty(), "empty file");

    // 0.3.1 pause field: missing = 0, written only when > 0, garbage / negative / zero = 0, order-independent, round trip, setTag
    load(d, "[g000000aa]\natton=a__b|instant=0|hold=1|pause=6\nbao=b__c|pause=3|instant=1\nkreia=k__x|instant=0|hold=1|pause=abc\nhk=h__x|pause=-4\nt3=t__x|pause=0\nmand=m__x|hold=0\n");
    { const pt::Section* s = pt::findGuid(d, 0xaa);
      const pt::Line* l1 = s ? pt::findTag(*s, "atton") : nullptr; CHECK(l1 && l1->pause == 6 && !l1->instant && l1->hold, "pause=6 parsed");
      const pt::Line* l2 = s ? pt::findTag(*s, "bao") : nullptr; CHECK(l2 && l2->pause == 3 && l2->instant, "pause before instant (order-independent)");
      const pt::Line* l3 = s ? pt::findTag(*s, "kreia") : nullptr; CHECK(l3 && l3->pause == 0, "garbage pause -> 0 (lenient, like instant/hold; not lossy)");
      const pt::Line* l4 = s ? pt::findTag(*s, "hk") : nullptr; CHECK(l4 && l4->pause == 0, "negative pause -> 0");
      const pt::Line* l5 = s ? pt::findTag(*s, "t3") : nullptr; CHECK(l5 && l5->pause == 0, "pause=0 -> 0");
      const pt::Line* l6 = s ? pt::findTag(*s, "mand") : nullptr; CHECK(l6 && l6->pause == 0 && !l6->hold, "missing pause -> 0");
      CHECK(!d.lossy, "pause parsing never flags lossy"); }
    CHECK(ser(d) == "[g000000aa]\natton=a__b|instant=0|hold=1|pause=6\nbao=b__c|instant=1|hold=1|pause=3\nkreia=k__x|instant=0|hold=1\nhk=h__x|instant=0|hold=1\nt3=t__x|instant=0|hold=1\nmand=m__x|instant=0|hold=0\n", "pause serialisation:\n%s", ser(d).c_str());
    { std::string o = ser(d); pt::Doc* d2 = new pt::Doc; load(*d2, o); CHECK(ser(*d2) == o, "pause lines are a fixed point"); delete d2; }
    CHECK(pt::setTag(d, 0xaa, "atton", "a__b", false, true, 5) && pt::findTag(*pt::findGuid(d, 0xaa), "atton")->pause == 5, "setTag with pause");
    CHECK(pt::setTag(d, 0xaa, "atton", "a__b", false, true, -2) && pt::findTag(*pt::findGuid(d, 0xaa), "atton")->pause == 0, "setTag negative pause stored as 0");
    CHECK(pt::setTag(d, 0xaa, "atton", "a__b", false, true, 7) && pt::setTag(d, 0xaa, "atton", "a__b", false, true, 0) && ser(d).find("atton=a__b|instant=0|hold=1\n") != std::string::npos && ser(d).find("atton=a__b|instant=0|hold=1|pause") == std::string::npos, "setTag pause 0 drops the field");
    { pt::Doc* d3 = new pt::Doc; load(*d3, "[g00000001]\nx=y__z|instant=1|hold=0\n"); CHECK(ser(*d3) == "[g00000001]\nx=y__z|instant=1|hold=0\n", "pause-less line round-trips byte-identical"); delete d3; }

    printf("playthrough_test checks %d failures %d\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
