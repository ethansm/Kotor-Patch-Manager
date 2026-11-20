// EquipPanel.h - pure helpers for the equip-screen compare box (ISF-EQBOX-3, W3b, variant B).
//
// Pure header: no engine calls, no globals with state, no allocation, no UB on hostile input.
// Host-testable with plain g++ (see EquipPanel_test.cpp). Integration into
// InventorySortFilter.cpp is a separate step and is NOT done here.
//
// Sources: u02 (shown creature = panel+0x68, vtable check), u03 (slot mask at panel+0x5098),
// u04 (single box shared by two panels = hazards R1/R2/R5), u07 (bit-31 row ids, 12 sequences).
#pragma once

#include <stddef.h>
#include <stdint.h>
// Needs gameaddr::InventoryPanelVtable / EquipPanelVtable: include GameAddr.h before this header (the .cpp already does; the host test includes ../_shared_src/GameAddr.h first).

namespace eqpanel {

// ---------------------------------------------------------------------------
// Item id. Equip rows carry bit 31 (u07 F2, e.g. 0x80000849). The masked id is the item.
// 0x80000000 and NoSelection (0x7F000000) mean "no item".
// ---------------------------------------------------------------------------
constexpr uint32_t kBit31 = 0x80000000u;
constexpr uint32_t kNoSelection = 0x7F000000u;

struct ResolvedItem {
    bool valid;
    uint32_t id;  // meaningful only when valid
};

// maskBit31 = true strips bit 31 (equip rows). maskBit31 = false keeps the raw id
// (backpack ids; u04 R5 says they must not change). "No item" in both modes:
//   id == 0, id == NoSelection, or rawId == 0x80000000 (checked on the raw value
//   when not masking). Zero is treated as "no item" as a defensive choice.
inline ResolvedItem resolveItemId(uint32_t rawId, bool maskBit31 = true) {
    const uint32_t id = maskBit31 ? (rawId & ~kBit31) : rawId;
    if (id == 0u || id == kNoSelection || rawId == kBit31) {
        return ResolvedItem{false, 0u};
    }
    return ResolvedItem{true, id};
}

// ---------------------------------------------------------------------------
// Slot mask -> local compare target. Only exact single-bit masks in the table map.
// Multi-bit, zero, and unknown values return None (u03: 0x40000 / 0x80000 two-hander
// normalisation is NOT handled here on purpose; they are None).
// Verified in u03: 0x10 = right hand, 0x20 = left hand. The other names are local labels.
// ---------------------------------------------------------------------------
enum class SlotTarget : uint8_t {
    None = 0,
    Head,
    Armor,
    Gloves,
    RightHand,
    LeftHand,
    Implant,
    Belt,
};

inline SlotTarget slotMaskToTarget(uint32_t mask) {
    switch (mask) {
        case 0x1u:   return SlotTarget::Head;
        case 0x2u:   return SlotTarget::Armor;
        case 0x8u:   return SlotTarget::Gloves;
        case 0x10u:  return SlotTarget::RightHand;
        case 0x20u:  return SlotTarget::LeftHand;
        case 0x200u: return SlotTarget::Implant;
        case 0x400u: return SlotTarget::Belt;
        default:     return SlotTarget::None;
    }
}

// ---------------------------------------------------------------------------
// Panel identity. Backpack = gameaddr::InventoryPanelVtable, equip = gameaddr::EquipPanelVtable (u04).
// Each entry says how the shared box must treat that screen. A vtable with no entry
// gets no box (descFor returns nullptr), which is the safe default.
// ---------------------------------------------------------------------------
enum class Panel : uint8_t {
    None = 0,
    Backpack = 1,
    Equip = 2,
};

struct PanelDesc {
    uint32_t vtable;
    Panel panel;
    bool maskRowBit31;  // true: strip bit 31 from row ids (u04 R5)
    bool allowCredits;  // true: cmpMoveCredits may run (u04 R13, backpack only)
    const char* name;
};

// Table lives inside an inline function so there is one copy across translation units.
inline const PanelDesc* panelTable(size_t& count) {
    static const PanelDesc kTable[] = {
        {static_cast<uint32_t>(gameaddr::InventoryPanelVtable), Panel::Backpack, false, true, "backpack"},
        {static_cast<uint32_t>(gameaddr::EquipPanelVtable), Panel::Equip, true, false, "equip"},
    };
    count = sizeof kTable / sizeof kTable[0];
    return kTable;
}

inline const PanelDesc* descFor(uint32_t vtable) {
    size_t n = 0;
    const PanelDesc* table = panelTable(n);
    for (size_t i = 0; i < n; ++i) {
        if (table[i].vtable == vtable) {
            return &table[i];
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Box state machine. One box shared by two screens (u04). The state tracks which screen
// is current, so the invariant can be checked from the state alone:
//   !shown  ->  key == None
//    shown  ->  current != None && key == current
// So a backpack box can never be drawn while the equip screen is current, and the
// reverse. The machine can hide, never show without a select, and every screen change
// hides the box (safe default: a box is redrawn on the next select).
// Deviation from the brief: `current` is added to {shown, key} because the invariant
// needs it.
// ---------------------------------------------------------------------------
struct PanelBoxState {
    Panel current;  // screen the event stream says is open (None = unknown or closed)
    bool shown;     // a compare box is drawn
    Panel key;      // screen the drawn box belongs to; None when !shown
};

inline PanelBoxState initialBoxState() {
    return PanelBoxState{Panel::None, false, Panel::None};
}

// A screen became current (build, populate, or switch). Always hides the box.
inline PanelBoxState onPanelChange(PanelBoxState s, Panel to) {
    s.current = to;
    s.shown = false;
    s.key = Panel::None;
    return s;
}

// A row was selected on the current screen. drawable = the compare produced rows.
// A select always belongs to the current screen. Without a current screen, or without
// drawable rows, the box is hidden.
inline PanelBoxState onSelect(PanelBoxState s, bool drawable) {
    if (s.current == Panel::None || !drawable) {
        s.shown = false;
        s.key = Panel::None;
        return s;
    }
    s.shown = true;
    s.key = s.current;
    return s;
}

// A screen closed. Hides the box when the closed screen is current or owns the box.
// Closing the current screen also clears `current`.
inline PanelBoxState onClose(PanelBoxState s, Panel closed) {
    if (closed == Panel::None) {
        return s;
    }
    if (closed == s.current || closed == s.key) {
        s.shown = false;
        s.key = Panel::None;
        if (closed == s.current) {
            s.current = Panel::None;
        }
    }
    return s;
}

// True when the state obeys the invariant above.
inline bool boxInvariantOk(PanelBoxState s) {
    if (!s.shown) {
        return s.key == Panel::None;
    }
    return s.current != Panel::None && s.key == s.current;
}

}  // namespace eqpanel
