#include "VfsRegistry.h"
#include "check.h"

int main() {
    // -------------------------------------------------------------
    // 1. Odyssey Resource Types & Flags
    // -------------------------------------------------------------
    KT_CHECK_EQ(vfsreg::ResTypeWav, 4u);
    KT_CHECK_EQ(vfsreg::ResTypeTga, 3u);
    KT_CHECK_EQ(vfsreg::ResTypeNcs, 2010u);
    KT_CHECK_EQ(vfsreg::ResType2da, 2017u);
    KT_CHECK_EQ(vfsreg::ResTypeTxi, 2022u);
    KT_CHECK_EQ(vfsreg::ResTypeUtc, 2027u);
    KT_CHECK_EQ(vfsreg::ResTypeDlg, 2029u);
    KT_CHECK_EQ(vfsreg::ResTypeSsf, 2060u);
    KT_CHECK_EQ(vfsreg::ResTypeMdl, 2002u);
    KT_CHECK_EQ(vfsreg::ResTypeTpc, 3007u);
    KT_CHECK_EQ(vfsreg::ResTypeMdX, 3008u);
    KT_CHECK_EQ(vfsreg::ResTypeLip, 3004u);

    // -------------------------------------------------------------
    // 2. Packed ID Encoding & Helpers
    // -------------------------------------------------------------
    KT_CHECK_EQ(vfsreg::kVfsPackedPrefix, 0x40FC0000u);
    KT_CHECK_EQ(vfsreg::kVfsIndexMask, 0x00003FFFu);

    KT_CHECK_EQ(vfsreg::makePackedId(0), 0x40FC0000u);
    KT_CHECK_EQ(vfsreg::makePackedId(15), 0x40FC000Fu);
    KT_CHECK_EQ(vfsreg::makePackedId(255), 0x40FC00FFu);

    KT_CHECK(vfsreg::isVfsPackedId(0x40FC0000u));
    KT_CHECK(vfsreg::isVfsPackedId(0x40FC0042u));
    KT_CHECK(vfsreg::isVfsPackedId(0x40FC3FFFu));
    KT_CHECK(!vfsreg::isVfsPackedId(0x40FB0000u));
    KT_CHECK(!vfsreg::isVfsPackedId(0x00712360u));
    KT_CHECK(!vfsreg::isVfsPackedId(0x80000000u));

    KT_CHECK_EQ(vfsreg::indexFromPackedId(vfsreg::makePackedId(0)), 0u);
    KT_CHECK_EQ(vfsreg::indexFromPackedId(vfsreg::makePackedId(123)), 123u);
    KT_CHECK_EQ(vfsreg::indexFromPackedId(vfsreg::makePackedId(0x3FFF)), 0x3FFFu);

    // -------------------------------------------------------------
    // 3. Exact 28-byte Struct Sizes & Field Offsets
    // -------------------------------------------------------------
    KT_CHECK_EQ(sizeof(vfsreg::KeyTableEntry), 0x1Cu);
    KT_CHECK_EQ(offsetof(vfsreg::KeyTableEntry, resref), 0x00u);
    KT_CHECK_EQ(offsetof(vfsreg::KeyTableEntry, pRes), 0x10u);
    KT_CHECK_EQ(offsetof(vfsreg::KeyTableEntry, packedId), 0x14u);
    KT_CHECK_EQ(offsetof(vfsreg::KeyTableEntry, refCount), 0x18u);
    KT_CHECK_EQ(offsetof(vfsreg::KeyTableEntry, resType), 0x1Au);

    KT_CHECK_EQ(sizeof(vfsreg::FakeKeyTable), 0x20u);
    KT_CHECK_EQ(offsetof(vfsreg::FakeKeyTable, tableType), 0x1Cu);

    // -------------------------------------------------------------
    // 4. normalizeResRef
    // -------------------------------------------------------------
    char norm[16];

    // Simple stem without directory or extension
    memset(norm, 0xFF, sizeof(norm));
    KT_CHECK(vfsreg::normalizeResRef("K1P_JuhaniH", norm));
    KT_CHECK_EQ(strcmp(norm, "k1p_juhanih"), 0);
    KT_CHECK_EQ(norm[11], 0);

    // Leading directory prefix with ':' and '\' plus .wav extension
    memset(norm, 0xFF, sizeof(norm));
    KT_CHECK(vfsreg::normalizeResRef("HD0:STREAMVOICE\\kvjv0001.wav", norm));
    KT_CHECK_EQ(strcmp(norm, "kvjv0001"), 0);
    KT_CHECK_EQ(norm[8], 0);

    // Leading directory with './' and '\' plus .wav extension
    memset(norm, 0xFF, sizeof(norm));
    KT_CHECK(vfsreg::normalizeResRef(".\\streamsounds\\p_juhani_atk1.wav", norm));
    KT_CHECK_EQ(strcmp(norm, "p_juhani_atk1"), 0);

    // Forward slashes and .mp3 extension
    memset(norm, 0xFF, sizeof(norm));
    KT_CHECK(vfsreg::normalizeResRef("streamsounds/combat_bark1.mp3", norm));
    KT_CHECK_EQ(strcmp(norm, "combat_bark1"), 0);

    // Exactly 16 characters stem (boundary check)
    memset(norm, 0xFF, sizeof(norm));
    KT_CHECK(vfsreg::normalizeResRef("1234567890abcdef", norm));
    KT_CHECK_EQ(memcmp(norm, "1234567890abcdef", 16), 0);

    // 16 chars with extension
    memset(norm, 0xFF, sizeof(norm));
    KT_CHECK(vfsreg::normalizeResRef("path/1234567890abcdef.mdl", norm));
    KT_CHECK_EQ(memcmp(norm, "1234567890abcdef", 16), 0);

    // 17 characters stem (must reject)
    KT_CHECK(!vfsreg::normalizeResRef("1234567890abcdefg", norm));
    KT_CHECK(!vfsreg::normalizeResRef("1234567890abcdefg.wav", norm));

    // Invalid character rejections
    KT_CHECK(!vfsreg::normalizeResRef("k1p-juhani", norm));
    KT_CHECK(!vfsreg::normalizeResRef("k1p juhani", norm));
    KT_CHECK(!vfsreg::normalizeResRef("k1p@juhani", norm));
    KT_CHECK(!vfsreg::normalizeResRef("k1p!juhani", norm));
    KT_CHECK(!vfsreg::normalizeResRef("", norm));
    KT_CHECK(!vfsreg::normalizeResRef(nullptr, norm));
    KT_CHECK(!vfsreg::normalizeResRef(".wav", norm));
    KT_CHECK(!vfsreg::normalizeResRef("folder/", norm));
    KT_CHECK(!vfsreg::normalizeResRef("folder\\", norm));

    // -------------------------------------------------------------
    // 5. Registry Operations
    // -------------------------------------------------------------
    vfsreg::Registry reg;
    KT_CHECK_EQ(reg.fakeTable.tableType, 4u);

    const uint8_t mockUtc[] = { 0x55, 0x66, 0x77, 0x88 };
    const uint8_t mockMdl[] = { 0x11, 0x22, 0x33 };

    // TGA rejection for ResMan
    int rejTga = reg.registerAsset("face_tex", vfsreg::ResTypeTga, mockUtc, sizeof(mockUtc), vfsreg::FlagResMan);
    KT_CHECK_EQ(rejTga, -1);

    // TPC accepted
    int tpcSlot = reg.registerAsset("face_tex", vfsreg::ResTypeTpc, mockUtc, sizeof(mockUtc), vfsreg::FlagResMan);
    KT_CHECK(tpcSlot >= 0);

    // Register ResMan asset
    int utcSlot = reg.registerAsset("k1p_juhani", vfsreg::ResTypeUtc, mockUtc, sizeof(mockUtc), vfsreg::FlagResMan);
    KT_CHECK(utcSlot >= 0);
    KT_CHECK(utcSlot != tpcSlot);
    KT_CHECK_EQ(reg.records[utcSlot].active, true);
    KT_CHECK_EQ(reg.records[utcSlot].size, (uint32_t)sizeof(mockUtc));
    KT_CHECK(reg.records[utcSlot].data == mockUtc);
    KT_CHECK_EQ(reg.records[utcSlot].flags, vfsreg::FlagResMan);
    KT_CHECK_EQ(reg.records[utcSlot].keyEntry.packedId, vfsreg::makePackedId((uint32_t)utcSlot));
    KT_CHECK_EQ(reg.records[utcSlot].keyEntry.resType, vfsreg::ResTypeUtc);
    KT_CHECK_EQ(strcmp(reg.records[utcSlot].keyEntry.resref, "k1p_juhani"), 0);

    // Duplicate update in-place
    const uint8_t mockUtcUpdated[] = { 0x99, 0xAA };
    int dupSlot = reg.registerAsset("K1P_JUHANI", vfsreg::ResTypeUtc, mockUtcUpdated, sizeof(mockUtcUpdated), vfsreg::FlagResMan);
    KT_CHECK_EQ(dupSlot, utcSlot);
    KT_CHECK(reg.records[utcSlot].data == mockUtcUpdated);
    KT_CHECK_EQ(reg.records[utcSlot].size, (uint32_t)sizeof(mockUtcUpdated));

    // Register 16-character stem
    int mdlSlot = reg.registerAsset("1234567890abcdef", vfsreg::ResTypeMdl, mockMdl, sizeof(mockMdl), vfsreg::FlagResMan);
    KT_CHECK(mdlSlot >= 0);

    // Find ResMan - standard case-insensitive
    vfsreg::AssetRecord* found = reg.findResMan("k1p_juhani", vfsreg::ResTypeUtc);
    KT_CHECK(found != nullptr);
    KT_CHECK(found == &reg.records[utcSlot]);

    found = reg.findResMan("K1P_JUHANI", vfsreg::ResTypeUtc);
    KT_CHECK(found != nullptr);
    KT_CHECK(found == &reg.records[utcSlot]);

    // Find ResMan - 16-char non-null-terminated CResRef buffer
    char rawCResRef16[16];
    memcpy(rawCResRef16, "1234567890ABCDEF", 16);
    found = reg.findResMan(rawCResRef16, vfsreg::ResTypeMdl);
    KT_CHECK(found != nullptr);
    KT_CHECK(found == &reg.records[mdlSlot]);

    // Miss passthrough on wrong resType or missing name
    KT_CHECK(reg.findResMan(rawCResRef16, vfsreg::ResTypeUtc) == nullptr);
    KT_CHECK(reg.findResMan("nonexistent", vfsreg::ResTypeUtc) == nullptr);

    // Find by packedId
    uint32_t validPacked = reg.records[utcSlot].keyEntry.packedId;
    KT_CHECK(reg.findByPackedId(validPacked) == &reg.records[utcSlot]);
    KT_CHECK(reg.findByPackedId(0x12345678u) == nullptr); // Bad prefix
    KT_CHECK(reg.findByPackedId(vfsreg::makePackedId(200)) == nullptr); // Inactive slot

    // Register Stream asset (flags = 1)
    const uint8_t mockWav[] = { 0x12, 0x34, 0x56 };
    int streamSlot = reg.registerAsset("HD0:STREAMVOICE\\kvjv0001.wav", vfsreg::ResTypeWav, mockWav, sizeof(mockWav), vfsreg::FlagMilesStream);
    KT_CHECK(streamSlot >= 0);
    KT_CHECK_EQ(reg.records[streamSlot].flags, vfsreg::FlagMilesStream);

    // Find Stream
    vfsreg::AssetRecord* streamFound = reg.findStream("kvjv0001");
    KT_CHECK(streamFound != nullptr);
    KT_CHECK(streamFound == &reg.records[streamSlot]);

    streamFound = reg.findStream("HD0:STREAMVOICE\\KVJV0001.WAV");
    KT_CHECK(streamFound != nullptr);
    KT_CHECK(streamFound == &reg.records[streamSlot]);

    KT_CHECK(reg.findStream("missing_bark.wav") == nullptr);

    // -------------------------------------------------------------
    // 6. inspectAudioContainer
    // -------------------------------------------------------------
    // 58-byte VO header + MP3 payload
    uint8_t voHeaderBuf[68];
    memset(voHeaderBuf, 0, sizeof(voHeaderBuf));
    memcpy(voHeaderBuf, "RIFF", 4);
    uint32_t riff50 = 50;
    memcpy(voHeaderBuf + 4, &riff50, 4);
    voHeaderBuf[58] = 0xFF; // MP3 sync word
    voHeaderBuf[59] = 0xFB;

    vfsreg::AudioSlice sliceVo = vfsreg::inspectAudioContainer(voHeaderBuf, sizeof(voHeaderBuf), 0);
    KT_CHECK(sliceVo.payload == voHeaderBuf + 58);
    KT_CHECK_EQ(sliceVo.length, 10u);
    KT_CHECK_EQ(sliceVo.offset, 58u);
    KT_CHECK_EQ(strcmp(sliceVo.ext, ".mp3"), 0);

    // 470-byte (0x1D6) SFX prefix + RIFF WAV
    uint8_t sfxHeaderBuf[0x1D6 + 24];
    memset(sfxHeaderBuf, 0xCD, sizeof(sfxHeaderBuf));
    memcpy(sfxHeaderBuf + 0x1D6, "RIFF", 4);

    vfsreg::AudioSlice sliceSfx = vfsreg::inspectAudioContainer(sfxHeaderBuf, sizeof(sfxHeaderBuf), 0);
    KT_CHECK(sliceSfx.payload == sfxHeaderBuf + 0x1D6);
    KT_CHECK_EQ(sliceSfx.length, 24u);
    KT_CHECK_EQ(sliceSfx.offset, 0x1D6u);
    KT_CHECK_EQ(strcmp(sliceSfx.ext, ".wav"), 0);

    // Layer 1 (FlagResMan) must reject 58-byte VO MP3 and 470-byte SFX containers,
    // while Layer 3 (FlagMilesStream) accepts them and Layer 1 accepts bare RIFF WAVE PCM
    KT_CHECK_EQ(reg.registerAsset("vo_on_resman", vfsreg::ResTypeWav, voHeaderBuf, sizeof(voHeaderBuf), vfsreg::FlagResMan), -1);
    KT_CHECK_EQ(reg.registerAsset("sfx_on_resman", vfsreg::ResTypeWav, sfxHeaderBuf, sizeof(sfxHeaderBuf), vfsreg::FlagResMan), -1);
    KT_CHECK(reg.registerAsset("vo_on_stream", vfsreg::ResTypeWav, voHeaderBuf, sizeof(voHeaderBuf), vfsreg::FlagMilesStream) >= 0);
    KT_CHECK(reg.registerAsset("sfx_on_stream", vfsreg::ResTypeWav, sfxHeaderBuf, sizeof(sfxHeaderBuf), vfsreg::FlagMilesStream) >= 0);
    const uint8_t bareRiffPcm[] = "RIFF\x24\x00\x00\x00WAVEfmt \x10\x00\x00\x00\x01\x00\x01\x00";
    KT_CHECK(reg.registerAsset("pcm_on_resman", vfsreg::ResTypeWav, bareRiffPcm, sizeof(bareRiffPcm), vfsreg::FlagResMan) >= 0);

    // Raw buffers
    uint8_t rawAudio[32];
    memset(rawAudio, 0x77, sizeof(rawAudio));

    vfsreg::AudioSlice sliceRawMp3 = vfsreg::inspectAudioContainer(rawAudio, sizeof(rawAudio), 8);
    KT_CHECK(sliceRawMp3.payload == rawAudio);
    KT_CHECK_EQ(sliceRawMp3.length, 32u);
    KT_CHECK_EQ(sliceRawMp3.offset, 0u);
    KT_CHECK_EQ(strcmp(sliceRawMp3.ext, ".mp3"), 0);

    vfsreg::AudioSlice sliceRawWav = vfsreg::inspectAudioContainer(rawAudio, sizeof(rawAudio), 0);
    KT_CHECK(sliceRawWav.payload == rawAudio);
    KT_CHECK_EQ(sliceRawWav.length, 32u);
    KT_CHECK_EQ(sliceRawWav.offset, 0u);
    KT_CHECK_EQ(strcmp(sliceRawWav.ext, ".wav"), 0);

    // -------------------------------------------------------------
    // 7. MilesCursorTable Operations
    // -------------------------------------------------------------
    vfsreg::MilesCursorTable cursors;
    const uint8_t testPayload[30] = "0123456789abcdefghijklmnopqr";

    // Invalid open
    KT_CHECK_EQ(cursors.open(nullptr, 30), 0u);

    // Valid open
    uint32_t h1 = cursors.open(testPayload, 30);
    KT_CHECK(h1 != 0u);
    KT_CHECK(vfsreg::MilesCursorTable::isVfsHandle(h1));
    KT_CHECK(!vfsreg::MilesCursorTable::isVfsHandle(0u));
    KT_CHECK(!vfsreg::MilesCursorTable::isVfsHandle(0x7FFFFFFFu));

    // Seek begin (origin 0)
    KT_CHECK_EQ(cursors.seek(h1, 5, 0), 5);
    KT_CHECK_EQ(cursors.seek(h1, -10, 0), 0); // clamp min
    KT_CHECK_EQ(cursors.seek(h1, 100, 0), 30); // clamp max

    // Seek cur (origin 1)
    cursors.seek(h1, 10, 0);
    KT_CHECK_EQ(cursors.seek(h1, 5, 1), 15);
    KT_CHECK_EQ(cursors.seek(h1, -25, 1), 0); // clamp min

    // Seek end (origin 2)
    KT_CHECK_EQ(cursors.seek(h1, -4, 2), 26);
    KT_CHECK_EQ(cursors.seek(h1, 10, 2), 30); // clamp max
    KT_CHECK_EQ(cursors.seek(h1, 0, 99), -1); // invalid origin

    // Read partial
    cursors.seek(h1, 0, 0);
    char outBuf[16] = {0};
    uint32_t bytesRead = cursors.read(h1, outBuf, 10);
    KT_CHECK_EQ(bytesRead, 10u);
    KT_CHECK_EQ(memcmp(outBuf, "0123456789", 10), 0);

    // Read to EOF
    char restBuf[32] = {0};
    bytesRead = cursors.read(h1, restBuf, 32);
    KT_CHECK_EQ(bytesRead, 20u);
    KT_CHECK_EQ(memcmp(restBuf, "abcdefghijklmnopqr", 18), 0);

    // Read at EOF returns 0
    bytesRead = cursors.read(h1, restBuf, 10);
    KT_CHECK_EQ(bytesRead, 0u);

    // Close
    KT_CHECK(cursors.close(h1));
    KT_CHECK(!cursors.close(h1)); // double close
    KT_CHECK_EQ(cursors.read(h1, outBuf, 5), 0u);
    KT_CHECK_EQ(cursors.seek(h1, 0, 0), -1);

    // Open up to capacity (kMaxCursors = 16)
    uint32_t handles[16];
    for (int i = 0; i < 16; ++i) {
        handles[i] = cursors.open(testPayload, 30);
        KT_CHECK(handles[i] != 0u);
    }
    // 17th open should fail (full)
    KT_CHECK_EQ(cursors.open(testPayload, 30), 0u);

    // Close slot 7 and reopen
    KT_CHECK(cursors.close(handles[7]));
    uint32_t reusedH = cursors.open(testPayload, 30);
    KT_CHECK(reusedH != 0u);
    KT_CHECK_EQ(reusedH, handles[7]);

    return kt::Report("vfs_registry_test");
}
