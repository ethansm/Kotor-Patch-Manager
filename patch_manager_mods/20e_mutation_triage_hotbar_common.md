# 20e - Mutation triage: hotbar headers + shared FileWatch / Party

Scope: `scripts/mutate_header.py` survivors for six headers (HotbarConfig.h, HotbarLogic.h, TwoDA.h, Tlk.h in `force_power_hotbar_src/`; FileWatch.h, Party.h in `_shared_src/`). Every baseline survivor is classified, and the worthwhile ones are killed with tests only (no header was changed).

Classes: **K** = killed by a test added here (cited as `file:line`), **E** = equivalent mutant (no observable difference), **W** = not worth it (cannot be observed deterministically, or owned by another test file).

Tests touched: `config_roundtrip_test.cpp`, `hotbar_logic_test.cpp`, `twoda_test.cpp`, `tlk_test.cpp`, `common_contract_test.cpp` (Party semantics the hotbar relies on, messages "hotbar relies on: ..."), `filewatch_test.cpp`. Party.h survivors that the hotbar does not rely on are W "owner: party_test" (party_test.cpp was not edited).

Reproduce: `TMPDIR=<private dir> python3 scripts/mutate_header.py <header> --jobs 4` (see the first note below), or `--only N` for one mutant.

Notes for whoever runs the gate next:
- Always give `mutate_header.py` a private `TMPDIR`. Its scratch root `$TMPDIR/kotor-mutate-h` is shared per user; a second mutate/run_tests session deletes it ("baseline is not green ... could not launch"). The given HotbarLogic baseline contains 5 such false kills (ids 1, 4, 5, 6, 7).
- A killed mutant can leave state behind: a test that aborts before its cleanup leaves files in `$TMPDIR` (e.g. `test_save2_<pid>.ini.bak`, `filewatch_test.*/ro` with mode 0555). The new config tests remove stale files first and use per-pid file names, because `--jobs 4` runs the same test binary concurrently (fixed names were racy: a parallel Save's `.tmp` made the "no .tmp / no .bak" checks flake).
- Mutants inside `#ifdef _WIN32` code (FileWatch.h) can never be killed by the host build; they are all E.
- Mutation guards that read the mutated value (FileWatch id 28: the test's "ns resolution" guard used `stamp().mtime` itself) hide the kill; the new check compares against `stat()` independently.


## HotbarConfig.h

Score: **before 383/457** (given baseline) -> **after 425/457**. Fresh baseline after commit 15be79f: 384/457.  Classified: K=41 E=31 W=1 (73 baseline survivors).

| mutant id | line | mutation | class | reason / killing test |
|---|---|---|---|---|
| 3 | 17 | `10` -> `11` in `constexpr int Slots = 10;  // digits 1..9, 0` | K | `config_roundtrip_test.cpp:276` |
| 5 | 18 | `1` -> `2` in `constexpr int SchemaVersion = 1;` | K | `config_roundtrip_test.cpp:9` |
| 6 | 18 | `1` -> `0` in `constexpr int SchemaVersion = 1;` | K | `config_roundtrip_test.cpp:9` |
| 22 | 29 | `return false;` -> `{}` in `if (v.empty()) return false;` | E | `v.empty()`: strtoll("") leaves e == c_str(), rejected by the next clause anyway |
| 24 | 30 | `0` -> `1` in `errno = 0; char* e = nullptr; long long n = strtoll(v.c_str(), &e, ...` | E | errno pre-set to 1 is not ERANGE (34); strtoll only overwrites it on failure |
| 32 | 31 | `<` -> `<=` in `if (*e \|\| e == v.c_str() \|\| errno == ERANGE \|\| n < -2147483647LL - ...` | K | `config_roundtrip_test.cpp:281` |
| 33 | 31 | `-` -> `+` in `if (*e \|\| e == v.c_str() \|\| errno == ERANGE \|\| n < -2147483647LL - ...` | K | `config_roundtrip_test.cpp:281` |
| 34 | 31 | `1` -> `2` in `if (*e \|\| e == v.c_str() \|\| errno == ERANGE \|\| n < -2147483647LL - ...` | K | `config_roundtrip_test.cpp:281` |
| 35 | 31 | `1` -> `0` in `if (*e \|\| e == v.c_str() \|\| errno == ERANGE \|\| n < -2147483647LL - ...` | K | `config_roundtrip_test.cpp:281` |
| 41 | 37 | `0` -> `1` in `constexpr RowDef kRows[Rows] = { {"Bottom", 0} };` | E | RowDef::modifier is never read (the Ctrl/Alt rows were removed in 1.0.0) |
| 44 | 40 | `0` -> `1` in `enum Kind { K_None = 0, K_Spell, K_Feat, K_Item, K_Other };` | E | enum numbering: every use is symbolic and the number is never persisted (Format/Parse write names) |
| 45 | 44 | `0` -> `1` in `enum TargetMode { TM_Default = 0, TM_LowHp };` | E | enum numbering: TM_Default is only ever compared by name |
| 54 | 62 | `<` -> `<=` in `while (a < b && (s[a] == ' ' \|\| s[a] == '\t' \|\| s[a] == '\r' \|\| s[a...` | E | `a <= b` differs only at a == b == size(), where s[size()] is NUL, not whitespace |
| 76 | 63 | `1` -> `2` in `while (b > a && (s[b - 1] == ' ' \|\| s[b - 1] == '\t' \|\| s[b - 1] ==...` | K | `config_roundtrip_test.cpp:287` |
| 77 | 63 | `1` -> `0` in `while (b > a && (s[b - 1] == ' ' \|\| s[b - 1] == '\t' \|\| s[b - 1] ==...` | K | `config_roundtrip_test.cpp:287` |
| 81 | 63 | `1` -> `2` in `while (b > a && (s[b - 1] == ' ' \|\| s[b - 1] == '\t' \|\| s[b - 1] ==...` | K | `config_roundtrip_test.cpp:287` |
| 82 | 63 | `1` -> `0` in `while (b > a && (s[b - 1] == ' ' \|\| s[b - 1] == '\t' \|\| s[b - 1] ==...` | K | `config_roundtrip_test.cpp:287` |
| 87 | 66 | `<=` -> `<` in `inline std::string lower(std::string s) { for (auto& c : s) if (c >...` | K | `config_roundtrip_test.cpp:290` |
| 94 | 80 | `<` -> `<=` in `if (!(key & 0x40000000) \|\| name.size() < 4 \|\| name.back() != ')') r...` | K | `config_roundtrip_test.cpp:293` |
| 95 | 80 | `4` -> `5` in `if (!(key & 0x40000000) \|\| name.size() < 4 \|\| name.back() != ')') r...` | K | `config_roundtrip_test.cpp:293` |
| 96 | 80 | `4` -> `3` in `if (!(key & 0x40000000) \|\| name.size() < 4 \|\| name.back() != ')') r...` | E | a 3-char name cannot hold " (" + digit + ")" (4 chars), so the result is 0 either way |
| 102 | 82 | `||` -> `&&` in `if (p == std::string::npos \|\| p + 3 > name.size() - 1) return 0;` | K | `config_roundtrip_test.cpp:293` |
| 105 | 82 | `3` -> `2` in `if (p == std::string::npos \|\| p + 3 > name.size() - 1) return 0;` | E | "no room for a digit" guard: skipping it enters the loop with zero digits and returns n = 0, the same result |
| 107 | 82 | `-` -> `+` in `if (p == std::string::npos \|\| p + 3 > name.size() - 1) return 0;` | E | `p + 3 > size + 1` is never true for p <= size-2; the guard it replaces only ever yields n = 0 anyway |
| 109 | 82 | `1` -> `0` in `if (p == std::string::npos \|\| p + 3 > name.size() - 1) return 0;` | E | same guard: with `size - 0` the zero-digit case falls through the loop and returns 0 |
| 110 | 82 | `return 0;` -> `{}` in `if (p == std::string::npos \|\| p + 3 > name.size() - 1) return 0;` | K | `config_roundtrip_test.cpp:293` |
| 132 | 85 | `<` -> `<=` in `for (size_t i = p + 2; i < name.size() - 1; ++i) { if (name[i] < '0...` | K | `config_roundtrip_test.cpp:293` |
| 143 | 93 | `<` -> `<=` in `if (t.thresholdPct < 0 && t.fallback < 0) return s;` | K | `config_roundtrip_test.cpp:299` |
| 144 | 93 | `0` -> `1` in `if (t.thresholdPct < 0 && t.fallback < 0) return s;` | K | `config_roundtrip_test.cpp:299` |
| 149 | 94 | `>=` -> `>` in `s += ":"; if (t.thresholdPct >= 0) s += std::to_string(t.thresholdP...` | K | `config_roundtrip_test.cpp:300` |
| 150 | 94 | `0` -> `1` in `s += ":"; if (t.thresholdPct >= 0) s += std::to_string(t.thresholdP...` | K | `config_roundtrip_test.cpp:300` |
| 153 | 100 | `32` -> `33` in `char b[32];` | E | b[32] holds "spell:%d" (max 16 chars); snprintf never truncates |
| 154 | 100 | `32` -> `31` in `char b[32];` | E | b[32] holds "spell:%d" (max 16 chars); snprintf never truncates |
| 159 | 113 | `3` -> `4` in `std::string f[3]; int n = 0; size_t a = 0;` | E | f[3] -> f[4]: the `n == 3` check returns before f[3] is touched |
| 182 | 122 | `return false;` -> `{}` in `if (f[0] != "lowhp") return false;` | K | `config_roundtrip_test.cpp:306` |
| 183 | 122 | `false` -> `true` in `if (f[0] != "lowhp") return false;` | K | `config_roundtrip_test.cpp:306` |
| 191 | 126 | `>` -> `>=` in `for (char ch : f[1]) { if (ch < '0' \|\| ch > '9' \|\| v > 1000) return...` | K | `config_roundtrip_test.cpp:310` |
| 193 | 126 | `>` -> `>=` in `for (char ch : f[1]) { if (ch < '0' \|\| ch > '9' \|\| v > 1000) return...` | E | `v > 1000` -> `>=`: differs only at v == 1000, and every value > 100 is rejected by the 1..100 check right after |
| 194 | 126 | `1000` -> `1001` in `for (char ch : f[1]) { if (ch < '0' \|\| ch > '9' \|\| v > 1000) return...` | E | guard only prevents int wrap-around; 999 / 1001 give the same accept/reject (wrap test is the 4294967346 case) |
| 195 | 126 | `1000` -> `999` in `for (char ch : f[1]) { if (ch < '0' \|\| ch > '9' \|\| v > 1000) return...` | E | guard only prevents int wrap-around; 999 / 1001 give the same accept/reject (wrap test is the 4294967346 case) |
| 196 | 126 | `return false;` -> `{}` in `for (char ch : f[1]) { if (ch < '0' \|\| ch > '9' \|\| v > 1000) return...` | K | `config_roundtrip_test.cpp:312` |
| 245 | 155 | `return false;` -> `{}` in `if (c == std::string::npos) return false;` | K | `config_roundtrip_test.cpp:314` |
| 251 | 157 | `return false;` -> `{}` in `if (v.empty()) return false;` | K | `config_roundtrip_test.cpp:314` |
| 256 | 160 | `0` -> `1` in `int n = 0;` | E | n = 0 is overwritten by parseInt32 on every success path |
| 265 | 165 | `0` -> `1` in `if (v[0] == '-' \|\| v[0] == '+') return false;   // strtoull would w...` | K | `config_roundtrip_test.cpp:315` |
| 267 | 165 | `||` -> `&&` in `if (v[0] == '-' \|\| v[0] == '+') return false;   // strtoull would w...` | K | `config_roundtrip_test.cpp:315` |
| 268 | 165 | `0` -> `1` in `if (v[0] == '-' \|\| v[0] == '+') return false;   // strtoull would w...` | K | `config_roundtrip_test.cpp:315` |
| 270 | 165 | `return false;` -> `{}` in `if (v[0] == '-' \|\| v[0] == '+') return false;   // strtoull would w...` | K | `config_roundtrip_test.cpp:315` |
| 272 | 166 | `0` -> `1` in `errno = 0; unsigned long long n = strtoull(v.c_str(), &end, 16);` | E | errno pre-set to 1 is not ERANGE (34); strtoull only overwrites it on failure |
| 290 | 176 | `return lower(a.name) == lower(b.name);` -> `{}` in `if (a.kind == K_Item) return lower(a.name) == lower(b.name);` | K | `config_roundtrip_test.cpp:318` |
| 294 | 183 | `24` -> `25` in `char b[24]; snprintf(b, sizeof b, "%s%d", kRows[row].name, (slot + ...` | E | b[24] holds "Bottom%d" (8 chars) |
| 295 | 183 | `24` -> `23` in `char b[24]; snprintf(b, sizeof b, "%s%d", kRows[row].name, (slot + ...` | E | b[24] holds "Bottom%d" (8 chars) |
| 332 | 198 | `<=` -> `<` in `if (key.size() == len + 1 && lower(key.substr(0, len)) == n && key[...` | K | `config_roundtrip_test.cpp:321` |
| 343 | 215 | `<` -> `<=` in `for (int i = 0; i < Slots; ++i) {` | W | `i <= Slots` reads slots[row][10], one past the array (intra-object UB into `notes`): the garbage is not deterministically observable |
| 357 | 234 | `>=` -> `>` in `inline int effectiveThreshold(const TargetSpec& t, const General& g...` | K | `config_roundtrip_test.cpp:324` |
| 358 | 234 | `0` -> `1` in `inline int effectiveThreshold(const TargetSpec& t, const General& g...` | K | `config_roundtrip_test.cpp:324` |
| 373 | 272 | `4096` -> `4097` in `char buf[4096]; size_t n;` | E | read chunk size: the whole-file result is the same (the partial final chunk is pinned by the 4097-byte readFile case, id 378) |
| 374 | 272 | `4096` -> `4095` in `char buf[4096]; size_t n;` | E | read chunk size: the whole-file result is the same (the partial final chunk is pinned by the 4097-byte readFile case, id 378) |
| 378 | 274 | `0` -> `1` in `while ((n = fread(buf, 1, sizeof buf, f)) > 0) out->append(buf, n);` | K | `config_roundtrip_test.cpp:330` |
| 383 | 284 | `>=` -> `>` in `if (text.size() >= 3 && (unsigned char)text[0] == 0xEF && (unsigned...` | K | `config_roundtrip_test.cpp:341` |
| 384 | 284 | `3` -> `4` in `if (text.size() >= 3 && (unsigned char)text[0] == 0xEF && (unsigned...` | K | `config_roundtrip_test.cpp:341` |
| 385 | 284 | `3` -> `2` in `if (text.size() >= 3 && (unsigned char)text[0] == 0xEF && (unsigned...` | E | `size() >= 2`: a 2-byte text reads text[2] == NUL, never 0xBF, so no BOM is skipped either way |
| 405 | 285 | `<=` -> `<` in `while (pos <= text.size()) {` | E | `pos <= size()` -> `<`: the extra final empty-line iteration is a no-op |
| 410 | 288 | `1` -> `2` in `pos = (e == std::string::npos) ? text.size() + 1 : e + 1;` | E | `text.size() + 1` is only a past-the-end sentinel; +2 is equally past the end |
| 417 | 289 | `==` -> `!=` in `if (e != std::string::npos && text[e] == '\r' && pos < text.size() ...` | K | `config_roundtrip_test.cpp:339` |
| 419 | 289 | `<` -> `<=` in `if (e != std::string::npos && text[e] == '\r' && pos < text.size() ...` | E | `pos < size()` -> `<=`: text[size()] is NUL, never "\n" |
| 424 | 293 | `1` -> `2` in `if (sc != std::string::npos) { note = trim(line.substr(sc + 1)); li...` | K | `config_roundtrip_test.cpp:344` |
| 430 | 296 | `200` -> `201` in `char msg[200];   // every %s below is clipped (%.Ns) so the line nu...` | E | msg[200]: every %s is clipped, the longest message is 189 chars |
| 431 | 296 | `200` -> `199` in `char msg[200];   // every %s below is clipped (%.Ns) so the line nu...` | E | msg[199]: every %s is clipped, the longest message is 189 chars |
| 468 | 313 | `0` -> `1` in `int n = 0;` | E | n = 0 is overwritten by parseInt32 before use |
| 493 | 356 | `64` -> `65` in `char b[64];` | E | b[64] holds the longest line ("KeyboardDevice=-2147483648\n", 27 chars) |
| 494 | 356 | `64` -> `63` in `char b[64];` | E | b[64] holds the longest line ("KeyboardDevice=-2147483648\n", 27 chars) |
| 507 | 381 | `false` -> `true` in `bool Save(const std::string& path) const { return filewatch::atomic...` | K | `config_roundtrip_test.cpp:351` |

## HotbarLogic.h

Score: **before 340/409** (given baseline) -> **after 387/409**.  Note: 5 of the 340 kills (ids 1, 4, 5, 6, 7) were harness "could not launch" errors from a shared scratch dir race, not test kills; ids 1/4/5/6 are killed by real tests, id 7 survives (E).  Classified: K=48 E=21 W=1 (70 baseline survivors).

| mutant id | line | mutation | class | reason / killing test |
|---|---|---|---|---|
| 2 | 10 | `0` -> `1` in `struct Candidate { int cur = 0, max = 0; bool ok = false, down = fa...` | K | `hotbar_logic_test.cpp:500` |
| 3 | 10 | `false` -> `true` in `struct Candidate { int cur = 0, max = 0; bool ok = false, down = fa...` | K | `hotbar_logic_test.cpp:500` |
| 7 | 14 | `<=` -> `<` in `if (!m \|\| n <= 0) return -1;` | E | `n <= 0` -> `n < 0`: n == 0 with a non-null array runs no iterations and the fallback needs selfIdx < 0; returns -1 either way (this id was a harness "could not launch" kill in the given baseline) |
| 12 | 15 | `<` -> `<=` in `if (thresholdPct < 0) thresholdPct = 0;` | E | `thresholdPct < 0` -> `<=`: only differs at 0, which the assignment leaves at 0 |
| 13 | 15 | `0` -> `1` in `if (thresholdPct < 0) thresholdPct = 0;` | E | `thresholdPct < 1`: only differs at 0, which the assignment leaves at 0 |
| 14 | 15 | `0` -> `1` in `if (thresholdPct < 0) thresholdPct = 0;` | K | `hotbar_logic_test.cpp:505` |
| 15 | 16 | `>` -> `>=` in `if (thresholdPct > 100) thresholdPct = 100;` | E | `thresholdPct > 100` -> `>=`: only differs at 100, which the assignment leaves at 100 |
| 16 | 16 | `100` -> `101` in `if (thresholdPct > 100) thresholdPct = 100;` | K | `hotbar_logic_test.cpp:509` |
| 17 | 16 | `100` -> `99` in `if (thresholdPct > 100) thresholdPct = 100;` | E | `thresholdPct > 99`: only differs at 100, which the assignment leaves at 100 |
| 18 | 16 | `100` -> `101` in `if (thresholdPct > 100) thresholdPct = 100;` | K | `hotbar_logic_test.cpp:509` |
| 20 | 17 | `1` -> `2` in `int best = -1;` | E | best = -2: only ever tested with `best < 0` / `best >= 0` |
| 27 | 20 | `<=` -> `<` in `if (!c.ok \|\| c.down \|\| !c.eligible \|\| c.max <= 0) continue;` | K | `hotbar_logic_test.cpp:512` |
| 28 | 20 | `0` -> `1` in `if (!c.ok \|\| c.down \|\| !c.eligible \|\| c.max <= 0) continue;` | K | `hotbar_logic_test.cpp:514` |
| 29 | 21 | `100` -> `101` in `if ((long long)c.cur * 100 >= (long long)thresholdPct * c.max) cont...` | K | `hotbar_logic_test.cpp:516` |
| 46 | 27 | `<` -> `<=` in `if (fallbackSelf && selfIdx >= 0 && selfIdx < n && m[selfIdx].ok &&...` | K | `hotbar_logic_test.cpp:521` |
| 85 | 61 | `24` -> `25` in `char b[24]; snprintf(b, sizeof b, "obj%08x", oid);` | E | b[24] holds "obj%08x" (11 chars) |
| 86 | 61 | `24` -> `23` in `char b[24]; snprintf(b, sizeof b, "obj%08x", oid);` | E | b[24] holds "obj%08x" (11 chars) |
| 92 | 73 | `0` -> `1` in `if (n <= 0) return -1;` | K | `hotbar_logic_test.cpp:526` |
| 96 | 74 | `0` -> `1` in `for (int i = 0; i < n; ++i) if ((unsigned)keys[i] == selKey) return i;` | E | loop start 1: a match at index 0 returns 0, and so does no match (the fallback is 0) |
| 134 | 118 | `<` -> `<=` in `size_t p = d.find(" This "); std::string t = (p != std::string::npo...` | K | `hotbar_logic_test.cpp:530` |
| 135 | 118 | `120` -> `121` in `size_t p = d.find(" This "); std::string t = (p != std::string::npo...` | K | `hotbar_logic_test.cpp:530` |
| 157 | 127 | `0` -> `1` in `struct TipRect { int lines = 1, maxLen = 0, x = 0, y = 0, w = 0, h ...` | K | `hotbar_logic_test.cpp:545` |
| 158 | 127 | `0` -> `1` in `struct TipRect { int lines = 1, maxLen = 0, x = 0, y = 0, w = 0, h ...` | K | `hotbar_logic_test.cpp:545` |
| 159 | 127 | `0` -> `1` in `struct TipRect { int lines = 1, maxLen = 0, x = 0, y = 0, w = 0, h ...` | K | `hotbar_logic_test.cpp:545` |
| 160 | 127 | `0` -> `1` in `struct TipRect { int lines = 1, maxLen = 0, x = 0, y = 0, w = 0, h ...` | K | `hotbar_logic_test.cpp:545` |
| 164 | 130 | `>` -> `>=` in `for (char c : text) { if (c == '\n') { ++r.lines; cur = 0; } else i...` | E | `++cur > maxLen` -> `>=`: differs only when cur == maxLen, which assigns the same value |
| 175 | 133 | `>=` -> `>` in `if (topAnchor >= 0) y = topAnchor - r.h - 10;   // picker tooltips ...` | K | `hotbar_logic_test.cpp:534` |
| 176 | 133 | `0` -> `1` in `if (topAnchor >= 0) y = topAnchor - r.h - 10;   // picker tooltips ...` | K | `hotbar_logic_test.cpp:534` |
| 182 | 134 | `>` -> `>=` in `if (x + r.w > clientRight - 8) x = clientRight - 8 - r.w;` | E | `x + w >= right - 8`: at equality the shifted x equals the original x |
| 183 | 134 | `-` -> `+` in `if (x + r.w > clientRight - 8) x = clientRight - 8 - r.w;` | K | `hotbar_logic_test.cpp:539` |
| 184 | 134 | `8` -> `9` in `if (x + r.w > clientRight - 8) x = clientRight - 8 - r.w;` | E | `x + w > right - 9`: at x + w == right - 8 the shifted x equals the original x |
| 185 | 134 | `8` -> `7` in `if (x + r.w > clientRight - 8) x = clientRight - 8 - r.w;` | K | `hotbar_logic_test.cpp:539` |
| 190 | 135 | `<` -> `<=` in `if (x < 8) x = 8;` | E | `x <= 8` -> sets 8 to 8 |
| 191 | 135 | `8` -> `9` in `if (x < 8) x = 8;` | E | `x < 9`: x == 8 is set to 8 again |
| 192 | 135 | `8` -> `7` in `if (x < 8) x = 8;` | K | `hotbar_logic_test.cpp:541` |
| 195 | 136 | `<` -> `<=` in `if (y < 8) y = 8;` | E | `y <= 8` -> sets 8 to 8 |
| 196 | 136 | `8` -> `9` in `if (y < 8) y = 8;` | E | `y < 9`: y == 8 is set to 8 again |
| 197 | 136 | `8` -> `7` in `if (y < 8) y = 8;` | K | `hotbar_logic_test.cpp:543` |
| 203 | 146 | `3` -> `4` in `constexpr float QtyOutOfStockRgb[3] = { 0.55f, 0.55f, 0.55f };` | K | `hotbar_logic_test.cpp:553` |
| 206 | 150 | `false` -> `true` in `struct SlotView { const char* state = ""; bool hideSettling = false...` | K | `hotbar_logic_test.cpp:548` |
| 207 | 150 | `false` -> `true` in `struct SlotView { const char* state = ""; bool hideSettling = false...` | K | `hotbar_logic_test.cpp:548` |
| 208 | 150 | `false` -> `true` in `struct SlotView { const char* state = ""; bool hideSettling = false...` | K | `hotbar_logic_test.cpp:548` |
| 209 | 150 | `0` -> `1` in `struct SlotView { const char* state = ""; bool hideSettling = false...` | K | `hotbar_logic_test.cpp:548` |
| 216 | 160 | `1` -> `2` in `v.showQty = v.qty > 1 \|\| v.outOfStock; v.grayQty = v.outOfStock;` | K | `hotbar_logic_test.cpp:551` |
| 225 | 170 | `>` -> `>=` in `inline int clampOffset(int off, int maxOff) { return off > maxOff ?...` | E | `off >= maxOff`: at equality returns maxOff == off |
| 230 | 172 | `>` -> `>=` in `inline int wheelSteps(long acc) { if (!acc) return 0; int steps = (...` | E | `acc >= 0`: acc == 0 already returned 0 above, so the sign test never sees 0 |
| 247 | 183 | `0` -> `1` in `inline double aspectCorrection(int w, int h, int rw, int rh) { retu...` | K | `hotbar_logic_test.cpp:557` |
| 250 | 183 | `0` -> `1` in `inline double aspectCorrection(int w, int h, int rw, int rh) { retu...` | K | `hotbar_logic_test.cpp:557` |
| 253 | 183 | `0` -> `1` in `inline double aspectCorrection(int w, int h, int rw, int rh) { retu...` | K | `hotbar_logic_test.cpp:557` |
| 256 | 183 | `0` -> `1` in `inline double aspectCorrection(int w, int h, int rw, int rh) { retu...` | K | `hotbar_logic_test.cpp:557` |
| 267 | 197 | `>` -> `>=` in `if (f.size() > 6 && lower(f.substr(0, 6)) == "saves:") f = f.substr...` | E | every f of size <= 9 yields "" (size check `< 10`) whether or not "SAVES:" is stripped |
| 268 | 197 | `6` -> `7` in `if (f.size() > 6 && lower(f.substr(0, 6)) == "saves:") f = f.substr...` | E | every f of size <= 9 yields "" (size check `< 10`) whether or not "SAVES:" is stripped |
| 269 | 197 | `6` -> `5` in `if (f.size() > 6 && lower(f.substr(0, 6)) == "saves:") f = f.substr...` | E | every f of size <= 9 yields "" (size check `< 10`) whether or not "SAVES:" is stripped |
| 297 | 200 | `0` -> `1` in `for (int i = 0; i < 6; ++i) if (f[i] < '0' \|\| f[i] > '9') return "";` | K | `hotbar_logic_test.cpp:564` |
| 303 | 200 | `>` -> `>=` in `for (int i = 0; i < 6; ++i) if (f[i] < '0' \|\| f[i] > '9') return "";` | K | `hotbar_logic_test.cpp:566` |
| 316 | 213 | `0` -> `1` in `struct KeyEvent { unsigned dik = 0; bool down = false, ctrl = false...` | K | `hotbar_logic_test.cpp:572` |
| 317 | 213 | `false` -> `true` in `struct KeyEvent { unsigned dik = 0; bool down = false, ctrl = false...` | K | `hotbar_logic_test.cpp:574` |
| 321 | 213 | `false` -> `true` in `struct KeyEvent { unsigned dik = 0; bool down = false, ctrl = false...` | K | `hotbar_logic_test.cpp:575` |
| 323 | 213 | `false` -> `true` in `struct KeyEvent { unsigned dik = 0; bool down = false, ctrl = false...` | K | `hotbar_logic_test.cpp:574` |
| 324 | 213 | `false` -> `true` in `struct KeyEvent { unsigned dik = 0; bool down = false, ctrl = false...` | K | `hotbar_logic_test.cpp:576` |
| 331 | 215 | `1` -> `2` in `struct KeyAction { const char* decision = "pass"; bool swallow = fa...` | K | `hotbar_logic_test.cpp:571` |
| 332 | 215 | `1` -> `0` in `struct KeyAction { const char* decision = "pass"; bool swallow = fa...` | K | `hotbar_logic_test.cpp:571` |
| 349 | 244 | `0` -> `1` in `enum TgaStatus { TGA_OK = 0, TGA_NOT_UNCOMPRESSED32, TGA_TRUNCATED,...` | W | enum numbering: the DLL compares tgaInspect()'s result against the names (TGA_OK), never the number |
| 352 | 246 | `0` -> `1` in `struct TgaInfo { int w = 0, h = 0; unsigned off = 0, bytes = 0; }; ...` | K | `hotbar_logic_test.cpp:583` |
| 353 | 246 | `0` -> `1` in `struct TgaInfo { int w = 0, h = 0; unsigned off = 0, bytes = 0; }; ...` | K | `hotbar_logic_test.cpp:583` |
| 354 | 246 | `0` -> `1` in `struct TgaInfo { int w = 0, h = 0; unsigned off = 0, bytes = 0; }; ...` | K | `hotbar_logic_test.cpp:583` |
| 355 | 246 | `0` -> `1` in `struct TgaInfo { int w = 0, h = 0; unsigned off = 0, bytes = 0; }; ...` | K | `hotbar_logic_test.cpp:583` |
| 356 | 248 | `<` -> `<=` in `if (size < 18 \|\| d[1] != 0 \|\| d[2] != 2 \|\| d[16] != 32) return TGA_...` | K | `hotbar_logic_test.cpp:581` |
| 357 | 248 | `18` -> `19` in `if (size < 18 \|\| d[1] != 0 \|\| d[2] != 2 \|\| d[16] != 32) return TGA_...` | K | `hotbar_logic_test.cpp:581` |
| 393 | 250 | `0` -> `1` in `t->w = w; t->h = h; t->off = off; t->bytes = 0;` | K | `hotbar_logic_test.cpp:585` |

## TwoDA.h

Score: **before 97/121** (given baseline) -> **after 114/121**.  Classified: K=17 E=7 W=0 (24 baseline survivors).

| mutant id | line | mutation | class | reason / killing test |
|---|---|---|---|---|
| 2 | 22 | `65536` -> `65537` in `std::string d; char buf[65536]; size_t n;` | E | read buffer size: result is identical (final-chunk handling is pinned by the 65537-byte file, id 7) |
| 3 | 22 | `65536` -> `65535` in `std::string d; char buf[65536]; size_t n;` | E | read buffer size: result is identical (final-chunk handling is pinned by the 65537-byte file, id 7) |
| 7 | 23 | `0` -> `1` in `while ((n = fread(buf, 1, sizeof buf, f)) > 0) d.append(buf, n);` | K | `twoda_test.cpp:180` |
| 28 | 51 | `<` -> `<=` in `for (; i < a.size() && b[i]; ++i) if (lower(a[i]) != lower(b[i])) r...` | E | `i <= a.size()` in ieq: a[size()] is NUL and never equals a non-NUL b[i]; the loop falls out with the same false |
| 29 | 51 | `&&` -> `||` in `for (; i < a.size() && b[i]; ++i) if (lower(a[i]) != lower(b[i])) r...` | E | `&&` -> `\|\|` in ieq: when a is longer than b the next compare is a[i] vs NUL, which fails with the same false (only an embedded NUL in a column name could differ) |
| 37 | 54 | `<=` -> `<` in `static char lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(...` | K | `twoda_test.cpp:167` |
| 43 | 57 | `16` -> `15` in `if (d.size() < 16 \|\| d.compare(0, 8, "2DA V2.b") != 0) { error = "n...` | K | `twoda_test.cpp:84` |
| 53 | 59 | `<` -> `<=` in `while (p < d.size() && d[p] != '\0') {` | E | `p <= size()`: d[size()] is NUL, so the column loop ends either way |
| 63 | 65 | `4` -> `5` in `if (p + 4 > d.size()) { error = "truncated"; return false; }` | K | `twoda_test.cpp:113` |
| 65 | 65 | `>` -> `>=` in `if (p + 4 > d.size()) { error = "truncated"; return false; }` | K | `twoda_test.cpp:113` |
| 68 | 66 | `4` -> `3` in `uint32_t nrows = rd32(d, p); p += 4;` | K | `twoda_test.cpp:79` |
| 69 | 67 | `>` -> `>=` in `if (nrows > 100000) { error = "implausible row count"; return false; }` | K | `twoda_test.cpp:104` |
| 71 | 67 | `100000` -> `99999` in `if (nrows > 100000) { error = "implausible row count"; return false; }` | K | `twoda_test.cpp:104` |
| 82 | 74 | `2` -> `3` in `if (p + cells * 2 + 2 > d.size()) { error = "truncated offsets"; re...` | K | `twoda_test.cpp:112` |
| 83 | 74 | `2` -> `1` in `if (p + cells * 2 + 2 > d.size()) { error = "truncated offsets"; re...` | K | `twoda_test.cpp:112` |
| 84 | 74 | `+` -> `-` in `if (p + cells * 2 + 2 > d.size()) { error = "truncated offsets"; re...` | K | `twoda_test.cpp:112` |
| 86 | 74 | `2` -> `1` in `if (p + cells * 2 + 2 > d.size()) { error = "truncated offsets"; re...` | K | `twoda_test.cpp:112` |
| 96 | 79 | `<` -> `<=` in `for (size_t c = 0; c < cols.size(); ++c) {` | K | `twoda_test.cpp:80` |
| 102 | 81 | `>=` -> `>` in `if (o >= d.size()) { row.push_back(""); continue; }` | E | `o > size()`: at o == size() the cell is d.c_str() + o == "" either way |
| 112 | 89 | `2` -> `1` in `static uint32_t rd32(const std::string& d, size_t p) { return (uint...` | K | `twoda_test.cpp:105` |
| 113 | 89 | `16` -> `17` in `static uint32_t rd32(const std::string& d, size_t p) { return (uint...` | K | `twoda_test.cpp:105` |
| 117 | 89 | `3` -> `2` in `static uint32_t rd32(const std::string& d, size_t p) { return (uint...` | K | `twoda_test.cpp:106` |
| 118 | 89 | `24` -> `25` in `static uint32_t rd32(const std::string& d, size_t p) { return (uint...` | K | `twoda_test.cpp:107` |
| 119 | 89 | `24` -> `23` in `static uint32_t rd32(const std::string& d, size_t p) { return (uint...` | E | `<< 23`: any non-zero byte 3 already gives >= 8.4M, far above the 100000 limit (the 118 case wraps to 0 and is killed) |

## Tlk.h

Score: **before 115/130** (given baseline) -> **after 124/130**.  Classified: K=9 E=6 W=0 (15 baseline survivors).

| mutant id | line | mutation | class | reason / killing test |
|---|---|---|---|---|
| 1 | 15 | `0` -> `1` in `uint32_t count = 0, strOff = 0;` | K | `tlk_test.cpp:74` |
| 2 | 15 | `0` -> `1` in `uint32_t count = 0, strOff = 0;` | K | `tlk_test.cpp:74` |
| 3 | 16 | `false` -> `true` in `bool ok = false;` | K | `tlk_test.cpp:74` |
| 4 | 21 | `false` -> `true` in `path = p; ok = false; error.clear(); cache.clear();` | K | `tlk_test.cpp:94` |
| 6 | 24 | `20` -> `21` in `unsigned char h[20];` | E | h[20] -> h[21]: fread still reads exactly 20 bytes; buffer size only |
| 16 | 25 | `8` -> `9` in `if (fread(h, 1, 20, f) != 20 \|\| memcmp(h, "TLK V3.0", 8) != 0) { fc...` | K | `tlk_test.cpp:108` |
| 17 | 25 | `8` -> `7` in `if (fread(h, 1, 20, f) != 20 \|\| memcmp(h, "TLK V3.0", 8) != 0) { fc...` | K | `tlk_test.cpp:106` |
| 38 | 41 | `40` -> `41` in `unsigned char e[40];` | E | e[40] -> e[41]: fread still reads exactly 40 bytes; buffer size only |
| 46 | 44 | `40` -> `41` in `if (entPos + 40 <= 0x7FFFFFFFull && fseek(f, (long)entPos, SEEK_SET...` | E | count < 10,000,000 bounds entPos + 40 below 4.0e8, far under 0x7FFFFFFF: the guard never trips, so +41 is unreachable |
| 47 | 44 | `40` -> `39` in `if (entPos + 40 <= 0x7FFFFFFFull && fseek(f, (long)entPos, SEEK_SET...` | E | same: entPos + 39 <= 0x7FFFFFFF always holds for an accepted ref |
| 48 | 44 | `<=` -> `<` in `if (entPos + 40 <= 0x7FFFFFFFull && fseek(f, (long)entPos, SEEK_SET...` | E | same: `<` vs `<=` at 0x7FFFFFFF is unreachable (entPos + 40 <= 4.0e8) |
| 72 | 48 | `<=` -> `<` in `else if (dataPos + size <= 0x7FFFFFFFull && fseek(f, (long)dataPos,...` | K | `tlk_test.cpp:159` |
| 93 | 59 | `24` -> `23` in `static uint32_t u32(const unsigned char* p) { return p[0] \| (p[1] <...` | K | `tlk_test.cpp:107` |
| 94 | 61 | `false` -> `true` in `std::string o; bool sp = false;` | E | `sp = true` initially: a leading space is already skipped by `o.empty()` |
| 125 | 69 | `0x20` -> `0x1F` in `else if (c < 0x20 \|\| c > 0x7E) r = '?';` | K | `tlk_test.cpp:221` |

## FileWatch.h

Score: **before 55/85** (given baseline) -> **after 64/85**.  Classified: K=9 E=17 W=4 (30 baseline survivors).

| mutant id | line | mutation | class | reason / killing test |
|---|---|---|---|---|
| 2 | 25 | `0` -> `1` in `unsigned long long size = 0;` | K | `filewatch_test.cpp:33` |
| 3 | 26 | `0` -> `1` in `long long mtime = 0;    // Win32: FILETIME ticks (100 ns since 1601...` | K | `filewatch_test.cpp:33` |
| 7 | 32 | `-` -> `+` in `return (mtime - 116444736000000000LL) / 10000;` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 8 | 32 | `10000` -> `10001` in `return (mtime - 116444736000000000LL) / 10000;` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 9 | 32 | `10000` -> `9999` in `return (mtime - 116444736000000000LL) / 10000;` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 18 | 45 | `return s;` -> `{}` in `if (!GetFileAttributesExA(path, GetFileExInfoStandard, &d)) return s;` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 19 | 46 | `true` -> `false` in `s.exists = true;` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 20 | 47 | `32` -> `33` in `s.size = (static_cast<unsigned long long>(d.nFileSizeHigh) << 32) \|...` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 21 | 47 | `32` -> `31` in `s.size = (static_cast<unsigned long long>(d.nFileSizeHigh) << 32) \|...` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 22 | 48 | `32` -> `33` in `s.mtime = static_cast<long long>((static_cast<unsigned long long>(d...` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 23 | 48 | `32` -> `31` in `s.mtime = static_cast<long long>((static_cast<unsigned long long>(d...` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 28 | 54 | `+` -> `-` in `s.mtime = static_cast<long long>(st.st_mtim.tv_sec) * 1000000000LL ...` | K | `filewatch_test.cpp:48` |
| 29 | 63 | `1000` -> `1001` in `unsigned throttleMs = 1000;     // minimum ms between stamp() calls` | K | `filewatch_test.cpp:68` |
| 30 | 63 | `1000` -> `999` in `unsigned throttleMs = 1000;     // minimum ms between stamp() calls` | K | `filewatch_test.cpp:68` |
| 33 | 67 | `0` -> `1` in `unsigned lastCheck = 0;         // nowMs of the last check` | E | ReloadPoll::lastCheck is only read once `checked` is true, and `checked` is set together with lastCheck |
| 51 | 96 | `return false;` -> `{}` in `if (!in) return false;` | K | `filewatch_test.cpp:158` |
| 52 | 96 | `false` -> `true` in `if (!in) return false;` | K | `filewatch_test.cpp:158` |
| 53 | 98 | `false` -> `true` in `if (!out) { fclose(in); return false; }` | K | `filewatch_test.cpp:164` |
| 54 | 99 | `4096` -> `4097` in `char buf[4096]; size_t n; bool ok = true;` | E | copy buffer size: result identical (partial final chunk pinned by the 4097-byte .bak case, id 60) |
| 55 | 99 | `4096` -> `4095` in `char buf[4096]; size_t n; bool ok = true;` | E | copy buffer size: result identical (partial final chunk pinned by the 4097-byte .bak case, id 60) |
| 60 | 100 | `0` -> `1` in `while ((n = fread(buf, 1, sizeof buf, in)) > 0) if (fwrite(buf, 1, ...` | K | `filewatch_test.cpp:168` |
| 64 | 100 | `false` -> `true` in `while ((n = fread(buf, 1, sizeof buf, in)) > 0) if (fwrite(buf, 1, ...` | W | copyFile fwrite failure needs disk-full fault injection (no seam); `stamp(bak).exists` guards the only reachable path |
| 78 | 118 | `&&` -> `||` in `ok = (fflush(f) == 0) && ok;` | W | fflush / fsync / fclose failures each need ENOSPC injection and are masked by the other two checks (a /dev/full tmp fails all three), so no single-fault test exists |
| 79 | 120 | `==` -> `!=` in `ok = (_commit(_fileno(f)) == 0) && ok;     // FlushFileBuffers` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 80 | 120 | `0` -> `1` in `ok = (_commit(_fileno(f)) == 0) && ok;     // FlushFileBuffers` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 81 | 120 | `&&` -> `||` in `ok = (_commit(_fileno(f)) == 0) && ok;     // FlushFileBuffers` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 84 | 122 | `&&` -> `||` in `ok = (fsync(fileno(f)) == 0) && ok;` | W | same: redundant with the fflush / fclose checks |
| 87 | 124 | `&&` -> `||` in `ok = (fclose(f) == 0) && ok;` | W | same: redundant with the fflush / fsync checks |
| 93 | 128 | `!=` -> `==` in `ok = MoveFileExA(tmp.c_str(), path, MOVEFILE_REPLACE_EXISTING) != 0;` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |
| 94 | 128 | `0` -> `1` in `ok = MoveFileExA(tmp.c_str(), path, MOVEFILE_REPLACE_EXISTING) != 0;` | E | Win32-only branch (#ifdef _WIN32): not compiled in the host build, so no host test can reach it |

## Party.h

Score: **before 57/79** (given baseline) -> **after 61/79**.  Classified: K=4 E=15 W=3 (22 baseline survivors).

| mutant id | line | mutation | class | reason / killing test |
|---|---|---|---|---|
| 6 | 35 | `0` -> `1` in `uint32_t app = 0, a = 0, b = 0, t = 0;` | E | local initialiser overwritten by a successful readAt before use (a failed read short-circuits) |
| 7 | 35 | `0` -> `1` in `uint32_t app = 0, a = 0, b = 0, t = 0;` | E | local initialiser overwritten by a successful readAt before use (a failed read short-circuits) |
| 8 | 35 | `0` -> `1` in `uint32_t app = 0, a = 0, b = 0, t = 0;` | E | local initialiser overwritten by a successful readAt before use (a failed read short-circuits) |
| 9 | 35 | `0` -> `1` in `uint32_t app = 0, a = 0, b = 0, t = 0;` | E | local initialiser overwritten by a successful readAt before use (a failed read short-circuits) |
| 25 | 45 | `0` -> `1` in `int n = 0;` | E | n = 0: partyCount returns 0 on a failed read through the early return |
| 27 | 46 | `0` -> `1` in `if (table == 0 \|\| !saferead::readAt<int>(SiteBase, table, 0, &n)) r...` | E | `table == 0` -> `== 1`: table 0 makes readAt(0 + 0) fail, which also returns 0 |
| 30 | 46 | `return 0;` -> `{}` in `if (table == 0 \|\| !saferead::readAt<int>(SiteBase, table, 0, &n)) r...` | E | failed readAt leaves n == 0, so falling through returns the same 0 |
| 35 | 58 | `0` -> `1` in `if (n <= 0) { *w = F_Count; return 0; }` | K | `common_contract_test.cpp:527` |
| 39 | 82 | `0` -> `1` in `if (table == 0 \|\| i < 0 \|\| i >= partyCount(table) \|\| !fn) return 0;` | E | `table == 0` guard: partyCount(0) is 0, so `i >= partyCount` returns 0 anyway |
| 49 | 91 | `0` -> `1` in `if (member == 0 \|\| !fn) return 0;` | W | owner: party_test. serverCreature(0) is never called by the hotbar (members() skips a null client creature) |
| 53 | 96 | `0` -> `1` in `struct Vitals { uintptr_t client = 0, server = 0; int cur = 0, max ...` | K | `common_contract_test.cpp:541` |
| 63 | 110 | `0` -> `1` in `uint32_t vt = 0, fMax = 0, fCur = 0;` | E | local initialiser overwritten by a successful readAt before use (a failed read short-circuits) |
| 64 | 110 | `0` -> `1` in `uint32_t vt = 0, fMax = 0, fCur = 0;` | E | local initialiser overwritten by a successful readAt before use (a failed read short-circuits) |
| 65 | 110 | `0` -> `1` in `uint32_t vt = 0, fMax = 0, fCur = 0;` | E | local initialiser overwritten by a successful readAt before use (a failed read short-circuits) |
| 75 | 117 | `0` -> `1` in `v.ok = v.max > 0;` | W | owner: party_test. max == 1 HP is an edge of Party.h; the hotbar only reads ok/cur/max of real party members |
| 80 | 126 | `<=` -> `<` in `if (!out \|\| cap <= 0) return 0;` | E | `cap < 0`: cap == 0 still returns 0 at `n <= 0` below |
| 81 | 126 | `0` -> `1` in `if (!out \|\| cap <= 0) return 0;` | W | owner: party_test. the hotbar always passes cap 3; cap == 1 is a Party.h edge |
| 84 | 128 | `return 0;` -> `{}` in `if (!t) return 0;` | E | `!t` guard: partyCount(0) is 0, so `n <= 0` returns 0 anyway |
| 86 | 130 | `>` -> `>=` in `if (n > cap) n = cap;` | E | `n >= cap`: at equality n = cap is unchanged |
| 87 | 131 | `<=` -> `<` in `if (n <= 0) return 0;` | E | `n < 0`: n == 0 runs no iterations and returns n == 0 |
| 88 | 131 | `0` -> `1` in `if (n <= 0) return 0;` | K | `common_contract_test.cpp:531` |
| 89 | 131 | `return 0;` -> `{}` in `if (n <= 0) return 0;` | K | `common_contract_test.cpp:535` |

## Bugs found

None. Every new check passed against the unmodified headers on the first run (HotbarConfig 157 + HotbarLogic 85 new checks, plus the Tlk / TwoDA / FileWatch / Party additions).

## Open items

- FileWatch ids 64, 78, 84, 87 (W) would need an ENOSPC / write-failure injection seam in `atomicWrite` / `copyFile`; not worth a header change.
- Party ids 49, 75, 81 (W) belong in `party_test.cpp` (owner), not in the hotbar contract.
