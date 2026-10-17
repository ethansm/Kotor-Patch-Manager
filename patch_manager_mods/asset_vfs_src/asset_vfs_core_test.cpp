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

    // Fallback direct name parsing
    uint32_t hDirect = 0;
    KT_CHECK(state.parseVfsStreamSpec("kvjv0001", &hDirect));
    KT_CHECK(state.cursors.isVfsHandle(hDirect));
    state.cursors.close(hDirect);

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

    // Case C: K2VFS_STREAM_REWRITE (2)
    streamCtx.returnCode = K2VFS_STREAM_REWRITE;
    streamCtx.rewriteTarget = "Custom/voice_pack_01.wav";

    std::memset(streamSpec, 0, sizeof(streamSpec));
    KT_CHECK(state.resolveStreamSpec("StreamVoice/original.wav", 8, streamSpec, sizeof(streamSpec)));
    KT_CHECK_EQ(streamCtx.calls, 3);
    KT_CHECK(std::strcmp(streamSpec, "Custom/voice_pack_01.wav") == 0);

    // Case D: Unregister stops callback
    state.unregisterStreamFilter(tokFilter);
    streamCtx.returnCode = K2VFS_STREAM_REWRITE;
    streamCtx.rewriteTarget = "Custom/should_not_reach.wav";
    std::memset(streamSpec, 0, sizeof(streamSpec));
    KT_CHECK(!state.resolveStreamSpec("StreamVoice/unregistered.wav", 8, streamSpec, sizeof(streamSpec)));
    KT_CHECK_EQ(streamCtx.calls, 3); // Unchanged

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
        KT_CHECK(state.parseVfsStreamSpec("snd_one", &hh));
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

    // Cleanup allocated memory
    for (void* p : g_allocatedPointers) {
        std::free(p);
    }
    g_allocatedPointers.clear();

    return 0;
}
