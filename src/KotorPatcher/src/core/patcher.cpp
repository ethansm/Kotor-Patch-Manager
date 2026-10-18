#include "patcher.h"
#include "config_reader.h"
#include "log_service.h"
#include "platform.h"
#include "registry.h"
#include "trampoline.h"
#include "wrapper_base.h"

#include <atomic>
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

namespace KotorPatcher {
    static std::vector<void*> g_loadedPatches;
    static std::vector<PatchInfo> g_patches;

    // The directory this patcher was loaded from, which is the game directory the
    // config's paths are relative to. Kept from Initialize so the patch modules it
    // names can be resolved the same way the config itself was.
    static std::string g_moduleDir;

    // REPLACE hooks allocate a code block that must be released at cleanup, and
    // FreeExec needs the length, so the size is tracked alongside the pointer.
    struct CodeBuffer { void* addr; std::size_t size; };
    static std::vector<CodeBuffer> g_allocatedCodeBuffers;

    static bool g_initialized = false;
    static Wrappers::WrapperGeneratorBase* g_wrapperGenerator = nullptr;

    // The config's target_version_sha, kept past InitializePatcher so the log
    // service's session header can name the build the logs came from. The deferred
    // apply runs on a worker after InitializePatcher returned, so it needs it too.
    static std::string g_versionSha;

    // How many modules had their KPatchInit run. ApplyPatches reports it when it
    // aborts: those modules may already hold the registry and have provided
    // interfaces, which stay valid until cleanup even though the apply failed.
    // Atomic because the deferred apply counts on a worker thread.
    static std::atomic<int> g_initCount{0};

    // Forward declaration: DeferredApply (below) calls it, the definition is with
    // the other apply code further down.
    static void PublishSessionInfo();

    // Submit the abort summary to the log service as a WARN from patch "patcher".
    // WARN is always recorded, even with no kplog.ini, so a failed apply leaves
    // patcher_log.txt behind to look at, and a clean run creates no file at all.
    // Register/Channel are idempotent, so doing them lazily here costs nothing on
    // the success path, where this is never called.
    static void SubmitCoreWarn(const char* line) {
        int32_t patch = LogService::Register("patcher");
        if (patch < 0) return;
        int32_t channel = LogService::Channel(patch, "core");
        if (channel < 0) return;
        LogService::Submit(channel, KPLOG_WARN, line, static_cast<uint32_t>(std::strlen(line)));
    }

    // KOTOR1 on Steam ships behind SteamStub DRM: its .text is encrypted on disk and
    // only decrypted in memory by the stub, which runs after our proxy has already
    // loaded us. At load time a hook site therefore still reads as ciphertext and
    // every VerifyBytes would fail. When we detect that, the apply is handed to a worker
    // that waits for decryption instead of giving up. GOG/retail, the non-stubbed
    // Steam builds, and the native Linux ELF decrypt nothing, so their hook sites read
    // correctly at init and this path is never taken.
    static constexpr long kDecryptPollIntervalMs = 15;
    static constexpr long kDecryptTimeoutMs = 30000;

    // Original bytes double as the "is the code readable yet" test: they only match once
    // any on-disk encryption has been undone in memory. Every hook site is checked, not
    // just the first. The stub leaves .rdata as plaintext, so a patch hooking a string
    // there reads as ready at load time and would otherwise mask the still-encrypted
    // .text sites of every other patch. A config of only DLL_ONLY patches has nothing
    // to test and reads as ready.
    static bool AllHookSitesReadable() {
        for (const auto& patch : g_patches) {
            if (patch.type == HookType::DLL_ONLY || patch.originalBytes.empty()) {
                continue;
            }
            if (!Trampoline::VerifyBytes(patch.hookAddress,
                    patch.originalBytes.data(), patch.originalBytes.size())) {
                return false;
            }
        }
        return true;
    }

