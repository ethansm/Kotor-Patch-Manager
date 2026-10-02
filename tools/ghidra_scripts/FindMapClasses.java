// Searches for RTTI-derived, namespace-qualified Map-related class symbols
// (vftables, methods) using fully-qualified names, since Ghidra's RTTI/
// Demangler analyzer stores class name in the symbol's namespace, not in
// the raw (unqualified) symbol name. Also finds the constructor function
// for each Map-related vftable found (the function that assigns it) and
// decompiles/disassembles it.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Map Classes

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
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

public class FindMapClasses extends GhidraScript {

    @Override
    public void run() throws Exception {
        println("=== Fully-qualified symbols containing 'Map' ===");
        SymbolTable st = currentProgram.getSymbolTable();
        SymbolIterator it = st.getAllSymbols(true);
        List<Symbol> mapVftables = new ArrayList<>();
        int count = 0;
        while (it.hasNext()) {
            Symbol s = it.next();
            String qn = s.getName(true);
            if (qn.contains("Map")) {
                println("  " + s.getSymbolType() + " " + qn + " @ " + s.getAddress());
                if (qn.contains("vftable")) {
                    mapVftables.add(s);
                }
                count++;
                if (count > 400) { println("  ... truncated"); break; }
            }
        }
        println("Total: " + count);

        println("");
        println("=== Constructors referencing Map-related vftables ===");
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (Symbol vft : mapVftables) {
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
            List<Address> seen = new ArrayList<>();
            for (Symbol vft : mapVftables) {
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
