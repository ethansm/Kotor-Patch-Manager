// @category KOTOR
// Args: substrings (case-insens) of imported/external symbol names (e.g. GetCursorPos DirectInput8Create).
// Prints each matching symbol, its address (IAT slot / thunk), and every reference with containing function.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.util.*;
public class ListImportXrefs extends GhidraScript {
    public void run() throws Exception {
        for (String pat : getScriptArgs()) {
            String p = pat.toLowerCase();
            SymbolIterator it = currentProgram.getSymbolTable().getAllSymbols(false);
            while (it.hasNext()) {
                Symbol s = it.next();
                if (!s.getName().toLowerCase().contains(p)) continue;
                boolean ext = s.isExternal() || s.getAddress().isExternalAddress();
                Function sf = getFunctionAt(s.getAddress());
                boolean thunk = sf != null && sf.isThunk();
                String blk = currentProgram.getMemory().getBlock(s.getAddress()) == null ? "?" : currentProgram.getMemory().getBlock(s.getAddress()).getName();
                if (!(ext || thunk || blk.equals(".idata") || blk.equals(".rdata") && s.getSymbolType() == SymbolType.LABEL && false)) continue;
                println("SYM " + s.getName(true) + " @ " + s.getAddress() + " blk=" + blk + " ext=" + ext + " thunk=" + thunk);
                for (Reference r : getReferencesTo(s.getAddress())) {
                    Function f = getFunctionContaining(r.getFromAddress());
                    println("   ref " + r.getFromAddress() + " " + r.getReferenceType() + " in " + (f == null ? "?" : f.getName() + "@" + f.getEntryPoint()));
                    // follow one thunk level
                    if (f != null && f.isThunk()) for (Reference r2 : getReferencesTo(f.getEntryPoint())) {
                        Function f2 = getFunctionContaining(r2.getFromAddress());
                        println("      via thunk " + r2.getFromAddress() + " in " + (f2 == null ? "?" : f2.getName() + "@" + f2.getEntryPoint()));
                    }
                }
            }
        }
    }
}