    // Runs off the loader path when the code is still encrypted at init. Polls until the
    // stub has decrypted .text, then applies normally. The patched functions are
    // gameplay/menu code that runs long after startup, so applying a moment into the game
    // (rather than before its entry point) is safe in practice.
    static void DeferredApply() {
        auto start = std::chrono::steady_clock::now();
        while (!AllHookSitesReadable()) {
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count();
            if (elapsedMs >= kDecryptTimeoutMs) {
                Platform::Log("[KotorPatcher] Timed out waiting for code decryption; game left unpatched\n");
                // No module was loaded, but the header should still list "patcher"'s
                // kpatch.log and the version SHA.
                PublishSessionInfo();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(kDecryptPollIntervalMs));
        }
        Platform::Log("[KotorPatcher] Code decrypted; applying deferred patches\n");
        ApplyPatches();
        PublishSessionInfo();
    }

    bool InitializePatcher() {
        if (g_initialized) return true;

        // Initialize wrapper generator
        g_wrapperGenerator = Wrappers::GetWrapperGenerator();
        if (!g_wrapperGenerator) {
            Platform::Log("[KotorPatcher] Failed to get wrapper generator\n");
            return false;
        }

        char platformMsg[128];
        snprintf(platformMsg, sizeof(platformMsg), "[KotorPatcher] Using wrapper generator: %s\n",
            g_wrapperGenerator->GetPlatformName());
        Platform::Log(platformMsg);

        // Get the directory holding this patcher module; the config sits beside it.
        g_moduleDir = Platform::SelfModuleDir();
        if (g_moduleDir.empty()) {
            Platform::Log("[KotorPatcher] Failed to resolve module directory\n");
            return false;
        }
        const std::string& moduleDir = g_moduleDir;

        // Start the log service and the registry before anything can use them. The
        // directory is the one kplog.ini and the log files live in. This is early on
        // purpose: every later failure path (config parse, apply) can then still leave a
        // WARN in patcher_log.txt, and patches loaded by ApplyPatches find kpatch.log
        // already provided when their KPatchInit runs. Neither call touches the
        // filesystem beyond starting the flush thread, so it is fine at loader time.
        LogService::Init(g_moduleDir);
        Registry::Init();
        // The core is just another provider. This runs outside any KPatchInit, so the
        // registry attributes it to "patcher". InitializePatcher can run again after a
        // failed attempt without CleanupPatcher in between, so skip a second Provide
        // rather than log a spurious duplicate.
        if (!Registry::Require(KPATCH_LOG_IFACE, KPATCH_LOG_VERSION)) {
            Registry::Provide(KPATCH_LOG_IFACE, KPATCH_LOG_VERSION, LogService::Api());
        }

        // moduleDir is a '\' path on Windows, but Win32 file I/O canonicalizes '/'
        // to '\', so appending a '/' separator opens the config on both platforms.
        std::string configPath = moduleDir + "/patch_config.toml";
        char configMsg[512];
        snprintf(configMsg, sizeof(configMsg), "[KotorPatcher] Loading config from: %s\n", configPath.c_str());
        Platform::Log(configMsg);

        std::string versionSha;
        if (!Config::ParseConfig(configPath, g_patches, versionSha)) {
            Platform::Log("[KotorPatcher] ERROR: Failed to parse config\n");
            // Also on disk: with no patches applied, patcher_log.txt is the one place a
            // player can find why nothing happened.
            SubmitCoreWarn("Failed to parse config (patch_config.toml); no patches applied");
            PublishSessionInfo();
            return false;
        }

        snprintf(configMsg, sizeof(configMsg), "[KotorPatcher] Loaded %zu patches from config\n", g_patches.size());
        Platform::Log(configMsg);

        g_versionSha = versionSha;

        // Set environment variable for patch DLLs to read
        if (!versionSha.empty()) {
            if (Platform::SetEnv("KOTOR_VERSION_SHA", versionSha.c_str())) {
                snprintf(configMsg, sizeof(configMsg), "[KotorPatcher] Set KOTOR_VERSION_SHA = %s...\n", versionSha.substr(0, 16).c_str());
                Platform::Log(configMsg);
            } else {
                Platform::Log("[KotorPatcher] WARNING: Failed to set KOTOR_VERSION_SHA environment variable\n");
            }
        } else {
            Platform::Log("[KotorPatcher] WARNING: No version SHA found in config\n");
        }

        // If any hook site is still encrypted (SteamStub), wait for the stub to decrypt
        // on a worker thread instead of failing every VerifyBytes now.
        if (!AllHookSitesReadable()) {
            Platform::Log("[KotorPatcher] Hook site still encrypted (SteamStub?); deferring apply\n");
            try {
                // The session header is written by DeferredApply once it has applied.
                std::thread(DeferredApply).detach();
                g_initialized = true;
                return true;
            } catch (...) {
                // No worker available: fall through to a synchronous apply. VerifyBytes
                // still fails safe if the code really is encrypted.
                Platform::Log("[KotorPatcher] WARNING: could not spawn worker; applying synchronously\n");
            }
        }

        // Apply patches. The session info is published whether or not it succeeded:
        // a failed apply is exactly when the header (which patches initialised, which
        // interfaces exist) is most useful.
        bool applied = ApplyPatches();
        PublishSessionInfo();
        if (!applied) {
            return false;
        }

        g_initialized = true;
        return true;
    }

