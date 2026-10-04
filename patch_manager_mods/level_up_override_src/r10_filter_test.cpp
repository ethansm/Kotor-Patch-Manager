// r10_filter_test.cpp -- host test of R10Filter.h (R10 PC level-up feat/power filter: modes, profile, tag table, fit rule, soft-lock fallback).
// argv[1] = path to the real research/companion_levelup/runtime/r10_tags.ini (test.toml args; run cwd = this directory).
#include "R10Filter.h"
#include "check.h"
#include <string.h>
#include <string>

using namespace r10f;

static Profile prof(const char* s) { Profile p; KT_CHECK(parseProfile(s, &p)); return p; }
static bool parseOk(const char* text, TagTable* t) { char e[96]; return parseTagIni(text, t, e, sizeof e); }
static std::string parseErr(const char* text) { static TagTable t; char e[96] = "unset"; bool ok = parseTagIni(text, &t, e, sizeof e); return ok ? std::string("OK") : std::string(e); }

int main(int argc, char** argv) {
    { // mode cycle + names
        Mode m = All;
        const Mode order[] = {BuildDim, BuildHide, Combat, Force, All};
        for (Mode want : order) { m = nextMode(m); KT_CHECK_EQ((int)m, (int)want); }
        KT_CHECK_EQ(modeName(All), "all"); KT_CHECK_EQ(modeName(BuildDim), "build_dim"); KT_CHECK_EQ(modeName(BuildHide), "build_hide");
        KT_CHECK_EQ(modeName(Combat), "combat"); KT_CHECK_EQ(modeName(Force), "force");
        for (int i = 0; i < ModeCount; ++i) KT_CHECK_EQ((int)parseMode(modeName((Mode)i)), i);
        KT_CHECK_EQ((int)parseMode("bogus"), (int)All); KT_CHECK_EQ((int)parseMode(""), (int)All); KT_CHECK_EQ((int)parseMode(nullptr), (int)All);
        KT_CHECK_EQ((int)parseMode("BUILD_DIM"), (int)All);   // tokens are lowercase, exact
        KT_CHECK(hides(BuildHide) && !hides(BuildDim) && !hides(All) && !hides(Combat) && !hides(Force));
        KT_CHECK(dims(BuildDim) && dims(Combat) && dims(Force) && !dims(All) && !dims(BuildHide));
    }
    { // profile parse
        Profile p;
        KT_CHECK(parseProfile("melee/single/light", &p) && p.valid() && p.role == T_Melee && p.style == T_Single && p.align == T_Light);
        KT_CHECK(parseProfile("caster/none/dark", &p) && p.role == T_Caster && p.style == 0 && p.align == T_Dark);
        KT_CHECK(parseProfile("tank/rifle/none", &p) && p.style == T_Rifle && p.align == 0);
        KT_CHECK(parseProfile("support/unarmed/neutral", &p) && p.align == T_Neutral);
        const char* bad[] = {"", "melee", "melee/single", "melee/single/light/x", "force/single/light", "melee/light/light", "melee/single/melee", "melee/single/general",
                             "none/single/light", "Melee/single/light", "melee//light", "/single/light", "melee/single/", "melee/single/lightlightlightlightlightlightlightlightlight"};
        for (const char* b : bad) { Profile q; q.role = T_Tank; KT_CHECK(!parseProfile(b, &q)); KT_CHECK(!q.valid() && q.role == 0 && q.style == 0 && q.align == 0); }
        KT_CHECK(!parseProfile(nullptr, &p) && !p.valid());
    }
    { // tag table parser: tolerance
        TagTable* t = new TagTable;
        KT_CHECK(parseOk("; c\r\n# c2\r\n\r\n[feats]\r\n1=general,combat\r\n 3 = melee , dual \r\n[powers]\r\n7=force,caster\r\n9=\r\n", t));
        KT_CHECK_EQ(t->nFeats, 2); KT_CHECK_EQ(t->nPowers, 2);
        KT_CHECK_EQ((int)t->tags(Feat, 1), (int)(T_General | T_Combat)); KT_CHECK_EQ((int)t->tags(Feat, 3), (int)(T_Melee | T_Dual));
        KT_CHECK_EQ((int)t->tags(Power, 7), (int)(T_Force | T_Caster)); KT_CHECK_EQ((int)t->tags(Feat, 7), 0);   // sections are separate
        KT_CHECK(t->known(Power, 9) && t->tags(Power, 9) == 0 && !t->known(Power, 8) && !t->known(Feat, 99999));
        KT_CHECK(parseOk("[feats]\n1=melee", t) && t->nFeats == 1);   // no trailing newline
        KT_CHECK(parseOk("", t) && t->nFeats == 0 && t->nPowers == 0);
        KT_CHECK(parseOk("[feats]\n4095=tank\n", t) && t->tags(Feat, 4095) == T_Tank);
        delete t;
    }
    { // parser errors carry the line number; an error leaves an EMPTY table
        KT_CHECK_EQ(parseErr("[feats]\n1=bogus\n").c_str(), "line 2: unknown tag word");
        KT_CHECK_EQ(parseErr("[feats]\n1=melee,Melee\n").c_str(), "line 2: unknown tag word");   // case-sensitive
        KT_CHECK_EQ(parseErr("[feats]\nx=melee\n").c_str(), "line 2: bad id");
        KT_CHECK_EQ(parseErr("[feats]\n=melee\n").c_str(), "line 2: bad id");
        KT_CHECK_EQ(parseErr("; a\n\n[powers]\n-1=force\n").c_str(), "line 4: bad id");
        KT_CHECK_EQ(parseErr("[feats]\n4096=melee\n").c_str(), "line 2: id out of range");
        KT_CHECK_EQ(parseErr("[feats]\n99999999999999=melee\n").c_str(), "line 2: id out of range");
        KT_CHECK_EQ(parseErr("1=melee\n[feats]\n").c_str(), "line 1: row outside a section");
        KT_CHECK_EQ(parseErr("[feats]\n\n[bogus]\n").c_str(), "line 3: unknown section");   // unknown section = ERROR (a typo must not drop rows)
        KT_CHECK_EQ(parseErr("[feats\n").c_str(), "line 1: bad section header");
        KT_CHECK_EQ(parseErr("[feats]\njunk\n").c_str(), "line 2: missing '='");
        KT_CHECK_EQ(parseErr("[feats]\n1=melee\n1=tank\n").c_str(), "line 3: duplicate id");
        KT_CHECK_EQ(parseErr("[feats]\n1=melee\n[powers]\n1=force\n").c_str(), "OK");   // same id in the other section is fine
        std::string longl = "[feats]\n1=melee," + std::string(300, 'x') + "\n";
        KT_CHECK_EQ(parseErr(longl.c_str()).c_str(), "line 2: line too long");
        TagTable* t = new TagTable;
        KT_CHECK(!parseOk("[feats]\n1=melee\n2=oops\n", t) && t->nFeats == 0 && !t->known(Feat, 1));
        char tiny[8];
        KT_CHECK(!parseTagIni("[feats]\n1=zz\n", t, tiny, sizeof tiny) && strlen(tiny) < sizeof tiny);   // truncated, still terminated
        KT_CHECK(!parseTagIni("[feats]\n1=zz\n", t, nullptr, 0));                                           // no err buffer is fine
        KT_CHECK(!parseTagIni(nullptr, t, nullptr, 0));
        delete t;
    }
    // Fit-rule cross-check against Python fits() in research/companion_levelup/r10/gen_r10_tags.py:
    //   general -> True; not (role in tags or style in tags) -> False; light row & dark build -> False; dark row & light build -> False; else True.
    // Each case: (tags, profile "role/style/align", expected) worked out from that function.
    {
        struct C { const char* tags; const char* prof; bool want; };
        const C cases[] = {
            {"general", "caster/none/dark", true},                      // general shortcut, nothing else matches
            {"general,light", "melee/single/dark", true},               // general beats the alignment conflict
            {"melee,single,combat", "melee/dual/neutral", true},        // role match
            {"melee,single,combat", "caster/single/light", true},       // style match alone
            {"melee,single,combat", "caster/dual/light", false},        // neither role nor style
            {"melee,combat", "melee/none/dark", true},                  // style none matches no style tag, role does
            {"single", "melee/none/light", false},                      // style none never matches a style tag
            {"force,caster,light", "caster/none/dark", false},          // light row vs dark build
            {"force,caster,dark", "caster/none/light", false},          // dark row vs light build
            {"force,caster,dark", "caster/none/none", true},            // alignment none conflicts with nothing
            {"force,caster,dark", "caster/none/neutral", true},         // neutral build conflicts with nothing
            {"force,caster,neutral", "caster/none/dark", true},         // neutral row never conflicts
            {"force,caster,light", "caster/none/light", true},          // same alignment
            {"force", "caster/none/light", false},                      // role-less force-only row fits only via role/style
            {"melee,tank", "ranged/rifle/dark", false},
        };
        for (const C& c : cases) {
            uint32_t m = 0;
            std::string s = c.tags;
            size_t pos = 0;
            while (pos <= s.size()) { size_t e = s.find(',', pos); if (e == std::string::npos) e = s.size(); m |= tagBit(s.substr(pos, e - pos).c_str()); pos = e + 1; }
            Profile p; KT_CHECK(parseProfile(c.prof, &p));
            if (fitsMask(m, p) != c.want) { std::printf("FAIL fit %s vs %s want %d\n", c.tags, c.prof, c.want); KT_CHECK(false); } else KT_CHECK(true);
        }
    }
    { // fits per mode (table-driven)
        TagTable* t = new TagTable;
        KT_CHECK(parseOk("[feats]\n1=general,combat\n2=melee,single,combat\n3=force,caster,light\n4=force,caster,dark\n5=general\n6=melee\n7=\n"
                         "[powers]\n1=force,caster,light\n2=force,melee,dark\n", t));
        const Profile dm = prof("melee/single/dark"), lc = prof("caster/none/light"), nn = prof("tank/rifle/none");
        for (Mode m : {All, BuildDim, BuildHide, Combat, Force}) {
            KT_CHECK(fits(*t, Feat, 9, dm, m));        // untagged / unknown id: always fits
            KT_CHECK(fits(*t, Feat, 7, dm, m));        // listed with an empty tag list: untagged
            KT_CHECK(fits(*t, Feat, 5, lc, m));        // general-only: fits every mode
            KT_CHECK(fits(*t, Feat, 1, lc, m));        // general+combat: fits every mode
            KT_CHECK(fits(*t, Feat, 60000, dm, m));    // out of table range
        }
        for (Kind k : {Feat, Power}) for (uint16_t id : {1, 2, 3, 4}) KT_CHECK(fits(*t, k, id, dm, All) && fits(*t, k, id, lc, All));   // All never filters
        KT_CHECK(fits(*t, Feat, 2, dm, BuildDim) && fits(*t, Feat, 2, dm, BuildHide));
        KT_CHECK(!fits(*t, Feat, 2, lc, BuildHide));                       // melee/single row, caster build
        KT_CHECK(fits(*t, Feat, 3, lc, BuildHide) && !fits(*t, Feat, 3, dm, BuildHide));   // caster row: role match for lc; melee build no match
        KT_CHECK(!fits(*t, Feat, 4, lc, BuildDim));                        // dark row, light build: opposite
        KT_CHECK(fits(*t, Feat, 4, nn, BuildDim) == false);                // tank/rifle: no role/style match
        KT_CHECK(fits(*t, Feat, 6, nn, BuildHide) == false && fits(*t, Feat, 6, dm, BuildHide));
        KT_CHECK(fits(*t, Power, 2, dm, BuildHide) && !fits(*t, Power, 1, dm, BuildHide));   // power 1 light caster: role no, style no
        KT_CHECK(fits(*t, Power, 1, lc, BuildHide));                // caster light vs light build: fits
        KT_CHECK(!fits(*t, Power, 2, prof("melee/single/light"), BuildHide));   // dark power vs light build
        // Combat / Force: tag test, general counts as both
        KT_CHECK(fits(*t, Feat, 2, dm, Combat) && !fits(*t, Feat, 2, dm, Force));
        KT_CHECK(!fits(*t, Feat, 3, dm, Combat) && fits(*t, Feat, 3, dm, Force));
        KT_CHECK(!fits(*t, Feat, 6, dm, Combat) && !fits(*t, Feat, 6, dm, Force));   // melee only: neither
        KT_CHECK(fits(*t, Power, 1, dm, Force) && !fits(*t, Power, 1, dm, Combat));
        KT_CHECK(fits(*t, Feat, 2, Profile(), Combat));                    // an empty profile does not affect the tag modes
        KT_CHECK(!fits(*t, Feat, 2, Profile(), BuildHide));                // empty profile matches no role/style -> filtered (callers gate on valid())
        KT_CHECK(fits(*t, Feat, 2, dm, ModeCount));                        // out-of-range mode never filters
        delete t;
    }
    { // filterChoosable
        TagTable* t = new TagTable;
        KT_CHECK(parseOk("[feats]\n1=melee,single\n2=caster\n3=melee\n4=caster\n5=general\n", t));
        const Profile m = prof("melee/single/none");
        const uint16_t ids[] = {4, 1, 9, 2, 3, 5, 7};
        uint16_t out[16];
        int k = filterChoosable(*t, Feat, m, BuildHide, ids, 7, 2, out);
        KT_CHECK_EQ(k, 5);
        const uint16_t want[] = {1, 9, 3, 5, 7};   // order preserved, untagged 9 and 7 kept, caster rows 4 and 2 dropped
        for (int i = 0; i < 5; ++i) KT_CHECK_EQ((int)out[i], (int)want[i]);
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, All, ids, 7, 2, out), 7);
        for (int i = 0; i < 7; ++i) KT_CHECK_EQ((int)out[i], (int)ids[i]);
        const uint16_t casters[] = {2, 4};
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, BuildHide, casters, 2, 1, out), -1);    // nothing kept, picks left: FALLBACK
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, BuildHide, casters, 2, 5, out), -1);
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, BuildHide, casters, 2, 0, out), 0);     // nothing kept, no picks left: empty is fine
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, BuildHide, casters, 2, -1, out), 0);
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, BuildHide, ids, 0, 3, out), -1);        // empty input with picks left, filtered mode: fallback
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, All, ids, 0, 3, out), 0);               // mode All is never a fallback
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, Combat, casters, 2, 1, out), -1);       // casters have no combat tag
        const uint16_t one[] = {1};
        KT_CHECK_EQ(filterChoosable(*t, Feat, m, BuildHide, one, 1, 1, out), 1);
        delete t;
    }
    { // dimInPlace (batch 2a): a fake panel vector of 10 ids, every mode, picks 1 / 0, both element widths
        TagTable* t = new TagTable;
        KT_CHECK(parseOk("[feats]\n1=melee,single,combat\n2=caster,force\n3=melee,combat\n4=caster,force,dark\n5=general\n6=ranged,rifle,combat\n"
                         "[powers]\n1=force,caster,light\n2=force,melee,dark\n", t));
        const Profile m = prof("melee/single/light");
        const uint16_t base[10] = {1, 2, 3, 4, 5, 6, 7, 8, 2, 1};   // 7, 8 untagged
        uint16_t v[12];
        int nc = -5;
        auto reset = [&] { memcpy(v, base, sizeof base); v[10] = 0xAAAA; v[11] = 0xBBBB; nc = -5; };
        reset();   // BuildDim: casters 2, 4 (x2 for 2) and ranged 6 go
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, BuildDim, v, 10, 1, &nc), (int)DimApplied);
        KT_CHECK_EQ(nc, 6);
        { const uint16_t want[] = {1, 3, 5, 7, 8, 1}; for (int i = 0; i < 6; ++i) KT_CHECK_EQ((int)v[i], (int)want[i]); }
        KT_CHECK(v[10] == 0xAAAA && v[11] == 0xBBBB);   // never writes past n
        reset();   // BuildHide behaves as BuildDim in batch 2a
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, BuildHide, v, 10, 0, &nc), (int)DimApplied);
        KT_CHECK_EQ(nc, 6);
        reset();   // Combat: drops the force-only casters 2, 4
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, Combat, v, 10, 1, &nc), (int)DimApplied);
        KT_CHECK_EQ(nc, 7);
        { const uint16_t want[] = {1, 3, 5, 6, 7, 8, 1}; for (int i = 0; i < 7; ++i) KT_CHECK_EQ((int)v[i], (int)want[i]); }
        reset();   // Force: keeps 2, 4, general 5, untagged 7, 8
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, Force, v, 10, 1, &nc), (int)DimApplied);
        KT_CHECK_EQ(nc, 6);
        { const uint16_t want[] = {2, 4, 5, 7, 8, 2}; for (int i = 0; i < 6; ++i) KT_CHECK_EQ((int)v[i], (int)want[i]); }
        for (Mode md : {All, ModeCount}) {   // All / unknown: off, untouched
            reset();
            KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, md, v, 10, 1, &nc), (int)DimOff);
            KT_CHECK_EQ(nc, 10); KT_CHECK(!memcmp(v, base, sizeof base));
        }
        reset();   // invalid profile in a build mode: off, untouched; tag modes ignore the profile
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, Profile(), BuildDim, v, 10, 1, &nc), (int)DimNoProfile);
        KT_CHECK_EQ(nc, 10); KT_CHECK(!memcmp(v, base, sizeof base));
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, Profile(), Combat, v, 10, 1, &nc), (int)DimApplied);
        KT_CHECK_EQ(nc, 7);
        const uint16_t fitAll[] = {1, 3, 5, 7};
        memcpy(v, fitAll, sizeof fitAll); nc = -5;
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, BuildDim, v, 4, 1, &nc), (int)DimNoChange);
        KT_CHECK_EQ(nc, 4);
        const uint16_t casters[] = {2, 4, 2};
        memcpy(v, casters, sizeof casters);   // nothing would remain: fallback with picks left, untouched
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, BuildDim, v, 3, 1, &nc), (int)DimFallback);
        KT_CHECK_EQ(nc, 3); KT_CHECK(!memcmp(v, casters, sizeof casters));
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, BuildDim, v, 3, 0, &nc), (int)DimApplied);   // no picks left: an empty vector is fine
        KT_CHECK_EQ(nc, 0);
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, BuildDim, v, 0, 1, &nc), (int)DimNoChange);   // empty in: nothing to drop, never a fallback
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, BuildDim, v, -1, 1, &nc), (int)DimBadCount);
        KT_CHECK_EQ(nc, -1);
        KT_CHECK_EQ((int)dimInPlace(*t, Feat, m, BuildDim, v, MaxVector + 1, 1, &nc), (int)DimBadCount);
        // 4-byte elements (powers vector if it holds uint32 ids); ids > 0xFFFF are never filtered
        uint32_t pw[5] = {1, 2, 0x10002, 9, 1};
        KT_CHECK_EQ((int)dimInPlace(*t, Power, prof("caster/none/light"), BuildDim, pw, 5, 1, &nc), (int)DimApplied);   // power 2: no role match + dark
        KT_CHECK_EQ(nc, 4);
        KT_CHECK(pw[0] == 1 && pw[1] == 0x10002 && pw[2] == 9 && pw[3] == 1);
        KT_CHECK_EQ(dimVerdictName(DimApplied), "applied"); KT_CHECK_EQ(dimVerdictName(DimFallback), "fallback"); KT_CHECK_EQ(dimVerdictName(99), "?");
        KT_CHECK_EQ((int)dimMode(BuildHide), (int)BuildDim); KT_CHECK_EQ((int)dimMode(Force), (int)Force);
        delete t;
    }
    { // the REAL generated table
        const char* path = argc > 1 ? argv[1] : "../../research/companion_levelup/runtime/r10_tags.ini";
        FILE* f = fopen(path, "rb");
        KT_CHECK(f != nullptr);
        if (f) {
            static char buf[65536];
            const size_t n = fread(buf, 1, sizeof buf - 1, f);
            fclose(f);
            buf[n] = 0;
            KT_CHECK(n > 100 && n < sizeof buf - 1);
            TagTable* t = new TagTable;
            char e[96];
            const bool ok = parseTagIni(buf, t, e, sizeof e);
            if (!ok) std::printf("parse error: %s\n", e);
            KT_CHECK(ok);
            KT_CHECK_EQ(t->nFeats, 82);
            KT_CHECK_EQ(t->nPowers, 73);
            KT_CHECK_EQ((int)t->tags(Feat, 1), (int)(T_General | T_Combat));
            KT_CHECK_EQ((int)t->tags(Feat, 3), (int)(T_Melee | T_Dual | T_Pistols | T_Combat));
            KT_CHECK((t->tags(Feat, 7) & T_General) != 0);
            // every power carries force + all five roles; every listed row is a tagged row
            int powerBad = 0, empty = 0;
            for (uint32_t id = 0; id < (uint32_t)MaxId; ++id) {
                if (t->known(Power, id) && (t->tags(Power, id) & (T_Force | RoleMask)) != (T_Force | RoleMask)) ++powerBad;
                if ((t->known(Feat, id) && !t->tags(Feat, id)) || (t->known(Power, id) && !t->tags(Power, id))) ++empty;
            }
            KT_CHECK_EQ(powerBad, 0);
            KT_CHECK_EQ(empty, 0);
            // Round trip through the filter: a light melee build never loses a general row and keeps fewer than all rows in BuildHide
            const Profile p = prof("melee/single/light");
            int kept = 0, total = 0;
            for (uint32_t id = 0; id < (uint32_t)MaxId; ++id) if (t->known(Feat, id)) { ++total; kept += fits(*t, Feat, (uint16_t)id, p, BuildHide); }
            KT_CHECK_EQ(total, 82);
            KT_CHECK(kept > 0 && kept < total);
            delete t;
        }
    }
    return kt::Report("r10_filter_test");
}
