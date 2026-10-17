#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "VfsRegistry.h"
#include "TwoDaSplicer.h"
#include "CharacterProvider.h"

namespace assetvfs {

// CRes field offsets (Odyssey Engine 32-bit MSVC)
constexpr size_t kResOffVtbl       = 0x00;
constexpr size_t kResOffDemands    = 0x04;
constexpr size_t kResOffRequests   = 0x06;
constexpr size_t kResOffPackedId   = 0x08;
constexpr size_t kResOffStatus     = 0x0C;
constexpr size_t kResOffData       = 0x10;
constexpr size_t kResOffKeyEntry   = 0x14;
constexpr size_t kResOffSize       = 0x18;
constexpr size_t kRes2DaOffLoaded  = 0x28;

// Writable prefix bytes before m_pResource (CResMDL writes at -6, CRes2DA writes at -8)
constexpr size_t kWorkPrefixBytes  = 16;

struct TwoDaPatchTable {
    static constexpr size_t kMax2DaCells = 256;

    struct StoredCell {
        char table_name[16];
        uint32_t row_index;
        int32_t template_row;
        char column_name[32];
        char value[64];
        bool active;
    };

    StoredCell cells[kMax2DaCells] = {};
    uint32_t count = 0;

    void clear() {
        count = 0;
        std::memset(cells, 0, sizeof(cells));
    }

    int32_t addCells(const K2Vfs2DaCell* newCells, uint32_t numCells) {
        if (!newCells || numCells == 0) return 0;
        uint32_t added = 0;
        for (uint32_t i = 0; i < numCells; ++i) {
            const K2Vfs2DaCell& src = newCells[i];
            if (!src.table_name || !src.column_name) continue;

            char normTable[16] = {0};
            if (!vfsreg::normalizeResRef(src.table_name, normTable)) {
                size_t tlen = std::strlen(src.table_name);
                if (tlen > 15) tlen = 15;
                for (size_t k = 0; k < tlen; ++k) {
                    char c = src.table_name[k];
                    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
                    normTable[k] = c;
                }
            }

            bool updated = false;
            for (uint32_t j = 0; j < count; ++j) {
                if (cells[j].active && cells[j].row_index == src.row_index) {
                    char existingNorm[16] = {0};
                    vfsreg::normalizeResRef(cells[j].table_name, existingNorm);
                    if (std::memcmp(normTable, existingNorm, 16) == 0 &&
                        k2char::strEqualNoCase(cells[j].column_name, src.column_name)) {
                        cells[j].template_row = src.template_row;
                        std::snprintf(cells[j].value, sizeof(cells[j].value), "%s", src.value ? src.value : "");
                        updated = true;
                        added++;
                        break;
                    }
                }
            }
            if (updated) continue;

            if (count >= kMax2DaCells) break;

            StoredCell& dst = cells[count++];
            dst.active = true;
            dst.row_index = src.row_index;
            dst.template_row = src.template_row;
            std::snprintf(dst.table_name, sizeof(dst.table_name), "%s", src.table_name);
            std::snprintf(dst.column_name, sizeof(dst.column_name), "%s", src.column_name);
            std::snprintf(dst.value, sizeof(dst.value), "%s", src.value ? src.value : "");
            added++;
        }
        return static_cast<int32_t>(added);
    }

    uint32_t buildSpliced(const char* tableResRef16, const uint8_t* baseBuf, uint32_t baseSize, uint8_t* outBuf, uint32_t outCap) const {
        if (!tableResRef16 || !baseBuf || baseSize == 0) return 0;
        char targetNorm[16] = {0};
        if (!vfsreg::normalizeResRef(tableResRef16, targetNorm)) {
            return 0;
        }

        twoda::CellPatch patchBuf[kMax2DaCells];
        size_t numPatches = 0;
        int32_t templateRow = -1;

        for (uint32_t i = 0; i < count; ++i) {
            if (!cells[i].active) continue;
            char tableNorm[16] = {0};
            if (!vfsreg::normalizeResRef(cells[i].table_name, tableNorm)) continue;
            if (std::memcmp(targetNorm, tableNorm, 16) == 0) {
                patchBuf[numPatches].row = cells[i].row_index;
                patchBuf[numPatches].colName = cells[i].column_name;
                patchBuf[numPatches].value = cells[i].value;
                if (cells[i].template_row >= 0) {
                    templateRow = cells[i].template_row;
                }
                numPatches++;
            }
        }

        if (numPatches == 0) {
            return 0;
        }

        return twoda::spliceRows(baseBuf, baseSize, patchBuf, numPatches, outBuf, outCap, templateRow);
    }
};


// Notice codes passed to EngineState::noticeFn (the provider turns them into KP_WARN lines).
constexpr int kNoticeServedReject = 1; // what = resref, owner = registrant
constexpr int kNoticePoolFull     = 2; // what = "static" | "dynamic", owner = registrant
typedef void (*NoticeFn)(int code, const char* what, const char* owner);

struct EngineState {
    bool enabled = true;
    vfsreg::Lock lock;            // guards reg, cursors, twodaPatches, handler/filter lists, work buffers
    vfsreg::Registry reg;
    vfsreg::MilesCursorTable cursors;
    TwoDaPatchTable twodaPatches;
    k2char::Catalog chars;
    uint8_t* workAlloc[vfsreg::Registry::kMaxAssets] = {};
    uint32_t workCap[vfsreg::Registry::kMaxAssets] = {};

