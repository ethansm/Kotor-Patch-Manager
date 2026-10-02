#!/usr/bin/env python3
"""ATLAS COPY of mods678_research/check_windows.py (2026-10-02): checker logic unchanged; only the paths, OWNERS (+"AT") and the
WINDOWS table differ (the original M6/M7/M8/CA rows are kept so the TOML cross-check and the pairwise-overlap check still see the
shipped windows; the "AT <id>" rows are the engine-AI atlas hook-site catalogue, shipped=False).
Reject list (windows that could not pass) is kept in REJECTED below, outside the table run.

Validate splice windows (Mods 6-8 + companion-ai) against the full-program dump (lesson 166) and the live hooks.

For each window (name, start, length, function entry, skip, consumed exit) it checks, from the dump's <fn>.asm:
  * the window starts and ends on whole instruction boundaries (length is the sum of whole instructions)
  * length >= 5
  * no relative branch/call (E8/E9/EB/7x/0F 8x) INSIDE the window (a re-executed copy would land wrong),
    unless the window is a skip_original_bytes=true window (bytes never re-run) -> reported as 'skip-only'
  * no branch from OUTSIDE the window targets an address strictly inside it (start is fine, it lands on our jump)
  * a RET may only be the LAST stolen instruction (the wrapper restores ESP exactly, so a trailing RET returns normally)
  * no overlap with any live hook window: the snapshot live_hooks_2026-09-30g.tsv AND (read-only, when present) the
    current live patch_config.toml in the game folder; a window's own installed patch id is skipped
  * no overlap with any OTHER window in the table (ranged-ai vs companion-ai vs the inventory windows, trace vs fixes)
  * consumed-exit hooks (framework fact, KotorPatcher wrapper_x86.cpp): EAX must be excluded from the restore so the
    handler's result reaches the TEST, so EAX = the handler result on BOTH exits: the stolen bytes (which run before
    the test) must not touch EAX, and EAX must be dead at the resume address and at the consumed exit (liveness walk
    over fall-through + Jcc/JMP targets inside the function; review SF2 2026-10-01: explicit AND implicit EAX readers
    (CDQ/CWD/CBW/CWDE, one-operand MUL/IMUL/DIV/IDIV, STOS/LODS/SCAS/CMPXCHG, XCHG/IN/OUT, SAHF, PUSHAD, CMOVcc EAX),
    RET (EAX = return value) and a direct CALL to a callee that reads EAX before writing it all count as reads; XOR/SUB
    EAX,EAX are writes; an indirect JMP (switch table) or an unmodelled mnemonic is "unknown" and fails the check)
  * consumed exits and skip-only resumes keep the stack: the ESP depth (forward walk from the function entry; direct
    CALL pops = the callee's RET n, an indirect CALL pops its pending arguments unless an ADD ESP follows) at the
    consumed exit equals the depth at which the wrapper jumps there (window start for skip-only, else window end),
    and for a skip-only window the depth at its end equals the depth at its start
  * TOML cross-check: every hook in the owning patch's source hooks.toml (ranged_ai_src, companion_ai_src,
    inventory_sort_filter_src) has a window here with the same address, length, bytes, skip flag and consumed exit;
    consumed hooks list "eax" in exclude_from_restore and non-consumed hooks do not; windows marked not shipped must
    NOT be in the toml
Prints the original bytes as a TOML-ready list.

Usage: check_windows.py            (runs the built-in table of windows)
"""
import re, sys, os, tomllib

DUMP = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp")
HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.dirname(os.path.dirname(HERE))   # patch_manager_mods (this copy lives in engine_ai_atlas/tools)
LIVE = os.path.join(SRC, "mods678_research", "live_hooks_2026-09-30g.tsv")   # regenerated 2026-09-30 23:58 from the live patch_config.toml (120 hooks: sibling added level-up-override, perf-lab 0.3.0, load-hang-probe 1.4.0)
LIVE_CONFIG = "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/patch_config.toml"  # read only

# window name prefix -> the patch id that owns it (its own installed hooks are not an overlap) and its source toml
OWNERS = {"M6": ("inventory-sort-filter", "inventory_sort_filter_src"), "M8": ("ranged-ai", "ranged_ai_src"),
          "CA": ("companion-ai", "companion_ai_src"), "M7": (None, None), "AT": (None, None)}


def W(name, start, length, entry, skip=False, cexit=None, eax_ret_ok=False, shipped=True):
    """eax_ret_ok: the consumed exit is the function epilogue and the handler's result IS the return value (by design).
    shipped=False: validated for later use, but deliberately absent from the owner's hooks.toml."""
    return (name, start, length, entry, skip, cexit, eax_ret_ok, shipped)


