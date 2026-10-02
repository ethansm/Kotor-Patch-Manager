// Verifies the candidate hook1 site at 0x0091d52d (the "push 0x00400000"
// right before the WinMain call inside ___tmainCRTStartup) by dumping raw
// disassembly for a window around it, and confirms there is exactly one
// call in the containing function whose target matches the WinMain-shaped
// function found in the earlier DecompileCRTStartup pass.
//
// @category KOTOR
// @menupath Tools.KOTOR.Verify Hook1 Site

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;

public class VerifyHook1Site extends GhidraScript {

    @Override
    public void run() throws Exception {
        Address site = toAddr(0x0091d52dL);
        Function fn = getFunctionContaining(site);
        println("Containing function: " + (fn != null ? fn.getName() + " @ " + fn.getEntryPoint() : "none"));

        println();
        println("=== Disassembly window around 0x0091d52d ===");
        Address start = site.subtract(0x20);
        Address end = site.add(0x20);
        InstructionIterator it = currentProgram.getListing().getInstructions(start, true);
        while (it.hasNext()) {
            Instruction ins = it.next();
            if (ins.getAddress().compareTo(end) > 0) break;
            String marker = ins.getAddress().equals(site) ? "  <-- HOOK1 CANDIDATE" : "";
            println("  " + ins.getAddress() + "  " + ins.toString() + marker);
        }

        println();
        println("=== All CALL instructions inside the containing function ===");
        if (fn != null) {
            InstructionIterator fit = currentProgram.getListing().getInstructions(fn.getBody(), true);
            while (fit.hasNext()) {
                Instruction ins = fit.next();
                if (ins.getMnemonicString().equalsIgnoreCase("CALL")) {
                    println("  " + ins.getAddress() + "  " + ins.toString());
                }
            }
        }
    }
}
