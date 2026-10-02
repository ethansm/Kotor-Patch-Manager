// Retry: dump code units (not just Instructions) across the known
// reference addresses, plus raw bytes, since the region may be
// undefined/data in Ghidra's listing.
//
// @category KOTOR
// @menupath Tools.KOTOR.Dump Console Init Region 2

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.CodeUnitIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;

public class DumpConsoleInitRegion2 extends GhidraScript {

    @Override
    public void run() throws Exception {
        Address start = toAddr(0x00986700L);
        Address end = toAddr(0x00986820L);

        println("=== code units ===");
        CodeUnitIterator it = currentProgram.getListing().getCodeUnits(start, true);
        int n = 0;
        while (it.hasNext() && n < 60) {
            CodeUnit cu = it.next();
            if (cu.getAddress().compareTo(end) > 0) break;
            Function fn = getFunctionContaining(cu.getAddress());
            println(cu.getAddress() + ": " + cu.toString() + " [" + cu.getClass().getSimpleName() + "]" + (fn != null ? " fn=" + fn.getName() : ""));
            n++;
        }

        println();
        println("=== raw bytes hex dump ===");
        Memory mem = currentProgram.getMemory();
        Address a = start;
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 0x120; i++) {
            byte b = mem.getByte(a.add(i));
            sb.append(String.format("%02x ", b));
            if ((i + 1) % 16 == 0) {
                println(String.format("%08x: ", start.add(i - 15).getOffset()) + sb.toString());
                sb.setLength(0);
            }
        }
        if (sb.length() > 0) println(sb.toString());
    }
}