    // Hand the log service what its session header needs: the build's version SHA and
    // the registry's provider list ("  kpatch.log v1 by patcher\n" ...). Called once
    // the modules have had their KPatchInit, so the list is complete. Files already
    // open get it as a "# session info" block, later ones in their header.
    static void PublishSessionInfo() {
        std::string dump;
        Registry::DumpProvided(dump);
        LogService::SetSessionInfo(g_versionSha, dump);
    }

    void CleanupPatcher() {
        // Order matters:
        //  1. Registry::Close() first: Require() returns null from here on, so no patch
        //     can fetch an interface whose provider is about to be unloaded.
        //  2. Free the wrappers/code buffers and unload the modules. A patch's DETACH
        //     code may still Submit log lines, so the log service stays up for this.
        //  3. LogService::Shutdown(): the bounded final flush and file close. It must
        //     come after the unloads so those late lines make it to disk.
        //  4. Registry::Reset(): forget the provided interfaces and the module handles
        //     seen. Last, because the OS may reuse a handle value for a different
        //     module later, which must then get its own KPatchInit.
        Registry::Close();

        // Free wrapper stubs
        if (g_wrapperGenerator) {
            g_wrapperGenerator->FreeAllWrappers();
        }

        // Free allocated code buffers from REPLACE hooks
        for (const auto& buffer : g_allocatedCodeBuffers) {
            Platform::FreeExec(buffer.addr, buffer.size);
        }
        g_allocatedCodeBuffers.clear();

        // Unload patch DLLs
        for (void* h : g_loadedPatches) {
            Platform::UnloadModule(h);
        }
        g_loadedPatches.clear();
        g_patches.clear();

        LogService::Shutdown();
        Registry::Reset();
        g_initCount = 0;
        g_versionSha.clear();
        g_initialized = false;
    }

    // Forward declarations
    static bool ApplySimpleHook(const PatchInfo& patch);
    static bool ApplyReplaceHook(const PatchInfo& patch);

    bool ApplyPatches() {
        for (const auto& patch : g_patches) {
            if (!ApplyPatch(patch)) {
                // One summary line so the failure is easy to find among the per-patch
                // lines: modules loaded before this one already ran KPatchInit and may
                // have provided interfaces. Those stay valid until CleanupPatcher (no
                // Close here), since the patches that hold them are still loaded.
                char abortMsg[96];
                snprintf(abortMsg, sizeof(abortMsg),
                    "Apply aborted: %d module(s) initialised", g_initCount.load());
                std::string logLine = std::string("[KotorPatcher] ") + abortMsg + "\n";
                Platform::Log(logLine.c_str());
                SubmitCoreWarn(abortMsg);
                return false;
            }
        }
        return true;
    }

