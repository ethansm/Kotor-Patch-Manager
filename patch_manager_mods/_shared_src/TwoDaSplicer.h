#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace twoda {

struct CellPatch {
    uint32_t row;
    const char* colName;
    const char* value;
};

struct ParsedView {
    uint32_t numCols = 0;
    uint32_t numRows = 0;
    std::vector<std::string> colNames;
    std::vector<std::string> rowLabels;
    const uint8_t* cellOffsetsRaw = nullptr;
    const char* stringPool = nullptr;
    size_t poolLen = 0;
    uint16_t poolHeaderSize = 0;
    char colTerminator = '\n';

    bool parse(const uint8_t* buf, size_t len) {
        clear();
        if (!buf || len < 16) return false;

        // Magic 4 bytes "2DA ", version 4 bytes "V2.b", newline 1 byte '\n' -> 9 bytes total
        if (std::memcmp(buf, "2DA V2.b\n", 9) != 0) {
            return false;
        }

        // Column headers starting at offset 9
        size_t p = 9;
        while (p < len) {
            if (buf[p] == '\n' || buf[p] == '\0') {
                colTerminator = static_cast<char>(buf[p]);
                p++; // skip terminator
                break;
            }
            size_t tab = p;
            while (tab < len && buf[tab] != '\t' && buf[tab] != '\n' && buf[tab] != '\0') {
                tab++;
            }
            if (tab >= len || buf[tab] != '\t') {
                return false;
            }
            colNames.emplace_back(reinterpret_cast<const char*>(buf + p), tab - p);
            p = tab + 1;
        }

        numCols = static_cast<uint32_t>(colNames.size());
        if (numCols == 0) return false;

        // Row count: 4-byte unsigned little-endian integer
        if (p + 4 > len) return false;
        numRows = static_cast<uint32_t>(buf[p]) |
                  (static_cast<uint32_t>(buf[p + 1]) << 8) |
                  (static_cast<uint32_t>(buf[p + 2]) << 16) |
                  (static_cast<uint32_t>(buf[p + 3]) << 24);
        p += 4;

        if (numCols > 0 && numRows > (SIZE_MAX / 2) / numCols) {
            return false;
        }

        // Row labels: numRows tab-separated strings (no trailing terminator)
        rowLabels.reserve(numRows);
        for (uint32_t r = 0; r < numRows; ++r) {
            if (p >= len) return false;
            size_t tab = p;
            while (tab < len && buf[tab] != '\t') {
                tab++;
            }
            if (tab >= len) return false;
            rowLabels.emplace_back(reinterpret_cast<const char*>(buf + p), tab - p);
            p = tab + 1;
        }

        // Cell offset table: numRows * numCols * 2 bytes
        size_t cellCount = static_cast<size_t>(numRows) * numCols;
        size_t cellBytes = cellCount * 2;
        if (p + cellBytes + 2 > len) return false;
        cellOffsetsRaw = buf + p;
        p += cellBytes;

        // String pool header: 2-byte little-endian uint16 poolSize
        poolHeaderSize = static_cast<uint16_t>(static_cast<uint16_t>(buf[p]) |
                                               (static_cast<uint16_t>(buf[p + 1]) << 8));
        p += 2;

        stringPool = reinterpret_cast<const char*>(buf + p);
        poolLen = len - p;
        if (poolLen == 0) return false;

        // Ensure string pool ends with a null terminator
        if (buf[len - 1] != '\0') return false;

        // If pool is not overflow-wrapped (<= 65535), check truncation
        if (poolHeaderSize > 0 && poolLen < poolHeaderSize) {
            return false;
        }

        // Validate that all cell offsets are within poolLen
        for (size_t i = 0; i < cellCount; ++i) {
            uint16_t off = static_cast<uint16_t>(cellOffsetsRaw[i * 2] | (cellOffsetsRaw[i * 2 + 1] << 8));
            if (off >= poolLen) {
                return false;
            }
        }

        return true;
    }

