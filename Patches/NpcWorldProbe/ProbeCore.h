// ProbeCore.h -- pure logic of the npc-world-probe client (doc 40 V2a). Header-only, std only, no Windows, no engine, no heap.
// The DLL (NpcWorldProbe.cpp) turns k2.npcworld events into nwp::Ev and supplies an nwp::Env; everything decided here is host-tested.
//
// Stages (ini npc_world_probe.ini [probe] stages=, comma list; default "t26,persist"):
//   t26       the old k1-spawn-probe 0.2.0 roster (c_drdmouse01 + k1p_native_npc in 204tel); the NB0 live check.
//   pool      rows from [probe] row1..row16 = resref,tag,area,x,y,z[,facing]  (P1 walker, P2 click, P3 hostility, P12/P14 test templates)
//   capacity  capacity_n copies of one template on a grid (capacity 1/6/18; needs npc_world.ini [world] spawn_cap >= capacity_n)
//   persist   P4/P7/P15 evidence: lib adds with +0x11AC/+0x11FC, engine-restored copies of watched tags, prev-visit verdict
//   frame     TICK frame-interval windows before / after the spawns (capacity cost, P10 style cost evidence)
// Not buildable as a client of k2.npcworld v1 (no library service, native conventions unresolved): P5, P6, P8, P9, P10 proper, P11, P13.
// Those need the NB2 service growth or an RE run first (doc 40 section 13).
#ifndef NWP_PROBE_CORE_H
#define NWP_PROBE_CORE_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

namespace nwp {

// Same values as K2NW_* in NpcWorld.h (the e2e test static_asserts the equality).
constexpr int EV_LOAD = 1, EV_ADDED = 2, EV_STABLE = 4, EV_TICK = 8;
constexpr int ORIGIN_ENGINE = 0, ORIGIN_LIB = 1;

constexpr unsigned ST_T26 = 1, ST_POOL = 2, ST_CAPACITY = 4, ST_PERSIST = 8, ST_FRAME = 16;
constexpr unsigned DefaultStages = ST_T26 | ST_PERSIST;
constexpr int MaxPool = 16, MaxWatch = 24, MaxCapacity = 32, MaxLine = 224;   // KpLog cuts a line at 232 bytes
constexpr uint32_t FrameGapResetUs = 2000000;   // a tick gap above this is a load stall, not frame time

enum Chan { CH_CORE = 0, CH_STAGE, CH_PERSIST, CH_FRAME, CH_SPAWN };

inline char lower(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; }
inline bool ciEq(const char* a, const char* b) {
    if (!a || !b) return false;
    for (;; ++a, ++b) {
        const char ca = lower(*a), cb = lower(*b);
        if (ca != cb) return false;
        if (!ca) return true;
    }
}
inline bool copyStr(char* dst, size_t cap, const char* s) {   // false (dst = "") when it does not fit
    dst[0] = 0;
    if (!s) return false;
    const size_t n = strlen(s);
    if (n >= cap) return false;
    memcpy(dst, s, n + 1);
    return true;
}

struct Row {
    char resref[17] = {};
    char tag[33] = {};
    char area[17] = {};
    float x = 0, y = 0, z = 0, facing = 0;
};

// ---- parsing ------------------------------------------------------------------------------------------------------------------------
// Stage list -> mask. Separators , ; | and blanks. Unknown tokens are counted in *unknown. A blank list gives DefaultStages.
inline unsigned parseStages(const char* s, int* unknown = nullptr) {
    if (unknown) *unknown = 0;
    if (!s) return DefaultStages;
    unsigned mask = 0;
    bool any = false;
    const char* p = s;
    while (*p) {
        while (*p == ',' || *p == ';' || *p == '|' || *p == ' ' || *p == '\t') ++p;
        if (!*p) break;
        char tok[16];
        size_t n = 0;
        while (*p && *p != ',' && *p != ';' && *p != '|' && *p != ' ' && *p != '\t') { if (n < sizeof tok - 1) tok[n++] = lower(*p); ++p; }
        tok[n] = 0;
        any = true;
        if (!strcmp(tok, "t26")) mask |= ST_T26;
        else if (!strcmp(tok, "pool")) mask |= ST_POOL;
        else if (!strcmp(tok, "capacity")) mask |= ST_CAPACITY;
        else if (!strcmp(tok, "persist")) mask |= ST_PERSIST;
        else if (!strcmp(tok, "frame")) mask |= ST_FRAME;
        else if (!strcmp(tok, "all")) mask |= ST_T26 | ST_POOL | ST_CAPACITY | ST_PERSIST | ST_FRAME;
        else if (!strcmp(tok, "none")) { /* explicit empty list */ }
        else if (unknown) ++*unknown;
    }
    return any ? mask : DefaultStages;
}

inline bool parseFloat(const char* s, float* out) {
    char* end = nullptr;
    const double v = strtod(s, &end);
    if (end == s || !isfinite(v)) return false;
    while (*end == ' ' || *end == '\t') ++end;
    if (*end) return false;
    *out = static_cast<float>(v);
    return true;
}

// "resref,tag,area,x,y,z[,facing]" -> Row. Fields are trimmed; resref <= 16, tag <= 32, area <= 16 chars; nothing empty.
inline bool parseRow(const char* line, Row* out) {
    if (!line || !out) return false;
    char f[7][40];
    int n = 0;
    const char* p = line;
    for (;;) {
        while (*p == ' ' || *p == '\t') ++p;
        const char* q = p;
        while (*q && *q != ',') ++q;
        const char* e = q;
        while (e > p && (e[-1] == ' ' || e[-1] == '\t')) --e;
        if (n >= 7) return false;
        const size_t len = static_cast<size_t>(e - p);
        if (len >= sizeof f[0]) return false;
        memcpy(f[n], p, len);
        f[n][len] = 0;
        ++n;
        if (!*q) break;
        p = q + 1;
    }
    if (n < 6) return false;
    Row r;
    if (!f[0][0] || !f[1][0] || !f[2][0]) return false;
    if (!copyStr(r.resref, sizeof r.resref, f[0]) || !copyStr(r.tag, sizeof r.tag, f[1]) || !copyStr(r.area, sizeof r.area, f[2])) return false;
    if (!parseFloat(f[3], &r.x) || !parseFloat(f[4], &r.y) || !parseFloat(f[5], &r.z)) return false;
    if (n == 7 && !parseFloat(f[6], &r.facing)) return false;
    *out = r;
    return true;
}

// ---- configuration -----------------------------------------------------------------------------------------------------------------
struct Config {
    unsigned stages = DefaultStages;
    Row pool[MaxPool];
    int nPool = 0;
    int badKeys = 0;   // unknown keys / unparsable values seen by set() (logged once by the DLL)
    // capacity stage
    int capN = 0;
    char capResref[17] = "c_drdmouse01";
    char capRealTag[33] = "";    // the template's real tag; non-empty = skip the stage when a creature with it is already present
    char capArea[17] = "204tel";
    float capX = -39.0f, capY = -63.1f, capZ = 0.0f, capStep = 0.9f;
    int capCols = 6;
    // frame stage
    int frameWindow = 120, framePreWindows = 4, framePostWindows = 12;

