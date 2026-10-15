# G2.3 skeptic input: cross (17 fids)

## X-T01 (P1): Do guikit::detail::wr (GuiKit.h) and CompanionAI.cpp wr<T> guard the store with only a readability probe (probeRead / rd<T> / readAt), with no probeWrite or writable-protection check, so a committed read-only target would fault?
- **CO-2c-02** v2=CONFIRMED (G1 CONFIRMED) | claimed high memory-safety | wr<T> guards writes with probeRead instead of probeWrite | file patch_manager_mods/companion_ai_src/CompanionAI.cpp fn wr<T>
  - G1 evidence: patch_manager_mods/companion_ai_src/CompanionAI.cpp:103-104 | G1 note: probeRead only; store at :104 unguarded for writability.
  - v2 evidence: patch_manager_mods/companion_ai_src/CompanionAI.cpp:103-104 | v2 notes: 
- **CO-2d-07** v2=CONFIRMED (G1 CONFIRMED) | claimed high memory-safety | GuiKit wr probes readability not writability before store | file patch_manager_mods/_shared_src/GuiKit.h fn guikit::detail::wr
  - G1 evidence: patch_manager_mods/_shared_src/GuiKit.h:125-126 | G1 note: Duplicate of HB-1c-01 (same wr, same lines).
  - v2 evidence: patch_manager_mods/_shared_src/GuiKit.h:125-126 | v2 notes: 
- **HB-1c-01** v2=CONFIRMED (G1 CONFIRMED) | claimed high memory-safety | wr probes readability not writability before direct store | file patch_manager_mods/_shared_src/GuiKit.h fn guikit::detail::wr
  - G1 evidence: patch_manager_mods/_shared_src/GuiKit.h:125-126 | G1 note: Readability probe via rd<T> only; store at :126 has no writability check.
  - v2 evidence: patch_manager_mods/_shared_src/GuiKit.h:125-126 | v2 notes: 
- **SH-3a-02** v2=CONFIRMED (G1 CONFIRMED) | claimed high memory-safety | wr<T> only checks readability before writing; read-only page write faults | file patch_manager_mods/_shared_src/GuiKit.h fn guikit::detail::wr
  - G1 evidence: patch_manager_mods/_shared_src/GuiKit.h:125-126 | G1 note: Duplicate of HB-1c-01 (same wr, same lines).
  - v2 evidence: patch_manager_mods/_shared_src/GuiKit.h:125-126 | v2 notes: 

## X-T02 (P1): Does saferead::readAt compute base+off in 32-bit uintptr_t with no wraparound check, so a high (corrupt) base plus positive off wraps to a low address that passes probeRead's checks and reads an unrelated location?
- **CO-2d-08** v2=CONFIRMED (G1 PARTIAL) | claimed medium memory-safety | readAt base+off 32-bit wraparound bypasses probeRead low-address check | file patch_manager_mods/_shared_src/SafeRead.h fn saferead::readAt
  - G1 evidence: patch_manager_mods/_shared_src/SafeRead.h:101,116 | G1 note: Low-address bypass needs off>0x10000; small struct offsets are always caught.
  - v2 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1665 ; no-hits | v2 notes: [det CONFIRMED] d004: readAt offsets 0x2000C and 0x20000 are at least 0x10000: wrap-bypass site exists in the codebase || [det CONFIRMED] d005: No saferead_test readAt call pins a positive-offset wrap from a high base: test-only gap || [gem2 PARTIAL] u007: Wrap precondition holds here (off 0x20000, peer unchecked); result is compare+counter only.
- **HB-1a-04** v2=CONFIRMED (G1 CONFIRMED) | claimed high memory-safety | readAt does not check base+off 32-bit wraparound before probing | file patch_manager_mods/_shared_src/SafeRead.h fn saferead::readAt
  - G1 evidence: patch_manager_mods/_shared_src/SafeRead.h:116 | G1 note: No wrap check on base+off before probing; the literal gap exists.
  - v2 evidence: patch_manager_mods/_shared_src/SafeRead.h:116 | v2 notes: 
