// Parity harness: compiles LevelUpOverride.cpp natively (stub/windows.h) and answers two kinds of stdin lines; parity.py feeds the same
// cases to simulate_rewrite.py and diffs. Table v2 (presets keyed by id).
//  "<preset id> key cls ability unspent csExtraMask|feats|spells|d0,..,d7" -> the DRY line (computeRewrite + formatDry)
//  "L <preset id> key hold n c1:l1,c2:l2,..."                                -> "<DECISION> <char level or ->" (levelNow; hold_class from the preset index)
//  "P <preset id> key hold pause n c1:l1,c2:l2,..."                          -> same, with levelNow's pauseAt (0.3.1)
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