    static constexpr size_t kMaxMissHandlers = 16;
    static constexpr size_t kMaxStreamFilters = 16;

    struct MissHandlerEntry {
        K2VfsResManMissFn fn = nullptr;
        void* user = nullptr;
        char owner[32] = {0};
        int32_t token = 0;
        bool active = false;
    };

    struct StreamFilterEntry {
        K2VfsStreamFilterFn fn = nullptr;
        void* user = nullptr;
        char owner[32] = {0};
        int32_t token = 0;
        bool active = false;
    };

    MissHandlerEntry missHandlers[kMaxMissHandlers] = {};
    StreamFilterEntry streamFilters[kMaxStreamFilters] = {};
    int32_t nextToken = 1;

    NoticeFn noticeFn = nullptr;
    bool warnedFull[2] = {false, false}; // [0] static pool, [1] dynamic pool

    EngineState() {
        init();
    }

    void reset() {
        enabled = true;
        reg.clear();
        cursors.clear();
        twodaPatches.clear();
        chars.clear();
        for (size_t i = 0; i < vfsreg::Registry::kMaxAssets; ++i) {
            workAlloc[i] = nullptr;
            workCap[i] = 0;
        }
        for (size_t i = 0; i < kMaxMissHandlers; ++i) {
            missHandlers[i] = {};
        }
        for (size_t i = 0; i < kMaxStreamFilters; ++i) {
            streamFilters[i] = {};
        }
        nextToken = 1;
        warnedFull[0] = warnedFull[1] = false;
    }

    void init() {
        reset();
    }

    // ---- registry (locked) ------------------------------------------------------------------

    // Returns slot index (>= 0) or vfsreg::kRegInvalid / kRegServed / kRegPoolFull.
    // `dynamic` registrations (miss handler / stream filter) use the reserved top slots.
    int registerAsset(const char* resref, uint16_t resType, const void* data, uint32_t size,
                      uint16_t flags, bool dynamic, const char* owner) {
        int rc = vfsreg::kRegInvalid;
        bool firstFull = false;
        {
            vfsreg::LockGuard g(lock);
            rc = reg.registerAsset(resref, resType, data, size, flags, dynamic);
            if (rc == vfsreg::kRegPoolFull && !warnedFull[dynamic ? 1 : 0]) {
                warnedFull[dynamic ? 1 : 0] = true;
                firstFull = true;
            }
        }
        if (noticeFn) {
            if (rc == vfsreg::kRegServed) {
                noticeFn(kNoticeServedReject, resref ? resref : "", owner ? owner : "");
            } else if (firstFull) {
                noticeFn(kNoticePoolFull, dynamic ? "dynamic" : "static", owner ? owner : "");
            }
        }
        return rc;
    }

    bool hasAsset(const char* resref, uint16_t resType, uint16_t flags) {
        vfsreg::LockGuard g(lock);
        if (flags == vfsreg::FlagResMan) {
            return reg.findResMan(resref, resType) != nullptr;
        }
        if (flags == vfsreg::FlagMilesStream) {
            return reg.findStream(resref) != nullptr;
        }
        return reg.findResMan(resref, resType) != nullptr || reg.findStream(resref) != nullptr;
    }

    int32_t addTwoDaCells(const K2Vfs2DaCell* newCells, uint32_t numCells) {
        vfsreg::LockGuard g(lock);
        return twodaPatches.addCells(newCells, numCells);
    }

