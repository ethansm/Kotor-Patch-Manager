// Host test for the "kpatch.log" service: line format, frame ownership, the
// kplog.ini config (levels, channels, hot reload by content), the ring and its
// WARN/ERR reserve, drop accounting, rate limiting, sinks, byte-cap and launch
// rotation, Mark, the session header, unwritable directories and shutdown.
//
// Every row starts a fresh session in a scratch directory next to the test binary
// (<build>/tests/logsvc_tmp_<width>) with the flush thread off, so the rows drive
// FlushNow()/PollConfigNow() themselves and use a fake clock: nothing here depends
// on timing, except the loose "does not block" bound on the 100k-submit row. The
// last rows ("real flush thread") are the exception: they run with the real clock
// and the flush thread on, and poll the disk with generous timeouts.
//
// Platform::Log lines (stderr) are only inspected where a row is about them.
#include "log_service.h"
#include "platform.h"
#include "check.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace KotorPatcher;

namespace {

    std::string g_dir;
    std::atomic<uint64_t> g_now{0};

    uint64_t FakeClock() { return g_now.load(); }

    // ---- scratch directory helpers --------------------------------------------

    void Wipe(const std::string& dir) {
        DIR* d = opendir(dir.c_str());
        if (!d) return;
        while (dirent* e = readdir(d)) {
            const std::string name = e->d_name;
            if (name == "." || name == "..") continue;
            const std::string path = dir + "/" + name;
            struct stat st;
            if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                chmod(path.c_str(), 0700);
                Wipe(path);
                rmdir(path.c_str());
            } else {
                unlink(path.c_str());
            }
        }
        closedir(d);
    }

    std::string P(const char* name) { return g_dir + "/" + name; }

    void WriteFile(const std::string& path, const std::string& content) {
        std::FILE* f = std::fopen(path.c_str(), "wb");
        if (!f) return;
        std::fwrite(content.data(), 1, content.size(), f);
        std::fclose(f);
    }

    bool Exists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }

    std::string ReadFile(const std::string& path) {
        std::string out;
        std::FILE* f = std::fopen(path.c_str(), "rb");
        if (!f) return out;
        char buf[4096];
        size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
        std::fclose(f);
        return out;
    }

    long SizeOf(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 ? static_cast<long>(st.st_size) : -1;
    }

    std::vector<std::string> Split(const std::string& text) {
        std::vector<std::string> out;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t nl = text.find('\n', pos);
            if (nl == std::string::npos) nl = text.size();
            out.push_back(text.substr(pos, nl - pos));
            pos = nl + 1;
        }
        return out;
    }

    // Lines of a file that are not session-header lines.
    std::vector<std::string> Body(const std::string& path) {
        std::vector<std::string> out;
        for (const std::string& l : Split(ReadFile(path))) {
            if (l.compare(0, 2, "# ") != 0) out.push_back(l);
        }
        return out;
    }

    int CountContaining(const std::vector<std::string>& lines, const char* needle) {
        int n = 0;
        for (const std::string& l : lines) {
            if (l.find(needle) != std::string::npos) ++n;
        }
        return n;
    }

    bool Contains(const std::string& haystack, const char* needle) {
        return haystack.find(needle) != std::string::npos;
    }

    bool EndsWith(const std::string& s, const char* suffix) {
        const size_t n = std::strlen(suffix);
        return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
    }

    // Run `fn` with fd 2 redirected to a temp file and return what it wrote.
    std::string CaptureStderr(const std::function<void()>& fn) {
        std::fflush(stderr);
        std::FILE* tmp = std::tmpfile();
        int saved = dup(2);
        dup2(fileno(tmp), 2);
        fn();
        std::fflush(stderr);
        dup2(saved, 2);
        close(saved);

        std::string text;
        std::rewind(tmp);
        char buf[512];
        size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf), tmp)) > 0) text.append(buf, n);
        std::fclose(tmp);
        return text;
    }

    // ---- row scaffolding --------------------------------------------------------

    // End any previous session, empty the scratch dir, reset the clock, write the
    // ini (nullptr = no ini file) and start a fresh session with no flush thread.
    // `precreate` runs after the wipe and before Init, for rows that need files to
    // exist before the session starts.
    void Begin(const char* ini, const std::function<void()>& precreate = nullptr) {
        LogService::Shutdown();
        Wipe(g_dir);
        g_now = 0;
        LogService::SetClockForTest(&FakeClock);
        if (ini) WriteFile(P("kplog.ini"), ini);
        if (precreate) precreate();
        LogService::Options opt;
        opt.startThread = false;
        LogService::Init(g_dir, opt);
    }

    using Clock = std::chrono::steady_clock;

    // Like Begin() but with the real clock and the real flush thread. `ini` may be
    // nullptr (no ini file).
    void BeginThreaded(const char* ini) {
        LogService::Shutdown();
        Wipe(g_dir);
        LogService::SetClockForTest(nullptr);
        if (ini) WriteFile(P("kplog.ini"), ini);
        LogService::Options opt;
        opt.startThread = true;
        LogService::Init(g_dir, opt);
    }

    // Poll `pred` every 10 ms for up to `timeoutMs`. Returns the elapsed
    // milliseconds when it became true, or -1 on timeout.
    long WaitFor(const std::function<bool()>& pred, long timeoutMs) {
        const auto t0 = Clock::now();
        for (;;) {
            const long ms = static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t0).count());
            if (pred()) return ms;
            if (ms >= timeoutMs) return -1;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // "[global] enabled=1" plus a patch section with the given extra lines.
    std::string Ini(const char* patch, const char* patchLines, const char* globalLines = "") {
        return std::string("[global]\nenabled=1\n") + globalLines + "\n[" + patch + "]\nenabled=1\n" + patchLines + "\n";
    }

    void Submit(int32_t ch, int level, const std::string& text) {
        LogService::Submit(ch, level, text.data(), static_cast<uint32_t>(text.size()));
    }

    void Row(const char* name) { std::printf("  -- %s\n", name); }

    // Highest level (0-4) enabled on a channel, or -1.
    int MaxLevel(int32_t ch) {
        int top = -1;
        for (int l = 0; l <= 4; ++l) {
            if (LogService::Enabled(ch, l)) top = l;
        }
        return top;
    }

} // namespace

