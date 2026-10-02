// Runs the same DFS trace as research/companion_levelup/wizard_twin.py through wiz:: (WizardLogic.h).
// Usage: wizard_test_bin <levelup_paths.ini>
#include "../../../../patch_manager_mods/level_up_override_src/WizardLogic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace {
constexpr int MaxTags = 32;
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

void addSynthetic() {
    Tag* t = findTag("_case");
    const char* a[5] = {"Melee", "single", "none", "n/a", "combat"};
    const char* b[5] = {"melee", "dual", "none", "n/a", "combat"};
    const char* c[5] = {"MELEE", "Dual", "none", "n/a", "Combat"};
    addItem(t, "_case__a", a, "A");
    addItem(t, "_case__b", b, "B");
    addItem(t, "_case__c", c, "C");
}

const char* const kKeys[] = {"role", "style", "alignment", "convert", "skills", "pick"};

struct Ctx { const Tag* tag; wiz::State st; bool reached[wiz::MaxItems]; };

void pathStr(const Ctx& c, char* out, size_t cap) {
    size_t o = 0;
    out[0] = 0;
    for (int k = 0; k < c.st.depth; ++k) {
        const wiz::Answer& a = c.st.stack[k];
        const char* val = a.page == wiz::Pick ? c.st.items[a.ref].id : c.st.items[a.ref].ans[a.page];
        int w = snprintf(out + o, cap - o, "%s%s=%s", k ? " " : "", kKeys[a.page], val);
        if (w > 0) o += static_cast<size_t>(w);
    }
}

void printView(const Ctx& c, const wiz::View& v) {
    char p[512];
    pathStr(c, p, sizeof p);
    if (v.result) { printf("%s | %s | RESULT %s\n", c.tag->name, p, c.st.items[v.resultItem].id); return; }
    if (v.page < 0) { printf("%s | %s | EMPTY\n", c.tag->name, p); return; }
    printf("%s | %s | page=%s opts=", c.tag->name, p, wiz::pageName(v.page));
    for (int i = 0; i < v.nOptions; ++i) printf("%s%s", i ? "," : "", v.options[i]);
    printf("\n");
}

bool sameView(const wiz::View& a, const wiz::View& b) {
    if (a.result != b.result || a.page != b.page || a.nOptions != b.nOptions) return false;
    for (int i = 0; i < a.nOptions; ++i) if (strcmp(a.options[i], b.options[i])) return false;
    return true;
}

void dfs(Ctx& c) {
    wiz::View v = wiz::view(c.st);
    printView(c, v);
    if (v.result) { c.reached[v.resultItem] = true; return; }
    if (v.page < 0) return;
    for (int o = 0; o < v.nOptions; ++o) {
        wiz::choose(c.st, o);
        dfs(c);
        wiz::back(c.st);
        wiz::View v2 = wiz::view(c.st);
        char p[512];
        pathStr(c, p, sizeof p);
        printf("%s | %s | back %s\n", c.tag->name, p, sameView(v, v2) ? "ok" : "MISMATCH");
    }
}
}  // namespace

int main(int argc, char** argv) {
    if (argc < 2 || !loadIni(argv[1])) { fprintf(stderr, "usage: wizard_test_bin <levelup_paths.ini>\n"); return 2; }
    addSynthetic();
    for (int ti = 0; ti < g_nTags; ++ti) {
        Tag& t = g_tags[ti];
        for (int i = 0; i < t.n; ++i) {
            t.items[i].id = t.raw[i].id;
            for (int k = 0; k < 5; ++k) t.items[i].ans[k] = t.raw[i].ans[k];
            t.items[i].shortName = t.raw[i].shortName;
        }
        Ctx c;
        c.tag = &t;
        wiz::reset(c.st, t.items, t.n);
        for (bool& r : c.reached) r = false;
        printf("%s | back-at-root=%s\n", t.name, wiz::back(c.st) ? "true" : "false");
        dfs(c);
        for (int i = 0; i < t.n; ++i) if (!c.reached[i]) printf("# FAIL unreachable preset %s\n", t.items[i].id);
    }
    return 0;
}
