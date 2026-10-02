// CompanionAiLogic.h -- the pure (host-testable, no <windows.h>) logic of companion-ai: INI parsing, the name/tag
// filters, the log gate (per-second cap, dedupe, file cap), throttles and the small decision helpers.
// Host tests: companion_ai_logic_test.cpp. Used by CompanionAI.cpp (one translation unit).
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

namespace cai {

constexpr int Invalid = 0x7F000000;   // OBJECT_INVALID

// ---------------------------------------------------------------- small string helpers
inline char lower(char c) { return (c >= 'A' && c <= 'Z') ? char(c + 32) : c; }
inline bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

// Case-insensitive glob: '*' matches any run (also empty), every other char matches itself.
inline bool globMatch(const char* pat, const char* s) {
    const char* starP = nullptr; const char* starS = nullptr;
    while (*s) {
        if (*pat == '*') { starP = pat++; starS = s; continue; }
        if (*pat && lower(*pat) == lower(*s)) { ++pat; ++s; continue; }
        if (starP) { pat = starP + 1; s = ++starS; continue; }
        return false;
    }
    while (*pat == '*') ++pat;
    return *pat == 0;
}

// Comma separated list of globs (script names or tags), stored lower case.
struct NameList {
    static constexpr int Max = 32, Len = 48;
    char pat[Max][Len] = {};
    int n = 0;
    void set(const char* s) {
        n = 0;
        while (s && *s && n < Max) {
            while (*s == ',' || isSpace(*s)) ++s;
            int k = 0;
            while (*s && *s != ',') { if (k < Len - 1) pat[n][k++] = lower(*s); ++s; }
            while (k > 0 && isSpace(pat[n][k - 1])) --k;
            pat[n][k] = 0;
            if (k) ++n;
        }
    }
    bool empty() const { return n == 0; }
    bool match(const char* name) const {
        if (!name || !*name) return false;
        for (int i = 0; i < n; ++i) if (globMatch(pat[i], name)) return true;
        return false;
    }
    bool operator==(const NameList& o) const {
        if (n != o.n) return false;
        for (int i = 0; i < n; ++i) if (strcmp(pat[i], o.pat[i])) return false;
        return true;
    }
    int format(char* out, int cap) const {   // "a,b,c"
        int len = 0; if (cap > 0) out[0] = 0;
        for (int i = 0; i < n && len < cap - 1; ++i) len += snprintf(out + len, cap - len, "%s%s", i ? "," : "", pat[i]);
        return len < cap ? len : cap - 1;
    }
};

// "11,12,16" -> bit mask (values 0..31)
inline unsigned parseMask(const char* s) {
    unsigned m = 0;
    for (const char* p = s; p && *p;) {
        if (*p >= '0' && *p <= '9') { int v = 0; while (*p >= '0' && *p <= '9') { v = v * 10 + (*p++ - '0'); if (v > 1000) v = 1000; } if (v < 32) m |= 1u << v; }
        else ++p;
    }
    return m;
}
inline bool inMask(unsigned mask, int v) { return v >= 0 && v < 32 && ((mask >> v) & 1u); }

// Decimal or 0x hex, optional sign; anything after the number is ignored. Returns def when no digits.
inline long parseInt(const char* s, long def) {
    if (!s) return def;
    while (isSpace(*s)) ++s;
    bool neg = false;
    if (*s == '-' || *s == '+') { neg = *s == '-'; ++s; }
    long v = 0; bool any = false;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        for (;; ++s) {
            int d = (*s >= '0' && *s <= '9') ? *s - '0' : (lower(*s) >= 'a' && lower(*s) <= 'f') ? lower(*s) - 'a' + 10 : -1;
            if (d < 0) break;
            any = true; if (v < 0x10000000L) v = v * 16 + d;
        }
    } else {
        for (; *s >= '0' && *s <= '9'; ++s) { any = true; if (v < 100000000L) v = v * 10 + (*s - '0'); }
    }
    if (!any) return def;
    return neg ? -v : v;
}

// ---------------------------------------------------------------- INI
// Minimal INI reader for one section. Keys are case-insensitive; values are trimmed; a ';' or '#' that starts a
// line or follows whitespace starts a comment. Later duplicates win (as with the last assignment in a file).
struct IniTable {
    static constexpr int Max = 96, KeyLen = 40, ValLen = 400;
    char key[Max][KeyLen] = {};
    char val[Max][ValLen] = {};
    int n = 0;
    bool sectionFound = false;

