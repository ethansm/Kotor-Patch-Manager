// ClickLogic.h -- pure click heuristics for Mouse Probe 0.3.0 (no Win32, no engine access, host-testable).
// Used by MouseProbe.cpp's ProbeMouseMsg / ProbeClickInWorld; pinned by common_contract_test.cpp.
#pragma once
#include <stdint.h>
#include <math.h>

namespace clicklogic {

constexpr uint32_t InvalidObj = 0x7F000000u;   // the engine's "no object" id
constexpr int MsgLDown = 0x201, MsgLUp = 0x202, MsgLDbl = 0x203, MsgRDown = 0x204, MsgRUp = 0x205, MsgRDbl = 0x206;
constexpr double MissedDblMs = 500.0;          // two presses closer than this ...
constexpr double MissedDblPx = 4.0;            // ... and this many pixels apart, with no WM_LBUTTONDBLCLK between
constexpr double MoveWindowMs = 3000.0;        // AddMoveToPointAction counts as click-related this soon after an LMB message

inline bool isMouseButtonMsg(int msg) { return msg >= MsgLDown && msg <= MsgRDbl; }
inline bool isLmbMsg(int msg) { return msg >= MsgLDown && msg <= MsgLDbl; }

// WM_* lParam: x = LOWORD, y = HIWORD (the engine has already flipped y). Signed, like GET_X_LPARAM.
inline int unpackX(uint32_t lParam) { return (int)(int16_t)(lParam & 0xFFFFu); }
inline int unpackY(uint32_t lParam) { return (int)(int16_t)((lParam >> 16) & 0xFFFFu); }

// HandleMouseClickInWorld: Branch B (run the default action) iff last != invalid && sel == hov && last == sel.
inline bool isBranchB(uint32_t hov, uint32_t sel, uint32_t last) { return last != InvalidObj && sel == hov && last == sel; }

enum class Verdict { None, MissedDbl, DblGuard };

struct ClickState {
    bool havePress = false, sawDblSincePress = false, haveLmb = false;
    double lastPressMs = 0, lastLmbMs = 0;
    int lastPressX = 0, lastPressY = 0;
    // Outputs of the last step(): time/distance since the PREVIOUS 0x201 (valid when haveDelta).
    bool haveDelta = false;
    double dtMs = 0, dpos = 0;

    Verdict step(int msg, int x, int y, int guard, double tMs) {
        haveDelta = havePress;
        if (havePress) {
            dtMs = tMs - lastPressMs;
            double dx = (double)(x - lastPressX), dy = (double)(y - lastPressY);
            dpos = sqrt(dx * dx + dy * dy);
        } else { dtMs = 0; dpos = 0; }
        if (isLmbMsg(msg)) { haveLmb = true; lastLmbMs = tMs; }
        Verdict v = Verdict::None;
        if (msg == MsgLDown) {
            if (havePress && !sawDblSincePress && dtMs <= MissedDblMs && dpos <= MissedDblPx) v = Verdict::MissedDbl;
            havePress = true; sawDblSincePress = false;
            lastPressMs = tMs; lastPressX = x; lastPressY = y;
        } else if (msg == MsgLDbl) {
            sawDblSincePress = true;
            if (guard != 0) v = Verdict::DblGuard;
        }
        return v;
    }
    // True when the last LMB press/release/dblclk seen is within windowMs of tMs.
    bool recentLmb(double tMs, double windowMs = MoveWindowMs) const { return haveLmb && tMs - lastLmbMs <= windowMs; }
};

} // namespace clicklogic
