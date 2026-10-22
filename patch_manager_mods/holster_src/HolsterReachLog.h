// HolsterReachLog.h -- the REACH step line of the holstered-weapons mod, split into three pieces (pure, no Win32, host-testable; tests in holster_reachlog_test.cpp).
// The KPM core cuts every log message at 232 bytes and appends "[...]"; the old single REACH line was ~480 bytes, so everything after err= was lost. Each piece stays <= 200 bytes
// in the worst case. Joining rule (what holster_log_check does): take piece 1 minus " #<seq>", then for each continuation append one space + its text after
// "REACH+ #<seq> owner=<hex> "; the result is byte for byte the old single line (the oracle in the test).
//   piece 1: REACH owner=%x w=%.2f reach=%.2f S=(..) E=(..) H=(..) #%u
//   piece 2: REACH+ #%u owner=%x T=(..) Enew=(..) side=%c cat=%d u=%.2f lt=(..) err=%.3f persist=%d wrote=%d
//   piece 3: REACH+ #%u owner=%x wr=%.0f hg=%d f=%.2f dzA=%.2f fy=%.2f fx=%.2f g=%.0f cap=%.0f hold=%u topo=%s bk=%d tw=%.0f sw=%.0f ex=%.2f of=%.2f vo=%.3f pb=%.1f
// Every function returns the snprintf length (>= cap means cut).
#pragma once
#include <stdio.h>
#include <stdint.h>

namespace hs {

struct ReachLogVals {
    uint32_t owner;
    float w, reach;
    float S[3], E[3], H[3], T[3], Enew[3], lt[3];
    char side;
    int cat;
    float u, err;
    int persist, wrote;
    float wr;                // wr=
    int hg;
    float f, dzA, fy, fx, g, cap;
    uint32_t hold;
    const char* topo;
    int bk;
    float tw, sw, ex, of, vo, pb;
};

inline int reachLogPiece1(char* buf, size_t cap, const ReachLogVals& v, uint32_t seq) {
    return snprintf(buf, cap, "REACH owner=%x w=%.2f reach=%.2f S=(%.3f,%.3f,%.3f) E=(%.3f,%.3f,%.3f) H=(%.3f,%.3f,%.3f) #%u",
                    static_cast<unsigned>(v.owner), static_cast<double>(v.w), static_cast<double>(v.reach),
                    static_cast<double>(v.S[0]), static_cast<double>(v.S[1]), static_cast<double>(v.S[2]),
                    static_cast<double>(v.E[0]), static_cast<double>(v.E[1]), static_cast<double>(v.E[2]),
                    static_cast<double>(v.H[0]), static_cast<double>(v.H[1]), static_cast<double>(v.H[2]), static_cast<unsigned>(seq));
}

inline int reachLogPiece2(char* buf, size_t cap, const ReachLogVals& v, uint32_t seq) {
    return snprintf(buf, cap, "REACH+ #%u owner=%x T=(%.3f,%.3f,%.3f) Enew=(%.3f,%.3f,%.3f) side=%c cat=%d u=%.2f lt=(%.3f,%.3f,%.3f) err=%.3f persist=%d wrote=%d",
                    static_cast<unsigned>(seq), static_cast<unsigned>(v.owner),
                    static_cast<double>(v.T[0]), static_cast<double>(v.T[1]), static_cast<double>(v.T[2]),
                    static_cast<double>(v.Enew[0]), static_cast<double>(v.Enew[1]), static_cast<double>(v.Enew[2]),
                    v.side, v.cat, static_cast<double>(v.u),
                    static_cast<double>(v.lt[0]), static_cast<double>(v.lt[1]), static_cast<double>(v.lt[2]),
                    static_cast<double>(v.err), v.persist, v.wrote);
}

inline int reachLogPiece3(char* buf, size_t cap, const ReachLogVals& v, uint32_t seq) {
    return snprintf(buf, cap, "REACH+ #%u owner=%x wr=%.0f hg=%d f=%.2f dzA=%.2f fy=%.2f fx=%.2f g=%.0f cap=%.0f hold=%u topo=%s bk=%d tw=%.0f sw=%.0f ex=%.2f of=%.2f vo=%.3f pb=%.1f",
                    static_cast<unsigned>(seq), static_cast<unsigned>(v.owner),
                    static_cast<double>(v.wr), v.hg, static_cast<double>(v.f), static_cast<double>(v.dzA), static_cast<double>(v.fy), static_cast<double>(v.fx),
                    static_cast<double>(v.g), static_cast<double>(v.cap), static_cast<unsigned>(v.hold), v.topo ? v.topo : "", v.bk,
                    static_cast<double>(v.tw), static_cast<double>(v.sw), static_cast<double>(v.ex), static_cast<double>(v.of), static_cast<double>(v.vo), static_cast<double>(v.pb));
}

}  // namespace hs