    void parse(const char* text, const char* section) {
        n = 0; sectionFound = false; bad[0] = 0;
        bool in = false;
        const char* p = text;
        while (p && *p) {
            const char* eol = p; while (*eol && *eol != '\n') ++eol;
            const char* a = p; const char* b = eol;
            while (a < b && isSpace(*a)) ++a;
            while (b > a && isSpace(b[-1])) --b;
            if (a < b && *a == '[') {
                const char* c = a + 1; const char* e = c; while (e < b && *e != ']') ++e;
                int len = int(e - c); in = (int)strlen(section) == len;
                for (int i = 0; in && i < len; ++i) if (lower(c[i]) != lower(section[i])) in = false;
                if (in) sectionFound = true;
            } else if (in && a < b && *a != ';' && *a != '#') {
                const char* eq = a; while (eq < b && *eq != '=') ++eq;
                if (eq < b) {
                    const char* ke = eq; while (ke > a && isSpace(ke[-1])) --ke;
                    const char* vs = eq + 1; while (vs < b && isSpace(*vs)) ++vs;
                    const char* ve = vs;   // value ends at an inline comment (';'/'#' after whitespace) or end of line
                    for (const char* q = vs; q < b; ++q) {
                        if ((*q == ';' || *q == '#') && (q == vs || isSpace(q[-1]))) break;
                        ve = q + 1;
                    }
                    while (ve > vs && isSpace(ve[-1])) --ve;
                    int klen = int(ke - a);
                    if (klen > 0 && klen < KeyLen) {
                        int slot = find(a, klen);
                        if (slot < 0 && n < Max) slot = n++;
                        if (slot >= 0) {
                            for (int i = 0; i < klen; ++i) key[slot][i] = lower(a[i]);
                            key[slot][klen] = 0;
                            int vlen = int(ve - vs); if (vlen > ValLen - 1) vlen = ValLen - 1;
                            memcpy(val[slot], vs, vlen); val[slot][vlen] = 0;
                        }
                    }
                }
            }
            p = *eol ? eol + 1 : eol;
        }
    }
    int find(const char* k, int klen) const {
        for (int i = 0; i < n; ++i) {
            if ((int)strlen(key[i]) != klen) continue;
            bool same = true;
            for (int j = 0; j < klen && same; ++j) if (key[i][j] != lower(k[j])) same = false;
            if (same) return i;
        }
        return -1;
    }
    const char* get(const char* k) const { int i = find(k, (int)strlen(k)); return i >= 0 ? val[i] : nullptr; }
    // Keys whose value could not be parsed (logged by the DLL after each load; reset by parse()).
    mutable char bad[256] = {};
    void noteBad(const char* k) const {
        size_t l = strlen(bad), kl = strlen(k);
        if (l + kl + 2 < sizeof(bad)) { if (l) bad[l++] = ' '; memcpy(bad + l, k, kl + 1); }
    }
    // 1 = on/true/yes, 0 = off/false/no, -1 = not a word
    static int boolWord(const char* v) {
        static const char* const on[] = {"on", "true", "yes"}, * const off[] = {"off", "false", "no"};
        for (int pass = 0; pass < 2; ++pass)
            for (const char* w : pass ? off : on) {
                size_t i = 0;
                while (w[i] && lower(v[i]) == w[i]) ++i;
                if (!w[i] && !v[i]) return pass ? 0 : 1;
            }
        return -1;
    }
    long getInt(const char* k, long def) const {
        const char* v = get(k);
        if (!v || !*v) return def;
        int w = boolWord(v);
        if (w >= 0) return w;
        long r = parseInt(v, LONG_MIN);
        if (r == LONG_MIN) { noteBad(k); return def; }
        return r;
    }
    // An unparseable flag value means OFF (vanilla), never the built-in default: "a1=of" must not leave a fix on.
    bool getBool(const char* k, bool def) const {
        const char* v = get(k);
        if (!v || !*v) return def;
        int w = boolWord(v);
        if (w >= 0) return w != 0;
        long r = parseInt(v, LONG_MIN);
        if (r == LONG_MIN) { noteBad(k); return false; }
        return r != 0;
    }
    const char* getStr(const char* k, const char* def) const { const char* v = get(k); return v ? v : def; }
};

enum ShipScope { ScopeOff = 0, ScopeAi = 1, ScopeAll = 2 };

inline unsigned maskOf3(int a, int b, int c) { return (1u << a) | (1u << b) | (1u << c); }

constexpr const char* DefaultTraceScripts = "k_ai_master,k_hen_*dlg,a_atkonend,a_force_combat,k_combat_rnd";

