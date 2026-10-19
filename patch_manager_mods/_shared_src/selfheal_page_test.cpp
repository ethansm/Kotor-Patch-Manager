// Host test of SelfHealPage.h: selection, row values per selection, every click path, the two-click apply, persistence through serializeFile.
#include "SelfHealPage.h"
#include "ModConfigDecl.h"
#include "check.h"

using namespace shpage;

static shroster::RosterEntry mk(const char* tag, const char* name, bool inParty) {
    shroster::RosterEntry e;
    memset(&e, 0, sizeof e);
    strncpy(e.tag, tag, sizeof e.tag - 1);
    strncpy(e.name, name, sizeof e.name - 1);
    e.inParty = inParty;
    return e;
}

struct Fixture {
    shui::CfgTable t;
    Model m;
    Fixture() {
        t.def = shui::CompCfg{true, 50, 3};
        shui::upsert(t, "atton", shui::CompCfg{false, 20, 1});
        m.t = &t;
        shroster::RosterEntry r[4] = {mk("", "Everyone (defaults)", false), mk("player", "Exile", true), mk("atton", "Atton", true), mk("kreia", "Kreia", true)};
        m.setRoster(r, 4);
    }
};

static void test_view_defaults_and_character() {
    Fixture f;
    RowView v[R_COUNT];
    f.m.view(v, 0);                                              // entry 0: defaults
    KT_CHECK(v[R_AUTO].value == 1 && v[R_PCT].value == 50 && v[R_KEEP].value == 3);
    KT_CHECK(v[R_AUTO].flags == 0 && v[R_PCT].flags == 0 && v[R_KEEP].flags == 0);
    KT_CHECK(v[R_FOLLOW].flags == MUI_RF_HIDDEN);
    KT_CHECK(strcmp(v[R_WHO].text, "Character: Everyone (defaults)") == 0 && v[R_WHO].maxV == 3);
    KT_CHECK(strcmp(v[R_RESET].text, "Reset defaults to factory") == 0 && v[R_RESET].flags == 0);
    KT_CHECK(strcmp(v[R_APPLY].text, "Apply defaults to everyone") == 0 && v[R_APPLY].flags == 0);
    f.m.sel = 2;                                                 // atton has a row of its own
    f.m.view(v, 0);
    KT_CHECK(v[R_AUTO].value == 0 && v[R_PCT].value == 20 && v[R_KEEP].value == 1);
    KT_CHECK(v[R_AUTO].flags == 0 && v[R_FOLLOW].flags == 0 && v[R_FOLLOW].value == 0);
    KT_CHECK(strcmp(v[R_RESET].text, "Reset to global") == 0 && v[R_RESET].flags == 0);
    f.m.sel = 3;                                                 // kreia follows the defaults
    f.m.view(v, 0);
    KT_CHECK(v[R_AUTO].value == 1 && v[R_PCT].value == 50 && v[R_KEEP].value == 3);   // shows the defaults
    KT_CHECK(v[R_AUTO].flags == MUI_RF_DISABLED && v[R_PCT].flags == MUI_RF_DISABLED && v[R_KEEP].flags == MUI_RF_DISABLED);
    KT_CHECK(v[R_FOLLOW].value == 1 && v[R_RESET].flags == MUI_RF_DISABLED);
    f.t.rows.clear();
    f.m.view(v, 0);
    KT_CHECK(v[R_APPLY].flags == MUI_RF_DISABLED);               // nothing to apply
    Model none;
    none.view(v, 0);                                             // no table: no crash
    KT_CHECK(v[R_APPLY].flags == MUI_RF_DISABLED && v[R_AUTO].value == 0);
    f.m.roster[1].inParty = false;
    f.m.sel = 1;
    f.m.view(v, 0);
    KT_CHECK(strstr(v[R_WHO].text, "(not in party)") != nullptr);
}

static void test_character_switch() {
    Fixture f;
    Outcome o = f.m.change(R_WHO, 2, 0);
    KT_CHECK(o.final == 2 && f.m.sel == 2 && o.refresh && !o.cfgChanged && f.m.key() == "atton");
    o = f.m.change(R_WHO, 99, 0);                                // clamped
    KT_CHECK(o.final == 3 && f.m.sel == 3);
    o = f.m.change(R_WHO, -4, 0);
    KT_CHECK(o.final == 0 && f.m.sel == 0 && f.m.key().empty() && f.m.defaultsSelected());
    // setRoster keeps the selection on the same tag, else falls back to the defaults entry
    f.m.sel = 3;
    shroster::RosterEntry r2[3] = {mk("", "Everyone (defaults)", false), mk("kreia", "Kreia", true), mk("atton", "Atton", true)};
    f.m.setRoster(r2, 3);
    KT_CHECK(f.m.sel == 1 && f.m.key() == "kreia");
    shroster::RosterEntry r3[2] = {mk("", "Everyone (defaults)", false), mk("atton", "Atton", true)};
    f.m.setRoster(r3, 2);
    KT_CHECK(f.m.sel == 0);
    f.m.setRoster(nullptr, 0);                                   // empty roster: just the defaults entry
    KT_CHECK(f.m.nRoster == 1 && f.m.sel == 0 && strcmp(f.m.roster[0].name, "Everyone (defaults)") == 0);
}

