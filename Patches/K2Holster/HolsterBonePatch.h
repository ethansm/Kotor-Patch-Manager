// HolsterBonePatch.h -- P0 flag-file bone experiment (doc 42): repoint the "rhand"/"lhand" attach-node literal of FUN_007637E0.
// Only the 5-byte `push imm32` is rewritten (same opcode, new immediate; no new hook). The original bytes are verified first; an unknown exe is refused.
//   site 0x763AF9: 68 D8 ED 99 00   push 0x99EDD8 ("rhand")
//   site 0x763E2B: 68 D0 ED 99 00   push 0x99EDD0 ("lhand")
#pragma once
#include <stdint.h>
#include <string.h>
#include "../Common/CodePatch.h"

namespace hs {

constexpr uintptr_t kRhandImm = 0x00763AF9u;
constexpr uintptr_t kLhandImm = 0x00763E2Bu;
constexpr uint8_t kRhandOrig[5] = {0x68, 0xD8, 0xED, 0x99, 0x00};
constexpr uint8_t kLhandOrig[5] = {0x68, 0xD0, 0xED, 0x99, 0x00};

// 1 patched | -1 bytes at `at` differ from `orig` (nothing written) | -2 VirtualProtect failed. `seen` (5 bytes) gets the found bytes.
inline int patchNodeLiteral(uintptr_t at, const uint8_t orig[5], uint32_t newPtr, uint8_t seen[5]) {
    uint8_t repl[5] = {0x68, 0, 0, 0, 0};
    memcpy(repl + 1, &newPtr, 4);
    return codepatch::writeChecked(at, orig, repl, 5, seen);
}

}  // namespace hs