- **HN-6h-06** v2=CONFIRMED (G1 PARTIAL) | claimed medium memory-safety | base+off 32-bit wraparound bypasses probeRead overflow check | file patch_manager_mods/_shared_src/SafeRead.h fn saferead::readAt
  - G1 evidence: patch_manager_mods/_shared_src/SafeRead.h:101,116 | G1 note: Overflow check tests a+len, not base+off; wrap goes unseen by it, impact bounded.
  - v2 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1665 ; no-hits | v2 notes: [det CONFIRMED] d004: readAt offsets 0x2000C and 0x20000 are at least 0x10000: wrap-bypass site exists in the codebase || [det CONFIRMED] d005: No saferead_test readAt call pins a positive-offset wrap from a high base: test-only gap || [gem2 PARTIAL] u007: Impact-bounded part true (no write). Probe overflow-check blindness unverified: SafeRead.h outside range.
- **QH-2a-10** v2=CONFIRMED (G1 PARTIAL) | claimed medium memory-safety | readAt base+off can wrap on 32-bit, bypassing probeRead low-address guard | file patch_manager_mods/_shared_src/SafeRead.h fn saferead::readAt
  - G1 evidence: patch_manager_mods/_shared_src/SafeRead.h:101,116 | G1 note: Same as SH-3a-03: the low guard blocks wraps when off<0x10000.
  - v2 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1665 ; no-hits | v2 notes: [det CONFIRMED] d004: readAt offsets 0x2000C and 0x20000 are at least 0x10000: wrap-bypass site exists in the codebase || [det CONFIRMED] d005: No saferead_test readAt call pins a positive-offset wrap from a high base: test-only gap || [gem2 PARTIAL] u007: Low-guard protection claim does not apply: off 0x20000 exceeds 0x10000, so wrap can escape.
- **SH-3a-03** v2=CONFIRMED (G1 PARTIAL) | claimed high memory-safety | readAt base+off can wrap on 32-bit and bypass low-address probe checks | file patch_manager_mods/_shared_src/SafeRead.h fn saferead::readAt
  - G1 evidence: patch_manager_mods/_shared_src/SafeRead.h:101,116 | G1 note: Wrap occurs, but probeRead rejects wrapped addresses below 0x10000 for off<=0x10000.
  - v2 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1665 ; no-hits | v2 notes: [det CONFIRMED] d004: readAt offsets 0x2000C and 0x20000 are at least 0x10000: wrap-bypass site exists in the codebase || [det CONFIRMED] d005: No saferead_test readAt call pins a positive-offset wrap from a high base: test-only gap || [gem2 PARTIAL] u007: Same as QH-2a-10. Escape needs peer 0xFFFF0000-0xFFFFFFFF; probe acceptance unread (SafeRead.h).

## X-T03 (P2): In embtex::heal, is ++s.heals done before detail::fill and the MaxHeals give-up message logged when heals==MaxHeals even if that same fill succeeded, and does embtex_test.cpp only count the line without checking it follows a successful fill?
- **CO-2d-09** v2=CONFIRMED (G1 CONFIRMED) | claimed low logic | heal() logs giving up on the 20th refill even if that fill succeeded | file patch_manager_mods/_shared_src/EmbedTex.h fn embtex::heal
  - G1 evidence: patch_manager_mods/_shared_src/EmbedTex.h:149,153,155-156 | G1 note: Fill result ignored; give-up log fires regardless; gating it hides the cap signal
  - v2 evidence: patch_manager_mods/_shared_src/EmbedTex.h:149,153,155-156 | v2 notes: 
- **QH-2a-11** v2=CONFIRMED (G1 CONFIRMED) | claimed low logic | heal logs 'giving up' on the 20th refill even when fill succeeded | file patch_manager_mods/_shared_src/EmbedTex.h fn embtex::heal
  - G1 evidence: patch_manager_mods/_shared_src/EmbedTex.h:155-156 | G1 note: Same defect as CO-2d-09; anchor exact at line 155
  - v2 evidence: patch_manager_mods/_shared_src/EmbedTex.h:155-156 | v2 notes: 
- **SH-3a-06** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | heal logs give-up message on 20th refill even when fill succeeded | file patch_manager_mods/_shared_src/EmbedTex.h fn embtex::heal
  - G1 evidence: patch_manager_mods/_shared_src/EmbedTex.h:155-156 | G1 note: Same defect as CO-2d-09; anchor exact at line 155
  - v2 evidence: patch_manager_mods/_shared_src/EmbedTex.h:155-156 | v2 notes: 