// Every key, its default (= the shipped value) and its meaning are in companion_ai.ini.
struct Config {
    // ---- trace layer (design_ai_trace.md)
    bool trace = true, scripts = true, enginePoints = true;
    NameList traceScripts;
    int shipScope = ScopeAi;
    bool printsAllScripts = false, partyOnly = true, logAll = false, enginePrint = true, logAur = true;
    NameList tags;
    int dedupeMs = 1000, maxLinesPerSec = 200, maxFileMb = 64, flushMs = 250;
    int a1BurstThreshold = 4, a1RateThreshold = 400;
    bool engineVerbose = false;
    int summarySec = 10, p1DetailLines = 20;
    // ---- engine fixes (design_ai_engine_fixes.md)
    bool a1 = true; unsigned a1Stances = maskOf3(11, 12, 16); bool a1PartyOnly = true;
    bool a1Retarget = true; unsigned a1RetargetStances = maskOf3(11, 12, 16); int a1RethinkMs = 3000;
    bool p1 = false; int p1IntervalMs = 250;
    bool fm1 = true; int fm1RethinkMs = 0;
    int fm4 = 1;                // 0 vanilla, 1 snapshot/restore (mode 2 of the design is not built: treated as 0)
    bool a4 = true;
    bool c1 = true, c1PartyOnly = true;
    bool a3 = false;
    bool log = true;            // fix event lines (A1/FM1/FM4/A4/A3) and the summaries
    int fm4Requested = 1;       // raw INI value (for the config line)
    // ---- Phase-4 probes (PROBE_DESIGN.md): log only, behaviour stays vanilla; the probe build ships them on
    bool probeFm2c = true, probeFm2f = true, probeFm2r = true, probeFm3 = true, probeRs = true, probeP2 = true, probePf = true,
         probeRar = true, probeShx = true, probeSs1 = true, probeGca = true, probeParty = true, probeA1x = true;
    int probeLines = 400;       // per-probe session budget of PR lines (summaries are exempt)

    Config() { traceScripts.set(DefaultTraceScripts); }
};

inline int clampi(long v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : int(v); }

inline void parseConfig(const IniTable& t, Config& c) {
    c = Config();
    c.trace = t.getBool("trace", c.trace);
    c.scripts = t.getBool("scripts", c.scripts);
    c.enginePoints = t.getBool("engine_points", c.enginePoints);
    if (const char* s = t.get("trace_scripts")) c.traceScripts.set(s);
    if (const char* s = t.get("shipbuild_scope")) {
        if (globMatch("off", s) || globMatch("0", s)) c.shipScope = ScopeOff;
        else if (globMatch("all", s)) c.shipScope = ScopeAll;
        else c.shipScope = ScopeAi;
    }
    c.printsAllScripts = t.getBool("prints_all_scripts", c.printsAllScripts);
    c.partyOnly = t.getBool("party_only", c.partyOnly);
    c.logAll = t.getBool("log_all", c.logAll);
    c.enginePrint = t.getBool("engine_print", c.enginePrint);
    c.logAur = t.getBool("log_aur", c.logAur);
    c.tags.set(t.getStr("tags", ""));
    c.dedupeMs = clampi(t.getInt("dedupe_ms", c.dedupeMs), 0, 600000);
    c.maxLinesPerSec = clampi(t.getInt("max_lines_per_sec", c.maxLinesPerSec), 1, 100000);
    c.maxFileMb = clampi(t.getInt("max_file_mb", c.maxFileMb), 1, 4000);
    c.flushMs = clampi(t.getInt("flush_ms", c.flushMs), 0, 60000);
    c.a1BurstThreshold = clampi(t.getInt("a1_burst_threshold", c.a1BurstThreshold), 2, 100000);
    c.a1RateThreshold = clampi(t.getInt("a1_rate_threshold", c.a1RateThreshold), 2, 1000000);
    c.engineVerbose = t.getBool("engine_verbose", c.engineVerbose);
    c.summarySec = clampi(t.getInt("summary_sec", c.summarySec), 1, 3600);
    c.p1DetailLines = clampi(t.getInt("p1_detail_lines", c.p1DetailLines), 0, 10000);

    c.a1 = t.getBool("a1", c.a1);
    if (const char* s = t.get("a1_stances")) c.a1Stances = parseMask(s);
    c.a1PartyOnly = t.getBool("a1_party_only", c.a1PartyOnly);
    c.a1Retarget = t.getBool("a1_retarget", c.a1Retarget);
    if (const char* s = t.get("a1_retarget_stances")) c.a1RetargetStances = parseMask(s);
    c.a1RethinkMs = clampi(t.getInt("a1_rethink_ms", c.a1RethinkMs), 0, 3000);
    c.p1 = t.getBool("p1", c.p1);
    c.p1IntervalMs = clampi(t.getInt("p1_interval_ms", c.p1IntervalMs), 0, 60000);
    c.fm1 = t.getBool("fm1", c.fm1);
    c.fm1RethinkMs = clampi(t.getInt("fm1_rethink_ms", c.fm1RethinkMs), 0, 3000);
    c.fm4Requested = clampi(t.getInt("fm4", c.fm4), 0, 9);
    if (strstr(t.bad, "fm4")) c.fm4Requested = 0;   // unparseable = vanilla, like the flags
    c.fm4 = c.fm4Requested == 1 ? 1 : 0;
    c.a4 = t.getBool("a4", c.a4);
    c.c1 = t.getBool("c1", c.c1);
    c.c1PartyOnly = t.getBool("c1_party_only", c.c1PartyOnly);
    c.a3 = t.getBool("a3", c.a3);
    c.log = t.getBool("log", c.log);
    c.probeFm2c = t.getBool("probe_fm2c", c.probeFm2c);
    c.probeFm2f = t.getBool("probe_fm2f", c.probeFm2f);
    c.probeFm2r = t.getBool("probe_fm2r", c.probeFm2r);
    c.probeFm3 = t.getBool("probe_fm3", c.probeFm3);
    c.probeRs = t.getBool("probe_rs", c.probeRs);
    c.probeP2 = t.getBool("probe_p2", c.probeP2);
    c.probePf = t.getBool("probe_pf", c.probePf);
    c.probeRar = t.getBool("probe_rar", c.probeRar);
    c.probeShx = t.getBool("probe_shx", c.probeShx);
    c.probeSs1 = t.getBool("probe_ss1", c.probeSs1);
    c.probeGca = t.getBool("probe_gca", c.probeGca);
    c.probeParty = t.getBool("probe_party", c.probeParty);
    c.probeA1x = t.getBool("probe_a1x", c.probeA1x);
    c.probeLines = clampi(t.getInt("probe_lines", c.probeLines), 0, 100000);
}

