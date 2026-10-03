// Runs the same cycle/BFS trace as research/companion_levelup/options_twin.py through opt:: (OptionsLogic.h).
// Usage: options_test_bin <levelup_paths.ini>
#include "../../../../patch_manager_mods/level_up_override_src/OptionsLogic.h"
#include <stdio.h>
#include <string.h>

namespace {
constexpr int MaxTags = 32, NR = opt::NumRows;
struct Raw { char id[64]; char ans[wiz::NumQuestions][24]; char shortName[96]; };
struct Tag {
    char name[32];
    int n;
    Raw raw[wiz::MaxItems];
    wiz::Item items[wiz::MaxItems];
};
Tag g_tags[MaxTags];
int g_nTags = 0;
char g_fileBuf[512 * 1024];

Tag* findTag(const char* name) {
    for (int i = 0; i < g_nTags; ++i) if (!strcmp(g_tags[i].name, name)) return &g_tags[i];
    if (g_nTags >= MaxTags) return nullptr;
    Tag* t = &g_tags[g_nTags++];
    snprintf(t->name, sizeof t->name, "%s", name);
    t->n = 0;
    return t;
}

void addItem(Tag* t, const char* id, const char* const ans[5], const char* sh) {
    if (t->n >= wiz::MaxItems) return;
    Raw& r = t->raw[t->n++];
    snprintf(r.id, sizeof r.id, "%s", id);
    for (int i = 0; i < 5; ++i) snprintf(r.ans[i], sizeof r.ans[i], "%s", ans[i]);
    snprintf(r.shortName, sizeof r.shortName, "%s", sh);
}

bool loadIni(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    size_t n = fread(g_fileBuf, 1, sizeof g_fileBuf - 1, f);
    fclose(f);
    g_fileBuf[n] = 0;
    bool inSec = false;
    for (char* line = strtok(g_fileBuf, "\n"); line; line = strtok(nullptr, "\n")) {
        while (*line == ' ' || *line == '\t') ++line;
        size_t len = strlen(line);
        while (len && (line[len - 1] == '\r' || line[len - 1] == ' ')) line[--len] = 0;
        if (!*line) continue;
        if (*line == '[') { inSec = !strcmp(line, "[presets]"); continue; }
        if (!inSec || *line == '#' || *line == ';') continue;
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char* val = eq + 1;
        char* f8[8];
        int nf = 0;
        f8[nf++] = val;
        for (char* p = val; *p && nf < 8; ++p) if (*p == '|') { *p = 0; f8[nf++] = p + 1; }
        if (nf < 8) continue;
        const char* ans[5] = {f8[0], f8[1], f8[2], f8[3], f8[4]};
        if (!wiz::selectable(ans)) continue;
        char* us = strstr(line, "__");
        if (!us) continue;
        char tagName[32];
        snprintf(tagName, sizeof tagName, "%.*s", static_cast<int>(us - line), line);
        Tag* t = findTag(tagName);
        if (t) addItem(t, line, ans, f8[7]);
    }
    return true;
}

void joinOpts(const opt::State& s, int row, char* out, size_t cap) {
    const char* v[opt::MaxValues];
    int nv = opt::options(s, row, v, opt::MaxValues);
    size_t o = 0;
    out[0] = 0;
    for (int j = 0; j < nv; ++j) {
        int w = snprintf(out + o, cap - o, "%s%s", j ? "," : "", v[j]);
        if (w > 0) o += static_cast<size_t>(w);
    }
}
}  // namespace

int main(int argc, char** argv) {
    if (argc < 2 || !loadIni(argv[1])) { fprintf(stderr, "usage: options_test_bin <levelup_paths.ini>\n"); return 2; }
    int rc = 0;
    for (int ti = 0; ti < g_nTags; ++ti) {
        Tag& t = g_tags[ti];
        for (int i = 0; i < t.n; ++i) {
            t.items[i].id = t.raw[i].id;
            for (int k = 0; k < 5; ++k) t.items[i].ans[k] = t.raw[i].ans[k];
            t.items[i].shortName = t.raw[i].shortName;
        }
        opt::State probe;
        opt::init(probe, t.items, t.n, 0);
        int rows[opt::NumRows + 1], nrows = 0;
        printf("%s | shown=", t.name);
        for (int r = 0; r < NR; ++r) if (opt::detail::rowShown(probe, r)) { rows[nrows++] = r; printf("%s%s", nrows > 1 ? "," : "", opt::rowName(r)); }
        printf("\n");
        for (int s0 = 0; s0 < t.n; ++s0) {
            opt::State st;
            opt::init(st, t.items, t.n, s0);
            int nr = nrows;
            int allrows[opt::NumRows + 1];
            for (int k = 0; k < nrows; ++k) allrows[k] = rows[k];
            if (opt::shown(st, opt::VariantRow)) allrows[nr++] = opt::VariantRow;
            for (int k = 0; k < nr; ++k) {
                for (int d = 1; d >= -1; d -= 2) {
                    opt::State c = st;
                    char vals[512];
                    joinOpts(st, allrows[k], vals, sizeof vals);
                    bool ch = opt::cycle(c, allrows[k], d);
                    printf("%s | start=%s | row=%s dir=%+d -> %s (vals %s)\n", t.name, t.items[s0].id, opt::rowName(allrows[k]), d,
                           ch ? opt::current(c).id : "NOCHANGE", vals);
                }
            }
            bool seen[wiz::MaxItems] = {};
            int queue[wiz::MaxItems], qh = 0, qt = 0;
            seen[s0] = true; queue[qt++] = s0;
            while (qh < qt) {
                opt::State c;
                opt::init(c, t.items, t.n, queue[qh++]);
                for (int r = 0; r <= opt::VariantRow; ++r)
                    for (int d = 1; d >= -1; d -= 2) {
                        opt::State c2 = c;
                        if (opt::cycle(c2, r, d) && !seen[c2.cur]) { seen[c2.cur] = true; queue[qt++] = c2.cur; }
                    }
            }
            printf("%s | start=%s | reach ", t.name, t.items[s0].id);
            bool first = true;
            for (int i = 0; i < t.n; ++i) if (seen[i]) { printf("%s%s", first ? "" : ",", t.items[i].id); first = false; }
            printf("\n");
            if (qt != t.n) { printf("# FAIL unreachable from %s\n", t.items[s0].id); rc = 1; }
        }
    }
    return rc;
}
