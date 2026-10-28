#pragma once
constexpr uintptr_t PlayThing = 0x00403000;   // thiscall(mgr, char idx)
constexpr int SomeGlobal = 0x00990010;        // global data table
constexpr uintptr_t kConflictFn = 0x00402000; // disagrees with the KDB
constexpr uintptr_t ZedAlt = 0x00600000;      // bare alias for Baz::Zed
