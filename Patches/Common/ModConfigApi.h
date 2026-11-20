/* ModConfigApi.h -- the "k2.modconfig" and "k2.menuui" v1 interfaces (provider: the mod-config patch) + C++ consumer helpers.
 *
 * Why: the Options screen has no place for a mod's settings. mod-config adds one "Mod Configuration" button under Gameplay. That
 * button opens a list page with one button per registered entry (hover shows the entry's description on the right). Each entry owns
 * an entry page that it builds itself. k2.modconfig is the registry a mod calls to add its entry. k2.menuui is the set of helpers an
 * entry uses to lay its page out in the vanilla look (pages, buttons, labels, the right-hand description pane, custom controls).
 *
 * Page model: Options -> "Mod Configuration" button (under Gameplay) -> list page (one button per registered entry, hover shows entry desc on the right) -> entry page.
 *
 * Build scope: NewPage/Add* are valid only inside build(), and only while the vanilla layout is current: a consumer that opens its own embedded layout (EmbedGui) calls them after its Close restored P+0x30. mod-config clones its own controls before it calls any build().
 *
 * Sliders and toggles are not in v1; they are appended later (struct_size growth).
 *
 * Description split (K2MenuUiApi.SetDescSplit, MUI_PANE_PREVIEW): by default the right half of an entry page is the vanilla description
 * pane (LB_DESC, text through SetDesc). An entry page can opt in to a split: SetDescSplit(page, previewPermille) shrinks the description
 * pane to the bottom (1000 - previewPermille)/1000 of its ORIGINAL rect (previewPermille is clamped to 600..850, so the description keeps
 * about 15..40%) and PaneRect(MUI_PANE_PREVIEW) answers the area above it (same x and width, a small gap kept between the two) for the
 * entry's own preview widgets. 0 turns the split off. The split is per entry page and only active while that page is shown; the provider
 * re-asserts it after every description write and on every tick, and puts the original rect back when the page is left, so no other
 * page (the list, other entries) sees it. Without a split MUI_PANE_PREVIEW is the whole original description rect.
 * Compatibility: the table only grows (SetDescSplit was the first growth; rows are slots 12..18, modal / gear slots 19..22: check
 * MenuClient::hasRows() / hasModal()). A consumer built with this header against an older
 * provider must check MenuClient::hasDescSplit(), i.e. struct_size >= offsetof(K2MenuUiApi, SetDescSplit) + sizeof(void*) and a non-null
 * pointer, before calling it. MenuClient::usable() requires only the v1 members that existed before, so older consumers and older
 * providers keep working in both directions. MUI_PANE_PREVIEW on an older provider answers 0 (fail).
 *
 * Threading: calls run on the game's main thread (the Options ctor, draw and dtor), as in K2Preview3dApi.
 *
 * Size guard (one rule, both sides): before reading any field of KPatchApi, check
 *   api->struct_size >= offsetof(KPatchApi, require) + sizeof(api->require)
 * Before using any function of K2ModConfigApi or K2MenuUiApi, the consumer checks struct_size >= sizeof(that struct) and that every
 * function pointer is non-null (ConfigClient::usable, MenuClient::usable).
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints, floats and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 */
#ifndef K2_MODCONFIGAPI_H
#define K2_MODCONFIGAPI_H

#include "kpatch_api.h"
#include <stddef.h>
#include <stdint.h>

#define K2MODCONFIG_IFACE   "k2.modconfig"
#define K2MODCONFIG_VERSION 1
#define K2MENUUI_IFACE      "k2.menuui"
#define K2MENUUI_VERSION    1

/* K2MenuUiApi.AddCustom flags. */
#define MUI_NOFREE 1   /* the control is listed and shown/hidden with its page, and is never freed */

/* K2MenuUiApi.PaneRect panes. */
#define MUI_PANE_COLUMN 0
#define MUI_PANE_DESC   1
#define MUI_PANE_TITLE  2
#define MUI_PANE_STATUS 3
#define MUI_PANE_BACK   4
#define MUI_PANE_VIEW   5
#define MUI_PANE_PREVIEW 6   /* top part of the description rect above an active SetDescSplit; the whole original rect without one */

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