WINDOWS = [
    # Mod 6 (inventory-sort-filter 0.2.0, live)
    W("M6 tab click entry",         0x8A8CF0, 6,  0x8A8CF0, cexit=0x8A8DE2, eax_ret_ok=True),  # handler result = the click handler's return value
    W("M6 backpack row buffer",     0x8A7E22, 79, 0x8A7BA0, skip=True),
    W("M6 flush before finalise",   0x8A7E76, 22, 0x8A7BA0),
    W("M6 row-select epilogue (re-tint)", 0x8A8478, 6, 0x8A83D0),  # H6d 0.2.0: MOV ESP,EBP; POP EBP; RET 4 (RET is the last stolen insn)
    # Mod 7 (designed, not built)
    W("M7 SetMouseCursor entry",    0x7B3870, 7,  0x7B3870),
    W("M7 hilite placeable colour", 0x7AA747, 10, 0x7AA460),
    W("M7 client update tick",      0x7AC470, 5,  0x7AC470),
    # Mod 8 engine (ranged-ai 0.2.0 live; 0.3.0 changes only the E-H3 handler, same windows)
    W("M8 hold-on-blocked-LOS",     0x6D83EA, 12, 0x6D7390, cexit=0x6D83FF),
    W("M8 retarget gate (IsPC)",    0x6D7C79, 6,  0x6D7390, cexit=0x6D7C8C),   # moved from 0x6D7C73 (stolen MOV ECX,[EAX+..] ran with EAX=handler result)
    W("M8 ranged-stance rules local", 0x6D850F, 6, 0x6D7390, skip=True),     # E-H3: MOV [EBP-0x174],EAX (skip-only; 0.3.0 kite_approach)
    W("M8 stance clamp",            0x77AA5E, 7,  0x77A9E0, cexit=0x77AAB4),
    W("M8 stance row fill",         0x77ACBC, 9,  0x77A9E0, cexit=0x77AE77),   # re-sited 2026-09-30: was 15 bytes ending in MOV EAX,[EBP-0xF8] (EAX rule)
    W("M8 stance click",            0x77D8D4, 6,  0x77D890, cexit=0x77D9CA),   # re-sited 2026-09-30: was 14 bytes incl. MOV EAX,..; SUB EAX,0x410 (EAX rule)
    W("M8 HUD stance default branch", 0x74DBDD, 7, 0x74DA40, skip=True),     # S-H4: MOV [ebp-0x1c],-1 (skip-only; handler writes 0x415 or -1)
    # companion-ai 0.1.0: AI trace (design_ai_trace.md; E4 dropped, the FM1 handler logs instead)
    W("CA T1 ShipBuild value",      0x692C50, 10, 0x692C40),                   # MOV [0xA7E114],1; handler overwrites [ebp-4] (pushed at 0x692C5A)
    W("CA T2 PrintString capture",  0x68F301, 7,  0x68F260, cexit=0x68F39F),   # MOV [EBP-0x10],0 (argc<2 path); exit = success tail
    W("CA T3 AurPostString capture", 0x6F62F2, 7, 0x6F6200),                   # MOV [EBP-0x34],0 after the 4 pops succeeded
    W("CA E1 attack entry (+0x570 reset)", 0x6D740F, 10, 0x6D7390),            # MOV [EAX+0x570],0xBB8, EAX = attacker (EAX restored)
    W("CA E1b approach-branch exit", 0x6D8BE2, 5, 0x6D7390),                   # MOV EAX,[0x9944B8] (return 2)
    W("CA E2 slot-6 fallback fires", 0x569A08, 6, 0x569320),                   # MOV ECX,[EBP-0x194]
    W("CA E3 P1 block-2 UpdatePerception(1)", 0x56BDBD, 8, 0x56B8A0),          # PUSH 1; MOV ECX,[EBP-0xD4]
    # companion-ai 0.1.0: engine fixes (design_ai_engine_fixes.md)
    W("CA A1 approach gate end/retarget", 0x6D853E, 10, 0x6D7390, cexit=0x6D7D2F),  # MOV [EBP-0x16C],0x7F000000
    W("CA P1 throttle interval",    0x56BDB4, 7,  0x56B8A0),                   # CMP [EBP-0x28],0xFA0 (flags read by the JC after it)
    W("CA P1 throttle stamp store", 0x56BDE3, 6,  0x56B8A0),                   # MOV [EAX+0x39C],EDX
    W("CA FM1 bump clear replace",  0x586F43, 10, 0x586B70, skip=True),        # PUSH 1; MOV ECX,[EBP+8]; CALL ClearAllActions (handler calls it)
    W("CA FM4 snapshot FollowInfo", 0x57451F, 6,  0x574500),                   # MOV ECX,[EAX+0x11AC]
    W("CA FM4 restore FollowInfo",  0x575183, 7,  0x574500),                   # MOV EAX,[EBP+0xC]; PUSH EAX; MOV ECX,[EBP+8]
    W("CA A4 NULL spell row",       0x6D9E2A, 15, 0x6D9500, skip=True, cexit=0x6D9EA4),  # MOV EDX,[EBP-0x68]; MOV EAX,[EDX+0x154]; MOV [EBP-0x15C],EAX
    W("CA C1 restart idx (0xC)",    0x5868B9, 7,  0x5867E0, skip=True),        # MOV [EBP-0x10],0
    W("CA C1 restart idx (0xF)",    0x5868F1, 7,  0x5867E0, skip=True),
    W("CA C1 restart idx (0x2E)",   0x586929, 7,  0x5867E0, skip=True),
    W("CA A3 dead-target lock clear", 0x6D78B7, 7, 0x6D7390),                  # MOV [EBP-0x3C],0 (lock-clear variant, no consumed exit)
    W("CA FM3 sentinel probe (NOT SHIPPED)", 0x51DBE7, 6, 0x51D790, shipped=False),  # MOV EDX,[0xA1B488]; log-only probe, dropped
                                                                               # from 0.1.0 (wrapper cost in the scheduler's hottest loop, review N2)
    # ---- engine-AI atlas hook-site catalogue (shipped=False) ----
    W("AT FM2", 0x5C2438, 9, 0x5BFC00, cexit=0x5C24C8, shipped=False),   # pathfind FAIL leg: log / fail-fast, skip the 2-4 s wait (consumed)
    W("AT FM2b", 0x5BC5F4, 9, 0x5BC1D0, shipped=False),   # action 0x11 re-approach counter (other area or !inRange; no LOS); reco
    W("AT FM2c", 0x55B69A, 6, 0x55AC80, shipped=False),   # split partial(2) vs fail(3) per request, -1 share (settles FM2b)
    W("AT FM2d", 0x55B625, 13, 0x55AC80, shipped=False),   # partial path granted (once per granted partial); redundant with FM2c
    W("AT FM3s", 0x51DBE1, 6, 0x51D790, shipped=False),   # FM3 sentinel set (once per level per tick); NEVER ship with the retire
    W("AT FM3e", 0x51DD33, 6, 0x51D790, shipped=False),   # FM3 level end: LOST detection
    W("AT FM3r", 0x51E0C9, 6, 0x51E0C0, shipped=False),   # FM3 RemoveObject (cursor fix / lost-by-remove)
    W("AT FM5a", 0x5C8BC2, 6, 0x5C87E0, shipped=False),   # OnBlocked post, follow state 2 (throttle/neuter by retype-to-0)
    W("AT FM5b", 0x5C9F9D, 5, 0x5C8C20, shipped=False),   # OnBlocked post, follow state 3
    W("AT FM5c", 0x5C3D8B, 5, 0x5C3460, shipped=False),   # OnBlocked post, walk (WalkUpdateLocation)
    W("AT FM5d", 0x5C124E, 6, 0x5BFC00, shipped=False),   # OnBlocked post, MoveToPoint door case (0x5C1240)
    W("AT FM5e", 0x5C5A76, 5, 0x5C3F70, shipped=False),   # OnBlocked post (0x5C5A68, fn 5C3F70)
    W("AT FM5f", 0x5CCBC2, 6, 0x5CC870, shipped=False),   # OnBlocked post (0x5CCBB4, fn 5CC870)
    W("AT P2", 0x56BE79, 6, 0x56B8A0, shipped=False),   # party perception cadence (4 s -> shorter) / mode-1 call counter
    W("AT RA-D", 0x5D5C22, 6, 0x5D56B0, shipped=False),   # RunActions dispatch: action-type histogram
    W("AT RA-R", 0x5D6A60, 9, 0x5D56B0, shipped=False),   # RunActions result x type histogram
    W("AT PF", 0x58A35E, 6, 0x58A330, shipped=False),   # party-flag census (writers of +0x11AC; transient (0,1)/(old,1) pairs i
    W("AT SHOUT-RX", 0x57074F, 9, 0x56EFD0, shipped=False),   # shout received (event 10 type 7 -> slot 7)
    W("AT SHOUT-TX", 0x547100, 10, 0x547100, shipped=False),   # shout send/attempt (delivered/attempted ratio; abort = 0 deliveries)
    W("AT EV", 0x56EFF9, 6, 0x56EFD0, shipped=False),   # creature EventHandler: every event (histogram)
    W("AT GCA", 0x67B204, 10, 0x67B100, shipped=False),   # GetCurrentAction result probe (expect 3/39/38 during approach/follow)
    W("AT SS1", 0x59D966, 6, 0x59CBA0, shipped=False),   # SetState commit: old -> new state
    W("AT SS2", 0x59E8F4, 10, 0x59E860, shipped=False),   # slot-6 (OnEndRound) run on SetState removal
    W("AT HSE", 0x67E70C, 10, 0x67E670, shipped=False),   # GetHasSpellEffect result
    W("AT FM6", 0x7E6546, 10, 0x7E5DF0, shipped=False),   # leader-change probe (per-slot hold flag +0x70 set)
    W("AT AI1-H3b", 0x55B110, 13, 0x55AC80, shipped=False),   # second partial-granted site (grid closest-result path)
    W("AT AI1-H4", 0x55B3DD, 9, 0x55AC80, shipped=False),   # pathfind 100-call cap hit
    W("AT AI1-H5", 0x55B1C7, 6, 0x55AC80, shipped=False),   # PlotPath FAIL because AI level 0
    W("AT AI1-H6", 0x5CBC1E, 7, 0x5CB430, shipped=False),   # walker blocked-step counter (>5 = silent end)
    W("AT AI1-H7", 0x586B70, 5, 0x586B70, shipped=False),   # BumpFriends entry
    W("AT AI1-H8", 0x586940, 6, 0x586940, shipped=False),   # bump candidate (GetIsCreatureBumpable)
    W("AT AI1-H10", 0x5C1D4A, 7, 0x5BFC00, shipped=False),   # new pathfind request (PI+0x58 = 1)
    W("AT AI1-H11", 0x5C0156, 7, 0x5BFC00, shipped=False),   # k_def_pathfail01 on blocked step (hostile blocker)
    W("AT AI2-SR", 0x58E730, 6, 0x58E730, shipped=False),   # StartRound entry
    W("AT AI2-ER", 0x58E990, 5, 0x58E990, shipped=False),   # EndRound entry
    W("AT AI2-S6", 0x58F48C, 12, 0x58E990, shipped=False),   # slot-6 (OnEndRound) site 1 EndCombatRound
    W("AT AI2-S6c", 0x58F4AD, 12, 0x58E990, cexit=0x58F514, shipped=False),   # slot-6 (OnEndRound) site 1: skip/replace the script run (consumed)
    W("AT AI2-F2", 0x58F3A1, 7, 0x58E990, skip=True, shipped=False),   # head-move gate in EndCombatRound
    W("AT AI2-RA", 0x6DACF0, 6, 0x6DACF0, shipped=False),   # ResolveAttack entry
    W("AT AI2-ROLL-M", 0x6DEC05, 6, 0x6DEA90, shipped=False),   # melee attack roll
    W("AT AI2-ROLL-R", 0x6DB122, 7, 0x6DAF00, shipped=False),   # ranged attack roll
    W("AT AI2-RNG", 0x6D8283, 6, 0x6D7390, shipped=False),   # attack range decision (per attack action; hot in an A1 spin)
    W("AT AI2-RT", 0x57C990, 9, 0x57C990, shipped=False),   # GetNearestVisibleEnemyInRange entry (retarget)
    W("AT AI2-TICK", 0x571030, 9, 0x571030, shipped=False),   # per-creature AIUpdate tick entry
    W("AT AI2-SD", 0x6DCBE0, 5, 0x6DCBE0, shipped=False),   # sub-attack damage delivery batch
    W("AT AI2-DRL", 0x58FD50, 6, 0x58FD50, shipped=False),   # DecrementRoundLength
    W("AT AI2-PZa", 0x58FC60, 6, 0x58FC60, shipped=False),   # round pause (a)
    W("AT AI2-PZb", 0x58FCD0, 6, 0x58FCD0, shipped=False),   # round pause (b)
    W("AT AI2-SS", 0x59E860, 6, 0x59E860, shipped=False),   # OnRemoveSetStateInternal entry (slot-6 site 3)
    W("AT AI2-ENGa", 0x590FF0, 9, 0x590FF0, shipped=False),   # combat-round engine function (a)
    W("AT AI2-ENGb", 0x5912B0, 6, 0x5912B0, shipped=False),   # combat-round engine function (b)
    W("AT AI3-H2", 0x5771D5, 7, 0x576DC0, shipped=False),   # LOS probe in DoPerception (d^2 <= r^2)
    W("AT AI3-H3", 0x579021, 7, 0x578850, shipped=False),   # spot check failed (hiders only)
    W("AT AI3-H4", 0x57883F, 7, 0x577FA0, shipped=False),   # listen check failed
    W("AT AI3-H5", 0x5D0F30, 9, 0x5D0F30, shipped=False),   # UpdatePerception entry (mode histogram)
    W("AT AI3-H6", 0x576DC0, 5, 0x576DC0, shipped=False),   # per-pair perception update entry
    W("AT AI3-H7", 0x58B140, 5, 0x58B140, shipped=False),   # combat-range getter
    W("AT AI4-H04", 0x51D842, 7, 0x51D790, shipped=False),   # scheduler pre-pass gate / counter (n_L+1 per level per tick)
    W("AT AI4-H05", 0x51D815, 5, 0x51D790, shipped=False),   # scheduler level start
    W("AT AI4-H06", 0x51DDA1, 7, 0x51D790, shipped=False),   # scheduler client-sync start
    W("AT AI4-H07", 0x51E151, 7, 0x51E120, shipped=False),   # SetAILevel: level really changes
    W("AT AI4-H10", 0x5D6D07, 6, 0x5D56B0, shipped=False),   # RunActions time (non-pending action)
    W("AT AI4-H11", 0x5D6D87, 12, 0x5D56B0, shipped=False),   # RunActions queue length
    W("AT AI4-H12", 0x5694CA, 8, 0x569320, shipped=False),   # AIUpdate force level 4 (party promotion)
    W("AT AI4-H13", 0x5699CE, 7, 0x569320, shipped=False),   # slot-6 fallback gate (E2 neighbour)
    W("AT AI4-H14", 0x569A72, 10, 0x569320, shipped=False),   # E2 fallback reset (+0x570 := 3000)
    W("AT AI4-H15", 0x56A0A9, 8, 0x569320, shipped=False),   # AIUpdate combat -> level 2
    W("AT AI5-H3", 0x5CA7C3, 6, 0x5CA6A0, shipped=False),   # FM6 consumer: state-5 hold test
    W("AT AI5-H4a", 0x7E7838, 7, 0x7E6CE0, shipped=False),   # exit5m edge (FollowInfo+0x38 := 0)
    W("AT AI5-H4b", 0x7E78AE, 7, 0x7E6CE0, shipped=False),   # enter5m edge (FollowInfo+0x38 := 1)
    W("AT AI5-H5", 0x5BF925, 10, 0x5BF7F0, shipped=False),   # 0x3D follow action engaged (per follower per AI tick)
    W("AT AI5-H6", 0x5C6340, 9, 0x5C6340, shipped=False),   # follow driver entry
    W("AT AI5-H7", 0x5C6CD4, 7, 0x5C6340, shipped=False),   # follow stuck guard (>0x32)
    W("AT AI5-H8", 0x5BD09E, 7, 0x5BCDC0, shipped=False),   # follow-radius re-plan
    W("AT AI5-H9", 0x7E5DF0, 10, 0x7E5DF0, shipped=False),   # SetLeaderBySlot entry
    W("AT AI5-H10", 0x7E8350, 9, 0x7E8350, shipped=False),   # ResetAllFollowData entry (UI path)
    W("AT AI5-H11", 0x5C6ACE, 7, 0x5C6340, shipped=False),   # follow state dispatch (state histogram)
    W("AT AI6-F1", 0x5E13C0, 6, 0x5E13C0, shipped=False),   # GetReputation entry (per perception pair)
    W("AT AI6-F2", 0x5E1671, 6, 0x5E13C0, shipped=False),   # GetReputation result
    W("AT AI6-F3", 0x67C430, 6, 0x67C430, shipped=False),   # GetIsEnemy/Friend/Neutral script commands
    W("AT AI6-F6", 0x6D3330, 6, 0x6D3330, shipped=False),   # CSWSFaction::AddMember
    W("AT AI6-F7", 0x5E1680, 6, 0x5E1680, shipped=False),   # GetAIStateReputation (AI target selection)
    W("AT AI6-F8", 0x693860, 10, 0x693860, shipped=False),   # AdjustReputation-related script command
    W("AT AI6-F9", 0x63A5B0, 10, 0x63A5B0, shipped=False),   # DeliverSpeech entry (SpeakString)
    W("AT AI6-F10", 0x5D51A0, 6, 0x5D51A0, shipped=False),   # Surrender entry
    W("AT AI6-F11", 0x5951A0, 6, 0x5951A0, shipped=False),   # OnApplyFactionEffect (115) entry
    W("AT AI6-F12", 0x66F0A0, 6, 0x66F0A0, shipped=False),   # ChangeFaction/ChangeToStandardFaction
    W("AT AI7-RS", 0x6FD8D0, 6, 0x6FD8D0, shipped=False),   # RunScript entry (slot attribution)
    W("AT AI7-CMD-OUT", 0x66900A, 6, 0x668FD0, shipped=False),   # script command result (every command)
    W("AT AI7-GNC", 0x68433C, 6, 0x683D40, shipped=False),   # GetNearestCreature per scanned object
    W("AT AI7-GNC-IN", 0x683D40, 10, 0x683D40, shipped=False),   # GetNearestCreature/Object entry
    W("AT AI7-TB", 0x6B8BB0, 6, 0x6B8BB0, shipped=False),   # GetTalentBestOrRandom entry
    W("AT AI7-HAS", 0x6B8840, 6, 0x6B8840, shipped=False),   # HasTalent entry
    W("AT AI7-INV", 0x6C0330, 9, 0x6C0330, shipped=False),   # item-spell inventory scan entry
    W("AT AI8-H2", 0x59DEA0, 10, 0x59DE30, shipped=False),   # SetState apply: +0xEC := 0 write
    W("AT AI8-H4", 0x56BB5B, 10, 0x56B8A0, shipped=False),   # heartbeat state re-assert
    W("AT AI8-H5", 0x62C47B, 13, 0x62C360, shipped=False),   # AoE heartbeat script run
    W("AT AI8-H6", 0x62C6D9, 13, 0x62C630, shipped=False),   # AoE enter script run
    W("AT AI8-H7", 0x62C750, 13, 0x62C630, shipped=False),   # AoE exit script run
    W("AT AI8-H8", 0x6D77AC, 8, 0x6D7390, shipped=False),   # attack refused tail (per refused attack tick)
    W("AT AI8-H11", 0x5E5936, 10, 0x5E5510, shipped=False),   # effect list load (IntList overflow)
]

