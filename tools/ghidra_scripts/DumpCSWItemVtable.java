// Buff Duration HUD Item 5, continued: CSWItem has real RTTI on Steam
// (confirmed by FindMainInterfaceActionRtti.java). Dump its vtable's
// full method list (address + decompiled signature/first lines) to look
// for an icon accessor (GetIcon/Icon field/GetInventoryIcon-shaped
// method) that active item-ability effects could resolve through
// directly, bypassing the quickbar entirely.
//
// @category KOTOR
// @menupath Tools.KOTOR.Dump CSWItem Vtable

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

public class DumpCSWItemVtable extends GhidraScript {

    @Override
    public void run() throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        SymbolIterator it = st.getAllSymbols(true);
        Address vftableAddr = null;
        while (it.hasNext()) {
            Symbol s = it.next();
            if (s.getName(true).equals("CSWItem::vftable")) {
                vftableAddr = s.getAddress();
                break;
            }
        }
        if (vftableAddr == null) {
            println("CSWItem::vftable symbol not found");
            return;
        }
        println("CSWItem::vftable @ " + vftableAddr);

        Memory mem = currentProgram.getMemory();
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            int slot = 0;
            Address cur = vftableAddr;
            while (slot < 60) {
                int ptr;
                try {
                    ptr = mem.getInt(cur);
                } catch (Exception e) {
                    break;
                }
                Address target = toAddr(ptr & 0xffffffffL);
                Function fn = getFunctionAt(target);
                if (fn == null) {
                    println("  [" + slot + "] " + cur + " -> 0x" + Integer.toHexString(ptr) + "  (no function here, stopping)");
                    break;
                }
                println("  [" + slot + "] " + cur + " -> " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());
                slot++;
                cur = cur.add(4);
            }
        } finally {
            decomp.dispose();
        }
    }
}
