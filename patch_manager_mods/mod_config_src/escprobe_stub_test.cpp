// escprobe_stub_test.cpp -- host test for EscProbeStub.h: stubs fit their stride and never overlap (the 0.3.6-dev crash was a 40-byte stride for a 48-byte stub).
#include "EscProbeStub.h"
#include "check.h"
#include <vector>

static uint32_t rd32(const uint8_t* p) { uint32_t v; memcpy(&v, p, 4); return v; }

static void test_stub_fits_stride() {
    uint8_t buf[escprobe::kStubLen + 16];
    memset(buf, 0xCC, sizeof buf);
    const int n = escprobe::emitStub(buf, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000);
    KT_CHECK_EQ(n, escprobe::kStubLen);
    KT_CHECK(n <= escprobe::kStride);
    for (int i = n; i < (int)sizeof buf; ++i) KT_CHECK_EQ(buf[i], 0xCC);   // nothing written past the stub
}

static void test_many_stubs_do_not_overlap() {
    const int slots = 27;
    std::vector<uint8_t> cave(escprobe::caveBytes(slots), 0xCC);
    for (int s = 0; s < slots; ++s) {
        const uint32_t base = 0x100000u + s * 0x40u;
        escprobe::emitStub(&cave[s * escprobe::kStride], base + 0, base + 4, base + 8, base + 12, base + 16, base + 20);
    }
    for (int s = 0; s < slots; ++s) {
        const uint8_t* c = &cave[s * escprobe::kStride];
        const uint32_t base = 0x100000u + s * 0x40u;
        KT_CHECK(c[0] == 0xFF && c[1] == 0x05 && rd32(c + 2) == base + 0);             // inc [cnt]
        KT_CHECK_EQ(c[escprobe::kStubLen - 6], 0xFF);                                     // jmp [orig] intact at the end of every stub
        KT_CHECK_EQ(c[escprobe::kStubLen - 5], 0x25);
        KT_CHECK_EQ(rd32(c + escprobe::kStubLen - 4), base + 20);
        // every embedded address of the stub is the slot's own (an overlap by the next stub would corrupt one of them)
        KT_CHECK_EQ(rd32(c + 8), base + 4);                                               // mov eax,[seq]
        KT_CHECK_EQ(rd32(c + 14), base + 4);                                              // mov [seq],eax
        KT_CHECK_EQ(rd32(c + 19), base + 8);                                              // mov [slot seq],eax
        KT_CHECK_EQ(rd32(c + 28), base + 12);                                             // mov [a1],eax
        KT_CHECK_EQ(rd32(c + 37), base + 16);                                             // mov [a2],eax
    }
    KT_CHECK_EQ(escprobe::caveBytes(slots), slots * escprobe::kStubLen);                  // the cave holds the LAST stub whole (the 0.3.6 cave was 8 bytes short)
}

int main() {
    test_stub_fits_stride();
    test_many_stubs_do_not_overlap();
    return kt::Report("escprobe_stub_test");
}