    uint32_t buildTwoDaSpliced(const char* tableResRef16, const uint8_t* baseBuf, uint32_t baseSize,
                               uint8_t* outBuf, uint32_t outCap) {
        vfsreg::LockGuard g(lock);
        return twodaPatches.buildSpliced(tableResRef16, baseBuf, baseSize, outBuf, outCap);
    }

    // ---- Miles cursor wrappers (locked) -----------------------------------------------------

    bool isVfsHandle(uint32_t h) {
        vfsreg::LockGuard g(lock);
        return cursors.isVfsHandle(h);
    }

    int32_t cursorSeek(uint32_t h, int32_t offset, uint32_t origin) {
        vfsreg::LockGuard g(lock);
        return cursors.seek(h, offset, origin);
    }

    uint32_t cursorRead(uint32_t h, void* dst, uint32_t bytes) {
        vfsreg::LockGuard g(lock);
        return cursors.read(h, dst, bytes);
    }

    bool cursorClose(uint32_t h) {
        vfsreg::LockGuard g(lock);
        return cursors.close(h);
    }

    // ---- handler / filter lists (locked) ----------------------------------------------------

    int32_t registerMissHandler(K2VfsResManMissFn fn, void* user, const char* owner) {
        if (!fn) return 0;
        vfsreg::LockGuard g(lock);
        for (size_t i = 0; i < kMaxMissHandlers; ++i) {
            if (!missHandlers[i].active) {
                int32_t tok = nextToken++;
                missHandlers[i].fn = fn;
                missHandlers[i].user = user;
                missHandlers[i].token = tok;
                missHandlers[i].active = true;
                if (owner) {
                    std::snprintf(missHandlers[i].owner, sizeof(missHandlers[i].owner), "%s", owner);
                } else {
                    missHandlers[i].owner[0] = '\0';
                }
                return tok;
            }
        }
        return 0;
    }

    void unregisterMissHandler(int32_t token) {
        if (token <= 0) return;
        vfsreg::LockGuard g(lock);
        for (size_t i = 0; i < kMaxMissHandlers; ++i) {
            if (missHandlers[i].active && missHandlers[i].token == token) {
                missHandlers[i] = {};
                return;
            }
        }
    }

    int32_t registerStreamFilter(K2VfsStreamFilterFn fn, void* user, const char* owner) {
        if (!fn) return 0;
        vfsreg::LockGuard g(lock);
        for (size_t i = 0; i < kMaxStreamFilters; ++i) {
            if (!streamFilters[i].active) {
                int32_t tok = nextToken++;
                streamFilters[i].fn = fn;
                streamFilters[i].user = user;
                streamFilters[i].token = tok;
                streamFilters[i].active = true;
                if (owner) {
                    std::snprintf(streamFilters[i].owner, sizeof(streamFilters[i].owner), "%s", owner);
                } else {
                    streamFilters[i].owner[0] = '\0';
                }
                return tok;
            }
        }
        return 0;
    }

    void unregisterStreamFilter(int32_t token) {
        if (token <= 0) return;
        vfsreg::LockGuard g(lock);
        for (size_t i = 0; i < kMaxStreamFilters; ++i) {
            if (streamFilters[i].active && streamFilters[i].token == token) {
                streamFilters[i] = {};
                return;
            }
        }
    }

    // ---- H1: CExoResMan::GetKeyEntry miss ---------------------------------------------------

    // Hook H1 uses skip_original_bytes = true, so the stolen `MOV DWORD [ecx],0` (zeroing *ppTable)
    // never runs. This function therefore owns that write: on EVERY path returning 0 it zeroes
    // *ppTable (when ppTable is non-null); on a hit (1) it sets *ppTable = fake table and *ppEntry.
    // The native tail at 0x712457 (reached on 0) still zeroes *ppEntry and EAX.
    int handleGetKeyEntryMiss(const char* rawResRef16, uint16_t resType, void** ppTable, void** ppEntry) {
        int rc = resolveKeyEntryMiss(rawResRef16, resType, ppTable, ppEntry);
        if (rc == 0 && ppTable) {
            *ppTable = nullptr;
        }
        return rc;
    }

