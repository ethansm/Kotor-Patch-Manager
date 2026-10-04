// r10_page_test.cpp -- host test of R10Page.h (the PC's R10 filter page model: rows, cycle wrap, row text, token round trip).
#include "R10Page.h"
#include "check.h"
#include <string>

using namespace r10p;

static std::string text(const State& st, int row) { char b[48]; rowText(st, row, b, sizeof b); return b; }
#define CHECK_STR(a, b) KT_CHECK_EQ((a).c_str(), (b))   // std::string vs literal (check.h compares C strings)

static std::string pcTok(const State& st) { char b[TokenLen]; toTokens(st, b, sizeof b, nullptr, nullptr); return b; }

int main() {
    { // defaults
        const State d = defaultState();
        KT_CHECK(!d.on);
        CHECK_STR(text(d, RowOn), "Filter: off"); CHECK_STR(text(d, RowRole), "Role: melee"); CHECK_STR(text(d, RowStyle), "Style: none");
        CHECK_STR(text(d, RowAlign), "Alignment: none"); CHECK_STR(text(d, RowFeats), "Feats: build_dim"); CHECK_STR(text(d, RowPowers), "Powers: build_dim");
        CHECK_STR(text(d, RowCount), ""); CHECK_STR(text(d, -1), "");
        CHECK_STR(pcTok(d), "melee/none/none");
    }
    { // cycle: every row wraps both ways after rowCount steps; the on row toggles
        const int counts[RowCount] = {2, 5, 7, 4, 4, 4};
        for (int row = 0; row < RowCount; ++row) {
            KT_CHECK_EQ(rowCount(row), counts[row]);
            for (int dir = -1; dir <= 1; dir += 2) {
                State st;
                const std::string start = text(st, row);
                for (int i = 1; i < counts[row]; ++i) { cycle(st, row, dir); KT_CHECK(text(st, row) != start); }
                cycle(st, row, dir);
                CHECK_STR(text(st, row), start.c_str());
            }
        }
        State st;
        cycle(st, RowRole, +1); CHECK_STR(text(st, RowRole), "Role: ranged");
        cycle(st, RowRole, -1); cycle(st, RowRole, -1); CHECK_STR(text(st, RowRole), "Role: tank");
        cycle(st, RowStyle, -1); CHECK_STR(text(st, RowStyle), "Style: rifle");
        cycle(st, RowAlign, +1); CHECK_STR(text(st, RowAlign), "Alignment: light");
        cycle(st, RowAlign, -1); cycle(st, RowAlign, -1); CHECK_STR(text(st, RowAlign), "Alignment: neutral");
        cycle(st, RowFeats, +1); CHECK_STR(text(st, RowFeats), "Feats: combat");
        cycle(st, RowPowers, -1); CHECK_STR(text(st, RowPowers), "Powers: all");
        cycle(st, RowOn, +1); KT_CHECK(st.on); cycle(st, RowOn, -1); KT_CHECK(!st.on);
        const State before = st;
        cycle(st, RowCount, +1); cycle(st, -1, +1);   // ignored
        KT_CHECK(st.role == before.role && st.style == before.style && st.align == before.align && st.feats == before.feats && st.powers == before.powers);
    }
    { // every profile combination round-trips through tokens and validates with R10Filter.h
        int n = 0;
        for (int r = 0; r < RoleCount; ++r) for (int s = 0; s < StyleCount; ++s) for (int a = 0; a < AlignCount; ++a) for (int m = 0; m < ModeCount; ++m) {
            State st; st.on = (n & 1) != 0; st.role = r; st.style = s; st.align = a; st.feats = m; st.powers = ModeCount - 1 - m;
            char pc[TokenLen]; const char* ff = nullptr; const char* pf = nullptr;
            toTokens(st, pc, sizeof pc, &ff, &pf);
            r10f::Profile p;
            KT_CHECK(r10f::parseProfile(pc, &p));
            const State back = fromTokens(st.on, pc, ff, pf);
            KT_CHECK(back.on == st.on && back.role == r && back.style == s && back.align == a && back.feats == m && back.powers == ModeCount - 1 - m);
            ++n;
        }
        KT_CHECK_EQ(n, RoleCount * StyleCount * AlignCount * ModeCount);
    }
    { // fromTokens: specific values, empty / invalid input
        State st = fromTokens(true, "caster/dual/dark", "force", "combat");
        KT_CHECK(st.on); CHECK_STR(pcTok(st), "caster/dual/dark");
        CHECK_STR(text(st, RowFeats), "Feats: force"); CHECK_STR(text(st, RowPowers), "Powers: combat");
        st = fromTokens(false, "", "", "");   // stored `all` is an absent field
        KT_CHECK(!st.on); CHECK_STR(pcTok(st), "melee/none/none"); CHECK_STR(text(st, RowFeats), "Feats: all"); CHECK_STR(text(st, RowPowers), "Powers: all");
        st = fromTokens(true, nullptr, nullptr, nullptr);
        KT_CHECK(st.on); CHECK_STR(pcTok(st), "melee/none/none"); CHECK_STR(text(st, RowFeats), "Feats: all");
        st = fromTokens(true, "force/single/light", "bogus", "build_hide");   // bad role -> default profile; unknown -> all; build_hide shown as build_dim
        CHECK_STR(pcTok(st), "melee/none/none"); CHECK_STR(text(st, RowFeats), "Feats: all"); CHECK_STR(text(st, RowPowers), "Powers: build_dim");
        st = fromTokens(false, "tank/rifle/neutral", "build_dim", "all");
        CHECK_STR(pcTok(st), "tank/rifle/neutral"); CHECK_STR(text(st, RowStyle), "Style: rifle"); CHECK_STR(text(st, RowAlign), "Alignment: neutral");
    }
    { // small buffers stay terminated
        char b[6]; rowText(defaultState(), RowRole, b, sizeof b); CHECK_STR(std::string(b), "Role:");
        char pc[4]; toTokens(defaultState(), pc, sizeof pc, nullptr, nullptr); CHECK_STR(std::string(pc), "mel");
    }
    return kt::Report("r10_page_test");
}
