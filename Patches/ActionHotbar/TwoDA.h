// Minimal reader for binary KotOR 2DA files ("2DA V2.b"). Pure C++17, host-testable (twoda_test.cpp).
// Layout: "2DA V2.b\n", column names each terminated by '\t' and the list by '\0', uint32 row count, row labels
// each terminated by '\t', uint16 offsets[rows*cols], uint16 data size, data blob (NUL-terminated strings).
#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace hb {

struct TwoDA {
    std::vector<std::string> cols;
    std::vector<std::string> labels;
    std::vector<std::vector<std::string>> rows;
    std::string error;

    bool Load(const char* path) {
        cols.clear(); labels.clear(); rows.clear(); error.clear();
        FILE* f = fopen(path, "rb");
        if (!f) { error = "cannot open"; return false; }
        constexpr size_t Chunk = 64 * 1024;   // heap, not stack: read straight into the string in 64 KiB steps
        std::string d; d.reserve(Chunk);
        for (;;) {
            size_t old = d.size(); d.resize(old + Chunk);
            size_t n = fread(&d[old], 1, Chunk, f);
            d.resize(old + n);
            if (n == 0) break;
        }
        fclose(f);
        return Parse(d);
    }

    // Replaces any previous state; on failure leaves cols/labels/rows empty and `error` set.
    bool Parse(const std::string& d) {
        cols.clear(); labels.clear(); rows.clear(); error.clear();
        if (ParseImpl(d)) return true;
        cols.clear(); labels.clear(); rows.clear();
        return false;
    }

    // Column index by name, ASCII case-insensitive (engine 2DA column names are); first match wins; -1 if none.
    int Col(const char* name) const {
        if (!name) return -1;
        for (size_t i = 0; i < cols.size(); ++i) if (ieq(cols[i], name)) return (int)i;
        return -1;
    }
    std::string Get(int row, const char* col) const {
        int c = Col(col);
        if (c < 0 || row < 0 || row >= (int)rows.size()) return "";
        return rows[row][c];
    }

    // Pure, width-independent check that the uint16 offset table (nrows*ncols cells, 2 bytes each) plus the 2-byte data size fit
    // in the file after offset p. sizeMax is the build's SIZE_MAX (a parameter so the host can test i686 width). Nothing is
    // multiplied: on i686 both nrows*ncols (100000 x 42950 -> 32704) and cells*2 (100000 x 21475 -> 32704) wrap size_t.
    static bool OffsetTableFits(uint64_t nrows, uint64_t ncols, uint64_t p, uint64_t dsize, uint64_t sizeMax) {
        if (ncols != 0 && nrows > sizeMax / ncols) return false;   // cell count itself would wrap size_t
        if (p + 2 > dsize) return false;
        return ncols == 0 || nrows <= ((dsize - p - 2) / 2) / ncols;   // cells <= floor((dsize-p-2)/2), so cells*2 cannot wrap either
    }

private:
    static bool ieq(const std::string& a, const char* b) {
        size_t i = 0;
        for (; i < a.size() && b[i]; ++i) if (lower(a[i]) != lower(b[i])) return false;
        return i == a.size() && !b[i];
    }
    static char lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

    bool ParseImpl(const std::string& d) {
        if (d.size() < 16 || d.compare(0, 8, "2DA V2.b") != 0) { error = "not a binary 2DA V2.b"; return false; }
        size_t p = 9;   // "2DA V2.b\n"
        while (p < d.size() && d[p] != '\0') {
            size_t e = d.find('\t', p);
            if (e == std::string::npos) { error = "bad column header"; return false; }
            cols.push_back(d.substr(p, e - p)); p = e + 1;
        }
        ++p;   // list terminator
        if (p + 4 > d.size()) { error = "truncated"; return false; }
        uint32_t nrows = rd32(d, p); p += 4;
        if (nrows > 100000) { error = "implausible row count"; return false; }
        for (uint32_t r = 0; r < nrows; ++r) {
            size_t e = d.find('\t', p);
            if (e == std::string::npos) { error = "bad row label"; return false; }
            labels.push_back(d.substr(p, e - p)); p = e + 1;
        }
        if (!OffsetTableFits(nrows, cols.size(), p, d.size(), (uint64_t)~(size_t)0)) { error = "truncated offsets"; return false; }
        size_t cells = (size_t)nrows * cols.size();   // exact: bounded by the file size above
        size_t offs = p; p += cells * 2;
        p += 2;   // data size
        for (uint32_t r = 0; r < nrows; ++r) {
            std::vector<std::string> row;
            for (size_t c = 0; c < cols.size(); ++c) {
                size_t o = p + rd16(d, offs + (r * cols.size() + c) * 2);
                if (o >= d.size()) { row.push_back(""); continue; }
                row.push_back(std::string(d.c_str() + o));
            }
            rows.push_back(row);
        }
        return true;
    }

    static uint32_t rd32(const std::string& d, size_t p) { return (uint8_t)d[p] | ((uint8_t)d[p + 1] << 8) | ((uint8_t)d[p + 2] << 16) | ((uint32_t)(uint8_t)d[p + 3] << 24); }
    static uint32_t rd16(const std::string& d, size_t p) { return (uint8_t)d[p] | ((uint8_t)d[p + 1] << 8); }
};

} // namespace hb
