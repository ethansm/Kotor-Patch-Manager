// Reader for one resource out of the game's BioWare KEY/BIF archives (chitin.key + data/*.bif). Shared since CH7-0 (users: action-hotbar, combat-hotbar). Pure C++17, stdio only,
// host-testable (keybif_test.cpp), also builds for the Win32 DLL. Reads only the headers, the tables it needs and the
// one resource, never a whole BIF. Little-endian layouts:
//   KEY: "KEY V1  ", u32 BIFCount @8, u32 KeyCount @12, u32 OffFileTable @16, u32 OffKeyTable @20.
//     file entry (12 B): u32 FileSize, u32 NameOff, u16 NameSize, u16 Drives   (name e.g. "data\2da.bif")
//     key entry (22 B): char ResRef[16] (NUL padded), u16 ResType, u32 ResID    (bif = ResID>>20, res = ResID&0xFFFFF)
//   BIF: "BIFFV1  ", u32 VarCount @8, u32 FixedCount @12, u32 OffVarTable @16.
//     var entry (16 B): u32 ID, u32 Offset, u32 FileSize, u32 ResType           (match ID&0xFFFFF == res index)
#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#ifndef _WIN32
#include <dirent.h>   // host-only case-insensitive path fallback; the Win32/Wine filesystem is already case-insensitive
#endif

