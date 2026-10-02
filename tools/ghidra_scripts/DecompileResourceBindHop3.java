// Final hop: FUN_004da820, called from FUN_0047d070(param_2,0,0,0) as the
// last step of constructing a CAurGUIImageInternal resource object --
// strongest remaining candidate for the literal resref-string-to-bound-
// resource entry point (item 1).
// Also decompiles FUN_00710b40 (the boolean helper FUN_00710a70 wraps)
// for completeness, and dumps RTTI for CAurGUIImage/CAurGUIImageInternal
// if present, since Ghidra auto-resolved these class names from the
// decompile (a new, previously-unknown class pair).
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Resource Bind Hop3

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

public class DecompileResourceBindHop3 extends GhidraScript {

    private static final String[] DECOMPILE_TARGETS = {
        "004da820",
        "00710b40"
    };

    private static final String[] KEYWORDS = {
        "CAurGUIImage"
    };

    @Override
    public void run() throws Exception {
        println("=== RTTI keyword search: CAurGUIImage* ===");
        SymbolTable st = currentProgram.getSymbolTable();
        SymbolIterator it = st.getAllSymbols(true);
        while (it.hasNext()) {
            Symbol s = it.next();
            String qname = s.getName(true);
            for (String kw : KEYWORDS) {
                if (qname.contains(kw)) {
                    println("  " + qname + " @ " + s.getAddress());
                    break;
                }
            }
        }

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : DECOMPILE_TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) {
                    println("  (no function defined at this exact address)");
                    fn = getFunctionContaining(addr);
                    if (fn != null) {
                        println("  containing function: " + fn.getName() + " @ " + fn.getEntryPoint());
                    } else {
                        continue;
                    }
                }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());

                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println("  -- Decompiled C --");
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed: " + (res != null ? res.getErrorMessage() : "null") + ")");
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
