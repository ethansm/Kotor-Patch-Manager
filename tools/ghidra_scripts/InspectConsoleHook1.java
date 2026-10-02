// Dumps disassembly context around the 3 candidate hits for
// AdditionalConsoleCommands hook1 ("push 0x00400000", 5 bytes), and
// cross-references calls to the known ConsoleFunc constructor addresses
// (NoParamConstructor 0x475C90=4675536, StringConstructor 4675696,
// IntConstructor 4675856) within +/-0x400 bytes of each hit, to see
// which candidate sits near a batch of built-in console command
// registrations (per the patch's own README: "shortly after the
// existing console commands are initialized").
//
// @category KOTOR
// @menupath Tools.KOTOR.Inspect Console Hook1

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Function;

public class InspectConsoleHook1 extends GhidraScript {

    @Override
    public void run() throws Exception {
        long[] candidates = new long[]{0x0091d52dL, 0x0092a4ccL, 0x0092a4e6L};
        long[] ctorAddrs = new long[]{0x00475c90L, 0x00475cb0L, 0x00475cd0L}; // guesses, will re-derive from DB below
        // Actually resolve constructor addresses from decimal values found in DB dump:
        // NoParamConstructor 4675536, StringConstructor 4675696, IntConstructor 4675856
        long[] realCtors = new long[]{4675536L, 4675696L, 4675856L};

        for (long c : candidates) {
            Address hit = toAddr(c);
            Function fn = getFunctionContaining(hit);
            println("=== candidate @ " + hit + "  fn=" + (fn != null ? fn.getName() : "none") + " ===");

            // dump disasm from hit-0x60 to hit+0x60
            Address winStart = hit.subtract(0x60);
            Address winEnd = hit.add(0x60);
            InstructionIterator it = currentProgram.getListing().getInstructions(winStart, true);
            while (it.hasNext()) {
                Instruction insn = it.next();
                if (insn.getAddress().compareTo(winEnd) > 0) break;
                String marker = insn.getAddress().equals(hit) ? " <-- HIT" : "";
                println("  " + insn.getAddress() + ": " + insn.toString() + marker);
            }

            // scan a wider window (+/- 0x400) for CALL instructions targeting the ctor addresses
            println("  -- CALLs to known ConsoleFunc ctor addrs within +/-0x400 --");
            Address scanStart = hit.subtract(0x400);
            Address scanEnd = hit.add(0x400);
            InstructionIterator it2 = currentProgram.getListing().getInstructions(scanStart, true);
            int found = 0;
            while (it2.hasNext()) {
                Instruction insn = it2.next();
                if (insn.getAddress().compareTo(scanEnd) > 0) break;
                if (insn.getMnemonicString().equals("CALL")) {
                    Address[] flows = insn.getFlows();
                    for (Address f : flows) {
                        long fo = f.getOffset();
                        for (long rc : realCtors) {
                            if (fo == rc) {
                                println("    " + insn.getAddress() + " CALLs ctor @ " + f);
                                found++;
                            }
                        }
                    }
                }
            }
            println("  ctor-call matches in window: " + found);
            println();
        }
    }
}
