// Buff Duration HUD Phase 3, R3/R5 (2026-09-29 planning session): RTTI
// keyword search for CSWSObject (and CWorldTimer, as a fallback), to get a
// Steam anchor address before attempting positional clustering toward
// CSWSObject::UpdateEffectList (GOG 6993840, R3 lead) or
// CSWSObject::HasSpellEffectApplied (GOG 7005088, R5 lead). Listing only
// (no auto-decompile of every constructor -- CSWSObject is a base class
// likely referenced very broadly, keep this cheap).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find CSWSObject Class

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

public class FindCSWSObjectClass extends GhidraScript {

    private static final String[] KEYWORDS = { "CSWSObject", "CWorldTimer" };

    @Override
    public void run() throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        for (String kw : KEYWORDS) {
            println("=== Fully-qualified symbols containing '" + kw + "' ===");
            SymbolIterator it = st.getAllSymbols(true);
            int count = 0;
            while (it.hasNext()) {
                Symbol s = it.next();
                String qn = s.getName(true);
                if (qn.contains(kw)) {
                    println("  " + s.getSymbolType() + " " + qn + " @ " + s.getAddress());
                    count++;
                    if (count > 100) { println("  ... truncated"); break; }
                }
            }
            println("Total for " + kw + ": " + count);
            println("");
        }
    }
}
