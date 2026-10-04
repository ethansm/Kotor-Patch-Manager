# 28 — Point-and-Click Movement Mod Proposal & Investigation Spec

Status: **RESEARCH & PROPOSAL SPEC (2026-10-04, static analysis verified, nothing installed).**  
Origin: Cleaned, fact-checked, and expanded from Gemini Webb mod research for Claude planning handoff.  
Legend: **[V]** verified in Ghidra / local knowledge DB; **[H]** historical / repo documentation; **[S]** speculative / requires runtime probe.

---

## 1. Executive Summary & Core Discovery

Gemini Webb's core architectural intuition is **validated**:
KotOR 2's Odyssey Engine retains the Aurora Engine's complete A* walkmesh pathfinding system (`CSWSCreature::AddMoveToPointAction`). You do **not** need to write a custom pathfinder or calculate 3D camera projection matrices.

### Crucial Corrections to Gemini Webb's Assumptions:
1. **Class Hierarchy [V]:** KotOR 2 uses Star Wars prefixes (`CSWS*` for server, `CSWC*` for client), not Neverwinter Nights' `CNWS*` (`CSWSCreature`, `CSWSMessage`, `CSWCMessage`).
2. **Camera Projection Math is Unnecessary [V]:** You do not need to hijack `D3DVIEWPORT8` or perform manual raycasting. The engine's per-frame input loop ([`ProcessInput`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research/dump_excerpts/007ac470.c#L1-L15) at `0x007AC470`) *already* raycasts the mouse cursor against the walkmesh every frame and caches the exact 3D world coordinates (`X`, `Y`, `Z`) at `CClientExoAppInternal + 0x4B4..0x4BC`.
3. **Pre-Existing Native Packet [V]:** The client-server protocol already defines a dedicated "walk to point" packet: Input Message 6, Subtype 1 ([`CSWSMessage::HandlePlayerToServerInputWalkToWaypoint`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/engine_research_findings/T2_map_swsmsg.tsv#L154)).

---

## 2. Verified Architectural Breakdown

```
[Mouse Click on Ground]
        │
        ▼
CClientExoAppInternal::HandleMouseClickInWorld (0x007B3CA0) [V]
        │
        ├── If hoveredId != 0x7F000000: Target interactable object (vanilla)
        └── If hoveredId == 0x7F000000: Ground clicked!
                │
                ├── Read 3D Target Vector from [this + 0x4B4..0x4BC] [V]
                ├── (Optional) Validate Walkmesh: CServerExoApp::TestSafeLocation (0x0051CD90) [V]
                └── Dispatch Movement via Route A (direct) or Route B (packet)
                        │
                        ▼
                CSWSCreature::AddMoveToPointAction (0x00564770) [V]
                        │
                        ▼
                Native A* Walkmesh Pathfinding Engaged
```

### 2.1 Screen-to-World Vector Capture [V]
- Every frame, [`CClientExoAppInternal::ProcessInput`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research/dump_excerpts/007ac470.c#L1-L50) (`0x007AC470` Steam / `0x00459080` GOG) fetches the module camera via [`CClientExoApp::GetModule`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B11a/functions.csv#L27) (`0x0073F430`), inverts screen Y (`screenHeight - 1 - mouseY`), and calls camera vtable `+0x30` to generate ray origin and ray direction vectors.
- It tests collision against the scene and calls [`CClientExoAppInternal::HandleMouseOver`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research/dump_excerpts/007b3a00.c#L1-L20) (`0x007B3A00` Steam / `0x00460630` GOG) with:
  - `param_1`: `CClientExoAppInternal*` (`this`)
  - `param_2`: `hoveredObjectId` (e.g. creature/placeable ID, or `0x7F000000` = `OBJECT_INVALID`)
  - `param_3`: pointer to `Vector3` intersection point (`X`, `Y`, `Z`)
- [`HandleMouseOver`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research/dump_excerpts/007b3a00.c#L14-L17) stores these directly into `this`:
  - `*(float*)(this + 0x4B4) = X`
  - `*(float*)(this + 0x4B8) = Y`
  - `*(float*)(this + 0x4BC) = Z`
  - `*(uint32_t*)(this + 0x4B0) = hoveredObjectId`

### 2.2 Click Interception Point [V]
- The primary world click handler is [`CClientExoAppInternal::HandleMouseClickInWorld`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B11a/functions.csv#L407) (`0x007B3CA0` Steam / `0x004608D0` GOG).
- Vanilla behavior:
  - Reads `this + 0x4B0`. If an object is clicked, calls [`SetLastTarget`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B11a/cards/chunk_5.md#L3425) (`0x007A0530`) and updates the HUD action menu (`0x0074CFE0`).
  - If `this + 0x4B0 == 0x7F000000` (ground hit), it drops the event or deselects.
- **Hook Site:** Injecting a detour at `0x007B3CA0` or wrapping it enables capturing ground clicks with zero matrix math.

### 2.3 Walkmesh Validation [V]
- Before executing a move action to avoid out-of-bounds clicks (e.g. background geometry/skyboxes):
  - Function: [`CServerExoApp::TestSafeLocation`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B9/check_validated.tsv#L1571) (`0x0051CD90` Steam / `0x0064C7B0` GOG).
  - Alternative: [`CServerExoApp::ComputeSafeLocation`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B9/check_validated.tsv#L1571) (`0x0051CDB0` Steam / `0x0064C7D0` GOG).

### 2.4 Action Queue Injection Routes [V]
Three viable implementation routes exist:

| Route | Function & Address | Calling Conv | Description |
|---|---|---|---|
| **Route A (Server Direct)** | [`CSWSCreature::AddMoveToPointAction`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B12a/final_rows.tsv#L88) (`0x00564770` Steam / `0x0067D3A0` GOG) | `__thiscall`, `RET 0x44` (68 bytes) | Injects native A* move action directly into creature's queue. Takes target Vector3, run flag, range tolerance. |
| **Route B (Client-to-Server Msg)** | Input Msg 6, Subtype 1 (`0x0065B4F0` Steam / `0x00801290` GOG) | Server handler: `CSWSMessage::HandlePlayerToServerInputWalkToWaypoint` | Standard engine network protocol. Dispatched via [`CSWCMessage::SendPlayerToServerInput_WalkToWayPoint`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/engine_research_findings/T2_map_cswcmsg.tsv#L93). |
| **Route C (NWScript Command Thunk)** | [`ExecuteCommandActionMoveToLocation`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B15/final_rows.tsv#L378) (`0x0068CEE0` Steam / `0x007A2530` GOG) | `__thiscall`, `RET 8` | The engine's NWScript opcode handler for Action 21 (`ActionMoveToLocation`) & Action 382 (`ActionForceMoveToLocation`). Pops engine location and invokes `0x00564770`. |

### 2.5 Input Conflict & State Cancellation (WASD Override) [V]
- Vanilla direct translation uses:
  - [`CSWSMessage::HandlePlayerToServerInputDriveControl`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/engine_research_findings/T2_map_swsmsg.tsv#L151) (`0x0065B270` Steam / `0x00801010` GOG).
  - [`CSWSMessage::HandlePlayerToServerInputAbortDriveControl`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/engine_research_findings/T2_map_swsmsg.tsv#L152) (`0x0065B3D0` Steam / `0x00801170` GOG).
- Clearing actions:
  - [`CSWSObject::ClearAllActions`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B0/final_rows.tsv#L10) (`0x00541080` Steam / `0x006A7D60` GOG).
  - Note: Creature `commandable` flag at `creature + 0xEC` must equal 1 for `ClearAllActions` to execute.
  - When WASD input is detected, calling `ClearAllActions` instantly cancels any queued or executing `AddMoveToPointAction`.

---

## 3. Temporary Specification Sheet

### 3.1 Input Modes & UX Triggers (Configurable)
To avoid interfering with camera rotation (RMB-drag) or target selection (LMB on objects), the trigger mode must be configurable in `swkotor2.ini`:

1. **Mode 0: Double-Click LMB on Ground** (Default recommendation)
   - Uses OS-level `WM_LBUTTONDBLCLK` (`0x203`) mapped in [`CExoInputInternal::OnMouseMessage`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/18_mouse_camera_investigation.md#L178) (`0x0072E030`).
   - Single-click retains vanilla behavior (clearing/setting target or interacting).
2. **Mode 1: Modifier Key + Click** (e.g. `Shift + LMB` or `Alt + LMB`)
   - Checks keyboard state (`GetAsyncKeyState(VK_SHIFT) & 0x8000`).
   - Prevents accidental moves during combat or exploration.
3. **Mode 2: Direct Single Click (CRPG / Diablo Style)**
   - Triggers whenever `hoveredObjectId == 0x7F000000` and no GUI element consumed the click.
   - Requires click-deadzone or movement-threshold check to avoid triggering during tiny mouse adjustments.

### 3.2 Movement Behavior Specs
- **Run vs Walk:**
  - Double-click or Shift+Click = Run (`bRun = 1`).
  - Single-click or Ctrl+Click = Walk (`bRun = 0`).
- **Visual Feedback:**
  - Spawn ground destination decal / pulse VFX (using native VFX engine `CSWCModule::AttachVFXModel` or floaty indicator).
- **Party Movement:**
  - Companions automatically follow via their native leader follow routine (`CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader` at `0x006D1100`).

---

## 4. Prior Art & Competitive Landscape

| Existing Mod / Tool | Mechanism | Limitation vs Click-to-Move |
|---|---|---|
| **Modern Camera Overhauls** | FOV, pitch, and camera offset adjustments | Modifies view only; movement is still strictly WASD. |
| **Movement Animation Fix** | `appearance.2da` edits | Visual mesh animation tweaks; does not touch input or routing. |
| **Vanilla RMB + W Steering** | Mouse Look mode (`swkotor2.ini`) | Steers forward direction with mouse; still requires holding keys/buttons. |
| **KPM Post-Combat Freeze Fix** | DLL hook at movement boundary | Fixes high-framerate WASD stall; does not add point-and-click routing. |

**Verdict:** Point-and-click movement has never been successfully released for KotOR 1 or 2 due to previous lack of native DLL injection frameworks. KPM provides the exact capability required.

---

## 5. Claude Handoff & Investigation Action Plan

### Phase 0: Read-Only Telemetry Probe (Estimated: 20 min)
- Hook [`CClientExoAppInternal::HandleMouseClickInWorld`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B11a/functions.csv#L407) (`0x007B3CA0`).
- Log `hoveredObjectId` (`this + 0x4B0`) and cached Vector3 coordinates (`this + 0x4B4..0x4BC`) on click.
- Verify ground clicks consistently yield `hoveredObjectId == 0x7F000000` and valid walkmesh coordinates.

### Phase 1: Injection Harness via KPM (Estimated: 30 min)
- Create `patch_manager_mods/point_and_click_movement/` with a standard `.kpatch` configuration.
- Implement Route A test hook invoking [`CSWSCreature::AddMoveToPointAction`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_batches/B12a/final_rows.tsv#L88) (`0x00564770`) for controlled character.

### Phase 2: Route Comparison & WASD Override (Estimated: 30 min)
- Compare Route A direct call vs Route B packet `0x0065B4F0`.
- Verify WASD input triggers `ClearAllActions` (`0x00541080`) cleanly without animation stutter.

### Phase 3: Edge Case Verification (Estimated: 25 min)
- Test clicking through UI dialogs/hotbars (ensure GUI consumes clicks before world handler).
- Test clicking off map / into skyboxes (confirm `TestSafeLocation` cleanly suppresses invalid paths).

---

## 6. Claude review (2026-10-04): claims checked against the DB and the steam_6a522e71 dump

**Premise is wrong: vanilla already has double-click-to-move.** Doc 18 (§1, line 8 and line 193) records the user's own report that "double-click-to-move does not work like originally." WndProc `0x40A750` turns WM_LBUTTONDBLCLK (CS_DBLCLKS is set, `0x4087A0`) into an LMB-axis event with value **-1**, which feeds `HandleInputEvent 0x7B12C0` / ProcessInput. So §3.1 Mode 0 duplicates a vanilla feature, and the §4 claim "never released" does not apply to double-click. The new parts are Mode 1 (modifier+click) and Mode 2 (single-click), plus repairing the broken vanilla double-click.

| Claim | Result |
|---|---|
| `0x007B3CA0` HandleMouseClickInWorld, `0x007B3A00` HandleMouseOver, `0x007AC470` ProcessInput | [V] B11a verified |
| `this+0x4B0` = hovered id, `+0x4B4..0x4BC` = hit vector | [V] dump `007b3a00.c` lines 14-17 |
| Ground click in `0x007B3CA0` does nothing | [V] roughly: with `+0x4B0 == 0x7F000000` it skips SetLastTarget `0x7A0530`; it may still refresh the HUD `0x74CFE0(1)`. `__fastcall`, one param, ret 0 (dump `007b3ca0.c`) |
| `0x0051CD90` TestSafeLocation, `0x00564770` AddMoveToPointAction (17 params, RET 0x44), `0x00541080` ClearAllActions | [V] B9 / B12a / B0 |
| Route B `0x0065B4F0` | [S] only a `vote` row in T2_map_swsmsg.tsv (RET 4), not in the DB. Dump copied to `dump_excerpts/0065b4f0.c` |
| `CSWCMessage::SendPlayerToServerInput_WalkToWayPoint` link (T2_map_cswcmsg L93) | **Wrong**: L93 is AbortDriveControl. The sender has no Steam mapping (Linux `0x082571E8` only) |
| "Hit vector is a walkmesh point" | [S] it is the scene-raycast hit. It may land on walls or props. Phase 0 must check it |

**Gaps in the plan:**
1. Route A is a server call: the client hovered id/PC id must go through `ClientToServerObjectId` (GOG `0x0064C2D0` / `0x007BCF80`; Steam address not mapped yet) before calling a CSWSCreature method.
2. `AddMoveToPointAction`'s 17-parameter layout is unknown. Phase 1 needs the vanilla caller (ExecuteCommandActionMoveToLocation `0x0068CEE0`) as the argument template.

**Revised Phase 0 (replaces §5 Phase 0):** trace the vanilla double-click path first. That means LMB -1 → `0x7B12C0` → whichever call issues the walk (probably the Route B sender). Then add the read-only probe on `0x007B3CA0`. If vanilla double-click already reaches the WalkToWaypoint message, Modes 1 and 2 only need to send that same message from the click handler. That also explains the doc 18 regression (Wine not synthesizing 0x203).