inline bool sameConfig(const Config& a, const Config& b) {
    return a.trace == b.trace && a.scripts == b.scripts && a.enginePoints == b.enginePoints && a.traceScripts == b.traceScripts &&
           a.shipScope == b.shipScope && a.printsAllScripts == b.printsAllScripts && a.partyOnly == b.partyOnly && a.logAll == b.logAll &&
           a.enginePrint == b.enginePrint && a.logAur == b.logAur && a.tags == b.tags && a.dedupeMs == b.dedupeMs &&
           a.maxLinesPerSec == b.maxLinesPerSec && a.maxFileMb == b.maxFileMb && a.flushMs == b.flushMs &&
           a.a1BurstThreshold == b.a1BurstThreshold && a.a1RateThreshold == b.a1RateThreshold && a.engineVerbose == b.engineVerbose &&
           a.summarySec == b.summarySec && a.p1DetailLines == b.p1DetailLines && a.a1 == b.a1 && a.a1Stances == b.a1Stances &&
           a.a1PartyOnly == b.a1PartyOnly && a.a1Retarget == b.a1Retarget && a.a1RetargetStances == b.a1RetargetStances &&
           a.a1RethinkMs == b.a1RethinkMs && a.p1 == b.p1 && a.p1IntervalMs == b.p1IntervalMs && a.fm1 == b.fm1 &&
           a.fm1RethinkMs == b.fm1RethinkMs && a.fm4 == b.fm4 && a.fm4Requested == b.fm4Requested && a.a4 == b.a4 && a.c1 == b.c1 &&
           a.c1PartyOnly == b.c1PartyOnly && a.a3 == b.a3 && a.log == b.log && a.probeFm2c == b.probeFm2c && a.probeFm2f == b.probeFm2f &&
           a.probeFm2r == b.probeFm2r && a.probeFm3 == b.probeFm3 && a.probeRs == b.probeRs && a.probeP2 == b.probeP2 &&
           a.probePf == b.probePf && a.probeRar == b.probeRar && a.probeShx == b.probeShx && a.probeSs1 == b.probeSs1 &&
           a.probeGca == b.probeGca && a.probeParty == b.probeParty && a.probeA1x == b.probeA1x && a.probeLines == b.probeLines;
}

// ---------------------------------------------------------------- log gate
// Decides, per line, whether it is written, plus which marker lines go out first. Pure: the caller formats and
// writes, then reports the bytes with wrote(). Order: file cap, dedupe (key != 0 only), per-second cap (forced
// lines -- fix events, summaries, markers -- bypass the per-second cap but never the file cap).
struct GateOut {
    bool write = false;
    unsigned repeats = 0;        // > 0: first write "previous line repeated N x"
    unsigned droppedMarker = 0;  // > 0: first write "N lines dropped by the per-second cap"
    bool capMarker = false;      // write the one-time file-cap marker (even though over the cap)
};

struct LogGate {
    // config
    unsigned maxPerSec = 200, dedupeMs = 1000;
    unsigned long long maxBytes = 64ull << 20;
    // state
    unsigned long long bytes = 0;
    bool capHit = false;
    unsigned secStart = 0, inSec = 0, droppedPending = 0;
    uint32_t lastKey = 0; unsigned lastKeyTime = 0, repeats = 0;
    // counters
    unsigned long long totalDropped = 0, totalDeduped = 0, totalLines = 0, totalCapDropped = 0;

