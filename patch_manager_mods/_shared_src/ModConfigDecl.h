// ModConfigDecl.h -- declarative Mod Configuration pages (doc 46 MCF3). A mod describes its options page as a small TOML text (a raw string
// literal inside its own DLL), this header parses it (TomlMini.h) and drives k2.menuui option rows (ModConfigApi.h, hasRows()) so the mod
// writes no layout code. Host test: moddecl_test.cpp. No heap, no STL; the optional file API (MCDECL_FILE_API) uses stdio + IniWrite.h.
//
// Schema (one [page], 1..32 [[row]]):
//   [page]
//   name  = "Companion Self-Heal"      # list button text            (default: title)
//   title = "Companion Self-Heal"      # page title                  (default: name)
//   desc  = "..."                      # list-page hover text
//   [[row]]
//   id = "auto"                        # unique, <= 31 chars; the key every API call and the hook use
//   kind = "toggle|stepper|list|dynamic"
//   label = "Auto-heal"                # caption prefix (default: id)
//   desc = "..."                       # description pane text while the row is hovered
//   default = 0                        # initial value (clamped into the range)
//   min = 5  max = 95  step = 5        # stepper / dynamic (list: 0 .. n-1 from choices)
//   wrap = false                       # values wrap at the ends (dynamic: default true)
//   style = "stepper|slider"           # stepper rows only (default stepper): "slider" asks for MUI_ROW_SLIDER; a provider without it gets a stepper (rowKind[])
//   fmt = "Heal below %d%%"            # %d / %% only; a fmt caption stands alone, otherwise "Label: value"
//   zero_text = "Auto"                 # shown instead of 0
//   choices = ["Off","Slow","Fast"]    # list: stored value = index (<= 12 choices, <= 31 chars each)
//   scale = 100                        # shown value = stored / scale with 2 decimals ("Label: 0.75"); stepper / dynamic only
//   apply = "live|restart"             # restart adds " (restart)" to the caption (default live)
//   ini = "enemy_strip.ini"  section = "EnemyStrip"  key = "MaxIcons"    # optional flat binding (section may be omitted)
// kind "dynamic" = a wrap stepper whose range and caption the consumer pushes at run time (Page::setRange / setText). Rows with an
// `ini` binding read their start value from the file at build and write every ACCEPTED change (the hook's final value) back; rows without
// one leave persistence to the hook. Toggles store 0/1, lists the index, scaled steppers the raw integer.
//
// Use:
//   static mcdecl::Page g_page;                                         // ~35 KB, static storage
//   g_page.load(kSchemaToml, strlen(kSchemaToml), err, sizeof err);     // once, e.g. in KPatchInit
//   g_page.menu = &g_menuClient; g_page.hooks = {...}; g_page.io = mcdecl::fileIo();   // fileIo() only with MCDECL_FILE_API
//   K2ModConfigEntry e = g_page.entry("companion-selfheal");            // then k2pc::registerOnce(st, cfgClient, e, log)
// Everything the entry callbacks need is reached through the Page (user = this).
#pragma once
#include "ModConfigApi.h"
#include "TomlMini.h"
#include "IniWrite.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

namespace mcdecl {

enum { kMaxRows = 32, kMaxChoices = 12, kChoiceLen = 32 };
enum Kind { K_TOGGLE = 0, K_STEPPER = 1, K_LIST = 2, K_DYNAMIC = 3 };

struct RowDecl {
    char id[32];
    int kind;
    char label[64];
    char desc[192];
    int32_t def, minV, maxV, step;
    bool wrap, restart;
    bool slider;                        // style = "slider" (stepper rows only): shown as MUI_ROW_SLIDER when the provider has it, else a stepper
    char fmt[48];
    char zero[24];
    char choices[kMaxChoices][kChoiceLen];
    int nChoices;
    int32_t scale;                      // 0 = none
    char ini[64], section[48], key[48];