# Windows that could not pass the checker (kept out of WINDOWS so the final run is ALL OK); see hook_catalogue.md.
# (name, start, length, function entry, skip, cexit, checker reason as printed by the failing run)
REJECTED = [
    ("AT EFAPPLY ApplyEffect entry (AI8-H10)", 0x544210, 9, 0x544210, False, None,
     "overlaps live hook 0x544210+9 (TSV and LIVE perf-lab:PerfLabEffectApply) = collides with the live perf-lab hook, do not ship"),
    ("AT AI2-S6 consumed variant (batch AI2 design)", 0x58F48C, 12, 0x58E990, False, 0x58F514,
     "consumed hook whose stolen bytes touch EAX: 0x58f48c MOV EAX,[EBP-0x1A8]; 0x58f492 MOV ECX,[EAX+0xb04]  (replaced by AT AI2-S6c, 0x58F4AD)"),
]


def load_fn(entry):
    bb = "%02x" % ((entry >> 16) & 0xff)
    path = os.path.join(DUMP, bb, "%08x.asm" % entry)
    ins = []
    for l in open(path).read().splitlines():
        m = re.match(r"([0-9a-f]{8})\s+([0-9a-f]+)\s+(.*)", l)
        if m:
            ins.append((int(m.group(1), 16), bytes.fromhex(m.group(2)), m.group(3)))
    return ins


