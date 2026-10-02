// @category KOTOR
// args: loHex-hiHex disp1,disp2,...  -- list instructions in range whose text contains "+ 0xNN]" for each disp (struct field scan)
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
public class ScanFieldRefs extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs()[0].split("-");
        long lo = Long.parseLong(a[0], 16), hi = Long.parseLong(a[1], 16);
        String[] d = getScriptArgs()[1].split(",");
        InstructionIterator it = currentProgram.getListing().getInstructions(toAddr(lo), true);
        while (it.hasNext()) {
            Instruction i = it.next();
            if (i.getAddress().getOffset() > hi) break;
            String s = i.toString();
            for (String x : d) if (s.contains("+ 0x" + x + "]")) {
                Function f = getFunctionContaining(i.getAddress());
                println(i.getAddress() + " " + s + "  [" + (f == null ? "-" : f.getEntryPoint().toString()) + "]");
                break;
            }
        }
    }
}
