// From-scratch investigation for AdditionalConsoleCommands hook2 (console
// history save flag). GOG target: 7 bytes "0F B6 15 D0 98 A2 00" = movzx
// edx, byte ptr [0x00A298D0] -- a GOG-specific global address that won't
// exist as-is on Steam (same situation as K2AspyrLoadingScreenLineFix's
// framebuffer-height global). Two independent approaches in one pass:
//   1. Defined Strings containing "hist" (case-insensitive) -- console
//      history is very likely saved to a file, so a filename/extension
//      string is a plausible anchor to trace forward from.
//   2. Structural byte search for the opcode "0F B6 15" (movzx edx, byte
//      ptr [imm32]) anywhere in .text, reporting containing-function names
//      so the candidate list can be manually narrowed instead of guessing
//      an address.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Console History Flag

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.Memory;

import java.util.*;

public class FindConsoleHistoryFlag extends GhidraScript {

    @Override
    public void run() throws Exception {
        println("=== Step 1: Defined Strings containing 'hist' (case-insensitive) ===");
        DataIterator strs = currentProgram.getListing().getDefinedData(true);
        int found = 0;
        while (strs.hasNext()) {
            Data d = strs.next();
            if (!d.hasStringValue()) continue;
            Object val = d.getValue();
            if (val == null) continue;
            String s = val.toString();
            if (s.toLowerCase().contains("hist")) {
                println("  " + d.getAddress() + "  \"" + s + "\"");
                found++;
            }
        }
        println("  total: " + found);

        println();
        println("=== Step 2: structural search for 'movzx edx, byte ptr [imm32]' (0F B6 15) in .text ===");
        Memory mem = currentProgram.getMemory();
        Address textStart = currentProgram.getAddressFactory().getAddress("0x00401000");
        Address textEnd = currentProgram.getAddressFactory().getAddress("0x009857ff");
        byte[] pattern = new byte[]{0x0F, (byte) 0xB6, 0x15};

        Address cur = textStart;
        int hits = 0;
        Map<String, Integer> byFunction = new LinkedHashMap<>();
        while (cur != null && cur.compareTo(textEnd) < 0) {
            Address hit = mem.findBytes(cur, textEnd, pattern, null, true, getMonitor());
            if (hit == null) break;
            hits++;
            Function fn = getFunctionContaining(hit);
            String key = fn != null ? fn.getName() + "@" + fn.getEntryPoint() : "none@" + hit;
            byFunction.merge(key, 1, Integer::sum);
            cur = hit.add(1);
        }
        println("  total raw hits: " + hits);
        println("  by containing function:");
        byFunction.entrySet().stream()
            .sorted((a, b) -> b.getValue() - a.getValue())
            .forEach(e -> println("    " + e.getValue() + "x  " + e.getKey()));
    }
}