def live_windows():
    w = []
    for l in open(LIVE):
        a, n, ty, f, p = l.rstrip("\n").split("\t")
        w.append((int(a, 16), int(n), p, "TSV " + p + ":" + f))
    return w


def live_config_windows():
    """Current live hooks (read-only). Returns (list, note)."""
    if not os.path.exists(LIVE_CONFIG):
        return [], "live patch_config.toml not found (skipped)"
    d = tomllib.load(open(LIVE_CONFIG, "rb"))
    w = []
    for p in d.get("patches", []):
        for h in p.get("hooks", []):
            w.append((int(h["address"]), len(h["original_bytes"]), p["id"], "LIVE " + p["id"] + ":" + h.get("function", h.get("type", "?"))))
    return w, f"live patch_config.toml: {len(d.get('patches', []))} ids, {len(w)} hooks"


# ---- EAX liveness (review SF2) ------------------------------------------------------------------------------------
EAXRE = r"\b(EAX|AX|AL|AH)\b"
FULL_WRITE = ("MOV", "MOVZX", "MOVSX", "LEA", "POP")
# mnemonics whose ONLY EAX contact is through explicit operands (anything else is modelled below or is "unknown")
EXPLICIT_ONLY = {"MOV", "MOVZX", "MOVSX", "LEA", "PUSH", "POP", "ADD", "ADC", "SUB", "SBB", "AND", "OR", "XOR", "CMP",
                 "TEST", "INC", "DEC", "NEG", "NOT", "SHL", "SHR", "SAR", "SAL", "ROL", "ROR", "RCL", "RCR", "SHLD", "SHRD",
                 "BT", "BTS", "BTR", "BTC", "BSF", "BSR", "NOP", "LEAVE", "CLD", "STD", "CLC", "STC", "CMC", "INT3",
                 "MOVSB", "MOVSW", "MOVSD", "CMPSB", "CMPSW", "CMPSD", "PUSHFD", "POPFD", "PUSHF", "POPF", "WAIT", "FWAIT"}
