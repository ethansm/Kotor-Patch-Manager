// GiDispatch.h -- the gui-input wheel dispatcher: subscription list and event dispatch.
// Pure C++ (no Win32 API calls inside), so order, capacity, and consumption rules are host-tested (gi_dispatch_test.cpp).
// Used by GuiInputProvider.cpp.
#pragma once
#include "GuiInput.h"
#include <string.h>

namespace gid {

constexpr int MaxSubscribers = 32;

struct Entry {
    int32_t token;
    K2WheelCallback fn;
    void* user;
    const char* owner;
};

struct Dispatcher {
    Entry entries[MaxSubscribers] = {};
    int count = 0;
    int32_t nextToken = 1;

    // Registers a wheel callback. Returns a token > 0 on success, or:
    // -1: null callback
    // -2: subscriber table full
    int32_t subscribe(K2WheelCallback fn, void* user, const char* owner = "") {
        if (!fn) return -1;
        if (count >= MaxSubscribers) return -2;
        int32_t tok = nextToken++;
        entries[count++] = { tok, fn, user, owner ? owner : "" };
        return tok;
    }

    // Unsubscribes all registrations matching fn.
    // Returns true if at least one entry was removed, false otherwise.
    bool unsubscribe(K2WheelCallback fn) {
        if (!fn) return false;
        bool any = false;
        for (int i = 0; i < count; ) {
            if (entries[i].fn == fn) {
                for (int k = i + 1; k < count; ++k) {
                    entries[k - 1] = entries[k];
                }
                --count;
                any = true;
            } else {
                ++i;
            }
        }
        return any;
    }

    // Unsubscribes by token.
    bool unsubscribeToken(int32_t token) {
        if (token <= 0) return false;
        for (int i = 0; i < count; ++i) {
            if (entries[i].token == token) {
                for (int k = i + 1; k < count; ++k) {
                    entries[k - 1] = entries[k];
                }
                --count;
                return true;
            }
        }
        return false;
    }

    bool empty() const { return count == 0; }

    // Dispatches a wheel event to subscribers in registration order.
    // Iterates over a local snapshot so registrations/unregistrations during callbacks don't break iteration.
    // If any subscriber returns non-zero (consumed), dispatch stops immediately and returns true (consumed).
    // If all subscribers return 0, returns false (not consumed, caller should chain/forward).
    bool dispatch(const K2WheelEvent& ev) const {
        if (count == 0) return false;
        Entry snap[MaxSubscribers];
        const int cnt = count;
        memcpy(snap, entries, sizeof(Entry) * cnt);
        for (int i = 0; i < cnt; ++i) {
            if (snap[i].fn && snap[i].fn(&ev, snap[i].user) != 0) {
                return true; // consumed
            }
        }
        return false;
    }
};

} // namespace gid
