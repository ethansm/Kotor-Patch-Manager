// Diagnostic: findBytes() has been returning 0 hits even for byte sequences
// directly confirmed present via disassembly (e.g. "5c 5d 00 00" at
// 0x00747f66). Test findBytes in isolation with known-good inputs to
// determine whether this is a real absence or an API-usage bug.
//
// @category KOTOR
// @menupath Tools.KOTOR.Debug FindBytes

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.mem.Memory;

public class DebugFindBytes extends GhidraScript {

    @Override
    public void run() throws Exception {
        Address knownAddr = currentProgram.getAddressFactory().getAddress("0x00747f66");
        Memory mem = currentProgram.getMemory();
        byte[] raw = new byte[4];
        mem.getBytes(knownAddr, raw);
        StringBuilder hex = new StringBuilder();
        for (byte b : raw) hex.append(String.format("%02x ", b & 0xff));
        println("Raw bytes at " + knownAddr + ": " + hex.toString().trim());

        println("");
        println("Test 1: findBytes from getMinAddress(), pattern '5c 5d 00 00', limit 10");
        Address[] r1 = findBytes(currentProgram.getMinAddress(), "5c 5d 00 00", 10);
        println("  -> " + r1.length + " hits");
        for (Address a : r1) println("     " + a);

        println("Test 2: findBytes from " + knownAddr.subtract(0x100) + " (close to known hit), same pattern, limit 10");
        Address[] r2 = findBytes(knownAddr.subtract(0x100), "5c 5d 00 00", 10);
        println("  -> " + r2.length + " hits");
        for (Address a : r2) println("     " + a);

        println("Test 3: findBytes with NO spaces: '5c5d0000'");
        Address[] r3 = findBytes(currentProgram.getMinAddress(), "5c5d0000", 10);
        println("  -> " + r3.length + " hits");

        println("Test 5: find() with raw byte[] instead of hex string");
        Address single2 = find(currentProgram.getMinAddress(), new byte[] {0x5c, 0x5d, 0x00, 0x00});
        println("  -> " + single2);

        println("Test 6: known-good short pattern from the CONFIRMED hook1 bytes 'c7 85 7c ff ff ff', limit 20");
        Address[] r6 = findBytes(currentProgram.getMinAddress(), "c7 85 7c ff ff ff", 20);
        println("  -> " + r6.length + " hits");
        for (Address a : r6) println("     " + a);
    }
}
