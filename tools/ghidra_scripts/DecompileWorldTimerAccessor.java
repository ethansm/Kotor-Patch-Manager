// @category KOTOR
// @menupath Tools.KOTOR.Decompile World Timer Accessor
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

public class DecompileWorldTimerAccessor extends GhidraScript {
    @Override
    public void run() throws Exception {
        println("=== RTTI keyword search: WorldTimer / Clock / Calendar ===");
        SymbolTable st = currentProgram.getSymbolTable();
        SymbolIterator it = st.getAllSymbols(true);
        String[] kws = {"WorldTimer", "Calendar", "GameClock", "CExoTimer"};
        while (it.hasNext()) {
            Symbol s = it.next();
            String qname = s.getName(true);
            for (String kw : kws) {
                if (qname.contains(kw)) {
                    println("  " + qname + " @ " + s.getAddress());
                    break;
                }
            }
        }

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            String[] targets = {"0051c350", "0051a920"};
            for (String hex : targets) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) { println("  (no function)"); continue; }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());
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