    int resolveKeyEntryMiss(const char* rawResRef16, uint16_t resType, void** ppTable, void** ppEntry) {
        if (!enabled || !rawResRef16 || !ppTable || !ppEntry) {
            return 0;
        }

        struct HandlerSnap {
            K2VfsResManMissFn fn;
            void* user;
            char owner[32];
        } snap[kMaxMissHandlers];
        size_t nSnap = 0;
        {
            vfsreg::LockGuard g(lock);
            vfsreg::AssetRecord* rec = reg.findResMan(rawResRef16, resType);
            if (rec) {
                *ppTable = &reg.fakeTable;
                *ppEntry = &rec->keyEntry;
                return 1;
            }
            for (size_t i = 0; i < kMaxMissHandlers; ++i) {
                if (missHandlers[i].active && missHandlers[i].fn) {
                    snap[nSnap].fn = missHandlers[i].fn;
                    snap[nSnap].user = missHandlers[i].user;
                    std::memcpy(snap[nSnap].owner, missHandlers[i].owner, sizeof(snap[nSnap].owner));
                    nSnap++;
                }
            }
        }
        if (nSnap == 0) {
            return 0;
        }

        char rawCopy[17] = {0};
        for (size_t i = 0; i < 16; ++i) {
            char c = rawResRef16[i];
            rawCopy[i] = c;
            if (c == '\0') break;
        }

        char norm[17] = {0};
        if (!vfsreg::normalizeResRef(rawCopy, norm)) {
            return 0;
        }

        // Handlers run WITHOUT the lock held (they may call back into the VFS API).
        for (size_t i = 0; i < nSnap; ++i) {
            K2VfsAssetEntry outEntry = {};
            int32_t handled = snap[i].fn(norm, resType, &outEntry, snap[i].user);
            if (handled == 1 && outEntry.data != nullptr && outEntry.size > 0) {
                int idx = registerAsset(norm, resType, outEntry.data, outEntry.size,
                                        vfsreg::FlagResMan, true, snap[i].owner);
                if (idx >= 0) {
                    *ppTable = &reg.fakeTable;
                    *ppEntry = &reg.records[idx].keyEntry;
                    return 1;
                }
            }
        }

        return 0;
    }

    // ---- H2: CExoResMan::ServiceFromImage ---------------------------------------------------

    // Mirrors the native contract: returns 0 (do nothing, native runs) when the CRes already has
    // flag 4 set or is not a servable VFS record; otherwise fills the CRes and returns the result of
    // OnResourceServiced (callOnServiced) or 1 when no callee is given. Non-zero = consumed.
    int handleServiceFromImage(uint8_t* pRes, uint8_t* (*allocFn)(uint32_t), int (*callOnServiced)(void*)) {
        if (!enabled || !pRes) {
            return 0;
        }
        {
            vfsreg::LockGuard g(lock);
            uint32_t packedId = *reinterpret_cast<const uint32_t*>(pRes + kResOffPackedId);
            vfsreg::AssetRecord* rec = reg.findByPackedId(packedId);
            if (!rec || !rec->data || rec->size == 0) {
                return 0;
            }
            uint32_t idx = static_cast<uint32_t>(rec - reg.records);
            if (idx >= vfsreg::Registry::kMaxAssets) {
                return 0;
            }
            if ((*reinterpret_cast<const uint32_t*>(pRes + kResOffStatus) & 0x4u) != 0) {
                return 0; // already serviced: native would return 0, leave everything untouched
            }

            uint32_t neededSize = rec->size + static_cast<uint32_t>(kWorkPrefixBytes);
            if (!workAlloc[idx] || workCap[idx] < neededSize) {
                if (!allocFn) {
                    return 0;
                }
                workAlloc[idx] = allocFn(neededSize);
                if (!workAlloc[idx]) {
                    return 0;
                }
                workCap[idx] = neededSize;
            }
            std::memset(workAlloc[idx], 0, kWorkPrefixBytes);
            std::memcpy(workAlloc[idx] + kWorkPrefixBytes, rec->data, rec->size);

            *reinterpret_cast<uint32_t*>(pRes + kResOffSize) = rec->size;
            *reinterpret_cast<void**>(pRes + kResOffData) = workAlloc[idx] + kWorkPrefixBytes;
            *reinterpret_cast<uint32_t*>(pRes + kResOffStatus) |= 0x4;
            reg.markServed(idx);
        }

        // External callee: lock released. A 0 result is passed through (nothing is rolled back).
        return callOnServiced ? callOnServiced(pRes) : 1;
    }

    // ---- H4: streaming source open ----------------------------------------------------------

