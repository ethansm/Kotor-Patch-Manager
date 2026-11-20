// TriggerCore.h -- pure logic of the voice-probe hotkey trigger (doc 41 V-C2, kvoice/V-C2_TRIGGER_DESIGN.md). No engine, no Win32.
// The DLL polls the key on the npc-world tick; on a fresh press it runs ONE named script with the PC as OBJECT_SELF by calling the
// engine's RunScript (never hooking it). Config comes from voice_probe.ini [trigger]; key 0 or an empty script = off (default).
// Env is the engine seam: VoiceProbe.cpp implements it with Party.h + RunScript, trigger_core_test.cpp with fakes.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace vpt {

constexpr uint32_t ObjectInvalid = 0x7F000000u;   // OBJECT_INVALID: never passed to RunScript
constexpr int MaxResref = 16;
constexpr int MaxLine = 200;                      // below the KpLog core's 232-byte cut
constexpr int MinCooldownMs = 250, MaxCooldownMs = 60000, DefaultCooldownMs = 1000;

// NCS resref: 1..16 chars of [A-Za-z0-9_].
inline bool validResref(const char* s) {
    if (!s || !s[0]) return false;
    int n = 0;
    for (; s[n]; ++n) {
        const char c = s[n];
        const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
        if (!ok || n >= MaxResref) return false;
    }
    return true;
}

struct Config {
    int vk = 0;                          // Win32 virtual-key code 1..254
    bool ctrl = true, shift = false;     // required modifiers
    uint32_t cooldownMs = DefaultCooldownMs;
    char script[MaxResref + 1] = "";
    const char* why = "off";             // nullptr = enabled; otherwise a static reason
    bool enabled() const { return why == nullptr; }
};

// Builds the config from the raw ini values. key 0 or an empty script = off; a bad key or resref = off with the reason.
inline Config makeConfig(int vk, int ctrl, int shift, int cooldownMs, const char* script) {
    Config c;
    c.ctrl = ctrl != 0;
    c.shift = shift != 0;
    c.cooldownMs = static_cast<uint32_t>(cooldownMs < MinCooldownMs ? MinCooldownMs : cooldownMs > MaxCooldownMs ? MaxCooldownMs : cooldownMs);
    if (vk == 0 || !script || !script[0]) { c.why = "off"; return c; }
    if (vk < 1 || vk > 254) { c.why = "bad key"; return c; }
    if (!validResref(script)) { c.why = "bad script resref"; return c; }
    c.vk = vk;
    memcpy(c.script, script, strlen(script) + 1);
    c.why = nullptr;
    return c;
}

inline void configLine(const Config& c, char* out, size_t cap) {
    snprintf(out, cap, "TRIGGER cfg state=%s key=%02x ctrl=%d shift=%d cooldown_ms=%lu script=%s", c.enabled() ? "on" : c.why, c.vk,
             c.ctrl ? 1 : 0, c.shift ? 1 : 0, static_cast<unsigned long>(c.cooldownMs), c.script[0] ? c.script : "-");
}

struct KeyEdge {
    bool prev = false;
    bool update(bool down) { const bool fire = down && !prev; prev = down; return fire; }
};

enum class Step { Idle, Fire, Cooldown, NotReady };

// Per-tick input. The DLL reads the trigger key first and leaves the rest false when it is up (cheap tick under Wine).
struct Input {
    bool areaReady = false;   // npc-world AREA_STABLE seen since the last AREA_LOAD
    bool foreground = false, key = false, ctrl = false, shift = false;
};

// One step per tick. A press fires once (edge), only with the game in the foreground and the modifiers held. The cooldown starts only
// when a fire actually called RunScript (markRan), so a press skipped for "no PC" does not block the next one.
struct Trigger {
    KeyEdge edge;
    bool ranOnce = false;
    uint32_t lastRunMs = 0;
    long fires = 0;               // presses that reached fire() (numbering of the TRIGGER lines)
    Step step(const Config& c, const Input& in, uint32_t nowMs) {
        if (!c.enabled()) return Step::Idle;
        const bool down = in.foreground && in.key && (!c.ctrl || in.ctrl) && (!c.shift || in.shift);
        if (!edge.update(down)) return Step::Idle;
        if (!in.areaReady) return Step::NotReady;
        if (ranOnce && nowMs - lastRunMs < c.cooldownMs) return Step::Cooldown;
        ++fires;
        return Step::Fire;
    }
    void markRan(uint32_t nowMs) { ranOnce = true; lastRunMs = nowMs; }
};

struct Env {
    // Object id of the player-controlled creature, or 0 / ObjectInvalid with *why set to a static reason.
    virtual uint32_t pcObjectId(const char** why) = 0;
    // Calls the engine RunScript; *called = false when the VM was not reachable. Returns the engine's return value (1 = ran).
    virtual int runScript(const char* script, uint32_t oid, bool* called) = 0;
    virtual void log(const char* line) = 0;
    virtual ~Env() {}
};

// One fire: resolve the PC, run the script, log exactly one line. True when RunScript was called.
inline bool fire(Env& env, const Config& c, long n, uint32_t nowMs) {
    char line[MaxLine];
    const char* why = "no pc";
    const uint32_t oid = env.pcObjectId(&why);
    if (oid == 0 || oid == ObjectInvalid) {
        snprintf(line, sizeof line, "TRIGGER #%ld t=%lu script=%s skip=%s", n, static_cast<unsigned long>(nowMs), c.script, why ? why : "no pc");
        env.log(line);
        return false;
    }
    bool called = false;
    const int ret = env.runScript(c.script, oid, &called);
    snprintf(line, sizeof line, "TRIGGER #%ld t=%lu script=%s oid=%08lx called=%d ret=%d", n, static_cast<unsigned long>(nowMs), c.script,
             static_cast<unsigned long>(oid), called ? 1 : 0, ret);
    env.log(line);
    return called;
}

}  // namespace vpt
