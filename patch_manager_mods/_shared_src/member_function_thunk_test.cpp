// member_function_thunk_test.cpp - host + -m32 test for Patches/Common/MemberFunctionThunk.h.
// Fake game objects = raw buffers; wrappers = GameAPIObject subclasses over them.
// The thunk is called the way the engine does: game pointer in ECX, one stack arg.
#include "TestHarness.h"
#include "MemberFunctionThunk.h"

namespace {

struct Panel : GameAPIObject {
    int hits = 0;
    void* last = nullptr;
    int otherHits = 0;
    void* otherLast = nullptr;
    explicit Panel(void* game) : GameAPIObject(game) {}
    void InitializeFunctions() override {}
    void InitializeOffsets() override {}
    void OnClick(void* p) { hits++; last = p; }
    void OnOther(void* p) { otherHits++; otherLast = p; }
};

typedef void(__fastcall* ThunkFn)(void*, void*, void*);

alignas(4) char gameA[64];
alignas(4) char gameB[64];
alignas(4) char gameC[64];
alignas(4) char argX[8];
alignas(4) char argY[8];

void TestRegistry() {
    ThunkRegistry::Objects().clear();
    Panel a(gameA), b(gameB);
    ThunkRegistry::Register(&a);
    KT_CHECK(ThunkRegistry::Objects().size() == 1);
    KT_CHECK(ThunkRegistry::Objects().count(gameA) == 1);
    KT_CHECK(ThunkRegistry::Lookup(gameA) == &a);
    KT_CHECK(ThunkRegistry::Lookup(gameB) == nullptr);
    KT_CHECK(ThunkRegistry::Lookup(nullptr) == nullptr);
    ThunkRegistry::Objects().clear();
}

void TestNullRejected() {
    ThunkRegistry::Objects().clear();
    Panel a(gameA);
    ThunkRegistry::Register(&a);
    size_t before = ThunkRegistry::Objects().size();
    Panel nul(nullptr);
    ThunkRegistry::Register(nullptr);
    KT_CHECK(ThunkRegistry::Objects().size() == before);
    ThunkRegistry::Register(&nul);
    KT_CHECK(ThunkRegistry::Objects().size() == before);
    ThunkRegistry::Unregister(nullptr);
    ThunkRegistry::Unregister(&nul);
    KT_CHECK(ThunkRegistry::Objects().size() == before);
    KT_CHECK(ThunkRegistry::Lookup(gameA) == &a);
    ThunkRegistry::Objects().clear();
}

void TestUnregisterOverwrite() {
    ThunkRegistry::Objects().clear();
    Panel a(gameA), b(gameB);
    ThunkRegistry::Register(&a);
    ThunkRegistry::Register(&b);
    KT_CHECK(ThunkRegistry::Objects().size() == 2);
    ThunkRegistry::Unregister(&a);
    KT_CHECK(ThunkRegistry::Objects().size() == 1);
    KT_CHECK(ThunkRegistry::Lookup(gameA) == nullptr);
    KT_CHECK(ThunkRegistry::Lookup(gameB) == &b);
    // Unregister of an unregistered wrapper is a no-op.
    ThunkRegistry::Unregister(&a);
    KT_CHECK(ThunkRegistry::Objects().size() == 1);
    // Re-register a new wrapper on the same game pointer overwrites.
    Panel b2(gameB);
    ThunkRegistry::Register(&b2);
    KT_CHECK(ThunkRegistry::Objects().size() == 1);
    KT_CHECK(ThunkRegistry::Lookup(gameB) == &b2);
    KT_CHECK(ThunkRegistry::Lookup(gameB) != &b);
    ThunkRegistry::Objects().clear();
}

void TestDispatch() {
    ThunkRegistry::Objects().clear();
    Panel a(gameA), b(gameB);
    ThunkRegistry::Register(&a);
    ThunkRegistry::Register(&b);
    ThunkFn click = reinterpret_cast<ThunkFn>(memberThunkAddr<Panel, &Panel::OnClick>());
    ThunkFn other = reinterpret_cast<ThunkFn>(memberThunkAddr<Panel, &Panel::OnOther>());
    KT_CHECK(click != nullptr);
    KT_CHECK(other != nullptr);
    KT_CHECK((void*)click != (void*)other);

    click(gameA, nullptr, argX);
    KT_CHECK(a.hits == 1);
    KT_CHECK(a.last == (void*)argX);
    KT_CHECK(b.hits == 0);
    KT_CHECK(b.last == nullptr);
    KT_CHECK(a.otherHits == 0);

    click(gameB, nullptr, argY);
    KT_CHECK(b.hits == 1);
    KT_CHECK(b.last == (void*)argY);
    KT_CHECK(a.hits == 1);
    KT_CHECK(a.last == (void*)argX);

    other(gameB, nullptr, argX);
    KT_CHECK(b.otherHits == 1);
    KT_CHECK(b.otherLast == (void*)argX);
    KT_CHECK(b.hits == 1);
    KT_CHECK(a.otherHits == 0);

    click(gameA, nullptr, nullptr);
    KT_CHECK(a.hits == 2);
    KT_CHECK(a.last == nullptr);
    ThunkRegistry::Objects().clear();
}

void TestUnregisteredGame() {
    ThunkRegistry::Objects().clear();
    Panel a(gameA);
    ThunkRegistry::Register(&a);
    ThunkFn click = reinterpret_cast<ThunkFn>(memberThunkAddr<Panel, &Panel::OnClick>());
    click(gameC, nullptr, argX);
    click(nullptr, nullptr, argX);
    KT_CHECK(a.hits == 0);
    KT_CHECK(a.otherHits == 0);
    // After Unregister the formerly valid game pointer is dead too.
    ThunkRegistry::Unregister(&a);
    click(gameA, nullptr, argX);
    KT_CHECK(a.hits == 0);
    KT_CHECK(ThunkRegistry::Objects().empty());
}

#if defined(__i386__)
// Engine-shaped call: ECX = game object, one stack argument, callee pops it.
// Returns ESP-before minus ESP-after (0 when the callee cleaned its stack arg).
// fn lives in ESI and the delta in EDI (both callee-saved); the param is pushed
// from memory so no assumption is made about ebx/ebp.
__attribute__((noinline)) int ThiscallDelta(void* fn, void* game, void* param) {
    int delta;
    void* ecxv = game;
    void* edxv = nullptr;
    void* eaxv = nullptr;
    __asm__ __volatile__(
        "movl %%esp, %%edi\n\t"
        "pushl %[p]\n\t"
        "call *%%esi\n\t"
        "subl %%esp, %%edi\n\t"
        : "=&D"(delta), "+c"(ecxv), "+d"(edxv), "+a"(eaxv)
        : "S"(fn), [p] "m"(param)
        : "memory", "cc");
    return delta;
}

void TestThiscallShape() {
    ThunkRegistry::Objects().clear();
    Panel a(gameA);
    ThunkRegistry::Register(&a);
    void* fn = memberThunkAddr<Panel, &Panel::OnClick>();
    int delta = ThiscallDelta(fn, gameA, argX);
    KT_CHECK(a.hits == 1);
    KT_CHECK(a.last == (void*)argX);
    KT_CHECK(delta == 0);
    // Unregistered path must also pop its stack argument.
    int delta2 = ThiscallDelta(fn, gameC, argY);
    KT_CHECK(a.hits == 1);
    KT_CHECK(delta2 == 0);
    ThunkRegistry::Objects().clear();
}
#endif

void TestAddressStable() {
    void* p1 = memberThunkAddr<Panel, &Panel::OnClick>();
    void* p2 = memberThunkAddr<Panel, &Panel::OnClick>();
    void* o1 = memberThunkAddr<Panel, &Panel::OnOther>();
    void* o2 = memberThunkAddr<Panel, &Panel::OnOther>();
    KT_CHECK(p1 == p2);
    KT_CHECK(o1 == o2);
    KT_CHECK(p1 != o1);
}

} // namespace

int main() {
    TestRegistry();
    TestNullRejected();
    TestUnregisterOverwrite();
    TestDispatch();
    TestUnregisteredGame();
#if defined(__i386__)
    TestThiscallShape();
#endif
    TestAddressStable();
    return kt::Report("member_function_thunk_test");
}
