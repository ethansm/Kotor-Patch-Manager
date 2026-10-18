#include "AssetVfsCore.h"
#include "check.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>

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

static std::vector<void*> g_allocatedPointers;
static uint8_t* testAlloc(uint32_t sz) {
    void* p = std::malloc(sz);
    g_allocatedPointers.push_back(p);
    return static_cast<uint8_t*>(p);
}


static int g_allocCalls = 0;
static int g_freeCalls = 0;
static uint8_t* countingAlloc(uint32_t sz) {
    g_allocCalls++;
    return static_cast<uint8_t*>(std::malloc(sz));
}
static void countingFree(void* p) {
    g_freeCalls++;
    std::free(p);
}

// ---- fake Miles (callback chaining tests) ----
struct MilesCallLog {
    int open, close, seek, read;
    uint32_t lastHandle;
    char lastName[300];
};
static MilesCallLog g_defLog, g_otherLog;
static assetvfs::MilesCallbackSet g_milesCur;
static int g_milesSetCalls = 0;
static int g_milesChanged = 0;
static int g_milesNoDefaults = 0;
static bool g_milesHasDefaults = true;

static uint32_t defOpen(const char* n, uint32_t* h) { g_defLog.open++; std::snprintf(g_defLog.lastName, sizeof(g_defLog.lastName), "%s", n); *h = 0x8ABCDEF0u; return 1; }
static void defClose(uint32_t h) { g_defLog.close++; g_defLog.lastHandle = h; }
static int32_t defSeek(uint32_t h, int32_t, uint32_t) { g_defLog.seek++; g_defLog.lastHandle = h; return 55; }
static uint32_t defRead(uint32_t h, void*, uint32_t) { g_defLog.read++; g_defLog.lastHandle = h; return 77; }
static uint32_t othOpen(const char* n, uint32_t* h) { g_otherLog.open++; std::snprintf(g_otherLog.lastName, sizeof(g_otherLog.lastName), "%s", n); *h = 0x7ABCDEF0u; return 1; }
static void othClose(uint32_t h) { g_otherLog.close++; g_otherLog.lastHandle = h; }
static int32_t othSeek(uint32_t h, int32_t, uint32_t) { g_otherLog.seek++; g_otherLog.lastHandle = h; return 11; }
static uint32_t othRead(uint32_t h, void*, uint32_t) { g_otherLog.read++; g_otherLog.lastHandle = h; return 22; }
static uint32_t ourOpen(const char*, uint32_t*) { return 0; }
static void ourClose(uint32_t) {}
static int32_t ourSeek(uint32_t, int32_t, uint32_t) { return 0; }
static uint32_t ourRead(uint32_t, void*, uint32_t) { return 0; }

static assetvfs::MilesCallbackSet makeSet(assetvfs::AilOpenCb o, assetvfs::AilCloseCb c, assetvfs::AilSeekCb s, assetvfs::AilReadCb r) {
    assetvfs::MilesCallbackSet m;
    m.open = o; m.close = c; m.seek = s; m.read = r;
    return m;
}
static assetvfs::MilesCallbackSet defaultsSet() { return makeSet(defOpen, defClose, defSeek, defRead); }
static assetvfs::MilesCallbackSet otherSet() { return makeSet(othOpen, othClose, othSeek, othRead); }
static assetvfs::MilesCallbackSet oursSet() { return makeSet(ourOpen, ourClose, ourSeek, ourRead); }

static void fakeMilesGet(assetvfs::MilesCallbackSet* out) { *out = g_milesCur; }
static void fakeMilesSet(const assetvfs::MilesCallbackSet& s) {
    g_milesSetCalls++;
    if (!s.open && !s.close && !s.seek && !s.read) {
        g_milesCur = g_milesHasDefaults ? defaultsSet() : assetvfs::MilesCallbackSet();
    } else {
        g_milesCur = s;
    }
}
static void fakeMilesNotice(int code) {
    if (code == assetvfs::kMilesNoticeChanged) g_milesChanged++;
    if (code == assetvfs::kMilesNoticeNoDefaults) g_milesNoDefaults++;
}
static void resetMilesFakes() {
    std::memset(&g_defLog, 0, sizeof(g_defLog));
    std::memset(&g_otherLog, 0, sizeof(g_otherLog));
    g_milesCur = defaultsSet();
    g_milesSetCalls = g_milesChanged = g_milesNoDefaults = 0;
    g_milesHasDefaults = true;
}

static int g_servicedCalls = 0;
static void* g_lastServiced = nullptr;
static int testServiced(void* r) {
    g_servicedCalls++;
    g_lastServiced = r;
    return 1;
}

