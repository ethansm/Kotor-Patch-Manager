// Host tests for the three-piece REACH step line (HolsterReachLog.h): every piece <= 200 bytes in the worst case (the core cuts at 232), the pieces join back to the old
// single line byte for byte, prefixes and the shared seq. Build/run: python3 scripts/run_tests.py holster_reachlog
#include "HolsterReachLog.h"
#include "check.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdint.h>

using namespace hs;

static void fill3(float* a, float x, float y, float z) { a[0] = x; a[1] = y; a[2] = z; }

static ReachLogVals typical() {
    ReachLogVals v;
    memset(&v, 0, sizeof v);
    v.owner = 0x7f001234u; v.w = 0.8765f; v.reach = 0.6123f;
    fill3(v.S, 0.1234f, -0.5f, 1.25f); fill3(v.E, 0.2f, -0.33333f, 1.1f); fill3(v.H, 0.3f, 0.4f, 0.95f);
    fill3(v.T, 0.31f, 0.41f, 0.9f); fill3(v.Enew, -0.2f, 0.1f, 1.05f); fill3(v.lt, 0.01f, 0.02f, 0.03f);
    v.side = 'L'; v.cat = 7; v.u = 0.55f; v.err = 0.0123f; v.persist = 1; v.wrote = 7;
    v.wr = 33.4f; v.hg = 1; v.f = 0.77f; v.dzA = -0.12f; v.fy = 0.2f; v.fx = -0.3f; v.g = 87.6f; v.cap = 120.f; v.hold = 3;
    v.topo = "hinge"; v.bk = 1; v.tw = -12.5f; v.sw = 33.4f; v.ex = 0.4f; v.of = 0.66f; v.vo = 0.045f; v.pb = 4.5f;
    return v;
}

// The ORIGINAL single format string of Holster.cpp (before the split) with the same argument order: the oracle.
static std::string oldLine(const ReachLogVals& v) {
    char b[1024];
    snprintf(b, sizeof b, "REACH owner=%x w=%.2f reach=%.2f S=(%.3f,%.3f,%.3f) E=(%.3f,%.3f,%.3f) H=(%.3f,%.3f,%.3f) T=(%.3f,%.3f,%.3f) Enew=(%.3f,%.3f,%.3f) side=%c cat=%d u=%.2f lt=(%.3f,%.3f,%.3f) err=%.3f persist=%d wrote=%d wr=%.0f hg=%d f=%.2f dzA=%.2f fy=%.2f fx=%.2f g=%.0f cap=%.0f hold=%u topo=%s bk=%d tw=%.0f sw=%.0f ex=%.2f of=%.2f vo=%.3f pb=%.1f",
             static_cast<unsigned>(v.owner), static_cast<double>(v.w), static_cast<double>(v.reach), static_cast<double>(v.S[0]), static_cast<double>(v.S[1]),
             static_cast<double>(v.S[2]), static_cast<double>(v.E[0]), static_cast<double>(v.E[1]), static_cast<double>(v.E[2]), static_cast<double>(v.H[0]),
             static_cast<double>(v.H[1]), static_cast<double>(v.H[2]), static_cast<double>(v.T[0]), static_cast<double>(v.T[1]), static_cast<double>(v.T[2]),
             static_cast<double>(v.Enew[0]), static_cast<double>(v.Enew[1]), static_cast<double>(v.Enew[2]), v.side, v.cat, static_cast<double>(v.u),
             static_cast<double>(v.lt[0]), static_cast<double>(v.lt[1]), static_cast<double>(v.lt[2]),
             static_cast<double>(v.err), v.persist, v.wrote, static_cast<double>(v.wr), v.hg, static_cast<double>(v.f), static_cast<double>(v.dzA), static_cast<double>(v.fy),
             static_cast<double>(v.fx), static_cast<double>(v.g), static_cast<double>(v.cap), static_cast<unsigned>(v.hold), v.topo, v.bk,
             static_cast<double>(v.tw), static_cast<double>(v.sw), static_cast<double>(v.ex), static_cast<double>(v.of), static_cast<double>(v.vo), static_cast<double>(v.pb));
    return b;
}