    // A patch module's path is written relative to the game directory, which is the
    // directory this patcher sits in. Handing it to the loader as-is would resolve it
    // against the working directory instead, and nothing guarantees what that is: macOS
    // starts an app bundle from "/", so a relative path lands in /patches and the module
    // silently fails to load. An absolute path, or a dyld directive such as
    // @loader_path, is already anchored and passes through untouched.
    static std::string ModulePath(const std::string& path) {
        if (path.empty() || path[0] == '/' || path[0] == '@' || path[0] == '\\') {
            return path;
        }
        // "C:\..." and "C:/..." are anchored too.
        if (path.size() >= 2 && path[1] == ':') {
            return path;
        }
        if (g_moduleDir.empty()) {
            return path;
        }

        // The config writes '/', and SelfModuleDir reports whatever the platform uses.
        // File I/O takes either, but the module loaders are stricter, so the joined path
        // is made to agree with the separator the platform handed back.
        const bool backslash = g_moduleDir.find('\\') != std::string::npos;
        std::string full = g_moduleDir + (backslash ? '\\' : '/') + path;
        if (backslash) {
            for (char& c : full) {
                if (c == '/') {
                    c = '\\';
                }
            }
        }
        return full;
    }

    // The id a module is attributed to in the registry and the logs: the config's
    // `id`, or the dll's file name without directory or extension when the config has
    // none ("patches/foo.dll" -> "foo").
    static std::string ModuleId(const PatchInfo& patch) {
        if (!patch.patchId.empty()) {
            return patch.patchId;
        }
        std::string id = patch.dllPath;
        // The config writes '/', but a hand-written path may use either separator.
        std::size_t slash = id.find_last_of("/\\");
        if (slash != std::string::npos) {
            id.erase(0, slash + 1);
        }
        std::size_t dot = id.find_last_of('.');
        if (dot != std::string::npos && dot > 0) {
            id.erase(dot);
        }
        return id;
    }

    // Load a patch module and give it its KPatchInit. Shared by the DLL_ONLY and
    // DETOUR paths so both register the handle for cleanup and both run the init:
    // KPatchInit runs right after the load and before the caller looks up the hook
    // function, so a module can set itself up first. Returns the handle, or nullptr
    // if the load failed (the caller logs it, in its own words). A module with no
    // KPatchInit is normal and silent; a config that lists the same dll for several
    // hooks loads it several times but initialises it once (the registry keys on the
    // handle).
    static void* LoadPatchModule(const PatchInfo& patch) {
        void* hPatch = Platform::LoadModule(ModulePath(patch.dllPath).c_str());
        if (!hPatch) {
            return nullptr;
        }
        g_loadedPatches.push_back(hPatch);

        const std::string id = ModuleId(patch);
        if (Registry::OnModuleLoaded(hPatch, id.c_str())) {
            ++g_initCount;
        }
        return hPatch;
    }

