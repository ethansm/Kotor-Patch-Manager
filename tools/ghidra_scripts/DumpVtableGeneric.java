// Generic reusable vtable dumper -- edit TARGET_VFTABLES[] (fully-qualified
// symbol names, e.g. "CSWGuiScene::vftable") to dump any class's vtable
// (function pointer array) with each slot's target function name/address/
// size. Adapted from DumpMapVtables.java (which hardcodes 2 specific
// classes) into a generic N-target version.
//
// @category KOTOR
// @menupath Tools.KOTOR.Dump Vtable Generic

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

import java.util.LinkedHashMap;
import java.util.Map;

public class DumpVtableGeneric extends GhidraScript {

    private static final String[] TARGET_VFTABLES = {
        "CSWSCreature::vftable","CSWSItem::vftable","CSWSPlaceable::vftable",
        "CGameEffectApplierRemover::vftable",
        "CSWSObject::vftable"
    };

    @Override
    public void run() throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        Map<String, Address> found = new LinkedHashMap<>();
        SymbolIterator it = st.getAllSymbols(true);
        while (it.hasNext()) {
            Symbol s = it.next();
            String qn = s.getName(true);
            for (String target : TARGET_VFTABLES) {
                if (qn.equals(target)) {
                    found.put(target, s.getAddress());
                }
            }
        }

        for (String target : TARGET_VFTABLES) {
            Address addr = found.get(target);
            println("=== " + target + (addr != null ? " @ " + addr : " -- NOT FOUND") + " ===");
            if (addr != null) {
                dumpVtable(addr, 60);
            }
            println("");
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
