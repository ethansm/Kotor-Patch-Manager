// Buff Duration HUD UX-redesign session, U1/U2: lightweight RTTI keyword
// search (name-only, no auto-decompile) for the framework's own
// pre-existing GUI widget classes (CSWGuiLabel, CSWGuiControl,
// CSWGuiProgressBar, CSWGuiBorder, CSWGuiImage) -- these are shared,
// framework-wide classes (used by many widgets across the whole game),
// not panel-specific embedded structs, so they're much more likely to
// have real RTTI than CSWGuiMainInterfaceChar/Action/Status did.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Gui Widget Rtti

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

public class FindGuiWidgetRtti extends GhidraScript {

    static final String[] KEYWORDS = {
        "CSWGuiLabel", "CSWGuiControl", "CSWGuiProgressBar", "CSWGuiBorder", "CSWGuiImage"
    };

    @Override
    public void run() throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        for (String kw : KEYWORDS) {
            println("=== '" + kw + "' ===");
            SymbolIterator it = st.getAllSymbols(true);
            int count = 0;
            while (it.hasNext()) {
                Symbol s = it.next();
                String qn = s.getName(true);
                if (qn.contains(kw)) {
                    println("  " + s.getSymbolType() + " " + qn + " @ " + s.getAddress());
                    count++;
                }
            }
            println("  (" + count + " hits)");
        }
    }
}