int main() {
    assetvfs::EngineState state;

    // -------------------------------------------------------------
    // 1. handleGetKeyEntryMiss (hit & miss)
    // -------------------------------------------------------------
    void* table = nullptr;
    void* entry = nullptr;

    // Null checks
    KT_CHECK_EQ(state.handleGetKeyEntryMiss(nullptr, vfsreg::ResTypeUtc, &table, &entry), 0);
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("k1p_juhani", vfsreg::ResTypeUtc, nullptr, &entry), 0);
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("k1p_juhani", vfsreg::ResTypeUtc, &table, nullptr), 0);

    // Register test UTC asset
    const uint8_t utcData[] = { 'U', 'T', 'C', '_', 'B', 'I', 'N', 'A', 'R', 'Y', '!' };
    int slot = state.reg.registerAsset("k1p_juhani", vfsreg::ResTypeUtc, utcData, sizeof(utcData));
    KT_CHECK(slot >= 0);

    // Test hit
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("k1p_juhani", vfsreg::ResTypeUtc, &table, &entry), 1);
    KT_CHECK(table == &state.reg.fakeTable);
    KT_CHECK_EQ(state.reg.fakeTable.tableType, 4u);
    KT_CHECK(entry != nullptr);

    auto* kte = static_cast<vfsreg::KeyTableEntry*>(entry);
    KT_CHECK_EQ(kte->resType, vfsreg::ResTypeUtc);
    KT_CHECK_EQ(kte->packedId, vfsreg::makePackedId(static_cast<uint32_t>(slot)));
    KT_CHECK_EQ(std::memcmp(kte->resref, "k1p_juhani\0\0\0\0\0\0", 16), 0);

    // Test miss (unknown resref or wrong resType)
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("unknown_asset", vfsreg::ResTypeUtc, &table, &entry), 0);
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("k1p_juhani", vfsreg::ResTypeDlg, &table, &entry), 0);

    // Test disabled state
    state.enabled = false;
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("k1p_juhani", vfsreg::ResTypeUtc, &table, &entry), 0);
    state.enabled = true;

    // -------------------------------------------------------------
    // 2. handleServiceFromImage & 16-byte writable prefix
    // -------------------------------------------------------------
    uint8_t resBuf[0x40] = {0};
    *reinterpret_cast<uint32_t*>(resBuf + assetvfs::kResOffPackedId) = vfsreg::makePackedId(static_cast<uint32_t>(slot));

    g_servicedCalls = 0;
    g_lastServiced = nullptr;
    int sRes = state.handleServiceFromImage(resBuf, testAlloc, testServiced);
    KT_CHECK_EQ(sRes, 1);
    KT_CHECK_EQ(g_servicedCalls, 1);
    KT_CHECK_EQ(g_lastServiced, static_cast<void*>(resBuf));

    uint32_t resSize = *reinterpret_cast<uint32_t*>(resBuf + assetvfs::kResOffSize);
    KT_CHECK_EQ(resSize, static_cast<uint32_t>(sizeof(utcData)));

    uint32_t resStatus = *reinterpret_cast<uint32_t*>(resBuf + assetvfs::kResOffStatus);
    KT_CHECK((resStatus & 0x4) != 0);

    uint8_t* pData = *reinterpret_cast<uint8_t**>(resBuf + assetvfs::kResOffData);
    KT_CHECK(pData != nullptr);
    KT_CHECK_EQ(std::memcmp(pData, utcData, sizeof(utcData)), 0);

    // Verify 16-byte writable prefix before m_pResource (at m_pResource - 6 and m_pResource - 8)
    uint8_t* pMdlTag = pData - 6;
    uint8_t* p2DaHeader = pData - 8;
    *reinterpret_cast<uint16_t*>(pMdlTag) = 0x55AA;
    *reinterpret_cast<uint16_t*>(p2DaHeader) = 0xAA55;
    KT_CHECK_EQ(*reinterpret_cast<uint16_t*>(pMdlTag), 0x55AAu);
    KT_CHECK_EQ(*reinterpret_cast<uint16_t*>(p2DaHeader), 0xAA55u);

    // Service non-VFS packed ID (should be ignored)
    uint8_t nonVfsBuf[0x40] = {0};
    *reinterpret_cast<uint32_t*>(nonVfsBuf + assetvfs::kResOffPackedId) = 0x12345678;
    KT_CHECK_EQ(state.handleServiceFromImage(nonVfsBuf, testAlloc, testServiced), 0);

    // -------------------------------------------------------------
    // 3. TwoDaPatchTable::buildSpliced (appearance, heads, soundset)
    // -------------------------------------------------------------
    auto baseApp = buildBinary2DA({"label", "race", "modela"}, {"0", "1"}, {{"BASE0", "human", "p_zero"}, {"BASE1", "droid", "p_one"}});
    auto baseHeads = buildBinary2DA({"label", "head"}, {"0", "1"}, {{"HEAD0", "h_zero"}, {"HEAD1", "h_one"}});
    auto baseSound = buildBinary2DA({"label", "resref"}, {"0", "1"}, {{"SS0", "s_zero"}, {"SS1", "s_one"}});

    K2Vfs2DaCell appCells[] = {
        { "appearance", 818, 0, "label", "P_JUHANI" },
        { "appearance.2da", 818, 0, "race", "cathar" },
        { "appearance", 818, 0, "modela", "p_juhani" }
    };
    KT_CHECK_EQ(state.twodaPatches.addCells(appCells, 3), 3);

    K2Vfs2DaCell headCells[] = {
        { "heads.2da", 250, 0, "label", "HEAD_JUHANI" },
        { "heads", 250, 0, "head", "p_juhani_h" }
    };
    KT_CHECK_EQ(state.twodaPatches.addCells(headCells, 2), 2);

    K2Vfs2DaCell soundCells[] = {
        { "soundset", 70, 0, "label", "SS_JUHANI" },
        { "soundset.2da", 70, 0, "resref", "p_juhani" }
    };
    KT_CHECK_EQ(state.twodaPatches.addCells(soundCells, 2), 2);

    // Splicing appearance
    uint32_t appReq = state.twodaPatches.buildSpliced("appearance", baseApp.data(), baseApp.size(), nullptr, 0);
    KT_CHECK(appReq > 0);
    std::vector<uint8_t> appOut(appReq);
    uint32_t appWritten = state.twodaPatches.buildSpliced("appearance.2da", baseApp.data(), baseApp.size(), appOut.data(), appOut.size());
    KT_CHECK_EQ(appWritten, appReq);

    twoda::ParsedView pvApp;
    KT_CHECK(pvApp.parse(appOut.data(), appWritten));
    KT_CHECK_EQ(pvApp.numRows, 819u);
    int colModela = pvApp.findColumn("modela");
    KT_CHECK(colModela >= 0);
    const char* valModela = pvApp.getCell(818, static_cast<uint32_t>(colModela));
    KT_CHECK(valModela != nullptr);
    KT_CHECK(std::string(valModela) == "p_juhani");

    // Splicing heads
    uint32_t headReq = state.twodaPatches.buildSpliced("heads.2da", baseHeads.data(), baseHeads.size(), nullptr, 0);
    KT_CHECK(headReq > 0);
    std::vector<uint8_t> headOut(headReq);
    uint32_t headWritten = state.twodaPatches.buildSpliced("heads", baseHeads.data(), baseHeads.size(), headOut.data(), headOut.size());
    KT_CHECK_EQ(headWritten, headReq);

    twoda::ParsedView pvHead;
    KT_CHECK(pvHead.parse(headOut.data(), headWritten));
    KT_CHECK_EQ(pvHead.numRows, 251u);
    int colHead = pvHead.findColumn("head");
    KT_CHECK(colHead >= 0);
    const char* valHead = pvHead.getCell(250, static_cast<uint32_t>(colHead));
    KT_CHECK(valHead != nullptr);
    KT_CHECK(std::string(valHead) == "p_juhani_h");

    // Splicing soundset
    uint32_t ssReq = state.twodaPatches.buildSpliced("soundset", baseSound.data(), baseSound.size(), nullptr, 0);
    KT_CHECK(ssReq > 0);
    std::vector<uint8_t> ssOut(ssReq);
    uint32_t ssWritten = state.twodaPatches.buildSpliced("soundset.2da", baseSound.data(), baseSound.size(), ssOut.data(), ssOut.size());
    KT_CHECK_EQ(ssWritten, ssReq);

    twoda::ParsedView pvSS;
    KT_CHECK(pvSS.parse(ssOut.data(), ssWritten));
    KT_CHECK_EQ(pvSS.numRows, 71u);
    int colResref = pvSS.findColumn("resref");
    KT_CHECK(colResref >= 0);
    const char* valResref = pvSS.getCell(70, static_cast<uint32_t>(colResref));
    KT_CHECK(valResref != nullptr);
    KT_CHECK(std::string(valResref) == "p_juhani");

    // Table with no patches
    KT_CHECK_EQ(state.twodaPatches.buildSpliced("spells", baseApp.data(), baseApp.size(), nullptr, 0), 0u);

    // -------------------------------------------------------------
    // 4. resolveStreamSpec + parseVfsStreamSpec + MilesCursorTable
    // -------------------------------------------------------------
    uint8_t audioBuf[128] = {0};
    std::memcpy(audioBuf, "RIFF", 4);
    uint32_t riffSz = 50;
    std::memcpy(audioBuf + 4, &riffSz, 4);
    for (size_t i = 58; i < sizeof(audioBuf); ++i) {
        audioBuf[i] = static_cast<uint8_t>(i);
    }

    int audioSlot = state.reg.registerAsset("kvjv0001", vfsreg::ResTypeWav, audioBuf, sizeof(audioBuf), vfsreg::FlagMilesStream);
    KT_CHECK(audioSlot >= 0);

    char spec[64] = {0};
    KT_CHECK(state.resolveStreamSpec("StreamVoice/kvjv0001.wav", 8, spec, sizeof(spec)));
    std::string specStr(spec);
    KT_CHECK(specStr.find("#*VFS_") == 0);
    KT_CHECK(specStr.find(".mp3") != std::string::npos);

    uint32_t h = 0;
    KT_CHECK(state.parseVfsStreamSpec(spec, &h));
    KT_CHECK(state.cursors.isVfsHandle(h));

    uint8_t readBuf[32] = {0};
    uint32_t r1 = state.cursors.read(h, readBuf, 10);
    KT_CHECK_EQ(r1, 10u);
    KT_CHECK_EQ(readBuf[0], audioBuf[58]);
    KT_CHECK_EQ(readBuf[9], audioBuf[67]);

    int32_t pos = state.cursors.seek(h, 20, 0); // SEEK_SET
    KT_CHECK_EQ(pos, 20);
    uint32_t r2 = state.cursors.read(h, readBuf, 5);
    KT_CHECK_EQ(r2, 5u);
    KT_CHECK_EQ(readBuf[0], audioBuf[58 + 20]);

    pos = state.cursors.seek(h, 5, 1); // SEEK_CUR
    KT_CHECK_EQ(pos, 30);

    pos = state.cursors.seek(h, -10, 2); // SEEK_END
    KT_CHECK_EQ(pos, static_cast<int32_t>(sizeof(audioBuf) - 58 - 10));

    KT_CHECK(state.cursors.close(h));
    KT_CHECK_EQ(state.cursors.read(h, readBuf, 10), 0u);

    // Non-VFS names are never claimed by stem: bare names and disk specs go to Miles' own callbacks
    uint32_t hDirect = 0;
    KT_CHECK(!state.parseVfsStreamSpec("kvjv0001", &hDirect));
    KT_CHECK_EQ(hDirect, 0u);
    KT_CHECK(!state.parseVfsStreamSpec("*C:\\x\\streamvoice\\kvjv0001.wav*100*0", &hDirect));
    KT_CHECK(!state.parseVfsStreamSpec("StreamVoice/kvjv0001.wav", &hDirect));
    // a dead slot or a missing/odd index is not served either
    KT_CHECK(!state.parseVfsStreamSpec("#*VFS_200.wav", &hDirect));
    KT_CHECK(!state.parseVfsStreamSpec("#*VFS_x.wav", &hDirect));

    // -------------------------------------------------------------
    // 5. Miss Handlers (RegisterResManMissHandler / Unregister)
    // -------------------------------------------------------------
    state.reset();

    struct MissContext {
        int calls = 0;
        char lastResref[32] = {0};
        uint16_t lastType = 0;
        const void* assetData = nullptr;
        uint32_t assetSize = 0;
        int32_t returnCode = 0;
    } missCtx;

    auto testMissFn = [](const char* resref, uint16_t res_type, K2VfsAssetEntry* out_entry, void* user) -> int32_t {
        auto* ctx = static_cast<MissContext*>(user);
        ctx->calls++;
        std::snprintf(ctx->lastResref, sizeof(ctx->lastResref), "%s", resref ? resref : "");
        ctx->lastType = res_type;
        if (ctx->returnCode == 1 && ctx->assetData && ctx->assetSize > 0) {
            out_entry->resref = resref;
            out_entry->res_type = res_type;
            out_entry->flags = K2VFS_FLAG_RESMAN;
            out_entry->data = ctx->assetData;
            out_entry->size = ctx->assetSize;
            return 1;
        }
        return ctx->returnCode;
    };

    // Null checks
    KT_CHECK_EQ(state.registerMissHandler(nullptr, nullptr, "test"), 0);

    // Register handler
    int32_t tokMiss1 = state.registerMissHandler(testMissFn, &missCtx, "mod_a");
    KT_CHECK(tokMiss1 > 0);

    // Pass (return 0) -> miss returns 0
    missCtx.returnCode = 0;
    table = nullptr;
    entry = nullptr;
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("dyn_item", vfsreg::ResTypeUtc, &table, &entry), 0);
    KT_CHECK_EQ(missCtx.calls, 1);
    KT_CHECK(std::strcmp(missCtx.lastResref, "dyn_item") == 0);
    KT_CHECK_EQ(missCtx.lastType, vfsreg::ResTypeUtc);

    // The decline above is negative-cached: asking again does not call the handler
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("dyn_item", vfsreg::ResTypeUtc, &table, &entry), 0);
    KT_CHECK_EQ(missCtx.calls, 1);
    // ... until any Register* call clears the cache
    const uint8_t unrelated[] = { 'x' };
    KT_CHECK(state.registerAsset("unrelated", vfsreg::ResTypeDlg, unrelated, sizeof(unrelated), 0, false, "t") >= 0);
    missCtx.returnCode = 0;
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("dyn_item", vfsreg::ResTypeUtc, &table, &entry), 0);
    KT_CHECK_EQ(missCtx.calls, 2);
    KT_CHECK(state.registerAsset("unrelated2", vfsreg::ResTypeDlg, unrelated, sizeof(unrelated), 0, false, "t") >= 0);
    missCtx.calls = 1;

    // Handled (return 1) -> dynamically registered & returned
    const uint8_t dynData[] = { 'D', 'Y', 'N', 'A', 'M', 'I', 'C', '_', 'V', 'F', 'S' };
    missCtx.returnCode = 1;
    missCtx.assetData = dynData;
    missCtx.assetSize = sizeof(dynData);

    KT_CHECK_EQ(state.handleGetKeyEntryMiss("dyn_item", vfsreg::ResTypeUtc, &table, &entry), 1);
    KT_CHECK_EQ(missCtx.calls, 2);
    KT_CHECK(table == &state.reg.fakeTable);
    KT_CHECK(entry != nullptr);
    auto* dynKey = static_cast<vfsreg::KeyTableEntry*>(entry);
    KT_CHECK_EQ(dynKey->resType, vfsreg::ResTypeUtc);
    KT_CHECK_EQ(std::memcmp(dynKey->resref, "dyn_item\0\0\0\0\0\0\0\0", 16), 0);

    // Now servicing the dynamically registered asset
    uint8_t dynResBuf[0x40] = {0};
    *reinterpret_cast<uint32_t*>(dynResBuf + assetvfs::kResOffPackedId) = dynKey->packedId;
    g_servicedCalls = 0;
    g_lastServiced = nullptr;
    KT_CHECK_EQ(state.handleServiceFromImage(dynResBuf, testAlloc, testServiced), 1);
    KT_CHECK_EQ(g_servicedCalls, 1);
    KT_CHECK_EQ(g_lastServiced, static_cast<void*>(dynResBuf));

    uint8_t* pDynData = *reinterpret_cast<uint8_t**>(dynResBuf + assetvfs::kResOffData);
    KT_CHECK(pDynData != nullptr);
    KT_CHECK_EQ(std::memcmp(pDynData, dynData, sizeof(dynData)), 0);

    // Next lookup for "dyn_item" hits static registry directly (miss handler not called)
    int callsBefore = missCtx.calls;
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("dyn_item", vfsreg::ResTypeUtc, &table, &entry), 1);
    KT_CHECK_EQ(missCtx.calls, callsBefore);

    // Unregister miss handler
    state.unregisterMissHandler(tokMiss1);
    missCtx.returnCode = 1;
    KT_CHECK_EQ(state.handleGetKeyEntryMiss("other_item", vfsreg::ResTypeUtc, &table, &entry), 0);
    KT_CHECK_EQ(missCtx.calls, callsBefore); // Not called

    // Test handler slot capacity limit (16 max)
    int32_t tokens[assetvfs::EngineState::kMaxMissHandlers];
    for (size_t i = 0; i < assetvfs::EngineState::kMaxMissHandlers; ++i) {
        tokens[i] = state.registerMissHandler(testMissFn, nullptr, "slot");
        KT_CHECK(tokens[i] > 0);
    }
    KT_CHECK_EQ(state.registerMissHandler(testMissFn, nullptr, "overflow"), 0);
    for (size_t i = 0; i < assetvfs::EngineState::kMaxMissHandlers; ++i) {
        state.unregisterMissHandler(tokens[i]);
    }

    // -------------------------------------------------------------
    // 6. Stream Filters (RegisterStreamFilter / Unregister)
    // -------------------------------------------------------------
    state.reset();

    struct StreamFilterContext {
        int calls = 0;
        char lastCandidate[260] = {0};
        uint16_t lastKind = 0;
        int32_t returnCode = 0;
        const void* memoryData = nullptr;
        uint32_t memorySize = 0;
        std::string rewriteTarget;
    } streamCtx;

    auto testFilterFn = [](const char* candidate_path, uint16_t stream_kind,
                           const void** out_data, uint32_t* out_size,
                           char* out_resolved_path, uint32_t path_cap, void* user) -> int32_t {
        auto* ctx = static_cast<StreamFilterContext*>(user);
        ctx->calls++;
        std::snprintf(ctx->lastCandidate, sizeof(ctx->lastCandidate), "%s", candidate_path ? candidate_path : "");
        ctx->lastKind = stream_kind;

        if (ctx->returnCode == K2VFS_STREAM_MEMORY && ctx->memoryData && ctx->memorySize > 0) {
            *out_data = ctx->memoryData;
            *out_size = ctx->memorySize;
            return K2VFS_STREAM_MEMORY;
        } else if (ctx->returnCode == K2VFS_STREAM_REWRITE && !ctx->rewriteTarget.empty()) {
            std::snprintf(out_resolved_path, path_cap, "%s", ctx->rewriteTarget.c_str());
            return K2VFS_STREAM_REWRITE;
        }
        return K2VFS_STREAM_PASS;
    };

    // Null checks
    KT_CHECK_EQ(state.registerStreamFilter(nullptr, nullptr, "test"), 0);

    int32_t tokFilter = state.registerStreamFilter(testFilterFn, &streamCtx, "filter_mod");
    KT_CHECK(tokFilter > 0);

    // Case A: PASS (0)
    streamCtx.returnCode = K2VFS_STREAM_PASS;
    char streamSpec[64] = {0};
    KT_CHECK(!state.resolveStreamSpec("StreamVoice/kvjv9999.wav", 8, streamSpec, sizeof(streamSpec)));
    KT_CHECK_EQ(streamCtx.calls, 1);
    KT_CHECK(std::strcmp(streamCtx.lastCandidate, "StreamVoice/kvjv9999.wav") == 0);
    KT_CHECK_EQ(streamCtx.lastKind, 8);

    // Case B: K2VFS_STREAM_MEMORY (1) with 58-byte VO MP3 container
    uint8_t dynAudio[128] = {0};
    std::memcpy(dynAudio, "RIFF", 4);
    uint32_t dynRiffSz = 50;
    std::memcpy(dynAudio + 4, &dynRiffSz, 4);
    for (size_t i = 58; i < sizeof(dynAudio); ++i) {
        dynAudio[i] = static_cast<uint8_t>(0xA0 + (i - 58));
    }

    streamCtx.returnCode = K2VFS_STREAM_MEMORY;
    streamCtx.memoryData = dynAudio;
    streamCtx.memorySize = sizeof(dynAudio);

    std::memset(streamSpec, 0, sizeof(streamSpec));
    KT_CHECK(state.resolveStreamSpec("StreamVoice/kvjv9999.wav", 8, streamSpec, sizeof(streamSpec)));
    KT_CHECK_EQ(streamCtx.calls, 2);
    std::string sspec(streamSpec);
    KT_CHECK(sspec.find("#*VFS_") == 0);
    KT_CHECK(sspec.find(".mp3") != std::string::npos);

    // Verify Miles playback cursor opens and reads the memory buffer
    uint32_t dynH = 0;
    KT_CHECK(state.parseVfsStreamSpec(streamSpec, &dynH));
    KT_CHECK(state.cursors.isVfsHandle(dynH));

    uint8_t streamReadBuf[32] = {0};
    uint32_t sr1 = state.cursors.read(dynH, streamReadBuf, 10);
    KT_CHECK_EQ(sr1, 10u);
    KT_CHECK_EQ(streamReadBuf[0], dynAudio[58]);
    KT_CHECK_EQ(streamReadBuf[9], dynAudio[67]);
    state.cursors.close(dynH);

    // Case C: K2VFS_STREAM_REWRITE (2) is disabled: treated as PASS + exactly one notice per owner
    static int rewriteNotices;
    static char rewriteOwner[32];
    rewriteNotices = 0;
    rewriteOwner[0] = '\0';
    state.noticeFn = [](int code, const char*, const char* owner) {
        if (code == assetvfs::kNoticeRewriteIgnored) {
            rewriteNotices++;
            std::snprintf(rewriteOwner, sizeof(rewriteOwner), "%s", owner);
        }
    };
    streamCtx.returnCode = K2VFS_STREAM_REWRITE;
    streamCtx.rewriteTarget = "Custom/voice_pack_01.wav";

    std::memset(streamSpec, 0, sizeof(streamSpec));
    KT_CHECK(!state.resolveStreamSpec("StreamVoice/original.wav", 8, streamSpec, sizeof(streamSpec)));
    KT_CHECK_EQ(streamCtx.calls, 3);
    KT_CHECK(streamSpec[0] == '\0');
    KT_CHECK(!state.resolveStreamSpec("StreamVoice/original2.wav", 8, streamSpec, sizeof(streamSpec)));
    KT_CHECK_EQ(streamCtx.calls, 4);
    KT_CHECK_EQ(rewriteNotices, 1);
    KT_CHECK(std::strcmp(rewriteOwner, "filter_mod") == 0);
    state.noticeFn = nullptr;

    // Case D: Unregister stops callback
    state.unregisterStreamFilter(tokFilter);
    streamCtx.returnCode = K2VFS_STREAM_REWRITE;
    streamCtx.rewriteTarget = "Custom/should_not_reach.wav";
    std::memset(streamSpec, 0, sizeof(streamSpec));
    KT_CHECK(!state.resolveStreamSpec("StreamVoice/unregistered.wav", 8, streamSpec, sizeof(streamSpec)));
    KT_CHECK_EQ(streamCtx.calls, 4); // Unchanged

    // Test stream filter slot capacity limit (16 max)
    int32_t sTokens[assetvfs::EngineState::kMaxStreamFilters];
    for (size_t i = 0; i < assetvfs::EngineState::kMaxStreamFilters; ++i) {
        sTokens[i] = state.registerStreamFilter(testFilterFn, nullptr, "slot");
        KT_CHECK(sTokens[i] > 0);
    }
    KT_CHECK_EQ(state.registerStreamFilter(testFilterFn, nullptr, "overflow"), 0);
    for (size_t i = 0; i < assetvfs::EngineState::kMaxStreamFilters; ++i) {
        state.unregisterStreamFilter(sTokens[i]);
    }

    // -------------------------------------------------------------
    // 7. H1: *ppTable is zeroed by the handler on a miss, filled (not zeroed) on a hit
    // -------------------------------------------------------------
    state.reset();
    {
        const uint8_t d[] = { 'H', '1' };
        int hslot = state.reg.registerAsset("h1_asset", vfsreg::ResTypeUtc, d, sizeof(d));
        KT_CHECK(hslot >= 0);
        void* fake = reinterpret_cast<void*>(0x1234);
        void* tbl = fake;
        void* ent = fake;
        // hit
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("h1_asset", vfsreg::ResTypeUtc, &tbl, &ent), 1);
        KT_CHECK(tbl == &state.reg.fakeTable);
        KT_CHECK(tbl != nullptr);
        KT_CHECK(ent == &state.reg.records[hslot].keyEntry);
        // plain miss, wrong type, null resref / ppEntry, disabled: all zero *ppTable
        tbl = fake; ent = fake;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("nope", vfsreg::ResTypeUtc, &tbl, &ent), 0);
        KT_CHECK(tbl == nullptr);
        tbl = fake;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("h1_asset", vfsreg::ResTypeDlg, &tbl, &ent), 0);
        KT_CHECK(tbl == nullptr);
        tbl = fake;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss(nullptr, vfsreg::ResTypeUtc, &tbl, &ent), 0);
        KT_CHECK(tbl == nullptr);
        tbl = fake;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("h1_asset", vfsreg::ResTypeUtc, &tbl, nullptr), 0);
        KT_CHECK(tbl == nullptr);
        tbl = fake;
        state.enabled = false;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("h1_asset", vfsreg::ResTypeUtc, &tbl, &ent), 0);
        KT_CHECK(tbl == nullptr);
        state.enabled = true;
        // invalid resref with an active handler (zero-handler fast path is covered above)
        auto passFn = [](const char*, uint16_t, K2VfsAssetEntry*, void*) -> int32_t { return 0; };
        int32_t passTok = state.registerMissHandler(passFn, nullptr, "pass");
        KT_CHECK(passTok > 0);
        tbl = fake;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("bad!name", vfsreg::ResTypeUtc, &tbl, &ent), 0);
        KT_CHECK(tbl == nullptr);
        tbl = fake;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("unknown_x", vfsreg::ResTypeUtc, &tbl, &ent), 0);
        KT_CHECK(tbl == nullptr);
        state.unregisterMissHandler(passTok);
        // null ppTable must not crash
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("nope", vfsreg::ResTypeUtc, nullptr, &ent), 0);
    }

    // -------------------------------------------------------------
    // 8. H2: already-serviced CRes is untouched (0); callee result is passed through
    // -------------------------------------------------------------
    state.reset();
    {
        const uint8_t d[] = { 'S', 'V', 'C', '!' };
        int sslot = state.reg.registerAsset("svc_asset", vfsreg::ResTypeUtc, d, sizeof(d));
        KT_CHECK(sslot >= 0);
        uint8_t res[0x40] = {0};
        *reinterpret_cast<uint32_t*>(res + assetvfs::kResOffPackedId) = vfsreg::makePackedId(static_cast<uint32_t>(sslot));

        // Already serviced (flag 4): return 0, nothing touched, callee not called
        *reinterpret_cast<uint32_t*>(res + assetvfs::kResOffStatus) = 0x4;
        g_servicedCalls = 0;
        KT_CHECK_EQ(state.handleServiceFromImage(res, testAlloc, testServiced), 0);
        KT_CHECK_EQ(g_servicedCalls, 0);
        KT_CHECK(*reinterpret_cast<void**>(res + assetvfs::kResOffData) == nullptr);
        KT_CHECK_EQ(*reinterpret_cast<uint32_t*>(res + assetvfs::kResOffSize), 0u);
        KT_CHECK(!state.reg.records[sslot].served);

        // Callee returns 0 -> handler returns 0 (data already filled, flag 4 stays set)
        *reinterpret_cast<uint32_t*>(res + assetvfs::kResOffStatus) = 0;
        auto failServiced = [](void*) -> int { return 0; };
        KT_CHECK_EQ(state.handleServiceFromImage(res, testAlloc, failServiced), 0);
        KT_CHECK((*reinterpret_cast<uint32_t*>(res + assetvfs::kResOffStatus) & 0x4) != 0);
        KT_CHECK(state.reg.records[sslot].served);

        // Callee returns a value -> passed through
        auto sevenServiced = [](void*) -> int { return 7; };
        uint8_t res2[0x40] = {0};
        *reinterpret_cast<uint32_t*>(res2 + assetvfs::kResOffPackedId) = vfsreg::makePackedId(static_cast<uint32_t>(sslot));
        KT_CHECK_EQ(state.handleServiceFromImage(res2, testAlloc, sevenServiced), 7);

        // No callee -> 1
        uint8_t res3[0x40] = {0};
        *reinterpret_cast<uint32_t*>(res3 + assetvfs::kResOffPackedId) = vfsreg::makePackedId(static_cast<uint32_t>(sslot));
        KT_CHECK_EQ(state.handleServiceFromImage(res3, testAlloc, nullptr), 1);
    }

    // -------------------------------------------------------------
    // 9. Cursor handle tag, served rejection, pool limits and notices
    // -------------------------------------------------------------
    state.reset();
    {
        uint8_t audio[80] = {0};
        for (size_t i = 0; i < sizeof(audio); ++i) audio[i] = static_cast<uint8_t>(i);
        KT_CHECK(state.reg.registerAsset("snd_one", vfsreg::ResTypeWav, audio, sizeof(audio), vfsreg::FlagMilesStream) >= 0);
        uint32_t hh = 0;
        char sndSpec[64] = {0};
        KT_CHECK(state.resolveStreamSpec("StreamSounds/snd_one.wav", 4, sndSpec, sizeof(sndSpec)));
        KT_CHECK(state.parseVfsStreamSpec(sndSpec, &hh));
        KT_CHECK_EQ(hh, 0xFFFF0001u);
        KT_CHECK(state.isVfsHandle(hh));
        KT_CHECK(!state.isVfsHandle(0x8ABCDEF0u));
        KT_CHECK(!state.isVfsHandle(0x80000003u));
        // stream open marks the record served -> re-register rejected
        KT_CHECK_EQ(state.registerAsset("snd_one", vfsreg::ResTypeWav, audio, sizeof(audio), vfsreg::FlagMilesStream, false, "t"),
                    vfsreg::kRegServed);
        uint8_t rb[4] = {0};
        KT_CHECK_EQ(state.cursorRead(hh, rb, 4), 4u);
        KT_CHECK_EQ(state.cursorSeek(hh, 0, 0), 0);
        KT_CHECK(state.cursorClose(hh));
        KT_CHECK(!state.isVfsHandle(hh));
        KT_CHECK(!state.cursorClose(hh));
    }

    state.reset();
    {
        static int servedNotices, fullNotices;
        static char lastWhat[32], lastOwner[32];
        servedNotices = fullNotices = 0;
        state.noticeFn = [](int code, const char* what, const char* owner) {
            if (code == assetvfs::kNoticeServedReject) servedNotices++;
            if (code == assetvfs::kNoticePoolFull) {
                fullNotices++;
                std::snprintf(lastWhat, sizeof(lastWhat), "%s", what);
                std::snprintf(lastOwner, sizeof(lastOwner), "%s", owner);
            }
        };
        const uint8_t d[] = "payload";
        char name[32];
        for (size_t i = 0; i < vfsreg::Registry::kStaticSlots; ++i) {
            std::snprintf(name, sizeof(name), "s_%03u", static_cast<unsigned>(i));
            KT_CHECK(state.registerAsset(name, vfsreg::ResTypeUtc, d, sizeof(d), 0, false, "mod_s") >= 0);
        }
        KT_CHECK_EQ(state.registerAsset("s_over1", vfsreg::ResTypeUtc, d, sizeof(d), 0, false, "mod_s"), vfsreg::kRegPoolFull);
        KT_CHECK_EQ(state.registerAsset("s_over2", vfsreg::ResTypeUtc, d, sizeof(d), 0, false, "mod_s"), vfsreg::kRegPoolFull);
        KT_CHECK_EQ(fullNotices, 1); // once per pool
        KT_CHECK(std::strcmp(lastWhat, "static") == 0);
        KT_CHECK(std::strcmp(lastOwner, "mod_s") == 0);

        // a miss handler still gets dynamic slots while the static pool is full
        struct DynCtx { const void* data; uint32_t size; } dctx = { d, sizeof(d) };
        auto dynFn = [](const char* resref, uint16_t t, K2VfsAssetEntry* out, void* user) -> int32_t {
            auto* c = static_cast<DynCtx*>(user);
            out->resref = resref; out->res_type = t; out->flags = K2VFS_FLAG_RESMAN;
            out->data = c->data; out->size = c->size;
            return 1;
        };
        KT_CHECK(state.registerMissHandler(dynFn, &dctx, "dyn_owner") > 0);
        void* t2 = nullptr; void* e2 = nullptr;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("dyn_res1", vfsreg::ResTypeUtc, &t2, &e2), 1);
        auto* ke = static_cast<vfsreg::KeyTableEntry*>(e2);
        KT_CHECK(vfsreg::indexFromPackedId(ke->packedId) >= vfsreg::Registry::kStaticSlots);

        // served static record: re-registering is rejected and notified
        const uint8_t d2[] = "other";
        state.reg.markServed(0);
        KT_CHECK_EQ(state.registerAsset("s_000", vfsreg::ResTypeUtc, d2, sizeof(d2), 0, false, "mod_s"), vfsreg::kRegServed);
        KT_CHECK_EQ(servedNotices, 1);
        KT_CHECK(state.reg.records[0].data == d);

        state.noticeFn = nullptr;
    }

    // -------------------------------------------------------------
    // 10. H1 zero-handler fast path predicate (type counters, handler counter)
    // -------------------------------------------------------------
    state.reset();
    {
        KT_CHECK(state.missCannotHit(vfsreg::ResTypeUtc));
        KT_CHECK(state.missCannotHit(vfsreg::ResTypeDlg));
        const uint8_t d[] = { 'F', 'P' };
        KT_CHECK(state.registerAsset("fp_asset", vfsreg::ResTypeUtc, d, sizeof(d), 0, false, "t") >= 0);
        KT_CHECK(!state.missCannotHit(vfsreg::ResTypeUtc));
        KT_CHECK(state.missCannotHit(vfsreg::ResTypeDlg));
        // a stream record is not a ResMan record
        KT_CHECK(state.registerAsset("fp_stream", vfsreg::ResTypeWav, d, sizeof(d), vfsreg::FlagMilesStream, false, "t") >= 0);
        KT_CHECK(state.missCannotHit(vfsreg::ResTypeWav));
        auto passFn = [](const char*, uint16_t, K2VfsAssetEntry*, void*) -> int32_t { return 0; };
        int32_t t1 = state.registerMissHandler(passFn, nullptr, "a");
        KT_CHECK(!state.missCannotHit(vfsreg::ResTypeDlg));
        state.unregisterMissHandler(t1);
        state.unregisterMissHandler(t1); // double unregister must not underflow the counter
        KT_CHECK(state.missCannotHit(vfsreg::ResTypeDlg));
        state.enabled = false;
        KT_CHECK(state.missCannotHit(vfsreg::ResTypeUtc));
        state.enabled = true;
    }

    // -------------------------------------------------------------
    // 11. Miss handler data is COPIED, owner-tagged, evicted on unregister unless served
    // -------------------------------------------------------------
    state.reset();
    {
        g_allocCalls = g_freeCalls = 0;
        state.copyAllocFn = countingAlloc;
        state.copyFreeFn = countingFree;
        static int servedKeptNotices;
        static char keptWhat[16];
        servedKeptNotices = 0;
        state.noticeFn = [](int code, const char* what, const char*) {
            if (code == assetvfs::kNoticeServedKept) {
                servedKeptNotices++;
                std::snprintf(keptWhat, sizeof(keptWhat), "%s", what);
            }
        };

        struct Ctx { uint8_t buf[8]; int claim; } ctx = {{ 1, 2, 3, 4, 5, 6, 7, 8 }, 1};
        auto claimFn = [](const char* resref, uint16_t t, K2VfsAssetEntry* out, void* user) -> int32_t {
            auto* c = static_cast<Ctx*>(user);
            if (!c->claim) return 0;
            out->resref = resref; out->res_type = t; out->flags = K2VFS_FLAG_RESMAN;
            out->data = c->buf; out->size = sizeof(c->buf);
            return 1;
        };
        int32_t tok = state.registerMissHandler(claimFn, &ctx, "owner_a");
        KT_CHECK(tok > 0);

        void* tb = nullptr; void* en = nullptr;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("copy_me", vfsreg::ResTypeUtc, &tb, &en), 1);
        KT_CHECK_EQ(g_allocCalls, 1);
        auto* key = static_cast<vfsreg::KeyTableEntry*>(en);
        size_t idx = vfsreg::indexFromPackedId(key->packedId);
        KT_CHECK(state.reg.records[idx].data != ctx.buf); // provider-owned copy, not the handler's pointer
        KT_CHECK(state.dynOf(idx).token == tok);
        std::memset(ctx.buf, 0xEE, sizeof(ctx.buf));      // handler reuses its buffer
        KT_CHECK_EQ(state.reg.records[idx].data[0], 1u);
        KT_CHECK_EQ(state.reg.records[idx].data[7], 8u);

        // unregister: unserved record is evicted, copy freed, type counter back to zero
        state.unregisterMissHandler(tok);
        KT_CHECK_EQ(g_freeCalls, 1);
        KT_CHECK(!state.reg.records[idx].active);
        KT_CHECK(state.reg.findResMan("copy_me", vfsreg::ResTypeUtc) == nullptr);
        KT_CHECK(state.missCannotHit(vfsreg::ResTypeUtc));
        KT_CHECK_EQ(servedKeptNotices, 0);

        // served record survives unregister (data intact) with one notice
        std::memcpy(ctx.buf, "\x01\x02\x03\x04\x05\x06\x07\x08", 8);
        tok = state.registerMissHandler(claimFn, &ctx, "owner_b");
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("keep_me", vfsreg::ResTypeUtc, &tb, &en), 1);
        key = static_cast<vfsreg::KeyTableEntry*>(en);
        idx = vfsreg::indexFromPackedId(key->packedId);
        uint8_t res[0x40] = {0};
        *reinterpret_cast<uint32_t*>(res + assetvfs::kResOffPackedId) = key->packedId;
        KT_CHECK_EQ(state.handleServiceFromImage(res, testAlloc, nullptr), 1);
        int freesBefore = g_freeCalls;
        state.unregisterMissHandler(tok);
        KT_CHECK_EQ(g_freeCalls, freesBefore);
        KT_CHECK(state.reg.records[idx].active);
        KT_CHECK_EQ(state.reg.records[idx].data[3], 4u);
        KT_CHECK_EQ(servedKeptNotices, 1);
        KT_CHECK(std::strcmp(keptWhat, "1") == 0);
        // a later lookup of the served key still hits
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("keep_me", vfsreg::ResTypeUtc, &tb, &en), 1);

        // one owner's eviction leaves another owner's unserved record alone
        int32_t tokA = state.registerMissHandler(claimFn, &ctx, "owner_c");
        int32_t tokB = state.registerMissHandler(claimFn, &ctx, "owner_d");
        // handlers are asked in slot order: owner_c claims first
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("first_claim", vfsreg::ResTypeUtc, &tb, &en), 1);
        size_t ownedIdx = vfsreg::indexFromPackedId(static_cast<vfsreg::KeyTableEntry*>(en)->packedId);
        KT_CHECK(state.dynOf(ownedIdx).token == tokA);
        state.unregisterMissHandler(tokB);
        KT_CHECK(state.reg.records[ownedIdx].active);
        state.unregisterMissHandler(tokA);
        KT_CHECK(!state.reg.records[ownedIdx].active);

        // a handler unregistered while its callback runs does not leave an orphan record behind
        static assetvfs::EngineState* selfState;
        static int32_t selfTok;
        selfState = &state;
        auto selfRemoveFn = [](const char* resref, uint16_t t, K2VfsAssetEntry* out, void* user) -> int32_t {
            auto* c = static_cast<Ctx*>(user);
            selfState->unregisterMissHandler(selfTok);
            out->resref = resref; out->res_type = t; out->flags = K2VFS_FLAG_RESMAN;
            out->data = c->buf; out->size = sizeof(c->buf);
            return 1;
        };
        selfTok = state.registerMissHandler(selfRemoveFn, &ctx, "owner_e");
        int allocsBefore = g_allocCalls;
        freesBefore = g_freeCalls;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("orphan_x", vfsreg::ResTypeUtc, &tb, &en), 0);
        KT_CHECK(tb == nullptr);
        KT_CHECK_EQ(g_allocCalls - allocsBefore, g_freeCalls - freesBefore); // copy made and freed

        state.noticeFn = nullptr;
        state.reset();
        state.copyAllocFn = assetvfs::defaultCopyAlloc;
        state.copyFreeFn = assetvfs::defaultCopyFree;
    }

    // -------------------------------------------------------------
    // 12. Negative cache + recursion guard
    // -------------------------------------------------------------
    state.reset();
    {
        static int outerCalls, innerResult, innerTable;
        static assetvfs::EngineState* st;
        outerCalls = 0; innerResult = -1; innerTable = -1;
        st = &state;
        auto reentrantFn = [](const char*, uint16_t, K2VfsAssetEntry*, void*) -> int32_t {
            outerCalls++;
            void* it = reinterpret_cast<void*>(0x1234);
            void* ie = nullptr;
            innerResult = st->handleGetKeyEntryMiss("inner_x", vfsreg::ResTypeUtc, &it, &ie);
            innerTable = (it == nullptr) ? 1 : 0;
            return 0;
        };
        KT_CHECK(state.registerMissHandler(reentrantFn, nullptr, "re") > 0);
        void* tb = nullptr; void* en = nullptr;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("outer_x", vfsreg::ResTypeUtc, &tb, &en), 0);
        KT_CHECK_EQ(outerCalls, 1);      // the inner miss did not call the handler again
        KT_CHECK_EQ(innerResult, 0);
        KT_CHECK_EQ(innerTable, 1);      // ... and still zeroed *ppTable
        KT_CHECK(!state.inMiss);
        // inner_x was never asked of any handler, so it is not negative-cached: a fresh ask calls the handler
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("inner_x", vfsreg::ResTypeUtc, &tb, &en), 0);
        KT_CHECK_EQ(outerCalls, 2);

        // negative cache is keyed by (name, type)
        state.reset();
        static int declineCalls;
        declineCalls = 0;
        auto declineFn = [](const char*, uint16_t, K2VfsAssetEntry*, void*) -> int32_t { declineCalls++; return 0; };
        KT_CHECK(state.registerMissHandler(declineFn, nullptr, "d") > 0);
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("nc_a", vfsreg::ResTypeUtc, &tb, &en), 0);
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("nc_a", vfsreg::ResTypeUtc, &tb, &en), 0);
        KT_CHECK_EQ(declineCalls, 1);
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("nc_a", vfsreg::ResTypeDlg, &tb, &en), 0); // other type
        KT_CHECK_EQ(declineCalls, 2);
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("NC_A", vfsreg::ResTypeUtc, &tb, &en), 0); // case-normalised
        KT_CHECK_EQ(declineCalls, 2);
        // RegisterMissHandler clears it
        KT_CHECK(state.registerMissHandler(declineFn, nullptr, "d2") > 0);
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("nc_a", vfsreg::ResTypeUtc, &tb, &en), 0);
        KT_CHECK_EQ(declineCalls, 4); // both handlers asked
        // RegisterAssets path (registerAsset) clears it
        const uint8_t one[] = { 1 };
        KT_CHECK(state.registerAsset("whatever", vfsreg::ResTypeDlg, one, 1, 0, false, "t") >= 0);
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("nc_a", vfsreg::ResTypeUtc, &tb, &en), 0);
        KT_CHECK_EQ(declineCalls, 6);
        // a registration failure (pool full) is not a "nobody has it" answer: not cached
        static int claimCalls;
        claimCalls = 0;
        auto claimFn2 = [](const char* r, uint16_t t, K2VfsAssetEntry* o, void*) -> int32_t {
            claimCalls++;
            static const uint8_t dd[] = { 9 };
            o->resref = r; o->res_type = t; o->flags = 0; o->data = dd; o->size = 1;
            return 1;
        };
        state.reset();
        KT_CHECK(state.registerMissHandler(claimFn2, nullptr, "c") > 0);
        // fill the dynamic pool
        char nm[32];
        for (size_t i = 0; i < vfsreg::Registry::kDynamicReserve; ++i) {
            std::snprintf(nm, sizeof(nm), "fill_%02u", static_cast<unsigned>(i));
            KT_CHECK_EQ(state.handleGetKeyEntryMiss(nm, vfsreg::ResTypeUtc, &tb, &en), 1);
        }
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("overflow_a", vfsreg::ResTypeUtc, &tb, &en), 0);
        int callsAfterFirst = claimCalls;
        KT_CHECK_EQ(state.handleGetKeyEntryMiss("overflow_a", vfsreg::ResTypeUtc, &tb, &en), 0);
        KT_CHECK_EQ(claimCalls, callsAfterFirst + 1); // asked again (not negative-cached)
    }

    // -------------------------------------------------------------
    // 13. Stream filter: kinds, primary-candidate gate, copy, ownership, stem conflict, replay
    // -------------------------------------------------------------
    state.reset();
    {
        g_allocCalls = g_freeCalls = 0;
        state.copyAllocFn = countingAlloc;
        state.copyFreeFn = countingFree;
        static int conflictNotices, keptNotices, fltCalls;
        conflictNotices = keptNotices = fltCalls = 0;
        state.noticeFn = [](int code, const char*, const char*) {
            if (code == assetvfs::kNoticeStemConflict) conflictNotices++;
            if (code == assetvfs::kNoticeServedKept) keptNotices++;
        };

        static uint8_t wav[64];
        std::memcpy(wav, "RIFF", 4);
        for (size_t i = 4; i < sizeof(wav); ++i) wav[i] = static_cast<uint8_t>(i);
        auto memFn = [](const char*, uint16_t, const void** od, uint32_t* os, char*, uint32_t, void*) -> int32_t {
            fltCalls++;
            *od = wav; *os = sizeof(wav);
            return K2VFS_STREAM_MEMORY;
        };
        int32_t tok = state.registerStreamFilter(memFn, nullptr, "voice_owner");
        KT_CHECK(tok > 0);
        char spec[64] = {0};

        // kinds other than 4 / 8 never reach a filter; fallback candidates (allowFilters=false) neither
        KT_CHECK(!state.resolveStreamSpec("StreamVoice/abc.wav", 1, spec, sizeof(spec)));
        KT_CHECK(!state.resolveStreamSpec("StreamVoice/abc.wav", 0, spec, sizeof(spec)));
        KT_CHECK(!state.resolveStreamSpec("StreamVoice/abc.wav", 8, spec, sizeof(spec), false));
        KT_CHECK_EQ(fltCalls, 0);

        // kind 4 and 8 do; plain RIFF is passed through unsliced; data is a provider-owned copy
        KT_CHECK(state.resolveStreamSpec("StreamVoice/abc.wav", 4, spec, sizeof(spec)));
        KT_CHECK_EQ(fltCalls, 1);
        KT_CHECK_EQ(g_allocCalls, 1);
        KT_CHECK(std::strstr(spec, ".wav") != nullptr);
        size_t sidx = static_cast<size_t>(std::strtoul(spec + 6, nullptr, 10));
        KT_CHECK(state.reg.records[sidx].data != wav);
        KT_CHECK_EQ(state.reg.records[sidx].size, static_cast<uint32_t>(sizeof(wav)));
        KT_CHECK(std::strcmp(state.dynOf(sidx).path, "streamvoice\\abc.wav") == 0);
        KT_CHECK_EQ(state.reg.records[sidx].data[0], static_cast<uint8_t>('R'));
        uint32_t ch = 0;
        KT_CHECK(state.parseVfsStreamSpec(spec, &ch));
        uint8_t rb[4] = {0};
        KT_CHECK_EQ(state.cursorRead(ch, rb, 4), 4u);
        KT_CHECK_EQ(rb[0], static_cast<uint8_t>('R')); // no slicing for plain RIFF
        state.cursorClose(ch);

        // same exact path again (any case / separator): served from the record, filter not asked again
        char spec2[64] = {0};
        KT_CHECK(state.resolveStreamSpec("STREAMVOICE\\ABC.WAV", 4, spec2, sizeof(spec2)));
        KT_CHECK_EQ(fltCalls, 1);
        KT_CHECK(std::strcmp(spec, spec2) == 0);

        // a different path with the SAME stem is not served that record: the filter claim is rejected
        char spec3[64] = {0};
        KT_CHECK(!state.resolveStreamSpec("OtherDir/abc.wav", 8, spec3, sizeof(spec3)));
        KT_CHECK_EQ(fltCalls, 2);
        KT_CHECK_EQ(conflictNotices, 1);
        KT_CHECK_EQ(state.reg.findStream("abc")->data[0], static_cast<uint8_t>('R'));

        // no synthetic vfs_flt_* retry records exist
        KT_CHECK(state.reg.findStream("vfs_flt_0") == nullptr);

        // replay: an already-issued live spec is accepted as-is (no filter call), a dead slot is not
        char replay[64] = {0};
        KT_CHECK(state.resolveStreamSpec(spec, 8, replay, sizeof(replay)));
        KT_CHECK(std::strcmp(replay, spec) == 0);
        KT_CHECK_EQ(fltCalls, 2);
        KT_CHECK(!state.resolveStreamSpec("#*VFS_77.mp3", 8, replay, sizeof(replay)));
        KT_CHECK_EQ(fltCalls, 2);

        // unserved MEMORY record from another stem is evicted on unregister; the served one survives
        char specU[64] = {0};
        KT_CHECK(state.resolveStreamSpec("StreamVoice/unserved.wav", 8, specU, sizeof(specU)));
        size_t uidx = static_cast<size_t>(std::strtoul(specU + 6, nullptr, 10));
        int freesBefore = g_freeCalls;
        state.unregisterStreamFilter(tok);
        KT_CHECK_EQ(g_freeCalls - freesBefore, 1);
        KT_CHECK(!state.reg.records[uidx].active);
        KT_CHECK(state.reg.records[sidx].active);
        KT_CHECK_EQ(state.reg.records[sidx].data[0], static_cast<uint8_t>('R'));
        KT_CHECK_EQ(keptNotices, 1);

        state.noticeFn = nullptr;
        state.reset();
        state.copyAllocFn = assetvfs::defaultCopyAlloc;
        state.copyFreeFn = assetvfs::defaultCopyFree;
    }

    // -------------------------------------------------------------
    // 14. Miles chaining: forward everything that is not ours, reset-then-reread, re-install on change
    // -------------------------------------------------------------
    state.reset();
    {
        resetMilesFakes();
        assetvfs::MilesChain chain;
        chain.bind(&state, oursSet(), fakeMilesGet, fakeMilesSet, fakeMilesNotice);

        // Miles starts with NULL callbacks: reset to defaults (set 0,0,0,0), re-read, then install ours
        g_milesCur = assetvfs::MilesCallbackSet();
        KT_CHECK(chain.ensure());
        KT_CHECK_EQ(g_milesSetCalls, 2);
        KT_CHECK(chain.prev.sameAs(defaultsSet()));
        KT_CHECK(g_milesCur.sameAs(oursSet()));
        KT_CHECK(chain.installed);
        KT_CHECK_EQ(g_milesChanged, 0);

        // a second match with nothing changed does not re-install
        KT_CHECK(chain.ensure());
        KT_CHECK_EQ(g_milesSetCalls, 2);

        // disk spec: forwarded unchanged to Miles' default open, never claimed by stem
        const uint8_t pcm[] = { 10, 11, 12, 13, 14, 15, 16, 17 };
        KT_CHECK(state.registerAsset("abc", vfsreg::ResTypeWav, pcm, sizeof(pcm), vfsreg::FlagMilesStream, false, "t") >= 0);
        uint32_t hd = 0;
        const char* diskSpec = "*C:\\x\\streamvoice\\abc.wav*100*0";
        KT_CHECK_EQ(chain.open(diskSpec, &hd), 1u);
        KT_CHECK_EQ(g_defLog.open, 1);
        KT_CHECK(std::strcmp(g_defLog.lastName, diskSpec) == 0);
        KT_CHECK_EQ(hd, 0x8ABCDEF0u);
        KT_CHECK(!state.reg.records[0].served);

        // heap-like handle: read / seek / close all forwarded
        uint8_t buf[4] = {0};
        KT_CHECK_EQ(chain.read(0x8ABCDEF0u, buf, 4), 77u);
        KT_CHECK_EQ(chain.seek(0x8ABCDEF0u, 5, 1), 55);
        chain.close(0x8ABCDEF0u);
        KT_CHECK_EQ(g_defLog.read, 1);
        KT_CHECK_EQ(g_defLog.seek, 1);
        KT_CHECK_EQ(g_defLog.close, 1);
        KT_CHECK_EQ(g_defLog.lastHandle, 0x8ABCDEF0u);

        // #*VFS_ spec: served from the cursor table, nothing forwarded
        char vspec[64] = {0};
        KT_CHECK(state.resolveStreamSpec("StreamSounds/abc.wav", 4, vspec, sizeof(vspec)));
        uint32_t hv = 0;
        KT_CHECK_EQ(chain.open(vspec, &hv), 1u);
        KT_CHECK_EQ(g_defLog.open, 1);
        KT_CHECK(state.isVfsHandle(hv));
        KT_CHECK_EQ(chain.read(hv, buf, 4), 4u);
        KT_CHECK_EQ(buf[0], 10u);
        KT_CHECK_EQ(chain.seek(hv, 2, 0), 2);
        KT_CHECK_EQ(g_defLog.read, 1);
        KT_CHECK_EQ(g_defLog.seek, 1);
        chain.close(hv);
        KT_CHECK(!state.isVfsHandle(hv));
        KT_CHECK_EQ(g_defLog.close, 1);
        // stale / double close and use of one of our dead handles is swallowed, never forwarded
        chain.close(hv);
        KT_CHECK_EQ(chain.read(hv, buf, 4), 0u);
        KT_CHECK_EQ(chain.seek(hv, 0, 0), -1);
        KT_CHECK_EQ(g_defLog.close, 1);
        KT_CHECK_EQ(g_defLog.read, 1);
        KT_CHECK_EQ(g_defLog.seek, 1);
        // an unservable #*VFS_ name fails the open instead of reaching the disk path
        uint32_t hx = 0;
        KT_CHECK_EQ(chain.open("#*VFS_99.wav", &hx), 0u);
        KT_CHECK_EQ(g_defLog.open, 1);
        KT_CHECK_EQ(chain.open(nullptr, &hx), 0u);

        // another module replaces the callbacks: one notice, adopt as prev, re-install ours
        g_milesCur = otherSet();
        KT_CHECK(chain.ensure());
        KT_CHECK_EQ(g_milesChanged, 1);
        KT_CHECK(chain.prev.sameAs(otherSet()));
        KT_CHECK(g_milesCur.sameAs(oursSet()));
        KT_CHECK_EQ(g_milesSetCalls, 3);
        KT_CHECK_EQ(chain.open(diskSpec, &hd), 1u);
        KT_CHECK_EQ(g_otherLog.open, 1);
        KT_CHECK_EQ(g_defLog.open, 1);
        KT_CHECK(chain.ensure());
        KT_CHECK_EQ(g_milesSetCalls, 3); // read-back equals ours: no re-install

        // partial overwrite (only `open` replaced): the entries that are still ours keep the old target
        assetvfs::MilesCallbackSet part = oursSet();
        part.open = defaultsSet().open;
        g_milesCur = part;
        KT_CHECK(chain.ensure());
        KT_CHECK_EQ(g_milesChanged, 2);
        KT_CHECK(chain.prev.open == defaultsSet().open);
        KT_CHECK(chain.prev.close == otherSet().close);
        KT_CHECK(chain.prev.seek == otherSet().seek);
        KT_CHECK(chain.prev.read == otherSet().read);
        KT_CHECK(g_milesCur.sameAs(oursSet()));

        // callbacks cleared behind our back: reset to defaults, re-read, adopt, re-install
        int setsBefore = g_milesSetCalls;
        g_milesCur = assetvfs::MilesCallbackSet();
        KT_CHECK(chain.ensure());
        KT_CHECK_EQ(g_milesSetCalls, setsBefore + 2);
        KT_CHECK(chain.prev.sameAs(defaultsSet()));
        KT_CHECK_EQ(g_milesChanged, 3);
    }

    state.reset();
    {
        // Miles cannot produce defaults: error notice once, nothing installed, no match
        resetMilesFakes();
        g_milesHasDefaults = false;
        assetvfs::MilesChain chain;
        chain.bind(&state, oursSet(), fakeMilesGet, fakeMilesSet, fakeMilesNotice);
        g_milesCur = assetvfs::MilesCallbackSet();
        KT_CHECK(!chain.ensure());
        KT_CHECK(!chain.ensure());
        KT_CHECK_EQ(g_milesNoDefaults, 1);
        KT_CHECK(!chain.installed);
        KT_CHECK(!g_milesCur.sameAs(oursSet()));
        // forwarding with no target is inert, not a crash
        uint32_t h = 0;
        KT_CHECK_EQ(chain.open("*x*1*0", &h), 0u);
        KT_CHECK_EQ(chain.read(0x8ABCDEF0u, &h, 4), 0u);
        KT_CHECK_EQ(chain.seek(0x8ABCDEF0u, 0, 0), -1);
        chain.close(0x8ABCDEF0u);

        // unbound / missing get-set: no match
        assetvfs::MilesChain unbound;
        KT_CHECK(!unbound.ensure());
    }


    // -------------------------------------------------------------
    // observer mode, config parsing, log budget, counters
    // -------------------------------------------------------------
    {
        using assetvfs::VfsConfig;
        VfsConfig d = VfsConfig::fromIni("", 1, assetvfs::defaultLogPrefixes(), 0);
        KT_CHECK_EQ(d.mode, assetvfs::kModeObserve);
        KT_CHECK(!d.modeInvalid);
        KT_CHECK_EQ(d.nPrefixes, 3u);
        KT_CHECK(d.prefixMatch("k1p_juhani"));
        KT_CHECK(d.prefixMatch("KVJDLG01"));
        KT_CHECK(d.prefixMatch("p_juhani_h"));
        KT_CHECK(!d.prefixMatch("p_carth"));
        KT_CHECK(!d.prefixMatch(""));
        KT_CHECK(!d.logAll);
        KT_CHECK_EQ(VfsConfig::fromIni("serve", 1, "", 0).mode, assetvfs::kModeServe);
        KT_CHECK_EQ(VfsConfig::fromIni("  Serve ", 1, "", 0).mode, assetvfs::kModeServe);
        KT_CHECK_EQ(VfsConfig::fromIni("OFF", 1, "", 0).mode, assetvfs::kModeOff);
        KT_CHECK_EQ(VfsConfig::fromIni("observe", 1, "", 0).mode, assetvfs::kModeObserve);
        KT_CHECK_EQ(VfsConfig::fromIni(nullptr, 1, nullptr, 0).mode, assetvfs::kModeObserve);
        KT_CHECK_EQ(VfsConfig::fromIni(nullptr, 0, nullptr, 0).mode, assetvfs::kModeOff);   // legacy enabled=0
        KT_CHECK_EQ(VfsConfig::fromIni("serve", 0, nullptr, 0).mode, assetvfs::kModeOff);   // opt-out wins
        VfsConfig bad = VfsConfig::fromIni("bogus", 1, "", 0);
        KT_CHECK(bad.modeInvalid);
        KT_CHECK_EQ(bad.mode, assetvfs::kModeObserve);
        VfsConfig e = VfsConfig::fromIni("serve", 1, "", 0);
        KT_CHECK_EQ(e.nPrefixes, 0u);
        KT_CHECK(!e.wants("k1p_x"));
        VfsConfig la = VfsConfig::fromIni("serve", 1, "", 1);
        KT_CHECK(la.logAll);
        KT_CHECK(la.wants("anything"));
        VfsConfig sp = VfsConfig::fromIni("serve", 1, " K1P_ , ,Vo_*x ,", 0);
        KT_CHECK_EQ(sp.nPrefixes, 2u);
        KT_CHECK(sp.prefixMatch("k1p_a"));
        KT_CHECK(sp.prefixMatch("VO_*xyz"));
        KT_CHECK(!sp.prefixMatch("k1"));
        VfsConfig many = VfsConfig::fromIni("serve", 1, "a,b,c,d,e,f,g,h,i,j", 0);
        KT_CHECK_EQ(many.nPrefixes, static_cast<uint32_t>(VfsConfig::kMaxPrefixes));

        char stem[40];
        assetvfs::streamStem("Sound\\Cut\\K1P_Hello.WAV", stem, sizeof(stem));
        KT_CHECK(std::strcmp(stem, "k1p_hello") == 0);
        assetvfs::streamStem("noext", stem, sizeof(stem));
        KT_CHECK(std::strcmp(stem, "noext") == 0);
        assetvfs::streamStem(nullptr, stem, sizeof(stem));
        KT_CHECK(stem[0] == '\0');
    }

    {
        // Log budget: prefix test first, 20 per name then every 100th, total cap 400, silent afterwards.
        assetvfs::EngineState b;
        b.applyConfig(assetvfs::VfsConfig::fromIni("observe", 1, "k1p_", 0));
        int taken = 0;
        for (int i = 0; i < 25; ++i) if (b.takeLog("k1p_one")) ++taken;
        KT_CHECK_EQ(taken, 20);
        taken = 0;
        for (int i = 0; i < 300; ++i) if (b.takeLog("k1p_one")) ++taken;
        KT_CHECK_EQ(taken, 3); // per-name calls 120, 220, 320 (25 + 300 = 325 calls so far)
        KT_CHECK(!b.takeLog("other_name"));            // prefix filtered: no budget consumed
        KT_CHECK(!b.takeLog(nullptr));
        assetvfs::EngineState c;
        c.applyConfig(assetvfs::VfsConfig::fromIni("observe", 1, "k1p_", 0));
        int total = 0;
        for (int n = 0; n < 40; ++n) {
            char nm[24];
            std::snprintf(nm, sizeof(nm), "k1p_name%02d", n);
            for (int i = 0; i < 20; ++i) if (c.takeLog(nm)) ++total;
        }
        KT_CHECK_EQ(total, 400);
        KT_CHECK(!c.takeLog("k1p_fresh"));
        // counters-only config logs nothing
        assetvfs::EngineState z;
        z.applyConfig(assetvfs::VfsConfig::fromIni("observe", 1, "", 0));
        KT_CHECK(!z.takeLog("k1p_one"));
        // log_all logs any name (within budget)
        assetvfs::EngineState la;
        la.applyConfig(assetvfs::VfsConfig::fromIni("observe", 1, "", 1));
        KT_CHECK(la.takeLog("whatever"));
        KT_CHECK(!la.missCannotHit(vfsreg::ResTypeUtc));
    }

    {
        // 2DA row counting
        assetvfs::EngineState t;
        K2Vfs2DaCell tc[] = {
            { "appearance", 818, 0, "label", "A" },
            { "appearance.2da", 818, 0, "race", "x" },
            { "appearance", 819, 0, "label", "B" },
            { "heads", 5, 0, "label", "H" }
        };
        KT_CHECK_EQ(t.addTwoDaCells(tc, 4), 4);
        KT_CHECK_EQ(t.countTwoDaRows("appearance"), 2u);
        KT_CHECK_EQ(t.countTwoDaRows("HEADS"), 1u);
        KT_CHECK_EQ(t.countTwoDaRows("none"), 0u);
        KT_CHECK_EQ(t.countTwoDaRows(nullptr), 0u);
    }

    {
        struct OCtx { int calls = 0; const uint8_t* data = nullptr; uint32_t size = 0; };
        static OCtx octx;
        octx = OCtx();
        static const uint8_t odata[] = { 'o', 'b', 's' };
        octx.data = odata;
        octx.size = sizeof(odata);
        auto obsHandler = [](const char*, uint16_t, K2VfsAssetEntry* out, void* user) -> int32_t {
            OCtx* c = static_cast<OCtx*>(user);
            c->calls++;
            out->data = c->data;
            out->size = c->size;
            return 1;
        };
        static int evMiss[4];
        static int evServe;
        static char evName[32];
        std::memset(evMiss, 0, sizeof(evMiss));
        evServe = 0;
        evName[0] = '\0';
        auto missEv = [](const char* n, uint16_t, int outcome) {
            evMiss[outcome]++;
            std::snprintf(evName, sizeof(evName), "%s", n);
        };
        auto serveEv = [](const char*, uint16_t, uint32_t, int) { evServe++; };

        // ---- observe: nothing changes ----
        assetvfs::EngineState ob;
        ob.applyConfig(assetvfs::VfsConfig::fromIni("observe", 1, "k1p_", 0));
        ob.missEventFn = missEv;
        ob.serveEventFn = serveEv;
        KT_CHECK(ob.observe && ob.enabled);
        int32_t tok = ob.registerMissHandler(obsHandler, &octx, "mod");
        KT_CHECK(tok > 0);
        void* ot = reinterpret_cast<void*>(0x1234);
        void* oe = reinterpret_cast<void*>(0x5678);
        KT_CHECK_EQ(ob.handleGetKeyEntryMiss("k1p_obs", vfsreg::ResTypeUtc, &ot, &oe), 0);
        KT_CHECK_EQ(octx.calls, 1);                       // handler is called ...
        KT_CHECK(ot == nullptr);                          // ... *ppTable is zeroed (missExit contract)
        KT_CHECK(oe == reinterpret_cast<void*>(0x5678));  // ... *ppEntry untouched
        KT_CHECK(!ob.hasAsset("k1p_obs", vfsreg::ResTypeUtc, vfsreg::FlagResMan)); // never registered
        KT_CHECK(ob.reg.findResMan("k1p_obs", vfsreg::ResTypeUtc) == nullptr);
        KT_CHECK_EQ(evMiss[assetvfs::kMissObserve], 1);
        KT_CHECK_EQ(evMiss[assetvfs::kMissServe], 0);
        KT_CHECK(std::strcmp(evName, "k1p_obs") == 0);
        KT_CHECK_EQ(ob.counters.observe.load(), 1u);
        KT_CHECK_EQ(ob.counters.served.load(), 0u);
        // a second lookup asks the handler again (handled names are not negative-cached)
        ot = reinterpret_cast<void*>(0x1234);
        KT_CHECK_EQ(ob.handleGetKeyEntryMiss("k1p_obs", vfsreg::ResTypeUtc, &ot, &oe), 0);
        KT_CHECK_EQ(octx.calls, 2);
        // unfiltered name: counters move, no event line
        int evBefore = evMiss[assetvfs::kMissObserve];
        KT_CHECK_EQ(ob.handleGetKeyEntryMiss("zzz_other", vfsreg::ResTypeUtc, &ot, &oe), 0);
        KT_CHECK_EQ(evMiss[assetvfs::kMissObserve], evBefore);
        KT_CHECK_EQ(ob.counters.observe.load(), 3u);

        // static registry hit in observe: reported, not served
        const uint8_t sdata[] = { 's', 't' };
        KT_CHECK(ob.registerAsset("k1p_static", vfsreg::ResTypeUtc, sdata, sizeof(sdata), vfsreg::FlagResMan, false, "t") >= 0);
        ot = reinterpret_cast<void*>(0x1234);
        oe = reinterpret_cast<void*>(0x5678);
        KT_CHECK_EQ(ob.handleGetKeyEntryMiss("k1p_static", vfsreg::ResTypeUtc, &ot, &oe), 0);
        KT_CHECK(ot == nullptr);
        KT_CHECK(oe == reinterpret_cast<void*>(0x5678));

        // H2 in observe: returns 0, the CRes is never written, no allocation
        const uint32_t sPacked = vfsreg::makePackedId(static_cast<uint32_t>(ob.reg.findResMan("k1p_static", vfsreg::ResTypeUtc) - ob.reg.records));
        uint8_t obsRes[0x40] = {0};
        *reinterpret_cast<uint32_t*>(obsRes + assetvfs::kResOffPackedId) = sPacked;
        uint8_t obsResCopy[0x40];
        std::memcpy(obsResCopy, obsRes, sizeof(obsRes));
        g_allocCalls = 0;
        g_servicedCalls = 0;
        KT_CHECK_EQ(ob.handleServiceFromImage(obsRes, countingAlloc, testServiced), 0);
        KT_CHECK_EQ(std::memcmp(obsRes, obsResCopy, sizeof(obsRes)), 0);
        KT_CHECK_EQ(g_allocCalls, 0);
        KT_CHECK_EQ(g_servicedCalls, 0);
        KT_CHECK_EQ(evServe, 0);

        // H4 in observe: a stream-filter MEMORY claim is reported, never registered; Miles is never touched
        static const uint8_t wav[] = { 'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E' };
        auto filt = [](const char*, uint16_t, const void** d, uint32_t* n, char*, uint32_t, void*) -> int32_t {
            *d = wav;
            *n = sizeof(wav);
            return K2VFS_STREAM_MEMORY;
        };
        KT_CHECK(ob.registerStreamFilter(filt, nullptr, "flt") > 0);
        char spec[260];
        KT_CHECK(ob.resolveStreamSpec("sound\\k1p_line.wav", 4, spec, sizeof(spec) - 1, true));
        KT_CHECK(std::strcmp(spec, "observe") == 0);
        KT_CHECK(ob.reg.findStream("sound\\k1p_line.wav") == nullptr);
        resetMilesFakes();
        assetvfs::MilesChain och;
        och.bind(&ob, oursSet(), fakeMilesGet, fakeMilesSet, fakeMilesNotice);
        KT_CHECK(!och.ensure());
        KT_CHECK_EQ(g_milesSetCalls, 0);
        KT_CHECK(!och.installed);
        KT_CHECK(g_milesCur.sameAs(defaultsSet()));

        // ---- off: everything is inert ----
        assetvfs::EngineState off;
        off.applyConfig(assetvfs::VfsConfig::fromIni("off", 1, "k1p_", 0));
        KT_CHECK(!off.enabled);
        off.registerMissHandler(obsHandler, &octx, "mod");
        int callsOff = octx.calls;
        ot = reinterpret_cast<void*>(0x1234);
        KT_CHECK_EQ(off.handleGetKeyEntryMiss("k1p_obs", vfsreg::ResTypeUtc, &ot, &oe), 0);
        KT_CHECK_EQ(octx.calls, callsOff);
        KT_CHECK(off.missCannotHit(vfsreg::ResTypeUtc));
        resetMilesFakes();
        assetvfs::MilesChain offch;
        offch.bind(&off, oursSet(), fakeMilesGet, fakeMilesSet, fakeMilesNotice);
        KT_CHECK(!offch.ensure());
        KT_CHECK_EQ(g_milesSetCalls, 0);

        // ---- serve: events, counters, SERVE line ----
        assetvfs::EngineState sv;
        sv.applyConfig(assetvfs::VfsConfig::fromIni("serve", 1, "k1p_", 0));
        sv.missEventFn = missEv;
        sv.serveEventFn = serveEv;
        KT_CHECK(sv.enabled && !sv.observe);
        sv.registerMissHandler(obsHandler, &octx, "mod");
        std::memset(evMiss, 0, sizeof(evMiss));
        void* st = nullptr;
        void* se = nullptr;
        KT_CHECK_EQ(sv.handleGetKeyEntryMiss("k1p_obs", vfsreg::ResTypeUtc, &st, &se), 1);
        KT_CHECK_EQ(evMiss[assetvfs::kMissServe], 1);
        KT_CHECK_EQ(sv.counters.served.load(), 1u);
        uint8_t svRes[0x40] = {0};
        *reinterpret_cast<uint32_t*>(svRes + assetvfs::kResOffPackedId) = static_cast<vfsreg::KeyTableEntry*>(se)->packedId;
        g_servicedCalls = 0;
        KT_CHECK_EQ(sv.handleServiceFromImage(svRes, testAlloc, testServiced), 1);
        KT_CHECK_EQ(evServe, 1);
        // pass event (handler declines for a different resource)
        auto declineHandler = [](const char*, uint16_t, K2VfsAssetEntry*, void*) -> int32_t { return 0; };
        assetvfs::EngineState sp2;
        sp2.applyConfig(assetvfs::VfsConfig::fromIni("serve", 1, "k1p_", 0));
        sp2.missEventFn = missEv;
        sp2.registerMissHandler(declineHandler, nullptr, "d");
        std::memset(evMiss, 0, sizeof(evMiss));
        KT_CHECK_EQ(sp2.handleGetKeyEntryMiss("k1p_nope", vfsreg::ResTypeUtc, &st, &se), 0);
        KT_CHECK_EQ(evMiss[assetvfs::kMissPass], 1);
        sv.reset(); // frees the provider-owned copy
    }

    // H5 (Release): only VFS packed ids skip the engine's archive-source release; real archive ids never do.
    {
        KT_CHECK(assetvfs::skipEngineSourceRelease(vfsreg::makePackedId(0)));
        KT_CHECK(assetvfs::skipEngineSourceRelease(vfsreg::makePackedId(5)));
        KT_CHECK(!assetvfs::skipEngineSourceRelease(0x00000000u));
        KT_CHECK(!assetvfs::skipEngineSourceRelease(0x40010000u)); // a BIF-style id (>>30 == 1) that is not ours
        KT_CHECK(!assetvfs::skipEngineSourceRelease(0x80FC0000u));
        KT_CHECK(!assetvfs::skipEngineSourceRelease(0xFFFFFFFFu));
    }

    // Cleanup allocated memory
    for (void* p : g_allocatedPointers) {
        std::free(p);
    }
    g_allocatedPointers.clear();

    return kt::Report("asset_vfs_core_test");
}
