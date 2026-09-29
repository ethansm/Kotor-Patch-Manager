// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.scalar.Scalar;
import ghidra.app.decompiler.*;
import java.util.*;
public class InvVtabAndRefs extends GhidraScript {
    public void run() throws Exception {
        long[][] vt = {{0x009a72acL}, {0x009a88a4L}};
        String[] names = {"CSWGuiInGameInventory","CSWGuiUpgrade"};
        long[][] slots = new long[2][29];
        for (int k=0;k<2;k++) for (int i=0;i<29;i++) {
            slots[k][i] = getInt(toAddr(vt[k][0]+4*i)) & 0xffffffffL;
        }
        for (int i=0;i<29;i++) {
            Function f = getFunctionAt(toAddr(slots[0][i]));
            println(String.format("slot %2d inv=%08x (%s size=%s) upg=%08x %s", i, slots[0][i], f==null?"?":f.getName(), f==null?"?":""+f.getBody().getNumAddresses(), slots[1][i], slots[0][i]==slots[1][i]?"SAME":"DIFF"));
        }
        // scalar-operand search for 0x3d34 (upgrade item ptr) and 0x3d2c
        long[] targets = {0x3d34L, 0x3d2cL};
        Map<String,Integer> hits = new TreeMap<>();
        InstructionIterator it = currentProgram.getListing().getInstructions(true);
        while (it.hasNext()) { Instruction ins = it.next();
            for (int op=0; op<ins.getNumOperands(); op++) for (Object o : ins.getOpObjects(op)) if (o instanceof Scalar) {
                long v = ((Scalar)o).getUnsignedValue();
                for (long t : targets) if (v==t) { Function f=getFunctionContaining(ins.getAddress());
                    String key=String.format("%x %s", t, f==null?"?":f.getName()+"@"+f.getEntryPoint());
                    hits.merge(key,1,Integer::sum);
                    if (ins.getMnemonicString().startsWith("MOV") && op==0) println("  WRITE? "+ins.getAddress()+" "+ins+"  in "+(f==null?"?":f.getName())); } } }
        for (Map.Entry<String,Integer> e : hits.entrySet()) println("REF "+e.getKey()+" x"+e.getValue());
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        Set<Long> dec = new LinkedHashSet<>();
        for (int i=0;i<29;i++) if (slots[0][i]!=slots[1][i]) dec.add(slots[0][i]);
        for (long a : dec) { Function f=getFunctionAt(toAddr(a)); if (f==null) continue;
            if (f.getBody().getNumAddresses() > 4000) { println("=== SKIP big "+f.getName()); continue; }
            println("=== DECOMP "+f.getName()+" @ "+f.getEntryPoint()+" size="+f.getBody().getNumAddresses());
            DecompileResults r=d.decompileFunction(f,120,monitor); println(r.decompileCompleted()? r.getDecompiledFunction().getC():"(fail)"); }
    }
}
