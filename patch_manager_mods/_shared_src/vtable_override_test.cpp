// vtable_override_test.cpp - host + -m32 test for Patches/Common/VTableOverride.h.
// Fake game object = a struct whose first member is the vtable pointer.
#include "check.h"
#include "VTableOverride.h"

namespace {

struct Obj { void** vptr; };
typedef int (*Fn)(void*);

int f0(void*) { return 10; }
int f1(void*) { return 11; }
int f2(void*) { return 12; }
int f3(void*) { return 13; }
int alt2(void*) { return 99; }

void* g_table[4] = { (void*)&f0, (void*)&f1, (void*)&f2, (void*)&f3 };

// Call-through: the override recovers the owner via GetOwner(self).
struct Owner { VTableOverride* vo; };
int thunk2(void* self) {
    Owner* o = static_cast<Owner*>(VTableOverride::GetOwner(self));
    Fn orig = reinterpret_cast<Fn>(o->vo->GetOriginal(2));
    return orig(self) + 1000;
}

#define VT(cond) kt::Check("VTableOverride: " #cond, (cond))

void TestInstallRestore() {
    Obj obj = { g_table };
    int owner = 0;
    {
        VTableOverride vo(&obj, &owner, 4);
        VT(vo.IsActive());
        VT(obj.vptr != g_table);
        VT(VTableOverride::GetOwner(&obj) == &owner);
        for (int i = 0; i < 4; ++i) {
            kt::Check("VTableOverride: slot copied == original", obj.vptr[i] == g_table[i]);
            kt::Check("VTableOverride: GetOriginal == table", vo.GetOriginal(i) == g_table[i]);
        }
    }
    VT(obj.vptr == g_table);
}

void TestOverride() {
    Obj obj = { g_table };
    int owner = 0;
    {
        VTableOverride vo(&obj, &owner, 4);
        vo.Override(2, (void*)&alt2);
        VT(obj.vptr[0] == g_table[0]);
        VT(obj.vptr[1] == g_table[1]);
        VT(obj.vptr[2] == (void*)&alt2);
        VT(obj.vptr[3] == g_table[3]);
        VT(vo.GetOriginal(2) == g_table[2]);
        VT(reinterpret_cast<Fn>(obj.vptr[2])(&obj) == 99);
    }
    VT(obj.vptr == g_table);
    VT(g_table[2] == (void*)&f2);
}

void TestBounds() {
    Obj obj = { g_table };
    int owner = 0;
    VTableOverride vo(&obj, &owner, 4);
    vo.Override(-1, (void*)&alt2);
    vo.Override(4, (void*)&alt2);
    for (int i = 0; i < 4; ++i)
        kt::Check("VTableOverride: out-of-range Override changed a slot", obj.vptr[i] == g_table[i]);
    VT(VTableOverride::GetOwner(&obj) == &owner);   // slot -1 (owner) untouched
    VT(vo.GetOriginal(-1) == nullptr);
    VT(vo.GetOriginal(4) == nullptr);
}

void TestInert() {
    int owner = 0;
    {
        VTableOverride vo(nullptr, &owner, 4);
        VT(!vo.IsActive());
        vo.Override(0, (void*)&alt2);
        VT(vo.GetOriginal(0) == nullptr);
    }
    Obj obj = { g_table };
    {
        VTableOverride vo(&obj, &owner, 0);
        VT(!vo.IsActive());
        VT(obj.vptr == g_table);
        vo.Override(0, (void*)&alt2);
    }
    VT(obj.vptr == g_table);
    {
        VTableOverride vo(&obj, &owner, -1);
        VT(!vo.IsActive());
        VT(obj.vptr == g_table);
    }
    VT(obj.vptr == g_table);
    Obj nul = { nullptr };
    {
        VTableOverride vo(&nul, &owner, 4);
        VT(!vo.IsActive());
        vo.Override(1, (void*)&alt2);
        VT(vo.GetOriginal(1) == nullptr);
    }
    VT(nul.vptr == nullptr);
    VT(VTableOverride::GetOwner(nullptr) == nullptr);
    VT(VTableOverride::GetOwner(&nul) == nullptr);
}

void TestCallThrough() {
    Obj obj = { g_table };
    Owner owner = { nullptr };
    {
        VTableOverride vo(&obj, &owner, 4);
        owner.vo = &vo;
        vo.Override(2, (void*)&thunk2);
        VT(reinterpret_cast<Fn>(obj.vptr[2])(&obj) == 12 + 1000);
        VT(reinterpret_cast<Fn>(obj.vptr[1])(&obj) == 11);
    }
    VT(reinterpret_cast<Fn>(obj.vptr[2])(&obj) == 12);
}

} // namespace

int main() {
    TestInstallRestore();
    TestOverride();
    TestBounds();
    TestInert();
    TestCallThrough();
    return kt::Report("vtable_override_test");
}
