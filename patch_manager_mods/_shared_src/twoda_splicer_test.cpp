#include "TwoDaSplicer.h"
#include "check.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// Helper to build synthetic 2DA V2.b binary buffer
static std::vector<uint8_t> buildBinary2DA(
    const std::vector<std::string>& cols,
    const std::vector<std::string>& labels,
    const std::vector<std::vector<std::string>>& cells,
    char colTerminator = '\n') {

    std::vector<uint8_t> buf;

    // Header: "2DA V2.b\n" (9 bytes)
    const char* hdr = "2DA V2.b\n";
    buf.insert(buf.end(), hdr, hdr + 9);

    // Column headers
    for (const auto& c : cols) {
        buf.insert(buf.end(), c.begin(), c.end());
        buf.push_back('\t');
    }
    buf.push_back(static_cast<uint8_t>(colTerminator));

    // Row count
    uint32_t numRows = static_cast<uint32_t>(labels.size());
    buf.push_back(static_cast<uint8_t>(numRows & 0xFF));
    buf.push_back(static_cast<uint8_t>((numRows >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((numRows >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((numRows >> 24) & 0xFF));

    // Row labels
    for (const auto& l : labels) {
        buf.insert(buf.end(), l.begin(), l.end());
        buf.push_back('\t');
    }

    // Build string pool & offset table
    std::vector<char> pool;
    pool.push_back('\0'); // offset 0 is ""
    std::unordered_map<std::string, uint16_t> poolMap;
    poolMap[""] = 0;

    std::vector<uint16_t> offsets;
    offsets.reserve(labels.size() * cols.size());

    for (size_t r = 0; r < labels.size(); ++r) {
        for (size_t c = 0; c < cols.size(); ++c) {
            std::string s = (r < cells.size() && c < cells[r].size()) ? cells[r][c] : "";
            if (s == "****") s = "";
            auto it = poolMap.find(s);
            if (it != poolMap.end()) {
                offsets.push_back(it->second);
            } else {
                uint16_t off = static_cast<uint16_t>(pool.size());
                poolMap[s] = off;
                pool.insert(pool.end(), s.begin(), s.end());
                pool.push_back('\0');
                offsets.push_back(off);
            }
        }
    }

    // Cell offsets table
    for (uint16_t off : offsets) {
        buf.push_back(static_cast<uint8_t>(off & 0xFF));
        buf.push_back(static_cast<uint8_t>((off >> 8) & 0xFF));
    }

    // Pool header size
    uint16_t poolSize = static_cast<uint16_t>(pool.size() & 0xFFFF);
    buf.push_back(static_cast<uint8_t>(poolSize & 0xFF));
    buf.push_back(static_cast<uint8_t>((poolSize >> 8) & 0xFF));

    // Pool data
    buf.insert(buf.end(), pool.begin(), pool.end());

    return buf;
}

// Simulates BioWare C2DA::Load2DArray in-place mutation and parsing
// (Reverses engineer Ghidra 0x0071c3d0 from swkotor2.exe)
struct SimulatedC2DA {
    uint32_t numCols = 0;
    uint32_t numRows = 0;
    std::vector<const char*> colNames;
    std::vector<const char*> rowLabels;
    const uint16_t* cellOffsets = nullptr;
    const char* stringPool = nullptr;
    bool loaded = false;

    bool Load(uint8_t* data, size_t len) {
        loaded = false;
        colNames.clear();
        rowLabels.clear();
        cellOffsets = nullptr;
        stringPool = nullptr;
        if (!data || len < 16) return false;
        if (std::memcmp(data, "2DA V2.b\n", 9) != 0) return false;

        // Step 1: Count columns (0x71c180 / 0x71c632)
        size_t p = 9;
        numCols = 0;
        while (p < len && data[p] != '\n' && data[p] != '\0') {
            size_t tab = p;
            while (tab < len && data[tab] != '\t' && data[tab] != '\n' && data[tab] != '\0') {
                tab++;
            }
            if (tab >= len || data[tab] != '\t') return false;
            numCols++;
            p = tab + 1;
        }
        if (numCols == 0) return false;

        // Step 2: In-place '\t' -> '\0' mutation for column headers (0x71c736..0x71c7a8)
        p = 9;
        for (uint32_t c = 0; c < numCols; ++c) {
            colNames.push_back(reinterpret_cast<const char*>(data + p));
            while (p < len && data[p] != '\t' && data[p] != '\0') {
                p++;
            }
            if (p >= len) return false;
            data[p] = '\0'; // in-place null-termination
            p++;
        }
        // Step 3: Engine skips 1-byte terminator (0x71c7aa: add ecx, 1)
        p++;

        // Step 4: Read numRows (0x71c7c8)
        if (p + 4 > len) return false;
        numRows = static_cast<uint32_t>(data[p]) |
                  (static_cast<uint32_t>(data[p + 1]) << 8) |
                  (static_cast<uint32_t>(data[p + 2]) << 16) |
                  (static_cast<uint32_t>(data[p + 3]) << 24);
        p += 4;

        // Step 5: In-place '\t' -> '\0' mutation for row labels (0x71c825..0x71c897)
        for (uint32_t r = 0; r < numRows; ++r) {
            rowLabels.push_back(reinterpret_cast<const char*>(data + p));
            while (p < len && data[p] != '\t' && data[p] != '\0') {
                p++;
            }
            if (p >= len) return false;
            data[p] = '\0'; // in-place null-termination
            p++;
        }

        // Step 6: Cell offset table pointer (0x71c8ae)
        size_t cellCount = static_cast<size_t>(numRows) * numCols;
        if (p + cellCount * 2 + 2 > len) return false;
        cellOffsets = reinterpret_cast<const uint16_t*>(data + p);
        p += cellCount * 2;

        // Step 7: Skip 2-byte poolSize (0x71c8dc)
        p += 2;

        // Step 8: String pool pointer (0x71c8eb)
        stringPool = reinterpret_cast<const char*>(data + p);
        loaded = true;
        return true;
    }

    const char* GetEntry(uint32_t r, uint32_t c) const {
        if (!loaded || r >= numRows || c >= numCols) return nullptr;
        size_t idx = static_cast<size_t>(r) * numCols + c;
        const uint8_t* raw = reinterpret_cast<const uint8_t*>(cellOffsets) + idx * 2;
        uint16_t off = static_cast<uint16_t>(raw[0] | (raw[1] << 8));
        return stringPool + off;
    }
};

static void test_base_parsing() {
    std::vector<std::string> cols = {
        "label", "race", "modeltype", "modela", "texa", "normalhead", "backuphead"
    };
    std::vector<std::string> labels;
    std::vector<std::vector<std::string>> cells;
    labels.reserve(20);
    cells.reserve(20);

    for (int r = 0; r < 20; ++r) {
        labels.push_back(std::to_string(r));
        if (r == 17) {
            // Row 17: Juhani template
            cells.push_back({
                "Juhani_Template", "4", "F", "pmba", "pmba", "17", "17"
            });
        } else {
            cells.push_back({
                "Row_" + std::to_string(r), "1", "M", "pmba", "pmba", "0", "0"
            });
        }
    }

    std::vector<uint8_t> baseBuf = buildBinary2DA(cols, labels, cells, '\n');

    twoda::ParsedView view;
    KT_CHECK(view.parse(baseBuf.data(), baseBuf.size()));
    KT_CHECK_EQ(view.numCols, 7);
    KT_CHECK_EQ(view.numRows, 20);

    // Column lookups
    KT_CHECK_EQ(view.findColumn("label"), 0);
    KT_CHECK_EQ(view.findColumn("LABEL"), 0);
    KT_CHECK_EQ(view.findColumn("race"), 1);
    KT_CHECK_EQ(view.findColumn("modeltype"), 2);
    KT_CHECK_EQ(view.findColumn("modela"), 3);
    KT_CHECK_EQ(view.findColumn("MODELA"), 3);
    KT_CHECK_EQ(view.findColumn("texa"), 4);
    KT_CHECK_EQ(view.findColumn("normalhead"), 5);
    KT_CHECK_EQ(view.findColumn("backuphead"), 6);
    KT_CHECK_EQ(view.findColumn("nonexistent"), -1);
    KT_CHECK_EQ(view.findColumn(nullptr), -1);

    // Cell values
    KT_CHECK(view.getCell(17, 0) != nullptr);
    KT_CHECK_EQ(std::strcmp(view.getCell(17, 0), "Juhani_Template"), 0);
    KT_CHECK_EQ(std::strcmp(view.getCell(17, 1), "4"), 0);
    KT_CHECK_EQ(std::strcmp(view.getCell(17, 2), "F"), 0);
    KT_CHECK_EQ(std::strcmp(view.getCell(17, 3), "pmba"), 0);
    KT_CHECK_EQ(std::strcmp(view.getCell(17, 5), "17"), 0);

    // Out of bounds
    KT_CHECK(view.getCell(20, 0) == nullptr);
    KT_CHECK(view.getCell(0, 7) == nullptr);
    KT_CHECK(view.getCell(100, 100) == nullptr);
}

static void test_juhani_splice() {
    std::vector<std::string> cols = {
        "label", "race", "modeltype", "modela", "texa", "normalhead", "backuphead"
    };
    std::vector<std::string> labels;
    std::vector<std::vector<std::string>> cells;
    labels.reserve(20);
    cells.reserve(20);

    for (int r = 0; r < 20; ++r) {
        labels.push_back(std::to_string(r));
        if (r == 17) {
            cells.push_back({
                "Juhani_Template", "4", "F", "pmba", "pmba", "17", "17"
            });
        } else {
            cells.push_back({
                "Row_" + std::to_string(r), "1", "M", "pmba", "pmba", "0", "0"
            });
        }
    }

    std::vector<uint8_t> baseBuf = buildBinary2DA(cols, labels, cells, '\n');

    twoda::CellPatch patches[3] = {
        { 818, "label", "K1P_Juhani" },
        { 818, "modela", "k1p_juhaniba" },
        { 818, "normalhead", "250" }
    };

    // Size query
    uint32_t reqSize = twoda::spliceRows(baseBuf.data(), baseBuf.size(), patches, 3, nullptr, 0, 17);
    KT_CHECK(reqSize > 0);

    // Serialization
    std::vector<uint8_t> outBuf(reqSize);
    uint32_t written = twoda::spliceRows(baseBuf.data(), baseBuf.size(), patches, 3, outBuf.data(), outBuf.size(), 17);
    KT_CHECK_EQ(written, reqSize);

    // Validate with ParsedView
    twoda::ParsedView spliced;
    KT_CHECK(spliced.parse(outBuf.data(), outBuf.size()));
    KT_CHECK_EQ(spliced.numRows, 819);
    KT_CHECK_EQ(spliced.numCols, 7);

    // Gap rows 20..817 have decimal labels and empty cells
    for (uint32_t r : { 20u, 50u, 200u, 500u, 816u, 817u }) {
        KT_CHECK_EQ(spliced.rowLabels[r].c_str(), std::to_string(r).c_str());
        for (uint32_t c = 0; c < 7; ++c) {
            const char* val = spliced.getCell(r, c);
            KT_CHECK(val != nullptr);
            KT_CHECK_EQ(std::strcmp(val, ""), 0);
        }
    }

    // Row 818
    KT_CHECK_EQ(spliced.rowLabels[818].c_str(), "818");
    KT_CHECK_EQ(std::strcmp(spliced.getCell(818, spliced.findColumn("label")), "K1P_Juhani"), 0);
    KT_CHECK_EQ(std::strcmp(spliced.getCell(818, spliced.findColumn("modela")), "k1p_juhaniba"), 0);
    KT_CHECK_EQ(std::strcmp(spliced.getCell(818, spliced.findColumn("normalhead")), "250"), 0);

    // Inherited from templateRow = 17
    KT_CHECK_EQ(std::strcmp(spliced.getCell(818, spliced.findColumn("race")), "4"), 0);
    KT_CHECK_EQ(std::strcmp(spliced.getCell(818, spliced.findColumn("modeltype")), "F"), 0);
    KT_CHECK_EQ(std::strcmp(spliced.getCell(818, spliced.findColumn("texa")), "pmba"), 0);
    KT_CHECK_EQ(std::strcmp(spliced.getCell(818, spliced.findColumn("backuphead")), "17"), 0);

    // Base row 17 remained unchanged
    KT_CHECK_EQ(std::strcmp(spliced.getCell(17, spliced.findColumn("label")), "Juhani_Template"), 0);
    KT_CHECK_EQ(std::strcmp(spliced.getCell(17, spliced.findColumn("modela")), "pmba"), 0);

    // Simulate C2DA::Load2DArray in-place mutation
    std::vector<uint8_t> engineBuf = outBuf;
    SimulatedC2DA sim;
    KT_CHECK(sim.Load(engineBuf.data(), engineBuf.size()));
    KT_CHECK_EQ(sim.numRows, 819);
    KT_CHECK_EQ(sim.numCols, 7);
    KT_CHECK_EQ(std::strcmp(sim.colNames[0], "label"), 0);
    KT_CHECK_EQ(std::strcmp(sim.colNames[3], "modela"), 0);
    KT_CHECK_EQ(std::strcmp(sim.rowLabels[818], "818"), 0);
    KT_CHECK_EQ(std::strcmp(sim.GetEntry(818, 0), "K1P_Juhani"), 0);
    KT_CHECK_EQ(std::strcmp(sim.GetEntry(818, 1), "4"), 0);
    KT_CHECK_EQ(std::strcmp(sim.GetEntry(818, 2), "F"), 0);
    KT_CHECK_EQ(std::strcmp(sim.GetEntry(818, 3), "k1p_juhaniba"), 0);
    KT_CHECK_EQ(std::strcmp(sim.GetEntry(818, 5), "250"), 0);
    KT_CHECK_EQ(std::strcmp(sim.GetEntry(500, 0), ""), 0);
}

static void test_heads_and_soundset_splice() {
    // heads.2da mock
    {
        std::vector<std::string> cols = { "head", "alttexture" };
        std::vector<std::string> labels;
        std::vector<std::vector<std::string>> cells;
        for (int r = 0; r < 20; ++r) {
            labels.push_back(std::to_string(r));
            cells.push_back({ "head_" + std::to_string(r), "alt_" + std::to_string(r) });
        }
        std::vector<uint8_t> baseBuf = buildBinary2DA(cols, labels, cells, '\0');

        twoda::CellPatch patch = { 250, "head", "k1p_juhanih" };
        uint32_t req = twoda::spliceRows(baseBuf.data(), baseBuf.size(), &patch, 1, nullptr, 0, -1);
        KT_CHECK(req > 0);

        std::vector<uint8_t> outBuf(req);
        uint32_t written = twoda::spliceRows(baseBuf.data(), baseBuf.size(), &patch, 1, outBuf.data(), outBuf.size(), -1);
        KT_CHECK_EQ(written, req);

        twoda::ParsedView spliced;
        KT_CHECK(spliced.parse(outBuf.data(), outBuf.size()));
        KT_CHECK_EQ(spliced.numRows, 251);
        KT_CHECK_EQ(spliced.numCols, 2);
        KT_CHECK_EQ(std::strcmp(spliced.getCell(250, 0), "k1p_juhanih"), 0);
        KT_CHECK_EQ(std::strcmp(spliced.getCell(250, 1), ""), 0); // templateRow < 0 -> empty
        KT_CHECK_EQ(std::strcmp(spliced.getCell(100, 0), ""), 0);

        // Engine simulation
        std::vector<uint8_t> engineBuf = outBuf;
        SimulatedC2DA sim;
        KT_CHECK(sim.Load(engineBuf.data(), engineBuf.size()));
        KT_CHECK_EQ(sim.numRows, 251);
        KT_CHECK_EQ(std::strcmp(sim.GetEntry(250, 0), "k1p_juhanih"), 0);
        KT_CHECK_EQ(std::strcmp(sim.GetEntry(250, 1), ""), 0);
    }

    // soundset.2da mock
    {
        std::vector<std::string> cols = { "label", "resref" };
        std::vector<std::string> labels;
        std::vector<std::vector<std::string>> cells;
        for (int r = 0; r < 10; ++r) {
            labels.push_back(std::to_string(r));
            cells.push_back({ "sound_" + std::to_string(r), "res_" + std::to_string(r) });
        }
        std::vector<uint8_t> baseBuf = buildBinary2DA(cols, labels, cells, '\n');

        twoda::CellPatch patches[2] = {
            { 70, "label", "kvj_juhani" },
            { 70, "resref", "kvjssf" }
        };
        uint32_t req = twoda::spliceRows(baseBuf.data(), baseBuf.size(), patches, 2, nullptr, 0, -1);
        KT_CHECK(req > 0);

        std::vector<uint8_t> outBuf(req);
        uint32_t written = twoda::spliceRows(baseBuf.data(), baseBuf.size(), patches, 2, outBuf.data(), outBuf.size(), -1);
        KT_CHECK_EQ(written, req);

        twoda::ParsedView spliced;
        KT_CHECK(spliced.parse(outBuf.data(), outBuf.size()));
        KT_CHECK_EQ(spliced.numRows, 71);
        KT_CHECK_EQ(spliced.numCols, 2);
        KT_CHECK_EQ(std::strcmp(spliced.getCell(70, 0), "kvj_juhani"), 0);
        KT_CHECK_EQ(std::strcmp(spliced.getCell(70, 1), "kvjssf"), 0);

        // Engine simulation
        std::vector<uint8_t> engineBuf = outBuf;
        SimulatedC2DA sim;
        KT_CHECK(sim.Load(engineBuf.data(), engineBuf.size()));
        KT_CHECK_EQ(sim.numRows, 71);
        KT_CHECK_EQ(std::strcmp(sim.GetEntry(70, 0), "kvj_juhani"), 0);
        KT_CHECK_EQ(std::strcmp(sim.GetEntry(70, 1), "kvjssf"), 0);
    }
}

static void test_error_handling() {
    std::vector<std::string> cols = { "colA", "colB" };
    std::vector<std::string> labels = { "0", "1" };
    std::vector<std::vector<std::string>> cells = { { "a0", "b0" }, { "a1", "b1" } };
    std::vector<uint8_t> validBuf = buildBinary2DA(cols, labels, cells, '\n');

    twoda::ParsedView view;

    // 1. Corrupt magic
    {
        std::vector<uint8_t> badMagic = validBuf;
        badMagic[0] = 'X';
        KT_CHECK(!view.parse(badMagic.data(), badMagic.size()));
        twoda::CellPatch p = { 0, "colA", "val" };
        KT_CHECK_EQ(twoda::spliceRows(badMagic.data(), badMagic.size(), &p, 1, nullptr, 0), 0);
    }

    // 2. Text 2DA magic "2DA V2.0\n"
    {
        std::vector<uint8_t> textMagic = validBuf;
        textMagic[6] = '0';
        KT_CHECK(!view.parse(textMagic.data(), textMagic.size()));
    }

    // 3. Truncated buffer (< 16 bytes)
    {
        KT_CHECK(!view.parse(validBuf.data(), 8));
        KT_CHECK(!view.parse(validBuf.data(), 0));
        KT_CHECK(!view.parse(nullptr, 100));
    }

    // 4. Truncated buffer in cell offset table
    {
        // Chop off last 10 bytes
        KT_CHECK(!view.parse(validBuf.data(), validBuf.size() - 10));
    }

    // 5. Unknown column in patch
    {
        twoda::CellPatch badCol = { 5, "unknown_col", "val" };
        KT_CHECK_EQ(twoda::spliceRows(validBuf.data(), validBuf.size(), &badCol, 1, nullptr, 0), 0);
    }

    // 6. Null colName in patch
    {
        twoda::CellPatch nullCol = { 5, nullptr, "val" };
        KT_CHECK_EQ(twoda::spliceRows(validBuf.data(), validBuf.size(), &nullCol, 1, nullptr, 0), 0);
    }

    // 7. Output buffer too small
    {
        twoda::CellPatch p = { 5, "colA", "val" };
        uint32_t req = twoda::spliceRows(validBuf.data(), validBuf.size(), &p, 1, nullptr, 0);
        KT_CHECK(req > 0);
        std::vector<uint8_t> smallBuf(req - 1);
        KT_CHECK_EQ(twoda::spliceRows(validBuf.data(), validBuf.size(), &p, 1, smallBuf.data(), smallBuf.size()), 0);
    }

    // 8. Value mapping: nullptr and "****" map to ""
    {
        twoda::CellPatch p1 = { 5, "colA", nullptr };
        twoda::CellPatch p2 = { 5, "colB", "****" };
        twoda::CellPatch ps[2] = { p1, p2 };
        uint32_t req = twoda::spliceRows(validBuf.data(), validBuf.size(), ps, 2, nullptr, 0);
        KT_CHECK(req > 0);
        std::vector<uint8_t> out(req);
        KT_CHECK_EQ(twoda::spliceRows(validBuf.data(), validBuf.size(), ps, 2, out.data(), out.size()), req);
        twoda::ParsedView pv;
        KT_CHECK(pv.parse(out.data(), out.size()));
        KT_CHECK_EQ(std::strcmp(pv.getCell(5, 0), ""), 0);
        KT_CHECK_EQ(std::strcmp(pv.getCell(5, 1), ""), 0);
    }

    // 9. String pool offset overflow (> 65535 unique bytes)
    {
        std::vector<twoda::CellPatch> hugePatches;
        // 700 patches, each with 100-character unique string -> 70,000 bytes > 65535
        std::vector<std::string> storage;
        storage.reserve(700);
        hugePatches.reserve(700);
        for (int i = 0; i < 700; ++i) {
            std::string s = "string_" + std::to_string(i) + "_";
            while (s.size() < 100) s += "X";
            storage.push_back(s);
            hugePatches.push_back({ static_cast<uint32_t>(i + 10), "colA", storage.back().c_str() });
        }
        uint32_t overflowRes = twoda::spliceRows(validBuf.data(), validBuf.size(),
                                                 hugePatches.data(), hugePatches.size(),
                                                 nullptr, 0);
        KT_CHECK_EQ(overflowRes, 0); // Must reject string pool overflow!
    }
}

static void test_real_files_if_present() {
    // Test with real heads.2da from npc_diversity_src if available
    const char* headsPath = "../npc_diversity_src/override/heads.2da";
    FILE* f = std::fopen(headsPath, "rb");
    if (f) {
        std::fseek(f, 0, SEEK_END);
        long sz = std::ftell(f);
        std::fseek(f, 0, SEEK_SET);
        if (sz > 0) {
            std::vector<uint8_t> fileBuf(static_cast<size_t>(sz));
            size_t n = std::fread(fileBuf.data(), 1, fileBuf.size(), f);
            std::fclose(f);
            if (n == fileBuf.size()) {
                twoda::ParsedView baseHeads;
                KT_CHECK(baseHeads.parse(fileBuf.data(), fileBuf.size()));
                uint32_t origRows = baseHeads.numRows;

                twoda::CellPatch patch = { 250, "head", "k1p_juhanih" };
                uint32_t req = twoda::spliceRows(fileBuf.data(), fileBuf.size(), &patch, 1, nullptr, 0);
                KT_CHECK(req > 0);

                std::vector<uint8_t> out(req);
                uint32_t written = twoda::spliceRows(fileBuf.data(), fileBuf.size(), &patch, 1, out.data(), out.size());
                KT_CHECK_EQ(written, req);

                twoda::ParsedView splicedHeads;
                KT_CHECK(splicedHeads.parse(out.data(), out.size()));
                KT_CHECK_EQ(splicedHeads.numRows, (origRows > 251 ? origRows : 251));
                KT_CHECK_EQ(std::strcmp(splicedHeads.getCell(250, splicedHeads.findColumn("head")), "k1p_juhanih"), 0);

                // Engine simulation
                std::vector<uint8_t> eng = out;
                SimulatedC2DA sim;
                KT_CHECK(sim.Load(eng.data(), eng.size()));
                KT_CHECK_EQ(sim.numRows, splicedHeads.numRows);
                KT_CHECK_EQ(std::strcmp(sim.GetEntry(250, splicedHeads.findColumn("head")), "k1p_juhanih"), 0);
            }
        }
    }

    // Test with real appearance.2da from npc_diversity_src if available
    const char* appPath = "../npc_diversity_src/override/appearance.2da";
    f = std::fopen(appPath, "rb");
    if (f) {
        std::fseek(f, 0, SEEK_END);
        long sz = std::ftell(f);
        std::fseek(f, 0, SEEK_SET);
        if (sz > 0) {
            std::vector<uint8_t> fileBuf(static_cast<size_t>(sz));
            size_t n = std::fread(fileBuf.data(), 1, fileBuf.size(), f);
            std::fclose(f);
            if (n == fileBuf.size()) {
                twoda::ParsedView baseApp;
                KT_CHECK(baseApp.parse(fileBuf.data(), fileBuf.size()));

                twoda::CellPatch patches[3] = {
                    { 818, "label", "K1P_Juhani" },
                    { 818, "modela", "k1p_juhaniba" },
                    { 818, "normalhead", "250" }
                };

                uint32_t req = twoda::spliceRows(fileBuf.data(), fileBuf.size(), patches, 3, nullptr, 0, 17);
                KT_CHECK(req > 0);

                std::vector<uint8_t> out(req);
                uint32_t written = twoda::spliceRows(fileBuf.data(), fileBuf.size(), patches, 3, out.data(), out.size(), 17);
                KT_CHECK_EQ(written, req);

                twoda::ParsedView splicedApp;
                KT_CHECK(splicedApp.parse(out.data(), out.size()));
                KT_CHECK_EQ(splicedApp.numRows, 819);
                KT_CHECK_EQ(std::strcmp(splicedApp.getCell(818, splicedApp.findColumn("label")), "K1P_Juhani"), 0);
                KT_CHECK_EQ(std::strcmp(splicedApp.getCell(818, splicedApp.findColumn("modela")), "k1p_juhaniba"), 0);
                KT_CHECK_EQ(std::strcmp(splicedApp.getCell(818, splicedApp.findColumn("normalhead")), "250"), 0);

                // Inherited race from row 17 in real appearance.2da
                const char* race17 = baseApp.getCell(17, baseApp.findColumn("race"));
                const char* race818 = splicedApp.getCell(818, splicedApp.findColumn("race"));
                KT_CHECK(race17 != nullptr && race818 != nullptr);
                KT_CHECK_EQ(std::strcmp(race17, race818), 0);

                // Engine simulation
                std::vector<uint8_t> eng = out;
                SimulatedC2DA sim;
                KT_CHECK(sim.Load(eng.data(), eng.size()));
                KT_CHECK_EQ(sim.numRows, 819);
                KT_CHECK_EQ(std::strcmp(sim.GetEntry(818, splicedApp.findColumn("label")), "K1P_Juhani"), 0);
            }
        }
    }
}

int main() {
    test_base_parsing();
    test_juhani_splice();
    test_heads_and_soundset_splice();
    test_error_handling();
    test_real_files_if_present();

    return kt::Report("twoda_splicer_test");
}