/* One mod's entry in the Mod Configuration list. Register copies the struct; the strings must be static. */
typedef struct K2ModConfigEntry {
    uint32_t struct_size;
    const char* id;       /* static */
    const char* name;     /* static: the list button text */
    const char* desc;     /* static: the hover text on the list page (right pane) */
    void* user;           /* passed back to every callback */
    /* Options ctor window: layout open; clone controls / CreateView here. */
    void (KPATCH_CALL *build)(void* panel, void* user);
    /* Page shown. */
    void (KPATCH_CALL *enter)(uint32_t page, void* user);
    /* Page hidden (Back, tick gap, panel destroy). */
    void (KPATCH_CALL *leave)(uint32_t page, void* user);
    /* Every Options draw while the page is shown; may be 0. */
    void (KPATCH_CALL *tick)(uint32_t page, void* user);
    /* Options dtor entry, before mod-config frees its controls. */
    void (KPATCH_CALL *destroy)(void* panel, void* user);
} K2ModConfigEntry;

typedef struct K2ModConfigApi {
    uint32_t struct_size;
    /* Adds an entry (copies *e; its strings must be static). > 0 token. 0 = refused (e is null, or all 24 slots are used). */
    int32_t  (KPATCH_CALL *Register)(const K2ModConfigEntry* e);
    /* Removes the entry with this token. An unknown token is a no-op. */
    void     (KPATCH_CALL *Unregister)(int32_t token);
} K2ModConfigApi;

typedef struct K2MuiRect { int32_t x, y, w, h; } K2MuiRect;

/* Option rows (doc 46). */
#define MUI_ROW_TOGGLE 0
#define MUI_ROW_STEPPER 1
#define MUI_ROW_SLIDER 2   /* look-alike slider (MCF5): a stepper's range / step / value / fmt / choices / zeroText, shown as "Label: value" + a draggable track.
                              Not a new API slot: a provider without sliders answers AddRow(kind 2) with < 0, and the caller falls back to MUI_ROW_STEPPER. */
#define MUI_RF_WRAP 1      /* stepper values wrap at the ends (ignored by a slider) */
#define MUI_RF_RESTART 2   /* shown text gets " (restart)" */
#define MUI_RF_DISABLED 4  /* greyed out: clicks are ignored */
#define MUI_RF_HIDDEN 8    /* not shown, takes no slot */

typedef struct K2MuiRowSpec {
    uint32_t struct_size;      /* sizeof(K2MuiRowSpec) */
    int32_t  kind;             /* MUI_ROW_*; 2 = slider: caption "Label: <formatted value>" left of the track, press / drag on the track sets the value */
    const char* label;         /* caption prefix ("Label: value") */
    const char* desc;          /* shown in the description pane while the row is hovered */
    int32_t  min, max, step;   /* stepper / slider range and step (toggle: ignored); a slider snaps the dragged value to min + n * step */
    int32_t  value;            /* initial value */
    int32_t  flags;            /* MUI_RF_* */
    const char* fmt;           /* "%d" / "%%" template, null = "label: value"; a fmt caption stands alone ("Heal below %d%%") */
    const char* const* choices;/* stepper/toggle value names, stored value = index; null = numbers (toggle: Off/On) */
    int32_t  nChoices;
    const char* zeroText;      /* shown instead of the number at value 0, e.g. "Auto" */
} K2MuiRowSpec;

/* Returns the FINAL value to store. dir = -1 / +1 for the arrows, 0 for a toggle flip; a slider passes dir = sign of (requested - old value). Return `requested` to accept, the old value to
   reject, anything else to snap. */
typedef int32_t (KPATCH_CALL *K2MuiChangeFn)(uint32_t page, int32_t row, int32_t requested, int32_t dir, void* user);

