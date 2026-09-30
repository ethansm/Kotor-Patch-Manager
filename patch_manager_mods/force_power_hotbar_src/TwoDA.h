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
        std::string d; char buf[65536]; size_t n;
        while ((n = fread(buf, 1, sizeof buf, f)) > 0) d.append(buf, n);
        fclose(f);
        return Parse(d);
    }

    bool Parse(const std::string& d) {
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
        size_t cells = (size_t)nrows * cols.size();
        if (p + cells * 2 + 2 > d.size()) { error = "truncated offsets"; return false; }
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

    int Col(const char* name) const { for (size_t i = 0; i < cols.size(); ++i) if (cols[i] == name) return (int)i; return -1; }
    std::string Get(int row, const char* col) const {
        int c = Col(col);
        if (c < 0 || row < 0 || row >= (int)rows.size()) return "";
        return rows[row][c];
    }

private:
    static uint32_t rd32(const std::string& d, size_t p) { return (uint8_t)d[p] | ((uint8_t)d[p + 1] << 8) | ((uint8_t)d[p + 2] << 16) | ((uint32_t)(uint8_t)d[p + 3] << 24); }
    static uint32_t rd16(const std::string& d, size_t p) { return (uint8_t)d[p] | ((uint8_t)d[p + 1] << 8); }
};

} // namespace hb
