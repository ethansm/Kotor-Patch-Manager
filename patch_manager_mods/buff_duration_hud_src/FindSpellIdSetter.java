// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.symbol.*;
import ghidra.app.decompiler.*;

public class FindSpellIdSetter extends GhidraScript {
    public void run() throws Exception {
        Listing l = currentProgram.getListing();
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        FunctionIterator fi = l.getFunctions(true);
        int n=0;
        while (fi.hasNext()) {
            Function f = fi.next();
            long a = f.getEntryPoint().getOffset();
            if (a < 0x400000 || a > 0x700000) continue;
            if (f.getBody().getNumAddresses() > 40) continue;
            InstructionIterator it = l.getInstructions(f.getBody(), true);
            boolean hit=false;
            while (it.hasNext()) {
                Instruction i = it.next();
                if (!i.getMnemonicString().equals("MOV")) continue;
                if (i.getNumOperands()<2) continue;
                String s = i.getDefaultOperandRepresentation(0);
                if (s.matches(".*\\+ 0x1c\\]")||s.contains("+ 0x1c]")) hit=true;
            }
            if (!hit) continue;
            int callers = 0;
            ReferenceIterator ri = currentProgram.getReferenceManager().getReferencesTo(f.getEntryPoint());
            StringBuilder sb=new StringBuilder();
            while (ri.hasNext()) { Reference r = ri.next(); callers++; if (callers<=8) sb.append(r.getFromAddress()).append(' '); }
            println("CAND "+f.getName()+"@"+f.getEntryPoint()+" size="+f.getBody().getNumAddresses()+" callers="+callers+" : "+sb);
        }
    }
}