typedef struct K2MenuUiApi {
    uint32_t struct_size;
    /* Adds a page under the Options panel with this title. build() only. Returns the page id that the other calls take. */
    uint32_t (KPATCH_CALL *NewPage)(void* panel, const char* title);
    /* Adds a button to the page. build() only. Item id >= 0; < 0 = fail. */
    int32_t  (KPATCH_CALL *AddButton)(uint32_t page, const char* text, const char* desc);
    /* Adds a label to the page. build() only. Item id >= 0; < 0 = fail. */
    int32_t  (KPATCH_CALL *AddLabel)(uint32_t page, const char* text);
    /* Adds an engine control `ctl` to the page; flags MUI_NOFREE. build() only. Item id >= 0; < 0 = fail. */
    int32_t  (KPATCH_CALL *AddCustom)(uint32_t page, void* ctl, int32_t flags);
    /* 1 once per click on the item (on its release edge); 0 otherwise. */
    int32_t  (KPATCH_CALL *Clicked)(uint32_t page, int32_t item);
    /* Sets the item's text. */
    void     (KPATCH_CALL *SetText)(uint32_t page, int32_t item, const char* text);
    /* Sets the item's rect. */
    void     (KPATCH_CALL *SetRect)(uint32_t page, int32_t item, const K2MuiRect* r);
    /* Writes the right-hand description pane now. */
    void     (KPATCH_CALL *SetDesc)(uint32_t page, const char* text);
    /* MUI_PANE_*: the pane rect in measured engine px into *out. 1 ok, 0 = fail. */
    int32_t  (KPATCH_CALL *PaneRect)(void* panel, int32_t which, K2MuiRect* out);
    /* The engine control pointer of the item, for hit tests. */
    void*    (KPATCH_CALL *Control)(uint32_t page, int32_t item);
    /* Newer than the 11-slot table (check MenuClient::hasDescSplit). Entry pages only (not the list page). Shrinks that page's
       description pane to the bottom (1000 - previewPermille)/1000 of its original rect; previewPermille 1..5000 is clamped to
       600..850, 0 = off (restore). 1 ok, 0 = unknown page, no panel, or previewPermille < 0. Takes effect while the page is shown. */
    int32_t  (KPATCH_CALL *SetDescSplit)(uint32_t page, int32_t previewPermille);

    /* ---- Option rows (slots 12..18; check MenuClient::hasRows). The provider lays rows out itself in the column pane and draws
       the value caption, the "<" ">" arrows (or the toggle button) and the description on hover, so consumers write no geometry.
       Entry pages only; a page may mix AddRow with AddButton/AddLabel/AddCustom, but its rows then share the column pane.
       A page with more than 8 visible rows scrolls: the provider adds an up / down / thumb bar on the right of the column and the mouse
       wheel (through gui-input, when installed) moves 1 row per notch. Pages that lay items out with SetRect / AddCustom do not scroll. */

    /* Adds a row. build() only. Row id >= 0 (0, 1, 2 ... in call order, separate from item ids); < 0 = fail (no panel, too many rows
       or controls, bad spec). The spec and its strings are copied except `choices` (the array and its strings must stay valid). */
    int32_t  (KPATCH_CALL *AddRow)(uint32_t page, const K2MuiRowSpec* spec);
    /* Registers the page's change handler (one per page; null clears). It runs on the main thread inside the provider's tick, never
       from Clicked, once per accepted arrow press / toggle flip. The handler is dropped with the Options panel: set it again in every
       build() (the declarative layer does). */
    void     (KPATCH_CALL *SetChangeHandler)(uint32_t page, K2MuiChangeFn fn, void* user);
    /* The row's stored value (toggle 0/1, stepper value or choice index); 0 for an unknown row. */
    int32_t  (KPATCH_CALL *GetValue)(uint32_t page, int32_t row);
    /* Silent: clamps, stores and redraws the caption, no handler call. */
    void     (KPATCH_CALL *SetValue)(uint32_t page, int32_t row, int32_t v);
    /* Overrides the shown caption (null / empty = back to the formatted one). For dynamic text such as roster names. */
    void     (KPATCH_CALL *SetRowText)(uint32_t page, int32_t row, const char* text);
    /* Replaces the row's flags (MUI_RF_*; only DISABLED and HIDDEN change at run time, WRAP and RESTART are kept from the spec). */
    void     (KPATCH_CALL *SetRowState)(uint32_t page, int32_t row, int32_t flags);
    /* Changes a stepper's or slider's range; the stored value is re-clamped (no handler call). */
    void     (KPATCH_CALL *SetRowRange)(uint32_t page, int32_t row, int32_t minV, int32_t maxV);

    /* ---- Modal pages, item visibility and the shared gear (slots 19..22; check MenuClient::hasModal). A modal is a second set of
       controls on top of an entry page: while it is open the entry page's own AddButton / AddLabel / AddRow controls are hidden and the
       modal's are shown in their place, in the same column pane (or the `box` NewModal was given). AddCustom controls of the parent
       (a preview, say) stay on screen (the parent's gear hides), so the modal can change what the preview shows. The entry's enter / tick /
       leave callbacks are not interrupted. A modal page id works with every call that takes a page id (AddButton, AddLabel, AddRow,
       AddCustom, Clicked, SetText, SetRect, SetChangeHandler, GetValue, SetValue, SetRowText, SetRowState, SetRowRange, Control). */

    /* Shows or hides one item. build() or any later call. Rows' own controls are managed by SetRowState (refused here); an unknown
       item is a no-op. Hidden items take no clicks and no hover. */
    void     (KPATCH_CALL *SetItemShown)(uint32_t page, int32_t item, int32_t shown);
    /* Creates a modal page owned by `page` (the entry page NewPage returned). build() only. `box` (may be null) is the rect the modal's rows
       lay out in; null = the column pane. `title` replaces the Options title while the modal is open (null / empty = keep the page title).
       Returns the modal page id (> 0), 0 = refused (not in build, not the entry's own page, all 8 modals used). */
    uint32_t (KPATCH_CALL *NewModal)(uint32_t page, const K2MuiRect* box, const char* title);
    /* Opens or closes a modal. Opening works only while the modal's entry page is shown (from enter / tick / a change handler / Clicked
       handling); it closes any other open modal. Back closes the open modal first; Escape, a tick gap and leaving the entry clear it. */
    void     (KPATCH_CALL *SetModalOpen)(uint32_t modalPage, int32_t open);
    /* Adds the shared settings gear to an entry page: a provider-drawn square button in the top-right corner of the preview pane (the
       description pane without a split), same look on every page. build() only. Item id >= 0 (read clicks with Clicked); < 0 = fail.
       The gear stays on screen while one of the page's modals is open and then works as the toggle: a click on it closes the modal (Back does
       too) and is NOT latched for Clicked; with no modal open a click latches as usual. */
    int32_t  (KPATCH_CALL *AddGear)(uint32_t page, const char* desc);
} K2MenuUiApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(__cplusplus)
#define K2MC_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define K2MC_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#ifdef K2MC_STATIC_ASSERT
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, struct_size) == 0, "K2ModConfigEntry.struct_size offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, id) == 1 * sizeof(void*), "K2ModConfigEntry.id offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, name) == 2 * sizeof(void*), "K2ModConfigEntry.name offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, desc) == 3 * sizeof(void*), "K2ModConfigEntry.desc offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, user) == 4 * sizeof(void*), "K2ModConfigEntry.user offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, build) == 5 * sizeof(void*), "K2ModConfigEntry.build offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, enter) == 6 * sizeof(void*), "K2ModConfigEntry.enter offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, leave) == 7 * sizeof(void*), "K2ModConfigEntry.leave offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, tick) == 8 * sizeof(void*), "K2ModConfigEntry.tick offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigEntry, destroy) == 9 * sizeof(void*), "K2ModConfigEntry.destroy offset");
K2MC_STATIC_ASSERT(sizeof(K2ModConfigEntry) == 10 * sizeof(void*), "K2ModConfigEntry size");

