// Parity harness: compiles LevelUpOverride.cpp natively (stub/windows.h) and answers two kinds of stdin lines; parity.py feeds the same
// cases to simulate_rewrite.py and diffs. Table v2 (presets keyed by id).
//  "<preset id> key cls ability unspent csExtraMask|feats|spells|d0,..,d7" -> the DRY line (computeRewrite + formatDry)
//  "L <preset id> key hold n c1:l1,c2:l2,..."                                -> "<DECISION> <char level or ->" (levelNow; hold_class from the preset index)
//  "P <preset id> key hold pause n c1:l1,c2:l2,..."                          -> same, with levelNow's pauseAt (0.3.1)
//  "K <preset id> cl key cls ability unspent csExtraMask kf kp|feats|spells|d0,..,d7"  -> "<DRY line> ## <carry kind> ## <CARRY line or ->"  (computeCarry; record = the preset's
//                                                           record at char level cl; kf/kp = the creature's known feats/powers as k<ids ,> (k = none) or - (unreadable))
//  "R <table path>"                                                         -> "OK rules=<0|1> feat=<n> power=<n> presets=<n>" or "ERR" (parseTable on another table; the main table is re-parsed after)
#include "../../../../patch_manager_mods/level_up_override_src/LevelUpOverride.cpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
static void ids(const std::string& s, std::vector<uint32_t>& out) { std::stringstream ss(s); std::string t; while (std::getline(ss, t, ',')) if (!t.empty()) out.push_back(std::stoul(t)); }
int main(int argc, char** argv) {
    if (argc < 2 || !parseTable(argv[1])) { fprintf(stderr, "table: %s\n", g_tableErr); return 2; }
    g_tableOk = true;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.size() > 2 && line[0] == 'R' && line[1] == ' ') {
            const bool ok = parseTable(line.c_str() + 2);
            if (ok) {
                int nf = 0, np = 0;
                for (int i = 0; i <= MaxFeatId; ++i) nf += g_fr[i].present;
                for (int i = 0; i <= MaxPowerId; ++i) np += g_pr[i].present;
                printf("OK rules=%d feat=%d power=%d presets=%d\n", g_rules ? 1 : 0, nf, np, g_nPresets);
            } else {
                puts("ERR");
            }
            if (!parseTable(argv[1])) return 2;
            g_tableOk = true;
            continue;
        }
        if (line.size() > 2 && line[0] == 'K' && line[1] == ' ') {
            const size_t bar = line.find('|');
            char tag[48], kf[2048], kp[2048]; int cl, key, cls, ab, un, mask;
            if (bar == std::string::npos || sscanf(line.substr(2, bar - 2).c_str(), "%47s %d %d %d %d %d %d %2047s %2047s", tag, &cl, &key, &cls, &ab, &un, &mask, kf, kp) != 9) return 3;
            std::stringstream ls(line.substr(bar + 1)); std::string f, s, d;
            std::getline(ls, f, '|'); std::getline(ls, s, '|'); std::getline(ls, d, '|');
            Pend p{}; p.cls = cls; p.ability = ab; p.unspent = un;
            std::vector<uint32_t> fv, sv, dv; ids(f, fv); ids(s, sv); ids(d, dv);
            for (uint32_t x : fv) p.feats[p.nFeats++] = (uint16_t)x;
            for (uint32_t x : sv) p.spells[p.nSpells++] = x;
            for (int i = 0; i < NumSkills; ++i) p.deltas[i] = (int)dv.at(i);
            KnownSet ks; bool haveF = kf[0] == 'k', haveP = kp[0] == 'k';
            std::vector<uint32_t> kfv, kpv;
            if (haveF) ids(kf + 1, kfv);
            if (haveP) ids(kp + 1, kpv);
            for (uint32_t x : kfv) ks.addF(x);
            for (uint32_t x : kpv) ks.addP(x);
            const Preset* pr = findPreset(tag);
            const Rec* rec = pr && cl >= 0 && cl <= MaxLevel && pr->lv[cl].present ? &pr->lv[cl] : nullptr;
            CarryInfo ci;
            Result r = computeCarry(rec, pr, cl, key, p, haveF ? &ks : nullptr, haveP, (uint8_t)mask, &ci);
            Buf b, c; formatDry(b, tag, key, r);
            if (!strcmp(ci.kind, "carry")) formatCarry(c, tag, key, ci); else c.add("-");
            char nk[24] = "None";
            if (!strcmp(ci.kind, "carry")) snprintf(nk, sizeof nk, "%d/%d", r.nKeptFeats, r.nKeptSpells);
            printf("%s ## %s ## %s ## nk=%s\n", b.s, ci.kind, c.s, nk);
            continue;
        }
        if (line.size() > 2 && (line[0] == 'L' || line[0] == 'P') && line[1] == ' ') {
            char id[48], cl[256]; int key, hold, n, pause = 0;
            if (line[0] == 'P' ? sscanf(line.c_str() + 2, "%47s %d %d %d %d %255s", id, &key, &hold, &pause, &n, cl) != 6
                               : sscanf(line.c_str() + 2, "%47s %d %d %d %255s", id, &key, &hold, &n, cl) != 5) return 3;
            std::vector<uint8_t> ci, cv;
            std::stringstream cs(cl); std::string t;
            while (std::getline(cs, t, ',')) { unsigned a, b; if (sscanf(t.c_str(), "%u:%u", &a, &b) != 2) return 3; ci.push_back((uint8_t)a); cv.push_back((uint8_t)b); }
            const Preset* p = findPreset(id);
            const Rec* rec = nullptr; int cl2 = -1;
            Decision d = levelNow(ci.data(), cv.data(), (int)ci.size(), p, key, hold != 0, pause, &rec, &cl2);
            if (cl2 >= 0) printf("%s %d\n", decisionName(d), cl2); else printf("%s -\n", decisionName(d));
            continue;
        }
        std::stringstream ls(line); std::string head, f, s, d;
        std::getline(ls, head, '|'); std::getline(ls, f, '|'); std::getline(ls, s, '|'); std::getline(ls, d, '|');
        char tag[48]; int key, cls, ab, un, mask;
        if (sscanf(head.c_str(), "%47s %d %d %d %d %d", tag, &key, &cls, &ab, &un, &mask) != 6) return 3;
        Pend p{}; p.cls = cls; p.ability = ab; p.unspent = un;
        std::vector<uint32_t> fv, sv, dv; ids(f, fv); ids(s, sv); ids(d, dv);
        for (uint32_t x : fv) p.feats[p.nFeats++] = (uint16_t)x;
        for (uint32_t x : sv) p.spells[p.nSpells++] = x;
        for (int i = 0; i < NumSkills; ++i) p.deltas[i] = (int)dv.at(i);
        Result r = computeRewrite(lookupRecord(tag, key), p, (uint8_t)mask);
        Buf b; formatDry(b, tag, key, r); puts(b.s);
    }
}
