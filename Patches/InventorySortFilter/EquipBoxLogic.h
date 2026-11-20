// EquipBoxLogic.h - pure decision glue for the equip-screen compare box (Phase 3 host-side prep).
//
// Pure header: no engine calls, no globals with state, no allocation, no UB on hostile input.
// Host-testable with plain g++ (see equip_box_logic_test.cpp).
//
// Glue between existing pure helpers:
//   - EquipPanel.h (resolveItemId, slotMaskToTarget, PanelBoxState onPanelChange/onSelect/onClose)
//   - ItemCompare.h (equip block / compare text)
//   - EquipBoxGeo.h (placement)
//
// Live unknowns (creature source panel+0x68, slot-edit field) are strictly input parameters,
// never assumed or read directly from raw memory.
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../Common/GameAddr.h"

#include "EquipPanel.h"
#include "EquipBoxGeo.h"

#include "../Common/ItemCompare.h"

namespace eqlogic {

// ---------------------------------------------------------------------------
// Hide reason: why a compare box is not shown. None/Ok indicates shown.
// ---------------------------------------------------------------------------
enum class HideReason : uint8_t {
    None = 0,
    Ok = 0,
    Unreadable = 1,
    NoItem = 2,
    NotEquipPanel = 3,
    Closed = 4,
    InvalidSlot = 5,
};
using Why = HideReason;

inline const char* hideReasonStr(HideReason r) {
    switch (r) {
        case HideReason::None:          return "None";
        case HideReason::Unreadable:    return "Unreadable";
        case HideReason::NoItem:        return "NoItem";
        case HideReason::NotEquipPanel: return "NotEquipPanel";
        case HideReason::Closed:        return "Closed";
        case HideReason::InvalidSlot:   return "InvalidSlot";
        default:                        return "?";
    }
}

// ---------------------------------------------------------------------------
// Equipped item ids per slot (pure input, no engine assumption).
// ---------------------------------------------------------------------------
struct EquippedSlots {
    uint32_t rightHand = 0;   // 0x10
    uint32_t leftHand  = 0;   // 0x20
    uint32_t head      = 0;   // 0x1
    uint32_t armor     = 0;   // 0x2
    uint32_t gloves    = 0;   // 0x8
    uint32_t implant   = 0;   // 0x200
    uint32_t belt      = 0;   // 0x400

    bool operator==(const EquippedSlots& o) const {
        return rightHand == o.rightHand &&
               leftHand  == o.leftHand &&
               head      == o.head &&
               armor     == o.armor &&
               gloves    == o.gloves &&
               implant   == o.implant &&
               belt      == o.belt;
    }
    bool operator!=(const EquippedSlots& o) const { return !(*this == o); }
};

inline uint32_t getEquippedId(const EquippedSlots& eq, eqpanel::SlotTarget target) {
    switch (target) {
        case eqpanel::SlotTarget::RightHand: return eq.rightHand;
        case eqpanel::SlotTarget::LeftHand:  return eq.leftHand;
        case eqpanel::SlotTarget::Head:      return eq.head;
        case eqpanel::SlotTarget::Armor:     return eq.armor;
        case eqpanel::SlotTarget::Gloves:    return eq.gloves;
        case eqpanel::SlotTarget::Implant:   return eq.implant;
        case eqpanel::SlotTarget::Belt:      return eq.belt;
        default: return 0;
    }
}

// ---------------------------------------------------------------------------
// Select input: all live values required to produce an equip compare decision.
// ---------------------------------------------------------------------------
struct SelectInput {
    uint32_t panelVtable = 0;           // panel vtable (e.g. gameaddr::EquipPanelVtable)
    uint32_t selectedRawId = 0;         // selected row item id (e.g. with bit 31)
    uint32_t slotMask = 0;              // edited slot mask (panel+0x5098)
    bool creatureReadable = false;      // creature-readable flag (panel+0x68 valid)
    EquippedSlots equipped{};           // currently equipped items
    bool isTwoHander = false;           // viewed item is a two-handed weapon
    bool isLeftOnly = false;            // viewed item is off-hand / left-only
    uint32_t itemSlotMask = 0;          // optional viewed item mask for auto-detection
    uintptr_t creature = 0;             // creature source pointer/id for companion switch tracking
};

// ---------------------------------------------------------------------------
// Decision struct: complete outcome of equip compare evaluation.
// ---------------------------------------------------------------------------
struct Decision {
    bool show = false;
    HideReason hideReason = HideReason::None;
    eqpanel::SlotTarget targetSlot = eqpanel::SlotTarget::None;
    uint32_t viewedItemId = 0;