IMPLICIT_READ = {"CDQ", "CWD", "CBW", "CWDE", "STOSB", "STOSW", "STOSD", "LODSB", "LODSW", "LODSD", "SCASB", "SCASW",
                 "SCASD", "CMPXCHG", "XCHG", "IN", "OUT", "SAHF", "PUSHAD", "PUSHA", "XLAT", "XLATB", "AAA", "AAS", "DAA",
                 "DAS", "AAM", "AAD"}
PARTIAL_WRITE = {"LAHF"}   # writes AH only: not a kill, not a read
_fn_cache, _callee_cache = {}, {}


def mnemonic(t):
    m = re.match(r"(\S+)\s*(.*)", t)
    op = m.group(1).upper().split(".")[0]   # Ghidra writes string-op prefixes as suffixes: MOVSD.REP, STOSD.REPNE
    return op, m.group(2)


def classify(op, args):
    """-> 'read' | 'write' | 'none' | 'unknown' for EAX, ignoring control flow (handled by the walkers)."""
    dst, _, src = args.partition(",")
    dst, src = dst.strip(), src.strip()
    if op in IMPLICIT_READ:
        return "read"
    if op in PARTIAL_WRITE:
        return "none"
    if op in ("MUL", "DIV", "IDIV") or (op == "IMUL" and "," not in args):
        return "read"   # one-operand forms read EDX:EAX / EAX
    if op == "IMUL":
        op = "ADD"      # two/three-operand IMUL behaves like an ALU op on its explicit operands
    if op.startswith("CMOV"):
        return "read" if re.search(EAXRE, args) else "none"   # conditional: dest kept (read) when not taken
    if op.startswith("SET"):
        return "none" if not re.search(EAXRE, dst) or "[" not in dst else "read"
    if op in ("FNSTSW", "FSTSW"):
        return "none" if dst in ("AX",) else ("read" if re.search(EAXRE, dst) else "none")   # writes AX only
    if op.startswith("F"):
        return "read" if re.search(EAXRE, args) else "none"   # x87: EAX only as an address register
    if op in ("XOR", "SUB", "SBB") and dst == "EAX" and src == "EAX":
        return "write" if op != "SBB" else "read"   # XOR/SUB EAX,EAX zero it; SBB EAX,EAX reads CF only, but keep safe
    if op not in EXPLICIT_ONLY:
        return "unknown"
    if re.search(EAXRE, src) or (re.search(EAXRE, dst) and "[" in dst):
        return "read"
    if dst == "EAX" and op in FULL_WRITE:
        return "write"
    if re.search(EAXRE, dst) and op not in FULL_WRITE and op != "PUSH" or op == "PUSH" and re.search(EAXRE, dst):
        return "read"   # ALU on EAX/AX/AL reads it; PUSH EAX reads it
    return "none"


