# r11b consumer port recon: holster-editor -> mod-config (read-only)

Source: patch_manager_mods/holster_editor_src/HolsterEditor.cpp (716 lines). Line anchors below are current.
Drift: none. The 388-396 slice starts at the `if (dOk) {` opener on 388 (389-396 are the body).

## 1. KEEP slices -> callback

| Slice | Current fn | Goes to | Notes |
|---|---|---|---|
| 582-586 CreateView + ViewControl | OptBindView, inside embedgui Open..Close | build(panel,user) | Must stay inside the Open window: Preview3dApi.h:15 says CreateView is valid only while the layout is open. Log line 586 ("expect -1: not listed") stays true until AddCustom lists it. |
| 594 Destroy token on bad restore | OptBindView | build() | Failure path. build() returns void, so mod-config cannot learn of the failure. Consumer must skip NewPage/Add* itself (OPEN 2). |
| 603 status clone | OptBindView after Close | build(), after Close | Becomes AddLabel(page, text); keep the returned item id. cloneLabel/cloneEntry (163-196) and their calls at 600-603 are vanilla-clone code; ModConfigApi.h:10 says mod-config clones its own controls before build(). |
| 388-396 view + status rects | firstTick (321-415, first call from OptTick:629) | tick(page,user) on first call | View/status rect come from PaneRect(panel, MUI_PANE_VIEW / MUI_PANE_STATUS) then SetRect(page,item,&r) (section 4). barY[] (349-355) and quitBottom (391) no longer read. Keep the panel pointer from build(). |
| 400-403 strings | firstTick | build() for static text; 403 in tick/enter | 400-401 (button, Back) and 402 (title) become AddButton/AddLabel text args. 403 depends on g_p3.ok() && g_hol.ok(), known only after connect: SetText in tick or enter. |
| 630-631 g_p3.poll, g_hol.poll | OptTick | KPatchInit (connect) | tick() runs only while the page is shown (ModConfigApi.h:66), so there is no per-frame home for poll (OPEN 3). |
| 680-681 view Destroy | OptDestroy | destroy(panel,user) | `if (e.token && g_p3.ok()) g_p3.p->Destroy(e.token);` Runs before mod-config frees its controls (ModConfigApi.h:68). |
| 682-698 FREEVIEW | OptDestroy | destroy(panel,user) | Gated on `clean` from releaseOwn(P) at 672, which is NOT in the KEEP list. releaseOwn drops refs and truncates P's list (529-557); under mod-config that list belongs to mod-config. Do not run releaseOwn in the port. FREEVIEW then needs its own proof, or stays off (free_view=0 default). OPEN 4. |
| 707-708 client init | KPatchInit (703-713) | KPatchInit | g_p3.init(api), g_hol.init(api). Add ConfigClient/MenuClient init(api) here. |

Non-KEEP mapping, for completeness:
- enter -> enterEditor (416-440); leave -> leaveEditor (441-468).
- Click/hover: OptTick 634-658 -> Clicked(page,item) in tick (K2MenuUiApi.Clicked, ModConfigApi.h:92); hoverTick (469) stays in tick.
- Connect at 575-576 (OptBindView) -> KPatchInit.
- enabled gate: the per-hook `if (!g_enabled ...) return` (559, 608, 663) -> skip Register in KPatchInit when enabled=0.

## 2. What the EmbedGui Open -> CreateView -> Close sequence needs from P (OptBindView 558-604)

- Preconditions (561-574): `vt` = *(P+0) must equal kOptionsVtable, else refuse (572). flags48 = word at ctlFlagsOff() must have bit 0x2 (layout open), else refuse. `slot` = P+0x30 (embedgui::PanelLayoutSlot, EmbedGui.h:42) must be nonzero. `count` = Panel_CtlCount (567), used for logging and the drift check.
- Open (579): embedgui::Open(P, kHseViewGui, sizeof). Success: P+0x30 = new obj; Handle{obj, saved = old P+0x30} (EmbedGui.h:90-127). Failure paths (new, Malloc, parse): null Handle and P+0x30 untouched. Parse failure frees the obj; Malloc failure leaks it (logged).
- Window (581-588), only if h.obj && p3: CreateView(P, "HSE_VIEW") -> token (582); ViewControl(token) -> g_ed.view (583). Both run while P+0x30 = our obj.
- Close (589): embedgui::Close(P, h) clears h.obj, then sets P+0x30 = saved FIRST, then release and dtor (EmbedGui.h:128-137). A null handle is a no-op.
- Restore check (590-597): re-reads P+0x30. If != slot: Destroy(token) when p3 ok (594), reset g_ed, return. After a failed Open the check still passes (P+0x30 untouched), so the failure means "no view", not "no layout".
- NewPage/AddButton/AddLabel/AddCustom are valid only after Close restored P+0x30 (ModConfigApi.h:10).
- Saved values: Handle.saved (the only saved pointer). Editor keeps panel=P, vanillaCount=count, token (uint32), view (ctl pointer). The view pointer already survives Close (FREEVIEW uses it), so this is existing behaviour.

