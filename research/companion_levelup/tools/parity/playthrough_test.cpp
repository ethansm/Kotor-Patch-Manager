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
    CHECK(ser(d) == "[G0000000A]\r\nhk47=hk47__gunner|instant=0|hold=1\r\nt3m4=t3m4__x|instant=0|hold=0\r\nbastila=bastila__y|instant=1|hold=1\r\n", "normalised output keeps the file's CRLF (F3):\n%s", ser(d).c_str());

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

    // F3 (A2 review): a rewrite keeps text after `]` on section lines, inline `;` comments on tag lines and the file's line endings
    {
        const std::string hdr = "[g00000001] ; my first run\n; note\natton=atton__a|instant=1|hold=0 ; keep me\nkreia=kreia__k ;and me\n[ GDEADBEEF ]   ; spaced\nbao=bao__b\n";
        load(d, hdr);
        CHECK(!d.lossy && pt::findTag(*pt::findGuid(d, 1), "kreia") && !strcmp(pt::findTag(*pt::findGuid(d, 1), "kreia")->preset, "kreia__k"), "inline comment is not part of the preset id");
        CHECK(pt::setTag(d, 1, "atton", "atton__z", false, true, 0), "setTag on a commented line");
        const std::string o = ser(d);
        CHECK(o.find("[g00000001] ; my first run\n") != std::string::npos, "text after ] kept:\n%s", o.c_str());
        CHECK(o.find("[ GDEADBEEF ]   ; spaced\n") != std::string::npos, "second header kept verbatim:\n%s", o.c_str());
        CHECK(o.find("kreia=kreia__k|instant=0|hold=1 ;and me\n") != std::string::npos, "unchanged tag line keeps its inline comment:\n%s", o.c_str());
        CHECK(o.find("atton=atton__z|instant=0|hold=1 ; keep me\n") != std::string::npos, "changed tag line keeps its inline comment:\n%s", o.c_str());
        pt::Doc* d2 = new pt::Doc; load(*d2, o); CHECK(ser(*d2) == o, "commented file is a fixed point"); delete d2;
        // CRLF: detected per file, written back; LF and new files stay LF
        std::string crlf; for (char c : hdr) { if (c == '\n') crlf += '\r'; crlf += c; }
        load(d, crlf);
        CHECK(pt::setTag(d, 1, "kreia", "kreia__q", false, true, 0), "setTag on CRLF file");
        const std::string oc = ser(d); bool lone = false;
        for (size_t i = 0; i < oc.size(); ++i) if (oc[i] == '\n' && (i == 0 || oc[i - 1] != '\r')) lone = true;
        CHECK(!lone && oc.find("[g00000001] ; my first run\r\n") != std::string::npos && oc.find("kreia=kreia__q|instant=0|hold=1 ;and me\r\n") != std::string::npos, "CRLF kept on every line:\n%s", oc.c_str());
        load(d, hdr); CHECK(pt::setTag(d, 1, "kreia", "kreia__q", false, true, 0) && ser(d).find('\r') == std::string::npos, "LF file stays LF");
        load(d, ""); CHECK(pt::setTag(d, 7, "atton", "a__b", false, true, 0) && ser(d) == "[g00000007]\natton=a__b|instant=0|hold=1\n", "new file is LF");
    }

    // F6: removeTagRaw deletes exactly one line of a LOSSY file (over-long comment) and nothing else
    {
        const std::string longc = "; " + std::string(300, 'a');
        const std::string raw = "pre=1\r\n[g00000005] ; a\r\nAtton = x__y|instant=0|hold=1 ; c\r\n" + longc + "\r\nkreia=k__z\r\n[g00000005]\natton=second\r\n[g00000006]\natton=other\r\n";
        load(d, raw); CHECK(d.lossy, "long comment line is lossy");
        std::string t = raw; size_t n = t.size();
        CHECK(pt::removeTagRaw(&t[0], &n, 5, "ATTON"), "raw removal finds the first line of the first section");
        t.resize(n);
        CHECK(t == "pre=1\r\n[g00000005] ; a\r\n" + longc + "\r\nkreia=k__z\r\n[g00000005]\natton=second\r\n[g00000006]\natton=other\r\n", "only that line is gone:\n%s", t.c_str());
        size_t n2 = t.size(); std::string t2 = t;
        CHECK(!pt::removeTagRaw(&t2[0], &n2, 5, "atton") && n2 == t.size() && t2 == t, "second section of the same GUID is never touched; absent tag -> false");
        CHECK(!pt::removeTagRaw(&t2[0], &n2, 9, "atton") && !pt::removeTagRaw(&t2[0], &n2, 6, "kreia") && t2 == t, "other guid / tag in another section -> false, unchanged");
        std::string t3 = "[g00000007]\natton=a__b"; size_t n3 = t3.size();
        CHECK(pt::removeTagRaw(&t3[0], &n3, 7, "atton") && n3 == 12 && t3.substr(0, n3) == "[g00000007]\n", "last line without a newline");
    }

    // A3 mutation gaps
    {
        std::unique_ptr<pt::Doc> d2(new pt::Doc);
        load(*d2, "[g00000001]\r\natton=a__b\n");   // 1 CRLF of 2 newline-terminated lines: not a majority -> LF
        CHECK(!d2->crlf, "CRLF tie is not CRLF");
        load(*d2, "[g00000001]\r\natton=a__b\r\nkreia=c__d\n");
        CHECK(d2->crlf, "CRLF majority (2 of 3)");
        load(*d2, "[g0000000a]\natton=p|instant=0\t; note\n");   // a TAB before the ';' belongs to the inline comment tail
        CHECK(ser(*d2).find("|hold=1\t; note\n") != std::string::npos, "inline comment keeps the TAB before ';': '%s'", ser(*d2).c_str());
        load(*d2, "[g0000000a]\natton=p ; note\n");
        CHECK(ser(*d2).find("|hold=1 ; note\n") != std::string::npos, "inline comment keeps the blank before ';'");
        // removeTagRaw only searches sections of ITS guid: the tag in a section of another guid, then the right one
        std::string t = "[g00000001]\natton=a__b\n[g00000002]\nkreia=c__d\natton=e__f\n"; size_t n = t.size();
        CHECK(pt::removeTagRaw(&t[0], &n, 2, "atton") && t.substr(0, n) == "[g00000001]\natton=a__b\n[g00000002]\nkreia=c__d\n", "guid 2 atton removed, guid 1 untouched: '%s'", t.substr(0, n).c_str());
        std::string u = "[g00000001]\nkreia=a__b\n[g00000002]\natton=e__f\n"; size_t un = u.size();
        CHECK(pt::removeTagRaw(&u[0], &un, 2, "atton") && u.substr(0, un) == "[g00000001]\nkreia=a__b\n[g00000002]\n", "section of another guid without the tag does not end the search: '%s'", u.substr(0, un).c_str());
    }

    // R10 filter fields: |pcfilter=/|ffilter=/|pfilter= are written only when set (ffilter/pfilter `all` = default), parse tolerant, raw tokens, round trip
    {
        std::unique_ptr<pt::Doc> f(new pt::Doc);
        load(*f, "[g000000bb]\natton=a__b|instant=0|hold=1|pause=4|pcfilter=melee/single/light|ffilter=build_hide|pfilter=combat ; n\nbao=b__c|pfilter=force|pcfilter=CASTER/none/dark\nkreia=k__x|ffilter=all|pfilter=all\nhk=h__x|pcfilter=bad value!|ffilter=|pfilter=waytoolongtokenvalue\nt3=t__x|pcfilter=a/b/c/d/e/f/g/h/i/j/k/l/m|ffilter=bad-char\nmand=m__x|pcfilter=\n");
        const pt::Section* s = pt::findGuid(*f, 0xbb);
        const pt::Line* a = s ? pt::findTag(*s, "atton") : nullptr;
        CHECK(a && !strcmp(a->pcfilter, "melee/single/light") && !strcmp(a->ffilter, "build_hide") && !strcmp(a->pfilter, "combat") && a->pause == 4, "all three filter fields parsed");
        const pt::Line* b = s ? pt::findTag(*s, "bao") : nullptr;
        CHECK(b && !strcmp(b->pcfilter, "caster/none/dark") && !*b->ffilter && !strcmp(b->pfilter, "force"), "order-independent, lowercased");
        const pt::Line* c = s ? pt::findTag(*s, "kreia") : nullptr;
        CHECK(c && !*c->pcfilter && !*c->ffilter && !*c->pfilter, "`all` is the default and stored empty");
        for (const char* t : {"hk", "t3", "mand"}) { const pt::Line* g = s ? pt::findTag(*s, t) : nullptr; CHECK(g && !*g->pcfilter && !*g->ffilter && !*g->pfilter, "garbage / over-long / empty values -> unset (%s)", t); }
        CHECK(!f->lossy, "filter parsing never flags lossy");
        const std::string want = "[g000000bb]\natton=a__b|instant=0|hold=1|pause=4|pcfilter=melee/single/light|ffilter=build_hide|pfilter=combat ; n\nbao=b__c|instant=0|hold=1|pcfilter=caster/none/dark|pfilter=force\nkreia=k__x|instant=0|hold=1\nhk=h__x|instant=0|hold=1\nt3=t__x|instant=0|hold=1\nmand=m__x|instant=0|hold=1\n";
        CHECK(ser(*f) == want, "filter serialisation:\n%s", ser(*f).c_str());
        { std::string o = ser(*f); std::unique_ptr<pt::Doc> f2(new pt::Doc); load(*f2, o); CHECK(ser(*f2) == o, "filter lines are a fixed point"); }
        // setTag keeps the filter fields of an existing line; a new line starts unset; setFilters edits an existing line only
        CHECK(pt::setTag(*f, 0xbb, "atton", "a__b", true, true, 0) && ser(*f).find("atton=a__b|instant=1|hold=1|pcfilter=melee/single/light|ffilter=build_hide|pfilter=combat ; n\n") != std::string::npos, "setTag preserves the filter fields");
        CHECK(pt::setTag(*f, 0xbb, "new", "n__p", false, true, 0) && !*pt::findTag(*pt::findGuid(*f, 0xbb), "new")->pcfilter, "new line: filters unset");
        CHECK(pt::setFilters(*f, 0xbb, "new", "Tank/Rifle/None", "build_dim", "all") && ser(*f).find("new=n__p|instant=0|hold=1|pcfilter=tank/rifle/none|ffilter=build_dim\n") != std::string::npos, "setFilters sets the fields (lowercased, all dropped): %s", ser(*f).c_str());
        CHECK(pt::setFilters(*f, 0xbb, "new", nullptr, "", "x y") && ser(*f).find("new=n__p|instant=0|hold=1\n") != std::string::npos, "setFilters null / empty / invalid clears");
        CHECK(!pt::setFilters(*f, 0xbb, "nope", "a", "b", "c") && !pt::setFilters(*f, 0x99, "new", "a", "b", "c") && !pt::setFilters(*f, 0xbb, nullptr, "a", "b", "c"), "setFilters: absent tag / section / null -> false");
        // defaults are not written
        CHECK(pt::setTag(*f, 0xcc, "x", "y__z", false, true, 0) && ser(*f).find("x=y__z|instant=0|hold=1\n") != std::string::npos && ser(*f).substr(ser(*f).find("[g000000cc]")).find("filter") == std::string::npos, "default line has no filter fields");
        std::unique_ptr<pt::Doc> f3(new pt::Doc); load(*f3, "[g00000001]\nx=y__z|instant=1|hold=0\n; keep\n");
        CHECK(ser(*f3) == "[g00000001]\nx=y__z|instant=1|hold=0\n; keep\n", "filter-less line round-trips byte-identical");
        // CRLF + filters
        std::unique_ptr<pt::Doc> f4(new pt::Doc); load(*f4, "[g00000002]\r\nx=y__z|pcfilter=melee/dual/dark\r\n");
        CHECK(ser(*f4) == "[g00000002]\r\nx=y__z|instant=0|hold=1|pcfilter=melee/dual/dark\r\n", "CRLF kept with filter fields");
        // worst-case line fits MaxText
        std::string big = "[g00000003]\n"; for (int i = 0; i < pt::MaxTags; ++i) { char l[400]; snprintf(l, sizeof l, "t%02d=preset_%036d|instant=1|hold=0|pause=2147483647|pcfilter=melee/unarmed/neutral|ffilter=build_hide|pfilter=build_hide ; %0200d\n", i, i, i); big += l; }
        std::unique_ptr<pt::Doc> f5(new pt::Doc); load(*f5, big); CHECK(ser(*f5).size() > 0, "worst-case document serialises within MaxText");
    }

    printf("playthrough_test checks %d failures %d\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
