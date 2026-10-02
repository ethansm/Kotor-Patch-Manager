// MSVC (the Windows release build, which has SDLCheck on) treats fopen/localtime as
// deprecated and errors on them. The service uses plain C stdio on purpose (FILE*
// is the one file API every target has, with no platform seam needed), so the
// warning is switched off for this file. Defining it unconditionally is harmless
// elsewhere and avoids any #ifdef.
#define _CRT_SECURE_NO_WARNINGS

#include "log_service.h"
#include "platform.h"

#include <atomic>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace KotorPatcher {
namespace LogService {

    namespace {

        constexpr int kMaxPatches      = 64;
        constexpr int kMaxChannels     = 256;
        constexpr int kMaxPatchIdLen   = 63;
        constexpr int kMaxChannelLen   = 31;
        constexpr uint32_t kRingSize   = 4096;
        // The last quarter of the ring is kept for WARN/ERR (and Mark): a flood of
        // INFO lines can fill 75% of it, after which only the lines that matter in
        // production still get in.
        constexpr uint32_t kReserveStart = kRingSize - kRingSize / 4;
        constexpr uint32_t kNoFrame    = 0xFFFFFFFFu;
        constexpr size_t kMaxIniBytes  = 1u << 20;  // a config bigger than this is not ours to read

        constexpr int kSinkPerPatch = 0;
        constexpr int kSinkMerged   = 1;
        constexpr int kSinkBoth     = 2;

        constexpr uint32_t kDefaultFlushMs  = 500;
        constexpr uint64_t kDefaultMaxBytes = 8000000;
        constexpr uint32_t kDefaultMaxLines = 2000;

        // One queued line. Fixed size so the critical section under the ring lock
        // is a single memcpy (no allocation, no formatting, no I/O). Timestamp and
        // frame are stamped by the submitter before it takes the lock; everything
        // else (patch name, channel name, level marker, text layout) is resolved by
        // the flush side, which keeps the submit path short.
        struct Record {
            uint64_t us;
            uint32_t frame;      // kNoFrame until a patch owns the frame counter
            int16_t channel;     // -1 for a Mark
            uint16_t len;
            uint8_t level;
            char text[232];
        };
        constexpr size_t kTextBytes = sizeof(Record::text);

        // Appended to a line that did not fit. Five visible characters so a reader
        // can tell "cut by the service" from "the patch ended the line there".
        const char kTruncMarker[] = "[...]";
        constexpr size_t kTruncLen = sizeof(kTruncMarker) - 1;

        struct Patch {
            std::string id;
            std::atomic<uint32_t> ringDrops{0};
            std::atomic<uint32_t> rateDrops{0};
            std::atomic<uint64_t> windowStart{0};
            std::atomic<uint32_t> windowCount{0};
        };

        struct PatchCfg {
            bool enabled = false;
            int level = KPLOG_INFO;
            bool hasChannels = false;
            bool star = false;
            std::vector<std::string> include;
            std::vector<std::string> exclude;
        };

        struct Config {
            bool enabled = false;
            int sink = kSinkPerPatch;
            uint32_t flushMs = kDefaultFlushMs;
            uint64_t maxBytes = kDefaultMaxBytes;
            uint32_t maxLines = kDefaultMaxLines;
            std::map<std::string, PatchCfg> patches;
        };

        // Flush-side state of one log file (keyed by path).
        struct FileState {
            std::FILE* f = nullptr;
            bool failed = false;     // could not be opened: its lines are dropped for the session
            uint64_t bytes = 0;      // bytes in the current file, header included
            uint64_t body = 0;       // bytes after the header (rotation needs something to rotate away)
            bool dirty = false;      // written since the last fflush
        };

        struct State {
            std::atomic<bool> initialized{false};
            std::atomic<bool> stopping{false};

            // Clock. The test clock replaces steady_clock; baseUs is the steady
            // time at Init so NowUs() counts from the start of the session.
            std::atomic<uint64_t (*)()> clock{nullptr};
            std::atomic<int64_t> baseUs{0};

            // Written in Init before `initialized` is published, then read-only.
            std::string dir;

            // Registration tables and the parsed config. Registration is not a hot
            // path (patches register once, at load) so a plain mutex is fine; the
            // hot-path readers use the atomics below and never take it. Entries are
            // written before the count is published with release, so a reader that
            // acquires the count sees fully built entries.
            std::mutex tableMutex;
            Patch patches[kMaxPatches];
            std::atomic<int32_t> patchCount{0};
            std::string chName[kMaxChannels];
            int16_t chPatch[kMaxChannels];
            std::atomic<int32_t> channelCount{0};
            std::atomic<uint8_t> level[kMaxChannels];
            Config cfg;
            std::string versionSha;
            std::string providedDump;
            bool infoSet = false;
            bool infoPending = false;

