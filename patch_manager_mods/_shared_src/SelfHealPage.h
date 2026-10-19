// SelfHealPage.h -- pure model of the Companion Self-Heal options page (doc 46 MCF4): which character is selected, what every row shows,
// and what a click does to the CfgTable. No engine, no mod-config calls: CompanionSelfHeal.cpp maps View -> Page::setValue/setText/
// setState and calls Model::change from the page's change hook, then saves the INI when Outcome::cfgChanged. Host test selfheal_page_test.cpp.
//
// Rows (schema order, enum Row):
//   WHO     dynamic wrap stepper over the roster; entry 0 = "Everyone (defaults)" edits [defaults]
//   AUTO    toggle     the selected character's cfg.autoOn
//   PCT     stepper    cfg.hpPct, 5..95 step 5 (the hook snaps to a multiple of 5)
//   KEEP    stepper    cfg.keep, 0..20
//   FOLLOW  toggle     On = the character has NO [companions] row and follows the defaults (hidden for entry 0)
//   RESET   button     (a toggle that always snaps back to 0) character: back to "follow global"; defaults entry: factory CompCfg{false,35,2}
//   APPLY   button     two-click arm: "Apply defaults to everyone" -> "Click again to confirm" for kConfirmMs -> erases every character row
// While a character follows the defaults, AUTO/PCT/KEEP are DISABLED and show the default values; turning FOLLOW off copies the defaults
// into a new row for the tag. Row keys are the roster tags (the page and the right-click popover edit the same CfgTable rows).
#pragma once
#include "ModConfigApi.h"
#include "SelfHealRoster.h"
#include "SelfHealUi.h"
#include <stdint.h>
#include <string>

namespace shpage {

// Page schema (ModConfigDecl.h / TomlMini.h subset: every string at most 95 bytes, enforced by selfheal_page_test).
constexpr const char* kPageSchema = R"TOML(
[page]
name  = "Companion Self-Heal"
title = "Companion Self-Heal"
desc  = "Auto-heal rules. Pick Everyone for the defaults, or a character for their own values."

[[row]]
id = "who"
kind = "dynamic"
label = "Character"
desc = "Who you are editing. Everyone sets the defaults. Absent characters can be set up ahead."
max = 1

[[row]]
id = "auto"
kind = "toggle"
label = "Auto-heal"
desc = "Use a healing item on this character automatically when their health drops below the threshold."

[[row]]
id = "pct"
kind = "stepper"
label = "Heal below"
desc = "Auto-heal triggers when health falls below this percentage."
default = 35
min = 5
max = 95
step = 5
fmt = "Heal below %d%%"

[[row]]
id = "keep"
kind = "stepper"
label = "Keep"
desc = "Items kept in reserve: auto-heal never uses the last N of an item. A manual click ignores this."
default = 2
min = 0
max = 20
step = 1
fmt = "Keep %d"

[[row]]
id = "follow"
kind = "toggle"
label = "Follow global"
desc = "On: uses the defaults. Off: gets their own copy that starts as the defaults."

[[row]]
id = "reset"
kind = "toggle"
label = "Reset"
desc = "Character: follow the defaults again. Everyone: factory defaults (off, 35%, keep 2)."

[[row]]
id = "apply"
kind = "toggle"
label = "Apply defaults to everyone"
desc = "Remove every character's own values so all follow the defaults. Click twice to confirm."
)TOML";


enum Row { R_WHO = 0, R_AUTO, R_PCT, R_KEEP, R_FOLLOW, R_RESET, R_APPLY, R_COUNT };
constexpr uint32_t kConfirmMs = 3000;

struct RowView {
    int value = 0, minV = 0, maxV = 1;
    int flags = 0;           // MUI_RF_DISABLED / MUI_RF_HIDDEN
    char text[72] = {};      // caption override; "" = the schema's own caption
};

struct Outcome {
    int final = 0;           // the value the provider must store for the clicked row
    bool cfgChanged = false; // the table changed: save the INI (key = Model::key())
    bool refresh = false;    // other rows changed too: re-push the whole View
};

inline int snap5(int v) { return shui::clampi(((v + 2) / 5) * 5, 5, 95); }

class Model {
public:
    shui::CfgTable* t = nullptr;
    shroster::RosterEntry roster[shroster::kMaxRoster];
    int nRoster = 1;
    int sel = 0;
    bool armed = false;
    uint32_t armedUntil = 0;

    Model() { memset(roster, 0, sizeof roster); strcpy(roster[0].name, "Everyone (defaults)"); }

    // Replaces the roster (party changed). The selection follows the same tag when it is still there, else falls back to entry 0.
    void setRoster(const shroster::RosterEntry* r, int n) {
        const std::string keep = sel > 0 && sel < nRoster ? roster[sel].tag : std::string();
        nRoster = n < 1 ? 1 : (n > shroster::kMaxRoster ? shroster::kMaxRoster : n);
        for (int i = 0; i < nRoster; ++i) roster[i] = (r && i < n) ? r[i] : roster[0];
        if (!r || n < 1) { memset(&roster[0], 0, sizeof roster[0]); strcpy(roster[0].name, "Everyone (defaults)"); }
        sel = keep.empty() ? 0 : shroster::indexOfTag(roster, nRoster, keep.c_str());
        armed = false;
    }

