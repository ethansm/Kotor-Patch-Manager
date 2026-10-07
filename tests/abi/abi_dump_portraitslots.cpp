// tests/abi/abi_dump_portraitslots.cpp - Dumps ABI layout for k2.portraitslots@1 (Doc 22 U6).
// Compiled at -m32 to verify 32-bit x86 DLL width struct layout.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include "PortraitSlots.h"

int main() {
    printf("[interface]\n");
    printf("name = \"%s\"\n", K2PS_IFACE);
    printf("major = %d\n", K2PS_VERSION);
    printf("header = \"patch_manager_mods/_shared_src/PortraitSlots.h\"\n");
    printf("provides = \"%s@%d\"\n\n", K2PS_IFACE, K2PS_VERSION);

    printf("[constants]\n");
    printf("K2PS_IFACE = \"%s\"\n", K2PS_IFACE);
    printf("K2PS_VERSION = %d\n", K2PS_VERSION);
    printf("K2PS_SNAPSHOT = %d\n", K2PS_SNAPSHOT);
    printf("K2PS_PRE_ENGINE_READ = %d\n", K2PS_PRE_ENGINE_READ);
    printf("K2PS_FRAME_BEGIN = %d\n", K2PS_FRAME_BEGIN);
    printf("K2PS_SLOT = %d\n", K2PS_SLOT);
    printf("K2PS_MENU_OVERLAY = %d\n", K2PS_MENU_OVERLAY);
    printf("K2PS_HUD_DRAW = %d\n", K2PS_HUD_DRAW);
    printf("K2PS_PHASE_COUNT = %d\n", K2PS_PHASE_COUNT);
    printf("K2PS_PRIO_SHIELD = %d\n", K2PS_PRIO_SHIELD);
    printf("K2PS_PRIO_BUFF = %d\n", K2PS_PRIO_BUFF);
    printf("K2PS_MAX_SLOTS = %d\n\n", K2PS_MAX_SLOTS);

    printf("[struct.K2PsContext]\n");
    printf("sizeof = %zu\n", sizeof(K2PsContext));
    printf("fields = [\n");
    #define FIELD_CTX(f) printf("    { name = \"%s\", offset = %zu, size = %zu },\n", #f, offsetof(K2PsContext, f), sizeof(((K2PsContext*)0)->f))
    FIELD_CTX(struct_size);
    FIELD_CTX(phase);
    FIELD_CTX(slot);
    FIELD_CTX(creature);
    FIELD_CTX(isLarge);
    FIELD_CTX(callerEbp);
    FIELD_CTX(frame);
    FIELD_CTX(inMenu);
    FIELD_CTX(panel);
    FIELD_CTX(panelRect);
    FIELD_CTX(menuCreature);
    FIELD_CTX(companions);
    FIELD_CTX(companionCtrl);
    #undef FIELD_CTX
    printf("]\n\n");

    printf("[struct.K2PortraitSlotsApi]\n");
    printf("sizeof = %zu\n", sizeof(K2PortraitSlotsApi));
    printf("fields = [\n");
    #define FIELD_API(f) printf("    { name = \"%s\", offset = %zu, size = %zu },\n", #f, offsetof(K2PortraitSlotsApi, f), sizeof(((K2PortraitSlotsApi*)0)->f))
    FIELD_API(struct_size);
    FIELD_API(Register);
    FIELD_API(Unregister);
    FIELD_API(CreatureForSlot);
    FIELD_API(SlotAt);
    FIELD_API(Frame);
    #undef FIELD_API
    printf("]\n");

    return 0;
}