    bool resolveStreamSpec(const char* candidatePath, uint16_t streamKind, char* outSpec, size_t specCap) {
        if (!candidatePath || !outSpec || specCap == 0) {
            return false;
        }

        struct FilterSnap {
            K2VfsStreamFilterFn fn;
            void* user;
            char owner[32];
        } snap[kMaxStreamFilters];
        size_t nSnap = 0;
        {
            vfsreg::LockGuard g(lock);
            vfsreg::AssetRecord* rec = reg.findStream(candidatePath);
            if (rec && rec->data && rec->size > 0) {
                uint32_t idx = static_cast<uint32_t>(rec - reg.records);
                if (idx < vfsreg::Registry::kMaxAssets) {
                    vfsreg::AudioSlice slice = vfsreg::inspectAudioContainer(rec->data, rec->size, streamKind);
                    const char* ext = slice.ext ? slice.ext : ".wav";
                    int n = std::snprintf(outSpec, specCap, "#*VFS_%u%s", idx, ext);
                    return (n > 0 && static_cast<size_t>(n) < specCap);
                }
            }
            for (size_t i = 0; i < kMaxStreamFilters; ++i) {
                if (streamFilters[i].active && streamFilters[i].fn) {
                    snap[nSnap].fn = streamFilters[i].fn;
                    snap[nSnap].user = streamFilters[i].user;
                    std::memcpy(snap[nSnap].owner, streamFilters[i].owner, sizeof(snap[nSnap].owner));
                    nSnap++;
                }
            }
        }

        // Not found in the static registry AND at least one stream filter is active (called unlocked):
        for (size_t i = 0; i < nSnap; ++i) {
            const void* outData = nullptr;
            uint32_t outSize = 0;
            char rewritten[260] = {0};
            int32_t rc = snap[i].fn(
                candidatePath, streamKind, &outData, &outSize,
                rewritten, sizeof(rewritten), snap[i].user);

            if (rc == K2VFS_STREAM_MEMORY && outData != nullptr && outSize > 0) {
                int idx = registerAsset(candidatePath, vfsreg::ResTypeWav, outData, outSize,
                                        vfsreg::FlagMilesStream, true, snap[i].owner);
                if (idx < 0) {
                    char synStem[17] = {0};
                    std::snprintf(synStem, sizeof(synStem), "vfs_flt_%u", static_cast<unsigned>(i));
                    idx = registerAsset(synStem, vfsreg::ResTypeWav, outData, outSize,
                                        vfsreg::FlagMilesStream, true, snap[i].owner);
                }
                if (idx >= 0 && static_cast<size_t>(idx) < vfsreg::Registry::kMaxAssets) {
                    vfsreg::AudioSlice slice = vfsreg::inspectAudioContainer(
                        static_cast<const uint8_t*>(outData), outSize, streamKind);
                    const char* ext = slice.ext ? slice.ext : ".wav";
                    int n = std::snprintf(outSpec, specCap, "#*VFS_%u%s", static_cast<unsigned>(idx), ext);
                    return (n > 0 && static_cast<size_t>(n) < specCap);
                }
            } else if (rc == K2VFS_STREAM_REWRITE && rewritten[0] != '\0') {
                int n = std::snprintf(outSpec, specCap, "%s", rewritten);
                return (n > 0 && static_cast<size_t>(n) < specCap);
            }
        }

        return false;
    }

    bool parseVfsStreamSpec(const char* filename, uint32_t* outHandle) {
        if (!filename || !outHandle) {
            return false;
        }
        *outHandle = 0;

        vfsreg::LockGuard g(lock);
        const char* vfsTag = std::strstr(filename, "#*VFS_");
        if (vfsTag) {
            const char* numPtr = vfsTag + 6;
            char* endPtr = nullptr;
            unsigned long idx = std::strtoul(numPtr, &endPtr, 10);
            if (endPtr != numPtr && idx < vfsreg::Registry::kMaxAssets && reg.records[idx].active) {
                const vfsreg::AssetRecord& rec = reg.records[idx];
                vfsreg::AudioSlice slice = vfsreg::inspectAudioContainer(rec.data, rec.size, rec.keyEntry.resType);
                uint32_t h = cursors.open(slice.payload, slice.length);
                if (h != 0) {
                    reg.markServed(idx);
                    *outHandle = h;
                    return true;
                }
            }
            return false;
        }

        vfsreg::AssetRecord* rec = reg.findStream(filename);
        if (rec && rec->data && rec->size > 0) {
            vfsreg::AudioSlice slice = vfsreg::inspectAudioContainer(rec->data, rec->size, rec->keyEntry.resType);
            uint32_t h = cursors.open(slice.payload, slice.length);
            if (h != 0) {
                reg.markServed(static_cast<size_t>(rec - reg.records));
                *outHandle = h;
                return true;
            }
        }

        return false;
    }
};

} // namespace assetvfs