K2MC_STATIC_ASSERT(offsetof(K2ModConfigApi, struct_size) == 0, "K2ModConfigApi.struct_size offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigApi, Register) == 1 * sizeof(void*), "K2ModConfigApi.Register offset");
K2MC_STATIC_ASSERT(offsetof(K2ModConfigApi, Unregister) == 2 * sizeof(void*), "K2ModConfigApi.Unregister offset");
K2MC_STATIC_ASSERT(sizeof(K2ModConfigApi) == 3 * sizeof(void*), "K2ModConfigApi size");

K2MC_STATIC_ASSERT(offsetof(K2MuiRect, x) == 0, "K2MuiRect.x offset");
K2MC_STATIC_ASSERT(offsetof(K2MuiRect, y) == 4, "K2MuiRect.y offset");
K2MC_STATIC_ASSERT(offsetof(K2MuiRect, w) == 8, "K2MuiRect.w offset");
K2MC_STATIC_ASSERT(offsetof(K2MuiRect, h) == 12, "K2MuiRect.h offset");
K2MC_STATIC_ASSERT(sizeof(K2MuiRect) == 16, "K2MuiRect size");

K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, struct_size) == 0, "K2MenuUiApi.struct_size offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, NewPage) == 1 * sizeof(void*), "K2MenuUiApi.NewPage offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, AddButton) == 2 * sizeof(void*), "K2MenuUiApi.AddButton offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, AddLabel) == 3 * sizeof(void*), "K2MenuUiApi.AddLabel offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, AddCustom) == 4 * sizeof(void*), "K2MenuUiApi.AddCustom offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, Clicked) == 5 * sizeof(void*), "K2MenuUiApi.Clicked offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetText) == 6 * sizeof(void*), "K2MenuUiApi.SetText offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetRect) == 7 * sizeof(void*), "K2MenuUiApi.SetRect offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetDesc) == 8 * sizeof(void*), "K2MenuUiApi.SetDesc offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, PaneRect) == 9 * sizeof(void*), "K2MenuUiApi.PaneRect offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, Control) == 10 * sizeof(void*), "K2MenuUiApi.Control offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetDescSplit) == 11 * sizeof(void*), "K2MenuUiApi.SetDescSplit offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, AddRow) == 12 * sizeof(void*), "K2MenuUiApi.AddRow offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetChangeHandler) == 13 * sizeof(void*), "K2MenuUiApi.SetChangeHandler offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, GetValue) == 14 * sizeof(void*), "K2MenuUiApi.GetValue offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetValue) == 15 * sizeof(void*), "K2MenuUiApi.SetValue offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetRowText) == 16 * sizeof(void*), "K2MenuUiApi.SetRowText offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetRowState) == 17 * sizeof(void*), "K2MenuUiApi.SetRowState offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetRowRange) == 18 * sizeof(void*), "K2MenuUiApi.SetRowRange offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetItemShown) == 19 * sizeof(void*), "K2MenuUiApi.SetItemShown offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, NewModal) == 20 * sizeof(void*), "K2MenuUiApi.NewModal offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, SetModalOpen) == 21 * sizeof(void*), "K2MenuUiApi.SetModalOpen offset");
K2MC_STATIC_ASSERT(offsetof(K2MenuUiApi, AddGear) == 22 * sizeof(void*), "K2MenuUiApi.AddGear offset");
K2MC_STATIC_ASSERT(sizeof(K2MenuUiApi) == 23 * sizeof(void*), "K2MenuUiApi size");
K2MC_STATIC_ASSERT(offsetof(K2MuiRowSpec, struct_size) == 0, "K2MuiRowSpec.struct_size offset");
K2MC_STATIC_ASSERT(offsetof(K2MuiRowSpec, kind) == 4, "K2MuiRowSpec.kind offset");
#undef K2MC_STATIC_ASSERT
#endif

