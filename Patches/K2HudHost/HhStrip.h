// HhStrip.h - pure math for the hud-host portrait-strip slot API (K2HudApi::StripSlot, doc 24 CS10).
// Slots run right-to-left to the left of a companion vit bar: order 0 is right-most. Each order holds the last width registered for it
// (a width includes the owner's own gap); unregistered orders count 0. Std only, no engine includes.
#pragma once
#include <stdint.h>

namespace hh {

struct Box { int x, y, w, h; };

struct Strip {
    static constexpr int kOrders = 8;
    int32_t width[kOrders] = {};

    static bool validOrder(int order) { return order >= 0 && order < kOrders; }
    // Records width for order (negative -> 0). Returns true when the stored value changed (callers log on change only).
    bool set(int order, int w) {
        if (!validOrder(order)) return false;
        if (w < 0) w = 0;
        const bool changed = width[order] != w;
        width[order] = w;
        return changed;
    }
    // Sum of the widths registered for orders below `order`.
    int sumBefore(int order) const {
        int s = 0;
        for (int i = 0; i < order && i < kOrders; ++i) s += width[i];
        return s;
    }
    // Register + place. out = {vit.x - sumBefore(order) - width, vit.y + (vit.h - height) / 2, width, height}. 0 = refused (bad order, null out).
    // The width is recorded first, so an order's own width never shifts its own x beyond the "- width" term. *changed = stored width changed.
    int slot(int order, const Box& vit, int w, int height, Box* out, bool* changed = nullptr) {
        if (!validOrder(order) || !out) return 0;
        if (w < 0) w = 0;
        const bool ch = set(order, w);
        if (changed) *changed = ch;
        *out = Box{ vit.x - sumBefore(order) - w, vit.y + (vit.h - height) / 2, w, height };
        return 1;
    }
};

} // namespace hh