    // The t26 roster is fixed in code (pool[] holds only ini rows). z 0.0: every 204tel floor point in the GIT is z 0.0.
    static int t26Count() { return 2; }
    static Row t26Row(int i) {
        Row r;
        if (i == 0) { copyStr(r.resref, sizeof r.resref, "c_drdmouse01"); copyStr(r.tag, sizeof r.tag, "MouseDroid"); copyStr(r.area, sizeof r.area, "204tel"); r.x = -39.0f; r.y = -63.1f; }
        else        { copyStr(r.resref, sizeof r.resref, "k1p_native_npc"); copyStr(r.tag, sizeof r.tag, "k1p_native_npc"); copyStr(r.area, sizeof r.area, "204tel"); r.x = -21.1f; r.y = -31.1f; }
        return r;
    }

    static bool toInt(const char* v, int lo, int hi, int* out) {
        char* end = nullptr;
        const long n = strtol(v, &end, 10);
        if (end == v || *end || n < lo || n > hi) return false;
        *out = static_cast<int>(n);
        return true;
    }

    // One "key=value" of the [probe] section. False (and badKeys++) for an unknown key or a value that does not parse.
    bool set(const char* key, const char* value) {
        if (!key || !value) { ++badKeys; return false; }
        bool ok = true;
        if (ciEq(key, "stages")) { int unk = 0; stages = parseStages(value, &unk); ok = unk == 0; }
        else if (ciEq(key, "capacity_n")) ok = toInt(value, 0, MaxCapacity, &capN);
        else if (ciEq(key, "capacity_resref")) ok = value[0] && copyStr(capResref, sizeof capResref, value);
        else if (ciEq(key, "capacity_realtag")) ok = copyStr(capRealTag, sizeof capRealTag, value);
        else if (ciEq(key, "capacity_area")) ok = value[0] && copyStr(capArea, sizeof capArea, value);
        else if (ciEq(key, "capacity_x")) ok = parseFloat(value, &capX);
        else if (ciEq(key, "capacity_y")) ok = parseFloat(value, &capY);
        else if (ciEq(key, "capacity_z")) ok = parseFloat(value, &capZ);
        else if (ciEq(key, "capacity_step")) ok = parseFloat(value, &capStep);
        else if (ciEq(key, "capacity_cols")) ok = toInt(value, 1, MaxCapacity, &capCols);
        else if (ciEq(key, "frame_window")) ok = toInt(value, 10, 100000, &frameWindow);
        else if (ciEq(key, "frame_pre_windows")) ok = toInt(value, 0, 1000, &framePreWindows);
        else if (ciEq(key, "frame_post_windows")) ok = toInt(value, 0, 1000, &framePostWindows);
        else if (key[0] && (key[0] == 'r' || key[0] == 'R') && (key[1] == 'o' || key[1] == 'O') && (key[2] == 'w' || key[2] == 'W') && key[3]) {
            Row r;
            ok = nPool < MaxPool && parseRow(value, &r);
            if (ok) pool[nPool++] = r;
        } else ok = false;
        if (!ok) ++badKeys;
        return ok;
    }
};

// Capacity row i: the template at a grid cell. The tag passed to Spawn() is only the library's duplicate key, so it is unique per cell.
inline Row capacityRow(const Config& c, int i) {
    Row r;
    copyStr(r.resref, sizeof r.resref, c.capResref);
    snprintf(r.tag, sizeof r.tag, "nwp_cap_%02d", i);
    copyStr(r.area, sizeof r.area, c.capArea);
    r.x = c.capX + static_cast<float>(i % c.capCols) * c.capStep;
    r.y = c.capY + static_cast<float>(i / c.capCols) * c.capStep;
    r.z = c.capZ;
    r.facing = 0.0f;
    return r;
}

// ---- frame statistics -----------------------------------------------------------------------------------------------------------------
struct FrameStats {
    int n = 0;
    uint64_t sumUs = 0;
    uint32_t maxUs = 0;
    int ge33 = 0, ge50 = 0;   // intervals >= 33.3 ms and >= 50 ms
    void clear() { *this = FrameStats(); }
    void push(uint32_t us) {
        ++n; sumUs += us;
        if (us > maxUs) maxUs = us;
        if (us >= 33333u) ++ge33;
        if (us >= 50000u) ++ge50;
    }
    uint32_t avgUs() const { return n ? static_cast<uint32_t>(sumUs / static_cast<uint64_t>(n)) : 0u; }
};

// ---- environment (supplied by the DLL, or by a fake in the host tests) -------------------------------------------------------------------
struct Env {
    virtual ~Env() {}
    virtual void log(Chan ch, int level, const char* msg) = 0;   // level: 0 info, 1 warn
    virtual bool tagPresent(const char* tag) = 0;
    virtual uint32_t spawn(const Row& r) = 0;                   // 0 = refused / failed
    virtual int spawned() = 0;                                  // library count this visit
    virtual int cap() = 0;                                      // library spawn cap
    virtual uint32_t nowUs() = 0;
};

// One event, already decoded from K2NwEvent. fieldsOk = f11ac/f11fc/areaId were readable (CREATURE_ADDED only).
struct Ev {
    int kind = 0;
    int loadSeq = 0, visitSeq = 0;
    uint32_t salt = 0;
    const char* area = "";
    const char* tag = "";
    uint32_t oid = 0;
    float x = 0, y = 0, z = 0;
    int origin = 0;
    bool fieldsOk = false;
    uint32_t f11ac = 0, f11fc = 0, areaId = 0;
};

class Probe {
public:
    void configure(const Config& c, Env* e) {
        cfg_ = c; env_ = e;
        nWatch_ = 0;
        if (cfg_.stages & ST_T26) for (int i = 0; i < Config::t26Count(); ++i) watch(Config::t26Row(i).tag);
        if (cfg_.stages & ST_POOL) for (int i = 0; i < cfg_.nPool; ++i) watch(cfg_.pool[i].tag);
        if ((cfg_.stages & ST_CAPACITY) && cfg_.capRealTag[0]) watch(cfg_.capRealTag);
    }
    const Config& config() const { return cfg_; }
    // The KPATCH event kinds to register for.
    int mask() const {
        int m = EV_LOAD | EV_ADDED | EV_STABLE;
        if (cfg_.stages & ST_FRAME) m |= EV_TICK;
        return m;
    }

