// NpcDivLog.h -- pure log formatters, counters and small sets for the npc-diversity debug log.
// Spec: .claude/review/npc_div/E_logging.md 4.2 (line grammar), 4.4 (counters), 4.5 (rate limits).
// Pure: no <windows.h>, no KpLog, no engine access. Hook code calls these and writes the lines.
//
// Conventions shared by every formatter:
//   - Output is at most 231 bytes plus NUL (cap is clamped to kLineMax = 232). cap 0 writes nothing.
//   - Return value: stored length without the NUL (clamped to cap-1 on truncation), not snprintf's would-be length.
//   - Hex is lowercase (%08x). Every string passes SanitizeInto: bytes outside 0x21..0x7E (space included) become '?'.
//   - String bounds: tag 32, cur 16 (resref size), hook/pre/why/new 16, where 8, sha 64, want/got 64.
//   - x/y print as %.1f. The caller passes their bit patterns separately (xb, yb).
//   - Counts is per session. Hooks run on the game thread only, so plain uint32 is enough.
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>

namespace NpcDiversity {
namespace log {

constexpr size_t kLineMax = 232;
constexpr uint32_t kSlotCount = 64;
constexpr uint32_t kAppCount = 1024;
constexpr uint32_t kSiteCount = 12;

namespace detail {

inline size_t ClampCap(size_t cap) { return cap < kLineMax ? cap : kLineMax; }

// Maps snprintf's result to the stored length. cap is >= 1 here.
inline int Stored(char* out, size_t cap, int r) {
    if (r < 0) {
        out[0] = '\0';
        return 0;
    }
    const size_t n = static_cast<size_t>(r);
    return static_cast<int>(n < cap ? n : cap - 1);
}

inline size_t BoundedLen(const char* s, size_t max) {
    size_t n = 0;
    if (!s) return 0;
    while (n < max && s[n] != '\0') ++n;
    return n;
}

// Murmur3 finaliser. Spreads keys over U32Set buckets.
inline uint32_t Fmix(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85EBCA6Bu;
    h ^= h >> 13;
    h *= 0xC2B2AE35u;
    h ^= h >> 16;
    return h;
}

// Appends " <idx>:<val>" (idx as %02u when pad2) at out[len]. Returns the new length.
inline size_t AppendEntry(char* out, size_t cap, size_t len, uint32_t idx, uint32_t val, bool pad2) {
    if (len + 1 >= cap) return len;
    const int r = pad2 ? std::snprintf(out + len, cap - len, " %02u:%u", idx, val)
                       : std::snprintf(out + len, cap - len, " %u:%u", idx, val);
    if (r < 0) return len;
    const size_t n = static_cast<size_t>(r);
    return len + (n < cap - len ? n : cap - len - 1);
}

// Appends a pre-formatted tail (with its leading space) at out[len]; truncates to cap-1. Returns the new length.
inline size_t AppendTail(char* out, size_t cap, size_t len, const char* tail) {
    if (len + 1 >= cap) return len;
    const int r = std::snprintf(out + len, cap - len, "%s", tail);
    if (r < 0) return len;
    const size_t n = static_cast<size_t>(r);
    return len + (n < cap - len ? n : cap - len - 1);
}

// Lowercase hex of n bytes. dst needs 2*n+1 bytes.
inline void HexInto(char* dst, const uint8_t* src, size_t n) {
    static const char kDigits[] = "0123456789abcdef";
    for (size_t i = 0; i < n; ++i) {
        dst[2 * i] = kDigits[src[i] >> 4];
        dst[2 * i + 1] = kDigits[src[i] & 0x0Fu];
    }
    dst[2 * n] = '\0';
}

}  // namespace detail

// Copies min(n, cap-1) bytes of src to dst. Every byte outside 0x21..0x7E becomes '?' (space too,
// so fields stay whitespace-delimited). NUL-terminates when cap > 0. Returns the bytes written.
inline size_t SanitizeInto(char* dst, size_t cap, const char* src, size_t n) {
    if (cap == 0) return 0;
    if (!src) n = 0;
    size_t i = 0;
    for (; i < n && i + 1 < cap; ++i) {
        const unsigned char c = static_cast<unsigned char>(src[i]);
        dst[i] = (c >= 0x21u && c <= 0x7Eu) ? static_cast<char>(c) : '?';
    }
    dst[i] = '\0';
    return i;
}

// Sanitised copy of a NUL-terminated string, at most max bytes (max <= CAP-1). Returns buf.
template <size_t CAP>
inline const char* SafeStr(char (&buf)[CAP], const char* s, size_t max) {
    SanitizeInto(buf, CAP, s, detail::BoundedLen(s, max));
    return buf;
}

// Sanitised copy of a (ptr, len) string, at most max bytes (max <= CAP-1). Returns buf.
template <size_t CAP>
inline const char* SafeMem(char (&buf)[CAP], const char* s, size_t len, size_t max) {
    SanitizeInto(buf, CAP, s, len < max ? len : max);
    return buf;
}

// Counters for one session (spec 4.4). Call reset() once before first use.
struct Counts {
    uint32_t texCalls;
    uint32_t texNullGuard;
    uint32_t texUnread;
    uint32_t texBodySkip;
    uint32_t texNonCommoner;
    uint32_t texCommoner;
    uint32_t texSwap;
    uint32_t texSkipRow;
    uint32_t texStashHit;
    uint32_t texStashMiss;

