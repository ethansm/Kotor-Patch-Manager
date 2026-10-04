// FakeKPatchApi.h - in-process fake of the KPatchApi registry for host tests (doc 24 T1).
//
// Mirrors KPM src/KotorPatcher/src/core/registry.cpp semantics: provide() rejects null/empty name, null iface,
// a duplicate (name, major) and a closed registry with nonzero; require() returns NULL for unknown names,
// another major, an empty name, or after close(). KPatchApi callbacks carry no user pointer, so the fake is
// one process-wide instance; call fakekp::reset() at the start of every test.
//
//   fakekp::reset();                              // empty, open registry; api()->struct_size = sizeof(KPatchApi)
//   fakekp::reset(8);                             // struct_size override (consumer must gate on it)
//   fakekp::provide("k2.portraitslots", 1, &tbl); // inject a provider from the test side
//   fakekp::withLog();                            // provide a fake "kpatch.log" v1 that records Submit() lines
//   consumer.init(fakekp::api(), "buff");         // hand the api to the code under test
//   fakekp::close();                              // require() returns NULL from now on
//   fakekp::state().requireCalls / provideCalls / logLines
#pragma once

#include "kpatch_api.h"
#include <cstring>
#include <string>
#include <vector>

namespace fakekp {

struct Entry { std::string name; uint32_t version; const void* iface; };

struct State {
    std::vector<Entry> entries;
    bool closed = false;
    int provideCalls = 0, requireCalls = 0, rejected = 0;
    std::vector<std::string> logLines;   // "<channel>:<level>:<text>" from the fake kpatch.log Submit
    std::vector<std::string> marks;
    KPatchApi api = {};
};

inline State& state() { static State s; return s; }

inline int32_t KPATCH_CALL provideImpl(const char* name, uint32_t version, const void* iface) {
    State& s = state(); ++s.provideCalls;
    if (s.closed || !name || !*name || !iface) { ++s.rejected; return 1; }
    for (const Entry& e : s.entries)
        if (e.version == version && e.name == name) { ++s.rejected; return 1; }   // first provider wins
    s.entries.push_back({ name, version, iface });
    return 0;
}

inline const void* KPATCH_CALL requireImpl(const char* name, uint32_t version) {
    State& s = state(); ++s.requireCalls;
    if (s.closed || !name || !*name) return nullptr;
    for (const Entry& e : s.entries)
        if (e.version == version && e.name == name) return e.iface;
    return nullptr;
}

inline void reset(uint32_t structSize = sizeof(KPatchApi), uint32_t abiVersion = KPATCH_ABI_VERSION) {
    State& s = state();
    s.entries.clear(); s.closed = false; s.provideCalls = s.requireCalls = s.rejected = 0;
    s.logLines.clear(); s.marks.clear();
    s.api = { structSize, abiVersion, provideImpl, requireImpl };
}

inline const KPatchApi* api() { return &state().api; }
inline int32_t provide(const char* name, uint32_t version, const void* iface) { return provideImpl(name, version, iface); }
inline void close() { state().closed = true; }
inline bool provided(const char* name, uint32_t version) {
    for (const Entry& e : state().entries) if (e.version == version && e.name == name) return true;
    return false;
}

// ---- fake "kpatch.log" v1: Register/Channel hand out small handles, Enabled is always on, Submit records ----
namespace logimpl {
inline int32_t KPATCH_CALL Register(const char* id) { return id && *id ? 1 : -1; }
inline int32_t KPATCH_CALL Channel(int32_t patch, const char* n) { return patch >= 0 && n ? 1 : -1; }
inline int32_t KPATCH_CALL Enabled(int32_t, int32_t) { return 1; }
inline void KPATCH_CALL Submit(int32_t ch, int32_t lvl, const char* line, uint32_t len) {
    state().logLines.push_back(std::to_string(ch) + ":" + std::to_string(lvl) + ":" + std::string(line ? line : "", line ? len : 0));
}
inline void KPATCH_CALL Mark(const char* label) { state().marks.push_back(label ? label : ""); }
inline uint32_t g_frame = 0;
inline void KPATCH_CALL FrameTick(int32_t) { ++g_frame; }
inline uint32_t KPATCH_CALL Frame() { return g_frame; }
inline uint64_t KPATCH_CALL NowUs() { return 1000ull * g_frame; }
inline const KPatchLogApi table = { sizeof(KPatchLogApi), Register, Channel, Enabled, Submit, Mark, FrameTick, Frame, NowUs };
}  // namespace logimpl

inline const KPatchLogApi* withLog() { logimpl::g_frame = 0; provideImpl(KPATCH_LOG_IFACE, KPATCH_LOG_VERSION, &logimpl::table); return &logimpl::table; }

}  // namespace fakekp