    void onLoad(const Ev& ev) {
        spawnedVisit_ = 0; restored_ = 0; libAdds_ = 0; engineAdds_ = 0;
        post_ = false; lastTickUs_ = 0; frame_.clear(); preLogged_ = 0; postLogged_ = 0;
        logf(CH_CORE, 0, "LOAD n=%d salt=%08x stages=%u", ev.loadSeq, static_cast<unsigned>(ev.salt), cfg_.stages);
    }

    void onAdded(const Ev& ev) {
        if (ev.origin == ORIGIN_LIB) {
            ++libAdds_;
            if (cfg_.stages & ST_PERSIST) {
                if (ev.fieldsOk) logf(CH_PERSIST, 0, "LIBADD oid=%08x tag=\"%s\" pos=%.1f,%.1f,%.1f f11ac=%08x f11fc=%08x areaId=%08x", ev.oid, ev.tag, ev.x, ev.y, ev.z, ev.f11ac, ev.f11fc, ev.areaId);
                else logf(CH_PERSIST, 0, "LIBADD oid=%08x tag=\"%s\" pos=%.1f,%.1f,%.1f fields unreadable", ev.oid, ev.tag, ev.x, ev.y, ev.z);
            }
            return;
        }
        ++engineAdds_;
        if ((cfg_.stages & ST_PERSIST) && watched(ev.tag)) {
            ++restored_;
            if (ev.fieldsOk) logf(CH_PERSIST, 0, "RESTORED oid=%08x tag=\"%s\" area=%s pos=%.1f,%.1f,%.1f f11ac=%08x f11fc=%08x", ev.oid, ev.tag, ev.area, ev.x, ev.y, ev.z, ev.f11ac, ev.f11fc);
            else logf(CH_PERSIST, 0, "RESTORED oid=%08x tag=\"%s\" area=%s pos=%.1f,%.1f,%.1f fields unreadable", ev.oid, ev.tag, ev.area, ev.x, ev.y, ev.z);
        }
    }