int main(int argc, char** argv) {
    std::printf("  log service, %zu-bit\n", sizeof(void*) * 8);

    std::string self = argc > 0 ? argv[0] : "";
    const size_t slash = self.find_last_of('/');
    const std::string base = slash == std::string::npos ? std::string(".") : self.substr(0, slash);
    g_dir = base + "/logsvc_tmp_" + std::to_string(sizeof(void*) * 8);
    mkdir(g_dir.c_str(), 0755);

    Row("line format / f=- / T= formatting");
    {
        Begin(Ini("alpha", "level=4").c_str());
        const int32_t p = LogService::Register("alpha");
        const int32_t ch = LogService::Channel(p, "draw");
        g_now = 1234567;
        Submit(ch, KPLOG_INFO, "hello");
        LogService::FlushNow();
        std::vector<std::string> body = Body(P("alpha_log.txt"));
        kptest::Check("one line before any tick", body.size() == 1);
        kptest::Check("T=ms.usfrac, f=-, tag, message",
                      !body.empty() && body[0] == "T=1234.567 f=- alpha/draw hello");

        LogService::FrameTick(p);
        g_now = 2000001;
        Submit(ch, KPLOG_INFO, "world");
        Submit(ch, KPLOG_DEBUG, "dbg");
        Submit(ch, KPLOG_TRACE, "trc");
        Submit(ch, KPLOG_WARN, "careful");
        Submit(ch, KPLOG_ERR, "broken");
        LogService::FlushNow();
        body = Body(P("alpha_log.txt"));
        kptest::Check("five more lines", body.size() == 6);
        kptest::Check("frame number after tick",
                      body.size() > 1 && body[1] == "T=2000.001 f=1 alpha/draw world");
        kptest::Check("DEBUG/TRACE carry no level marker",
                      body.size() > 3 && body[2] == "T=2000.001 f=1 alpha/draw dbg" &&
                      body[3] == "T=2000.001 f=1 alpha/draw trc");
        kptest::Check("WARN line is marked",
                      body.size() > 4 && body[4] == "T=2000.001 f=1 alpha/draw WARN careful");
        kptest::Check("ERR line is marked",
                      body.size() > 5 && body[5] == "T=2000.001 f=1 alpha/draw ERR broken");

        g_now = 5;
        Submit(ch, KPLOG_INFO, "with newline\n");
        LogService::FlushNow();
        body = Body(P("alpha_log.txt"));
        kptest::Check("trailing newline not doubled",
                      body.size() == 7 && body[6] == "T=0.005 f=1 alpha/draw with newline");
    }

    Row("NowUs / Frame / api table");
    {
        kptest::Check("NowUs before Init is 0", (LogService::Shutdown(), LogService::NowUs()) == 0);
        Begin(nullptr);
        g_now = 777;
        kptest::Check("NowUs follows the service clock", LogService::NowUs() == 777);
        const KPatchLogApi* api = LogService::Api();
        kptest::Check("struct_size and pointers set",
                      api && api->struct_size == sizeof(KPatchLogApi) && api->Register && api->Channel &&
                      api->Enabled && api->Submit && api->Mark && api->FrameTick && api->Frame && api->NowUs);
        const int32_t p = api->Register("viaapi");
        const int32_t ch = api->Channel(p, "c");
        kptest::Check("handles through the table", p == 0 && ch == 0);
        kptest::Check("Enabled through the table", api->Enabled(ch, KPLOG_WARN) == 1 && api->Enabled(ch, KPLOG_INFO) == 0);
        kptest::Check("NowUs through the table", api->NowUs() == 777);
        kptest::Check("Frame is 0 before a tick", api->Frame() == 0);
        api->FrameTick(p);
        api->FrameTick(p);
        kptest::Check("Frame counts the owner's ticks", api->Frame() == 2);
        LogService::SetClockForTest(nullptr);
        LogService::Shutdown();
        LogService::Init(g_dir, LogService::Options{false});
        const uint64_t a = LogService::NowUs();
        const uint64_t b = LogService::NowUs();
        kptest::Check("steady clock: small and monotonic", b >= a && b < 5000000);
        LogService::SetClockForTest(&FakeClock);
    }

    Row("frame owner");
    {
        Begin(Ini("alpha", "level=2").c_str());
        const int32_t a = LogService::Register("alpha");
        const int32_t b = LogService::Register("beta");
        const int32_t ca = LogService::Channel(a, "x");
        kptest::Check("frame starts at 0", LogService::Frame() == 0);
        LogService::FrameTick(b);
        kptest::Check("first caller (beta) owns the counter", LogService::Frame() == 1);
        LogService::FrameTick(a);
        LogService::FrameTick(a);
        kptest::Check("other patch's ticks are ignored", LogService::Frame() == 1);
        LogService::FrameTick(b);
        kptest::Check("owner still advances", LogService::Frame() == 2);
        Submit(ca, KPLOG_INFO, "m");
        LogService::FlushNow();
        std::vector<std::string> body = Body(P("alpha_log.txt"));
        kptest::Check("lines carry the owner's frame", body.size() == 1 && body[0] == "T=0.000 f=2 alpha/x m");
    }

    Row("default levels (no ini)");
    {
        Begin(nullptr);
        const int32_t p = LogService::Register("alpha");
        const int32_t ch = LogService::Channel(p, "x");
        kptest::Check("ERR enabled", LogService::Enabled(ch, KPLOG_ERR) == 1);
        kptest::Check("WARN enabled", LogService::Enabled(ch, KPLOG_WARN) == 1);
        kptest::Check("INFO disabled", LogService::Enabled(ch, KPLOG_INFO) == 0);
        kptest::Check("TRACE disabled", LogService::Enabled(ch, KPLOG_TRACE) == 0);
        Submit(ch, KPLOG_INFO, "quiet");
        LogService::FlushNow();
        kptest::Check("a disabled INFO writes nothing", !Exists(P("alpha_log.txt")));
        Submit(ch, KPLOG_WARN, "loud");
        LogService::FlushNow();
        std::vector<std::string> body = Body(P("alpha_log.txt"));
        kptest::Check("WARN is recorded", body.size() == 1 && EndsWith(body[0], "alpha/x WARN loud"));
    }

    Row("ini: enabled switches");
    {
        Begin("[global]\nenabled=0\n[alpha]\nenabled=1\nlevel=4\n");
        int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("global enabled=0 -> WARN only", MaxLevel(ch) == KPLOG_WARN);

        Begin("[global]\nenabled=1\n[alpha]\nenabled=0\nlevel=4\n");
        ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("patch enabled=0 -> WARN only", MaxLevel(ch) == KPLOG_WARN);

        Begin("[global]\nenabled=1\n[alpha]\nlevel=4\n");
        ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("patch section without enabled -> WARN only", MaxLevel(ch) == KPLOG_WARN);

        Begin("[global]\nenabled=1\n[other]\nenabled=1\nlevel=4\n");
        ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("no section for the patch -> WARN only", MaxLevel(ch) == KPLOG_WARN);

        Begin("[global]\nenabled=1\n[alpha]\nenabled=1\n");
        ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("enabled with no level -> INFO", MaxLevel(ch) == KPLOG_INFO);
    }

    Row("ini: level by number and by name");
    {
        const struct { const char* val; int want; } cases[] = {
            {"1", 1}, {"2", 2}, {"3", 3}, {"4", 4}, {"0", 1},
            {"err", 1}, {"warn", 1}, {"info", 2}, {"debug", 3}, {"TRACE", 4}, {"Debug", 3},
        };
        bool allOk = true;
        for (const auto& c : cases) {
            Begin(Ini("alpha", (std::string("level=") + c.val).c_str()).c_str());
            const int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
            if (MaxLevel(ch) != c.want) {
                allOk = false;
                std::printf("      level=%s -> %d, wanted %d\n", c.val, MaxLevel(ch), c.want);
            }
        }
        kptest::Check("level values (0 clamps to WARN)", allOk);

        Begin(Ini("alpha", "level=9\n").c_str());
        int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("out-of-range level ignored -> INFO", MaxLevel(ch) == KPLOG_INFO);

        Begin(Ini("alpha", "level=3   ; verbose please\n# a whole-line comment\n").c_str());
        ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("trailing and full-line comments", MaxLevel(ch) == KPLOG_DEBUG);
    }

    Row("ini: channels list / -x / *");
    {
        Begin(Ini("alpha", "level=4\nchannels=a, b").c_str());
        int32_t p = LogService::Register("alpha");
        int32_t a = LogService::Channel(p, "a"), b = LogService::Channel(p, "b"), c = LogService::Channel(p, "c");
        kptest::Check("listed channels get the level", MaxLevel(a) == 4 && MaxLevel(b) == 4);
        kptest::Check("unlisted channel stays WARN", MaxLevel(c) == KPLOG_WARN);

        Begin(Ini("alpha", "level=4\nchannels=*,-x").c_str());
        p = LogService::Register("alpha");
        const int32_t x = LogService::Channel(p, "x"), y = LogService::Channel(p, "y");
        kptest::Check("* with -x: x excluded", MaxLevel(x) == KPLOG_WARN);
        kptest::Check("* with -x: others on", MaxLevel(y) == 4);

        Begin(Ini("alpha", "level=4\nchannels=*").c_str());
        p = LogService::Register("alpha");
        kptest::Check("* selects everything", MaxLevel(LogService::Channel(p, "anything")) == 4);

        Begin(Ini("alpha", "level=4").c_str());
        p = LogService::Register("alpha");
        kptest::Check("absent channels key selects all", MaxLevel(LogService::Channel(p, "anything")) == 4);

        Begin(Ini("alpha", "level=4\nchannels=a,-a").c_str());
        p = LogService::Register("alpha");
        kptest::Check("exclusion beats inclusion", MaxLevel(LogService::Channel(p, "a")) == KPLOG_WARN);

        Begin(Ini("alpha", "level=4\nchannels=-x").c_str());
        p = LogService::Register("alpha");
        kptest::Check("only exclusions selects all but those",
                      MaxLevel(LogService::Channel(p, "x")) == KPLOG_WARN &&
                      MaxLevel(LogService::Channel(p, "y")) == 4);

        Begin(Ini("alpha", "level=4\nchannels=*,-x").c_str());
        p = LogService::Register("alpha");
        const int32_t q = LogService::Register("beta");
        kptest::Check("another patch's channel is unaffected by the list",
                      MaxLevel(LogService::Channel(q, "y")) == KPLOG_WARN);
    }

    Row("hot reload by content (same second)");
    {
        Begin("[global]\nenabled=1\n[alpha]\nenabled=1\nlevel=1\n");
        const int32_t p = LogService::Register("alpha");
        const int32_t ch = LogService::Channel(p, "x");
        kptest::Check("starts at WARN", MaxLevel(ch) == KPLOG_WARN);
        // Same length as before, rewritten immediately: only a content compare
        // (not mtime or size) can notice this.
        WriteFile(P("kplog.ini"), "[global]\nenabled=1\n[alpha]\nenabled=1\nlevel=3\n");
        kptest::Check("not applied until polled", MaxLevel(ch) == KPLOG_WARN);
        LogService::PollConfigNow();
        kptest::Check("applied on poll", MaxLevel(ch) == KPLOG_DEBUG);
        LogService::PollConfigNow();
        kptest::Check("unchanged content: still applied", MaxLevel(ch) == KPLOG_DEBUG);
        WriteFile(P("kplog.ini"), "[global]\nenabled=1\n[alpha]\nenabled=1\nlevel=4\n");
        LogService::PollConfigNow();
        kptest::Check("second change applied", MaxLevel(ch) == KPLOG_TRACE);
        unlink(P("kplog.ini").c_str());
        LogService::PollConfigNow();
        kptest::Check("removing the file returns to defaults", MaxLevel(ch) == KPLOG_WARN);
        LogService::PollConfigNow();
        kptest::Check("polling a missing file again is stable", MaxLevel(ch) == KPLOG_WARN);
        WriteFile(P("kplog.ini"), "[global]\nenabled=1\n[alpha]\nenabled=1\nlevel=2\n");
        LogService::PollConfigNow();
        kptest::Check("re-creating the file applies it", MaxLevel(ch) == KPLOG_INFO);
    }

    Row("ini: malformed lines ignored");
    {
        const std::string log = CaptureStderr([] {
            Begin("this line has no equals\n"
                  "[global]\n"
                  "enabled=1\n"
                  "bogus_key=5\n"
                  "flush_ms=abc\n"
                  "max_lines_per_sec=-3\n"
                  "sink=sideways\n"
                  "[alpha\n"
                  "[alpha]\n"
                  "enabled=maybe\n"
                  "enabled=1\n"
                  "level=banana\n"
                  "level=3\n"
                  "=novalue\n");
        });
        const int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("valid lines still applied", MaxLevel(ch) == KPLOG_DEBUG);
        kptest::Check("a malformed-lines note was logged once", Contains(log, "malformed") && log.find("malformed") == log.rfind("malformed"));
    }

    Row("channel registered after a reload");
    {
        Begin(Ini("alpha", "level=2").c_str());
        const int32_t p = LogService::Register("alpha");
        const int32_t a = LogService::Channel(p, "a");
        WriteFile(P("kplog.ini"), Ini("alpha", "level=4").c_str());
        LogService::PollConfigNow();
        const int32_t b = LogService::Channel(p, "b");
        kptest::Check("existing channel updated", MaxLevel(a) == 4);
        kptest::Check("new channel gets the current level at once", MaxLevel(b) == 4);
        const int32_t late = LogService::Register("late");
        kptest::Check("channel of a patch with no section is WARN", MaxLevel(LogService::Channel(late, "z")) == KPLOG_WARN);
    }

    Row("Register / Channel limits and idempotence");
    {
        Begin(nullptr);
        const int32_t a = LogService::Register("alpha");
        const int32_t b = LogService::Register("beta");
        kptest::Check("register is idempotent", LogService::Register("alpha") == a && a != b && a >= 0 && b >= 0);
        kptest::Check("channel is idempotent", LogService::Channel(a, "x") == LogService::Channel(a, "x"));
        kptest::Check("same name, other patch -> other channel", LogService::Channel(a, "x") != LogService::Channel(b, "x"));
        kptest::Check("null/empty id rejected", LogService::Register(nullptr) == -1 && LogService::Register("") == -1);
        kptest::Check("illegal characters rejected",
                      LogService::Register("a b") == -1 && LogService::Register("a/b") == -1 &&
                      LogService::Register("a\\b") == -1 && LogService::Register("a:b") == -1);
        kptest::Check("legal punctuation accepted", LogService::Register("my-patch_v1.2") >= 0);
        const std::string id63(63, 'p'), id64(64, 'p');
        kptest::Check("63-char id accepted, 64 rejected", LogService::Register(id63.c_str()) >= 0 && LogService::Register(id64.c_str()) == -1);
        kptest::Check("channel name 31 ok, 32 rejected, empty rejected",
                      LogService::Channel(a, std::string(31, 'c').c_str()) >= 0 &&
                      LogService::Channel(a, std::string(32, 'c').c_str()) == -1 &&
                      LogService::Channel(a, "") == -1 && LogService::Channel(a, nullptr) == -1);
        kptest::Check("bad patch handle for Channel",
                      LogService::Channel(-1, "x") == -1 && LogService::Channel(63, "x") == -1 &&
                      LogService::Channel(1000, "x") == -1);

        Begin(nullptr);
        bool all = true;
        int32_t last = -1;
        for (int i = 0; i < 64; ++i) {
            const std::string id = "p" + std::to_string(i);
            last = LogService::Register(id.c_str());
            all = all && last == i;
        }
        kptest::Check("64 patches accepted, handles 0..63", all);
        kptest::Check("65th patch rejected", LogService::Register("one-too-many") == -1);
        kptest::Check("existing patch still resolves at the limit", LogService::Register("p5") == 5);

        Begin(nullptr);
        const int32_t p = LogService::Register("alpha");
        all = true;
        for (int i = 0; i < 256; ++i) {
            const std::string name = "c" + std::to_string(i);
            all = all && LogService::Channel(p, name.c_str()) == i;
        }
        kptest::Check("256 channels accepted, handles 0..255", all);
        kptest::Check("257th channel rejected", LogService::Channel(p, "c256") == -1);
        kptest::Check("existing channel still resolves at the limit", LogService::Channel(p, "c7") == 7);
        const int32_t q = LogService::Register("beta");
        kptest::Check("limit is across all patches", LogService::Channel(q, "c0") == -1);
    }

    Row("truncation marker");
    {
        Begin(Ini("alpha", "level=2").c_str());
        const int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        Submit(ch, KPLOG_INFO, std::string(500, 'x'));
        Submit(ch, KPLOG_INFO, std::string(232, 'y'));
        Submit(ch, KPLOG_INFO, std::string(233, 'z'));
        LogService::FlushNow();
        const std::vector<std::string> body = Body(P("alpha_log.txt"));
        const std::string prefix = "T=0.000 f=- alpha/x ";
        kptest::Check("three lines", body.size() == 3);
        const std::string msg0 = body.size() > 0 ? body[0].substr(prefix.size()) : "";
        kptest::Check("over-long line is cut to 232 with [...]", msg0.size() == 232 && EndsWith(msg0, "xx[...]") && msg0.compare(0, 227, std::string(227, 'x')) == 0);
        const std::string msg1 = body.size() > 1 ? body[1].substr(prefix.size()) : "";
        kptest::Check("exactly 232 is kept whole", msg1 == std::string(232, 'y'));
        const std::string msg2 = body.size() > 2 ? body[2].substr(prefix.size()) : "";
        kptest::Check("233 is truncated", msg2.size() == 232 && EndsWith(msg2, "[...]"));
    }

    Row("null / zero-length lines, bad ids");
    {
        Begin(Ini("alpha", "level=4").c_str());
        const int32_t p = LogService::Register("alpha");
        const int32_t ch = LogService::Channel(p, "x");
        LogService::Submit(ch, KPLOG_INFO, nullptr, 5);
        LogService::Submit(ch, KPLOG_INFO, "abc", 0);
        LogService::Submit(ch, KPLOG_INFO, "\n", 1);
        LogService::Submit(-1, KPLOG_INFO, "bad", 3);
        LogService::Submit(5000, KPLOG_INFO, "bad", 3);
        LogService::Submit(ch + 1, KPLOG_INFO, "bad", 3);  // not registered yet
        LogService::Submit(ch, -1, "bad", 3);
        LogService::Submit(ch, 99, "bad", 3);
        LogService::Mark(nullptr);
        LogService::FrameTick(-1);
        LogService::FrameTick(77);
        LogService::FlushNow();
        kptest::Check("null/empty/bad submits write nothing, no crash", !Exists(P("alpha_log.txt")) && !Exists(P("kp_log.txt")));
        kptest::Check("Enabled rejects bad ids",
                      LogService::Enabled(-1, 0) == 0 && LogService::Enabled(1000, 0) == 0 &&
                      LogService::Enabled(ch + 1, 0) == 0 && LogService::Enabled(ch, -1) == 0 &&
                      LogService::Enabled(ch, 5) == 0 && LogService::Enabled(ch, 99) == 0);
        kptest::Check("bad FrameTick did not claim ownership", LogService::Frame() == 0);
        Submit(ch, KPLOG_INFO, "good");
        LogService::FlushNow();
        kptest::Check("a good line afterwards lands", Body(P("alpha_log.txt")).size() == 1);
    }

    Row("sinks: perpatch / merged / both");
    {
        const char* sinks[] = {"perpatch", "merged", "both"};
        for (int i = 0; i < 3; ++i) {
            const std::string ini = std::string("[global]\nenabled=1\nsink=") + sinks[i] +
                                    "\n[alpha]\nenabled=1\n[beta]\nenabled=1\n";
            Begin(ini.c_str());
            const int32_t a = LogService::Channel(LogService::Register("alpha"), "x");
            const int32_t b = LogService::Channel(LogService::Register("beta"), "y");
            Submit(a, KPLOG_INFO, "from alpha");
            Submit(b, KPLOG_INFO, "from beta");
            LogService::FlushNow();
            const bool wantPer = i != 1, wantMerged = i != 0;
            char what[64];
            std::snprintf(what, sizeof(what), "%s: per-patch files", sinks[i]);
            kptest::Check(what, Exists(P("alpha_log.txt")) == wantPer && Exists(P("beta_log.txt")) == wantPer);
            std::snprintf(what, sizeof(what), "%s: merged file", sinks[i]);
            kptest::Check(what, Exists(P("kp_log.txt")) == wantMerged);
            if (wantPer) {
                const auto la = Body(P("alpha_log.txt")), lb = Body(P("beta_log.txt"));
                std::snprintf(what, sizeof(what), "%s: each file has its own line", sinks[i]);
                kptest::Check(what, la.size() == 1 && lb.size() == 1 && Contains(la[0], "alpha/x from alpha") && Contains(lb[0], "beta/y from beta"));
            }
            if (wantMerged) {
                const auto lm = Body(P("kp_log.txt"));
                std::snprintf(what, sizeof(what), "%s: merged has both, in order", sinks[i]);
                kptest::Check(what, lm.size() == 2 && Contains(lm[0], "alpha/x from alpha") && Contains(lm[1], "beta/y from beta"));
            }
        }
    }

    Row("byte cap rotation");
    {
        // A stale .1 with no main file: launch rotation does not apply, so only the
        // cap rotation can replace it.
        Begin(Ini("alpha", "level=2", "max_bytes=2000").c_str(), [] {
            WriteFile(P("alpha_log.txt.1"), "STALE-GENERATION\n");
        });
        const int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        for (int i = 0; i < 30; ++i) {
            char text[128];
            std::snprintf(text, sizeof(text), "SEQ%03d %s", i, std::string(80, '.').c_str());
            Submit(ch, KPLOG_INFO, text);
        }
        LogService::FlushNow();
        const std::string cur = ReadFile(P("alpha_log.txt")), old = ReadFile(P("alpha_log.txt.1"));
        kptest::Check("rotated to .1", !old.empty());
        kptest::Check("stale .1 was replaced", !Contains(old, "STALE-GENERATION"));
        kptest::Check("old file starts with a session header", old.compare(0, 16, "# kplog session ") == 0);
        kptest::Check("new file starts with a fresh header", cur.compare(0, 16, "# kplog session ") == 0);
        kptest::Check("both files within the cap", SizeOf(P("alpha_log.txt")) <= 2000 && SizeOf(P("alpha_log.txt.1")) <= 2000);
        const int total = CountContaining(Body(P("alpha_log.txt")), "SEQ") + CountContaining(Body(P("alpha_log.txt.1")), "SEQ");
        kptest::Check("no line lost across one rotation", total == 30);
        kptest::Check("only one .1 generation", !Exists(P("alpha_log.txt.2")));
        kptest::Check("rotation boundary is in order",
                      Contains(old, "SEQ000") && Contains(cur, "SEQ029") && !Contains(cur, "SEQ000"));

        // Keep going until the second rotation: .1 must be replaced again.
        for (int i = 30; i < 90; ++i) {
            char text[128];
            std::snprintf(text, sizeof(text), "SEQ%03d %s", i, std::string(80, '.').c_str());
            Submit(ch, KPLOG_INFO, text);
            if (i % 10 == 9) LogService::FlushNow();
        }
        LogService::FlushNow();
        const std::string old2 = ReadFile(P("alpha_log.txt.1"));
        kptest::Check("later rotations replace the existing .1", !Contains(old2, "SEQ000") && Contains(old2, "SEQ0") && old2 != old);
        kptest::Check("still within the cap", SizeOf(P("alpha_log.txt")) <= 2000 && SizeOf(P("alpha_log.txt.1")) <= 2000);
    }

    Row("launch rotation");
    {
        Begin(Ini("alpha", "level=2").c_str(), [] {
            WriteFile(P("alpha_log.txt"), "PREVIOUS SESSION\n");
            WriteFile(P("alpha_log.txt.1"), "ANCIENT\n");
        });
        const int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("files untouched until the first line", ReadFile(P("alpha_log.txt")) == "PREVIOUS SESSION\n");
        Submit(ch, KPLOG_INFO, "first");
        LogService::FlushNow();
        kptest::Check("previous session moved to .1", ReadFile(P("alpha_log.txt.1")) == "PREVIOUS SESSION\n");
        kptest::Check("older .1 discarded", !Contains(ReadFile(P("alpha_log.txt.1")), "ANCIENT"));
        std::string cur = ReadFile(P("alpha_log.txt"));
        kptest::Check("new file has header and the line", cur.compare(0, 16, "# kplog session ") == 0 && Contains(cur, "alpha/x first"));
        Submit(ch, KPLOG_INFO, "second");
        LogService::FlushNow();
        cur = ReadFile(P("alpha_log.txt"));
        kptest::Check("rotation happens once per session", Contains(cur, "first") && Contains(cur, "second") &&
                      ReadFile(P("alpha_log.txt.1")) == "PREVIOUS SESSION\n");
    }

    Row("rate limit");
    {
        Begin(Ini("alpha", "level=2", "max_lines_per_sec=10").c_str());
        const int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        for (int i = 0; i < 25; ++i) Submit(ch, KPLOG_INFO, "info " + std::to_string(i));
        for (int i = 0; i < 5; ++i) Submit(ch, KPLOG_WARN, "warn " + std::to_string(i));
        for (int i = 0; i < 3; ++i) Submit(ch, KPLOG_ERR, "err " + std::to_string(i));
        LogService::FlushNow();
        std::vector<std::string> body = Body(P("alpha_log.txt"));
        kptest::Check("10 INFO accepted", CountContaining(body, "alpha/x info ") == 10);
        kptest::Check("WARN/ERR exempt", CountContaining(body, "WARN warn ") == 5 && CountContaining(body, "ERR err ") == 3);
        kptest::Check("DROPPED line with the rate count", CountContaining(body, "alpha/kplog DROPPED 15 (ring 0, rate 15)") == 1);
        kptest::Check("DROPPED line format", CountContaining(body, "T=0.000 f=- alpha/kplog DROPPED") == 1);

        LogService::FlushNow();
        kptest::Check("drop counters reset after reporting", CountContaining(Body(P("alpha_log.txt")), "DROPPED") == 1);

        g_now = 999999;
        Submit(ch, KPLOG_INFO, "still inside the window");
        LogService::FlushNow();
        kptest::Check("same window: still limited", CountContaining(Body(P("alpha_log.txt")), "still inside the window") == 0);

        g_now = 1000000;
        for (int i = 0; i < 12; ++i) Submit(ch, KPLOG_INFO, "next " + std::to_string(i));
        LogService::FlushNow();
        body = Body(P("alpha_log.txt"));
        kptest::Check("new window accepts 10 more", CountContaining(body, "alpha/x next ") == 10);
        kptest::Check("and reports its own drops", CountContaining(body, "DROPPED 2 (ring 0, rate 2)") == 1);
    }

    Row("ring overflow: non-blocking, WARN reserve");
    {
        Begin(Ini("alpha", "level=2", "max_lines_per_sec=0").c_str());
        const int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        const auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < 100000; ++i) LogService::Submit(ch, KPLOG_INFO, "spam line", 9);
        const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        char detail[64];
        std::snprintf(detail, sizeof(detail), "(%.3f s)", secs);
        kptest::Check("100k submits without a flush finish fast", secs < 2.0, detail);

        for (int i = 0; i < 5; ++i) Submit(ch, KPLOG_WARN, "reserved " + std::to_string(i));
        Submit(ch, KPLOG_ERR, "reserved err");
        LogService::FlushNow();
        std::vector<std::string> body = Body(P("alpha_log.txt"));
        kptest::Check("INFO stops at 75% of the ring", CountContaining(body, "alpha/x spam line") == 3072);
        kptest::Check("WARN/ERR still land in the reserve", CountContaining(body, "WARN reserved ") == 5 && CountContaining(body, "ERR reserved err") == 1);
        kptest::Check("ring drop count reported", CountContaining(body, "alpha/kplog DROPPED 96928 (ring 96928, rate 0)") == 1);

        // Fill the whole ring, then run out of reserve too.
        for (int i = 0; i < 3172; ++i) LogService::Submit(ch, KPLOG_INFO, "spam2", 5);
        for (int i = 0; i < 1100; ++i) Submit(ch, KPLOG_WARN, "w" + std::to_string(i));
        LogService::FlushNow();
        body = Body(P("alpha_log.txt"));
        kptest::Check("reserve holds exactly the last 25%", CountContaining(body, "alpha/x WARN w") == 1024);
        kptest::Check("lines beyond a full ring are dropped, counted", CountContaining(body, "DROPPED 176 (ring 176, rate 0)") == 1);
    }

    Row("8 threads x 10k submits with concurrent flushes");
    {
        Begin(Ini("mt", "level=2", "max_lines_per_sec=0\nmax_bytes=1000000000").c_str());
        const int32_t ch = LogService::Channel(LogService::Register("mt"), "t");
        std::atomic<int> done{0};
        std::vector<std::thread> threads;
        for (int t = 0; t < 8; ++t) {
            threads.emplace_back([t, ch, &done] {
                char buf[48];
                for (int i = 0; i < 10000; ++i) {
                    const int n = std::snprintf(buf, sizeof(buf), "T%d S%d", t, i);
                    LogService::Submit(ch, KPLOG_INFO, buf, static_cast<uint32_t>(n));
                }
                ++done;
            });
        }
        int flushes = 0;
        while (done.load() < 8) {
            LogService::FlushNow();
            ++flushes;
        }
        for (std::thread& th : threads) th.join();
        LogService::FlushNow();

        int lastSeq[8];
        for (int& v : lastSeq) v = -1;
        long flushed = 0, dropped = 0;
        bool ordered = true;
        for (const std::string& line : Body(P("mt_log.txt"))) {
            int t, seq;
            if (std::sscanf(line.c_str(), "T=%*s f=%*s mt/t T%d S%d", &t, &seq) == 2 && t >= 0 && t < 8) {
                ++flushed;
                if (seq <= lastSeq[t]) ordered = false;
                lastSeq[t] = seq;
                continue;
            }
            const size_t at = line.find("mt/kplog DROPPED ");
            if (at != std::string::npos) dropped += std::atol(line.c_str() + at + 17);
        }
        char detail[96];
        std::snprintf(detail, sizeof(detail), "(flushed %ld, dropped %ld, %d concurrent flushes)", flushed, dropped, flushes);
        kptest::Check("flushed + dropped == 80000", flushed + dropped == 80000, detail);
        kptest::Check("each thread's sequence is increasing", ordered);
        kptest::Check("something was flushed", flushed > 0);
    }

    Row("Mark reaches every open file");
    {
        Begin("[global]\nenabled=1\nsink=both\n[alpha]\nenabled=1\n[beta]\nenabled=1\n");
        const int32_t a = LogService::Channel(LogService::Register("alpha"), "x");
        const int32_t b = LogService::Channel(LogService::Register("beta"), "y");
        Submit(a, KPLOG_INFO, "one");
        Submit(b, KPLOG_INFO, "two");
        LogService::FlushNow();
        g_now = 42000;
        LogService::Mark("F9 pressed");
        LogService::FlushNow();
        const char* want = "T=42.000 f=- MARK F9 pressed";
        kptest::Check("mark in alpha_log", CountContaining(Body(P("alpha_log.txt")), want) == 1);
        kptest::Check("mark in beta_log", CountContaining(Body(P("beta_log.txt")), want) == 1);
        kptest::Check("mark in kp_log (once)", CountContaining(Body(P("kp_log.txt")), want) == 1);

        Begin("[global]\nenabled=1\nsink=perpatch\n[alpha]\nenabled=1\n");
        LogService::Mark("nobody listening");
        LogService::FlushNow();
        kptest::Check("perpatch with no open file: mark goes nowhere", !Exists(P("alpha_log.txt")) && !Exists(P("kp_log.txt")));

        Begin("[global]\nenabled=1\nsink=merged\n");
        LogService::Mark("to merged");
        LogService::FlushNow();
        kptest::Check("merged sink: mark opens kp_log", CountContaining(Body(P("kp_log.txt")), "MARK to merged") == 1);
    }

    Row("session header");
    {
        Begin(Ini("alpha", "level=2").c_str());
        const int32_t a = LogService::Channel(LogService::Register("alpha"), "x");
        Submit(a, KPLOG_INFO, "first");
        LogService::FlushNow();
        std::vector<std::string> lines = Split(ReadFile(P("alpha_log.txt")));
        bool allHash = lines.size() >= 4 && lines[0].compare(0, 2, "# ") == 0 && lines[1].compare(0, 2, "# ") == 0 && lines[2].compare(0, 2, "# ") == 0;
        kptest::Check("header lines start with '# '", allHash);
        int y, mo, d, h, mi, sec;
        const bool okFirst = lines.size() > 0 &&
            std::sscanf(lines[0].c_str(), "# kplog session %d-%d-%d %d:%d:%d | kpatch.log v1 | patcher ABI 1", &y, &mo, &d, &h, &mi, &sec) == 6 &&
            y >= 2024 && mo >= 1 && mo <= 12;
        kptest::Check("session line: wall clock | service | ABI", okFirst);
        kptest::Check("SHA unknown before SetSessionInfo", lines.size() > 1 && lines[1] == "# KOTOR_VERSION_SHA=unknown");
        kptest::Check("interfaces not yet applied", lines.size() > 2 && lines[2] == "# interfaces: (not yet applied)");

        LogService::SetSessionInfo("abc123", "kpatch.log v1 (patcher)\nother.svc v2 (some-patch)\n");
        kptest::Check("nothing written until the next flush", !Contains(ReadFile(P("alpha_log.txt")), "session info"));
        LogService::FlushNow();
        const std::string after = ReadFile(P("alpha_log.txt"));
        kptest::Check("open file gets a '# session info' block",
                      Contains(after, "# session info:\n# KOTOR_VERSION_SHA=abc123\n# interfaces:\n# kpatch.log v1 (patcher)\n# other.svc v2 (some-patch)\n"));
        LogService::FlushNow();
        const std::string again = ReadFile(P("alpha_log.txt"));
        kptest::Check("the block is written once", again == after);

        const int32_t b = LogService::Channel(LogService::Register("beta"), "y");
        WriteFile(P("kplog.ini"), "[global]\nenabled=1\n[beta]\nenabled=1\n");
        LogService::PollConfigNow();
        Submit(b, KPLOG_INFO, "late file");
        LogService::FlushNow();
        const std::string beta = ReadFile(P("beta_log.txt"));
        kptest::Check("file opened later has the info in its header",
                      Contains(beta, "# KOTOR_VERSION_SHA=abc123\n# interfaces:\n# kpatch.log v1 (patcher)\n# other.svc v2 (some-patch)\n") &&
                      !Contains(beta, "not yet applied"));
        kptest::Check("...and no separate session-info block", !Contains(beta, "# session info:"));
        kptest::Check("body lines are not header lines", Body(P("beta_log.txt")).size() == 1);
    }

    Row("unwritable directory");
    {
        LogService::Shutdown();
        Wipe(g_dir);
        const std::string missing = g_dir + "/no/such/dir";
        g_now = 0;
        LogService::SetClockForTest(&FakeClock);
        LogService::Init(missing, LogService::Options{false});
        const int32_t a = LogService::Channel(LogService::Register("alpha"), "x");
        const int32_t b = LogService::Channel(LogService::Register("beta"), "x");
        for (int i = 0; i < 3; ++i) {
            Submit(a, KPLOG_WARN, "to alpha");
            Submit(b, KPLOG_WARN, "to beta");
        }
        const std::string log = CaptureStderr([] { LogService::FlushNow(); });
        int lines = 0;
        for (const std::string& l : Split(log)) {
            if (l.find("kplog: cannot open") != std::string::npos) ++lines;
        }
        kptest::Check("exactly one line per path", lines == 2);
        kptest::Check("line names the path",
                      Contains(log, ("cannot open " + missing + "/alpha_log.txt, dropping its lines").c_str()) &&
                      Contains(log, ("cannot open " + missing + "/beta_log.txt, dropping its lines").c_str()));
        Submit(a, KPLOG_WARN, "more");
        const std::string log2 = CaptureStderr([] { LogService::FlushNow(); });
        kptest::Check("no repeat for later lines", !Contains(log2, "cannot open"));
        LogService::Mark("m");
        LogService::FlushNow();

        const std::string ro = g_dir + "/ro";
        mkdir(ro.c_str(), 0755);
        chmod(ro.c_str(), 0500);
        if (access(ro.c_str(), W_OK) != 0) {
            LogService::Shutdown();
            LogService::Init(ro, LogService::Options{false});
            const int32_t c = LogService::Channel(LogService::Register("gamma"), "x");
            Submit(c, KPLOG_ERR, "x");
            Submit(c, KPLOG_ERR, "y");
            const std::string log3 = CaptureStderr([] { LogService::FlushNow(); });
            kptest::Check("read-only dir: one line, no crash", log3.find("cannot open") != std::string::npos && log3.find("cannot open") == log3.rfind("cannot open"));
        } else {
            std::printf("    (read-only directory variant skipped: running with write access to a 0500 dir)\n");
        }
        chmod(ro.c_str(), 0700);
    }

    Row("shutdown flushes; submit after shutdown");
    {
        Begin(Ini("alpha", "level=2").c_str());
        const int32_t p = LogService::Register("alpha");
        const int32_t ch = LogService::Channel(p, "x");
        Submit(ch, KPLOG_INFO, "pending 1");
        Submit(ch, KPLOG_INFO, "pending 2");
        kptest::Check("nothing on disk before flush", !Exists(P("alpha_log.txt")));
        LogService::Shutdown();
        const std::string content = ReadFile(P("alpha_log.txt"));
        kptest::Check("Shutdown flushed the ring", Contains(content, "alpha/x pending 1") && Contains(content, "pending 2"));

        Submit(ch, KPLOG_INFO, "after shutdown");
        Submit(ch, KPLOG_ERR, "after shutdown err");
        LogService::Mark("after shutdown");
        LogService::FrameTick(p);
        LogService::FlushNow();
        LogService::Shutdown();  // twice is harmless
        kptest::Check("Submit/Mark after Shutdown change nothing", ReadFile(P("alpha_log.txt")) == content);
        kptest::Check("Enabled/Register/Channel are inert",
                      LogService::Enabled(ch, KPLOG_ERR) == 0 && LogService::Register("alpha") == -1 &&
                      LogService::Channel(p, "x") == -1 && LogService::Frame() == 0);

        // A new session in the same directory, without Begin()'s wipe: the old
        // session's file is what launch rotation has to preserve.
        {
            LogService::Options opt;
            opt.startThread = false;
            LogService::Init(g_dir, opt);
        }
        const int32_t p2 = LogService::Register("alpha");
        const int32_t ch2 = LogService::Channel(p2, "x");
        kptest::Check("re-Init starts a fresh session", p2 == 0 && ch2 == 0 && LogService::Frame() == 0);
        Submit(ch2, KPLOG_INFO, "second session");
        LogService::FlushNow();
        kptest::Check("previous session kept as .1", Contains(ReadFile(P("alpha_log.txt.1")), "pending 1"));
        kptest::Check("new session in the main file",
                      Contains(ReadFile(P("alpha_log.txt")), "second session") && !Contains(ReadFile(P("alpha_log.txt")), "pending 1"));
    }

    // ---- rows with the real flush thread ------------------------------------------

    Row("real flush thread: WARN reaches disk, INFO via the periodic flush");
    {
        // flush_ms=5000 so only the WARN wake can explain a fast arrival.
        BeginThreaded("[global]\nenabled=1\nflush_ms=5000\n[alpha]\nenabled=1\nlevel=2\n");
        const int32_t p = LogService::Register("alpha");
        const int32_t ch = LogService::Channel(p, "x");
        Submit(ch, KPLOG_WARN, "thread warn");
        const long warnMs = WaitFor([] { return Contains(ReadFile(P("alpha_log.txt")), "alpha/x WARN thread warn"); }, 1500);
        char detail[64];
        std::snprintf(detail, sizeof(detail), "(observed %ld ms, flush_ms=5000)", warnMs);
        kptest::Check("WARN on disk within 1500 ms (wake)", warnMs >= 0, detail);
        kptest::Check("WARN wake beat the 5 s period", warnMs >= 0 && warnMs < 1000);
        LogService::Shutdown();

        BeginThreaded("[global]\nenabled=1\n[alpha]\nenabled=1\nlevel=2\n");
        const int32_t p2 = LogService::Register("alpha");
        const int32_t ch2 = LogService::Channel(p2, "x");
        Submit(ch2, KPLOG_INFO, "thread info");
        const long infoMs = WaitFor([] { return Contains(ReadFile(P("alpha_log.txt")), "alpha/x thread info"); }, 1500);
        std::snprintf(detail, sizeof(detail), "(observed %ld ms, flush_ms=500)", infoMs);
        kptest::Check("INFO on disk within 1500 ms (periodic)", infoMs >= 0, detail);
        kptest::Check("INFO line has no WARN marker", !Contains(ReadFile(P("alpha_log.txt")), "WARN thread info"));
        Submit(ch2, KPLOG_ERR, "thread err");
        const long errMs = WaitFor([] { return Contains(ReadFile(P("alpha_log.txt")), "alpha/x ERR thread err"); }, 1500);
        std::snprintf(detail, sizeof(detail), "(observed %ld ms)", errMs);
        kptest::Check("ERR on disk within 1500 ms", errMs >= 0, detail);
        LogService::Mark("thread mark");
        const long markMs = WaitFor([] { return Contains(ReadFile(P("alpha_log.txt")), "MARK thread mark"); }, 1500);
        std::snprintf(detail, sizeof(detail), "(observed %ld ms)", markMs);
        kptest::Check("Mark on disk within 1500 ms", markMs >= 0, detail);
        LogService::Shutdown();
    }

    Row("real flush thread: hot reload via the thread");
    {
        BeginThreaded(nullptr);
        const int32_t ch = LogService::Channel(LogService::Register("alpha"), "x");
        kptest::Check("INFO off without an ini", !LogService::Enabled(ch, KPLOG_INFO));
        WriteFile(P("kplog.ini"), "[global]\nenabled=1\n[alpha]\nenabled=1\nlevel=2\n");
        const long ms = WaitFor([ch] { return LogService::Enabled(ch, KPLOG_INFO) != 0; }, 2500);
        char detail[64];
        std::snprintf(detail, sizeof(detail), "(observed %ld ms)", ms);
        kptest::Check("thread picked up the ini within 2500 ms", ms >= 0, detail);
        LogService::Shutdown();
    }

    Row("real flush thread: Shutdown with the ring lock held is bounded");
    {
        BeginThreaded(nullptr);
        LogService::Channel(LogService::Register("alpha"), "x");
        std::atomic<bool> held{false};
        // The helper owns the lock: unlocking or try-locking a std::mutex from a
        // thread that does not hold it (or that already does) is undefined.
        std::thread helper([&held] {
            LogService::LockRingForTest();
            held = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(900));
            LogService::UnlockRingForTest();
        });
        while (!held.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
        const auto t0 = Clock::now();
        const std::string log = CaptureStderr([] { LogService::Shutdown(); });
        const long ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t0).count();
        char detail[64];
        std::snprintf(detail, sizeof(detail), "(returned after %ld ms)", ms);
        kptest::Check("Shutdown returned in < 600 ms", ms < 600, detail);
        kptest::Check("...but waited for the lock first", ms >= 150, detail);
        kptest::Check("logged 'final flush skipped (lock busy)'", Contains(log, "kplog: final flush skipped (lock busy)"));
        kptest::Check("service is inert afterwards", LogService::Register("alpha") == -1);
        helper.join();
        // A new session must still start cleanly after the abandoned shutdown.
        BeginThreaded(nullptr);
        const int32_t c2 = LogService::Channel(LogService::Register("alpha"), "x");
        Submit(c2, KPLOG_WARN, "after abandoned shutdown");
        const long arrived = WaitFor([] { return Contains(ReadFile(P("alpha_log.txt")), "after abandoned shutdown"); }, 1500);
        kptest::Check("next session works", arrived >= 0);
        LogService::Shutdown();
    }

    Row("real flush thread: an old session's thread leaves a new session alone");
    {
        BeginThreaded("[global]\nenabled=1\nflush_ms=100\n");
        const int32_t a = LogService::Channel(LogService::Register("alpha"), "x");
        Submit(a, KPLOG_WARN, "old session");
        LogService::Shutdown();
        // Second session: no thread of its own, so nothing but the (stale) first
        // thread could flush it. flush_ms=100 there too, so a thread that kept
        // running would have flushed several times during the wait below.
        LogService::SetClockForTest(nullptr);
        WriteFile(P("kplog.ini"), "[global]\nenabled=1\nflush_ms=100\n");
        LogService::Options noThread;
        noThread.startThread = false;
        LogService::Init(g_dir, noThread);
        const int32_t b = LogService::Channel(LogService::Register("alpha"), "x");
        Submit(b, KPLOG_WARN, "new session");  // this also wakes the old thread
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        // The first session's file is still there (rotation happens on the new
        // session's first write), so the check is that the new line is not in it
        // and the file was not rotated yet.
        kptest::Check("stale thread did not flush the new session",
                      !Contains(ReadFile(P("alpha_log.txt")), "new session") && !Exists(P("alpha_log.txt.1")));
        LogService::FlushNow();
        kptest::Check("owner's FlushNow does", Contains(ReadFile(P("alpha_log.txt")), "alpha/x WARN new session"));
        kptest::Check("old session survived as .1", CountContaining(Body(P("alpha_log.txt.1")), "old session") == 1);
        LogService::Shutdown();

        // The same race at its tightest: the old thread has to still be asleep (or
        // just woken) when the next Init() clears `stopping`, which only happens if
        // Init follows Shutdown immediately. Repeated so the window is hit.
        int leaked = 0;
        for (int i = 0; i < 100; ++i) {
            Wipe(g_dir);
            LogService::SetClockForTest(nullptr);
            LogService::Init(g_dir, LogService::Options());
            LogService::Shutdown();
            LogService::Init(g_dir, noThread);
            const int32_t e = LogService::Channel(LogService::Register("alpha"), "x");
            Submit(e, KPLOG_WARN, "churn");  // wakes a thread that is still waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            if (Contains(ReadFile(P("alpha_log.txt")), "churn")) ++leaked;
            LogService::Shutdown();
        }
        char churn[64];
        std::snprintf(churn, sizeof(churn), "(100 Init/Shutdown/Init cycles, %d leaked)", leaked);
        kptest::Check("no stale thread flush in rapid session churn", leaked == 0, churn);
        LogService::Shutdown();

        // Back to back thread sessions: right files, one header each, no duplicates.
        Wipe(g_dir);
        BeginThreaded(nullptr);
        const int32_t c = LogService::Channel(LogService::Register("alpha"), "x");
        Submit(c, KPLOG_WARN, "first");
        LogService::Shutdown();
        LogService::Options withThread;
        LogService::Init(g_dir, withThread);
        const int32_t d = LogService::Channel(LogService::Register("alpha"), "x");
        Submit(d, KPLOG_WARN, "second");
        const long ms = WaitFor([] { return Contains(ReadFile(P("alpha_log.txt")), "WARN second"); }, 1500);
        char detail[64];
        std::snprintf(detail, sizeof(detail), "(observed %ld ms)", ms);
        kptest::Check("second thread session reaches disk", ms >= 0, detail);
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        const std::string cur = ReadFile(P("alpha_log.txt"));
        int headers = 0;
        for (size_t at = cur.find("# kplog session"); at != std::string::npos; at = cur.find("# kplog session", at + 1)) ++headers;
        kptest::Check("current file: one header, only the new line",
                      headers == 1 && CountContaining(Body(P("alpha_log.txt")), "WARN second") == 1 && !Contains(cur, "WARN first"));
        kptest::Check("previous file: only the old line",
                      CountContaining(Body(P("alpha_log.txt.1")), "WARN first") == 1 &&
                      !Contains(ReadFile(P("alpha_log.txt.1")), "WARN second"));
        LogService::Shutdown();
    }

    Row("real flush thread: Submit after Shutdown is a no-op");
    {
        BeginThreaded("[global]\nenabled=1\n[alpha]\nenabled=1\nlevel=2\n");
        const int32_t p = LogService::Register("alpha");
        const int32_t ch = LogService::Channel(p, "x");
        Submit(ch, KPLOG_WARN, "before shutdown");
        LogService::Shutdown();
        const std::string content = ReadFile(P("alpha_log.txt"));
        kptest::Check("Shutdown flushed the pending WARN", Contains(content, "WARN before shutdown"));
        Submit(ch, KPLOG_WARN, "after shutdown");
        Submit(ch, KPLOG_ERR, "after shutdown err");
        LogService::Mark("after shutdown");
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        kptest::Check("nothing written after Shutdown", ReadFile(P("alpha_log.txt")) == content);
    }

    LogService::Shutdown();
    Wipe(g_dir);
    rmdir(g_dir.c_str());
    return kptest::Report();
}
