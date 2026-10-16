// DiversitySaltSource.h -- where npc-diversity gets its per-visit salt (NB1, doc 40 section 11/12). Host-tested; the dispatcher only
// owns the k2nw::Client. The salt is npc-world's VisitSalt(): chosen at AREA_LOAD, constant for the whole visit, so every
// SetAppearance of one visit sees the same value and a new area entry gives a new one.
#pragma once
#include <stdint.h>
#include "DiversityLogic.h"

namespace NpcDiversity {

// 0 unless visit_variety is on AND the client is connected to a provider (an inert client returns 0 from visitSalt()).
// poll() retries the provider lookup (one require per 256 calls) because load order between patches is not a contract.
// Never called when the switch is off, so the default path costs nothing.
template <class Client>
inline uint32_t VisitSaltFrom(bool visitVariety, Client& nw) {
    if (!visitVariety) return 0u;
    nw.poll();
    return ActiveVisitSalt(visitVariety, nw.visitSalt());
}

} // namespace NpcDiversity
