// Buff Duration HUD Item 5 / Phase A.5: RTTI keyword search for
// CSWGuiMainInterfaceAction on the Steam binary (per the established
// shortcut -- framework-wide/shared HUD classes tend to have real RTTI,
// unlike panel-embedded structs like CSWGuiMainInterfaceChar). Also
// searches CSWGuiQuickPanel (known false-positive name match from the
// GOG address DB, to rule out) and CItemRepository/CSWItem (item-icon
// source candidates) so one run covers the whole lead.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find MainInterfaceAction Rtti

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

public class FindMainInterfaceActionRtti extends GhidraScript {

    static final String[] KEYWORDS = {
        "CSWGuiMainInterfaceAction", "CSWGuiQuickPanel", "CItemRepository", "CSWItem"
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
