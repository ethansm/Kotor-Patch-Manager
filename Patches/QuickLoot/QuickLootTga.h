// QuickLootTga.h -- pure decoder for the override TGA sources (doc 52 QL15): an uncompressed 32-bit true-colour TGA (type 2, 32 bpp,
// no colour map) to top-down RGBA. The origin bit (0x20 of the descriptor) is honoured: bottom-left, the default and what the game's
// override files use, is flipped, so the result matches PIL's Image.open(...).convert("RGBA") that the generator uses. The 18-byte
// header's ID field is skipped; a footer or developer area after the pixels is ignored. RLE, colour-mapped and non-32-bit files are
// refused. No Win32, no engine calls.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace qltga {

constexpr unsigned kMaxDim = 8192;

// Decodes d[0..n) into rgba (rgbaCap bytes) and sets *w and *h. False on any refusal; rgba is written only after every check.
inline bool decodeTga32(const uint8_t* d, size_t n, uint8_t* rgba, size_t rgbaCap, unsigned* w, unsigned* h) {
    if (!d || !rgba || !w || !h || n < 18) return false;
    if (d[1] != 0 || d[2] != 2 || d[16] != 32) return false;             // no colour map, type 2, 32 bpp
    if (d[17] & 0x10) return false;                                      // right-to-left: refused, not mis-drawn
    const unsigned width = d[12] | (d[13] << 8);
    const unsigned height = d[14] | (d[15] << 8);
    if (width == 0 || height == 0 || width > kMaxDim || height > kMaxDim) return false;
    const uint64_t pixels = uint64_t{width} * height;
    const uint64_t off = 18u + d[0];
    if (off + pixels * 4 > n) return false;
    if (static_cast<uint64_t>(rgbaCap) < pixels * 4) return false;
    const bool topLeft = (d[17] & 0x20) != 0;
    const uint8_t* src = d + off;
    for (unsigned row = 0; row < height; ++row) {
        const unsigned dy = topLeft ? row : height - 1 - row;
        const uint8_t* s = src + size_t{row} * width * 4;
        uint8_t* t = rgba + size_t{dy} * width * 4;
        for (unsigned x = 0; x < width; ++x) {
            t[4 * x + 0] = s[4 * x + 2];
            t[4 * x + 1] = s[4 * x + 1];
            t[4 * x + 2] = s[4 * x + 0];
            t[4 * x + 3] = s[4 * x + 3];
        }
    }
    *w = width;
    *h = height;
    return true;
}

}  // namespace qltga
