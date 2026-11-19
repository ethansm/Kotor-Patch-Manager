#pragma once
#include <cstdint>
#include <string>

#include "kpatch_api.h"

// The "kpatch.log" service: the core's shared, cheap, non-blocking log sink for
// patch modules (RFC #180 interface "kpatch.log" v1, see Patches/Common/kpatch_api.h).
//
// Shape: patches call Enabled() (a read of one atomic byte), format a line, and
// Submit() it. Submit stamps time and frame, copies the line into a fixed ring
// under a memcpy-only lock and returns. FlushNow() (driven by a flush thread in
// the patcher) drains the ring, formats the lines and writes them to
// <dir>/<patchId>_log.txt and/or <dir>/kp_log.txt. Nothing on the submit path
// allocates, formats, does I/O or waits on a file.
//
// Configuration is <dir>/kplog.ini, re-read whenever its content changes. With no
// ini everything is off except WARN and ERR, which are always recorded.
//
// Threading: every API call is safe from any thread. The service's state is never
// destroyed (see log_service.cpp), so a late call from a patch thread after
// Shutdown() is a harmless no-op rather than a use-after-free.

namespace KotorPatcher {
namespace LogService {

    struct Options {
        // Whether Init() starts the background flush thread: it flushes every
        // flush_ms (kplog.ini, default 500 ms) and sooner when a WARN/ERR or Mark
        // arrives, and re-reads kplog.ini about once a second. The thread is
        // detached and never joined; Shutdown() stops it. Without it the owner
        // drives FlushNow()/PollConfigNow() itself (the deterministic tests do).
        bool startThread = true;
    };

    // Start a fresh session. `dir` is the directory kplog.ini and the log files live
    // in (the patcher's SelfModuleDir). May be called again after Shutdown(): it then
    // resets the tables, files and counters, so a new session starts clean. Calling it
    // while a session is running is a no-op.
    void Init(const std::string& dir, const Options& opt = Options());

    // The interface table the core publishes as "kpatch.log" v1.
    const KPatchLogApi* Api();

    // Record the build's KOTOR_VERSION_SHA and the registry's provider dump for the
    // session header. Files opened afterwards carry them in their header; files that
    // are already open get a "# session info" block on the next FlushNow().
    void SetSessionInfo(const std::string& versionSha, const std::string& providedDump);

    // Drain the ring into the files now. This is what the flush thread calls too.
    void FlushNow();

    // Re-read kplog.ini if its content changed since the last read and apply it.
    // The flush thread calls this once a second. A missing file reads as empty, so
    // deleting the ini returns everything to the defaults.
    void PollConfigNow();

    // Replace the monotonic clock (microseconds since Init) with `nowUs`, or restore
    // steady_clock with nullptr. Test seam: it makes timestamps and the rate-limit
    // window deterministic.
    void SetClockForTest(uint64_t (*nowUs)());

    // Final flush, close the files, then every call becomes a no-op until the next Init().
    // Never blocks for long: if the flush or ring lock cannot be taken within about
    // 200 ms (another thread died or is stuck holding it) the final flush is skipped
    // with one Platform::Log line. It does not join the flush thread.
    void Shutdown();

    // Test-only: hold / release the ring mutex, to prove Shutdown() stays bounded
    // when a submitter is stuck. Lock and unlock from the same thread, and do not
    // call any other service function from that thread while it is held.
    void LockRingForTest();
    void UnlockRingForTest();

    // The functions behind the KPatchLogApi table (same semantics as the header
    // documents). Exposed so the core and tests can call them without the table.
    int32_t  Register(const char* patchId);
    int32_t  Channel(int32_t patch, const char* name);
    int32_t  Enabled(int32_t channel, int32_t level);
    void     Submit(int32_t channel, int32_t level, const char* line, uint32_t len);
    void     Mark(const char* label);
    void     FrameTick(int32_t patch);
    uint32_t Frame();
    uint64_t NowUs();

} // namespace LogService
} // namespace KotorPatcher
