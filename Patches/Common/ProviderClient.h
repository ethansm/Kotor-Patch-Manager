/* ProviderClient.h -- one reusable lazy-connect helper for provider clients, replacing the copy-pasted plumbing in each consumer.
 *
 * Why: every mod that uses an optional provider (k2.modconfig, k2.menuui, k2.preview3d, k2.holster) keeps one client struct per
 * interface, inits each in KPatchInit, polls each from a frame hook, and (for mod-config) registers its entry exactly once after
 * the provider answers. That was copied into holster-editor, enemy-effects-strip and others. This header holds the shared part.
 *
 * Contents (namespace k2pc, header-only, no heap, no STL, no Windows imports):
 *   ClientSet<Cs...>  references to existing clients; init(api) / poll() / allOk() / anyMissing() over all of them.
 *   RegisterOnce      state for the one-shot mod-config registration; registerOnce() / unregister().
 *
 * Duck typing: a client is anything with `void init(const KPatchApi*)`, `void poll()` and `bool ok() const`. That covers
 * k2mc::ConfigClient, k2mc::MenuClient, k2p3::Client and k2holster::Client without changing them. Each client keeps its own retry
 * rule (the first poll() tries, then every 256th call until it connects), so ClientSet adds no counter of its own.
 *
 * ClientSet holds pointers, not copies: the clients must outlive the set. Build it with CTAD or makeSet:
 *     k2mc::ConfigClient cfg; k2mc::MenuClient menu;
 *     auto set = k2pc::makeSet(cfg, menu);      // or  k2pc::ClientSet set(cfg, menu);
 *     set.init(api);      // KPatchInit
 *     set.poll();         // every frame
 *     if (set.allOk()) ...
 *
 * registerOnce policy (decided and pinned by providerclient_test):
 *   - Does nothing and returns false until the config client is ok().
 *   - The first call with an ok() client calls Register exactly once, stores the token, logs once, and returns true.
 *   - A refused registration (token <= 0: table full or bad entry) is FINAL: registered stays true, the token stays <= 0, one
 *     WARN line is logged, and later calls return true without calling Register again. It never retries per frame (a retry would
 *     hammer the provider and flood the log every frame for no gain: a full table does not empty itself).
 *   - Later calls return true and do nothing. Register copies the entry; the caller keeps its own struct and strings (static).
 *   - unregister() calls Unregister(token) for an accepted token, then clears the state so registerOnce may run again.
 */
#ifndef K2_PROVIDERCLIENT_H
#define K2_PROVIDERCLIENT_H

#include "kpatch_api.h"
#include "ModConfigApi.h"
#include <stdint.h>

#ifdef __cplusplus
namespace k2pc {

// ---- ClientSet: variadic fan-out over existing clients (pointers; empty set = trivially ok) ----
template <class... Cs>
struct ClientSet;

template <>
struct ClientSet<> {
    void init(const KPatchApi*) {}
    void poll() {}
    bool allOk() const { return true; }
};

template <class C, class... Rest>
struct ClientSet<C, Rest...> {
    C* head;
    ClientSet<Rest...> tail;

    explicit ClientSet(C& c, Rest&... rest) : head(&c), tail(rest...) {}

    void init(const KPatchApi* api) { head->init(api); tail.init(api); }
    void poll() { head->poll(); tail.poll(); }
    bool allOk() const { return head->ok() && tail.allOk(); }
    bool anyMissing() const { return !allOk(); }
};

template <class... Cs>
ClientSet(Cs&...) -> ClientSet<Cs...>;

template <class... Cs>
inline ClientSet<Cs...> makeSet(Cs&... cs) { return ClientSet<Cs...>(cs...); }

// ---- RegisterOnce: one-shot mod-config registration state ----
struct RegisterOnce {
    bool registered = false;   // Register has been answered (accepted or refused); never called again until unregister()
    int32_t token = 0;         // > 0 accepted; <= 0 refused (only meaningful when registered)

    bool accepted() const { return registered && token > 0; }
};

namespace detail {
// "<prefix><n>" into buf (no snprintf: keeps the header free of CRT formatting imports). buf must hold >= 48 bytes.
inline void msgInt(char* buf, const char* prefix, int32_t n) {
    char* o = buf;
    while (*prefix) *o++ = *prefix++;
    char tmp[12];
    int len = 0;
    uint32_t u = n < 0 ? 0u - static_cast<uint32_t>(n) : static_cast<uint32_t>(n);
    do { tmp[len++] = static_cast<char>('0' + u % 10u); u /= 10u; } while (u);
    if (n < 0) *o++ = '-';
    while (len) *o++ = tmp[--len];
    *o = 0;
}
}  // namespace detail

// Returns true once Register has been answered (accepted or refused), false while the provider is not connected.
// log may be null; level is KPLOG_INFO for an accepted token and KPLOG_WARN for a refusal (each logged once).
template <class Cfg>
inline bool registerOnce(RegisterOnce& st, Cfg& cfgClient, const K2ModConfigEntry& entry, void (*log)(int level, const char* msg)) {
    if (st.registered) return true;
    if (!cfgClient.ok()) return false;
    st.token = cfgClient.p->Register(&entry);
    st.registered = true;
    if (log) {
        char buf[64];
        if (st.token > 0) {
            detail::msgInt(buf, "page REGISTER ok token=", st.token);
            log(KPLOG_INFO, buf);
        } else {
            log(KPLOG_WARN, "page REGISTER refused (mod-config table full or bad entry)");
        }
    }
    return true;
}

// Removes an accepted entry (if the provider is still connected) and clears the state. Safe to call when never registered.
template <class Cfg>
inline void unregister(RegisterOnce& st, Cfg& cfgClient) {
    if (st.registered && st.token > 0 && cfgClient.ok()) cfgClient.p->Unregister(st.token);
    st.registered = false;
    st.token = 0;
}

}  // namespace k2pc
#endif

#endif /* K2_PROVIDERCLIENT_H */
