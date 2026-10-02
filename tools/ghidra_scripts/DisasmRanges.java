// @category KOTOR
// args: start-end,start-end (hex) -- raw disassembly with bytes
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
public class DisasmRanges extends GhidraScript {
    public void run() throws Exception {
        for (String s : getScriptArgs()[0].split(",")) {
            String[] p = s.split("-");
            long a = Long.parseLong(p[0], 16), e = Long.parseLong(p[1], 16);
            println("--- " + p[0] + ".." + p[1]);
            Instruction i = getInstructionAt(toAddr(a));
            if (i == null) i = getInstructionAfter(toAddr(a));
            while (i != null && i.getAddress().getOffset() < e) {
                StringBuilder b = new StringBuilder();
                for (byte x : i.getBytes()) b.append(String.format("%02x ", x));
                println(i.getAddress() + "  " + String.format("%-24s", b) + i);
                i = i.getNext();
            }
        }
    }
}
