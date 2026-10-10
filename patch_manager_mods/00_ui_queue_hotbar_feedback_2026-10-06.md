# UI & Queue Feedback Tracker (2026-10-06)

Feedback captured from play-testing sessions across Combat Action Hotbar, Companion Self-Heal / Queue, and Non-Combat Queue HUD.

---

## 1. Combat Action Bar (`combat-hotbar`)
- **Color Theme**: Change button/container tint to **Red** (combat theme matching target banner/combat queue, replacing default green `{0.3f, 0.95f, 0.4f}`).
- **Spacing / Padding**: Increase padding between buttons and pill boundary (`pad`, `groupGap`, `sidePad` in `CombatHotbar.cpp` / `combat_hotbar.ini`).

---

## 2. Companion Queue (`queue-hud` & `companion-selfheal`)
- **Icon Sizing**: Restore larger icon dimensions (in 0.4.0-dev it shrank to 24px; revert/scale closer to previous 38px vit-height scaling or make configurable in `queue_hud.ini`).
- **Right-Click Menu Blank (Invisible Text)**:
  - Popover opens and top half click toggles Auto correctly (hit detection works).
  - Text labels (`Auto`, `HpM/P`, `KeepM/P`, values) are blank on screen.
  - Root cause: `LBL_QUEUE1` template text draw suppression or `LBL_MOULDING3` popover background z-order occluding label children.

---

## 3. Non-Combat Queue (`queue-hud`)
- **Combat UX Parity in Dark Teal**:
  - Replicate native combat queue geometry (left chevron pointer bar + rounded slot container + wide "Cancel" bar button underneath).
  - Skin with dark teal theme (`rgb ~0.08, 0.45, 0.45`) replacing vanilla red.
  - Reference screenshots: `Screenshot_2026-10-05_23-59-00.png` & `Screenshot_2026-10-05_23-58-52.png`.
- **Queuing Multiple Items Out of Combat**:
  - Engine behavior: In non-combat exploration, native item execution bypasses `CSWSCombatRound` and runs immediate `PerformUseItem`, which drops or overwrites subsequent clicks while paused.
  - Solution: Either hook out-of-combat `PerformUseItem` to append to `creature+0x100` (`CExoLinkedList` action queue) or maintain an action queue buffer in DLL that dispatches subsequent items upon animation/execution completion.
- **Companion Actions Eaten (Dropped Items)**:
  - Root cause: Follower AI loop (`AIActionFollowLeader` 0x3D) or heartbeat scripts invoke `ClearAllActions` on followers when follow distance/state updates, purging the queued item before execution.
  - Solution: Guard queued item actions from follow-state wipes or re-queue purged item actions until successfully consumed.

---

## 4. Action Hotbar (`action-hotbar`)
- **Main Character "No Character Detected" / Unidentified**:
  - Symptom: Action hotbar works on companions, but when controlling the main character (Exile), editor states `"Character not identified yet - try again"` and slots fail to bind/display.
  - Location: `patch_manager_mods/action_hotbar_src/HotbarLogic.h:73-79` (`characterKeyFor`) and `ActionHotbar.cpp:330-343` (`computeCharacterKey`).
  - Root Cause:
    ```cpp
    inline std::string characterKeyFor(unsigned oid, int partyCount, bool* invalid = nullptr) {
        bool bad = (oid == 0xFFFFFFFFu || oid == 0);
        if (invalid) *invalid = bad;
        if (bad) return (oid == 0xFFFFFFFFu && partyCount == 1) ? "objfffffffd" : "";
        ...
    }
    ```
    The engine frequently leaves the Exile's client creature object ID `+4` as `0xFFFFFFFF` (`-1`). The hotbar had a fallback to `"objfffffffd"` guarded strictly by `partyCount == 1`. When companions are present in the party (`partyCount > 1`), this fallback evaluates to `false` and returns `""`, completely disabling the hotbar for the main character while dynamic companion IDs (`obj8000xxxx`) succeed.
  - Fix: Check if the controlled creature matches party member 0 (`party::member(t, 0)`), or allow the `0xFFFFFFFF` fallback whenever controlled creature is member 0 regardless of `partyCount`.
- **Per-Save Snapshot Empty Folder (`save folder not recognised ('')`)**:
  - Symptom: `WARN persave: save written: save folder not recognised (''), no snapshot` logged 3 times during game saves.
  - Cause: Active savegame directory name probe read returned empty string `""` at the time of save event hook.
  - Fix: Delay or re-read save path when save event fires, or verify save directory global address.

---

## 5. Companion Self-Heal (`companion-selfheal`)
- **Empty Tag Warning on Player Switch**:
  - Symptom: `WARN member=2 has an empty tag, INI key slot1` / `member=1 has an empty tag, INI key slot0`.
  - Cause: When player switches controlled character to a companion, the Exile becomes a follower in slot 1 or 2. The Exile has an empty creature tag `""`, causing spurious warnings and falling back to raw slot index instead of identifying the PC.
  - Fix: Explicitly recognize the Exile (`"player"` / `"exile"`) when member creature matches party leader or has empty tag with PC stats.