    void configure(unsigned perSec, unsigned dedupe, unsigned long long cap) {
        maxPerSec = perSec ? perSec : 1; dedupeMs = dedupe; maxBytes = cap;
        if (capHit && bytes < maxBytes) capHit = false;   // raised via hot reload: resume
    }
    GateOut admit(unsigned now, uint32_t key, bool force) {
        GateOut o;
        if (bytes >= maxBytes) {
            ++totalCapDropped;
            if (!capHit) { capHit = true; o.capMarker = true; }
            return o;
        }
        if (key != 0 && key == lastKey && now - lastKeyTime < dedupeMs) { ++repeats; ++totalDeduped; return o; }
        if (repeats) { o.repeats = repeats; repeats = 0; }
        if (now - secStart >= 1000) { secStart = now; inSec = 0; }
        if (!force && inSec >= maxPerSec) {
            ++droppedPending; ++totalDropped;
            lastKey = 0;   // the next identical line is written once the cap lifts
            return o;
        }
        if (droppedPending) { o.droppedMarker = droppedPending; droppedPending = 0; }
        ++inSec; ++totalLines;
        lastKey = key; lastKeyTime = now;
        o.write = true;
        return o;
    }
    void wrote(unsigned long long n) { bytes += n; }
};

inline uint32_t fnv1a(const void* p, size_t n, uint32_t h = 2166136261u) {
    const unsigned char* b = static_cast<const unsigned char*>(p);
    for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 16777619u; }
    return h;
}
inline uint32_t lineKey(int obj, const char* a, const char* b) {
    uint32_t h = fnv1a(&obj, sizeof obj);
    if (a) h = fnv1a(a, strlen(a), h);
    h = fnv1a("|", 1, h);
    if (b) h = fnv1a(b, strlen(b), h);
    return h ? h : 1;
}

// ---------------------------------------------------------------- throttles
// Per-subject log throttle: the first `first` lines, then only when the signature changes or `periodMs` passed.
struct LineThrottle {
    int logged = 0; unsigned sig = ~0u; unsigned last = 0;
    bool allow(unsigned now, unsigned s, int first, unsigned periodMs) {
        if (logged < first || s != sig || now - last >= periodMs) { ++logged; sig = s; last = now; return true; }
        return false;
    }
};

// Periodic timer.
inline bool due(unsigned now, unsigned& last, unsigned periodMs) {
    if (now - last >= periodMs) { last = now; return true; }
    return false;
}

// E1: attack-entry bursts. Entries closer than burstUs belong to one RunActions loop (that loop is capped at 1 ms).
struct BurstTrack {
    unsigned long long lastUs = 0;
    unsigned burst = 0, maxBurst = 0, perSec = 0;
    void onEntry(unsigned long long nowUs, unsigned burstUs = 2000) {
        burst = (lastUs && nowUs - lastUs < burstUs) ? burst + 1 : 1;
        if (burst > maxBurst) maxBurst = burst;
        lastUs = nowUs; ++perSec;
    }
    bool spinning(unsigned burstThr, unsigned rateThr) const { return maxBurst >= burstThr || perSec >= rateThr; }
    void resetSecond() { maxBurst = 0; perSec = 0; }
};

// E3: perception-1 rate aggregation (calls, distinct creatures, per-creature max, share with elapsed >= 4000).
struct RateAgg {
    static constexpr int Slots = 64;
    int id[Slots] = {}; unsigned cnt[Slots] = {};
    int n = 0; unsigned total = 0, ge4000 = 0, overflow = 0;
    void add(int who, unsigned elapsedMs) {
        ++total; if (elapsedMs >= 4000) ++ge4000;
        for (int i = 0; i < n; ++i) if (id[i] == who) { ++cnt[i]; return; }
        if (n < Slots) { id[n] = who; cnt[n] = 1; ++n; } else ++overflow;
    }
    unsigned maxPer() const { unsigned m = 0; for (int i = 0; i < n; ++i) if (cnt[i] > m) m = cnt[i]; return m; }
    int sharePct() const { return total ? int((ge4000 * 100ull) / total) : 0; }
    void reset() { n = 0; total = ge4000 = overflow = 0; }
};

// ---------------------------------------------------------------- decisions
// Weapon range test shared with ranged-ai's E-H1/E-H3: a zero/garbage range never counts as in range.
inline bool inWeaponRange(float maxRange, float dist2) {
    float lim = maxRange + 0.1f; lim *= lim;
    return maxRange > 1.0f && dist2 <= lim;
}

// A1: why the approach branch was reached (log label only).
inline const char* a1Reason(int ranged, float dist2, float maxRange, int los, bool sameArea) {
    if (!sameArea) return "area";
    if (!ranged) return "melee";
    if (!inWeaponRange(maxRange, dist2)) return "range";
    if (los == 0) return "los";
    return "other";
}

