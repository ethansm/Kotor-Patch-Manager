# R8: D3b call sequence (holster-editor 3D view with holstered weapons)

Read-only recon, 2026-10-16. Only this file was written. Paths relative to `patch_manager_mods/` unless shown.

## OUTPUT

Token = `g_p3.p->CreateView(P, "HSE_VIEW")`, already made in `OptBindView` (holster_editor_src/HolsterEditor.cpp:265).

Setup, once, right after CreateView:
1. `g_p3.p->SetBeforeFree(cbUnregWeapons, &g_ed)`. Callback `(token, user)`: for hand in {0 RIGHT, 1 LEFT}: `g = g_p3.p->WeaponGob(token, hand)`; if g, `g_hol.p->UnregisterPreview(g)`. It fires before any weapon Gob is freed, while WeaponGob still returns them.
2. `g_p3.p->SetWeapons(token, 1)`. Only records the flag when nothing is built yet.

(a) Bind party member N:
3. `uintptr_t t = party::table(&why)`; `n = party::partyCount(t)`; require `0 <= N < n`.
4. `void* c = (void*)party::member(t, N)`. Index 0 is the controlled leader. This is the client creature that Bind expects.
5. `g_p3.p->Bind(token, c)`. Bind only stores the pointer. Validation (vtable check) happens in Rebuild.
No body or appearance argument is needed. buildPreview reads the appearance from the bound creature.

(b) Build and weapons:
6. `g_p3.p->Rebuild(token)`. buildPreview detaches weapons (BeforeFree fires), rebuilds the creature, then re-attaches weapons if weaponsOn.
7. Optional: `g_p3.p->SetWeapons(token, 1)` again. The API comment asks for it; the code already re-attaches, so it is harmless.
- SetWeapons attaches the equipped hand items only. Right = creature+0x238, left = creature+0x23C. Each id goes to `getItemObj`, then the ResRef at item+0x128 (16 chars). Empty or `i_null` is skipped. `FnCreateModel(name)` makes the Gob, which is attached to the body at node "rhand" or "lhand".
- There is no drawn or holstered flag in the provider. The holstered look comes only from holster re-posing the registered hand Gobs (step 9 to 10).

(c) Holster registration:
8. Wait one frame after Rebuild, then for hand in {0,1}: `g = g_p3.p->WeaponGob(token, hand)`; if g, `g_hol.p->RegisterPreview(g, NULL, hand == K2PREVIEW3D_HAND_LEFT)`.
- RegisterPreview takes no mname or body. Holster derives the model name from gob+0x84 to +0x08. The body name comes from the parent, which is filled in later from beh+0x14.
9. Per frame, holster H7 `HolsterPlace(gob, beh)` runs `previewPlace`. That sets parent and node from beh, computes the mount pose and applies the tune. It needs `g_editor` (default on) and at least one registered entry.
10. Only after step 9 has run at least once: `g_hol.p->BeginEdit(g, scope)`. Before that, parent is NULL and the body name is empty.
11. End: `UnregisterPreview(g)` also ends the edit on that gob. `UnregisterPreview(NULL)` clears all.

(d) Camera and framing:
12. `g_p3.p->SetView(token, yaw, zoom, panZ)`. Yaw is in degrees; facing = (cos, sin, 0). fov = min(22.726 / zoom, 150). panZ is the creature Z offset in metres. Defaults are yaw 0, zoom 1, panZ 0.
- Back close-up candidate: yaw = front + 180, zoom about 2 to 3, panZ = camZ minus hipZ. camZ comes from `g_hol.p->GetNodeWorld(g_p3.p->RigGob(token, PART_BODY), "camerahook", pos, q)`. hipZ comes from `GetMountWorld` during an open edit. Which Gob owns camerahook is unclear (see OPEN).

(e) Party picker:
13. Size and list: `party::partyCount(party::table())`, then `party::member(t, i)` for each i.
14. Names: none in Party.h. d2d_party_preview.md:19 suggests `serverCreature` then 0x5632F0, but it is unverified. Use a fallback label such as "Member i".
15. Switch to N: clear registrations (step 1 callback, or `UnregisterPreview(NULL)`), then steps 4 to 6, then step 8. Re-read the count on every switch.
16. Panel dtor: `OptDestroy` calls `Destroy(token)` (HolsterEditor.cpp:333). That fires BeforeFree first, then frees. Bind survives Destroy.