    int findColumn(const char* colName) const {
        if (!colName) return -1;
        for (size_t i = 0; i < colNames.size(); ++i) {
            if (iequals(colNames[i], colName)) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    const char* getCell(uint32_t row, uint32_t col) const {
        if (row >= numRows || col >= numCols || !cellOffsetsRaw || !stringPool) {
            return nullptr;
        }
        size_t idx = static_cast<size_t>(row) * numCols + col;
        uint16_t off = static_cast<uint16_t>(cellOffsetsRaw[idx * 2] | (cellOffsetsRaw[idx * 2 + 1] << 8));
        if (off >= poolLen) return nullptr;
        return stringPool + off;
    }

    uint16_t getCellOffset(uint32_t row, uint32_t col) const {
        if (row >= numRows || col >= numCols || !cellOffsetsRaw) {
            return 0xFFFF;
        }
        size_t idx = static_cast<size_t>(row) * numCols + col;
        return static_cast<uint16_t>(cellOffsetsRaw[idx * 2] | (cellOffsetsRaw[idx * 2 + 1] << 8));
    }

private:
    void clear() {
        numCols = 0;
        numRows = 0;
        colNames.clear();
        rowLabels.clear();
        cellOffsetsRaw = nullptr;
        stringPool = nullptr;
        poolLen = 0;
        poolHeaderSize = 0;
        colTerminator = '\n';
    }

    static bool iequals(const std::string& a, const char* b) {
        if (!b) return false;
        size_t i = 0;
        for (; i < a.size() && b[i]; ++i) {
            char c1 = (a[i] >= 'A' && a[i] <= 'Z') ? static_cast<char>(a[i] + 32) : a[i];
            char c2 = (b[i] >= 'A' && b[i] <= 'Z') ? static_cast<char>(b[i] + 32) : b[i];
            if (c1 != c2) return false;
        }
        return i == a.size() && b[i] == '\0';
    }
};

inline uint32_t spliceRows(const uint8_t* baseBuf, size_t baseLen,
                           const CellPatch* patches, size_t numPatches,
                           uint8_t* outBuf, size_t outCap,
                           int32_t templateRow = -1) {
    if (!baseBuf || baseLen == 0) return 0;
    ParsedView base;
    if (!base.parse(baseBuf, baseLen)) return 0;
    if (base.numCols == 0) return 0;

    uint32_t maxPatchRow = 0;
    bool hasPatchRows = false;
    if (patches && numPatches > 0) {
        for (size_t i = 0; i < numPatches; ++i) {
            if (!patches[i].colName) return 0;
            int c = base.findColumn(patches[i].colName);
            if (c < 0) return 0; // unknown column name
            if (!hasPatchRows || patches[i].row > maxPatchRow) {
                maxPatchRow = patches[i].row;
                hasPatchRows = true;
            }
        }
    } else if (numPatches > 0 && !patches) {
        return 0;
    }

    uint32_t newNumRows = base.numRows;
    if (hasPatchRows && maxPatchRow >= newNumRows) {
        newNumRows = maxPatchRow + 1;
    }

    std::vector<bool> rowHasPatch(newNumRows, false);
    if (patches && numPatches > 0) {
        for (size_t i = 0; i < numPatches; ++i) {
            rowHasPatch[patches[i].row] = true;
        }
    }

    std::vector<std::string> outRowLabels(newNumRows);
    for (uint32_t r = 0; r < newNumRows; ++r) {
        if (r < base.numRows) {
            outRowLabels[r] = base.rowLabels[r];
        } else {
            outRowLabels[r] = std::to_string(r);
        }
    }

    // Grid of string pointers for each cell
    std::vector<const char*> grid(static_cast<size_t>(newNumRows) * base.numCols, "");
    bool validTemplate = (templateRow >= 0 && static_cast<uint32_t>(templateRow) < base.numRows);

    for (uint32_t r = 0; r < newNumRows; ++r) {
        if (r < base.numRows) {
            for (uint32_t c = 0; c < base.numCols; ++c) {
                const char* val = base.getCell(r, c);
                grid[static_cast<size_t>(r) * base.numCols + c] = val ? val : "";
            }
        } else {
            if (rowHasPatch[r] && validTemplate) {
                for (uint32_t c = 0; c < base.numCols; ++c) {
                    const char* val = base.getCell(static_cast<uint32_t>(templateRow), c);
                    grid[static_cast<size_t>(r) * base.numCols + c] = val ? val : "";
                }
            } else {
                for (uint32_t c = 0; c < base.numCols; ++c) {
                    grid[static_cast<size_t>(r) * base.numCols + c] = "";
                }
            }
        }
    }

    // Apply patches
    if (patches && numPatches > 0) {
        for (size_t i = 0; i < numPatches; ++i) {
            uint32_t r = patches[i].row;
            int c = base.findColumn(patches[i].colName);
            const char* val = patches[i].value;
            if (!val || std::strcmp(val, "****") == 0) {
                val = "";
            }
            grid[static_cast<size_t>(r) * base.numCols + static_cast<size_t>(c)] = val;
        }
    }

    // Build string pool with deduplication
    std::vector<char> pool;
    pool.push_back('\0'); // offset 0 is ""
    std::unordered_map<std::string, uint16_t> poolMap;
    poolMap[""] = 0;

    std::vector<uint16_t> cellOffsets(grid.size(), 0);

    for (size_t i = 0; i < grid.size(); ++i) {
        const char* s = grid[i];
        if (!s || std::strcmp(s, "****") == 0) {
            s = "";
        }
        auto it = poolMap.find(s);
        if (it != poolMap.end()) {
            cellOffsets[i] = it->second;
        } else {
            size_t slen = std::strlen(s);
            size_t nextOffset = pool.size();
            if (nextOffset >= 65536) {
                return 0; // string pool offset overflow
            }
            if (nextOffset + slen + 1 > 65535) {
                return 0; // string pool size overflow
            }
            uint16_t off = static_cast<uint16_t>(nextOffset);
            poolMap[s] = off;
            pool.insert(pool.end(), s, s + slen + 1);
            cellOffsets[i] = off;
        }
    }

    // Calculate serialized buffer size
    // 9 bytes: "2DA V2.b\n"
    size_t colSectionLen = 0;
    for (const auto& col : base.colNames) {
        colSectionLen += col.size() + 1; // name + '\t'
    }
    colSectionLen += 1; // colTerminator ('\n' or '\0')

    size_t rowLabelsLen = 0;
    for (uint32_t r = 0; r < newNumRows; ++r) {
        rowLabelsLen += outRowLabels[r].size() + 1; // label + '\t'
    }

    size_t cellOffsetsLen = cellOffsets.size() * 2;
    size_t poolLen = pool.size();

    size_t totalSize = 9 + colSectionLen + 4 + rowLabelsLen + cellOffsetsLen + 2 + poolLen;
    if (totalSize > UINT32_MAX) return 0;
    uint32_t requiredSize = static_cast<uint32_t>(totalSize);

    if (!outBuf) {
        return requiredSize;
    }
    if (outCap < requiredSize) {
        return 0;
    }

    uint8_t* p = outBuf;
    std::memcpy(p, "2DA V2.b\n", 9);
    p += 9;

    char term = (base.colTerminator == '\0') ? '\0' : '\n';
    for (const auto& col : base.colNames) {
        std::memcpy(p, col.data(), col.size());
        p += col.size();
        *p++ = '\t';
    }
    *p++ = static_cast<uint8_t>(term);

    // Row count
    p[0] = static_cast<uint8_t>(newNumRows & 0xFF);
    p[1] = static_cast<uint8_t>((newNumRows >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((newNumRows >> 16) & 0xFF);
    p[3] = static_cast<uint8_t>((newNumRows >> 24) & 0xFF);
    p += 4;

    // Row labels
    for (uint32_t r = 0; r < newNumRows; ++r) {
        const std::string& lbl = outRowLabels[r];
        std::memcpy(p, lbl.data(), lbl.size());
        p += lbl.size();
        *p++ = '\t';
    }

    // Cell offsets
    for (size_t i = 0; i < cellOffsets.size(); ++i) {
        uint16_t off = cellOffsets[i];
        p[0] = static_cast<uint8_t>(off & 0xFF);
        p[1] = static_cast<uint8_t>((off >> 8) & 0xFF);
        p += 2;
    }

    // Pool size header
    uint16_t ps = static_cast<uint16_t>(poolLen & 0xFFFF);
    p[0] = static_cast<uint8_t>(ps & 0xFF);
    p[1] = static_cast<uint8_t>((ps >> 8) & 0xFF);
    p += 2;

    // String pool data
    if (!pool.empty()) {
        std::memcpy(p, pool.data(), pool.size());
        p += pool.size();
    }

    return static_cast<uint32_t>(p - outBuf);
}

} // namespace twoda
