// Pure decisions for load-hang-probe (no windows.h, host-testable).
#pragma once
#include <stdint.h>

namespace probe {

// The watchdog may pause the main thread (SuspendThread + GetThreadContext) only while the DR0 breakpoint is NOT armed on it.
// 11-14 Telos freeze: the main thread sat in the DR0 exception path (thousands of traps per second under PROTON_LOG=1) while the
// watchdog's SuspendThread waited on it; both blocked in ntsync_schedule and the probe went silent.
inline bool shouldSample(bool armed, bool drArmed) { return armed && !drArmed; }

enum class FixGate { Act, NotReady, NotForeground, Iconic, Cooldown, Exhausted };

constexpr int kFixMaxTries = 5;
constexpr uint32_t kFixRetryMs = 2000;

// When the probe may call the engine's own AppActivated (main thread, frame boundary, load screen up with a1b4e0==0).
// fgOurs = the OS foreground window belongs to this process (not only the game window: after a minimize the foreground can be
// another of our windows). An iconic game window is reported, never restored by the probe.
inline FixGate fixGate(bool areaReady, bool fgOurs, bool iconic, int tries, uint32_t lastTryMs, uint32_t nowMs) {
    if (tries >= kFixMaxTries) return FixGate::Exhausted;
    if (!areaReady) return FixGate::NotReady;
    if (!fgOurs) return FixGate::NotForeground;
    if (iconic) return FixGate::Iconic;
    if (lastTryMs && nowMs - lastTryMs < kFixRetryMs) return FixGate::Cooldown;
    return FixGate::Act;
}

// hangprobe_debug.txt: fix=auto is the default; "fix=off" disables it.
inline bool parseFixOff(const char* line) { return line[0] == 'f' && line[1] == 'i' && line[2] == 'x' && line[3] == '=' && line[4] == 'o' && line[5] == 'f' && line[6] == 'f'; }

}  // namespace probe