#ifdef __cplusplus
namespace k2mc {

// Optional consumer, same shape as k2p3::Client. init(api) once; poll() every frame. connect() is tried on the first poll and then on
// every 256th call until it succeeds. ok() false = the interface is unavailable (the mod should skip its entry).
struct ConfigClient {
    const KPatchApi* api = nullptr;
    const K2ModConfigApi* p = nullptr;
    bool connected = false;
    unsigned pollCount = 0;
    bool reportedMissing = false;
    void (*onConnect)(bool ok, const char* why) = nullptr;

    void init(const KPatchApi* a) {
        api = a;
        p = nullptr;
        connected = false;
        pollCount = 0;
        reportedMissing = false;
    }

    // Every field the consumer calls is present and non-null, and the table is at least the v1 size.
    static bool usable(const K2ModConfigApi* t) {
        return t && t->struct_size >= sizeof(K2ModConfigApi) && t->Register && t->Unregister;
    }

    bool connect() {
        if (connected && p) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2ModConfigApi* t = static_cast<const K2ModConfigApi*>(api->require(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION));
        if (!usable(t)) {
            if (!reportedMissing && onConnect) onConnect(false, t ? "k2.modconfig v1 table too small" : "k2.modconfig v1 not provided");
            reportedMissing = true;
            return false;
        }
        p = t;
        connected = true;
        if (onConnect) onConnect(true, "connected");
        return true;
    }