    void reset() {
        memset(this, 0, sizeof *this);
        kind = -1;
        step = 1;
        maxV = 1;
    }
};

struct PageDecl {
    char name[64], title[64], desc[192];
    int nRows;
    RowDecl rows[kMaxRows];
    bool havePage;

    void reset() {
        name[0] = title[0] = desc[0] = 0;
        nRows = 0;
        havePage = false;
    }
    int find(const char* id) const {
        for (int i = 0; i < nRows; ++i)
            if (strcmp(rows[i].id, id) == 0) return i;
        return -1;
    }
};

// ---------------------------------------------------------------------------------------------------- schema parser
namespace detail {

struct Ctx {
    PageDecl* pd;
    int cur;               // -2 before any header, -1 in [page], >= 0 the row being read
    bool seenPage;
    char msg[96];          // dynamic messages ("unknown row key 'x'")
};

inline const char* setStr(char* dst, size_t cap, const tomlmini::Value& v) {
    if (v.type != tomlmini::V_STR) return "expected a string";
    if (strlen(v.s) >= cap) return "string too long for this field";
    memcpy(dst, v.s, strlen(v.s) + 1);
    return nullptr;
}
inline const char* setInt(int32_t* dst, const tomlmini::Value& v) {
    if (v.type != tomlmini::V_INT) return "expected an integer";
    *dst = v.i;
    return nullptr;
}
inline const char* setBool(bool* dst, const tomlmini::Value& v) {
    if (v.type != tomlmini::V_BOOL) return "expected true or false";
    *dst = v.i != 0;
    return nullptr;
}
inline const char* unknownKey(Ctx* c, const char* what, const char* key) {
    size_t o = 0;
    for (const char* p = what; *p && o + 1 < sizeof c->msg; ++p) c->msg[o++] = *p;
    for (const char* p = " key '"; *p && o + 1 < sizeof c->msg; ++p) c->msg[o++] = *p;
    for (const char* p = key; *p && o + 1 < sizeof c->msg; ++p) c->msg[o++] = *p;
    if (o + 2 < sizeof c->msg) c->msg[o++] = '\'';
    c->msg[o] = 0;
    return c->msg;
}

inline const char* onTable(void* u, const char* name, bool isArray) {
    Ctx* c = static_cast<Ctx*>(u);
    if (strcmp(name, "page") == 0) {
        if (isArray) return "[page] is a plain table, not [[page]]";
        if (c->seenPage) return "duplicate [page]";
        c->seenPage = true;
        c->pd->havePage = true;
        c->cur = -1;
        return nullptr;
    }
    if (strcmp(name, "row") == 0) {
        if (!isArray) return "rows are [[row]]";
        if (!c->seenPage) return "[[row]] before [page]";
        if (c->pd->nRows >= kMaxRows) return "too many rows (max 32)";
        c->pd->rows[c->pd->nRows].reset();
        c->cur = c->pd->nRows++;
        return nullptr;
    }
    return "unknown table (only [page] and [[row]])";
}

inline const char* parseKind(const tomlmini::Value& v, int* out) {
    if (v.type != tomlmini::V_STR) return "expected a string";
    if (strcmp(v.s, "toggle") == 0) *out = K_TOGGLE;
    else if (strcmp(v.s, "stepper") == 0) *out = K_STEPPER;
    else if (strcmp(v.s, "list") == 0) *out = K_LIST;
    else if (strcmp(v.s, "dynamic") == 0) *out = K_DYNAMIC;
    else return "kind must be toggle, stepper, list or dynamic";
    return nullptr;
}

inline const char* onKv(void* u, const char* key, const tomlmini::Value& v) {
    Ctx* c = static_cast<Ctx*>(u);
    PageDecl& pd = *c->pd;
    if (c->cur == -2) return "key outside a table";
    if (c->cur == -1) {
        if (strcmp(key, "name") == 0) return setStr(pd.name, sizeof pd.name, v);
        if (strcmp(key, "title") == 0) return setStr(pd.title, sizeof pd.title, v);
        if (strcmp(key, "desc") == 0) return setStr(pd.desc, sizeof pd.desc, v);
        return unknownKey(c, "unknown page", key);
    }
    RowDecl& r = pd.rows[c->cur];
    if (strcmp(key, "id") == 0) return setStr(r.id, sizeof r.id, v);
    if (strcmp(key, "kind") == 0) return parseKind(v, &r.kind);
    if (strcmp(key, "label") == 0) return setStr(r.label, sizeof r.label, v);
    if (strcmp(key, "desc") == 0) return setStr(r.desc, sizeof r.desc, v);
    if (strcmp(key, "default") == 0) return setInt(&r.def, v);
    if (strcmp(key, "min") == 0) return setInt(&r.minV, v);
    if (strcmp(key, "max") == 0) return setInt(&r.maxV, v);
    if (strcmp(key, "step") == 0) return setInt(&r.step, v);
    if (strcmp(key, "wrap") == 0) return setBool(&r.wrap, v);
    if (strcmp(key, "style") == 0) {
        if (v.type != tomlmini::V_STR) return "expected a string";
        if (strcmp(v.s, "slider") == 0) r.slider = true;
        else if (strcmp(v.s, "stepper") == 0) r.slider = false;
        else return "style must be stepper or slider";
        return nullptr;
    }
    if (strcmp(key, "fmt") == 0) return setStr(r.fmt, sizeof r.fmt, v);
    if (strcmp(key, "zero_text") == 0) return setStr(r.zero, sizeof r.zero, v);
    if (strcmp(key, "scale") == 0) return setInt(&r.scale, v);
    if (strcmp(key, "ini") == 0) return setStr(r.ini, sizeof r.ini, v);
    if (strcmp(key, "section") == 0) return setStr(r.section, sizeof r.section, v);
    if (strcmp(key, "key") == 0) return setStr(r.key, sizeof r.key, v);
    if (strcmp(key, "apply") == 0) {
        if (v.type != tomlmini::V_STR) return "expected a string";
        if (strcmp(v.s, "live") == 0) r.restart = false;
        else if (strcmp(v.s, "restart") == 0) r.restart = true;
        else return "apply must be live or restart";
        return nullptr;
    }
    if (strcmp(key, "choices") == 0) {
        if (v.type != tomlmini::V_ARR) return "expected an array of strings";
        if (v.n > kMaxChoices) return "too many choices (max 12)";
        for (int i = 0; i < v.n; ++i) {
            if (strlen(v.arr[i]) >= (size_t)kChoiceLen) return "choice too long (max 31)";
            memcpy(r.choices[i], v.arr[i], strlen(v.arr[i]) + 1);
        }
        r.nChoices = v.n;
        return nullptr;
    }
    return unknownKey(c, "unknown row", key);
}

inline void catMsg(char* dst, size_t cap, const char* s) {
    size_t o = strlen(dst);
    while (*s && o + 1 < cap) dst[o++] = *s++;
    dst[o] = 0;
}

inline void rowErr(char* err, int cap, int row, const char* id, const char* msg) {
    if (!err || cap <= 0) return;
    err[0] = 0;
    catMsg(err, (size_t)cap, "row ");
    char t[12];
    int n = 0;
    unsigned u = (unsigned)row + 1u;
    do { t[n++] = (char)('0' + u % 10u); u /= 10u; } while (u);
    while (n) { char one[2] = {t[--n], 0}; catMsg(err, (size_t)cap, one); }
    if (id && id[0]) { catMsg(err, (size_t)cap, " ("); catMsg(err, (size_t)cap, id); catMsg(err, (size_t)cap, ")"); }
    catMsg(err, (size_t)cap, ": ");
    catMsg(err, (size_t)cap, msg);
}

inline void itoa10(char* out, int32_t v) {   // out holds >= 12 bytes
    char t[12];
    int n = 0;
    unsigned u = v < 0 ? 0u - (unsigned)v : (unsigned)v;
    do { t[n++] = (char)('0' + u % 10u); u /= 10u; } while (u);
    int o = 0;
    if (v < 0) out[o++] = '-';
    while (n) out[o++] = t[--n];
    out[o] = 0;
}

inline int32_t clampI(int32_t v, int32_t lo, int32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }

}  // namespace detail

// Parses and validates a schema. false + a short message in err on any problem (page left unusable: nRows is reset to 0).
inline bool parseSchema(const char* text, size_t n, PageDecl& pd, char* err, int errCap) {
    pd.reset();
    detail::Ctx ctx;
    ctx.pd = &pd;
    ctx.cur = -2;
    ctx.seenPage = false;
    ctx.msg[0] = 0;
    tomlmini::Callbacks cb;
    cb.user = &ctx;
    cb.table = detail::onTable;
    cb.kv = detail::onKv;
    if (!tomlmini::parse(text, n, cb, err, errCap)) { pd.nRows = 0; return false; }
    auto fail = [&](int row, const char* id, const char* msg) {
        if (row < 0) {
            if (err && errCap > 0) { err[0] = 0; detail::catMsg(err, (size_t)errCap, msg); }
        } else {
            detail::rowErr(err, errCap, row, id, msg);
        }
        pd.nRows = 0;
        return false;
    };
    if (!pd.havePage) return fail(-1, nullptr, "missing [page]");
    if (pd.nRows < 1) return fail(-1, nullptr, "no [[row]] entries");
    if (!pd.title[0]) strcpy(pd.title, pd.name);
    if (!pd.name[0]) strcpy(pd.name, pd.title);
    if (!pd.name[0]) return fail(-1, nullptr, "[page] needs a name or title");
    for (int i = 0; i < pd.nRows; ++i) {
        RowDecl& r = pd.rows[i];
        if (!r.id[0]) return fail(i, nullptr, "missing id");
        for (int j = 0; j < i; ++j)
            if (strcmp(pd.rows[j].id, r.id) == 0) return fail(i, r.id, "duplicate id");
        if (r.kind < 0) return fail(i, r.id, "missing kind");
        if (!r.label[0]) { strcpy(r.label, r.id); }
        if (r.scale < 0) return fail(i, r.id, "scale must be positive");
        if (r.scale > 0 && (r.kind == K_TOGGLE || r.kind == K_LIST)) return fail(i, r.id, "scale applies to stepper and dynamic rows only");
        if (r.ini[0] && !r.key[0]) return fail(i, r.id, "ini needs a key");
        if ((r.section[0] || r.key[0]) && !r.ini[0]) return fail(i, r.id, "section / key need an ini file");
        if (r.ini[0] && r.kind == K_DYNAMIC) return fail(i, r.id, "dynamic rows have no ini binding");
        switch (r.kind) {
            case K_TOGGLE:
                r.minV = 0; r.maxV = 1; r.step = 1; r.wrap = false;
                r.def = r.def != 0 ? 1 : 0;
                break;
            case K_LIST:
                if (r.nChoices < 2) return fail(i, r.id, "a list needs at least 2 choices");
                r.minV = 0; r.maxV = r.nChoices - 1; r.step = 1;
                r.def = detail::clampI(r.def, 0, r.maxV);
                break;
            case K_STEPPER:
                if (r.minV > r.maxV) return fail(i, r.id, "min is above max");
                if (r.step < 1) return fail(i, r.id, "step must be at least 1");
                r.def = detail::clampI(r.def, r.minV, r.maxV);
                break;
            case K_DYNAMIC:
                if (r.minV > r.maxV) return fail(i, r.id, "min is above max");
                if (r.step < 1) return fail(i, r.id, "step must be at least 1");
                r.wrap = true;
                r.def = detail::clampI(r.def, r.minV, r.maxV);
                break;
        }
        if (r.slider && r.kind != K_STEPPER) return fail(i, r.id, "style = \"slider\" applies to stepper rows only");
        if (r.kind != K_LIST && r.nChoices > 0) return fail(i, r.id, "choices apply to list rows only");
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------- INI read (pure) and IO hooks
// Value of key in section of an INI text (flat file: section null / ""). Same matching rules as iniw: case-insensitive names, last
// duplicate wins, a commented default (";key=1") is absent, ' ;' ends the value. true when found; out is trimmed and NUL-terminated.
inline bool iniGet(const char* in, size_t n, const char* section, const char* key, char* out, int cap) {
    if (!out || cap <= 0) return false;
    out[0] = 0;
    if (!in || !key || !*key) return false;
    const bool flat = !section || !*section;
    bool inTarget = flat, found = false;
    size_t pos = 0;
    iniw::detail::Line ln;
    while (iniw::detail::nextLine(in, n, pos, ln)) {
        const char* name = nullptr;
        size_t nlen = 0;
        if (iniw::detail::headerOf(ln.p, ln.body, name, nlen)) {
            inTarget = !flat && iniw::detail::nameIs(name, nlen, section);
            continue;
        }
        if (!inTarget) continue;
        const char* k = nullptr;
        size_t klen = 0, eq = 0;
        if (!iniw::detail::keyOf(ln.p, ln.body, k, klen, eq) || !iniw::detail::nameIs(k, klen, key)) continue;
        size_t s = eq + 1, e = ln.body;
        for (size_t i = s; i < e; ++i)
            if (ln.p[i] == ';' && i > s && iniw::detail::isBlank(ln.p[i - 1])) { e = i; break; }
        while (s < e && iniw::detail::isBlank(ln.p[s])) ++s;
        while (e > s && iniw::detail::isBlank(ln.p[e - 1])) --e;
        size_t len = e - s;
        if (len >= (size_t)cap) len = (size_t)cap - 1;
        memcpy(out, ln.p + s, len);
        out[len] = 0;
        found = true;   // keep scanning: the last match wins
    }
    return found;
}

// Injected file access (tests use a fake; fileIo() below is the real one).
struct IniIo {
    bool (*read)(const char* path, const char* section, const char* key, char* out, int cap) = nullptr;   // false = file or key absent
    bool (*write)(const char* path, const char* section, const char* key, const char* val) = nullptr;     // false = write failed
};

#ifdef MCDECL_FILE_API
#include <stdio.h>
// Needs INIW_FILE_API defined before the first include of IniWrite.h (ModConfigDecl.h includes it).
inline bool fileRead(const char* path, const char* section, const char* key, char* out, int cap) {
    static char buf[16384];
    FILE* f = path ? fopen(path, "rb") : nullptr;
    if (!f) return false;
    const size_t n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    return iniGet(buf, n, section, key, out, cap);
}
inline bool fileWrite(const char* path, const char* section, const char* key, const char* val) { return iniw::updateFile(path, section, key, val); }
inline IniIo fileIo() {
    IniIo io;
    io.read = fileRead;
    io.write = fileWrite;
    return io;
}
#endif

// ---------------------------------------------------------------------------------------------------- Page
// "Label: 0.75" for a scaled row (value / scale, 2 decimals, truncated toward zero); zero_text at 0.
inline void formatScaled(char* out, int cap, const RowDecl& r, int32_t v) {
    if (cap <= 0) return;
    out[0] = 0;
    size_t o = 0;
    auto put = [&](const char* s) { while (*s && o + 1 < (size_t)cap) out[o++] = *s++; out[o] = 0; };
    put(r.label);
    put(": ");
    if (v == 0 && r.zero[0]) {
        put(r.zero);
        if (r.restart) put(" (restart)");   // same rule as the provider's rowFormat: RF_RESTART always appends
        return;
    }
    const bool neg = v < 0;
    unsigned long long a = neg ? (unsigned long long)(-(long long)v) : (unsigned long long)v;
    const unsigned long long sc = r.scale > 0 ? (unsigned long long)r.scale : 1ull;
    const unsigned long long whole = a / sc, frac = (a % sc) * 100ull / sc;
    char t[24];
    int n = 0;
    unsigned long long w = whole;
    do { t[n++] = (char)('0' + w % 10ull); w /= 10ull; } while (w && n < 20);
    if (neg && (whole || frac)) put("-");
    while (n) { char one[2] = {t[--n], 0}; put(one); }
    char d[4] = {'.', (char)('0' + frac / 10ull), (char)('0' + frac % 10ull), 0};
    put(d);
    if (r.restart) put(" (restart)");
}

struct DeclHooks {
    void* user = nullptr;
    // A change was requested on row `row` (decl index) with id; return the FINAL value (requested = accept, the old value = reject).
    int32_t (*onChange)(void* user, int row, const char* id, int32_t requested, int dir) = nullptr;
    void (*onEnter)(void* user) = nullptr;
    void (*onLeave)(void* user) = nullptr;
    void (*onTick)(void* user) = nullptr;
    // End of build(): the rows exist and the change handler is set; `page` is the provider page id, `panel` the Options panel. Add custom items here
    // (a preview via AddCustom, the shared gear via AddGear, a NewModal) so they come after the rows. Called only when the page was built.
    void (*onBuilt)(void* user, uint32_t page, void* panel) = nullptr;
};

struct Page {
    PageDecl decl;
    const char* choicePtr[kMaxRows][kMaxChoices] = {};
    int32_t values[kMaxRows] = {};      // mirror of the provider's stored values (decl index)
    int rowId[kMaxRows];                // provider row id per decl row, -1 = AddRow failed
    int32_t rowKind[kMaxRows];          // MUI_ROW_* the provider accepted per decl row (a style = "slider" row on an older provider reads MUI_ROW_STEPPER)
    int32_t lo[kMaxRows] = {}, hi[kMaxRows] = {};   // the provider's current range per row (decl range at build, then setRange)
    k2mc::MenuClient* menu = nullptr;
    DeclHooks hooks;
    IniIo io;
    uint32_t page = 0;
    bool built = false;
    bool loaded = false;

    bool load(const char* text, size_t n, char* err, int errCap) {
        loaded = parseSchema(text, n, decl, err, errCap);
        return loaded;
    }

    // ---- row access by id
    int index(const char* id) const { return loaded && id ? decl.find(id) : -1; }
    bool live() const { return built && menu && menu->ok() && menu->hasRows(); }
    int32_t get(const char* id) const { const int i = index(id); return i >= 0 ? values[i] : 0; }

    void refreshCaption(int i) {
        if (!live() || i < 0 || i >= decl.nRows || rowId[i] < 0) return;
        const RowDecl& r = decl.rows[i];
        if (r.scale <= 0) return;
        char b[96];
        formatScaled(b, sizeof b, r, values[i]);
        menu->p->SetRowText(page, rowId[i], b);
    }
    // Silent set (no hook call, no ini write): pushes the value to the provider and keeps the mirror.
    void setValue(const char* id, int32_t v) {
        const int i = index(id);
        if (i < 0) return;
        values[i] = v;
        if (!live() || rowId[i] < 0) return;
        menu->p->SetValue(page, rowId[i], v);
        values[i] = menu->p->GetValue(page, rowId[i]);   // the provider clamped it
        refreshCaption(i);
    }
    void setText(const char* id, const char* text) {
        const int i = index(id);
        if (i >= 0 && live() && rowId[i] >= 0) menu->p->SetRowText(page, rowId[i], text);
    }
    void setRange(const char* id, int32_t lo_, int32_t hi_) {
        const int i = index(id);
        if (i < 0 || !live() || rowId[i] < 0) return;
        menu->p->SetRowRange(page, rowId[i], lo_, hi_);
        const RowDecl& r = decl.rows[i];
        if (r.kind == K_LIST) {   // the provider keeps a choice row inside [0, nChoices - 1]
            lo_ = detail::clampI(lo_, 0, r.maxV);
            hi_ = detail::clampI(hi_, 0, r.maxV);
        }
        lo[i] = lo_;
        hi[i] = hi_;
        values[i] = menu->p->GetValue(page, rowId[i]);
        refreshCaption(i);
    }
    void setState(const char* id, int32_t flags) {
        const int i = index(id);
        if (i >= 0 && live() && rowId[i] >= 0) menu->p->SetRowState(page, rowId[i], flags);
    }

    // ---- one RowDecl -> the provider's row spec
    void fillSpec(int i, int32_t value, K2MuiRowSpec* sp) {
        const RowDecl& r = decl.rows[i];
        memset(sp, 0, sizeof *sp);
        sp->struct_size = sizeof *sp;
        sp->kind = r.kind == K_TOGGLE ? MUI_ROW_TOGGLE : (r.kind == K_STEPPER && r.slider ? MUI_ROW_SLIDER : MUI_ROW_STEPPER);
        sp->label = r.label;
        sp->desc = r.desc;
        sp->min = r.minV;
        sp->max = r.maxV;
        sp->step = r.step;
        sp->value = value;
        sp->flags = (r.wrap ? MUI_RF_WRAP : 0) | (r.restart ? MUI_RF_RESTART : 0);
        sp->fmt = (r.fmt[0] && r.scale <= 0) ? r.fmt : nullptr;
        sp->zeroText = (r.zero[0] && r.scale <= 0) ? r.zero : nullptr;
        if (r.kind == K_LIST) {
            for (int c = 0; c < r.nChoices; ++c) choicePtr[i][c] = r.choices[c];
            sp->choices = choicePtr[i];
            sp->nChoices = r.nChoices;
        }
    }

    int32_t startValue(int i) {
        const RowDecl& r = decl.rows[i];
        int32_t v = r.def;
        if (r.ini[0]) {
            char b[48];
            if (io.read && io.read(r.ini, r.section[0] ? r.section : nullptr, r.key, b, sizeof b)) {
                char* end = nullptr;
                const long x = strtol(b, &end, 10);
                if (end != b) v = detail::clampI((int32_t)x, r.minV, r.maxV);
                else if (r.kind == K_TOGGLE) {
                    if (!strcmp(b, "true") || !strcmp(b, "on") || !strcmp(b, "yes")) v = 1;
                    else if (!strcmp(b, "false") || !strcmp(b, "off") || !strcmp(b, "no")) v = 0;
                }
            }
        }
        return v;
    }

    // ---- K2ModConfigEntry callbacks (user = this)
    static void KPATCH_CALL cbBuild(void* panel, void* user) { static_cast<Page*>(user)->build(panel); }
    static void KPATCH_CALL cbEnter(uint32_t, void* user) { Page* p = static_cast<Page*>(user); if (p->built && p->hooks.onEnter) p->hooks.onEnter(p->hooks.user); }
    static void KPATCH_CALL cbLeave(uint32_t, void* user) { Page* p = static_cast<Page*>(user); if (p->built && p->hooks.onLeave) p->hooks.onLeave(p->hooks.user); }
    static void KPATCH_CALL cbTick(uint32_t, void* user) { Page* p = static_cast<Page*>(user); if (p->built && p->hooks.onTick) p->hooks.onTick(p->hooks.user); }
    static void KPATCH_CALL cbDestroy(void*, void* user) { static_cast<Page*>(user)->built = false; }
    static int32_t KPATCH_CALL cbChange(uint32_t, int32_t row, int32_t requested, int32_t dir, void* user) {
        return static_cast<Page*>(user)->change(row, requested, dir);
    }

    // The Options ctor window: NewPage + one AddRow per decl row + the change handler. A provider without rows leaves the page unbuilt.
    void build(void* panel) {
        built = false;
        for (int i = 0; i < kMaxRows; ++i) { rowId[i] = -1; rowKind[i] = -1; }
        if (!loaded || !menu || !menu->ok() || !menu->hasRows() || !panel) return;
        const K2MenuUiApi* ui = menu->p;
        page = ui->NewPage(panel, decl.title);
        if (!page) return;
        bool any = false;
        for (int i = 0; i < decl.nRows; ++i) {
            K2MuiRowSpec sp;
            values[i] = startValue(i);
            lo[i] = decl.rows[i].minV;
            hi[i] = decl.rows[i].maxV;
            fillSpec(i, values[i], &sp);
            rowId[i] = ui->AddRow(page, &sp);
            if (rowId[i] < 0 && sp.kind == MUI_ROW_SLIDER) {   // no new API slot: a provider that does not know kind 2 refuses it, so ask once more for a stepper
                sp.kind = MUI_ROW_STEPPER;
                rowId[i] = ui->AddRow(page, &sp);
            }
            if (rowId[i] >= 0) { any = true; rowKind[i] = sp.kind; }
        }
        if (!any) return;
        ui->SetChangeHandler(page, &Page::cbChange, this);
        built = true;
        for (int i = 0; i < decl.nRows; ++i) refreshCaption(i);
        if (hooks.onBuilt) hooks.onBuilt(hooks.user, page, panel);
    }

    // The provider's change handler: the hook decides, an ini-bound row writes the accepted value, a scaled row refreshes its caption.
    int32_t change(int32_t providerRow, int32_t requested, int dir) {
        int i = -1;
        for (int k = 0; k < decl.nRows; ++k)
            if (rowId[k] == providerRow) { i = k; break; }
        if (i < 0) return requested;
        const RowDecl& r = decl.rows[i];
        const int32_t old = values[i];   // before the hook: a hook that re-pushes its rows (setValue) must not hide the change from the ini write
        int32_t fin = requested;
        if (hooks.onChange) fin = hooks.onChange(hooks.user, i, r.id, requested, dir);
        // Same clamp as the provider stores (mui::rowClamp): toggles 0/1, else the row's CURRENT range (a dynamic row's setRange included).
        if (r.kind == K_TOGGLE) fin = fin != 0 ? 1 : 0;
        else fin = lo[i] <= hi[i] ? detail::clampI(fin, lo[i], hi[i]) : detail::clampI(fin, hi[i], lo[i]);
        if (r.ini[0] && fin != old && io.write) {
            char b[16];
            detail::itoa10(b, fin);
            io.write(r.ini, r.section[0] ? r.section : nullptr, r.key, b);
        }
        values[i] = fin;
        if (r.scale > 0 && live()) {
            char b[96];
            formatScaled(b, sizeof b, r, fin);
            menu->p->SetRowText(page, providerRow, b);
        }
        return fin;
    }

    // The entry for k2pc::registerOnce / ConfigClient.p->Register. id must be a static string; name / desc point into this Page.
    K2ModConfigEntry entry(const char* id) {
        K2ModConfigEntry e;
        memset(&e, 0, sizeof e);
        e.struct_size = sizeof e;
        e.id = id;
        e.name = decl.name;
        e.desc = decl.desc;
        e.user = this;
        e.build = &Page::cbBuild;
        e.enter = &Page::cbEnter;
        e.leave = &Page::cbLeave;
        e.tick = &Page::cbTick;
        e.destroy = &Page::cbDestroy;
        return e;
    }
};

}  // namespace mcdecl
