// Lists every call to the CRT allocators with the constant size (if any) and whether the result is NULL-tested:
// <outdir>/alloc_sites.tsv (allocator, site, fn, fn_name, size, checked, after). Allocators (Steam 6a522e71):
// operator new 0x919723, _malloc 0x9215E6, _realloc 0x934311, __calloc_impl 0x931AB0 (override: args after outdir,
// name=hexaddr:argIndexOfSize(0-based push order from the call backwards)).
// size: the immediate of the N-th PUSH before the call (walking back <= 8 insns), "var:<operand>" otherwise.
// checked: TEST/CMP/OR on EAX (or on the register EAX was just copied into) within 3 insns after the call.
// after: the 4 insns after the call, for manual judgement. Headless arg: outdir [name=addr:argIdx ...]. Doc 10 Phase 2.
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.scalar.Scalar;
import java.io.*;
import java.util.*;

public class NewNullCheck extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        File root = new File(a[0]);
        LinkedHashMap<String, long[]> allocs = new LinkedHashMap<>();
        if (a.length > 1) {
            for (int k = 1; k < a.length; k++) {
                String[] p = a[k].split("[=:]");
                allocs.put(p[0], new long[] { Long.parseLong(p[1], 16), Integer.parseInt(p[2]) });
            }
        } else {
            allocs.put("operator_new", new long[] { 0x919723, 0 });
            allocs.put("malloc", new long[] { 0x9215E6, 0 });
            allocs.put("realloc", new long[] { 0x934311, 1 });
            allocs.put("calloc", new long[] { 0x931AB0, -1 }); // n*m: report both pushes
        }
        int n = 0;
        try (PrintWriter w = new PrintWriter(new BufferedWriter(new FileWriter(new File(root, "alloc_sites.tsv"))))) {
            w.println("allocator\tsite\tfn\tfn_name\tsize\tchecked\tafter");
            for (Map.Entry<String, long[]> e : allocs.entrySet()) {
                Address tgt = toAddr(e.getValue()[0]);
                int argIdx = (int) e.getValue()[1];
                for (Reference r : getReferencesTo(tgt)) {
                    if (!r.getReferenceType().isCall() && !r.getReferenceType().isJump()) continue;
                    Address site = r.getFromAddress();
                    Instruction ci = getInstructionAt(site);
                    Function f = getFunctionContaining(site);
                    // walk back for pushes
                    List<Instruction> pushes = new ArrayList<>();
                    Instruction p = ci == null ? null : ci.getPrevious();
                    for (int k = 0; p != null && k < 8 && pushes.size() < 3; k++, p = p.getPrevious()) {
                        if (p.getFlowType().isCall() || p.getFlowType().isJump()) break;
                        if (p.getMnemonicString().equals("PUSH")) pushes.add(p);
                    }
                    String size;
                    if (argIdx < 0) size = pushes.size() >= 2 ? opnd(pushes.get(0)) + "*" + opnd(pushes.get(1)) : "?";
                    else size = pushes.size() > argIdx ? opnd(pushes.get(argIdx)) : "?";
                    // look forward: follow the result through register/stack-slot copies (MSVC debug code stores
                    // EAX to [EBP-x] and tests the slot), stop at the first branch after a test or at a call
                    String checked = "no";
                    Set<String> held = new HashSet<>(Collections.singleton("EAX"));
                    StringBuilder after = new StringBuilder();
                    Instruction q = ci == null ? null : ci.getNext();
                    for (int k = 0; q != null && k < 8; k++, q = q.getNext()) {
                        String m = q.getMnemonicString();
                        if (k < 5) after.append(k > 0 ? " ; " : "").append(q);
                        if (q.getFlowType().isCall()) break;
                        if (q.getNumOperands() < 2) { if (q.getFlowType().isJump()) break; continue; }
                        String o0 = q.getDefaultOperandRepresentation(0), o1 = q.getDefaultOperandRepresentation(1);
                        if (m.equals("MOV")) {
                            if (held.contains(o1)) held.add(o0); else held.remove(o0);
                        } else if ((m.equals("TEST") || m.equals("OR")) && held.contains(o0) && o0.equals(o1)) { checked = "yes"; break; }
                        else if (m.equals("CMP") && held.contains(o0) && (o1.equals("0x0") || o1.equals("0"))) { checked = "yes"; break; }
                        else if (m.equals("TEST") && held.contains(o0)) { checked = "yes"; break; }
                    }
                    w.println(e.getKey() + "\t" + String.format("%08x", site.getOffset()) + "\t"
                        + (f == null ? "-" : String.format("%08x", f.getEntryPoint().getOffset())) + "\t" + (f == null ? "-" : f.getName(true))
                        + "\t" + size + "\t" + checked + "\t" + after);
                    n++;
                }
            }
        }
        println("NewNullCheck: " + n + " sites");
    }

    static String opnd(Instruction p) {
        Object[] o = p.getOpObjects(0);
        if (o.length == 1 && o[0] instanceof Scalar) return "0x" + Long.toHexString(((Scalar) o[0]).getUnsignedValue());
        String r = p.getDefaultOperandRepresentation(0);
        // PUSH reg: resolve a MOV reg,imm within the previous 4 insns
        Instruction b = p.getPrevious();
        for (int k = 0; b != null && k < 4; k++, b = b.getPrevious()) {
            if (b.getFlowType().isCall() || b.getFlowType().isJump()) break;
            if (b.getNumOperands() == 2 && b.getDefaultOperandRepresentation(0).equals(r)) {
                Object[] s = b.getOpObjects(1);
                if (b.getMnemonicString().equals("MOV") && s.length == 1 && s[0] instanceof Scalar)
                    return "0x" + Long.toHexString(((Scalar) s[0]).getUnsignedValue());
                break;
            }
        }
        return "var:" + r;
    }
}