static void test_edit_defaults_and_character() {
    Fixture f;
    Outcome o = f.m.change(R_PCT, 55, 0);                        // defaults entry
    KT_CHECK(o.cfgChanged && o.final == 55 && f.t.def.hpPct == 55 && f.t.rows.size() == 1);
    o = f.m.change(R_PCT, 57, 0);                                // snapped to a multiple of 5
    KT_CHECK(o.final == 55 && !o.cfgChanged);
    o = f.m.change(R_PCT, 500, 0);
    KT_CHECK(o.final == 95 && f.t.def.hpPct == 95);
    o = f.m.change(R_KEEP, 21, 0);
    KT_CHECK(o.final == 20 && f.t.def.keep == 20);
    o = f.m.change(R_AUTO, 0, 0);
    KT_CHECK(o.final == 0 && !f.t.def.autoOn && o.cfgChanged);
    f.m.sel = 2;                                                 // atton: own row
    o = f.m.change(R_KEEP, 4, 0);
    KT_CHECK(o.cfgChanged && o.final == 4 && shui::lookup(f.t, "atton").keep == 4 && shui::lookup(f.t, "atton").hpPct == 20);
    KT_CHECK(f.t.def.keep == 20);                                // defaults untouched
    o = f.m.change(R_AUTO, 1, 0);
    KT_CHECK(shui::lookup(f.t, "atton").autoOn);
    f.m.sel = 3;                                                 // kreia follows: edits are refused and change nothing
    const shui::CfgTable before = f.t;
    o = f.m.change(R_PCT, 30, 0);
    KT_CHECK(!o.cfgChanged && o.final == f.t.def.hpPct && f.t == before);
    o = f.m.change(R_AUTO, 1, 0);
    KT_CHECK(!o.cfgChanged && f.t == before);
    o = f.m.change(99, 1, 0);                                    // bad row
    KT_CHECK(!o.cfgChanged && f.t == before);
    o = f.m.change(-1, 1, 0);
    KT_CHECK(!o.cfgChanged);
}

static void test_follow_and_reset() {
    Fixture f;
    f.m.sel = 3;                                                 // kreia follows
    Outcome o = f.m.change(R_FOLLOW, 0, 0);                      // follow off: a row appears as a copy of the defaults
    KT_CHECK(o.cfgChanged && o.refresh && o.final == 0 && shui::hasRow(f.t, "kreia") && shui::lookup(f.t, "kreia") == f.t.def);
    f.t.def.hpPct = 10;                                          // later default changes no longer reach kreia
    KT_CHECK(shui::lookup(f.t, "kreia").hpPct == 50);
    o = f.m.change(R_FOLLOW, 0, 0);                              // already not following: no-op
    KT_CHECK(!o.cfgChanged);
    o = f.m.change(R_FOLLOW, 1, 0);                              // follow on: row erased
    KT_CHECK(o.cfgChanged && o.final == 1 && !shui::hasRow(f.t, "kreia") && shui::lookup(f.t, "kreia") == f.t.def);
    o = f.m.change(R_FOLLOW, 1, 0);
    KT_CHECK(!o.cfgChanged);
    f.m.sel = 0;
    o = f.m.change(R_FOLLOW, 0, 0);                              // hidden for the defaults entry
    KT_CHECK(!o.cfgChanged);
    // Reset
    f.m.sel = 2;                                                 // atton has a row
    o = f.m.change(R_RESET, 1, 0);
    KT_CHECK(o.cfgChanged && o.final == 0 && o.refresh && !shui::hasRow(f.t, "atton"));
    o = f.m.change(R_RESET, 1, 0);                               // already following: nothing to reset
    KT_CHECK(!o.cfgChanged && o.final == 0);
    f.m.sel = 0;
    f.t.def = shui::CompCfg{true, 70, 9};
    o = f.m.change(R_RESET, 1, 0);
    KT_CHECK(o.cfgChanged && f.t.def == shui::CompCfg());        // factory {false, 35, 2}
    KT_CHECK(f.t.def.hpPct == 35 && f.t.def.keep == 2 && !f.t.def.autoOn);
    o = f.m.change(R_RESET, 1, 0);
    KT_CHECK(!o.cfgChanged);
}