            // Config values the hot and flush paths read without a lock.
            std::atomic<int> sink{kSinkPerPatch};
            std::atomic<uint32_t> flushMs{kDefaultFlushMs};
            std::atomic<uint64_t> maxBytes{kDefaultMaxBytes};
            std::atomic<uint32_t> maxLines{kDefaultMaxLines};

            // kplog.ini change detection: compares the bytes, never the mtime
            // (unreliable under Wine and on filesystems with 1 s timestamps).
            std::mutex pollMutex;
            std::string lastContent;
            bool haveContent = false;

            // The ring is two equal buffers that the flusher swaps under the lock,
            // so draining is O(1) under the lock and the formatting and I/O happen
            // after it is released.
            std::mutex ringMutex;
            Record* ring = nullptr;
            Record* spare = nullptr;
            uint32_t ringCount = 0;

            std::atomic<int32_t> frameOwner{-1};
            std::atomic<uint32_t> frame{0};

            // Serialises flushers (the flush thread, FlushNow, Shutdown) and owns
            // the file table.
            std::mutex flushMutex;
            std::map<std::string, FileState> files;
        };

        // Heap-allocated and intentionally never deleted, for the same reasons as the
        // registry: a static object would be destroyed at process exit while patch
        // threads may still be submitting (Linux exit() runs static destructors with
        // other threads alive; Windows ExitProcess kills threads that may hold the
        // ring mutex). Leaking a few MB at process end is harmless. The function-local
        // static pointer makes construction thread-safe with no #ifdef.
        State& S() {
            static State* state = new State();
            return *state;
        }

        bool Accepting(const State& s) {
            return s.initialized.load(std::memory_order_acquire) &&
                   !s.stopping.load(std::memory_order_acquire);
        }

        // ---- small string helpers -------------------------------------------------

        std::string Trim(const std::string& in) {
            size_t b = 0, e = in.size();
            while (b < e && (in[b] == ' ' || in[b] == '\t' || in[b] == '\r' || in[b] == '\n')) ++b;
            while (e > b && (in[e - 1] == ' ' || in[e - 1] == '\t' || in[e - 1] == '\r' || in[e - 1] == '\n')) --e;
            return in.substr(b, e - b);
        }

        std::string Lower(std::string in) {
            for (char& c : in) {
                if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
            }
            return in;
        }