    void onStable(const Ev& ev) {
        logf(CH_STAGE, 0, "STABLE visit=%d load=%d area=%s salt=%08x engine_adds=%d restored=%d", ev.visitSeq, ev.loadSeq, ev.area, static_cast<unsigned>(ev.salt), engineAdds_, restored_);
        if ((cfg_.stages & ST_PERSIST) && prevSpawned_ > 0 && ciEq(prevArea_, ev.area)) {
            const char* v = restored_ == 0 ? "none-persisted" : (restored_ >= prevSpawned_ ? "all-persisted" : "partial");
            logf(CH_PERSIST, 0, "PERSIST-CHECK area=%s prev_spawned=%d restored_now=%d verdict=%s", ev.area, prevSpawned_, restored_, v);
        }
        if (cfg_.stages & ST_T26) for (int i = 0; i < Config::t26Count(); ++i) spawnIfFree("t26", Config::t26Row(i), ev.area);
        if (cfg_.stages & ST_POOL) for (int i = 0; i < cfg_.nPool; ++i) spawnIfFree("pool", cfg_.pool[i], ev.area);
        if ((cfg_.stages & ST_CAPACITY)) capacity(ev.area);
        if (spawnedVisit_ > 0) {
            copyStr(prevArea_, sizeof prevArea_, ev.area);
            prevSpawned_ = spawnedVisit_;
            post_ = true;
            frame_.clear();
        }
    }

