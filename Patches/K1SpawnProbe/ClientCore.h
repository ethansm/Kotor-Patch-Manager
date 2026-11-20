// ClientCore.h -- Pure logic of the k1-spawn-probe client (doc 38 stage 2), extracted from K1SpawnProbe.cpp so the host can test it.
// Header-only, std only (<stdint.h>, <stdio.h>). No Windows headers, no exceptions, no heap, no globals.
// Holds: the log line budget (3000 total, 2500 of them ADDTOAREA), the nested event scope (the outermost event owns the log handle),
// the dump gate (one NPC dump, up to 3 DUMP-ANY ids), the tag-scan budget (64 scans), and RunStable (the SPAWN stable sequence).
// The DLL supplies an Env that writes the log, calls npc-world and reads the library's seen-tag list.
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "ClientLogic.h"

namespace k1spawn {

enum class LogKind { Other, AddToArea };

// Line budget for the client's log. reserve() is check-and-charge: a refused reservation charges nothing and adds its lines to dropped().
class LogBudget {
public:
    static constexpr int kTotal = 3000;
    static constexpr int kAddToArea = 2500;

    bool reserve(int lines, LogKind k) {
        if (lines < 0) return false;
        const bool overTotal = lines > kTotal - used_;
        const bool overAdd = k == LogKind::AddToArea && lines > kAddToArea - addUsed_;
        if (overTotal || overAdd) {
            dropped_ += lines;
            return false;
        }
        used_ += lines;
        if (k == LogKind::AddToArea) addUsed_ += lines;
        return true;
    }

    // No charge: true when `lines` more would stay within the total cap.
    bool fits(int lines) const { return lines >= 0 && lines <= kTotal - used_; }
    bool addToAreaAllowed() const { return addUsed_ < kAddToArea && used_ < kTotal; }
    bool capped() const { return used_ >= kTotal; }

    // True exactly once, the first time the total cap is reached. Formats "CAP reached at n=<seq>" into out (NUL-terminated when n > 0).
    // The marker line is not charged against the budget.
    bool takeCapMarker(long seq, char* out, int n) {
        if (!capped() || capMarked_) return false;
        capMarked_ = true;
        if (out && n > 0) snprintf(out, static_cast<size_t>(n), "CAP reached at n=%ld", seq);
        return true;
    }

    int used() const { return used_; }
    long dropped() const { return dropped_; }

private:
    int used_ = 0;
    int addUsed_ = 0;
    long dropped_ = 0;
    bool capMarked_ = false;
};

// Nesting depth of one callback. Events nest (a spawn inside onStable re-enters the creature hook), so only the outermost one opens the log.
class EventScope {
public:
    // True only for the 0 -> 1 transition.
    bool open() { return depth_++ == 0; }
    // True only for the 1 -> 0 transition. Closing at depth 0 changes nothing and returns false.
    bool close() {
        if (depth_ <= 0) return false;
        return --depth_ == 0;
    }
    int depth() const { return depth_; }

private:
    int depth_ = 0;
};

// The seam between this logic and the DLL: log, npc-world, and the seen-tag list.
struct Env {
    virtual ~Env() {}
    virtual void line(const char* text) = 0;
    virtual void openLog() = 0;
    virtual void closeLog() = 0;
    virtual uint32_t spawn(const char* resref, const char* tag, float x, float y, float z, float facing) = 0;
    virtual bool tagPresent(const char* tag) = 0;
    virtual void flush() {}
};

// RAII event: the outermost ScopedEvent on a scope opens the log, and the matching outermost end closes it.
class ScopedEvent {
public:
    ScopedEvent(EventScope& scope, Env& env) : scope_(scope), env_(env) {
        if (scope_.open()) env_.openLog();
    }
    ~ScopedEvent() {
        if (scope_.close()) env_.closeLog();
    }
    ScopedEvent(const ScopedEvent&) = delete;
    ScopedEvent& operator=(const ScopedEvent&) = delete;

private:
    EventScope& scope_;
    Env& env_;
};

// Dump triggers. The NPC dump is taken once per process. DUMP-ANY is taken for the first kMaxAny distinct creature ids.
// wantX() only asks. The flags change only in commitX(), so a dump the caller refused or skipped leaves the gate open.
class DumpGate {
public:
    static constexpr int kMaxAny = 3;