namespace hb {

constexpr uint16_t ResType2DA = 2017;
constexpr uint32_t KeyBifMaxResource = 8u * 1024u * 1024u;

namespace keybif_detail {

struct File {
    FILE* f = nullptr;
    uint64_t size = 0;
    ~File() { if (f) fclose(f); }
    bool Open(const char* path) {
        f = fopen(path, "rb");
        if (!f) return false;
        if (fseek(f, 0, SEEK_END) != 0) return false;
        long n = ftell(f);
        if (n < 0) return false;
        size = (uint64_t)n;
        return true;
    }
    // Reads exactly n bytes at off; false when the range leaves the file or the read is short.
    bool ReadAt(uint64_t off, size_t n, void* dst) {
        if (off > size || n > size - off) return false;
        if (fseek(f, (long)off, SEEK_SET) != 0) return false;
        return n == 0 || fread(dst, 1, n, f) == n;
    }
};

inline uint32_t u32(const unsigned char* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
inline uint16_t u16(const unsigned char* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
inline char lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

inline bool ieqResRef(const unsigned char* name16, const char* resref) {
    size_t n = 0;
    while (n < 16 && name16[n]) ++n;
    size_t m = 0;
    while (resref[m]) ++m;
    if (n != m) return false;
    for (size_t i = 0; i < n; ++i) if (lower((char)name16[i]) != lower(resref[i])) return false;
    return true;
}

#ifndef _WIN32
// Resolves `rel` ("data/2da.bif") under `base` one component at a time, matching each ASCII case-insensitively.
inline bool ResolveCI(const std::string& base, const std::string& rel, std::string& result) {
    std::string cur = base.empty() ? "." : base;
    size_t p = 0;
    while (p < rel.size()) {
        size_t e = rel.find('/', p);
        if (e == std::string::npos) e = rel.size();
        std::string comp = rel.substr(p, e - p);
        p = e + 1;
        if (comp.empty()) continue;
        DIR* d = opendir(cur.c_str());
        if (!d) return false;
        std::string hit;
        while (dirent* ent = readdir(d)) {
            std::string n = ent->d_name;
            if (n.size() != comp.size()) continue;
            bool same = true;
            for (size_t i = 0; i < n.size() && same; ++i) same = lower(n[i]) == lower(comp[i]);
            if (same) { hit = n; if (n == comp) break; }
        }
        closedir(d);
        if (hit.empty()) return false;
        cur += "/" + hit;
    }
    result = cur;
    return true;
}
#endif

// Opens the BIF named by the KEY ("data\2da.bif"): as given, then fully lowercased (the Steam dir has data/ and Data/),
// then (non-Windows hosts only) a per-component case-insensitive directory match (real file is data/2DA.bif).
inline bool OpenBif(File& bif, const std::string& gameDir, std::string name) {
    for (char& c : name) if (c == '\\') c = '/';
    std::string base = gameDir;
    if (!base.empty() && base.back() != '/' && base.back() != '\\') base += '/';
    if (bif.Open((base + name).c_str())) return true;
    if (bif.f) { fclose(bif.f); bif.f = nullptr; }
    std::string low = name;
    for (char& c : low) c = lower(c);
    if (low != name) {
        if (bif.Open((base + low).c_str())) return true;
        if (bif.f) { fclose(bif.f); bif.f = nullptr; }
    }
#ifndef _WIN32
    std::string found;
    if (ResolveCI(gameDir, name, found) && bif.Open(found.c_str())) return true;
    if (bif.f) { fclose(bif.f); bif.f = nullptr; }
#endif
    return false;
}

}  // namespace keybif_detail

// Finds `resref`/`resType` in the KEY at keyPath and reads its bytes out of the matching BIF under gameDir.
// On failure returns false with a short `err`; `out` is cleared first and left empty on failure.
inline bool ReadKeyBifResource(const char* keyPath, const char* gameDir, const char* resref, uint16_t resType,
                               std::string& out, std::string& err) {
    using namespace keybif_detail;
    out.clear(); err.clear();
    if (!keyPath || !resref || !*resref) { err = "bad arguments"; return false; }
    File key;
    if (!key.Open(keyPath)) { err = "cannot open key"; return false; }
    unsigned char h[24];
    if (!key.ReadAt(0, sizeof h, h)) { err = "key too short"; return false; }
    if (std::string((const char*)h, 8) != "KEY V1  ") { err = "bad key magic"; return false; }
    const uint32_t bifCount = u32(h + 8), keyCount = u32(h + 12), offFiles = u32(h + 16), offKeys = u32(h + 20);
    if ((uint64_t)offFiles + (uint64_t)bifCount * 12 > key.size) { err = "key file table out of range"; return false; }
    if ((uint64_t)offKeys + (uint64_t)keyCount * 22 > key.size) { err = "key table out of range"; return false; }

    // Scan the key table in chunks for the first (resref, type) match.
    bool found = false;
    uint32_t resId = 0;
    unsigned char buf[22 * 512];
    for (uint32_t i = 0; i < keyCount && !found;) {
        uint32_t n = keyCount - i < 512 ? keyCount - i : 512;
        if (!key.ReadAt((uint64_t)offKeys + (uint64_t)i * 22, (size_t)n * 22, buf)) { err = "key table truncated"; return false; }
        for (uint32_t j = 0; j < n; ++j) {
            const unsigned char* e = buf + (size_t)j * 22;
            if (u16(e + 16) == resType && ieqResRef(e, resref)) { resId = u32(e + 18); found = true; break; }
        }
        i += n;
    }
    if (!found) { err = "resref not found"; return false; }

    const uint32_t bifIdx = resId >> 20, resIdx = resId & 0xFFFFF;
    if (bifIdx >= bifCount) { err = "bif index " + std::to_string(bifIdx) + " out of range"; return false; }
    unsigned char fe[12];
    if (!key.ReadAt((uint64_t)offFiles + (uint64_t)bifIdx * 12, sizeof fe, fe)) { err = "key file table truncated"; return false; }
    const uint32_t nameOff = u32(fe + 4);
    const uint16_t nameSize = u16(fe + 8);
    if (nameSize == 0 || nameSize > 260) { err = "bad bif name size"; return false; }
    std::string name(nameSize, '\0');
    if (!key.ReadAt(nameOff, nameSize, &name[0])) { err = "bif name out of range"; return false; }
    while (!name.empty() && name.back() == '\0') name.pop_back();
    if (name.empty()) { err = "empty bif name"; return false; }

    File bif;
    if (!OpenBif(bif, gameDir ? gameDir : "", name)) {
        for (char& c : name) if (c == '\\') c = '/';
        err = "cannot open bif " + name;
        return false;
    }
    unsigned char bh[20];
    if (!bif.ReadAt(0, sizeof bh, bh)) { err = "bif too short"; return false; }
    if (std::string((const char*)bh, 8) != "BIFFV1  ") { err = "bad bif magic"; return false; }
    const uint32_t varCount = u32(bh + 8), offVars = u32(bh + 16);
    if (resIdx >= varCount) { err = "resource index " + std::to_string(resIdx) + " out of range"; return false; }
    if ((uint64_t)offVars + (uint64_t)varCount * 16 > bif.size) { err = "bif var table out of range"; return false; }

    // Fast path: entry at the resource index (normal layout); fall back to scanning for the matching ID.
    unsigned char ve[16];
    bool have = bif.ReadAt((uint64_t)offVars + (uint64_t)resIdx * 16, sizeof ve, ve) && (u32(ve) & 0xFFFFF) == resIdx;
    for (uint32_t i = 0; !have && i < varCount; ++i) {
        if (!bif.ReadAt((uint64_t)offVars + (uint64_t)i * 16, sizeof ve, ve)) { err = "bif var table truncated"; return false; }
        have = (u32(ve) & 0xFFFFF) == resIdx;
    }
    if (!have) { err = "resource id not in bif"; return false; }
    const uint32_t off = u32(ve + 4), size = u32(ve + 8);
    if (size > KeyBifMaxResource) { err = "resource too large"; return false; }
    if ((uint64_t)off + size > bif.size) { err = "resource out of range"; return false; }
    std::string data(size, '\0');
    if (size && !bif.ReadAt(off, size, &data[0])) { err = "resource read failed"; return false; }
    out.swap(data);
    return true;
}

}  // namespace hb
