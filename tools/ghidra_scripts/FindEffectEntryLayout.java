// Follows the "+ %d (Effect_AC_*_Bonus)" debug-format strings (found via
// FindEffectHudClasses.java's keyword search) into their containing
// function(s), to find code that walks a creature's active-effect list
// and reads per-effect fields -- looking for the per-effect struct layout
// and a DurationRemaining-shaped float field, for the Buff Duration HUD
// mod's Target 1 (data layer).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Effect Entry Layout

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

public class FindEffectEntryLayout extends GhidraScript {

    private static final String[] LABEL_KEYWORDS = {
        "Effect_AC_Deflection_Bonus",
        "Effect_AC_Shield_Bonus",
        "Effect_AC_Armor_Bonus",
        "Effect_AC_Natural_Bonus",
        "Effect_AC_Dodge_Bonus",
        "Effect_Attack_Bonus",
        "Effect_Damage_Bonus"
    };

    @Override
    public void run() throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        List<Symbol> labels = new ArrayList<>();
        SymbolIterator it = st.getAllSymbols(true);
        while (it.hasNext()) {
            Symbol s = it.next();
            String qn = s.getName(true);
            for (String kw : LABEL_KEYWORDS) {
                if (qn.contains(kw)) { labels.add(s); break; }
            }
        }
        println("Found " + labels.size() + " matching labels");

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            List<Address> seen = new ArrayList<>();
            for (Symbol lbl : labels) {
                println("---- label " + lbl.getName(true) + " @ " + lbl.getAddress() + " ----");
                Reference[] refs = getReferencesTo(lbl.getAddress());
                for (Reference r : refs) {
                    Address from = r.getFromAddress();
                    Function fn = getFunctionContaining(from);
                    println("  ref from " + from + (fn != null ? " in " + fn.getName() + " @ " + fn.getEntryPoint()
                        + " size=" + fn.getBody().getNumAddresses() : " (no function)"));
                    if (fn != null && !seen.contains(fn.getEntryPoint())) {
                        seen.add(fn.getEntryPoint());
                    }
                }
            }

            println("");
            println("=== Decompile + disasm of each distinct containing function ===");
            for (Address entry : seen) {
                Function fn = getFunctionAt(entry);
                println("==== FUNCTION " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses() + " ====");
                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed)");
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
