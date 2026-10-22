#include "HolsterQuat.h"
#include "HolsterReach.h"
#include <cstdio>
using namespace hs;
int main(){ Q q = reachClampRot(Q{-0.5f,0.5f,0.5f,0.5f}, 180.f, nullptr); // 240 deg -> clamp
  std::printf("clamp180 w=%.9g x=%.6f\n", q.w, q.x);
  Q hint{0.0871557f, 0.996f,0,0}; Q r = reachClampRot(qaxis(std::array<float,3>{1,0,0}.data(),190.f), 180.f, &hint);
  std::printf("hinted 190 -> w=%.9g x=%.6f\n", r.w, r.x);
  Q b = blendQ(Q{1,0,0,0}, r, 1.f); std::printf("blend w=%.9g x=%.6f\n", b.w, b.x); }
