// Dumps the CSWGuiInGameMap and CSWGuiMainInterface vtables (function
// pointer arrays) with each entry's target function name/address/size, to
// find the specific resize/layout method containing the K2AspyrMapAspectFix
// hook targets (as opposed to their huge constructors, already examined).
//
// @category KOTOR
// @menupath Tools.KOTOR.Dump Map Vtables

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

public class DumpMapVtables extends GhidraScript {

    @Override
    public void run() throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        SymbolIterator it = st.getAllSymbols(true);
        Address inGameMapVft = null;
        Address mainInterfaceVft = null;
        while (it.hasNext()) {
            Symbol s = it.next();
            String qn = s.getName(true);
            if (qn.equals("CSWGuiInGameMap::vftable")) inGameMapVft = s.getAddress();
            if (qn.equals("CSWGuiMainInterface::vftable")) mainInterfaceVft = s.getAddress();
        }

        if (inGameMapVft != null) {
            println("=== CSWGuiInGameMap::vftable @ " + inGameMapVft + " ===");
            dumpVtable(inGameMapVft, 80);
        } else {
            println("CSWGuiInGameMap::vftable not found by exact name");
        }

        println("");
        if (mainInterfaceVft != null) {
            println("=== CSWGuiMainInterface::vftable @ " + mainInterfaceVft + " ===");
            dumpVtable(mainInterfaceVft, 80);
        } else {
            println("CSWGuiMainInterface::vftable not found by exact name");
        }
    }

    private void dumpVtable(Address start, int maxEntries) throws Exception {
        Memory mem = currentProgram.getMemory();
        Address addr = start;
        for (int i = 0; i < maxEntries; i++) {
            long ptr;
            try {
                ptr = mem.getInt(addr) & 0xFFFFFFFFL;
            } catch (Exception e) {
                println("  [" + i + "] @ " + addr + " -- out of bounds, stopping");
                break;
            }
            Address target;
            try {
                target = currentProgram.getAddressFactory().getAddress(String.format("0x%08x", ptr));
            } catch (Exception e) {
                println("  [" + i + "] @ " + addr + " raw=0x" + Long.toHexString(ptr) + " -- not a valid address, stopping");
                break;
            }
            Function fn = getFunctionAt(target);
            if (fn == null) {
                println("  [" + i + "] @ " + addr + " -> 0x" + Long.toHexString(ptr)
                    + " (no function here -- likely end of vtable)");
                break;
            }
            println("  [" + i + "] @ " + addr + " -> " + fn.getName() + " @ " + fn.getEntryPoint()
                + " size=" + fn.getBody().getNumAddresses());
            addr = addr.add(4);
        }
    }
}
