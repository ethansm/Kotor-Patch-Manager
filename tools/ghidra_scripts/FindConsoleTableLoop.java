// Determines the full bounds of the console-command function-pointer
// table (each entry a 4-byte address pointing into the 0x983d00-0x9847xx
// tiny-wrapper region), then finds every reference TO the table's start
// address (not to individual entries) to locate the loop that walks it.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Console Table Loop

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;

public class FindConsoleTableLoop extends GhidraScript {

    private boolean looksLikeWrapperPtr(long v) {
        return v >= 0x00983d00L && v <= 0x00985000L;
    }

    @Override
    public void run() throws Exception {
        Memory mem = currentProgram.getMemory();

        // Walk backward from 0x986700 to find table start
        Address probe = toAddr(0x00986700L);
        Address tableStart = probe;
        while (true) {
            Address prev = tableStart.subtract(4);
            long v = mem.getInt(prev) & 0xFFFFFFFFL;
            if (!looksLikeWrapperPtr(v)) break;
            tableStart = prev;
        }
        // Walk forward from 0x986700 to find table end
        Address tableEnd = probe;
        while (true) {
            long v = mem.getInt(tableEnd) & 0xFFFFFFFFL;
            if (!looksLikeWrapperPtr(v)) break;
            tableEnd = tableEnd.add(4);
        }
        int entryCount = (int) tableEnd.subtract(tableStart) / 4;
        println("Table bounds: " + tableStart + " to " + tableEnd.subtract(4) + "  (" + entryCount + " entries)");

        println();
        println("=== references TO table start " + tableStart + " ===");
        ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(tableStart);
        int n = 0;
        while (refs.hasNext()) {
            Reference r = refs.next();
            Address from = r.getFromAddress();
            Function fn = getFunctionContaining(from);
            println("  ref from " + from + " in fn=" + (fn != null ? fn.getName() + "@" + fn.getEntryPoint() : "none") + "  type=" + r.getReferenceType());
            n++;
        }
        println("total refs to table start: " + n);

        // Also check a few bytes before tableStart in case it's referenced via a slightly offset base
        println();
        println("=== scanning for LEA/MOV immediate loads of table-adjacent addresses (+/- 0x10) ===");
        for (long off = -0x10; off <= 0x10; off += 4) {
            Address candidate = tableStart.add(off);
            ReferenceIterator r2 = currentProgram.getReferenceManager().getReferencesTo(candidate);
            while (r2.hasNext()) {
                Reference r = r2.next();
                Address from = r.getFromAddress();
                Function fn = getFunctionContaining(from);
                println("  candidate base " + candidate + " ref from " + from + " fn=" + (fn != null ? fn.getName() : "none"));
            }
        }
    }
}