    std::string key() const { return sel > 0 && sel < nRoster ? std::string(roster[sel].tag) : std::string(); }
    bool defaultsSelected() const { return sel == 0; }
    bool following() const { return sel > 0 && t && !shui::hasRow(*t, key()); }
    shui::CompCfg current() const {
        if (!t) return shui::CompCfg();
        return sel == 0 ? t->def : shui::lookup(*t, key());
    }
    bool armLive(uint32_t now) const { return armed && static_cast<int32_t>(armedUntil - now) > 0; }

    void view(RowView out[R_COUNT], uint32_t now) const {
        const shui::CompCfg c = current();
        const bool lock = following();
        for (int i = 0; i < R_COUNT; ++i) out[i] = RowView();
        out[R_WHO].value = sel;
        out[R_WHO].minV = 0;
        out[R_WHO].maxV = nRoster - 1;
        {
            std::string s = "Character: " + std::string(roster[sel].name);
            if (sel > 0 && !roster[sel].inParty) s += " (not in party)";
            strncpy(out[R_WHO].text, s.c_str(), sizeof out[R_WHO].text - 1);
        }
        out[R_AUTO].value = c.autoOn ? 1 : 0;
        out[R_PCT].value = c.hpPct;
        out[R_PCT].minV = 5; out[R_PCT].maxV = 95;
        out[R_KEEP].value = c.keep;
        out[R_KEEP].minV = 0; out[R_KEEP].maxV = 20;
        if (lock) out[R_AUTO].flags = out[R_PCT].flags = out[R_KEEP].flags = MUI_RF_DISABLED;
        out[R_FOLLOW].value = lock ? 1 : 0;
        if (sel == 0) out[R_FOLLOW].flags = MUI_RF_HIDDEN;
        strcpy(out[R_RESET].text, sel == 0 ? "Reset defaults to factory" : "Reset to global");
        if (sel > 0 && lock) out[R_RESET].flags = MUI_RF_DISABLED;
        strcpy(out[R_APPLY].text, armLive(now) ? "Click again to confirm" : "Apply defaults to everyone");
        if (!t || t->rows.empty()) out[R_APPLY].flags = MUI_RF_DISABLED;
    }

    // One click on `row` with the provider's requested value. Toggles arrive flipped, steppers stepped. Returns what to store.
    Outcome change(int row, int requested, uint32_t now) {
        Outcome o;
        RowView v[R_COUNT];
        view(v, now);
        o.final = (row >= 0 && row < R_COUNT) ? v[row].value : requested;
        if (!t || row < 0 || row >= R_COUNT) return o;
        if (row != R_APPLY && armed) { armed = false; o.refresh = true; }   // any other click disarms the confirm
        if (row == R_WHO) {
            sel = shui::clampi(requested, 0, nRoster - 1);
            o.final = sel;
            o.refresh = true;
            return o;
        }
        if (row == R_AUTO || row == R_PCT || row == R_KEEP) {
            if (following()) return o;                                  // disabled: the provider already ignores it; stay safe
            const shui::CompCfg old = current();
            shui::CompCfg nc = old;
            if (row == R_AUTO) nc.autoOn = requested != 0;
            else if (row == R_PCT) nc.hpPct = snap5(requested);
            else nc.keep = shui::clampi(requested, 0, 20);
            if (sel == 0) t->def = nc; else shui::upsert(*t, key(), nc);
            o.cfgChanged = !(nc == old);
            o.final = row == R_AUTO ? (nc.autoOn ? 1 : 0) : (row == R_PCT ? nc.hpPct : nc.keep);
            return o;
        }
        if (row == R_FOLLOW) {
            if (sel == 0) return o;                                      // hidden for the defaults entry
            const bool wantFollow = requested != 0;
            if (wantFollow == following()) return o;
            if (wantFollow) shui::erase(*t, key());
            else shui::upsert(*t, key(), t->def);                        // a new row starts as a copy of the defaults
            o.cfgChanged = true;
            o.refresh = true;
            o.final = wantFollow ? 1 : 0;
            return o;
        }
        if (row == R_RESET) {
            o.final = 0;
            if (sel == 0) {
                const shui::CompCfg factory;
                o.cfgChanged = !(t->def == factory);
                t->def = factory;
            } else if (!following()) {
                shui::erase(*t, key());
                o.cfgChanged = true;
            }
            o.refresh = true;
            return o;
        }
        // R_APPLY: first click arms, second click inside the window applies
        o.final = 0;
        if (armLive(now)) {
            armed = false;
            o.cfgChanged = shui::eraseAll(*t) > 0;
        } else if (!t->rows.empty()) {
            armed = true;
            armedUntil = now + kConfirmMs;
        }
        o.refresh = true;
        return o;
    }

    // Per frame/tick: true when the confirm window just ran out (re-push the View to restore the caption).
    bool tick(uint32_t now) {
        if (armed && !armLive(now)) { armed = false; return true; }
        return false;
    }
};

}  // namespace shpage
