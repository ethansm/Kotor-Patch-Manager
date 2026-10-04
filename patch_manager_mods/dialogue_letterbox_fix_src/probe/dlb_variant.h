// Probe variant marker. K2DialogueLetterboxFix.cpp includes this header only when it is on the include path
// (`#if __has_include("dlb_variant.h")`), so the shipping dir (no dlb_variant.h) compiles none of the probe code.
#pragma once
#define DLB_PROBE 1
