// Verifies the hook2 candidate at 0x004752d8 (DAT_00a30828 read inside
// FUN_004752a0, the console command parser) by dumping the exact 7 raw
// bytes and disassembly, to confirm the shape matches GOG's original
// 7-byte pattern "0F B6 15 D0 98 A2 00" (movzx edx, byte ptr [imm32])
// with only the embedded address differing -- same situation as
// K2AspyrLoadingScreenLineFix's framebuffer-height global.
//
// @category KOTOR
// @menupath Tools.KOTOR.Verify Hook2 Site

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.Memory;

public class VerifyHook2Site extends GhidraScript {

    @Override
    public void run() throws Exception {
        Address site = toAddr(0x004752d8L);
        Memory mem = currentProgram.getMemory();

        byte[] raw = new byte[7];
        mem.getBytes(site, raw);
        StringBuilder sb = new StringBuilder();
        for (byte b : raw) sb.append(String.format("%02X ", b));
        println("Raw 7 bytes at " + site + ": " + sb.toString().trim());

        Function fn = getFunctionContaining(site);
        println("Containing function: " + (fn != null ? fn.getName() + " @ " + fn.getEntryPoint() : "none"));

        println();
        println("=== Disassembly window around 0x004752d8 ===");
        Address start = site.subtract(0x20);
        Address end = site.add(0x30);
        InstructionIterator it = currentProgram.getListing().getInstructions(start, true);
        while (it.hasNext()) {
            Instruction ins = it.next();
            if (ins.getAddress().compareTo(end) > 0) break;
            String marker = ins.getAddress().equals(site) ? "  <-- HOOK2 CANDIDATE" : "";
            println("  " + ins.getAddress() + "  " + ins.toString() + marker);
        }
    }
}