    void onTick(const Ev&) {
        if (!(cfg_.stages & ST_FRAME) || !env_) return;
        const uint32_t now = env_->nowUs();
        const uint32_t last = lastTickUs_;
        lastTickUs_ = now ? now : 1u;   // 0 means "no previous tick"
        if (!last) return;
        const uint32_t d = now - last;   // wraps correctly in uint32
        if (d >= FrameGapResetUs) return;
        frame_.push(d);
        if (frame_.n < cfg_.frameWindow) return;
        int& logged = post_ ? postLogged_ : preLogged_;
        const int budget = post_ ? cfg_.framePostWindows : cfg_.framePreWindows;
        if (logged < budget) {
            ++logged;
            logf(CH_FRAME, 0, "FRAME phase=%s spawned=%d win=%d avg=%u.%02ums max=%u.%02ums ge33=%d ge50=%d", post_ ? "post" : "pre", spawnedVisit_, frame_.n,
                 static_cast<unsigned>(frame_.avgUs() / 1000), static_cast<unsigned>(frame_.avgUs() % 1000 / 10),
                 static_cast<unsigned>(frame_.maxUs / 1000), static_cast<unsigned>(frame_.maxUs % 1000 / 10), frame_.ge33, frame_.ge50);
        }
        frame_.clear();
    }

    int spawnedVisit() const { return spawnedVisit_; }
    int restored() const { return restored_; }
    int libAdds() const { return libAdds_; }
    int engineAdds() const { return engineAdds_; }
    bool post() const { return post_; }
    int prevSpawned() const { return prevSpawned_; }
    int nWatch() const { return nWatch_; }

private:
    Config cfg_;
    Env* env_ = nullptr;
    char watch_[MaxWatch][33];
    int nWatch_ = 0;
    int spawnedVisit_ = 0, restored_ = 0, libAdds_ = 0, engineAdds_ = 0;
    int prevSpawned_ = 0;
    char prevArea_[17] = {};
    bool post_ = false;
    uint32_t lastTickUs_ = 0;
    FrameStats frame_;
    int preLogged_ = 0, postLogged_ = 0;

    void watch(const char* tag) {
        if (!tag || !tag[0] || nWatch_ >= MaxWatch || watched(tag)) return;
        copyStr(watch_[nWatch_++], sizeof watch_[0], tag);
    }
    bool watched(const char* tag) const {
        for (int i = 0; i < nWatch_; ++i) if (ciEq(watch_[i], tag)) return true;
        return false;
    }

    void logf(Chan ch, int level, const char* fmt, ...) {
        if (!env_) return;
        char buf[MaxLine];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof buf, fmt, ap);
        va_end(ap);
        env_->log(ch, level, buf);
    }

    bool spawnOne(const Row& r) {
        const uint32_t oid = env_->spawn(r);
        if (!oid) {
            logf(CH_SPAWN, 1, "REFUSED resref=%s tag=%s at %.1f,%.1f,%.1f spawned=%d cap=%d", r.resref, r.tag, r.x, r.y, r.z, env_->spawned(), env_->cap());
            return false;
        }
        ++spawnedVisit_;
        logf(CH_SPAWN, 0, "SPAWNED resref=%s tag=%s oid=%08x at %.1f,%.1f,%.1f lib=%d/%d", r.resref, r.tag, oid, r.x, r.y, r.z, env_->spawned(), env_->cap());
        return true;
    }

    void spawnIfFree(const char* stage, const Row& r, const char* area) {
        if (!env_ || !ciEq(r.area, area)) return;
        if (env_->tagPresent(r.tag)) { logf(CH_STAGE, 0, "SKIP stage=%s resref=%s tag=%s already present", stage, r.resref, r.tag); return; }
        spawnOne(r);
    }

    void capacity(const char* area) {
        if (!env_ || cfg_.capN <= 0) { logf(CH_STAGE, 0, "CAPACITY nothing to do (capacity_n=%d)", cfg_.capN); return; }
        if (!ciEq(cfg_.capArea, area)) { logf(CH_STAGE, 0, "CAPACITY skipped: area %s is not capacity_area %s", area, cfg_.capArea); return; }
        if (cfg_.capRealTag[0] && env_->tagPresent(cfg_.capRealTag)) { logf(CH_STAGE, 0, "CAPACITY skipped: tag %s already present (restored from the save)", cfg_.capRealTag); return; }
        const int cap = env_->cap();
        if (cap < cfg_.capN) logf(CH_STAGE, 1, "CAPACITY capacity_n=%d exceeds the library spawn cap %d: raise npc_world.ini [world] spawn_cap", cfg_.capN, cap);
        int ok = 0;
        for (int i = 0; i < cfg_.capN; ++i) {
            if (!spawnOne(capacityRow(cfg_, i))) break;
            ++ok;
        }
        logf(CH_STAGE, 0, "CAPACITY requested=%d spawned=%d cap=%d", cfg_.capN, ok, cap);
    }
};

} // namespace nwp

#endif /* NWP_PROBE_CORE_H */
