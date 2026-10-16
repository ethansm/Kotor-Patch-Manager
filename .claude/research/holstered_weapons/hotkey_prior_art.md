# Hotkey prior art (holster toggle recon)

Read-only recon. Anchors verified with rg/sed. Paths are repo-relative.

## 1. Mechanisms
- A. DirectInput engine hook (full key-down/up stream): action-hotbar detour `CaptureHotbarKey` at 0x0072CB84 (CExoInputInternal::GetEvents+0x3A4). Hook site: `patch_manager_mods/action_hotbar_src/kotor2-steam-aspyr.hooks.toml:8-10`. Function: `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1952`. Swallows bound digits by zeroing the record. Device filter: `ActionHotbar.cpp:471`.
- B. Per-frame GetAsyncKeyState polling (no hook):
  - combat-hotbar F9-F11: `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:238` (edge via `ch::KeyEdge`, `:113`); modifiers at `:177-178`; called from K2HUD_TARGET_MENU_PRE at `:811`.
  - companion-selfheal Ctrl+Shift+H/J: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1298,1302`.
  - shield-bar F9 dump: `patch_manager_mods/shield_bar_src/ShieldBar.cpp:490`.
- C. WndProc subclass: only for WM_MOUSEWHEEL, in gui-input (`patch_manager_mods/gui_input_src/GuiInputProvider.cpp`, via `_shared_src/WindowHook.h:12`). No keyboard subclass found. Mouse buttons are polled (`_shared_src/GuiKit.h:691-692`).

## 2. Shared input library
- gui-input, `k2.guiinput@1` (kind=library, v0.1.0-dev). API struct `K2GuiInputApi` at `patch_manager_mods/_shared_src/GuiInput.h:43-54`: `GameHwnd`, `CursorEngine`, `WheelSubscribe`, `WheelUnsubscribe`, `Foreground`. No key or keyboard function.
- Consumers: action-hotbar (wheel, `ActionHotbar.cpp:896,1204-1214`); GuiKit-based mods (`GuiKit.h:614` `g_gkInput`).
- hud-host `k2.hud@1` gives frame/menu callbacks (combat-hotbar `CombatHotbar.cpp:885-887`). Not a key API.
- Foreground helper: `GuiKit.h:682-689` (`winForeground`). companion-selfheal has its own `foreground()` (`CompanionSelfHeal.cpp:94-98`).
- Conclusion: no shared keybind module. Each mod reads its own INI and polls or hooks.

## 3. Key configuration
- combat_hotbar.ini `[Invoke]`: `Enabled=1`, `Mod=3` (bit0 Ctrl, bit1 Shift), `Key0..Key2` as VK codes. Defaults 0x78/0x79/0x7A (F9-F11) at `CombatHotbar.cpp:92,120-128`. Read with GetPrivateProfileIntA. Example: `Key0=120`. Values outside 1..255 fall back to default.
- action_hotbar.ini `[General]`: `KeyboardDevice=<n>` (DirectInput index), `HotbarConfig.h:253,283`. Digits 1-0 are fixed, not configurable (`action_hotbar_src/README.md` Keys table).
- companion_selfheal: `[General] ProbeKey` default 0x48 (H), `[probe] CallKey` default 0x4A (J), both with fixed Ctrl+Shift (`CompanionSelfHeal.cpp:106-112`; `README.md:50-53`).
- shield-bar: F9 hard-coded, no INI.
- holster.ini: no key entries (probe, ch_*, place*). holster_src has no GetAsyncKeyState, VK_ or input hook.

## 4. Avoiding fires in menus or typing
- action-hotbar (strongest gate): `gameplayHudActive` at `ActionHotbar.cpp:335` calls `gateEval` (`:310`). A non-ignored modal on top closes the gate (`:319`). A non-whitelisted panel on top closes it too. Whitelist: Messages, Fade, Pause, AutoPause vtables (`ActionHotbar.cpp:69`; `_shared_src/GameAddr.h:82-85`). Digits also need a fresh HUD tick, `HudFreshMs` 250 (`ActionHotbar.cpp:123`). Edit/Pick mode passes digits through (`HotbarLogic.h:317-318`; `g_editOpen` at `ActionHotbar.cpp:756`).
- combat-hotbar: only `winForeground()` plus Ctrl/Shift match (`CombatHotbar.cpp:238`). No modal or panel check.
- companion-selfheal: `gateOpen` (`CompanionSelfHeal.cpp:301`) needs inMenu==0, HUD gate and foreground. The Ctrl+Shift+H dump checks only foreground (`:1298`).
- No chat, console or text-entry detection in these sources. Typing in KOTOR II is in modal dialogs, so the modal check covers it (inferred, not tested).

## 5. Holster docs
- `patch_manager_mods/42_holstered_weapons.md` and `42_holstered_weapons_master_plan.md`: no hotkey, keybind, toggle or button mention (rg `-i hotkey|toggle|keybind|press|button|shortcut`).

## Notes for a holster toggle
- Reuse the combat-hotbar pattern: GetAsyncKeyState edge, decimal VK in INI, foreground check.
- Copy the action-hotbar modal and panel gate, since no shared gate exists.

## Open
- Combat-hotbar F9-F11 has no modal gate. Does it fire with a modal open? Not verified live.
- GetPrivateProfileIntA hex (`0x78`) handling is unverified. Examples above assume decimal.