    uint32_t remCalls;
    uint32_t remNullGuard;
    uint32_t remUnread;
    uint32_t remBodySkip;
    uint32_t remExcluded;
    uint32_t remTagUnread;
    uint32_t remChanged;
    uint32_t remTvChg;
    uint32_t remNoChange;
    uint32_t remSkipRow;
    uint32_t remSkipGender;
    uint32_t remTvArmor;  // tv write withheld: pDesc.subType >= 3 (armor body path)
    uint32_t remVary;     // creature passed the keep-default and vary gates (0.1.4)
    uint32_t remNoVary;   // left vanilla by the ShouldVary roll (why=skip:vary)
    uint32_t remKeep;     // left vanilla by the keep-default tag/position lists (why=keep)

    uint32_t abiBadApp;
    uint32_t abiBadGender;
    uint32_t abiBadDesc14;

    uint32_t fail[kSiteCount];  // by SafeRead site; the STAT read line prints sites 1..10
    uint32_t slot[2][kSlotCount];  // [gender][slot], swaps only
    uint32_t appTex[kAppCount];
    uint32_t appRem[kAppCount];
    uint32_t appOvf;  // single overflow counter for both app arrays

    uint32_t frDistinct;
    uint32_t retDistinct;
    uint32_t oidDistinct;

    float xMin, xMax, yMin, yMax;  // reset() sets min to +inf and max to -inf

    void reset() {
        *this = Counts{};
        const float inf = std::numeric_limits<float>::infinity();
        xMin = yMin = inf;
        xMax = yMax = -inf;
    }
};

// Open-addressing set of uint32 keys, linear probing. N must be a power of two. Key 0 is a normal key.
// insert() never loops forever: a new key that finds no free slot counts in ovf.
template <size_t N>
struct U32Set {
    static_assert(N >= 2 && (N & (N - 1)) == 0, "U32Set: N must be a power of two");
    uint32_t keys[N] = {};
    bool used[N] = {};
    uint32_t count = 0;
    uint32_t ovf = 0;

    void reset() {
        for (size_t i = 0; i < N; ++i) {
            keys[i] = 0;
            used[i] = false;
        }
        count = 0;
        ovf = 0;
    }

    // True when k was absent and is now stored. False when already present, or when full (ovf++).
    bool insert(uint32_t k) {
        const size_t mask = N - 1;
        size_t i = detail::Fmix(k) & mask;
        for (size_t probe = 0; probe < N; ++probe) {
            if (!used[i]) {
                used[i] = true;
                keys[i] = k;
                ++count;
                return true;
            }
            if (keys[i] == k) return false;
            i = (i + 1) & mask;
        }
        ++ovf;
        return false;
    }
};

// Call-count rate limit (not time). True for the first `first` calls, then once per `every` calls:
// calls first+every, first+2*every, ... (with first=300, every=500: 1..300, 800, 1300, ...).
struct Budget {
    uint32_t n = 0;

    bool take(uint32_t first, uint32_t every) {
        if (n != std::numeric_limits<uint32_t>::max()) ++n;
        if (n <= first) return true;
        if (every == 0) return false;
        return (n - first) % every == 0;
    }
};

// ---- Line formatters (spec 4.2). Keys and field order follow the grammar. ----

inline int FormatBanner0(char* out, size_t cap, const char* version, int kpl, const char* sha,
                         uint32_t base, uint32_t ts) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char v[33];
    char s[65];
    const int r = std::snprintf(out, cap, "BANNER v=%s kpl=%d sha=%s base=%08x ts=%08x",
                                SafeStr(v, version, 32), kpl ? 1 : 0, SafeStr(s, sha, 64), base, ts);
    return detail::Stored(out, cap, r);
}

inline int FormatBannerHook(char* out, size_t cap, const char* hook, uint32_t addr, const char* pre) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char h[17];
    char p[17];
    const int r = std::snprintf(out, cap, "BANNER hook=%s addr=%08x pre=%s", SafeStr(h, hook, 16), addr,
                                SafeStr(p, pre, 16));
    return detail::Stored(out, cap, r);
}

