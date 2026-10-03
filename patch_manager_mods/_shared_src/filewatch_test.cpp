// Host test of filewatch (FileWatch.h): stamp resolution, throttled change polling, atomic write + one-time .bak.
// POSIX build of the header (st_mtim ns); files live in a mkdtemp dir under $TMPDIR and are removed at exit.
#include "FileWatch.h"
#include "check.h"
#include <filesystem>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace filewatch;

static std::string g_dir;

static std::string P(const char* name) { return g_dir + "/" + name; }
static void put(const std::string& p, const std::string& s) { FILE* f = fopen(p.c_str(), "wb"); if (f) { fwrite(s.data(), 1, s.size(), f); fclose(f); } }
static std::string get(const std::string& p) {
    std::string o; FILE* f = fopen(p.c_str(), "rb"); if (!f) return "<missing>";
    char b[256]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) o.append(b, n);
    fclose(f); return o;
}
static bool exists(const std::string& p) { struct stat st; return lstat(p.c_str(), &st) == 0; }
static bool setMtime(const std::string& p, long sec, long nsec) {
    timespec ts[2] = { {sec, nsec}, {sec, nsec} };
    return utimensat(AT_FDCWD, p.c_str(), ts, 0) == 0;
}

static void test_stamp() {
    std::string f = P("a.ini");
    Stamp m = stamp(f.c_str());
    KT_CHECK(!m.exists);
    KT_CHECK(m == stamp((f + "2").c_str()));   // two missing files compare equal
    put(f, "aaa");
    Stamp s1 = stamp(f.c_str());
    KT_CHECK(s1.exists);
    KT_CHECK_EQ(s1.size, 3ull);
    KT_CHECK(s1 != m);
    KT_CHECK(s1 == stamp(f.c_str()));           // unchanged file: equal
    // Same second, different size: detected (the hotbar's old 1 s stat resolution missed this).
    setMtime(f, 1700000000, 0); Stamp t1 = stamp(f.c_str());
    put(f, "aaaa"); setMtime(f, 1700000000, 0);   // same second (even same ns), size differs
    KT_CHECK(stamp(f.c_str()) != t1);
    // Same size, same second, different content: only the sub-second part differs.
    put(f, "aaaa"); setMtime(f, 1700000000, 123456789);
    Stamp n1 = stamp(f.c_str());
    if (n1.mtime % 1000000000LL != 123456789LL) {
        std::printf("note: filesystem has no ns mtime; same-second/same-size check skipped\n");
    } else {
        put(f, "bbbb"); setMtime(f, 1700000000, 123456790);
        Stamp n2 = stamp(f.c_str());
        KT_CHECK_EQ(n2.size, n1.size);
        KT_CHECK_EQ(n2.mtime / 1000000000LL, n1.mtime / 1000000000LL);   // same second
        KT_CHECK(n2 != n1);                                              // ns differs -> detected
        KT_CHECK_EQ(n1.mtimeMs(), 1700000000123LL);
    }
}

static void test_poll() {
    std::string f = P("p.ini");
    ReloadPoll p; p.throttleMs = 1000;
    KT_CHECK_EQ(p.seenMs(), -2LL);              // unprimed
    // First call is a change (nothing seen yet), also for a missing file: the hotbar loads / writes its template then.
    KT_CHECK(changed(p, f.c_str(), 0));
    KT_CHECK_EQ(p.seenMs(), -1LL);              // missing
    KT_CHECK(!changed(p, f.c_str(), 1000));     // still missing: no change
    put(f, "one"); setMtime(f, 1700000000, 0);
    KT_CHECK(!changed(p, f.c_str(), 1500));     // inside the window (since 1000): no stamp taken
    KT_CHECK(changed(p, f.c_str(), 2000));      // missing -> present
    KT_CHECK_EQ(p.seenMs(), 1700000000000LL);
    KT_CHECK(!changed(p, f.c_str(), 3000));     // unchanged
    put(f, "twotwo"); setMtime(f, 1700000000, 0);   // same second, size differs
    KT_CHECK(!changed(p, f.c_str(), 3999));     // throttled (window began at 3000)
    KT_CHECK(changed(p, f.c_str(), 4000));      // exactly one window later
    put(f, "XXXXXX"); setMtime(f, 1700000000, 0);   // same size + same mtime: invisible to a stamp (documented limit)
    KT_CHECK(!changed(p, f.c_str(), 5000));
    put(f, "again"); setMtime(f, 1700000005, 0);
    KT_CHECK(changed(p, f.c_str(), 6000));   // new mtime + size
    // force bypasses the throttle
    put(f, "forced!"); setMtime(f, 1700000009, 0);
    KT_CHECK(!changed(p, f.c_str(), 6100));              // 100 ms after the last check: throttled
    KT_CHECK(changed(p, f.c_str(), 6100, true));         // forced
    // present -> missing
    remove(f.c_str());
    KT_CHECK(changed(p, f.c_str(), 9000));
    KT_CHECK_EQ(p.seenMs(), -1LL);
    // unsigned tick wrap: now < last as a plain number, but the elapsed time is huge -> due
    ReloadPoll w; w.throttleMs = 1000;
    changed(w, f.c_str(), 0xFFFFFF00u);
    put(f, "w"); 
    KT_CHECK(!changed(w, f.c_str(), 0xFFFFFF10u + 0));   // 16 ms later
    KT_CHECK(changed(w, f.c_str(), 0x00000400u));        // 0x500 ms later across the wrap (>= 1000)
}