// P1: should block 2 run UpdatePerception(1) now? Times are the engine's (day, ms) pairs. A never-stamped creature
// (0,0) or a clock that went backwards resyncs (runs). `elapsedDays`/`elapsedMs` are 0x51AF60's outputs.
inline bool p1ShouldRun(unsigned nowDay, unsigned nowMs, unsigned stampDay, unsigned stampMs,
                        unsigned elapsedDays, unsigned elapsedMs, unsigned intervalMs) {
    if ((stampDay | stampMs) == 0) return true;
    if (nowDay < stampDay || (nowDay == stampDay && nowMs < stampMs)) return true;
    return elapsedDays != 0 || elapsedMs >= intervalMs;
}

// FM1: keep the bumpee's queue only when it is non-empty and every action is a follow action.
constexpr uint16_t ActFollowLeader = 0x3D, ActCheckMoveToFollowRadius = 0x40, ActFollowOwner = 0x46;
inline bool onlyFollowTypes(const uint16_t* types, int n) {
    if (n <= 0) return false;
    for (int i = 0; i < n; ++i)
        if (types[i] != ActFollowLeader && types[i] != ActCheckMoveToFollowRadius && types[i] != ActFollowOwner) return false;
    return true;
}

// C1: belt and braces against an endless restart loop. Every restart follows a successful group delete, so a real
// call needs at most groups-count restarts; past `cap` restarts for the same (object, frame) within 50 ms the
// handler falls back to the vanilla index.
struct RestartCap {
    int obj = 0, frame = 0; unsigned t = 0; int n = 0;
    bool allow(int o, int f, unsigned now, int cap) {
        if (o != obj || f != frame || now - t > 50) { obj = o; frame = f; n = 0; }
        t = now;
        return ++n <= cap;
    }
};

// ---------------------------------------------------------------- handler decisions (review SF3: the DLL calls these;
// every behaviour-changing choice of A1/A4/C1/P1/FM1/FM4 and of the INI reload lives here and is host-tested)

// A1 (0x6D853E). Effective stance 11/12 (after ranged-ai E-H3), not backing off, no force jump => a would-spin pass.
inline bool a1SpinCandidate(int eff, int backoff, int forceJump) { return (eff == 11 || eff == 12) && backoff == 0 && forceJump == 0; }
// The fix acts on the RAW stance (stats+0x146) mask, party members only by default.
inline bool a1Applies(const Config& c, int rawStance, int party) {
    return c.a1 && inMask(c.a1Stances, rawStance) && (!c.a1PartyOnly || party != 0);
}
// Retarget first (range = the attack's max weapon range [ebp-0x2C]; review N5: the vanilla PC tail computes its own
// range with 0x571570(self, 0, 1), this uses the range the approach test just used).
inline bool a1TryRetarget(const Config& c, int rawStance, float maxRange) {
    return c.a1Retarget && inMask(c.a1RetargetStances, rawStance) && maxRange > 0.5f;
}
inline int a1NewTarget(int found) { return found ? found : Invalid; }   // 0x57C990 may return 0 or INVALID for "none"
// Shorten the fallback timer after an END (not after a retarget) when a1_rethink_ms < 3000.
inline bool a1RethinkWanted(const Config& c, int newId) { return newId == Invalid && c.a1RethinkMs < 3000; }
inline bool a1RethinkWrite(const Config& c, int cur570) { return cur570 > c.a1RethinkMs; }

// A4 (0x6D9E2A, a4=1). Returns the handler result (1 => 0x6D9EA4 = vanilla's JZ target) and the value for [ebp-0x15C].
struct A4Out { int ret; int local; bool guarded; };
inline A4Out a4Decide(bool rowReadable, int v) {
    if (!rowReadable) return {1, 0, true};   // NULL / unreadable row: the "not hostile" path instead of a crash
    return {v == 0 ? 1 : 0, v, false};       // same branch vanilla's CMP/JZ takes
}
inline int a4VanillaResult(int v) { return v == 0 ? 1 : 0; }   // a4=0: identical mapping after the direct (faulting) read

// C1 (0x5868B9/0x5868F1/0x586929): the restart index written to [ebp-0x10].
inline bool c1Who(const Config& c, int objType, int party) { return !c.c1PartyOnly || (objType == 5 && party != 0); }
inline int c1Value(bool enabled, bool who, bool capOk) { return (enabled && who && capOk) ? -1 : 0; }

// P1 (0x56BDB4). With p1=0 a stamp can only exist if p1 was on earlier this session: zero it and force the run so the
// creature is back in the vanilla never-stamped state.
inline bool p1ParityReset(bool p1, unsigned stampA, unsigned stampB) { return !p1 && (stampA | stampB) != 0; }
inline unsigned p1Local(bool run) { return run ? 0xFA0u : 0u; }   // [ebp-0x28] for the stolen CMP/JC

// FM1 (0x586F43): fallback cap after a bump clear.
inline bool fm1RethinkWrite(const Config& c, int party, int combat, int cur570) {
    return c.fm1 && party && c.fm1RethinkMs > 0 && combat == 1 && cur570 > c.fm1RethinkMs;
}