inline int FormatBannerRows(char* out, size_t cap, uint32_t appLo, uint32_t appHi, uint32_t appN,
                            uint32_t headLo, uint32_t headHi, uint32_t headN, uint32_t appCount,
                            uint32_t headCount) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap, "BANNER rows app=%u-%u n=%u head=%u-%u n=%u appcount=%u headcount=%u",
                                appLo, appHi, appN, headLo, headHi, headN, appCount, headCount);
    return detail::Stored(out, cap, r);
}

// cur is the 16-byte resref. newFlag is a short token chosen by the caller (for example "1" or "0").
inline int FormatTex(char* out, size_t cap, uint32_t pd, uint32_t fr, uint32_t ret, uint32_t app, uint32_t g,
                     uint32_t tv, int dm, int hd, uint32_t ph, const char* cur, size_t curLen, uint32_t h,
                     int st, uint32_t slot, const char* newFlag, int sw) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char cu[17];
    char nf[9];
    const int r = std::snprintf(out, cap,
                                "TEX pd=%08x fr=%08x ret=%08x app=%u g=%u tv=%u dm=%d hd=%d ph=%u cur=%s "
                                "h=%08x st=%d slot=%02u new=%s sw=%d",
                                pd, fr, ret, app, g, tv, dm, hd, ph, SafeMem(cu, cur, curLen, 16), h, st, slot,
                                SafeStr(nf, newFlag, 8), sw);
    return detail::Stored(out, cap, r);
}


// 0.1.3 grammar: the TEX line plus the descriptor subType and the creature oid (0 = oid unknown for this pDesc).
inline int FormatTexExt(char* out, size_t cap, uint32_t pd, uint32_t fr, uint32_t ret, uint32_t app, uint32_t g,
                        uint32_t tv, int dm, int hd, uint32_t ph, const char* cur, size_t curLen, uint32_t h,
                        int st, uint32_t slot, const char* newFlag, int sw, int stype, uint32_t oid) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int base = FormatTex(out, cap, pd, fr, ret, app, g, tv, dm, hd, ph, cur, curLen, h, st, slot, newFlag, sw);
    char tail[40];
    std::snprintf(tail, sizeof tail, " stype=%d oid=%08x", stype, oid);
    return static_cast<int>(detail::AppendTail(out, cap, static_cast<size_t>(base), tail));
}

// why: ok, nochg, excl:comp, excl:story, excl:notag, skip:dm, skip:gender, skip:row,
//      unread:desc, unread:tag, unread:oid, unread:xy, tvarmor (0.1.3), keep, skip:vary (0.1.4).
//      skip:dm is no longer produced; it stays in this list.
// xb and yb are the float bit patterns of x and y. app and tv print as before>after (tv right after app).
inline int FormatRmp(char* out, size_t cap, uint32_t self, uint32_t pd, uint32_t oid, uint32_t tp, int tl,
                     const char* tag, size_t tagLen, float x, float y, uint32_t xb, uint32_t yb,
                     uint32_t appBefore, uint32_t appAfter, uint32_t tvBefore, uint32_t tvAfter, uint32_t h,
                     const char* why) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char tg[33];
    char wy[17];
    const int r = std::snprintf(out, cap,
                                "RMP this=%08x pd=%08x oid=%08x tp=%08x tl=%d tag=%s xy=%.1f,%.1f "
                                "xyb=%08x,%08x app=%u>%u tv=%u>%u h=%08x why=%s",
                                self, pd, oid, tp, tl, SafeMem(tg, tag, tagLen, 32), x, y, xb, yb, appBefore,
                                appAfter, tvBefore, tvAfter, h, SafeStr(wy, why, 16));
    return detail::Stored(out, cap, r);
}

// 0.1.3 grammar: the RMP line plus the descriptor subType (stype) the tv guard looked at.
inline int FormatRmpExt(char* out, size_t cap, uint32_t self, uint32_t pd, uint32_t oid, uint32_t tp, int tl,
                        const char* tag, size_t tagLen, float x, float y, uint32_t xb, uint32_t yb,
                        uint32_t appBefore, uint32_t appAfter, uint32_t tvBefore, uint32_t tvAfter, uint32_t h,
                        const char* why, int stype) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int base = FormatRmp(out, cap, self, pd, oid, tp, tl, tag, tagLen, x, y, xb, yb, appBefore, appAfter,
                               tvBefore, tvAfter, h, why);
    char tail[24];
    std::snprintf(tail, sizeof tail, " stype=%d", stype);
    return static_cast<int>(detail::AppendTail(out, cap, static_cast<size_t>(base), tail));
}

