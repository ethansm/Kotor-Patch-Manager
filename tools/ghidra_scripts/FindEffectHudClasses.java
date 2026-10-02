// Searches for RTTI-derived, namespace-qualified class symbols matching
// keywords relevant to the "Buff Duration HUD" custom mod investigation
// (effect-list classes, HUD/render classes), using fully-qualified names
// (Symbol.getName(true)) per the project's established RTTI lookup fix.
// Adapted from FindMapClasses.java -- same technique, different keyword
// set and no assumption that a match is a vftable-only interesting hit
// (also lists non-vftable qualified symbols so plain class-name RTTI
// entries and member functions show up, not just vftables).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Effect HUD Classes

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

import java.util.ArrayList;
import java.util.List;

public class FindEffectHudClasses extends GhidraScript {

    // Edit this list to change what's searched for.
    static final String[] KEYWORDS = { "MainInterfaceChar", "MainInterfaceAction", "MainInterfaceStatus" };

    @Override
    public void run() throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        List<Symbol> vftables = new ArrayList<>();

        for (String kw : KEYWORDS) {
            println("=== Fully-qualified symbols containing '" + kw + "' ===");
            SymbolIterator it = st.getAllSymbols(true);
            int count = 0;
            while (it.hasNext()) {
                Symbol s = it.next();
                String qn = s.getName(true);
                if (qn.contains(kw)) {
                    println("  " + s.getSymbolType() + " " + qn + " @ " + s.getAddress());
                    if (qn.contains("vftable")) {
                        vftables.add(s);
                    }
                    count++;
                    if (count > 400) { println("  ... truncated"); break; }
                }
            }
            println("Total for '" + kw + "': " + count);
            println("");
        }

        println("=== Constructors referencing matched vftables ===");
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            List<Address> seen = new ArrayList<>();
            for (Symbol vft : vftables) {
                println("---- vftable: " + vft.getName(true) + " @ " + vft.getAddress() + " ----");
                Reference[] refs = getReferencesTo(vft.getAddress());
                for (Reference r : refs) {
                    Address from = r.getFromAddress();
                    Function fn = getFunctionContaining(from);
                    println("  ref from " + from + (fn != null ? " in " + fn.getName() + " @ " + fn.getEntryPoint()
                        + " size=" + fn.getBody().getNumAddresses() : " (no function)"));
                }
            }

            println("");
            println("=== Decompile + disasm of each distinct constructor function found above ===");
            for (Symbol vft : vftables) {
                Reference[] refs = getReferencesTo(vft.getAddress());
                for (Reference r : refs) {
                    Function fn = getFunctionContaining(r.getFromAddress());
                    if (fn == null || seen.contains(fn.getEntryPoint())) continue;
                    seen.add(fn.getEntryPoint());

                    println("==== FUNCTION " + fn.getName() + " @ " + fn.getEntryPoint()
                        + " size=" + fn.getBody().getNumAddresses() + " ====");
                    DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                    if (res != null && res.decompileCompleted()) {
                        println(res.getDecompiledFunction().getC());
                    } else {
                        println("  (decompile failed)");
                    }
                    println("  -- disasm --");
                    Listing listing = currentProgram.getListing();
                    InstructionIterator insnIt = listing.getInstructions(fn.getBody(), true);
                    while (insnIt.hasNext()) {
                        Instruction insn = insnIt.next();
                        byte[] bytes = insn.getBytes();
                        StringBuilder hexStr = new StringBuilder();
                        for (byte b : bytes) hexStr.append(String.format("%02x ", b & 0xff));
                        println("    " + insn.getAddress() + "  " + String.format("%-30s", hexStr.toString()) + insn.toString());
                    }
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