(f) Gaps the cards must fill: see FINDING and VERDICT.

## EVIDENCE

- Preview3dApi.h:18-26 (destroy and rebuild order, Gob lifetime), 60-63 (BeforeFree), 71 (Bind), 74 (Rebuild), 76 (SetWeapons), 78 (WeaponGob), 81 (SetView), 85 (RigGob, body = camerahook), 88 (Destroy), 90 (SetBeforeFree), 93 (ViewControl).
- preview_3d_src/Preview3dProvider.cpp: constants 60 (CrRight/Left +0x238/+0x23C), 61 (+0x128 ResRef), 69 (kDefaultFov), 70 (kRigModel "charrec_light"); realValid 369; notifyBeforeFree 176; detachWeapons 235; itemModelName 253; attachWeapons 264-297; refreshWeapons 298; destroyCreature 330-341; applyView 384; buildPreview 401-443 (weapons re-attached if weaponsOn); apiBind 536; apiRebuild 547-569; apiSetWeapons 571; apiWeaponGob 584; apiSetView 592; Destroy 625-632.
- _shared_src/Party.h: table 33; partyCount 44; member 80 (PartyGetAt, index 0 = controlled); serverCreature 89.
- _shared_src/HolsterApi.h: RegisterPreview 51; UnregisterPreview 52; BeginEdit 53; GetMountWorld 59; GetNodeWorld 60.
- holster_src/Holster.cpp: g_editor 51; gobModelName 430-434; previewPlace 609 (parent/node update 615-624); HolsterPlace 672-673; RegisterPreview 792-811 (beh=0 accepted); UnregisterPreview 813; BeginEdit 835 (parent from registry); kWorldKnowsHand 586; effLeft 587-591; GetNodeWorld 927.
- holster_src/HolsterPreview.h: previewRegister 44; previewSweep 71 (no caller in holster_src).
- preview_3d_src/Preview3dLogic.h: yawVec 76-79; fovForZoom 73; defaults 106.
- holster_editor_src/HolsterEditor.cpp: g_p3 57, g_hol 58; connect 258; CreateView 265; ViewControl 266; OptDestroy 328-335 (Destroy at 333); the file has no Bind, Rebuild, SetWeapons, SetView, SetBeforeFree, RegisterPreview or BeginEdit (rg confirmed).
- .claude/research/holster_edit/d2d_party_preview.md:12-14, 19 (rg only, for party terms; the camera and SetView rg was also run and gave lines 32 and 71). No u0*.md exists.

## FINDING

- The editor view is created but never bound. Only CreateView, ViewControl and Destroy are called. g_hol is connected and tested, but never used for register or edit.
- Left-hand holster edits are not supported. kWorldKnowsHand is false, so effLeft forces every preview Gob to the right-hand tune (Holster.cpp:586-591).
- RegisterPreview with beh=NULL works. The parent, node and body name are filled one frame later (Holster.cpp:615-624). BeginEdit too early gives an empty body name.
- The provider has no drawn or holstered state. Only equipped hand items are shown.
- Stale-gob cleanup depends entirely on BeforeFree. previewSweep is never called.
- OptDestroy does not unregister anything. The BeforeFree callback in step 1 has to cover that.

## OPEN

1. Which yaw faces the camera (front)? Needs a live SetView yaw log or REA. Not derivable from source.
2. Does camerahook live on the rig model (CreateView attaches the camera there, Provider:509) or on the body Gob (Preview3dApi.h:85)? This sets the GetNodeWorld target.
3. Does H7 HolsterPlace fire for provider-attached Gobs in the editor? MEMORY.md says HE0 LIVE 819e4462; not re-checked here.
4. Name accessor 0x5632F0: unverified.
5. Hip height and panZ values: no measurements in source.

## VERDICT

GAPS

- G1: OptBindView needs steps 1, 2, 4 to 6, 8 and 10.
- G2: OptTick needs a per-frame SetView with framing constants (OPEN 1, 2, 5).
- G3: Party picker and fallback names (step 13 to 15, OPEN 4).
- G4: Explicit UnregisterPreview(NULL) in OptDestroy before Destroy.
- G5: Decide on left-hand tune (kWorldKnowsHand) before claiming left-hand support.