    bool wantNpc(bool hit) const { return hit && !npcDumped_; }

    bool wantAny(unsigned id) const {
        if (anyDumps_ >= kMaxAny) return false;
        for (int i = 0; i < anyDumps_; ++i) {
            if (anyIds_[i] == id) return false;
        }
        return true;
    }

    void commitNpc() { npcDumped_ = true; }

    // Records the id only when wantAny(id) holds, so a repeated id or a 4th id changes nothing.
    void commitAny(unsigned id) {
        if (!wantAny(id)) return;
        anyIds_[anyDumps_++] = id;
    }

private:
    bool npcDumped_ = false;
    int anyDumps_ = 0;
    unsigned anyIds_[kMaxAny] = {};
};

// Budget for the fallback tag scan (scanForTag). A scan runs only when both tag strings came back empty, at most kMaxScans per process,
// and never once the log is capped (its lines would be refused anyway).
class ScanBudget {
public:
    static constexpr int kMaxScans = 64;

    bool allow(bool libTagEmpty, bool tagStrEmpty, bool logCapped) {
        if (!libTagEmpty || !tagStrEmpty || logCapped || scans_ >= kMaxScans) return false;
        ++scans_;
        return true;
    }

    int scans() const { return scans_; }

private:
    int scans_ = 0;
};

// One line through the budget: reserved as Other, skipped (and counted as dropped) when refused.
inline void EmitLine(Env& env, LogBudget& budget, const char* text) {
    if (!budget.reserve(1, LogKind::Other)) return;
    env.line(text);
}

// The POP-STABLE sequence for one visit (K1SpawnProbe.cpp onStable). Every row is decided and spawned whether or not its lines fit the budget.
inline void RunStable(Env& env, const char* area, int visitSeq, unsigned salt, bool enabled, LogBudget& budget) {
    const char* a = area ? area : "";
    int rows[kMaxRowsPerArea];
    const int n = RowsForArea(a, rows, kMaxRowsPerArea);
    char buf[256];

    snprintf(buf, sizeof buf, "SPAWN stable area=%s visit=%d salt=%08x rows=%d spawn=%s", a, visitSeq, salt, n, enabled ? "on" : "off");
    EmitLine(env, budget, buf);
    if (!enabled) return;

    for (int i = 0; i < n; ++i) {
        const RosterRow& row = kRoster[rows[i]];
        if (env.tagPresent(row.tag)) {
            snprintf(buf, sizeof buf, "SPAWN skip resref=%.16s tag=%.32s already present (library seen-tag list)", row.resref, row.tag);
            EmitLine(env, budget, buf);
            continue;
        }
        snprintf(buf, sizeof buf, "SPAWN begin resref=%.16s tag=%.32s area=%s at %.1f,%.1f,%.1f", row.resref, row.tag, a, row.x, row.y, row.z);
        EmitLine(env, budget, buf);
        env.flush();  // durable only when the DLL flushes (k1-spawn-probe 0.3.3: no-op, KpLog flush thread; up to ~500 ms of lines can be lost in a crash)
        const uint32_t oid = env.spawn(row.resref, row.tag, row.x, row.y, row.z, row.facing);
        if (oid) snprintf(buf, sizeof buf, "SPAWN ok resref=%.16s id=%08x", row.resref, static_cast<unsigned>(oid));
        else snprintf(buf, sizeof buf, "SPAWN fail resref=%.16s refused by npc-world (reason in npc-world_log.txt)", row.resref);
        EmitLine(env, budget, buf);
    }
}

}  // namespace k1spawn