        // Patch ids become file names and channel names go into every line, so both
        // are limited to a conservative alphabet: no path separators, no spaces.
        bool ValidName(const char* name, size_t maxLen) {
            if (!name) return false;
            size_t n = 0;
            for (; name[n]; ++n) {
                const char c = name[n];
                const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                                (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-';
                if (!ok || n >= maxLen) return false;
            }
            return n >= 1;
        }

        bool ParseUInt(const std::string& v, uint64_t& out) {
            if (v.empty() || v.size() > 18) return false;
            uint64_t n = 0;
            for (char c : v) {
                if (c < '0' || c > '9') return false;
                n = n * 10 + static_cast<uint64_t>(c - '0');
            }
            out = n;
            return true;
        }

        bool ParseBool(const std::string& v, bool& out) {
            const std::string l = Lower(v);
            if (l == "1" || l == "true" || l == "on" || l == "yes") { out = true; return true; }
            if (l == "0" || l == "false" || l == "off" || l == "no") { out = false; return true; }
            return false;
        }

        bool ParseLevel(const std::string& v, int& out) {
            uint64_t n;
            if (ParseUInt(v, n)) {
                if (n > static_cast<uint64_t>(KPLOG_TRACE)) return false;
                out = static_cast<int>(n);
                return true;
            }
            const std::string l = Lower(v);
            if (l == "err")   { out = KPLOG_ERR;   return true; }
            if (l == "warn")  { out = KPLOG_WARN;  return true; }
            if (l == "info")  { out = KPLOG_INFO;  return true; }
            if (l == "debug") { out = KPLOG_DEBUG; return true; }
            if (l == "trace") { out = KPLOG_TRACE; return true; }
            return false;
        }

        bool ParseSink(const std::string& v, int& out) {
            const std::string l = Lower(v);
            if (l == "perpatch") { out = kSinkPerPatch; return true; }
            if (l == "merged")   { out = kSinkMerged;   return true; }
            if (l == "both")     { out = kSinkBoth;     return true; }
            return false;
        }

        void ParseChannels(const std::string& v, PatchCfg& pc) {
            pc.hasChannels = true;
            pc.star = false;
            pc.include.clear();
            pc.exclude.clear();
            size_t pos = 0;
            while (pos <= v.size()) {
                size_t comma = v.find(',', pos);
                if (comma == std::string::npos) comma = v.size();
                const std::string item = Trim(v.substr(pos, comma - pos));
                if (item == "*") {
                    pc.star = true;
                } else if (!item.empty() && item[0] == '-') {
                    const std::string name = Trim(item.substr(1));
                    if (!name.empty()) pc.exclude.push_back(name);
                } else if (!item.empty()) {
                    pc.include.push_back(item);
                }
                pos = comma + 1;
            }
        }

        bool Has(const std::vector<std::string>& list, const std::string& name) {
            for (const std::string& s : list) {
                if (s == name) return true;
            }
            return false;
        }

        // ---- kplog.ini ----------------------------------------------------------

        // Never fails: a line that cannot be understood is skipped (and counted) so a
        // typo in the ini cannot take logging down, and a half-written file during a
        // hot reload still applies whatever lines are already valid.
        Config ParseIni(const std::string& text, int& malformed) {
            enum Section { kNone, kGlobal, kPatch };
            Config c;
            Section section = kNone;
            PatchCfg* cur = nullptr;
            malformed = 0;

            size_t pos = 0;
            bool first = true;
            while (pos <= text.size()) {
                size_t nl = text.find('\n', pos);
                if (nl == std::string::npos) nl = text.size();
                std::string line = text.substr(pos, nl - pos);
                pos = nl + 1;

                if (first) {
                    first = false;
                    if (line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);  // UTF-8 BOM
                }
                // A comment starts at the first ';' or '#' anywhere, so trailing
                // comments after a value work. No valid value contains either.
                const size_t cpos = line.find_first_of(";#");
                if (cpos != std::string::npos) line.erase(cpos);
                line = Trim(line);
                if (line.empty()) continue;

                if (line[0] == '[') {
                    const std::string name = line.size() >= 2 && line.back() == ']'
                        ? Trim(line.substr(1, line.size() - 2)) : std::string();
                    if (name.empty()) {
                        ++malformed;
                        section = kNone;
                        cur = nullptr;
                    } else if (Lower(name) == "global") {
                        section = kGlobal;
                        cur = nullptr;
                    } else {
                        section = kPatch;
                        cur = &c.patches[name];
                    }
                    continue;
                }

                const size_t eq = line.find('=');
                if (eq == std::string::npos) { ++malformed; continue; }
                const std::string key = Lower(Trim(line.substr(0, eq)));
                const std::string val = Trim(line.substr(eq + 1));

                bool ok = false;
                uint64_t n = 0;
                if (section == kGlobal) {
                    if (key == "enabled")                  ok = ParseBool(val, c.enabled);
                    else if (key == "sink")                ok = ParseSink(val, c.sink);
                    else if (key == "flush_ms")            { ok = ParseUInt(val, n) && n >= 1 && n <= 0xFFFFFFFFu; if (ok) c.flushMs = static_cast<uint32_t>(n); }
                    else if (key == "max_bytes")           { ok = ParseUInt(val, n) && n >= 1; if (ok) c.maxBytes = n; }
                    else if (key == "max_lines_per_sec")   { ok = ParseUInt(val, n) && n <= 0xFFFFFFFFu; if (ok) c.maxLines = static_cast<uint32_t>(n); }
                } else if (section == kPatch && cur) {
                    if (key == "enabled")                  ok = ParseBool(val, cur->enabled);
                    else if (key == "level")               ok = ParseLevel(val, cur->level);
                    else if (key == "channels")            { ParseChannels(val, *cur); ok = true; }
                }
                if (!ok) ++malformed;
            }
            return c;
        }

        // Effective level of one channel under `c`. WARN and ERR are always on, so
        // the answer is never below WARN even if the ini asks for level=0.
        uint8_t EffectiveLevel(const Config& c, const std::string& patchId, const std::string& channel) {
            if (!c.enabled) return KPLOG_WARN;
            const auto it = c.patches.find(patchId);
            if (it == c.patches.end() || !it->second.enabled) return KPLOG_WARN;
            const PatchCfg& pc = it->second;

            // A list of only exclusions ("channels=-spam") means "everything except
            // these": selecting nothing would make the line a silent no-op, which is
            // never what someone writing it wants.
            const bool onlyExclusions = pc.include.empty() && !pc.exclude.empty();
            bool selected = !pc.hasChannels || pc.star || onlyExclusions || Has(pc.include, channel);
            if (Has(pc.exclude, channel)) selected = false;
            if (!selected) return KPLOG_WARN;
            return static_cast<uint8_t>(pc.level < KPLOG_WARN ? KPLOG_WARN : pc.level);
        }

        // Caller holds tableMutex.
        void RecomputeLevels(State& s) {
            const int32_t n = s.channelCount.load(std::memory_order_relaxed);
            for (int32_t i = 0; i < n; ++i) {
                s.level[i].store(EffectiveLevel(s.cfg, s.patches[s.chPatch[i]].id, s.chName[i]),
                                 std::memory_order_relaxed);
            }
        }

        std::string ReadWholeFile(const std::string& path) {
            std::string out;
            std::FILE* f = std::fopen(path.c_str(), "rb");
            if (!f) return out;
            char buf[4096];
            size_t n;
            while (out.size() < kMaxIniBytes && (n = std::fread(buf, 1, sizeof(buf), f)) > 0) {
                out.append(buf, n);
            }
            std::fclose(f);
            return out;
        }

        std::string JoinPath(const std::string& dir, const std::string& file) {
            if (dir.empty()) return file;
            const char last = dir[dir.size() - 1];
            if (last == '/' || last == '\\') return dir + file;
            return dir + "/" + file;
        }

        void DoPoll(State& s) {
            std::lock_guard<std::mutex> pollLock(s.pollMutex);
            const std::string content = ReadWholeFile(JoinPath(s.dir, "kplog.ini"));
            if (s.haveContent && content == s.lastContent) return;
            s.haveContent = true;
            s.lastContent = content;

            int malformed = 0;
            Config c = ParseIni(content, malformed);
            if (malformed > 0) {
                char msg[160];
                std::snprintf(msg, sizeof(msg),
                              "[KotorPatcher] kplog: ignored %d malformed line(s) in kplog.ini\n", malformed);
                Platform::Log(msg);
            }

            std::lock_guard<std::mutex> lock(s.tableMutex);
            s.cfg = c;
            s.sink.store(c.sink);
            s.flushMs.store(c.flushMs);
            s.maxBytes.store(c.maxBytes);
            s.maxLines.store(c.maxLines);
            RecomputeLevels(s);
        }

        // ---- clock ----------------------------------------------------------------

        int64_t SteadyUs() {
            return std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        uint64_t NowUsImpl(State& s) {
            if (!s.initialized.load(std::memory_order_acquire)) return 0;
            uint64_t (*fn)() = s.clock.load();
            if (fn) return fn();
            const int64_t d = SteadyUs() - s.baseUs.load();
            return d > 0 ? static_cast<uint64_t>(d) : 0;
        }

        // ---- ring -----------------------------------------------------------------

        // The only work under the ring lock is a count check and one memcpy of a
        // fixed-size record.
        bool Push(State& s, const Record& r) {
            std::lock_guard<std::mutex> lock(s.ringMutex);
            if (s.ringCount >= kRingSize) return false;
            if (s.ringCount >= kReserveStart && r.level > KPLOG_WARN) return false;
            std::memcpy(&s.ring[s.ringCount], &r, sizeof(Record));
            ++s.ringCount;
            return true;
        }

        // Fill r.text from `line`, cutting an over-long line and ending it with the
        // truncation marker. Returns nothing: r.len is set.
        void FillText(Record& r, const char* line, size_t len) {
            if (len > kTextBytes) {
                std::memcpy(r.text, line, kTextBytes - kTruncLen);
                std::memcpy(r.text + kTextBytes - kTruncLen, kTruncMarker, kTruncLen);
                r.len = static_cast<uint16_t>(kTextBytes);
            } else {
                std::memcpy(r.text, line, len);
                r.len = static_cast<uint16_t>(len);
            }
        }

        uint32_t CurrentFrame(const State& s) {
            return s.frameOwner.load() >= 0 ? s.frame.load() : kNoFrame;
        }

        // ---- files (flush side; caller holds flushMutex) --------------------------

        // The header opens every file of a session so a log read on its own says
        // when it was written, which build it came from and which interfaces were
        // live. Every line starts with "# " so a parser can skip them all.
        std::string InfoLines(State& s) {
            std::lock_guard<std::mutex> lock(s.tableMutex);
            std::string out = "# KOTOR_VERSION_SHA=" + (s.versionSha.empty() ? std::string("unknown") : s.versionSha) + "\n";
            if (!s.infoSet) {
                out += "# interfaces: (not yet applied)\n";
                return out;
            }
            out += "# interfaces:\n";
            size_t pos = 0;
            while (pos < s.providedDump.size()) {
                size_t nl = s.providedDump.find('\n', pos);
                if (nl == std::string::npos) nl = s.providedDump.size();
                const std::string line = s.providedDump.substr(pos, nl - pos);
                pos = nl + 1;
                if (!line.empty()) out += "# " + line + "\n";
            }
            return out;
        }

        std::string BuildHeader(State& s) {
            char when[64] = "unknown";
            const std::time_t t = std::time(nullptr);
            const std::tm* tmv = std::localtime(&t);
            if (tmv) std::strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", tmv);
            char first[160];
            std::snprintf(first, sizeof(first), "# kplog session %s | kpatch.log v%d | patcher ABI %u\n",
                          when, KPATCH_LOG_VERSION, static_cast<unsigned>(KPATCH_ABI_VERSION));
            return std::string(first) + InfoLines(s);
        }

        void ReportOpenFailure(FileState& fs, const std::string& path) {
            fs.failed = true;
            fs.f = nullptr;
            char msg[1024];
            std::snprintf(msg, sizeof(msg),
                          "[KotorPatcher] kplog: cannot open %s, dropping its lines\n", path.c_str());
            Platform::Log(msg);
        }

        // Move the current file aside as <path>.1. Windows rename() refuses to
        // overwrite, so the old .1 is removed first (keep exactly one generation).
        void RotateAside(const std::string& path) {
            const std::string old = path + ".1";
            std::remove(old.c_str());
            std::rename(path.c_str(), old.c_str());
        }

        bool OpenWithHeader(State& s, const std::string& path, FileState& fs) {
            fs.f = std::fopen(path.c_str(), "wb");
            if (!fs.f) {
                ReportOpenFailure(fs, path);
                return false;
            }
            const std::string header = BuildHeader(s);
            std::fwrite(header.data(), 1, header.size(), fs.f);
            fs.bytes = header.size();
            fs.body = 0;
            fs.dirty = true;
            return true;
        }

        // First open of a path in this session. The previous session's file is kept
        // as .1 (launch rotation) so a crash leaves its log behind for the next
        // launch instead of being overwritten by it.
        bool FirstOpen(State& s, const std::string& path, FileState& fs) {
            std::FILE* probe = std::fopen(path.c_str(), "rb");
            if (probe) {
                std::fclose(probe);
                RotateAside(path);
            }
            return OpenWithHeader(s, path, fs);
        }

        void WriteTo(State& s, const std::string& path, const char* data, size_t n) {
            FileState& fs = s.files[path];
            if (fs.failed) return;
            if (!fs.f && !FirstOpen(s, path, fs)) return;

            // Byte cap. Rotating needs something to rotate away (body > 0), so a cap
            // smaller than the header cannot make every write rotate forever.
            const uint64_t cap = s.maxBytes.load();
            if (cap && fs.body > 0 && fs.bytes + n > cap) {
                std::fclose(fs.f);
                fs.f = nullptr;
                RotateAside(path);
                if (!OpenWithHeader(s, path, fs)) return;
            }
            if (std::fwrite(data, 1, n, fs.f) == n) {
                fs.bytes += n;
                fs.body += n;
            }
            fs.dirty = true;
        }

        std::string PatchPath(State& s, const std::string& id) {
            return JoinPath(s.dir, id + "_log.txt");
        }

        std::string MergedPath(State& s) {
            return JoinPath(s.dir, "kp_log.txt");
        }

        void WritePatchLine(State& s, int sink, const std::string& id, const std::string& line) {
            if (sink == kSinkPerPatch || sink == kSinkBoth) WriteTo(s, PatchPath(s, id), line.data(), line.size());
            if (sink == kSinkMerged || sink == kSinkBoth) WriteTo(s, MergedPath(s), line.data(), line.size());
        }

        // A Mark goes to every file that is open, so whichever log you read shows
        // where you pressed the key, and to the merged file if that sink is active.
        void WriteMark(State& s, int sink, const std::string& line) {
            std::vector<std::string> targets;
            for (const auto& kv : s.files) {
                if (kv.second.f) targets.push_back(kv.first);
            }
            if (sink == kSinkMerged || sink == kSinkBoth) {
                const std::string merged = MergedPath(s);
                bool listed = false;
                for (const std::string& t : targets) listed = listed || t == merged;
                if (!listed) targets.push_back(merged);
            }
            for (const std::string& t : targets) WriteTo(s, t, line.data(), line.size());
        }

        std::string LineHead(uint64_t us, uint32_t frame) {
            char buf[64];
            if (frame == kNoFrame) {
                std::snprintf(buf, sizeof(buf), "T=%" PRIu64 ".%03u f=- ", us / 1000, static_cast<unsigned>(us % 1000));
            } else {
                std::snprintf(buf, sizeof(buf), "T=%" PRIu64 ".%03u f=%u ", us / 1000, static_cast<unsigned>(us % 1000), frame);
            }
            return buf;
        }

        // Called with flushMutex held.
        void FlushLocked(State& s) {
            // Swap the buffers under the ring lock; everything else happens outside it.
            Record* batch;
            uint32_t count;
            {
                std::lock_guard<std::mutex> lock(s.ringMutex);
                batch = s.ring;
                count = s.ringCount;
                s.ring = s.spare;
                s.spare = batch;
                s.ringCount = 0;
            }

            const int sink = s.sink.load();

            // Sessions info supplied after a file was opened: append it to the files
            // that are open now. Files opened later get it in their header.
            bool pending;
            {
                std::lock_guard<std::mutex> lock(s.tableMutex);
                pending = s.infoPending;
                s.infoPending = false;
            }
            if (pending) {
                const std::string block = "# session info:\n" + InfoLines(s);
                for (auto& kv : s.files) {
                    if (!kv.second.f) continue;
                    std::fwrite(block.data(), 1, block.size(), kv.second.f);
                    kv.second.bytes += block.size();
                    kv.second.dirty = true;
                }
            }

            const int32_t channels = s.channelCount.load(std::memory_order_acquire);
            const int32_t patches = s.patchCount.load(std::memory_order_acquire);
            std::string line;
            for (uint32_t i = 0; i < count; ++i) {
                const Record& r = batch[i];
                line = LineHead(r.us, r.frame);
                if (r.channel < 0) {
                    line += "MARK ";
                    line.append(r.text, r.len);
                    line += '\n';
                    WriteMark(s, sink, line);
                    continue;
                }
                if (r.channel >= channels) continue;  // cannot happen; never index past the table
                const Patch& p = s.patches[s.chPatch[r.channel]];
                line += p.id + "/" + s.chName[r.channel] + " ";
                if (r.level == KPLOG_WARN) line += "WARN ";
                else if (r.level == KPLOG_ERR) line += "ERR ";
                line.append(r.text, r.len);
                line += '\n';
                WritePatchLine(s, sink, p.id, line);
            }

            // Report what was dropped since the last flush, per patch, so a reader
            // knows the log has a hole and why.
            const uint64_t now = NowUsImpl(s);
            for (int32_t i = 0; i < patches; ++i) {
                Patch& p = s.patches[i];
                const uint32_t ring = p.ringDrops.exchange(0);
                const uint32_t rate = p.rateDrops.exchange(0);
                if (ring == 0 && rate == 0) continue;
                char buf[160];
                std::snprintf(buf, sizeof(buf), "%s/kplog DROPPED %" PRIu64 " (ring %u, rate %u)\n",
                              p.id.c_str(), static_cast<uint64_t>(ring) + rate, ring, rate);
                WritePatchLine(s, sink, p.id, LineHead(now, CurrentFrame(s)) + buf);
            }

            for (auto& kv : s.files) {
                if (kv.second.f && kv.second.dirty) {
                    std::fflush(kv.second.f);
                    kv.second.dirty = false;
                }
            }
        }

        // ---- ABI trampolines -------------------------------------------------------
        // Free functions with the ABI's calling convention, because KPatchLogApi holds
        // plain function pointers that patches built by other compilers call directly.
        int32_t KPATCH_CALL ApiRegister(const char* patchId) { return Register(patchId); }
        int32_t KPATCH_CALL ApiChannel(int32_t patch, const char* name) { return Channel(patch, name); }
        int32_t KPATCH_CALL ApiEnabled(int32_t channel, int32_t level) { return Enabled(channel, level); }
        void KPATCH_CALL ApiSubmit(int32_t channel, int32_t level, const char* line, uint32_t len) { Submit(channel, level, line, len); }
        void KPATCH_CALL ApiMark(const char* label) { Mark(label); }
        void KPATCH_CALL ApiFrameTick(int32_t patch) { FrameTick(patch); }
        uint32_t KPATCH_CALL ApiFrame() { return Frame(); }
        uint64_t KPATCH_CALL ApiNowUs() { return NowUs(); }

        KPatchLogApi g_api = {
            sizeof(KPatchLogApi),
            &ApiRegister, &ApiChannel, &ApiEnabled, &ApiSubmit,
            &ApiMark, &ApiFrameTick, &ApiFrame, &ApiNowUs,
        };

    } // namespace

    const KPatchLogApi* Api() {
        return &g_api;
    }

    void Init(const std::string& dir, const Options& opt) {
        // The flush thread is not started here yet; a later change adds it behind
        // opt.startThread. Until then the owner calls FlushNow()/PollConfigNow().
        (void)opt;
        State& s = S();
        if (s.initialized.load() && !s.stopping.load()) return;  // a session is already running

        {
            std::lock_guard<std::mutex> flushLock(s.flushMutex);
            for (auto& kv : s.files) {
                if (kv.second.f) std::fclose(kv.second.f);
            }
            s.files.clear();

            {
                std::lock_guard<std::mutex> ringLock(s.ringMutex);
                if (!s.ring) {
                    s.ring = new Record[kRingSize];
                    s.spare = new Record[kRingSize];
                }
                s.ringCount = 0;
            }

            {
                std::lock_guard<std::mutex> lock(s.tableMutex);
                s.dir = dir;
                for (int i = 0; i < kMaxPatches; ++i) {
                    Patch& p = s.patches[i];
                    p.id.clear();
                    p.ringDrops.store(0);
                    p.rateDrops.store(0);
                    p.windowStart.store(0);
                    p.windowCount.store(0);
                }
                for (int i = 0; i < kMaxChannels; ++i) {
                    s.chName[i].clear();
                    s.chPatch[i] = 0;
                    s.level[i].store(KPLOG_WARN);
                }
                s.patchCount.store(0);
                s.channelCount.store(0);
                s.cfg = Config();
                s.versionSha.clear();
                s.providedDump.clear();
                s.infoSet = false;
                s.infoPending = false;
            }
            s.sink.store(kSinkPerPatch);
            s.flushMs.store(kDefaultFlushMs);
            s.maxBytes.store(kDefaultMaxBytes);
            s.maxLines.store(kDefaultMaxLines);
            s.frameOwner.store(-1);
            s.frame.store(0);
            s.baseUs.store(SteadyUs());
        }
        {
            std::lock_guard<std::mutex> pollLock(s.pollMutex);
            s.haveContent = false;
            s.lastContent.clear();
        }

        DoPoll(s);  // picks up an existing kplog.ini (and applies the defaults if none)

        s.stopping.store(false, std::memory_order_release);
        s.initialized.store(true, std::memory_order_release);
    }

    void SetSessionInfo(const std::string& versionSha, const std::string& providedDump) {
        State& s = S();
        if (!Accepting(s)) return;
        std::lock_guard<std::mutex> lock(s.tableMutex);
        s.versionSha = versionSha;
        s.providedDump = providedDump;
        s.infoSet = true;
        s.infoPending = true;
    }

    void FlushNow() {
        State& s = S();
        if (!s.initialized.load(std::memory_order_acquire)) return;
        std::lock_guard<std::mutex> lock(s.flushMutex);
        if (!s.initialized.load(std::memory_order_acquire)) return;  // Shutdown won the race
        FlushLocked(s);
    }

    void PollConfigNow() {
        State& s = S();
        if (!Accepting(s)) return;
        DoPoll(s);
    }

    void SetClockForTest(uint64_t (*nowUs)()) {
        S().clock.store(nowUs);
    }

    void Shutdown() {
        State& s = S();
        if (!s.initialized.load(std::memory_order_acquire)) return;
        // Stop intake first so the final flush sees a ring that no longer grows.
        s.stopping.store(true, std::memory_order_release);
        std::lock_guard<std::mutex> lock(s.flushMutex);
        FlushLocked(s);
        for (auto& kv : s.files) {
            if (kv.second.f) std::fclose(kv.second.f);
            kv.second.f = nullptr;
        }
        s.initialized.store(false, std::memory_order_release);
    }

    int32_t Register(const char* patchId) {
        State& s = S();
        if (!Accepting(s)) return -1;
        if (!ValidName(patchId, kMaxPatchIdLen)) return -1;

        std::lock_guard<std::mutex> lock(s.tableMutex);
        const int32_t n = s.patchCount.load(std::memory_order_relaxed);
        for (int32_t i = 0; i < n; ++i) {
            if (s.patches[i].id == patchId) return i;
        }
        if (n >= kMaxPatches) return -1;
        s.patches[n].id = patchId;
        s.patchCount.store(n + 1, std::memory_order_release);
        return n;
    }

    int32_t Channel(int32_t patch, const char* name) {
        State& s = S();
        if (!Accepting(s)) return -1;
        if (patch < 0 || patch >= s.patchCount.load(std::memory_order_acquire)) return -1;
        if (!ValidName(name, kMaxChannelLen)) return -1;

        std::lock_guard<std::mutex> lock(s.tableMutex);
        const int32_t n = s.channelCount.load(std::memory_order_relaxed);
        for (int32_t i = 0; i < n; ++i) {
            if (s.chPatch[i] == patch && s.chName[i] == name) return i;
        }
        if (n >= kMaxChannels) return -1;
        s.chPatch[n] = static_cast<int16_t>(patch);
        s.chName[n] = name;
        // A channel registered after a config reload starts at the level that
        // config gives it, not at a stale default.
        s.level[n].store(EffectiveLevel(s.cfg, s.patches[patch].id, s.chName[n]), std::memory_order_relaxed);
        s.channelCount.store(n + 1, std::memory_order_release);
        return n;
    }

    int32_t Enabled(int32_t channel, int32_t level) {
        State& s = S();
        if (!Accepting(s)) return 0;
        if (channel < 0 || channel >= s.channelCount.load(std::memory_order_acquire)) return 0;
        if (level < 0) return 0;
        return level <= static_cast<int32_t>(s.level[channel].load(std::memory_order_relaxed)) ? 1 : 0;
    }

    void Submit(int32_t channel, int32_t level, const char* line, uint32_t len) {
        State& s = S();
        if (!Accepting(s)) return;
        if (!line || len == 0) return;
        if (!Enabled(channel, level)) return;

        // Callers may or may not include the newline; the flusher adds exactly one.
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) --len;
        if (len == 0) return;

        Patch& p = s.patches[s.chPatch[channel]];
        const uint64_t now = NowUsImpl(s);

        // Per-patch rate limit over a 1 s window of the service clock. WARN/ERR are
        // exempt: the lines you need most must not be the ones a noisy INFO loop
        // pushes out. The window reset is a CAS so one thread wins it; a line racing
        // the reset may be miscounted by one, which is fine for a flood guard.
        if (level >= KPLOG_INFO) {
            const uint32_t cap = s.maxLines.load(std::memory_order_relaxed);
            if (cap != 0) {
                uint64_t start = p.windowStart.load(std::memory_order_relaxed);
                if (now >= start + 1000000u) {
                    if (p.windowStart.compare_exchange_strong(start, now)) p.windowCount.store(0);
                }
                if (p.windowCount.fetch_add(1) >= cap) {
                    p.rateDrops.fetch_add(1);
                    return;
                }
            }
        }

        Record r;
        r.us = now;
        r.frame = CurrentFrame(s);
        r.channel = static_cast<int16_t>(channel);
        r.level = static_cast<uint8_t>(level);
        FillText(r, line, len);
        if (!Push(s, r)) p.ringDrops.fetch_add(1);
        // A later change wakes the flush thread here for WARN/ERR so they reach the
        // disk promptly; with no thread the owner flushes.
    }

    void Mark(const char* label) {
        State& s = S();
        if (!Accepting(s)) return;
        if (!label) return;
        Record r;
        r.us = NowUsImpl(s);
        r.frame = CurrentFrame(s);
        r.channel = -1;
        r.level = KPLOG_WARN;  // markers use the reserve like WARN/ERR
        FillText(r, label, std::strlen(label));
        Push(s, r);  // a mark has no patch to charge a drop to
    }

    void FrameTick(int32_t patch) {
        State& s = S();
        if (!Accepting(s)) return;
        if (patch < 0 || patch >= s.patchCount.load(std::memory_order_acquire)) return;
        int32_t expected = -1;
        // The first patch to tick becomes the owner; for everyone after, the CAS
        // fails and `expected` holds the owner.
        if (s.frameOwner.compare_exchange_strong(expected, patch) || expected == patch) {
            s.frame.fetch_add(1);
        }
    }

    uint32_t Frame() {
        State& s = S();
        if (!Accepting(s)) return 0;
        return s.frame.load();
    }

    uint64_t NowUs() {
        return NowUsImpl(S());
    }

} // namespace LogService
} // namespace KotorPatcher
