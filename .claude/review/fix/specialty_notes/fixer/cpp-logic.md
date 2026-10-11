## Specialty: cpp-logic (fixer)
- Mods build with MinGW i686 C++17 (-O2, static). Keep 32-bit assumptions; no new headers outside the file's existing include style (`"../_shared/X.h"`).
- Typical fixes: boundary condition, stale cache flag set without validity check, missing null check, wrong clamp, integer overflow in size math, comment contradicting code (fix the code only when the claim proves the code is wrong; else fix the comment).
- Do not move code between files or touch shared headers unless the task allows that file.
- Game-engine behaviour you cannot see in the code is not yours to assume: report not-fixable.
