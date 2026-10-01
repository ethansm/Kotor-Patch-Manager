// Parity harness: compiles LevelUpOverride.cpp natively (stub/windows.h) and prints formatDry() for every case line from
// stdin: "tag key cls ability unspent|feats|spells|d0,..,d7". parity.py feeds the same cases to simulate_rewrite.py and diffs.
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
        std::stringstream ls(line); std::string head, f, s, d;
        std::getline(ls, head, '|'); std::getline(ls, f, '|'); std::getline(ls, s, '|'); std::getline(ls, d, '|');
        char tag[32]; int key, cls, ab, un;
        if (sscanf(head.c_str(), "%31s %d %d %d %d", tag, &key, &cls, &ab, &un) != 5) return 3;
        Pend p{}; p.cls = cls; p.ability = ab; p.unspent = un;
        std::vector<uint32_t> fv, sv, dv; ids(f, fv); ids(s, sv); ids(d, dv);
        for (uint32_t x : fv) p.feats[p.nFeats++] = (uint16_t)x;
        for (uint32_t x : sv) p.spells[p.nSpells++] = x;
        for (int i = 0; i < NumSkills; ++i) p.deltas[i] = (int)dv.at(i);
        Result r = computeRewrite(lookupRecord(tag, key), p);
        Buf b; formatDry(b, tag, key, r); puts(b.s);
    }
}