def load_fn_cached(entry):
    if entry not in _fn_cache:
        try:
            _fn_cache[entry] = load_fn(entry)
        except OSError:
            _fn_cache[entry] = None
    return _fn_cache[entry]


def callee_reads_eax(target):
    """True when the callee (from the dump) reads EAX before writing it on some path (e.g. __chkstk, _alloca_probe).
    Its own CALLs count as clobbers and its RETs as 'no read' (a void callee does not consume EAX)."""
    if target in _callee_cache:
        return _callee_cache[target]
    _callee_cache[target] = False
    ins = load_fn_cached(target)
    if not ins:
        return False
    v, _ = walk_eax(ins, target, callee=True)
    _callee_cache[target] = v.startswith("LIVE")
    return _callee_cache[target]


def walk_eax(ins, at, callee=False):
    pos = {a: i for i, (a, b, t) in enumerate(ins)}
    seen, verdicts = set(), []
    def walk(addr, depth):
        while True:
            if addr not in pos: verdicts.append(f"unknown ({addr:#x} outside fn)"); return
            if addr in seen: return
            seen.add(addr)
            i = pos[addr]; a, b, t = ins[i]
            op, args = mnemonic(t)
            if op == "RET":
                if not callee: verdicts.append(f"LIVE (returned by {a:#x} {t})")
                return
            if op == "CALL":
                if re.search(EAXRE, args): verdicts.append(f"LIVE (read by {a:#x} {t})"); return
                mc = re.match(r"0x([0-9a-f]+)$", args.strip())
                if mc and not callee and callee_reads_eax(int(mc.group(1), 16)):
                    verdicts.append(f"LIVE (callee of {a:#x} {t} reads EAX)")
                return   # otherwise a CALL clobbers EAX
            if op == "JMP" and not re.match(r"0x[0-9a-f]+$", args.strip()):
                if re.search(EAXRE, args): verdicts.append(f"LIVE (read by {a:#x} {t})")
                else: verdicts.append(f"unknown (indirect jump {a:#x} {t})")
                return
            mj = re.match(r"(J\w+|LOOP\w*)\s+0x([0-9a-f]+)$", t)
            if not mj:
                c = classify(op, args)
                if c == "read": verdicts.append(f"LIVE (read by {a:#x} {t})"); return
                if c == "write": return
                if c == "unknown": verdicts.append(f"unknown (unmodelled {a:#x} {t})"); return
            else:
                if op.startswith("LOOP") or op == "JECXZ": pass
                if depth > 40: verdicts.append("unknown (depth)"); return
                walk(int(mj.group(2), 16), depth + 1)
                if mj.group(1) == "JMP": return
            if i + 1 >= len(ins): verdicts.append("unknown (end)"); return
            addr = ins[i + 1][0]
    walk(at, 0)
    if not verdicts: return "dead on all paths", False
    only_ret = all(v.startswith("LIVE (returned") for v in verdicts)
    return "; ".join(verdicts), only_ret


def eax_state(ins, at):
    """'dead on all paths' only if on EVERY path EAX is fully written before any (explicit or implicit) read.
    Returns (verdict, only_ret) where only_ret is True when every live verdict is a RET."""
    return walk_eax(ins, at)


# ---- ESP depth (review SF2: the consumed exit must be reached at the same stack depth) --------------------------------
# A full-function depth walk cannot attribute arguments around indirect (vtable) calls nested inside an argument push
# sequence, so the check is local: pending(X) = bytes already pushed at X that a later CALL will consume, found by
# walking forward from X to the next CALL whose argument size is known (direct: the callee's RET n, or the ADD ESP,n
# that follows a cdecl call; indirect: only the ADD ESP,n form). Reaching the epilogue (MOV ESP,EBP / LEAVE) first
# means ESP is re-derived from EBP, so any depth is safe there ("reset"). Every explored path must agree.
def callee_pops(target):
    ins = load_fn_cached(target)
    if not ins:
        return None
    pops = set()
    for a, b, t in ins:
        op, args = mnemonic(t)
        if op == "RET":
            m = re.match(r"0x([0-9a-f]+)", args.strip())
            pops.add(int(m.group(1), 16) if m else 0)
    return pops.pop() if len(pops) == 1 else None


def call_args(ins, i):
    """Bytes of arguments the CALL at index i consumes (callee pops or caller's ADD ESP), or None if not determinable."""
    a, b, t = ins[i]
    op, args = mnemonic(t)
    nop, nargs = mnemonic(ins[i + 1][2]) if i + 1 < len(ins) else ("", "")
    cdecl = None
    m = re.match(r"ESP,(0x[0-9a-f]+|\d+)$", nargs.replace(" ", "")) if nop == "ADD" else None
    if m:
        cdecl = int(m.group(1), 0)
    mc = re.match(r"0x([0-9a-f]+)$", args.strip())
    if mc:
        pops = callee_pops(int(mc.group(1), 16))
        if pops is not None:
            return pops + (cdecl or 0)
    return cdecl


