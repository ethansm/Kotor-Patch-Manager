// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
public class DisasmArgs extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs(); // entryHex [fromHex toHex]
        Function fn = getFunctionContaining(toAddr(Long.parseLong(a[0],16)));
        long lo = a.length>2? Long.parseLong(a[1],16):0, hi = a.length>2? Long.parseLong(a[2],16):Long.MAX_VALUE;
        InstructionIterator it = currentProgram.getListing().getInstructions(fn.getBody(), true);
        while (it.hasNext()) { Instruction i = it.next(); long o=i.getAddress().getOffset(); if(o<lo||o>hi) continue;
            StringBuilder h=new StringBuilder(); for(byte b:i.getBytes()) h.append(String.format("%02x ",b&0xff));
            println(i.getAddress()+"  "+String.format("%-24s",h)+i); }
    }
}