- **SH-3c-03** v2=PARTIAL (G1 PARTIAL) | claimed medium test-gap | embtex_test never checks when give-up logs; misses width-change rebuild and null-new paths | file patch_manager_mods/_shared_src/embtex_test.cpp fn main / embtex::heal
  - G1 evidence: patch_manager_mods/_shared_src/embtex_test.cpp:91-93,83-85,121 | G1 note: Give-up count+heals asserted at :92-93; width-change and null-new paths untested in range
  - v2 evidence: patch_manager_mods/_shared_src/embtex_test.cpp:66 | v2 notes: [det PARTIAL] d006: allocation-failed path has no newFn-null test (gap); Tex_W hits at :49 and :66 are layout asserts, not rebuild tests

## X-T04 (P2): Do the failure returns in createLabelFromTemplate (CF_Vtable/CF_Template/CF_Count/CF_Flags) and bindByTag (CF_Vtable before initControl) return 0 after g_gk.alloc (+ labelCtor) without destroying or freeing the allocated control?
- **HB-1c-02** v2=CONFIRMED (G1 CONFIRMED) | claimed medium memory-safety | Engine-heap label leaked when create/bind fails after ctor | file patch_manager_mods/_shared_src/GuiKit.h fn guikit::createLabelFromTemplate / bindByTag
  - G1 evidence: patch_manager_mods/_shared_src/GuiKit.h:167, :174, :176, :179 (create); :551 (bind) | G1 note: Generic destroy unsafe on CF_Vtable (wrong vtable); no free binding in range.
  - v2 evidence: patch_manager_mods/_shared_src/GuiKit.h:167, :174, :176, :179 (create); :551 (bind) | v2 notes: 
- **SH-3a-07** v2=CONFIRMED (G1 PARTIAL) | claimed medium memory-safety | Allocated label leaked on createLabelFromTemplate and bindByTag failure paths | file patch_manager_mods/_shared_src/GuiKit.h fn guikit::createLabelFromTemplate / bindByTag
  - G1 evidence: patch_manager_mods/_shared_src/GuiKit.h:174 (leaks); :556, :562, :563 (not leaked) | G1 note: Right for createLabelFromTemplate:174 only; bindByTag:563 control is listed and returned.
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d007: No label dtor or free helper in K2Addr.h, GameAddr.h or GuiKit.h: failure paths cannot release || [re UNSURE] re.md X-T04: label ctor not located at brief address

## X-T05 (P2): Answer each separately: (a) WindowHook::install returns true when old is set without checking h==hwnd or IsWindow(hwnd); (b) guikit wheelInstall's static subOk is never reset by wheelUninstall, so it reinstalls on a stale hwnd; (c) k2input::Client::connect calls onConnect(false,...) on every attempt with no reportedMissing guard?
- **HB-1c-07** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | WindowHook install ignores recreated HWND when old proc non-null | file patch_manager_mods/_shared_src/WindowHook.h fn kp::WindowHook::install
  - G1 evidence: patch_manager_mods/_shared_src/WindowHook.h:33-36 | G1 note: Returns true whenever old set; no h==hwnd or IsWindow check. Only in-range caller passes stored hwnd.
  - v2 evidence: patch_manager_mods/_shared_src/WindowHook.h:33-36 | v2 notes: 
- **SH-3a-08** v2=PARTIAL (G1 PARTIAL) | claimed medium logic | Stale hwnd reused after game window recreation; new window never subclassed | file patch_manager_mods/_shared_src/WindowHook.h fn kp::WindowHook::install / guikit::wheelInstall
  - G1 evidence: patch_manager_mods/_shared_src/GuiKit.h:686-687 | G1 note: Old set means no gameWindow() re-lookup; recreation trigger is engine knowledge, UNSURE.
  - v2 evidence: patch_manager_mods/gui_input_src/GuiInputProvider.cpp:53,62 (stale reuse); GuiInputProvider.cpp:117 (no re-subclass gate); patch_manager_mods/_shared_src/GuiKit.h:621-628 (standalone re-validates) | v2 notes: [gem2 PARTIAL] u008: Provider returns stale g_gameHwnd (:53,:62); standalone re-validates; subclass-skip needs WindowHook.h || [re UNSURE] re.md X-T05: CreateWindowEx/DestroyWindow callers listed; main-window recreation not confirmed
