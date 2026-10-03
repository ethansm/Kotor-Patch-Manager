// wait_line.h - poll a log file until a line matches (used by smoke_probe11/12; the probe DLL writes its logs from a helper thread).
// A pattern "A ... B ... C" matches a line containing A, B and C in that order (the " ... " is the wildcard).
#pragma once
#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>

static inline std::string wlReadFile(const char* name) {
    std::string s;
    FILE* f = fopen(name, "rb");
    if (!f) return s;
    char b[4096]; size_t k;
    while ((k = fread(b, 1, sizeof b, f)) > 0) s.append(b, k);
    fclose(f);
    return s;
}

// First line of `text` matching `pattern`, or "" if none.
static inline std::string wlFindLine(const std::string& text, const std::string& pattern) {
    std::vector<std::string> parts;
    size_t p = 0;
    for (;;) {
        size_t q = pattern.find(" ... ", p);
        if (q == std::string::npos) { parts.push_back(pattern.substr(p)); break; }
        parts.push_back(pattern.substr(p, q - p)); p = q + 5;
    }
    size_t pos = 0;
    while (pos < text.size()) {
        size_t e = text.find('\n', pos);
        if (e == std::string::npos) e = text.size();
        std::string line = text.substr(pos, e - pos);
        size_t at = 0; bool ok = true;
        for (auto& part : parts) {
            size_t f = line.find(part, at);
            if (f == std::string::npos) { ok = false; break; }
            at = f + part.size();
        }
        if (ok) return line;
        pos = e + 1;
    }
    return "";
}

// Poll `file` every 100 ms until a line matches `pattern` or `deadline_ms` elapsed (one last look at the deadline). Returns the line or "".
static inline std::string waitForLine(const char* file, const std::string& pattern, int deadline_ms) {
    DWORD t0 = GetTickCount();
    for (;;) {
        std::string line = wlFindLine(wlReadFile(file), pattern);
        if (!line.empty()) return line;
        if ((int)(GetTickCount() - t0) >= deadline_ms) return "";
        Sleep(100);
    }
}