struct Pieces { char p[3][256]; };
static Pieces make(const ReachLogVals& v, uint32_t seq) {
    Pieces r;
    reachLogPiece1(r.p[0], sizeof r.p[0], v, seq);
    reachLogPiece2(r.p[1], sizeof r.p[1], v, seq);
    reachLogPiece3(r.p[2], sizeof r.p[2], v, seq);
    return r;
}

// The checker's joining rule.
static bool join(const Pieces& pc, std::string& out) {
    std::string p1 = pc.p[0];
    const size_t hash = p1.rfind(" #");
    if (hash == std::string::npos) return false;
    out = p1.substr(0, hash);
    for (int i = 1; i < 3; ++i) {
        const std::string s = pc.p[i];
        if (s.compare(0, 8, "REACH+ #") != 0) return false;
        const size_t o = s.find(" owner=");
        if (o == std::string::npos) return false;
        const size_t sp = s.find(' ', o + 7);
        if (sp == std::string::npos) return false;
        out += " " + s.substr(sp + 1);
    }
    return true;
}

static uint32_t seqOf(const char* s) {   // the number after the last/first '#'
    const char* h = strchr(s, '#');
    return h ? static_cast<uint32_t>(strtoul(h + 1, nullptr, 10)) : 0u;
}

// (a) worst case: every piece <= 200 bytes.
static void testWorstLengths() {
    const char* topos[4] = { "hinge", "over", "thigh", "hip" };   // the strings reachTopoName can return
    size_t maxLen = 0;
    bool ok = true;
    for (int t = 0; t < 4; ++t) {
        ReachLogVals v = typical();
        v.owner = 0xffffffffu;
        fill3(v.S, -999.999f, -999.999f, -999.999f); fill3(v.E, -999.999f, -999.999f, -999.999f); fill3(v.H, -999.999f, -999.999f, -999.999f);
        fill3(v.T, -999.999f, -999.999f, -999.999f); fill3(v.Enew, -999.999f, -999.999f, -999.999f); fill3(v.lt, -999.999f, -999.999f, -999.999f);
        v.w = v.reach = v.u = v.f = v.dzA = v.fy = v.fx = v.ex = v.of = -999.99f; v.err = -999.999f; v.vo = -999.999f; v.pb = -999.9f;
        v.wr = v.g = v.cap = v.tw = v.sw = -999.f;
        v.cat = -999; v.persist = 1; v.wrote = 7; v.hg = 1; v.bk = 1; v.hold = 255; v.topo = topos[t];
        const Pieces pc = make(v, 4294967295u);
        for (int i = 0; i < 3; ++i) { const size_t n = strlen(pc.p[i]); if (n > maxLen) maxLen = n; if (n > 200) ok = false; }
    }
    printf("worst piece length %zu\n", maxLen);
    KT_CHECK(ok);
    KT_CHECK(maxLen > 0);
}

// (b) typical values: the join equals the old single line byte for byte.
static void testJoinEqualsOld() {
    const char* topos[4] = { "hinge", "over", "thigh", "hip" };
    bool ok = true;
    for (int t = 0; t < 4; ++t) {
        ReachLogVals v = typical();
        v.topo = topos[t];
        v.side = (t & 1) ? 'R' : 'L';
        v.cat = t * 3 - 1;
        std::string joined;
        const Pieces pc = make(v, 1000u + static_cast<uint32_t>(t));
        if (!join(pc, joined) || joined != oldLine(v)) ok = false;
    }
    KT_CHECK(ok);
    // a worst-case line joins too
    ReachLogVals w = typical();
    w.owner = 0xffffffffu; fill3(w.S, -999.999f, 999.999f, -0.0005f); w.cat = 123; w.pb = -999.9f; w.topo = "thigh";
    std::string joined;
    const Pieces pw = make(w, 4294967295u);
    KT_CHECK(join(pw, joined));
    KT_CHECK(joined == oldLine(w));
    KT_CHECK(oldLine(w).size() > 232);   // the old line really was over the core cut
}

