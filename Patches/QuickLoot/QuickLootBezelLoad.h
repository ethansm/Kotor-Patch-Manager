// QuickLootBezelLoad.h -- reads the six panel sources for the bezel composer (doc 52 QL15, option A: the bezels are built in the DLL
// from the installed game's own textures). Order, as the engine and the generator's texture() use it: override/<resref>.tpc, then
// override/<resref>.tga, then the texture pack texturepacks/swpc_tex_gui.erf. The paths are relative: the game runs from its own
// folder, as quick_loot.ini already assumes. The pack is never read whole (132 MB): the header and tables first (qlerf::tablesEnd),
// then the one resource (qlerf::locate). stdio only, so it is host-testable with files in the working directory.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <utility>
#include <vector>
#include "QuickLootErf.h"
#include "QuickLootImage.h"
#include "QuickLootTga.h"
#include "QuickLootTpc.h"

namespace qlload {

constexpr uint16_t kTpcType = 3007;                  // ResType TPC
constexpr const char* kPackPath = "texturepacks/swpc_tex_gui.erf";
constexpr const char* kOverrideDir = "override";
constexpr size_t kMaxFileBytes = size_t{64} << 20;   // a longer file is refused; the panels are 1 to 4 MB
constexpr unsigned kMaxPanelDim = 2048;              // a larger panel is refused before its RGBA buffer is allocated: the installed upscales are 2048, and six of them stay near 96 MB in a 32-bit process

enum class From : uint8_t { None, OverrideTpc, OverrideTga, Pack };

// Whole small file into out. False when missing, empty, too big, or short on read.
inline bool readWhole(const char* path, std::vector<uint8_t>* out) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    bool ok = false;
    do {
        if (fseek(f, 0, SEEK_END) != 0) break;
        const long end = ftell(f);
        if (end <= 0 || static_cast<size_t>(end) > kMaxFileBytes) break;
        if (fseek(f, 0, SEEK_SET) != 0) break;
        out->assign(static_cast<size_t>(end), 0);
        if (fread(out->data(), 1, out->size(), f) != out->size()) break;
        ok = true;
    } while (false);
    fclose(f);
    return ok;
}

// Copies one resource of the ERF at path into out. The rest of the pack is never read.
inline bool readPackResource(const char* path, const char* resref, uint16_t type, std::vector<uint8_t>* out) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    bool ok = false;
    do {
        uint8_t head[qlerf::kHeaderBytes];
        if (fread(head, 1, sizeof head, f) != sizeof head) break;
        uint64_t tables = 0;
        if (!qlerf::tablesEnd(head, sizeof head, &tables)) break;
        if (fseek(f, 0, SEEK_END) != 0) break;
        const long end = ftell(f);
        if (end < 0) break;
        if (tables > static_cast<uint64_t>(end)) break;   // the tables run past the file: refuse before allocating them
        std::vector<uint8_t> tab(static_cast<size_t>(tables), 0);
        if (fseek(f, 0, SEEK_SET) != 0) break;
        if (fread(tab.data(), 1, tab.size(), f) != tab.size()) break;
        qlerf::Locator loc;
        if (!qlerf::locate(tab.data(), tab.size(), static_cast<uint64_t>(end), resref, type, &loc)) break;
        if (loc.size > kMaxFileBytes) break;
        out->assign(static_cast<size_t>(loc.size), 0);
        if (fseek(f, static_cast<long>(loc.offset), SEEK_SET) != 0) break;
        if (fread(out->data(), 1, out->size(), f) != out->size()) break;
        ok = true;
    } while (false);
    fclose(f);
    return ok;
}

// TPC bytes to RGBA, mip 0. DXT5 only (every vanilla pnl_* panel is DXT5). False for any other encoding or a refused header.
inline bool imageFromTpc(const uint8_t* d, size_t n, qlimg::Image* out) {
    qltpc::Header h;
    if (!qltpc::parseHeader(d, n, &h) || h.enc != qltpc::EncDxt5) return false;
    if (h.width > kMaxPanelDim || h.height > kMaxPanelDim) return false;
    qlimg::Image img;
    img.w = h.width;
    img.h = h.height;
    img.px.assign(size_t{h.width} * h.height * 4, 0);
    if (!qltpc::decodeDxt5Mip0(d, n, h, img.px.data(), img.px.size())) return false;
    *out = std::move(img);
    return true;
}

// Uncompressed 32-bit TGA bytes to RGBA, top row first. False for any other format or a refused file.
inline bool imageFromTga(const uint8_t* d, size_t n, qlimg::Image* out) {
    if (!d || n < 18) return false;
    const unsigned w = d[12] | (d[13] << 8), h = d[14] | (d[15] << 8);
    if (w == 0 || h == 0 || w > kMaxPanelDim || h > kMaxPanelDim) return false;
    qlimg::Image img;
    img.w = w;
    img.h = h;
    img.px.assign(size_t{w} * h * 4, 0);
    unsigned gw = 0, gh = 0;
    if (!qltga::decodeTga32(d, n, img.px.data(), img.px.size(), &gw, &gh) || gw != w || gh != h) return false;
    *out = std::move(img);
    return true;
}

// One panel by resref, in engine order. overrideDir and packPath are paths as the process sees them. The return says which source
// served it (None: the caller composes nothing and the bezel stays off).
inline From loadPanel(const char* overrideDir, const char* packPath, const char* resref, qlimg::Image* out) {
    if (!overrideDir || !packPath || !resref || !out) return From::None;
    char path[512];
    std::vector<uint8_t> bytes;
    snprintf(path, sizeof path, "%s/%s.tpc", overrideDir, resref);
    if (readWhole(path, &bytes) && imageFromTpc(bytes.data(), bytes.size(), out)) return From::OverrideTpc;
    snprintf(path, sizeof path, "%s/%s.tga", overrideDir, resref);
    if (readWhole(path, &bytes) && imageFromTga(bytes.data(), bytes.size(), out)) return From::OverrideTga;
    if (readPackResource(packPath, resref, kTpcType, &bytes) && imageFromTpc(bytes.data(), bytes.size(), out)) return From::Pack;
    return From::None;
}

}  // namespace qlload
