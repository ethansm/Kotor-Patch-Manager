// Scratch: production chain reachWristDelta(180, hint=prev dqTot) -> reachSplitWrist(80/60, hints) over a physical turn lo..hi about each axis.
// target fixed = identity; cur = conj(rot(axis,deg)) so target*conj(cur) = rot(axis,deg) (continuous, not w-flipped).
#include "HolsterQuat.h"
#include "HolsterReach.h"
#include <cstdio>
#include <vector>
using namespace hs;
int main(int argc, char** argv) {
    const float lo = argc > 1 ? (float)atof(argv[1]) : 120.f, hi = argc > 2 ? (float)atof(argv[2]) : 260.f;
    const float axes[3][3] = { { 0.8f, 0.f, 0.6f }, { 0.6f, 0.48f, 0.64f }, { 0.f, 0.6f, 0.8f } };
    const float z[3] = { 0.f, 0.f, 1.f };
    const int n = (int)((hi - lo) / 0.1f + 0.5f);
    for (int mode = 0; mode < 2; ++mode)   // mode 0: production (wristDelta hint chain). mode 1: sweep style (w>=0 flip, no wristDelta)
    for (int a = 0; a < 3; ++a) {
        Q prevTw{1,0,0,0}, prevComb{1,0,0,0}, hint{1,0,0,0};
        bool have = false;
        float maxIn = 0, maxE = 0, maxE0 = 0, maxEtrue = 0, maxEsplitVsIn = 0, firstBad = -1, firstFlip = -1;
        int bad = 0; float prevInAx = 0;
        for (int i = 0; i <= n; ++i) {
            const float deg = lo + 0.1f * i;
            const Q rot = qaxis(axes[a], deg);
            Q dq;
            if (mode == 0) {
                const Q cur = reachConj(rot);
                dq = reachWristDelta(Q{1,0,0,0}, Q{1,0,0,0}, 0.f, cur, 180.f, 1.f, have ? &hint : nullptr);
                hint = dq;
            } else { dq = rot; if (dq.w < 0) dq = { -dq.w, -dq.x, -dq.y, -dq.z }; }
            const float inAng = reachQAngleDeg(dq);
            if (inAng > maxIn) maxIn = inAng;
            const Q dqn = qnormalize(dq);
            Q tw, sw; bool ov = false;
            reachSplitWrist(dq, z, 80.f, 60.f, have ? &prevTw : nullptr, tw, sw, have ? &prevComb : nullptr, &ov);
            const Q comb = qmul(sw, tw);
            Q raw, dis; qSwingTwist(dqn, z, dis, raw);
            const Q t0 = reachClampRot(raw, 80.f, have ? &prevTw : nullptr);
            const Q s0 = reachClampRot(qmul(dqn, reachConj(t0)), 60.f, nullptr);
            const float E = reachQAngleDeg(qmul(comb, reachConj(dqn)));
            const float E0 = reachQAngleDeg(qmul(qmul(s0, t0), reachConj(dqn)));
            const float Etrue = reachQAngleDeg(qmul(comb, reachConj(rot)));   // vs the true (unclamped) turn
            const float Edelta = reachQAngleDeg(qmul(dqn, reachConj(rot)));   // truncation by reachWristDelta
            if (E > maxE) maxE = E; if (E0 > maxE0) maxE0 = E0; if (Etrue > maxEtrue) maxEtrue = Etrue; if (Edelta > maxEsplitVsIn) maxEsplitVsIn = Edelta;
            if (E > E0 + 1.f) { ++bad; if (firstBad < 0) firstBad = deg; }
            // axis flip of dq (dot of vector parts sign change)
            const float ax = dq.x * axes[a][0] + dq.y * axes[a][1] + dq.z * axes[a][2];
            if (i > 0 && ax * prevInAx < 0 && firstFlip < 0) firstFlip = deg;
            if (std::fabs(ax) > 1e-6f) prevInAx = ax;
            prevTw = tw; prevComb = comb; have = true;
        }
        std::printf("%s axis%d maxInAngle=%.2f maxE=%.2f maxE0=%.2f badSteps=%d firstBad=%.1f firstAxisFlipOfInput=%.1f maxDeltaTruncVsTrue=%.2f maxEvsTrueTurn=%.2f\n",
                    mode ? "SWEEP " : "CHAIN ", a, maxIn, maxE, maxE0, bad, firstBad, firstFlip, maxEsplitVsIn, maxEtrue);
    }
}
