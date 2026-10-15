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
    KT_CHECK_EQ(m.size, 0ull); KT_CHECK_EQ(m.mtime, 0LL); KT_CHECK_EQ(m.mtimeMs(), -1LL);   // a missing file stamps as all zero
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
    { struct stat rs; KT_CHECK(stat(f.c_str(), &rs) == 0);   // independent of the guard below: mtime is exactly seconds * 1e9 PLUS the nanoseconds
      KT_CHECK_EQ(n1.mtime, (long long)rs.st_mtim.tv_sec * 1000000000LL + (long long)rs.st_mtim.tv_nsec);
      KT_CHECK_EQ(n1.mtime / 1000000000LL, 1700000000LL); }
    if (n1.mtime % 1000000000LL != 123456789LL) {
        std::printf("note: filesystem has no ns mtime; same-second/same-size check skipped\n");
    } else {
        put(f, "bbbb"); setMtime(f, 1700000000, 123456790);
        Stamp n2 = stamp(f.c_str());
        KT_CHECK_EQ(n2.size, n1.size);
        KT_CHECK_EQ(n2.mtime / 1000000000LL, n1.mtime / 1000000000LL);   // same second
        KT_CHECK(n2 != n1);                                              // ns differs -> detected
        KT_CHECK_EQ(n1.mtimeMs(), 1700000000123LL);
        KT_CHECK_EQ(n1.mtime, 1700000000LL * 1000000000LL + 123456789LL);
    }
}

static void test_poll() {
    std::string f = P("p.ini");
    ReloadPoll p; p.throttleMs = 1000;
    KT_CHECK_EQ(p.seenMs(), -2LL);              // unprimed
    { ReloadPoll d; std::string g = P("dflt.ini");   // the default throttle is 1000 ms (the hotbar relies on it): 999 ms is still inside, 1000 is due
      KT_CHECK_EQ(d.throttleMs, 1000u);
      KT_CHECK(changed(d, g.c_str(), 5000)); put(g, "x");
      KT_CHECK(!changed(d, g.c_str(), 5999)); KT_CHECK(changed(d, g.c_str(), 6000)); }
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

// changed(..., &out) hands back the stamp it read. out is written on every poll that reaches stat (missing, changed, quiet,
// forced); a throttled poll never stats, so out is left as the caller had it. The sentinel (999 / -1) makes a dropped store visible.
static void test_out() {
    std::string f = P("out.ini");
    ReloadPoll p; p.throttleMs = 1000;
    Stamp out;
    out.size = 999; out.mtime = -1;                       // sentinel: a dropped store leaves it as is
    KT_CHECK(changed(p, f.c_str(), 0, false, &out));      // first call is a change, also for a missing file
    KT_CHECK(out == stamp(f.c_str()));
    KT_CHECK(!out.exists);
    put(f, "one"); setMtime(f, 1700000000, 0);
    out.size = 999; out.mtime = -1;
    KT_CHECK(changed(p, f.c_str(), 2000, false, &out));   // missing -> present
    KT_CHECK(out == stamp(f.c_str()));
    KT_CHECK(out.exists);
    KT_CHECK_EQ(out.size, 3ull);
    out.size = 999; out.mtime = -1;
    KT_CHECK(!changed(p, f.c_str(), 3000, false, &out));  // unchanged: no change, but out still gets the current stamp
    KT_CHECK(out == stamp(f.c_str()));
    put(f, "twotwo"); setMtime(f, 1700000000, 0);
    out.size = 999; out.mtime = -1;
    KT_CHECK(!changed(p, f.c_str(), 3500, false, &out));  // throttled (window began at 3000): no stat, out untouched
    KT_CHECK_EQ(out.size, 999ull);
    KT_CHECK_EQ(out.mtime, -1LL);
    KT_CHECK(changed(p, f.c_str(), 3600, true, &out));    // forced past the throttle: out gets the new stamp
    KT_CHECK(out == stamp(f.c_str()));
    KT_CHECK_EQ(out.size, 6ull);
    remove(f.c_str());
    out.size = 999; out.mtime = -1;
    KT_CHECK(changed(p, f.c_str(), 9000, false, &out));   // present -> missing: out is the zero stamp again
    KT_CHECK(out == stamp(f.c_str()));
    KT_CHECK(!out.exists);
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
    // .bak copy failures: the old file must survive and atomicWrite must report false.
    if (geteuid() != 0) {
        std::string u = P("unreadable.ini");   // exists (stat works) but cannot be opened for the .bak copy
        put(u, "old"); chmod(u.c_str(), 0000);
        KT_CHECK(!atomicWrite(u.c_str(), "new", true));
        chmod(u.c_str(), 0644);
        KT_CHECK(get(u) == "old"); KT_CHECK(!exists(u + ".tmp")); KT_CHECK(!exists(u + ".bak"));
    }
    {   std::string d = P("dangling.ini");   // .bak is a dangling symlink: stat says "absent", fopen("wb") cannot create it
        put(d, "old");
        KT_CHECK(symlink("/nonexistent_dir_xyz/bak", (d + ".bak").c_str()) == 0);
        KT_CHECK(!atomicWrite(d.c_str(), "new", true));
        KT_CHECK(get(d) == "old"); KT_CHECK(!exists(d + ".tmp"));
        unlink((d + ".bak").c_str()); }
    {   std::string big = P("big.ini");   // the .bak copy keeps the final partial chunk (4096 * k + 1 bytes)
        for (size_t n : { (size_t)4097, (size_t)1, (size_t)8193 }) {
            remove(big.c_str()); remove((big + ".bak").c_str());
            std::string body(n, 'a'); body[n - 1] = 'Z'; put(big, body);
            KT_CHECK(atomicWrite(big.c_str(), "new", true));
            KT_CHECK(get(big + ".bak") == body); KT_CHECK(get(big) == "new"); } }
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
    test_out();
    test_atomic();
    test_failures();
    chmod(g_dir.c_str(), 0755);
    std::error_code ec; fs::remove_all(g_dir, ec);
    return kt::Report("filewatch_test");
}