def pending_at(ins, x, limit=80):
    """-> (set of results, notes). Results: int pending bytes, 'reset' (epilogue first), 'ret'."""
    pos = {a: i for i, (a, b, t) in enumerate(ins)}
    results, notes, seen = set(), [], set()
    work = [(x, 0, 0)]
    while work:
        addr, pushed, steps = work.pop()
        while True:
            if addr not in pos or steps > limit:
                notes.append(f"path from {x:#x} not resolved at {addr:#x}"); break
            if (addr, pushed) in seen: break
            seen.add((addr, pushed))
            i = pos[addr]; a, b, t = ins[i]
            op, args = mnemonic(t)
            dst = args.partition(",")[0].strip()
            steps += 1
            if op == "PUSH": pushed += 4
            elif op == "POP": pushed -= 4
            elif op in ("PUSHFD", "PUSHF"): pushed += 4
            elif op in ("POPFD", "POPF"): pushed -= 4
            elif op in ("SUB", "ADD") and dst == "ESP" and re.match(r"ESP,(0x[0-9a-f]+|\d+)$", args.replace(" ", "")):
                k = int(args.replace(" ", "").split(",")[1], 0)
                pushed += k if op == "SUB" else -k   # SUB ESP,n = an argument passed by value (e.g. a Vector)
            elif (op == "MOV" and dst == "ESP" and args.replace(" ", "") == "ESP,EBP") or op == "LEAVE":
                results.add("reset"); break
            elif op == "RET":
                results.add("ret" if pushed == 0 else f"ret with {pushed} pushed"); break
            elif re.search(r"\bESP\b", dst) and "[" not in dst:
                notes.append(f"ESP written at {a:#x} {t}"); break
            elif op == "CALL":
                n = call_args(ins, i)
                if n is None:
                    notes.append(f"argument size of {a:#x} {t} unknown"); break
                if pushed <= n:
                    results.add(n - pushed); break
                pushed -= n   # extra pushes belong to a later call (nested call inside an argument sequence)
                if i + 1 < len(ins) and mnemonic(ins[i + 1][2])[0] == "ADD" and ins[i + 1][2].replace(" ", "").startswith("ADDESP,"):
                    i += 1   # the cdecl cleanup was already counted in n
            mj = re.match(r"(J\w+|LOOP\w*)\s+0x([0-9a-f]+)$", t)
            if mj:
                work.append((int(mj.group(2), 16), pushed, steps))
                if mj.group(1) == "JMP": break
            elif op == "JMP":
                notes.append(f"indirect jump {a:#x}"); break
            if i + 1 >= len(ins): break
            addr = ins[i + 1][0]
    return results, notes


def depth_verdict(ins, x):
    """A single pending value (int) or 'reset'; None when unresolved/inconsistent. Returns (value, text)."""
    res, notes = pending_at(ins, x)
    nums = {r for r in res if isinstance(r, int)}
    txt = f"{sorted(map(str, res))}" + (f" notes={notes[:2]}" if notes else "")
    if len(nums) > 1 or any(isinstance(r, str) and r.startswith("ret with") for r in res):
        return None, txt
    if nums:
        return nums.pop(), txt
    if "reset" in res or "ret" in res:
        return "reset", txt
    return None, txt


def is_rel(b, txt):
    return bool(re.match(r"(J\w+|CALL|LOOP\w*)\s+0x", txt))


def owner_of(name):
    return OWNERS.get(name.split(" ", 1)[0], (None, None))


def toml_crosscheck(checked):
    """checked: {addr: (len, bytes, skip, cexit, name, shipped)} from the asm walk. Returns list of problem strings."""
    problems, lines = [], []
    for prefix, (pid, srcdir) in OWNERS.items():
        if not srcdir:
            continue
        path = os.path.join(SRC, srcdir, "kotor2-steam-aspyr.hooks.toml")
        if not os.path.exists(path):
            problems.append(f"{pid}: {path} missing"); continue
        hooks = tomllib.load(open(path, "rb")).get("hooks", [])
        mine = {a: v for a, v in checked.items() if v[4].startswith(prefix + " ") and v[5]}
        unshipped = {a: v for a, v in checked.items() if v[4].startswith(prefix + " ") and not v[5]}
        seen = set()
        for h in hooks:
            a = int(h["address"]); ob = bytes(h["original_bytes"])
            skip = bool(h.get("skip_original_bytes", False)); cexit = h.get("consumed_exit_address")
            excl = [x.lower() for x in h.get("exclude_from_restore", [])]
            if a in unshipped:
                problems.append(f"{pid}: toml hook {a:#x} ({h.get('function')}) is marked NOT shipped in WINDOWS"); continue
            if a not in mine:
                problems.append(f"{pid}: toml hook {a:#x} ({h.get('function')}) has no window in WINDOWS"); continue
            seen.add(a)
            ln, blob, wskip, wexit, wname, _ = mine[a]
            bad = []
            if ob != blob: bad.append("original_bytes differ from the asm")
            if len(ob) != ln: bad.append(f"length {len(ob)} != window {ln}")
            if skip != wskip: bad.append(f"skip_original_bytes {skip} != window {wskip}")
            if (cexit or None) != wexit: bad.append(f"consumed_exit {cexit} != window {wexit}")
            if cexit and "eax" not in excl: bad.append("consumed hook does not exclude eax from the restore")
            if not cexit and "eax" in excl: bad.append("non-consumed hook excludes eax (the handler result would leak into the game's EAX)")
            if bad: problems.append(f"{pid}: {a:#x} {h.get('function')}: " + "; ".join(bad))
            else: lines.append(f"  toml {pid} {a:#010x} {h.get('function')}: matches window '{wname}'")
        for a, v in mine.items():
            if a not in seen:
                problems.append(f"{pid}: window '{v[4]}' {a:#x} is not in {srcdir}/kotor2-steam-aspyr.hooks.toml")
    return problems, lines