// FM4 (0x57451F snapshot / 0x575183 restore): single-slot pairing keyed by creature, cleared on every use.
struct Fm4Pair {
    int cr = 0;
    bool begin(int fm4, bool log) { cr = 0; return fm4 == 1 || log; }   // snapshot hook entry: anything to do?
    void commit(int creature) { cr = creature; }                          // after a successful copy
    bool take(int creature) { bool m = cr != 0 && cr == creature; cr = 0; return m; }   // restore hook: same creature?
};
inline bool fm4ShouldRestore(int fm4, bool liveFollowInfoOk) { return liveFollowInfoOk && fm4 == 1; }

// INI hot reload (loadIni). Pre: before reading the file; post: after trying to open and parse it.
inline bool reloadDue(bool force, unsigned now, unsigned lastCheck) { return force || now - lastCheck >= 1000; }
enum class ReloadPre { Skip, Read, MissingSilent, MissingKeepLog, MissingDefaultsLog };
inline ReloadPre reloadPre(bool force, bool exists, bool seen, bool unchanged) {
    if (!exists) return force ? ReloadPre::MissingDefaultsLog : seen ? ReloadPre::MissingKeepLog : ReloadPre::MissingSilent;
    if (!force && seen && unchanged) return ReloadPre::Skip;
    return ReloadPre::Read;
}
enum class ReloadPost { RetryLater, KeepBadSection, Apply };
inline ReloadPost reloadPost(bool force, bool opened, bool sectionFound) {
    if (!opened) return ReloadPost::RetryLater;                          // e.g. an editor holds the file
    if (!sectionFound && !force) return ReloadPost::KeepBadSection;       // half-written: keep the current config
    return ReloadPost::Apply;
}

// Per-subject log throttle table (review N1: FM1/E2/A4 lines), LRU over `Slots` subjects.
struct SubjectThrottle {
    static constexpr int Slots = 32;
    struct E { int kind = -1, id = 0; unsigned seen = 0; LineThrottle t; } e[Slots];
    bool allow(int kind, int id, unsigned now, unsigned sig, int first, unsigned periodMs) {
        E* slot = nullptr; E* oldest = &e[0];
        for (auto& x : e) {
            if (x.kind == kind && x.id == id) { slot = &x; break; }
            if (x.kind < 0) { if (oldest->kind >= 0) oldest = &x; }
            else if (oldest->kind >= 0 && now - x.seen > now - oldest->seen) oldest = &x;
        }
        if (!slot) { slot = oldest; *slot = E(); slot->kind = kind; slot->id = id; }
        slot->seen = now;
        return slot->t.allow(now, sig, first, periodMs);
    }
};

// ---------------------------------------------------------------- Phase-4 probe helpers (PROBE_DESIGN.md section 4; log only)
// RunScript 0x6FD8D0 return address -> which engine site ran the script (return = CALL + 5). 0 = not a probed site.
// The hot-ish RS handler calls this first: at most 7 compares, then out.
inline int rsSite(unsigned ret) {
    switch (ret) {
    case 0x58F514u: return 1;   // S6A EndCombatRound slot-6 re-run
    case 0x569A6Cu: return 2;   // S6B AIUpdate fallback
    case 0x59E909u: return 3;   // S6C SetState removal
    case 0x5709F5u: return 4;   // S7 shout delivery (listener)
    case 0x57074Au: return 5;   // S12 OnBlocked
    case 0x570645u:
    case 0x57069Fu: return 6;   // S1 event table
    default: return 0;
    }
}
inline const char* rsName(int site) {
    static const char* const n[] = {"-", "S6A", "S6B", "S6C", "S7", "S12", "S1"};
    return (site >= 0 && site <= 6) ? n[site] : "-";
}

// SetPartyMemberFlag 0x58A330 callers: the 15 CALL sites + 5 (return address at [ebp+4]). -1 = unknown.
constexpr int PfSites = 15;
inline int pfSite(unsigned ret) {
    switch (ret) {
    case 0x574537u: return 0;    // SAVE_A
    case 0x575170u: return 1;    // SAVE_B
    case 0x599AF4u: return 2;    // EFF89
    case 0x5F89AAu: return 3;    // XFER_A
    case 0x5F8A13u: return 4;    // XFER_B
    case 0x5F8EB9u: return 5;    // ADDMEMBER
    case 0x5F93B4u: return 6;    // CREATEPARTY
    case 0x5FEAC7u: return 7;    // SWITCHPC
    case 0x6306A5u: return 8;    // CHARGEN
    case 0x630B17u: return 9;    // PLAYERLOAD
    case 0x66CA5Au: return 10;   // CMD574
    case 0x6900C1u: return 11;   // CMD575
    case 0x69B901u: return 12;   // CMD846
    case 0x89FE45u: return 13;   // SELECT_DEL
    case 0x8A050Eu: return 14;   // SELECT_ADD
    default: return -1;
    }
}
inline const char* pfName(int site) {
    static const char* const n[PfSites] = {"SAVE_A", "SAVE_B", "EFF89", "XFER_A", "XFER_B", "ADDMEMBER", "CREATEPARTY", "SWITCHPC",
                                           "CHARGEN", "PLAYERLOAD", "CMD574", "CMD575", "CMD846", "SELECT_DEL", "SELECT_ADD"};
    return (site >= 0 && site < PfSites) ? n[site] : "?";
}
// The save bounce (SaveCreature clears and re-sets the flag) and the party transfer write the flag twice per creature.
inline bool pfTransient(int site) { return site == 0 || site == 1 || site == 3 || site == 4; }