    // Equipped item id(s) to compare:
    // For single slot / left-only: compareItemIds[0] = target item, count = 1.
    // For two-hander: compareItemIds[0] = right hand, compareItemIds[1] = left hand, count = 2.
    uint32_t compareItemIds[2] = {0, 0};
    unsigned compareItemCount = 0;

    // Explicit helper fields
    uint32_t equippedCompareId = 0;           // primary equipped item id
    uint32_t equippedCompareIdSecondary = 0;  // secondary equipped item id (e.g. left hand for two-hander)
    bool comparesBothHands = false;           // true when comparing two-hander against both hands
    bool comparesLeftOnly = false;            // true when comparing left-only against left hand

    // Deduplication & re-decide flags
    bool deduped = false;                     // true if identical repeat select
    bool changed = true;                      // true if new/modified select requiring redraw
};

// ---------------------------------------------------------------------------
// Stateful context for tracking box state across panel change, select, and close.
// ---------------------------------------------------------------------------
struct State {
    eqpanel::PanelBoxState panelState = eqpanel::initialBoxState();
    uintptr_t lastCreature = 0;
    uint32_t lastRawId = 0;
    uint32_t lastSlotMask = 0;
    uint32_t lastVtable = 0;
    EquippedSlots lastEquipped{};
    bool lastTwoHander = false;
    bool lastLeftOnly = false;
    bool hasLast = false;
    Decision lastDecision{};
};

inline State initialState() {
    return State{};
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------

inline void onPanelChange(State& s, eqpanel::Panel toPanel) {
    s.panelState = eqpanel::onPanelChange(s.panelState, toPanel);
    s.hasLast = false;
    s.lastDecision = Decision{};
    if (toPanel != eqpanel::Panel::Equip) {
        s.lastDecision.show = false;
        s.lastDecision.hideReason = (toPanel == eqpanel::Panel::None) ? HideReason::Closed : HideReason::NotEquipPanel;
    }
}

inline void onPanelChange(State& s, uint32_t vtable) {
    const eqpanel::PanelDesc* desc = eqpanel::descFor(vtable);
    eqpanel::Panel p = desc ? desc->panel : eqpanel::Panel::None;
    onPanelChange(s, p);
    s.lastVtable = vtable;
}

inline void onClose(State& s, eqpanel::Panel closedPanel) {
    s.panelState = eqpanel::onClose(s.panelState, closedPanel);
    s.hasLast = false;
    s.lastDecision = Decision{};
    s.lastDecision.show = false;
    s.lastDecision.hideReason = HideReason::Closed;
}

inline void onClose(State& s, uint32_t vtable) {
    const eqpanel::PanelDesc* desc = eqpanel::descFor(vtable);
    eqpanel::Panel p = desc ? desc->panel : eqpanel::Panel::None;
    onClose(s, p);
}

// Evaluates selection and produces decision using state for dedupe & companion tracking.
inline Decision decide(State& s, const SelectInput& in) {
    // 1. Panel identity check
    const eqpanel::PanelDesc* desc = eqpanel::descFor(in.panelVtable);
    if (desc == nullptr || desc->panel != eqpanel::Panel::Equip) {
        Decision d;
        d.show = false;
        d.hideReason = (in.panelVtable == 0 || s.panelState.current == eqpanel::Panel::None)
                           ? HideReason::Closed
                           : HideReason::NotEquipPanel;
        s.panelState = eqpanel::onSelect(s.panelState, false);
        s.hasLast = false;
        s.lastDecision = d;
        return d;
    }

    // Auto-sync panel state to Equip if open
    if (s.panelState.current != eqpanel::Panel::Equip) {
        s.panelState = eqpanel::onPanelChange(s.panelState, eqpanel::Panel::Equip);
    }

    // 2. Creature readability check
    if (!in.creatureReadable) {
        Decision d;
        d.show = false;
        d.hideReason = HideReason::Unreadable;
        s.panelState = eqpanel::onSelect(s.panelState, false);
        s.hasLast = false;
        s.lastDecision = d;
        return d;
    }

    // 3. Selected item validity check (0, 0x80000000, 0x7F000000 -> NoItem)
    eqpanel::ResolvedItem item = eqpanel::resolveItemId(in.selectedRawId, desc->maskRowBit31);
    if (!item.valid) {
        Decision d;
        d.show = false;
        d.hideReason = HideReason::NoItem;
        s.panelState = eqpanel::onSelect(s.panelState, false);
        s.hasLast = false;
        s.lastDecision = d;
        return d;
    }

    // 4. Slot target resolution
    eqpanel::SlotTarget target = eqpanel::slotMaskToTarget(in.slotMask);
    if (target == eqpanel::SlotTarget::None) {
        // Weapon set 2 normalisation fallbacks if encountered
        if (in.slotMask == 0x40000u) {
            target = eqpanel::SlotTarget::RightHand;
        } else if (in.slotMask == 0x80000u) {
            target = eqpanel::SlotTarget::LeftHand;
        } else {
            Decision d;
            d.show = false;
            d.hideReason = HideReason::InvalidSlot;
            s.panelState = eqpanel::onSelect(s.panelState, false);
            s.hasLast = false;
            s.lastDecision = d;
            return d;
        }
    }

    // 5. Two-hander vs left-only detection
    // Slot mask bit convention from EquipPanel.h (u03: 0x10 = right hand, 0x20 = left hand)
    // and ItemCompare.h target logic (:351-356, :383):
    //   - One-handed items that can equip in either hand have both bits set: (mask & 0x30) == 0x30.
    //   - Two-handers equip only in the primary right-hand slot: (mask & 0x30) == 0x10 (right-hand bit only).
    //   - Left-only items (shields / off-hand) equip only in left hand: (mask & 0x30) == 0x20.
    bool twoHand = in.isTwoHander;
    bool leftOnly = in.isLeftOnly;
    if (in.itemSlotMask != 0) {
        if ((in.itemSlotMask & 0x30u) == 0x20u) leftOnly = true;
        if ((in.itemSlotMask & 0x30u) == 0x10u) twoHand = true;
    }

    // 6. Deduplication and companion switch check
    bool creatureChanged = (s.lastCreature != in.creature);
    bool sameSelect = s.hasLast &&
                      !creatureChanged &&
                      s.lastRawId == in.selectedRawId &&
                      s.lastSlotMask == in.slotMask &&
                      s.lastVtable == in.panelVtable &&
                      s.lastEquipped == in.equipped &&
                      s.lastTwoHander == twoHand &&
                      s.lastLeftOnly == leftOnly;

    if (sameSelect) {
        Decision d = s.lastDecision;
        d.deduped = true;
        d.changed = false;
        return d;
    }

    // 7. Produce full decision
    Decision d;
    d.show = true;
    d.hideReason = HideReason::None;
    d.targetSlot = target;
    d.viewedItemId = item.id;
    d.deduped = false;
    d.changed = true;

    if (twoHand) {
        d.comparesBothHands = true;
        d.comparesLeftOnly = false;
        d.compareItemIds[0] = in.equipped.rightHand;
        d.compareItemIds[1] = in.equipped.leftHand;
        d.compareItemCount = 2;
        d.equippedCompareId = in.equipped.rightHand;
        d.equippedCompareIdSecondary = in.equipped.leftHand;
    } else if (leftOnly || target == eqpanel::SlotTarget::LeftHand) {
        d.comparesBothHands = false;
        d.comparesLeftOnly = true;
        d.compareItemIds[0] = in.equipped.leftHand;
        d.compareItemIds[1] = 0;
        d.compareItemCount = 1;
        d.equippedCompareId = in.equipped.leftHand;
        d.equippedCompareIdSecondary = 0;
    } else {
        d.comparesBothHands = false;
        d.comparesLeftOnly = false;
        d.compareItemIds[0] = getEquippedId(in.equipped, target);
        d.compareItemIds[1] = 0;
        d.compareItemCount = 1;
        d.equippedCompareId = d.compareItemIds[0];
        d.equippedCompareIdSecondary = 0;
    }

    // Update state
    s.panelState = eqpanel::onSelect(s.panelState, true);
    s.hasLast = true;
    s.lastCreature = in.creature;
    s.lastRawId = in.selectedRawId;
    s.lastSlotMask = in.slotMask;
    s.lastVtable = in.panelVtable;
    s.lastEquipped = in.equipped;
    s.lastTwoHander = twoHand;
    s.lastLeftOnly = leftOnly;
    s.lastDecision = d;
    return d;
}

// Stateful overload taking direct arguments
inline Decision decide(State& s,
                       uint32_t panelVtable,
                       uint32_t selectedRawId,
                       uint32_t slotMask,
                       bool creatureReadable,
                       const EquippedSlots& equipped,
                       bool isTwoHander = false,
                       bool isLeftOnly = false,
                       uintptr_t creature = 0) {
    SelectInput in;
    in.panelVtable = panelVtable;
    in.selectedRawId = selectedRawId;
    in.slotMask = slotMask;
    in.creatureReadable = creatureReadable;
    in.equipped = equipped;
    in.isTwoHander = isTwoHander;
    in.isLeftOnly = isLeftOnly;
    in.creature = creature;
    return decide(s, in);
}

// Stateless pure overload
inline Decision decide(uint32_t panelVtable,
                       uint32_t selectedRawId,
                       uint32_t slotMask,
                       bool creatureReadable,
                       const EquippedSlots& equipped,
                       bool isTwoHander = false,
                       bool isLeftOnly = false,
                       uintptr_t creature = 0) {
    State dummy;
    dummy.panelState.current = eqpanel::Panel::Equip;
    return decide(dummy, panelVtable, selectedRawId, slotMask, creatureReadable, equipped, isTwoHander, isLeftOnly, creature);
}

// ---------------------------------------------------------------------------
// Glue helpers with ItemCompare.h and EquipBoxGeo.h
// ---------------------------------------------------------------------------

// Translates decision + equipped slots into ItemCompare input.
inline void fillCompareInput(const Decision& d, const EquippedSlots& eq, itemcmp::CompareInput* in) {
    if (!d.show || in == nullptr) return;
    in->viewed.ok = true;
    in->viewed.id = d.viewedItemId;

    switch (d.targetSlot) {
        case eqpanel::SlotTarget::RightHand:
            in->right.state = (eq.rightHand != 0) ? itemcmp::SlotState::Item : itemcmp::SlotState::Empty;
            in->right.item.id = eq.rightHand;
            if (d.comparesBothHands) {
                in->left.state = (eq.leftHand != 0) ? itemcmp::SlotState::Item : itemcmp::SlotState::Empty;
                in->left.item.id = eq.leftHand;
            }
            break;
        case eqpanel::SlotTarget::LeftHand:
            in->left.state = (eq.leftHand != 0) ? itemcmp::SlotState::Item : itemcmp::SlotState::Empty;
            in->left.item.id = eq.leftHand;
            break;
        case eqpanel::SlotTarget::Head:
            in->head.state = (eq.head != 0) ? itemcmp::SlotState::Item : itemcmp::SlotState::Empty;
            in->head.item.id = eq.head;
            break;
        case eqpanel::SlotTarget::Armor:
            in->armor.state = (eq.armor != 0) ? itemcmp::SlotState::Item : itemcmp::SlotState::Empty;
            in->armor.item.id = eq.armor;
            break;
        case eqpanel::SlotTarget::Gloves:
            in->gauntlet.state = (eq.gloves != 0) ? itemcmp::SlotState::Item : itemcmp::SlotState::Empty;
            in->gauntlet.item.id = eq.gloves;
            break;
        case eqpanel::SlotTarget::Implant:
            in->implant.state = (eq.implant != 0) ? itemcmp::SlotState::Item : itemcmp::SlotState::Empty;
            in->implant.item.id = eq.implant;
            break;
        case eqpanel::SlotTarget::Belt:
            in->belt.state = (eq.belt != 0) ? itemcmp::SlotState::Item : itemcmp::SlotState::Empty;
            in->belt.item.id = eq.belt;
            break;
        default:
            break;
    }
}

// Sizing and placement using EquipBoxGeo.h
inline bool choosePlacement(const Decision& d, const eqgeo::Rect& lb, const eqgeo::Rect& bar1,
                            int sh, int rowCount, eqgeo::Out* out) {
    if (!d.show || out == nullptr) {
        if (out) *out = eqgeo::Out{0, 0, 0, 0, 0, eqgeo::kModeNone, false};
        return false;
    }
    eqgeo::In in{lb, bar1, sh, rowCount};
    *out = eqgeo::compute(in);
    return out->ok;
}

// Which weapon set a slot mask edits: 1 = set 2 (0x40000/0x80000), else 0.
inline unsigned weaponSet(uint32_t sm) { return (sm == 0x40000u || sm == 0x80000u) ? 1u : 0u; }
// Hand mask the compare uses for the viewed item: set-2 hands are compared against the same hands.
inline uint32_t viewedHandMask(uint32_t sm) { return sm == 0x40000u ? 0x10u : sm == 0x80000u ? 0x20u : sm; }
// Snapshot retake: slot i keeps the old snapshot pointer when the live slot holds the item the player last previewed
// (the creature is wearing that preview, not a real equip); every other slot takes the live pointer. changed[i] = out[i] != snap[i].
inline unsigned snapRetake(const uintptr_t live[7], const uintptr_t snap[7], uintptr_t lastViewed, uintptr_t out[7]) {
    unsigned changed = 0;
    for (int i = 0; i < 7; ++i) {
        out[i] = (lastViewed && live[i] == lastViewed) ? snap[i] : live[i];
        if (out[i] != snap[i]) changed |= 1u << i;
    }
    return changed;   // bitmask of slots whose snapshot facts must be re-read
}

}  // namespace eqlogic

namespace eqbox = eqlogic;