inline int FormatCenTex(char* out, size_t cap, uint32_t app, uint32_t g, uint32_t tv, int dm, const char* cur,
                        size_t curLen) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char cu[17];
    const int r = std::snprintf(out, cap, "CEN k=tex app=%u g=%u tv=%u dm=%d cur=%s", app, g, tv, dm,
                                SafeMem(cu, cur, curLen, 16));
    return detail::Stored(out, cap, r);
}

inline int FormatCenRemap(char* out, size_t cap, uint32_t app, uint32_t g, uint32_t tv, const char* tag,
                          size_t tagLen) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char tg[33];
    const int r = std::snprintf(out, cap, "CEN k=remap app=%u g=%u tv=%u tag=%s", app, g, tv,
                                SafeMem(tg, tag, tagLen, 32));
    return detail::Stored(out, cap, r);
}

// ---- STAT lines (spec 4.3 periodic block). The extra skiprow/stash/skipg keys are appended at the end. ----

inline int FormatStatTex(char* out, size_t cap, const Counts& c) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap,
                                "STAT tex calls=%u nullg=%u unread=%u bcskip=%u nonc=%u comm=%u swap=%u "
                                "frd=%u retd=%u skiprow=%u stashhit=%u stashmiss=%u",
                                c.texCalls, c.texNullGuard, c.texUnread, c.texBodySkip, c.texNonCommoner,
                                c.texCommoner, c.texSwap, c.frDistinct, c.retDistinct, c.texSkipRow,
                                c.texStashHit, c.texStashMiss);
    return detail::Stored(out, cap, r);
}

inline int FormatStatRemap(char* out, size_t cap, const Counts& c) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap,
                                "STAT remap calls=%u nullg=%u unread=%u bcskip=%u excl=%u tagun=%u chg=%u "
                                "nochg=%u oidd=%u skiprow=%u skipg=%u tvchg=%u tvarmor=%u vary=%u novary=%u keep=%u",
                                c.remCalls, c.remNullGuard, c.remUnread, c.remBodySkip, c.remExcluded,
                                c.remTagUnread, c.remChanged, c.remNoChange, c.oidDistinct, c.remSkipRow,
                                c.remSkipGender, c.remTvChg, c.remTvArmor, c.remVary, c.remNoVary, c.remKeep);
    return detail::Stored(out, cap, r);
}

inline int FormatStatAbi(char* out, size_t cap, const Counts& c) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap, "STAT abi app=%u g=%u dm=%u", c.abiBadApp, c.abiBadGender,
                                c.abiBadDesc14);
    return detail::Stored(out, cap, r);
}

inline int FormatStatXy(char* out, size_t cap, const Counts& c) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap, "STAT xy x=%.1f..%.1f y=%.1f..%.1f", c.xMin, c.xMax, c.yMin, c.yMax);
    return detail::Stored(out, cap, r);
}

// One chunk of 12 slots for gender 0 or 1, all 64 slots printed (zeros included). Each slot appears in
// exactly one chunk. Returns the next start index, or -1 when this chunk ends the line set.
// Returns -1 with nothing written for gender > 1 or start >= 64.
inline int FormatStatSlot(char* out, size_t cap, const Counts& c, uint32_t gender, uint32_t start) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return -1;
    out[0] = '\0';
    if (gender > 1u || start >= kSlotCount) return -1;
    const int r = std::snprintf(out, cap, "STAT slot g=%u", gender);
    size_t len = static_cast<size_t>(detail::Stored(out, cap, r));
    uint32_t end = start + 12u;
    if (end > kSlotCount) end = kSlotCount;
    for (uint32_t i = start; i < end; ++i) {
        len = detail::AppendEntry(out, cap, len, i, c.slot[gender][i], true);
    }
    return end < kSlotCount ? static_cast<int>(end) : -1;
}

