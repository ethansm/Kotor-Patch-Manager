# SH-T11
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3b-01 | PARTIAL | guikit_test.cpp:24-80; GuiKit.h:215,247,256,263,269,276 | yes | 1-80 call none of the nine; 81+ unread; setFill/setAlpha/appendToPanel/playHoverSound defs outside range |

- Anchor is exact at guikit_test.cpp:1, but it is the file header comment; main starts at line 24.
- guikit_test.cpp:36-80 calls only imageFor and createLabelFromTemplate; no setter or getter call in 1-80.
- guikit_test.cpp:32 sets only alloc, labelCtor, initControl, exoCtor, exoDtor, loadImage; textSet, setFont, setAlign, textColor, callSetRect are never set.
- GuiKit.h:215-223 setRect returns false unless g_gk.callSetRect and the vtable slot resolve.
- GuiKit.h:247-255 setText returns false unless g_gk.textSet and textObjOk(ctl) hold.
- GuiKit.h:256-282 setFont, setAlign, setTextColor each gate on their hook plus textObjOk; getTextColor (269-275) gates on textObjOk.
- GuiKit.h:205-285 has no setFill, setAlpha, appendToPanel, or playHoverSound definitions; border section starts at 284.
- guikit_test.cpp:1-2 header scopes the file to imageFor and createLabelFromTemplate; full parity test is elsewhere.
- Fix direction (add success-path tests) is sound but needs fake hooks for the five setters first.
- Not read: guikit_test.cpp:81 onward (outside brief ranges); whether it calls any of the nine is OPEN.
