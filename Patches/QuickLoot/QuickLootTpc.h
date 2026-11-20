// QuickLootTpc.h -- pure TPC reader for the bezel composer (doc 52 QL15, option A: the bezels are built in the DLL from the
// installed game's own textures, so no derived art ships). Header parse plus mip 0 decode of DXT5 (the datapad source) to
// top-down RGBA. No Win32, no engine calls: quick_loot_tpc_test.cpp runs it on the host.
//
// Layout (pykotor io_tpc.py, the reader scripts/character_page_mockup.py texture() uses):
//   u32 dataSize @0 (0 = uncompressed), f32 alphaTest @4, u16 width @8, u16 height @0xA,
//   u8 pixel type @0xC (compressed: 2 = DXT1, 4 = DXT5; uncompressed: 1 grey, 2 RGB, 4 RGBA, 12 BGRA), u8 mip count @0xD.
//   Pixel data starts at 0x80, mip 0 first. For DXT the dataSize is the mip 0 size (w*h for DXT5, w*h/2 for DXT1).
// Orientation: the blocks decode as a DDS would (top-down), then the whole image is flipped vertically. The generator does the
// same: pykotor's decode is transposed FLIP_TOP_BOTTOM in texture().
// Not decoded yet: DXT1, uncompressed (RGB/BGRA are the 2048 px BIF sources; a later item reads them).
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace qltpc {

constexpr size_t kHeaderBytes = 0x80;
constexpr unsigned kMaxDim = 0x8000;        // pykotor MAX_DIMENSIONS: >= this is rejected

enum Enc { EncInvalid = 0, EncDxt1, EncDxt5, EncGrey, EncRgb, EncRgba, EncBgra };

struct Header {
    uint32_t dataSize = 0;
    float alphaTest = 0.0f;
    unsigned width = 0, height = 0, mips = 0;
    Enc enc = EncInvalid;
};

inline uint16_t rd16(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }
inline uint32_t rd32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

// Parses the 128-byte header. False when the buffer is too short, the size is out of range, or the format is not one of the
// layouts above. Compressed cube maps (height / width == 6, the pykotor rule) are rejected. Animated textures are NOT detected:
// their flipbook layout lives in the TXI sidecar, which this reader does not open.
inline bool parseHeader(const uint8_t* d, size_t n, Header* out) {
    if (!d || !out || n < kHeaderBytes) return false;
    Header h;
    h.dataSize = rd32(d);
    memcpy(&h.alphaTest, d + 4, 4);
    h.width = rd16(d + 8);
    h.height = rd16(d + 10);
    const unsigned type = d[12];
    h.mips = d[13];
    if (h.width == 0 || h.height == 0 || h.width >= kMaxDim || h.height >= kMaxDim) return false;
    const bool compressed = h.dataSize != 0;
    if (compressed) {
        if (h.height / h.width == 6) return false;                    // cube map
        if (type == 2) {
            h.enc = EncDxt1;
            if (h.dataSize != (h.width * h.height) / 2) return false;
        } else if (type == 4) {
            h.enc = EncDxt5;
            if (h.dataSize != h.width * h.height) return false;
        } else {
            return false;
        }
    } else {
        switch (type) {
            case 1: h.enc = EncGrey; break;
            case 2: h.enc = EncRgb; break;
            case 4: h.enc = EncRgba; break;
            case 12: h.enc = EncBgra; break;
            default: return false;
        }
    }
    *out = h;
    return true;
}

// RGB565 -> 8-bit RGB by bit replication (the D3D expansion).
inline void expand565(uint16_t c, uint8_t* rgb) {
    const unsigned r5 = (c >> 11) & 31u, g6 = (c >> 5) & 63u, b5 = c & 31u;
    rgb[0] = static_cast<uint8_t>((r5 << 3) | (r5 >> 2));
    rgb[1] = static_cast<uint8_t>((g6 << 2) | (g6 >> 4));
    rgb[2] = static_cast<uint8_t>((b5 << 3) | (b5 >> 2));
}

// One DXT5 block (16 bytes) -> 16 RGBA pixels, row-major, top-down inside the block. Alpha: two endpoints then 16 3-bit
// indices (48 bits, little-endian). Colour: two RGB565 endpoints then 16 2-bit indices; DXT5 colour is always the 4-colour
// palette, never the 3-colour one (that is DXT1 only).
inline void decodeDxt5Block(const uint8_t* b, uint8_t* rgba) {
    const unsigned a0 = b[0], a1 = b[1];
    unsigned apal[8];
    apal[0] = a0;
    apal[1] = a1;
    if (a0 > a1) {
        for (unsigned i = 2; i < 8; ++i) apal[i] = ((8 - i) * a0 + (i - 1) * a1) / 7;
    } else {
        for (unsigned i = 2; i < 6; ++i) apal[i] = ((6 - i) * a0 + (i - 1) * a1) / 5;
        apal[6] = 0;
        apal[7] = 255;
    }
    uint64_t abits = 0;
    for (unsigned i = 0; i < 6; ++i) abits |= static_cast<uint64_t>(b[2 + i]) << (8 * i);

    uint8_t cpal[4][3];
    expand565(rd16(b + 8), cpal[0]);
    expand565(rd16(b + 10), cpal[1]);
    for (unsigned ch = 0; ch < 3; ++ch) {
        cpal[2][ch] = static_cast<uint8_t>((2 * cpal[0][ch] + cpal[1][ch]) / 3);
        cpal[3][ch] = static_cast<uint8_t>((cpal[0][ch] + 2 * cpal[1][ch]) / 3);
    }
    const uint32_t cbits = rd32(b + 12);
    for (unsigned k = 0; k < 16; ++k) {
        const unsigned ai = static_cast<unsigned>((abits >> (3 * k)) & 7u);
        const unsigned ci = (cbits >> (2 * k)) & 3u;
        uint8_t* p = rgba + 4 * k;
        p[0] = cpal[ci][0];
        p[1] = cpal[ci][1];
        p[2] = cpal[ci][2];
        p[3] = static_cast<uint8_t>(apal[ai]);
    }
}

// Mip 0 of a DXT5 TPC -> rgba, top row first. The caller passes the buffer capacity (rgbaCap, bytes) and the header may be hand
// built, so every check is repeated here: enc DXT5; width and height in 1..kMaxDim-1 and multiples of 4; rgbaCap >= w*h*4;
// n >= 0x80 + w*h block bytes (one byte per pixel for DXT5). All checks run before the first write, so on false rgba is untouched.
inline bool decodeDxt5Mip0(const uint8_t* d, size_t n, const Header& h, uint8_t* rgba, size_t rgbaCap) {
    if (!d || !rgba || h.enc != EncDxt5) return false;
    if (h.width == 0 || h.height == 0 || h.width >= kMaxDim || h.height >= kMaxDim) return false;
    if ((h.width % 4) != 0 || (h.height % 4) != 0) return false;
    const uint64_t pixels = static_cast<uint64_t>(h.width) * h.height;
    if (static_cast<uint64_t>(rgbaCap) < pixels * 4) return false;
    if (static_cast<uint64_t>(n) < kHeaderBytes + pixels) return false;
    const size_t bw = h.width / 4, bh = h.height / 4;
    const uint8_t* blocks = d + kHeaderBytes;
    uint8_t tile[64];
    for (size_t by = 0; by < bh; ++by) {
        for (size_t bx = 0; bx < bw; ++bx) {
            decodeDxt5Block(blocks + (by * bw + bx) * 16, tile);
            for (unsigned py = 0; py < 4; ++py) {
                const size_t y = by * 4 + py;                  // DDS-style row
                const size_t dy = h.height - 1 - y;            // flipped: output row
                for (unsigned px = 0; px < 4; ++px) {
                    memcpy(rgba + (dy * h.width + bx * 4 + px) * 4, tile + (py * 4 + px) * 4, 4);
                }
            }
        }
    }
    return true;
}

}  // namespace qltpc
