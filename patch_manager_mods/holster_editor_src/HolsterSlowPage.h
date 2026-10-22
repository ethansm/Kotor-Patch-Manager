// HolsterSlowPage.h -- the "Slow holster" Mod Configuration entry (doc 46 declarative layer), a second entry beside "Holstered Weapons".
// No Win32 here: HolsterEditor.cpp loads kPageSchema in KPatchInit, slow_holster_page_test.cpp parses it on the host.
//
// One stepper row (style slider, 1..8, 1 = normal speed) bound to the flat holster.ini key slow_holster_x. ModConfigDecl writes the accepted
// value there as a plain integer on every CHANGE (never on build, never for an unchanged value), with no handler; the holster DLL reads the
// key through its own 500 ms ini poll, so the row applies live (an integer is a valid value for holster's int-or-float reader).
//
// ModConfigDecl's stock reader (mcdecl::fileRead) keeps only the first 16 KB of the file, and the shipped holster.ini template is 21 KB: a
// slow_holster_x line appended past that cut would read back as 1 (normal) on the next open and the row would show the wrong value. readIni()
// below takes up to 64 KB and is installed as io.read; the write path (iniw::updateFile) reads the whole file and has no such cap.
#pragma once
#include <stdio.h>
#include <string.h>
#include "ModConfigDecl.h"

namespace hsp {

constexpr const char* kEntryId = "holster-slow";      // distinct from "holster-editor" (Holstered Weapons); registered second, so list order is stable
constexpr const char* kIniFile = "holster.ini";       // game-dir file, relative path (as holster.dll opens it)
constexpr const char* kIniKey = "slow_holster_x";     // flat key: holster.ini has no [section] headers
constexpr const char* kRowId = "slow";
constexpr int kMin = 1;
constexpr int kMax = 8;
constexpr int kDefault = 1;                            // 1 = normal speed, shown when the key is absent (nothing is written for it)

// Every TOML string value is capped at 95 characters by the schema reader (the test pins it). apply = "live": holster polls holster.ini.
constexpr const char* kPageSchema = R"TOML(
[page]
name = "Slow holster"
title = "Slow holster"
desc = "Slows the holster and draw motion to study it. Applies live; saved to holster.ini."

[[row]]
id = "slow"
kind = "stepper"
style = "slider"
label = "Slow holster"
desc = "1 = normal speed, 4 = four times slower. For watching the draw and stow paths."
default = 1
min = 1
max = 8
step = 1
fmt = "Slow holster: %dx"
apply = "live"
ini = "holster.ini"
key = "slow_holster_x"
)TOML";

// Same contract as mcdecl::IniIo::read (false = file or key absent), but over up to 64 KB instead of 16 KB.
inline bool readIni(const char* path, const char* section, const char* key, char* out, int cap) {
    static char buf[64 * 1024];
    FILE* f = path ? fopen(path, "rb") : nullptr;
    if (!f) return false;
    const size_t n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    return mcdecl::iniGet(buf, n, section, key, out, cap);
}

}  // namespace hsp