    bool ApplyPatch(const PatchInfo& patch) {
        // Handle DLL_ONLY patches (load DLL, no hooks)
        if (patch.type == HookType::DLL_ONLY) {
            void* hPatch = LoadPatchModule(patch);
            if (!hPatch) {
                Platform::Log(("[KotorPatcher] Failed to load DLL-only patch: " + patch.dllPath +
                    " (" + Platform::LastLoadError() + ")\n").c_str());
                return false;
            }

            char successMsg[256];
            snprintf(successMsg, sizeof(successMsg), "[KotorPatcher] Loaded DLL-only patch: %s\n", patch.dllPath.c_str());
            Platform::Log(successMsg);
            return true;
        }

        // Handle SIMPLE hooks (no DLL loading)
        if (patch.type == HookType::SIMPLE) {
            return ApplySimpleHook(patch);
        }

        // Handle REPLACE hooks (JMP to code block, no DLL loading)
        if (patch.type == HookType::REPLACE) {
            return ApplyReplaceHook(patch);
        }

        // DETOUR hook - load DLL and create wrapper
        // Load patch DLL
        void* hPatch = LoadPatchModule(patch);
        if (!hPatch) {
            Platform::Log(("[KotorPatcher] Failed to load: " + patch.dllPath +
                " (" + Platform::LastLoadError() + ")\n").c_str());
            return false;
        }
        // Get function address
        void* funcAddr = Platform::GetSymbol(hPatch, patch.functionName.c_str());
        if (!funcAddr) {
            Platform::Log(("[KotorPatcher] Function not found: " + patch.functionName + "\n").c_str());
            return false;
        }

        char addrMsg[256];
        snprintf(addrMsg, sizeof(addrMsg), "[KotorPatcher] Symbol resolved: %s at 0x%08" PRIXPTR "\n",
            patch.functionName.c_str(), reinterpret_cast<uintptr_t>(funcAddr));
        Platform::Log(addrMsg);

        // Verify original bytes
        if (!Trampoline::VerifyBytes(patch.hookAddress, patch.originalBytes.data(), patch.originalBytes.size())) {
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg), "[KotorPatcher] Original bytes mismatch at hookAddress 0x%08" PRIXPTR " - wrong game version?\n", patch.hookAddress);
            Platform::Log(errorMsg);
            return false;
        }

        // Generate wrapper for DETOUR type
        Wrappers::WrapperConfig wrapperConfig;
        wrapperConfig.patchFunction = funcAddr;
        wrapperConfig.hookAddress = patch.hookAddress;
        wrapperConfig.originalBytes = patch.originalBytes;
        wrapperConfig.parameters = patch.parameters;

        char debugMsg[256];
        snprintf(debugMsg, sizeof(debugMsg), "[KotorPatcher] Got %zu original bytes\n", wrapperConfig.originalBytes.size());
        Platform::Log(debugMsg);

        // Map our HookType to WrapperConfig::HookType
        wrapperConfig.type = Wrappers::WrapperConfig::HookType::DETOUR;

        wrapperConfig.preserveRegisters = patch.preserveRegisters;
        wrapperConfig.preserveFlags = patch.preserveFlags;
        wrapperConfig.excludeFromRestore = patch.excludeFromRestore;
        wrapperConfig.skipOriginalBytes = patch.skipOriginalBytes;
        wrapperConfig.consumedExitAddress = patch.consumedExitAddress;
        wrapperConfig.originalFunction = patch.originalFunction;

        char skipMsg[128];
        snprintf(skipMsg, sizeof(skipMsg), "[KotorPatcher] skipOriginalBytes = %s\n",
            patch.skipOriginalBytes ? "true" : "false");
        Platform::Log(skipMsg);

        void* wrapper = g_wrapperGenerator->GenerateWrapper(wrapperConfig);
        if (!wrapper) {
            Platform::Log("[KotorPatcher] Failed to generate wrapper\n");
            return false;
        }

        void* targetAddress = wrapper;

        // Write trampoline to target (either wrapper or direct function)
        if (!Trampoline::WriteJump(patch.hookAddress, targetAddress)) {
            Platform::Log("[KotorPatcher] Failed to write trampoline\n");
            return false;
        }

        // Clear out the remaining bytes with NOPs
        if (!Trampoline::WriteNoOps(patch.hookAddress + 5, patch.originalBytes.size() - 5)) {
            Platform::Log("[KotorPatcher] Failed to write No-Ops after trampoline\n");
            return false;
        }

        char successMsg[256];
        snprintf(successMsg, sizeof(successMsg), "[KotorPatcher] Applied DETOUR hook at 0x%08" PRIXPTR " -> %s\n",
            patch.hookAddress,
            patch.functionName.c_str());
        Platform::Log(successMsg);

        return true;
    }

    // Apply a SIMPLE hook (direct byte replacement)
    static bool ApplySimpleHook(const PatchInfo& patch) {
        // Verify original bytes
        if (!Trampoline::VerifyBytes(patch.hookAddress, patch.originalBytes.data(), patch.originalBytes.size())) {
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg), "[KotorPatcher] Original bytes mismatch at hookAddress 0x%08" PRIXPTR " - wrong game version?\n", patch.hookAddress);
            Platform::Log(errorMsg);
            return false;
        }

        // Overwrite the site with the replacement bytes (same length as originals).
        if (!Platform::WriteCode(reinterpret_cast<void*>(patch.hookAddress),
                patch.replacementBytes.data(), patch.replacementBytes.size())) {
            Platform::Log("[KotorPatcher] Failed to write bytes for SIMPLE hook\n");
            return false;
        }

        char successMsg[256];
        snprintf(successMsg, sizeof(successMsg), "[KotorPatcher] Applied SIMPLE hook at 0x%08" PRIXPTR " (%zu bytes replaced)\n",
            patch.hookAddress,
            patch.replacementBytes.size());
        Platform::Log(successMsg);

        return true;
    }

    // Apply a REPLACE hook (JMP to code block, execute raw assembly, JMP back)
    static bool ApplyReplaceHook(const PatchInfo& patch) {
        // Verify original bytes
        if (!Trampoline::VerifyBytes(patch.hookAddress, patch.originalBytes.data(), patch.originalBytes.size())) {
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg), "[KotorPatcher] Original bytes mismatch at hookAddress 0x%08" PRIXPTR " - wrong game version?\n", patch.hookAddress);
            Platform::Log(errorMsg);
            return false;
        }

        // Calculate buffer size: replacement_bytes + 5-byte return JMP
        std::size_t bufferSize = patch.replacementBytes.size() + 5;

        // Allocate executable memory for replacement code + return JMP
        void* codeBuf = Platform::AllocExec(bufferSize, patch.hookAddress);
        if (!codeBuf) {
            Platform::Log("[KotorPatcher] Failed to allocate memory for REPLACE hook\n");
            return false;
        }

        // Track allocated buffer for cleanup
        g_allocatedCodeBuffers.push_back({ codeBuf, bufferSize });

        // Write replacement bytes to allocated memory (not executable until ProtectExec)
        std::memcpy(codeBuf, patch.replacementBytes.data(), patch.replacementBytes.size());

        // Calculate return address (after original bytes)
        void* returnAddr = reinterpret_cast<void*>(patch.hookAddress + patch.originalBytes.size());

        // Write JMP back to game code at end of replacement bytes
        uint8_t* returnJmp = static_cast<uint8_t*>(codeBuf) + patch.replacementBytes.size();
        int32_t offset = 0;
        if (!Trampoline::ComputeRel32(reinterpret_cast<uintptr_t>(returnJmp),
                                      reinterpret_cast<uintptr_t>(returnAddr), offset)) {
            // AllocExec places the block wherever the kernel likes, which on a 64-bit
            // target can be further than a rel32 from the game image.
            Platform::Log("[KotorPatcher] REPLACE code block is out of rel32 range of the game\n");
            return false;
        }
        *returnJmp = 0xE9;  // JMP opcode
        std::memcpy(returnJmp + 1, &offset, 4);

        if (!Platform::ProtectExec(codeBuf, bufferSize)) {
            Platform::Log("[KotorPatcher] Failed to make the REPLACE code block executable\n");
            return false;
        }

        // Write JMP at hook address to code buffer
        if (!Trampoline::WriteJump(patch.hookAddress, codeBuf)) {
            Platform::Log("[KotorPatcher] Failed to write REPLACE hook JMP\n");
            return false;
        }

        // Write NOPs for remaining bytes (if original_bytes > 5)
        if (patch.originalBytes.size() > 5) {
            if (!Trampoline::WriteNoOps(patch.hookAddress + 5, patch.originalBytes.size() - 5)) {
                Platform::Log("[KotorPatcher] Failed to write NOPs for REPLACE hook\n");
                return false;
            }
        }

        char successMsg[256];
        snprintf(successMsg, sizeof(successMsg), "[KotorPatcher] Applied REPLACE hook at 0x%08" PRIXPTR " (%zu bytes code, %zu bytes replaced)\n",
            patch.hookAddress,
            patch.replacementBytes.size(),
            patch.originalBytes.size());
        Platform::Log(successMsg);

        return true;
    }

    const std::vector<PatchInfo>& GetLoadedPatches() {
        return g_patches;
    }
}