def main():
    live = live_windows()
    cfg, cfg_note = live_config_windows()
    ok = True
    spans = []      # (start, end, name) of every window that resolved
    checked = {}
    print(f"live snapshot: {os.path.basename(LIVE)} ({len(live)} hooks); {cfg_note}")
    if not cfg:
        print("NOTE: only the 09-30g TSV is checked; it predates ranged-ai, character-effects-panel and inventory-sort-filter "
              "0x8A8478 (review N9)")
    for name, start, length, entry, skip, cexit, eax_ret_ok, shipped in WINDOWS:
        ins = load_fn(entry)
        addrs = [a for a, _, _ in ins]
        if start not in addrs:
            print(f"[FAIL] {name}: {start:#x} is not an instruction start in fn {entry:#x}")
            ok = False
            continue
        # walk whole instructions
        total, inside = 0, []
        for a, b, t in ins:
            if a >= start and total < length:
                inside.append((a, b, t))
                total += len(b)
        end = start + total
        problems = []
        if total != length:
            problems.append(f"length {length} is not on an instruction boundary (whole insns = {total})")
        if length < 5:
            problems.append("window < 5 bytes")
        rets = [i for i, (a, b, t) in enumerate(inside) if t.startswith("RET")]
        if rets and rets != [len(inside) - 1]:
            problems.append("RET inside the window is not the last stolen instruction")
        rel_in = [(a, t) for a, b, t in inside if is_rel(b, t)]
        if rel_in and not skip:
            problems.append("relative branch inside window: " + ", ".join(f"{a:#x} {t}" for a, t in rel_in))
        # external targets into the window interior
        for a, b, t in ins:
            m = re.match(r"(J\w+|LOOP\w*)\s+0x([0-9a-f]+)", t)
            if m:
                tg = int(m.group(2), 16)
                if start < tg < end and not (start <= a < end):
                    problems.append(f"external branch {a:#x} -> {tg:#x} lands inside window")
        own_id = owner_of(name)[0]
        for la, ln, pid, lname in live + cfg:
            if own_id and pid == own_id:
                continue  # the window's own installed hooks (window == hook)
            if la < end and start < la + ln:
                problems.append(f"overlaps live hook {la:#x}+{ln} ({lname})")
        if cexit is not None:
            if not skip:
                eaxuse = [f"{a:#x} {t}" for a, b, t in inside if re.search(r"\b(EAX|AX|AL|AH)\b", t)]
                if eaxuse:
                    problems.append("consumed hook whose stolen bytes touch EAX: " + "; ".join(eaxuse))
            if cexit not in addrs:
                problems.append(f"consumed exit {cexit:#x} is not an instruction start in fn {entry:#x}")
            for label, at in (("resume", end), ("consumed exit", cexit)):
                v, only_ret = eax_state(ins, at)
                accepted = v.startswith("dead") or (eax_ret_ok and label == "consumed exit" and only_ret)
                note = "  (accepted: handler result is the return value by design)" if accepted and not v.startswith("dead") else ""
                print(f"        EAX at {label} {at:#x}: {v}{note}")
                if not accepted:
                    problems.append(f"EAX may be live at {label} {at:#x} (EAX = handler result there)")
        if cexit is not None or skip:
            v_start, t_start = depth_verdict(ins, start)
            v_end, t_end = depth_verdict(ins, end)
            # the wrapper jumps to the consumed exit after the stolen bytes ran (stack = vanilla at the window end),
            # or, for a skip-only window, with the stack as at the window start
            v_jump = v_start if skip else v_end
            def compatible(u, v):
                return u is not None and v is not None and (u == v or "reset" in (u, v))
            if cexit is not None:
                v_exit, t_exit = depth_verdict(ins, cexit)
                print(f"        stack: pending at the jump ({'start' if skip else 'end'}) {v_jump}, at consumed exit {cexit:#x} {v_exit} {t_exit}")
                if not compatible(v_jump, v_exit):
                    problems.append(f"stack depth at the consumed exit {cexit:#x} ({v_exit}) != at the jump ({v_jump})")
            if skip:
                print(f"        stack: skip-only pending at start {v_start}, at resume {end:#x} {v_end} {t_end}")
                if not compatible(v_start, v_end):
                    problems.append(f"skip-only window: pending at the resume {end:#x} ({v_end}) != at the start ({v_start})")
        blob = b"".join(b for _, b, _ in inside)
        checked[start] = (total, blob, skip, cexit, name, shipped)
        spans.append((start, end, name))
        status = "OK  " if not problems else "FAIL"
        if problems:
            ok = False
        print(f"[{status}] {name}: {start:#010x} len {total}{' (skip-only)' if skip else ''}"
              f"{f' consumed->{cexit:#010x}' if cexit else ''}  bytes = [{', '.join('0x%02X' % x for x in blob)}]")
        for p in problems:
            print("        ! " + p)
        for a, b, t in inside:
            print(f"        {a:08x}  {b.hex():22} {t}")
    # windows against each other (ranged-ai vs companion-ai vs inventory, trace vs fixes)
    pair_problems = []
    for i in range(len(spans)):
        for j in range(i + 1, len(spans)):
            s1, e1, n1 = spans[i]; s2, e2, n2 = spans[j]
            if s1 < e2 and s2 < e1:
                pair_problems.append(f"window '{n1}' {s1:#x}..{e1:#x} overlaps window '{n2}' {s2:#x}..{e2:#x}")
    print(f"\nPairwise overlap among {len(spans)} windows: " + ("none" if not pair_problems else f"{len(pair_problems)} PROBLEM(S)"))
    for p in pair_problems:
        print("        ! " + p)
    ok &= not pair_problems
    tprob, tlines = toml_crosscheck(checked)
    print("\nTOML cross-check (source hooks.toml vs these windows): " + ("all hooks match" if not tprob else f"{len(tprob)} PROBLEM(S)"))
    for l in tlines:
        print(l)
    for p in tprob:
        print("        ! " + p)
    ok &= not tprob
    print("\nALL OK" if ok else "\nPROBLEMS FOUND")


if __name__ == "__main__":
    main()