Port order inside build(panel,user):
1. Preconditions. Refuse = return before NewPage.
2. Open -> CreateView -> ViewControl.
3. Close -> restore check. On failure: Destroy(token), return (OPEN 2).
4. page = NewPage(panel, "Holstered Weapons") (ModConfigApi.h:85); AddButton/AddLabel (86-89) for title/status/back as needed; AddCustom(page, view, MUI_NOFREE) (91).

## 3. ViewControl(token) -> AddCustom(page, ctl, MUI_NOFREE)

ModConfigApi.h:
```
#define MUI_NOFREE 1   /* the control is listed and shown/hidden with its page, and is never freed */   (line 37)
/* Adds an engine control `ctl` to the page; flags MUI_NOFREE. build() only. Item id >= 0; < 0 = fail. */  (line 90)
int32_t  (KPATCH_CALL *AddCustom)(uint32_t page, void* ctl, int32_t flags);   (line 91)
```
Preview3dApi.h:91-93:
```
/* The CSWGui3DSceneView control CreateView bound (the GUI control the consumer lists with appendToPanel when the layout control had ID -1, and resizes with its rect slot), or 0 for an unknown token. */
void*    (KPATCH_CALL *ViewControl)(uint32_t token);   (line 93)
```
Mapping:
- ctl = g_p3.p->ViewControl(token) (void*, currently cast to uintptr_t at 583). Skip AddCustom when token or ctl is 0.
- AddCustom(page, ctl, MUI_NOFREE) returns the item id. Use it for SetRect and Control().
- This replaces the appendToPanel loop in firstTick (325-330). Do not also call guikit::appendToPanel on the view (double listing). Only the view moves to AddCustom; btn/back/title/status become AddButton/AddLabel items.
- Lifetime: Preview3dApi.h:9 says a token stays valid until the owning panel is destroyed.
- MUI_NOFREE means mod-config never frees the view. Destroy(token) and any FREEVIEW stay with the consumer's destroy callback.

## 4. hse:: helpers and hse_layout_test

Callers (rg, excluding .md):
- hse::viewRect (HseLayout.h:95): HolsterEditor.cpp:389 only.
- hse::statusFloor (HseLayout.h:109): HolsterEditor.cpp:392 only.
- menulayout::statusStrip (MenuLayout.h:57): HolsterEditor.cpp:393. Also _shared_src/menu_layout_test.cpp (shared, stays).
- hse::modalLayout (HseLayout.h:32) and hse::legendText (HseLayout.h:122): no callers outside hse_layout_test.cpp. Already unused by HolsterEditor.cpp, unchanged by the port.

Once PaneRect(MUI_PANE_VIEW) and PaneRect(MUI_PANE_STATUS) supply the rects directly, viewRect and statusFloor have no callers, and the holster-editor has no caller for statusStrip. This holds only if the provider returns the pane rect itself.

hse_layout_test.cpp (158 lines). main (151-158) runs testRectHelpers (60), testModalLayout (68), testViewRect (112), testStatusFloor (127), testLegend (147).
- Dropped if viewRect/statusFloor are deleted: testViewRect (112-125), testStatusFloor (127-145), and their calls in main. testStatusFloor also checks statusStrip; menu_layout_test covers that directly.
- Kept: testRectHelpers, testModalLayout, testLegend (they test symbols that stay).
- Caveat: mod_config_src has NO pane geometry. MenuUiLogic.h (353 lines) has ItemKind BUTTON/LABEL/CUSTOM (262) and no PaneRect. Its only entry points are McBind/McTick/McDestroy (mod_config_src/exports.def). PaneRect VIEW/STATUS values are undefined today, so do not delete viewRect/statusFloor until the provider defines them.

