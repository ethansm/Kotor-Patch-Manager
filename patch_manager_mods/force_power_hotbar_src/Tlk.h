// Minimal on-demand reader for KotOR dialog.tlk (V3.0). Pure C++17, host-testable (twoda_test.cpp).
// Header: "TLK V3.0", u32 language, u32 count, u32 stringsOffset. Entries at +20, 40 bytes each:
// u32 flags, char[16] sound, u32 volVar, u32 pitchVar, u32 offset (relative to stringsOffset), u32 size, f32 soundLen.
#pragma once
#include <cstdint>
#include <cstdio>
#include <map>
#include <cstring>
#include <string>

namespace hb {

struct Tlk {
    std::string path;
    uint32_t count = 0, strOff = 0;
    bool ok = false;
    std::string error;
    std::map<uint32_t, std::string> cache;

    bool Open(const char* p) {
        path = p; ok = false; error.clear(); cache.clear();
        FILE* f = fopen(p, "rb");
        if (!f) { error = "cannot open"; return false; }
        unsigned char h[20];
        if (fread(h, 1, 20, f) != 20 || memcmp(h, "TLK V3.0", 8) != 0) { fclose(f); error = "not TLK V3.0"; return false; }
        count = u32(h + 12); strOff = u32(h + 16);
        fclose(f);
        ok = count > 0 && count < 10000000;
        if (!ok) error = "implausible count";
        return ok;
    }

    // ASCII-fied, whitespace-collapsed text of a strref ("" if invalid). Cached.
    std::string Get(uint32_t ref) {
        if (!ok || ref >= count) return "";
        auto it = cache.find(ref); if (it != cache.end()) return it->second;
        std::string out;
        FILE* f = fopen(path.c_str(), "rb");
        if (f) {
            unsigned char e[40];
            if (fseek(f, (long)(20 + (long)ref * 40), SEEK_SET) == 0 && fread(e, 1, 40, f) == 40) {
                uint32_t off = u32(e + 28), size = u32(e + 32);
                if (size < 65536 && fseek(f, (long)(strOff + off), SEEK_SET) == 0) {
                    std::string raw(size, '\0');
                    if (fread(&raw[0], 1, size, f) == size) out = Clean(raw);
                }
            }
            fclose(f);
        }
        cache[ref] = out;
        return out;
    }

private:
    static uint32_t u32(const unsigned char* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
    static std::string Clean(const std::string& raw) {
        std::string o; bool sp = false;
        for (unsigned char c : raw) {
            if (c == 0) break;
            char r = (char)c;
            if (c == '\r' || c == '\n' || c == '\t') r = ' ';
            else if (c == 0x91 || c == 0x92) r = '\'';
            else if (c == 0x93 || c == 0x94) r = '"';
            else if (c == 0x96 || c == 0x97) r = '-';
            else if (c < 0x20 || c > 0x7E) r = '?';
            if (r == ' ') { if (sp || o.empty()) continue; sp = true; } else sp = false;
            o += r;
        }
        while (!o.empty() && o.back() == ' ') o.pop_back();
        return o;
    }
};

} // namespace hb