static void test_apply_two_click() {
    Fixture f;
    shui::upsert(f.t, "kreia", shui::CompCfg{true, 80, 0});
    RowView v[R_COUNT];
    Outcome o = f.m.change(R_APPLY, 1, 1000);                    // first click arms
    KT_CHECK(!o.cfgChanged && o.refresh && o.final == 0 && f.m.armed && f.t.rows.size() == 2);
    f.m.view(v, 1500);
    KT_CHECK(strcmp(v[R_APPLY].text, "Click again to confirm") == 0);
    o = f.m.change(R_APPLY, 1, 1000 + kConfirmMs - 1);           // second click inside the window applies
    KT_CHECK(o.cfgChanged && f.t.rows.empty() && !f.m.armed && f.t.def == (shui::CompCfg{true, 50, 3}));
    f.m.view(v, 5000);
    KT_CHECK(strcmp(v[R_APPLY].text, "Apply defaults to everyone") == 0);
    // window expiry
    shui::upsert(f.t, "kreia", shui::CompCfg{true, 80, 0});
    o = f.m.change(R_APPLY, 1, 10000);
    KT_CHECK(f.m.armed && !f.m.tick(10000 + kConfirmMs - 1) && f.m.armed);
    KT_CHECK(f.m.tick(10000 + kConfirmMs) && !f.m.armed);        // expired: tick reports it once
    KT_CHECK(!f.m.tick(10000 + kConfirmMs + 1));
    o = f.m.change(R_APPLY, 1, 20000);                           // arm again; a click after the window only re-arms
    o = f.m.change(R_APPLY, 1, 20000 + kConfirmMs + 5);
    KT_CHECK(!o.cfgChanged && f.m.armed && f.t.rows.size() == 1);
    // any other row disarms
    o = f.m.change(R_PCT, 60, 20000 + kConfirmMs + 6);
    KT_CHECK(!f.m.armed && o.refresh);
    o = f.m.change(R_APPLY, 1, 30000);
    KT_CHECK(f.m.armed);
    o = f.m.change(R_WHO, 1, 30010);
    KT_CHECK(!f.m.armed);
    // empty table: nothing to arm
    f.t.rows.clear();
    o = f.m.change(R_APPLY, 1, 40000);
    KT_CHECK(!f.m.armed && !o.cfgChanged);
    // uint32 wrap of the tick counter
    shui::upsert(f.t, "kreia", shui::CompCfg{true, 80, 0});
    const uint32_t nearWrap = 0xFFFFFFFFu - 1000u;
    o = f.m.change(R_APPLY, 1, nearWrap);
    KT_CHECK(f.m.armed);
    o = f.m.change(R_APPLY, 1, nearWrap + 2000u);                // crosses zero, still inside 3 s
    KT_CHECK(o.cfgChanged && f.t.rows.empty());
}

static void test_persistence_round_trip() {
    Fixture f;
    const std::string orig = "[defaults]\ndefault=auto:1,hp:50,keep:3\n\n[companions]\natton=auto:0,hp:20,keep:1\n";
    f.m.sel = 3;
    f.m.change(R_FOLLOW, 0, 0);
    f.m.change(R_PCT, 70, 0);
    std::string file = shui::serializeFile(f.t, orig);
    shui::CfgTable back = shui::parseFile(file);
    KT_CHECK(back == f.t);
    KT_CHECK(shui::lookup(back, "kreia").hpPct == 70 && shui::lookup(back, "atton").keep == 1);
    f.m.sel = 3;
    f.m.change(R_RESET, 1, 0);
    file = shui::serializeFile(f.t, file);
    back = shui::parseFile(file);
    KT_CHECK(back == f.t && !shui::hasRow(back, "kreia"));      // the erase persisted
    KT_CHECK(file.find("kreia") == std::string::npos);
}

static void test_shipped_schema() {   // the 10-18 first install: a 140-char desc made TomlMini reject the whole page
    static mcdecl::PageDecl pd;
    char err[96];
    const bool ok = mcdecl::parseSchema(kPageSchema, strlen(kPageSchema), pd, err, sizeof err);
    if (!ok) printf("schema error: %s\n", err);
    KT_CHECK(ok);
    KT_CHECK_EQ(pd.nRows, (int)R_COUNT);
    const char* ids[R_COUNT] = {"who", "auto", "pct", "keep", "follow", "reset", "apply"};
    for (int i = 0; i < (int)R_COUNT && i < pd.nRows; ++i) KT_CHECK(strcmp(pd.rows[i].id, ids[i]) == 0);
    KT_CHECK(strcmp(pd.name, "Companion Self-Heal") == 0);
}

int main() {
    test_shipped_schema();
    test_view_defaults_and_character();
    test_character_switch();
    test_edit_defaults_and_character();
    test_follow_and_reset();
    test_apply_two_click();
    test_persistence_round_trip();
    return kt::Report("selfheal_page_test");
}