// (c) prefixes and (d) the shared seq.
static void testPrefixAndSeq() {
    const ReachLogVals v = typical();
    const uint32_t seqs[3] = { 1u, 77u, 4294967295u };
    bool okPre = true, okSeq = true, okOwner = true;
    for (int k = 0; k < 3; ++k) {
        const Pieces pc = make(v, seqs[k]);
        if (strncmp(pc.p[0], "REACH owner=", 12) != 0) okPre = false;
        if (strncmp(pc.p[1], "REACH+ #", 8) != 0 || strncmp(pc.p[2], "REACH+ #", 8) != 0) okPre = false;
        for (int i = 0; i < 3; ++i) if (seqOf(pc.p[i]) != seqs[k]) okSeq = false;
        if (!strstr(pc.p[1], " owner=7f001234 T=(") || !strstr(pc.p[2], " owner=7f001234 wr=")) okOwner = false;
        // piece 1 ends with " #<seq>"
        char tail[24];
        snprintf(tail, sizeof tail, " #%u", static_cast<unsigned>(seqs[k]));
        const size_t n = strlen(pc.p[0]), tl = strlen(tail);
        if (n < tl || strcmp(pc.p[0] + n - tl, tail) != 0) okPre = false;
    }
    KT_CHECK(okPre);
    KT_CHECK(okSeq);
    KT_CHECK(okOwner);
    // a null topo name must not crash
    ReachLogVals nv = typical();
    nv.topo = nullptr;
    char b[256];
    KT_CHECK(reachLogPiece3(b, sizeof b, nv, 1u) > 0);
}

// (e) HW43 PREVIEWHAND: exact prefix, worst-case length <= 200, open line, hd<0 prints -1, NaN safe.
static void testPreviewHand() {
    PreviewHandVals v;
    memset(&v, 0, sizeof v);
    v.gob = 0x1a2b3c4du; v.model = "w_BlstrRfl_001"; v.hand = 0; v.slideF = 1.f; v.atHand = 1; v.carryOk = 0; v.reachW = 0.5f; v.w = 1.f; v.hd = 0.0123f;
    v.dirty = 1; v.solved = 3; v.prevInit = 1; v.lockGob = 1; v.sdir = 1; v.t0age = 1234;
    char b[512];
    previewHandLine(b, sizeof b, v);
    KT_CHECK(strcmp(b, "PREVIEWHAND gob=1a2b3c4d model=w_BlstrRfl_001 hand=0 slideF=1.00 atHand=1 carryOk=0 reachW=0.50 w=1.00 hd=0.012 dirty=1 solved=3 prevInit=1 lockGob=1 sdir=1 t0age=1234") == 0);
    PreviewHandVals w = v;
    w.gob = 0xffffffffu; w.model = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; w.hand = -1; w.slideF = -1e30f; w.atHand = 1; w.carryOk = 1; w.reachW = 1e30f; w.w = -1e30f; w.hd = 1e30f;
    w.dirty = -1; w.solved = 4294967295u; w.prevInit = 1; w.lockGob = 1; w.sdir = -1; w.t0age = -2000000000;
    const int n = previewHandLine(b, sizeof b, w);
    KT_CHECK(n > 0 && n <= 200);
    KT_CHECK(strncmp(b, "PREVIEWHAND gob=ffffffff model=ABCDEFGHIJKLMNOPQRST hand=", 56) == 0);
    KT_CHECK(strstr(b, "t0age=-99999") != nullptr);
    char c[256];
    KT_CHECK(previewHandLine(c, sizeof c, w) <= 200);
    w.hd = -1.f; w.slideF = NAN;
    previewHandLine(b, sizeof b, w);
    KT_CHECK(strstr(b, " hd=-1.000 ") != nullptr && strstr(b, "slideF=0.00") != nullptr);
    w.model = nullptr;
    KT_CHECK(previewHandLine(b, sizeof b, w) > 0);
    const int m = previewHandOpenLine(b, sizeof b, 0xdeadbeefu, "w_BlstrRfl_001", 1, 12);
    KT_CHECK(m > 0 && m <= 200);
    KT_CHECK(strcmp(b, "PREVIEWHAND open gob=deadbeef model=w_BlstrRfl_001 lockGob=1 reachEntries=12") == 0);
}

int main() {
    testWorstLengths();
    testJoinEqualsOld();
    testPrefixAndSeq();
    testPreviewHand();
    return kt::Report("holster_reachlog");
}
