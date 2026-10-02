// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
public class FindStores1c extends GhidraScript {
    public void run() throws Exception {
        Listing l = currentProgram.getListing();
        InstructionIterator it = l.getInstructions(true);
        while (it.hasNext()) {
            Instruction i = it.next();
            long a = i.getAddress().getOffset();
            if (a < 0x401000 || a > 0x800000) continue;
            String m = i.getMnemonicString();
            if (!m.equals("MOV")) continue;
            String s = i.getDefaultOperandRepresentation(0);
            if (s.endsWith("+ 0x1c]") && !s.contains("*")) {
                Function f = getFunctionContaining(i.getAddress());
                println("ST "+i.getAddress()+" "+i+"  in "+(f==null?"?":f.getName()+" sz="+f.getBody().getNumAddresses()));
            }
        }
    }
}