static void test_accept() {
    std::string f = P("acc.ini");
    put(f, "x");
    ReloadPoll p; p.throttleMs = 1000;
    Stamp s = accept(p, f.c_str());                      // prime after an initial load
    KT_CHECK(s.exists);
    KT_CHECK(!changed(p, f.c_str(), 0));                 // primed: first poll is no change
    // our own save: accept the post-save stamp so it is not reloaded
    put(f, "own save, longer"); 
    accept(p, stamp(f.c_str()));
    KT_CHECK(!changed(p, f.c_str(), 5000));
    put(f, "hand edit, even longer");
    KT_CHECK(changed(p, f.c_str(), 7000));
}

static void test_atomic() {
    std::string f = P("w.ini");
    KT_CHECK(atomicWrite(f.c_str(), "first", true));
    KT_CHECK(get(f) == "first");
    KT_CHECK(!exists(f + ".tmp"));
    KT_CHECK(!exists(f + ".bak"));                       // no old file -> no .bak
    KT_CHECK(atomicWrite(f.c_str(), "second", true));
    KT_CHECK(get(f) == "second");
    KT_CHECK(get(f + ".bak") == "first");                // old file saved
    KT_CHECK(atomicWrite(f.c_str(), "third", true));
    KT_CHECK(get(f) == "third");
    KT_CHECK(get(f + ".bak") == "first");                // .bak kept from the FIRST write only
    KT_CHECK(atomicWrite(f.c_str(), "fourth", false));   // makeBakOnce=false never touches .bak
    KT_CHECK(get(f + ".bak") == "first");
    std::string g = P("nobak.ini");
    put(g, "old");
    KT_CHECK(atomicWrite(g.c_str(), "new", false));
    KT_CHECK(get(g) == "new");
    KT_CHECK(!exists(g + ".bak"));
    KT_CHECK(!exists(g + ".tmp"));
    KT_CHECK(atomicWrite(g.c_str(), "", false) && get(g).empty());   // empty text is a valid write
}

static void test_failures() {
    // Rename failure: target is a directory. Old thing intact, no .tmp, no partial .bak.
    for (bool bak : { false, true }) {
        std::string d = P(bak ? "dirA" : "dirB");
        fs::create_directory(d);
        put(d + "/inner", "keep");
        KT_CHECK(!atomicWrite(d.c_str(), "text", bak));
        KT_CHECK(fs::is_directory(d));
        KT_CHECK(get(d + "/inner") == "keep");
        KT_CHECK(!exists(d + ".tmp"));
        KT_CHECK(!exists(d + ".bak"));
    }
    // Unwritable directory (skipped as root: root ignores the mode bits).
    if (geteuid() == 0) { std::printf("note: running as root; unwritable-dir check skipped\n"); return; }
    std::string sub = P("ro");
    fs::create_directory(sub);
    std::string f = sub + "/f.ini";
    put(f, "old");
    chmod(sub.c_str(), 0555);
    KT_CHECK(!atomicWrite(f.c_str(), "new", true));
    chmod(sub.c_str(), 0755);
    KT_CHECK(get(f) == "old");
    KT_CHECK(!exists(f + ".tmp"));
    KT_CHECK(!exists(f + ".bak"));
    // Unwritable file but writable dir: the replace succeeds (rename needs dir rights) -- the old content is replaced atomically.
    // Missing parent directory: fails cleanly.
    KT_CHECK(!atomicWrite(P("nodir/x.ini").c_str(), "t", true));
}

int main() {
    const char* t = std::getenv("TMPDIR");
    std::string tmpl = std::string(t && *t ? t : "/tmp") + "/filewatch_test.XXXXXX";
    char* d = mkdtemp(&tmpl[0]);
    if (!d) { std::perror("mkdtemp"); return 5; }
    g_dir = d;
    test_stamp();
    test_poll();
    test_accept();
    test_atomic();
    test_failures();
    chmod(g_dir.c_str(), 0755);
    std::error_code ec; fs::remove_all(g_dir, ec);
    return kt::Report("filewatch_test");
}
