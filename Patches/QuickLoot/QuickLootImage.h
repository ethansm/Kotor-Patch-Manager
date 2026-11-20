// QuickLootImage.h -- the RGBA image the bezel source loader and the bezel composer pass around (doc 52 QL15). Top row first,
// 4 bytes per pixel (R, G, B, A). Plain data: no Win32, no engine calls.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <vector>

namespace qlimg {

struct Image {
    unsigned w = 0, h = 0;
    std::vector<uint8_t> px;   // w * h * 4 bytes, RGBA, top row first
};

}  // namespace qlimg
