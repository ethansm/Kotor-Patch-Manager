# SH-T15
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3b-06 | PARTIAL | patch_manager_mods/_shared_src/sceneview_test.cpp:67-68; patch_manager_mods/_shared_src/SceneView.h:47-72 | yes | Overwrite of all 7 call lambdas true; "defaults never tested" true only within read ranges |

- SceneView.h:47-72 - seven call* members (callSceneInit, callSetPos, callSetOri, callGetVec, callGetParent, callCamAttach, callCamFov) default to lambdas casting a uint32 vtable slot to __thiscall.
- SceneView.h:51-54 - callSetPos default body; the reviewer's anchor text is found here, not in sceneview_test.cpp.
- sceneview_test.cpp:64 - installFakes calls g_sv.rebind() first, then assigns fakes.
- sceneview_test.cpp:65-68 - installFakes assigns fakes to every call* member (67: SceneInit, SetPos, SetOri, GetVec, GetParent; 68: CamAttach, CamFov).
- sceneview_test.cpp:116 - testCreateView calls installFakes before createView, so default call lambdas are not reached there.
- sceneview_test.cpp:105 - testRebind constructs a default Engine e and checks it.
- sceneview_test.cpp:106-111 - testRebind asserts only pointer defaults (alloc..getModel); no assertion on any call* member.
- SceneView.h:75-80 - rebind() comment says it resets test fakes; body past line 80 unread, so unknown whether it touches call* members.
- Reviewer cites sceneview_test.cpp:26,30,31 and "function main"; outside read ranges. No main in read ranges; lambdas are Engine member initializers.
- fix_sound=yes caveat: a default-path test needs a 32-bit-addressable fake fn, since the uint32 slot truncates host pointers on x64.
