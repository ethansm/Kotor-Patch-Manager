# HB-T05
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1a-06 | PARTIAL | patch_manager_mods/action_hotbar_src/HotbarConfig.h:77-78; :74 | no | Mechanism real; merge matches :74 identity rule; defect needs engine item data. |

- HotbarConfig.h:75-80 normalizeItemName: trims, finds first " (" at :77, truncates there at :78, trims. No trailing-suffix check.
- Anchor `t.find(" (")` matches HotbarConfig.h:77 exactly. Anchors :167 and :317 are outside the read ranges and were not checked.
- HotbarConfig.h:74 comment: identity is text before " (" for "Name (3)" and "Name (Type) (3)" entries. The merge is documented intent.
- HotbarConfig.h:87-98 StackCount: separate trailing " (<digits>)" parser using rfind. normalizeItemName does not call it.
- HotbarConfig.h:184: "item" config parse stores normalizeItemName(v) as the ActionRef name; empty result is rejected. Saved names are normalized on load.
- HotbarConfig.h:188: Same(ActionRef, ActionRef) declared; body is outside the read range, so identity comparison is unverified.
- HotbarLogic.h:541-546 tipTitleQty uses rfind(" (") and strips only the last group. "X (Red) (3)" gives title "X (Red)", while normalizeItemName gives "X".
- fix_sound no: an implied count-only fix would change the identity of saved item keys loaded at :184 and contradicts the :74 rule. Unsafe without migration.
- Namespace hb:: is not visible in read range :70-100 and was not checked.
- Whether the engine emits distinct items sharing text before the first " (" is outside the ranges (item data).