// Nonzero app histogram. kind 0 = tex, 1 = remap. Emits up to 12 nonzero entries at or after start.
// Returns the index of the next nonzero entry when more follow, or -1 when this line is the last.
// Returns -1 with nothing written when no nonzero entry exists at or after start.
inline int FormatStatApp(char* out, size_t cap, const Counts& c, uint32_t kind, uint32_t start) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return -1;
    out[0] = '\0';
    if (kind > 1u || start >= kAppCount) return -1;
    const uint32_t* hist = kind == 0u ? c.appTex : c.appRem;
    uint32_t i = start;
    while (i < kAppCount && hist[i] == 0u) ++i;
    if (i == kAppCount) return -1;
    const int r = std::snprintf(out, cap, "STAT app k=%s", kind == 0u ? "tex" : "remap");
    size_t len = static_cast<size_t>(detail::Stored(out, cap, r));
    uint32_t emitted = 0;
    while (i < kAppCount && emitted < 12u) {
        if (hist[i] != 0u) {
            len = detail::AppendEntry(out, cap, len, i, hist[i], false);
            ++emitted;
        }
        ++i;
    }
    uint32_t next = i;
    while (next < kAppCount && hist[next] == 0u) ++next;
    return next < kAppCount ? static_cast<int>(next) : -1;
}

inline int FormatStatRead(char* out, size_t cap, const Counts& c) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap, "STAT read 1:%u 2:%u 3:%u 4:%u 5:%u 6:%u 7:%u 8:%u 9:%u 10:%u",
                                c.fail[1], c.fail[2], c.fail[3], c.fail[4], c.fail[5], c.fail[6], c.fail[7],
                                c.fail[8], c.fail[9], c.fail[10]);
    return detail::Stored(out, cap, r);
}

inline int FormatStatCensus(char* out, size_t cap, uint32_t keys, uint32_t ovf) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap, "STAT census keys=%u ovf=%u", keys, ovf);
    return detail::Stored(out, cap, r);
}

// ---- Probe and one-shot lines (spec 4.3 probe channel) ----

inline int FormatDsc0(char* out, size_t cap, uint32_t pd, const uint8_t (&bytes)[32]) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char hx[65];
    detail::HexInto(hx, bytes, 32);
    const int r = std::snprintf(out, cap, "DSC0 pd=%08x hex=%s", pd, hx);
    return detail::Stored(out, cap, r);
}

// off is the byte offset of this 64-byte window within the creature block (printed %03x).
inline int FormatCreN(char* out, size_t cap, uint32_t self, uint32_t off, const uint8_t (&bytes)[64]) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char hx[129];
    detail::HexInto(hx, bytes, 64);
    const int r = std::snprintf(out, cap, "CRE0 this=%08x off=%03x hex=%s", self, off, hx);
    return detail::Stored(out, cap, r);
}

// 64 bytes behind the pointer stored at this+off (0.1.3 tag-route probe). p is the pointer value.
inline int FormatPtrN(char* out, size_t cap, uint32_t self, uint32_t off, uint32_t p, const uint8_t (&bytes)[64]) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char hx[129];
    detail::HexInto(hx, bytes, 64);
    const int r = std::snprintf(out, cap, "PTR this=%08x off=%03x p=%08x hex=%s", self, off, p, hx);
    return detail::Stored(out, cap, r);
}

inline int FormatCre0(char* out, size_t cap, uint32_t self, const uint8_t (&bytes)[64]) {
    return FormatCreN(out, cap, self, 0u, bytes);
}

inline int FormatHookBytes(char* out, size_t cap, uint32_t addr, const char* want, const char* got) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char w[65];
    char g[65];
    const int r = std::snprintf(out, cap, "HOOKBYTES addr=%08x want=%s got=%s", addr, SafeStr(w, want, 64),
                                SafeStr(g, got, 64));
    return detail::Stored(out, cap, r);
}

inline int FormatAbi(char* out, size_t cap, uint32_t app, uint32_t g, int dm, uint32_t tv, uint32_t pd,
                     const char* where) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    char wh[9];
    const int r = std::snprintf(out, cap, "ABI app=%u g=%u dm=%d tv=%u pd=%08x where=%s", app, g, dm, tv, pd,
                                SafeStr(wh, where, 8));
    return detail::Stored(out, cap, r);
}

inline int FormatRead(char* out, size_t cap, uint32_t site, uint32_t addr, uint32_t len, uint32_t caller,
                      uint32_t n) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap, "READ site=%u addr=%08x len=%u caller=%08x n=%u", site, addr, len,
                                caller, n);
    return detail::Stored(out, cap, r);
}

inline int FormatDump(char* out, size_t cap) {
    cap = detail::ClampCap(cap);
    if (!out || cap == 0) return 0;
    const int r = std::snprintf(out, cap, "DUMP trigger=file path=npc-diversity.dump");
    return detail::Stored(out, cap, r);
}

}  // namespace log
}  // namespace NpcDiversity
