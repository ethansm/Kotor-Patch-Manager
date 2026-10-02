// Follow-on for AdditionalConsoleCommands hook2. The prior pass found 311
// raw hits for the opcode "0F B6 15" (movzx reg, byte ptr [imm32]) across
// ~190 functions -- too generic by function-count alone. This pass
// decodes the actual referenced global address for every hit and keeps
// only the ones targeting .data (0x9f4000-0xa81f3b) -- a mutable boolean
// config flag almost certainly lives in .data, not .rdata (read-only
// tables/constants) or .text. Also narrows further to single-byte-sized
// data items only (excludes hits that are really part of a larger
// multi-byte read misaligned into this 3-byte prefix).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Console History Flag 2

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.mem.Memory;

import java.util.*;

public class FindConsoleHistoryFlag2 extends GhidraScript {

    @Override
    public void run() throws Exception {
        Memory mem = currentProgram.getMemory();
        Address textStart = currentProgram.getAddressFactory().getAddress("0x00401000");
        Address textEnd = currentProgram.getAddressFactory().getAddress("0x009857ff");
        Address dataStart = currentProgram.getAddressFactory().getAddress("0x009f4000");
        Address dataEnd = currentProgram.getAddressFactory().getAddress("0x00a81f3b");
        byte[] pattern = new byte[]{0x0F, (byte) 0xB6, 0x15};

        Address cur = textStart;
        int hits = 0;
        int inData = 0;
        List<String> report = new ArrayList<>();
        while (cur != null && cur.compareTo(textEnd) < 0) {
            Address hit = mem.findBytes(cur, textEnd, pattern, null, true, getMonitor());
            if (hit == null) break;
            hits++;

            // Read the 4-byte little-endian imm32 operand right after the 3-byte opcode.
            byte[] buf = new byte[4];
            mem.getBytes(hit.add(3), buf);
            long target = (buf[0] & 0xFFL) | ((buf[1] & 0xFFL) << 8) | ((buf[2] & 0xFFL) << 16) | ((buf[3] & 0xFFL) << 24);
            Address targetAddr = null;
            try {
                targetAddr = currentProgram.getAddressFactory().getAddress(String.format("0x%08x", target));
            } catch (Exception e) {
                cur = hit.add(1);
                continue;
            }

            if (targetAddr.compareTo(dataStart) >= 0 && targetAddr.compareTo(dataEnd) <= 0) {
                inData++;
                Function fn = getFunctionContaining(hit);
                String key = fn != null ? fn.getName() + "@" + fn.getEntryPoint() : "none";
                report.add(hit + "  reads [" + targetAddr + "]  in " + key);
            }
            cur = hit.add(1);
        }

        println("Total raw hits: " + hits);
        println("Hits targeting .data (" + dataStart + "-" + dataEnd + "): " + inData);
        println();
        for (String r : report) {
            println("  " + r);
        }
    }
}
