// MouseFixLogic.h -- Pure logic for mouse delta sanitization, edge warping, and double click synthesis.
// Part of the Mouse Camera Fix (project design doc 18).
// Host-testable without Win32 or engine dependencies.
#pragma once
#include <stdint.h>
#include <math.h>

namespace mousefix {

struct FixConfig {
    bool enabled = true;
    bool sanitizeNorm = true;
    bool synthDblClk = true;
    bool edgeWarp = true;
    float sensScale = 0.05f; // Maps ~20 px/frame to 1.0 maximum angular velocity
    int edgeMargin = 50;     // Pixels from window boundary triggering recentering warp
};

// Computes sanitized normalized delta [nx, ny] in [-1.0, 1.0] from cursor coordinates.
// Neutralizes spurious desktop coordinates when not dragging (kills camera pitch lock).
inline void computeSanitizedDelta(
    bool dragging,
    bool wasDragging,
    int curX, int curY,
    int lastX, int lastY,
    float sensScale,
    float& outNx, float& outNy)
{
    if (!dragging || !wasDragging) {
        outNx = 0.0f;
        outNy = 0.0f;
        return;
    }
    float dx = static_cast<float>(curX - lastX);
    float dy = static_cast<float>(curY - lastY);
    float nx = -dx * sensScale;
    float ny = -dy * sensScale;
    if (nx > 1.0f) nx = 1.0f; else if (nx < -1.0f) nx = -1.0f;
    if (ny > 1.0f) ny = 1.0f; else if (ny < -1.0f) ny = -1.0f;
    outNx = nx;
    outNy = ny;
}

// Persistent state for the sanitize path (last drag cursor position, whether the previous frame was a drag).
struct SanitizeState { bool wasDragging = false; int lastX = 0, lastY = 0; };
enum class SanitizeResult { Skip, Write };

// One ProbeMouseNorm frame. cursorOk=false (GetCursorPos unresolved or failed): Skip, state untouched, nothing is written,
// so the engine keeps its own normalised delta and no (0,0) point reaches the delta math or the edge-warp check.
inline SanitizeResult sanitizeFrame(SanitizeState& st, bool dragging, bool cursorOk, int curX, int curY,
                                    float sensScale, float& outNx, float& outNy) {
    if (!cursorOk) return SanitizeResult::Skip;
    computeSanitizedDelta(dragging, st.wasDragging, curX, curY, st.lastX, st.lastY, sensScale, outNx, outNy);
    if (dragging) { st.wasDragging = true; st.lastX = curX; st.lastY = curY; } else { st.wasDragging = false; }
    return SanitizeResult::Write;
}

// Checks if the cursor is near the client boundary and calculates the window center warp point.
inline bool checkEdgeWarp(
    int curX, int curY,
    int winLeft, int winTop, int winRight, int winBottom,
    int margin,
    int& outWarpX, int& outWarpY)
{
    if (winRight <= winLeft || winBottom <= winTop) return false;
    // Clamp the band to a quarter of the smaller side so the centre and an interior region
    // stay outside it (no warp on every frame), and do the math in 64-bit.
    long long w = static_cast<long long>(winRight) - winLeft;
    long long h = static_cast<long long>(winBottom) - winTop;
    long long maxMargin = (w < h ? w : h) / 4;
    long long m = margin < 0 ? 0 : margin;
    if (m > maxMargin) m = maxMargin;
    if (static_cast<long long>(curX) <= winLeft + m || static_cast<long long>(curX) >= winRight - m ||
        static_cast<long long>(curY) <= winTop + m || static_cast<long long>(curY) >= winBottom - m)
    {
        outWarpX = (winLeft + winRight) / 2;
        outWarpY = (winTop + winBottom) / 2;
        return true;
    }
    return false;
}

// Decision helper for double-click synthesis when Wine drops WM_LBUTTONDBLCLK under XWayland.
inline bool shouldSynthesizeDblClick(int msg, bool isMissedDbl, bool configEnabled) {
    return configEnabled && msg == 0x201 && isMissedDbl;
}

} // namespace mousefix