## 5. Manifest / exports / hooks / DllMain, and CombatHotbar precedent

holster_editor_src/manifest.toml:
- id "holster-editor", version "0.1.1-probe", kind "patch", conflicts [].
- requires ["holster","preview-3d"]; requires_interfaces ["k2.holster@1","k2.preview3d@1"].
- Port: add "mod-config" to requires and "k2.modconfig@1","k2.menuui@1" to requires_interfaces. Holster and preview-3d stay (g_hol and g_p3 are still used).
- mod_config_src/manifest.toml:8-10: provides ["k2.modconfig@1","k2.menuui@1"], kind "provider", requires [].
- Version: the memory rule is minor = big/new, so this port likely takes a minor bump. Owner's call.

exports.def: `LIBRARY HolsterEditor` / EXPORTS KPatchInit, OptBindView, OptTick, OptDestroy. Port: KPatchInit only. Callbacks are passed as pointers in K2ModConfigEntry (ModConfigApi.h:54-70). Keep KPatchInit written literally (comment at HolsterEditor.cpp:702).

hooks.toml: three detours. 0x008A16FE OptBindView (original_bytes 6 bytes, [ebp-0xac] param). 0x0040EC90 OptTick (CSWGuiPanel::Draw). 0x008A1930 OptDestroy. Port: all three move to mod-config's McBind/McTick/McDestroy. The holster file becomes metadata-only. Shape to copy, from preview_3d_src/kotor2-steam-aspyr.hooks.toml:
```
# preview-3d: DLL-only provider (no engine hooks; k2.preview3d@1 viewers are created by the clients that require it).
[metadata]
target_versions = ["6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"]

hooks = []
```
The KPM rule behind it is in patch_manager_mods/40_npc_behaviour_variety.md:341: a hookless patch loads as DLL_ONLY and still runs KPatchInit.

DllMain: HolsterEditor.cpp:716 `BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }`, no I/O (715). Unchanged.

KPatchInit (703-713): readConfig (704); KpLog_Init (705); g_api (706); g_p3.init (707); g_hol.init (708); embedgui::g_log (709); banner (710-712). It only inits. connect is lazy: OptBindView (575) and poll in OptTick (630-631).

combat_hotbar_src/CombatHotbar.cpp:878-889 (no drift). Hookless consumer connect in KPatchInit:
```
__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_hud.init(api, "combat-hotbar");
    g_hud.onConnect = onConnect;
    ...
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);
    g_hud.connect();
}
```
ConfigClient has the same shape (ModConfigApi.h:183-200). Its retry is poll-driven: "tried on the first poll and then on every 256th call" (ModConfigApi.h:160-161, 202-204).

## 6. File I/O

- The only file read is readConfig(): HolsterEditor.cpp:206-233. fopen("holster_editor.ini","r") at 207 is CWD-relative. Called once from KPatchInit (704).
- Keys (224-230): enabled, debug, button_clone, reflow, desc, free_labels, free_view. Source ini: patch_manager_mods/holster_editor_src/holster_editor.ini.
- No file I/O in OptBindView, OptTick, OptDestroy. rg finds no other fopen/ifstream in holster_editor_src (hse_layout_test only uses std::nanf).
- Stays in KPatchInit: readConfig. It must run before Register so enabled=0 can skip the entry. DllMain must not do I/O (715).
- Config values are cached globals (g_cfg, g_enabled). Nothing config-related has to move into build().
- Engine-memory reads that DO belong in build(): vt/flags48/slot/count (564-567) and the restore check (590-591). The vanilla list reads in firstTick (333-355) are replaced by PaneRect.

## Open items

1. PaneRect(MUI_PANE_VIEW/STATUS) is undefined in mod_config_src (section 4). This blocks deleting viewRect/statusFloor.
2. build() cannot report a failed restore (593-597). Provider needs a way to refuse the entry or leave the page empty.
3. poll (630-631): no per-frame caller once the page is hidden. Connect at KPatchInit must succeed, which depends on load order vs requires.
4. FREEVIEW (682-698) depends on releaseOwn. Freeing a MUI_NOFREE view under mod-config is unassigned.