// P2 (0x56BE79): the engine runs UpdatePerception(0) iff +0x10E8 == 0 && mode == 1 && elapsed >= 4000 (0x56BE8B..0x56BEA9).
inline bool p2Runs(int isPC, int mode, unsigned elapsed) { return isPC == 0 && mode == 1 && elapsed >= 4000u; }

// SHX (0x5475E8, the single RET 0xC of the shout delivery): which of the 5 exits was taken, from the frame at the exit.
enum ShoutExit { ShoutNoArea = 0, ShoutAbortStart, ShoutEnd, ShoutXBreak, ShoutAbortMid };
inline int shoutExit(bool area, int idx, int count, bool lastOk) {
    if (!area) return ShoutNoArea;          // 00547180 JZ
    if (idx < 0) return ShoutAbortStart;    // 005471b5 JL (0x522AF0 found no start)
    if (idx >= count) return ShoutEnd;      // 005471c4 JGE
    return lastOk ? ShoutXBreak : ShoutAbortMid;   // 00547203 JNZ with a resolvable / an unresolvable last candidate
}
inline const char* shoutName(int k) {
    static const char* const n[] = {"noarea", "ABORT-START", "end", "xbreak", "ABORT-MID"};
    return (k >= 0 && k <= 4) ? n[k] : "?";
}

// FM2C (0x55B69A): the label for why the search failed / was partial (log only).
inline const char* fm2cReason(int cp, int cached, int res) {
    if (cp == 1) return "client";
    if (cached == -1) return "code-1";
    if (res == 2) return "partial";
    if (res == 3) return "blocked";
    return "other";
}

// FM3 (UpdateState level end): the sentinel object left its level (or was removed) => the level cannot end on it.
inline bool fm3Lost(int level, int nowLevel, bool removed) { return removed || nowLevel != level; }

// A1X: the strict 2D test the attack itself applies: dist2 <= (maxRange + 0.1)^2 (candidate retargeted by 0x57C990 uses 3D minus radii).
inline bool a1Strict(float ax, float ay, float tx, float ty, float maxR) {
    float dx = ax - tx, dy = ay - ty, lim = maxR + 0.1f;
    return dx * dx + dy * dy <= lim * lim;
}
// A1X: an A1 end/retarget must be followed by a slot-6 run (or E2) within the rethink time + 1 s.
inline bool a1OrphanDue(unsigned now, unsigned endAt, int rethinkMs) {
    return endAt != 0 && now - endAt > static_cast<unsigned>(rethinkMs) + 1000u;
}

// Per-(creature, target) event counter inside a sliding window (gap since the last event). Returns true when it (re)started.
struct PairCount {
    int cr = 0, tgt = 0; unsigned n = 0, first = 0, last = 0; float lastD = 0;
};
inline bool pairBump(PairCount& p, int cr, int tgt, unsigned now, unsigned windowMs) {
    bool reset = p.n == 0 || p.cr != cr || p.tgt != tgt || now - p.last > windowMs;
    if (reset) { p.cr = cr; p.tgt = tgt; p.n = 1; p.first = now; p.lastD = 0; } else ++p.n;
    p.last = now;
    return reset;
}

// RAR histogram bucket: (action type 1..0x47, else 0) * 5 + (result 1..4, else 0). 0x48 types x 5 results.
constexpr int RarTypes = 0x48, RarBuckets = RarTypes * 5;
inline int rarIndex(int type, int res) { return ((type >= 1 && type <= 0x47) ? type : 0) * 5 + ((res >= 1 && res <= 4) ? res : 0); }

// Per-probe session line budget: `take` is false once `budget` lines were granted; `marker` is true exactly once afterwards.
struct ProbeCap {
    unsigned used = 0; bool refused = false, marked = false;
    bool take(unsigned budget) {
        if (used < budget) { ++used; return true; }
        refused = true;
        return false;
    }
    bool marker() { if (refused && !marked) { marked = true; return true; } return false; }
};

// Copy a C string of at most cap-1 chars, replacing control characters so a log line stays one line.
inline void sanitize(char* s) { for (; *s; ++s) if ((unsigned char)*s < 0x20) *s = ' '; }

} // namespace cai