    void poll() {
        if (!connected && (pollCount++ & 255) == 0) connect();
    }
    bool ok() const { return connected && p; }
};

// Optional consumer, same shape as k2p3::Client. Same polling rules as ConfigClient.
struct MenuClient {
    const KPatchApi* api = nullptr;
    const K2MenuUiApi* p = nullptr;
    bool connected = false;
    unsigned pollCount = 0;
    bool reportedMissing = false;
    void (*onConnect)(bool ok, const char* why) = nullptr;

    void init(const KPatchApi* a) {
        api = a;
        p = nullptr;
        connected = false;
        pollCount = 0;
        reportedMissing = false;
    }

    // Every pre-SetDescSplit field the consumer calls is present and non-null, and the table is at least the 11-slot size (so an older
    // provider without SetDescSplit stays usable). Check hasDescSplit() before calling SetDescSplit.
    static bool usable(const K2MenuUiApi* t) {
        return t && t->struct_size >= offsetof(K2MenuUiApi, Control) + sizeof(t->Control) && t->NewPage && t->AddButton && t->AddLabel && t->AddCustom && t->Clicked &&
               t->SetText && t->SetRect && t->SetDesc && t->PaneRect && t->Control;
    }

    // The table is long enough to hold SetDescSplit and it is non-null (an older provider answers false).
    static bool hasDescSplit(const K2MenuUiApi* t) {
        return t && t->struct_size >= offsetof(K2MenuUiApi, SetDescSplit) + sizeof(t->SetDescSplit) && t->SetDescSplit;
    }
    bool hasDescSplit() const { return connected && p && hasDescSplit(p); }

    // The table holds the seven option-row slots (12..18) and all of them are non-null (an older provider answers false).
    static bool hasRows(const K2MenuUiApi* t) {
        return t && t->struct_size >= offsetof(K2MenuUiApi, SetRowRange) + sizeof(t->SetRowRange) && t->AddRow && t->SetChangeHandler && t->GetValue &&
               t->SetValue && t->SetRowText && t->SetRowState && t->SetRowRange;
    }
    bool hasRows() const { return connected && p && hasRows(p); }

    // The table holds the four modal / gear slots (19..22) and all of them are non-null (an older provider answers false).
    static bool hasModal(const K2MenuUiApi* t) {
        return t && t->struct_size >= offsetof(K2MenuUiApi, AddGear) + sizeof(t->AddGear) && t->SetItemShown && t->NewModal && t->SetModalOpen && t->AddGear;
    }
    bool hasModal() const { return connected && p && hasModal(p); }

    bool connect() {
        if (connected && p) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2MenuUiApi* t = static_cast<const K2MenuUiApi*>(api->require(K2MENUUI_IFACE, K2MENUUI_VERSION));
        if (!usable(t)) {
            if (!reportedMissing && onConnect) onConnect(false, t ? "k2.menuui v1 table too small" : "k2.menuui v1 not provided");
            reportedMissing = true;
            return false;
        }
        p = t;
        connected = true;
        if (onConnect) onConnect(true, "connected");
        return true;
    }

    void poll() {
        if (!connected && (pollCount++ & 255) == 0) connect();
    }
    bool ok() const { return connected && p; }
};

} // namespace k2mc
#endif

#endif /* K2_MODCONFIGAPI_H */
