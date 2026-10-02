// RTTI/vtable keyword search (adapted from FindMapClasses.java) for classes
// relevant to the Inventory 3D Viewport mod's render path: CSWGui3DSceneView,
// CSWGuiScene, and related Scene/Gob classes. These are already known-real
// class names (confirmed via the GOG kotor2_gog_aspyr.db address database,
// which has full function tables for them) -- this script looks for their
// RTTI on OUR Steam Aspyr binary, since the GOG database's addresses don't
// transfer directly (no GOG binary on this machine to byte-diff against).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find SceneView Classes

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

public class FindSceneViewClasses extends GhidraScript {

    static final String[] KEYWORDS = { "SceneView", "GuiScene", "CAurScene", "Gob" };

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
                    if (qn.contains("vftable") && !qn.contains("meta_ptr")) {
                        vftables.add(s);
                    }
                    count++;
                    if (count > 300) { println("  ... truncated"); break; }
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
                    if (fn != null && !seen.contains(fn.getEntryPoint())) seen.add(fn.getEntryPoint());
                }
            }

            println("");
            println("=== Decompile + disasm of each distinct constructor function found above ===");
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
