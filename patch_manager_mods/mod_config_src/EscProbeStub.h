// EscProbeStub.h -- the machine-code stub of the Esc input probe (mod-config 0.3.6-dev), pure byte emission with no engine access, so a host test can
// check the layout. One stub per Options vtable slot, laid out in a code cave at a FIXED stride. Bug fixed in 0.3.7-dev: the stub is 48 bytes but the
// first probe build used a 40-byte stride, so neighbouring stubs overlapped (the next stub's first bytes landed inside this stub's `mov [a2],eax` imm32
// and its `jmp [orig]`) and the game crashed with a page fault in the cave. The stride and the cave size are now derived from kStubLen (a static_assert keeps kStride >= kStubLen).
#pragma once
#include <stdint.h>
#include <string.h>

namespace escprobe {

constexpr int kStubLen = 48;                 // bytes emitted per stub (checked by emitStub's return value and the host test)
constexpr int kStride = 48;                  // distance between stubs in the cave
static_assert(kStride >= kStubLen, "probe stubs must not overlap");
constexpr int caveBytes(int slots) { return slots * kStride; }

// Emits the stub at `c` (room for kStubLen bytes) and returns the bytes written. All operands are absolute 32-bit addresses:
//   inc [cnt]; push eax; mov eax,[seq]; inc eax; mov [seq],eax; mov [slotSeq],eax; mov eax,[esp+8]; mov [a1],eax; mov eax,[esp+12]; mov [a2],eax;
//   pop eax; jmp [orig]
inline int emitStub(uint8_t* c, uint32_t cnt, uint32_t seq, uint32_t slotSeq, uint32_t a1, uint32_t a2, uint32_t orig) {
    uint8_t* q = c;
    auto u32 = [&](uint32_t v) { memcpy(q, &v, 4); q += 4; };
    *q++ = 0xFF; *q++ = 0x05; u32(cnt);                         // inc dword [cnt]
    *q++ = 0x50;                                                // push eax
    *q++ = 0xA1; u32(seq);                                      // mov eax,[seq]
    *q++ = 0x40;                                                // inc eax
    *q++ = 0xA3; u32(seq);                                      // mov [seq],eax
    *q++ = 0xA3; u32(slotSeq);                                  // mov [slot seq],eax
    *q++ = 0x8B; *q++ = 0x44; *q++ = 0x24; *q++ = 0x08;         // mov eax,[esp+8]   first stack arg (after the pushed eax and the return address)
    *q++ = 0xA3; u32(a1);
    *q++ = 0x8B; *q++ = 0x44; *q++ = 0x24; *q++ = 0x0C;         // mov eax,[esp+12]  second stack arg
    *q++ = 0xA3; u32(a2);
    *q++ = 0x58;                                                // pop eax
    *q++ = 0xFF; *q++ = 0x25; u32(orig);                        // jmp dword [orig]
    return static_cast<int>(q - c);
}

}  // namespace escprobe
